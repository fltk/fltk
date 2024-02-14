#
# CMakeLists.txt to build fltk-options for the FLTK project using CMake (www.cmake.org)
#
# Copyright 2023-2024 by Bill Spitzak and others.
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
set(TARGETS fltk-options)

if(APPLE AND NOT FLTK_BACKEND_X11)

  # macOS

  set(ICON_NAME fltk-options.icns)
  set(ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/icons/${ICON_NAME}")
  add_executable(fltk-options MACOSX_BUNDLE fltk-options.cxx ${ICON_PATH})

  # create macOS bundle wrapper script

  set(WRAPPER "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/fltk-options")
  add_custom_command(
    TARGET fltk-options POST_BUILD
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/../CMake/macOS-bundle-wrapper.in ${WRAPPER}
    COMMAND chmod u+x,g+x,o+x ${WRAPPER}
    BYPRODUCTS ${WRAPPER}
    VERBATIM
  )
  unset(WRAPPER)

else()

  # Option WIN32 builds a Windows GUI program, ignored on other platforms
  add_executable(fltk-options WIN32 fltk-options.cxx)

endif()

target_link_libraries(fltk-options PRIVATE fltk::fltk)
set_target_properties(fltk-options PROPERTIES EXPORT_NAME options)

# Add the console app (Windows only)
# This is done for all Windows targets, even if cross-compiling.

if(WIN32)
  list(APPEND TARGETS fltk-options-cmd)
  add_executable(fltk-options-cmd fltk-options.cxx)
  target_link_libraries(fltk-options-cmd PRIVATE fltk::fltk)
  set_target_properties(fltk-options-cmd PROPERTIES EXPORT_NAME options-cmd)
endif()

# Create aliases for all executables,
# replacing 'fltk-options' with 'fltk::options'

foreach(tgt ${TARGETS})
  string(REPLACE "fltk-options" "fltk::options" alias ${tgt})
  add_executable(${alias} ALIAS ${tgt})
  unset(alias)
endforeach()

# Install fltk-options GUI and commandline tool

if(APPLE AND NOT FLTK_BACKEND_X11)

  # On macOS, fltk-options will be installed twice:
  # - The bundled version goes into the destination folder ${FLTK_BINDIR}.
  # - The binary without bundle goes into ${FLTK_BINDIR} as well.
  # These folders are relative to the install prefix, usually 'bin'.
  # The command line tool is the same executable as the one included in the bundle.
  # Note:
  #   Both the bundle and the commandline tool are currently installed side by side.
  #   This may be changed in the future.

  # set bundle properties
  set_target_properties(fltk-options PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/fltk-options.plist")
  set_target_properties(fltk-options PROPERTIES MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
  set_target_properties(fltk-options PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "org.fltk.fltk-options")

  # install command line tool
  install(PROGRAMS $<TARGET_FILE:fltk-options>
          DESTINATION ${FLTK_BINDIR})

endif(APPLE AND NOT FLTK_BACKEND_X11)

# Install the GUI and (on Windows only) the commandline tool 'fltk-options-cmd'

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
