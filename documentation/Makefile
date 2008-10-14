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
HTMLFILES	=	 html/index.html \
                    	 src/index.dox \
                         src/preface.dox \
                         src/intro.dox \
                         src/basics.dox \
                         src/common.dox \
                         src/drawing.dox \
                         src/editor.dox \
                         src/events.dox \
                         src/subclassing.dox \
                         src/opengl.dox \
                         src/fluid.dox \
                         src/advanced.dox \
                         src/unicode.dox \
                         src/enumerations.dox \
                         src/glut.dox \
                         src/forms.dox \
                         src/osissues.dox \
                         src/migration_1_1.dox \
                         src/migration_1_3.dox \
                         src/development.dox \
                         src/license.dox \
                         src/examples.dox

IMAGEFILES	= \
		src/adjuster1.gif \
		src/boxtypes.gif \
		src/buttons.gif \
		src/charts.gif \
		src/choice.gif \
		src/clock.gif \
		src/counter.gif \
		src/cubeview.gif \
		src/dial.gif \
		src/editor.gif \
		src/editor-replace.gif \
		src/fl_alert.gif \
		src/fl_ask.gif \
		src/Fl_Check_Button.gif \
		src/fl_choice.gif \
		src/fl_color_chooser.jpg \
		src/Fl_File_Chooser.jpg \
		src/Fl_File_Input.gif \
		src/FL.gif \
		src/Fl_Help_Dialog.gif \
		src/fl_input.gif \
		src/Fl_Light_Button.gif \
		src/fl_message.gif \
		src/fl_password.gif \
		src/Fl_Return_Button.gif \
		src/Fl_Roller.gif \
		src/Fl_Round_Button.gif \
		src/Fl_Scroll.gif \
		src/fl_show_colormap.gif \
		src/Fl_Tile.gif \
		src/fluid1.gif \
		src/fluid2.gif \
		src/fluid3-cxx.gif \
		src/fluid3-gui.gif \
		src/fluid3-style.gif \
		src/fluid4.gif \
		src/fluid-catgets.gif \
		src/fluid-gettext.gif \
		src/fluid-org.gif \
		src/fluid_prefs.gif \
		src/fluid_widget_cxx.gif \
		src/fluid_widget_gui.gif \
		src/fluid_widget_style.gif \
		src/Fl_Value_Input.gif \
		src/Fl_Value_Output.gif \
		src/hello.C.gif \
		src/input_choice.jpg \
		src/menubar.gif \
		src/menu_button.gif \
		src/menu.gif \
		src/positioner.gif \
		src/resizebox1.gif \
		src/resizebox2.gif \
		src/round_clock.gif \
		src/scrollbar.gif \
		src/slider.gif \
		src/symbols.gif \
		src/tabs.gif \
		src/text.gif \
		src/valuators.gif \
		src/value_slider.gif


MANPAGES =	fltk.$(CAT3EXT) fltk-config.$(CAT1EXT) fluid.$(CAT1EXT) \
		blocks.$(CAT6EXT) checkers.$(CAT6EXT) sudoku.$(CAT6EXT)

all:	$(MANPAGES)

alldocs: pdf html $(MANPAGES)

clean:
#	$(RM) fltk.ps
	$(RM) refman.pdf
	$(RMDIR) html latex
	$(RM) *~ *.bck *.bck *.bak

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

html: html/index.html

html/index.html: $(IMAGEFILES)
	echo "Generating HTML documentation..."
	-$(RMDIR) html
	-$(INSTALL_DIR) html
	-$(HTMLDOC)

#fltk.ps: $(HTMLFILES) $(IMAGEFILES)
#	echo "Generating PostScript documentation..."
#	$(RM) fltk.ps
#	-$(HTMLDOC) --verbose --batch fltk.book $(MEDIA) -f fltk.ps

pdf: refman.pdf

refman.pdf: latex/refman.tex
	(cd latex; pdflatex refman.tex)
	(cd latex; makeindex refman.idx)
	(cd latex; pdflatex refman.tex)
	latex_count=5; \
	cd latex ; \
	while egrep -s 'Rerun (LaTeX|to get cross-references right)' refman.log && [ $$latex_count -gt 0 ] ;\
	    do \
	      echo "Rerunning latex...." ;\
	      pdflatex refman.tex ;\
	      latex_count=`expr $$latex_count - 1` ;\
	    done ; \
	cd .. 

latex/refman.tex: html/index.html

#
# End of "$Id$".
#
