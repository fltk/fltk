#
# "$Id: Makefile,v 1.9.2.10.2.9 2002/01/07 18:05:32 easysw Exp $"
#
# Documentation makefile for the Fast Light Tool Kit (FLTK).
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

# These are the HTML "source" files...
HTMLFILES	= \
			basics.html \
			common.html \
			drawing.html \
			editor.html \
			enumerations.html \
			events.html \
			Fl.html \
			Fl_Adjuster.html \
			Fl_Bitmap.html \
			Fl_Box.html \
			Fl_Browser_.html \
			Fl_Browser.html \
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
			Fl_File_Browser.html \
			Fl_File_Chooser.html \
			Fl_File_Icon.html \
			Fl_Float_Input.html \
			Fl_Free.html \
			Fl_GIF_Image.html \
			Fl_Gl_Window.html \
			Fl_Group.html \
			Fl_Help_Dialog.html \
			Fl_Help_View.html \
			Fl_Hold_Browser.html \
			Fl.html \
			Fl_Image.html \
			Fl_Input_.html \
			Fl_Input.html \
			Fl_Int_Input.html \
			Fl_JPEG_Image.html \
			Fl_Light_Button.html \
			Fl_Menu_Bar.html \
			Fl_Menu_Button.html \
			Fl_Menu_.html \
			Fl_Menu_Item.html \
			Fl_Menu_Window.html \
			Fl_Multi_Browser.html \
			Fl_Multiline_Input.html \
			Fl_Multiline_Output.html \
			Fl_Output.html \
			Fl_Overlay_Window.html \
			Fl_Pack.html \
			Fl_Pixmap.html \
			Fl_PNG_Image.html \
			Fl_PNM_Image.html \
			Fl_Positioner.html \
			Fl_Repeat_Button.html \
			Fl_Return_Button.html \
			Fl_RGB_Image.html \
			Fl_Roller.html \
			Fl_Round_Button.html \
			Fl_Scrollbar.html \
			Fl_Scroll.html \
			Fl_Secret_Input.html \
			Fl_Select_Browser.html \
			Fl_Shared_Image.html \
			Fl_Single_Window.html \
			Fl_Slider.html \
			Fl_Tabs.html \
			Fl_Text_Buffer.html \
			Fl_Text_Display.html \
			Fl_Text_Editor.html \
			Fl_Tiled_Image.html \
			Fl_Tile.html \
			Fl_Timer.html \
			Fl_Tooltip.html \
			fluid.html \
			Fl_Valuator.html \
			Fl_Value_Input.html \
			Fl_Value_Output.html \
			Fl_Value_Slider.html \
			Fl_Widget.html \
			Fl_Window.html \
			Fl_Wizard.html \
			Fl_XBM_Image.html \
			Fl_XPM_Image.html \
			forms.html \
			functions.html \
			glut.html \
			intro.html \
			license.html \
			migration.html \
			opengl.html \
			osissues.html \
			preface.html \
			subclassing.html \
			widgets.html

MANPAGES =	fltk.$(CAT3EXT) fltk-config.$(CAT1EXT) fluid.$(CAT1EXT)

all:	$(MANPAGES)

alldocs:	fltk.ps fltk.pdf fltk.d/index.html $(MANPAGES)

clean:
	rm -f fltk.ps
	rm -f fltk.pdf
	rm -rf fltk.d

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
	cp fltk-config.$(CAT1EXT) $(mandir)/cat1
	chmod 644 $(mandir)/cat1/fltk-config.$(CAT1EXT)
	-mkdir -p $(mandir)/cat3
	cp fltk.$(CAT3EXT) $(mandir)/cat3
	chmod 644 $(mandir)/cat3/fltk.$(CAT3EXT)
	-mkdir -p $(mandir)/man1
	cp fluid.man $(mandir)/man1/fluid.1
	chmod 644 $(mandir)/man1/fluid.1
	cp fltk-config.man $(mandir)/man1/fltk-config.1
	chmod 644 $(mandir)/man1/fltk-config.1
	-mkdir -p $(mandir)/man3
	cp fltk.man $(mandir)/man3/fltk.3
	chmod 644 $(mandir)/man3/fltk.3


uninstall:
	rm -rf $(docdir)
	rm -f $(mandir)/cat1/fluid.$(CAT1EXT)
	rm -f $(mandir)/man1/fluid.1
	rm -f $(mandir)/cat3/fltk.$(CAT3EXT)
	rm -f $(mandir)/man3/fltk.3


# Base html files are now the readable ones, so this target is not make by
# default...
fltk.d/index.html: $(HTMLFILES)
	echo "Generating HTML documentation..."
	-mkdir fltk.d
	-rm -f fltk.d/*
	$(HTMLDOC) --verbose --batch fltk.book -d fltk.d -t html

fltk.ps: $(HTMLFILES)
	echo "Generating PostScript documentation..."
	rm -f fltk.ps
	$(HTMLDOC) --verbose --batch fltk.book -f fltk.ps

fltk.pdf: $(HTMLFILES)
	echo "Generating PDF documentation..."
	rm -f fltk.pdf
	$(HTMLDOC) --verbose --batch fltk.book -f fltk.pdf

#
# End of "$Id: Makefile,v 1.9.2.10.2.9 2002/01/07 18:05:32 easysw Exp $".
#
