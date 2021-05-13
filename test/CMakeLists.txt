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
# Add the ANDROID_OK option if the example can be compiled for Android
# as well as for other platforms.

if (NOT ANDROID)

CREATE_EXAMPLE (adjuster adjuster.cxx fltk ANDROID_OK)
CREATE_EXAMPLE (arc arc.cxx fltk ANDROID_OK)
CREATE_EXAMPLE (animated animated.cxx fltk ANDROID_OK)
CREATE_EXAMPLE (ask ask.cxx fltk ANDROID_OK)
CREATE_EXAMPLE (bitmap bitmap.cxx fltk ANDROID_OK)
CREATE_EXAMPLE (blocks "blocks.cxx;blocks.plist;blocks.icns" "fltk;${AUDIOLIBS}")
CREATE_EXAMPLE (boxtype boxtype.cxx fltk ANDROID_OK)
CREATE_EXAMPLE (browser browser.cxx fltk ANDROID_OK)
CREATE_EXAMPLE (button button.cxx fltk ANDROID_OK)
CREATE_EXAMPLE (buttons buttons.cxx fltk ANDROID_OK)
CREATE_EXAMPLE (checkers "checkers.cxx;checkers_pieces.fl;checkers.icns" "fltk_images;fltk" ANDROID_OK)
CREATE_EXAMPLE (clock clock.cxx fltk ANDROID_OK)
CREATE_EXAMPLE (colbrowser colbrowser.cxx fltk)
CREATE_EXAMPLE (color_chooser color_chooser.cxx fltk ANDROID_OK)
CREATE_EXAMPLE (cursor cursor.cxx fltk ANDROID_OK)
CREATE_EXAMPLE (curve curve.cxx fltk ANDROID_OK)
CREATE_EXAMPLE (demo demo.cxx fltk)
CREATE_EXAMPLE (device device.cxx "fltk_images;fltk")
CREATE_EXAMPLE (doublebuffer doublebuffer.cxx fltk ANDROID_OK)
CREATE_EXAMPLE (editor "editor.cxx;editor.plist" fltk ANDROID_OK)
CREATE_EXAMPLE (fast_slow fast_slow.fl fltk ANDROID_OK)
CREATE_EXAMPLE (file_chooser file_chooser.cxx "fltk_images;fltk")
CREATE_EXAMPLE (fltk-versions fltk-versions.cxx fltk)
CREATE_EXAMPLE (fonts fonts.cxx fltk)
CREATE_EXAMPLE (forms forms.cxx "fltk_forms;fltk")
if (OPENGL_FOUND)
  CREATE_EXAMPLE (handle_events handle_events.cxx "fltk_gl;fltk") # opt. Fl_Gl_Window
else()
  CREATE_EXAMPLE (handle_events handle_events.cxx fltk) # w/o Fl_Gl_Window
endif()
CREATE_EXAMPLE (hello hello.cxx fltk)
CREATE_EXAMPLE (help_dialog help_dialog.cxx "fltk_images;fltk")
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
CREATE_EXAMPLE (resize-example1 "resize-example1.cxx;resize-arrows.cxx" fltk)
CREATE_EXAMPLE (resize-example2 "resize-example2.cxx;resize-arrows.cxx" fltk)
CREATE_EXAMPLE (resize-example3a "resize-example3a.cxx;resize-arrows.cxx" fltk)
CREATE_EXAMPLE (resize-example3b "resize-example3b.cxx;resize-arrows.cxx" fltk)
CREATE_EXAMPLE (resize-example3c "resize-example3c.cxx;resize-arrows.cxx" fltk)
CREATE_EXAMPLE (resize-example4a "resize-example4a.cxx;resize-arrows.cxx" fltk)
CREATE_EXAMPLE (resize-example4b "resize-example4b.cxx;resize-arrows.cxx" fltk)
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

# /EXPERIMENTAL/ Build some of the demo programs linked against the shared lib(s)

if (OPTION_BUILD_SHARED_LIBS)

  if (MSVC) # MS Visual C++ aka. Visual Studio

    # We need to link with fl_call_main.c, but this must not be compiled with
    # macro FL_DLL, whereas the other source file(s) must be compiled with FL_DLL
    # to link against the shared libs.
    #
    #  ;-)  I *love* Visual Studio (MSVC)  ;-)
    #
    # Trick: build an object library with just this one file and link the executable
    # against the shared library plus this object library. Another way would be to
    # add the extra source file fl_call_main.c to the source files of all targets
    # and set the FL_DLL compile_definition property only on the main *source* files
    # but this doesn't work since the same file must be compiled for the "static"
    # and the "shared" target, for instance hello.cxx for "hello" and "hello-shared".

    add_library (CALL_MAIN OBJECT EXCLUDE_FROM_ALL ../src/fl_call_main.c)

    # define a list of shared targets so we can set COMPILE_DEFINITIONS in a loop
    set (SHARED_TARGETS)

    CREATE_EXAMPLE (hello-shared hello.cxx "fltk_SHARED;CALL_MAIN")
    CREATE_EXAMPLE (pixmap_browser-shared pixmap_browser.cxx "fltk_SHARED;CALL_MAIN")
    CREATE_EXAMPLE (unittests-shared unittests.cxx "fltk_SHARED;CALL_MAIN")

    list (APPEND SHARED_TARGETS hello pixmap_browser unittests)

    if (OPENGL_FOUND)
      CREATE_EXAMPLE (glpuzzle-shared glpuzzle.cxx "fltk_SHARED;CALL_MAIN;${OPENGL_LIBRARIES}")
      CREATE_EXAMPLE (shape-shared shape.cxx "fltk_SHARED;CALL_MAIN;${OPENGL_LIBRARIES}")

      list (APPEND SHARED_TARGETS glpuzzle shape)

    endif (OPENGL_FOUND)

    # apply property COMPILE_DEFINITIONS in a loop for all necessary source files
    foreach (tgt ${SHARED_TARGETS})
      target_compile_definitions (${tgt}-shared PRIVATE "FL_DLL")
    endforeach ()

  else () # not MSVC

    CREATE_EXAMPLE (hello-shared hello.cxx fltk_SHARED)
    CREATE_EXAMPLE (pixmap_browser-shared pixmap_browser.cxx "fltk_images_SHARED;fltk_SHARED")
    CREATE_EXAMPLE (unittests-shared unittests.cxx fltk_SHARED)

    if (OPENGL_FOUND)
      CREATE_EXAMPLE (glpuzzle-shared glpuzzle.cxx "fltk_gl_SHARED;fltk_SHARED;${OPENGL_LIBRARIES}")
      CREATE_EXAMPLE (shape-shared shape.cxx "fltk_gl_SHARED;fltk_SHARED;${OPENGL_LIBRARIES}")
    endif (OPENGL_FOUND)

  endif (MSVC) # (not MSVC)

endif (OPTION_BUILD_SHARED_LIBS)

endif (NOT ANDROID)

#####################################################
# We need some support files for the demo programs
#####################################################

# copy the required test files

file (COPY
  demo.menu rgb.txt browser.cxx editor.cxx valuators.fl
  help_dialog.html
  DESTINATION ${TESTFILE_PATH}
)

# copy the image directory for help_dialog.html

file (COPY
  ${CMAKE_CURRENT_SOURCE_DIR}/images
  DESTINATION ${TESTFILE_PATH}
)

# The main test program 'demo' needs additional hints and configurations.
# Note: CMake defines "CMAKE_INTDIR" which is the build type folder (e.g. "Debug")
# for multi config builds (MSVC, Xcode)

target_compile_definitions (demo PRIVATE GENERATED_BY_CMAKE)
