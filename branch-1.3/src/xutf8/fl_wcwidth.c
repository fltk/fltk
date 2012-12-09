/*
 * "$Id$"
 *
 * wrapper(s) around Markus Kuhn's wcwidth() implementation.
 *
 * Copyright 2006-2010 by Bill Spitzak and others.
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

/* include Markus Kuhn's wcwidth() implementation.
 * Note: only the filename has been changes at the moment.
 * forward declare the routines as static to avoid name leakage.
 */

#if 0
#include <stdio.h>              /* for size_t only */
#endif

static int mk_wcwidth(unsigned int ucs);
#if 0
static int mk_wcswidth(const unsigned int *pwcs, size_t n);
static int mk_wcwidth_cjk(unsigned int ucs);
static int mk_wcswidth_cjk(const unsigned int *pwcs, size_t n);
#endif

#include "mk_wcwidth.c"

int fl_wcwidth(unsigned int ucs) {
  /* warning: we have problems if sizeof(wchar_t) == 2 and ucs > 0xffff */
  return mk_wcwidth(ucs);
}

/*
 * End of "$Id$".
 */
