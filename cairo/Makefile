#
# CAIRO library makefile for the Fast Light Toolkit (FLTK).
#
# Copyright 1997-2009 by Easy Software Products.
# Copyright 2010-2021 by Bill Spitzak and others.
#
# This library is free software. Distribution and use rights are outlined in
# the file "COPYING" which should have been included with this file.  If this
# file is missing or damaged, see the license at:
#
#     https://www.fltk.org/COPYING.php
#
# Please see the following page on how to report bugs and issues:
#
#     https://www.fltk.org/bugs.php
#

# Note:	see ../configure.in and/or ../makeinclude for definition of
#	FL_VERSION (x.y.z), FL_ABI_VERSION (x.y.0), and FL_DSO_VERSION (x.y)

FLTKFLAGS = -DFL_LIBRARY
include ../makeinclude

#
# Object files...
#
CAIROCPPFILES   = Fl_Cairo.cxx
CAIROOBJECTS	= $(CAIROCPPFILES:.cxx=.o)

#
# Make all of the targets...
#

all:	$(CAIROLIBNAME) $(CAIRODSONAME)

$(CAIROLIBNAME): $(CAIROOBJECTS)
	echo $(LIBCOMMAND) $@ ...
	$(RM) $@
	$(LIBCOMMAND) $@ $(CAIROOBJECTS)
	$(RANLIB) $@

libfltk_cairo.so.$(FL_DSO_VERSION): $(CAIROOBJECTS) ../src/libfltk.so.$(FL_DSO_VERSION)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(CAIROOBJECTS) -L../src -lfltk $(CAIROLIBS)
	$(RM) libfltk_cairo.so
	$(LN) $(CAIRODSONAME) libfltk_cairo.so

libfltk_cairo.sl.$(FL_DSO_VERSION): $(CAIROOBJECTS) ../src/libfltk.sl.$(FL_DSO_VERSION)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(CAIROOBJECTS) -L../src -lfltk $(CAIROLIBS)
	$(RM) libfltk_cairo.sl
	$(LN) libfltk_cairo.sl.$(FL_DSO_VERSION) libfltk_cairo.sl

libfltk_cairo.$(FL_DSO_VERSION).dylib: $(CAIROOBJECTS) ../src/libfltk.$(FL_DSO_VERSION).dylib
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ \
		-install_name $(libdir)/$@ \
		-current_version $(FL_VERSION) \
		-compatibility_version $(FL_ABI_VERSION) \
		$(CAIROOBJECTS)  -L../src $(LDLIBS) $(CAIROLIBS) -lfltk
	$(RM) libfltk_cairo.dylib
	$(LN) libfltk_cairo.$(FL_DSO_VERSION).dylib libfltk_cairo.dylib

libfltk_cairo_s.a: $(CAIROOBJECTS)
	echo $(DSOCOMMAND) libfltk_cairo_s.o ...
	$(DSOCOMMAND) libfltk_cairo_s.o $(CAIROOBJECTS)
	echo $(LIBCOMMAND) libfltk_cairo_s.a src/libfltk_cairo_s.o
	$(RM) $@
	$(LIBCOMMAND) src/libfltk_cairo_s.a src/libfltk_cairo_s.o
	$(CHMOD) +x src/libfltk_cairo_s.a


cygfltknox_cairo-$(FL_DSO_VERSION).dll: $(CAIROLIBNAME) ../src/cygfltknox-$(FL_DSO_VERSION).dll
	echo $(DSOCOMMAND) $(CAIROLIBNAME) ...
	$(DSOCOMMAND) $(CAIROLIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk_cairo.dll.a \
		-L../src -lfltk $(CAIROLIBS) $(LDLIBS)

mgwfltknox_cairo-$(FL_DSO_VERSION).dll: $(CAIROLIBNAME) ../src/mgwfltknox-$(FL_DSO_VERSION).dll
	echo $(DSOCOMMAND) $(CAIROLIBNAME) ...
	$(DSOCOMMAND) $(CAIROLIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk_cairo.dll.a \
		-L../src -lfltk $(CAIROLIBS) $(LDLIBS)

#
# Clean all of the targets and object files...
#

clean:
	-$(RM) *.o *.dll.a core.* *~ *.bak *.bck
	-$(RM) $(CAIROOBJECTS)  $(CAIROLIBNAME) $(CAIRODSONAME) \
		libfltk_cairo.so src/libfltk_cairo.sl  src/libfltk_cairo.dylib

#
# Install everything...
#

install: $(CAIROLIBNAME) $(CAIRODSONAME)
	echo "Installing libfltk_cairo$(LIBEXT) in $(libdir)..."
	-$(INSTALL_DIR) $(DESTDIR)$(libdir)
	$(INSTALL_LIB) $(CAIROLIBNAME) $(DESTDIR)$(libdir)

	if test x$(CAIRODSONAME) = xlibfltk_cairo.so.$(FL_DSO_VERSION); then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.so*;\
		$(INSTALL_LIB) libfltk_cairo.so.$(FL_DSO_VERSION) $(DESTDIR)$(libdir); \
		$(LN) libfltk_cairo.so.$(FL_DSO_VERSION) $(DESTDIR)$(libdir)/libfltk_cairo.so;\
	fi
	if test x$(CAIRODSONAME) = xsrc/libfltk_cairo.sl.$(FL_DSO_VERSION); then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.sl*;\
		$(INSTALL_LIB) libfltk_cairo.sl.$(FL_DSO_VERSION) $(DESTDIR)$(libdir); \
		$(LN) libfltk_cairo.sl.$(FL_DSO_VERSION) $(DESTDIR)$(libdir)/libfltk_cairo.sl;\
	fi
	if test x$(CAIRODSONAME) = xlibfltk_cairo.$(FL_DSO_VERSION).dylib; then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.*dylib;\
		$(INSTALL_LIB) libfltk_cairo.$(FL_DSO_VERSION).dylib $(DESTDIR)$(libdir); \
		$(LN) libfltk_cairo.$(FL_DSO_VERSION).dylib $(DESTDIR)$(libdir)/libfltk_cairo.dylib;\
	fi
	if test x$(CAIRODSONAME) = xlibfltk_cairo_s.a; then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo_s.a;\
		$(INSTALL_LIB) libfltk_cairo_s.a $(DESTDIR)$(libdir); \
	fi
	if test x$(CAIRODSONAME) = xcygfltknox_cairo-$(FL_DSO_VERSION).dll; then\
		$(RM) $(DESTDIR)$(bindir)/$(CAIRODSONAME); \
		$(INSTALL_LIB) $(CAIRODSONAME) $(DESTDIR)$(bindir); \
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.dll.a;\
		$(INSTALL_LIB) libfltk_cairo.dll.a $(DESTDIR)$(libdir); \
	fi
	if test x$(CAIRODSONAME) = xmgwfltknox_cairo-$(FL_DSO_VERSION).dll; then\
		$(RM) $(DESTDIR)$(bindir)/$(CAIRODSONAME); \
		$(INSTALL_LIB) $(CAIRODSONAME) $(DESTDIR)$(bindir); \
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.dll.a;\
		$(INSTALL_LIB) libfltk_cairo.dll.a $(DESTDIR)$(libdir); \
	fi
#
# Uninstall everything...
#

uninstall:
	echo "Uninstalling libfltk_cairo$(LIBEXT) in $(libdir)..."
	if test x$(CAIROLIBNAME) != x; then\
		$(RM) $(DESTDIR)$(libdir)/$(CAIROLIBNAME);\
	fi
	if test x$(CAIRODSONAME) = xlibfltk_cairo.so.$(FL_DSO_VERSION); then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.so*;\
	fi
	if test x$(CAIRODSONAME) = xlibfltk_cairo.sl.$(FL_DSO_VERSION); then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.sl*;\
	fi
	if test x$(CAIRODSONAME) = xlibfltk_cairo.$(FL_DSO_VERSION).dylib; then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.*dylib;\
	fi
	if test x$(CAIRODSONAME) = xlibfltk_cairo_s.a; then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo_s.a;\
	fi
	if test x$(CAIRODSONAME) = xcygfltknox_cairo-$(FL_DSO_VERSION).dll; then\
		$(RM) $(DESTDIR)$(bindir)/$(CAIRODSONAME); \
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.dll.a;\
	fi
	if test x$(CAIRODSONAME) = xmgwfltknox_cairo-$(FL_DSO_VERSION).dll; then\
		$(RM) $(DESTDIR)$(bindir)/$(CAIRODSONAME); \
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.dll.a;\
	fi

$(CAIROOBJECTS):	../makeinclude

depend: $(CAIROCPPFILES)
	makedepend -Y -I.. -f makedepend -w 20 $(CAIROCPPFILES)
	echo "# DO NOT DELETE THIS LINE -- make depend depends on it." > makedepend.tmp
	echo "" >> makedepend.tmp
	grep '^[a-zA-Z]' makedepend | ( LC_ALL=C sort -u -f >> makedepend.tmp; )
	mv makedepend.tmp makedepend

include makedepend
