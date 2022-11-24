#
# Main CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
# Originally written by Michael Surette
#
# Copyright 1998-2022 by Bill Spitzak and others.
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
# *FIXME* In the current version of FLTK's CMake build files we're
# using 'include_directories()' to define directories that must be
# used in compile commands (typically "-Idirectories").
#
# include_directories() is a global command that affects *all* source
# files in the current directory and all subdirectories. This can lead
# to conflicts and should be replaced with target_include_directories()
# which can be applied to particular targets and source files only.
#
# This could remove some of these potential build conflicts, for
# instance # if the bundled image libs and Cairo or Pango are used
# together (Pango depends on Cairo and Cairo depends on libpng).
# However, this is not a proper solution!
#
# That said, order of "-I..." switches matters, and therefore the
# bundled libraries (png, jpeg, zlib) *must* appear before any other
# include_directories() statements that might introduce conflicts.
# Currently 'resources.cmake' is included before this file and thus
# 'include_directories (${FREETYPE_PATH})' is executed before this
# file but this doesn't matter.
#
# This *MUST* be fixed using target_include_directories() as
# appropriate but this would need a major rework.
#
# Albrecht-S April 6, 2022
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
#  Bundled Compression Library : zlib
#######################################################################

if (OPTION_USE_SYSTEM_ZLIB)
  find_package (ZLIB)
endif ()

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

if (OPTION_USE_SYSTEM_LIBPNG)
  find_package (PNG)
endif ()

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
  if (NOT APPLE)
    option (OPTION_USE_WAYLAND "support both Wayland and X11 backends" ON)
  endif (NOT APPLE)
  if (OPTION_USE_WAYLAND)
    pkg_check_modules(WLDCLIENT wayland-client)
    pkg_check_modules(WLDCURSOR wayland-cursor)
    pkg_check_modules(WLDPROTO wayland-protocols)
    pkg_check_modules(XKBCOMMON xkbcommon)
    pkg_check_modules(DBUS dbus-1)
    if (NOT(DBUS_FOUND AND WLDCLIENT_FOUND AND WLDCURSOR_FOUND AND WLDPROTO_FOUND AND XKBCOMMON_FOUND))
      message (STATUS "Not all software modules 'wayland-client wayland-cursor wayland-protocols xkbcommon dbus-1' are present")
      message (STATUS "Consequently, OPTION_USE_WAYLAND is set to OFF.")
      unset (OPTION_USE_WAYLAND CACHE)
      set (OPTION_USE_WAYLAND 0)
    endif (NOT(DBUS_FOUND AND WLDCLIENT_FOUND AND WLDCURSOR_FOUND AND WLDPROTO_FOUND AND XKBCOMMON_FOUND))
  endif (OPTION_USE_WAYLAND)

  if (OPTION_USE_WAYLAND)
    option (OPTION_WAYLAND_ONLY "support Wayland backend only" OFF)
    set (FLTK_USE_WAYLAND 1)
    if (NOT OPTION_WAYLAND_ONLY)
      include (FindX11)
    endif (NOT OPTION_WAYLAND_ONLY)
    if (X11_FOUND)
      set (FLTK_USE_X11 1) # to build a hybrid Wayland/X11 library
    else ()
      set (FLTK_USE_X11 0) # to build a Wayland-only library
    endif (X11_FOUND)
    unset (OPTION_USE_CAIRO CACHE)
    set (OPTION_USE_CAIRO TRUE CACHE BOOL "all drawing to X11 windows uses Cairo")
    option (OPTION_USE_SYSTEM_LIBDECOR "use libdecor from the system" OFF)
    unset (OPTION_USE_XRENDER CACHE)
    unset (OPTION_USE_XINERAMA CACHE)
    unset (OPTION_USE_XFT CACHE)
    unset (OPTION_USE_XCURSOR CACHE)
    unset (OPTION_USE_XFIXES CACHE)
    if (X11_FOUND)
      if (NOT X11_Xfixes_FOUND)
        message(WARNING "Install development headers for libXfixes (e.g., libxfixes-dev)")
      endif()
      set (HAVE_XFIXES 1)
      if (NOT X11_Xrender_FOUND)
        message(WARNING "Install development headers for libXrender (e.g., libxrender-dev)")
      endif()
      set (HAVE_XRENDER 1)
      if (NOT X11_Xft_FOUND)
        message(WARNING "Install development headers for libXft (e.g., libxft-dev)")
      endif()
      set (USE_XFT 1)
      if (NOT X11_Xcursor_FOUND)
        message(WARNING "Install development headers for libXcursor (e.g., libxcursor-dev)")
      endif()
      set (HAVE_XCURSOR 1)
      if (NOT X11_Xinerama_FOUND)
        message(WARNING "Install development headers for libXinerama (e.g., libxinerama-dev)")
      endif()
      set (HAVE_XINERAMA 1)
      if (NOT (X11_Xfixes_FOUND AND X11_Xrender_FOUND AND X11_Xft_FOUND AND X11_Xcursor_FOUND
          AND X11_Xinerama_FOUND))
        message (FATAL_ERROR "*** Terminating: one or more required software package(s) missing.")
      endif ()
    endif (X11_FOUND)
    unset (OPTION_USE_PANGO CACHE)
    set (OPTION_USE_PANGO TRUE CACHE BOOL "use lib Pango")
    if (OPTION_USE_SYSTEM_LIBDECOR)
      pkg_check_modules(SYSTEM_LIBDECOR libdecor-0)
      if (NOT SYSTEM_LIBDECOR_FOUND)
        set (OPTION_USE_SYSTEM_LIBDECOR OFF)
      endif (NOT SYSTEM_LIBDECOR_FOUND)
    endif (OPTION_USE_SYSTEM_LIBDECOR)

    option (OPTION_ALLOW_GTK_PLUGIN "Allow to use libdecor's GTK plugin" ON)

  endif (OPTION_USE_WAYLAND)
endif (UNIX)

if (WIN32)
  option (OPTION_USE_GDIPLUS "use GDI+ when possible for antialiased graphics" ON)
  if (OPTION_USE_GDIPLUS)
    set (USE_GDIPLUS TRUE)
    if (NOT MSVC)
      list (APPEND FLTK_LDLIBS "-lgdiplus")
    endif (NOT MSVC)
  endif (OPTION_USE_GDIPLUS)
endif (WIN32)

#######################################################################
if (APPLE)
  option (OPTION_APPLE_X11 "use X11" OFF)
  if (CMAKE_OSX_SYSROOT)
    list (APPEND FLTK_CFLAGS "-isysroot ${CMAKE_OSX_SYSROOT}")
  endif (CMAKE_OSX_SYSROOT)
endif (APPLE)

# find X11 libraries and headers
set (PATH_TO_XLIBS)
if ((NOT APPLE OR OPTION_APPLE_X11) AND NOT WIN32 AND NOT OPTION_USE_WAYLAND)
  include (FindX11)
  if (X11_FOUND)
    set (FLTK_USE_X11 1)
    list (APPEND FLTK_LDLIBS -lX11)
    if (X11_Xext_FOUND)
      list (APPEND FLTK_LDLIBS -lXext)
    endif (X11_Xext_FOUND)
    get_filename_component (PATH_TO_XLIBS ${X11_X11_LIB} PATH)
  endif (X11_FOUND)
endif ((NOT APPLE OR OPTION_APPLE_X11) AND NOT WIN32 AND NOT OPTION_USE_WAYLAND)

if (OPTION_APPLE_X11)
  if (NOT(${CMAKE_SYSTEM_VERSION} VERSION_LESS 17.0.0)) # a.k.a. macOS version ≥ 10.13
    list (APPEND FLTK_CFLAGS "-D_LIBCPP_HAS_THREAD_API_PTHREAD")
  endif (NOT(${CMAKE_SYSTEM_VERSION} VERSION_LESS 17.0.0))
  include_directories (AFTER SYSTEM /opt/X11/include/freetype2)
  include_directories (AFTER SYSTEM /opt/X11/include) # for Xft.h
  if (PATH_TO_XLIBS)
    set (LDFLAGS "-L${PATH_TO_XLIBS} ${LDFLAGS}")
  endif (PATH_TO_XLIBS)
  if (X11_INCLUDE_DIR)
    set (TEMP_INCLUDE_DIR ${X11_INCLUDE_DIR})
    list (TRANSFORM TEMP_INCLUDE_DIR PREPEND "-I")
    list (APPEND FLTK_CFLAGS "${TEMP_INCLUDE_DIR}")
  endif (X11_INCLUDE_DIR)
endif (OPTION_APPLE_X11)

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
option (OPTION_PRINT_SUPPORT "allow print support" ON)
option (OPTION_FILESYSTEM_SUPPORT "allow file system support" ON)

option (FLTK_BUILD_FLUID    "Build FLUID"              ON)
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
set (FLTK_HAVE_CAIROEXT 0)

if (OPTION_CAIRO OR OPTION_CAIROEXT)
  pkg_search_module (PKG_CAIRO cairo)

  # fl_debug_var (PKG_CAIRO_FOUND)

  if (PKG_CAIRO_FOUND)
    set (FLTK_HAVE_CAIRO 1)
    if (OPTION_CAIROEXT)
      set (FLTK_HAVE_CAIROEXT 1)
    endif (OPTION_CAIROEXT)
    add_subdirectory (cairo)

    if (0)
      fl_debug_var (PKG_CAIRO_INCLUDE_DIRS)
      fl_debug_var (PKG_CAIRO_CFLAGS)
      fl_debug_var (PKG_CAIRO_LIBRARIES)
      fl_debug_var (PKG_CAIRO_LIBRARY_DIRS)
      fl_debug_var (PKG_CAIRO_STATIC_INCLUDE_DIRS)
      fl_debug_var (PKG_CAIRO_STATIC_CFLAGS)
      fl_debug_var (PKG_CAIRO_STATIC_LIBRARIES)
      fl_debug_var (PKG_CAIRO_STATIC_LIBRARY_DIRS)
    endif()

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
option (OPTION_USE_SVG "read/write SVG files" ON)

if (OPTION_USE_SVG)
  set (FLTK_USE_SVG 1)
endif (OPTION_USE_SVG)

#######################################################################
set (HAVE_GL LIB_GL OR LIB_MesaGL)

if (HAVE_GL)
   option (OPTION_USE_GL "use OpenGL" ON)
endif (HAVE_GL)

if (OPTION_USE_GL)
  if (OPTION_USE_WAYLAND)
    pkg_check_modules(WLD_EGL wayland-egl)
    pkg_check_modules(PKG_EGL egl)
    pkg_check_modules(PKG_GL gl)
    if (NOT (WLD_EGL_FOUND AND PKG_EGL_FOUND AND PKG_GL_FOUND))
      message (STATUS "Modules 'wayland-egl, egl, and gl' are required to build for the Wayland backend.")
      message (FATAL_ERROR "*** Aborting ***")
    endif (NOT (WLD_EGL_FOUND AND PKG_EGL_FOUND AND PKG_GL_FOUND))
  endif (OPTION_USE_WAYLAND)
  if (OPTION_APPLE_X11)
    set (OPENGL_FOUND TRUE)
    find_library(OPENGL_LIB GL)
    get_filename_component (PATH_TO_GLLIB ${OPENGL_LIB} DIRECTORY)
    # with GL, must use XQuartz libX11 else "Insufficient GL support"
    set (OPENGL_LIBRARIES -L${PATH_TO_GLLIB} -lX11 -lGLU -lGL)
    unset(HAVE_GL_GLU_H CACHE)
    find_file (HAVE_GL_GLU_H GL/glu.h PATHS ${X11_INCLUDE_DIR})
  else()
    include (FindOpenGL)
    if (APPLE)
      set (HAVE_GL_GLU_H ${HAVE_OPENGL_GLU_H})
    endif (APPLE)
  endif (OPTION_APPLE_X11)
else ()
  set (OPENGL_FOUND FALSE)
  set (HAVE_GL FALSE)
  set (HAVE_GL_GLU_H FALSE)
  set (HAVE_GLXGETPROCADDRESSARB FALSE)
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
  elseif (APPLE AND NOT OPTION_APPLE_X11)
    set (GLLIBS "-framework OpenGL")
  elseif (OPTION_USE_WAYLAND)
    set (GLLIBS "-lwayland-egl -lEGL -lGLU -lGL")
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
  option (OPTION_USE_PANGO "use lib Pango" OFF)
  if (NOT OPTION_USE_WAYLAND)
    option (OPTION_USE_CAIRO "all drawing to X11 windows uses Cairo" OFF)
  endif (NOT OPTION_USE_WAYLAND)
endif (X11_Xft_FOUND)

# test option compatibility: Cairo for Xlib requires Pango
if (OPTION_USE_CAIRO)
    unset (OPTION_USE_PANGO CACHE)
    set (OPTION_USE_PANGO TRUE CACHE BOOL "use lib Pango")
endif (OPTION_USE_CAIRO)

if (OPTION_USE_PANGO OR OPTION_USE_CAIRO)
  if (OPTION_USE_WAYLAND OR OPTION_APPLE_X11)
    set (USE_PANGOXFT false)
  else ()
    set (USE_PANGOXFT true)
  endif (OPTION_USE_WAYLAND OR OPTION_APPLE_X11)
endif (OPTION_USE_PANGO OR OPTION_USE_CAIRO)

# test option compatibility: Pango requires Xft
if (USE_PANGOXFT)
  if (NOT X11_Xft_FOUND)
    message (STATUS "Pango requires Xft but Xft library or headers could not be found.")
    message (STATUS "Please install Xft development files and try again or disable OPTION_USE_PANGO.")
    message (FATAL_ERROR "*** Aborting ***")
  else ()
    if (NOT OPTION_USE_XFT)
      message (STATUS "Pango requires Xft but usage of Xft was disabled.")
      message (STATUS "Please enable OPTION_USE_XFT and try again or disable OPTION_USE_PANGO.")
      message (FATAL_ERROR "*** Aborting ***")
    endif (NOT OPTION_USE_XFT)
  endif (NOT X11_Xft_FOUND)
endif (USE_PANGOXFT)

#######################################################################
if ((X11_Xft_FOUND OR NOT USE_PANGOXFT) AND OPTION_USE_PANGO)
  pkg_check_modules(CAIRO cairo)
  if (USE_PANGOXFT)
    pkg_check_modules(PANGOXFT pangoxft)
  endif (USE_PANGOXFT)
  pkg_check_modules(PANGOCAIRO pangocairo)

  if ((PANGOXFT_FOUND OR NOT USE_PANGOXFT) AND PANGOCAIRO_FOUND AND CAIRO_FOUND)
    if (USE_PANGOXFT)
      include_directories (${PANGOXFT_INCLUDE_DIRS})
    else ()
      include_directories (${PANGOCAIRO_INCLUDE_DIRS})
    endif (USE_PANGOXFT)
    include_directories (${CAIRO_INCLUDE_DIRS})

    find_library (HAVE_LIB_PANGO pango-1.0 ${CMAKE_LIBRARY_PATH})
    if (USE_PANGOXFT)
      find_library (HAVE_LIB_PANGOXFT pangoxft-1.0 ${CMAKE_LIBRARY_PATH})
    endif (USE_PANGOXFT)
    find_library (HAVE_LIB_PANGOCAIRO pangocairo-1.0 ${CMAKE_LIBRARY_PATH})
    find_library (HAVE_LIB_CAIRO cairo ${CMAKE_LIBRARY_PATH})
    find_library (HAVE_LIB_GOBJECT gobject-2.0 ${CMAKE_LIBRARY_PATH})

    mark_as_advanced (HAVE_LIB_PANGO)
    if (USE_PANGOXFT)
      mark_as_advanced (HAVE_LIB_PANGOXFT)
    endif (USE_PANGOXFT)
    mark_as_advanced (HAVE_LIB_PANGOCAIRO)
    mark_as_advanced (HAVE_LIB_CAIRO)
    mark_as_advanced (HAVE_LIB_GOBJECT)

    set (USE_PANGO TRUE)

    # add required libraries to fltk-config ...
    if (USE_PANGOXFT)
      list (APPEND FLTK_LDLIBS ${PANGOXFT_LDFLAGS})
    endif (USE_PANGOXFT)
    list (APPEND FLTK_LDLIBS ${PANGOCAIRO_LDFLAGS})
    list (APPEND FLTK_LDLIBS ${CAIRO_LDFLAGS})

    # *FIXME* Libraries should not be added explicitly if possible
    if (OPTION_USE_WAYLAND)
      list (APPEND FLTK_LDLIBS -lgtk-3 -lgdk-3 -lgio-2.0)
      if (NOT OPTION_WAYLAND_ONLY)
        list (APPEND FLTK_LDLIBS -lX11)
      endif (NOT OPTION_WAYLAND_ONLY)
    endif (OPTION_USE_WAYLAND)

    if (APPLE)
      get_filename_component(PANGO_L_PATH ${HAVE_LIB_PANGO} PATH)
      set (LDFLAGS "${LDFLAGS} -L${PANGO_L_PATH}")
    endif (APPLE)

  else ()

    # this covers Debian, Ubuntu, FreeBSD, NetBSD, Darwin
    if (APPLE AND OPTION_APPLE_X11)
      find_file(FINK_PREFIX NAMES /opt/sw /sw)
      list (APPEND CMAKE_INCLUDE_PATH  ${FINK_PREFIX}/include)
      include_directories (${FINK_PREFIX}/include/cairo)
      list (APPEND CMAKE_LIBRARY_PATH  ${FINK_PREFIX}/lib)
    endif (APPLE AND OPTION_APPLE_X11)

    find_file(HAVE_PANGO_H pango-1.0/pango/pango.h ${CMAKE_INCLUDE_PATH})
    find_file(HAVE_PANGOXFT_H pango-1.0/pango/pangoxft.h ${CMAKE_INCLUDE_PATH})

    if (HAVE_PANGO_H AND HAVE_PANGOXFT_H)
      find_library(HAVE_LIB_PANGO pango-1.0 ${CMAKE_LIBRARY_PATH})
      find_library(HAVE_LIB_PANGOXFT pangoxft-1.0 ${CMAKE_LIBRARY_PATH})
      if (APPLE)
        set (HAVE_LIB_GOBJECT TRUE)
      else()
        find_library(HAVE_LIB_GOBJECT gobject-2.0 ${CMAKE_LIBRARY_PATH})
      endif (APPLE)
    endif (HAVE_PANGO_H AND HAVE_PANGOXFT_H)

    if (HAVE_LIB_PANGO AND HAVE_LIB_PANGOXFT AND HAVE_LIB_GOBJECT)
      set (USE_PANGO TRUE)
      # remove last 3 components of HAVE_PANGO_H and put in PANGO_H_PREFIX
      get_filename_component(PANGO_H_PREFIX ${HAVE_PANGO_H} PATH)
      get_filename_component(PANGO_H_PREFIX ${PANGO_H_PREFIX} PATH)
      get_filename_component(PANGO_H_PREFIX ${PANGO_H_PREFIX} PATH)

      get_filename_component(PANGOLIB_DIR ${HAVE_LIB_PANGO} PATH)
      # glib.h is usually in ${PANGO_H_PREFIX}/glib-2.0/ ...
      find_path(GLIB_H_PATH glib.h
                PATHS ${PANGO_H_PREFIX}/glib-2.0
                      ${PANGO_H_PREFIX}/glib/glib-2.0)
      include_directories (${PANGO_H_PREFIX}/pango-1.0 ${GLIB_H_PATH} ${PANGOLIB_DIR}/glib-2.0/include)

      # *FIXME* Libraries should not be added explicitly if possible
      list (APPEND FLTK_LDLIBS -lpango-1.0 -lpangoxft-1.0 -lgobject-2.0)

    endif (HAVE_LIB_PANGO AND HAVE_LIB_PANGOXFT AND HAVE_LIB_GOBJECT)
  endif ((PANGOXFT_FOUND OR NOT USE_PANGOXFT) AND PANGOCAIRO_FOUND AND CAIRO_FOUND)

  if (USE_PANGO AND (OPTION_USE_CAIRO OR OPTION_USE_WAYLAND))
    set (FLTK_USE_CAIRO 1)
    # fl_debug_var (FLTK_USE_CAIRO)
  endif (USE_PANGO AND (OPTION_USE_CAIRO OR OPTION_USE_WAYLAND))

endif ((X11_Xft_FOUND OR NOT USE_PANGOXFT) AND OPTION_USE_PANGO)

if (OPTION_USE_WAYLAND AND NOT OPTION_USE_SYSTEM_LIBDECOR)

  # Note: Disable OPTION_ALLOW_GTK_PLUGIN to get cairo titlebars rather than GTK
  if (OPTION_ALLOW_GTK_PLUGIN)
    pkg_check_modules(GTK gtk+-3.0)
    if (GTK_FOUND)
      include_directories (${GTK_INCLUDE_DIRS})
    else ()
      message(WARNING "Installation of the development files for the GTK library "
      "(e.g., libgtk-3-dev) is recommended when using the gnome desktop.")
    endif (GTK_FOUND)
  endif (OPTION_ALLOW_GTK_PLUGIN)

endif (OPTION_USE_WAYLAND AND NOT OPTION_USE_SYSTEM_LIBDECOR)

if (OPTION_USE_XFT)
  set (USE_XFT X11_Xft_FOUND)
  list (APPEND FLTK_LDLIBS -lXft)
  set (FLTK_XFT_FOUND TRUE)
  if (APPLE AND OPTION_APPLE_X11)
    find_library(LIB_fontconfig fontconfig "/opt/X11/lib")
  endif (APPLE AND OPTION_APPLE_X11)
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
set (FL_NO_PRINT_SUPPORT FALSE)
if (X11_FOUND AND NOT OPTION_PRINT_SUPPORT)
  set (FL_NO_PRINT_SUPPORT TRUE)
endif (X11_FOUND AND NOT OPTION_PRINT_SUPPORT)
#######################################################################

#######################################################################
set (FL_CFG_NO_FILESYSTEM_SUPPORT TRUE)
if (OPTION_FILESYSTEM_SUPPORT)
  set (FL_CFG_NO_FILESYSTEM_SUPPORT FALSE)
endif (OPTION_FILESYSTEM_SUPPORT)
#######################################################################

#######################################################################
option (OPTION_USE_KDIALOG "Fl_Native_File_Chooser may run kdialog" ON)
if (OPTION_USE_KDIALOG)
  set (USE_KDIALOG 1)
else ()
  set (USE_KDIALOG 0)
endif (OPTION_USE_KDIALOG)
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
