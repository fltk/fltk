/*
 * Mac-specific configuration file for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2011 by Bill Spitzak and others.
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

#ifdef __APPLE__
#  ifdef __BIG_ENDIAN__
#    define WORDS_BIGENDIAN 1
#  else
#    define WORDS_BIGENDIAN 0
#  endif
#endif
