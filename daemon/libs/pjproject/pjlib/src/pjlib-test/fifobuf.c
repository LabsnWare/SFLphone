/* $Id: fifobuf.c 2394 2008-12-23 17:27:53Z bennylp $ */
/* 
 * Copyright (C) 2008-2009 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 *
 *  Additional permission under GNU GPL version 3 section 7:
 *
 *  If you modify this program, or any covered work, by linking or
 *  combining it with the OpenSSL project's OpenSSL library (or a
 *  modified version of that library), containing parts covered by the
 *  terms of the OpenSSL or SSLeay licenses, Teluu Inc. (http://www.teluu.com)
 *  grants you additional permission to convey the resulting work.
 *  Corresponding Source for a non-source form of such a combination
 *  shall include the source code for the parts of OpenSSL used as well
 *  as that of the covered work.
 */
#include "test.h"

/* To prevent warning about "translation unit is empty"
 * when this test is disabled. 
 */
int dummy_fifobuf_test;

#if INCLUDE_FIFOBUF_TEST

#include <pjlib.h>

int fifobuf_test()
{
    enum { SIZE = 1024, MAX_ENTRIES = 128, 
	   MIN_SIZE = 4, MAX_SIZE = 64, 
	   LOOP=10000 };
    pj_pool_t *pool;
    pj_fifobuf_t fifo;
    unsigned available = SIZE;
    void *entries[MAX_ENTRIES];
    void *buffer;
    int i;

    pool = pj_pool_create(mem, NULL, SIZE+256, 0, NULL);
    if (!pool)
	return -10;

    buffer = pj_pool_alloc(pool, SIZE);
    if (!buffer)
	return -20;

    pj_fifobuf_init (&fifo, buffer, SIZE);
    
    // Test 1
    for (i=0; i<LOOP*MAX_ENTRIES; ++i) {
	int size;
	int c, f;
	c = i%2;
	f = (i+1)%2;
	do {
	    size = MIN_SIZE+(pj_rand() % MAX_SIZE);
	    entries[c] = pj_fifobuf_alloc (&fifo, size);
	} while (entries[c] == 0);
	if ( i!=0) {
	    pj_fifobuf_free(&fifo, entries[f]);
	}
    }
    if (entries[(i+1)%2])
	pj_fifobuf_free(&fifo, entries[(i+1)%2]);

    if (pj_fifobuf_max_size(&fifo) < SIZE-4) {
	pj_assert(0);
	return -1;
    }

    // Test 2
    entries[0] = pj_fifobuf_alloc (&fifo, MIN_SIZE);
    if (!entries[0]) return -1;
    for (i=0; i<LOOP*MAX_ENTRIES; ++i) {
	int size = MIN_SIZE+(pj_rand() % MAX_SIZE);
	entries[1] = pj_fifobuf_alloc (&fifo, size);
	if (entries[1])
	    pj_fifobuf_unalloc(&fifo, entries[1]);
    }
    pj_fifobuf_unalloc(&fifo, entries[0]);
    if (pj_fifobuf_max_size(&fifo) < SIZE-4) {
	pj_assert(0);
	return -2;
    }

    // Test 3
    for (i=0; i<LOOP; ++i) {
	int count, j;
	for (count=0; available>=MIN_SIZE+4 && count < MAX_ENTRIES;) {
	    int size = MIN_SIZE+(pj_rand() % MAX_SIZE);
	    entries[count] = pj_fifobuf_alloc (&fifo, size);
	    if (entries[count]) {
		available -= (size+4);
		++count;
	    }
	}
	for (j=0; j<count; ++j) {
	    pj_fifobuf_free (&fifo, entries[j]);
	}
	available = SIZE;
    }

    if (pj_fifobuf_max_size(&fifo) < SIZE-4) {
	pj_assert(0);
	return -3;
    }
    pj_pool_release(pool);
    return 0;
}

#endif	/* INCLUDE_FIFOBUF_TEST */


