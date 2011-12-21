/*
 * "$Id$"
 *
 * This is a placekeeper stub that puuls in scandir implementations for host
 * systems that do not provide a compatible one natively
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


#if defined(WIN32) && !defined(__CYGWIN__)
#  include "scandir_win32.c"
#else

#  include "flstring.h"

/* NOTE: Most (all?) modern non-WIN32 hosts DO have a usable scandir */
#  if !HAVE_SCANDIR 
#    include <stdlib.h>
#    include <sys/types.h>
#    include <errno.h>

#    if HAVE_DIRENT_H
#      include <dirent.h>
#      define NAMLEN(dirent) strlen((dirent)->d_name)
#    else /* HAVE_DIRENT_H */
#      define dirent direct
#      define NAMLEN(dirent) (dirent)->d_namlen
#      if HAVE_SYS_NDIR_H
#        include <sys/ndir.h>
#      endif /* HAVE_SYS_NDIR_H */
#      if HAVE_SYS_DIR_H
#        include <sys/dir.h>
#      endif /* HAVE_SYS_DIR_H */
#      if HAVE_NDIR_H
#        include <ndir.h>
#      endif /* HAVE_NDIR_H */
#    endif

/* This warning added to help identify any non-WIN32 hosts that actually try to use 
 * our "private" implementation of the scandir function, which was suspect... */
#    if defined(__GNUC__)
#      warning Attempting to use the deprecated scandir() replacement function
#    endif /*__GNUC__*/
#    error No compatible scandir implementation found (STR 2687 applies!)

#  endif /* !HAVE_SCANDIR */
#endif

/*
 * End of "$Id$".
 */
