#
# "$Id: Makefile,v 1.1.2.3 2004/07/23 19:26:26 easysw Exp $"
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

LIBPNG	=	../lib/libpng$(LIBEXT)


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
	echo "Installing libpng$(LIBEXT) in $(libdir)..."
	-$(MKDIR) $(libdir)
	$(RM) $(libdir)/libpng$(LIBEXT)
	$(CP) $(LIBPNG) $(libdir)
	$(RANLIB) $(libdir)/libpng$(LIBEXT)


#
# Uninstall everything...
#

uninstall:
	echo "Uninstalling libpng$(LIBEXT) in $(libdir)..."
	$(RM) $(libdir)/libpng$(LIBEXT)


#
# libpng.a
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
# End of "$Id: Makefile,v 1.1.2.3 2004/07/23 19:26:26 easysw Exp $".
#
