#
# "$Id$"
#
# PNG library makefile for the Fast Light Toolkit (FLTK).
#
# Copyright 1997-2004 by Easy Software Products.
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

LIBNAMEROOT=ftlk_png
# I ought to be able to do "EXTRA_INCLUDE_DIRS += ;../zlib" but it doesn't work for me (OW1.3)
!undef EXTRA_INCLUDE_DIRS
EXTRA_INCLUDE_DIRS=$(ROOT);../zlib

!include ../watcom.mif


#
# Object files...
#

LIBOBJS = png.obj pngset.obj pngget.obj pngrutil.obj pngtrans.obj pngwutil.obj &
          pngread.obj pngrio.obj pngwio.obj pngwrite.obj pngrtran.obj &
          pngwtran.obj pngmem.obj pngerror.obj pngpread.obj


#
# Make all targets...
#

all: $(LIBNAME)

$(LIBNAME): $(LIBOBJS)
    $(LIB) $(LIBOPTS) $@ $<

#
# Clean all directories
#
clean : .SYMBOLIC
    @echo Cleaning up.
CLEANEXTS = obj
    @for %a in ($(CLEANEXTS)) do -rm -f $(ODIR)\*.%a
    -rm -f *.err
    -rm -f $(LIBNAME)

#
# End of "$Id$".
#
