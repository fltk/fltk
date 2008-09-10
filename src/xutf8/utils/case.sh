#!/bin/sh


nopsc=`grep 'CAPITAL' ../UnicodeData-2.txt`

IFS="
"

#echo "#
# List of case chars
#
# 
#       Format:  Three tab-separated columns
#                Column #1 is the non-spacing Unicode (in hex as 0xXXXX)
#                Column #2 is the spacing Unicode (in hex as 0xXXXX)
#                Column #3 the Unicode name (follows a comment sign, '#')
# " > case.txt

rm -f case.txt


for line in ${nopsc}
do
	ucs=`echo ${line} | cut -d\; -f1`
	name=`echo ${line} | cut -d\; -f2 | cut -d\; -f1| sed s/CAPITAL/SMALL/`
	small=`grep ";${name};" ../UnicodeData-2.txt` 
	if test "X${small}" != X ;then
		tbl=`echo ${small} | cut -d\; -f1`
	#	echo "0x${ucs}	0x${tbl}	#	${name}" >> space.txt
		echo "/* U+${ucs} */ 0x${tbl}," >> case.txt
	else
	#	echo "0x${ucs}	0x${ucs}	#	${name}" >> space.txt
		echo "/* U+${ucs} */ 0x0," >> case.txt
	fi
done

unset nospc

# echo "/* EOF */" >> space.txt

cat case.txt | ./create_table "spacing" > "../headers/case.h" 2> ../headers/case_tbl.txt

rm -f case.txt

cat >../case.c << ENDOFTEXT
/******************************************************************************
              Copyright 2001 by O'ksi'D

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of O'ksi'D
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.
O'ksi'D makes no representations about the suitability of
this software for any purpose.  It is provided "as is" without
express or implied warranty.

O'ksi'D DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL O'ksi'D BE LIABLE FOR ANY SPECIAL, INDIRECT
OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
OR PERFORMANCE OF THIS SOFTWARE.

        Author: Jean-Marc Lienher ( http://oksid.ch )

******************************************************************************/

#include "headers/case.h"

ENDOFTEXT

echo "
int 
XUtf8Tolower(
	int ucs)
{
	int ret;
" >>../case.c

tables=`cat ../headers/case_tbl.txt`

for line in ${tables} 
do
	tbl=`echo ${line} | cut -d']' -f1`
	bot=`echo ${line} | cut -d'_' -f3 | cut -d'[' -f1`
	eot=`echo ${line} | cut -d'	' -f2 | cut -d'+' -f2 | cut -d' ' -f1`
	echo "\
	if (ucs <= 0x${eot}) {
		if (ucs >= 0x${bot}) {
			ret = ${tbl}ucs - 0x${bot}];
			if (ret > 0) return ret;
		}
		return ucs;
	}
" >>../case.c

done

echo "	return ucs;
}
" >>../case.c


