#
# ZIP library makefile for the Fast Light Toolkit (FLTK).
#
# Copyright 1998-2023 by Bill Spitzak and others.
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

OBJS	=	adler32.o compress.o crc32.o uncompr.o deflate.o \
		trees.o zutil.o inflate.o inftrees.o inffast.o \
		gzclose.o gzlib.o gzread.o gzwrite.o infback.o

LIBZ	=	../lib/libfltk_z$(LIBEXT)

#
# Make all targets...
#

all:	$(LIBZ)

#
# Clean all targets and object files...
#

clean:
	$(RM) $(OBJS)
	$(RM) $(LIBZ)

#
# Install everything...
#

install: $(LIBZ)
	echo "Installing libfltk_z$(LIBEXT) in $(libdir)..."
	-$(INSTALL_DIR) $(DESTDIR)$(libdir)
	$(INSTALL_LIB) $(LIBZ) $(DESTDIR)$(libdir)
	$(RANLIB) $(DESTDIR)$(libdir)/libfltk_z$(LIBEXT)
	echo "Installing zlib headers in $(includedir)/FL/images..."
	-$(INSTALL_DIR) $(DESTDIR)$(includedir)/FL/images
	$(INSTALL_DATA) zconf.h $(DESTDIR)$(includedir)/FL/images
	$(INSTALL_DATA) zlib.h $(DESTDIR)$(includedir)/FL/images
	$(INSTALL_DATA) zutil.h $(DESTDIR)$(includedir)/FL/images

#
# Uninstall everything...
#

uninstall:
	echo "Uninstalling libfltk_z$(LIBEXT) in $(libdir)..."
	$(RM) $(libdir)/libfltk_z$(LIBEXT)
	echo "Uninstalling zlib headers in $(includedir)/FL/images..."
	$(RM) $(includedir)/FL/images/zconf.h
	$(RM) $(includedir)/FL/images/zlib.h
	$(RM) $(includedir)/FL/images/zutil.h

#
# libfltk_z.a
#

$(LIBZ):	$(OBJS)
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
