#
# "$Id"
#
# Fluid makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998 by Bill Spitzak and others.
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
# Please report all bugs and problems to "fltk-bugs@easysw.com".
#

PROGRAM = fluid

CPPFILES = \
	Fl_Function_Type.C \
	Fl_Menu_Type.C \
	Fl_Group_Type.C \
	Fl_Widget_Type.C \
	Fl_Type.C \
	Fl_Window_Type.C \
	Fluid_Image.C \
	code.C \
	factory.C \
	file.C \
	fluid.C \
	about_panel.C \
	widget_panel.C \
	alignment_panel.C \
	function_panel.C \
	gif.C

################################################################

OBJECTS = $(CPPFILES:.C=.o)

include ../makeinclude

.SUFFIXES : .C .c .o .do .fl .H

.C.o :
	@echo $<:
	@$(CXX) -I.. $(CXXFLAGS) -c $<
.c.o :
	@echo $<:
	@$(CC) -I.. $(CFLAGS) -c $<
.C :
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) -o $@ $< -L../lib -lfltk $(LDLIBS)

.fl.C .fl.H :
	-fluid -c $<

# better way to do this for gnu make:
# %.H %.C: %.fl
#	fluid -c $<

$(PROGRAM) : $(OBJECTS) ../lib/$(LIBNAME)
	@echo linking $@:
	@$(CXX) $(CXXFLAGS) -o $(PROGRAM) $(OBJECTS) -L../lib -lfltk $(LDLIBS)

clean :
	-@ rm -f *.o $(PROGRAM) $(CLEAN) core *~ makedepend
	@touch makedepend

depend:
	$(MAKEDEPEND) -I.. $(CXXFLAGS) $(CPPFILES) $(CFILES) > makedepend
include makedepend

install: $(PROGRAM)
	strip $(PROGRAM)
	-mkdir -p $(bindir)
	cp $(PROGRAM) $(bindir)/$(PROGRAM)
	@chmod a+rx,g-w,o-w $(bindir)/$(PROGRAM)

uninstall:
	-@ rm -f $(bindir)/$(PROGRAM)

#
# End of "$Id: Makefile,v 1.2 1998/10/20 21:06:18 mike Exp $".
#
