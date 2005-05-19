#
# "$Id$"
#
# PNG library makefile for the Fast Light Toolkit (FLTK).
#
# Copyright 1997-2004 by Easy Software Products.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.
#
# Please report all bugs and problems to "fltk-bugs@fltk.org".
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
