#
# "$Id$"
#
# Top-level makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2007 by Bill Spitzak and others.
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
# Please report all bugs and problems on the following page:
#
#      http://www.fltk.org/str.php
#

include makeinclude

DIRS	=	$(IMAGEDIRS) src fluid test documentation

all: makeinclude fltk-config
	for dir in $(DIRS); do\
		echo "=== making $$dir ===";\
		(cd $$dir; $(MAKE) $(MFLAGS)) || exit 1;\
	done

install: makeinclude
	-mkdir -p $(DESTDIR)$(bindir)
	$(RM) $(DESTDIR)$(bindir)/fltk-config
	$(INSTALL_SCRIPT) fltk-config $(DESTDIR)$(bindir)
	for dir in FL $(DIRS); do\
		echo "=== installing $$dir ===";\
		(cd $$dir; $(MAKE) $(MFLAGS) install) || exit 1;\
	done

install-desktop: makeinclude
	cd documentation; $(MAKE) $(MFLAGS) $(INSTALL_DESKTOP)
	cd fluid; $(MAKE) $(MFLAGS) $(INSTALL_DESKTOP)
	cd test; $(MAKE) $(MFLAGS) $(INSTALL_DESKTOP)

uninstall: makeinclude
	$(RM) $(DESTDIR)$(bindir)/fltk-config
	for dir in FL $(DIRS); do\
		echo "=== uninstalling $$dir ===";\
		(cd $$dir; $(MAKE) $(MFLAGS) uninstall) || exit 1;\
	done

uninstall-desktop: makeinclude
	cd documentation; $(MAKE) $(MFLAGS) $(UNINSTALL_DESKTOP)
	cd fluid; $(MAKE) $(MFLAGS) $(UNINSTALL_DESKTOP)
	cd test; $(MAKE) $(MFLAGS) $(UNINSTALL_DESKTOP)

depend: makeinclude
	for dir in $(DIRS); do\
		echo "=== making dependencies in $$dir ===";\
		(cd $$dir; $(MAKE) $(MFLAGS) depend) || exit 1;\
	done

clean:
	-$(RM) core *.o
	for dir in $(DIRS); do\
		echo "=== cleaning $$dir ===";\
		(cd $$dir; $(MAKE) $(MFLAGS) clean) || exit 1;\
	done

distclean: clean
	$(RM) config.*
	$(RM) fltk-config fltk.list makeinclude
	$(RM) fltk.spec
	$(RM) FL/Makefile
	$(RM) documentation/*.$(CAT1EXT)
	$(RM) documentation/*.$(CAT3EXT)
	$(RM) documentation/*.$(CAT6EXT)
	$(RM) documentation/fltk.pdf
	$(RM) documentation/fltk.ps
	$(RM) -r documentation/fltk.d
	for file in test/*.fl; do\
		$(RM) test/`basename $$file .fl`.cxx; \
		$(RM) test/`basename $$file .fl`.h; \
	done

fltk-config: configure configh.in fltk-config.in
	if test -f config.status; then \
		./config.status --recheck; \
		./config.status; \
	else \
		./configure; \
	fi
	touch config.h
	chmod +x fltk-config

makeinclude: configure configh.in makeinclude.in
	if test -f config.status; then \
		./config.status --recheck; \
		./config.status; \
	else \
		./configure; \
	fi
	touch config.h
	chmod +x fltk-config

configure: configure.in
	autoconf

portable-dist:
	epm -v -s fltk.xpm fltk

native-dist:
	epm -v -f native fltk

etags:
	etags FL/*.H FL/*.h src/*.cxx src/*.c src/*.h fluid/*.h fluid/*.cxx test/*.h test/*.cxx

#
# End of "$Id$".
#
