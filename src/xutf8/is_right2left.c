/* "$Id: $"
 *
 * Author: Jean-Marc Lienher ( http://oksid.ch )
 * Copyright 2000-2010 by O'ksi'D.
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
 * This file is required on all platforms for utf8 support
 */

unsigned short 
XUtf8IsRightToLeft(unsigned int ucs) {

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

/*
 * End of "$Id$".
 */
