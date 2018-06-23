/*
 * "$Id$"
 *
 * Configuration file for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2018 by Bill Spitzak and others.
 */

#ifndef FL_CONFIG_LIB_H
#define FL_CONFIG_LIB_H

#include <config.h>


/* find the right graphics configuration */
#if !defined(FL_CFG_GFX_XLIB) && !defined(FL_CFG_GFX_QUARTZ) && !defined(FL_CFG_GFX_GDI)

#ifdef __APPLE__ /* default configurations */
# define FL_CFG_GFX_QUARTZ
# ifdef HAVE_GL
#  define FL_CFG_GFX_OPENGL
# endif
#elif defined(_WIN32)
# define FL_CFG_GFX_GDI
# ifdef HAVE_GL
#  define FL_CFG_GFX_OPENGL
# endif
#elif defined(USE_X11) /* X11 */
# define FL_CFG_GFX_XLIB
# ifdef HAVE_GL
#  define FL_CFG_GFX_OPENGL
# endif
#endif

#endif


/* find the right printer driver configuration */
#if !defined(FL_CFG_PRN_PS) && !defined(FL_CFG_PRN_QUARTZ) && !defined(FL_CFG_PRN_WIN32)

#ifdef __APPLE__ /* default configurations */
# define FL_CFG_PRN_QUARTZ
#elif defined(_WIN32)
# define FL_CFG_PRN_WIN32
#elif defined(USE_X11) /* X11 */
# define FL_CFG_PRN_PS
#endif

#endif


/* find the right window manager configuration */
#if !defined(FL_CFG_WIN_X11) && !defined(FL_CFG_WIN_COCOA) && !defined(FL_CFG_WIN_WIN32)

#ifdef __APPLE__ /* default configurations */
# define FL_CFG_WIN_COCOA
#elif defined(_WIN32)
# define FL_CFG_WIN_WIN32
#elif defined(USE_X11) /* X11 */
# define FL_CFG_WIN_X11
#endif

#endif


/* find the right system configuration */
#if !defined(FL_CFG_SYS_POSIX) && !defined(FL_CFG_SYS_WIN32)

#ifdef __APPLE__ /* default configurations */
# define FL_CFG_SYS_POSIX
#elif defined(_WIN32)
# define FL_CFG_SYS_WIN32
#elif defined(USE_X11) /* X11 */
# define FL_CFG_SYS_POSIX
#endif

#endif


#endif

/*
 * End of "$Id$".
 */
