#
# "$Id$"
#
# Top-level makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2010 by Bill Spitzak and others.
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

include makeinclude

DIRS = $(IMAGEDIRS) src $(CAIRODIR) fluid test documentation

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
	for dir in examples $(DIRS); do\
		echo "=== cleaning $$dir ===";\
		(cd $$dir; $(MAKE) $(MFLAGS) clean) || exit 1;\
	done

distclean: clean
	$(RM) config.h config.log config.status
	$(RM) fltk-config fltk.list makeinclude
	$(RM) fltk.spec
	$(RM) FL/Makefile
	$(RM) FL/abi-version.h
	$(RM) documentation/*.$(CAT1EXT)
	$(RM) documentation/*.$(CAT3EXT)
	$(RM) documentation/*.$(CAT6EXT)
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

makeinclude: configure configh.in makeinclude.in config.guess config.sub
	if test -f config.status; then \
		./config.status --recheck; \
		./config.status; \
	else \
		./configure; \
	fi
	touch config.h
	chmod +x fltk-config

configure: configure.ac
	autoconf

config.guess config.sub:
	-automake --add-missing 2> /dev/null
	if [ ! -e config.sub   ]; then echo NOTE: Using frozen copy of config.sub;   cp misc/config.sub   . ; fi
	if [ ! -e config.guess ]; then echo NOTE: Using frozen copy of config.guess; cp misc/config.guess . ; fi

portable-dist:
	epm -v -s fltk.xpm fltk

native-dist:
	epm -v -f native fltk

etags:
	etags FL/*.H FL/*.h src/*.cxx src/*.c src/*.h src/xutf8/*.h src/xutf8/*.c cairo/*.cxx fluid/*.h fluid/*.cxx test/*.h test/*.cxx

#
# Run the clang.llvm.org static code analysis tool on the C sources.
# (at least checker-231 is required for scan-build to work this way)
#

.PHONY: clang clang-changes
clang:
	$(RM) -r clang
	scan-build -V -k -o `pwd`/clang $(MAKE) $(MFLAGS) clean all
clang-changes:
	scan-build -V -k -o `pwd`/clang $(MAKE) $(MFLAGS) all


#
# End of "$Id$".
#
