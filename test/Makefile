#
# "$Id: Makefile,v 1.19.2.7.2.24 2001/12/30 05:16:10 easysw Exp $"
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
	threads.cxx \
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
	resize$(EXEEXT) \
	resizebox$(EXEEXT) \
	scroll$(EXEEXT) \
	subwindow$(EXEEXT) \
	symbols$(EXEEXT) \
	tabs$(EXEEXT) \
	$(THREADS) \
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
editor$(EXEEXT): editor.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) editor.o -o $@ $(LINKFLTK) $(LDLIBS) $(IMAGELIBS)
	$(POSTBUILD) $@ ../FL/mac.r
file_chooser$(EXEEXT): file_chooser.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) file_chooser.o -o $@ $(LINKFLTK) $(LDLIBS) $(IMAGELIBS)
	$(POSTBUILD) $@ ../FL/mac.r
help$(EXEEXT): help.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) help.o -o $@ $(LINKFLTK) $(LDLIBS) $(IMAGELIBS)
	$(POSTBUILD) $@ ../FL/mac.r
keyboard$(EXEEXT): keyboard_ui.o keyboard.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) keyboard.o keyboard_ui.o $(LINKFLTK) $(LDLIBS) -o $@
	$(POSTBUILD) $@ ../FL/mac.r

mandelbrot$(EXEEXT): mandelbrot_ui.o mandelbrot.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) mandelbrot.o mandelbrot_ui.o $(LINKFLTK) $(LDLIBS) -o $@
	$(POSTBUILD) $@ ../FL/mac.r

pixmap_browser$(EXEEXT): pixmap_browser.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) pixmap_browser.o -o $@ $(LINKFLTK) $(LDLIBS) $(IMAGELIBS)
	$(POSTBUILD) $@ ../FL/mac.r

# OpenGL demos...
CubeView$(EXEEXT): CubeMain.o CubeView.o CubeViewUI.o
	echo Linking $@...
	$(CXX) $(CXXFLAGS) CubeMain.o CubeView.o CubeViewUI.o \
		$(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@
	$(POSTBUILD) $@ ../FL/mac.r
CubeMain.o: CubeViewUI.h CubeView.h
CubeView.o: CubeView.h

cube$(EXEEXT): cube.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) cube.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@
	$(POSTBUILD) $@ ../FL/mac.r
fractals$(EXEEXT): fractals.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) fractals.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@
	$(POSTBUILD) $@ ../FL/mac.r
fullscreen$(EXEEXT): fullscreen.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) fullscreen.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@
	$(POSTBUILD) $@ ../FL/mac.r
glpuzzle$(EXEEXT): glpuzzle.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) glpuzzle.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@
	$(POSTBUILD) $@ ../FL/mac.r
gl_overlay$(EXEEXT): gl_overlay.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) gl_overlay.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@
	$(POSTBUILD) $@ ../FL/mac.r
shiny$(EXEEXT): shiny_panel.o shiny.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) shiny.o shiny_panel.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@
	$(POSTBUILD) $@ ../FL/mac.r

shape$(EXEEXT): shape.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) shape.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS) -o $@
	$(POSTBUILD) $@ ../FL/mac.r

# If you have libjpeg installed, you might want to try this test program:

jpeg_image$(EXEEXT): jpeg_image.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) jpeg_image.o $(LINKFLTK) $(LDLIBS) -ljpeg -lXext -o $@
	$(POSTBUILD) $@ ../FL/mac.r

depend:	$(CPPFILES)
	makedepend -Y -I.. -f makedepend $(CPPFILES)

clean:
	-@ rm -f $(ALL) $(GLALL) jpeg_image *.o core *~

install:
	@echo Nothing to install in test directory.

uninstall:
	@echo Nothing to uninstall in test directory.

#
# End of "$Id: Makefile,v 1.19.2.7.2.24 2001/12/30 05:16:10 easysw Exp $".
#
