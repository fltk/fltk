/******************************************************************************
              Copyright 2000-2001 by O'ksi'D

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of O'ksi'D
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.
O'ksi'D makes no representations about the suitability of
this software for any purpose.  It is provided "as is" without
express or implied warranty.

O'ksi'D DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL O'ksi'D BE LIABLE FOR ANY SPECIAL, INDIRECT
OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
OR PERFORMANCE OF THIS SOFTWARE.

        Author: Jean-Marc Lienher ( http://oksid.ch )

******************************************************************************/

/*
 * This file is required on all platforms for utf8 support
 */

#include "headers/spacing.h"


unsigned short 
XUtf8IsNonSpacing(
	unsigned int ucs)
{

	if (ucs <= 0x0361) {
		if (ucs >= 0x0300) return ucs_table_0300[ucs - 0x0300];
		return 0;
	}

	if (ucs <= 0x0486) {
		if (ucs >= 0x0483) return ucs_table_0483[ucs - 0x0483];
		return 0;
	}

	if (ucs <= 0x05C4) {
		if (ucs >= 0x0591) return ucs_table_0591[ucs - 0x0591];
		return 0;
	}

	if (ucs <= 0x06ED) {
		if (ucs >= 0x064B) return ucs_table_064B[ucs - 0x064B];
		return 0;
	}

	if (ucs <= 0x0D4D) {
		if (ucs >= 0x0901) return ucs_table_0901[ucs - 0x0901];
		return 0;
	}

	if (ucs <= 0x0FB9) {
		if (ucs >= 0x0E31) return ucs_table_0E31[ucs - 0x0E31];
		return 0;
	}

	if (ucs <= 0x20E1) {
		if (ucs >= 0x20D0) return ucs_table_20D0[ucs - 0x20D0];
		return 0;
	}

	if (ucs <= 0x309A) {
		if (ucs >= 0x302A) return ucs_table_302A[ucs - 0x302A];
		return 0;
	}

	if (ucs <= 0xFB1E) {
		if (ucs >= 0xFB1E) return ucs_table_FB1E[ucs - 0xFB1E];
		return 0;
	}

	if (ucs <= 0xFE23) {
		if (ucs >= 0xFE20) return ucs_table_FE20[ucs - 0xFE20];
		return 0;
	}

	return 0;
}

