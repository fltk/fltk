#
# "$Id$"
#
# GNU ZIP library makefile for the Fast Light Toolkit (FLTK).
#
# Copyright 1998-2011 by Bill Spitzak and others.
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

LIBNAMEROOT=ftlk_z

!include ../watcom.mif


#
# Object files...
#

LIBOBJS = adler32.obj compress.obj crc32.obj uncompr.obj deflate.obj &
          trees.obj zutil.obj inflate.obj inftrees.obj inffast.obj &
		  gzclose.obj gzlib.obj gzread.obj gzwrite.obj infback.obj


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
