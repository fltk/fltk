#
# "$Id: Makefile 5667 2007-02-07 02:35:44Z mike $"
#
# PNG library makefile for the Fast Light Toolkit (FLTK).
#
# Copyright 1997-2007 by Easy Software Products.
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
# Please report all bugs and problems to "fltk-bugs@fltk.org".
#

include ../makeinclude

#
# Object files...
#

CAIROOBJECTS	=	Fl_Cairo.o

$(CAIROLIBNAME): $(CAIROOBJECTS)
	echo $(LIBCOMMAND) $@ ...
	$(RM) $@
	$(LIBCOMMAND) $@ $(CAIROOBJECTS)
	$(RANLIB) $@

libfltk_cairo.so.1.3: $(CAIROOBJECTS) libfltk.so.1.3
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(CAIROOBJECTS) -L. $(IMAGELIBS) -lfltk
	$(RM) libfltk_cairo.so
	$(LN) libfltk_cairo.so.1.3 libfltk_cairo.so

libfltk_cairo.sl.1.3: $(CAIROOBJECTS) libfltk.sl.1.3
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(CAIROOBJECTS) -L. $(IMAGELIBS) -lfltk
	$(RM) libfltk_cairo.sl
	$(LN) libfltk_cairo.sl.1.3 libfltk_cairo.sl

libfltk_cairo.1.3.dylib: $(CAIROOBJECTS) libfltk.1.3.dylib
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ \
		-install_name $(libdir)/$@ \
		-current_version 1.3.0 \
		-compatibility_version 1.3.0 \
		$(CAIROOBJECTS)  -L. $(LDLIBS) $(IMAGELIBS) -lfltk
	$(RM) libfltk_cairo.dylib
	$(LN) libfltk_cairo.1.3.dylib libfltk_cairo.dylib

libfltk_cairo_s.a: $(CAIROOBJECTS)
	echo $(DSOCOMMAND) libfltk_cairo_s.o ...
	$(DSOCOMMAND) libfltk_cairo_s.o $(CAIROOBJECTS)
	echo $(LIBCOMMAND) libfltk_cairo_s.a libfltk_cairo_s.o
	$(RM) $@
	$(LIBCOMMAND) libfltk_cairo_s.a libfltk_cairo_s.o
	$(CHMOD) +x libfltk_cairo_s.a


cygfltknox_cairo-1.3.dll: $(CAIROLIBNAME) cygfltknox-1.3.dll
	echo $(DSOCOMMAND) $(CAIROLIBNAME) ...
	$(DSOCOMMAND) $(CAIROLIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk_cairo.dll.a \
		-L. -lfltk $(CAIROLIBS) $(LDLIBS)

mgwfltknox_cairo-1.3.dll: $(CAIROLIBNAME) mgwfltknox-1.3.dll
	echo $(DSOCOMMAND) $(CAIROLIBNAME) ...
	$(DSOCOMMAND) $(CAIROLIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk_cairo.dll.a \
		-L. -lfltk $(CAIROLIBS) $(LDLIBS)

#
# Make all of the targets...
#

all:	$(CAIROLIBNAME) $(CAIRODSONAME)


#
# Clean all of the targets and object files...
#

clean:
	-$(RM) *.o *.dll.a core.* *~ *.bak *.bck
	-$(RM) $(CAIROOBJECTS)  $(CAIROLIBNAME) $(CAIRODSONAME) \
		libfltk_cairo.so  libfltk_cairo.sl  libfltk_cairo.dylib

#
# Install everything...
#

install: $(CAIROLIBNAME) $(CAIRODSONAME)
	echo "Installing libfltk_cairo$(LIBEXT) in $(libdir)..."
	-$(INSTALL_DIR) $(DESTDIR)$(libdir)
	$(INSTALL_LIB) $(CAIROLIBNAME) $(DESTDIR)$(libdir)

	if test x$(CAIRODSONAME) = xlibfltk_cairo.so.1.3; then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.so*;\
		$(INSTALL_LIB) libfltk_cairo.so.1.3 $(DESTDIR)$(libdir); \
		$(LN) libfltk_cairo.so.1.3 $(DESTDIR)$(libdir)/libfltk_cairo.so;\
	fi
	if test x$(CAIRODSONAME) = xlibfltk_cairo.sl.1.3; then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.sl*;\
		$(INSTALL_LIB) libfltk_cairo.sl.1.3 $(DESTDIR)$(libdir); \
		$(LN) libfltk_cairo.sl.1.3 $(DESTDIR)$(libdir)/libfltk_cairo.sl;\
	fi
	if test x$(CAIRODSONAME) = xlibfltk_cairo.1.3.dylib; then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.*dylib;\
		$(INSTALL_LIB) libfltk_cairo.1.3.dylib $(DESTDIR)$(libdir); \
		$(LN) libfltk_cairo.1.3.dylib $(DESTDIR)$(libdir)/libfltk_cairo.dylib;\
	fi
	if test x$(CAIRODSONAME) = xlibfltk_cairo_s.a; then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo_s.a;\
		$(INSTALL_LIB) libfltk_cairo_s.a $(DESTDIR)$(libdir); \
	fi
	if test x$(CAIRODSONAME) = xcygfltknox_cairo-1.3.dll; then\
		$(RM) $(DESTDIR)$(bindir)/$(CAIRODSONAME); \
		$(INSTALL_LIB) $(CAIRODSONAME) $(DESTDIR)$(bindir); \
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.dll.a;\
		$(INSTALL_LIB) libfltk_cairo.dll.a $(DESTDIR)$(libdir); \
	fi
	if test x$(CAIRODSONAME) = xmgwfltknox_cairo-1.3.dll; then\
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
	if test x$(CAIRODSONAME) = xlibfltk_cairo.so.1.3; then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.so*;\
	fi
	if test x$(CAIRODSONAME) = xlibfltk_cairo.sl.1.3; then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.sl*;\
	fi
	if test x$(CAIRODSONAME) = xlibfltk_cairo.1.3.dylib; then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.*dylib;\
	fi
	if test x$(CAIRODSONAME) = xlibfltk_cairo_s.a; then\
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo_s.a;\
	fi
	if test x$(CAIRODSONAME) = xcygfltknox_cairo-1.3.dll; then\
		$(RM) $(DESTDIR)$(bindir)/$(CAIRODSONAME); \
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.dll.a;\
	fi
	if test x$(CAIRODSONAME) = xmgwfltknox_cairo-1.3.dll; then\
		$(RM) $(DESTDIR)$(bindir)/$(CAIRODSONAME); \
		$(RM) $(DESTDIR)$(libdir)/libfltk_cairo.dll.a;\
	fi

$(CAIROOBJECTS):	../makeinclude

#
# End of "$Id: Makefile 5667 2007-02-07 02:35:44Z mike $".
#
