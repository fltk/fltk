#
# "$Id$"
#
# CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
# Written by Michael Surette
#
# Copyright 1998-2015 by Bill Spitzak and others.
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
set(FLTK_VERSION_PATCH "4")
set(FLTK_VERSION "${FLTK_VERSION_MAJOR}.${FLTK_VERSION_MINOR}")
set(FLTK_VERSION_FULL "${FLTK_VERSION}.${FLTK_VERSION_PATCH}")

set(EXECUTABLE_OUTPUT_PATH ${FLTK_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${FLTK_BINARY_DIR}/lib)
set(ARCHIVE_OUTPUT_PATH ${FLTK_BINARY_DIR}/lib)

# Search for modules in the FLTK source dir first
set(CMAKE_MODULE_PATH "${FLTK_SOURCE_DIR}/CMake")

set(FLTK_INCLUDE_DIRS ${FLTK_BINARY_DIR} ${FLTK_SOURCE_DIR})
include_directories(${FLTK_INCLUDE_DIRS})

# Setup install locations
if(CMAKE_VERSION VERSION_GREATER 2.8.4)
    # Use GNUInstallDirs if available.
    include(GNUInstallDirs)
else()
    # Else set reasonable defaults.
    set(CMAKE_INSTALL_BINDIR bin)
    set(CMAKE_INSTALL_LIBDIR lib)
    set(CMAKE_INSTALL_INCLUDEDIR include)
    set(CMAKE_INSTALL_DATADIR share)
    set(CMAKE_INSTALL_MANDIR share/man)
endif(CMAKE_VERSION VERSION_GREATER 2.8.4)

set(FLTK_BINDIR ${CMAKE_INSTALL_BINDIR} CACHE PATH
    "Binary install path relative to CMAKE_INSTALL_PREFIX unless set to an absolute path.")
set(FLTK_LIBDIR ${CMAKE_INSTALL_LIBDIR} CACHE PATH
    "Library install path relative to CMAKE_INSTALL_PREFIX unless set to an absolute path.")
set(FLTK_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH
    "Public header install path relative to CMAKE_INSTALL_PREFIX unless set to an absolute path.")
set(FLTK_DATADIR ${CMAKE_INSTALL_DATADIR} CACHE PATH
    "Non-arch data install path relative to CMAKE_INSTALL_PREFIX unless set to an absolute path.")
set(FLTK_MANDIR ${CMAKE_INSTALL_MANDIR} CACHE PATH
    "Manual install path relative to CMAKE_INSTALL_PREFIX unless set to an absolute path.")
set(FLTK_DOCDIR ${CMAKE_INSTALL_DATADIR}/doc CACHE PATH
    "Non-arch doc install path relative to CMAKE_INSTALL_PREFIX unless set to an absolute path.")


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

# set where config files go
if(WIN32 AND NOT CYGWIN)
   set(FLTK_CONFIG_PATH CMake)
elseif(APPLE AND NOT OPTION_APPLE_X11)
   set(FLTK_CONFIG_PATH FLTK/.framework/Resources/CMake)
else()
   set(FLTK_CONFIG_PATH ${FLTK_DATADIR}/fltk)
endif(WIN32 AND NOT CYGWIN)

include(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

if(APPLE)
   set(HAVE_STRCASECMP 1)
   set(HAVE_DIRENT_H 1)
   set(HAVE_SNPRINTF 1)
   set(HAVE_VSNPRINTF 1)
   set(HAVE_SCANDIR 1)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated")
   if(OPTION_APPLE_X11)
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U__APPLE__")
     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U__APPLE__")
   else()
     set(__APPLE_QUARTZ__ 1)
     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa")
     set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -framework Cocoa")
   endif(OPTION_APPLE_X11)
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
