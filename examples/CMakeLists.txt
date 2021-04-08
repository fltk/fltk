#
# CMakeLists.txt used to build example apps by the CMake build system
#
# Copyright 2020-2021 by Bill Spitzak and others.
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

include (../CMake/fl_create_example.cmake)

set (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/../bin/examples)
file (MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

################################################################################

# create separate lists of all source (.cxx) files
# depending on the required FLTK and system libraries

############################################################
# simple examples w/o extra libs
############################################################

set (SIMPLE_SOURCES
  howto-add_fd-and-popen
  howto-browser-with-icons
  howto-drag-and-drop
  howto-parse-args
  howto-text-over-image-button
  menubar-add
  nativefilechooser-simple-app
  progress-simple
  shapedwindow
  table-as-container
  table-simple
  table-sort
  table-spreadsheet
  table-spreadsheet-with-keyboard-nav
  table-with-keynav
  tabs-simple
  textdisplay-with-colors
  texteditor-simple
  tree-as-container
  tree-custom-draw-items
  tree-custom-sort
  tree-of-tables
  tree-simple
  wizard-simple
)

############################################################
# examples requiring fltk_images
############################################################

set (IMAGE_SOURCES
  clipboard
)

############################################################
# examples requiring OpenGL + libGLEW
############################################################

set (OPENGL_SOURCES
  OpenGL3-glut-test
  OpenGL3test
)

############################################################
# create simple example programs
############################################################

foreach (src ${SIMPLE_SOURCES})
  CREATE_EXAMPLE (${src} ${src}.cxx fltk)
endforeach (src)

############################################################
# create example programs with fltk_images library
############################################################

foreach (src ${IMAGE_SOURCES})
  CREATE_EXAMPLE (${src} ${src}.cxx "fltk_images;fltk")
endforeach (src)

############################################################
# create example programs with OpenGL + libGLEW
############################################################

# Note: macOS does not need libGLEW

if (APPLE AND (NOT OPTION_APPLE_X11))
  if (NOT LIB_GLEW)
    set (LIB_GLEW TRUE)
  endif ()
  set (REQUIRED_LIBS fltk_gl fltk ${OPENGL_LIBRARIES})
else ()
  set (REQUIRED_LIBS fltk_gl fltk ${OPENGL_LIBRARIES} GLEW)
endif ()

if (OPENGL_FOUND AND LIB_GLEW)
  foreach (src ${OPENGL_SOURCES})
    CREATE_EXAMPLE (${src} ${src}.cxx "${REQUIRED_LIBS}")
  endforeach (src)
else ()
  message (STATUS
    "OpenGL or libGLEW not present: OpenGL example programs will not be built.")
  fl_debug_var (OPENGL_FOUND)
  fl_debug_var (LIB_GLEW)
  message ("")
endif (OPENGL_FOUND AND LIB_GLEW)

unset (REQUIRED_LIBS)
