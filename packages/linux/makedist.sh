#!/bin/sh
#
# makedist - make an irix distribution.
#

if [ test `uname -r` =~ '5.*' ]; then
	gendist -v -dist . -sbase ../.. -idb fltk5x.list -spec fltk.spec
else
	gendist -v -dist . -sbase ../.. -idb fltk.list -spec fltk.spec
fi

tar cvf fltk-1.0-irix-`uname -r`.tardist fltk fltk.idb fltk.man fltk.sw

