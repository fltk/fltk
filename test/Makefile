#
# "$Id: Makefile,v 1.19.2.7.2.36 2002/06/03 12:32:57 easysw Exp $"
#
# Test/example program makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2002 by Bill Spitzak and others.
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
	preferences.cxx \
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
	preferences$(EXEEXT) \
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
	shiny$(EXEEXT)

all:	$(ALL) $(GLDEMOS)

gldemos:	$(GLALL)

depend:	$(CPPFILES)
	makedepend -Y -I.. -f makedepend $(CPPFILES)

# Automatically generated dependencies...
include makedepend

clean:
	-@ rm -f $(ALL) $(GLALL) *.o core *~

install:
	@echo Nothing to install in test directory.

uninstall:
	@echo Nothing to uninstall in test directory.


# FLUID file rules
.fl.cxx .fl.h:	../fluid/fluid$(EXEEXT)
	echo Generating $<...
	../fluid/fluid$(EXEEXT) -c $<

# All demos depend on the FLTK library...
$(ALL): ../lib/$(LIBNAME)

# General demos...
adjuster$(EXEEXT): adjuster.o

arc$(EXEEXT): arc.o

ask$(EXEEXT): ask.o

bitmap$(EXEEXT): bitmap.o

boxtype$(EXEEXT): boxtype.o

browser$(EXEEXT): browser.o

button$(EXEEXT): button.o

buttons$(EXEEXT): buttons.o

checkers$(EXEEXT): checkers.o

clock$(EXEEXT): clock.o

colbrowser$(EXEEXT): colbrowser.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) -o $@ $< $(LINKFLTKFORMS) $(LDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r

color_chooser$(EXEEXT): color_chooser.o

cursor$(EXEEXT): cursor.o

curve$(EXEEXT): curve.o

demo$(EXEEXT): demo.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) -o $@ $< $(LINKFLTKFORMS) $(LDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r

doublebuffer$(EXEEXT): doublebuffer.o

editor$(EXEEXT): editor.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) editor.o -o $@ $(LINKFLTK) $(IMAGELIBS) $(LDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r

fast_slow$(EXEEXT): fast_slow.o
fast_slow.cxx:	fast_slow.fl

file_chooser$(EXEEXT): file_chooser.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) file_chooser.o -o $@ $(LINKFLTK) $(IMAGELIBS) $(LDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r

fonts$(EXEEXT): fonts.o

forms$(EXEEXT): forms.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) -o $@ $< $(LINKFLTKFORMS) $(LDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r

hello$(EXEEXT): hello.o

help$(EXEEXT): help.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) help.o -o $@ $(LINKFLTK) $(IMAGELIBS) $(LDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r

iconize$(EXEEXT): iconize.o

image$(EXEEXT): image.o

inactive$(EXEEXT): inactive.o
inactive.cxx:	inactive.fl

input$(EXEEXT): input.o

keyboard$(EXEEXT): keyboard_ui.o keyboard.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) -o $@ keyboard.o keyboard_ui.o $(LINKFLTK) $(LDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r
keyboard_ui.o:	keyboard_ui.h
keyboard_ui.cxx:	keyboard_ui.fl

label$(EXEEXT): label.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) -o $@ $< $(LINKFLTKFORMS) $(LDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r

line_style$(EXEEXT): line_style.o

list_visuals$(EXEEXT): list_visuals.o

mandelbrot$(EXEEXT): mandelbrot_ui.o mandelbrot.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) -o $@ mandelbrot.o mandelbrot_ui.o $(LINKFLTK) $(LDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r
mandelbrot_ui.o:	mandelbrot_ui.h
mandelbrot_ui.cxx:	mandelbrot_ui.fl

menubar$(EXEEXT): menubar.o

message$(EXEEXT): message.o

minimum$(EXEEXT): minimum.o

navigation$(EXEEXT): navigation.o

output$(EXEEXT): output.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) -o $@ $< $(LINKFLTKFORMS) $(LDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r

overlay$(EXEEXT): overlay.o

pack$(EXEEXT): pack.o

pixmap$(EXEEXT): pixmap.o

pixmap_browser$(EXEEXT): pixmap_browser.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) pixmap_browser.o -o $@ $(LINKFLTK) $(IMAGELIBS) $(LDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r

preferences$(EXEEXT):	preferences.o
preferences.cxx:	preferences.fl

radio$(EXEEXT): radio.o
radio.cxx:	radio.fl

resize$(EXEEXT): resize.o
resize.cxx:	resize.fl

resizebox$(EXEEXT): resizebox.o

scroll$(EXEEXT): scroll.o

subwindow$(EXEEXT): subwindow.o

symbols$(EXEEXT): symbols.o

tabs$(EXEEXT): tabs.o
tabs.cxx:	tabs.fl

threads$(EXEEXT): threads.o

tile$(EXEEXT): tile.o

tiled_image$(EXEEXT): tiled_image.o

valuators$(EXEEXT): valuators.o
valuators.cxx:	valuators.fl

# All OpenGL demos depend on the FLTK and FLTK_GL libraries...
$(GLALL): ../lib/$(LIBNAME) ../lib/$(GLLIBNAME)

# OpenGL demos...
CubeView$(EXEEXT): CubeMain.o CubeView.o CubeViewUI.o
	echo Linking $@...
	$(CXX) $(CXXFLAGS) -o $@ CubeMain.o CubeView.o CubeViewUI.o \
		$(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r
CubeMain.o: CubeViewUI.h CubeView.h
CubeView.o: CubeView.h
CubeViewUI.o:	CubeViewUI.h
CubeViewUI.cxx:	CubeViewUI.fl

cube$(EXEEXT): cube.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) -o $@ cube.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r
fractals$(EXEEXT): fractals.o fracviewer.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) -o $@ fractals.o fracviewer.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r
fullscreen$(EXEEXT): fullscreen.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) -o $@ fullscreen.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r
glpuzzle$(EXEEXT): glpuzzle.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) -o $@ glpuzzle.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r
gl_overlay$(EXEEXT): gl_overlay.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) -o $@ gl_overlay.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r
shiny$(EXEEXT): shiny_panel.o shiny.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) -o $@ shiny.o shiny_panel.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r
shiny_panel.o:	shiny_panel.h
shiny_panel.cxx:	shiny_panel.fl

shape$(EXEEXT): shape.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) -o $@ shape.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(POSTBUILD) $@ ../FL/mac.r


#
# End of "$Id: Makefile,v 1.19.2.7.2.36 2002/06/03 12:32:57 easysw Exp $".
#
