#
# Test/demo program Makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2024 by Bill Spitzak and others.
#
# This library is free software. Distribution and use rights are outlined in
# the file "COPYING" which should have been included with this file.  If this
# file is missing or damaged, see the license at:
#
#      https://www.fltk.org/COPYING.php
#
# Please see the following page on how to report bugs and issues:
#
#      https://www.fltk.org/bugs.php
#

include ../makeinclude

CPPUNITTEST = \
	unittests.cxx \
	unittest_about.cxx \
	unittest_points.cxx \
	unittest_complex_shapes.cxx \
	unittest_fast_shapes.cxx \
	unittest_circles.cxx \
	unittest_text.cxx \
	unittest_unicode.cxx \
	unittest_symbol.cxx \
	unittest_images.cxx \
	unittest_viewport.cxx \
	unittest_scrollbarsize.cxx \
	unittest_schemes.cxx \
	unittest_terminal.cxx \
	unittest_core.cxx

OBJUNITTEST = \
	$(CPPUNITTEST:.cxx=.o)

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
	clipboard.cxx \
	clock.cxx \
	colbrowser.cxx \
	color_chooser.cxx \
	contrast.cxx \
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
	flex_demo.cxx \
	flex_login.cxx \
	fltk-versions.cxx \
	fonts.cxx \
	forms.cxx \
	fractals.cxx \
	fracviewer.cxx \
	fullscreen.cxx \
	gl_overlay.cxx \
	glpuzzle.cxx \
	glut_test.cxx \
	grid_alignment \
	grid_buttons \
	grid_dialog \
	grid_login \
	hello.cxx \
	help_dialog.cxx \
	icon.cxx \
	iconize.cxx \
	image.cxx \
	inactive.cxx \
	input.cxx \
	input_choice.cxx \
	keyboard.cxx \
	label.cxx \
	line_style.cxx \
	line_style_docs.cxx \
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
	resize.cxx \
	resizebox.cxx \
	resize-example1.cxx \
	resize-example2.cxx \
	resize-example3a.cxx \
	resize-example3b.cxx \
	resize-example3c.cxx \
	resize-example4a.cxx \
	resize-example4b.cxx \
	resize-example5a.cxx \
	resize-example5b.cxx \
	resize-example5c.cxx \
	rotated_text.cxx \
	scroll.cxx \
	shape.cxx \
	subwindow.cxx \
	sudoku.cxx \
	symbols.cxx \
	table.cxx \
	tabs.cxx \
	terminal.cxx \
	threads.cxx \
	tile.cxx \
	tiled_image.cxx \
	tree.cxx \
	twowin.cxx \
	utf8.cxx \
	valuators.cxx \
	windowfocus.cxx \
	wizard.cxx \
	$(CPPUNITTEST)

ALL =	\
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
	clipboard$(EXEEXT) \
	clock$(EXEEXT) \
	colbrowser$(EXEEXT) \
	color_chooser$(EXEEXT) \
	contrast$(EXEEXT) \
	cursor$(EXEEXT) \
	curve$(EXEEXT) \
	demo$(EXEEXT) \
	device$(EXEEXT) \
	doublebuffer$(EXEEXT) \
	editor$(EXEEXT) \
	file_chooser$(EXEEXT) \
	flex_demo$(EXEEXT) \
	flex_login$(EXEEXT) \
	fltk-versions$(EXEEXT) \
	fonts$(EXEEXT) \
	forms$(EXEEXT) \
	grid_alignment$(EXEEXT) \
	grid_buttons$(EXEEXT) \
	grid_dialog$(EXEEXT) \
	grid_login$(EXEEXT) \
	hello$(EXEEXT) \
	help_dialog$(EXEEXT) \
	icon$(EXEEXT) \
	iconize$(EXEEXT) \
	image$(EXEEXT) \
	input$(EXEEXT) \
	input_choice$(EXEEXT) \
	label$(EXEEXT) \
	line_style$(EXEEXT) \
	line_style_docs$(EXEEXT) \
	list_visuals$(EXEEXT) \
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
	device$(EXEEXT) \
	resizebox$(EXEEXT) \
	resize-example1$(EXEEXT) \
	resize-example2$(EXEEXT) \
	resize-example3a$(EXEEXT) \
	resize-example3b$(EXEEXT) \
	resize-example3c$(EXEEXT) \
	resize-example4a$(EXEEXT) \
	resize-example4b$(EXEEXT) \
	resize-example5a$(EXEEXT) \
	resize-example5b$(EXEEXT) \
	resize-example5c$(EXEEXT) \
	rotated_text$(EXEEXT) \
	scroll$(EXEEXT) \
	subwindow$(EXEEXT) \
	sudoku$(EXEEXT) \
	symbols$(EXEEXT) \
	table$(EXEEXT) \
	$(THREADS) \
	tile$(EXEEXT) \
	tiled_image$(EXEEXT) \
	twowin$(EXEEXT) \
	utf8$(EXEEXT) \
	windowfocus$(EXEEXT) \
	wizard$(EXEEXT)

ALLFLUID = \
	checkers$(EXEEXT) \
	fast_slow$(EXEEXT) \
	inactive$(EXEEXT) \
	keyboard$(EXEEXT) \
	mandelbrot$(EXEEXT) \
	preferences$(EXEEXT) \
	radio$(EXEEXT) \
	resize$(EXEEXT) \
	tabs$(EXEEXT) \
	terminal$(EXEEXT) \
	tree$(EXEEXT) \
	valuators$(EXEEXT) \
	CubeView$(EXEEXT)

GLALL = \
	cube$(EXEEXT) \
	fractals$(EXEEXT) \
	fullscreen$(EXEEXT) \
	gl_overlay$(EXEEXT) \
	glpuzzle$(EXEEXT) \
	glut_test$(EXEEXT) \
	shape$(EXEEXT) \
	unittests$(EXEEXT)

all:	$(ALL) $(GLDEMOS)
	for dir in $(FLUIDDIR); do\
	  $(MAKE)  $(MFLAGS) $(ALLFLUID);\
	done

gldemos:	$(GLALL)

depend:	$(CPPFILES)
	makedepend -Y -I.. -f makedepend -w 20 $(CPPFILES)
	echo "# DO NOT DELETE THIS LINE -- make depend depends on it." > makedepend.tmp
	echo "" >> makedepend.tmp
	grep '^[a-zA-Z]' makedepend | ( LC_ALL=C sort -u -f >> makedepend.tmp; )
	mv makedepend.tmp makedepend

# Automatically generated dependencies...
include makedepend

clean:
	$(RM) $(ALL) $(ALLFLUID) $(GLALL) core
	$(RMDIR) *.app
	$(RM) *.o core.* *~ *.bck *.bak
	$(RM) CubeViewUI.cxx CubeViewUI.h
	$(RM) checkers_pieces.cxx checkers_pieces.h
	$(RM) fast_slow.cxx fast_slow.h
	$(RM) inactive.cxx inactive.h
	$(RM) keyboard_ui.cxx keyboard_ui.h
	$(RM) mandelbrot_ui.cxx mandelbrot_ui.h
	$(RM) preferences.cxx preferences.h
	$(RM) radio.cxx radio.h
	$(RM) resize.cxx resize.h
	$(RM) tabs.cxx tabs.h
	$(RM) terminal.cxx terminal.h
	$(RM) tree.cxx tree.h
	$(RM) valuators.cxx valuators.h

install:	all
	echo "Installing example programs to $(DESTDIR)$(docdir)/examples..."
	-$(INSTALL_DIR) "$(DESTDIR)$(docdir)/examples"
	for file in *.h *.cxx *.fl demo.menu; do \
		$(INSTALL_DATA) $$file "$(DESTDIR)$(docdir)/examples"; \
	done
	-$(INSTALL_DIR) "$(DESTDIR)$(docdir)/examples/pixmaps"
	for file in pixmaps/*.xbm pixmaps/*.xpm; do \
		$(INSTALL_DATA) $$file "$(DESTDIR)$(docdir)/examples/pixmaps"; \
	done

install-linux:
	echo Installing games to $(DESTDIR)$(bindir)...
	-$(INSTALL_DIR) "$(DESTDIR)$(bindir)"
	-$(INSTALL_DIR) "$(DESTDIR)$(datadir)/applications"
	-$(INSTALL_DIR) "$(DESTDIR)$(datadir)/icons/hicolor/32x32/apps"
	-$(INSTALL_DIR) "$(DESTDIR)$(datadir)/icons/hicolor/128x128/apps"
	for game in blocks checkers sudoku; do \
		$(INSTALL_BIN) $$game "$(DESTDIR)$(bindir)"; \
		$(INSTALL_DATA) desktop/$$game.desktop "$(DESTDIR)$(datadir)/applications"; \
		$(INSTALL_DATA) desktop/$$game-32.png "$(DESTDIR)$(datadir)/icons/hicolor/32x32/apps/$$game.png"; \
		$(INSTALL_DATA) desktop/$$game-128.png "$(DESTDIR)$(datadir)/icons/hicolor/128x128/apps/$$game.png"; \
	done

install-osx:
	echo Installing games in $(DESTDIR)/Applications...
	for game in blocks checkers sudoku; do \
		if test ! -d "$(DESTDIR)/Applications/$$game.app"; then \
			$(INSTALL_DIR) "$(DESTDIR)/Applications/$$game.app"; \
			$(INSTALL_DIR) "$(DESTDIR)/Applications/$$game.app/Contents"; \
			$(INSTALL_DIR) "$(DESTDIR)/Applications/$$game.app/Contents/MacOS"; \
			$(INSTALL_DIR) "$(DESTDIR)/Applications/$$game.app/Contents/Resources"; \
		fi; \
		$(INSTALL_DATA) $$game.app/Contents/Info.plist "$(DESTDIR)/Applications/$$game.app/Contents"; \
		$(INSTALL_BIN) $$game.app/Contents/MacOS/$$game "$(DESTDIR)/Applications/$$game.app/Contents/MacOS"; \
		$(INSTALL_DATA) $$game.app/Contents/Resources/$$game.icns "$(DESTDIR)/Applications/$$game.app/Contents/Resources"; \
	done

uninstall:
	echo "Removing examples programs from $(DESTDIR)$(docdir)/examples..."
	-$(RMDIR) "$(DESTDIR)$(docdir)/examples"

uninstall-linux:
	echo Removing games from $(DESTDIR)$(bindir)...
	for game in blocks checkers sudoku; do \
		$(RM) "$(DESTDIR)$(bindir)/$$game"; \
		$(RM) "$(DESTDIR)$(datadir)/applications/$$game.desktop"; \
		$(RM) "$(DESTDIR)$(datadir)/icons/hicolor/32x32/apps/$$game.png"; \
		$(RM) "$(DESTDIR)$(datadir)/icons/hicolor/128x128/apps/$$game.png"; \
	done

uninstall-osx:
	echo Removing games from $(DESTDIR)/Applications...
	$(RMDIR) "$(DESTDIR)/Applications/blocks.app"
	$(RMDIR) "$(DESTDIR)/Applications/checkers.app"
	$(RMDIR) "$(DESTDIR)/Applications/sudoku.app"

# FLUID file rules
.fl.cxx .fl.h:
	echo Generating $@ and header from $<...
	$(FLUID_BUILD) -c $<

# All demos depend on the FLTK library...
$(ALL): $(LIBNAME)

# General demos...
unittests$(EXEEXT): $(OBJUNITTEST)

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
	$(OSX_ONLY) $(RM) -f -r blocks.app
	$(OSX_ONLY) mkdir -p blocks.app/Contents/MacOS blocks.app/Contents/Resources
	$(OSX_ONLY) $(INSTALL_BIN) blocks$(EXEEXT) blocks.app/Contents/MacOS
	$(OSX_ONLY) $(INSTALL_BIN) mac-resources/blocks.icns blocks.app/Contents/Resources/
	$(OSX_ONLY) $(INSTALL_BIN) mac-resources/blocks.plist blocks.app/Contents/Info.plist

checkers$(EXEEXT): checkers.o checkers_pieces.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) checkers.o checkers_pieces.o -o $@ $(LINKFLTKIMG) $(LDLIBS)
	$(OSX_ONLY) $(RM) -f -r checkers.app
	$(OSX_ONLY) mkdir -p checkers.app/Contents/MacOS checkers.app/Contents/Resources
	$(OSX_ONLY) $(INSTALL_BIN) checkers$(EXEEXT) checkers.app/Contents/MacOS
	$(OSX_ONLY) $(INSTALL_BIN) mac-resources/checkers.icns checkers.app/Contents/Resources/
	$(OSX_ONLY) $(INSTALL_BIN) mac-resources/checkers.plist checkers.app/Contents/Info.plist

checkers.cxx:		checkers_pieces.h
checkers_pieces.o:	checkers_pieces.h
checkers_pieces.h:	checkers_pieces.fl
checkers_pieces.cxx:	checkers_pieces.fl ../fluid/fluid$(EXEEXT)

clipboard$(EXEEXT): clipboard.o $(IMGLIBNAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) clipboard.o -o $@ $(LINKFLTKIMG) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

clock$(EXEEXT): clock.o

colbrowser$(EXEEXT): colbrowser.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ colbrowser.o $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@
	$(OSX_ONLY) mkdir -p colbrowser.app/Contents/Resources
	$(OSX_ONLY) cp -f rgb.txt colbrowser.app/Contents/Resources/

color_chooser$(EXEEXT): color_chooser.o

contrast$(EXEEXT): contrast.o

cursor$(EXEEXT): cursor.o

curve$(EXEEXT): curve.o

demo$(EXEEXT): demo.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ demo.o $(LINKFLTK) $(LDLIBS)
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
	$(OSX_ONLY) cp -f mac-resources/editor.plist editor.app/Contents/Info.plist

fast_slow$(EXEEXT): fast_slow.o
fast_slow.cxx:	fast_slow.fl ../fluid/fluid$(EXEEXT)

file_chooser$(EXEEXT): file_chooser.o $(IMGLIBNAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) file_chooser.o -o $@ $(LINKFLTKIMG) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

flex_demo$(EXEEXT): flex_demo.o

flex_login$(EXEEXT): flex_login.o

fltk-versions$(EXEEXT): fltk-versions.o

fonts$(EXEEXT): fonts.o

forms$(EXEEXT): forms.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ forms.o $(LINKFLTKFORMS) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

grid_alignment$(EXEEXT): grid_alignment.o

grid_buttons$(EXEEXT): grid_buttons.o

grid_dialog$(EXEEXT): grid_dialog.o

grid_login$(EXEEXT): grid_login.o

hello$(EXEEXT): hello.o

help_dialog$(EXEEXT): help_dialog.o $(IMGLIBNAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) help_dialog.o -o $@ $(LINKFLTKIMG) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@
	$(OSX_ONLY) mkdir -p help_dialog.app/Contents/Resources
	$(OSX_ONLY) cp -f help_dialog.html help_dialog.app/Contents/Resources/

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
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ label.o $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

line_style$(EXEEXT): line_style.o

line_style_docs$(EXEEXT): line_style_docs.o

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
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ output.o $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

overlay$(EXEEXT): overlay.o

pack$(EXEEXT): pack.o

pixmap$(EXEEXT): pixmap.o $(IMGLIBNAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ pixmap.o $(LINKFLTKIMG) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

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

resize-example1$(EXEEXT): resize-example1.o resize-arrows.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) resize-example1.o resize-arrows.o -o $@ $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

resize-example2$(EXEEXT): resize-example2.o resize-arrows.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) resize-example2.o resize-arrows.o -o $@ $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

resize-example3a$(EXEEXT): resize-example3a.o resize-arrows.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) resize-example3a.o resize-arrows.o -o $@ $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

resize-example3b$(EXEEXT): resize-example3b.o resize-arrows.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) resize-example3b.o resize-arrows.o -o $@ $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

resize-example3c$(EXEEXT): resize-example3c.o resize-arrows.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) resize-example3c.o resize-arrows.o -o $@ $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

resize-example4a$(EXEEXT): resize-example4a.o resize-arrows.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) resize-example4a.o resize-arrows.o -o $@ $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

resize-example4b$(EXEEXT): resize-example4b.o resize-arrows.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) resize-example4b.o resize-arrows.o -o $@ $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

resize-example5a$(EXEEXT): resize-example5a.o resize-arrows.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) resize-example5a.o resize-arrows.o -o $@ $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

resize-example5b$(EXEEXT): resize-example5b.o resize-arrows.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) resize-example5b.o resize-arrows.o -o $@ $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

resize-example5c$(EXEEXT): resize-example5c.o resize-arrows.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) resize-example5c.o resize-arrows.o -o $@ $(LINKFLTK) $(LDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

rotated_text$(EXEEXT): rotated_text.o

scroll$(EXEEXT): scroll.o

subwindow$(EXEEXT): subwindow.o

sudoku: sudoku.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) sudoku.o -o $@ $(AUDIOLIBS) $(LINKFLTKIMG) $(LDLIBS)
	$(OSX_ONLY) $(RM) -f -r sudoku.app
	$(OSX_ONLY) mkdir -p sudoku.app/Contents/MacOS sudoku.app/Contents/Resources
	$(OSX_ONLY) $(INSTALL_BIN) sudoku$(EXEEXT) sudoku.app/Contents/MacOS
	$(OSX_ONLY) $(INSTALL_BIN) mac-resources/sudoku.icns sudoku.app/Contents/Resources/
	$(OSX_ONLY) $(INSTALL_BIN) mac-resources/sudoku.plist sudoku.app/Contents/Info.plist

sudoku.exe: sudoku.o sudoku.rc
	echo Linking $@...
	$(RC) sudoku.rc sudokures.o
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) sudoku.o sudokures.o -o $@ $(AUDIOLIBS) $(LINKFLTKIMG) $(LDLIBS)

symbols$(EXEEXT): symbols.o

table$(EXEEXT): table.o

tabs$(EXEEXT): tabs.o
tabs.cxx:	tabs.fl ../fluid/fluid$(EXEEXT)

terminal$(EXEEXT): terminal.o
terminal.cxx:	terminal.fl ../fluid/fluid$(EXEEXT)

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

glut_test$(EXEEXT): glut_test.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ glut_test.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

unittests$(EXEEXT): $(OBJUNITTEST)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJUNITTEST) $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

shape$(EXEEXT): shape.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ shape.o $(LINKFLTKGL) $(LINKFLTK) $(GLDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@

cairo_test$(EXEEXT): cairo_test.o
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(CAIROFLAGS) $(LDFLAGS) -o $@ cairo_test.o $(LINKFLTK) $(CAIROLIBS) $(GLDLIBS)
	$(OSX_ONLY) ../fltk-config --post $@
