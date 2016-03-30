//
// "$Id$"
//
// A base class for platform specific system calls.
//
// Copyright 1998-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//


#include <FL/Fl_System_Driver.H>
#include <FL/fl_utf8.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

const int Fl_System_Driver::flNoValue =	    0x0000;
const int Fl_System_Driver::flWidthValue =  0x0004;
const int Fl_System_Driver::flHeightValue = 0x0008;
const int Fl_System_Driver::flXValue =      0x0001;
const int Fl_System_Driver::flYValue =      0x0002;
const int Fl_System_Driver::flXNegative =   0x0010;
const int Fl_System_Driver::flYNegative =   0x0020;


Fl_System_Driver::Fl_System_Driver()
{
}


Fl_System_Driver::~Fl_System_Driver()
{
}

void Fl_System_Driver::warning(const char* format, ...) {
  va_list args;
  va_start(args, format);
  driver()->warning(format, args);
  va_end(args);
}

void Fl_System_Driver::warning(const char* format, va_list args) {
  vfprintf(stderr, format, args);
  fputc('\n', stderr);
  fflush(stderr);
}

void Fl_System_Driver::error(const char* format, ...) {
  va_list args;
  va_start(args, format);
  driver()->error(format, args);
  va_end(args);
}

void Fl_System_Driver::error(const char *format, va_list args) {
  vfprintf(stderr, format, args);
  fputc('\n', stderr);
  fflush(stderr);
}

void Fl_System_Driver::fatal(const char* format, ...) {
  va_list args;
  va_start(args, format);
  driver()->fatal(format, args);
  va_end(args);
}

void Fl_System_Driver::fatal(const char *format, va_list args) {
  vfprintf(stderr, format, args);
  fputc('\n', stderr);
  fflush(stderr);
  exit(1);
}

/* the following function was stolen from the X sources as indicated. */

/* Copyright 	Massachusetts Institute of Technology  1985, 1986, 1987 */
/* $XConsortium: XParseGeom.c,v 11.18 91/02/21 17:23:05 rws Exp $ */

/*
 Permission to use, copy, modify, distribute, and sell this software and its
 documentation for any purpose is hereby granted without fee, provided that
 the above copyright notice appear in all copies and that both that
 copyright notice and this permission notice appear in supporting
 documentation, and that the name of M.I.T. not be used in advertising or
 publicity pertaining to distribution of the software without specific,
 written prior permission.  M.I.T. makes no representations about the
 suitability of this software for any purpose.  It is provided "as is"
 without express or implied warranty.
 */

/*
 *    XParseGeometry parses strings of the form
 *   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
 *   width, height, xoffset, and yoffset are unsigned integers.
 *   Example:  "=80x24+300-49"
 *   The equal sign is optional.
 *   It returns a bitmask that indicates which of the four values
 *   were actually found in the string.  For each value found,
 *   the corresponding argument is updated;  for each value
 *   not found, the corresponding argument is left unchanged.
 */

static int ReadInteger(char* string, char** NextString)
{
  int Result = 0;
  int Sign = 1;
  
  if (*string == '+')
    string++;
  else if (*string == '-') {
    string++;
    Sign = -1;
  }
  for (; (*string >= '0') && (*string <= '9'); string++) {
    Result = (Result * 10) + (*string - '0');
  }
  *NextString = string;
  if (Sign >= 0)
    return (Result);
  else
    return (-Result);
}

int Fl_System_Driver::XParseGeometry(const char* string, int* x, int* y,
                   unsigned int* width, unsigned int* height)
{
  int mask = Fl_System_Driver::flNoValue;
  char *strind;
  unsigned int tempWidth = 0, tempHeight = 0;
  int tempX = 0, tempY = 0;
  char *nextCharacter;
  
  if ( (string == NULL) || (*string == '\0')) return(mask);
  if (*string == '=')
    string++;  /* ignore possible '=' at beg of geometry spec */
  
  strind = (char *)string;
  if (*strind != '+' && *strind != '-' && *strind != 'x') {
    tempWidth = ReadInteger(strind, &nextCharacter);
    if (strind == nextCharacter)
      return (0);
    strind = nextCharacter;
    mask |= flWidthValue;
  }
  
  if (*strind == 'x' || *strind == 'X') {
    strind++;
    tempHeight = ReadInteger(strind, &nextCharacter);
    if (strind == nextCharacter)
      return (0);
    strind = nextCharacter;
    mask |= flHeightValue;
  }
  
  if ((*strind == '+') || (*strind == '-')) {
    if (*strind == '-') {
      strind++;
      tempX = -ReadInteger(strind, &nextCharacter);
      if (strind == nextCharacter)
        return (0);
      strind = nextCharacter;
      mask |= flXNegative;
      
    } else {
      strind++;
      tempX = ReadInteger(strind, &nextCharacter);
      if (strind == nextCharacter)
        return(0);
      strind = nextCharacter;
    }
    mask |= flXValue;
    if ((*strind == '+') || (*strind == '-')) {
      if (*strind == '-') {
        strind++;
        tempY = -ReadInteger(strind, &nextCharacter);
        if (strind == nextCharacter)
          return(0);
        strind = nextCharacter;
        mask |= flYNegative;
        
      } else {
        strind++;
        tempY = ReadInteger(strind, &nextCharacter);
        if (strind == nextCharacter)
          return(0);
        strind = nextCharacter;
      }
      mask |= flYValue;
    }
  }
  
  /* If strind isn't at the end of the string the it's an invalid
   geometry specification. */
  
  if (*strind != '\0') return (0);
  
  if (mask & flXValue)
    *x = tempX;
  if (mask & flYValue)
    *y = tempY;
  if (mask & flWidthValue)
    *width = tempWidth;
  if (mask & flHeightValue)
    *height = tempHeight;
  return (mask);
}

char *Fl_System_Driver::getenv(const char *v) {
  return ::getenv(v);
}

unsigned Fl_System_Driver::utf8towc(const char* src, unsigned srclen, wchar_t* dst, unsigned dstlen) {
  const char* p = src;
  const char* e = src+srclen;
  unsigned count = 0;
  if (dstlen) for (;;) {
    if (p >= e) {
      dst[count] = 0;
      return count;
    }
    if (!(*p & 0x80)) { /* ascii */
      dst[count] = *p++;
    } else {
      int len; unsigned ucs = fl_utf8decode(p,e,&len);
      p += len;
      dst[count] = (wchar_t)ucs;
    }
    if (++count == dstlen) {dst[count-1] = 0; break;}
  }
  /* we filled dst, measure the rest: */
  while (p < e) {
    if (!(*p & 0x80)) p++;
    else {
      int len; fl_utf8decode(p,e,&len);
      p += len;
    }
    ++count;
  }
  return count;
}

unsigned Fl_System_Driver::utf8fromwc(char* dst, unsigned dstlen, const wchar_t* src, unsigned srclen)
{
  unsigned i = 0;
  unsigned count = 0;
  if (dstlen) for (;;) {
    unsigned ucs;
    if (i >= srclen) {dst[count] = 0; return count;}
    ucs = src[i++];
    if (ucs < 0x80U) {
      dst[count++] = ucs;
      if (count >= dstlen) {dst[count-1] = 0; break;}
    } else if (ucs < 0x800U) { /* 2 bytes */
      if (count+2 >= dstlen) {dst[count] = 0; count += 2; break;}
      dst[count++] = 0xc0 | (ucs >> 6);
      dst[count++] = 0x80 | (ucs & 0x3F);
    } else if (ucs >= 0x10000) {
      if (ucs > 0x10ffff) {
        ucs = 0xfffd;
        goto J1;
      }
      if (count+4 >= dstlen) {dst[count] = 0; count += 4; break;}
      dst[count++] = 0xf0 | (ucs >> 18);
      dst[count++] = 0x80 | ((ucs >> 12) & 0x3F);
      dst[count++] = 0x80 | ((ucs >> 6) & 0x3F);
      dst[count++] = 0x80 | (ucs & 0x3F);
    } else {
J1:
      /* all others are 3 bytes: */
      if (count+3 >= dstlen) {dst[count] = 0; count += 3; break;}
      dst[count++] = 0xe0 | (ucs >> 12);
      dst[count++] = 0x80 | ((ucs >> 6) & 0x3F);
      dst[count++] = 0x80 | (ucs & 0x3F);
    }
  }
  /* we filled dst, measure the rest: */
  while (i < srclen) {
    unsigned ucs = src[i++];
    if (ucs < 0x80U) {
      count++;
    } else if (ucs < 0x800U) { /* 2 bytes */
      count += 2;
    } else if (ucs >= 0x10000 && ucs <= 0x10ffff) {
      count += 4;
    } else {
      count += 3;
    }
  }
  return count;
}

int Fl_System_Driver::utf8locale() {
  static int ret = 2;
  if (ret == 2) {
    char* s;
    ret = 1; // assume UTF-8 if no locale
    if (((s = getenv("LC_CTYPE")) && *s) ||
        ((s = getenv("LC_ALL"))   && *s) ||
        ((s = getenv("LANG"))     && *s)) {
      ret = (strstr(s,"utf") || strstr(s,"UTF"));
    }
  }
  return ret;
}

unsigned Fl_System_Driver::utf8to_mb(const char* src, unsigned srclen, char* dst, unsigned dstlen) {
  wchar_t lbuf[1024];
  wchar_t* buf = lbuf;
  unsigned length = fl_utf8towc(src, srclen, buf, 1024);
  int ret; // note: wcstombs() returns unsigned(length) or unsigned(-1)
  if (length >= 1024) {
    buf = (wchar_t*)(malloc((length+1)*sizeof(wchar_t)));
    fl_utf8towc(src, srclen, buf, length+1);
  }
  if (dstlen) {
    ret = wcstombs(dst, buf, dstlen);
    if (ret >= (int)dstlen-1) ret = wcstombs(0,buf,0);
  } else {
    ret = wcstombs(0,buf,0);
  }
  if (buf != lbuf) free(buf);
  if (ret >= 0) return (unsigned)ret;
  // on any errors we return the UTF-8 as raw text...
  if (srclen < dstlen) {
    memcpy(dst, src, srclen);
    dst[srclen] = 0;
  } else {
    // Buffer insufficent or buffer query
  }
  return srclen;
}

unsigned Fl_System_Driver::utf8from_mb(char* dst, unsigned dstlen, const char* src, unsigned srclen) {
  wchar_t lbuf[1024];
  wchar_t* buf = lbuf;
  int length;
  unsigned ret;
  length = mbstowcs(buf, src, 1024);
  if (length >= 1024) {
    length = mbstowcs(0, src, 0)+1;
    buf = (wchar_t*)(malloc(length*sizeof(wchar_t)));
    mbstowcs(buf, src, length);
  }
  if (length >= 0) {
    ret = fl_utf8fromwc(dst, dstlen, buf, length);
    if (buf != lbuf) free(buf);
    return ret;
  }
  // errors in conversion return the UTF-8 unchanged
  if (srclen < dstlen) {
    memcpy(dst, src, srclen);
    dst[srclen] = 0;
  } else {
    // Buffer insufficent or buffer query
  }
  return srclen;
}

int Fl_System_Driver::clocale_printf(FILE *output, const char *format, va_list args) {
  return vfprintf(output, format, args);
}

//
// End of "$Id$".
//
