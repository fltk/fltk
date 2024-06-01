#
# Library Makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 2022-2024 by Bill Spitzak and others.
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

include ../../makeinclude

OBJECTS =  fl_libdecor.o libdecor-cairo-blur.o fl_libdecor-plugins.o \
  ../../src/xdg-decoration-protocol.o ../../src/xdg-shell-protocol.o \
  ../../src/text-input-protocol.o ../../src/gtk-shell-protocol.o desktop-settings.o os-compatibility.o

PROTOCOLS = `pkg-config --variable=pkgdatadir wayland-protocols`

Linux_CFLAGS =
FreeBSD_CFLAGS = -I/usr/local/include
EXTRA_DECOR =  ${${UNAME}_CFLAGS}

CFLAGS_DECOR =  -I. -I../.. -I../../src -I../src -I../src/plugins $(EXTRA_DECOR) -fPIC -D_GNU_SOURCE \
   -DHAVE_MEMFD_CREATE -DHAVE_MKOSTEMP -DHAVE_POSIX_FALLOCATE

all : $(OBJECTS)

depend:
	: echo "libdecor/build: make depend..."

fl_libdecor.o : fl_libdecor.c ../src/libdecor.c ../../src/xdg-shell-protocol.c ../../src/xdg-decoration-protocol.c ../../src/text-input-protocol.c ../../src/gtk-shell-protocol.c
	$(CC) $(CFLAGS) $(CFLAGS_DECOR) -c  fl_libdecor.c -DLIBDECOR_PLUGIN_API_VERSION=1

fl_libdecor-plugins.o : fl_libdecor-plugins.c ../src/plugins/cairo/libdecor-cairo.c
	$(CC) $(CFLAGS) $(CFLAGS_DECOR) -c  fl_libdecor-plugins.c  -DLIBDECOR_PLUGIN_API_VERSION=1

libdecor-cairo-blur.o : ../src/plugins/common/libdecor-cairo-blur.c
	$(CC) $(CFLAGS_DECOR) -c  ../src/plugins/common/libdecor-cairo-blur.c

os-compatibility.o : ../src/os-compatibility.c
	$(CC) $(CFLAGS_DECOR) -c  ../src/os-compatibility.c

desktop-settings.o : ../src/desktop-settings.c
	$(CC) $(CFLAGS_DECOR)  -c ../src/desktop-settings.c $(LIBDECORDBUS)

../../src/xdg-shell-protocol.c :
	wayland-scanner private-code $(PROTOCOLS)/stable/xdg-shell/xdg-shell.xml \
	    ../../src/xdg-shell-protocol.c
	wayland-scanner client-header $(PROTOCOLS)/stable/xdg-shell/xdg-shell.xml \
	    ../../src/xdg-shell-client-protocol.h

../../src/xdg-decoration-protocol.c :
	wayland-scanner private-code \
	    $(PROTOCOLS)/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml \
	    ../../src/xdg-decoration-protocol.c
	wayland-scanner client-header \
	    $(PROTOCOLS)/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml \
	    ../../src/xdg-decoration-client-protocol.h

../../src/text-input-protocol.c :
	wayland-scanner private-code \
	    $(PROTOCOLS)/unstable/text-input/text-input-unstable-v3.xml \
	    ../../src/text-input-protocol.c
	wayland-scanner client-header \
	    $(PROTOCOLS)/unstable/text-input/text-input-unstable-v3.xml \
	    ../../src/text-input-client-protocol.h

../../src/gtk-shell-protocol.c :
	wayland-scanner private-code \
	    gtk-shell.xml ../../src/gtk-shell-protocol.c
	wayland-scanner client-header \
	    gtk-shell.xml ../../src/gtk-shell-client-protocol.h

install:
	echo "Nothing to install"

uninstall:

clean:
	$(RM) *.o ../../src/xdg-*.c ../../src/xdg-*.h ../../src/xdg-*.o ../../src/text-input-* ../../src/gtk-shell-*
