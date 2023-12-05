#
# JPEG library CMake configuration for the Fast Light Toolkit (FLTK).
#
# Copyright 1998-2023 by Bill Spitzak and others.
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

set (systemdependent_SRCS jmemnobs.c)

# library object files common to compression and decompression

set (common_SRCS
     jaricom.c jcomapi.c jutils.c jerror.c jmemmgr.c
)

# compression library object files

set (compression_SRCS
     jcapimin.c jcapistd.c jcarith.c jctrans.c jcparam.c jdatadst.c jcinit.c
     jcmaster.c jcmarker.c jcmainct.c jcprepct.c jccoefct.c jccolor.c
     jcsample.c jchuff.c jcdctmgr.c jfdctfst.c jfdctflt.c
     jfdctint.c
)

# decompression library object files

set (decompression_SRCS
     jdapimin.c jdapistd.c jdarith.c jdtrans.c jdatasrc.c jdmaster.c
     jdinput.c jdmarker.c jdhuff.c jdmainct.c jdcoefct.c
     jdpostct.c jddctmgr.c jidctfst.c jidctflt.c jidctint.c
     jdsample.c jdcolor.c jquant1.c jquant2.c jdmerge.c
)

list (APPEND BUILD_SRCS "${systemdependent_SRCS};${common_SRCS}")
list (APPEND BUILD_SRCS "${compression_SRCS};${decompression_SRCS}")

#######################################################################

# Suppress some Visual Studio compiler warnings

set (msvc_warnings /wd4267)

#######################################################################
# Build the static library
#######################################################################

FL_ADD_LIBRARY (fltk_jpeg STATIC "${BUILD_SRCS}")

if (MSVC)
  target_compile_options (fltk_jpeg PRIVATE ${msvc_warnings})
endif (MSVC)

#######################################################################
# Build the shared library (optional)
#######################################################################

if (OPTION_BUILD_SHARED_LIBS)

  FL_ADD_LIBRARY (fltk_jpeg SHARED "${BUILD_SRCS}")

  if (MSVC)
    target_compile_options (fltk_jpeg_SHARED PRIVATE ${msvc_warnings})
  endif (MSVC)

endif (OPTION_BUILD_SHARED_LIBS)

#######################################################################
# Install the library headers
#######################################################################

install (FILES jconfig.h jerror.h jmorecfg.h jpeglib.h
               fltk_jpeg_prefix.h
         DESTINATION ${FLTK_INCLUDEDIR}/FL/images
)
