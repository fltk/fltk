#
# CMakeLists.txt used to build test and demo apps by the CMake build system
#
# Copyright 2004-2021 by Bill Spitzak and others.
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
#######################################################################

include (../CMake/FLTK-Functions.cmake)
include (../CMake/fl_create_example.cmake)

set (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/../bin/test)
set (TESTFILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/../data)

# create data and binary directory to copy scripts and data files

file (MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
file (MAKE_DIRECTORY ${TESTFILE_PATH})

#######################################################################
# audio libs for test apps
if (WIN32)
  set (AUDIOLIBS winmm)
elseif (APPLE)
  set (AUDIOLIBS "-framework CoreAudio")
elseif (HAVE_ALSA_ASOUNDLIB_H)
  find_library(LIB_asound asound)
  if (LIB_asound)
    set (AUDIOLIBS ${LIB_asound})
  endif (LIB_asound)
  mark_as_advanced (LIB_asound)
endif (WIN32)

#######################################################################
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

#######################################################################

CREATE_EXAMPLE (adjuster adjuster.cxx fltk)
CREATE_EXAMPLE (arc arc.cxx fltk)
CREATE_EXAMPLE (animated animated.cxx fltk)
CREATE_EXAMPLE (ask ask.cxx fltk)
CREATE_EXAMPLE (bitmap bitmap.cxx fltk)
CREATE_EXAMPLE (blocks "blocks.cxx;blocks.plist;blocks.icns" "fltk;${AUDIOLIBS}")
CREATE_EXAMPLE (boxtype boxtype.cxx fltk)
CREATE_EXAMPLE (browser browser.cxx fltk)
CREATE_EXAMPLE (button button.cxx fltk)
CREATE_EXAMPLE (buttons buttons.cxx fltk)
CREATE_EXAMPLE (checkers "checkers.cxx;checkers.plist;checkers.icns" fltk)
CREATE_EXAMPLE (clock clock.cxx fltk)
CREATE_EXAMPLE (colbrowser colbrowser.cxx fltk)
CREATE_EXAMPLE (color_chooser color_chooser.cxx fltk)
CREATE_EXAMPLE (cursor cursor.cxx fltk)
CREATE_EXAMPLE (curve curve.cxx fltk)
CREATE_EXAMPLE (demo demo.cxx fltk)
CREATE_EXAMPLE (device device.cxx fltk)
CREATE_EXAMPLE (doublebuffer doublebuffer.cxx fltk)
CREATE_EXAMPLE (editor "editor.cxx;editor.plist" fltk)
CREATE_EXAMPLE (fast_slow fast_slow.fl fltk)
CREATE_EXAMPLE (file_chooser file_chooser.cxx "fltk_images;fltk")
CREATE_EXAMPLE (fltk-versions fltk-versions.cxx fltk)
CREATE_EXAMPLE (fonts fonts.cxx fltk)
CREATE_EXAMPLE (forms forms.cxx "fltk_forms;fltk")
CREATE_EXAMPLE (hello hello.cxx fltk)
CREATE_EXAMPLE (help_dialog help.cxx "fltk_images;fltk")
CREATE_EXAMPLE (icon icon.cxx fltk)
CREATE_EXAMPLE (iconize iconize.cxx fltk)
CREATE_EXAMPLE (image image.cxx fltk)
CREATE_EXAMPLE (inactive inactive.fl fltk)
CREATE_EXAMPLE (input input.cxx fltk)
CREATE_EXAMPLE (input_choice input_choice.cxx fltk)
CREATE_EXAMPLE (keyboard "keyboard.cxx;keyboard_ui.fl" fltk)
CREATE_EXAMPLE (label label.cxx fltk)
CREATE_EXAMPLE (line_style line_style.cxx fltk)
CREATE_EXAMPLE (list_visuals list_visuals.cxx fltk)
CREATE_EXAMPLE (mandelbrot "mandelbrot_ui.fl;mandelbrot.cxx" fltk)
CREATE_EXAMPLE (menubar menubar.cxx fltk)
CREATE_EXAMPLE (message message.cxx fltk)
CREATE_EXAMPLE (minimum minimum.cxx fltk)
CREATE_EXAMPLE (native-filechooser native-filechooser.cxx "fltk_images;fltk")
CREATE_EXAMPLE (navigation navigation.cxx fltk)
CREATE_EXAMPLE (output output.cxx fltk)
CREATE_EXAMPLE (overlay overlay.cxx fltk)
CREATE_EXAMPLE (pack pack.cxx fltk)
CREATE_EXAMPLE (pixmap pixmap.cxx fltk)
CREATE_EXAMPLE (pixmap_browser pixmap_browser.cxx "fltk_images;fltk")
CREATE_EXAMPLE (preferences preferences.fl fltk)
CREATE_EXAMPLE (offscreen offscreen.cxx fltk)
CREATE_EXAMPLE (radio radio.fl fltk)
CREATE_EXAMPLE (resize resize.fl fltk)
CREATE_EXAMPLE (resizebox resizebox.cxx fltk)
CREATE_EXAMPLE (rotated_text rotated_text.cxx fltk)
CREATE_EXAMPLE (scroll scroll.cxx fltk)
CREATE_EXAMPLE (subwindow subwindow.cxx fltk)
CREATE_EXAMPLE (sudoku "sudoku.cxx;sudoku.plist;sudoku.icns;sudoku.rc" "fltk_images;fltk;${AUDIOLIBS}")
CREATE_EXAMPLE (symbols symbols.cxx fltk)
CREATE_EXAMPLE (tabs tabs.fl fltk)
CREATE_EXAMPLE (table table.cxx fltk)
CREATE_EXAMPLE (threads threads.cxx fltk)
CREATE_EXAMPLE (tile tile.cxx fltk)
CREATE_EXAMPLE (tiled_image tiled_image.cxx fltk)
CREATE_EXAMPLE (tree tree.fl fltk)
CREATE_EXAMPLE (twowin twowin.cxx fltk)
CREATE_EXAMPLE (utf8 utf8.cxx fltk)
CREATE_EXAMPLE (valuators valuators.fl fltk)
CREATE_EXAMPLE (unittests unittests.cxx fltk)
CREATE_EXAMPLE (windowfocus windowfocus.cxx fltk)

# OpenGL demos...
if (OPENGL_FOUND)
  CREATE_EXAMPLE (CubeView "CubeMain.cxx;CubeView.cxx;CubeViewUI.fl" "fltk_gl;fltk")
  CREATE_EXAMPLE (cube cube.cxx "fltk_gl;fltk;${OPENGL_LIBRARIES}")
  CREATE_EXAMPLE (fractals "fractals.cxx;fracviewer.cxx" "fltk_gl;fltk")
  CREATE_EXAMPLE (fullscreen fullscreen.cxx "fltk_gl;fltk")
  CREATE_EXAMPLE (glpuzzle glpuzzle.cxx "fltk_gl;fltk;${OPENGL_LIBRARIES}")
  CREATE_EXAMPLE (gl_overlay gl_overlay.cxx "fltk_gl;fltk;${OPENGL_LIBRARIES}")
  CREATE_EXAMPLE (shape shape.cxx "fltk_gl;fltk;${OPENGL_LIBRARIES}")
endif (OPENGL_FOUND)

# Cairo demo - must also be built w/o Cairo (displays a message box)
if (FLTK_HAVE_CAIRO)
  CREATE_EXAMPLE (cairo_test cairo_test.cxx "fltk_cairo;fltk")
else ()
  CREATE_EXAMPLE (cairo_test cairo_test.cxx fltk)
endif (FLTK_HAVE_CAIRO)

#####################################################
# We need some support files for the demo programs
#####################################################

# copy the required test files

file (COPY
  rgb.txt browser.cxx editor.cxx valuators.fl
  help_dialog.html
  DESTINATION ${TESTFILE_PATH}
)

# copy and modify test/demo.menu (replace "help" with "help_dialog")

file (READ demo.menu menu_in)
string (REPLACE "HelpDialog:help " "HelpDialog:help_dialog " menu_out ${menu_in})
file (WRITE ${TESTFILE_PATH}/demo.menu ${menu_out})

# copy the image directory for help_dialog.html

file (COPY
  ${CMAKE_CURRENT_SOURCE_DIR}/images
  DESTINATION ${TESTFILE_PATH}
)

# The main test program 'demo' needs additional hints and configurations.
# Note: CMake defines "CMAKE_INTDIR" which is the build type folder (e.g. "Debug")
# for multi config builds (MSVC, Xcode)

target_compile_definitions (demo PRIVATE GENERATED_BY_CMAKE)
