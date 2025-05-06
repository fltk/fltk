#
# Generate version numbers and configure header files
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

#######################################################################
# Calculate limits and check FL_ABI_VERSION syntax
#######################################################################

# Initialize FL_ABI_VERSION
set(FL_ABI_VERSION "${FLTK_ABI_VERSION}")

# These are the limits (min/max) FL_ABI_VERSION is allowed to have
math(EXPR abi_version_min "${FLTK_VERSION_MAJOR} * 10000 + ${FLTK_VERSION_MINOR} * 100")
math(EXPR abi_version_max "${abi_version_min} + ${FLTK_VERSION_PATCH} + 1")

if(FL_ABI_VERSION STREQUAL "")

  # no version set, silently use default
  set(FL_ABI_VERSION "${abi_version_min}")

else()

  # check syntax of reuested ABI version (five digits)

  string(REGEX MATCH "[1-9][0-9][0-9][0-9][0-9]" reg_match "${FL_ABI_VERSION}")
  if(NOT reg_match STREQUAL "${FL_ABI_VERSION}")
    message(STATUS "FLTK_ABI_VERSION \"${FLTK_ABI_VERSION}\" is invalid. Using default = ${abi_version_min}")
    set(FL_ABI_VERSION "${abi_version_min}")
  endif()

  # check minor version (first three numbers must match)

  string(SUBSTRING "${abi_version_min}" 0 3 abi_version_minor)
  string(SUBSTRING "${FL_ABI_VERSION}"  0 3 abi_version_temp)

  if(NOT abi_version_temp STREQUAL ${abi_version_minor})
    set(FL_ABI_VERSION "${abi_version_min}")
    message(STATUS "FLTK_ABI_VERSION \"${FLTK_ABI_VERSION}\" doesn't match minor version. Using default = ${abi_version_min}")
    set(FL_ABI_VERSION "${abi_version_min}")
  endif()

endif()

if(FL_ABI_VERSION STRLESS ${abi_version_min})
  # should never happen
  set(FL_ABI_VERSION "${abi_version_min}")
elseif(FL_ABI_VERSION STRGREATER ${abi_version_max})
  # accept w/o warning
  set(FL_ABI_VERSION "${abi_version_max}")
endif()

# reset all temporary variables

unset(abi_version_min)
unset(abi_version_max)
unset(abi_version_minor)
unset(abi_version_temp)
unset(reg_match)

#######################################################################
# configure the header file "FL/fl_config.h" in the build tree
#######################################################################

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/fl_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/FL/fl_config.h
  @ONLY
)

#######################################################################
# generate the header file "config.h" in the build tree
#######################################################################

# prepare some variables for config.h

if(IS_ABSOLUTE "${FLTK_DATADIR}")
  set(PREFIX_DATA "${FLTK_DATADIR}/fltk")
else(IS_ABSOLUTE "${FLTK_DATADIR}")
  set(PREFIX_DATA "${CMAKE_INSTALL_PREFIX}/${FLTK_DATADIR}/fltk")
endif(IS_ABSOLUTE "${FLTK_DATADIR}")

if(IS_ABSOLUTE "${FLTK_DOCDIR}")
  set(PREFIX_DOC "${FLTK_DOCDIR}/fltk")
else(IS_ABSOLUTE "${FLTK_DOCDIR}")
  set(PREFIX_DOC "${CMAKE_INSTALL_PREFIX}/${FLTK_DOCDIR}/fltk")
endif(IS_ABSOLUTE "${FLTK_DOCDIR}")

set(CONFIG_H_IN config.h.in)
set(CONFIG_H config.h)

# generate the header file

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/${CONFIG_H_IN}"
  "${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_H}"
  @ONLY
)
