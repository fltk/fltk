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

add_executable(fluid ${CPPFILES})
target_link_libraries(fluid fltk fltk_images fltk_forms)

# link in optional libraries
if(USE_XFT)
   target_link_libraries(fluid ${X11_Xft_LIB})
endif(USE_XFT)

if(HAVE_XINERAMA)
   target_link_libraries(fluid ${X11_Xinerama_LIB})
endif(HAVE_XINERAMA)

install(TARGETS fluid
    EXPORT FLTK-Targets
    RUNTIME DESTINATION ${FLTK_BINDIR}
    LIBRARY DESTINATION ${FLTK_LIBDIR}
    ARCHIVE DESTINATION ${FLTK_LIBDIR}
    )

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
