#
# Function used by the CMake build system for the Fast Light Tool Kit (FLTK).
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
# fl_expand_name - a function to expand a variable name with spaces
#######################################################################
#
# This function returns a string comprising of the given name and
# enough spaces to have at least the given minimal length (min_len).
# Currently min_len must not be greater than 50.
#
# If the string is already at least min_len it is not changed,
# otherwise the string is returned to the given variable (out)
# in the parent scope.
#
# Syntax:
#   fl_expand_name (out in min_len)
#
# Example:
#   fl_expand_name (var WIN32 30)
#   fl_expand_name (var UNIX  40)
#
#######################################################################

function(fl_expand_name out in min_len)
  string(LENGTH "${in}" len)
  if(len LESS min_len)
    set(spaces "                         ")
    set(temp "${in}")
    set(temp "${in}${spaces}${spaces}")
    string(SUBSTRING "${temp}" 0 ${min_len} temp)
    set(${out} "${temp}" PARENT_SCOPE)
  else()
    set(${out} "${in}" PARENT_SCOPE)
  endif()
endfunction(fl_expand_name)

#######################################################################
# fl_debug_var - a function to output debugging info
#######################################################################
#
# This function displays the name and value of a CMake variable.
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

function(fl_debug_var name)
  set(var "${name}")
  fl_expand_name(var "${name}" 40)
  message(STATUS "${var} = '${${name}}'")
endfunction(fl_debug_var)


#######################################################################
# fl_debug_target - a function to output info about a target
#######################################################################
#
# This function displays properties of a CMake target.
#
# Currently there's a fixed number of properties.
#
# Syntax:
#   fl_debug_target(target)
#
# Example:
#   fl_debug_target(fltk)
#   fl_debug_target(fluid)
#   fl_debug_target(fltk_image)
#   fl_debug_target(fltk::forms)
#
#######################################################################

function(fl_debug_target name)
  message(STATUS "+++ fl_debug_target(${name})")
  set(var "${name}")
  fl_expand_name(var "${name}" 40)

  if(NOT TARGET ${name})
    message(STATUS "${var} = <not a target>")
    message(STATUS "")
    return()
  endif()

  get_target_property(_type ${name} TYPE)
  # message(STATUS "${var} = target, type = ${_type}")

  # these properties are always supported:
  set(_props NAME TYPE ALIASED_TARGET)

  # these properties can't be read from executable target types
  ### if(NOT _type STREQUAL "EXECUTABLE")
  ###   list(APPEND _props
  ###       LOCATION
  ###       IMPORTED_LOCATION
  ###       INTERFACE_LOCATION)
  ### endif()

  list(APPEND _props
      INCLUDE_DIRECTORIES
      LINK_DIRECTORIES
      LINK_LIBRARIES
      COMPILE_DEFINITIONS
      INTERFACE_COMPILE_DEFINITIONS
      INTERFACE_INCLUDE_DIRECTORIES
      INTERFACE_LINK_DIRECTORIES
      INTERFACE_LINK_LIBRARIES)

  foreach(prop ${_props})
    get_target_property(${prop} ${name} ${prop})
    if(NOT ${prop})
      set(${prop} "")
    endif()
    fl_debug_var(${prop})
  endforeach()
  message(STATUS "")

endfunction(fl_debug_target)
