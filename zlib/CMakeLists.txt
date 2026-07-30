#
# ZLIB library CMake configuration for the Fast Light Toolkit (FLTK).
#
# Copyright 1998-2026 by Bill Spitzak and others.
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

# source files for zlib

set(ZLIB_SRCS
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    inflate.c
    infback.c
    inftrees.c
    inffast.c
    trees.c
    uncompr.c
    zutil.c
)

#######################################################################

# Suppress some Visual Studio compiler warnings

set(msvc_warnings /wd4267 /wd4996)

#######################################################################
# Build the static library
#######################################################################

fl_add_library(fltk_z STATIC "${ZLIB_SRCS}")

if(MSVC)
  target_compile_options(fltk_z PRIVATE ${msvc_warnings})
endif(MSVC)

#######################################################################
# Build the shared library (optional)
#######################################################################

if(FLTK_BUILD_SHARED_LIBS)

  # ensure to export all symbols for Windows DLL's
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

  fl_add_library(fltk_z SHARED "${ZLIB_SRCS}")

  if(MSVC)
    target_compile_options(fltk_z-shared PRIVATE ${msvc_warnings})
  endif(MSVC)

endif(FLTK_BUILD_SHARED_LIBS)

set(FLTK_LIBRARIES ${FLTK_LIBRARIES} PARENT_SCOPE)
set(FLTK_LIBRARIES_SHARED ${FLTK_LIBRARIES_SHARED} PARENT_SCOPE)

#######################################################################
# Fix legacy MinGW (mingw.org) build - since zlib 1.3.2
#######################################################################

# The older MinGW (from the earlier mingw.org website) does not
# provide EWOULDBLOCK whose *usage* was introduced in zlib 1.3.2.
# If it doesn't exist we
#   #define EWOULDBLOCK EAGAIN
# for zlib compilation (for both static and shared libs).
# This check and related target_compile_definitions() may be removed
# if this is "fixed" upstream in a later zlib version.

include(CheckSymbolExists)
check_symbol_exists(EWOULDBLOCK "errno.h" HAVE_EWOULDBLOCK)
mark_as_advanced(HAVE_EWOULDBLOCK)

if(NOT HAVE_EWOULDBLOCK)
  foreach(_tgt fltk_z fltk_z-shared)
    if(TARGET ${_tgt})
      target_compile_definitions(${_tgt} PRIVATE EWOULDBLOCK=EAGAIN)
    endif()
  endforeach()
endif()

#######################################################################
# Install the library headers
#######################################################################

install(FILES zconf.h zlib.h zutil.h
        DESTINATION ${FLTK_INCLUDEDIR}/FL/images
)
