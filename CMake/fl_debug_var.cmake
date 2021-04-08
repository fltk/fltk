#
# Macro used by the CMake build system for the Fast Light Tool Kit (FLTK).
# Written by Michael Surette
#
# Copyright 1998-2020 by Bill Spitzak and others.
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
# fl_debug_var - a macro to output debugging info
#######################################################################
#
# This macro displays the name and value of a CMake variable.
# The variable name is expanded with spaces to be (at least)
# <min_len> (currently 30) characters wide for better readability.
# VARNAME must be a string literal, e.g. WIN32 or "WIN32".
#
# Syntax:
#   fl_debug_var(VARNAME)
#
# Example:
#   fl_debug_var(WIN32)
#   fl_debug_var("UNIX")
#
#######################################################################

macro (fl_debug_var name)
  set (min_len 30)
  set (var "${name}")
  string(LENGTH "${var}" len)
  while (len LESS min_len)
    # add one space until min_len is reached
    # ** string(APPEND var " ") # requires CMake 3.4.x (otherwise loop...)
    set (var "${var} ")
    string(LENGTH "${var}" len)
  endwhile (len LESS min_len)
  message (STATUS "${var} = '${${name}}'")
endmacro (fl_debug_var)
