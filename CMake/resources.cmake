#
# "$Id: CMakeLists.txt 10092 2014-02-02 00:49:50Z AlbrechtS $"
#
# Main CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
# Written by Michael Surette
#
# Copyright 1998-2010 by Bill Spitzak and others.
#
# This library is free software. Distribution and use rights are outlined in
# the file "COPYING" which should have been included with this file.  If this
# file is missing or damaged, see the license at:
#
#     http://www.fltk.org/COPYING.php
#
# Please report all bugs and problems on the following page:
#
#     http://www.fltk.org/str.php
#

#######################################################################
# check for headers, libraries and functions
#######################################################################
# headers
find_file(HAVE_ALSA_ASOUNDLIB_H alsa/asoundlib.h)
find_file(HAVE_DIRENT_H dirent.h)
find_file(HAVE_DLFCN_H dlfcn.h)
find_file(HAVE_FREETYPE_H freetype.h PATH_SUFFIXES freetype2 freetype2/freetype)
find_file(HAVE_GL_GL_H GL/gl.h)
find_file(HAVE_GL_GLU_H GL/glu.h)
find_file(HAVE_LIBPNG_PNG_H libpng/png.h)
find_file(HAVE_LOCALE_H locale.h)
find_file(HAVE_NDIR_H ndir.h)
find_file(HAVE_OPENGL_GLU_H OpenGL/glu.h)
find_file(HAVE_PNG_H png.h)
find_file(HAVE_PTHREAD_H pthread.h)
find_file(HAVE_STDIO_H stdio.h)
find_file(HAVE_STRINGS_H strings.h)
find_file(HAVE_SYS_DIR_H sys/dir.h)
find_file(HAVE_SYS_NDIR_H sys/ndir.h)
find_file(HAVE_SYS_SELECT_H sys/select.h)
find_file(HAVE_SYS_STDTYPES_H sys/stdtypes.h)
find_path(HAVE_XDBE_H Xdbe.h PATH_SUFFIXES X11/extensions extensions)

mark_as_advanced(HAVE_ALSA_ASOUNDLIB_H HAVE_DIRENT_H HAVE_DLFCN_H)
mark_as_advanced(HAVE_FREETYPE_H HAVE_GL_GL_H HAVE_GL_GLU_H)
mark_as_advanced(HAVE_LIBPNG_PNG_H HAVE_LOCALE_H HAVE_NDIR_H)
mark_as_advanced(HAVE_OPENGL_GLU_H HAVE_PNG_H HAVE_PTHREAD_H)
mark_as_advanced(HAVE_STDIO_H HAVE_STRINGS_H HAVE_SYS_DIR_H)
mark_as_advanced(HAVE_SYS_NDIR_H HAVE_SYS_SELECT_H)
mark_as_advanced(HAVE_SYS_STDTYPES_H HAVE_XDBE_H)

# where to find freetype headers
find_path(FREETYPE_PATH freetype.h PATH_SUFFIXES freetype2)
find_path(FREETYPE_PATH freetype/freetype.h PATH_SUFFIXES freetype2)
if(FREETYPE_PATH)
   include_directories(${FREETYPE_PATH})
endif(FREETYPE_PATH)
mark_as_advanced(FREETYPE_PATH)

#######################################################################
# libraries
find_library(LIB_CAIRO cairo)
find_library(LIB_dl dl)
find_library(LIB_fontconfig fontconfig)
find_library(LIB_freetype freetype)
find_library(LIB_GL GL)
find_library(LIB_MesaGL MesaGL)
find_library(LIB_jpeg jpeg)
find_library(LIB_png png)
find_library(LIB_zlib z)

mark_as_advanced(LIB_CAIRO LIB_dl LIB_fontconfig LIB_freetype)
mark_as_advanced(LIB_GL LIB_MesaGL)
mark_as_advanced(LIB_jpeg LIB_png LIB_zlib)

#######################################################################
# functions
include(CheckFunctionExists)

if(HAVE_DLFCN_H)
   set(CMAKE_REQUIRED_LIBRARIES dl)
   set(HAVE_DLFCN_H 1)
endif(HAVE_DLFCN_H)
CHECK_FUNCTION_EXISTS(dlsym                  HAVE_DLSYM)

CHECK_FUNCTION_EXISTS(localeconv             HAVE_LOCALECONV)

if(LIB_png)
   set(CMAKE_REQUIRED_LIBRARIES png)
endif(LIB_png)
CHECK_FUNCTION_EXISTS(png_get_valid          HAVE_PNG_GET_VALID)
CHECK_FUNCTION_EXISTS(png_set_tRNS_to_alpha  HAVE_PNG_SET_TRNS_TO_ALPHA)

CHECK_FUNCTION_EXISTS(scandir                HAVE_SCANDIR)
CHECK_FUNCTION_EXISTS(snprintf               HAVE_SNPRINTF)

# not really true but we convert strcasecmp calls to _stricmp calls in flstring.h
if(MSVC)
   set(HAVE_STRCASECMP 1)
endif(MSVC)

CHECK_FUNCTION_EXISTS(strcasecmp             HAVE_STRCASECMP)

CHECK_FUNCTION_EXISTS(strlcat                HAVE_STRLCAT)
CHECK_FUNCTION_EXISTS(strlcpy                HAVE_STRLCPY)
CHECK_FUNCTION_EXISTS(vsnprintf              HAVE_VSNPRINTF)

set(CMAKE_REQUIRED_LIBRARIES)

if(HAVE_SCANDIR AND NOT HAVE_SCANDIR_POSIX)
   set(MSG "POSIX compatible scandir")
   message(STATUS "Looking for ${MSG}")
   try_compile(V
      ${FLTK_BINARY_DIR}
      ${FLTK_SOURCE_DIR}/CMake/posixScandir.cxx
      )
   if(V)
      message(STATUS "${MSG} - found")
      set(HAVE_SCANDIR_POSIX 1 CACHE INTERNAL "")
   else()
      message(STATUS "${MSG} - not found")
      set(HAVE_SCANDIR_POSIX HAVE_SCANDIR_POSIX-NOTFOUND)
   endif(V)
endif(HAVE_SCANDIR AND NOT HAVE_SCANDIR_POSIX)
mark_as_advanced(HAVE_SCANDIR_POSIX)
