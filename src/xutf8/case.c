/******************************************************************************
              Copyright 2001 by O'ksi'D
 *                      All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *      Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *      Neither the name of O'ksi'D nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER 
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *


        Author: Jean-Marc Lienher ( http://oksid.ch )

******************************************************************************/

/*
 * This file is required on all platforms for utf8 support
 */

#include "headers/case.h"
#include <stdlib.h>


int 
XUtf8Tolower(
	int ucs)
{
	int ret;

	if (ucs <= 0x02B6) {
		if (ucs >= 0x0041) {
			ret = ucs_table_0041[ucs - 0x0041];
			if (ret > 0) return ret;
		}
		return ucs;
	}

	if (ucs <= 0x0556) {
		if (ucs >= 0x0386) {
			ret = ucs_table_0386[ucs - 0x0386];
			if (ret > 0) return ret;
		}
		return ucs;
	}

	if (ucs <= 0x10C5) {
		if (ucs >= 0x10A0) {
			ret = ucs_table_10A0[ucs - 0x10A0];
			if (ret > 0) return ret;
		}
		return ucs;
	}

	if (ucs <= 0x1FFC) {
		if (ucs >= 0x1E00) {
			ret = ucs_table_1E00[ucs - 0x1E00];
			if (ret > 0) return ret;
		}
		return ucs;
	}

	if (ucs <= 0x2133) {
		if (ucs >= 0x2102) {
			ret = ucs_table_2102[ucs - 0x2102];
			if (ret > 0) return ret;
		}
		return ucs;
	}

	if (ucs <= 0x24CF) {
		if (ucs >= 0x24B6) {
			ret = ucs_table_24B6[ucs - 0x24B6];
			if (ret > 0) return ret;
		}
		return ucs;
	}

	if (ucs <= 0x33CE) {
		if (ucs >= 0x33CE) {
			ret = ucs_table_33CE[ucs - 0x33CE];
			if (ret > 0) return ret;
		}
		return ucs;
	}

	if (ucs <= 0xFF3A) {
		if (ucs >= 0xFF21) {
			ret = ucs_table_FF21[ucs - 0xFF21];
			if (ret > 0) return ret;
		}
		return ucs;
	}

	return ucs;
}

int 
XUtf8Toupper(
	int ucs)
{
	int i;
	static unsigned short *table = NULL;

	if (!table) {
		table = (unsigned short*) malloc(
			sizeof(unsigned short) * 0x10000);
		for (i = 0; i < 0x10000; i++) {
			table[i] = (unsigned short) i;
		}	
		for (i = 0; i < 0x10000; i++) {
			int l;
			l = XUtf8Tolower(i);			
			if (l != i) table[l] = (unsigned short) i;
		}	

	}
	if (ucs >= 0x10000 || ucs < 0) return ucs;
	return table[ucs];
}

