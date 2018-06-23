/*
 * "$Id$"
 *
 * Copyright 2016-2018 by Bill Spitzak and others.
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

/** \file
 Definitions of platform-dependent types.
 The exact nature of these types varies with the platform.
 Therefore, portable FLTK applications should not assume these types
 have a specific size, or that they are pointers.
 */

#ifdef FL_DOXYGEN

/** An integral type large enough to store a pointer or a long value.
 A pointer value can be safely cast to fl_intptr_t, and later cast back
 to its initial pointer type without change to the pointer value.
 A variable of type fl_intptr_t can also store a long int value. */
typedef opaque fl_intptr_t;
/** An unsigned integral type large enough to store a pointer or an unsigned long value.
 A pointer value can be safely cast to fl_uintptr_t, and later cast back
 to its initial pointer type without change to the pointer value.
 A variable of type fl_uintptr_t can also store an unsigned long int value. */
typedef opaque fl_uintptr_t;

typedef opaque Fl_Offscreen; /**< an offscreen drawing buffer */
typedef opaque Fl_Bitmask; /**< mask */
typedef opaque Fl_Region; /**< a region made of several rectangles */
typedef opaque FL_SOCKET; /**< socket or file descriptor */
typedef opaque GLContext; /**< an OpenGL graphics context, into which all OpenGL calls are rendered */

#  define FL_COMMAND  opaque   /**< An alias for FL_CTRL on Windows and X11, or FL_META on MacOS X */
#  define FL_CONTROL  opaque   /**< An alias for FL_META on Windows and X11, or FL_CTRL on MacOS X */

#else

#ifndef FL_PLATFORM_TYPES_H
#define FL_PLATFORM_TYPES_H

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

#ifdef _WIN64

#if defined(_MSC_VER)
# include <stddef.h>  /* M$VC */
#else
# include <stdint.h>
#endif

typedef intptr_t fl_intptr_t;
typedef uintptr_t fl_uintptr_t;

#elif defined(__ANDROID__)

#include <sys/stat.h>
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
#endif /* __OBJC__ */

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#  define FL_COMMAND	FL_META
#  define FL_CONTROL 	FL_CTRL

#elif defined(_WIN32)
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

#elif defined(__ANDROID__)

#ifdef __cplusplus
typedef class Fl_Rect_Region *Fl_Region;
#else
typedef struct Fl_Rect_Region *Fl_Region;
#endif

// TODO: the types below have not yet been ported
typedef unsigned long Fl_Offscreen;
typedef unsigned long Fl_Bitmask;
typedef int FL_SOCKET;
typedef struct __GLXcontextRec *GLContext;
#include <sys/types.h>
#include <dirent.h>

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
#  define FL_COMMAND	FL_CTRL   /**< An alias for FL_CTRL on Windows and X11, or FL_META on MacOS X */
#  define FL_CONTROL	FL_META   /**< An alias for FL_META on Windows and X11, or FL_CTRL on MacOS X */
#endif

#endif /* FL_PLATFORM_TYPES_H */

#endif // FL_DOXYGEN

/*
 * End of "$Id$".
 */
