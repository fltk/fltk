#
# "$Id: Makefile,v 1.1.2.6 2004/09/08 16:04:42 easysw Exp $"
#
# Xutf8 library makefile for the Fast Light Toolkit (FLTK).
#
# Copyright 1997-2005 by Jean-Marc Lienher.
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

include ../../makeinclude


#
# Object files... set this based on the operating system and host
#
UNAME := $(shell uname)

ifeq '$(OS)' "Windows_NT"
OBJS	=	case.o is_right2left.o is_spacing.o 
endif # end of WIN32 options

ifeq ($(strip $(UNAME)),Linux)
OBJS	=	case.o is_right2left.o is_spacing.o \
		keysym2Ucs.o utf8Input.o utf8Utils.o \
		utf8Wrap.o 
endif # end of linux options

ifeq ($(strip $(UNAME)),Darwin)
OBJS	=	case.o is_right2left.o is_spacing.o 
endif # end of OSX options



XUTF8	=	../../lib/libfltk_xutf8$(LIBEXT)


#
# Make all targets...
#

all:	$(XUTF8) $(XDSONAME)


#
# Clean all targets and object files...
#

clean:
	$(RM) $(OBJS)
	$(RM) $(XUTF8) $(XDSONAME) libfltk_xutf8.so


#
# Install everything...
#

install: $(XUTF8)
	echo "Installing libfltk_xutf8$(LIBEXT) in $(libdir)..."
	-$(MKDIR) $(libdir)
	$(RM) $(libdir)/libfltk_xutf8$(LIBEXT)
	$(CP) $(XUTF8) $(libdir)
	$(RANLIB) $(libdir)/libfltk_xutf8$(LIBEXT)
	if test x$(XDSONAME) = xlibfltk_xutf8.so.1.1; then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_xutf8.so*;\
		$(CP) libfltk_xutf8.so.1.1 $(DESTDIR)$(libdir); \
		$(CHMOD) 755 $(DESTDIR)$(libdir)/libfltk_xutf8.so.1.1; \
		$(LN) libfltk_xutf8.so.1.1 $(DESTDIR)$(libdir)/libfltk_xutf8.so;\
	fi


#
# Uninstall everything...
#

uninstall:
	echo "Uninstalling libfltk_xutf8$(LIBEXT) in $(libdir)..."
	$(RM) $(libdir)/libfltk_xutf8$(LIBEXT)
	if test x$(XDSONAME) = xlibfltk_xutf8.so.1.1; then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_xutf8.so*;\
	fi


#
# libfltk_xutf8.a
#

$(XUTF8):	$(OBJS)
	echo Archiving $@...
	$(RM) $@
	$(LIBCOMMAND) $@ $(OBJS)
	$(RANLIB) $@

libfltk_xutf8.so.1.1: $(OBJS)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(OBJS)
	$(RM) libfltk_xutf8.so
	$(LN) libfltk_xutf8.so.1.1 libfltk_xutf8.so

#
# Make dependencies...
#

depend:	$(OBJS:.o=.c)
	makedepend -Y -I.. -f makedepend $(OBJS:.o=.c)

include makedepend

$(OBJS):	../../makeinclude


#
# End of "$Id: Makefile,v 1.1.2.6 2004/09/08 16:04:42 easysw Exp $".
#
# DO NOT DELETE
