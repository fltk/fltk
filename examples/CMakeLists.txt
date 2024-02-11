#
# CMakeLists.txt used to build example apps by the CMake build system
#
# Copyright 2020-2024 by Bill Spitzak and others.
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
################################################################################

include(../CMake/fl_create_example.cmake)
include(../CMake/FLTK-Functions.cmake)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/../bin/examples)
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

################################################################################

# create separate lists of all source (.cxx) files
# depending on the required FLTK and system libraries

############################################################
# simple examples w/o extra libs
############################################################

set(SIMPLE_SOURCES
  browser-simple
  callbacks
  chart-simple
  draggable-group
  grid-simple
  howto-add_fd-and-popen
  howto-browser-with-icons
  howto-drag-and-drop
  howto-draw-an-x
  howto-flex-simple
  howto-menu-with-images
  howto-parse-args
  howto-remap-numpad-keyboard-keys
  howto-text-over-image-button
  menubar-add
  nativefilechooser-simple
  nativefilechooser-simple-app
  progress-simple
  shapedwindow
  simple-terminal
  table-as-container
  table-simple
  table-sort
  table-spreadsheet
  table-spreadsheet-with-keyboard-nav
  table-with-keynav
  table-with-right-column-stretch-fit
  table-with-right-click-menu
  tabs-simple
  textdisplay-with-colors
  texteditor-simple
  texteditor-with-dynamic-colors
  tree-as-container
  tree-custom-draw-items
  tree-custom-sort
  tree-of-tables
  tree-simple
  wizard-simple
)

############################################################
# simple FLUID examples w/o extra libs
############################################################

set(FLUID_SOURCES
  fluid-callback
)

############################################################
# examples requiring fltk_images
############################################################

set(IMAGE_SOURCES
  animgifimage
  animgifimage-play
  animgifimage-resize
  animgifimage-simple
  howto-simple-svg
)

############################################################
# examples requiring cairo
############################################################

set(CAIRO_SOURCES
  cairo-draw-x
)

############################################################
# examples requiring OpenGL + libGLEW
############################################################

set(OPENGL_SOURCES
  OpenGL3-glut-test
  OpenGL3test
)

############################################################
# create simple example programs
############################################################

foreach(src ${SIMPLE_SOURCES})
  fl_create_example(${src} ${src}.cxx fltk::fltk)
endforeach(src)

############################################################
# create FLUID example programs
############################################################

foreach(src ${FLUID_SOURCES})
  fl_create_example(${src} ${src}.fl fltk::fltk)
endforeach(src)

############################################################
# create example programs with fltk_images library
############################################################

foreach(src ${IMAGE_SOURCES})
  fl_create_example(${src} ${src}.cxx "fltk::images")
endforeach(src)

############################################################
# create example programs requiring cairo
############################################################

if(FLTK_HAVE_CAIRO)
  set(cairo_libs "fltk::fltk;cairo")
else()
  set(cairo_libs "fltk::fltk")
endif()

foreach(src ${CAIRO_SOURCES})
  fl_create_example(${src} ${src}.cxx "${cairo_libs}")
endforeach(src)

############################################################
# create example programs with OpenGL + libGLEW
############################################################

# Note: macOS does not need libGLEW

if(APPLE AND NOT FLTK_BACKEND_X11)
  if(NOT LIB_GLEW)
    set(LIB_GLEW TRUE)
  endif()
  set(REQUIRED_LIBS fltk::gl)
else()
  set(REQUIRED_LIBS fltk::gl ${LIB_GLEW})
endif()

if(OPENGL_FOUND AND LIB_GLEW)
  foreach(tgt ${OPENGL_SOURCES})
    fl_create_example(${tgt} ${tgt}.cxx "${REQUIRED_LIBS}")
    set_property(TARGET ${tgt} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${tgt} PROPERTY CXX_STANDARD_REQUIRED TRUE)
  endforeach(tgt)

else()
  message(STATUS
    "OpenGL or libGLEW not present: OpenGL example programs will not be built.")
  fl_debug_var(OPENGL_FOUND)
  fl_debug_var(LIB_GLEW)
  message("")
endif(OPENGL_FOUND AND LIB_GLEW)

unset(REQUIRED_LIBS)
