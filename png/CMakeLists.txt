#
# PNG library CMake configuration for the Fast Light Toolkit (FLTK).
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

# source files for png
set(PNG_SRCS
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
#  hence we also link against the bundled ZLIB. Therefore there's
#  no need to check which ZLIB version to use in this context.
#######################################################################

#######################################################################
# Build some files on ARM (e.g. Apple M1 systems)
#######################################################################

# We can only determine the target architecture if it is set
# in CMAKE_OSX_ARCHITECTURES, otherwise we *assume* it is true and
# compile these files even if this results in some warnings.
# This includes all non-macOS platforms.

if(CMAKE_OSX_ARCHITECTURES)
  string(REGEX MATCH "arm64" is_arm "${CMAKE_OSX_ARCHITECTURES}")
else()
  set(is_arm TRUE)
endif()

if(is_arm)
  list(APPEND PNG_SRCS
       arm/arm_init.c
       arm/filter_neon_intrinsics.c
       arm/palette_neon_intrinsics.c
  )
endif()

unset(is_arm)

#######################################################################
# Build some files on ppc64.
# We compile these files whatever the architecture resulting in
# void code on non-ppc64 architectures.
#######################################################################

list(APPEND PNG_SRCS
     powerpc/powerpc_init.c
     powerpc/filter_vsx_intrinsics.c
)

#######################################################################
# Set common variables for static and shared builds
#######################################################################

set(compile_defs
      HAVE_PNG_H=1
      HAVE_PNG_GET_VALID=1
      HAVE_PNG_SET_TRNS_TO_ALPHA=1
)

set(include_dirs
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      $<INSTALL_INTERFACE:include>
)

#######################################################################
# Build the static library
#######################################################################

fl_add_library(fltk_png STATIC "${PNG_SRCS}")
set(target     fltk_png)

target_link_libraries     (${target} PUBLIC fltk::z)
target_compile_definitions(${target} PUBLIC ${compile_defs})
target_include_directories(${target} PUBLIC ${include_dirs})

list(APPEND FLTK_IMAGE_LIBRARIES fltk::png)

# Propagate to parent scope (modified by fl_add_library and here)
set(FLTK_LIBRARIES       ${FLTK_LIBRARIES}       PARENT_SCOPE)
set(FLTK_IMAGE_LIBRARIES ${FLTK_IMAGE_LIBRARIES} PARENT_SCOPE)

#######################################################################
# Build the shared library (optional)
#######################################################################

if(FLTK_BUILD_SHARED_LIBS)

  # ensure to export all symbols for Windows DLL's
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

  fl_add_library(fltk_png SHARED "${PNG_SRCS}")
  set(target     fltk_png-shared)

  target_link_libraries     (${target} PUBLIC fltk::z-shared)
  target_compile_definitions(${target} PUBLIC ${compile_defs})
  target_include_directories(${target} PUBLIC ${include_dirs})

  list(APPEND FLTK_IMAGE_LIBRARIES_SHARED fltk::png-shared)

  # Propagate to parent scope (modified by fl_add_library and here)
  set(FLTK_LIBRARIES_SHARED       ${FLTK_LIBRARIES_SHARED}       PARENT_SCOPE)
  set(FLTK_IMAGE_LIBRARIES_SHARED ${FLTK_IMAGE_LIBRARIES_SHARED} PARENT_SCOPE)

endif()

#######################################################################
# Install the library headers
#######################################################################

install(FILES png.h pngconf.h pnglibconf.h pngprefix.h
        DESTINATION ${FLTK_INCLUDEDIR}/FL/images
)
