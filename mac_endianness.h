/*
 * "$Id$"
 *
 * Mac-specific configuration file for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2011 by Bill Spitzak and others.
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
 
#ifdef __APPLE__
#  ifdef __BIG_ENDIAN__
#    define WORDS_BIGENDIAN 1
#  else
#    define WORDS_BIGENDIAN 0
#  endif
#endif

/*
 * End of "$Id$".
 */
