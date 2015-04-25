#
# "$Id$"
#
# CMakeLists.txt to build docs for the FLTK project using CMake (www.cmake.org)
#
# Copyright 1998-2015 by Bill Spitzak and others.
#
# This library is free software. Distribution and use rights are outlined in
# the file "COPYING" which should have been included with this file.  If this
# file is missing or damaged, see the license at:
#
#     http://www.fltk.org/COPYING.php
#
# Please report all bugs and problems on the following page:
#
#     http://www.fltk.org/str.php
#

set (DOCS)

#--------------------------
# build html documentation
#--------------------------

if(OPTION_BUILD_HTML_DOCUMENTATION)

    list(APPEND DOCS html)

    # generate Doxygen file "Doxyfile"

    set(GENERATE_HTML	YES)
    set(GENERATE_LATEX	NO)
    set(LATEX_HEADER	"")

    configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
	${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	@ONLY
    )

    # generate html docs

    add_custom_target(html
	# ALL
	${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating HTML documentation" VERBATIM
	DEPENDS fltk
	)

endif(OPTION_BUILD_HTML_DOCUMENTATION)

#--------------------------
# build pdf documentation
#--------------------------

if (OPTION_BUILD_PDF_DOCUMENTATION)

    list(APPEND DOCS pdf)

    # generate Doxygen file "Doxybook"

    set(GENERATE_HTML	NO)
    set(GENERATE_LATEX	YES)
    set(LATEX_HEADER	"${CMAKE_CURRENT_BINARY_DIR}/fltk-book.tex")

    configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
	${CMAKE_CURRENT_BINARY_DIR}/Doxybook
	@ONLY
    )

    # generate LaTeX header fltk-book.tex

    set(FL_VERSION ${FLTK_VERSION_FULL})
    set(DOXY_VERSION ${DOXYGEN_VERSION})
    execute_process(COMMAND date +%Y
	OUTPUT_VARIABLE YEAR
	)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/fltk-book.tex.in
	${CMAKE_CURRENT_BINARY_DIR}/fltk-book.tex
	@ONLY
	)

    # generate fltk.pdf

    add_custom_command(
	OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/fltk.pdf
	COMMAND ${DOXYGEN_EXECUTABLE} Doxybook
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/make_pdf
	COMMAND cp -f latex/refman.pdf fltk.pdf
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating PDF documentation" VERBATIM
	DEPENDS fltk
	)

    # add target 'pdf'

    add_custom_target(pdf
	# ALL
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/fltk.pdf
	)

endif(OPTION_BUILD_PDF_DOCUMENTATION)

#----------------------------------
# add target 'docs' for all docs
#----------------------------------

if (DOCS)

    add_custom_target(docs
	# ALL
	DEPENDS ${DOCS}
	)

endif (DOCS)

#----------------------------------
# install html + pdf documentation
#----------------------------------

if(OPTION_INSTALL_HTML_DOCUMENTATION AND OPTION_BUILD_HTML_DOCUMENTATION)

    install(DIRECTORY ${CMAKE_BINARY_DIR}/documentation/html
	DESTINATION ${FLTK_DATADIR}/doc/fltk
	)

endif(OPTION_INSTALL_HTML_DOCUMENTATION AND OPTION_BUILD_HTML_DOCUMENTATION)

if(OPTION_INSTALL_PDF_DOCUMENTATION AND OPTION_BUILD_PDF_DOCUMENTATION)

    install(FILES ${CMAKE_BINARY_DIR}/documentation/fltk.pdf
	DESTINATION ${FLTK_DATADIR}/doc/fltk/
	)

endif(OPTION_INSTALL_PDF_DOCUMENTATION AND OPTION_BUILD_PDF_DOCUMENTATION)
