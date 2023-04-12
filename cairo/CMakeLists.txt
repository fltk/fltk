#
# CMakeLists.txt to build a dummy Cairo library for the FLTK project using CMake
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

########################################################################
# Note: since FLTK 1.4.0 Fl_Cairo_Window support [1] is included in
# libfltk and libfltk_cairo is no longer necessary. This directory is
# used to build an "empty" dummy library for backwards compatibility,
# just in case users expect it to exist.
# ----------------------------------------------------------------------
# The entire 'cairo' folder will be removed in a later FLTK release.
########################################################################

# Build dummy fltk_cairo library

set (cairo_SRCS cairo_dummy.c)

FL_ADD_LIBRARY (fltk_cairo STATIC "${cairo_SRCS}")

# Build shared dummy library (optional)

if (OPTION_BUILD_SHARED_LIBS)

  FL_ADD_LIBRARY (fltk_cairo SHARED "${cairo_SRCS}")
  target_link_libraries (fltk_cairo_SHARED fltk_SHARED)

endif (OPTION_BUILD_SHARED_LIBS)
