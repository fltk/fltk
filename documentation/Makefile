#
# "$Id: Makefile,v 1.4 1999/01/07 19:17:09 mike Exp $"
#
# Documentation makefile for the Fast Light Tool Kit (FLTK).
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

HTMLFILES	= \
		src/preface.html \
		src/intro.html \
		src/basics.html \
		src/common.html \
		src/editor.html \
		src/drawing.html \
		src/events.html \
		src/subclassing.html \
		src/fluid.html \
		src/opengl.html \
		src/widgets.html \
		src/Fl_Adjuster.html \
		src/Fl_Box.html \
		src/Fl_Browser.html \
		src/Fl_Browser_.html \
		src/Fl_Button.html \
		src/Fl_Chart.html \
		src/Fl_Check_Button.html \
		src/Fl_Choice.html \
		src/Fl_Clock.html \
		src/Fl_Color_Chooser.html \
		src/Fl_Counter.html \
		src/Fl_Dial.html \
		src/Fl_Double_Window.html \
		src/Fl_End.html \
		src/Fl_Float_Input.html \
		src/Fl_Free.html \
		src/Fl_Gl_Window.html \
		src/Fl_Group.html \
		src/Fl_Hold_Browser.html \
		src/Fl_Input.html \
		src/Fl_Input_.html \
		src/Fl_Int_Input.html \
		src/Fl_Light_Button.html \
		src/Fl_Menu_.html \
		src/Fl_Menu_Bar.html \
		src/Fl_Menu_Button.html \
		src/Fl_Menu_Item.html \
		src/Fl_Menu_Window.html \
		src/Fl_Multi_Browser.html \
		src/Fl_Multiline_Input.html \
		src/Fl_Multiline_Output.html \
		src/Fl_Output.html \
		src/Fl_Overlay_Window.html \
		src/Fl_Pack.html \
		src/Fl_Positioner.html \
		src/Fl_Repeat_Button.html \
		src/Fl_Return_Button.html \
		src/Fl_Roller.html \
		src/Fl_Round_Button.html \
		src/Fl_Scroll.html \
		src/Fl_Scrollbar.html \
		src/Fl_Secret_Input.html \
		src/Fl_Select_Browser.html \
		src/Fl_Single_Window.html \
		src/Fl_Slider.html \
		src/Fl_Tabs.html \
		src/Fl_Tile.html \
		src/Fl_Timer.html \
		src/Fl_Valuator.html \
		src/Fl_Value_Input.html \
		src/Fl_Value_Output.html \
		src/Fl_Value_Slider.html \
		src/Fl_Widget.html \
		src/Fl_Window.html \
		src/functions.html \
		src/enumerations.html \
		src/glut.html \
		src/forms.html \
		src/osissues.html \
		src/license.html

all:	index.html fltk.ps.gz fltk.pdf

clean:
	rm -f index.html toc.html doc*.html *.gif *.jpg
	rm -f fltk.ps fltk.ps.gz
	rm -f fltk.pdf

depend:

install:

index.html: $(HTMLFILES)
	@echo "Generating HTML documentation..."
	@rm -f index.html toc.html doc*.html *.gif *.jpg
	@htmldoc -d . -t html --toclevels 2 --title src/FL.gif $(HTMLFILES)

fltk.ps.gz: $(HTMLFILES)
	@echo "Generating PostScript documentation..."
	@rm -f fltk.ps fltk.ps.gz
	@htmldoc -f fltk.ps --jpeg=90 --duplex --verbose --toclevels 2 --title src/FL.gif $(HTMLFILES)
	@gzip -9 fltk.ps

fltk.pdf: $(HTMLFILES)
	@echo "Generating PDF documentation..."
	@rm -f fltk.pdf
	@htmldoc -f fltk.pdf --compression=9 --jpeg=90 --duplex --verbose --toclevels 2 --title src/FL.gif $(HTMLFILES)

#
# End of "$Id: Makefile,v 1.4 1999/01/07 19:17:09 mike Exp $".
#
