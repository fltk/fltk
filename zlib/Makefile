#
# "$Id: Makefile,v 1.1.2.5 2004/09/06 11:20:53 easysw Exp $"
#
# GNU ZIP library makefile for the Fast Light Toolkit (FLTK).
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

include ../makeinclude


#
# Object files...
#

OBJS	=	adler32.o compress.o crc32.o gzio.o uncompr.o deflate.o \
		trees.o zutil.o inflate.o infblock.o inftrees.o infcodes.o \
		infutil.o inffast.o

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
	-$(MKDIR) $(libdir)
	$(RM) $(libdir)/libfltk_z$(LIBEXT)
	$(CP) $(LIBZ) $(libdir)
	$(RANLIB) $(libdir)/libfltk_z$(LIBEXT)
	echo "Installing zlib headers in $(includedir)/FL/images..."
	-$(MKDIR) $(includedir)/FL/images
	$(CP) zconf.h zlib.h zutil.h $(includedir)/FL/images


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

depend:	$(OBJS:.o=.c)
	makedepend -Y -I.. -f makedepend $(OBJS:.o=.c)

include makedepend

$(OBJS):	../makeinclude


#
# End of "$Id: Makefile,v 1.1.2.5 2004/09/06 11:20:53 easysw Exp $".
#
