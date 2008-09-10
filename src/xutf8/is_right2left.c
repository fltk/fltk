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

unsigned short 
XUtf8IsRightToLeft(
	unsigned int ucs)
{

#if 0
	/* for debug only */
	if (ucs <= 0x005A) {
		if (ucs >= 0x0041) return 1;
		return 0;
	}
#endif

	/* HEBREW */
	if (ucs <= 0x05F4) {
		if (ucs >= 0x0591) return 1;
		return 0;
	}
	
	/* ARABIC */
	if (ucs <= 0x06ED) {
		if (ucs >= 0x060C)  return 1;
		return 0;
	}

	if (ucs <= 0x06F9) {
		 if (ucs >= 0x06F0) return 1;
		return 0;
	}

	if (ucs == 0x200F) return 1;

	if (ucs == 0x202B) return 1;

	if (ucs == 0x202E) return 1;

	if (ucs <= 0xFB4F) {
		if (ucs >= 0xFB1E) return 1;
		return 0;
	}
	
	if (ucs <= 0xFDFB) {
		if (ucs >= 0xFB50) return 1;
		return 0;
	}

	if (ucs <= 0xFEFC) {
		if (ucs >= 0xFE70) return 1;
		return 0;
	}

	return 0;
}

