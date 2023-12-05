#
# PNG library CMake configuration for the Fast Light Toolkit (FLTK).
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

# source files for png
set (PNG_SRCS
  png.c
  pngerror.c
  pngget.c
  pngmem.c
  pngpread.c
  pngread.c
  pngrio.c
  pngrtran.c
  pngrutil.c
  pngset.c
  pngtrans.c
  pngwio.c
  pngwrite.c
  pngwtran.c
  pngwutil.c
)

#######################################################################
# Note: This file is used only if we build the bundled PNG library,
#  and if we do this we MUST also build and use the *bundled* ZLIB,
#  hence we MUST also link against the bundled ZLIB (see below).
#  There's no need to check which ZLIB version to use in this context.
#######################################################################

#######################################################################
# Build some files on ARM (e.g. Apple M1 systems)
#######################################################################

# We can only determine the target architecture if it is set
# in CMAKE_OSX_ARCHITECTURES, otherwise we *assume* it is true and
# compile these files even if this results in some warnings.
# This includes all non-macOS platforms.

if (CMAKE_OSX_ARCHITECTURES)
  string (REGEX MATCH "arm64" is_arm "${CMAKE_OSX_ARCHITECTURES}")
else ()
  set (is_arm TRUE)
endif ()

if (is_arm)
  LIST (APPEND PNG_SRCS
        arm/arm_init.c
        arm/filter_neon_intrinsics.c
        arm/palette_neon_intrinsics.c
  )
endif ()

unset (is_arm)

#######################################################################
# Build some files on ppc64
# We compile these files whatever the architecture resulting in void code
# on non-ppc64 architectures.
#######################################################################

LIST (APPEND PNG_SRCS
      powerpc/powerpc_init.c
      powerpc/filter_vsx_intrinsics.c
)

#######################################################################
# Build the static library
#######################################################################

FL_ADD_LIBRARY (fltk_png STATIC "${PNG_SRCS}")
target_link_libraries (fltk_png PUBLIC fltk_z)

#######################################################################
# Build the shared library (optional)
#######################################################################

if (OPTION_BUILD_SHARED_LIBS)

  FL_ADD_LIBRARY (fltk_png SHARED "${PNG_SRCS}")
  target_link_libraries (fltk_png_SHARED PUBLIC fltk_z_SHARED)

endif ()

#######################################################################
# Install the library headers
#######################################################################

install (FILES png.h pngconf.h pnglibconf.h pngprefix.h
         DESTINATION ${FLTK_INCLUDEDIR}/FL/images
)
