#
# CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
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
# basic setup
#######################################################################

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(ARCHIVE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)

# Search for modules in the FLTK source dir first
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

# Setup install locations (requires CMake 2.8.4)

include(GNUInstallDirs)

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
# Initialize variables needed to collect include directories etc..
# Some of these variables are used to *append* other values later
#######################################################################

set(FLTK_BUILD_INCLUDE_DIRECTORIES "")
set(FLTK_IMAGE_INCLUDE_DIRECTORIES "")
set(FLTK_IMAGE_LIBRARIES "")
set(FLTK_IMAGE_LIBRARIES_SHARED "")

set(FLTK_CFLAGS "")
set(FLTK_LIBRARIES "")
set(FLTK_LIBRARIES_SHARED "")

# Remember root of FLTK source directory in case we're later in a subdirectory.
# Used for instance to find the source directory for doxygen docs

set(FLTK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(FLTK_SOURCE_DIR STREQUAL ${CMAKE_SOURCE_DIR})
  set(FLTK_IS_TOPLEVEL TRUE)
else()
  set(FLTK_IS_TOPLEVEL FALSE)
endif()

# Note: FLTK_INCLUDE_DIRS is used to export the required include directories
# in FLTKConfig.cmake etc.
# ### FIXME ### check if we really need this ...

set(FLTK_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# FLTK_BUILD_INCLUDE_DIRECTORIES is used to build the main FLTK lib

set(FLTK_BUILD_INCLUDE_DIRECTORIES)

# Some of these variables are used to *append* other values later

set(FLTK_LDLIBS "")
set(FLTK_LIBRARIES "")
set(IMAGELIBS "")
set(LDFLAGS "")
set(LINK_LIBS "")
set(STATICIMAGELIBS "")

#######################################################################
# platform dependent information
#######################################################################

# set where config files go
if(WIN32 AND NOT CYGWIN)
  set(FLTK_CONFIG_PATH CMake)
elseif(APPLE AND NOT FLTK_BACKEND_X11)
  set(FLTK_CONFIG_PATH FLTK.framework/Resources/CMake)
else()
  set(FLTK_CONFIG_PATH ${FLTK_DATADIR}/fltk)
endif(WIN32 AND NOT CYGWIN)

include(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

if(CMAKE_GENERATOR MATCHES "Xcode")
  # Tell Xcode to regenerate scheme information automatically whenever the
  # CMake configuration changes without asking the user
  set(CMAKE_XCODE_GENERATE_SCHEME 1)
endif()

if(APPLE)
  set(HAVE_STRCASECMP 1)
  set(HAVE_DIRENT_H 1)
  set(HAVE_SNPRINTF 1)
  set(HAVE_VSNPRINTF 1)
  set(HAVE_SCANDIR 1)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated")
  if(FLTK_BACKEND_X11)
    if(NOT(${CMAKE_SYSTEM_VERSION} VERSION_LESS 17.0.0)) # a.k.a. macOS version ≥ 10.13
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LIBCPP_HAS_THREAD_API_PTHREAD")
    endif(NOT(${CMAKE_SYSTEM_VERSION} VERSION_LESS 17.0.0))
  else()
    set(FLTK_COCOA_FRAMEWORKS "-framework Cocoa")
    set(UTI_CONDITION FALSE) # TRUE when framework UniformTypeIdentifiers is used
    set(SCK_CONDITION FALSE) # TRUE when framework ScreenCaptureKit is used
    string(LENGTH "${CMAKE_OSX_DEPLOYMENT_TARGET}" TARGET_LEN)
    string(LENGTH "${CMAKE_SYSTEM_VERSION}" SDK_LEN)
    if(TARGET_LEN GREATER 0)
      if( ${CMAKE_OSX_DEPLOYMENT_TARGET} VERSION_GREATER_EQUAL 11.0)
        set(UTI_CONDITION TRUE)
      endif()
      if( ${CMAKE_OSX_DEPLOYMENT_TARGET} VERSION_GREATER_EQUAL 15.0)
        set(SCK_CONDITION TRUE)
      endif()
    elseif(SDK_LEN GREATER 0)
      if( ${CMAKE_SYSTEM_VERSION} VERSION_GREATER_EQUAL 20.0 )
        set(UTI_CONDITION TRUE)
      endif()
      if( ${CMAKE_SYSTEM_VERSION} VERSION_GREATER_EQUAL 24.0 )
        set(SCK_CONDITION TRUE)
      endif()
    endif()
    if(UTI_CONDITION) # a.k.a. macOS version ≥ 11.0
      list(APPEND FLTK_COCOA_FRAMEWORKS "-framework UniformTypeIdentifiers")
      if(SCK_CONDITION) # a.k.a. macOS version ≥ 15.0
        list(APPEND FLTK_COCOA_FRAMEWORKS "-framework ScreenCaptureKit")
      endif(SCK_CONDITION)
    endif(UTI_CONDITION)
  endif(FLTK_BACKEND_X11)
endif(APPLE)

if(WIN32)
  # we do no longer define WIN32 or _WIN32 (since FLTK 1.4.0)
  # ... but if we did, we'd define _WIN32 (since FLTK 1.4.0)
  # add_definitions (-D_WIN32)
  if(MSVC)
    add_definitions (-DWIN32_LEAN_AND_MEAN)
    add_definitions (-D_CRT_SECURE_NO_WARNINGS)
    if(NOT MSVC_VERSION VERSION_LESS 1900) # Visual Studio 2015
      add_compile_options (/utf-8)         # equivalent to `/source-charset:utf-8 /execution-charset:utf-8`
    endif()
    set(BORDER_WIDTH 2)
  endif(MSVC)
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
CHECK_TYPE_SIZE("long long" HAVE_LONG_LONG)

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
