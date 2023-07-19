#
# CMakeLists.txt to build fluid for the FLTK project using CMake (www.cmake.org)
#
# Copyright 1998-2023 by Bill Spitzak and others.
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

# Source files for 'fluid-lib' = all source files except fluid.cxx

set (CPPFILES
  CodeEditor.cxx
  StyleParse.cxx
  Fd_Snap_Action.cxx
  Fl_Function_Type.cxx
  Fl_Group_Type.cxx
  Fl_Menu_Type.cxx
  Fl_Type.cxx
  Fl_Widget_Type.cxx
  Fl_Window_Type.cxx
  Fl_Button_Type.cxx
  Fluid_Image.cxx
  about_panel.cxx
  align_widget.cxx
  alignment_panel.cxx
  code.cxx
  custom_widgets.cxx
  factory.cxx
  file.cxx
  function_panel.cxx
  pixmaps.cxx
  shell_command.cxx
  template_panel.cxx
  undo.cxx
  widget_browser.cxx
  widget_panel.cxx
)

# List header files in Apple's Xcode IDE

set (HEADERFILES
  CodeEditor.h
  Fd_Snap_Action.h
  Fl_Function_Type.h
  Fl_Group_Type.h
  Fl_Menu_Type.h
  Fl_Type.h
  Fl_Widget_Type.h
  Fl_Window_Type.h
  Fl_Button_Type.h
  Fluid_Image.h
  StyleParse.h
  about_panel.h
  align_widget.h
  alignment_panel.h
  code.h
  comments.h
  custom_widgets.h
  factory.h
  file.h
  function_panel.h
  print_panel.h
  pixmaps.h
  shell_command.h
  template_panel.h
  undo.h
  widget_browser.h
  widget_panel.h
)

# Add ExternalCodeEditor: platform specific files

if (WIN32)
  list (APPEND CPPFILES ExternalCodeEditor_WIN32.cxx)
  list (APPEND HEADERFILES ExternalCodeEditor_WIN32.h)
else ()
  list (APPEND CPPFILES ExternalCodeEditor_UNIX.cxx)
  list (APPEND HEADERFILES ExternalCodeEditor_UNIX.h)
endif (WIN32)

source_group("Header Files" FILES ${HEADERFILES})

# Build a local object library to avoid compiling all source files
# for all fluid targets (fluid, fluid-cmd, fluid-shared). This
# library includes everything except the main program (fluid.cxx).

add_library (fluid-lib OBJECT EXCLUDE_FROM_ALL ${CPPFILES} ${HEADERFILES})

# Build fluid with all its variants (fluid-cmd, fluid-shared) ...

set (FLUID_TARGETS fluid)                   # fluid and optional fluid-cmd target
set (FLUID_LIBS fluid-lib fltk fltk_images) # libraries used to link fluid executables

if (APPLE AND (NOT OPTION_APPLE_X11))

  # macOS

  set (ICON_NAME fluid.icns)
  set (ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/icons/${ICON_NAME}")
  add_executable (fluid MACOSX_BUNDLE fluid.cxx fluid.h ${ICON_PATH})

  # create macOS bundle wrapper script

  set (WRAPPER "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/fluid")
  add_custom_command (
    TARGET fluid POST_BUILD
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/../CMake/macOS-bundle-wrapper.in ${WRAPPER}
    COMMAND chmod u+x,g+x,o+x ${WRAPPER}
    BYPRODUCTS ${WRAPPER}
    VERBATIM
  )
  unset (WRAPPER)

else ()

  # Option 'WIN32' builds a Windows GUI program, ignored on other platforms
  add_executable (fluid WIN32 fluid.cxx fluid.h)

endif ()

# Link fluid with Cairo if OPTION_CAIRO is enabled
if (FLTK_HAVE_CAIRO)
  target_include_directories (fluid PRIVATE ${PKG_CAIRO_INCLUDE_DIRS})
  if (PKG_CAIRO_LIBRARY_DIRS)
    fl_target_link_directories (fluid PRIVATE ${PKG_CAIRO_LIBRARY_DIRS})
  endif ()
endif (FLTK_HAVE_CAIRO)

if (USE_GDIPLUS)        # can only be true on Windows
  list (APPEND FLUID_LIBS gdiplus)
endif (USE_GDIPLUS)

target_link_libraries (fluid ${FLUID_LIBS})

# Add fluid-cmd console app (Windows only) for converting .fl to .cxx/.h files.
# This is done for all Windows targets, even if cross-compiling.

if (WIN32)
  list (APPEND FLUID_TARGETS fluid-cmd)
  add_executable (fluid-cmd fluid.cxx fluid.h)
  target_link_libraries (fluid-cmd ${FLUID_LIBS})

  # Link fluid-cmd with Cairo if OPTION_CAIRO is enabled (same as above)

  if (FLTK_HAVE_CAIRO)
    target_include_directories (fluid-cmd PRIVATE ${PKG_CAIRO_INCLUDE_DIRS})
    if (PKG_CAIRO_LIBRARY_DIRS)
      fl_target_link_directories (fluid-cmd PRIVATE ${PKG_CAIRO_LIBRARY_DIRS})
    endif ()
  endif (FLTK_HAVE_CAIRO)
endif (WIN32)

# Build fluid-shared (experimental)

if (OPTION_BUILD_SHARED_LIBS)
  list (APPEND FLUID_TARGETS fluid-shared)
  add_executable (fluid-shared fluid.cxx fluid.h)
  if (MSVC)
    target_link_libraries (fluid-shared fluid-lib fltk_SHARED)
  else ()
    target_link_libraries (fluid-shared fluid-lib fltk_images_SHARED)
  endif (MSVC)
endif ()

# Install fluid GUI and commandline tool

if (APPLE AND (NOT OPTION_APPLE_X11))

  # On macOS, Fluid must be installed twice. The bundled version of Fluid needs
  # to go into the /Applications folder to make it visible as a user App with
  # full GUI. The binary without bundle should go into ${FLTK_BINDIR}, usually
  # /usr/local/bin, so it will be picked up as a command line tool by
  # the build process of other apps.
  # On macOS the command line tool is the same target ('fluid') as the one
  # included in the bundle.

  # create bundle
  set_target_properties (fluid PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/fluid.plist")
  set_target_properties (fluid PROPERTIES MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
  set_target_properties (fluid PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "org.fltk.fluid")

  # The line below would wrongly install /Applications/fluid.icns
  # ## set_target_properties (fluid PROPERTIES RESOURCE ${ICON_PATH})

  # install fluid GUI and commandline tools
  # install (TARGETS fluid DESTINATION "/Applications")

  # install command line tool
  install (PROGRAMS $<TARGET_FILE:fluid> DESTINATION ${FLTK_BINDIR})

else()

  # install Fluid GUI and optional commandline tool 'fluid-cmd' (only on Windows)

  install (TARGETS ${FLUID_TARGETS}
    EXPORT FLTK-Targets
    RUNTIME DESTINATION ${FLTK_BINDIR}
    LIBRARY DESTINATION ${FLTK_LIBDIR}
    ARCHIVE DESTINATION ${FLTK_LIBDIR}
  )

endif (APPLE AND (NOT OPTION_APPLE_X11))

# Install desktop files

if (UNIX)
  install (FILES fluid.desktop
    DESTINATION ${FLTK_DATADIR}/applications
  )
  # Install mime-type file (x-fluid.desktop method is deprecated)
  install (FILES fluid.xml
    DESTINATION ${FLTK_DATADIR}/mime/packages
  )

  # Install desktop icons
  foreach (icon 32 48 64 128)
    install (FILES icons/fluid-${icon}.png
      DESTINATION ${FLTK_DATADIR}/icons/hicolor/${icon}x${icon}/apps
      RENAME fluid.png
    )
  endforeach()
endif (UNIX)
