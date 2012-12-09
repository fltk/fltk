#
# "$Id$"
#
# JPEG library makefile for the Fast Light Toolkit (FLTK).
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
