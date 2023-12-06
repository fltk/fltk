#
# This file sets variables for common use in export.cmake and install.cmake
# Written by Michael Surette
#
# Copyright 1998-2020 by Bill Spitzak and others.
#
# This library is free software. Distribution and use rights are outlined in
# the file "COPYING" which should have been included with this file.  If this
# file is missing or damaged, see the license at:
#
#     https://www.fltk.org/COPYING.php
#
# Please see the following page on how to report bugs and issues:
#
#     https://www.fltk.org/bugs.php
#

#######################################################################

set (DEBUG_VARIABLES_CMAKE 0)
if (DEBUG_VARIABLES_CMAKE)
  message (STATUS "[** variables.cmake **]")
  fl_debug_var (HAVE_DLSYM)
  fl_debug_var (CMAKE_DL_LIBS)
  fl_debug_var (CMAKE_EXE_LINKER_FLAGS)
  fl_debug_var (LDFLAGS)
  fl_debug_var (LIBS)
  fl_debug_var (GLLIBS)
  fl_debug_var (IMAGELIBS)
  fl_debug_var (STATICIMAGELIBS)
  fl_debug_var (FLTK_LDLIBS)
  fl_debug_var (LIB_jpeg)
  fl_debug_var (LIB_png)
  fl_debug_var (LIB_zlib)
endif (DEBUG_VARIABLES_CMAKE)

#######################################################################
# add several libraries
# FIXME: libraries may need reordering.
# FIXME: check fontconfig conditions (only if Xft is used or ...)

if (WIN32)
  list (APPEND FLTK_LDLIBS -lole32 -luuid -lcomctl32 -lws2_32)
elseif (APPLE)
  list (APPEND FLTK_LDLIBS "-framework Cocoa")
else ()
  list (APPEND FLTK_LDLIBS -lm)
endif (WIN32)

if (LIB_fontconfig)
  list(APPEND FLTK_LDLIBS -lfontconfig)
endif (LIB_fontconfig)

# add "-ldl" or whatever is necessary according to CMake (CMAKE_DL_LIBS)
if (HAVE_DLSYM)
  foreach (LIB ${CMAKE_DL_LIBS})
    list (APPEND FLTK_LDLIBS "-l${LIB}")
  endforeach ()
endif (HAVE_DLSYM)

#######################################################################
# Set variables for fltk-config (generated from fltk-config.in)
#######################################################################

# Variables in fltk-config.in (@VAR@) are used in configure(.ac)
# and in CMake so their names and usage must be synchronized.
# CMake generates two instances of fltk-config, one that can be used
# directly in the build tree (see export.cmake) and one that is copied
# to the installation directory (see install.cmake). Common variables
# should be set here, whereas variables with different values should
# be set in install.cmake or export.cmake, respectively.

if (WIN32)
  set (LDFLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif ()

set (IMAGELIBS)
set (STATICIMAGELIBS)

if (FLTK_BUILTIN_JPEG_FOUND)
  list (APPEND IMAGELIBS -lfltk_jpeg)
  list (APPEND STATICIMAGELIBS \$libdir/libfltk_jpeg.a)
else ()
  if (LIB_jpeg)
    list (APPEND IMAGELIBS -ljpeg)
    list (APPEND STATICIMAGELIBS -ljpeg)
  endif (LIB_jpeg)
endif (FLTK_BUILTIN_JPEG_FOUND)

if (FLTK_BUILTIN_PNG_FOUND)
  list (APPEND IMAGELIBS -lfltk_png)
  list (APPEND STATICIMAGELIBS \$libdir/libfltk_png.a)
else ()
  if (LIB_png)
    list (APPEND IMAGELIBS -lpng)
    list (APPEND STATICIMAGELIBS -lpng)
  endif (LIB_png)
endif (FLTK_BUILTIN_PNG_FOUND)

if (FLTK_BUILTIN_ZLIB_FOUND)
  list (APPEND IMAGELIBS -lfltk_z)
  list (APPEND STATICIMAGELIBS \$libdir/libfltk_z.a)
else ()
  if (LIB_zlib)
    list (APPEND IMAGELIBS -lz)
    list (APPEND STATICIMAGELIBS -lz)
  endif (LIB_zlib)
endif (FLTK_BUILTIN_ZLIB_FOUND)

string (REPLACE ";" " " IMAGELIBS "${IMAGELIBS}")
string (REPLACE ";" " " STATICIMAGELIBS "${STATICIMAGELIBS}")

#######################################################################
set (CC ${CMAKE_C_COMPILER})
set (CXX ${CMAKE_CXX_COMPILER})

set (ARCHFLAGS ${OPTION_ARCHFLAGS})

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_UPPER)
if (${BUILD_UPPER})
  set (CFLAGS "${CMAKE_C_FLAGS_${BUILD_UPPER}} ${CFLAGS}")
endif (${BUILD_UPPER})

set (CFLAGS "${OPTION_OPTIM} ${CMAKE_C_FLAGS} ${CFLAGS}")
foreach (arg ${FLTK_CFLAGS})
  set (CFLAGS "${CFLAGS} ${arg}")
endforeach (arg ${FLTK_CFLAGS})

set (CXXFLAGS "${CFLAGS}")

foreach (arg ${FLTK_LDLIBS})
  set (LINK_LIBS "${LINK_LIBS} ${arg}")
endforeach (arg ${FLTK_LDLIBS})

set (LIBS "${LINK_LIBS}")

if (${CMAKE_SYSTEM_NAME} STREQUAL "AIX")
    set (SHAREDSUFFIX "_s")
else ()
    set (SHAREDSUFFIX "")
endif (${CMAKE_SYSTEM_NAME} STREQUAL "AIX")

if (DEBUG_VARIABLES_CMAKE)
  message (STATUS "") # empty line
  fl_debug_var (HAVE_DLSYM)
  fl_debug_var (CMAKE_DL_LIBS)
  fl_debug_var (CMAKE_EXE_LINKER_FLAGS)
  fl_debug_var (LDFLAGS)
  fl_debug_var (LIBS)
  fl_debug_var (GLLIBS)
  fl_debug_var (IMAGELIBS)
  fl_debug_var (STATICIMAGELIBS)
  fl_debug_var (FLTK_LDLIBS)
  fl_debug_var (LIB_jpeg)
  fl_debug_var (LIB_png)
  fl_debug_var (LIB_zlib)
  message (STATUS "[** end of variables.cmake **]")
endif (DEBUG_VARIABLES_CMAKE)
unset (DEBUG_VARIABLES_CMAKE)
