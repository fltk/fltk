#
# "$Id: Makefile,v 1.15 1999/01/07 19:17:48 mike Exp $"
#
# Test/example program makefile for the Fast Light Tool Kit (FLTK).
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

CPPFILES =\
	adjuster.cxx arc.cxx ask.cxx bitmap.cxx boxtype.cxx browser.cxx button.cxx \
	buttons.cxx checkers.cxx clock.cxx colbrowser.cxx color_chooser.cxx \
	cube.cxx cursor.cxx curve.cxx demo.cxx doublebuffer.cxx editor.cxx \
	file_chooser.cxx fonts.cxx forms.cxx fractals.cxx fullscreen.cxx \
	gl_overlay.cxx glpuzzle.cxx hello.cxx iconize.cxx image.cxx input.cxx \
	keyboard.cxx label.cxx list_visuals.cxx mandelbrot.cxx menubar.cxx \
	message.cxx minimum.cxx navigation.cxx output.cxx overlay.cxx pixmap.cxx \
	pixmap_browser.cxx radio.cxx resizebox.cxx scroll.cxx shape.cxx shiny.cxx \
	subwindow.cxx symbols.cxx tabs.cxx tile.cxx valuators.cxx fast_slow.cxx \
	resize.cxx pack.cxx inactive.cxx

ALL =	adjuster arc ask bitmap boxtype browser button buttons checkers \
	clock colbrowser color_chooser cube cursor curve demo doublebuffer \
	editor file_chooser fonts forms fractals fullscreen gl_overlay \
	glpuzzle hello iconize image input keyboard label list_visuals \
	mandelbrot menubar message minimum navigation output overlay pixmap \
	pixmap_browser radio resizebox scroll shape shiny subwindow \
	symbols tabs tile valuators fast_slow resize pack inactive

all:	$(ALL)

include ../makeinclude

$(ALL):  ../lib/$(LIBNAME)

.SUFFIXES: .cxx .c .o .fl .H

.cxx:
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) $< -L../lib -lfltk $(LDLIBS) -o $@

.fl.cxx:
	@echo $@:
	@../fluid/fluid -c $<

.fl:
	@echo $@:
	@../fluid/fluid -c $<
	@$(CXX) -I.. $(CXXFLAGS) $@.cxx -L../lib -lfltk $(LDLIBS) -o $@

# Other programs needing special "help"...
shiny: shiny.cxx shiny_panel.cxx
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) shiny.cxx -L../lib -lfltk $(LDLIBS) -o $@
keyboard: keyboard.cxx keyboard_ui.cxx
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) keyboard.cxx -L../lib -lfltk $(LDLIBS) -o $@
mandelbrot: mandelbrot.cxx mandelbrot_ui.cxx
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) mandelbrot.cxx -L../lib -lfltk $(LDLIBS) -o $@

# If you have libjpeg installed, you might want to try this test program:

jpeg_image: jpeg_image.cxx
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) -I../../../local/jpeg-6b -L../../../local/jpeg-6b jpeg_image.cxx -L../lib -lfltk $(LDLIBS) -ljpeg -lXext -o $@

depend:
	$(MAKEDEPEND) -I.. $(CXXFLAGS) $(CPPFILES) > makedepend

clean:
	-@ rm -f $(ALL) jpeg_image *.o core *~

install:
	@echo Nothing to install in test directory.

#
# End of "$Id: Makefile,v 1.15 1999/01/07 19:17:48 mike Exp $".
#
