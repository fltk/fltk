/*
 * "$Id$"
 *
 * Configuration file for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2011 by Bill Spitzak and others.
 */

#define FLTK_DATADIR "/usr/local/share/fltk"
#define FLTK_DOCDIR "/usr/local/share/doc/fltk"
#define BORDER_WIDTH 2
#define HAVE_GL 1
#define HAVE_GL_GLU_H 1
#define USE_COLORMAP 1
#define HAVE_XINERAMA 0
#define USE_XFT 0
#define HAVE_XDBE 0
#define USE_XDBE HAVE_XDBE
#define __APPLE_QUARTZ__ 1
#define HAVE_OVERLAY 0
#define HAVE_GL_OVERLAY HAVE_OVERLAY
#if defined(__BIG_ENDIAN__)
#define WORDS_BIGENDIAN 1
#else
#define WORDS_BIGENDIAN 0
#endif
#define U16 unsigned short
#define U32 unsigned
#define HAVE_DIRENT_H 1
#define HAVE_SCANDIR 1
#define HAVE_VSNPRINTF 1
#define HAVE_SNPRINTF 1
#define HAVE_STRINGS_H 1
#define HAVE_STRCASECMP 1
#define HAVE_STRLCAT 1
#define HAVE_STRLCPY 1
#define HAVE_LOCALE_H 1
#define HAVE_LOCALECONV 1
#define HAVE_SYS_SELECT_H 1
#define USE_POLL 0
#define HAVE_LIBPNG 1
#define HAVE_LIBZ 1
#define HAVE_LIBJPEG 1
#define HAVE_PNG_H 1
#define HAVE_PNG_GET_VALID 1
#define HAVE_PNG_SET_TRNS_TO_ALPHA 1
#define HAVE_PTHREAD 1
#define HAVE_PTHREAD_H 1
#define HAVE_LONG_LONG 1
#define FLTK_LLFMT "%lld"
#define FLTK_LLCAST (long long)
#define HAVE_STRTOLL 1
#define HAVE_DLFCN_H 1
#define HAVE_DLSYM 1

/*
 * End of "$Id$".
 */
