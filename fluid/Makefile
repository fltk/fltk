#
# FLUID Makefile for the Fast Light Tool Kit (FLTK).
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

CPPFILES = \
	CodeEditor.cxx \
	StyleParse.cxx \
	Fd_Snap_Action.cxx \
	Fl_Button_Type.cxx \
	Fl_Function_Type.cxx \
	Fl_Grid_Type.cxx \
	Fl_Group_Type.cxx \
	Fl_Menu_Type.cxx \
	Fl_Type.cxx \
	Fl_Widget_Type.cxx \
	Fl_Window_Type.cxx \
	Fluid_Image.cxx \
	about_panel.cxx \
	align_widget.cxx \
	settings_panel.cxx \
	autodoc.cxx \
	code.cxx \
	custom_widgets.cxx \
	factory.cxx \
	file.cxx \
	fluid.cxx \
	fluid_filename.cxx \
	function_panel.cxx \
	mergeback.cxx \
	pixmaps.cxx \
	shell_command.cxx \
	codeview_panel.cxx \
	template_panel.cxx \
	undo.cxx \
	widget_browser.cxx \
	widget_panel.cxx

# ExternalCodeEditor: platform specific files

CPPFILES_WIN = ExternalCodeEditor_WIN32.cxx
CPPFILES_OSX = ExternalCodeEditor_UNIX.cxx
CPPFILES_X11 = ExternalCodeEditor_UNIX.cxx
CPPFILES_XFT = ExternalCodeEditor_UNIX.cxx
CPPFILES_CAIRO = ExternalCodeEditor_UNIX.cxx
CPPFILES_WAYLAND = ExternalCodeEditor_UNIX.cxx
CPPFILES_WAYLANDX11 = ExternalCodeEditor_UNIX.cxx

CPPFILES += $(CPPFILES_$(BUILD))

################################################################

OBJECTS = $(CPPFILES:.cxx=.o)

all:	$(FLUID) fluid$(EXEEXT)

fluid$(EXEEXT):		$(OBJECTS) $(LIBNAME) $(IMGLIBNAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) -o $@ $(OBJECTS) $(LINKFLTKIMG) $(LDFLAGS) $(LDLIBS)
	$(OSX_ONLY) $(RM) -r -f fluid.app
	$(OSX_ONLY) mkdir -p fluid.app/Contents/MacOS fluid.app/Contents/Resources
	$(OSX_ONLY) $(INSTALL_BIN) fluid fluid.app/Contents/MacOS
	$(OSX_ONLY) $(INSTALL) icons/fluid.icns fluid.app/Contents/Resources
	$(OSX_ONLY) $(INSTALL) fluid.plist fluid.app/Contents/Info.plist

fluid-shared$(EXEEXT):	$(OBJECTS) ../src/$(DSONAME) ../src/$(IMGDSONAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) -o $@ $(OBJECTS) $(LINKSHARED) $(LDFLAGS) $(LDLIBS)

clean:
	-$(RM) *.o core.* *~ *.bck *.bak
	-$(RM) core fluid$(EXEEXT) fluid-shared$(EXEEXT)
	$(OSX_ONLY) -$(RMDIR) fluid.app

depend:	$(CPPFILES)
	makedepend -Y -I.. -f makedepend -w 20 $(CPPFILES)
	echo "# DO NOT DELETE THIS LINE -- make depend depends on it." > makedepend.tmp
	echo "" >> makedepend.tmp
	grep '^[a-zA-Z]' makedepend | ( LC_ALL=C sort -u -f >> makedepend.tmp; )
	mv makedepend.tmp makedepend

# Automatically generated dependencies...
include makedepend

install: all
	echo "Installing FLUID in $(DESTDIR)$(bindir)..."
	-$(INSTALL_DIR) "$(DESTDIR)$(bindir)"
	$(INSTALL_BIN) $(FLUID) "$(DESTDIR)$(bindir)/fluid$(EXEEXT)"

install-linux:
	-$(INSTALL_DIR) "$(DESTDIR)$(datadir)/applications"
	$(INSTALL_DATA) fluid.desktop "$(DESTDIR)$(datadir)/applications"
	for size in 16 32 48 64 128; do \
		if test ! -d "$(DESTDIR)$(datadir)/icons/hicolor/$${size}x$${size}/apps"; then \
			$(INSTALL_DIR) "$(DESTDIR)$(datadir)/icons/hicolor/$${size}x$${size}/apps"; \
		fi; \
		$(INSTALL_DATA) icons/fluid-$$size.png "$(DESTDIR)$(datadir)/icons/hicolor/$${size}x$${size}/apps/fluid.png"; \
	done
	-$(INSTALL_DIR) "$(DESTDIR)$(datadir)/mimelnk/application"
	$(INSTALL_DATA) x-fluid.desktop "$(DESTDIR)$(datadir)/mimelnk/application"

install-osx:
	echo Installing Fluid in $(DESTDIR)/Applications...
	-$(INSTALL_DIR) "$(DESTDIR)/Applications/fluid.app"
	-$(INSTALL_DIR) "$(DESTDIR)/Applications/fluid.app/Contents"
	$(INSTALL_DATA) fluid.app/Contents/Info.plist "$(DESTDIR)/Applications/fluid.app/Contents/Info.plist"
	-$(INSTALL_DIR) "$(DESTDIR)/Applications/fluid.app/Contents/MacOS"
	$(RM) "$(DESTDIR)/Applications/fluid.app/Contents/MacOS/fluid"
	$(LN) "$(bindir)/fluid" "$(DESTDIR)/Applications/fluid.app/Contents/MacOS/fluid"
	-$(INSTALL_DIR) "$(DESTDIR)/Applications/fluid.app/Contents/Resources"
	$(INSTALL_DATA) fluid.app/Contents/Resources/fluid.icns "$(DESTDIR)/Applications/fluid.app/Contents/Resources"

uninstall:
	$(RM) "$(DESTDIR)$(bindir)/fluid$(EXEEXT)"

uninstall-linux:
	$(RM) "$(DESTDIR)$(datadir)/applications/fluid.desktop"
	$(RM) "$(DESTDIR)$(datadir)/icons/hicolor"/*/fluid.png
	$(RM) "$(DESTDIR)$(datadir)/mimelnk/application/x-fluid.desktop"

uninstall-osx:
	$(RM) -r "$(DESTDIR)/Applications/fluid.app"

# Note: The rebuild target can only be used if you have the original .fl
#	files. This is normally only used by FLTK maintainers...
#	It *must* be executed *after* fluid has been built and
#	fluid must be rebuilt if any {.fl|.cxx|.h} files were changed.

rebuild: fluid$(EXEEXT)
	echo 'Rebuilding fluid (.fl) and .cxx/.h files from .fl files ...'
	./fluid$(EXEEXT) -u -c about_panel.fl
	./fluid$(EXEEXT) -u -c settings_panel.fl
	./fluid$(EXEEXT) -u -c function_panel.fl
	./fluid$(EXEEXT) -u -c print_panel.fl
	./fluid$(EXEEXT) -u -c codeview_panel.fl
	./fluid$(EXEEXT) -u -c template_panel.fl
	./fluid$(EXEEXT) -u -c widget_panel.fl
