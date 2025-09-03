#
# Macros used by the CMake build system for the Fast Light Tool Kit (FLTK).
#
# Copyright 2024 by Bill Spitzak and others.
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
# The macros in this file are used to generate the CMake build summary.
# Fixed widths of title fields are intentionally hard coded in two of
# these macros so we can easily change the alignment.
########################################################################

include(${CMAKE_CURRENT_LIST_DIR}/fl_debug_var.cmake)

########################################################################
# Output a summary line like "<title> <value>"
########################################################################
# <title>  will be expanded to a fixed width (can be empty)
# <value>  text to be displayed
########################################################################

macro(fl_summary title value)
  fl_expand_name(label "${title}" 24)
  message(STATUS "${label} ${value}")
endmacro(fl_summary title value)

########################################################################
# Output a summary line like "<title>   will be built ..."
#                         or "<title>   will not be built ..."
########################################################################
# title   will be expanded to a fixed width (must not be empty)
# subdir  = relative build directory (e.g. lib or bin/test)
# build   = CMake variable name (bool): whether <title> is built
# option  = option name the user can set to build <title>
########################################################################

macro(fl_summary_build title subdir build option)
  if(${build})
    set(value "will be built in: ${CMAKE_CURRENT_BINARY_DIR}/${subdir}")
  else()
    set(value "will not be built (set ${option}=ON to build)")
  endif()
  fl_expand_name(label "${title}" 19)
  message(STATUS "${label} ${value}")
endmacro(fl_summary_build title var subdir)

########################################################################
# Output a simple summary line like "<title>   {Yes|No}"
########################################################################
# title   will be expanded to a fixed width (must not be empty)
# var     = CMake variable name, must evaluate to true or false
########################################################################

macro(fl_summary_yn title var)
  if(${var})
    set(value "Yes")
  else()
    set(value "No")
  endif()
  fl_summary("${title}" ${value})
endmacro(fl_summary_yn title var)

########################################################################
# Output summary line for image libs (bundled or system libs)
########################################################################
# title = "Image Libraries" or empty
# name  = displayed name = { JPEG | PNG | ZLIB }
# lib   = CMake library name (system library, if it was found)
########################################################################

macro(fl_summary_image title name lib)
  fl_expand_name(name4 "${name}" 8)
  if(FLTK_USE_BUNDLED_${name})
    set(value "${name4} = Bundled")
  else()
    set(value "${name4} = System: ${${lib}}")
  endif()
  fl_summary("${title}" "${value}")
endmacro(fl_summary_image title name lib)
