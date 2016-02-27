/*
 * "$Id$"
 *
 * snprintf() and vsnprintf() functions for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2010 by Bill Spitzak and others.
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

#include <stdio.h>
#include "flstring.h"

#ifdef HAVE_SYS_STDTYPES_H
#  include <sys/stdtypes.h>
#endif /* HAVE_SYS_STDTYPES_H */

#ifdef __cplusplus
extern "C" {
#endif

int fl_vsnprintf(char* buffer, size_t bufsize, const char* format, va_list ap) {
#if defined(HAVE_VSNPRINTF) && defined(__linux__)
  return vsnprintf(buffer, bufsize, format, ap);
#else
  char		*bufptr,		/* Pointer to position in buffer */
		*bufend,		/* Pointer to end of buffer */
		sign,			/* Sign of format width */
		size,			/* Size character (h, l, L) */
		type;			/* Format type character */
  int		width,			/* Width of field */
		prec;			/* Number of characters of precision */
  char		tformat[100],		/* Temporary format string for sprintf() */
		*tptr,			/* Pointer into temporary format */
		temp[1024];		/* Buffer for formatted numbers */
  char		*s;			/* Pointer to string */
  int		slen;			/* Length of string */
  int		bytes;			/* Total number of bytes needed */


 /*
  * Loop through the format string, formatting as needed...
  */

  bufptr = buffer;
  bufend = buffer + bufsize - 1;
  bytes  = 0;

  while (*format) {
    if (*format == '%') {
      tptr = tformat;
      *tptr++ = *format++;

      if (*format == '%') {
        if (bufptr && bufptr < bufend) *bufptr++ = *format;
        bytes ++;
        format ++;
	continue;
      } else if (strchr(" -+#\'", *format)) {
        *tptr++ = *format;
        sign = *format++;
      } else sign = 0;

      if (*format == '*') {
        /* Get width from argument... */
	format ++;
	width = va_arg(ap, int);
	snprintf(tptr, sizeof(tformat) - (tptr - tformat), "%d", width);
	tptr += strlen(tptr);
      } else {
	width = 0;
	while (isdigit(*format & 255)) {
	  if (tptr < (tformat + sizeof(tformat) - 1)) *tptr++ = *format;
	  width = width * 10 + *format++ - '0';
	}
      }

      if (*format == '.') {
	if (tptr < (tformat + sizeof(tformat) - 1)) *tptr++ = *format;
        format ++;

        if (*format == '*') {
          /* Get precision from argument... */
	  format ++;
	  prec = va_arg(ap, int);
	  snprintf(tptr, sizeof(tformat) - (tptr - tformat), "%d", prec);
	  tptr += strlen(tptr);
	} else {
	  prec = 0;
	  while (isdigit(*format & 255)) {
	    if (tptr < (tformat + sizeof(tformat) - 1)) *tptr++ = *format;
	    prec = prec * 10 + *format++ - '0';
	  }
	}
      } else prec = -1;

      size = '\0';

      if (*format == 'l' && format[1] == 'l') {
        size = 'L';
	if (tptr < (tformat + sizeof(tformat) - 2)) {
	  *tptr++ = 'l';
	  *tptr++ = 'l';
	}
	format += 2;
      } else if (*format == 'h' || *format == 'l' || *format == 'L') {
	if (tptr < (tformat + sizeof(tformat) - 1)) *tptr++ = *format;
        size = *format++;
      }

      if (!*format) break;

      if (tptr < (tformat + sizeof(tformat) - 1)) *tptr++ = *format;
      type  = *format++;
      *tptr = '\0';

      switch (type) {
	case 'E' : /* Floating point formats */
	case 'G' :
	case 'e' :
	case 'f' :
	case 'g' :
	  if ((width + 2) > (int)sizeof(temp)) break;

	  sprintf(temp, tformat, va_arg(ap, double));

          bytes += (int) strlen(temp);

          if (bufptr) {
	    if ((bufptr + strlen(temp)) > bufend) {
	      strncpy(bufptr, temp, (size_t)(bufend - bufptr));
	      bufptr = bufend;
	    } else {
	      strcpy(bufptr, temp);
	      bufptr += strlen(temp);
	    }
	  }
	  break;

        case 'B' : /* Integer formats */
	case 'X' :
	case 'b' :
        case 'd' :
	case 'i' :
	case 'o' :
	case 'u' :
	case 'x' :
	  if ((width + 2) > (int)sizeof(temp)) break;

#ifdef HAVE_LONG_LONG
	  if (size == 'L')
	    sprintf(temp, tformat, va_arg(ap, long long));
	  else
#endif /* HAVE_LONG_LONG */
	  if (size == 'l')
	    sprintf(temp, tformat, va_arg(ap, long));
	  else
	    sprintf(temp, tformat, va_arg(ap, int));

          bytes += (int) strlen(temp);

	  if (bufptr) {
	    if ((bufptr + strlen(temp)) > bufend) {
	      strncpy(bufptr, temp, (size_t)(bufend - bufptr));
	      bufptr = bufend;
	    } else {
	      strcpy(bufptr, temp);
	      bufptr += strlen(temp);
	    }
	  }
	  break;
	    
	case 'p' : /* Pointer value */
	  if ((width + 2) > (int)sizeof(temp)) break;

	  sprintf(temp, tformat, va_arg(ap, void *));

          bytes += (int) strlen(temp);

	  if (bufptr) {
	    if ((bufptr + strlen(temp)) > bufend) {
	      strncpy(bufptr, temp, (size_t)(bufend - bufptr));
	      bufptr = bufend;
	    } else {
	      strcpy(bufptr, temp);
	      bufptr += strlen(temp);
	    }
	  }
	  break;

        case 'c' : /* Character or character array */
	  bytes += width;

	  if (bufptr) {
	    if (width <= 1) *bufptr++ = va_arg(ap, int);
	    else {
	      if ((bufptr + width) > bufend) width = (int) (bufend - bufptr);

	      memcpy(bufptr, va_arg(ap, char *), (size_t)width);
	      bufptr += width;
	    }
	  }
	  break;

	case 's' : /* String */
	  if ((s = va_arg(ap, char *)) == NULL) s = "(null)";

	  slen = (int) strlen(s);
	  if (slen > width && prec != width) width = slen;

          bytes += width;

	  if (bufptr) {
	    if ((bufptr + width) > bufend) width = (int) (bufend - bufptr);

            if (slen > width) slen = width;

	    if (sign == '-') {
	      strncpy(bufptr, s, (size_t)slen);
	      memset(bufptr + slen, ' ', (size_t)(width - slen));
	    } else {
	      memset(bufptr, ' ', (size_t)(width - slen));
	      strncpy(bufptr + width - slen, s, (size_t)slen);
	    }

	    bufptr += width;
	  }
	  break;

	case 'n' : /* Output number of chars so far */
	  *(va_arg(ap, int *)) = bytes;
	  break;
      }
    } else {
      bytes ++;

      if (bufptr && bufptr < bufend) *bufptr++ = *format;
      format ++;
    }
  }

 /*
  * Nul-terminate the string and return the number of characters needed.
  */

  if (bufptr) *bufptr = '\0';

  return (bytes);
#endif /* HAVE_VSNPRINTF */
}

int fl_snprintf(char* str, size_t size, const char* fmt, ...) {
  int ret;
  va_list ap;
  va_start(ap, fmt);
  ret = vsnprintf(str, size, fmt, ap);
  va_end(ap);
  return ret;
}

#ifdef __cplusplus
}
#endif

/*
 * End of "$Id$".
 */

