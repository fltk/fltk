#
# Main CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
# Originally written by Michael Surette
#
# Copyright 1998-2025 by Bill Spitzak and others.
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
# instance if the bundled image libs and Cairo or Pango are used
# together (Pango depends on Cairo and Cairo depends on libpng).
# However, this is not a proper solution!
#
# That said, order of "-I..." switches matters, and therefore the
# bundled libraries (png, jpeg, zlib) *must* appear before any other
# include_directories() statements that might introduce conflicts.
# Currently 'resources.cmake' is included before this file and thus
# 'include_directories(${FREETYPE_PATH})' is executed before this
# file but this doesn't matter.
#
# This *MUST* be fixed using target_include_directories() as
# appropriate but this would need a major rework.
#
# Albrecht-S April 6, 2022
#
#######################################################################

set(DEBUG_OPTIONS_CMAKE 0)
if(DEBUG_OPTIONS_CMAKE)
  message(STATUS "[** options.cmake **]")
  fl_debug_var(WIN32)
  fl_debug_var(FLTK_LDLIBS)
endif(DEBUG_OPTIONS_CMAKE)

#######################################################################
# options
#######################################################################
set(FLTK_OPTION_OPTIM ""
  CACHE STRING
  "custom optimization flags"
)
# *FIXME* add_definitions()
add_definitions(${FLTK_OPTION_OPTIM})

#######################################################################
set(FLTK_ARCHFLAGS ""
  CACHE STRING
  "custom architecture flags"
)
# *FIXME* add_definitions()
add_definitions(${FLTK_ARCHFLAGS})

#######################################################################
set(FLTK_ABI_VERSION ""
  CACHE STRING
  "FLTK ABI Version FL_ABI_VERSION: 1xxyy for 1.x.y (xx,yy with leading zero)"
)
set(FL_ABI_VERSION ${FLTK_ABI_VERSION})

#######################################################################
#  Select MSVC (Visual Studio) Runtime: DLL (/MDx) or static (/MTx)
#  where x = 'd' for Debug builds, empty ('') for non-Debug builds.
#  Note: this might be handled better by the 'MSVC_RUNTIME_LIBRARY'
#  target property for each target rather than setting a global
#  CMake variable - but this version does the latter.
#  This also applies when using LLVM/clang on Windows (#1058).
#######################################################################

if(WIN32 AND NOT MINGW AND NOT MSYS)
  option(FLTK_MSVC_RUNTIME_DLL "use MSVC Runtime-DLL (/MDx)" ON)
  if(FLTK_MSVC_RUNTIME_DLL)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif()
else(WIN32 AND NOT MINGW AND NOT MSYS)
  # suppress CMake warning if the user sets FLTK_MSVC_RUNTIME_DLL on other platforms
  if(DEFINED FLTK_MSVC_RUNTIME_DLL)
    unset(FLTK_MSVC_RUNTIME_DLL)
    unset(FLTK_MSVC_RUNTIME_DLL CACHE)
  endif()
endif(WIN32 AND NOT MINGW AND NOT MSYS)

#######################################################################

if(APPLE)
  option(FLTK_BACKEND_X11 "use X11" OFF)
  if(CMAKE_OSX_SYSROOT)
    list(APPEND FLTK_CFLAGS "-isysroot ${CMAKE_OSX_SYSROOT}")
  endif(CMAKE_OSX_SYSROOT)
elseif(UNIX)
  option(FLTK_BACKEND_X11 "use X11" ON)
endif(APPLE)

#######################################################################
#  Bundled Library Options
#######################################################################

if(WIN32 OR (APPLE AND NOT FLTK_BACKEND_X11))
  option(FLTK_USE_SYSTEM_LIBJPEG "use system libjpeg" OFF)
  option(FLTK_USE_SYSTEM_LIBPNG  "use system libpng"  OFF)
  option(FLTK_USE_SYSTEM_ZLIB    "use system zlib"    OFF)
else()
  option(FLTK_USE_SYSTEM_LIBJPEG "use system libjpeg" ON)
  option(FLTK_USE_SYSTEM_LIBPNG  "use system libpng"  ON)
  option(FLTK_USE_SYSTEM_ZLIB    "use system zlib"    ON)
endif()

# Set default values of internal build options

set(FLTK_USE_BUNDLED_JPEG FALSE)
set(FLTK_USE_BUNDLED_PNG  FALSE)
set(FLTK_USE_BUNDLED_ZLIB FALSE)

# Collect libraries to build fltk_images (starting empty)

set(FLTK_IMAGE_LIBRARIES "")

#######################################################################
#  Ensure that png and zlib are both system or both local for compatibility
#######################################################################

if(FLTK_USE_SYSTEM_ZLIB)
  find_package(ZLIB)
  if(NOT ZLIB_FOUND)
    set(FLTK_USE_BUNDLED_ZLIB TRUE)
  endif()
else()
  set(FLTK_USE_BUNDLED_ZLIB TRUE)
endif()

if(FLTK_USE_SYSTEM_LIBPNG AND NOT FLTK_USE_BUNDLED_ZLIB)
  find_package(PNG)
  if(NOT PNG_FOUND)
    set(FLTK_USE_BUNDLED_PNG  TRUE)
    set(FLTK_USE_BUNDLED_ZLIB TRUE)
  endif()
else()
  set(FLTK_USE_BUNDLED_PNG  TRUE)
  set(FLTK_USE_BUNDLED_ZLIB TRUE)
endif()

# Issue warnings if we deviate from the user's choice

if(FLTK_USE_SYSTEM_LIBPNG AND FLTK_USE_BUNDLED_PNG)
  message(STATUS "System PNG or ZLIB not usable, falling back to local PNG for compatibility.")
endif()

if(FLTK_USE_SYSTEM_ZLIB AND FLTK_USE_BUNDLED_ZLIB)
  message(STATUS "System PNG or ZLIB not usable, falling back to local ZLIB for compatibility.")
endif()

#######################################################################
#  Bundled Compression Library : zlib
#######################################################################

if(FLTK_USE_BUNDLED_ZLIB)

  add_subdirectory(zlib)

  set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib)

  # FIXME - include_directories()
  include_directories(${ZLIB_INCLUDE_DIR})

endif()

set(HAVE_LIBZ 1)

#######################################################################
#  Bundled Image Library : libpng
#######################################################################

if(FLTK_USE_BUNDLED_PNG)

  add_subdirectory(png)
  set(FLTK_PNG_LIBRARIES fltk::png)
  list(APPEND FLTK_IMAGE_LIBRARIES fltk::png)

  # Definitions for 'config.h' - do we still need this?
  # See also png/CMakeLists.txt (target_compile_definitions).
  # Note: config.h is generated by either configure or CMake,
  # hence we should support it in 1.4.x (may be changed in 1.5.0)

  set(HAVE_PNG_H 1)
  set(HAVE_PNG_GET_VALID 1)
  set(HAVE_PNG_SET_TRNS_TO_ALPHA 1)

  # *FIXME* include_directories()
  include_directories(${FLTK_SOURCE_DIR}/png)

else() # use system libpng and zlib

  set(FLTK_PNG_LIBRARIES ${PNG_LIBRARIES})
  list(APPEND FLTK_IMAGE_LIBRARIES ${PNG_LIBRARIES})

  # *FIXME* include_directories()
  include_directories(${PNG_INCLUDE_DIRS})

  set(_INCLUDE_SAVED ${CMAKE_REQUIRED_INCLUDES})
  list(APPEND CMAKE_REQUIRED_INCLUDES ${PNG_INCLUDE_DIRS})

  # Note: we do not check for <libpng/png.h> explicitly.
  # This is assumed to exist if we have PNG_FOUND and don't find <png.h>

  # FIXME - Force search by unsetting the chache variable. Maybe use
  # FIXME - another cache variable to check for option changes?
  # unset(HAVE_PNG_H CACHE) # force search

  check_include_file(png.h HAVE_PNG_H)
  mark_as_advanced(HAVE_PNG_H)

  set(CMAKE_REQUIRED_INCLUDES ${_INCLUDE_SAVED})
  unset(_INCLUDE_SAVED)

endif()

set(HAVE_LIBPNG 1)

#######################################################################
#  Bundled Image Library : libjpeg
#######################################################################

if(FLTK_USE_SYSTEM_LIBJPEG)
  find_package(JPEG)
  if(NOT JPEG_FOUND)
    set(FLTK_USE_BUNDLED_JPEG TRUE)
    message(STATUS "cannot find system jpeg library - using built-in")
  endif()
else()
  set(FLTK_USE_BUNDLED_JPEG TRUE)
endif()

if(FLTK_USE_BUNDLED_JPEG)

  add_subdirectory(jpeg)
  set(FLTK_JPEG_LIBRARIES fltk::jpeg)
  # list(APPEND FLTK_IMAGE_LIBRARIES fltk::jpeg)

  # *FIXME* include_directories
  include_directories(${FLTK_SOURCE_DIR}/jpeg)

else()

  set(FLTK_JPEG_LIBRARIES ${JPEG_LIBRARIES})
  list(APPEND FLTK_IMAGE_LIBRARIES ${JPEG_LIBRARIES})

endif()

set(HAVE_LIBJPEG 1)

#######################################################################

if(UNIX)
  option(FLTK_INSTALL_LINKS "create backwards compatibility links" OFF)
  list(APPEND FLTK_LDLIBS -lm)
  if(NOT APPLE)
    option(FLTK_BACKEND_WAYLAND "support the Wayland backend" ON)
  endif(NOT APPLE)
  if(FLTK_BACKEND_WAYLAND)
    if(NOT PKG_CONFIG_FOUND)
      message(FATAL_ERROR "Option FLTK_BACKEND_WAYLAND requires availability of pkg-config on the build machine.")
    endif(NOT PKG_CONFIG_FOUND)
    pkg_check_modules(WLDCLIENT IMPORTED_TARGET wayland-client>=1.18)
    pkg_check_modules(WLDCURSOR IMPORTED_TARGET wayland-cursor)
    pkg_check_modules(WLDPROTO IMPORTED_TARGET wayland-protocols>=1.15)
    pkg_check_modules(XKBCOMMON IMPORTED_TARGET xkbcommon)
    if(NOT(WLDCLIENT_FOUND AND WLDCURSOR_FOUND AND WLDPROTO_FOUND AND XKBCOMMON_FOUND))
      message(STATUS "Not all software modules 'wayland-client>=1.18 wayland-cursor wayland-protocols>=1.15 xkbcommon' are present")
      message(STATUS "Consequently, FLTK_BACKEND_WAYLAND is turned off.")
      unset(FLTK_BACKEND_WAYLAND CACHE)
      set(FLTK_BACKEND_WAYLAND 0)
    endif(NOT(WLDCLIENT_FOUND AND WLDCURSOR_FOUND AND WLDPROTO_FOUND AND XKBCOMMON_FOUND))
  endif(FLTK_BACKEND_WAYLAND)

  if(FLTK_BACKEND_WAYLAND)
    set(FLTK_USE_WAYLAND 1)
    if(FLTK_BACKEND_X11)
      include(FindX11)
    endif()
    if(FLTK_BACKEND_X11 AND X11_FOUND)
      set(FLTK_USE_X11 1) # build a hybrid Wayland/X11 library
    else()
      set(FLTK_USE_X11 0) # build a Wayland-only library
    endif()
    unset(FLTK_GRAPHICS_CAIRO CACHE)
    set(FLTK_GRAPHICS_CAIRO TRUE CACHE BOOL "all drawing to X11 windows uses Cairo")
    option(FLTK_USE_SYSTEM_LIBDECOR "use libdecor from the system" ON)
    set(USE_SYSTEM_LIBDECOR 1)
    unset(FLTK_USE_XRENDER CACHE)
    unset(FLTK_USE_XINERAMA CACHE)
    unset(FLTK_USE_XFT CACHE)
    unset(FLTK_USE_XCURSOR CACHE)
    unset(FLTK_USE_XFIXES CACHE)
    if(X11_FOUND)
      if(NOT X11_Xfixes_FOUND)
        message(WARNING "Install development headers for libXfixes (e.g., libxfixes-dev)")
      endif()
      set(HAVE_XFIXES 1)
      if(NOT X11_Xrender_FOUND)
        message(WARNING "Install development headers for libXrender (e.g., libxrender-dev)")
      endif()
      set(HAVE_XRENDER 1)
      if(NOT X11_Xft_FOUND)
        message(WARNING "Install development headers for libXft (e.g., libxft-dev)")
      endif()
      if(NOT X11_Xcursor_FOUND)
        message(WARNING "Install development headers for libXcursor (e.g., libxcursor-dev)")
      endif()
      set(HAVE_XCURSOR 1)
      if(NOT X11_Xinerama_FOUND)
        message(WARNING "Install development headers for libXinerama (e.g., libxinerama-dev)")
      endif()
      set(HAVE_XINERAMA 1)
      if(NOT (X11_Xfixes_FOUND AND X11_Xrender_FOUND AND X11_Xft_FOUND AND X11_Xcursor_FOUND
          AND X11_Xinerama_FOUND))
        message(FATAL_ERROR "*** Terminating: one or more required software package(s) missing.")
      endif()
    endif(X11_FOUND)
    unset(FLTK_USE_PANGO CACHE)
    set(FLTK_USE_PANGO TRUE CACHE BOOL "use lib Pango")
    if(FLTK_USE_SYSTEM_LIBDECOR)
      pkg_check_modules(SYSTEM_LIBDECOR IMPORTED_TARGET libdecor-0>=0.2.0 QUIET)
      if(NOT SYSTEM_LIBDECOR_FOUND)
        message(STATUS "Warning: system libdecor doesn't satisfy version ≥ 0.2.0,")
        message(STATUS "         using bundled libdecor library instead.")
        set(USE_SYSTEM_LIBDECOR 0)
      else()
        pkg_get_variable(LIBDECOR_LIBDIR libdecor-0 libdir)
        set(LIBDECOR_PLUGIN_DIR ${LIBDECOR_LIBDIR}/libdecor/plugins-1)
        if(EXISTS ${LIBDECOR_PLUGIN_DIR} AND IS_DIRECTORY ${LIBDECOR_PLUGIN_DIR})
          set(LIBDECOR_PLUGIN_DIR "\"${LIBDECOR_PLUGIN_DIR}\"" )
        else()
          set(USE_SYSTEM_LIBDECOR 0)
        endif()
      endif(NOT SYSTEM_LIBDECOR_FOUND)
    else()
      set(USE_SYSTEM_LIBDECOR 0)
    endif(FLTK_USE_SYSTEM_LIBDECOR)

    if(USE_SYSTEM_LIBDECOR)
      set(FLTK_USE_LIBDECOR_GTK ON)
    else()
      option(FLTK_USE_LIBDECOR_GTK "Allow to use libdecor's GTK plugin" ON)
    endif(USE_SYSTEM_LIBDECOR)

    if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "FreeBSD")
      check_include_file(linux/input.h LINUX_INPUT_H)
      if(NOT LINUX_INPUT_H)
        message(FATAL_ERROR "Required include file 'linux/input.h' is missing. Please install package 'evdev-proto'")
      endif(NOT LINUX_INPUT_H)
    endif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "FreeBSD")

  endif(FLTK_BACKEND_WAYLAND)
endif(UNIX)

if(WIN32)
  option(FLTK_GRAPHICS_GDIPLUS "use GDI+ when possible for antialiased graphics" ON)
  if(FLTK_GRAPHICS_GDIPLUS)
    set(USE_GDIPLUS TRUE)
    if(NOT MSVC)
      list(APPEND FLTK_LDLIBS "-lgdiplus")
    endif(NOT MSVC)
  endif(FLTK_GRAPHICS_GDIPLUS)
endif(WIN32)

#######################################################################

# find X11 libraries and headers
set(PATH_TO_XLIBS)
if(FLTK_BACKEND_X11)
  include(FindX11)
  if(X11_FOUND)
    set(FLTK_USE_X11 1)
    list(APPEND FLTK_LDLIBS -lX11)
    if(X11_Xext_FOUND)
      list(APPEND FLTK_LDLIBS -lXext)
    endif(X11_Xext_FOUND)
    get_filename_component(PATH_TO_XLIBS ${X11_X11_LIB} PATH)
  endif(X11_FOUND)
endif()

if(APPLE AND FLTK_BACKEND_X11)
  if(NOT(${CMAKE_SYSTEM_VERSION} VERSION_LESS 17.0.0)) # a.k.a. macOS version ≥ 10.13
    list(APPEND FLTK_CFLAGS "-D_LIBCPP_HAS_THREAD_API_PTHREAD")
  endif(NOT(${CMAKE_SYSTEM_VERSION} VERSION_LESS 17.0.0))
  # FIXME: include_directories(!)
  # FIXME: how can we implement "AFTER SYSTEM" ?
  include_directories(AFTER SYSTEM /opt/X11/include/freetype2)
  include_directories(AFTER SYSTEM /opt/X11/include) # for Xft.h
  if(PATH_TO_XLIBS)
    set(LDFLAGS "-L${PATH_TO_XLIBS} ${LDFLAGS}")
  endif(PATH_TO_XLIBS)
  if(X11_INCLUDE_DIR)
    set(TEMP_INCLUDE_DIR ${X11_INCLUDE_DIR})
    list(TRANSFORM TEMP_INCLUDE_DIR PREPEND "-I")
    list(APPEND FLTK_CFLAGS "${TEMP_INCLUDE_DIR}")
  endif(X11_INCLUDE_DIR)
endif(APPLE AND FLTK_BACKEND_X11)

#######################################################################
option(FLTK_USE_POLL "use poll if available" OFF)
mark_as_advanced(FLTK_USE_POLL)

if(FLTK_USE_POLL)
  check_symbol_exists(poll   "poll.h"   USE_POLL)
endif(FLTK_USE_POLL)

#######################################################################
option(FLTK_BUILD_SHARED_LIBS
  "Build shared libraries in addition to static libraries"
  OFF
)

#######################################################################

option(FLTK_OPTION_PRINT_SUPPORT      "allow print support"        ON)
option(FLTK_OPTION_FILESYSTEM_SUPPORT "allow file system support"  ON)

option(FLTK_BUILD_FORMS        "Build forms compatibility library" ON)
option(FLTK_BUILD_FLUID        "Build FLUID"                       ON)
option(FLTK_BUILD_FLTK_OPTIONS "Build fltk-options"                ON)
option(FLTK_BUILD_EXAMPLES     "Build example programs"            OFF)

if(FLTK_IS_TOPLEVEL)
  option(FLTK_BUILD_TEST       "Build test/demo programs"          ON)
else()
  option(FLTK_BUILD_TEST       "Build test/demo programs"          OFF)
endif()

if(FLTK_BUILD_FORMS)
  set(FLTK_HAVE_FORMS 1)
else()
  set(FLTK_HAVE_FORMS 0)
endif()

#######################################################################
if(DOXYGEN_FOUND)
  option(FLTK_BUILD_HTML_DOCS "build html docs" ON)
  option(FLTK_INSTALL_HTML_DOCS "install html docs" OFF)
  option(FLTK_BUILD_FLUID_DOCS "build FLUID docs" OFF)
  option(FLTK_INSTALL_FLUID_DOCS "install FLUID docs" OFF)

  option(FLTK_INCLUDE_DRIVER_DOCS "include driver (developer) docs" OFF)
  mark_as_advanced(FLTK_INCLUDE_DRIVER_DOCS)

  if(LATEX_FOUND)
    option(FLTK_BUILD_PDF_DOCS "build pdf docs" ON)
    option(FLTK_INSTALL_PDF_DOCS "install pdf docs" OFF)
  endif(LATEX_FOUND)
endif(DOXYGEN_FOUND)

if(FLTK_BUILD_HTML_DOCS OR FLTK_BUILD_PDF_DOCS)
  add_subdirectory(documentation)
endif(FLTK_BUILD_HTML_DOCS OR FLTK_BUILD_PDF_DOCS)

if(FLTK_BUILD_FLUID_DOCS)
  add_subdirectory(fluid/documentation)
endif(FLTK_BUILD_FLUID_DOCS)

#######################################################################
# Include optional Cairo support
#######################################################################

option(FLTK_OPTION_CAIRO_WINDOW "add support for Fl_Cairo_Window" OFF)
option(FLTK_OPTION_CAIRO_EXT
  "use FLTK code instrumentation for Cairo extended use" OFF
)

set(FLTK_HAVE_CAIRO 0)
set(FLTK_HAVE_CAIROEXT 0)

if(FLTK_OPTION_CAIRO_WINDOW OR FLTK_OPTION_CAIRO_EXT)

  # On Windows we don't use pkg-config *if* FLTK_CAIRO_DIR is set
  # to prevent that CMake finds the system lib(s).

  if(WIN32 AND FLTK_CAIRO_DIR)
    set(PKG_CAIRO_FOUND FALSE)
  else()
    pkg_search_module(PKG_CAIRO cairo)
  endif()

  if(PKG_CAIRO_FOUND)
    set(FLTK_HAVE_CAIRO 1)
    if(FLTK_OPTION_CAIRO_EXT)
      set(FLTK_HAVE_CAIROEXT 1)
    endif(FLTK_OPTION_CAIRO_EXT)

    list(APPEND FLTK_BUILD_INCLUDE_DIRECTORIES ${PKG_CAIRO_INCLUDE_DIRS})

    # Cairo libs and flags for fltk-config

    # Hint: use either PKG_CAIRO_* or PKG_CAIRO_STATIC_* variables to
    # create the list of libraries used to link programs with Cairo
    # by running fltk-config --use-cairo --compile ...
    # Currently we're using the non-STATIC variables to link Cairo shared.

    set(CAIROLIBS)
    foreach(lib ${PKG_CAIRO_LIBRARIES})
      list(APPEND CAIROLIBS "-l${lib}")
    endforeach()

    string(REPLACE ";" " " CAIROLIBS  "${CAIROLIBS}")
    string(REPLACE ";" " " CAIROFLAGS "${PKG_CAIRO_CFLAGS}")

  else(PKG_CAIRO_FOUND)

    if(NOT WIN32)
      message(STATUS "*** Cairo was requested but not found - please check your Cairo installation")
      message(STATUS "***   or disable options FLTK_OPTION_CAIRO_WINDOW and FLTK_OPTION_CAIRO_EXT.")
      message(FATAL_ERROR "*** Terminating: missing Cairo libs or headers.")
    endif()

    # Tweak Cairo includes / libs / paths for Windows (TEMPORARY solution).
    # Todo: find a better way to set the required variables and flags!
    # The current version was tested with 32-bit (MinGW) and 64-bit (Visual
    # Studio and MSYS2). The latter can also be used with the Cairo library
    # provided by MSYS2, but then the build depends on the MSYS2 installation.
    # AlbrechtS (05/2024)

    message(STATUS "--- Cairo not found: trying to find Cairo for Windows ...")

    if(CMAKE_SIZEOF_VOID_P STREQUAL "8")
      set(_cairo_suffix x64)
    else()
      set(_cairo_suffix x86)
    endif()

    find_library(FLTK_CAIRO_LIB cairo
                  PATHS ${FLTK_CAIRO_DIR}
                  PATH_SUFFIXES lib lib/${_cairo_suffix}
                  NO_DEFAULT_PATH
                )

    if(NOT FLTK_CAIRO_DIR AND NOT FLTK_CAIRO_LIB)
      message(STATUS "--- Please set FLTK_CAIRO_DIR to point to the Cairo installation folder ...")
      message(STATUS "    ... with files 'include/cairo.h' and 'lib/${_cairo_suffix}/cairo.lib'")
      message(STATUS "--- Example: cmake -DFLTK_CAIRO_DIR=\"C:/cairo-windows\" ...")
      message(STATUS "--- Note: this may be changed in the future.")
      message(FATAL_ERROR "*** Terminating: missing Cairo libs or headers.")
    endif()

    set(CAIROLIBS "-lcairo")                             # should be correct: needs cairo.lib

    # Simulate results of 'pkg_search_module (PKG_CAIRO cairo)' and more (above).
    # These variables will be used later

    set(PKG_CAIRO_LIBRARIES "cairo")
    set(PKG_CAIRO_INCLUDE_DIRS "${FLTK_CAIRO_DIR}/include")
    set(PKG_CAIRO_LIBRARY_DIRS "${FLTK_CAIRO_DIR}/lib/${_cairo_suffix}/")

    # FIXME - include_directories()
    include_directories(${PKG_CAIRO_INCLUDE_DIRS})

    set(FLTK_HAVE_CAIRO 1)
    if(FLTK_OPTION_CAIRO_EXT)
      set(FLTK_HAVE_CAIROEXT 1)
    endif(FLTK_OPTION_CAIRO_EXT)

  endif(PKG_CAIRO_FOUND)

  if(0) # 1 = DEBUG, 0 = no output
    message(STATUS "--- options.cmake: Cairo related variables ---")
    if(WIN32)
      fl_debug_var(FLTK_CAIRO_DIR)
      fl_debug_var(_cairo_suffix)
    endif()
    fl_debug_pkg(PKG_CAIRO cairo)
    message(STATUS "--- fltk-config/Cairo variables ---")
    fl_debug_var(FLTK_LDLIBS)
    fl_debug_var(CAIROFLAGS)
    fl_debug_var(CAIROLIBS)
    message(STATUS "--- End of Cairo related variables ---")
  endif() # 1 = DEBUG, ...

  unset(_cairo_suffix)

endif(FLTK_OPTION_CAIRO_WINDOW OR FLTK_OPTION_CAIRO_EXT)

#######################################################################

option(FLTK_OPTION_SVG "read/write SVG image files" ON)

if(FLTK_OPTION_SVG)
  set(FLTK_USE_SVG 1)
else()
  set(FLTK_USE_SVG 0)
endif(FLTK_OPTION_SVG)

#######################################################################

# FIXME: GLU libs have already been searched in resources.cmake

set(HAVE_GL LIB_GL OR LIB_MesaGL)
set(FLTK_USE_GL FALSE)

if(HAVE_GL)
  option(FLTK_BUILD_GL "use OpenGL and build fltk_gl library" ON)
  if(FLTK_BUILD_GL)
    set(FLTK_USE_GL TRUE)
  endif()
endif()

if(FLTK_BUILD_GL)
  if(FLTK_BACKEND_WAYLAND)
    pkg_check_modules(WLD_EGL IMPORTED_TARGET wayland-egl)
    pkg_check_modules(PKG_EGL IMPORTED_TARGET egl)
    pkg_check_modules(PKG_GL  IMPORTED_TARGET gl)
    pkg_check_modules(PKG_GLU IMPORTED_TARGET glu)

    if(NOT (WLD_EGL_FOUND AND PKG_EGL_FOUND AND PKG_GL_FOUND AND PKG_GLU_FOUND))
      message(STATUS "Modules 'wayland-egl, egl, gl, and glu' are required to build for the Wayland backend.")
      message(FATAL_ERROR "*** Aborting ***")
    endif()

  endif(FLTK_BACKEND_WAYLAND)

  if(FLTK_BACKEND_X11)
    set(OPENGL_FOUND TRUE)
    find_library(OPENGL_LIB GL)
    get_filename_component(PATH_TO_GLLIB ${OPENGL_LIB} DIRECTORY)
    find_library(GLU_LIB GLU)
    get_filename_component(PATH_TO_GLULIB ${GLU_LIB} DIRECTORY)
    # FIXME: we should find a better way to resolve this issue:
    # with GL, must use XQuartz libX11 else "Insufficient GL support"
    set(OPENGL_LIBRARIES -L${PATH_TO_GLULIB} -L${PATH_TO_GLLIB} -lX11 -lGLU -lGL)
    find_path(OPENGL_INCLUDE_DIR NAMES GL/gl.h OpenGL/gl.h HINTS ${X11_INCLUDE_DIR})
    unset(HAVE_GL_GLU_H CACHE)
    find_file(HAVE_GL_GLU_H GL/glu.h PATHS ${X11_INCLUDE_DIR})
  else()
    find_package(OpenGL)
    if(APPLE)
      set(HAVE_GL_GLU_H ${HAVE_OPENGL_GLU_H})
    endif(APPLE)
  endif(FLTK_BACKEND_X11)
else(FLTK_BUILD_GL)
  set(OPENGL_FOUND FALSE)
  set(HAVE_GL FALSE)
  set(HAVE_GL_GLU_H FALSE)
  set(HAVE_GLXGETPROCADDRESSARB FALSE)
endif(FLTK_BUILD_GL)

mark_as_advanced(OPENGL_LIB) # internal cache variable, not relevant for users

# FIXME: the following is necessary because this variable may have been removed
# from the cache above. It has been marked "advanced" before in resources.cmake.
mark_as_advanced(HAVE_GL_GLU_H)

# Note: GLLIBS is a CMake 'list' and is used exclusively to generate fltk-config !

# FIXME, this should be improved!
# We should probably deduct this from OPENGL_LIBRARIES but it turned
# out to be difficult since FindOpenGL seems to return different
# syntax depending on the platform (and maybe also CMake version).
# Hence we use the following code...

set(GLLIBS)
set(FLTK_GL_FOUND FALSE)

if(OPENGL_FOUND)
  set(FLTK_GL_FOUND TRUE)
  find_path(FLTK_OPENGL_GLU_INCLUDE_DIR NAMES GL/glu.h OpenGL/glu.h HINTS ${OPENGL_INCLUDE_DIR} ${X11_INCLUDE_DIR})
  set(CMAKE_REQUIRED_INCLUDES ${OPENGL_INCLUDE_DIR}/GL ${FLTK_OPENGL_GLU_INCLUDE_DIR})

  if(WIN32)
    list(APPEND GLLIBS -lglu32 -lopengl32)
  elseif(APPLE AND NOT FLTK_BACKEND_X11)
    list(APPEND GLLIBS "-framework OpenGL")
  elseif(FLTK_BACKEND_WAYLAND)
    foreach(_lib WLD_EGL PKG_EGL PKG_GLU PKG_GL)
      list(APPEND GLLIBS ${${_lib}_LDFLAGS})
    endforeach(_lib )
  else()
    list(APPEND GLLIBS -lGLU -lGL)
  endif(WIN32)

  # check if function glXGetProcAddressARB exists
  set(TEMP_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
  set(CMAKE_REQUIRED_LIBRARIES ${OPENGL_LIBRARIES})
  check_symbol_exists(glXGetProcAddressARB  "glx.h"   HAVE_GLXGETPROCADDRESSARB)
  set(CMAKE_REQUIRED_LIBRARIES ${TEMP_REQUIRED_LIBRARIES})
  unset(TEMP_REQUIRED_LIBRARIES)
endif(OPENGL_FOUND)

#######################################################################
option(FLTK_OPTION_LARGE_FILE "enable large file support" ON)

if(FLTK_OPTION_LARGE_FILE)
  if(NOT MSVC)
    add_definitions(-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)
    list(APPEND FLTK_CFLAGS -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)
  endif(NOT MSVC)
endif(FLTK_OPTION_LARGE_FILE)

#######################################################################
# Create an option whether we want to check for pthreads.
# We must not do it on Windows unless we run under Cygwin, since we
# always use native threads on Windows (even if libpthread is available).

# Note: HAVE_PTHREAD_H has already been determined in resources.cmake
# before this file is included (or set to 0 for WIN32).

if(WIN32 AND NOT CYGWIN)
  # set(HAVE_PTHREAD_H 0) # (see resources.cmake)
  set(FLTK_USE_PTHREADS FALSE)
else()
  option(FLTK_USE_PTHREADS "use multi-threading with pthreads" ON)
endif(WIN32 AND NOT CYGWIN)

# initialize more variables
set(USE_THREADS 0)
set(HAVE_PTHREAD 0)
set(FLTK_PTHREADS_FOUND FALSE)

if(FLTK_USE_PTHREADS)

  include(FindThreads)

  if(CMAKE_HAVE_THREADS_LIBRARY)
    add_definitions("-D_THREAD_SAFE -D_REENTRANT")
    set(USE_THREADS 1)
    set(FLTK_THREADS_FOUND TRUE)
  endif(CMAKE_HAVE_THREADS_LIBRARY)

  if(CMAKE_USE_PTHREADS_INIT AND NOT WIN32)
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
  endif(CMAKE_USE_PTHREADS_INIT AND NOT WIN32)

else(FLTK_USE_PTHREADS)

  set(HAVE_PTHREAD_H 0)

endif(FLTK_USE_PTHREADS)

set(debug_threads 0) # set to 1 to show debug info
if(debug_threads)
  message("")
  message(STATUS "options.cmake: set debug_threads to 0 to disable the following info:")
  fl_debug_var(FLTK_USE_PTHREADS)
  fl_debug_var(HAVE_PTHREAD)
  fl_debug_var(HAVE_PTHREAD_H)
  fl_debug_var(FLTK_THREADS_FOUND)
  fl_debug_var(CMAKE_EXE_LINKER_FLAGS)
  message(STATUS "options.cmake: end of debug_threads info.")
endif(debug_threads)
unset(debug_threads)


#######################################################################
if(X11_Xinerama_FOUND)
  option(FLTK_USE_XINERAMA "use lib Xinerama" ON)
endif(X11_Xinerama_FOUND)

if(FLTK_USE_XINERAMA)
  set(HAVE_XINERAMA ${X11_Xinerama_FOUND})
  list(APPEND FLTK_BUILD_INCLUDE_DIRECTORIES ${X11_Xinerama_INCLUDE_PATH})
  list(APPEND FLTK_LDLIBS -lXinerama)
  set(FLTK_XINERAMA_FOUND TRUE)
else()
  set(FLTK_XINERAMA_FOUND FALSE)
endif(FLTK_USE_XINERAMA)

#######################################################################
if(X11_Xfixes_FOUND)
  option(FLTK_USE_XFIXES "use lib Xfixes" ON)
endif(X11_Xfixes_FOUND)

if(FLTK_USE_XFIXES)
  set(HAVE_XFIXES ${X11_Xfixes_FOUND})
  list(APPEND FLTK_BUILD_INCLUDE_DIRECTORIES ${X11_Xfixes_INCLUDE_PATH})
  list(APPEND FLTK_LDLIBS -lXfixes)
  set(FLTK_XFIXES_FOUND TRUE)
else()
  set(FLTK_XFIXES_FOUND FALSE)
endif(FLTK_USE_XFIXES)

#######################################################################
if(X11_Xcursor_FOUND)
  option(FLTK_USE_XCURSOR "use lib Xcursor" ON)
endif(X11_Xcursor_FOUND)

if(FLTK_USE_XCURSOR)
  set(HAVE_XCURSOR ${X11_Xcursor_FOUND})
  list(APPEND FLTK_BUILD_INCLUDE_DIRECTORIES ${X11_Xcursor_INCLUDE_PATH})
  list(APPEND FLTK_LDLIBS -lXcursor)
  set(FLTK_XCURSOR_FOUND TRUE)
else()
  set(FLTK_XCURSOR_FOUND FALSE)
endif(FLTK_USE_XCURSOR)

#######################################################################
if(X11_Xft_FOUND)
  option(FLTK_USE_PANGO "use lib Pango" OFF)
  if(NOT FLTK_BACKEND_WAYLAND)
    option(FLTK_GRAPHICS_CAIRO "all drawing to X11 windows uses Cairo" OFF)
  endif(NOT FLTK_BACKEND_WAYLAND)
  if(NOT FLTK_GRAPHICS_CAIRO)
    option(FLTK_USE_XFT "use lib Xft" ON)
  endif()
endif(X11_Xft_FOUND)

# test option compatibility: Cairo for Xlib requires Pango
if(FLTK_GRAPHICS_CAIRO)
  unset(FLTK_USE_PANGO CACHE)
  set(FLTK_USE_PANGO TRUE CACHE BOOL "use lib Pango")
endif(FLTK_GRAPHICS_CAIRO)

if(FLTK_USE_X11 AND FLTK_USE_PANGO AND NOT FLTK_GRAPHICS_CAIRO)
  set(USE_PANGOXFT true)
endif()

# test option compatibility: PangoXft requires Xft
if(USE_PANGOXFT)
  if(NOT X11_Xft_FOUND)
    message(STATUS "PangoXft requires Xft but Xft library or headers could not be found.")
    message(STATUS "Please install Xft development files and try again or disable FLTK_USE_PANGO.")
    message(FATAL_ERROR "*** Aborting ***")
  else()
    if(NOT FLTK_USE_XFT)
      message(STATUS "PangoXft requires Xft but usage of Xft was disabled.")
      message(STATUS "Please enable FLTK_USE_XFT and try again or disable FLTK_USE_PANGO.")
      message(FATAL_ERROR "*** Aborting ***")
    endif(NOT FLTK_USE_XFT)
  endif(NOT X11_Xft_FOUND)
endif(USE_PANGOXFT)

#######################################################################
if((X11_Xft_FOUND OR NOT USE_PANGOXFT) AND FLTK_USE_PANGO)
  if(NOT PKG_CONFIG_FOUND)
    message(FATAL_ERROR "Option FLTK_USE_PANGO requires availability of pkg-config on the build machine.")
  endif(NOT PKG_CONFIG_FOUND)
  pkg_check_modules(CAIRO IMPORTED_TARGET cairo)
  if(USE_PANGOXFT)
    pkg_check_modules(PANGOXFT IMPORTED_TARGET pangoxft)
  endif(USE_PANGOXFT)
  pkg_check_modules(PANGOCAIRO IMPORTED_TARGET pangocairo)

  if((PANGOXFT_FOUND OR NOT USE_PANGOXFT) AND PANGOCAIRO_FOUND AND CAIRO_FOUND)
    if(USE_PANGOXFT)
      list(APPEND FLTK_BUILD_INCLUDE_DIRECTORIES ${PANGOXFT_INCLUDE_DIRS})
    else()
      list(APPEND FLTK_BUILD_INCLUDE_DIRECTORIES ${PANGOCAIRO_INCLUDE_DIRS})
    endif(USE_PANGOXFT)
    list(APPEND FLTK_BUILD_INCLUDE_DIRECTORIES ${CAIRO_INCLUDE_DIRS})

    set(USE_PANGO TRUE)

    # add required libraries to fltk-config ...
    if(USE_PANGOXFT)
      list(APPEND FLTK_LDLIBS ${PANGOXFT_LDFLAGS})
    endif(USE_PANGOXFT)
    list(APPEND FLTK_LDLIBS ${PANGOCAIRO_LDFLAGS})

  endif((PANGOXFT_FOUND OR NOT USE_PANGOXFT) AND PANGOCAIRO_FOUND AND CAIRO_FOUND)

  if(USE_PANGO AND (FLTK_GRAPHICS_CAIRO OR FLTK_BACKEND_WAYLAND))
    set(FLTK_USE_CAIRO 1)
    # fl_debug_var(FLTK_USE_CAIRO)
  endif()

endif((X11_Xft_FOUND OR NOT USE_PANGOXFT) AND FLTK_USE_PANGO)

if(FLTK_BACKEND_WAYLAND)

  # Note: Disable FLTK_USE_LIBDECOR_GTK to get cairo titlebars rather than GTK
  if(FLTK_USE_LIBDECOR_GTK AND NOT USE_SYSTEM_LIBDECOR)
    pkg_check_modules(GTK IMPORTED_TARGET gtk+-3.0)
    if(GTK_FOUND)
      list(APPEND FLTK_BUILD_INCLUDE_DIRECTORIES ${GTK_INCLUDE_DIRS})
      list(APPEND FLTK_LDLIBS ${GTK_LDFLAGS})
    else()
      message(WARNING "Installation of the development files for the GTK library "
      "(e.g., libgtk-3-dev) is recommended when using the gnome desktop.")
    endif(GTK_FOUND)
  endif(FLTK_USE_LIBDECOR_GTK AND NOT USE_SYSTEM_LIBDECOR)

endif()

if(FLTK_USE_XFT)
  set(USE_XFT X11_Xft_FOUND)
  list(APPEND FLTK_LDLIBS -lXft)
  set(FLTK_XFT_FOUND TRUE)
  if(APPLE AND FLTK_BACKEND_X11)
    find_library(LIB_fontconfig fontconfig "/opt/X11/lib")
  endif()
else()
  set(FLTK_XFT_FOUND FALSE)
endif(FLTK_USE_XFT)

#######################################################################
if(X11_Xrender_FOUND)
  option(FLTK_USE_XRENDER "use lib Xrender" ON)
endif(X11_Xrender_FOUND)

if(FLTK_USE_XRENDER)
  set(HAVE_XRENDER ${X11_Xrender_FOUND})
  if(HAVE_XRENDER)
    list(APPEND FLTK_BUILD_INCLUDE_DIRECTORIES ${X11_Xrender_INCLUDE_PATH})
    list(APPEND FLTK_LDLIBS -lXrender)
    set(FLTK_XRENDER_FOUND TRUE)
  else(HAVE_XRENDER)
    set(FLTK_XRENDER_FOUND FALSE)
  endif(HAVE_XRENDER)
else(FLTK_USE_XRENDER)
  set(FLTK_XRENDER_FOUND FALSE)
endif(FLTK_USE_XRENDER)

#######################################################################
set(FL_NO_PRINT_SUPPORT FALSE)
if(X11_FOUND AND NOT FLTK_OPTION_PRINT_SUPPORT)
  set(FL_NO_PRINT_SUPPORT TRUE)
endif(X11_FOUND AND NOT FLTK_OPTION_PRINT_SUPPORT)
#######################################################################

#######################################################################
set(FL_CFG_NO_FILESYSTEM_SUPPORT TRUE)
if(FLTK_OPTION_FILESYSTEM_SUPPORT)
  set(FL_CFG_NO_FILESYSTEM_SUPPORT FALSE)
endif(FLTK_OPTION_FILESYSTEM_SUPPORT)
#######################################################################

#######################################################################
option(CMAKE_SUPPRESS_REGENERATION "suppress rules to re-run CMake on rebuild" OFF)
mark_as_advanced(CMAKE_SUPPRESS_REGENERATION)

#######################################################################
# Clean up ...

# *** FIXME *** Do we need all these variables ?

list(REMOVE_DUPLICATES FLTK_BUILD_INCLUDE_DIRECTORIES)
list(REMOVE_DUPLICATES FLTK_IMAGE_INCLUDE_DIRECTORIES)
list(REMOVE_DUPLICATES FLTK_IMAGE_LIBRARIES)

#######################################################################
# Debugging ...

if(DEBUG_OPTIONS_CMAKE)
  message(STATUS "") # empty line
  fl_debug_var(WIN32)
  fl_debug_var(LIBS)
  fl_debug_var(GLLIBS)
  fl_debug_var(FLTK_LDLIBS)
  fl_debug_var(OPENGL_FOUND)
  fl_debug_var(OPENGL_INCLUDE_DIR)
  fl_debug_var(OPENGL_LIBRARIES)
  fl_debug_var(CMAKE_MSVC_RUNTIME_LIBRARY)
  message("--- bundled libraries ---")
  fl_debug_var(FLTK_USE_SYSTEM_LIBJPEG)
  fl_debug_var(FLTK_USE_SYSTEM_LIBPNG)
  fl_debug_var(FLTK_USE_SYSTEM_ZLIB)
  fl_debug_var(FLTK_USE_BUNDLED_JPEG)
  fl_debug_var(FLTK_USE_BUNDLED_PNG)
  fl_debug_var(FLTK_USE_BUNDLED_ZLIB)

  message(STATUS "--- *FIXME* include directories ---")
  fl_debug_var(FLTK_BUILD_INCLUDE_DIRECTORIES)
  fl_debug_var(FLTK_IMAGE_INCLUDE_DIRECTORIES)

  message("--- X11 ---")
  fl_debug_var(X11_FOUND)
  fl_debug_var(X11_INCLUDE_DIR)
  fl_debug_var(X11_LIBRARIES)
  fl_debug_var(X11_X11_LIB)
  fl_debug_var(X11_X11_INCLUDE_PATH)
  fl_debug_var(X11_Xft_INCLUDE_PATH)
  fl_debug_var(X11_Xft_LIB)
  fl_debug_var(X11_Xft_FOUND)
  fl_debug_var(PATH_TO_XLIBS)
  message(STATUS "[** end of options.cmake **]")
endif(DEBUG_OPTIONS_CMAKE)
unset(DEBUG_OPTIONS_CMAKE)
