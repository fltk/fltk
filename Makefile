#
# Top-level Makefile for the Fast Light Tool Kit (FLTK).
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

include makeinclude

DIRS = $(IMAGEDIRS) $(LIBDECORDIR) src $(CAIRODIR) $(FLUIDDIR) fltk-options $(TESTDIR) \
       documentation

all: makeinclude fltk-config
	for dir in $(DIRS); do\
		echo "=== making $$dir ===";\
		(cd $$dir; $(MAKE) $(MFLAGS)) || exit 1;\
	done

# Build test programs (and 'all') if FLTK was configured with '--disable-test'
test: all
	echo "=== making test ===";\
	(cd test; $(MAKE) $(MFLAGS)) || exit 1

install: makeinclude
	-mkdir -p "$(DESTDIR)$(bindir)"
	$(RM) "$(DESTDIR)$(bindir)/fltk-config"
	$(INSTALL_SCRIPT) fltk-config "$(DESTDIR)$(bindir)"
	for dir in FL $(DIRS); do\
		echo "=== installing $$dir ===";\
		(cd $$dir; $(MAKE) $(MFLAGS) install) || exit 1;\
	done

install-desktop: makeinclude
	cd documentation; $(MAKE) $(MFLAGS) $(INSTALL_DESKTOP)
	cd fluid; $(MAKE) $(MFLAGS) $(INSTALL_DESKTOP)
	cd fltk-options; $(MAKE) $(MFLAGS) $(INSTALL_DESKTOP)
	cd test; $(MAKE) $(MFLAGS) $(INSTALL_DESKTOP)

uninstall: makeinclude
	$(RM) "$(DESTDIR)$(bindir)/fltk-config"
	for dir in FL $(DIRS); do\
		echo "=== uninstalling $$dir ===";\
		(cd $$dir; $(MAKE) $(MFLAGS) uninstall) || exit 1;\
	done

uninstall-desktop: makeinclude
	cd documentation; $(MAKE) $(MFLAGS) $(UNINSTALL_DESKTOP)
	cd fluid; $(MAKE) $(MFLAGS) $(UNINSTALL_DESKTOP)
	cd fltk-options; $(MAKE) $(MFLAGS) $(UNINSTALL_DESKTOP)
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
	$(RM) FL/fl_config.h
	$(RM) documentation/*.$(CAT1EXT)
	$(RM) documentation/*.$(CAT3EXT)
	$(RM) documentation/*.$(CAT6EXT)
	$(RM) documentation/fltk.ps
	$(RM) -r documentation/fltk.d
	for file in test/*.fl; do\
		$(RM) test/`basename $$file .fl`.cxx; \
		$(RM) test/`basename $$file .fl`.h; \
	done
	$(RM) -rf autom4te.cache/
	$(RM) configure

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

configure: configure.ac
	autoconf

portable-dist:
	epm -v -s fltk.xpm fltk

native-dist:
	epm -v -f native fltk

etags:
	etags FL/*.H FL/*.h src/*.cxx src/*.c src/*.h src/xutf8/*.h src/xutf8/*.c cairo/*.c fluid/*.h fluid/*.cxx test/*.h test/*.cxx

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
