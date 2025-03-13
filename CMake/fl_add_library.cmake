#
# Macro used by the CMake build system for the Fast Light Tool Kit (FLTK).
# Originally written by Michael Surette
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
# fl_add_library - add a static or shared library to the build
#======================================================================
#
# Input:
#
#   LIBNAME: name of the library, including 'fltk_' prefix if applicable.
#
#   LIBTYPE: either "STATIC" or "SHARED"
#
#   SOURCES: Files needed to build the library
#
# Output:
#
#   FLTK_LIBRARIES or FLTK_LIBRARIES_SHARED (in parent scope)
#
# This function adds the given library to the build, adds it to
# either FLTK_LIBRARIES or FLTK_LIBRARIES_SHARED, respectively,
# and "exports" the modified variable to the parent scope.
#
# For each library an alias is defined (see comment below).
#
#######################################################################

function(fl_add_library LIBNAME LIBTYPE SOURCES)

  # message(STATUS "Building library  **************** ${LIBNAME} ${LIBTYPE}")

  set(suffix "")
  if(LIBTYPE STREQUAL "SHARED")
    set(suffix "-shared")
  endif()

  set(TARGET_NAME ${LIBNAME}${suffix})

  ## Strip 'fltk_' from target name (if it exists in the name)
  ## and use the result as EXPORT_NAME property. This makes it
  ## easy to export library targets fltk::fltk and fltk::images
  ## rather than fltk::fltk_images.

  string(REPLACE "fltk_" "" EXPORT_NAME ${TARGET_NAME})

  if(MSVC)
    set(OUTPUT_NAME_DEBUG   "${LIBNAME}d")
  else()
    set(OUTPUT_NAME_DEBUG   "${LIBNAME}")
  endif(MSVC)

  set(OUTPUT_NAME_RELEASE "${LIBNAME}")

  add_library(${TARGET_NAME} ${LIBTYPE} ${SOURCES})

  # Create an alias 'fltk::alias_name' for the library
  # where 'alias_name' is the library name without the prefix 'fltk_'
  #
  # e.g. 'fltk'               -> 'fltk::fltk'
  # and  'fltk-shared'        -> 'fltk::fltk-shared'
  # but  'fltk_images'        -> 'fltk::images'
  # and  'fltk_images-shared' -> 'fltk::images-shared'

  if(NOT (LIBNAME STREQUAL "fltk"))
    string(REPLACE "fltk_" "" alias_name ${LIBNAME})
  else()
    set(alias_name ${LIBNAME})
  endif()
  set(alias_name "fltk::${alias_name}${suffix}")

  add_library(${alias_name} ALIAS ${TARGET_NAME})

  if(0)
    fl_debug_var(TARGET_NAME)
    fl_debug_var(LIBTYPE)
    fl_debug_var(alias_name)
    # fl_debug_var(SOURCES)
  endif()

  # Target properties for all libraries

  # Set 'PRIVATE' target compile definitions for the library
  # so they are not inherited by consumers

  target_compile_definitions(${TARGET_NAME} PRIVATE "FL_LIBRARY")

  # Set PUBLIC properties, e.g. C++ standard and include and linker directories.
  # These properties are inherited by consumers of the libraries

  target_compile_features(${TARGET_NAME} PUBLIC "cxx_std_${CMAKE_CXX_STANDARD}")

  if(0) # DEBUG
    message(STATUS "fl_add_library and alias         : fltk::${alias_name} ALIAS ${TARGET_NAME}")
    fl_debug_var(TARGET_NAME)
    fl_debug_var(FLTK_INCLUDE_DIRS)
    fl_debug_var(CMAKE_CURRENT_BINARY_DIR)
    fl_debug_var(CMAKE_CURRENT_SOURCE_DIR)
    fl_debug_var(FLTK_BUILD_INCLUDE_DIRECTORIES)
  endif()

  # Special handling for the core 'fltk' library,
  # no matter if it's SHARED or STATIC
  # FIXME: maybe this should be in src/CMakeLists.txt (?)

  if(LIBNAME STREQUAL "fltk")

    target_include_directories(${TARGET_NAME} PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
      $<INSTALL_INTERFACE:include>
    )

    ### FIXME: why does the simplified else() block not work?
    ### Needs investigation, using 'if(1)' for now...

    if(1)

      foreach(dir ${FLTK_BUILD_INCLUDE_DIRECTORIES})
        target_include_directories(${TARGET_NAME} PRIVATE
          $<BUILD_INTERFACE:${dir}>
        )
      endforeach()

    else()

      ### This generates a wrong string in property INTERFACE_INCLUDE_DIRECTORIES:
      ### ... $<INSTALL_INTERFACE:include>;/git/fltk/modern-cmake/src/$<BUILD_INTERFACE:/usr/include/freetype2; ...
      ### ... --- OK ---------------------^^^^^^^ WRONG ^^^^^^^^^^^^^^-------- would be OK -------------------- ...
      ### End of string: ';/usr/include/libpng16' (WRONG: missing '>')
      ### I don't see anything wrong with this statement though but
      ### maybe I'm missing something. Albrecht, Jan 22 2024

      target_include_directories(${TARGET_NAME} PRIVATE
        $<BUILD_INTERFACE:${FLTK_BUILD_INCLUDE_DIRECTORIES}>
      )

    endif()

    target_link_directories(${TARGET_NAME} PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../lib>
      $<INSTALL_INTERFACE:lib>
    )

    if(APPLE AND NOT FLTK_BACKEND_X11)
      foreach(item ${FLTK_COCOA_FRAMEWORKS})
        target_link_libraries(${TARGET_NAME} PUBLIC "${item}")
      endforeach()
    endif()

    # we must link fltk with cairo if Cairo or Wayland is enabled (or both)
    if(FLTK_HAVE_CAIRO OR FLTK_USE_CAIRO)
      target_include_directories(${TARGET_NAME} PUBLIC ${PKG_CAIRO_INCLUDE_DIRS})
      target_link_directories   (${TARGET_NAME} PUBLIC ${PKG_CAIRO_LIBRARY_DIRS})
      target_link_libraries     (${TARGET_NAME} PUBLIC ${PKG_CAIRO_LIBRARIES})
    endif()

  endif(LIBNAME STREQUAL "fltk")

  # Set additional target properties for static libraries

  if(LIBTYPE STREQUAL "STATIC")
    set_target_properties(${TARGET_NAME}
      PROPERTIES
      OUTPUT_NAME         ${LIBNAME}
      OUTPUT_NAME_DEBUG   ${OUTPUT_NAME_DEBUG}
      OUTPUT_NAME_RELEASE ${OUTPUT_NAME_RELEASE}
      EXPORT_NAME         ${EXPORT_NAME}
    )
  endif(LIBTYPE STREQUAL "STATIC")

  # Set additional target properties for shared (dynamic) libraries (DLL's)

  if(LIBTYPE STREQUAL "SHARED")
    set_target_properties(${TARGET_NAME} PROPERTIES
      VERSION             ${FLTK_VERSION}
      SOVERSION           ${FLTK_VERSION_MAJOR}.${FLTK_VERSION_MINOR}
      OUTPUT_NAME         ${LIBNAME}
      OUTPUT_NAME_DEBUG   ${OUTPUT_NAME_DEBUG}
      OUTPUT_NAME_RELEASE ${OUTPUT_NAME_RELEASE}
      EXPORT_NAME         ${EXPORT_NAME}
    )
    # Visual Studio only:
    if(MSVC)
      set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME         ${LIBNAME}_dll
        OUTPUT_NAME_DEBUG   ${LIBNAME}_dlld
        OUTPUT_NAME_RELEASE ${LIBNAME}_dll
      )
      target_compile_definitions(${TARGET_NAME} PUBLIC FL_DLL)
    endif(MSVC)
  endif(LIBTYPE STREQUAL "SHARED")

  # Debug library output names (optional)
  set(DEBUG_ONAME 0)

  if(DEBUG_ONAME)
    get_target_property(XX_NAME          ${TARGET_NAME} NAME)
    get_target_property(XX_ONAME         ${TARGET_NAME} OUTPUT_NAME)
    get_target_property(XX_ONAME_DEBUG   ${TARGET_NAME} OUTPUT_NAME_DEBUG)
    get_target_property(XX_ONAME_RELEASE ${TARGET_NAME} OUTPUT_NAME_RELEASE)
    get_target_property(XX_EXPORT_NAME   ${TARGET_NAME} EXPORT_NAME)

    message(STATUS "--- DEBUG_ONAME ---")
    fl_debug_var(TARGET_NAME)
    fl_debug_var(XX_NAME)
    fl_debug_var(XX_ONAME)
    fl_debug_var(XX_ONAME_DEBUG)
    fl_debug_var(XX_ONAME_RELEASE)
    fl_debug_var(XX_EXPORT_NAME)
    message(STATUS "--- /DEBUG_ONAME ---")
  endif(DEBUG_ONAME)

  if(MSVC)
    if(FLTK_OPTION_LARGE_FILE)
      set_target_properties(${TARGET_NAME} PROPERTIES
        LINK_FLAGS /LARGEADDRESSAWARE
      )
    endif(FLTK_OPTION_LARGE_FILE)
  endif(MSVC)

  install(TARGETS ${TARGET_NAME} EXPORT FLTK-Targets
    RUNTIME DESTINATION ${FLTK_BINDIR}
    LIBRARY DESTINATION ${FLTK_LIBDIR}
    ARCHIVE DESTINATION ${FLTK_LIBDIR}
  )

  if(LIBTYPE STREQUAL "SHARED")
    list(APPEND FLTK_LIBRARIES_SHARED "${TARGET_NAME}")
    set(FLTK_LIBRARIES_SHARED "${FLTK_LIBRARIES_SHARED}" PARENT_SCOPE)
  else()
    list(APPEND FLTK_LIBRARIES "${TARGET_NAME}")
    set(FLTK_LIBRARIES "${FLTK_LIBRARIES}" PARENT_SCOPE)
  endif()

  if(0)
    fl_debug_var(fl_add_library_DEBUG)
    fl_debug_var(FLTK_LIBRARIES)
    fl_debug_var(FLTK_LIBRARIES_SHARED)
    fl_debug_var(fl_add_library_END)
    message("")
  endif()

endfunction(fl_add_library LIBNAME LIBTYPE SOURCES)
