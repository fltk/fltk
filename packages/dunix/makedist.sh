#!/bin/sh
#
# makedist - make a digital unix distribution.
#

rm -rf fltk-1.0-dunix
mkdir fltk-1.0-dunix

rm -rf usr
mkdir usr

ln -sf ../../../lib usr

ln -sf ../../../documentation usr/info

mkdir usr/include
mkdir usr/include/FL
ln -sf FL usr/include/Fl
cd usr/include/FL
ln -s ../../../../../FL/*.[hH] .
for file in *.H; do
	ln -s $file `basename $file .H`.h
done

kits fltk.key ../.. fltk-1.0-dunix

echo "Archiving distribution..."

tar cf fltk-1.0-dunix.tar fltk-1.0-dunix

echo "Compressing distribution..."
gzip -9 fltk-1.0-dunix.tar

echo "Removing temporary distribution files..."
rm -rf fltk-1.0-dunix
rm -rf usr
