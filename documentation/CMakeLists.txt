#
# CMakeLists.txt to build docs for the FLTK project using CMake (www.cmake.org)
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

set(DOCS)
set(GENERATE_DOCS FALSE)
set(YEAR "")
set(CURRENT_DATE "")

if(FLTK_BUILD_HTML_DOCS OR FLTK_BUILD_PDF_DOCS)
  set(GENERATE_DOCS TRUE)
endif()

if(FLTK_INCLUDE_DRIVER_DOCS)
  set(DRIVER_DOCS "DriverDev")
else()
  set(DRIVER_DOCS "")
endif()

#------------------------------------------------
# generate files used for both HTML and PDF docs
#------------------------------------------------

if(GENERATE_DOCS)

  # create required variables

  execute_process(COMMAND date "+%Y"
    OUTPUT_VARIABLE YEAR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # note: current locale is used for abbreviated month
  execute_process(COMMAND date "+%b %d, %Y"
    OUTPUT_VARIABLE CURRENT_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # Find "short" doxygen version if it was built from Git
  # Note: this is still needed in CMake 3.15 but later CMake versions
  #       (notably 3.25) remove the Git revision in 'DOXYGEN_VERSION'.
  # Todo: Find the "first good" CMake version and remove this redundant
  #       code once we require this as our minimal version and replace the
  #       variable DOXYGEN_VERSION_SHORT with DOXYGEN_VERSION below.

  if(DOXYGEN_FOUND)
    # strip trailing git revision if doxygen was built from source
    string(REGEX REPLACE " .*$" "" DOXYGEN_VERSION_SHORT ${DOXYGEN_VERSION})
  endif(DOXYGEN_FOUND)

  # configure copyright.dox (includes current year)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/copyright.dox.in
    ${CMAKE_CURRENT_BINARY_DIR}/copyright.dox
    @ONLY
  )

  # configure generated.dox (includes date and versions)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/generated.dox.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated.dox
    @ONLY
  )

  if(0) # debug
    fl_debug_var(YEAR)
    fl_debug_var(CURRENT_DATE)
    fl_debug_var(FLTK_GIT_REVISION)
    fl_debug_var(DOXYGEN_FOUND)
    fl_debug_var(DOXYGEN_EXECUTABLE)
    fl_debug_var(DOXYGEN_VERSION)
    fl_debug_var(DOXYGEN_VERSION_SHORT)
  endif()

endif(GENERATE_DOCS)

#--------------------------
# build html documentation
#--------------------------

if(FLTK_BUILD_HTML_DOCS)

  list(APPEND DOCS html)

  # generate Doxygen file "Doxyfile"

  set(GENERATE_HTML  YES)
  set(GENERATE_LATEX NO)
  set(LATEX_HEADER   "")
  set(DOXYFILE       "Doxyfile")
  set(LOGFILE        "${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}_error.log")

  # configure Doxygen input file for HTML docs (Doxyfile.in)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}.in
    @ONLY
  )

  # convert Doxyfile to used doxygen version

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/convert_doxyfile
            ${DOXYGEN_EXECUTABLE}
            ${DOXYFILE}.in
            ${DOXYFILE}
            ${LOGFILE}
    BYPRODUCTS ${LOGFILE}
    COMMENT "Converting ${DOXYFILE} to doxygen version ${DOXYGEN_VERSION_SHORT}" VERBATIM
  )

  # generate HTML documentation

  add_custom_target(html
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating HTML documentation" VERBATIM
  )

endif(FLTK_BUILD_HTML_DOCS)

#--------------------------
# build pdf documentation
#--------------------------

if(FLTK_BUILD_PDF_DOCS)

  list(APPEND DOCS pdf)

  # generate Doxygen input file "Doxybook"

  set(GENERATE_HTML  NO)
  set(GENERATE_LATEX YES)
  set(LATEX_HEADER   "${CMAKE_CURRENT_BINARY_DIR}/fltk-book.tex")
  set(DOXYFILE       "Doxybook")
  set(LOGFILE        "${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}_error.log")

  # configure Doxygen input file for PDF docs (Doxybook.in)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}.in
    @ONLY
  )

  # convert Doxybook to current doxygen version

  # set DOXY_VERSION for compatibility with configure/make,
  # to be replaced in fltk-title.tex.in
  # FIXME: this can be simplified when configure/make is no longer supported

  set(DOXY_VERSION "${DOXYGEN_VERSION_SHORT}") #

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/convert_doxyfile
            ${DOXYGEN_EXECUTABLE}
            ${DOXYFILE}.in
            ${DOXYFILE}
            ${LOGFILE}
    BYPRODUCTS ${LOGFILE}
    COMMENT "Converting ${DOXYFILE} to doxygen version ${DOXYGEN_VERSION_SHORT}" VERBATIM
  )

  # generate LaTeX title fltk-title.tex

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fltk-title.tex.in
    ${CMAKE_CURRENT_BINARY_DIR}/fltk-title.tex
    @ONLY
  )

  # generate fltk.pdf

  add_custom_command(
    OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/fltk.pdf
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/make_header
            ${DOXYGEN_EXECUTABLE}
            ${CMAKE_CURRENT_BINARY_DIR}/fltk-title.tex
            ${CMAKE_CURRENT_BINARY_DIR}/fltk-book.tex
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/make_pdf
    COMMAND cp -f latex/refman.pdf fltk.pdf
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}
            ${CMAKE_CURRENT_BINARY_DIR}/fltk-title.tex
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating PDF documentation" VERBATIM
  )

  # add target 'pdf'

  add_custom_target(pdf
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/fltk.pdf
  )

endif(FLTK_BUILD_PDF_DOCS)

#----------------------------------
# add target 'docs' for all docs
#----------------------------------

if(DOCS)

  add_custom_target(docs
    DEPENDS ${DOCS}
  )

endif(DOCS)

#----------------------------------
# install html + pdf documentation
#----------------------------------

if(FLTK_INSTALL_HTML_DOCS AND FLTK_BUILD_HTML_DOCS)

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
    DESTINATION ${FLTK_DATADIR}/doc/fltk
  )

endif(FLTK_INSTALL_HTML_DOCS AND FLTK_BUILD_HTML_DOCS)

if(FLTK_INSTALL_PDF_DOCS AND FLTK_BUILD_PDF_DOCS)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fltk.pdf
    DESTINATION ${FLTK_DATADIR}/doc/fltk/
  )

endif(FLTK_INSTALL_PDF_DOCS AND FLTK_BUILD_PDF_DOCS)
