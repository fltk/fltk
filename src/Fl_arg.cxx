//
// "$Id: Fl_arg.cxx,v 1.5.2.8 2001/03/20 18:02:52 spitzak Exp $"
//
// Optional argument initialization code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

// OPTIONAL initialization code for a program using fltk.
// You do not need to call this!  Feel free to make up your own switches.

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Window.H>
#include <FL/filename.H>
#include <FL/fl_draw.H>
#include <ctype.h>
#include <string.h>

#ifdef WIN32
int XParseGeometry(const char*, int*, int*, unsigned int*, unsigned int*);
#define NoValue		0x0000
#define XValue  	0x0001
#define YValue		0x0002
#define WidthValue  	0x0004
#define HeightValue  	0x0008
#define AllValues 	0x000F
#define XNegative 	0x0010
#define YNegative 	0x0020
#endif

static int match(const char *a, const char *match, int atleast = 1) {
  const char *b = match;
  while (*a && (*a == *b || tolower(*a) == *b)) {a++; b++;}
  return !*a && b >= match+atleast;
}

// flags set by previously parsed arguments:
extern char fl_show_iconic; // in Fl_x.C
static char arg_called;
static char return_i;
static const char *name;
static const char *geometry;
static const char *title;
// these are in Fl_get_system_colors and are set by the switches:
extern const char *fl_fg;
extern const char *fl_bg;
extern const char *fl_bg2;

// consume a switch from argv.  Returns number of words eaten, 0 on error:
int Fl::arg(int argc, char **argv, int &i) {
  arg_called = 1;
  const char *s = argv[i];

  if (!s) {i++; return 1;}	// something removed by calling program?

  // a word that does not start with '-', or a word after a '--', or
  // the word '-' by itself all start the "non-switch arguments" to
  // a program.  Return 0 to indicate that we don't understand the
  // word, but set a flag (return_i) so that args() will return at
  // that point:
  if (s[0] != '-' || s[1] == '-' || !s[1]) {return_i = 1; return 0;}
  s++; // point after the dash

  if (match(s, "iconic")) {
    fl_show_iconic = 1;
    i++;
    return 1;
  }

  const char *v = argv[i+1];
  if (i >= argc-1 || !v)
    return 0;	// all the rest need an argument, so if missing it is an error

  if (match(s, "geometry")) {

    int flags, gx, gy; unsigned int gw, gh;
    flags = XParseGeometry(v, &gx, &gy, &gw, &gh);
    if (!flags) return 0;
    geometry = v;

#ifndef WIN32
  } else if (match(s, "display")) {
    Fl::display(v);
#endif

  } else if (match(s, "title")) {
    title = v;

  } else if (match(s, "name")) {
    name = v;

  } else if (match(s, "bg2", 3) || match(s, "background2", 11)) {
    fl_bg2 = v;

  } else if (match(s, "bg") || match(s, "background")) {
    fl_bg = v;

  } else if (match(s, "fg") || match(s, "foreground")) {
    fl_fg = v;

  } else return 0; // unrecognized

  i += 2;
  return 2;
}

// consume all switches from argv.  Returns number of words eaten.
// Returns zero on error.  'i' will either point at first word that
// does not start with '-', at the error word, or after a '--', or at
// argc.  If your program does not take any word arguments you can
// report an error if i < argc.

int Fl::args(int argc, char** argv, int& i, int (*cb)(int,char**,int&)) {
  arg_called = 1;
  i = 1; // skip argv[0]
  while (i < argc) {
    if (cb && cb(argc,argv,i)) continue;
    if (!arg(argc,argv,i)) return return_i ? i : 0;
  }
  return i;
}


// show a main window, use any parsed arguments
void Fl_Window::show(int argc, char **argv) {
  if (!argc) {show(); return;}
  if (!arg_called) Fl::args(argc,argv);

  // set colors first, so background_pixel is correct:
  static char beenhere;
  if (!beenhere) {
    if (geometry) {
      int flags = 0, gx = x(), gy = y(); unsigned int gw = w(), gh = h();
      flags = XParseGeometry(geometry, &gx, &gy, &gw, &gh);
      if (flags & XNegative) gx = Fl::w()-w()+gx;
      if (flags & YNegative) gy = Fl::h()-h()+gy;
      //  int mw,mh; minsize(mw,mh);
      //  if (mw > gw) gw = mw;
      //  if (mh > gh) gh = mh;
      Fl_Widget *r = resizable();
      if (!r) resizable(this);
      // for WIN32 we assumme window is not mapped yet:
      if (flags & (XValue | YValue))
	x(-1), resize(gx,gy,gw,gh);
      else
	size(gw,gh);
      resizable(r);
    }
  }

  // set the class, which is used by X version of get_system_colors:
  if (name) {xclass(name); name = 0;}
  else if (!xclass()) xclass(filename_name(argv[0]));

  if (title) {label(title); title = 0;}
  else if (!label()) label(xclass());
  show();

  if (!beenhere) {
    beenhere = 1;
    Fl::get_system_colors(); // opens display!  May call Fl::fatal()
  }

#ifndef WIN32
  // set the command string, used by state-saving window managers:
  int j;
  int n=0; for (j=0; j<argc; j++) n += strlen(argv[j])+1;
#ifdef __GNUC__
  char buffer[n];
#else
  char *buffer = new char[n];
#endif
  char *p = buffer;
  for (j=0; j<argc; j++) for (const char *q = argv[j]; (*p++ = *q++););
  XChangeProperty(fl_display, fl_xid(this), XA_WM_COMMAND, XA_STRING, 8, 0,
		  (unsigned char *)buffer, p-buffer-1);
#ifndef __GNUC__
  delete[] buffer;
#endif
#endif

}

// Calls useful for simple demo programs, with automatic help message:

static const char * const helpmsg =
"options are:\n"
" -d[isplay] host:n.n\n"
" -g[eometry] WxH+X+Y\n"
" -t[itle] windowtitle\n"
" -n[ame] classname\n"
" -i[conic]\n"
" -fg color\n"
" -bg color\n"
" -bg2 color";

const char * const Fl::help = helpmsg+13;

void Fl::args(int argc, char **argv) {
  int i; if (Fl::args(argc,argv,i) < argc) Fl::error(helpmsg);
}

#ifdef WIN32

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
  register int Result = 0;
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

int XParseGeometry(const char* string, int* x, int* y,
		   unsigned int* width, unsigned int* height)
{
  int mask = NoValue;
  register char *strind;
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
    mask |= WidthValue;
  }

  if (*strind == 'x' || *strind == 'X') {	
    strind++;
    tempHeight = ReadInteger(strind, &nextCharacter);
    if (strind == nextCharacter)
      return (0);
    strind = nextCharacter;
    mask |= HeightValue;
  }

  if ((*strind == '+') || (*strind == '-')) {
    if (*strind == '-') {
      strind++;
      tempX = -ReadInteger(strind, &nextCharacter);
      if (strind == nextCharacter)
	return (0);
      strind = nextCharacter;
      mask |= XNegative;

    } else {
      strind++;
      tempX = ReadInteger(strind, &nextCharacter);
      if (strind == nextCharacter)
	return(0);
      strind = nextCharacter;
      }
    mask |= XValue;
    if ((*strind == '+') || (*strind == '-')) {
      if (*strind == '-') {
	strind++;
	tempY = -ReadInteger(strind, &nextCharacter);
	if (strind == nextCharacter)
	  return(0);
	strind = nextCharacter;
	mask |= YNegative;

      } else {
	strind++;
	tempY = ReadInteger(strind, &nextCharacter);
	if (strind == nextCharacter)
	  return(0);
	strind = nextCharacter;
      }
      mask |= YValue;
    }
  }
	
  /* If strind isn't at the end of the string the it's an invalid
     geometry specification. */

  if (*strind != '\0') return (0);

  if (mask & XValue)
    *x = tempX;
  if (mask & YValue)
    *y = tempY;
  if (mask & WidthValue)
    *width = tempWidth;
  if (mask & HeightValue)
    *height = tempHeight;
  return (mask);
}

#endif // ifdef WIN32

//
// End of "$Id: Fl_arg.cxx,v 1.5.2.8 2001/03/20 18:02:52 spitzak Exp $".
//
