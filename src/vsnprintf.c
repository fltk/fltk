/*
 * "$Id: vsnprintf.c,v 1.3.2.5 2001/04/27 14:39:27 easysw Exp $"
 *
 * vsnprintf() function for the Fast Light Tool Kit (FLTK).
 *
 * Emulates this call on systems that lack it (pretty much everything
 * except glibc systems).
 *
 * KNOWN BUGS:
 *
 * Field width & Precision is ignored for %%, %c, and %s.
 *
 * A malicious user who manages to create a %-fmt string that prints
 * more than 99 characters can still overflow the temporary buffer.
 * For instance %110f will overflow.
 *
 * Only handles formats that are both documented in the glibc man page
 * for printf and also handled by your system's sprintf().
 *
 * Copyright 1998-2001 by Bill Spitzak and others.
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
 * Please report all bugs and problems to "fltk-bugs@fltk.org".
 */

#include <stdio.h>
#include <stdarg.h>
#include <config.h>

#ifdef HAVE_SYS_STDTYPES_H
#  include <sys/stdtypes.h>
#endif /* HAVE_SYS_STDTYPES_H */

#ifdef __cplusplus
extern "C" {
#endif

#if !HAVE_VSNPRINTF

int vsnprintf(char* str, size_t size, const char* fmt, va_list ap) {
  const char* e = str+size-1;
  char* p = str;
  char copy[20];
  char* copy_p;
  char sprintf_out[100];

  while (*fmt && p < e) {
    if (*fmt != '%') {
      *p++ = *fmt++;
    } else {
      fmt++;
      copy[0] = '%';
      for (copy_p = copy+1; copy_p < copy+19;) {
	switch ((*copy_p++ = *fmt++)) {
	case 0:
	  fmt--; goto CONTINUE;
	case '%':
	  *p++ = '%'; goto CONTINUE;
	case 'c':
	  *p++ = va_arg(ap, int);
	  goto CONTINUE;
	case 'd':
	case 'i':
	case 'o':
	case 'u':
	case 'x':
	case 'X':
	  *copy_p = 0;
	  sprintf(sprintf_out, copy, va_arg(ap, int));
	  copy_p = sprintf_out;
	  goto DUP;
	case 'e':
	case 'E':
	case 'f':
	case 'g':
	  *copy_p = 0;
	  sprintf(sprintf_out, copy, va_arg(ap, double));
	  copy_p = sprintf_out;
	  goto DUP;
	case 'p':
	  *copy_p = 0;
	  sprintf(sprintf_out, copy, va_arg(ap, void*));
	  copy_p = sprintf_out;
	  goto DUP;
	case 'n':
	  *(va_arg(ap, int*)) = p-str;
	  goto CONTINUE;
	case 's':
	  copy_p = va_arg(ap, char*);
	  if (!copy_p) copy_p = "NULL";
	DUP:
	  while (*copy_p && p < e) *p++ = *copy_p++;
	  goto CONTINUE;
	}
      }
    }
  CONTINUE:;
  }
  *p = 0;
  if (*fmt) return -1;
  return p-str;
}

#endif

#if !HAVE_SNPRINTF

int snprintf(char* str, size_t size, const char* fmt, ...) {
  int ret;
  va_list ap;
  va_start(ap, fmt);
  ret = vsnprintf(str, size, fmt, ap);
  va_end(ap);
  return ret;
}

#endif

#ifdef __cplusplus
}
#endif

/*
 * End of "$Id: vsnprintf.c,v 1.3.2.5 2001/04/27 14:39:27 easysw Exp $".
 */

