#
# CMakeLists.txt to build fltk-options for the FLTK project using CMake (www.cmake.org)
#
# Copyright 2023 by Bill Spitzak and others.
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

set (CPPFILES
  fltk-options.cxx
)

set (HEADERFILES
)

set (FLTK_OPTIONS_TARGETS fltk-options)  # fltk-options and optional fltk-options-cmd target
set (FLTK_OPTIONS_LIBS fltk)  # libraries used to link fltk-options executables

if (APPLE AND (NOT OPTION_APPLE_X11))

  # macOS

  set (ICON_NAME fltk-options.icns)
  set (ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/icons/${ICON_NAME}")
  add_executable (fltk-options MACOSX_BUNDLE ${CPPFILES} ${HEADERFILES} ${ICON_PATH})

  # create macOS bundle wrapper script

  set (WRAPPER "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/fltk-options")
  add_custom_command (
    TARGET fltk-options POST_BUILD
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/../CMake/macOS-bundle-wrapper.in ${WRAPPER}
    COMMAND chmod u+x,g+x,o+x ${WRAPPER}
    BYPRODUCTS ${WRAPPER}
    VERBATIM
  )
  unset (WRAPPER)

else ()

  # option WIN32 builds a Windows GUI program, ignored on other platforms
  add_executable (fltk-options WIN32 ${CPPFILES} ${HEADERFILES})

endif ()

# we must link all programs with fltk_cairo if option CAIROEXT is enabled
if (FLTK_HAVE_CAIROEXT)
  list (APPEND FLTK_OPTIONS_LIBS fltk_cairo cairo)
endif (FLTK_HAVE_CAIROEXT)

if (FLTK_HAVE_CAIRO)
  fl_target_link_directories (fltk-options PRIVATE "${PKG_CAIRO_LIBRARY_DIRS}")
endif (FLTK_HAVE_CAIRO)

if (USE_GDIPLUS)        # can only be true on Windows
  list (APPEND FLTK_OPTIONS_LIBS gdiplus)
endif (USE_GDIPLUS)

target_link_libraries (fltk-options ${FLTK_OPTIONS_LIBS})


# Add fltk-options-cmd console app (Windows only) for editing FLTK options.
# This is done for all Windows targets, even if cross-compiling.

if (WIN32)
  list (APPEND FLTK_OPTIONS_TARGETS fltk-options-cmd)
  add_executable (fltk-options-cmd ${CPPFILES} ${HEADERFILES})
  target_link_libraries (fltk-options-cmd ${FLTK_OPTIONS_LIBS})
  if (FLTK_HAVE_CAIRO)
    fl_target_link_directories (fltk-options-cmd PRIVATE "${PKG_CAIRO_LIBRARY_DIRS}")
  endif (FLTK_HAVE_CAIRO)
endif ()


# install fltk-options GUI and commandline tool

if (APPLE AND (NOT OPTION_APPLE_X11))

  # On macOS, fltk-options must be installed twice. The bundled version of
  # fltk-options needs to go into the /Applications folder to make it visible
  # as a user App with full GUI.
  # The binary without bundle should go into ${FLTK_BINDIR}, usually
  # /usr/local/bin, so it will be picked up as a command line tool by
  # the build process of other apps.
  # On macOS the command line tool is the same target ('fltk-options') as the
  # one included in the bundle.

  # create bundle
  set_target_properties (fltk-options PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/fltk-options.plist")
  set_target_properties (fltk-options PROPERTIES MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
  set_target_properties (fltk-options PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "org.fltk.fltk-options")

  # The line below would wrongly install /Applications/fltk-options.icns
  # ## set_target_properties (fltk-options PROPERTIES RESOURCE ${ICON_PATH})

  # install fltk-options GUI and commandline tools
  #install (TARGETS fltk-options DESTINATION "/Applications")

  # install command line tool
  install (PROGRAMS $<TARGET_FILE:fltk-options> DESTINATION ${FLTK_BINDIR})

else()

  # install fltk-options GUI and optional commandline tool 'fltk-options-cmd'
  # (only on Windows)

  install (TARGETS ${FLTK_OPTIONS_TARGETS}
    EXPORT FLTK-Targets
    RUNTIME DESTINATION ${FLTK_BINDIR}
    LIBRARY DESTINATION ${FLTK_LIBDIR}
    ARCHIVE DESTINATION ${FLTK_LIBDIR}
  )

  endif (APPLE AND (NOT OPTION_APPLE_X11))

# install desktop files

if (UNIX)
  install (FILES fltk-options.desktop
    DESTINATION ${FLTK_DATADIR}/applications
  )
  # Install mime-type file (x-fltk-options.desktop method is deprecated)
  install (FILES fltk-options.xml
    DESTINATION ${FLTK_DATADIR}/mime/packages
  )

  # Install desktop icons
  foreach (icon 32 48 64 128)
    install (FILES icons/fltk-options-${icon}.png
      DESTINATION ${FLTK_DATADIR}/icons/hicolor/${icon}x${icon}/apps
      RENAME fltk-options.png
    )
  endforeach()
endif (UNIX)
