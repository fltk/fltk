/*
 * "$Id$"
 *
 * wrapper(s) around Markus Kuhn's wcwidth() implementation.
 *
 * Copyright 2006-2010 by Bill Spitzak and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems on the following page:
 *
 *     http://www.fltk.org/str.php
 */

/* include Markus Kuhn's wcwidth() implementation.
 * Note: only the filename has been changes at the moment.
 * forward declare the routines as static to avoid name leakage.
 */

#include <wchar.h>
static int mk_wcwidth(wchar_t ucs);
static int mk_wcswidth(const wchar_t *pwcs, size_t n);
static int mk_wcwidth_cjk(wchar_t ucs);
static int mk_wcswidth_cjk(const wchar_t *pwcs, size_t n);

#include "mk_wcwidth.c"

int fl_wcwidth(unsigned int ucs) {
  // warning: we have problems if sizeof(wchar_t) == 2 and ucs > 0xffff
  return mk_wcwidth(ucs);
}

/*
 * End of "$Id$".
 */
