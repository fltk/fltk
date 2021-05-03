#
# FLUID Makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2021 by Bill Spitzak and others.
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
	ExternalCodeEditor_UNIX.cxx \
	ExternalCodeEditor_WIN32.cxx \
	Fl_Function_Type.cxx \
	Fl_Group_Type.cxx \
	Fl_Menu_Type.cxx \
	Fl_Type.cxx \
	Fl_Widget_Type.cxx \
	Fl_Window_Type.cxx \
	Fluid_Image.cxx \
	about_panel.cxx \
	align_widget.cxx \
	alignment_panel.cxx \
	code.cxx \
	factory.cxx \
	file.cxx \
	fluid.cxx \
	function_panel.cxx \
	template_panel.cxx \
	undo.cxx \
	widget_panel.cxx

################################################################

OBJECTS = $(CPPFILES:.cxx=.o)

all:	$(FLUID) fluid$(EXEEXT)

fluid$(EXEEXT):		$(OBJECTS) $(LIBNAME) $(FLLIBNAME) \
			$(IMGLIBNAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) -o $@ $(OBJECTS) $(LINKFLTKFORMS) $(LINKFLTKIMG) $(LDFLAGS) $(LDLIBS)
	$(OSX_ONLY) $(RM) -r -f fluid.app
	$(OSX_ONLY) mkdir -p fluid.app/Contents/MacOS fluid.app/Contents/Resources
	$(OSX_ONLY) $(INSTALL_BIN) fluid fluid.app/Contents/MacOS
	$(OSX_ONLY) $(INSTALL) icons/fluid.icns fluid.app/Contents/Resources
	$(OSX_ONLY) $(INSTALL) fluid.plist fluid.app/Contents/Info.plist

fluid-shared$(EXEEXT):	$(OBJECTS) ../src/$(DSONAME) ../src/$(FLDSONAME) \
			../src/$(IMGDSONAME)
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
	-$(INSTALL_DIR) $(DESTDIR)$(bindir)
	$(INSTALL_BIN) $(FLUID) $(DESTDIR)$(bindir)/fluid$(EXEEXT)

install-linux:
	-$(INSTALL_DIR) $(DESTDIR)/usr/share/applications
	$(INSTALL_DATA) fluid.desktop  $(DESTDIR)/usr/share/applications
	for size in 16 32 48 64 128; do \
		if test ! -d $(DESTDIR)/usr/share/icons/hicolor/$${size}x$${size}/apps; then \
			$(INSTALL_DIR) $(DESTDIR)/usr/share/icons/hicolor/$${size}x$${size}/apps; \
		fi; \
		$(INSTALL_DATA) icons/fluid-$$size.png $(DESTDIR)/usr/share/icons/hicolor/$${size}x$${size}/apps/fluid.png; \
	done
	-$(INSTALL_DIR) $(DESTDIR)/usr/share/mimelnk/application
	$(INSTALL_DATA) x-fluid.desktop  $(DESTDIR)/usr/share/mimelnk/application

install-osx:
	echo Installing Fluid in $(DESTDIR)/Applications...
	-$(INSTALL_DIR) $(DESTDIR)/Applications/fluid.app
	-$(INSTALL_DIR) $(DESTDIR)/Applications/fluid.app/Contents
	$(INSTALL_DATA) fluid.app/Contents/Info.plist $(DESTDIR)/Applications/fluid.app/Contents/Info.plist
	-$(INSTALL_DIR) $(DESTDIR)/Applications/fluid.app/Contents/MacOS
	$(RM) $(DESTDIR)/Applications/fluid.app/Contents/MacOS/fluid
	$(LN) $(bindir)/fluid $(DESTDIR)/Applications/fluid.app/Contents/MacOS/fluid
	-$(INSTALL_DIR) $(DESTDIR)/Applications/fluid.app/Contents/Resources
	$(INSTALL_DATA) fluid.app/Contents/Resources/fluid.icns $(DESTDIR)/Applications/fluid.app/Contents/Resources

uninstall:
	$(RM) $(DESTDIR)$(bindir)/fluid$(EXEEXT)

uninstall-linux:
	$(RM) $(DESTDIR)/usr/share/applications/fluid.desktop
	$(RM) $(DESTDIR)/usr/share/icons/hicolor/*/fluid.png
	$(RM) $(DESTDIR)/usr/share/mimelnk/application/x-fluid.desktop

uninstall-osx:
	$(RM) -r $(DESTDIR)/Applications/fluid.app

# Note: The rebuild target can only be used if you have the original .fl
#	files. This is normally only used by the FLTK maintainers...
#	It *must* be executed *after* fluid has been built and
#	fluid must be rebuilt if any {.fl|.cxx|.h} files were changed.

rebuild: fluid$(EXEEXT)
	echo 'Rebuilding fluid (.fl) and .cxx/.h files from .fl files ...'
	./fluid$(EXEEXT) -u -c about_panel.fl
	./fluid$(EXEEXT) -u -c alignment_panel.fl
	./fluid$(EXEEXT) -u -c function_panel.fl
	./fluid$(EXEEXT) -u -c print_panel.fl
	./fluid$(EXEEXT) -u -c template_panel.fl
	./fluid$(EXEEXT) -u -c widget_panel.fl
