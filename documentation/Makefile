#
# "$Id$"
#
# Documentation makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2009 by Bill Spitzak and others.
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
SRC_DOCDIR      = 	./src
# Standard US Letter
#MEDIA		=	--size letter

# Standard A4
#MEDIA		=	--size a4

# 7x8.5in (1/2 legal page) with reduced-size fonts
#MEDIA		=	--size 7x8.5in --fontsize 9.0

# These are the HTML "source" files...
HTMLFILES	= 	 \
			 $(SRC_DOCDIR)/index.dox \
                         $(SRC_DOCDIR)/preface.dox \
                         $(SRC_DOCDIR)/intro.dox \
                         $(SRC_DOCDIR)/basics.dox \
                         $(SRC_DOCDIR)/common.dox \
                         $(SRC_DOCDIR)/drawing.dox \
                         $(SRC_DOCDIR)/editor.dox \
                         $(SRC_DOCDIR)/events.dox \
                         $(SRC_DOCDIR)/subclassing.dox \
                         $(SRC_DOCDIR)/opengl.dox \
                         $(SRC_DOCDIR)/fluid.dox \
                         $(SRC_DOCDIR)/advanced.dox \
                         $(SRC_DOCDIR)/unicode.dox \
                         $(SRC_DOCDIR)/enumerations.dox \
                         $(SRC_DOCDIR)/glut.dox \
                         $(SRC_DOCDIR)/forms.dox \
                         $(SRC_DOCDIR)/osissues.dox \
                         $(SRC_DOCDIR)/migration_1_1.dox \
                         $(SRC_DOCDIR)/migration_1_3.dox \
                         $(SRC_DOCDIR)/development.dox \
                         $(SRC_DOCDIR)/license.dox \
                         $(SRC_DOCDIR)/examples.dox

IMAGEFILES	= \
		$(SRC_DOCDIR)/adjuster1.gif \
		$(SRC_DOCDIR)/boxtypes.gif \
		$(SRC_DOCDIR)/buttons.gif \
		$(SRC_DOCDIR)/charts.gif \
		$(SRC_DOCDIR)/choice.gif \
		$(SRC_DOCDIR)/clock.gif \
		$(SRC_DOCDIR)/counter.gif \
		$(SRC_DOCDIR)/cubeview.gif \
		$(SRC_DOCDIR)/dial.gif \
		$(SRC_DOCDIR)/editor.gif \
		$(SRC_DOCDIR)/editor-replace.gif \
		$(SRC_DOCDIR)/fl_alert.gif \
		$(SRC_DOCDIR)/fl_ask.gif \
		$(SRC_DOCDIR)/Fl_Check_Button.gif \
		$(SRC_DOCDIR)/fl_choice.gif \
		$(SRC_DOCDIR)/fl_color_chooser.jpg \
		$(SRC_DOCDIR)/Fl_File_Chooser.jpg \
		$(SRC_DOCDIR)/Fl_File_Input.gif \
		$(SRC_DOCDIR)/FL.gif \
		$(SRC_DOCDIR)/Fl_Help_Dialog.gif \
		$(SRC_DOCDIR)/fl_input.gif \
		$(SRC_DOCDIR)/Fl_Light_Button.gif \
		$(SRC_DOCDIR)/fl_message.gif \
		$(SRC_DOCDIR)/fl_password.gif \
		$(SRC_DOCDIR)/Fl_Return_Button.gif \
		$(SRC_DOCDIR)/Fl_Roller.gif \
		$(SRC_DOCDIR)/Fl_Round_Button.gif \
		$(SRC_DOCDIR)/Fl_Scroll.gif \
		$(SRC_DOCDIR)/fl_show_colormap.gif \
		$(SRC_DOCDIR)/Fl_Tile.gif \
		$(SRC_DOCDIR)/fluid1.gif \
		$(SRC_DOCDIR)/fluid2.gif \
		$(SRC_DOCDIR)/fluid3-cxx.gif \
		$(SRC_DOCDIR)/fluid3-gui.gif \
		$(SRC_DOCDIR)/fluid3-style.gif \
		$(SRC_DOCDIR)/fluid4.gif \
		$(SRC_DOCDIR)/fluid-catgets.gif \
		$(SRC_DOCDIR)/fluid-gettext.gif \
		$(SRC_DOCDIR)/fluid-org.gif \
		$(SRC_DOCDIR)/fluid_prefs.gif \
		$(SRC_DOCDIR)/fluid_widget_cxx.gif \
		$(SRC_DOCDIR)/fluid_widget_gui.gif \
		$(SRC_DOCDIR)/fluid_widget_style.gif \
		$(SRC_DOCDIR)/Fl_Value_Input.gif \
		$(SRC_DOCDIR)/Fl_Value_Output.gif \
		$(SRC_DOCDIR)/hello.C.gif \
		$(SRC_DOCDIR)/input_choice.jpg \
		$(SRC_DOCDIR)/menubar.gif \
		$(SRC_DOCDIR)/menu_button.gif \
		$(SRC_DOCDIR)/menu.gif \
		$(SRC_DOCDIR)/positioner.gif \
		$(SRC_DOCDIR)/resizebox1.gif \
		$(SRC_DOCDIR)/resizebox2.gif \
		$(SRC_DOCDIR)/round_clock.gif \
		$(SRC_DOCDIR)/scrollbar.gif \
		$(SRC_DOCDIR)/slider.gif \
		$(SRC_DOCDIR)/symbols.gif \
		$(SRC_DOCDIR)/tabs.gif \
		$(SRC_DOCDIR)/text.gif \
		$(SRC_DOCDIR)/valuators.gif \
		$(SRC_DOCDIR)/value_slider.gif

EPSFILES =	\
		$(SRC_DOCDIR)/FL.eps \
		$(SRC_DOCDIR)/FL200.eps\
		$(SRC_DOCDIR)/Fl_Check_Button.eps \
		$(SRC_DOCDIR)/Fl_File_Chooser.eps \
		$(SRC_DOCDIR)/Fl_File_Input.eps \
		$(SRC_DOCDIR)/Fl_Help_Dialog.eps \
		$(SRC_DOCDIR)/Fl_Light_Button.eps \
		$(SRC_DOCDIR)/Fl_Return_Button.eps \
		$(SRC_DOCDIR)/Fl_Roller.eps \
		$(SRC_DOCDIR)/Fl_Round_Button.eps \
		$(SRC_DOCDIR)/Fl_Scroll.eps \
		$(SRC_DOCDIR)/Fl_Tile.eps \
		$(SRC_DOCDIR)/Fl_Value_Input.eps \
		$(SRC_DOCDIR)/Fl_Value_Output.eps \
		$(SRC_DOCDIR)/adjuster1.eps \
		$(SRC_DOCDIR)/boxtypes.eps \
		$(SRC_DOCDIR)/buttons.eps \
		$(SRC_DOCDIR)/charts.eps \
		$(SRC_DOCDIR)/choice.eps \
		$(SRC_DOCDIR)/clock.eps \
		$(SRC_DOCDIR)/counter.eps \
		$(SRC_DOCDIR)/cubeview.eps \
		$(SRC_DOCDIR)/dial.eps \
		$(SRC_DOCDIR)/editor-replace.eps \
		$(SRC_DOCDIR)/editor.eps \
		$(SRC_DOCDIR)/fl_alert.eps \
		$(SRC_DOCDIR)/fl_ask.eps \
		$(SRC_DOCDIR)/fl_choice.eps \
		$(SRC_DOCDIR)/fl_color_chooser.eps \
		$(SRC_DOCDIR)/fl_input.eps \
		$(SRC_DOCDIR)/fl_message.eps \
		$(SRC_DOCDIR)/fl_password.eps \
		$(SRC_DOCDIR)/fl_show_colormap.eps \
		$(SRC_DOCDIR)/fluid-catgets.eps \
		$(SRC_DOCDIR)/fluid-gettext.eps \
		$(SRC_DOCDIR)/fluid-org.eps \
		$(SRC_DOCDIR)/fluid1.eps \
		$(SRC_DOCDIR)/fluid2.eps \
		$(SRC_DOCDIR)/fluid3-cxx.eps \
		$(SRC_DOCDIR)/fluid3-gui.eps \
		$(SRC_DOCDIR)/fluid3-style.eps \
		$(SRC_DOCDIR)/fluid4.eps \
		$(SRC_DOCDIR)/fluid_prefs.eps \
		$(SRC_DOCDIR)/fluid_widget_cxx.eps \
		$(SRC_DOCDIR)/fluid_widget_gui.eps \
		$(SRC_DOCDIR)/fluid_widget_style.eps \
		$(SRC_DOCDIR)/hello.C.eps \
		$(SRC_DOCDIR)/input_choice.eps \
		$(SRC_DOCDIR)/menu.eps \
		$(SRC_DOCDIR)/menu_button.eps \
		$(SRC_DOCDIR)/menubar.eps \
		$(SRC_DOCDIR)/positioner.eps \
		$(SRC_DOCDIR)/resizebox1.eps \
		$(SRC_DOCDIR)/resizebox2.eps \
		$(SRC_DOCDIR)/round_clock.eps \
		$(SRC_DOCDIR)/scrollbar.eps \
		$(SRC_DOCDIR)/slider.eps \
		$(SRC_DOCDIR)/symbols.eps \
		$(SRC_DOCDIR)/tabs.eps \
		$(SRC_DOCDIR)/text.eps \
		$(SRC_DOCDIR)/tiny.eps \
		$(SRC_DOCDIR)/valuators.eps \
		$(SRC_DOCDIR)/value_slider.eps

MANPAGES =	$(SRC_DOCDIR)/fltk.$(CAT3EXT) $(SRC_DOCDIR)/fltk-config.$(CAT1EXT) \
		$(SRC_DOCDIR)/fluid.$(CAT1EXT) $(SRC_DOCDIR)/blocks.$(CAT6EXT) \
		$(SRC_DOCDIR)/checkers.$(CAT6EXT) $(SRC_DOCDIR)/sudoku.$(CAT6EXT)

all:	$(MANPAGES)

alldocs: pdf-dist html $(MANPAGES)

clean:
#	$(RM) fltk.ps
	$(RM) refman.pdf
	$(RMDIR) html latex
	$(RM) *~ *.bck *.bck *.bak *.log

depend:

install: $(MANPAGES)
	echo "Installing documentation files in $(DESTDIR)$(docdir)..."
	-$(INSTALL_DIR) $(DESTDIR)$(docdir)
	echo Installing file from $(INST_DOC_DIR)
	if test -f html/index.html ; then \
	  for file in html/* ; do \
		$(INSTALL_DATA) $$file $(DESTDIR)$(docdir); \
	  done \
	fi
	$(INSTALL_DATA) fltk.pdf $(DESTDIR)$(docdir); \
	echo "Installing man pages in $(DESTDIR)$(mandir)..."
	-$(INSTALL_DIR) $(DESTDIR)$(mandir)/cat1
	$(INSTALL_MAN) $(SRC_DOCDIR)/fluid.$(CAT1EXT) $(DESTDIR)$(mandir)/cat1
	$(INSTALL_MAN) $(SRC_DOCDIR)/fltk-config.$(CAT1EXT) $(DESTDIR)$(mandir)/cat1
	-$(INSTALL_DIR) $(DESTDIR)$(mandir)/cat3
	$(INSTALL_MAN) $(SRC_DOCDIR)/fltk.$(CAT3EXT) $(DESTDIR)$(mandir)/cat3
	-$(INSTALL_DIR) $(DESTDIR)$(mandir)/man1
	$(INSTALL_MAN) $(SRC_DOCDIR)/fluid.man $(DESTDIR)$(mandir)/man1/fluid.1
	$(INSTALL_MAN) $(SRC_DOCDIR)/fltk-config.man $(DESTDIR)$(mandir)/man1/fltk-config.1
	-$(INSTALL_DIR) $(DESTDIR)$(mandir)/man3
	$(INSTALL_MAN) $(SRC_DOCDIR)/fltk.man $(DESTDIR)$(mandir)/man3/fltk.3

install-linux install-osx:
	-$(INSTALL_DIR) $(DESTDIR)$(mandir)/cat6
	$(INSTALL_MAN) $(SRC_DOCDIR)/blocks.$(CAT6EXT) $(DESTDIR)$(mandir)/cat6
	$(INSTALL_MAN) $(SRC_DOCDIR)/checkers.$(CAT6EXT) $(DESTDIR)$(mandir)/cat6
	$(INSTALL_MAN) $(SRC_DOCDIR)/sudoku.$(CAT6EXT) $(DESTDIR)$(mandir)/cat6
	-$(INSTALL_DIR) $(DESTDIR)$(mandir)/man6
	$(INSTALL_MAN) $(SRC_DOCDIR)/blocks.man $(DESTDIR)$(mandir)/man6/blocks.6
	$(INSTALL_MAN) $(SRC_DOCDIR)/checkers.man $(DESTDIR)$(mandir)/man6/checkers.6
	$(INSTALL_MAN) $(SRC_DOCDIR)/sudoku.man $(DESTDIR)$(mandir)/man6/sudoku.6


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

html/index.html: $(HTMLFILES) $(IMAGEFILES) Doxyfile
	echo "Generating HTML documentation..."
	-$(RMDIR) html
	-$(INSTALL_DIR) html
	-$(DOXYDOC)

html-dist: html
	echo "Stripping HTML files..."
	./strip_tags

#fltk.ps: $(HTMLFILES) $(IMAGEFILES)
#	echo "Generating PostScript documentation..."
#	$(RM) fltk.ps
#	-$(HTMLDOC) --verbose --batch $(SRC_DOCDIR)/fltk.book $(MEDIA) -f fltk.ps

pdf-dist: latex/refman.pdf
	cp -f latex/refman.pdf fltk.pdf

pdf: 	latex/refman.pdf

latex/refman.pdf: latex/refman.tex Doxybook $(EPSFILES)
	echo "Generating PDF documentation..."
	$(DOXYDOC) Doxybook ;\
	(cd latex ;\
	pdflatex --interaction=nonstopmode refman.tex ;\
	makeindex refman.idx ;\
	pdflatex --interaction=nonstopmode refman.tex ;\
	latex_count=5;\
	while egrep -s 'Rerun (LaTeX|to get cross-references right)' refman.log \
		&& [ $$latex_count -gt 0 ] ;\
	    do \
	      echo "Rerunning latex...." ;\
	      pdflatex --interaction=nonstopmode refman.tex ;\
	      latex_count=`expr $$latex_count - 1` ;\
	    done ; \
	cd ..) > pdfall.log

latex/refman.tex: Doxybook $(EPSFILES)

#
# End of "$Id$".
#
