#
# "$Id$"
#
# Test/example program makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2019 by Bill Spitzak and others.
#
# This library is free software. Distribution and use rights are outlined in
# the file "COPYING" which should have been included with this file.  If this
# file is missing or damaged, see the license at:
#
#      http://www.fltk.org/COPYING.php
#
# Please report all bugs and problems on the following page:
#
#      http://www.fltk.org/str.php
#

include ../makeinclude

CPPFILES =\
	adjuster.cxx \
	animated.cxx \
	arc.cxx \
	ask.cxx \
	bitmap.cxx \
	blocks.cxx \
	boxtype.cxx \
	browser.cxx \
	button.cxx \
	buttons.cxx \
	cairo_test.cxx \
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
	device.cxx \
	doublebuffer.cxx \
	editor.cxx \
	fast_slow.cxx \
	file_chooser.cxx \
	fonts.cxx \
	forms.cxx \
	fractals.cxx \
	fracviewer.cxx \
	fullscreen.cxx \
	gl_overlay.cxx \
	glpuzzle.cxx \
	hello.cxx \
	help.cxx \
	icon.cxx \
	iconize.cxx \
	image.cxx \
	inactive.cxx \
	input.cxx \
	input_choice.cxx \
	keyboard.cxx \
	label.cxx \
	line_style.cxx \
	list_visuals.cxx \
	mandelbrot.cxx \
	menubar.cxx \
	message.cxx \
	minimum.cxx \
	native-filechooser.cxx \
	navigation.cxx \
	offscreen.cxx \
	output.cxx \
	overlay.cxx \
	pack.cxx \
	pixmap_browser.cxx \
	pixmap.cxx \
	preferences.cxx \
	radio.cxx \
	resizebox.cxx \
	resize.cxx \
	rotated_text.cxx \
	scroll.cxx \
	shape.cxx \
	subwindow.cxx \
	sudoku.cxx \
	symbols.cxx \
	table.cxx \
	tabs.cxx \
	threads.cxx \
	tile.cxx \
	tiled_image.cxx \
	tree.cxx \
	twowin.cxx \
	unittests.cxx \
	utf8.cxx \
	valuators.cxx \
	windowfocus.cxx

ALL =	\
	unittests$(EXEEXT) \
	animated$(EXEEXT) \
	adjuster$(EXEEXT) \
	arc$(EXEEXT) \
	ask$(EXEEXT) \
	bitmap$(EXEEXT) \
	blocks$(EXEEXT) \
	boxtype$(EXEEXT) \
	browser$(EXEEXT) \
	button$(EXEEXT) \
	buttons$(EXEEXT) \
	cairo_test$(EXEEXT) \
	checkers$(EXEEXT) \
	clock$(EXEEXT) \
	colbrowser$(EXEEXT) \
	color_chooser$(EXEEXT) \
	cursor$(EXEEXT) \
	curve$(EXEEXT) \
	demo$(EXEEXT) \
	device$(EXEEXT) \
	doublebuffer$(EXEEXT) \
	editor$(EXEEXT) \
	fast_slow$(EXEEXT) \
	file_chooser$(EXEEXT) \
	fonts$(EXEEXT) \
	forms$(EXEEXT) \
	hello$(EXEEXT) \
	help$(EXEEXT) \
	icon$(EXEEXT) \
	iconize$(EXEEXT) \
	image$(EXEEXT) \
	inactive$(EXEEXT) \
	input$(EXEEXT) \
	input_choice$(EXEEXT) \
	keyboard$(EXEEXT) \
	label$(EXEEXT) \
	line_style$(EXEEXT) \
	list_visuals$(EXEEXT) \
	mandelbrot$(EXEEXT) \
	menubar$(EXEEXT) \
	message$(EXEEXT) \
	minimum$(EXEEXT) \
	native-filechooser$(EXEEXT) \
	navigation$(EXEEXT) \
	offscreen$(EXEEXT) \
	output$(EXEEXT) \
	overlay$(EXEEXT) \
	pack$(EXEEXT) \
	pixmap$(EXEEXT) \
	pixmap_browser$(EXEEXT) \
	preferences$(EXEEXT) \
	device$(EXEEXT) \
	radio$(EXEEXT) \
	resize$(EXEEXT) \
	resizebox$(EXEEXT) \
	rotated_text$(EXEEXT) \
	scroll$(EXEEXT) \
	subwindow$(EXEEXT) \
	sudoku$(EXEEXT) \
	symbols$(EXEEXT) \
	table$(EXEEXT) \
	tabs$(EXEEXT) \
	$(THREADS) \
	tile$(EXEEXT) \
	tiled_image$(EXEEXT) \
	tree$(EXEEXT) \
	twowin$(EXEEXT) \
	valuators$(EXEEXT) \
	cairotest$(EXEEXT) \
	utf8$(EXEEXT) \
	windowfocus$(EXEEXT)


GLALL = \
	cube$(EXEEXT) \
	CubeView$(EXEEXT) \
	fractals$(EXEEXT) \
	fullscreen$(EXEEXT) \
	gl_overlay$(EXEEXT) \
	glpuzzle$(EXEEXT) \
	shape$(EXEEXT)

all:	$(ALL) $(GLDEMOS)

gldemos:	$(GLALL)

depend:	$(CPPFILES)
	makedepend -Y -I.. -f makedepend $(CPPFILES)

# Automatically generated dependencies...
include makedepend

clean:
	$(RM) $(ALL) $(GLALL) core
	for file in $(ALL) $(GLALL); do \
		if [ $$file = "blocks" -o $$file = "checkers" -o $$file = "sudoku" ]; then \
			continue; \
		fi; \
		$(OSX_ONLY) rm -f -r $$file.app; \
	done
	$(RM) *.o core.* *~ *.bck *.bak
	$(RM) CubeViewUI.cxx CubeViewUI.h
	$(RM) fast_slow.cxx fast_slow.h
	$(RM) inactive.cxx inactive.h
	$(RM) keyboard_ui.cxx keyboard_ui.h
	$(RM) mandelbrot_ui.cxx mandelbrot_ui.h
	$(RM) preferences.cxx preferences.h
	$(RM) radio.cxx radio.h
	$(RM) resize.cxx resize.h
	$(RM) tabs.cxx tabs.h
	$(RM) tree.cxx tree.h
	$(RM) valuators.cxx valuators.h
	$(OSX_ONLY) $(RM) blocks.app/Contents/MacOS/blocks$(EXEEXT)
	$(OSX_ONLY) $(RM) checkers.app/Contents/MacOS/checkers$(EXEEXT)
	$(OSX_ONLY) $(RM) sudoku.app/Contents/MacOS/sudoku$(EXEEXT)

install:	all
	echo "Installing example programs to $(DESTDIR)$(docdir)/examples..."
	-$(INSTALL_DIR) $(DESTDIR)$(docdir)/examples
	for file in *.h *.cxx *.fl demo.menu; do \
		$(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/examples; \
	done
	-$(INSTALL_DIR) $(DESTDIR)$(docdir)/examples/pixmaps
	for file in pixmaps/*.xbm pixmaps/*.xpm; do \
		$(INSTALL_DATA) $$file $(DESTDIR)$(docdir)/examples/pixmaps; \
	done

install-linux:
	echo Installing games to $(DESTDIR)$(bindir)...
	-$(INSTALL_DIR) $(DESTDIR)$(bindir)
	-$(INSTALL_DIR) $(DESTDIR)/usr/share/applications
	-$(INSTALL_DIR) $(DESTDIR)/usr/share/icons/hicolor/32x32/apps
	-$(INSTALL_DIR) $(DESTDIR)/usr/share/icons/hicolor/128x128/apps
	for game in blocks checkers sudoku; do \
		$(INSTALL_BIN) $$game $(DESTDIR)$(bindir); \
		$(INSTALL_DATA) desktop/$$game.desktop  $(DESTDIR)/usr/share/applications; \
		$(INSTALL_DATA) desktop/$$game-32.png $(DESTDIR)/usr/share/icons/hicolor/32x32/apps/$$game.png; \
		$(INSTALL_DATA) desktop/$$game-128.png $(DESTDIR)/usr/share/icons/hicolor/128x128/apps/$$game.png; \
	done

install-osx:
	echo Installing games in $(DESTDIR)/Applications...
	for game in blocks checkers sudoku; do \
		if test ! -d $(DESTDIR)/Applications/$$game.app; then \
			$(INSTALL_DIR) $(DESTDIR)/Applications/$$game.app; \
			$(INSTALL_DIR) $(DESTDIR)/Applications/$$game.app/Contents; \
			$(INSTALL_DIR) $(DESTDIR)/Applications/$$game.app/Contents/MacOS; \
			$(INSTALL_DIR) $(DESTDIR)/Applications/$$game.app/Contents/Resources; \
		fi; \
		$(INSTALL_DATA) $$game.app/Contents/Info.plist $(DESTDIR)/Applications/$$game.app/Contents; \
		$(INSTALL_DATA) $$game.app/Contents/PkgInfo $(DESTDIR)/Applications/$$game.app/Contents; \
		$(INSTALL_BIN) $$game.app/Contents/MacOS/$$game $(DESTDIR)/Applications/$$game.app/Contents/MacOS; \
		$(INSTALL_DATA) $$game.app/Contents/Resources/$$game.icns $(DESTDIR)/Applications/$$game.app/Contents/Resources; \
	done

uninstall:
	echo "Removing examples programs from $(DESTDIR)$(docdir)/examples..."
	-$(RMDIR) $(DESTDIR)$(docdir)/examples

uninstall-linux:
	echo Removing games from $(DESTDIR)$(bindir)...
	for game in blocks checkers sudoku; do \
		$(RM) $(DESTDIR)$(bindir)/$$game; \
		$(RM) $(DESTDIR)/usr/share/applications/$$game.desktop; \
		$(RM) $(DESTDIR)/usr/share/icons/hicolor/32x32/apps/$$game.png; \
		$(RM) $(DESTDIR)/usr/share/icons/hicolor/128x128/apps/$$game.png; \
	done

uninstall-osx:
	echo Removing games from $(DESTDIR)/Applications...
	$(RMDIR) $(DESTDIR)/Applications/blocks.app
	$(RMDIR) $(DESTDIR)/Applications/checkers.app
	$(RMDIR) $(DESTDIR)/Applications/sudoku.app

# FLUID file rules
.fl.cxx .fl.h:	../fluid/fluid$(EXEEXT)
	echo Generating $@ and header from $<...
	../fluid/fluid$(EXEEXT) -c $<

# All demos depend on the FLTK library...
$(ALL): $(LIBNAME)

# General demos...
unittests$(EXEEXT): unittests.o

unittests.o: unittests.cxx unittest_about.cxx unittest_points.cxx unittest_lines.cxx unittest_circles.cxx \
	unittest_rects.cxx unittest_text.cxx unittest_symbol.cxx unittest_viewport.cxx unittest_images.cxx \
	unittest_schemes.cxx unittest_scrollbarsize.cxx

adjuster$(EXEEXT): adjuster.o

animated$(EXEEXT): animated.o

arc$(EXEEXT): arc.o

ask$(EXEEXT): ask.o

bitmap$(EXEEXT): bitmap.o

boxtype$(EXEEXT): boxtype.o

browser$(EXEEXT): browser.o

button$(EXEEXT): button.o

buttons$(EXEEXT): buttons.o

blocks$(EXEEXT): blocks.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) blocks.o -o $@ $(AUDIOLIBS) $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) $(RM) -r -f blocks.app/Contents/MacOS
	$(OSX_ONLY) mkdir blocks.app/Contents/MacOS
	$(OSX_ONLY) $(INSTALL_BIN) blocks$(EXEEXT) blocks.app/Contents/MacOS

checkers$(EXEEXT): checkers.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) checkers.o -o $@ $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) $(RM) -r -f checkers.app/Contents/MacOS
	$(OSX_ONLY) mkdir checkers.app/Contents/MacOS
	$(OSX_ONLY) $(INSTALL_BIN) checkers$(EXEEXT) checkers.app/Contents/MacOS

clock$(EXEEXT): clock.o

colbrowser$(EXEEXT): colbrowser.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ colbrowser.o $(LINKFLTKFORMS) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@
	$(OSX_ONLY) mkdir -p colbrowser.app/Contents/Resources
	$(OSX_ONLY) cp -f rgb.txt colbrowser.app/Contents/Resources/

color_chooser$(EXEEXT): color_chooser.o

cursor$(EXEEXT): cursor.o

curve$(EXEEXT): curve.o

demo$(EXEEXT): demo.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ demo.o $(LINKFLTKFORMS) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@
	$(OSX_ONLY) mkdir -p demo.app/Contents/Resources
	$(OSX_ONLY) cp -f demo.menu demo.app/Contents/Resources/

device$(EXEEXT): device.o $(IMGLIBNAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) device.o -o $@ $(LINKFLTKIMG) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

doublebuffer$(EXEEXT): doublebuffer.o

editor$(EXEEXT): editor.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) editor.o -o $@ $(LINKFLTKIMG) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@
	$(OSX_ONLY) cp -f ../ide/Xcode4/plists/editor-Info.plist editor.app/Contents/Info.plist

fast_slow$(EXEEXT): fast_slow.o
fast_slow.cxx:	fast_slow.fl ../fluid/fluid$(EXEEXT)

file_chooser$(EXEEXT): file_chooser.o $(IMGLIBNAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) file_chooser.o -o $@ $(LINKFLTKIMG) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

fonts$(EXEEXT): fonts.o

forms$(EXEEXT): forms.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ forms.o $(LINKFLTKFORMS) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

hello$(EXEEXT): hello.o

help$(EXEEXT): help.o $(IMGLIBNAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) help.o -o $@ $(LINKFLTKIMG) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@
	$(OSX_ONLY) mkdir -p help.app/Contents/Resources
	$(OSX_ONLY) cp -f help-test.html help.app/Contents/Resources/

icon$(EXEEXT): icon.o

iconize$(EXEEXT): iconize.o

image$(EXEEXT): image.o

inactive$(EXEEXT): inactive.o
inactive.cxx:	inactive.fl ../fluid/fluid$(EXEEXT)

input$(EXEEXT): input.o

input_choice$(EXEEXT): input_choice.o

keyboard$(EXEEXT): keyboard_ui.o keyboard.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ keyboard.o keyboard_ui.o $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@
keyboard_ui.o:	keyboard_ui.h
keyboard_ui.cxx:	keyboard_ui.fl ../fluid/fluid$(EXEEXT)

label$(EXEEXT): label.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ label.o $(LINKFLTKFORMS) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

line_style$(EXEEXT): line_style.o

list_visuals$(EXEEXT): list_visuals.o

mandelbrot$(EXEEXT): mandelbrot_ui.o mandelbrot.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ mandelbrot.o mandelbrot_ui.o $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@
mandelbrot_ui.o:	mandelbrot_ui.h
mandelbrot_ui.cxx:	mandelbrot_ui.fl ../fluid/fluid$(EXEEXT)

menubar$(EXEEXT): menubar.o

message$(EXEEXT): message.o

minimum$(EXEEXT): minimum.o

native-filechooser$(EXEEXT): native-filechooser.o $(IMGLIBNAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) native-filechooser.o -o $@ $(LINKFLTKIMG) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

navigation$(EXEEXT): navigation.o

offscreen$(EXEEXT): offscreen.o

output$(EXEEXT): output.o $(FLLIBNAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ output.o $(LINKFLTKFORMS) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

overlay$(EXEEXT): overlay.o

pack$(EXEEXT): pack.o

pixmap$(EXEEXT): pixmap.o

pixmap_browser$(EXEEXT): pixmap_browser.o $(IMGLIBNAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) pixmap_browser.o -o $@ $(LINKFLTKIMG) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

preferences$(EXEEXT):	preferences.o
preferences.cxx:	preferences.fl ../fluid/fluid$(EXEEXT)

device$(EXEEXT): device.o

radio$(EXEEXT): radio.o
radio.cxx:	radio.fl ../fluid/fluid$(EXEEXT)

resize$(EXEEXT): resize.o
resize.cxx:	resize.fl ../fluid/fluid$(EXEEXT)

resizebox$(EXEEXT): resizebox.o

rotated_text$(EXEEXT): rotated_text.o

scroll$(EXEEXT): scroll.o

subwindow$(EXEEXT): subwindow.o

sudoku: sudoku.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) sudoku.o -o $@ $(AUDIOLIBS) $(LINKFLTKIMG) $(LDLIBS)
	$(OSX_ONLY) $(RM) -r -f sudoku.app/Contents/MacOS
	$(OSX_ONLY) mkdir sudoku.app/Contents/MacOS
	$(OSX_ONLY) $(INSTALL_BIN) sudoku$(EXEEXT) sudoku.app/Contents/MacOS

sudoku.exe: sudoku.o sudoku.rc
	echo Linking $@...
	$(RC) sudoku.rc sudokures.o
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) sudoku.o sudokures.o -o $@ $(AUDIOLIBS) $(LINKFLTKIMG) $(LDLIBS)

symbols$(EXEEXT): symbols.o

table$(EXEEXT): table.o

tabs$(EXEEXT): tabs.o
tabs.cxx:	tabs.fl ../fluid/fluid$(EXEEXT)

threads$(EXEEXT): threads.o
# This ensures that we have this dependency even if threads are not
# enabled in the current tree...
threads.o:	threads.h

tile$(EXEEXT): tile.o

tiled_image$(EXEEXT): tiled_image.o

tree$(EXEEXT): tree.o
tree.cxx:	tree.fl ../fluid/fluid$(EXEEXT)

twowin$(EXEEXT): twowin.o

valuators$(EXEEXT): valuators.o
valuators.cxx:	valuators.fl ../fluid/fluid$(EXEEXT)

# All OpenGL demos depend on the FLTK and FLTK_GL libraries...
$(GLALL): $(LIBNAME) $(GLLIBNAME)

# OpenGL demos...
CubeView$(EXEEXT): CubeMain.o CubeView.o CubeViewUI.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ \
		CubeMain.o CubeView.o CubeViewUI.o \
		$(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@
CubeMain.o: CubeViewUI.h CubeView.h CubeViewUI.cxx
CubeView.o: CubeView.h
CubeViewUI.o:	CubeViewUI.cxx CubeViewUI.h
CubeViewUI.cxx:	CubeViewUI.fl ../fluid/fluid$(EXEEXT)

cube$(EXEEXT): cube.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ cube.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

fractals$(EXEEXT): fractals.o fracviewer.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ fractals.o fracviewer.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

fullscreen$(EXEEXT): fullscreen.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ fullscreen.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

glpuzzle$(EXEEXT): glpuzzle.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ glpuzzle.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

gl_overlay$(EXEEXT): gl_overlay.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ gl_overlay.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

shape$(EXEEXT): shape.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ shape.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

cairo_test$(EXEEXT): cairo_test.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(CAIROFLAGS) $(LDFLAGS) -o $@ cairo_test.o $(LINKFLTK) $(LINKFLTKCAIRO) $(GLDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

#
# End of "$Id$".
#
