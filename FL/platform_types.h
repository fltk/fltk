/*
 * "$Id: platform_types.h 11467 2016-03-29 19:41:14Z manolo $"
 *
 * Copyright 2016 by Bill Spitzak and others.
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

#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

/* Platform-dependent types are defined here.
  These types must be defined by any platform:
  Fl_Offscreen, Fl_Bitmask, Fl_Region, FL_SOCKET, struct dirent, struct stat

  NOTE: *FIXME* AlbrechtS 13 Apr 2016 (concerning FL_SOCKET)
  ----------------------------------------------------------
    The socket API is partially inconsistent because some of the methods
    use int explicitly, but the callback typedefs use FL_SOCKET. With the
    definition of FL_SOCKET below we can have different data sizes and
    different signedness of socket numbers on *some* platforms.
 */

#ifdef __APPLE__
typedef struct CGContext* Fl_Offscreen;
typedef struct CGImage* Fl_Bitmask;
typedef struct flCocoaRegion* Fl_Region;
typedef int FL_SOCKET;
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#  define FL_COMMAND	FL_META
#  define FL_CONTROL 	FL_CTRL

#elif defined(WIN32)
typedef struct HBITMAP__ *HBITMAP;
typedef HBITMAP Fl_Offscreen;
typedef HBITMAP Fl_Bitmask;
typedef struct HRGN__ *Fl_Region;
# if defined(_WIN64) && defined(_MSC_VER)
typedef  unsigned __int64 FL_SOCKET;	/* *FIXME* - FL_SOCKET (see above) */
# else
typedef  int FL_SOCKET;
# endif
#include <sys/stat.h>
struct dirent {char d_name[1];};

#elif defined(FL_PORTING)
# pragma message "FL_PORTING: define OS-dependent types"
typedef void* Fl_Offscreen;
typedef void* Fl_Bitmask;
typedef void *Fl_Region;
typedef int FL_SOCKET;
# pragma message "FL_PORTING: define struct dirent and implement scandir() for the platform"
struct dirent {char d_name[1];};
# pragma message "FL_PORTING: define struct stat and implement stat() for the platform"
struct stat { /* the FLTK source code uses part of the stat() API */
  unsigned st_mode;
  unsigned st_size;
};
#define        S_IFDIR  0040000  /* directory */
#define        S_IFREG  0100000  /* regular */

#else
typedef unsigned long Fl_Offscreen;
typedef unsigned long Fl_Bitmask;
typedef struct _XRegion *Fl_Region;
typedef int FL_SOCKET;
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#endif /* __APPLE__ */


#ifndef __APPLE__
#  define FL_COMMAND	FL_CTRL   /**< An alias for FL_CTRL on WIN32 and X11, or FL_META on MacOS X */
#  define FL_CONTROL	FL_META   /**< An alias for FL_META on WIN32 and X11, or FL_CTRL on MacOS X */
#endif

#endif /* PLATFORM_TYPES_H */

/*
 * End of "$Id: platform_types.h 11467 2016-03-29 19:41:14Z manolo $".
 */
