#
# "$Id$"
#
# JPEG library makefile for the Fast Light Toolkit (FLTK).
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
# Please report all bugs and problems on the following page:
#
#     http://www.fltk.org/str.php
#

LIBNAMEROOT=ftlk_jpeg

!include ../watcom.mif


#
# Object files...
#

LIBOBJS = jmemnobs.obj &
          jcapimin.obj jcapistd.obj jccoefct.obj jccolor.obj jcdctmgr.obj &
          jchuff.obj jcinit.obj jcmainct.obj jcmarker.obj jcmaster.obj jcomapi.obj &
          jcparam.obj jcphuff.obj jcprepct.obj jcsample.obj jctrans.obj &
          jdapimin.obj jdapistd.obj jdatadst.obj jdatasrc.obj jdcoefct.obj &
          jdcolor.obj jddctmgr.obj jdhuff.obj jdinput.obj jdmainct.obj jdmarker.obj &
          jdmaster.obj jdmerge.obj jdphuff.obj jdpostct.obj jdsample.obj &
          jdtrans.obj jerror.obj jfdctflt.obj jfdctfst.obj jfdctint.obj &
          jidctflt.obj jidctfst.obj jidctint.obj jidctred.obj jquant1.obj &
          jquant2.obj jutils.obj jmemmgr.obj

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
