#
# "$Id: Makefile,v 1.19.2.7.2.15 2001/11/29 22:22:08 easysw Exp $"
#
# Test/example program makefile for the Fast Light Tool Kit (FLTK).
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

include ../makeinclude

CPPFILES =\
	adjuster.cxx \
	arc.cxx \
	ask.cxx \
	bitmap.cxx \
	boxtype.cxx \
	browser.cxx \
	button.cxx \
	buttons.cxx \
	checkers.cxx \
	clock.cxx \
	colbrowser.cxx \
	color_chooser.cxx \
	cube.cxx \
	CubeMain.cxx \
	CubeView.cxx \
	cursor.cxx \
	curve.cxx \
	demo.cxx \
	doublebuffer.cxx \
	editor.cxx \
	fast_slow.cxx \
	file_chooser.cxx \
	fonts.cxx \
	forms.cxx \
	fractals.cxx \
	fullscreen.cxx \
	gl_overlay.cxx \
	glpuzzle.cxx \
	hello.cxx \
	help.cxx \
	iconize.cxx \
	image.cxx \
	inactive.cxx \
	input.cxx \
	keyboard.cxx \
	label.cxx \
	line_style.cxx \
	list_visuals.cxx \
	mandelbrot.cxx \
	menubar.cxx \
	message.cxx \
	minimum.cxx \
	navigation.cxx \
	output.cxx \
	overlay.cxx \
	pack.cxx \
	pixmap_browser.cxx \
	pixmap.cxx \
	radio.cxx \
	resizebox.cxx \
	resize.cxx \
	scroll.cxx \
	shape.cxx \
	shiny.cxx \
	subwindow.cxx \
	symbols.cxx \
	tabs.cxx \
	tile.cxx \
	tiled_image.cxx \
	valuators.cxx

ALL =	\
	adjuster$(EXEEXT) \
	arc$(EXEEXT) \
	ask$(EXEEXT) \
	bitmap$(EXEEXT) \
	boxtype$(EXEEXT) \
	browser$(EXEEXT) \
	button$(EXEEXT) \
	buttons$(EXEEXT) \
	checkers$(EXEEXT) \
	clock$(EXEEXT) \
	colbrowser$(EXEEXT) \
	color_chooser$(EXEEXT) \
	cursor$(EXEEXT) \
	curve$(EXEEXT) \
	demo$(EXEEXT) \
	doublebuffer$(EXEEXT) \
	editor$(EXEEXT) \
	fast_slow$(EXEEXT) \
	file_chooser$(EXEEXT) \
	fonts$(EXEEXT) \
	forms$(EXEEXT) \
	hello$(EXEEXT) \
	help$(EXEEXT) \
	iconize$(EXEEXT) \
	image$(EXEEXT) \
	inactive$(EXEEXT) \
	input$(EXEEXT) \
	keyboard$(EXEEXT) \
	label$(EXEEXT) \
	line_style$(EXEEXT) \
	list_visuals$(EXEEXT) \
	mandelbrot$(EXEEXT) \
	menubar$(EXEEXT) \
	message$(EXEEXT) \
	minimum$(EXEEXT) \
	navigation$(EXEEXT) \
	output$(EXEEXT) \
	overlay$(EXEEXT) \
	pack$(EXEEXT) \
	pixmap$(EXEEXT) \
	pixmap_browser$(EXEEXT) \
	radio$(EXEEXT) \
	resizebox$(EXEEXT) \
	scroll$(EXEEXT) \
	subwindow$(EXEEXT) \
	symbols$(EXEEXT) \
	tabs$(EXEEXT) \
	tile$(EXEEXT) \
	tiled_image$(EXEEXT) \
	valuators$(EXEEXT)

GLALL = \
	cube$(EXEEXT) \
	CubeView$(EXEEXT) \
	fractals$(EXEEXT) \
	fullscreen$(EXEEXT) \
	gl_overlay$(EXEEXT) \
	glpuzzle$(EXEEXT) \
	shape$(EXEEXT) \
	shiny$(EXEEXT) \

all:	$(ALL) $(GLDEMOS)

gldemos:	$(GLALL)

# FLUID file rules
.fl.cxx .fl.h:
	echo Generating $<...
	../fluid/fluid -c $<

$(ALL):  ../lib/$(LIBNAME)

# Programs needing special instructions...
editor$(EXEEXT): editor.cxx
	echo Compiling and linking $@...
	$(CXX) -I.. $(CXXFLAGS) editor.cxx -o $@ $(LINKFLTK) $(LDLIBS) $(IMAGELIBS)
file_chooser$(EXEEXT): file_chooser.cxx
	echo Compiling and linking $@...
	$(CXX) -I.. $(CXXFLAGS) file_chooser.cxx -o $@ $(LINKFLTK) $(LDLIBS) $(IMAGELIBS)
help$(EXEEXT): help.cxx
	echo Compiling and linking $@...
	$(CXX) -I.. $(CXXFLAGS) help.cxx -o $@ $(LINKFLTK) $(LDLIBS) $(IMAGELIBS)
keyboard$(EXEEXT): keyboard.cxx keyboard_ui.cxx
	echo Compiling and linking $@...
	$(CXX) -I.. $(CXXFLAGS) keyboard.cxx $(LINKFLTK) $(LDLIBS) -o $@
mandelbrot$(EXEEXT): mandelbrot.cxx mandelbrot_ui.cxx
	echo Compiling and linking $@...
	$(CXX) -I.. $(CXXFLAGS) mandelbrot.cxx $(LINKFLTK) $(LDLIBS) -o $@
pixmap_browser$(EXEEXT): pixmap_browser.cxx
	echo Compiling and linking $@...
	$(CXX) -I.. $(CXXFLAGS) pixmap_browser.cxx -o $@ $(LINKFLTK) $(LDLIBS) $(IMAGELIBS)

# OpenGL demos...
CubeView$(EXEEXT): CubeMain.o CubeView.o CubeViewUI.o
	echo Compiling and linking $@...
	$(CXX) $(CXXFLAGS) CubeMain.o CubeView.o CubeViewUI.o \
		$(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@
CubeMain.o: CubeViewUI.h CubeView.h
CubeView.o: CubeView.h

cube$(EXEEXT): cube.cxx
	echo Compiling and linking $@...
	$(CXX) -I.. $(CXXFLAGS) cube.cxx $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@
fractals$(EXEEXT): fractals.cxx
	echo Compiling and linking $@...
	$(CXX) -I.. $(CXXFLAGS) fractals.cxx $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@
fullscreen$(EXEEXT): fullscreen.cxx
	echo Compiling and linking $@...
	$(CXX) -I.. $(CXXFLAGS) fullscreen.cxx $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@
glpuzzle$(EXEEXT): glpuzzle.cxx
	echo Compiling and linking $@...
	$(CXX) -I.. $(CXXFLAGS) glpuzzle.cxx $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@
gl_overlay$(EXEEXT): gl_overlay.cxx
	echo Compiling and linking $@...
	$(CXX) -I.. $(CXXFLAGS) gl_overlay.cxx $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@
shiny$(EXEEXT): shiny.cxx shiny_panel.cxx
	echo Compiling and linking $@...
	$(CXX) -I.. $(CXXFLAGS) shiny.cxx $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@
shape$(EXEEXT): shape.cxx
	echo Compiling and linking $@...
	$(CXX) -I.. $(CXXFLAGS) shape.cxx $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@

# If you have libjpeg installed, you might want to try this test program:

jpeg_image$(EXEEXT): jpeg_image.cxx
	echo Compiling and linking $@...
	$(CXX) -I.. $(CXXFLAGS) jpeg_image.cxx $(LINKFLTK) $(LDLIBS) -ljpeg -lXext -o $@

depend:
#	$(MAKEDEPEND) -I.. $(CXXFLAGS) $(CPPFILES) > makedepend
	makedepend -Y -I.. -f makedepend $(CPPFILES)

clean:
	-@ rm -f $(ALL) $(GLALL) jpeg_image *.o core *~

install:
	@echo Nothing to install in test directory.

uninstall:
	@echo Nothing to uninstall in test directory.

#
# End of "$Id: Makefile,v 1.19.2.7.2.15 2001/11/29 22:22:08 easysw Exp $".
#
