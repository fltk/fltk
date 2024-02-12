#
# CMakeLists.txt to build fluid for the FLTK project using CMake (www.cmake.org)
#
# Copyright 1998-2024 by Bill Spitzak and others.
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

# Source files for 'fluid-lib' = all source files except the main file (fluid.cxx)
# Note: macOS (Xcode) needs at least one source file (fluid.cxx) to link the main
# program fluid properly

set(CPPFILES
  CodeEditor.cxx
  StyleParse.cxx
  Fd_Snap_Action.cxx
  Fl_Function_Type.cxx
  Fl_Grid_Type.cxx
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
  fluid_filename.cxx
  function_panel.cxx
  mergeback.cxx
  pixmaps.cxx
  shell_command.cxx
  sourceview_panel.cxx
  template_panel.cxx
  undo.cxx
  widget_browser.cxx
  widget_panel.cxx
)

# List header files in Apple's Xcode IDE

set(HEADERFILES
  CodeEditor.h
  Fd_Snap_Action.h
  Fl_Function_Type.h
  Fl_Grid_Type.h
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
  fluid.h
  function_panel.h
  mergeback.h
  print_panel.h
  pixmaps.h
  shell_command.h
  sourceview_panel.h
  template_panel.h
  undo.h
  widget_browser.h
  widget_panel.h
)

# Add ExternalCodeEditor: platform specific files

if(WIN32)
  list(APPEND CPPFILES ExternalCodeEditor_WIN32.cxx)
  list(APPEND HEADERFILES ExternalCodeEditor_WIN32.h)
else()
  list(APPEND CPPFILES ExternalCodeEditor_UNIX.cxx)
  list(APPEND HEADERFILES ExternalCodeEditor_UNIX.h)
endif(WIN32)

source_group("Header Files" FILES ${HEADERFILES})

# make a group in the IDE for easy access to documentation files

set(USER_DOC_FILES
  Doxyfile
  documentation/src/index.dox
  documentation/src/page_introduction.dox
  documentation/src/page_getting_started.dox
  documentation/src/page_ui_overview.dox
  documentation/src/page_basics.dox
  documentation/src/page_widget_types.dox
  documentation/src/page_layout.dox
  documentation/src/page_code_integration.dox
  documentation/src/page_build_integration.dox
  documentation/src/page_advanced.dox
  documentation/src/page_shortcuts.dox
  documentation/src/page_faqs.dox
  documentation/src/page_glossary.dox
  documentation/src/page_appendices.dox
  documentation/src/page_index.dox
  documentation/src/page_legal.dox
  documentation/src/page_conclusion.dox
)
source_group("Documentation: User" FILES ${USER_DOC_FILES})

set(DEV_DOC_FILES
  Doxyfile-dev
  documentation/src-dev/index.dox
  documentation/src-dev/page_introduction.dox
)
source_group("Documentation: Developer" FILES ${DEV_DOC_FILES})

# Build a local object library to avoid compiling all source files
# multiple times for all fluid targets on Windows (fluid + fluid-cmd).

add_library(fluid-lib OBJECT EXCLUDE_FROM_ALL)
target_sources(fluid-lib PRIVATE ${CPPFILES} ${HEADERFILES})
target_link_libraries(fluid-lib PUBLIC fltk::images)

# Build targets

if(APPLE AND NOT FLTK_BACKEND_X11)

  # macOS

  set(ICON_NAME fluid.icns)
  set(ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/icons/${ICON_NAME}")
  add_executable(fluid MACOSX_BUNDLE fluid.cxx ${ICON_PATH})

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
  add_executable(fluid WIN32 fluid.cxx)

endif()

target_link_libraries(fluid PRIVATE fluid-lib)

# Build the console app on Windows
# This is done for all Windows targets, even if cross-compiling.

if(WIN32)
  list(APPEND TARGETS fluid-cmd)
  add_executable(fluid-cmd fluid.cxx)
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

  # On macOS, Fluid must be installed twice. The bundled version of Fluid needs
  # to go into the /Applications folder to make it visible as a user App with
  # full GUI. The binary without bundle should go into ${FLTK_BINDIR}, usually
  # /usr/local/bin, so it will be picked up as a command line tool by
  # the build process of other apps.
  # On macOS the command line tool is the same target ('fluid') as the one
  # included in the bundle.

  # create bundle
  set_target_properties(fluid PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/fluid.plist")
  set_target_properties(fluid PROPERTIES MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
  set_target_properties(fluid PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "org.fltk.fluid")

  # The line below would wrongly install /Applications/fluid.icns
  # ## set_target_properties(fluid PROPERTIES RESOURCE ${ICON_PATH})

  # install fluid GUI and commandline tools
  # install(TARGETS fluid DESTINATION "/Applications")

  # install command line tool
  install(PROGRAMS $<TARGET_FILE:fluid> DESTINATION ${FLTK_BINDIR})

else()

  # install Fluid GUI and optional commandline tool 'fluid-cmd' (only on Windows)

  install(TARGETS ${TARGETS}
    EXPORT FLTK-Targets
    RUNTIME DESTINATION ${FLTK_BINDIR}
    LIBRARY DESTINATION ${FLTK_LIBDIR}
    ARCHIVE DESTINATION ${FLTK_LIBDIR}
  )

endif(APPLE AND NOT FLTK_BACKEND_X11)

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
