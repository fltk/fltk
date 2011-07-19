/*
 * Author: Jean-Marc Lienher ( http://oksid.ch )
 * Copyright 2000-2003 by O'ksi'D.
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file.  If this
 * file is missing or damaged, see the license at:
 *
 *     http://www.fltk.org/COPYING.php
 *
 * Please report all bugs and problems on the following page:
 *
 *     http://www.fltk.org/str.php
 */

/*	
 *    generate the "if(){} else if ..." structure of ucs2fontmap()
 */

#include <wchar.h>
#include <stdio.h>
#include <iconv.h>
char uni[0x10000];
#include "../utf8Utils.c"

int main(int argc, char **argv) {

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
      /* XConvertUtf8ToUcs((unsigned char*)ob, 6 - l2, &ucs); */
      printf ("%x --> %X\n", i, ucs & 0xFFFF);
    }
  }
  iconv_close(cd);
  return 0;
}

/*
 * End of "$Id$".
 */
