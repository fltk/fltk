#
# "$Id: Makefile,v 1.7 1999/01/07 18:49:36 mike Exp $"
#
# Fluid makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-1999 by Bill Spitzak and others.
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
	Fl_Function_Type.cxx \
	Fl_Menu_Type.cxx \
	Fl_Group_Type.cxx \
	Fl_Widget_Type.cxx \
	Fl_Type.cxx \
	Fl_Window_Type.cxx \
	Fluid_Image.cxx \
	code.cxx \
	factory.cxx \
	file.cxx \
	fluid.cxx \
	about_panel.cxx \
	widget_panel.cxx \
	alignment_panel.cxx \
	function_panel.cxx \
	gif.cxx

################################################################

OBJECTS = $(CPPFILES:.cxx=.o)

include ../makeinclude

.SUFFIXES : .cxx .c .o

.cxx.o :
	@echo $<:
	@$(CXX) -I.. $(CXXFLAGS) -c $<
.c.o :
	@echo $<:
	@$(CC) -I.. $(CFLAGS) -c $<
.cxx :
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) -o $@ $< -L../lib -lfltk $(LDLIBS)

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
# Note: The rebuild target can only be used if you have the original .fl
#       files.  This is normally only used by the FLTK maintainers...
#

rebuild:
	./fluid -c about_panel.fl
	./fluid -c alignment_panel.fl
	./fluid -c function_panel.fl
	./fluid -c widget_panel.fl

#
# End of "$Id: Makefile,v 1.7 1999/01/07 18:49:36 mike Exp $".
#
