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
set(YEAR "")
set(CURRENT_DATE "")

#------------------------------------------------
# generate files used for both HTML and PDF docs
#------------------------------------------------

if(FLTK_BUILD_FLUID_DOCS OR FLTK_BUILD_PDF_DOCS)

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
  # Note: this is still needed in CMake 3.12.0 but later CMake versions
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

endif(FLTK_BUILD_FLUID_DOCS OR FLTK_BUILD_PDF_DOCS)

#-------------------------------
# build FLUID html documentation
#-------------------------------

if(FLTK_BUILD_FLUID_DOCS)

  #list(APPEND DOCS html)

  # generate Doxygen file "Doxyfile"

  set(GENERATE_FLUID_HTML YES)
  set(GENERATE_LATEX NO)
  set(LATEX_HEADER   "")
  set(FL_HTML_INDEX  "FL_HTML_INDEX")
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

  # generate screen shot using FLUID --autodoc target_dir
  # generate HTML documentation

  add_custom_target(fluid_docs
    COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/src/
    COMMAND fltk::fluid -scheme gtk+ --autodoc ${CMAKE_CURRENT_BINARY_DIR}/src/
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating HTML documentation" VERBATIM
  )
  add_dependencies(fluid_docs fltk::fluid)

endif(FLTK_BUILD_FLUID_DOCS)

#--------------------------
# build pdf documentation
#--------------------------

if(FLTK_BUILD_PDF_DOCS AND FLTK_BUILD_FLUID_DOCS)

  # generate Doxygen input file "Doxybook"

  set(GENERATE_FLUID_HTML  NO)
  set(GENERATE_LATEX YES)
  set(LATEX_HEADER   "${CMAKE_CURRENT_BINARY_DIR}/fluid-book.tex")
  set(FL_HTML_INDEX  "FL_NO_HTML_INDEX")
  set(DOXYFILE       "Doxybook")
  set(LOGFILE        "${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}_error.log")

  # configure Doxygen input file for PDF docs (Doxybook.in)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}.in
    @ONLY
  )

  # convert Doxybook to current doxygen version

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

  # generate LaTeX title fluid-title.tex

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fluid-title.tex.in
    ${CMAKE_CURRENT_BINARY_DIR}/fluid-title.tex
    @ONLY
  )

  # generate fluid.pdf

  add_custom_command(
    OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/fluid.pdf
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/make_header
            ${DOXYGEN_EXECUTABLE}
            ${CMAKE_CURRENT_BINARY_DIR}/fluid-title.tex
            ${CMAKE_CURRENT_BINARY_DIR}/fluid-book.tex
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/make_pdf
    COMMAND cp -f latex/refman.pdf fluid.pdf
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}
            ${CMAKE_CURRENT_BINARY_DIR}/fluid-title.tex
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating FLUID PDF documentation" VERBATIM
  )

  # add target 'pdf'

  add_custom_target(fluid_pdf
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/fluid.pdf
  )
  add_dependencies(fluid_pdf fluid_docs)

endif(FLTK_BUILD_PDF_DOCS AND FLTK_BUILD_FLUID_DOCS)

#---------------------------------------
# install FLUID html + pdf documentation
#---------------------------------------

if(FLTK_INSTALL_FLUID_DOCS AND FLTK_BUILD_FLUID_DOCS)

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
    DESTINATION ${FLTK_DATADIR}/doc/fltk/fluid
  )

endif(FLTK_INSTALL_FLUID_DOCS AND FLTK_BUILD_FLUID_DOCS)

if(FLTK_INSTALL_PDF_DOCS AND FLTK_BUILD_PDF_DOCS AND FLTK_BUILD_FLUID_DOCS)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fluid.pdf
    DESTINATION ${FLTK_DATADIR}/doc/fltk/
  )

endif(FLTK_INSTALL_PDF_DOCS AND FLTK_BUILD_PDF_DOCS AND FLTK_BUILD_FLUID_DOCS)
