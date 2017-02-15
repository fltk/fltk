#
# "$Id$"
#
# FLUID makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2017 by Bill Spitzak and others.
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

CPPFILES = \
	CodeEditor.cxx \
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

# ExternalCodeEditor: platform specific files

CPPFILES_WIN = ExternalCodeEditor_WIN32.cxx
CPPFILES_OSX = ExternalCodeEditor_UNIX.cxx
CPPFILES_X11 = ExternalCodeEditor_UNIX.cxx
CPPFILES_XFT = ExternalCodeEditor_UNIX.cxx

CPPFILES += $(CPPFILES_$(BUILD))

################################################################

OBJECTS = $(CPPFILES:.cxx=.o)

all:	$(FLUID) fluid$(EXEEXT)

fluid$(EXEEXT):		$(OBJECTS) $(LIBNAME) $(FLLIBNAME) \
			$(IMGLIBNAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LINKFLTKFORMS) $(LINKFLTKIMG) $(LDLIBS)
	$(OSX_ONLY) $(INSTALL_BIN) fluid fluid.app/Contents/MacOS

fluid-shared$(EXEEXT):	$(OBJECTS) ../src/$(DSONAME) ../src/$(FLDSONAME) \
			../src/$(IMGDSONAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LINKSHARED) $(LDLIBS)

clean:
	-$(RM) *.o core.* *~ *.bck *.bak
	-$(RM) core fluid$(EXEEXT) fluid-shared$(EXEEXT)
	-$(RM) fluid.app/Contents/MacOS/fluid$(EXEEXT)

depend:	$(CPPFILES)
	makedepend -Y -I.. -f makedepend $(CPPFILES)

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
	$(INSTALL_DATA) fluid.app/Contents/PkgInfo $(DESTDIR)/Applications/fluid.app/Contents/PkgInfo
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


#
# Note: The rebuild target can only be used if you have the original .fl
#       files.  This is normally only used by the FLTK maintainers...
#

rebuild:
	./fluid -u -c about_panel.fl
	./fluid -u -c alignment_panel.fl
	./fluid -u -c function_panel.fl
	./fluid -u -c print_panel.fl
	./fluid -u -c template_panel.fl
	./fluid -u -c widget_panel.fl

#
# End of "$Id$".
#
