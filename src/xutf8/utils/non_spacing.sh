#!/bin/sh


nopsc=`grep ';Mn;' ../UnicodeData-2.txt`

IFS="
"

#echo "#
# List of non-spacing chars
#
#
#       Format:  Three tab-separated columns
#                Column #1 is the non-spacing Unicode (in hex as 0xXXXX)
#                Column #2 is the spacing Unicode (in hex as 0xXXXX)
#                Column #3 the Unicode name (follows a comment sign, '#')
# " > space.txt

rm -f space.txt


for line in ${nopsc}
do
	ucs=`echo ${line} | cut -d\; -f1`
	name=`echo ${line} | cut -d\; -f2`
	space=`grep " 0020 ${ucs};" ../UnicodeData-2.txt`
	if test "X${space}" != X ;then
		tbl=`echo ${space} | cut -d\; -f1`
	#	echo "0x${ucs}	0x${tbl}	#	${name}" >> space.txt
		echo "/* U+${ucs} */ 0x${tbl}," >> space.txt
	else
	#	echo "0x${ucs}	0x${ucs}	#	${name}" >> space.txt
		echo "/* U+${ucs} */ 0x${ucs}," >> space.txt
	fi
done

unset nospc

# echo "/* EOF */" >> space.txt

cat space.txt | ./create_table "spacing" > "../headers/spacing.h" 2> ../headers/spacing_tbl.txt

rm -f space.txt

cat >../is_spacing.c << ENDOFTEXT
/*
 * Author: Jean-Marc Lienher ( http://oksid.ch )
 * Copyright 2000-2003 by O'ksi'D.
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file.  If this
 * file is missing or damaged, see the license at:
 *
 *     https://www.fltk.org/COPYING.php
 *
 * Please see the following page on how to report bugs and issues:
 *
 *     https://www.fltk.org/bugs.php
 */

#include "headers/spacing.h"

ENDOFTEXT

echo "
unsigned short
XUtf8IsNonSpacing(
	unsigned int ucs)
{
" >>../is_spacing.c

tables=`cat ../headers/spacing_tbl.txt`

for line in ${tables}
do
	tbl=`echo ${line} | cut -d']' -f1`
	bot=`echo ${line} | cut -d'_' -f3 | cut -d'[' -f1`
	eot=`echo ${line} | cut -d'	' -f2 | cut -d'+' -f2 | cut -d' ' -f1`
	echo "\
	if (ucs <= 0x${eot}) {
		if (ucs >= 0x${bot}) return ${tbl}ucs - 0x${bot}];
		return 0;
	}
" >>../is_spacing.c

done

echo "	return 0;
}
" >>../is_spacing.c


