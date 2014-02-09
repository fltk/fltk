#
# "$Id: CMakeLists.txt 10092 2014-02-02 00:49:50Z AlbrechtS $"
#
# CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
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
# basic setup
#######################################################################
# The FLTK version
set(FLTK_VERSION_MAJOR "1")
set(FLTK_VERSION_MINOR "3")
set(FLTK_VERSION_PATCH "2")
set(FLTK_VERSION "${FLTK_VERSION_MAJOR}.${FLTK_VERSION_MINOR}")
set(FLTK_VERSION_FULL "${FLTK_VERSION}.${FLTK_VERSION_PATCH}")

set(EXECUTABLE_OUTPUT_PATH ${FLTK_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${FLTK_BINARY_DIR}/lib)
set(ARCHIVE_OUTPUT_PATH ${FLTK_BINARY_DIR}/lib)

# Search for modules in the FLTK source dir first
set(CMAKE_MODULE_PATH "${FLTK_SOURCE_DIR}/CMake")

include_directories(${FLTK_BINARY_DIR} ${FLTK_SOURCE_DIR})

#######################################################################
# platform dependent information
#######################################################################

include(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

if(APPLE)
   set(__APPLE_QUARTZ__ 1)
   set(HAVE_STRTOLL 1)
   set(HAVE_STRCASECMP 1)
   set(HAVE_DIRENT_H 1)
   set(HAVE_SNPRINTF 1)
   set(HAVE_VSNPRINTF 1)
   set(HAVE_SCANDIR 1)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa")
   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -framework Cocoa")
endif(APPLE)

if(WIN32)
   if(MSVC)
      add_definitions(-DWIN32_LEAN_AND_MEAN)
      add_definitions(-D_CRT_SECURE_NO_WARNINGS)
   endif(MSVC)
   if(CMAKE_C_COMPILER_ID STREQUAL GNU)
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-subsystem,windows")
   endif(CMAKE_C_COMPILER_ID STREQUAL GNU)
   if(MINGW AND EXISTS /mingw)
      list(APPEND CMAKE_PREFIX_PATH /mingw)
   endif(MINGW AND EXISTS /mingw)
endif(WIN32)

#######################################################################
# size of ints
include(CheckTypeSize)

CHECK_TYPE_SIZE(short SIZEOF_SHORT)
CHECK_TYPE_SIZE(int   SIZEOF_INT)
CHECK_TYPE_SIZE(long  SIZEOF_LONG)
CHECK_TYPE_SIZE("long long"  HAVE_LONG_LONG)

if(${SIZEOF_SHORT} MATCHES "^2$")
   set(U16 "unsigned short")
endif(${SIZEOF_SHORT} MATCHES "^2$")

if(${SIZEOF_INT} MATCHES "^4$")
   set(U32 "unsigned")
else()
   if(${SIZEOF_LONG} MATCHES "^4$")
      set(U32 "unsigned long")
   endif(${SIZEOF_LONG} MATCHES "^4$")
endif(${SIZEOF_INT} MATCHES "^4$")

if(${SIZEOF_INT} MATCHES "^8$")
   set(U64 "unsigned")
else()
   if(${SIZEOF_LONG} MATCHES "^8$")
      set(U64 "unsigned long")
   endif(${SIZEOF_LONG} MATCHES "^8$")
endif(${SIZEOF_INT} MATCHES "^8$")
