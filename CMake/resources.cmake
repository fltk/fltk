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

#######################################################################
# check for headers, libraries and functions
#######################################################################
# headers
find_file(HAVE_ALSA_ASOUNDLIB_H alsa/asoundlib.h)
find_file(HAVE_DLFCN_H dlfcn.h)
find_file(HAVE_FREETYPE_H freetype.h PATH_SUFFIXES freetype2 freetype2/freetype)
find_file(HAVE_GL_GLU_H GL/glu.h)
find_file(HAVE_LIBPNG_PNG_H libpng/png.h)
find_file(HAVE_LOCALE_H locale.h)
find_file(HAVE_OPENGL_GLU_H OpenGL/glu.h)
find_file(HAVE_PNG_H png.h)
find_file(HAVE_STDIO_H stdio.h)
find_file(HAVE_STRINGS_H strings.h)
find_file(HAVE_SYS_SELECT_H sys/select.h)
find_file(HAVE_SYS_STDTYPES_H sys/stdtypes.h)
find_file(HAVE_X11_XREGION_H X11/Xregion.h)
find_path(HAVE_XDBE_H Xdbe.h PATH_SUFFIXES X11/extensions extensions)

if (WIN32 AND NOT CYGWIN)
  # we don't use pthreads on Windows (except for Cygwin, see options.cmake)
  set(HAVE_PTHREAD_H 0)
else ()
  find_file(HAVE_PTHREAD_H pthread.h)
endif (WIN32 AND NOT CYGWIN)

# Special case for Microsoft Visual Studio generator (MSVC):
#
# The header files <GL/glu.h> and <locale.h> are located in the SDK's
# for Visual Studio. If CMake is invoked from a desktop icon or the Windows
# menu it doesn't have the correct paths to find these header files.
# The CMake folks recommend not to search for these files at all, because
# they must always be there, but we do anyway.
# If we don't find them we issue a warning and suggest to rerun CMake from
# a "Developer Command Prompt for Visual Studio xxxx", but we fix the issue
# by setting the *local* instance (not the cache variable) of the corresponding
# CMake variable to '1' since we "know" the header file is available.
#
# If the user builds the solution, everything should run smoothly despite
# the fact that the header files were not found.
#
# If the configuration is changed somehow (e.g. by editing CMakeLists.txt)
# CMake will be rerun from within Visual Studio, find the header file, and
# set the cache variable for the header file to its correct path. The latter is
# only informational so you can see that (and where) the headers were found.
#
# Note: these cache variables can only be seen in "advanced" mode.

if (MSVC)
  set (MSVC_RERUN_MESSAGE FALSE)

  if (NOT HAVE_GL_GLU_H)
    message(STATUS "Warning: Header file GL/glu.h was not found.")
    set (HAVE_GL_GLU_H 1)
    set (MSVC_RERUN_MESSAGE TRUE)
  endif (NOT HAVE_GL_GLU_H)

  if (NOT HAVE_LOCALE_H)
    message(STATUS "Warning: Header file locale.h was not found.")
    set (HAVE_LOCALE_H 1)
    set (MSVC_RERUN_MESSAGE TRUE)
  endif (NOT HAVE_LOCALE_H)

  if (MSVC_RERUN_MESSAGE)
    message(STATUS "The FLTK team recommends to rerun CMake from a")
    message(STATUS "\"Developer Command Prompt for Visual Studio xxxx\"")
  endif (MSVC_RERUN_MESSAGE)

  unset (MSVC_RERUN_MESSAGE)
endif (MSVC)

# Simulate the behavior of autoconf macro AC_HEADER_DIRENT, see:
# https://www.gnu.org/software/autoconf/manual/autoconf-2.69/html_node/Particular-Headers.html
# "Check for the following header files. For the first one that is found
#  and defines ‘DIR’, define the listed C preprocessor macro ..."
#
# Note: we don't check if it really defines 'DIR', but we stop processing
# once we found the first suitable header file.

find_file(HAVE_DIRENT_H dirent.h)
if(NOT HAVE_DIRENT_H)
  find_file(HAVE_SYS_NDIR_H sys/ndir.h)
  if(NOT HAVE_SYS_NDIR_H)
    find_file(HAVE_SYS_DIR_H sys/dir.h)
    if(NOT HAVE_SYS_DIR_H)
      find_file(HAVE_NDIR_H ndir.h)
    endif(NOT HAVE_SYS_DIR_H)
  endif(NOT HAVE_SYS_NDIR_H)
endif(NOT HAVE_DIRENT_H)

mark_as_advanced(HAVE_ALSA_ASOUNDLIB_H HAVE_DIRENT_H HAVE_DLFCN_H)
mark_as_advanced(HAVE_FREETYPE_H HAVE_GL_GLU_H)
mark_as_advanced(HAVE_LIBPNG_PNG_H HAVE_LOCALE_H HAVE_NDIR_H)
mark_as_advanced(HAVE_OPENGL_GLU_H HAVE_PNG_H HAVE_PTHREAD_H)
mark_as_advanced(HAVE_STDIO_H HAVE_STRINGS_H HAVE_SYS_DIR_H)
mark_as_advanced(HAVE_SYS_NDIR_H HAVE_SYS_SELECT_H)
mark_as_advanced(HAVE_SYS_STDTYPES_H HAVE_XDBE_H)
mark_as_advanced(HAVE_X11_XREGION_H)

# where to find freetype headers
find_path(FREETYPE_PATH freetype.h PATH_SUFFIXES freetype2)
find_path(FREETYPE_PATH freetype/freetype.h PATH_SUFFIXES freetype2)
if(FREETYPE_PATH)
   include_directories(${FREETYPE_PATH})
endif(FREETYPE_PATH)
mark_as_advanced(FREETYPE_PATH)

#######################################################################
# libraries
find_library(LIB_CAIRO cairo)
find_library(LIB_dl dl)
find_library(LIB_fontconfig fontconfig)
find_library(LIB_freetype freetype)
find_library(LIB_GL GL)
find_library(LIB_MesaGL MesaGL)
find_library(LIB_jpeg jpeg)
find_library(LIB_png png)
find_library(LIB_zlib z)

mark_as_advanced(LIB_CAIRO LIB_dl LIB_fontconfig LIB_freetype)
mark_as_advanced(LIB_GL LIB_MesaGL)
mark_as_advanced(LIB_jpeg LIB_png LIB_zlib)

#######################################################################
# functions
include(CheckFunctionExists)

# save CMAKE_REQUIRED_LIBRARIES (is this really necessary ?)
if(DEFINED CMAKE_REQUIRED_LIBRARIES)
  set(SAVED_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
else(DEFINED CMAKE_REQUIRED_LIBRARIES)
  unset(SAVED_REQUIRED_LIBRARIES)
endif(DEFINED CMAKE_REQUIRED_LIBRARIES)
set(CMAKE_REQUIRED_LIBRARIES)

if(HAVE_DLFCN_H)
   set(HAVE_DLFCN_H 1)
endif(HAVE_DLFCN_H)

set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_DL_LIBS})
CHECK_FUNCTION_EXISTS(dlsym			HAVE_DLSYM)
set(CMAKE_REQUIRED_LIBRARIES)

CHECK_FUNCTION_EXISTS(localeconv		HAVE_LOCALECONV)

if(LIB_png)
  set(CMAKE_REQUIRED_LIBRARIES ${LIB_png})
  CHECK_FUNCTION_EXISTS(png_get_valid		HAVE_PNG_GET_VALID)
  CHECK_FUNCTION_EXISTS(png_set_tRNS_to_alpha	HAVE_PNG_SET_TRNS_TO_ALPHA)
  set(CMAKE_REQUIRED_LIBRARIES)
endif(LIB_png)

CHECK_FUNCTION_EXISTS(scandir                HAVE_SCANDIR)
CHECK_FUNCTION_EXISTS(snprintf               HAVE_SNPRINTF)

# not really true but we convert strcasecmp calls to _stricmp calls in flstring.h
if(MSVC)
   set(HAVE_STRCASECMP 1)
endif(MSVC)

CHECK_FUNCTION_EXISTS(strcasecmp             HAVE_STRCASECMP)

CHECK_FUNCTION_EXISTS(strlcat                HAVE_STRLCAT)
CHECK_FUNCTION_EXISTS(strlcpy                HAVE_STRLCPY)
CHECK_FUNCTION_EXISTS(vsnprintf              HAVE_VSNPRINTF)

if(HAVE_SCANDIR AND NOT HAVE_SCANDIR_POSIX)
   set(MSG "POSIX compatible scandir")
   message(STATUS "Looking for ${MSG}")
   try_compile(V
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/CMake/posixScandir.cxx
      )
   if(V)
      message(STATUS "${MSG} - found")
      set(HAVE_SCANDIR_POSIX 1 CACHE INTERNAL "")
   else()
      message(STATUS "${MSG} - not found")
      set(HAVE_SCANDIR_POSIX HAVE_SCANDIR_POSIX-NOTFOUND)
   endif(V)
endif(HAVE_SCANDIR AND NOT HAVE_SCANDIR_POSIX)
mark_as_advanced(HAVE_SCANDIR_POSIX)

# restore CMAKE_REQUIRED_LIBRARIES (is this really necessary ?)
if(DEFINED SAVED_REQUIRED_LIBRARIES)
  set(CMAKE_REQUIRED_LIBRARIES ${SAVED_REQUIRED_LIBRARIES})
  unset(SAVED_REQUIRED_LIBRARIES)
else(DEFINED SAVED_REQUIRED_LIBRARIES)
  unset(CMAKE_REQUIRED_LIBRARIES)
endif(DEFINED SAVED_REQUIRED_LIBRARIES)

#######################################################################
# packages

# Doxygen: necessary for HTML and PDF docs
find_package(Doxygen)

# LaTex: necessary for PDF docs (note: FindLATEX doesn't return LATEX_FOUND)

# Note: we only check existence of `latex' (LATEX_COMPILER), hence
# building the pdf docs may still fail because of other missing tools.

set (LATEX_FOUND)
if (DOXYGEN_FOUND)
  find_package(LATEX)
  if (LATEX_COMPILER AND NOT LATEX_FOUND)
    set(LATEX_FOUND YES)
  endif (LATEX_COMPILER AND NOT LATEX_FOUND)
endif (DOXYGEN_FOUND)

# message("Doxygen  found : ${DOXYGEN_FOUND}")
# message("LaTex    found : ${LATEX_FOUND}")
# message("LaTex Compiler : ${LATEX_COMPILER}")

#
# End of "$Id$".
#
