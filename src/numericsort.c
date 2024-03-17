/*
 * Numeric sorting routine for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2018 by Bill Spitzak and others.
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file.  If this
 * file is missing or damaged, see the license at:
 *
 *     https://www.fltk.org/COPYING.php
 *
 * Please see the following page on how to report bugs and issues:
 *
 *     https://www.fltk.org/bugs.php
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <FL/platform_types.h>
#include <FL/filename.H>
#include <FL/fl_utf8.h>

/**
  \file numericsort.c
*/

/*
  numericsort() - Compare two directory entries, possibly with a
                  case-insensitive comparison...
*/

static int numericsort(struct dirent **A, struct dirent **B, int cs) {
  const char* a = (*A)->d_name;
  const char* b = (*B)->d_name;
  int len_a, len_b;
  const char *end_a = a + strlen(a);
  const char *end_b = b + strlen(b);
  unsigned UTF_A, UTF_B;
  int ret = 0;
  while (1) {
    UTF_A = fl_utf8decode(a, end_a, &len_a);
    a += len_a;
    UTF_B = fl_utf8decode(b, end_b, &len_b);
    b += len_b;
    if (UTF_A <= 255 && UTF_B <= 255 && isdigit(UTF_A) && isdigit(UTF_B)) {
      int diff,magdiff;
      while (UTF_A == '0') {UTF_A = fl_utf8decode(a, end_a, &len_a); a += len_a;}
      while (UTF_B == '0') {UTF_B = fl_utf8decode(b, end_b, &len_b); b += len_b;}
      while (UTF_A <= 255 && isdigit(UTF_A) && UTF_A == UTF_B) {
        UTF_A = fl_utf8decode(a, end_a, &len_a); a += len_a;
        UTF_B = fl_utf8decode(b, end_b, &len_b); b += len_b;
      }
      diff = (UTF_A <= 255 && UTF_B <= 255 && isdigit(UTF_A) && isdigit(UTF_B)) ? UTF_A - UTF_B : 0;
      magdiff = 0;
      while (UTF_A <= 255 && isdigit(UTF_A)) {
        magdiff++;
        UTF_A = fl_utf8decode(a, end_a, &len_a); a += len_a;
      }
      while (UTF_B <= 255 && isdigit(UTF_B)) {
        magdiff--;
        UTF_B = fl_utf8decode(b, end_b, &len_b); b += len_b;
      }
      if (magdiff) {ret = magdiff; break;} /* compare # of significant digits */
      if (diff) {ret = diff; break;}       /* compare first non-zero digit */
    } else {
      if (cs) {
        /* compare case-sensitive */
        if ((ret = UTF_A - UTF_B)) break;
      } else {
        /* compare case-insensitive */
        if ((ret = fl_tolower(UTF_A) - fl_tolower(UTF_B))) break;
      }

      if (a >= end_a) break;
    }
  }
  if (!ret) return 0;
  else return (ret < 0) ? -1 : 1;
}

/**
  Compares directory entries alphanumerically (case-insensitive).

  \note This comparison is UTF-8 aware.

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

  \note This comparison is UTF-8 aware.

  \see fl_casenumericsort()
*/

int fl_numericsort(struct dirent **A, struct dirent **B) {
  return numericsort(A, B, 1);
}
