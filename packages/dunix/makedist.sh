#!/bin/sh
#
# makedist - make a digital unix distribution.
#

rm -rf fltk-1.0.5-dunix
mkdir fltk-1.0.5-dunix

echo "Building distribution tree..."
rm -rf usr
mkdir usr
mkdir usr/bin
mkdir usr/bin/X11
mkdir usr/include
mkdir usr/include/FL
mkdir usr/info
mkdir usr/info/fltk
mkdir usr/lib
mkdir usr/man
mkdir usr/man/man1

cp ../../fluid/fluid usr/bin/X11
strip usr/bin/X11/fluid

cp ../../lib/libfltk.a usr/lib
cp ../../src/libfltk.so.1 usr/lib
ln -sf libfltk.so.1 usr/lib/libfltk.so

cp ../../documentation/*.html usr/info/fltk
cp ../../documentation/*.gif usr/info/fltk
cp ../../documentation/*.jpg usr/info/fltk

cp ../../documentation/fluid.1 usr/man/man1

ln -sf FL usr/include/Fl
cd usr/include/FL
cp ../../../../../FL/*.[hH] .
for file in *.H; do
	ln -s $file `basename $file .H`.h
done
cd ../../..

kits fltk.key . fltk-1.0.5-dunix

echo "Archiving distribution..."

tar cf fltk-1.0.5-dunix.tar fltk-1.0.5-dunix

echo "Compressing distribution..."
rm -f fltk-1.0.5-dunix.tar.gz
gzip -9 fltk-1.0.5-dunix.tar

echo "Removing temporary distribution files..."
rm -rf fltk-1.0.5-dunix
rm -rf usr
