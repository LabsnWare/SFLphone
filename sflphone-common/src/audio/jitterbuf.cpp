/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 2004-2005, Horizon Wimba, Inc.
 *
 * Contributors:
 * Steve Kann <stevek@stevek.com>
 *
 * A license has been granted to Digium (via disclaimer) for the use of
 * this code.
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 */

/*! \file
 *
 * \brief jitterbuf: an application-independent jitterbuffer
 * \author Steve Kann <stevek@stevek.com>
 *
 */

#include "jitterbuf.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "global.h"

/*! define these here, just for ancient compiler systems */
#define JB_LONGMAX 2147483647L
#define JB_LONGMIN (-JB_LONGMAX - 1L)

#define jb_warn(...) (warnf ? warnf(__VA_ARGS__) : (void)0)
#define jb_err(...) (errf ? errf(__VA_ARGS__) : (void)0)
#define jb_dbg(...) (dbgf ? dbgf(__VA_ARGS__) : (void)0)

#ifdef DEEP_DEBUG
#define jb_dbg2(...) (dbgf ? dbgf(__VA_ARGS__) : (void)0)
#else
#define jb_dbg2(...) ((void)0)
#endif

static jb_output_function_t warnf, errf, dbgf;

void jb_setoutput(jb_output_function_t err, jb_output_function_t warn, jb_output_function_t dbg) 
{
	errf = err;
	warnf = warn;
	dbgf = dbg;
}

static void increment_losspct(jitterbuf *jb) 
{
	jb->info.losspct = (100000 + 499 * jb->info.losspct)/500;    
}

static void decrement_losspct(jitterbuf *jb) 
{
	jb->info.losspct = (499 * jb->info.losspct)/500;    
}

void jb_reset(jitterbuf *jb) 
{
	/* only save settings */
	jb_conf s = jb->info.conf;
	memset(jb, 0, sizeof(*jb));
	jb->info.conf = s;

	/* initialize length, using the default value */
	jb->info.current = jb->info.target = jb->info.conf.target_extra = JB_TARGET_EXTRA;
	jb->info.silence_begin_ts = -1; 
}

jitterbuf * jb_new() 
{
	jitterbuf *jb;

	if (!(jb = (jitterbuf *)malloc(sizeof(*jb)))) 
		return NULL;

	jb_reset(jb);

	jb_dbg2("jb_new() = %x\n", jb);
	return jb;
}

void jb_destroy(jitterbuf *jb) 
{
	jb_frame *frame; 
	jb_dbg2("jb_destroy(%x)\n", jb);

	/* free all the frames on the "free list" */
	frame = jb->free;
	while (frame != NULL) {
		jb_frame *next = frame->next;
		free(frame);
		frame = next;
	}

	/* free ourselves! */ 
	free(jb);
}



#if 0
static int longcmp(const void *a, const void *b) 
{
	return *(long *)a - *(long *)b;
}
#endif

/*!	\brief simple history manipulation 
 	\note maybe later we can make the history buckets variable size, or something? */
/* drop parameter determines whether we will drop outliers to minimize
 * delay */
static int history_put(jitterbuf *jb, long ts, long now, long ms) 
{
	long delay = now - (ts - jb->info.resync_offset);
	long threshold = 2 * jb->info.jitter + jb->info.conf.resync_threshold;
	long kicked;

	/* don't add special/negative times to history */
	if (ts <= 0) 
		return 0;

	/* check for drastic change in delay */
	if (jb->info.conf.resync_threshold != -1) {
		if (abs(delay - jb->info.last_delay) > threshold) {
			jb->info.cnt_delay_discont++;
			if (jb->info.cnt_delay_discont > 3) {
				/* resync the jitterbuffer */
				jb->info.cnt_delay_discont = 0;
				jb->hist_ptr = 0;
				jb->hist_maxbuf_valid = 0;

				jb_warn("Resyncing the jb. last_delay %ld, this delay %ld, threshold %ld, new offset %ld\n", jb->info.last_delay, delay, threshold, ts - now);
				jb->info.resync_offset = ts - now;
				jb->info.last_delay = delay = 0; /* after resync, frame is right on time */
			} else {
				return -1;
			}
		} else {
			jb->info.last_delay = delay;
			jb->info.cnt_delay_discont = 0;
		}
	}

	kicked = jb->history[jb->hist_ptr % JB_HISTORY_SZ];

	jb->history[(jb->hist_ptr++) % JB_HISTORY_SZ] = delay;

	/* optimization; the max/min buffers don't need to be recalculated, if this packet's
	 * entry doesn't change them.  This happens if this packet is not involved, _and_ any packet
	 * that got kicked out of the history is also not involved 
	 * We do a number of comparisons, but it's probably still worthwhile, because it will usually
	 * succeed, and should be a lot faster than going through all 500 packets in history */
	if (!jb->hist_maxbuf_valid)
		return 0;

	/* don't do this until we've filled history 
	 * (reduces some edge cases below) */
	if (jb->hist_ptr < JB_HISTORY_SZ)
		goto invalidate;

	/* if the new delay would go into min */
	if (delay < jb->hist_minbuf[JB_HISTORY_MAXBUF_SZ-1])
		goto invalidate;

	/* or max.. */
	if (delay > jb->hist_maxbuf[JB_HISTORY_MAXBUF_SZ-1])
		goto invalidate;

	/* or the kicked delay would be in min */
	if (kicked <= jb->hist_minbuf[JB_HISTORY_MAXBUF_SZ-1]) 
		goto invalidate;

	if (kicked >= jb->hist_maxbuf[JB_HISTORY_MAXBUF_SZ-1]) 
		goto invalidate;

	/* if we got here, we don't need to invalidate, 'cause this delay didn't 
	 * affect things */
	return 0;
	/* end optimization */


invalidate:
	jb->hist_maxbuf_valid = 0;
	return 0;
}

static void history_calc_maxbuf(jitterbuf *jb) 
{
	int i,j;

	if (jb->hist_ptr == 0) 
		return;


	/* initialize maxbuf/minbuf to the latest value */
	for (i=0;i<JB_HISTORY_MAXBUF_SZ;i++) {
/*
 * jb->hist_maxbuf[i] = jb->history[(jb->hist_ptr-1) % JB_HISTORY_SZ];
 * jb->hist_minbuf[i] = jb->history[(jb->hist_ptr-1) % JB_HISTORY_SZ];
 */
		jb->hist_maxbuf[i] = JB_LONGMIN;
		jb->hist_minbuf[i] = JB_LONGMAX;
	}

	/* use insertion sort to populate maxbuf */
	/* we want it to be the top "n" values, in order */

	/* start at the beginning, or JB_HISTORY_SZ frames ago */
	i = (jb->hist_ptr > JB_HISTORY_SZ) ? (jb->hist_ptr - JB_HISTORY_SZ) : 0; 

	for (;i<jb->hist_ptr;i++) {
		long toins = jb->history[i % JB_HISTORY_SZ];

		/* if the maxbuf should get this */
		if (toins > jb->hist_maxbuf[JB_HISTORY_MAXBUF_SZ-1])  {

			/* insertion-sort it into the maxbuf */
			for (j=0;j<JB_HISTORY_MAXBUF_SZ;j++) {
				/* found where it fits */
				if (toins > jb->hist_maxbuf[j]) {
					/* move over */
					memmove(jb->hist_maxbuf + j + 1, jb->hist_maxbuf + j, (JB_HISTORY_MAXBUF_SZ - (j + 1)) * sizeof(jb->hist_maxbuf[0]));
					/* insert */
					jb->hist_maxbuf[j] = toins;

					break;
				}
			}
		}

		/* if the minbuf should get this */
		if (toins < jb->hist_minbuf[JB_HISTORY_MAXBUF_SZ-1])  {

			/* insertion-sort it into the maxbuf */
			for (j=0;j<JB_HISTORY_MAXBUF_SZ;j++) {
				/* found where it fits */
				if (toins < jb->hist_minbuf[j]) {
					/* move over */
					memmove(jb->hist_minbuf + j + 1, jb->hist_minbuf + j, (JB_HISTORY_MAXBUF_SZ - (j + 1)) * sizeof(jb->hist_minbuf[0]));
					/* insert */
					jb->hist_minbuf[j] = toins;

					break;
				}
			}
		}

		if (0) { 
			int k;
			_debug("toins = %ld\n", toins);
			_debug("maxbuf =");
			for (k=0;k<JB_HISTORY_MAXBUF_SZ;k++) 
				_debug("%ld ", jb->hist_maxbuf[k]);
			_debug("\nminbuf =");
			for (k=0;k<JB_HISTORY_MAXBUF_SZ;k++) 
				_debug("%ld ", jb->hist_minbuf[k]);
			_debug("\n");
		}
	}

	jb->hist_maxbuf_valid = 1;
}

static void history_get(jitterbuf *jb) 
{
	long max, min, jitter;
	int idx;
	int count;

	if (!jb->hist_maxbuf_valid) 
		history_calc_maxbuf(jb);

	/* count is how many items in history we're examining */
	count = (jb->hist_ptr < JB_HISTORY_SZ) ? jb->hist_ptr : JB_HISTORY_SZ;

	/* idx is the "n"ths highest/lowest that we'll look for */
	idx = count * JB_HISTORY_DROPPCT / 100;

	/* sanity checks for idx */
	if (idx > (JB_HISTORY_MAXBUF_SZ - 1)) 
		idx = JB_HISTORY_MAXBUF_SZ - 1;

	if (idx < 0) {
		jb->info.min = 0;
		jb->info.jitter = 0;
		return;
	}

	max = jb->hist_maxbuf[idx];
	min = jb->hist_minbuf[idx];

	jitter = max - min;

	/* these debug stmts compare the difference between looking at the absolute jitter, and the
	 * values we get by throwing away the outliers */
	// printf("[%ld] min=%ld, max=%ld, jitter=%ld\n", index, min, max, jitter);
	// printf("[%ld] min=%ld, max=%ld, jitter=%ld\n", 0, jb->hist_minbuf[0], jb->hist_maxbuf[0], jb->hist_maxbuf[0]-jb->hist_minbuf[0]);

	jb->info.min = min;
	jb->info.jitter = jitter;
}

/* returns 1 if frame was inserted into head of queue, 0 otherwise */
static int queue_put(jitterbuf *jb, void *data, const enum jb_frame_type type, long ms, long ts) 
{
	jb_frame *frame;
	jb_frame *p;
	int head = 0;
	long resync_ts = ts - jb->info.resync_offset;

	if ((frame = jb->free)) {
		jb->free = frame->next;
	} else if (!(frame = (jb_frame *)malloc(sizeof(*frame)))) {
		jb_err("cannot allocate frame\n");
		return 0;
	}

	jb->info.frames_cur++;

	frame->data = data;
	frame->ts = resync_ts;
	frame->ms = ms;
	frame->type = type;

	/* 
	 * frames are a circular list, jb-frames points to to the lowest ts, 
	 * jb->frames->prev points to the highest ts
	 */

	if (!jb->frames) {  /* queue is empty */
		jb->frames = frame;
		frame->next = frame;
		frame->prev = frame;
		head = 1;
	} else if (resync_ts < jb->frames->ts) {
		frame->next = jb->frames;
		frame->prev = jb->frames->prev;

		frame->next->prev = frame;
		frame->prev->next = frame;

		/* frame is out of order */
		jb->info.frames_ooo++;

		jb->frames = frame;
		head = 1;
	} else { 
		p = jb->frames;

		/* frame is out of order */
		if (resync_ts < p->prev->ts) jb->info.frames_ooo++;

		while (resync_ts < p->prev->ts && p->prev != jb->frames) 
			p = p->prev;

		frame->next = p;
		frame->prev = p->prev;

		frame->next->prev = frame;
		frame->prev->next = frame;
	}
	return head;
}

static long queue_next(jitterbuf *jb) 
{
	if (jb->frames) 
		return jb->frames->ts;
	else 
		return -1;
}

static long queue_last(jitterbuf *jb) 
{
	if (jb->frames) 
		return jb->frames->prev->ts;
	else 
		return -1;
}

static jb_frame *_queue_get(jitterbuf *jb, long ts, int all) 
{
	jb_frame *frame;
	frame = jb->frames;

	if (!frame)
		return NULL;

	_debug("queue_get: ASK %ld FIRST %ld\n", ts, frame->ts);

	if (all || ts >= frame->ts) {
		/* remove this frame */
		frame->prev->next = frame->next;
		frame->next->prev = frame->prev;

		if (frame->next == frame)
			jb->frames = NULL;
		else
			jb->frames = frame->next;


		/* insert onto "free" single-linked list */
		frame->next = jb->free;
		jb->free = frame;

		jb->info.frames_cur--;

		/* we return the frame pointer, even though it's on free list, 
		 * but caller must copy data */
		return frame;
	} 

	return NULL;
}

static jb_frame *queue_get(jitterbuf *jb, long ts) 
{
	return _queue_get(jb,ts,0);
}

static jb_frame *queue_getall(jitterbuf *jb) 
{
	return _queue_get(jb,0,1);
}

/* some diagnostics */
void jb_dbginfo(jitterbuf *jb) 
{

	_debug("\njb info: fin=%ld fout=%ld flate=%ld flost=%ld fdrop=%ld fcur=%ld\n",
		jb->info.frames_in, jb->info.frames_out, jb->info.frames_late, jb->info.frames_lost, jb->info.frames_dropped, jb->info.frames_cur);
	
	_debug("jitter=%ld current=%ld target=%ld min=%ld sil=%ld len=%ld len/fcur=%ld\n",
		jb->info.jitter, jb->info.current, jb->info.target, jb->info.min, jb->info.silence_begin_ts, jb->info.current - jb->info.min, 
		jb->info.frames_cur ? (jb->info.current - jb->info.min)/jb->info.frames_cur : -8);
	if (jb->info.frames_in > 0) 
		_debug("jb info: Loss PCT = %ld%%, Late PCT = %ld%%\n",
			jb->info.frames_lost * 100/(jb->info.frames_in + jb->info.frames_lost), 
			jb->info.frames_late * 100/jb->info.frames_in);
        _debug("jb info: queue %ld -> %ld.  last_ts %ld (queue len: %ld) last_ms %ld\n\n",
		queue_next(jb), 
		queue_last(jb),
		jb->info.next_voice_ts, 
		queue_last(jb) - queue_next(jb),
		jb->info.last_voice_ms);
}


#ifdef DEEP_DEBUG
static void jb_chkqueue(jitterbuf *jb) 
{
	int i=0;
	jb_frame *p = jb->frames;

	if (!p) {
		return;
	}

	do {
		if (p->next == NULL)  {
			jb_err("Queue is BROKEN at item [%d]", i);	
		}
		i++;
		p=p->next;
	} while (p->next != jb->frames);
}

static void jb_dbgqueue(jitterbuf *jb) 
{
	int i=0;
	jb_frame *p = jb->frames;

	jb_dbg("queue: ");

	if (!p) {
		jb_dbg("EMPTY\n");
		return;
	}

	do {
		jb_dbg("[%d]=%ld ", i++, p->ts);
		p=p->next;
	} while (p->next != jb->frames);

	jb_dbg("\n");
}
#endif

jb_return_code jb_put(jitterbuf *jb, void *data, const enum jb_frame_type type, long ms, long ts, long now) 
{
	long numts;

	_debug("jb_put(%p,%p,%ld,%ld,%ld)\n", jb, data, ms, ts, now);

	numts = 0;
	if (jb->frames)
		numts = jb->frames->prev->ts - jb->frames->ts;

	_debug("numts %ld\n", numts);

	if (numts >= jb->info.conf.max_jitterbuf) {
		if (!jb->dropem) {
			_debug("Attempting to exceed Jitterbuf max %ld timeslots\n",
				jb->info.conf.max_jitterbuf);
			jb->dropem = 1;
		}
		jb->info.frames_dropped++;
		return JB_DROP;
	} else {
		jb->dropem = 0;
	}

	if (type == JB_TYPE_VOICE) {
		/* presently, I'm only adding VOICE frames to history and drift calculations; mostly because with the
		 * IAX integrations, I'm sending retransmitted control frames with their awkward timestamps through */
		if (history_put(jb,ts,now,ms)) {
			jb->info.frames_dropped++;
			return JB_DROP;
		}
	}

	jb->info.frames_in++;

	/* if put into head of queue, caller needs to reschedule */
	if (queue_put(jb,data,type,ms,ts)) {
		return JB_SCHED;
	}
	return JB_OK;
}


static enum jb_return_code _jb_get(jitterbuf *jb, jb_frame *frameout, long now, long interpl) 
{
	jb_frame *frame;
	long diff;
	static int dbg_cnt = 0;

	_debug("_jb_get\n");

	/*if ((now - jb_next(jb)) > 2 * jb->info.last_voice_ms) jb_warn("SCHED: %ld", (now - jb_next(jb))); */
	/* get jitter info */
	history_get(jb);

	if (dbg_cnt && dbg_cnt % 50 == 0) {
		jb_dbg("\n");
	}
	dbg_cnt++;

	/* target */
	jb->info.target = jb->info.jitter + jb->info.min + jb->info.conf.target_extra; 

	/* if a hard clamp was requested, use it */
	if ((jb->info.conf.max_jitterbuf) && ((jb->info.target - jb->info.min) > jb->info.conf.max_jitterbuf)) {
		_debug("clamping target from %ld to %ld\n", (jb->info.target - jb->info.min), jb->info.conf.max_jitterbuf);
		jb->info.target = jb->info.min + jb->info.conf.max_jitterbuf;
	}

	diff = jb->info.target - jb->info.current;

	_debug("diff = %ld lms=%ld last = %ld now = %ld, djust delay = %ld\n", diff,
		jb->info.last_voice_ms, jb->info.last_adjustment, now);

	/* let's work on non-silent case first */
	if (!jb->info.silence_begin_ts) {

	  /*
	        if(diff > 0)
		    printf("diff > 0\n");

		if((jb->info.last_adjustment + JB_ADJUST_DELAY) < now)
		    printf("(jb->info.last_adjustment + JB_ADJUST_DELAY) < now\n");

		if((diff > queue_last(jb)  - queue_next(jb)))
		    printf("diff > queue_last(jb)  - queue_next(jb)\n");
	  */
	  _debug("** Non-silent case update timing info **\n");
	  _debug("diff %ld\n", diff);
	  _debug("jb->info.last_adjustment %ld\n", jb->info.last_adjustment);
	  _debug("JB_ADJUST_DELAY %ld\n", JB_ADJUST_DELAY);
	  _debug("now %ld\n", now);

		/* we want to grow */
		if ((diff > 0) && 
			/* we haven't grown in the delay length */
			(((jb->info.last_adjustment + JB_ADJUST_DELAY) < now) || 
			/* we need to grow more than the "length" we have left */
			(diff > queue_last(jb)  - queue_next(jb)) ) ) {

		        
		        

			/* grow by interp frame length */
			jb->info.current += interpl;
			jb->info.next_voice_ts += interpl;
			jb->info.last_voice_ms = interpl;
			jb->info.last_adjustment = now;
			jb->info.cnt_contig_interp++;
			if (jb->info.conf.max_contig_interp && jb->info.cnt_contig_interp >= jb->info.conf.max_contig_interp) {
				jb->info.silence_begin_ts = jb->info.next_voice_ts - jb->info.current;
			}

			_debug("Non silent case\n");
			return JB_INTERP;
		}

	        _debug("queue get\n");

		frame = queue_get(jb, jb->info.next_voice_ts - jb->info.current);

		if(!frame)
		  _debug("frame not valid\n");

		/* not a voice frame; just return it. */
		if (frame && frame->type != JB_TYPE_VOICE) {
			if (frame->type == JB_TYPE_SILENCE) {
				jb->info.silence_begin_ts = frame->ts;
				jb->info.cnt_contig_interp = 0;
			}

			*frameout = *frame;
			jb->info.frames_out++;
			_debug("Not a voice packet\n");
			return JB_OK;
		}


		/* voice frame is later than expected */
		if (frame && frame->ts + jb->info.current < jb->info.next_voice_ts) {
			if (frame->ts + jb->info.current > jb->info.next_voice_ts - jb->info.last_voice_ms) {
				/* either we interpolated past this frame in the last jb_get */
				/* or the frame is still in order, but came a little too quick */ 
				*frameout = *frame;
				/* reset expectation for next frame */
				jb->info.next_voice_ts = frame->ts + jb->info.current + frame->ms;
				jb->info.frames_out++;
				decrement_losspct(jb);
				jb->info.cnt_contig_interp = 0;
				_debug("Either we interpolated past this frame in the last jb_get" \
				       "or the frame is still in order, but came a little too quick\n");
				return JB_OK;
			} else {
				/* voice frame is late */
				*frameout = *frame;
				jb->info.frames_out++;
				decrement_losspct(jb);
				jb->info.frames_late++;
				jb->info.frames_lost--;
				_debug("Voice frame is late\n");
				_debug("late: wanted=%ld, this=%ld, next=%ld\n", jb->info.next_voice_ts - jb->info.current, frame->ts, queue_next(jb));
				return JB_DROP;
			}
		}

		/* keep track of frame sizes, to allow for variable sized-frames */
		if (frame && frame->ms > 0) {
			jb->info.last_voice_ms = frame->ms;
		}

		/* we want to shrink; shrink at 1 frame / 500ms */
		/* unless we don't have a frame, then shrink 1 frame */
		/* every 80ms (though perhaps we can shrink even faster */
		/* in this case) */
		if (diff < -jb->info.conf.target_extra && 
			((!frame && jb->info.last_adjustment + 80 < now) || 
			(jb->info.last_adjustment + 500 < now))) {

			jb->info.last_adjustment = now;
			jb->info.cnt_contig_interp = 0;

			if (frame) {
				*frameout = *frame;
				/* shrink by frame size we're throwing out */
				jb->info.current -= frame->ms;
				jb->info.frames_out++;
				decrement_losspct(jb);
				jb->info.frames_dropped++;
				_debug("Shrink by frame size we're throwing out");
				return JB_DROP;
			} else {
				/* shrink by last_voice_ms */
				jb->info.current -= jb->info.last_voice_ms;
				jb->info.frames_lost++;
				increment_losspct(jb);
				jb_dbg("S");
				_debug("No frames, shrink by last_voice_ms");
				return JB_NOFRAME;
			}
		}

		/* lost frame */
		if (!frame) {
			/* this is a bit of a hack for now, but if we're close to
			 * target, and we find a missing frame, it makes sense to
			 * grow, because the frame might just be a bit late;
			 * otherwise, we presently get into a pattern where we return
			 * INTERP for the lost frame, then it shows up next, and we
			 * throw it away because it's late */
	  		/* I've recently only been able to replicate this using
			 * iaxclient talking to app_echo on asterisk.  In this case,
			 * my outgoing packets go through asterisk's (old)
			 * jitterbuffer, and then might get an unusual increasing delay 
			 * there if it decides to grow?? */
			/* Update: that might have been a different bug, that has been fixed..
			 * But, this still seemed like a good idea, except that it ended up making a single actual
			 * lost frame get interpolated two or more times, when there was "room" to grow, so it might
			 * be a bit of a bad idea overall */
			/*if (diff > -1 * jb->info.last_voice_ms) { 
				jb->info.current += jb->info.last_voice_ms;
				jb->info.last_adjustment = now;
				jb_warn("g");
				return JB_INTERP;
			} */
			jb->info.frames_lost++;
			increment_losspct(jb);
			jb->info.next_voice_ts += interpl;
			jb->info.last_voice_ms = interpl;
			jb->info.cnt_contig_interp++;
			if (jb->info.conf.max_contig_interp && jb->info.cnt_contig_interp >= jb->info.conf.max_contig_interp) {
				jb->info.silence_begin_ts = jb->info.next_voice_ts - jb->info.current;
			}
			
			return JB_INTERP;
		}

		/* normal case; return the frame, increment stuff */
		*frameout = *frame;
		jb->info.next_voice_ts += frame->ms;
		jb->info.frames_out++;
		jb->info.cnt_contig_interp = 0;
		decrement_losspct(jb);
		jb_dbg("v");
		return JB_OK;
	} else {     

	        _debug("Silence???\n");
		/* TODO: after we get the non-silent case down, we'll make the
		 * silent case -- basically, we'll just grow and shrink faster
		 * here, plus handle next_voice_ts a bit differently */

		/* to disable silent special case altogether, just uncomment this: */
		/* jb->info.silence_begin_ts = 0; */

 		/* shrink interpl len every 10ms during silence */
 		if (diff < -jb->info.conf.target_extra &&
 			jb->info.last_adjustment + 10 <= now) {
 			jb->info.current -= interpl;
 			jb->info.last_adjustment = now;
 		}

		frame = queue_get(jb, now - jb->info.current);
		if (!frame) {
			return JB_NOFRAME;
		} else if (frame->type != JB_TYPE_VOICE) {
			/* normal case; in silent mode, got a non-voice frame */
			*frameout = *frame;
			jb->info.frames_out++;
			return JB_OK;
		}
		if (frame->ts < jb->info.silence_begin_ts) {
			/* voice frame is late */
			*frameout = *frame;
			jb->info.frames_out++;
			decrement_losspct(jb);
			jb->info.frames_late++;
			jb->info.frames_lost--;
			jb_dbg("l");
			/*jb_warn("\nlate: wanted=%ld, this=%ld, next=%ld\n", jb->info.next_voice_ts - jb->info.current, frame->ts, queue_next(jb));
			jb_warninfo(jb); */
			return JB_DROP;
		} else {
			/* voice frame */
			/* try setting current to target right away here */
			jb->info.current = jb->info.target;
			jb->info.silence_begin_ts = 0;
			jb->info.next_voice_ts = frame->ts + jb->info.current + frame->ms;
			jb->info.last_voice_ms = frame->ms;
			jb->info.frames_out++;
			decrement_losspct(jb);
			*frameout = *frame;
			jb_dbg("V");
			return JB_OK;
		}
	}
}

long jb_next(jitterbuf *jb) 
{
	if (jb->info.silence_begin_ts) {
		if (jb->frames) {
			long next = queue_next(jb);
			history_get(jb);
			/* shrink during silence */
			if (jb->info.target - jb->info.current < -jb->info.conf.target_extra)
				return jb->info.last_adjustment + 10;
			return next + jb->info.target;
		}
		else 
			return JB_LONGMAX;
	} else {
		return jb->info.next_voice_ts;
	}
}

enum jb_return_code jb_get(jitterbuf *jb, jb_frame *frameout, long now, long interpl) 
{
        _debug("\n***** JB_GET *****\n\n");

	enum jb_return_code ret = _jb_get(jb, frameout, now, interpl);
#if 0
	static int lastts=0;
	int thists = ((ret == JB_OK) || (ret == JB_DROP)) ? frameout->ts : 0;
	jb_warn("jb_get(%x,%x,%ld) = %d (%d)\n", jb, frameout, now, ret, thists);
	if (thists && thists < lastts) jb_warn("XXXX timestamp roll-back!!!\n");
	lastts = thists;
#endif
	if (ret == JB_INTERP) 
		frameout->ms = jb->info.last_voice_ms;
	
	return ret;
}

enum jb_return_code jb_getall(jitterbuf *jb, jb_frame *frameout) 
{
	jb_frame *frame;
	frame = queue_getall(jb);

	if (!frame) {
		return JB_NOFRAME;
	}

	*frameout = *frame;
	return JB_OK;
}


enum jb_return_code jb_getinfo(jitterbuf *jb, jb_info *stats) 
{

	history_get(jb);

	*stats = jb->info;

	return JB_OK;
}

enum jb_return_code jb_setconf(jitterbuf *jb, jb_conf *conf) 
{
	/* take selected settings from the struct */

	jb->info.conf.max_jitterbuf = conf->max_jitterbuf;
 	jb->info.conf.resync_threshold = conf->resync_threshold;
	jb->info.conf.max_contig_interp = conf->max_contig_interp;

	/* -1 indicates use of the default JB_TARGET_EXTRA value */
	jb->info.conf.target_extra = ( conf->target_extra == -1 )
		? JB_TARGET_EXTRA
		: conf->target_extra
		;
		
	/* update these to match new target_extra setting */
	jb->info.current = jb->info.conf.target_extra;
	jb->info.target = jb->info.conf.target_extra;

	return JB_OK;
}

