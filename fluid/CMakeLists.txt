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

set(MAIN_FILES fluid.cxx fluid.h)

# Source files for 'fluid-lib': all source files except ${MAIN_FILES}

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
  settings_panel.cxx
  autodoc.cxx
  code.cxx
  custom_widgets.cxx
  factory.cxx
  file.cxx
  fluid_filename.cxx
  function_panel.cxx
  mergeback.cxx
  pixmaps.cxx
  shell_command.cxx
  codeview_panel.cxx
  template_panel.cxx
  undo.cxx
  widget_browser.cxx
  widget_panel.cxx
  ExternalCodeEditor_${SUFFIX}.cxx
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
  settings_panel.h
  autodoc.h
  code.h
  comments.h
  custom_widgets.h
  factory.h
  file.h
  function_panel.h
  mergeback.h
  print_panel.h
  pixmaps.h
  shell_command.h
  codeview_panel.h
  template_panel.h
  undo.h
  widget_browser.h
  widget_panel.h
  ExternalCodeEditor_${SUFFIX}.h
)

source_group("Header Files" FILES ${HEADERFILES})

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
