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
# examples requiring fltk::images
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
# examples requiring OpenGL3 + GLEW
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

foreach(src ${CAIRO_SOURCES})
  fl_create_example(${src} ${src}.cxx fltk::fltk)
endforeach(src)

############################################################
# create example programs with OpenGL3 + GLEW
############################################################
#
# Note 1: macOS (Quartz) does not need GLEW, it's included in OpenGL
# Note 2: find_package(GLEW) finds either shared or static libs or both.
# Note 3: on Windows we set the variable GLEW_USE_STATIC_LIBS=TRUE because
#         we *want* to find static libs but we *can* also use shared libs.
# Note 4: FindGLEW.cmake changed the library suffixes for MinGW in CMake 3.28.0,
#         obviously "assuming" that ".lib" is Visual Studio only. There have been
#         discussions about finding the "wrong" libraries since CMake 3.25 or so.
#         Therefore the static lib "glew32s.lib" is not found if CMake 3.28 or
#         higher is used (current version, as of this writing: 3.29.3). However,
#         this assumption is probably not true for a pure C library (glew32s).
#         This library *is* found and works well with CMake 3.15.0 - 3.27.9.
#         Users may need to copy or rename "glew32s.lib" to "glew32s.a"
#         if CMake 3.28 or higher is used.
#         Albrecht-S, May 13, 2024

if(OPENGL_FOUND)
  if(WIN32)
    set(GLEW_USE_STATIC_LIBS TRUE)
  endif()
  set(_glew_lib GLEW::glew)
  set(_glew_static FALSE)
  if(APPLE AND NOT FLTK_BACKEND_X11) # macOS Quartz
    set(_glew_lib)
    set(GLEW_FOUND TRUE)
  else()
    # set(GLEW_VERBOSE TRUE) # make `find_package(GLEW)` verbose
    set(_CMAKE_PREFIX_PATH_SAVED ${CMAKE_PREFIX_PATH})
    set(CMAKE_PREFIX_PATH ${FLTK_GLEW_DIR} ${CMAKE_PREFIX_PATH})
    find_package(GLEW MODULE)
    set(CMAKE_PREFIX_PATH ${_CMAKE_PREFIX_PATH_SAVED})
    unset(_CMAKE_PREFIX_PATH_SAVED)
    # Did we find the static lib? If yes, use it
    if(TARGET GLEW::glew_s)
      set(_glew_lib GLEW::glew_s)
      set(_glew_static TRUE)
    endif()
  endif()

  if(0) # Debug
    fl_debug_var(OPENGL_FOUND)
    fl_debug_var(GLEW_FOUND)
    fl_debug_var(GLEW_DIR)
    fl_debug_var(_glew_lib)
    fl_debug_target(GLEW::glew)
    fl_debug_target(GLEW::glew_s)
    fl_debug_target(GLEW::GLEW)
  endif() # /Debug

endif(OPENGL_FOUND)

if(OPENGL_FOUND AND (TARGET "${_glew_lib}" OR APPLE))

  # GLEW was found, create the OpenGL3 targets:

  foreach(tgt ${OPENGL_SOURCES})
    fl_create_example(${tgt} ${tgt}.cxx "fltk::gl;${_glew_lib}")
    set_property(TARGET ${tgt} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${tgt} PROPERTY CXX_STANDARD_REQUIRED TRUE)
    # define preprocessor macro GLEW_STATIC only if we link to the static lib
    if(_glew_static)
      target_compile_definitions(${tgt} PRIVATE "GLEW_STATIC")
    endif()
  endforeach(tgt)

else()

  message(STATUS
    "OpenGL or GLEW not present: OpenGL3 example programs will not be built.")
  fl_debug_var(OPENGL_FOUND)
  fl_debug_var(GLEW_FOUND)
  message("")

endif() # (OPENGL_FOUND AND TARGET "${_glew_lib}")
