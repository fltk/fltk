#
# "$Id: Makefile,v 1.10.2.6.2.17 2002/03/28 13:20:11 easysw Exp $"
#
# FLUID makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2002 by Bill Spitzak and others.
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

PROGRAM = fluid$(EXEEXT)

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
	function_panel.cxx

################################################################

OBJECTS = $(CPPFILES:.cxx=.o)

CLEAN	= core

include ../makeinclude

$(PROGRAM) : $(OBJECTS) ../lib/$(LIBNAME)
	echo Linking $@...
	$(CXX) $(CXXFLAGS) -o $(PROGRAM) $(OBJECTS) $(LINKFLTKFORMS) \
		$(IMAGELIBS) $(LDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r

clean :
	-@ rm -f *.o $(PROGRAM) $(CLEAN)

depend:	$(CPPFILES)
	makedepend -Y -I.. -f makedepend $(CPPFILES)

# Automatically generated dependencies...
include makedepend

install: $(PROGRAM)
	echo "Installing FLUID in $(bindir)..."
	-mkdir -p $(bindir)
	cp $(PROGRAM) $(bindir)
	strip $(bindir)/$(PROGRAM)
	chmod 755 $(bindir)/$(PROGRAM)

uninstall:
	rm -f $(bindir)/$(PROGRAM)

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
# End of "$Id: Makefile,v 1.10.2.6.2.17 2002/03/28 13:20:11 easysw Exp $".
#
