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

# Add the console app (Windows only)
# This is done for all Windows targets, even if cross-compiling.

if(WIN32)
  list(APPEND TARGETS fltk-options-cmd)
  add_executable(fltk-options-cmd fltk-options.cxx)
  target_link_libraries(fltk-options-cmd PRIVATE fltk::fltk)
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

  # On macOS, fltk-options must be installed twice. The bundled version of
  # fltk-options needs to go into the /Applications folder to make it visible
  # as a user App with full GUI.
  # The binary without bundle should go into ${FLTK_BINDIR}, usually
  # /usr/local/bin, so it will be picked up as a command line tool by
  # the build process of other apps.
  # On macOS the command line tool is the same target ('fltk-options') as the
  # one included in the bundle.

  # create bundle
  set_target_properties(fltk-options PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/fltk-options.plist")
  set_target_properties(fltk-options PROPERTIES MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
  set_target_properties(fltk-options PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "org.fltk.fltk-options")

  # The line below would wrongly install /Applications/fltk-options.icns
  # ## set_target_properties(fltk-options PROPERTIES RESOURCE ${ICON_PATH})

  # install fltk-options GUI and commandline tools
  # install(TARGETS fltk-options DESTINATION "/Applications")

  # install command line tool
  install(PROGRAMS $<TARGET_FILE:fltk-options> DESTINATION ${FLTK_BINDIR})

else()

  # install fltk-options GUI and optional commandline tool 'fltk-options-cmd'
  # (only on Windows)

  install(TARGETS ${TARGETS}
    EXPORT FLTK-Targets
    RUNTIME DESTINATION ${FLTK_BINDIR}
    LIBRARY DESTINATION ${FLTK_LIBDIR}
    ARCHIVE DESTINATION ${FLTK_LIBDIR}
  )

endif(APPLE AND NOT FLTK_BACKEND_X11)

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
