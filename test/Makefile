#
# "$Id: Makefile,v 1.10 1998/10/21 20:10:23 mike Exp $"
#
# Test/example program makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998 by Bill Spitzak and others.
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
	adjuster.C arc.C ask.C bitmap.C boxtype.C browser.C button.C \
	buttons.C checkers.C  clock.C colbrowser.C color_chooser.C \
	cube.C cursor.C curve.C demo.C doublebuffer.C  file_chooser.C \
	fonts.C forms.C fractals.C fullscreen.C gl_overlay.C \
	glpuzzle.C  hello.C iconize.C image.C input.C keyboard.C \
	label.C list_visuals.C mandelbrot.C  menubar.C message.C \
	minimum.C navigation.C output.C overlay.C pixmap.C  \
	pixmap_browser.C radio.C resizebox.C scroll.C shape.C shiny.C \
	subwindow.C  symbols.C tabs.C tile.C valuators.C fast_slow.C \
	resize.C pack.C inactive.C style.C

ALL =	adjuster arc ask bitmap boxtype browser button buttons checkers \
	clock colbrowser color_chooser cube cursor curve demo doublebuffer \
	file_chooser fonts forms fractals fullscreen gl_overlay glpuzzle \
	hello iconize image input keyboard label list_visuals mandelbrot \
	menubar message minimum navigation output overlay pixmap \
	pixmap_browser radio resizebox scroll shape shiny subwindow \
	symbols tabs tile valuators fast_slow resize pack inactive style

all:	$(ALL)

include ../makeinclude

$(ALL):  ../lib/$(LIBNAME)

.SUFFIXES: .C .c .o .fl .H

.C:
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) $< -L../lib -lfltk $(LDLIBS) -o $@

.fl.C:
	@echo $@:
	@../fluid/fluid -c $<

.fl:
	@echo $@:
	@../fluid/fluid -c $<
	@$(CXX) -I.. $(CXXFLAGS) $@.C -L../lib -lfltk $(LDLIBS) -o $@

# Programs needing the OpenGL libraries:
cube: cube.C
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) cube.C -L../lib -lfltk $(GLDLIBS) -o $@
fullscreen: fullscreen.C
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) fullscreen.C -L../lib -lfltk $(GLDLIBS) -o $@
fractals: fractals.C
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) fractals.C -L../lib -lfltk $(GLDLIBS) -o $@
gl_overlay: gl_overlay.C
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) gl_overlay.C -L../lib -lfltk $(GLDLIBS) -o $@
glpuzzle: glpuzzle.C
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) glpuzzle.C -L../lib -lfltk $(GLDLIBS) -o $@
shape: shape.C
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) shape.C -L../lib -lfltk $(GLDLIBS) -o $@
shiny: shiny.C shiny_panel.C
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) shiny.C -L../lib -lfltk $(GLDLIBS) -o $@

# Other programs needing special "help"...
keyboard: keyboard.C keyboard_ui.C
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) keyboard.C -L../lib -lfltk $(LDLIBS) -o $@
mandelbrot: mandelbrot.C mandelbrot_ui.C
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) mandelbrot.C -L../lib -lfltk $(LDLIBS) -o $@
style: style.C style_ui.C
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) style.C -L../lib -lfltk $(LDLIBS) -o $@

# If you have libjpeg installed, you might want to try this test program:

jpeg_image: jpeg_image.C
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) -I../../../local/jpeg-6b -L../../../local/jpeg-6b jpeg_image.C -L../lib -lfltk $(LDLIBS) -ljpeg -lXext -o $@

depend:
	$(MAKEDEPEND) -I.. $(CXXFLAGS) $(CPPFILES) > makedepend

clean:
	-@ rm -f $(ALL) jpeg_image *.o core *~

#
# End of "$Id: Makefile,v 1.10 1998/10/21 20:10:23 mike Exp $".
#
