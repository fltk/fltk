#!/bin/sh
#
# makedist - make an irix distribution.
#

case `uname -r` in
	5.*)
	gendist -v -dist . -sbase ../.. -idb fltk5x.list -spec fltk.spec
	;;
	*)
	gendist -v -dist . -sbase ../.. -idb fltk.list -spec fltk.spec
	;;
esac

tar cvf fltk-1.0.1-irix-`uname -r`.tardist fltk fltk.idb fltk.man fltk.sw

