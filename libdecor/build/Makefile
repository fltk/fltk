#
# Library Makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 2022 by Bill Spitzak and others.
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

CFLAGS_DECOR =  -I. -I../.. -I../../src -I../src -fPIC -D_GNU_SOURCE -DUSE_SYSTEM_LIBDECOR=0
OBJECTS =  fl_libdecor.o libdecor-cairo-blur.o fl_libdecor-plugins.o \
  ../../src/xdg-decoration-protocol.o ../../src/xdg-shell-protocol.o \
  ../../src/text-input-protocol.o cursor-settings.o os-compatibility.o
PROTOCOLS = /usr/share/wayland-protocols

all : demo $(GLWLDEMO)

depend:
	: echo "libdecor/build: make depend..."

fl_libdecor.o : fl_libdecor.c ../src/libdecor.c ../../src/xdg-shell-protocol.c ../../src/xdg-decoration-protocol.c ../../src/text-input-protocol.c
	$(CC) $(CFLAGS) $(CFLAGS_DECOR) -c  fl_libdecor.c -DLIBDECOR_PLUGIN_API_VERSION=1 -DLIBDECOR_PLUGIN_DIR=\"/usr/local/lib\"

fl_libdecor-plugins.o : fl_libdecor-plugins.c ../src/plugins/cairo/libdecor-cairo.c
	$(CC) $(CFLAGS) $(CFLAGS_DECOR) -c  fl_libdecor-plugins.c  -DLIBDECOR_PLUGIN_API_VERSION=1 -DLIBDECOR_PLUGIN_DIR=\"/usr/local/lib\"

libdecor-cairo-blur.o : ../src/plugins/cairo/libdecor-cairo-blur.c
	$(CC) $(CFLAGS_DECOR) -c  ../src/plugins/cairo/libdecor-cairo-blur.c

os-compatibility.o : ../src/os-compatibility.c
	$(CC) $(CFLAGS_DECOR) -c  ../src/os-compatibility.c

cursor-settings.o : ../src/cursor-settings.c
	$(CC) $(CFLAGS_DECOR)  -c ../src/cursor-settings.c -DHAS_DBUS `pkg-config --cflags dbus-1`

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
	
demo : ../demo/demo.c $(OBJECTS)
	$(CC)  -o demo ../demo/demo.c -D_GNU_SOURCE -I../.. -I../src -I. -I../../src $(OBJECTS) $(LDLIBS) -lm -rdynamic -no-pie -Wl,--defsym=fl_libdecor_using_weston=0

egl : ../demo/egl.c $(OBJECTS)
	$(CC)  -o egl ../demo/egl.c -D_GNU_SOURCE -I../.. -I../src -I. -I../../src $(OBJECTS) $(GLDLIBS) -lm -rdynamic -no-pie -Wl,--defsym=fl_libdecor_using_weston=0


install:
	echo "Nothing to install"

uninstall:

clean:
	$(RM) *.o ../../src/xdg-*.c ../../src/xdg-*.h ../../src/xdg-*.o ../../src/text-input-* demo egl
