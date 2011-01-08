#
# "$Id$"
#
# JPEG library makefile for the Fast Light Toolkit (FLTK).
#
# Copyright 1997-2009 by Easy Software Products.
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

include ../makeinclude


#
# Object files...
#

OBJS	=	jmemnobs.o \
		jcapimin.o jcapistd.o jccoefct.o jccolor.o jcdctmgr.o \
		jchuff.o jcinit.o jcmainct.o jcmarker.o jcmaster.o jcomapi.o \
		jcparam.o jcphuff.o jcprepct.o jcsample.o jctrans.o \
		jdapimin.o jdapistd.o jdatadst.o jdatasrc.o jdcoefct.o \
		jdcolor.o jddctmgr.o jdhuff.o jdinput.o jdmainct.o jdmarker.o \
		jdmaster.o jdmerge.o jdphuff.o jdpostct.o jdsample.o \
		jdtrans.o jerror.o jfdctflt.o jfdctfst.o jfdctint.o \
		jidctflt.o jidctfst.o jidctint.o jidctred.o jquant1.o \
		jquant2.o jutils.o jmemmgr.o

LIBJPEG	=	../lib/libfltk_jpeg$(LIBEXT)


#
# Make all targets...
#

all:	$(LIBJPEG)


#
# Clean all targets and object files...
#

clean:
	$(RM) $(OBJS)
	$(RM) $(LIBJPEG)


#
# Install everything...
#

install:	$(LIBJPEG)
	echo "Installing $(LIBJPEG) in $(libdir)..."
	-$(INSTALL_DIR) $(DESTDIR)$(libdir)
	$(INSTALL_LIB) $(LIBJPEG) $(DESTDIR)$(libdir)
	$(RANLIB) $(DESTDIR)$(libdir)/libfltk_jpeg$(LIBEXT)
	echo "Installing jpeg headers in $(includedir)/FL/images..."
	-$(INSTALL_DIR) $(DESTDIR)$(includedir)/FL/images
	$(INSTALL_DATA) jconfig.h $(DESTDIR)$(includedir)/FL/images
	$(INSTALL_DATA) jerror.h $(DESTDIR)$(includedir)/FL/images
	$(INSTALL_DATA) jmorecfg.h $(DESTDIR)$(includedir)/FL/images
	$(INSTALL_DATA) jpeglib.h $(DESTDIR)$(includedir)/FL/images


#
# Uninstall everything...
#

uninstall:
	echo "Uninstalling libfltk_jpeg$(LIBEXT) in $(libdir)..."
	$(RM) $(libdir)/libfltk_jpeg$(LIBEXT)
	echo "Uninstalling jpeg headers in $(includedir)/FL/images..."
	$(RM) $(includedir)/FL/images/jconfig.h
	$(RM) $(includedir)/FL/images/jerror.h
	$(RM) $(includedir)/FL/images/jmorecfg.h
	$(RM) $(includedir)/FL/images/jpeglib.h


#
# libfltk_jpeg.a
#

$(LIBJPEG):	$(OBJS)
	echo Archiving $@...
	$(RM) $@
	$(LIBCOMMAND) $@ $(OBJS)
	$(RANLIB) $@


#
# Make dependencies...
#

depend:	$(OBJS:.o=.c)
	makedepend -Y -I.. -f makedepend $(OBJS:.o=.c)

include makedepend

$(OBJS):	../makeinclude

#
# End of "$Id$".
#
