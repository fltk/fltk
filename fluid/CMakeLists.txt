#
# CMakeLists.txt to build fluid for the FLTK project using CMake (www.cmake.org)
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

# Targets that will be built: fluid and fluid-cmd (Windows)
set(TARGETS fluid)

# Source files for 'fluid-lib' = all source files except the main files
# (main.cxx and main.h)
# Note: macOS (Xcode) needs at least one source file (main.cxx) to link the main
# program fluid properly

set(CPPFILES
  app/align_widget.cxx
  app/Fd_Snap_Action.cxx
  app/fluid.cxx
  app/Fluid_Image.cxx
  app/mergeback.cxx
  app/shell_command.cxx
  app/undo.cxx
  io/code.cxx
  io/file.cxx
  nodes/factory.cxx
  nodes/Fl_Button_Type.cxx
  nodes/Fl_Function_Type.cxx
  nodes/Fl_Grid_Type.cxx
  nodes/Fl_Group_Type.cxx
  nodes/Fl_Menu_Type.cxx
  nodes/Fl_Type.cxx
  nodes/Fl_Widget_Type.cxx
  nodes/Fl_Window_Type.cxx
  panels/about_panel.cxx
  panels/codeview_panel.cxx
  panels/function_panel.cxx
  panels/settings_panel.cxx
  panels/template_panel.cxx
  panels/widget_panel.cxx
  rsrcs/pixmaps.cxx
  tools/autodoc.cxx
  tools/fluid_filename.cxx
  widgets/Code_Editor.cxx
  widgets/Code_Viewer.cxx
  widgets/Text_Viewer.cxx
  widgets/Formula_Input.cxx
  widgets/Bin_Button.cxx
  widgets/Style_Parser.cxx
  widgets/widget_browser.cxx
)

# List header files in Apple's Xcode IDE

set(HEADERFILES
  app/align_widget.h
  app/Fd_Snap_Action.h
  app/fluid.h
  app/Fluid_Image.h
  app/mergeback.h
  app/shell_command.h
  app/undo.h
  io/code.h
  io/file.h
  nodes/factory.h
  nodes/Fl_Button_Type.h
  nodes/Fl_Function_Type.h
  nodes/Fl_Grid_Type.h
  nodes/Fl_Group_Type.h
  nodes/Fl_Menu_Type.h
  nodes/Fl_Type.h
  nodes/Fl_Widget_Type.h
  nodes/Fl_Window_Type.h
  panels/about_panel.h
  panels/codeview_panel.h
  panels/function_panel.h
  panels/settings_panel.h
  panels/template_panel.h
  panels/widget_panel.h
  rsrcs/comments.h
  rsrcs/pixmaps.h
  tools/autodoc.h
  tools/fluid_filename.h
  widgets/Code_Editor.h
  widgets/Code_Viewer.h
  widgets/Text_Viewer.h
  widgets/Formula_Input.h
  widgets/Bin_Button.h
  widgets/Style_Parser.h
  widgets/widget_browser.h
)

# Add ExternalCodeEditor: platform specific files

if(WIN32)
  list(APPEND CPPFILES tools/ExternalCodeEditor_WIN32.cxx)
  list(APPEND HEADERFILES tools/ExternalCodeEditor_WIN32.h)
else()
  list(APPEND CPPFILES tools/ExternalCodeEditor_UNIX.cxx)
  list(APPEND HEADERFILES tools/ExternalCodeEditor_UNIX.h)
endif(WIN32)

source_group(
  TREE
    ${CMAKE_CURRENT_SOURCE_DIR}
  FILES
    ${CPPFILES}
    ${HEADERFILES}
    main.cxx
    main.h
    CMakeLists.txt
)

# Build a local object library to avoid compiling all source files
# multiple times for all fluid targets on Windows (fluid + fluid-cmd).

add_library(fluid-lib OBJECT EXCLUDE_FROM_ALL)
target_sources(fluid-lib PRIVATE ${CPPFILES} ${HEADERFILES})
target_include_directories(fluid-lib PRIVATE .)
target_link_libraries(fluid-lib PUBLIC fltk::images)

# Build targets

if(APPLE AND NOT FLTK_BACKEND_X11)

  # macOS

  set(ICON_NAME fluid.icns)
  set(ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/icons/${ICON_NAME}")
  add_executable(fluid MACOSX_BUNDLE main.cxx main.h ${ICON_PATH})

  # create macOS bundle wrapper script

  set(WRAPPER "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/fluid")
  add_custom_command(
    TARGET fluid POST_BUILD
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/../CMake/macOS-bundle-wrapper.in ${WRAPPER}
    COMMAND chmod u+x,g+x,o+x ${WRAPPER}
    BYPRODUCTS ${WRAPPER}
    VERBATIM
  )
  unset(WRAPPER)

else()

  # Option 'WIN32' builds a Windows GUI program, ignored on other platforms
  add_executable(fluid WIN32 main.cxx main.h)

endif()

target_include_directories(fluid PRIVATE .)
target_link_libraries(fluid PRIVATE fluid-lib)

# Build the console app on Windows
# This is done for all Windows targets, even if cross-compiling.

if(WIN32)
  list(APPEND TARGETS fluid-cmd)
  add_executable(fluid-cmd main.cxx main.h)
  target_include_directories(fluid-cmd PRIVATE .)
  target_link_libraries(fluid-cmd PRIVATE fluid-lib)
  set(FLTK_FLUID_EXECUTABLE fltk::fluid-cmd)
else()
  set(FLTK_FLUID_EXECUTABLE fltk::fluid)
endif()

set(FLTK_FLUID_EXECUTABLE "${FLTK_FLUID_EXECUTABLE}" PARENT_SCOPE)

# Create aliases for all targets

foreach(tgt ${TARGETS})
  add_executable(fltk::${tgt} ALIAS ${tgt})
endforeach()

# Install fluid GUI and commandline tool

if(APPLE AND NOT FLTK_BACKEND_X11)

  # On macOS, fluid will be installed twice:
  # - The bundled version of Fluid goes into the destination folder ${FLTK_BINDIR}.
  # - The binary without bundle goes into ${FLTK_BINDIR} as well.
  # The command line tool is the same executable as the one included in the bundle.
  # Note:
  #   Both the bundle and the commandline tool are currently installed side by side.
  #   This may be changed in the future.

  # Set bundle properties
  set_target_properties(fluid PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/fluid.plist")
  set_target_properties(fluid PROPERTIES MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
  set_target_properties(fluid PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "org.fltk.fluid")

  # Install command line tool
  install(PROGRAMS $<TARGET_FILE:fluid>
          DESTINATION ${FLTK_BINDIR})

endif(APPLE AND NOT FLTK_BACKEND_X11)

# Install the GUI and (on Windows only) the commandline tool 'fluid-cmd'

install(TARGETS ${TARGETS}
        EXPORT  FLTK-Targets
        RUNTIME DESTINATION ${FLTK_BINDIR}
        LIBRARY DESTINATION ${FLTK_LIBDIR}
        ARCHIVE DESTINATION ${FLTK_LIBDIR}
        BUNDLE  DESTINATION ${FLTK_BINDIR} # macOS: bundles
)

# Install desktop files

if(UNIX)
  install(FILES fluid.desktop
    DESTINATION ${FLTK_DATADIR}/applications
  )
  # Install mime-type file(x-fluid.desktop method is deprecated)
  install(FILES fluid.xml
    DESTINATION ${FLTK_DATADIR}/mime/packages
  )

  # Install desktop icons
  foreach(icon 32 48 64 128)
    install(FILES icons/fluid-${icon}.png
      DESTINATION ${FLTK_DATADIR}/icons/hicolor/${icon}x${icon}/apps
      RENAME fluid.png
    )
  endforeach()
endif(UNIX)
