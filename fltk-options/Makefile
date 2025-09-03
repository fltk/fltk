#
# fltk-options Makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 2023-2024 by Bill Spitzak and others.
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
	fltk-options.cxx

################################################################

OBJECTS = $(CPPFILES:.cxx=.o)

all:	$(FLTK_OPTIONS) fltk-options$(EXEEXT)

fltk-options$(EXEEXT):	$(OBJECTS) $(LIBNAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) -o $@ $(OBJECTS) $(LINKFLTKIMG) $(LDFLAGS) $(LDLIBS)
	$(OSX_ONLY) $(RM) -r -f fltk-options.app
	$(OSX_ONLY) mkdir -p fltk-options.app/Contents/MacOS fltk-options.app/Contents/Resources
	$(OSX_ONLY) $(INSTALL_BIN) fltk-options fltk-options.app/Contents/MacOS
	$(OSX_ONLY) $(INSTALL) icons/fltk-options.icns fltk-options.app/Contents/Resources
	$(OSX_ONLY) $(INSTALL) fltk-options.plist fltk-options.app/Contents/Info.plist

fltk-options-shared$(EXEEXT):	$(OBJECTS) ../src/$(DSONAME)
	echo Linking $@...
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) -o $@ $(OBJECTS) $(LINKSHARED) $(LDFLAGS) $(LDLIBS)

clean:
	-$(RM) *.o core.* *~ *.bck *.bak
	-$(RM) core fltk-options$(EXEEXT) fltk-options-shared$(EXEEXT)
	$(OSX_ONLY) -$(RMDIR) fltk-options.app

depend:	$(CPPFILES)
	makedepend -Y -I.. -f makedepend -w 20 $(CPPFILES)
	echo "# DO NOT DELETE THIS LINE -- make depend depends on it." > makedepend.tmp
	echo "" >> makedepend.tmp
	grep '^[a-zA-Z]' makedepend | ( LC_ALL=C sort -u -f >> makedepend.tmp; )
	mv makedepend.tmp makedepend

# Automatically generated dependencies...
include makedepend

install: all
	echo "Installing fltk-options in $(DESTDIR)$(bindir)..."
	-$(INSTALL_DIR) "$(DESTDIR)$(bindir)"
	$(INSTALL_BIN) $(FLTK_OPTIONS) "$(DESTDIR)$(bindir)/fltk-options$(EXEEXT)"

install-linux:
	-$(INSTALL_DIR) "$(DESTDIR)/usr/share/applications"
	$(INSTALL_DATA) fltk-options.desktop "$(DESTDIR)/usr/share/applications"
	for size in 16 32 48 64 128; do \
		if test ! -d "$(DESTDIR)/usr/share/icons/hicolor/$${size}x$${size}/apps"; then \
			$(INSTALL_DIR) "$(DESTDIR)/usr/share/icons/hicolor/$${size}x$${size}/apps"; \
		fi; \
		$(INSTALL_DATA) icons/fltk-options-$$size.png "$(DESTDIR)/usr/share/icons/hicolor/$${size}x$${size}/apps/fltk-options.png"; \
	done
	-$(INSTALL_DIR) "$(DESTDIR)/usr/share/mimelnk/application"
	$(INSTALL_DATA) x-fltk-options.desktop "$(DESTDIR)/usr/share/mimelnk/application"

install-osx:
	echo Installing fltk-options in $(DESTDIR)/Applications...
	-$(INSTALL_DIR) "$(DESTDIR)/Applications/fltk-options.app"
	-$(INSTALL_DIR) "$(DESTDIR)/Applications/fltk-options.app/Contents"
	$(INSTALL_DATA) fltk-options.app/Contents/Info.plist "$(DESTDIR)/Applications/fltk-options.app/Contents/Info.plist"
	-$(INSTALL_DIR) "$(DESTDIR)/Applications/fltk-options.app/Contents/MacOS"
	$(RM) "$(DESTDIR)/Applications/fltk-options.app/Contents/MacOS/fltk-options"
	$(LN) $(bindir)/fltk-options "$(DESTDIR)/Applications/fltk-options.app/Contents/MacOS/fltk-options"
	-$(INSTALL_DIR) "$(DESTDIR)/Applications/fltk-options.app/Contents/Resources"
	$(INSTALL_DATA) fltk-options.app/Contents/Resources/fltk-options.icns "$(DESTDIR)/Applications/fltk-options.app/Contents/Resources"

uninstall:
	$(RM) "$(DESTDIR)$(bindir)/fltk-options$(EXEEXT)"

uninstall-linux:
	$(RM) "$(DESTDIR)/usr/share/applications/fltk-options.desktop"
	$(RM) "$(DESTDIR)/usr/share/icons/hicolor/*/fltk-options.png"
	$(RM) "$(DESTDIR)/usr/share/mimelnk/application/x-fltk-options.desktop"

uninstall-osx:
	$(RM) -r "$(DESTDIR)/Applications/fltk-options.app"
