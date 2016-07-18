#
# "$Id$"
#
# CMakeLists.txt to build fluid for the FLTK project using CMake (www.cmake.org)
#
# Copyright 1998-2015 by Bill Spitzak and others.
#
# This library is free software. Distribution and use rights are outlined in
# the file "COPYING" which should have been included with this file.  If this
# file is missing or damaged, see the license at:
#
#     http://www.fltk.org/COPYING.php
#
# Please report all bugs and problems on the following page:
#
#     http://www.fltk.org/str.php
#

set(CPPFILES
    CodeEditor.cxx
    Fl_Function_Type.cxx
    Fl_Group_Type.cxx
    Fl_Menu_Type.cxx
    Fl_Type.cxx
    ExternalCodeEditor_UNIX.cxx
    ExternalCodeEditor_WIN32.cxx
    Fl_Widget_Type.cxx
    Fl_Window_Type.cxx
    Fluid_Image.cxx
    about_panel.cxx
    align_widget.cxx
    alignment_panel.cxx
    code.cxx
    factory.cxx
    file.cxx
    fluid.cxx
    function_panel.cxx
    template_panel.cxx
    undo.cxx
    widget_panel.cxx
)

if(APPLE AND (NOT OPTION_APPLE_X11) AND (NOT OPTION_APPLE_SDL))
  set( ICON_NAME fluid.icns )
  set( ICON_PATH "${PROJECT_SOURCE_DIR}/fluid/Fluid.app/Contents/Resources/${ICON_NAME}" )
  add_executable(fluid MACOSX_BUNDLE ${CPPFILES} ${ICON_PATH})
else()
  add_executable(fluid ${CPPFILES})
endif(APPLE AND (NOT OPTION_APPLE_X11) AND (NOT OPTION_APPLE_SDL))

target_link_libraries(fluid fltk fltk_images fltk_forms)

# install fluid

if(APPLE AND (NOT OPTION_APPLE_X11) AND (NOT OPTION_APPLE_SDL))
  # create bundle
  set_target_properties(fluid PROPERTIES MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
  set_target_properties(fluid PROPERTIES RESOURCE ${ICON_PATH})
  # install
  install(TARGETS fluid DESTINATION ${FLTK_BINDIR})
else()
  install(TARGETS fluid
    EXPORT FLTK-Targets
    RUNTIME DESTINATION ${FLTK_BINDIR}
    LIBRARY DESTINATION ${FLTK_LIBDIR}
    ARCHIVE DESTINATION ${FLTK_LIBDIR}
    )
endif(APPLE AND (NOT OPTION_APPLE_X11) AND (NOT OPTION_APPLE_SDL))

# install desktop files

if(UNIX)
    install(FILES fluid.desktop
        DESTINATION ${FLTK_DATADIR}/applications
        )
    # Install mime-type file. x-fluid.desktop method is deprecated.
    install(FILES fluid.xml
        DESTINATION ${FLTK_DATADIR}/mime/packages
        )

    # Install desktop icons.
    foreach(icon 32 48 64 128)
        install(FILES icons/fluid-${icon}.png
            DESTINATION ${FLTK_DATADIR}/icons/hicolor/${icon}x${icon}/apps
            RENAME fluid.png
        )
    endforeach()
endif(UNIX)
