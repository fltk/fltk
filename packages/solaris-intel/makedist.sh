#!/bin/sh
#
# makedist - make a solaris intel distribution.
#

rm -rf fltk

echo "Making distribution..."

cd ../..
#
# Create a dummy prototype file with the full path for the pkginfo file;
# this is a bug in Solaris' pkgmk program...
#

cwd=`pwd`
echo "i pkginfo=$cwd/packages/solaris-intel/fltk.pkginfo" >fltk.prototype
cat packages/solaris-intel/fltk.prototype >>fltk.prototype
pkgmk -d packages/solaris-intel -b $cwd -f fltk.prototype fltk
rm fltk.prototype
cd packages/solaris-intel

echo "Packing distribution..."

pkgtrans -s . fltk-1.0.1-solaris-intel.pkg fltk
rm -rf fltk

echo "Compressing distribution..."

rm -f fltk-1.0.1-solaris-intel.pkg.gz
gzip -9 fltk-1.0.1-solaris-intel.pkg
