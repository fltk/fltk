#
# "$Id: Makefile,v 1.9.2.10 2001/02/12 15:12:14 easysw Exp $"
#
# Documentation makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2001 by Bill Spitzak and others.
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

# Get configuration stuff...
include ../makeinclude

# What media size to use:

# "Universal" size - minimum of US Letter and A4
MEDIA		=	--size universal

# Standard US Letter
#MEDIA		=	--size letter

# Standard A4
#MEDIA		=	--size a4

# 7x8.5in (1/2 legal page) with reduced-size fonts
#MEDIA		=	--size 7x8.5in --fontsize 9.0

# These are the HTML "source" files; the files are formatted in the
# order listed...
HTMLFILES	= \
		preface.html \
		intro.html \
		basics.html \
		common.html \
		editor.html \
		drawing.html \
		events.html \
		subclassing.html \
		fluid.html \
		opengl.html \
		widgets.html \
		Fl_Adjuster.html \
		Fl_Box.html \
		Fl_Browser.html \
		Fl_Browser_.html \
		Fl_Button.html \
		Fl_Chart.html \
		Fl_Check_Button.html \
		Fl_Choice.html \
		Fl_Clock.html \
		Fl_Color_Chooser.html \
		Fl_Counter.html \
		Fl_Dial.html \
		Fl_Double_Window.html \
		Fl_End.html \
		Fl_Float_Input.html \
		Fl_Free.html \
		Fl_Gl_Window.html \
		Fl_Group.html \
		Fl_Hold_Browser.html \
		Fl_Input.html \
		Fl_Input_.html \
		Fl_Int_Input.html \
		Fl_Light_Button.html \
		Fl_Menu_.html \
		Fl_Menu_Bar.html \
		Fl_Menu_Button.html \
		Fl_Menu_Item.html \
		Fl_Menu_Window.html \
		Fl_Multi_Browser.html \
		Fl_Multiline_Input.html \
		Fl_Multiline_Output.html \
		Fl_Output.html \
		Fl_Overlay_Window.html \
		Fl_Pack.html \
		Fl_Positioner.html \
		Fl_Repeat_Button.html \
		Fl_Return_Button.html \
		Fl_Roller.html \
		Fl_Round_Button.html \
		Fl_Scroll.html \
		Fl_Scrollbar.html \
		Fl_Secret_Input.html \
		Fl_Select_Browser.html \
		Fl_Single_Window.html \
		Fl_Slider.html \
		Fl_Tabs.html \
		Fl_Tile.html \
		Fl_Timer.html \
		Fl_Valuator.html \
		Fl_Value_Input.html \
		Fl_Value_Output.html \
		Fl_Value_Slider.html \
		Fl_Widget.html \
		Fl_Window.html \
		functions.html \
		enumerations.html \
		glut.html \
		forms.html \
		osissues.html \
		license.html

MANPAGES =	fltk.$(CAT3EXT) fluid.$(CAT1EXT)

all:	fltk.ps fltk.pdf $(MANPAGES)

clean:
	rm -f fltk.ps
	rm -f fltk.pdf

depend:

# According to FHS this should be:
# htmldir = $(datadir)/doc/html/en/fltk
# Possibly the "en/" can be removed, it is not clear.
# KDE uses "HTML" instead of "html", which is correct?
docdir = $(datadir)/doc/fltk

install: $(MANPAGES)
	-mkdir -p $(docdir)
	cp $(HTMLFILES) *.gif *.jpg index.html $(docdir)
	chmod 644 $(docdir)/*
	-mkdir -p $(mandir)/cat1
	cp fluid.$(CAT1EXT) $(mandir)/cat1
	chmod 644 $(mandir)/cat1/fluid.$(CAT1EXT)
	-mkdir -p $(mandir)/cat3
	cp fltk.$(CAT3EXT) $(mandir)/cat3
	chmod 644 $(mandir)/cat3/fltk.$(CAT3EXT)
	-mkdir -p $(mandir)/man1
	cp fluid.man $(mandir)/man1/fluid.1
	chmod 644 $(mandir)/man1/fluid.1
	-mkdir -p $(mandir)/man3
	cp fltk.man $(mandir)/man3/fltk.3
	chmod 644 $(mandir)/man3/fltk.3

# Base html files are now the readable ones, so this target is not make by
# default...
fltk.d/index.html: $(HTMLFILES)
	echo "Generating HTML documentation..."
	-mkdir fltk.d
	-rm -f fltk.d/*
	$(HTMLDOC) -d fltk.d -t html --verbose --toclevels 2 --bodycolor white --titleimage FL.gif $(HTMLFILES)

fltk.ps: $(HTMLFILES)
	echo "Generating PostScript documentation..."
	$(HTMLDOC) -f fltk.ps --jpeg --duplex --verbose --toclevels 2 --titleimage FL.gif $(HTMLFILES)

fltk.pdf: $(HTMLFILES)
	echo "Generating PDF documentation..."
	rm -f fltk.pdf
	$(HTMLDOC) -f fltk.pdf --jpeg --compression=9 --duplex --verbose --toclevels 2 --titleimage FL.gif $(HTMLFILES)

#
# End of "$Id: Makefile,v 1.9.2.10 2001/02/12 15:12:14 easysw Exp $".
#
