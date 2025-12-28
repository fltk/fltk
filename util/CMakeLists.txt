#
# CMakeLists.txt to build utilities for the FLTK project using CMake (www.cmake.org)
#
# Copyright 1998-2025 by Bill Spitzak and others.
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

# Build utilities for FLTK development

# Note: utilities are only built internally during the build process
# and not installed. They are tools used to generate source files or
# assist in building documentation.

#######################################################################
# cmap - colormap generation program
#######################################################################

# This program produces the contents of "fl_cmap.h"
# It's used during the build process to generate src/fl_cmap.h

add_executable(cmap cmap.cxx)

# cmap doesn't need to link against FLTK libraries - it's a standalone tool
target_link_libraries(cmap PRIVATE m)

# Set properties
set_target_properties(cmap PROPERTIES
  OUTPUT_NAME cmap
  EXCLUDE_FROM_ALL TRUE
)

#######################################################################
# PDF documentation tool to generate a png image from a
# Doxygen `@code` segment with international characters
#######################################################################

if(FLTK_BUILD_PDF_DOCS)

  # PDF documentation helper tool
  add_executable(code_snapshot
    code_snapshot.cxx
    ../fluid/widgets/Code_Viewer.cxx
    ../fluid/widgets/Code_Editor.cxx
    ../fluid/widgets/Style_Parser.cxx
  )

  target_link_libraries(code_snapshot PRIVATE fltk::images)

  set_target_properties(code_snapshot PROPERTIES
    OUTPUT_NAME code_snapshot
    EXCLUDE_FROM_ALL FALSE
  )

  target_include_directories(code_snapshot PRIVATE
    ../fluid
  )

  message(STATUS "PDF code snapshot tools will be built (FLTK_BUILD_PDF_DOCS=ON)")

endif(FLTK_BUILD_PDF_DOCS)

#######################################################################
# Install rules (if any tools need to be installed)
#######################################################################

# Currently no utilities are installed as they are build-time tools only
# If you need to install any utilities, add install() commands here
