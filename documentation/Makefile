#
# "$Id$"
#
# Documentation makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2007 by Bill Spitzak and others.
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
# Please report all bugs and problems on the following page:
#
#      http://www.fltk.org/str.php
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
		preface.html \
		intro.html \
		basics.html \
		common.html \
		editor.html \
		drawing.html \
		events.html \
		subclassing.html \
		opengl.html \
		fluid.html \
		advanced.html \
		widgets.html \
		Fl.html \
		Fl_Adjuster.html \
		Fl_Bitmap.html \
		Fl_BMP_Image.html \
		Fl_Box.html \
		Fl_Browser_.html \
		Fl_Browser.html \
		Fl_Button.html \
		Fl_Chart.html \
		Fl_Check_Browser.html \
		Fl_Check_Button.html \
		Fl_Choice.html \
		Fl_Clock.html \
		Fl_Clock_Output.html \
		Fl_Color_Chooser.html \
		Fl_Counter.html \
		Fl_Dial.html \
		Fl_Double_Window.html \
		Fl_End.html \
		Fl_File_Browser.html \
		Fl_File_Chooser.html \
		Fl_File_Icon.html \
		Fl_File_Input.html \
		Fl_Float_Input.html \
		Fl_Free.html \
		Fl_GIF_Image.html \
		Fl_Gl_Window.html \
		Fl_Group.html \
		Fl_Help_Dialog.html \
		Fl_Help_View.html \
		Fl_Hold_Browser.html \
		Fl_Image.html \
		Fl_Input.html \
		Fl_Input_.html \
		Fl_Input_Choice.html \
		Fl_Int_Input.html \
		Fl_JPEG_Image.html \
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
		Fl_Pixmap.html \
		Fl_PNG_Image.html \
		Fl_PNM_Image.html \
		Fl_Positioner.html \
		Fl_Preferences.html \
		Fl_Progress.html \
		Fl_Repeat_Button.html \
		Fl_RGB_Image.html \
		Fl_Return_Button.html \
		Fl_Roller.html \
		Fl_Round_Button.html \
		Fl_Scroll.html \
		Fl_Scrollbar.html \
		Fl_Secret_Input.html \
		Fl_Select_Browser.html \
		Fl_Shared_Image.html \
		Fl_Single_Window.html \
		Fl_Slider.html \
		Fl_Spinner.html \
		Fl_Tabs.html \
		Fl_Text_Buffer.html \
		Fl_Text_Display.html \
		Fl_Text_Editor.html \
		Fl_Tile.html \
		Fl_Tiled_Image.html \
		Fl_Timer.html \
		Fl_Toggle_Button.html \
		Fl_Tooltip.html \
		Fl_Valuator.html \
		Fl_Value_Input.html \
		Fl_Value_Output.html \
		Fl_Value_Slider.html \
		Fl_Widget.html \
		Fl_Window.html \
		Fl_Wizard.html \
		Fl_XBM_Image.html \
		Fl_XPM_Image.html \
		functions.html \
		enumerations.html \
		glut.html \
		forms.html \
		osissues.html \
		migration.html \
		license.html \
		examples.html

IMAGEFILES	= \
		adjuster1.gif \
		boxtypes.gif \
		buttons.gif \
		charts.gif \
		choice.gif \
		clock.gif \
		counter.gif \
		cubeview.gif \
		dial.gif \
		editor.gif \
		editor-replace.gif \
		fl_alert.gif \
		fl_ask.gif \
		Fl_Check_Button.gif \
		fl_choice.gif \
		fl_color_chooser.jpg \
		Fl_File_Chooser.jpg \
		Fl_File_Input.gif \
		FL.gif \
		Fl_Help_Dialog.gif \
		fl_input.gif \
		Fl_Light_Button.gif \
		fl_message.gif \
		fl_password.gif \
		Fl_Return_Button.gif \
		Fl_Roller.gif \
		Fl_Round_Button.gif \
		Fl_Scroll.gif \
		fl_show_colormap.gif \
		Fl_Tile.gif \
		fluid1.gif \
		fluid2.gif \
		fluid3-cxx.gif \
		fluid3-gui.gif \
		fluid3-style.gif \
		fluid4.gif \
		fluid-catgets.gif \
		fluid-gettext.gif \
		fluid-org.gif \
		fluid_prefs.gif \
		fluid_widget_cxx.gif \
		fluid_widget_gui.gif \
		fluid_widget_style.gif \
		Fl_Value_Input.gif \
		Fl_Value_Output.gif \
		hello.C.gif \
		input_choice.jpg \
		menubar.gif \
		menu_button.gif \
		menu.gif \
		positioner.gif \
		resizebox1.gif \
		resizebox2.gif \
		round_clock.gif \
		scrollbar.gif \
		slider.gif \
		symbols.gif \
		tabs.gif \
		text.gif \
		valuators.gif \
		value_slider.gif


MANPAGES =	fltk.$(CAT3EXT) fltk-config.$(CAT1EXT) fluid.$(CAT1EXT) \
		blocks.$(CAT6EXT) checkers.$(CAT6EXT) sudoku.$(CAT6EXT)

all:	$(MANPAGES)

alldocs:	fltk.ps fltk.pdf fltk.d/index.html $(MANPAGES)

clean:
	$(RM) fltk.ps
	$(RM) fltk.pdf
	$(RMDIR) fltk.d
	$(RM) *~ *.bck *.bck

depend:

install: $(MANPAGES)
	echo "Installing documentation files in $(DESTDIR)$(docdir)..."
	-$(INSTALL_DIR) $(DESTDIR)$(docdir)
	for file in $(HTMLFILES) $(IMAGEFILES) index.html; do \
		$(INSTALL_DATA) $$file $(DESTDIR)$(docdir); \
	done
	echo "Installing man pages in $(DESTDIR)$(mandir)..."
	-$(INSTALL_DIR) $(DESTDIR)$(mandir)/cat1
	$(INSTALL_MAN) fluid.$(CAT1EXT) $(DESTDIR)$(mandir)/cat1
	$(INSTALL_MAN) fltk-config.$(CAT1EXT) $(DESTDIR)$(mandir)/cat1
	-$(INSTALL_DIR) $(DESTDIR)$(mandir)/cat3
	$(INSTALL_MAN) fltk.$(CAT3EXT) $(DESTDIR)$(mandir)/cat3
	-$(INSTALL_DIR) $(DESTDIR)$(mandir)/man1
	$(INSTALL_MAN) fluid.man $(DESTDIR)$(mandir)/man1/fluid.1
	$(INSTALL_MAN) fltk-config.man $(DESTDIR)$(mandir)/man1/fltk-config.1
	-$(INSTALL_DIR) $(DESTDIR)$(mandir)/man3
	$(INSTALL_MAN) fltk.man $(DESTDIR)$(mandir)/man3/fltk.3

install-linux install-osx:
	-$(INSTALL_DIR) $(DESTDIR)$(mandir)/cat6
	$(INSTALL_MAN) blocks.$(CAT6EXT) $(DESTDIR)$(mandir)/cat6
	$(INSTALL_MAN) checkers.$(CAT6EXT) $(DESTDIR)$(mandir)/cat6
	$(INSTALL_MAN) sudoku.$(CAT6EXT) $(DESTDIR)$(mandir)/cat6
	-$(INSTALL_DIR) $(DESTDIR)$(mandir)/man6
	$(INSTALL_MAN) blocks.man $(DESTDIR)$(mandir)/man6/blocks.6
	$(INSTALL_MAN) checkers.man $(DESTDIR)$(mandir)/man6/checkers.6
	$(INSTALL_MAN) sudoku.man $(DESTDIR)$(mandir)/man6/sudoku.6


uninstall:
	$(RMDIR) $(DESTDIR)$(docdir)
	$(RM) $(DESTDIR)$(mandir)/cat1/fluid.$(CAT1EXT)
	$(RM) $(DESTDIR)$(mandir)/man1/fluid.1
	$(RM) $(DESTDIR)$(mandir)/cat1/fltk-config.$(CAT1EXT)
	$(RM) $(DESTDIR)$(mandir)/man1/fltk-config.1
	$(RM) $(DESTDIR)$(mandir)/cat3/fltk.$(CAT3EXT)
	$(RM) $(DESTDIR)$(mandir)/man3/fltk.3

uninstall-linux uninstall-osx:
	$(RM) $(DESTDIR)$(mandir)/cat6/blocks.$(CAT6EXT)
	$(RM) $(DESTDIR)$(mandir)/cat6/checkers.$(CAT6EXT)
	$(RM) $(DESTDIR)$(mandir)/cat6/sudoku.$(CAT6EXT)
	$(RM) $(DESTDIR)$(mandir)/man6/blocks.6
	$(RM) $(DESTDIR)$(mandir)/man6/checkers.6
	$(RM) $(DESTDIR)$(mandir)/man6/sudoku.6

# Base html files are now the readable ones, so this target is not make by
# default...
fltk.d/index.html: $(HTMLFILES) $(IMAGEFILES)
	echo "Generating HTML documentation..."
	-$(RMDIR) fltk.d
	-$(INSTALL_DIR) fltk.d
	-$(HTMLDOC) --verbose --batch fltk.book -d fltk.d -t html

fltk.ps: $(HTMLFILES) $(IMAGEFILES)
	echo "Generating PostScript documentation..."
	$(RM) fltk.ps
	-$(HTMLDOC) --verbose --batch fltk.book $(MEDIA) -f fltk.ps

fltk.pdf: $(HTMLFILES) $(IMAGEFILES)
	echo "Generating PDF documentation..."
	$(RM) fltk.pdf
	-$(HTMLDOC) --verbose --batch fltk.book $(MEDIA) -f fltk.pdf

#
# End of "$Id$".
#
