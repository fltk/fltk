#
# Main CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
# Originally written by Michael Surette
#
# Copyright 1998-2023 by Bill Spitzak and others.
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
# Important implementation note for FLTK developers
#######################################################################
#
# In the current version of FLTK's CMake build files (1.3.x) we're
# using 'include_directories()' to define directories that must be
# used in compile commands (typically "-Idirectories").
#
# include_directories() is a global command that affects *all* source
# files in the current directory and all subdirectories. This can lead
# to conflicts and should be replaced with target_include_directories()
# which can be applied to particular targets and source files only.
#
# This is a known issue and will be addressed in FLTK 1.4.0.
#
# Albrecht-S December 4, 2023
#
#######################################################################

set (DEBUG_OPTIONS_CMAKE 0)
if (DEBUG_OPTIONS_CMAKE)
  message (STATUS "[** options.cmake **]")
  fl_debug_var (WIN32)
  fl_debug_var (FLTK_LDLIBS)
endif (DEBUG_OPTIONS_CMAKE)

#######################################################################
# options
#######################################################################
set (OPTION_OPTIM ""
  CACHE STRING
  "custom optimization flags"
)
add_definitions (${OPTION_OPTIM})

#######################################################################
set (OPTION_ARCHFLAGS ""
  CACHE STRING
  "custom architecture flags"
)
add_definitions (${OPTION_ARCHFLAGS})

#######################################################################
set (OPTION_ABI_VERSION ""
  CACHE STRING
  "FLTK ABI Version FL_ABI_VERSION: 1xxyy for 1.x.y (xx,yy with leading zero)"
)
set (FL_ABI_VERSION ${OPTION_ABI_VERSION})


#######################################################################
#  Select MSVC (Visual Studio) Runtime: DLL (/MDx) or static (/MTx)
#  where x = 'd' for Debug builds, empty ('') for non-Debug builds.
#  Note: this might be handled better by the 'MSVC_RUNTIME_LIBRARY'
#  target property for each target rather than setting a global
#  CMake variable - but this version does the latter.
#######################################################################

if (MSVC)
  option (FLTK_MSVC_RUNTIME_DLL "use MSVC Runtime-DLL (/MDx)" ON)
  if (FLTK_MSVC_RUNTIME_DLL)
    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  else ()
    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif ()
endif (MSVC)

#######################################################################
#  Bundled Library Options
#######################################################################

option (OPTION_USE_SYSTEM_ZLIB      "use system zlib"    ON)

if (APPLE)
  option (OPTION_USE_SYSTEM_LIBJPEG "use system libjpeg" OFF)
  option (OPTION_USE_SYSTEM_LIBPNG  "use system libpng"  OFF)
else ()
  option (OPTION_USE_SYSTEM_LIBJPEG "use system libjpeg" ON)
  option (OPTION_USE_SYSTEM_LIBPNG  "use system libpng"  ON)
endif ()

#######################################################################
#  Make sure that png and zlib are either system or local for compatibility
#######################################################################

if (OPTION_USE_SYSTEM_ZLIB)
  find_package (ZLIB)
endif ()

if (OPTION_USE_SYSTEM_LIBPNG)
  find_package (PNG)
endif ()

# If we use the system zlib, we must also use the system png zlib and vice versa
# If either of them is not available, we fall back to using both local libraries
if (OPTION_USE_SYSTEM_LIBPNG AND NOT (OPTION_USE_SYSTEM_ZLIB AND ZLIB_FOUND))
    set (PNG_FOUND FALSE)
    set (OPTION_USE_SYSTEM_LIBPNG OFF)
    message (STATUS "Local z lib selected: overriding png lib to local for compatibility.\n")
endif ()
if (OPTION_USE_SYSTEM_ZLIB AND NOT (OPTION_USE_SYSTEM_LIBPNG AND PNG_FOUND))
    set (ZLIB_FOUND FALSE)
    set (OPTION_USE_SYSTEM_ZLIB OFF)
    message (STATUS "Local png lib selected: overriding z lib to local for compatibility.\n")
endif ()

#######################################################################
#  Bundled Compression Library : zlib
#######################################################################

if (OPTION_USE_SYSTEM_ZLIB AND ZLIB_FOUND)
  set (FLTK_USE_BUILTIN_ZLIB FALSE)
  set (FLTK_ZLIB_LIBRARIES ${ZLIB_LIBRARIES})
  include_directories (${ZLIB_INCLUDE_DIRS})
else()
  if (OPTION_USE_SYSTEM_ZLIB)
    message (STATUS "cannot find system zlib library - using built-in\n")
  endif ()

  add_subdirectory (zlib)
  set (FLTK_USE_BUILTIN_ZLIB TRUE)
  set (FLTK_ZLIB_LIBRARIES fltk_z)
  set (ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/zlib)
endif ()

set (HAVE_LIBZ 1)

#######################################################################
#  Bundled Image Library : libjpeg
#######################################################################

if (OPTION_USE_SYSTEM_LIBJPEG)
  find_package (JPEG)
endif ()

if (OPTION_USE_SYSTEM_LIBJPEG AND JPEG_FOUND)
  set (FLTK_USE_BUILTIN_JPEG FALSE)
  set (FLTK_JPEG_LIBRARIES ${JPEG_LIBRARIES})
  include_directories (${JPEG_INCLUDE_DIR})
else ()
  if (OPTION_USE_SYSTEM_LIBJPEG)
    message (STATUS "cannot find system jpeg library - using built-in\n")
  endif ()

  add_subdirectory (jpeg)
  set (FLTK_USE_BUILTIN_JPEG TRUE)
  set (FLTK_JPEG_LIBRARIES fltk_jpeg)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/jpeg)
endif ()

set (HAVE_LIBJPEG 1)

#######################################################################
#  Bundled Image Library : libpng
#######################################################################

if (OPTION_USE_SYSTEM_LIBPNG AND PNG_FOUND)

  set (FLTK_USE_BUILTIN_PNG FALSE)
  set (FLTK_PNG_LIBRARIES ${PNG_LIBRARIES})
  include_directories (${PNG_INCLUDE_DIRS})
  add_definitions (${PNG_DEFINITIONS})

  set (_INCLUDE_SAVED ${CMAKE_REQUIRED_INCLUDES})
  list (APPEND CMAKE_REQUIRED_INCLUDES ${PNG_INCLUDE_DIRS})

  # Note: we do not check for <libpng/png.h> explicitly.
  # This is assumed to exist if we have PNG_FOUND and don't find <png.h>

  # FIXME - Force search by unsetting the chache variable. Maybe use
  # FIXME - another cache variable to check for option changes?

  unset (HAVE_PNG_H CACHE) # force search
  check_include_file (png.h HAVE_PNG_H)
  mark_as_advanced (HAVE_PNG_H)

  set (CMAKE_REQUIRED_INCLUDES ${_INCLUDE_SAVED})
  unset (_INCLUDE_SAVED)

else ()

  if (OPTION_USE_SYSTEM_LIBPNG)
    message (STATUS "cannot find system png library - using built-in\n")
  endif ()

  add_subdirectory (png)
  set (FLTK_USE_BUILTIN_PNG TRUE)
  set (FLTK_PNG_LIBRARIES fltk_png)
  set (HAVE_PNG_H 1)
  set (HAVE_PNG_GET_VALID 1)
  set (HAVE_PNG_SET_TRNS_TO_ALPHA 1)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/png)
endif ()

set (HAVE_LIBPNG 1)

#######################################################################
if (UNIX)
  option (OPTION_CREATE_LINKS "create backwards compatibility links" OFF)
  list (APPEND FLTK_LDLIBS -lm)
endif (UNIX)

#######################################################################
if (APPLE)
  if (CMAKE_OSX_SYSROOT)
    list (APPEND FLTK_CFLAGS "-isysroot ${CMAKE_OSX_SYSROOT}")
  endif (CMAKE_OSX_SYSROOT)
endif (APPLE)

# find X11 libraries and headers
set (PATH_TO_XLIBS)
if (NOT APPLE AND NOT WIN32)
  include (FindX11)
  if (X11_FOUND)
    set (USE_X11 1)
    list (APPEND FLTK_LDLIBS -lX11)
    if (X11_Xext_FOUND)
      list (APPEND FLTK_LDLIBS -lXext)
    endif (X11_Xext_FOUND)
    get_filename_component (PATH_TO_XLIBS ${X11_X11_LIB} PATH)
  endif (X11_FOUND)
endif (NOT APPLE AND NOT WIN32)

#######################################################################
option (OPTION_USE_POLL "use poll if available" OFF)
mark_as_advanced (OPTION_USE_POLL)

if (OPTION_USE_POLL)
  CHECK_FUNCTION_EXISTS(poll USE_POLL)
endif (OPTION_USE_POLL)

#######################################################################
option (OPTION_BUILD_SHARED_LIBS
  "Build shared libraries (in addition to static libraries)"
  OFF
)

#######################################################################
option (OPTION_FILESYSTEM_SUPPORT "allow file system support" ON)

option (FLTK_BUILD_TEST     "Build test/demo programs" ON)
option (FLTK_BUILD_EXAMPLES "Build example programs"   OFF)

if (DEFINED OPTION_BUILD_EXAMPLES)
  message (WARNING
    "'OPTION_BUILD_EXAMPLES' is obsolete, please use 'FLTK_BUILD_TEST' instead.")
  message (STATUS
    "To remove this warning, please delete 'OPTION_BUILD_EXAMPLES' from the CMake cache")
endif (DEFINED OPTION_BUILD_EXAMPLES)

#######################################################################
if (DOXYGEN_FOUND)
  option (OPTION_BUILD_HTML_DOCUMENTATION "build html docs" ON)
  option (OPTION_INSTALL_HTML_DOCUMENTATION "install html docs" OFF)

  option (OPTION_INCLUDE_DRIVER_DOCUMENTATION "include driver (developer) docs" OFF)
  mark_as_advanced (OPTION_INCLUDE_DRIVER_DOCUMENTATION)

  if (LATEX_FOUND)
    option (OPTION_BUILD_PDF_DOCUMENTATION "build pdf docs" ON)
    option (OPTION_INSTALL_PDF_DOCUMENTATION "install pdf docs" OFF)
  endif (LATEX_FOUND)
endif (DOXYGEN_FOUND)

if (OPTION_BUILD_HTML_DOCUMENTATION OR OPTION_BUILD_PDF_DOCUMENTATION)
  add_subdirectory (documentation)
endif (OPTION_BUILD_HTML_DOCUMENTATION OR OPTION_BUILD_PDF_DOCUMENTATION)

#######################################################################
# Include optional Cairo support
#######################################################################

option (OPTION_CAIRO "add support for Fl_Cairo_Window" OFF)
option (OPTION_CAIROEXT
  "use FLTK code instrumentation for Cairo extended use" OFF
)

set (FLTK_HAVE_CAIRO 0)
set (FLTK_USE_CAIRO 0)

if (OPTION_CAIRO OR OPTION_CAIROEXT)
  pkg_search_module (PKG_CAIRO cairo)

  # fl_debug_var (PKG_CAIRO_FOUND)

  if (PKG_CAIRO_FOUND)
    set (FLTK_HAVE_CAIRO 1)
    if (OPTION_CAIROEXT)
      set (FLTK_USE_CAIRO 1)
    endif (OPTION_CAIROEXT)
    add_subdirectory (cairo)

    # fl_debug_var (PKG_CAIRO_INCLUDE_DIRS)
    # fl_debug_var (PKG_CAIRO_CFLAGS)
    # fl_debug_var (PKG_CAIRO_STATIC_CFLAGS)
    # fl_debug_var (PKG_CAIRO_LIBRARIES)
    # fl_debug_var (PKG_CAIRO_STATIC_LIBRARIES)

    include_directories (${PKG_CAIRO_INCLUDE_DIRS})

    # Cairo libs and flags for fltk-config

    # Hint: use either PKG_CAIRO_* or PKG_CAIRO_STATIC_* variables to
    # create the list of libraries used to link programs with cairo
    # by running fltk-config --use-cairo --compile ...
    # Currently we're using the non-STATIC variables to link cairo shared.

    set (CAIROLIBS)
    foreach (lib ${PKG_CAIRO_LIBRARIES})
      list (APPEND CAIROLIBS "-l${lib}")
    endforeach()

    string (REPLACE ";" " " CAIROLIBS  "${CAIROLIBS}")
    string (REPLACE ";" " " CAIROFLAGS "${PKG_CAIRO_CFLAGS}")

    # fl_debug_var (FLTK_LDLIBS)
    # fl_debug_var (CAIROFLAGS)
    # fl_debug_var (CAIROLIBS)

  else ()
    message (STATUS "*** Cairo was requested but not found - please check your cairo installation")
    message (STATUS "***   or disable options OPTION_CAIRO and OPTION_CAIRO_EXT.")
    message (FATAL_ERROR "*** Terminating: missing Cairo libs or headers.")
  endif (PKG_CAIRO_FOUND)

endif (OPTION_CAIRO OR OPTION_CAIROEXT)

#######################################################################
set (HAVE_GL LIB_GL OR LIB_MesaGL)

if (HAVE_GL)
   option (OPTION_USE_GL "use OpenGL" ON)
endif (HAVE_GL)

if (OPTION_USE_GL)
  include (FindOpenGL)
  if (APPLE)
    set (HAVE_GL_GLU_H ${HAVE_OPENGL_GLU_H})
  endif (APPLE)
else ()
  set (OPENGL_FOUND FALSE)
endif (OPTION_USE_GL)

if (OPENGL_FOUND)
  set (CMAKE_REQUIRED_INCLUDES ${OPENGL_INCLUDE_DIR}/GL)

  # Set GLLIBS (used in fltk-config).
  # We should probably deduct this from OPENGL_LIBRARIES but it turned
  # out to be difficult since FindOpenGL seems to return different
  # syntax depending on the platform (and maybe also CMake version).
  # Hence we use the following code...

  if (WIN32)
    set (GLLIBS "-lglu32 -lopengl32")
  elseif (APPLE)
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
option (OPTION_LARGE_FILE "enable large file support" ON)

if (OPTION_LARGE_FILE)
  if (NOT MSVC)
    add_definitions (-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)
    list (APPEND FLTK_CFLAGS -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)
  endif (NOT MSVC)
endif (OPTION_LARGE_FILE)

#######################################################################
# Create an option whether we want to check for pthreads.
# We must not do it on Windows unless we run under Cygwin, since we
# always use native threads on Windows (even if libpthread is available).

# Note: HAVE_PTHREAD_H has already been determined in resources.cmake
# before this file is included (or set to 0 for WIN32).

if (WIN32 AND NOT CYGWIN)
  # set (HAVE_PTHREAD_H 0) # (see resources.cmake)
  set (OPTION_USE_THREADS FALSE)
else ()
  option (OPTION_USE_THREADS "use multi-threading with pthreads" ON)
endif (WIN32 AND NOT CYGWIN)

# initialize more variables
set (USE_THREADS 0)
set (HAVE_PTHREAD 0)
set (FLTK_PTHREADS_FOUND FALSE)

if (OPTION_USE_THREADS)

  include (FindThreads)

  if (CMAKE_HAVE_THREADS_LIBRARY)
    add_definitions ("-D_THREAD_SAFE -D_REENTRANT")
    set (USE_THREADS 1)
    set (FLTK_THREADS_FOUND TRUE)
  endif (CMAKE_HAVE_THREADS_LIBRARY)

  if (CMAKE_USE_PTHREADS_INIT AND NOT WIN32)
    set (HAVE_PTHREAD 1)
    if (NOT APPLE)
      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
    endif (NOT APPLE)
    list (APPEND FLTK_LDLIBS -lpthread)
    list (APPEND FLTK_CFLAGS -D_THREAD_SAFE -D_REENTRANT)
    set (FLTK_PTHREADS_FOUND TRUE)
  else()
    set (HAVE_PTHREAD 0)
    set (HAVE_PTHREAD_H 0)
    set (FLTK_PTHREADS_FOUND FALSE)
  endif (CMAKE_USE_PTHREADS_INIT AND NOT WIN32)

else (OPTION_USE_THREADS)

  set (HAVE_PTHREAD_H 0)

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
if (X11_Xinerama_FOUND)
  option (OPTION_USE_XINERAMA "use lib Xinerama" ON)
endif (X11_Xinerama_FOUND)

if (OPTION_USE_XINERAMA)
  set (HAVE_XINERAMA ${X11_Xinerama_FOUND})
  include_directories (${X11_Xinerama_INCLUDE_PATH})
  list (APPEND FLTK_LDLIBS -lXinerama)
  set (FLTK_XINERAMA_FOUND TRUE)
else()
  set (FLTK_XINERAMA_FOUND FALSE)
endif (OPTION_USE_XINERAMA)

#######################################################################
if (X11_Xfixes_FOUND)
  option (OPTION_USE_XFIXES "use lib Xfixes" ON)
endif (X11_Xfixes_FOUND)

if (OPTION_USE_XFIXES)
  set (HAVE_XFIXES ${X11_Xfixes_FOUND})
  include_directories (${X11_Xfixes_INCLUDE_PATH})
  list (APPEND FLTK_LDLIBS -lXfixes)
  set (FLTK_XFIXES_FOUND TRUE)
else()
  set (FLTK_XFIXES_FOUND FALSE)
endif (OPTION_USE_XFIXES)

#######################################################################
if (X11_Xcursor_FOUND)
  option (OPTION_USE_XCURSOR "use lib Xcursor" ON)
endif (X11_Xcursor_FOUND)

if (OPTION_USE_XCURSOR)
  set (HAVE_XCURSOR ${X11_Xcursor_FOUND})
  include_directories (${X11_Xcursor_INCLUDE_PATH})
  list (APPEND FLTK_LDLIBS -lXcursor)
  set (FLTK_XCURSOR_FOUND TRUE)
else()
  set (FLTK_XCURSOR_FOUND FALSE)
endif (OPTION_USE_XCURSOR)

#######################################################################
if (X11_Xft_FOUND)
  option (OPTION_USE_XFT "use lib Xft" ON)
endif (X11_Xft_FOUND)

if (OPTION_USE_XFT)
  set (USE_XFT X11_Xft_FOUND)
  list (APPEND FLTK_LDLIBS -lXft)
  set (FLTK_XFT_FOUND TRUE)
else()
  set (FLTK_XFT_FOUND FALSE)
endif (OPTION_USE_XFT)

#######################################################################
if (X11_Xrender_FOUND)
  option (OPTION_USE_XRENDER "use lib Xrender" ON)
endif (X11_Xrender_FOUND)

if (OPTION_USE_XRENDER)
  set (HAVE_XRENDER ${X11_Xrender_FOUND})
  if (HAVE_XRENDER)
    include_directories (${X11_Xrender_INCLUDE_PATH})
    list (APPEND FLTK_LDLIBS -lXrender)
    set (FLTK_XRENDER_FOUND TRUE)
  else(HAVE_XRENDER)
    set (FLTK_XRENDER_FOUND FALSE)
  endif (HAVE_XRENDER)
else(OPTION_USE_XRENDER)
  set (FLTK_XRENDER_FOUND FALSE)
endif (OPTION_USE_XRENDER)

#######################################################################
if (X11_FOUND)
  option (OPTION_USE_XDBE "use lib Xdbe" ON)
endif (X11_FOUND)

if (OPTION_USE_XDBE AND HAVE_XDBE_H)
  set (HAVE_XDBE 1)
  set (FLTK_XDBE_FOUND TRUE)
else()
  set (FLTK_XDBE_FOUND FALSE)
endif (OPTION_USE_XDBE AND HAVE_XDBE_H)

#######################################################################
set (FL_CFG_NO_FILESYSTEM_SUPPORT TRUE)
if (OPTION_FILESYSTEM_SUPPORT)
  set (FL_CFG_NO_FILESYSTEM_SUPPORT FALSE)
endif (OPTION_FILESYSTEM_SUPPORT)
#######################################################################

#######################################################################
option (CMAKE_SUPPRESS_REGENERATION "suppress rules to re-run CMake on rebuild" OFF)
mark_as_advanced (CMAKE_SUPPRESS_REGENERATION)

#######################################################################
# Debugging ...

if (DEBUG_OPTIONS_CMAKE)
  message (STATUS "") # empty line
  fl_debug_var (WIN32)
  fl_debug_var (LIBS)
  fl_debug_var (GLLIBS)
  fl_debug_var (FLTK_LDLIBS)
  fl_debug_var (OPENGL_FOUND)
  fl_debug_var (OPENGL_INCLUDE_DIR)
  fl_debug_var (OPENGL_LIBRARIES)
  fl_debug_var (CMAKE_MSVC_RUNTIME_LIBRARY)
  message ("--- bundled libraries ---")
  fl_debug_var (OPTION_USE_SYSTEM_LIBJPEG)
  fl_debug_var (OPTION_USE_SYSTEM_LIBPNG)
  fl_debug_var (OPTION_USE_SYSTEM_ZLIB)
  fl_debug_var (FLTK_USE_BUILTIN_JPEG)
  fl_debug_var (FLTK_USE_BUILTIN_PNG)
  fl_debug_var (FLTK_USE_BUILTIN_ZLIB)
  message ("--- X11 ---")
  fl_debug_var (X11_FOUND)
  fl_debug_var (X11_INCLUDE_DIR)
  fl_debug_var (X11_LIBRARIES)
  fl_debug_var (X11_X11_LIB)
  fl_debug_var (X11_X11_INCLUDE_PATH)
  fl_debug_var (X11_Xft_INCLUDE_PATH)
  fl_debug_var (X11_Xft_LIB)
  fl_debug_var (X11_Xft_FOUND)
  fl_debug_var (PATH_TO_XLIBS)
  message (STATUS "[** end of options.cmake **]")
endif (DEBUG_OPTIONS_CMAKE)
unset (DEBUG_OPTIONS_CMAKE)
