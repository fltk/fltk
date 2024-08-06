#
# This file sets variables for common use in export.cmake and install.cmake
# Originally written by Michael Surette
#
# Copyright 1998-2024 by Bill Spitzak and others.
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

set(DEBUG_VARIABLES_CMAKE 0)
if(DEBUG_VARIABLES_CMAKE)
  message(STATUS "[** variables.cmake **]")
  fl_debug_var(HAVE_DLSYM)
  fl_debug_var(CMAKE_DL_LIBS)
  fl_debug_var(CMAKE_EXE_LINKER_FLAGS)
  fl_debug_var(LDFLAGS)
  fl_debug_var(GLLIBS)
  fl_debug_var(IMAGELIBS)
  fl_debug_var(STATICIMAGELIBS)
  fl_debug_var(FLTK_LDLIBS)
  fl_debug_var(LIB_jpeg)
  fl_debug_var(LIB_png)
  fl_debug_var(LIB_zlib)
  fl_debug_var(FLTK_LIBRARIES)
endif(DEBUG_VARIABLES_CMAKE)

#######################################################################
# add several libraries
# FIXME: libraries may need reordering.
# FIXME: check fontconfig conditions (only if Xft is used or ...)

if(WIN32)
  list(APPEND FLTK_LDLIBS -lole32 -luuid -lcomctl32 -lws2_32 -lwinspool)
elseif(APPLE AND NOT FLTK_BACKEND_X11)
  list(APPEND FLTK_LDLIBS ${FLTK_COCOA_FRAMEWORKS})
elseif(FLTK_BACKEND_WAYLAND)
  foreach(_lib WLDCURSOR WLDCLIENT XKBCOMMON DBUS)
    list(APPEND FLTK_LDLIBS "${${_lib}_LDFLAGS}")
  endforeach()
  if(USE_SYSTEM_LIBDECOR)
    list(APPEND FLTK_LDLIBS ${SYSTEM_LIBDECOR_LDFLAGS})
  endif(USE_SYSTEM_LIBDECOR)
else()
  list(APPEND FLTK_LDLIBS -lm)
endif(WIN32)

if(LIB_fontconfig)
  list(APPEND FLTK_LDLIBS -lfontconfig)
endif(LIB_fontconfig)

# add "-ldl" or whatever is necessary according to CMake (CMAKE_DL_LIBS)
if(HAVE_DLSYM)
  foreach(LIB ${CMAKE_DL_LIBS})
    list(APPEND FLTK_LDLIBS "-l${LIB}")
  endforeach()
endif(HAVE_DLSYM)

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

if(WIN32)
  set(LDFLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif()

set(IMAGELIBS)
set(STATICIMAGELIBS)

if(FLTK_USE_BUNDLED_JPEG)
  list(APPEND IMAGELIBS -lfltk_jpeg)
  list(APPEND STATICIMAGELIBS \$libdir/libfltk_jpeg.a)
else()
  if(LIB_jpeg)
    # fl_debug_var(LIB_jpeg)
    list(APPEND IMAGELIBS ${LIB_jpeg})
    list(APPEND STATICIMAGELIBS ${LIB_jpeg})
  endif(LIB_jpeg)
endif(FLTK_USE_BUNDLED_JPEG)

if(FLTK_USE_BUNDLED_PNG)
  list(APPEND IMAGELIBS -lfltk_png)
  list(APPEND STATICIMAGELIBS \$libdir/libfltk_png.a)
else()
  if(LIB_png)
    # fl_debug_var(LIB_png)
    list(APPEND IMAGELIBS ${LIB_png})
    list(APPEND STATICIMAGELIBS ${LIB_png})
  endif(LIB_png)
endif(FLTK_USE_BUNDLED_PNG)

if(FLTK_USE_BUNDLED_ZLIB)
  list(APPEND IMAGELIBS -lfltk_z)
  list(APPEND STATICIMAGELIBS \$libdir/libfltk_z.a)
else()
  if(LIB_zlib)
    list(APPEND IMAGELIBS ${LIB_zlib})
    list(APPEND STATICIMAGELIBS ${LIB_zlib})
  endif(LIB_zlib)
endif(FLTK_USE_BUNDLED_ZLIB)

# remove duplicates from CMake "list" variables for fltk-config

list(REMOVE_DUPLICATES GLLIBS)
list(REMOVE_DUPLICATES FLTK_LDLIBS)
list(REMOVE_DUPLICATES IMAGELIBS)
list(REMOVE_DUPLICATES STATICIMAGELIBS)

# convert CMake lists to strings with spaces for fltk-config

string(REPLACE ";" " " GLLIBS           "${GLLIBS}")
string(REPLACE ";" " " LIBS             "${FLTK_LDLIBS}")
string(REPLACE ";" " " IMAGELIBS        "${IMAGELIBS}")
string(REPLACE ";" " " STATICIMAGELIBS  "${STATICIMAGELIBS}")

#######################################################################
set(CC ${CMAKE_C_COMPILER})
set(CXX ${CMAKE_CXX_COMPILER})

set(ARCHFLAGS ${FLTK_ARCHFLAGS})

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_UPPER)
if(${BUILD_UPPER})
  set(CFLAGS "${CMAKE_C_FLAGS_${BUILD_UPPER}} ${CFLAGS}")
endif(${BUILD_UPPER})

set(CFLAGS "${FLTK_OPTION_OPTIM} ${CMAKE_C_FLAGS} ${CFLAGS}")
foreach(arg ${FLTK_CFLAGS})
  set(CFLAGS "${CFLAGS} ${arg}")
endforeach(arg ${FLTK_CFLAGS})

set(CXXFLAGS "${CFLAGS}")

if(${CMAKE_SYSTEM_NAME} STREQUAL "AIX")
    set(SHAREDSUFFIX "_s")
else()
    set(SHAREDSUFFIX "")
endif(${CMAKE_SYSTEM_NAME} STREQUAL "AIX")

if(DEBUG_VARIABLES_CMAKE)
  message(STATUS "") # empty line
  fl_debug_var(HAVE_DLSYM)
  fl_debug_var(CMAKE_DL_LIBS)
  fl_debug_var(CMAKE_EXE_LINKER_FLAGS)
  fl_debug_var(LDFLAGS)
  fl_debug_var(FLTK_LDLIBS)
  fl_debug_var(LIBS)
  fl_debug_var(GLLIBS)
  fl_debug_var(IMAGELIBS)
  fl_debug_var(STATICIMAGELIBS)
  fl_debug_var(LIB_jpeg)
  fl_debug_var(LIB_png)
  fl_debug_var(LIB_zlib)
  fl_debug_var(FLTK_LIBRARIES)
  message(STATUS "[** end of variables.cmake **]")
endif(DEBUG_VARIABLES_CMAKE)
unset(DEBUG_VARIABLES_CMAKE)
