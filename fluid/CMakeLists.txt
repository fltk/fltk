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

# Targets that will be built: fluid, fluid-shared, and fluid-cmd (Windows only)
set(TARGETS "")

# Defaults to be used and potentially modified later

set(BACKEND_APPLE FALSE)        # FIXME: should be global, e.g. FLTK_BACKEND_APPLE
set(ICON_NAME "")
set(ICON_PATH "")
set(SUFFIX "UNIX")              # suffix for platform specific source files

# platform specific settings

if(APPLE AND NOT FLTK_BACKEND_X11)    # macOS "native"
  set(BACKEND_APPLE TRUE)
  set(ICON_NAME fluid.icns)
  set(ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/icons/${ICON_NAME}")
elseif(WIN32)
  set(SUFFIX "WIN32")
endif()

# This macro is used to avoid duplicate code to create executable programs.
# This must be a macro because it changes at least one global variable: TARGETS.
# This macro also uses some (local) variables defined above.
# In the future this might be converted to a function to avoid side effects.

macro(make_target TARGET GUI SOURCES LIBS EXPORT_NAME)

  if(ICON_PATH)
    list(APPEND SOURCES ${ICON_PATH}) # macOS only
  endif()

  # message(STATUS "[fluid] make_target ${TARGET} ${GUI} ${SOURCES} ${LIBS} ${EXPORT_NAME}")

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

endmacro(make_target TARGET GUI SOURCES LIBS EXPORT_NAME)


# Main source and header files used for the executable because macOS (Xcode)
# needs at least one source file (main.cxx) to link the main program properly

set(MAIN_FILES main.cxx main.h)

# Source files for 'fluid-lib': all source files except ${MAIN_FILES}

set(CPPFILES
  Fluid.cxx
  Project.cxx
  app/args.cxx
  app/Snap_Action.cxx
  app/Image_Asset.cxx
  app/history.cxx
  app/Menu.cxx
  app/shell_command.cxx
  app/templates.cxx
  io/Code_Writer.cxx
  io/Project_Writer.cxx
  io/Project_Reader.cxx
  io/String_Writer.cxx
  nodes/Tree.cxx
  nodes/Button_Node.cxx
  nodes/Function_Node.cxx
  nodes/Grid_Node.cxx
  nodes/Group_Node.cxx
  nodes/Menu_Node.cxx
  nodes/Node.cxx
  nodes/Widget_Node.cxx
  nodes/Window_Node.cxx
  nodes/callbacks.cxx
  nodes/factory.cxx
  panels/about_panel.cxx
  panels/codeview_panel.cxx
  panels/function_panel.cxx
  panels/settings_panel.cxx
  panels/template_panel.cxx
  panels/widget_panel.cxx
  panels/widget_panel/Grid_Tab.cxx
  panels/widget_panel/Grid_Child_Tab.cxx
  proj/align_widget.cxx
  proj/i18n.cxx
  proj/mergeback.cxx
  proj/undo.cxx
  rsrcs/pixmaps.cxx
  tools/ExternalCodeEditor_${SUFFIX}.cxx
  tools/autodoc.cxx
  tools/filename.cxx
  widgets/App_Menu_Bar.cxx
  widgets/Code_Editor.cxx
  widgets/Code_Viewer.cxx
  widgets/Text_Viewer.cxx
  widgets/Formula_Input.cxx
  widgets/Bin_Button.cxx
  widgets/Style_Parser.cxx
  widgets/Node_Browser.cxx
)

# List header files in Apple's Xcode IDE

set(HEADERFILES
  Fluid.h
  Project.h
  app/args.h
  app/Snap_Action.h
  app/Image_Asset.h
  app/history.h
  app/Menu.h
  app/shell_command.h
  app/templates.h
  io/Code_Writer.h
  io/Project_Writer.h
  io/Project_Reader.h
  io/String_Writer.h
  nodes/Tree.h
  nodes/Button_Node.h
  nodes/Function_Node.h
  nodes/Grid_Node.h
  nodes/Group_Node.h
  nodes/Menu_Node.h
  nodes/Node.h
  nodes/Widget_Node.h
  nodes/Window_Node.h
  nodes/callbacks.h
  nodes/factory.h
  panels/about_panel.h
  panels/codeview_panel.h
  panels/function_panel.h
  panels/settings_panel.h
  panels/template_panel.h
  panels/widget_panel.h
  panels/widget_panel/Grid_Tab.h
  panels/widget_panel/Grid_Child_Tab.h
  proj/align_widget.h
  proj/mergeback.h
  proj/i18n.h
  proj/undo.h
  rsrcs/comments.h
  rsrcs/pixmaps.h
  tools/ExternalCodeEditor_${SUFFIX}.h
  tools/autodoc.h
  tools/filename.h
  widgets/App_Menu_Bar.h
  widgets/Code_Editor.h
  widgets/Code_Viewer.h
  widgets/Text_Viewer.h
  widgets/Formula_Input.h
  widgets/Bin_Button.h
  widgets/Style_Parser.h
  widgets/Node_Browser.h
)

source_group(
  TREE
    ${CMAKE_CURRENT_SOURCE_DIR}
  FILES
    ${CPPFILES}
    ${HEADERFILES}
    ${MAIN_FILES}
    CMakeLists.txt
)

# Build a local object library to avoid compiling all source files
# multiple times for all fluid targets on Windows (fluid + fluid-cmd).

add_library(fluid-lib OBJECT EXCLUDE_FROM_ALL)
target_sources(fluid-lib PRIVATE ${CPPFILES} ${HEADERFILES})
target_include_directories(fluid-lib PUBLIC .)
target_link_libraries(fluid-lib PUBLIC fltk::images)

# Build targets

make_target(fluid TRUE "${MAIN_FILES}" fluid-lib fluid)

# Build the console app on Windows
# This is done for all Windows targets, even if cross-compiling.

if(WIN32)
  make_target(fluid-cmd FALSE "${MAIN_FILES}" fluid-lib fluid-cmd)
  set(FLTK_FLUID_EXECUTABLE fltk::fluid-cmd)
else()
  set(FLTK_FLUID_EXECUTABLE fltk::fluid)
endif()

# Add the "shared" executable (linked against the shared FLTK libs).
# Note 1: only the GUI version is built as "shared" executable.
# Note 2: For MSVC we need the special object library 'call_main'.

if(FLTK_BUILD_SHARED_LIBS)

  add_library(fluid-lib-shared OBJECT EXCLUDE_FROM_ALL)
  target_sources(fluid-lib-shared PRIVATE ${CPPFILES} ${HEADERFILES})
  target_include_directories(fluid-lib-shared PUBLIC .)
  if(MSVC)
    target_link_libraries(fluid-lib-shared PUBLIC fltk::fltk-shared)
  else()
    target_link_libraries(fluid-lib-shared PUBLIC fltk::images-shared)
  endif(MSVC)

  if(MSVC)
    make_target(fluid-shared TRUE "${MAIN_FILES}" "fluid-lib-shared;call_main" fluid-shared)
  else()
    make_target(fluid-shared TRUE "${MAIN_FILES}" fluid-lib-shared fluid-shared)
  endif()

  # experimental
  # if(NOT WIN32)
  #   set(FLTK_FLUID_EXECUTABLE fltk::fluid-shared)
  #   message(STATUS "** experimental ** FLTK_FLUID_EXECUTABLE = ${FLTK_FLUID_EXECUTABLE}")
  # endif()

endif(FLTK_BUILD_SHARED_LIBS)

# export the variable FLTK_FLUID_EXECUTABLE to the parent scope

set(FLTK_FLUID_EXECUTABLE "${FLTK_FLUID_EXECUTABLE}" PARENT_SCOPE)

# Create aliases for all targets

foreach(tgt ${TARGETS})
  add_executable(fltk::${tgt} ALIAS ${tgt})
endforeach()

# Install the GUI and (on Windows only) the commandline tool 'fluid-cmd'
# message(STATUS "Fluid: INSTALL TARGETS: ${TARGETS}")

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
