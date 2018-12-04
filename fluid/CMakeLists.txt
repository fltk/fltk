#
# "$Id$"
#
# CMakeLists.txt to build fluid for the FLTK project using CMake (www.cmake.org)
#
# Copyright 1998-2016 by Bill Spitzak and others.
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

if(APPLE AND NOT OPTION_APPLE_X11)
    set(FLUID_ICON "${CMAKE_CURRENT_SOURCE_DIR}/fluid.app/Contents/Resources/fluid.icns")
    add_executable(fluid MACOSX_BUNDLE ${CPPFILES} "${FLUID_ICON}")
    FLTK_SET_BUNDLE_ICON(fluid "${FLUID_ICON}")
else()
    add_executable(fluid WIN32 ${CPPFILES})
endif(APPLE AND NOT OPTION_APPLE_X11)

target_link_libraries(fluid fltk fltk_images fltk_forms)

# install fluid

install(TARGETS fluid
    EXPORT FLTK-Targets
    DESTINATION ${FLTK_BINDIR}
)

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
