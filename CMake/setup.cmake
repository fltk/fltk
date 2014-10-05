#
# "$Id: CMakeLists.txt 10092 2014-02-02 00:49:50Z AlbrechtS $"
#
# CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
# Written by Michael Surette
#
# Copyright 1998-2014 by Bill Spitzak and others.
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
set(FLTK_VERSION_PATCH "3")
set(FLTK_VERSION "${FLTK_VERSION_MAJOR}.${FLTK_VERSION_MINOR}")
set(FLTK_VERSION_FULL "${FLTK_VERSION}.${FLTK_VERSION_PATCH}")

set(EXECUTABLE_OUTPUT_PATH ${FLTK_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${FLTK_BINARY_DIR}/lib)
set(ARCHIVE_OUTPUT_PATH ${FLTK_BINARY_DIR}/lib)

# Search for modules in the FLTK source dir first
set(CMAKE_MODULE_PATH "${FLTK_SOURCE_DIR}/CMake")

set(FLTK_INCLUDE_DIRS ${FLTK_BINARY_DIR} ${FLTK_SOURCE_DIR})
include_directories(${FLTK_INCLUDE_DIRS})

#######################################################################
# platform dependent information
#######################################################################

# fix no WIN32 defined issue
if(NOT WIN32)
    if(_WIN32)
        set(WIN32 _WIN32)
    elseif(__WIN32__)
        set(WIN32 __WIN32__)
    endif(_WIN32)
endif(NOT WIN32)

# set where config and example files go
if(WIN32 AND NOT CYGWIN)
   set(FLTK_CONFIG_PATH CMake)
   set(FLTK_EXAMPLES_PATH bin/fltk-examples)
elseif(APPLE)
   set(FLTK_CONFIG_PATH FLTK/.framework/Resources/CMake)
   set(FLTK_EXAMPLES_PATH share/fltk-examples)
else()
   set(FLTK_CONFIG_PATH lib/fltk)
   set(FLTK_EXAMPLES_PATH share/fltk-examples)
endif(WIN32 AND NOT CYGWIN)

include(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

if(APPLE)
   set(__APPLE_QUARTZ__ 1)
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
######## from ide/VisualC2010/config.h
        set(BORDER_WIDTH 2)
########
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
