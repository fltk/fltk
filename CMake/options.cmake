#
# "$Id$"
#
# Main CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
# Written by Michael Surette
#
# Copyright 1998-2018 by Bill Spitzak and others.
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

set (DEBUG_OPTIONS_CMAKE 0)
if (DEBUG_OPTIONS_CMAKE)
  message (STATUS "[** options.cmake **]")
  fl_debug_var (WIN32)
  fl_debug_var (FLTK_LDLIBS)
endif (DEBUG_OPTIONS_CMAKE)

#######################################################################
# *Temporary* option to modify header file searches
#######################################################################
# Note: The old, deprecated behavior (ON) was to use find_file() for
# header searches, the new behavior (ON) is to use check_include_files()
# which seems to be more reliable but more difficult to set up and
# slower because it uses a compilation test. Default is "new" (OFF).
# This option should be removed as soon as the new search strategy
# is considered stable.
# Currently used only in resources.cmake.
#######################################################################
option(USE_FIND_FILE
  "Deprecated: use find_file() for header searches. Should be OFF."
  OFF
)
mark_as_advanced(USE_FIND_FILE)

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
if (UNIX)
  option(OPTION_CREATE_LINKS "create backwards compatibility links" OFF)
endif (UNIX)

#######################################################################
##  Add a TEMPORARY OPTION to enable high-DPI support under Windows.
##  May be removed once high-DPI support under Windows is complete.
#######################################################################
if (WIN32)
  option(OPTION_HIDPI "build with experimental high-DPI support" OFF)
  if (OPTION_HIDPI)
    add_definitions("-DFLTK_HIDPI_SUPPORT")
  endif (OPTION_HIDPI)
endif (WIN32)

#######################################################################
if(APPLE)
   option(OPTION_APPLE_X11 "use X11" OFF)
   option(OPTION_APPLE_SDL "use SDL" OFF)
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

if (OPTION_APPLE_X11)
  include_directories(AFTER SYSTEM /opt/X11/include/freetype2)
endif (OPTION_APPLE_X11)

if (OPTION_APPLE_SDL)
   find_package(SDL2 REQUIRED)
   if (SDL2_FOUND)
      set(USE_SDL 1)
      set(FL_PORTING 1)
      list(APPEND FLTK_LDLIBS SDL2_LIBRARY)
   endif(SDL2_FOUND)
endif(OPTION_APPLE_SDL)

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
option(OPTION_PRINT_SUPPORT "allow print support" ON)
option(OPTION_FILESYSTEM_SUPPORT "allow file system support" ON)

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
   "use FLTK code instrumentation for cairo extended use" OFF
   )

if(OPTION_CAIRO OR OPTION_CAIROEXT AND LIB_CAIRO)
   pkg_search_module(PKG_CAIRO cairo)
endif(OPTION_CAIRO OR OPTION_CAIROEXT AND LIB_CAIRO)

if(PKG_CAIRO_FOUND)
   set(FLTK_HAVE_CAIRO 1)
   add_subdirectory(cairo)
   list(APPEND FLTK_LDLIBS -lcairo -lpixman-1)
   include_directories(${PKG_CAIRO_INCLUDE_DIRS})
   string(REPLACE ";" " " CAIROFLAGS "${PKG_CAIRO_CFLAGS}")
endif(PKG_CAIRO_FOUND)

if(LIB_CAIRO AND OPTION_CAIROEXT AND PKG_CAIRO_FOUND)
   set(FLTK_USE_CAIRO 1)
   set(FLTK_CAIRO_FOUND TRUE)
else()
   set(FLTK_CAIRO_FOUND FALSE)
endif(LIB_CAIRO AND OPTION_CAIROEXT AND PKG_CAIRO_FOUND)

#######################################################################
option(OPTION_USE_NANOSVG "support SVG images" ON)

if(OPTION_USE_NANOSVG)
  set(FLTK_USE_NANOSVG 1)
endif(OPTION_USE_NANOSVG)

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
   elseif(OPTION_APPLE_SDL)
      set(OPENGL_FOUND FALSE)
   else()
      include(FindOpenGL)
      if(APPLE)
         set(HAVE_GL_GLU_H ${HAVE_OPENGL_GLU_H})
      endif(APPLE)
   endif(OPTION_APPLE_X11)
else ()
  set(OPENGL_FOUND FALSE)
endif(OPTION_USE_GL)

if (OPENGL_FOUND)
  set (CMAKE_REQUIRED_INCLUDES ${OPENGL_INCLUDE_DIR}/GL)

  # Set GLLIBS (used in fltk-config).
  # We should probably deduct this from OPENGL_LIBRARIES but it turned
  # out to be difficult since FindOpenGL seems to return different
  # syntax depending on the platform (and maybe also CMake version).
  # Hence we use the following code...

  if (WIN32)
    set (GLLIBS "-lglu32 -lopengl32")
  elseif (APPLE AND NOT OPTION_APPLE_X11)
    set (GLLIBS "-framework OpenGL")
  else ()
    set (GLLIBS "-lGLU -lGL")
  endif (WIN32)

  # check if function glXGetProcAddressARB exists
  set (TEMP_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
  set (CMAKE_REQUIRED_LIBRARIES ${OPENGL_LIBRARIES})
  CHECK_FUNCTION_EXISTS (glXGetProcAddressARB HAVE_GLXGETPROCADDRESSARB)
  set (CMAKE_REQUIRED_LIBRARIES ${TEMP_REQUIRED_LIBRARIES})
  unset (TEMP_REQUIRED_LIBRARIES)

  set (FLTK_GL_FOUND TRUE)
else ()
  set (FLTK_GL_FOUND FALSE)
  set (GLLIBS)
endif (OPENGL_FOUND)

#######################################################################
# Create an option whether we want to check for pthreads.
# We must not do it on Windows unless we run under Cygwin, since we
# always use native threads on Windows (even if libpthread is available).

# Note: HAVE_PTHREAD_H has already been determined in resources.cmake
# before this file is included (or set to 0 for WIN32).

if (WIN32 AND NOT CYGWIN)
  # set(HAVE_PTHREAD_H 0) # (see resources.cmake)
  set(OPTION_USE_THREADS FALSE)
else ()
  option(OPTION_USE_THREADS "use multi-threading with pthreads" ON)
endif (WIN32 AND NOT CYGWIN)

# initialize more variables
set(USE_THREADS 0)
set(HAVE_PTHREAD 0)
set(FLTK_PTHREADS_FOUND FALSE)

if (OPTION_USE_THREADS)

  include(FindThreads)

  if (CMAKE_HAVE_THREADS_LIBRARY)
    add_definitions("-D_THREAD_SAFE -D_REENTRANT")
    set(USE_THREADS 1)
    set(FLTK_THREADS_FOUND TRUE)
  endif (CMAKE_HAVE_THREADS_LIBRARY)

  if (CMAKE_USE_PTHREADS_INIT AND NOT WIN32)
    set(HAVE_PTHREAD 1)
    if (NOT APPLE)
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
    endif (NOT APPLE)
    list(APPEND FLTK_LDLIBS -lpthread)
    list(APPEND FLTK_CFLAGS -D_THREAD_SAFE -D_REENTRANT)
    set(FLTK_PTHREADS_FOUND TRUE)
  else()
    set(HAVE_PTHREAD 0)
    set(HAVE_PTHREAD_H 0)
    set(FLTK_PTHREADS_FOUND FALSE)
  endif(CMAKE_USE_PTHREADS_INIT AND NOT WIN32)

else (OPTION_USE_THREADS)

  set(HAVE_PTHREAD_H 0)

endif (OPTION_USE_THREADS)

set (debug_threads 0) # set to 1 to show debug info
if (debug_threads)
  message ("")
  message (STATUS "options.cmake: set debug_threads to 0 to disable the following info:")
  fl_debug_var(OPTION_USE_THREADS)
  fl_debug_var(HAVE_PTHREAD)
  fl_debug_var(HAVE_PTHREAD_H)
  fl_debug_var(FLTK_THREADS_FOUND)
  fl_debug_var(CMAKE_EXE_LINKER_FLAGS)
  message (STATUS "options.cmake: end of debug_threads info.")
endif (debug_threads)
unset (debug_threads)

#######################################################################
option (OPTION_LARGE_FILE "enable large file support" ON)

if (OPTION_LARGE_FILE)
  if (NOT MSVC)
    add_definitions (-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE)
    list (APPEND FLTK_CFLAGS -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE)
  endif (NOT MSVC)
endif (OPTION_LARGE_FILE)

#######################################################################
option (OPTION_USE_SYSTEM_ZLIB "use system zlib" ON)

if (OPTION_USE_SYSTEM_ZLIB)
  include (FindZLIB)
endif (OPTION_USE_SYSTEM_ZLIB)

if (ZLIB_FOUND)
  set (FLTK_ZLIB_LIBRARIES ${ZLIB_LIBRARIES})
  include_directories (${ZLIB_INCLUDE_DIRS})
  set (FLTK_BUILTIN_ZLIB_FOUND FALSE)
else()
  if (OPTION_USE_SYSTEM_ZLIB)
    message(STATUS "\ncannot find system zlib library - using built-in\n")
  endif (OPTION_USE_SYSTEM_ZLIB)

  add_subdirectory (zlib)
  set (FLTK_ZLIB_LIBRARIES fltk_z)
  set (ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/zlib)
  set (FLTK_BUILTIN_ZLIB_FOUND TRUE)
endif (ZLIB_FOUND)

set (HAVE_LIBZ 1)

#######################################################################
option (OPTION_USE_SYSTEM_LIBJPEG "use system libjpeg" ON)

if (OPTION_USE_SYSTEM_LIBJPEG)
  include (FindJPEG)
endif (OPTION_USE_SYSTEM_LIBJPEG)

if (JPEG_FOUND)
  set(FLTK_JPEG_LIBRARIES ${JPEG_LIBRARIES})
  include_directories(${JPEG_INCLUDE_DIR})
  set(FLTK_BUILTIN_JPEG_FOUND FALSE)
else ()
  if (OPTION_USE_SYSTEM_LIBJPEG)
    message (STATUS "\ncannot find system jpeg library - using built-in\n")
  endif (OPTION_USE_SYSTEM_LIBJPEG)

  add_subdirectory (jpeg)
  set (FLTK_JPEG_LIBRARIES fltk_jpeg)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/jpeg)
  set (FLTK_BUILTIN_JPEG_FOUND TRUE)
endif (JPEG_FOUND)

set (HAVE_LIBJPEG 1)

#######################################################################
option (OPTION_USE_SYSTEM_LIBPNG "use system libpng" ON)

if (OPTION_USE_SYSTEM_LIBPNG)
  include (FindPNG)
endif (OPTION_USE_SYSTEM_LIBPNG)

if (PNG_FOUND)
  set (FLTK_PNG_LIBRARIES ${PNG_LIBRARIES})
  include_directories (${PNG_INCLUDE_DIR})
  add_definitions (${PNG_DEFINITIONS})
  set (FLTK_BUILTIN_PNG_FOUND FALSE)
else()
  if (OPTION_USE_SYSTEM_LIBPNG)
    message (STATUS "\ncannot find system png library - using built-in\n")
  endif (OPTION_USE_SYSTEM_LIBPNG)

  add_subdirectory (png)
  set (FLTK_PNG_LIBRARIES fltk_png)
  set (HAVE_PNG_H 1)
  set (HAVE_PNG_GET_VALID 1)
  set (HAVE_PNG_SET_TRNS_TO_ALPHA 1)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/png)
  set (FLTK_BUILTIN_PNG_FOUND TRUE)
endif (PNG_FOUND)

set (HAVE_LIBPNG 1)

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
   option(OPTION_USE_PANGO "use lib Pango" OFF)
endif(X11_Xft_FOUND)

# test option compatibility: Pango requires Xft
if (OPTION_USE_PANGO)
  if (NOT X11_Xft_FOUND)
    message(STATUS "Pango requires Xft but Xft library or headers could not be found.")
    message(STATUS "Please install Xft development files and try again or disable OPTION_USE_PANGO.")
    message(FATAL_ERROR "*** Aborting ***")
  else ()
    if (NOT OPTION_USE_XFT)
      message(STATUS "Pango requires Xft but usage of Xft was disabled.")
      message(STATUS "Please enable OPTION_USE_XFT and try again or disable OPTION_USE_PANGO.")
      message(FATAL_ERROR "*** Aborting ***")
    endif (NOT OPTION_USE_XFT)
  endif (NOT X11_Xft_FOUND)
endif (OPTION_USE_PANGO)

#######################################################################
if(X11_Xft_FOUND AND OPTION_USE_PANGO)
#this covers Debian, Ubuntu, FreeBSD, NetBSD, Darwin
   if(APPLE AND OPTION_APPLE_X11)
	list(APPEND CMAKE_INCLUDE_PATH  /sw/include)
    	list(APPEND CMAKE_LIBRARY_PATH  /sw/lib)
   endif(APPLE AND OPTION_APPLE_X11)
   find_file(HAVE_PANGO_H pango-1.0/pango/pango.h ${CMAKE_INCLUDE_PATH})
   find_file(HAVE_PANGOXFT_H pango-1.0/pango/pangoxft.h ${CMAKE_INCLUDE_PATH})

  if(HAVE_PANGO_H AND HAVE_PANGOXFT_H)
    find_library(HAVE_LIB_PANGO pango-1.0 ${CMAKE_LIBRARY_PATH})
    find_library(HAVE_LIB_PANGOXFT pangoxft-1.0 ${CMAKE_LIBRARY_PATH})
    if(APPLE)
    	set(HAVE_LIB_GOBJECT TRUE)
    else()
    	find_library(HAVE_LIB_GOBJECT gobject-2.0 ${CMAKE_LIBRARY_PATH})
    endif(APPLE)
  endif(HAVE_PANGO_H AND HAVE_PANGOXFT_H)
  if(HAVE_LIB_PANGO AND HAVE_LIB_PANGOXFT AND HAVE_LIB_GOBJECT)
    set(USE_PANGO TRUE)
    # message(STATUS "USE_PANGO=" ${USE_PANGO})
    # remove last 3 components of HAVE_PANGO_H and put in PANGO_H_PREFIX
    get_filename_component(PANGO_H_PREFIX ${HAVE_PANGO_H} PATH)
    get_filename_component(PANGO_H_PREFIX ${PANGO_H_PREFIX} PATH)
    get_filename_component(PANGO_H_PREFIX ${PANGO_H_PREFIX} PATH)

    get_filename_component(PANGOLIB_DIR ${HAVE_LIB_PANGO} PATH)
    # glib.h is usually in ${PANGO_H_PREFIX}/glib-2.0/ ...
    find_path(GLIB_H_PATH glib.h ${PANGO_H_PREFIX}/glib-2.0)
    if(NOT GLIB_H_PATH) # ... but not under NetBSD
      find_path(GLIB_H_PATH glib.h ${PANGO_H_PREFIX}/glib/glib-2.0)
    endif(NOT GLIB_H_PATH)
    include_directories(${PANGO_H_PREFIX}/pango-1.0 ${GLIB_H_PATH} ${PANGOLIB_DIR}/glib-2.0/include)
    list(APPEND FLTK_LDLIBS -lpango-1.0 -lpangoxft-1.0 -lgobject-2.0)
  endif(HAVE_LIB_PANGO AND HAVE_LIB_PANGOXFT AND HAVE_LIB_GOBJECT)
endif(X11_Xft_FOUND AND OPTION_USE_PANGO)

if(OPTION_USE_XFT)
   set(USE_XFT X11_Xft_FOUND)
   list(APPEND FLTK_LDLIBS -lXft)
   set(FLTK_XFT_FOUND TRUE)
   if(APPLE AND OPTION_APPLE_X11)
     find_library(LIB_fontconfig fontconfig "/opt/X11/lib")
   endif(APPLE AND OPTION_APPLE_X11)
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
set(FL_NO_PRINT_SUPPORT FALSE)
if(X11_FOUND AND NOT OPTION_PRINT_SUPPORT)
   set(FL_NO_PRINT_SUPPORT TRUE)
endif(X11_FOUND AND NOT OPTION_PRINT_SUPPORT)
#######################################################################

#######################################################################
set(FL_CFG_NO_FILESYSTEM_SUPPORT TRUE)
if(OPTION_FILESYSTEM_SUPPORT)
   set(FL_CFG_NO_FILESYSTEM_SUPPORT FALSE)
endif(OPTION_FILESYSTEM_SUPPORT)
#######################################################################

#######################################################################
# prior to CMake 3.0 this feature was buggy
if(NOT CMAKE_VERSION VERSION_LESS 3.0.0)
    option(CMAKE_SUPPRESS_REGENERATION
	   "suppress rules to re-run CMake on rebuild" OFF)
    mark_as_advanced(CMAKE_SUPPRESS_REGENERATION)
endif(NOT CMAKE_VERSION VERSION_LESS 3.0.0)

#######################################################################
# Debugging ...

if (DEBUG_OPTIONS_CMAKE)
  message (STATUS "") # empty line
  fl_debug_var (WIN32)
  fl_debug_var (LIBS)
  fl_debug_var (GLLIBS)
  fl_debug_var (FLTK_LDLIBS)
  fl_debug_var (USE_FIND_FILE)
  fl_debug_var (OPENGL_FOUND)
  fl_debug_var (OPENGL_INCLUDE_DIR)
  fl_debug_var (OPENGL_LIBRARIES)
  message (STATUS "[** end of options.cmake **]")
endif (DEBUG_OPTIONS_CMAKE)
unset (DEBUG_OPTIONS_CMAKE)
