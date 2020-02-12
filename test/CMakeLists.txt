#
# "$Id$"
#
# CMakeLists.txt used to build test and demo apps by the CMake build system
#
# Copyright 2004-2019 by Bill Spitzak and others.
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
#######################################################################

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/../bin/examples)

#######################################################################
# audio libs for test apps
if(WIN32)
  set(AUDIOLIBS winmm)
elseif(APPLE)
  set(AUDIOLIBS "-framework CoreAudio")
elseif(HAVE_ALSA_ASOUNDLIB_H)
  find_library(LIB_asound asound)
  if(LIB_asound)
    set(AUDIOLIBS ${LIB_asound})
  endif(LIB_asound)
endif(WIN32)

#######################################################################
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

#######################################################################
# Add the ANDROID_OK option if the example can be compiled for Android
# as well as for other platforms.
if(NOT ANDROID)
CREATE_EXAMPLE(adjuster adjuster.cxx fltk ANDROID_OK)
CREATE_EXAMPLE(arc arc.cxx fltk ANDROID_OK)
CREATE_EXAMPLE(animated animated.cxx fltk ANDROID_OK)
CREATE_EXAMPLE(ask ask.cxx fltk ANDROID_OK)
CREATE_EXAMPLE(bitmap bitmap.cxx fltk ANDROID_OK)
CREATE_EXAMPLE(blocks blocks.cxx "fltk;${AUDIOLIBS}")
CREATE_EXAMPLE(boxtype boxtype.cxx fltk ANDROID_OK)
CREATE_EXAMPLE(browser browser.cxx fltk ANDROID_OK)
CREATE_EXAMPLE(button button.cxx fltk ANDROID_OK)
CREATE_EXAMPLE(buttons buttons.cxx fltk ANDROID_OK)
CREATE_EXAMPLE(checkers "checkers.cxx;checkers_pieces.fl" "fltk;fltk_images" ANDROID_OK)
CREATE_EXAMPLE(clock clock.cxx fltk ANDROID_OK)
CREATE_EXAMPLE(colbrowser colbrowser.cxx "fltk;fltk_forms")
CREATE_EXAMPLE(color_chooser color_chooser.cxx fltk ANDROID_OK)
CREATE_EXAMPLE(cursor cursor.cxx fltk ANDROID_OK)
CREATE_EXAMPLE(curve curve.cxx fltk ANDROID_OK)
CREATE_EXAMPLE(demo demo.cxx fltk)
CREATE_EXAMPLE(device device.cxx fltk)
CREATE_EXAMPLE(doublebuffer doublebuffer.cxx fltk ANDROID_OK)
CREATE_EXAMPLE(editor editor.cxx fltk ANDROID_OK)
CREATE_EXAMPLE(fast_slow fast_slow.fl fltk ANDROID_OK)
CREATE_EXAMPLE(file_chooser file_chooser.cxx "fltk;fltk_images")
CREATE_EXAMPLE(fonts fonts.cxx fltk)
CREATE_EXAMPLE(forms forms.cxx "fltk;fltk_forms")
CREATE_EXAMPLE(hello hello.cxx fltk)
CREATE_EXAMPLE(help_dialog help_dialog.cxx "fltk;fltk_images")
CREATE_EXAMPLE(icon icon.cxx fltk)
CREATE_EXAMPLE(iconize iconize.cxx fltk)
CREATE_EXAMPLE(image image.cxx fltk)
CREATE_EXAMPLE(inactive inactive.fl fltk)
CREATE_EXAMPLE(input input.cxx fltk)
CREATE_EXAMPLE(input_choice input_choice.cxx fltk)
CREATE_EXAMPLE(keyboard "keyboard.cxx;keyboard_ui.fl" fltk)
CREATE_EXAMPLE(label label.cxx "fltk;fltk_forms")
CREATE_EXAMPLE(line_style line_style.cxx fltk)
CREATE_EXAMPLE(list_visuals list_visuals.cxx fltk)
CREATE_EXAMPLE(mandelbrot "mandelbrot_ui.fl;mandelbrot.cxx" fltk)
CREATE_EXAMPLE(menubar menubar.cxx fltk)
CREATE_EXAMPLE(message message.cxx fltk)
CREATE_EXAMPLE(minimum minimum.cxx fltk)
CREATE_EXAMPLE(native-filechooser native-filechooser.cxx "fltk;fltk_images")
CREATE_EXAMPLE(navigation navigation.cxx fltk)
CREATE_EXAMPLE(output output.cxx "fltk;fltk_forms")
CREATE_EXAMPLE(overlay overlay.cxx fltk)
CREATE_EXAMPLE(pack pack.cxx fltk)
CREATE_EXAMPLE(pixmap pixmap.cxx fltk)
CREATE_EXAMPLE(pixmap_browser pixmap_browser.cxx "fltk;fltk_images")
CREATE_EXAMPLE(preferences preferences.fl fltk)
CREATE_EXAMPLE(offscreen offscreen.cxx fltk)
CREATE_EXAMPLE(radio radio.fl fltk)
CREATE_EXAMPLE(resize resize.fl fltk)
CREATE_EXAMPLE(resizebox resizebox.cxx fltk)
CREATE_EXAMPLE(rotated_text rotated_text.cxx fltk)
CREATE_EXAMPLE(scroll scroll.cxx fltk)
CREATE_EXAMPLE(subwindow subwindow.cxx fltk)
CREATE_EXAMPLE(sudoku sudoku.cxx "fltk;fltk_images;${AUDIOLIBS}")
CREATE_EXAMPLE(symbols symbols.cxx fltk)
CREATE_EXAMPLE(tabs tabs.fl fltk)
CREATE_EXAMPLE(table table.cxx fltk)
CREATE_EXAMPLE(threads threads.cxx fltk)
CREATE_EXAMPLE(tile tile.cxx fltk)
CREATE_EXAMPLE(tiled_image tiled_image.cxx fltk)
CREATE_EXAMPLE(tree tree.fl fltk)
CREATE_EXAMPLE(twowin twowin.cxx fltk)
CREATE_EXAMPLE(utf8 utf8.cxx fltk)
CREATE_EXAMPLE(valuators valuators.fl fltk)
CREATE_EXAMPLE(unittests unittests.cxx fltk)
CREATE_EXAMPLE(windowfocus windowfocus.cxx fltk)

CREATE_EXAMPLE(fltk-versions ../examples/fltk-versions.cxx fltk)

# OpenGL demos...
if(OPENGL_FOUND)
CREATE_EXAMPLE(CubeView "CubeMain.cxx;CubeView.cxx;CubeViewUI.fl" "fltk;fltk_gl")
CREATE_EXAMPLE(cube cube.cxx "fltk;fltk_gl;${OPENGL_LIBRARIES}")
CREATE_EXAMPLE(fractals "fractals.cxx;fracviewer.cxx" "fltk;fltk_gl")
CREATE_EXAMPLE(fullscreen fullscreen.cxx "fltk;fltk_gl")
CREATE_EXAMPLE(glpuzzle glpuzzle.cxx "fltk;fltk_gl;${OPENGL_LIBRARIES}")
CREATE_EXAMPLE(gl_overlay gl_overlay.cxx "fltk;fltk_gl;${OPENGL_LIBRARIES}")
CREATE_EXAMPLE(shape shape.cxx "fltk;fltk_gl;${OPENGL_LIBRARIES}")
endif(OPENGL_FOUND)

# Cairo demo - must also be built w/o Cairo (displays a message box)
if (FLTK_HAVE_CAIRO)
  CREATE_EXAMPLE(cairo_test cairo_test.cxx "fltk;fltk_cairo")
else ()
  CREATE_EXAMPLE(cairo_test cairo_test.cxx fltk)
endif (FLTK_HAVE_CAIRO)

endif(NOT ANDROID)

# We need some support files for the demo programs:

# Note: this is incomplete as of 11 Feb 2015
# Todo: currently all files are copied, but some of them need configuration:
# - demo.menu: fluid can't be started (wrong path)
# - demo.menu: help_dialog (help_dialog.html) can't find its images (not copied)
# - maybe more ...

# prepare for a "better" test file installation path
set (TESTFILE_PATH ${EXECUTABLE_OUTPUT_PATH})

# *FIXME* *DEBUG*
# message ("test/CMakeLists.txt: EXECUTABLE_OUTPUT_PATH = '${EXECUTABLE_OUTPUT_PATH}'")
# message ("test/CMakeLists.txt: TESTFILE_PATH          = '${TESTFILE_PATH}'")

# use a target filename to make sure the target directory gets created
configure_file(demo.menu ${TESTFILE_PATH}/demo.menu COPYONLY)

# use target directory only to avoid redundancy
configure_file(rgb.txt ${TESTFILE_PATH} COPYONLY)
configure_file(help_dialog.html ${TESTFILE_PATH} COPYONLY)
configure_file(browser.cxx ${TESTFILE_PATH} COPYONLY)
configure_file(editor.cxx ${TESTFILE_PATH} COPYONLY)

# Apple macOS creates bundles instead of executables and needs a little bit
# more help for demos to run correctly
if(APPLE AND NOT OPTION_APPLE_X11)

    # make the menu structure part of the app
    target_sources(demo PRIVATE demo.menu)
    set_target_properties(demo PROPERTIES MACOSX_BUNDLE TRUE RESOURCE demo.menu )

    # add a sample RGB file that otherwise only exists under X11
    target_sources(colbrowser PRIVATE rgb.txt)
    set_target_properties(colbrowser PROPERTIES MACOSX_BUNDLE TRUE RESOURCE rgb.txt )

    # help_dialog displays an html file as an example
    target_sources(help_dialog PRIVATE help_dialog.html)
    set_target_properties(help_dialog PROPERTIES MACOSX_BUNDLE TRUE RESOURCE help_dialog.html )

endif(APPLE AND NOT OPTION_APPLE_X11)
