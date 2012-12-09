/* "$Id: $"
 *
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

#define KEYSYM2UCS_INCLUDED

#if !defined(WIN32) && !defined(__APPLE__)

#include "../../FL/Xutf8.h"
#include "imKStoUCS.c"

long XKeysymToUcs(KeySym keysym) {
  return (long) KeySymToUcs4(keysym);
}

#endif /* X11 only */

/*
 * End of "$Id$".
 */
