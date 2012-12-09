/*
 * "$Id$"
 *
 * BSD string functions for the Fast Light Tool Kit (FLTK).
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

#include "flstring.h"


/*
 * 'fl_strlcat()' - Safely concatenate two strings.
 */

size_t				/* O - Length of string */
fl_strlcat(char       *dst,	/* O - Destination string */
           const char *src,	/* I - Source string */
	   size_t     size) {	/* I - Size of destination string buffer */
  size_t	srclen;		/* Length of source string */
  size_t	dstlen;		/* Length of destination string */


 /*
  * Figure out how much room is left...
  */

  dstlen = strlen(dst);
  size   -= dstlen + 1;

  if (!size) return (dstlen);	/* No room, return immediately... */

 /*
  * Figure out how much room is needed...
  */

  srclen = strlen(src);

 /*
  * Copy the appropriate amount...
  */

  if (srclen > size) srclen = size;

  memcpy(dst + dstlen, src, srclen);
  dst[dstlen + srclen] = '\0';

  return (dstlen + srclen);
}


/*
 * 'fl_strlcpy()' - Safely copy two strings.
 */

size_t				/* O - Length of string */
fl_strlcpy(char       *dst,	/* O - Destination string */
           const char *src,	/* I - Source string */
	   size_t      size) {	/* I - Size of destination string buffer */
  size_t	srclen;		/* Length of source string */


 /*
  * Figure out how much room is needed...
  */

  size --;

  srclen = strlen(src);

 /*
  * Copy the appropriate amount...
  */

  if (srclen > size) srclen = size;

  memcpy(dst, src, srclen);
  dst[srclen] = '\0';

  return (srclen);
}

#define C_RANGE(c,l,r) ( (c) >= (l) && (c) <= (r) )

/**
 * locale independent ascii oriented case cmp
 * returns 0 if string successfully compare, -1 if s<t, +1 if s>t
 */
int fl_ascii_strcasecmp(const char *s, const char *t) {
  if (!s || !t) return (s==t ? 0 : (!s ? -1 : +1));
  
  for(;*s && *t; s++,t++) {
    if (*s == *t) continue;
    if (*s < *t) {
      if ( (*s+0x20)!=*t || !C_RANGE(*s,'A','Z') ) return -1;
    } else { 	/* (*s > *t) */
      if ( (*s-0x20)!=*t || !C_RANGE(*s,'a','z') ) return +1;
    }
  }
  return (*s==*t) ? 0 : (*t ? -1 : +1);
}

/*
 * End of "$Id$".
 */
