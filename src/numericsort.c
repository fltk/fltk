/*
 * "$Id$"
 *
 * Numeric sorting routine for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2018 by Bill Spitzak and others.
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

#include <ctype.h>
#include <stdlib.h>
#include <FL/platform_types.h>
#include <FL/filename.H>

/**
  \file numericsort.c
*/

/*
  numericsort() - Compare two directory entries, possibly with a
                  case-insensitive comparison...

  *FIXME* This is not UTF-8 aware -- particularly case-insensitive comparison.
  *FIXME* If you fix it, don't forget to update the documentation below.
*/

static int numericsort(struct dirent **A, struct dirent **B, int cs) {
  const char* a = (*A)->d_name;
  const char* b = (*B)->d_name;
  int ret = 0;
  for (;;) {
    if (isdigit(*a & 255) && isdigit(*b & 255)) {
      int diff,magdiff;
      while (*a == '0') a++;
      while (*b == '0') b++;
      while (isdigit(*a & 255) && *a == *b) {a++; b++;}
      diff = (isdigit(*a & 255) && isdigit(*b & 255)) ? *a - *b : 0;
      magdiff = 0;
      while (isdigit(*a & 255)) {magdiff++; a++;}
      while (isdigit(*b & 255)) {magdiff--; b++;}
      if (magdiff) {ret = magdiff; break;} /* compare # of significant digits */
      if (diff) {ret = diff; break;}       /* compare first non-zero digit */
    } else {
      if (cs) {
      	/* compare case-sensitive */
	if ((ret = *a-*b)) break;
      } else {
	/* compare case-insensitive */
	if ((ret = tolower(*a & 255)-tolower(*b & 255))) break;
      }

      if (!*a) break;
      a++; b++;
    }
  }
  if (!ret) return 0;
  else return (ret < 0) ? -1 : 1;
}

/**
  Compares directory entries alphanumerically (case-insensitive).

  \note	This comparison is not (yet) UTF-8 aware.

  \todo Make comparison UTF-8 aware.

  \see fl_numericsort()
*/

int fl_casenumericsort(struct dirent **A, struct dirent **B) {
  return numericsort(A, B, 0);
}

/**
  Compares directory entries alphanumerically (case-sensitive).

  Numbers are compared without sign, i.e. "-" is not taken as a sign of
  following numerical values. The following list of files would be in
  ascending order (examples are ASCII and numbers only for simplicity):

   -# 1zzz.txt
   -# 2xxx.txt
   -# 19uuu.txt
   -# 100aaa.txt
   -# file1z.txt
   -# file5a.txt
   -# file5z.txt
   -# file30z.txt
   -# file200a.txt
   -# temp+5.txt   ('+' is lexically lower than '-')
   -# temp-5.txt   ('-' is not a sign)
   -# temp-100.txt (100 is bigger than 5, no sign)

  \param[in] A first directory entry
  \param[in] B second directory entry

  \returns comparison result (-1, 0, or +1)
  \retval  -1 A \< B
  \retval   0 A == B
  \retval  +1 A \> B

  \note	This comparison is not (yet) UTF-8 aware:
    - UTF-8 characters are compared according to their binary values.
    - Locale settings may influence the result in unexpected ways.
    - The latter is particularly true for fl_casenumericsort().
  This may be changed in a future release.

  \todo Make comparison UTF-8 aware.

  \see fl_casenumericsort()
*/

int fl_numericsort(struct dirent **A, struct dirent **B) {
  return numericsort(A, B, 1);
}

/*
 * End of "$Id$".
 */
