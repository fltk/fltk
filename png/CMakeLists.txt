#
# PNG library CMake configuration for the Fast Light Toolkit (FLTK).
#
# Copyright 1998-2020 by Bill Spitzak and others.
#
# This library is free software. Distribution and use rights are outlined in
# the file "COPYING" which should have been included with this file.  If this
# file is missing or damaged, see the license at:
#
#     https://www.fltk.org/COPYING.php
#
# Please report all bugs and problems on the following page:
#
#     https://www.fltk.org/str.php
#

# source files for png
set(PNG_SRCS
  png.c
  pngerror.c
  pngget.c
  pngmem.c
  pngpread.c
  pngread.c
  pngrio.c
  pngrtran.c
  pngrutil.c
  pngset.c
  pngtrans.c
  pngwio.c
  pngwrite.c
  pngwtran.c
  pngwutil.c
)

#######################################################################
FL_ADD_LIBRARY(fltk_png STATIC "${PNG_SRCS}")
# install the png headers
install(FILES png.h;pngconf.h;pnglibconf.h
    DESTINATION ${FLTK_INCLUDEDIR}/FL/images
)

if(OPTION_USE_SYSTEM_ZLIB)
    target_link_libraries(fltk_png ${FLTK_ZLIB_LIBRARIES})
else()
    target_link_libraries(fltk_png fltk_z)
endif(OPTION_USE_SYSTEM_ZLIB)

#######################################################################
if(OPTION_BUILD_SHARED_LIBS)
#######################################################################
FL_ADD_LIBRARY(fltk_png SHARED "${PNG_SRCS}")

if(OPTION_USE_SYSTEM_ZLIB)
    target_link_libraries(fltk_png_SHARED ${FLTK_ZLIB_LIBRARIES})
else()
    target_link_libraries(fltk_png_SHARED fltk_z_SHARED)
endif(OPTION_USE_SYSTEM_ZLIB)

#######################################################################
endif(OPTION_BUILD_SHARED_LIBS)
#######################################################################
