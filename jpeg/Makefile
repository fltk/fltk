# *************************************************************************
#  FLTK - DO NOT CHANGE when upgrading the JPEG library, unless required. *
# *************************************************************************
#
# JPEG library makefile for the Fast Light Toolkit (FLTK).
#
# Copyright 1997-2024 by Bill Spitzak and others.
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

include ../makeinclude

#
# Object files...
#

OBJS	=	\
		jaricom.o \
		jcapimin.o \
		jcapistd.o \
		jcarith.o \
		jccoefct.o \
		jccolor.o \
		jcdctmgr.o \
		jchuff.o \
		jcinit.o \
		jcmainct.o \
		jcmarker.o \
		jcmaster.o \
		jcomapi.o \
		jcparam.o \
		jcprepct.o \
		jcsample.o \
		jctrans.o \
		jdapimin.o \
		jdapistd.o \
		jdarith.o \
		jdatadst.o \
		jdatasrc.o \
		jdcoefct.o \
		jdcolor.o \
		jddctmgr.o \
		jdhuff.o \
		jdinput.o \
		jdmainct.o \
		jdmarker.o \
		jdmaster.o \
		jdmerge.o \
		jdpostct.o \
		jdsample.o \
		jdtrans.o \
		jerror.o \
		jfdctflt.o \
		jfdctfst.o \
		jfdctint.o \
		jidctflt.o \
		jidctfst.o \
		jidctint.o \
		jmemmgr.o \
		jmemnobs.o \
		jquant1.o \
		jquant2.o \
		jutils.o

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
	-$(INSTALL_DIR) "$(DESTDIR)$(libdir)"
	$(INSTALL_LIB) $(LIBJPEG) "$(DESTDIR)$(libdir)"
	$(RANLIB) "$(DESTDIR)$(libdir)/libfltk_jpeg$(LIBEXT)"
	echo "Installing jpeg headers in $(includedir)/FL/images..."
	-$(INSTALL_DIR) "$(DESTDIR)$(includedir)/FL/images"
	$(INSTALL_DATA) jconfig.h "$(DESTDIR)$(includedir)/FL/images"
	$(INSTALL_DATA) jerror.h "$(DESTDIR)$(includedir)/FL/images"
	$(INSTALL_DATA) jmorecfg.h "$(DESTDIR)$(includedir)/FL/images"
	$(INSTALL_DATA) jpeglib.h "$(DESTDIR)$(includedir)/FL/images"
	$(INSTALL_DATA) fltk_jpeg_prefix.h "$(DESTDIR)$(includedir)/FL/images"

#
# Uninstall everything...
#

uninstall:
	echo "Uninstalling libfltk_jpeg$(LIBEXT) in $(libdir)..."
	$(RM) "$(libdir)/libfltk_jpeg$(LIBEXT)"
	echo "Uninstalling jpeg headers in $(includedir)/FL/images..."
	$(RM) "$(includedir)/FL/images/jconfig.h"
	$(RM) "$(includedir)/FL/images/jerror.h"
	$(RM) "$(includedir)/FL/images/jmorecfg.h"
	$(RM) "$(includedir)/FL/images/jpeglib.h"
	$(RM) "$(includedir)/FL/images/fltk_jpeg_prefix.h"

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
	makedepend -Y -I.. -f makedepend -w 20 $(OBJS:.o=.c)
	echo "# DO NOT DELETE THIS LINE -- make depend depends on it." > makedepend.tmp
	echo "" >> makedepend.tmp
	grep '^[a-zA-Z]' makedepend | ( LC_ALL=C sort -u -f >> makedepend.tmp; )
	mv makedepend.tmp makedepend

include makedepend

$(OBJS):	../makeinclude
