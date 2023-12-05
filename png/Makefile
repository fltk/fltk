#
# PNG library Makefile for the Fast Light Toolkit (FLTK).
#
# Copyright 1997-2011 by Easy Software Products.
# Copyright 2012-2023 by Bill Spitzak and others.
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

OBJS	=	png.o pngset.o pngget.o pngrutil.o pngtrans.o pngwutil.o \
		pngread.o pngrio.o pngwio.o pngwrite.o pngrtran.o \
		pngwtran.o pngmem.o pngerror.o pngpread.o \
		arm/arm_init.o arm/filter_neon_intrinsics.o arm/palette_neon_intrinsics.o \
		powerpc/powerpc_init.o powerpc/filter_vsx_intrinsics.o

LIBPNG	=	../lib/libfltk_png$(LIBEXT)


#
# Make all of the targets...
#

all:	$(LIBPNG)


#
# Clean all of the targets and object files...
#

clean:
	$(RM) $(OBJS)
	$(RM) $(LIBPNG)


#
# Install everything...
#

install: $(LIBPNG)
	echo "Installing libfltk_png$(LIBEXT) in $(libdir)..."
	-$(INSTALL_DIR) $(DESTDIR)$(libdir)
	$(INSTALL_LIB) $(LIBPNG) $(DESTDIR)$(libdir)
	$(RANLIB) $(DESTDIR)$(libdir)/libfltk_png$(LIBEXT)
	echo "Installing png headers in $(includedir)/FL/images..."
	-$(INSTALL_DIR) $(DESTDIR)$(includedir)/FL/images
	$(INSTALL_DATA) png.h $(DESTDIR)$(includedir)/FL/images
	$(INSTALL_DATA) pngconf.h $(DESTDIR)$(includedir)/FL/images
	$(INSTALL_DATA) pnglibconf.h $(DESTDIR)$(includedir)/FL/images
	$(INSTALL_DATA) pngprefix.h $(DESTDIR)$(includedir)/FL/images


#
# Uninstall everything...
#

uninstall:
	echo "Uninstalling libfltk_png$(LIBEXT) in $(libdir)..."
	$(RM) $(DESTDIR)$(libdir)/libfltk_png$(LIBEXT)
	echo "Uninstalling png headers in $(includedir)/FL/images..."
	$(RM) $(DESTDIR)$(includedir)/FL/images/png.h
	$(RM) $(DESTDIR)$(includedir)/FL/images/pngconf.h
	$(RM) $(DESTDIR)$(includedir)/FL/images/pnglibconf.h
	$(RM) $(DESTDIR)$(includedir)/FL/images/pngprefix.h


#
# libfltk_png.a
#

$(LIBPNG):	$(OBJS)
	echo Archiving $@...
	$(RM) $@
	$(LIBCOMMAND) $@ $(OBJS)
	$(RANLIB) $@


#
# Make dependencies...
#

depend: $(OBJS:.o=.c)
	makedepend -Y -I.. -f makedepend -w 20 $(OBJS:.o=.c)
	echo "# DO NOT DELETE THIS LINE -- make depend depends on it." > makedepend.tmp
	echo "" >> makedepend.tmp
	grep '^[a-zA-Z]' makedepend | ( LC_ALL=C sort -u -f >> makedepend.tmp; )
	mv makedepend.tmp makedepend

include makedepend

$(OBJS):	../makeinclude
