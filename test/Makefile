#
# "$Id: Makefile,v 1.19.2.1 1999/03/25 15:26:43 mike Exp $"
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
	CubeMain.cxx CubeView.cxx \
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

ALL =	CubeView adjuster arc ask bitmap boxtype browser button buttons checkers \
	clock colbrowser color_chooser cube cursor curve demo doublebuffer \
	editor file_chooser fonts forms fractals fullscreen gl_overlay \
	glpuzzle hello iconize image input keyboard label list_visuals \
	mandelbrot menubar message minimum navigation output overlay pixmap \
	pixmap_browser radio resizebox scroll shape shiny subwindow \
	symbols tabs tile valuators fast_slow resize pack inactive

all:	$(ALL)

include ../makeinclude

$(ALL):  ../lib/$(LIBNAME)

.SUFFIXES: .cxx .c .o .fl .h

.cxx:
	$(CXX) -I.. $(CXXFLAGS) $< -L../lib -lfltk $(LDLIBS) -o $@

.cxx.o:
	$(CXX) -I.. $(CXXFLAGS) $< -c

.fl.cxx .fl.h:
	../fluid/fluid -c $<

.fl:
	../fluid/fluid -c $<
	$(CXX) -I.. $(CXXFLAGS) $@.cxx -L../lib -lfltk $(LDLIBS) -o $@

# Other programs needing special "help"...
CubeView: CubeMain.o CubeView.o CubeViewUI.o
	$(CXX) -I.. $(CXXFLAGS) CubeMain.o CubeView.o CubeViewUI.o \
		-L../lib -lfltk $(GLDLIBS) -o $@
CubeMain.o: CubeViewUI.h CubeView.h
CubeView.o: CubeView.h

cube: cube.cxx
	$(CXX) -I.. $(CXXFLAGS) cube.cxx -L../lib -lfltk $(GLDLIBS) -o $@
fractals: fractals.cxx
	$(CXX) -I.. $(CXXFLAGS) fractals.cxx -L../lib -lfltk $(GLDLIBS) -o $@
fullscreen: fullscreen.cxx
	$(CXX) -I.. $(CXXFLAGS) fullscreen.cxx -L../lib -lfltk $(GLDLIBS) -o $@
glpuzzle: glpuzzle.cxx
	$(CXX) -I.. $(CXXFLAGS) glpuzzle.cxx -L../lib -lfltk $(GLDLIBS) -o $@
gl_overlay: gl_overlay.cxx
	$(CXX) -I.. $(CXXFLAGS) gl_overlay.cxx -L../lib -lfltk $(GLDLIBS) -o $@
shiny: shiny.cxx shiny_panel.cxx
	$(CXX) -I.. $(CXXFLAGS) shiny.cxx -L../lib -lfltk $(GLDLIBS) -o $@
keyboard: keyboard.cxx keyboard_ui.cxx
	$(CXX) -I.. $(CXXFLAGS) keyboard.cxx -L../lib -lfltk $(LDLIBS) -o $@
mandelbrot: mandelbrot.cxx mandelbrot_ui.cxx
	$(CXX) -I.. $(CXXFLAGS) mandelbrot.cxx -L../lib -lfltk $(LDLIBS) -o $@
shape: shape.cxx
	$(CXX) -I.. $(CXXFLAGS) shape.cxx -L../lib -lfltk $(GLDLIBS) -o $@

# If you have libjpeg installed, you might want to try this test program:

jpeg_image: jpeg_image.cxx
	$(CXX) -I.. $(CXXFLAGS) -I../../../local/jpeg-6b -L../../../local/jpeg-6b jpeg_image.cxx -L../lib -lfltk $(LDLIBS) -ljpeg -lXext -o $@

depend:
	$(MAKEDEPEND) -I.. $(CXXFLAGS) $(CPPFILES) > makedepend

clean:
	-@ rm -f $(ALL) jpeg_image *.o core *~

install:
	@echo Nothing to install in test directory.

#
# End of "$Id: Makefile,v 1.19.2.1 1999/03/25 15:26:43 mike Exp $".
#
