#
# CMakeLists.txt used to build test and demo apps by the CMake build system
#
# Copyright 2004-2024 by Bill Spitzak and others.
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

include(../CMake/FLTK-Functions.cmake)
include(../CMake/fl_create_example.cmake)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/../bin/test)
set(TESTFILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/../data)

# create data and binary directory to copy scripts and data files

file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
file(MAKE_DIRECTORY ${TESTFILE_PATH})

#######################################################################
# Define additional example programs for testing, for instance:
#   set(extra_tests issue-276 str-1895)
# Use the source file test/'name'.cxx for each additional program.
# These test programs will be built with image and GL libraries.
# Leave the variable 'extra_tests' empty to disable extra test programs.

set(extra_tests)

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
  mark_as_advanced (LIB_asound)
endif(WIN32)

#######################################################################
# Define standard libraries for those OpenGL demos that can be built
# w/o OpenGL as well. These demos show an alternate GUI or a warning
# when built w/o OpenGL.
#
# Note: use "${GLDEMO_LIBS}" for their library parameter
#        or "${GLDEMO_SHARED}" to build with shared FLTK libraries

if(OPENGL_FOUND)
  set(GLDEMO_LIBS fltk::gl)
  if(MSVC)
    set(GLDEMO_SHARED fltk::fltk-shared)
  else()
    set(GLDEMO_SHARED fltk::gl-shared)
  endif()
else()
  set(GLDEMO_LIBS fltk::fltk)
  set(GLDEMO_SHARED fltk::fltk-shared)
endif()

# Similar approach for the 'forms' demo (shared lib not supported)

if(FLTK_BUILD_FORMS)
  set(FORMS_LIBS fltk::forms)
else()
  set(FORMS_LIBS fltk::fltk)
endif()

#######################################################################

fl_create_example(adjuster adjuster.cxx fltk::fltk)
fl_create_example(arc arc.cxx fltk::fltk)
fl_create_example(animated animated.cxx fltk::fltk)
fl_create_example(ask ask.cxx fltk::fltk)
fl_create_example(bitmap bitmap.cxx fltk::fltk)
fl_create_example(blocks "blocks.cxx;blocks.plist;blocks.icns" "fltk::fltk;${AUDIOLIBS}")
fl_create_example(boxtype boxtype.cxx fltk::fltk)
fl_create_example(browser browser.cxx fltk::fltk)
fl_create_example(button button.cxx fltk::fltk)
fl_create_example(buttons buttons.cxx fltk::fltk)
# Cairo demo, built with and w/o Cairo (libcairo is linked implicitly - or not at all)
fl_create_example(cairo_test cairo_test.cxx fltk::fltk)
fl_create_example(checkers "checkers.cxx;checkers_pieces.fl;checkers.icns" fltk::images)
fl_create_example(clipboard clipboard.cxx fltk::images)
fl_create_example(clock clock.cxx fltk::fltk)
fl_create_example(colbrowser colbrowser.cxx fltk::fltk)
fl_create_example(color_chooser color_chooser.cxx fltk::fltk)
fl_create_example(contrast contrast.cxx fltk::fltk)
fl_create_example(coordinates coordinates.cxx fltk::fltk)
fl_create_example(cursor cursor.cxx fltk::fltk)
fl_create_example(curve curve.cxx fltk::fltk)
fl_create_example(demo demo.cxx fltk::fltk)
fl_create_example(device device.cxx fltk::images)
fl_create_example(doublebuffer doublebuffer.cxx fltk::fltk)
fl_create_example(editor "editor.cxx;editor.plist" fltk::fltk)
fl_create_example(fast_slow fast_slow.fl fltk::fltk)
fl_create_example(file_chooser file_chooser.cxx fltk::images)
fl_create_example(flex_demo flex_demo.cxx fltk::fltk)
fl_create_example(flex_login flex_login.cxx fltk::fltk)
fl_create_example(fltk-versions fltk-versions.cxx fltk::fltk)
fl_create_example(fonts fonts.cxx fltk::fltk)
fl_create_example(forms forms.cxx "${FORMS_LIBS}")
fl_create_example(fullscreen fullscreen.cxx "${GLDEMO_LIBS}")
fl_create_example(grid_alignment grid_alignment.cxx fltk::fltk)
fl_create_example(grid_buttons grid_buttons.cxx fltk::fltk)
fl_create_example(grid_dialog grid_dialog.cxx fltk::fltk)
fl_create_example(grid_login grid_login.cxx fltk::fltk)
fl_create_example(handle_events handle_events.cxx "${GLDEMO_LIBS}")
fl_create_example(handle_keys handle_keys.cxx fltk::fltk)
fl_create_example(hello hello.cxx fltk::fltk)
fl_create_example(help_dialog help_dialog.cxx fltk::images)
fl_create_example(icon icon.cxx fltk::fltk)
fl_create_example(iconize iconize.cxx fltk::fltk)
fl_create_example(image image.cxx fltk::fltk)
fl_create_example(inactive inactive.fl fltk::fltk)
fl_create_example(input input.cxx fltk::fltk)
fl_create_example(input_choice input_choice.cxx fltk::fltk)
fl_create_example(keyboard "keyboard.cxx;keyboard_ui.fl" fltk::fltk)
fl_create_example(label label.cxx fltk::fltk)
fl_create_example(line_style line_style.cxx fltk::fltk)
fl_create_example(line_style_docs line_style_docs.cxx fltk::fltk)
fl_create_example(list_visuals list_visuals.cxx fltk::fltk)
fl_create_example(mandelbrot "mandelbrot_ui.fl;mandelbrot.cxx" fltk::fltk)
fl_create_example(menubar menubar.cxx fltk::fltk)
fl_create_example(message message.cxx fltk::fltk)
fl_create_example(minimum minimum.cxx fltk::fltk)
fl_create_example(native-filechooser native-filechooser.cxx fltk::images)
fl_create_example(navigation navigation.cxx fltk::fltk)
fl_create_example(output output.cxx fltk::fltk)
fl_create_example(overlay overlay.cxx fltk::fltk)
fl_create_example(pack pack.cxx fltk::fltk)
fl_create_example(pixmap pixmap.cxx fltk::images)
fl_create_example(pixmap_browser pixmap_browser.cxx fltk::images)
fl_create_example(preferences preferences.fl fltk::fltk)
fl_create_example(offscreen offscreen.cxx fltk::fltk)
fl_create_example(radio radio.fl fltk::fltk)
fl_create_example(resize resize.fl fltk::fltk)
fl_create_example(resizebox resizebox.cxx fltk::fltk)
fl_create_example(resize-example1 "resize-example1.cxx;resize-arrows.cxx" fltk::fltk)
fl_create_example(resize-example2 "resize-example2.cxx;resize-arrows.cxx" fltk::fltk)
fl_create_example(resize-example3a "resize-example3a.cxx;resize-arrows.cxx" fltk::fltk)
fl_create_example(resize-example3b "resize-example3b.cxx;resize-arrows.cxx" fltk::fltk)
fl_create_example(resize-example3c "resize-example3c.cxx;resize-arrows.cxx" fltk::fltk)
fl_create_example(resize-example4a "resize-example4a.cxx;resize-arrows.cxx" fltk::fltk)
fl_create_example(resize-example4b "resize-example4b.cxx;resize-arrows.cxx" fltk::fltk)
fl_create_example(resize-example5a "resize-example5a.cxx;resize-arrows.cxx" fltk::fltk)
fl_create_example(resize-example5b "resize-example5b.cxx;resize-arrows.cxx" fltk::fltk)
fl_create_example(resize-example5c "resize-example5c.cxx;resize-arrows.cxx" fltk::fltk)
fl_create_example(rotated_text rotated_text.cxx fltk::fltk)
fl_create_example(scroll scroll.cxx fltk::fltk)
fl_create_example(subwindow subwindow.cxx fltk::fltk)
fl_create_example(sudoku "sudoku.cxx;sudoku.plist;sudoku.icns;sudoku.rc" "fltk::images;${AUDIOLIBS}")
fl_create_example(symbols symbols.cxx fltk::fltk)
fl_create_example(tabs tabs.fl fltk::fltk)
fl_create_example(table table.cxx fltk::fltk)
fl_create_example(terminal terminal.fl fltk::fltk)
fl_create_example(threads threads.cxx fltk::fltk)
fl_create_example(tile tile.cxx fltk::fltk)
fl_create_example(tiled_image tiled_image.cxx fltk::fltk)
fl_create_example(tree tree.fl fltk::fltk)
fl_create_example(twowin twowin.cxx fltk::fltk)
fl_create_example(utf8 utf8.cxx fltk::fltk)
fl_create_example(valuators valuators.fl fltk::fltk)
fl_create_example(windowfocus windowfocus.cxx fltk::fltk)
fl_create_example(wizard wizard.cxx fltk::fltk)

# unittests uses multiple source files and can be built with or w/o OpenGL and "shared"

SET (UNITTEST_SRCS
  unittests.cxx
  unittests.h
  unittest_about.cxx
  unittest_points.cxx
  unittest_core.cxx
  unittest_complex_shapes.cxx
  unittest_fast_shapes.cxx
  unittest_circles.cxx
  unittest_text.cxx
  unittest_unicode.cxx
  unittest_symbol.cxx
  unittest_images.cxx
  unittest_viewport.cxx
  unittest_scrollbarsize.cxx
  unittest_schemes.cxx
  unittest_terminal.cxx
)
fl_create_example(unittests "${UNITTEST_SRCS}" "${GLDEMO_LIBS}")

# Create additional test programs (used by developers for testing)

if(extra_tests)
  foreach(name ${extra_tests})
    fl_create_example(${name} ${name}.cxx "fltk::images;${GLDEMO_LIBS}")
  endforeach()
endif()

# OpenGL demos. These programs can't be built w/o OpenGL

if(OPENGL_FOUND)
  fl_create_example(CubeView "CubeMain.cxx;CubeView.cxx;CubeViewUI.fl" "${GLDEMO_LIBS}")
  fl_create_example(cube cube.cxx "${GLDEMO_LIBS}")
  fl_create_example(fractals "fractals.cxx;fracviewer.cxx" "${GLDEMO_LIBS}")
  fl_create_example(glut_test glut_test.cxx "${GLDEMO_LIBS}")
  fl_create_example(glpuzzle glpuzzle.cxx "${GLDEMO_LIBS}")
  fl_create_example(gl_overlay gl_overlay.cxx "${GLDEMO_LIBS}")
  fl_create_example(shape shape.cxx "${GLDEMO_LIBS}")
endif(OPENGL_FOUND)

# Test FLTK headers for variable shadowing [-Wshadow] (STR 2728)
#
# The following test program is added only for GNU or Clang compilers which
# are known to support the '-Wshadow' compiler flag. We could also implement
# a compile test but using GNU and Clang appears to be sufficient.
#
# Note 1: usage of `file(GLOB ...)` is discouraged by CMake documentation because
#   it doesn't trigger rebuilding when new header files are added. However, this
#   is better than nothing.
#
# Note 2: a better way to generate the header file might be with a custom command
#   to be able to generate proper dependencies but this is left for later...
#
# Note 3: this test program is only included in CMake builds and was inspired
#   by STR 2728: "Add test program to test for shadowed variables (-Wshadow)".
#   Its sole purpose is to issue compilation warnings during build time if
#   variables are shadowed in public headers.

set(_compilers GNU Clang AppleClang)
if(CMAKE_CXX_COMPILER_ID IN_LIST _compilers)

  set(include_all "${CMAKE_CURRENT_BINARY_DIR}/include_all.h")
  file(WRITE ${include_all} "/* DO NOT EDIT - this file is created by CMake */\n")

  file(GLOB all_headers "${FLTK_SOURCE_DIR}/FL/*.[hH]")
  foreach(hd ${all_headers})
    get_filename_component(hd "${hd}" NAME)
    # skip headers that must not be included directly and fl_config.h if it exists
    string(REGEX MATCH "^(mac|win32|x11|wayland|fl_config)\.[hH]$" skip "${hd}")
    if(skip STREQUAL "")
      file(APPEND ${include_all} "#include <FL/${hd}>\n")
    endif()
  endforeach()
  file(APPEND ${include_all} "/* End of generated file */\n")

  # now add the 'shadow_variables' target and set the compiler flag

  fl_create_example(shadow_variables shadow_variables.cxx fltk::fltk)
  set_target_properties(shadow_variables PROPERTIES COMPILE_FLAGS -Wshadow)

endif() # GNU or Clang (-Wshadow test)
unset(_compilers)


# *** EXPERIMENTAL ***
# Build some of the demo programs linked against the shared FLTK lib(s).
# This is currently pretty complicated and should be improved.
# It will be simplified significantly when we use more "modern CMake".

if(FLTK_BUILD_SHARED_LIBS)

  if(MSVC) # MS Visual C++ aka. Visual Studio

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
    set(SHARED_TARGETS "")

    fl_create_example(hello-shared hello.cxx "CALL_MAIN;fltk::fltk-shared")
    fl_create_example(pixmap_browser-shared pixmap_browser.cxx "CALL_MAIN;fltk::fltk-shared")
    fl_create_example(unittests-shared "${UNITTEST_SRCS}" "CALL_MAIN;${GLDEMO_SHARED}")

    list(APPEND SHARED_TARGETS hello pixmap_browser unittests)

    if(OPENGL_FOUND)
      fl_create_example(glpuzzle-shared glpuzzle.cxx "CALL_MAIN;${GLDEMO_SHARED}")
      fl_create_example(shape-shared shape.cxx "CALL_MAIN;${GLDEMO_SHARED}")
      list(APPEND SHARED_TARGETS glpuzzle shape)
    endif(OPENGL_FOUND)

    # apply property COMPILE_DEFINITIONS "-D FL_DLL" in a loop for all necessary source files
    foreach(tgt ${SHARED_TARGETS})
      target_compile_definitions (${tgt}-shared PRIVATE "FL_DLL")
    endforeach()

  else() # not MSVC

    fl_create_example(cairo_test-shared cairo_test.cxx fltk::fltk-shared)
    fl_create_example(hello-shared hello.cxx fltk::fltk-shared)
    fl_create_example(pixmap_browser-shared pixmap_browser.cxx fltk::images-shared)
    fl_create_example(unittests-shared "${UNITTEST_SRCS}" "${GLDEMO_SHARED}")

    if(OPENGL_FOUND)
      fl_create_example(glpuzzle-shared glpuzzle.cxx "${GLDEMO_SHARED}")
      fl_create_example(shape-shared shape.cxx "${GLDEMO_SHARED}")
    endif(OPENGL_FOUND)


  endif(MSVC) # (not MSVC)

endif(FLTK_BUILD_SHARED_LIBS)

#####################################################
# We need some support files for the demo programs
#####################################################

# copy the required test files

file(COPY
  demo.menu rgb.txt browser.cxx editor.cxx valuators.fl
  help_dialog.html
  DESTINATION ${TESTFILE_PATH}
)

# copy the image directory for help_dialog.html

file(COPY
  ${CMAKE_CURRENT_SOURCE_DIR}/images
  DESTINATION ${TESTFILE_PATH}
)

# The main test program 'demo' needs additional hints and configurations.
# Note: CMake defines "CMAKE_INTDIR" which is the build type folder (e.g. "Debug")
# for multi config builds (MSVC, Xcode)

target_compile_definitions (demo PRIVATE GENERATED_BY_CMAKE)
