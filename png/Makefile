#
# "$Id: Makefile,v 1.1.2.5 2004/09/06 11:20:53 easysw Exp $"
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

include ../makeinclude

#
# Object files...
#

OBJS	=	png.o pngset.o pngget.o pngrutil.o pngtrans.o pngwutil.o \
		pngread.o pngrio.o pngwio.o pngwrite.o pngrtran.o \
		pngwtran.o pngmem.o pngerror.o pngpread.o

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
	-$(MKDIR) $(libdir)
	$(RM) $(libdir)/libfltk_png$(LIBEXT)
	$(CP) $(LIBPNG) $(libdir)
	$(RANLIB) $(libdir)/libfltk_png$(LIBEXT)
	echo "Installing png headers in $(includedir)/FL/images..."
	-$(MKDIR) $(includedir)/FL/images
	$(CP) png.h pngconf.h $(includedir)/FL/images


#
# Uninstall everything...
#

uninstall:
	echo "Uninstalling libfltk_png$(LIBEXT) in $(libdir)..."
	$(RM) $(libdir)/libfltk_png$(LIBEXT)
	echo "Uninstalling png headers in $(includedir)/FL/images..."
	$(RM) $(includedir)/FL/images/png.h
	$(RM) $(includedir)/FL/images/pngconf.h


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

depend:	$(OBJS:.o=.c)
	makedepend -Y -I.. -f makedepend $(OBJS:.o=.c)

include makedepend

$(OBJS):	../makeinclude


#
# End of "$Id: Makefile,v 1.1.2.5 2004/09/06 11:20:53 easysw Exp $".
#
