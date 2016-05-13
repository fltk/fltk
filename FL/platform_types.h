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
  Fl_Offscreen, Fl_Bitmask, Fl_Region, FL_SOCKET, GLContext, struct dirent, struct stat,
  fl_intptr_t, fl_uintptr_t

  NOTE: *FIXME* AlbrechtS 13 Apr 2016 (concerning FL_SOCKET)
  ----------------------------------------------------------
    The Fl::add_fd() API is partially inconsistent because some of the methods
    explicitly use 'int', but the callback typedefs use FL_SOCKET. With the
    definition of FL_SOCKET below we can have different data sizes and
    different signedness of socket numbers on *some* platforms.
 */

/**
  \todo	typedef's fl_intptr_t and fl_uintptr_t should be documented.
*/
#ifdef _WIN64

#if defined(_MSC_VER)
# include <stddef.h>  /* M$VC */
#else
# include <stdint.h>
#endif

typedef intptr_t fl_intptr_t;
typedef uintptr_t fl_uintptr_t;

#else /* ! _WIN64 */

typedef long fl_intptr_t;
typedef unsigned long fl_uintptr_t;

#endif /* _WIN64 */


#ifdef __APPLE__
typedef struct CGContext* Fl_Offscreen;
typedef struct CGImage* Fl_Bitmask;
typedef struct flCocoaRegion* Fl_Region;
typedef int FL_SOCKET;
#ifdef __OBJC__
  @class NSOpenGLContext;
  typedef NSOpenGLContext* GLContext;
#elif defined(__cplusplus)
  typedef class NSOpenGLContext* GLContext;
#endif // __OBJC__

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
typedef struct HGLRC__ *GLContext;
#include <sys/stat.h>
struct dirent {char d_name[1];};

#elif defined(FL_PORTING)
# pragma message "FL_PORTING: define OS-dependent types"
typedef void* Fl_Offscreen;
typedef void* Fl_Bitmask;
typedef void *Fl_Region;
typedef int FL_SOCKET;
typedef void *GLContext;
# pragma message "FL_PORTING: define struct dirent and implement scandir() for the platform"
struct dirent {char d_name[1];};
# pragma message "FL_PORTING: define struct stat and implement stat() for the platform"
struct stat { /* the FLTK source code uses part of the stat() API */
  unsigned st_mode;
  unsigned st_size;
};
#define        S_IFDIR  0040000  /* directory */
#define        S_IFREG  0100000  /* regular */

#else /* X11 */

typedef unsigned long Fl_Offscreen;
typedef unsigned long Fl_Bitmask;
typedef struct _XRegion *Fl_Region;
typedef int FL_SOCKET;
typedef struct __GLXcontextRec *GLContext;
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
