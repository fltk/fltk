#
# "$Id: CMakeLists.txt 10092 2014-02-02 00:49:50Z AlbrechtS $"
#
# macros.cmake defines macros used by the build system
# Written by Michael Surette
#
# Copyright 1998-2014 by Bill Spitzak and others.
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

#######################################################################
# macros used by the build system
#######################################################################
macro(FL_ADD_LIBRARY LIBNAME LIBTYPE LIBFILES)

    if(${LIBTYPE} STREQUAL "SHARED")
        set(LIBRARY_NAME ${LIBNAME}_SHARED)
    else()
        set(LIBRARY_NAME ${LIBNAME})
    endif(${LIBTYPE} STREQUAL "SHARED")

    add_library(${LIBRARY_NAME} ${LIBTYPE} ${LIBFILES})

    set_target_properties(${LIBRARY_NAME}
        PROPERTIES
        OUTPUT_NAME ${LIBNAME}
        DEBUG_OUTPUT_NAME "${LIBNAME}d"
        CLEAN_DIRECT_OUTPUT TRUE
        COMPILE_DEFINITIONS "FL_LIBRARY"
        )

    if(${LIBTYPE} STREQUAL "SHARED")
    set_target_properties(${LIBRARY_NAME}
        PROPERTIES
        VERSION ${FLTK_VERSION_FULL}
        SOVERSION ${FLTK_VERSION_MAJOR}.${FLTK_VERSION_MINOR}
        PREFIX "lib"    # for MSVC static/shared coexistence
        )
    endif(${LIBTYPE} STREQUAL "SHARED")

    if(MSVC)
        if(OPTION_LARGE_FILE)
            set_target_properties(${LIBNAME}
                PROPERTIES
                LINK_FLAGS /LARGEADDRESSAWARE
                )
        endif(OPTION_LARGE_FILE)

        if(${LIBTYPE} STREQUAL "SHARED")
            set_target_properties(${LIBRARY_NAME}
                PROPERTIES
                COMPILE_DEFINITIONS "FL_DLL"
                )
            endif(${LIBTYPE} STREQUAL "SHARED")
    endif(MSVC)

    install(TARGETS ${LIBRARY_NAME}
        EXPORT FLTK-Targets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

    list(APPEND FLTK_LIBRARIES "${LIBRARY_NAME}")
    set(FLTK_LIBRARIES ${FLTK_LIBRARIES} PARENT_SCOPE)

endmacro(FL_ADD_LIBRARY LIBNAME LIBTYPE LIBFILES)

#######################################################################
macro(CREATE_EXAMPLE NAME SOURCES LIBRARIES)

    set(srcs)			# source files
    set(flsrcs)			# fluid source files

    set(tname ${NAME})		# target name
    set(oname ${NAME})		# output (executable) name

    # rename reserved target name "help" (CMake 2.8.12 and later)
    if(${tname} MATCHES "^help$")
        set(tname "test_help")
    endif(${tname} MATCHES "^help$")

    foreach(src ${SOURCES})
        if("${src}" MATCHES "\\.fl$")
            list(APPEND flsrcs ${src})
        else()
            list(APPEND srcs ${src})
        endif("${src}" MATCHES "\\.fl$")
    endforeach(src)

    if(flsrcs)
        set(FLTK_WRAP_UI TRUE)
        fltk_wrap_ui(${tname} ${flsrcs})
    endif(flsrcs)

    add_executable(${tname} WIN32 ${srcs} ${${tname}_FLTK_UI_SRCS})
    set_target_properties(${tname}
	PROPERTIES OUTPUT_NAME ${oname}
	)

    target_link_libraries(${tname} ${LIBRARIES})

    # link in optional libraries
    if(USE_XFT)
        target_link_libraries(${tname} ${X11_Xft_LIB})
    endif(USE_XFT)

    if(HAVE_XINERAMA)
        target_link_libraries(${tname} ${X11_Xinerama_LIB})
    endif(HAVE_XINERAMA)

    # install the example
    install(TARGETS ${tname}
        DESTINATION ${FLTK_EXAMPLES_PATH}
        )

endmacro(CREATE_EXAMPLE NAME SOURCES LIBRARIES)

#######################################################################
