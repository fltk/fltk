/*
 * "$Id$"
 *
 * This is a placekeeper stub that pulls in scandir implementations for host
 * systems that do not provide a compatible one natively
 *
 * Copyright 1998-2013 by Bill Spitzak and others.
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


#if defined(WIN32) && !defined(__CYGWIN__)
#  include "scandir_win32.c"
#else
#  include <config.h>
#  ifndef HAVE_SCANDIR
#   include "scandir_posix.c"
#  endif /* HAVE_SCANDIR */
#endif

/* Avoid "ISO C forbids an empty translation unit" warning */
typedef int dummy;

/*
 * End of "$Id$".
 */
