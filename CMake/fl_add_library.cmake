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
# FL_ADD_LIBRARY - add a static or shared library to the build
#######################################################################

macro (FL_ADD_LIBRARY LIBNAME LIBTYPE LIBFILES)

  if (${LIBTYPE} STREQUAL "SHARED")
    set (TARGET_NAME ${LIBNAME}_SHARED)
  else ()
    set (TARGET_NAME ${LIBNAME})
  endif (${LIBTYPE} STREQUAL "SHARED")

  if (MSVC)
    set (OUTPUT_NAME_DEBUG   "${LIBNAME}d")
    set (OUTPUT_NAME_RELEASE "${LIBNAME}")
  else ()
    set (OUTPUT_NAME_DEBUG   "${LIBNAME}")
    set (OUTPUT_NAME_RELEASE "${LIBNAME}")
  endif (MSVC)

  add_library(${TARGET_NAME} ${LIBTYPE} ${LIBFILES})

  # target properties for all libraries

  set_target_properties(${TARGET_NAME}
    PROPERTIES
    CLEAN_DIRECT_OUTPUT TRUE
    COMPILE_DEFINITIONS "FL_LIBRARY"
  )

  # additional target properties for static libraries

  if (${LIBTYPE} STREQUAL "STATIC")
    set_target_properties(${TARGET_NAME}
      PROPERTIES
      OUTPUT_NAME ${LIBNAME}
      OUTPUT_NAME_DEBUG   ${OUTPUT_NAME_DEBUG}
      OUTPUT_NAME_RELEASE ${OUTPUT_NAME_RELEASE}
    )
  endif (${LIBTYPE} STREQUAL "STATIC")

  # additional target properties for shared (dynamic) libraries (DLL's)

  if (${LIBTYPE} STREQUAL "SHARED")
    set_target_properties(${TARGET_NAME}
      PROPERTIES
      VERSION ${FLTK_VERSION}
      SOVERSION ${FLTK_VERSION_MAJOR}.${FLTK_VERSION_MINOR}
      OUTPUT_NAME ${LIBNAME}
      OUTPUT_NAME_DEBUG   ${OUTPUT_NAME_DEBUG}
      OUTPUT_NAME_RELEASE ${OUTPUT_NAME_RELEASE}
    )
    # MSVC only:
    if (MSVC)
      set_target_properties(${TARGET_NAME}
        PROPERTIES
        OUTPUT_NAME         lib${LIBNAME}
        OUTPUT_NAME_DEBUG   lib${OUTPUT_NAME_DEBUG}
        OUTPUT_NAME_RELEASE lib${OUTPUT_NAME_RELEASE}
        # PREFIX "lib"    # for MSVC static/shared coexistence *DOES NOT WORK*
      )
    endif (MSVC)
  endif (${LIBTYPE} STREQUAL "SHARED")

  # Debug library output names (optional)
  set (DEBUG_ONAME 0)

  if (DEBUG_ONAME)
    get_target_property (XX_ONAME         ${TARGET_NAME} OUTPUT_NAME)
    get_target_property (XX_ONAME_DEBUG   ${TARGET_NAME} OUTPUT_NAME_DEBUG)
    get_target_property (XX_ONAME_RELEASE ${TARGET_NAME} OUTPUT_NAME_RELEASE)

    fl_debug_var (TARGET_NAME)
    fl_debug_var (XX_ONAME)
    fl_debug_var (XX_ONAME_DEBUG)
    fl_debug_var (XX_ONAME_RELEASE)
    message (STATUS "---")
  endif (DEBUG_ONAME)

  if (MSVC)
    if (OPTION_LARGE_FILE)
      set_target_properties(${TARGET_NAME}
        PROPERTIES
        LINK_FLAGS /LARGEADDRESSAWARE
      )
    endif (OPTION_LARGE_FILE)

    if (${LIBTYPE} STREQUAL "SHARED")
      set_target_properties(${TARGET_NAME}
        PROPERTIES
        COMPILE_DEFINITIONS "FL_DLL"
      )
    endif (${LIBTYPE} STREQUAL "SHARED")
  endif (MSVC)

  install (TARGETS ${TARGET_NAME}
    EXPORT FLTK-Targets
    RUNTIME DESTINATION ${FLTK_BINDIR}
    LIBRARY DESTINATION ${FLTK_LIBDIR}
    ARCHIVE DESTINATION ${FLTK_LIBDIR}
  )

  list (APPEND FLTK_LIBRARIES "${TARGET_NAME}")
  set (FLTK_LIBRARIES ${FLTK_LIBRARIES} PARENT_SCOPE)

endmacro (FL_ADD_LIBRARY LIBNAME LIBTYPE LIBFILES)
