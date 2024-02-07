#
# JPEG library CMake configuration for the Fast Light Toolkit (FLTK).
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

# memmgr back ends: compile only one of these into a working library
# (For now, let's use the mode that requires the image fit into memory.
# This is the recommended mode for Win32 anyway.)

set(systemdependent_SRCS jmemnobs.c)

# library object files common to compression and decompression

set(common_SRCS
    jaricom.c jcomapi.c jutils.c jerror.c jmemmgr.c
)

# compression library object files

set(compression_SRCS
    jcapimin.c jcapistd.c jcarith.c jctrans.c jcparam.c jdatadst.c jcinit.c
    jcmaster.c jcmarker.c jcmainct.c jcprepct.c jccoefct.c jccolor.c
    jcsample.c jchuff.c jcdctmgr.c jfdctfst.c jfdctflt.c
    jfdctint.c
)

# decompression library object files

set(decompression_SRCS
    jdapimin.c jdapistd.c jdarith.c jdtrans.c jdatasrc.c jdmaster.c
    jdinput.c jdmarker.c jdhuff.c jdmainct.c jdcoefct.c
    jdpostct.c jddctmgr.c jidctfst.c jidctflt.c jidctint.c
    jdsample.c jdcolor.c jquant1.c jquant2.c jdmerge.c
)

list(APPEND BUILD_SRCS "${systemdependent_SRCS};${common_SRCS}")
list(APPEND BUILD_SRCS "${compression_SRCS};${decompression_SRCS}")

#######################################################################
# Set common variables for static and shared builds
#######################################################################

set(_include_dirs
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Suppress Visual Studio compiler warnings

set(msvc_warnings /wd4267)

#######################################################################
# Build the static library
#######################################################################

fl_add_library(fltk_jpeg STATIC "${BUILD_SRCS}")
set(target     fltk_jpeg)

target_include_directories(${target} PUBLIC ${_include_dirs})

if(MSVC)
  target_compile_options(${target} PRIVATE ${msvc_warnings})
endif(MSVC)

list(APPEND FLTK_IMAGE_LIBRARIES fltk::jpeg)

# Propagate to parent scope (modified by fl_add_library and here)
set(FLTK_LIBRARIES       ${FLTK_LIBRARIES}       PARENT_SCOPE)
set(FLTK_IMAGE_LIBRARIES ${FLTK_IMAGE_LIBRARIES} PARENT_SCOPE)

#######################################################################
# Build the shared library (optional)
#######################################################################

if(FLTK_BUILD_SHARED_LIBS)

  # ensure to export all symbols for Windows DLL's
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

  fl_add_library(fltk_jpeg SHARED "${BUILD_SRCS}")
  set(target     fltk_jpeg-shared)

  target_include_directories(${target} PUBLIC ${_include_dirs})

  if(MSVC)
    target_compile_options(${target} PRIVATE ${msvc_warnings})
  endif(MSVC)

  list(APPEND FLTK_IMAGE_LIBRARIES_SHARED fltk::jpeg-shared)

  # Propagate to parent scope (modified by fl_add_library and here)
  set(FLTK_LIBRARIES_SHARED       ${FLTK_LIBRARIES_SHARED}       PARENT_SCOPE)
  set(FLTK_IMAGE_LIBRARIES_SHARED ${FLTK_IMAGE_LIBRARIES_SHARED} PARENT_SCOPE)

endif(FLTK_BUILD_SHARED_LIBS)

#######################################################################
# Install the library headers
#######################################################################

install(FILES
          jconfig.h jerror.h jmorecfg.h jpeglib.h fltk_jpeg_prefix.h
        DESTINATION
          ${FLTK_INCLUDEDIR}/FL/images
)
