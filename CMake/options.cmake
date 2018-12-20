#
# "$Id$"
#
# Main CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
# Written by Michael Surette
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

#######################################################################
# options
#######################################################################
set(OPTION_OPTIM ""
   CACHE STRING
   "custom optimization flags"
   )
add_definitions(${OPTION_OPTIM})

#######################################################################
set(OPTION_ARCHFLAGS ""
   CACHE STRING
   "custom architecture flags"
   )
add_definitions(${OPTION_ARCHFLAGS})

#######################################################################
set(OPTION_ABI_VERSION ""
   CACHE STRING
   "FLTK ABI Version FL_ABI_VERSION: 1xxyy for 1.x.y (xx,yy with leading zero)"
   )
set(FL_ABI_VERSION ${OPTION_ABI_VERSION})

#######################################################################
#######################################################################
if(UNIX)
   option(OPTION_CREATE_LINKS "create backwards compatibility links" OFF)
   list(APPEND FLTK_LDLIBS -lm)
endif(UNIX)

#######################################################################
if(APPLE)
   option(OPTION_APPLE_X11 "use X11" OFF)
endif(APPLE)

if((NOT APPLE OR OPTION_APPLE_X11) AND NOT WIN32)
   include(FindX11)
   if(X11_FOUND)
      set(USE_X11 1)
      list(APPEND FLTK_LDLIBS -lX11)
      if (X11_Xext_FOUND)
	  list(APPEND FLTK_LDLIBS -lXext)
      endif(X11_Xext_FOUND)
   endif(X11_FOUND)
endif((NOT APPLE OR OPTION_APPLE_X11) AND NOT WIN32)

#######################################################################
option(OPTION_USE_POLL "use poll if available" OFF)
mark_as_advanced(OPTION_USE_POLL)

if(OPTION_USE_POLL)
   CHECK_FUNCTION_EXISTS(poll USE_POLL)
endif(OPTION_USE_POLL)

#######################################################################
option(OPTION_BUILD_SHARED_LIBS
    "Build shared libraries(in addition to static libraries)"
    OFF
    )

#######################################################################
option(OPTION_BUILD_EXAMPLES "build example programs" ON)

#######################################################################
if(DOXYGEN_FOUND)
    option(OPTION_BUILD_HTML_DOCUMENTATION "build html docs" OFF)
    option(OPTION_INSTALL_HTML_DOCUMENTATION "install html docs" OFF)
    if(LATEX_FOUND)
	option(OPTION_BUILD_PDF_DOCUMENTATION "build pdf docs" OFF)
	option(OPTION_INSTALL_PDF_DOCUMENTATION "install pdf docs" OFF)
    endif(LATEX_FOUND)
endif(DOXYGEN_FOUND)

if(OPTION_BUILD_HTML_DOCUMENTATION OR OPTION_BUILD_PDF_DOCUMENTATION)
    add_subdirectory(documentation)
endif(OPTION_BUILD_HTML_DOCUMENTATION OR OPTION_BUILD_PDF_DOCUMENTATION)

#######################################################################
include(FindPkgConfig)

option(OPTION_CAIRO "use lib Cairo" OFF)
option(OPTION_CAIROEXT
   "use FLTK code instrumentation for Cairo extended use" OFF
   )

if ((OPTION_CAIRO OR OPTION_CAIROEXT) AND LIB_CAIRO)
   pkg_search_module(PKG_CAIRO cairo)
endif ((OPTION_CAIRO OR OPTION_CAIROEXT) AND LIB_CAIRO)

if (PKG_CAIRO_FOUND)
   set (FLTK_HAVE_CAIRO 1)
   add_subdirectory(cairo)
   list(APPEND FLTK_LDLIBS -lcairo -lpixman-1)
   include_directories(${PKG_CAIRO_INCLUDE_DIRS})
   string(REPLACE ";" " " CAIROFLAGS "${PKG_CAIRO_CFLAGS}")
endif (PKG_CAIRO_FOUND)

if (LIB_CAIRO AND OPTION_CAIROEXT AND PKG_CAIRO_FOUND)
   set(FLTK_USE_CAIRO 1)
   set(FLTK_CAIRO_FOUND TRUE)
else()
   set(FLTK_CAIRO_FOUND FALSE)
endif (LIB_CAIRO AND OPTION_CAIROEXT AND PKG_CAIRO_FOUND)

if ((OPTION_CAIRO OR OPTION_CAIROEXT) AND NOT PKG_CAIRO_FOUND)
   message(STATUS "*** Cairo was requested but not found - please install Cairo ...")
   message(STATUS "*** or disable options OPTION_CAIRO and OPTION_CAIRO_EXT")
   message(FATAL_ERROR "*** Terminating: missing Cairo libs or headers")
endif ((OPTION_CAIRO OR OPTION_CAIROEXT) AND NOT PKG_CAIRO_FOUND)

#######################################################################
set(HAVE_GL LIB_GL OR LIB_MesaGL)

if(HAVE_GL)
   option(OPTION_USE_GL "use OpenGL" ON)
endif(HAVE_GL)

if(OPTION_USE_GL)
   if(OPTION_APPLE_X11)
      set(OPENGL_FOUND TRUE)
      get_filename_component(PATH_TO_XLIBS ${X11_X11_LIB} PATH)
      set(OPENGL_LIBRARIES -L${PATH_TO_XLIBS} -lGLU -lGL)
      find_file(HAVE_GL_GLU_H GL/glu.h PATHS /opt/X11/include)
   else()
      include(FindOpenGL)
      if(APPLE)
         set(HAVE_GL_GLU_H ${HAVE_OPENGL_GLU_H})
      endif(APPLE)
   endif(OPTION_APPLE_X11)
endif(OPTION_USE_GL)

if(OPENGL_FOUND)
   set(CMAKE_REQUIRED_INCLUDES ${OPENGL_INCLUDE_DIR}/GL)
   set(GLLIB "-lGLU -lGL")

   # check if function glXGetProcAddressARB exists
   set(TEMP_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
   set(CMAKE_REQUIRED_LIBRARIES GLU GL)
   CHECK_FUNCTION_EXISTS(glXGetProcAddressARB HAVE_GLXGETPROCADDRESSARB)
   set(CMAKE_REQUIRED_LIBRARIES ${TEMP_REQUIRED_LIBRARIES})
   unset(TEMP_REQUIRED_LIBRARIES)

   set(FLTK_GL_FOUND TRUE)
else()
   set(FLTK_GL_FOUND FALSE)
endif(OPENGL_FOUND)

#######################################################################
option(OPTION_USE_THREADS "use multi-threading" ON)

if(OPTION_USE_THREADS)
   include(FindThreads)
endif(OPTION_USE_THREADS)

if(OPTION_USE_THREADS AND CMAKE_HAVE_THREADS_LIBRARY)
   add_definitions("-D_THREAD_SAFE -D_REENTRANT")
   set(USE_THREADS 1)
   set(FLTK_THREADS_FOUND TRUE)
else()
   set(FLTK_THREADS_FOUND FALSE)
endif(OPTION_USE_THREADS AND CMAKE_HAVE_THREADS_LIBRARY)

if(OPTION_USE_THREADS AND CMAKE_USE_PTHREADS_INIT)
   set(HAVE_PTHREAD 1)
   if(NOT APPLE)
     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
   endif(NOT APPLE)
   list(APPEND FLTK_LDLIBS -lpthread)
   list(APPEND FLTK_CFLAGS -D_THREAD_SAFE -D_REENTRANT)
   set(FLTK_PTHREADS_FOUND TRUE)
else()
   set(HAVE_PTHREAD 0)
   set(HAVE_PTHREAD_H 0)
   set(FLTK_PTHREADS_FOUND FALSE)
endif(OPTION_USE_THREADS AND CMAKE_USE_PTHREADS_INIT)

#######################################################################
option(OPTION_LARGE_FILE "enable large file support" ON)

if(OPTION_LARGE_FILE)
   if(NOT MSVC)
      add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE)
      list(APPEND FLTK_CFLAGS -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE)
   endif(NOT MSVC)
endif(OPTION_LARGE_FILE)

#######################################################################
option(OPTION_USE_SYSTEM_ZLIB "use system zlib" ON)

if(OPTION_USE_SYSTEM_ZLIB AND LIB_zlib)
   include(FindZLIB)
endif(OPTION_USE_SYSTEM_ZLIB AND LIB_zlib)

if(OPTION_USE_SYSTEM_ZLIB AND ZLIB_FOUND)
   set(FLTK_ZLIB_LIBRARIES ${ZLIB_LIBRARIES})
   include_directories(${ZLIB_INCLUDE_DIRS})
   set(FLTK_BUILTIN_ZLIB_FOUND FALSE)
else()
   add_subdirectory(zlib)
   set(FLTK_ZLIB_LIBRARIES fltk_z)
   set(ZLIB_INCLUDE_DIR ${FLTK_SOURCE_DIR}/zlib)
   include_directories(${FLTK_SOURCE_DIR}/zlib)
   set(FLTK_BUILTIN_ZLIB_FOUND TRUE)
endif(OPTION_USE_SYSTEM_ZLIB AND ZLIB_FOUND)

if(OPTION_USE_SYSTEM_ZLIB AND NOT ZLIB_FOUND)
   message(STATUS "\ncannot find system zlib library - using built-in\n")
endif(OPTION_USE_SYSTEM_ZLIB AND NOT ZLIB_FOUND)

set(HAVE_LIBZ 1)

#######################################################################
option(OPTION_USE_SYSTEM_LIBJPEG "use system libjpeg" ON)

if(OPTION_USE_SYSTEM_LIBJPEG AND LIB_jpeg)
   include(FindJPEG)
endif(OPTION_USE_SYSTEM_LIBJPEG AND LIB_jpeg)

if(OPTION_USE_SYSTEM_LIBJPEG AND JPEG_FOUND)
   set(FLTK_JPEG_LIBRARIES ${JPEG_LIBRARIES})
   include_directories(${JPEG_INCLUDE_DIR})
   set(FLTK_BUILTIN_JPEG_FOUND FALSE)
else()
   add_subdirectory(jpeg)
   set(FLTK_JPEG_LIBRARIES fltk_jpeg)
   include_directories(${FLTK_SOURCE_DIR}/jpeg)
   set(FLTK_BUILTIN_JPEG_FOUND TRUE)
endif(OPTION_USE_SYSTEM_LIBJPEG AND JPEG_FOUND)

if(OPTION_USE_SYSTEM_LIBJPEG AND NOT JPEG_FOUND)
   message(STATUS "\ncannot find system jpeg library - using built-in\n")
endif(OPTION_USE_SYSTEM_LIBJPEG AND NOT JPEG_FOUND)

set(HAVE_LIBJPEG 1)

#######################################################################
option(OPTION_USE_SYSTEM_LIBPNG "use system libpng" ON)

if(OPTION_USE_SYSTEM_LIBPNG AND LIB_png)
   include(FindPNG)
endif(OPTION_USE_SYSTEM_LIBPNG AND LIB_png)

if(OPTION_USE_SYSTEM_LIBPNG AND PNG_FOUND)
   set(FLTK_PNG_LIBRARIES ${PNG_LIBRARIES})
   include_directories(${PNG_INCLUDE_DIR})
   add_definitions(${PNG_DEFINITIONS})
   set(FLTK_BUILTIN_PNG_FOUND FALSE)
else()
   add_subdirectory(png)
   set(FLTK_PNG_LIBRARIES fltk_png)
   set(HAVE_PNG_H 1)
   set(HAVE_PNG_GET_VALID 1)
   set(HAVE_PNG_SET_TRNS_TO_ALPHA 1)
   include_directories(${FLTK_SOURCE_DIR}/png)
   set(FLTK_BUILTIN_PNG_FOUND TRUE)
endif(OPTION_USE_SYSTEM_LIBPNG AND PNG_FOUND)

if(OPTION_USE_SYSTEM_LIBPNG AND NOT PNG_FOUND)
   message(STATUS "\ncannot find system png library - using built-in\n")
endif(OPTION_USE_SYSTEM_LIBPNG AND NOT PNG_FOUND)

set(HAVE_LIBPNG 1)

#######################################################################
if(X11_Xinerama_FOUND)
   option(OPTION_USE_XINERAMA "use lib Xinerama" ON)
endif(X11_Xinerama_FOUND)

if(OPTION_USE_XINERAMA)
   set(HAVE_XINERAMA ${X11_Xinerama_FOUND})
   include_directories(${X11_Xinerama_INCLUDE_PATH})
   list(APPEND FLTK_LDLIBS -lXinerama)
   set(FLTK_XINERAMA_FOUND TRUE)
else()
   set(FLTK_XINERAMA_FOUND FALSE)
endif(OPTION_USE_XINERAMA)

#######################################################################
if(X11_Xfixes_FOUND)
   option(OPTION_USE_XFIXES "use lib Xfixes" ON)
endif(X11_Xfixes_FOUND)

if(OPTION_USE_XFIXES)
   set(HAVE_XFIXES ${X11_Xfixes_FOUND})
   include_directories(${X11_Xfixes_INCLUDE_PATH})
   list(APPEND FLTK_LDLIBS -lXfixes)
   set(FLTK_XFIXES_FOUND TRUE)
else()
   set(FLTK_XFIXES_FOUND FALSE)
endif(OPTION_USE_XFIXES)

#######################################################################
if(X11_Xcursor_FOUND)
   option(OPTION_USE_XCURSOR "use lib Xcursor" ON)
endif(X11_Xcursor_FOUND)

if(OPTION_USE_XCURSOR)
   set(HAVE_XCURSOR ${X11_Xcursor_FOUND})
   include_directories(${X11_Xcursor_INCLUDE_PATH})
   list(APPEND FLTK_LDLIBS -lXcursor)
   set(FLTK_XCURSOR_FOUND TRUE)
else()
   set(FLTK_XCURSOR_FOUND FALSE)
endif(OPTION_USE_XCURSOR)

#######################################################################
if(X11_Xft_FOUND)
   option(OPTION_USE_XFT "use lib Xft" ON)
endif(X11_Xft_FOUND)

if(OPTION_USE_XFT)
   set(USE_XFT X11_Xft_FOUND)
   list(APPEND FLTK_LDLIBS -lXft)
   set(FLTK_XFT_FOUND TRUE)
else()
   set(FLTK_XFT_FOUND FALSE)
endif(OPTION_USE_XFT)

#######################################################################
if(X11_Xrender_FOUND)
   option(OPTION_USE_XRENDER "use lib Xrender" ON)
endif(X11_Xrender_FOUND)

if(OPTION_USE_XRENDER)
   set(HAVE_XRENDER ${X11_Xrender_FOUND})
   if(HAVE_XRENDER)
      include_directories(${X11_Xrender_INCLUDE_PATH})
      list(APPEND FLTK_LDLIBS -lXrender)
      set(FLTK_XRENDER_FOUND TRUE)
   else(HAVE_XRENDER)
      set(FLTK_XRENDER_FOUND FALSE)
   endif(HAVE_XRENDER)
else(OPTION_USE_XRENDER)
   set(FLTK_XRENDER_FOUND FALSE)
endif(OPTION_USE_XRENDER)

#######################################################################
if(X11_FOUND)
   option(OPTION_USE_XDBE "use lib Xdbe" ON)
endif(X11_FOUND)

if(OPTION_USE_XDBE AND HAVE_XDBE_H)
   set(HAVE_XDBE 1)
   set(FLTK_XDBE_FOUND TRUE)
else()
   set(FLTK_XDBE_FOUND FALSE)
endif(OPTION_USE_XDBE AND HAVE_XDBE_H)

#######################################################################
# prior to CMake 3.0 this feature was buggy
if(NOT CMAKE_VERSION VERSION_LESS 3.0.0)
    option(CMAKE_SUPPRESS_REGENERATION
	   "suppress rules to re-run CMake on rebuild" OFF)
    mark_as_advanced(CMAKE_SUPPRESS_REGENERATION)
endif(NOT CMAKE_VERSION VERSION_LESS 3.0.0)
