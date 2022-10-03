#
# CMakeLists.txt to build docs for the FLTK project using CMake (www.cmake.org)
#
# Copyright 1998-2022 by Bill Spitzak and others.
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

set (DOCS)
set (GENERATE_DOCS FALSE)
set (GIT_REVISION "")
set (YEAR "")
set (CURRENT_DATE "")

if (OPTION_BUILD_HTML_DOCUMENTATION OR OPTION_BUILD_PDF_DOCUMENTATION)
  set (GENERATE_DOCS TRUE)
endif ()

if (OPTION_INCLUDE_DRIVER_DOCUMENTATION)
  set (DRIVER_DOCS "DriverDev")
else ()
  set (DRIVER_DOCS "")
endif ()

#------------------------------------------------
# generate files used for both HTML and PDF docs
#------------------------------------------------

if (GENERATE_DOCS)

  # create required variables

  execute_process (COMMAND date "+%Y"
    OUTPUT_VARIABLE YEAR
  )

  # note: current locale is used for abbreviated month
  execute_process (COMMAND date "+%b %d, %Y"
    OUTPUT_VARIABLE CURRENT_DATE
  )

  # find git revision

  # FIXME: This must also work with tarballs where git is not available.
  #        For now we just ignore errors and set GIT_REVISION = "unkown".
  #        In the future tarball/zip generation should create a file
  #        that contains the git revision.

  execute_process (COMMAND
    git --git-dir=${CMAKE_SOURCE_DIR}/.git rev-parse --short=10 HEAD
    OUTPUT_VARIABLE GIT_REVISION
    ERROR_QUIET
  )
  # strip trailing newline from git revision
  string (REPLACE "\n" "" GIT_REVISION "${GIT_REVISION}")
  # set to "'unkown'" if git is not available
  if (GIT_REVISION STREQUAL "")
    set (GIT_REVISION "'unkown'")
  endif()

  # find doxygen version

  if (DOXYGEN_FOUND)
    # strip trailing git revision if doxygen was built from source
    string (REGEX REPLACE " .*$" "" DOXYGEN_VERSION_SHORT ${DOXYGEN_VERSION})
    # strip trailing newline
    string (REPLACE "\n" "" DOXYGEN_VERSION_SHORT "${DOXYGEN_VERSION_SHORT}")
  endif (DOXYGEN_FOUND)

  # configure copyright.dox (includes current year)
  configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/copyright.dox.in
    ${CMAKE_CURRENT_BINARY_DIR}/copyright.dox
    @ONLY
  )

  # configure generated.dox (includes date and versions)
  configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/generated.dox.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated.dox
    @ONLY
  )

  if (0) # debug
    fl_debug_var (GIT_REVISION)
    fl_debug_var (DOXYGEN_FOUND)
    fl_debug_var (DOXYGEN_EXECUTABLE)
    fl_debug_var (DOXYGEN_VERSION)
    fl_debug_var (DOXYGEN_VERSION_SHORT)
  endif ()

endif (GENERATE_DOCS)

#--------------------------
# build html documentation
#--------------------------

if (OPTION_BUILD_HTML_DOCUMENTATION)

  list (APPEND DOCS html)

  # generate Doxygen file "Doxyfile"

  set (GENERATE_HTML  YES)
  set (GENERATE_LATEX NO)
  set (LATEX_HEADER   "")

  # configure Doxygen input file for HTML docs (Doxyfile.in)

  configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.in
    @ONLY
  )

  # convert Doxyfile to current doxygen version

  add_custom_command (
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    COMMAND ${DOXYGEN_EXECUTABLE} -u -s - < Doxyfile.in > Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Converting Doxyfile to current doxygen version" VERBATIM
  )

  # generate HTML documentation

  add_custom_target (html
    COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating HTML documentation" VERBATIM
  )

endif (OPTION_BUILD_HTML_DOCUMENTATION)

#--------------------------
# build pdf documentation
#--------------------------

if (OPTION_BUILD_PDF_DOCUMENTATION)

  list (APPEND DOCS pdf)

  # generate Doxygen input file "Doxybook"

  set (GENERATE_HTML  NO)
  set (GENERATE_LATEX YES)
  set (LATEX_HEADER   "${CMAKE_CURRENT_BINARY_DIR}/fltk-book.tex")

  # strip potential " (Git-hash)" from the original version
  string (REGEX REPLACE " .*$" "" DOXY_VERSION ${DOXYGEN_VERSION})

  # configure Doxygen input file for PDF docs (Doxybook.in)

  configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxybook.in
    @ONLY
  )

  # convert Doxybook to current doxygen version

  add_custom_command (
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Doxybook
    COMMAND ${DOXYGEN_EXECUTABLE} -u -s - < Doxybook.in > Doxybook
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Converting Doxybook to current doxygen version" VERBATIM
  )

  # generate LaTeX title fltk-title.tex

  configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fltk-title.tex.in
    ${CMAKE_CURRENT_BINARY_DIR}/fltk-title.tex
    @ONLY
  )

  # generate fltk.pdf

  add_custom_command (
    OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/fltk.pdf
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/make_header
            ${CMAKE_CURRENT_BINARY_DIR}/fltk-title.tex
            ${CMAKE_CURRENT_BINARY_DIR}/fltk-book.tex
    COMMAND ${DOXYGEN_EXECUTABLE} Doxybook
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/make_pdf
    COMMAND cp -f latex/refman.pdf fltk.pdf
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Doxybook
            ${CMAKE_CURRENT_BINARY_DIR}/fltk-title.tex
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating PDF documentation" VERBATIM
  )

  # add target 'pdf'

  add_custom_target (pdf
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/fltk.pdf
  )

endif (OPTION_BUILD_PDF_DOCUMENTATION)

#----------------------------------
# add target 'docs' for all docs
#----------------------------------

if (DOCS)

  add_custom_target (docs
    DEPENDS ${DOCS}
  )

endif (DOCS)

#----------------------------------
# install html + pdf documentation
#----------------------------------

if (OPTION_INSTALL_HTML_DOCUMENTATION AND OPTION_BUILD_HTML_DOCUMENTATION)

  install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
    DESTINATION ${FLTK_DATADIR}/doc/fltk
  )

endif (OPTION_INSTALL_HTML_DOCUMENTATION AND OPTION_BUILD_HTML_DOCUMENTATION)

if (OPTION_INSTALL_PDF_DOCUMENTATION AND OPTION_BUILD_PDF_DOCUMENTATION)

  install (FILES ${CMAKE_CURRENT_BINARY_DIR}/fltk.pdf
    DESTINATION ${FLTK_DATADIR}/doc/fltk/
  )

endif (OPTION_INSTALL_PDF_DOCUMENTATION AND OPTION_BUILD_PDF_DOCUMENTATION)
