#
# CMakeLists.txt to build fltk-options for the FLTK project using CMake (www.cmake.org)
#
# Copyright 2023-2025 by Bill Spitzak and others.
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

# Targets that will be built: fltk-options and fltk-options-cmd (Windows)
set(TARGETS "")

if(APPLE AND NOT FLTK_BACKEND_X11)
  set(BACKEND_APPLE TRUE)
  set(ICON_NAME fltk-options.icns)
  set(ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/icons/${ICON_NAME}")
  list(APPEND SOURCES ${ICON_PATH})
else()
  set(BACKEND_APPLE FALSE)
  set(ICON_NAME "")
  set(ICON_PATH "")
endif()

# This macro is used to avoid duplicate code to create executable programs.
# This must be a macro because it changes at least one global variable: TARGETS.
# This macro also uses some (local) variables defined above.
# In the future this might be converted to a function to avoid side effects.

macro(make_target TARGET GUI SOURCES LIBS EXPORT_NAME)

  if(ICON_PATH)
    list(APPEND SOURCES ${ICON_PATH}) # macOS only
  endif()

  # message(STATUS "[fltk-options] make_target ${TARGET} ${GUI} ${SOURCES} ${LIBS} ${EXPORT_NAME}")

  # Options WIN32 and MACOSX_BUNDLE build a Windows GUI program or macOS bundle,
  # respectively. Both options are ignored on other platforms.

  if(${GUI})
    add_executable(${TARGET} WIN32 MACOSX_BUNDLE ${SOURCES})
  else()
    add_executable(${TARGET} ${SOURCES})
  endif(${GUI})

  list(APPEND TARGETS ${TARGET})

  if(BACKEND_APPLE)

    # set bundle properties
    set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.plist")
    set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "org.fltk.${TARGET}")

    # install command line tool
    install(PROGRAMS $<TARGET_FILE:${TARGET}>
            DESTINATION ${FLTK_BINDIR})

    # create macOS bundle wrapper script

    set(WRAPPER "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/${TARGET}")
    add_custom_command(
      TARGET ${TARGET} POST_BUILD
      COMMAND cp ${FLTK_SOURCE_DIR}/CMake/macOS-bundle-wrapper.in ${WRAPPER}
      COMMAND chmod u+x,g+x,o+x ${WRAPPER}
      BYPRODUCTS ${WRAPPER}
      VERBATIM
    )
    unset(WRAPPER)

  endif(BACKEND_APPLE)

  target_link_libraries(${TARGET} PRIVATE ${LIBS})
  set_target_properties(${TARGET} PROPERTIES EXPORT_NAME ${EXPORT_NAME})

endmacro(make_target TARGET GUI LIBS EXPORT_NAME)

set(SOURCES fltk-options.cxx)

make_target(fltk-options TRUE "${SOURCES}" fltk::fltk options)

# Add the console app (Windows only)
# This is done for all Windows targets, even if cross-compiling.

if(WIN32)
  make_target(fltk-options-cmd FALSE "${SOURCES}" fltk::fltk options-cmd)
endif()

# Add the "shared" executable (linked against the shared FLTK libs).
# Note 1: only the GUI version is built as "shared" executable.
# Note 2: For MSVC we need the special object library 'call_main'.

if(FLTK_BUILD_SHARED_LIBS)
  if(MSVC)
    set(libs fltk::fltk-shared call_main)
  else()
    set(libs fltk::fltk-shared)
  endif()
  make_target(fltk-options-shared TRUE "${SOURCES}" "${libs}" options-shared)
endif()

# Create aliases for all executables,
# replacing 'fltk-options' with 'fltk::options'

foreach(tgt ${TARGETS})
  string(REPLACE "fltk-options" "fltk::options" alias ${tgt})
  add_executable(${alias} ALIAS ${tgt})
  unset(alias)
endforeach()

# Install fltk-options GUI and commandline tool

# Install the GUI and (on Windows only) the commandline tool 'fltk-options-cmd'
# message(STATUS "fltk-options: INSTALL TARGETS: ${TARGETS}")

install(TARGETS ${TARGETS}
        EXPORT  FLTK-Targets
        RUNTIME DESTINATION ${FLTK_BINDIR}
        LIBRARY DESTINATION ${FLTK_LIBDIR}
        ARCHIVE DESTINATION ${FLTK_LIBDIR}
        BUNDLE  DESTINATION ${FLTK_BINDIR} # macOS: bundles
)

# Install desktop files

if(UNIX)
  install(FILES fltk-options.desktop
    DESTINATION ${FLTK_DATADIR}/applications
  )
  # Install mime-type file(x-fltk-options.desktop method is deprecated)
  install(FILES fltk-options.xml
    DESTINATION ${FLTK_DATADIR}/mime/packages
  )

  # Install desktop icons
  foreach(icon 32 48 64 128)
    install(FILES icons/fltk-options-${icon}.png
      DESTINATION ${FLTK_DATADIR}/icons/hicolor/${icon}x${icon}/apps
      RENAME fltk-options.png
    )
  endforeach()
endif(UNIX)
