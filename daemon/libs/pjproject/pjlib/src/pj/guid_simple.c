/* $Id: guid_simple.c 2394 2008-12-23 17:27:53Z bennylp $ */
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
#include <pj/guid.h>
#include <pj/assert.h>
#include <pj/rand.h>
#include <pj/os.h>
#include <pj/string.h>

PJ_DEF_DATA(const unsigned) PJ_GUID_STRING_LENGTH=32;

static char guid_chars[64];

PJ_DEF(unsigned) pj_GUID_STRING_LENGTH()
{
    return PJ_GUID_STRING_LENGTH;
}

static void init_guid_chars(void)
{
    char *p = guid_chars;
    unsigned i;

    for (i=0; i<10; ++i)
	*p++ = '0'+i;

    for (i=0; i<26; ++i) {
	*p++ = 'a'+i;
	*p++ = 'A'+i;
    }

    *p++ = '-';
    *p++ = '.';
}

PJ_DEF(pj_str_t*) pj_generate_unique_string(pj_str_t *str)
{
    char *p, *end;

    PJ_CHECK_STACK();

    if (guid_chars[0] == '\0') {
	pj_enter_critical_section();
	if (guid_chars[0] == '\0') {
	    init_guid_chars();
	}
	pj_leave_critical_section();
    }

    /* This would only work if PJ_GUID_STRING_LENGTH is multiple of 2 bytes */
    pj_assert(PJ_GUID_STRING_LENGTH % 2 == 0);

    for (p=str->ptr, end=p+PJ_GUID_STRING_LENGTH; p<end; ) {
	/* Assumes rand() only has 16bit randomness */
	unsigned short val = pj_rand();
	*p++ = guid_chars[(val >> 8)   & 63];
	*p++ = guid_chars[(val & 0xFF) & 63];
    }

    str->slen = PJ_GUID_STRING_LENGTH;
    return str;
}
