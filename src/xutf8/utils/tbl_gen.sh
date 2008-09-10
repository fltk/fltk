#!/bin/sh
#/******************************************************************************
#*
#*    generates ucs2fontmap.c and headers/*_.h
#*
#*                 Copyright (c) 2000-2001  O'ksi'D
#*
#* Permission to use, copy, modify, distribute, and sell this software
#* and its documentation for any purpose is hereby granted without fee,
#* provided that the above copyright notice appear in all copies and
#* that both that copyright notice and this permission notice appear
#* in supporting documentation, and that the name of O'ksi'D
#* not be used in advertising or publicity pertaining to distribution
#* of the software without specific, written prior permission.
#* O'ksi'D makes no representations about the suitability of
#* this software for any purpose.  It is provided "as is" without
#* express or implied warranty.
#* 
#* O'ksi'D DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
#* SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
#* IN NO EVENT SHALL O'ksi'D BE LIABLE FOR ANY SPECIAL, INDIRECT
#* OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
#* OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
#* OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
#* OR PERFORMANCE OF THIS SOFTWARE.
#*
#*   Author : Jean-Marc Lienher ( http://oksid.ch )
#*
#******************************************************************************/

# iso10646-1

encode="iso8859-1 iso8859-2 iso8859-3 \
	iso8859-4 iso8859-5 iso8859-6 iso8859-7 iso8859-8 iso8859-9 \
	iso8859-10 iso8859-13 iso8859-14 iso8859-15 \
	koi8-1 big5-0 ksc5601.1987-0 gb2312.1980-0 jisx0201.1976-0 \
	jisx0208.1983-0 jisx0212.1990-0 symbol dingbats"

mkdir -p ../headers/
rm -f ../headers/* ucs2fontmap

for enc in ${encode}
do
	echo ${enc}

	case ${enc} in 
	ksc5601.1987-0)
#		cat ../MAPPINGS/EASTASIA/KSC/KSC5601.TXT | \
		cat ../MAPPINGS/EASTASIA/KSC/KSX1001.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	koi8-1)
		cat ../MAPPINGS/VENDORS/MISC/KOI8-R.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	iso8859-14)
		cat ../MAPPINGS/ISO8859/8859-14.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	iso8859-13)
		cat ../MAPPINGS/ISO8859/8859-13.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	iso8859-5)
		cat ../MAPPINGS/ISO8859/8859-5.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	iso8859-6)
		cat ../MAPPINGS/ISO8859/8859-6.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	iso8859-1)
		cat ../MAPPINGS/ISO8859/8859-1.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	iso8859-10)
		cat ../MAPPINGS/ISO8859/8859-10.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	iso8859-15)
		cat ../MAPPINGS/ISO8859/8859-15.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	iso8859-2)
		cat ../MAPPINGS/ISO8859/8859-2.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	iso8859-3)
		cat ../MAPPINGS/ISO8859/8859-3.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	iso8859-4)
		cat ../MAPPINGS/ISO8859/8859-4.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	iso8859-7)
		cat ../MAPPINGS/ISO8859/8859-7.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	iso8859-8)
		cat ../MAPPINGS/ISO8859/8859-8.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	iso8859-9)
		cat ../MAPPINGS/ISO8859/8859-9.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	dingbats)
		cat ../MAPPINGS/VENDORS/ADOBE/zdingbat.txt | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	symbol)
		cat ../MAPPINGS/VENDORS/ADOBE/symbol.txt | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	big5-0)
		cat ../MAPPINGS/EASTASIA/OTHER/BIG5.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	gb2312.1980-0)
		cat ../MAPPINGS/EASTASIA/GB/GB2312.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	jisx0212.1990-0)
		cat ../MAPPINGS/EASTASIA/JIS/JIS0212.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	jisx0208.1983-0)
		cat ../MAPPINGS/EASTASIA/JIS/JIS0208.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	jisx0201.1976-0)
		cat ../MAPPINGS/EASTASIA/JIS/JIS0201.TXT | \
			./convert_map "${enc}" > ${enc}.txt
		;;
	esac
	nm=`echo ${enc} |tr '.' '_' | tr '-' '_'`
	cat ${enc}.txt | sort | uniq | \
	 	./create_table "${nm}"  >> ../headers/${nm}_.h 2>> ../headers/tbl.txt 
	rm -f ${enc}.txt
	enc=" "
done

cat > ../ucs2fontmap.c << ENDOFTEXT
/******************************************************************************
              Copyright 2000-2001 by O'ksi'D

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

ENDOFTEXT


he=`cd ..; ls headers/*.h`
for hea in ${he}
do
 	echo "#include \"${hea}\"" >> ../ucs2fontmap.c
done


cat ../headers/tbl.txt | ./conv_gen >> ../ucs2fontmap.c


