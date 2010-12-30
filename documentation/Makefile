#
# "$Id$"
#
# Documentation makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2010 by Bill Spitzak and others.
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

SRC_DOCDIR      = 	./src

# These are the HTML "source" files...
HTMLFILES	= 	 \
			 $(SRC_DOCDIR)/index.dox \
                         $(SRC_DOCDIR)/preface.dox \
                         $(SRC_DOCDIR)/intro.dox \
                         $(SRC_DOCDIR)/basics.dox \
                         $(SRC_DOCDIR)/common.dox \
                         $(SRC_DOCDIR)/editor.dox \
                         $(SRC_DOCDIR)/drawing.dox \
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

MANPAGES =	$(SRC_DOCDIR)/fltk.$(CAT3EXT) $(SRC_DOCDIR)/fltk-config.$(CAT1EXT) \
		$(SRC_DOCDIR)/fluid.$(CAT1EXT) $(SRC_DOCDIR)/blocks.$(CAT6EXT) \
		$(SRC_DOCDIR)/checkers.$(CAT6EXT) $(SRC_DOCDIR)/sudoku.$(CAT6EXT)

all:	$(MANPAGES)

alldocs: pdf-dist html $(MANPAGES)

clean:
	$(RM) refman.pdf
	$(RMDIR) html latex
	$(RM) *~ *.bck *.bak *.log
	$(RM) $(MANPAGES) $(SRC_DOCDIR)/*.0

depend:

install: $(MANPAGES)
	echo "Installing documentation files in $(DESTDIR)$(docdir) ..."
	-$(INSTALL_DIR) $(DESTDIR)$(docdir)
	if test -f html/index.html ; then \
	  for file in html/* ; do \
		$(INSTALL_DATA) $$file $(DESTDIR)$(docdir); \
	  done \
	fi
	if test -f fltk.pdf ; then \
	  echo "Installing fltk.pdf in $(DESTDIR)$(docdir) ..."; \
	  $(INSTALL_DATA) fltk.pdf $(DESTDIR)$(docdir); \
	fi
	echo "Installing man pages in $(DESTDIR)$(mandir) ..."
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

# The HTML files are now generated using doxygen, and this needs
# an installed doxygen version and may take some time, so this target
# is not made by default.

html: html/index.html

html/index.html: $(HTMLFILES) Doxyfile
	echo "Generating HTML documentation..."
	-$(RMDIR) html
	-$(INSTALL_DIR) html
	-$(DOXYDOC)
	if test "x$(DOXYDOC)" = "x" ; then \
	  echo "Sorry - doxygen not found. Please install doxygen and run configure."; \
	fi

html-dist: html
	echo "Stripping HTML files ..."
	./strip_tags

pdf-dist: latex/refman.pdf
	cp -f latex/refman.pdf fltk.pdf

pdf: 	latex/refman.pdf

latex/refman.pdf: latex/refman.tex Doxybook
	echo "Generating PDF documentation ..."
	$(DOXYDOC) Doxybook ;\
	(cd latex ;\
	pdflatex --interaction=nonstopmode refman.tex ;\
	makeindex refman.idx ;\
	pdflatex --interaction=nonstopmode refman.tex ;\
	latex_count=5;\
	while egrep -s 'Rerun (LaTeX|to get cross-references right)' refman.log \
		&& [ $$latex_count -gt 0 ] ;\
	    do \
	      echo "Rerunning latex ..." ;\
	      pdflatex --interaction=nonstopmode refman.tex ;\
	      latex_count=`expr $$latex_count - 1` ;\
	    done ; \
	cd ..) > pdfall.log

latex/refman.tex: Doxybook

#
# End of "$Id$".
#
