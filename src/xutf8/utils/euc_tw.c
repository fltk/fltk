/******************************************************************************
 *	
 *    generate the "if(){} else if ..." structure of ucs2fontmap()
 *
 *                 Copyright (c) 2000  O'ksi'D
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 *   Author : Jean-Marc Lienher ( http://oksid.ch )
 *
 ******************************************************************************/

#include <wchar.h>
#include <stdio.h>
#include <iconv.h>
char uni[0x10000];
#include "../utf8Utils.c"

int main(int argc, char **argv)
{

	iconv_t cd;

	int i;
	cd = iconv_open("EUC-TW", "UTF16");
	for(i = 0; i < 0x10000; i++) uni[i] = 0;
	for(i = 0x00000000; i < 0xFFFFFFFF; i++) {
		char buf[4], ob[6];
		char *b = buf;
		int ucs = -1;
		int l1 = 4, l2 = 6;
		char *o = ob ;
		buf[0] = i & 0xff;
		buf[1] = (i >> 8) & 0xFF;
		buf[2] = (i >> 16) & 0xFF;
		buf[3] = (i >> 24) & 0xFF;
		iconv(cd, NULL, NULL, NULL, NULL);
		iconv(cd, &b, &l1, &o, &l2);
		if (l2 != 6) {
			ucs = (unsigned)ob[0];
			ucs += (unsigned) (ob[1] << 8);
			//XConvertUtf8ToUcs((unsigned char*)ob, 6 - l2, &ucs); 
			printf ("%x --> %X\n", i, ucs & 0xFFFF);
		}
		
	}
	iconv_close(cd);
	return 0;
}
