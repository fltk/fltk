#
# "$Id$"
#
# This file sets variables for common use in export.cmake and install.cmake
# Written by Michael Surette
#
# Copyright 1998-2015 by Bill Spitzak and others.
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
set(FL_MAJOR_VERSION ${FLTK_VERSION_MAJOR})
set(FL_MINOR_VERSION ${FLTK_VERSION_MINOR})
set(FL_PATCH_VERSION ${FLTK_VERSION_PATCH})

#######################################################################
# add several libraries (STR #3011)
# FIXME: libraries may need reordering, and this version does not yet
# correctly support static linking and local zlib, png, and jpeg libs.

if(LIB_fontconfig)
   list(APPEND FLTK_LDLIBS -lfontconfig)
endif(LIB_fontconfig)

if(HAVE_DLSYM)
   list(APPEND FLTK_LDLIBS -ldl)
endif(HAVE_DLSYM)

if(LIB_png)
   list(APPEND IMAGELIBS -lpng)
endif(LIB_png)

if(LIB_zlib)
   list(APPEND IMAGELIBS -lz)
endif(LIB_zlib)

if(LIB_jpeg)
   list(APPEND IMAGELIBS -ljpeg)
endif(LIB_jpeg)

string(REPLACE ";" " " IMAGELIBS "${IMAGELIBS}")
set(STATICIMAGELIBS "${IMAGELIBS}")

#######################################################################
set(CC ${CMAKE_C_COMPILER})
set(CXX ${CMAKE_CXX_COMPILER})

set(ARCHFLAGS ${OPTION_ARCHFLAGS})

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_UPPER)
if(${BUILD_UPPER})
    set(CFLAGS "${CMAKE_C_FLAGS_${BUILD_UPPER}} ${CFLAGS}")
endif(${BUILD_UPPER})

set(CFLAGS "${OPTION_OPTIM} ${CMAKE_C_FLAGS} ${CFLAGS}")
foreach(arg ${FLTK_CFLAGS})
    set(CFLAGS "${CFLAGS} ${arg}")
endforeach(arg ${FLTK_CFLAGS})

set(CXXFLAGS ${CFLAGS})

foreach(arg ${FLTK_LDLIBS})
  set(LINK_LIBS "${LINK_LIBS} ${arg}")
endforeach(arg ${FLTK_LDLIBS})

set(LIBS ${LINK_LIBS})

if (${CMAKE_SYSTEM_NAME} STREQUAL "AIX")
    set(SHAREDSUFFIX "_s")
else ()
    set(SHAREDSUFFIX "")
endif (${CMAKE_SYSTEM_NAME} STREQUAL "AIX")
