#
# "$Id$"
#
# macros.cmake
# Written by Michael Surette
#
# Copyright 1998-2016 by Bill Spitzak and others.
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

    if (${LIBTYPE} STREQUAL "SHARED")
        set (LIBRARY_NAME ${LIBNAME}_SHARED)
    else ()
        set (LIBRARY_NAME ${LIBNAME})
    endif (${LIBTYPE} STREQUAL "SHARED")

    if (MSVC)
	set (LIBRARY_NAME_DEBUG "${LIBRARY_NAME}d")
    else ()
	set (LIBRARY_NAME_DEBUG "${LIBRARY_NAME}")
    endif (MSVC)

    add_library(${LIBRARY_NAME} ${LIBTYPE} ${LIBFILES})

    set_target_properties(${LIBRARY_NAME}
        PROPERTIES
        OUTPUT_NAME ${LIBRARY_NAME}
        DEBUG_OUTPUT_NAME ${LIBRARY_NAME_DEBUG}
        CLEAN_DIRECT_OUTPUT TRUE
        COMPILE_DEFINITIONS "FL_LIBRARY"
	)

    if (${LIBTYPE} STREQUAL "SHARED")
	set_target_properties(${LIBRARY_NAME}
	    PROPERTIES
	    VERSION ${FLTK_VERSION_FULL}
	    SOVERSION ${FLTK_VERSION_MAJOR}.${FLTK_VERSION_MINOR}
	    PREFIX "lib"    # for MSVC static/shared coexistence
	    )
    endif (${LIBTYPE} STREQUAL "SHARED")

    if (MSVC)
	if (OPTION_LARGE_FILE)
	    set_target_properties(${LIBRARYNAME}
		PROPERTIES
		LINK_FLAGS /LARGEADDRESSAWARE
		)
	endif (OPTION_LARGE_FILE)

	if (${LIBTYPE} STREQUAL "SHARED")
	    set_target_properties(${LIBRARY_NAME}
		PROPERTIES
		COMPILE_DEFINITIONS "FL_DLL"
		)
	endif (${LIBTYPE} STREQUAL "SHARED")
    endif (MSVC)

    install(TARGETS ${LIBRARY_NAME}
        EXPORT FLTK-Targets
        RUNTIME DESTINATION ${FLTK_BINDIR}
        LIBRARY DESTINATION ${FLTK_LIBDIR}
        ARCHIVE DESTINATION ${FLTK_LIBDIR}
	)

    list(APPEND FLTK_LIBRARIES "${LIBRARY_NAME}")
    set (FLTK_LIBRARIES ${FLTK_LIBRARIES} PARENT_SCOPE)

endmacro(FL_ADD_LIBRARY LIBNAME LIBTYPE LIBFILES)

#######################################################################
function(CREATE_EXAMPLE NAME SOURCES LIBRARIES)

    set (srcs)			# source files
    set (flsrcs)		# fluid source files
    set (icns)			# mac icons

    set (tname ${NAME})		# target name

    # rename reserved target name "help" (CMake 2.8.12 and later)
    if (${tname} MATCHES "^help$")
        set (tname "test_help")
    endif (${tname} MATCHES "^help$")

    foreach(src ${SOURCES})
        if ("${src}" MATCHES "\\.fl$")
            list(APPEND flsrcs ${src})
        elseif ("${src}" MATCHES "\\.icns$")
            set(icns "${src}")
        else ()
            list(APPEND srcs ${src})
        endif ("${src}" MATCHES "\\.fl$")
    endforeach(src)

    set (FLUID_SOURCES)
    if (flsrcs)
        FLTK_RUN_FLUID(FLUID_SOURCES "${flsrcs}")
    endif (flsrcs)

    if (APPLE AND NOT OPTION_APPLE_X11)
        add_executable(${tname} MACOSX_BUNDLE ${srcs} ${FLUID_SOURCES} ${icns})
        if (icns)
            FLTK_SET_BUNDLE_ICON(${tname} ${icns})
        endif (icns)
    else ()
        add_executable(${tname} WIN32 ${srcs} ${FLUID_SOURCES})
    endif (APPLE AND NOT OPTION_APPLE_X11)

    set_target_properties(${tname}
        PROPERTIES OUTPUT_NAME ${NAME}
    )

    target_link_libraries(${tname} ${LIBRARIES})

endfunction(CREATE_EXAMPLE NAME SOURCES LIBRARIES)

#######################################################################

#
# End of "$Id$".
#
