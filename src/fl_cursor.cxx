//
// "$Id: fl_cursor.cxx,v 1.6.2.6.2.8 2003/01/30 21:43:30 easysw Exp $"
//
// Mouse cursor support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2003 by Bill Spitzak and others.
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

// Change the current cursor.
// Under X the cursor is attached to the X window.  I tried to hide
// this and pretend that changing the cursor is a drawing function.
// This avoids a field in the Fl_Window, and I suspect is more
// portable to other systems.

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/x.H>
#if !defined(WIN32) && !defined(__APPLE__)
#  include <X11/cursorfont.h>
#endif
#include <FL/fl_draw.H>

void fl_cursor(Fl_Cursor c, Fl_Color fg, Fl_Color bg) {
  if (Fl::first_window()) Fl::first_window()->cursor(c,fg,bg);
}

void Fl_Window::default_cursor(Fl_Cursor c, Fl_Color fg, Fl_Color bg) {
//  if (c == FL_CURSOR_DEFAULT) c = FL_CURSOR_ARROW;

  cursor_default = c;
  cursor_fg      = fg;
  cursor_bg      = bg;

  cursor(c, fg, bg);
}

#ifdef WIN32

#  ifndef IDC_HAND
#    define IDC_HAND	MAKEINTRESOURCE(32649)
#  endif // !IDC_HAND

void Fl_Window::cursor(Fl_Cursor c, Fl_Color, Fl_Color) {
  if (!shown()) return;
  if (c == FL_CURSOR_DEFAULT) {
    c = cursor_default;
  }
  if (c > FL_CURSOR_NESW) {
    i->cursor = 0;
  } else if (c == FL_CURSOR_DEFAULT) {
    i->cursor = fl_default_cursor;
  } else {
    LPSTR n;
    switch (c) {
    case FL_CURSOR_ARROW:	n = IDC_ARROW; break;
    case FL_CURSOR_CROSS:	n = IDC_CROSS; break;
    case FL_CURSOR_WAIT:	n = IDC_WAIT; break;
    case FL_CURSOR_INSERT:	n = IDC_IBEAM; break;
    case FL_CURSOR_HELP:	n = IDC_HELP; break;
    case FL_CURSOR_HAND: {
          OSVERSIONINFO osvi;

          // Get the OS version: Windows 98 and 2000 have a standard
	  // hand cursor.
          memset(&osvi, 0, sizeof(OSVERSIONINFO));
          osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
          GetVersionEx(&osvi);

          if (osvi.dwMajorVersion > 4 ||
  	      (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion > 0 &&
  	       osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)) n = IDC_HAND;
          else n = IDC_UPARROW;
	} break;
    case FL_CURSOR_MOVE:	n = IDC_SIZEALL; break;
    case FL_CURSOR_N:
    case FL_CURSOR_S:
    case FL_CURSOR_NS:		n = IDC_SIZENS; break;
    case FL_CURSOR_NE:
    case FL_CURSOR_SW:
    case FL_CURSOR_NESW:	n = IDC_SIZENESW; break;
    case FL_CURSOR_E:
    case FL_CURSOR_W:
    case FL_CURSOR_WE:		n = IDC_SIZEWE; break;
    case FL_CURSOR_SE:
    case FL_CURSOR_NW:
    case FL_CURSOR_NWSE:	n = IDC_SIZENWSE; break;
    default:			n = IDC_NO; break;
    }
    i->cursor = LoadCursor(NULL, n);
  }
  SetCursor(i->cursor);
}

#elif defined(__APPLE__)

static Cursor crsrHAND =
{
  { 0x0600, 0x0900, 0x0900, 0x0900, 0x09C0, 0x0938, 0x6926, 0x9805,
    0x8801, 0x4801, 0x2002, 0x2002, 0x1004, 0x0804, 0x0408, 0x0408 },
  { 0x0600, 0x0F00, 0x0F00, 0x0F00, 0x0FC0, 0x0FF8, 0x6FFE, 0xFFFF,
    0xFFFF, 0x7FFF, 0x3FFE, 0x3FFE, 0x1FFC, 0x0FFC, 0x07F8, 0x07F8 },
  { 1, 5 } // Hotspot: ( y, x )
}, *crsrHANDptr = &crsrHAND;
static Cursor crsrHELP =
{
  { 0x0000, 0x4000, 0x6000, 0x7000, 0x783C, 0x7C7E, 0x7E66, 0x7F06,
    0x7F8C, 0x7C18, 0x6C18, 0x4600, 0x0618, 0x0318, 0x0300, 0x0000 },
  { 0xC000, 0xE000, 0xF000, 0xF83C, 0xFC7E, 0xFEFF, 0xFFFF, 0xFFFF,
    0xFFFE, 0xFFFC, 0xFE3C, 0xEF3C, 0xCF3C, 0x07BC, 0x0798, 0x0380 },
  { 1, 1 }
}, *crsrHELPptr = &crsrHELP;
static Cursor crsrMOVE =
{
  { 0x0000, 0x0180, 0x03C0, 0x07E0, 0x07E0, 0x1998, 0x399C, 0x7FFE,
    0x7FFE, 0x399C, 0x1998, 0x07E0, 0x07E0, 0x03C0, 0x0180, 0x0000 },
  { 0x0180, 0x03C0, 0x07E0, 0x0FF0, 0x1FF8, 0x3FFC, 0x7FFE, 0xFFFF,
    0xFFFF, 0x7FFE, 0x3FFC, 0x1FF8, 0x0FF0, 0x07E0, 0x03C0, 0x0180 },
  { 8, 8 }
}, *crsrMOVEptr = &crsrMOVE;
static Cursor crsrNS =
{
  { 0x0000, 0x0180, 0x03C0, 0x07E0, 0x0FF0, 0x0180, 0x0180, 0x0180,
    0x0180, 0x0180, 0x0180, 0x0FF0, 0x07E0, 0x03C0, 0x0180, 0x0000 },
  { 0x0180, 0x03C0, 0x07E0, 0x0FF0, 0x1FF8, 0x1FF8, 0x03C0, 0x03C0,
    0x03C0, 0x03C0, 0x1FF8, 0x1FF8, 0x0FF0, 0x07E0, 0x03C0, 0x0180 },
  { 8, 8 }
}, *crsrNSptr = &crsrNS;
static Cursor crsrWE =
{
  { 0x0000, 0x0000, 0x0000, 0x0000, 0x0810, 0x1818, 0x381C, 0x7FFE,
    0x7FFE, 0x381C, 0x1818, 0x0810, 0x0000, 0x0000, 0x0000, 0x0000 },
  { 0x0000, 0x0000, 0x0000, 0x0C30, 0x1C38, 0x3C3C, 0x7FFE, 0xFFFF,
    0xFFFF, 0x7FFE, 0x3C3C, 0x1C38, 0x0C30, 0x0000, 0x0000, 0x0000 },
  { 8, 8 }
}, *crsrWEptr = &crsrWE;
static Cursor crsrNWSE =
{
  { 0x0000, 0x7E00, 0x7C00, 0x7800, 0x7C00, 0x6E00, 0x4710, 0x03B0,
    0x01F0, 0x00F0, 0x01F0, 0x03F0, 0x0000, 0x0000, 0x0000, 0x0000 },
  { 0xFF00, 0xFF00, 0xFE00, 0xFC00, 0xFE00, 0xFF18, 0xEFB8, 0xC7F8,
    0x03F8, 0x01F8, 0x03F8, 0x07F8, 0x07F8, 0x0000, 0x0000, 0x0000 },
  { 8, 8 }
}, *crsrNWSEptr = &crsrNWSE;
static Cursor crsrNESW =
{
  { 0x0000, 0x03F0, 0x01F0, 0x00F0, 0x01F0, 0x03B0, 0x4710, 0x6E00,
    0x7C00, 0x7800, 0x7C00, 0x7E00, 0x0000, 0x0000, 0x0000, 0x0000 },
  { 0x07F8, 0x07F8, 0x03F8, 0x01F8, 0x03F8, 0xC7F8, 0xEFB8, 0xFF18,
    0xFE00, 0xFC00, 0xFE00, 0xFF00, 0xFF00, 0x0000, 0x0000, 0x0000 },
  { 8, 8 }
}, *crsrNESWptr = &crsrNESW;
static Cursor crsrNONE =
{
  { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
  { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
  { 0, 0 }
}, *crsrNONEptr = &crsrNONE;
static Cursor crsrARROW =
{
  { 0x0000, 0x4000, 0x6000, 0x7000, 0x7800, 0x7C00, 0x7E00, 0x7F00,
    0x7F80, 0x7C00, 0x6C00, 0x4600, 0x0600, 0x0300, 0x0300, 0x0000 },
  { 0xC000, 0xE000, 0xF000, 0xF800, 0xFC00, 0xFE00, 0xFF00, 0xFF80,
    0xFFC0, 0xFFC0, 0xFE00, 0xEF00, 0xCF00, 0x0780, 0x0780, 0x0380 },
  { 1, 1 }
}, *crsrARROWptr = &crsrARROW;


void Fl_Window::cursor(Fl_Cursor c, Fl_Color, Fl_Color) {
  if (!shown()) return;
  if (c == FL_CURSOR_DEFAULT) {
    c = cursor_default;
  }
  switch (c) {
  case FL_CURSOR_CROSS:     i->cursor = GetCursor( crossCursor ); break;
  case FL_CURSOR_WAIT:      i->cursor = GetCursor( watchCursor ); break;
  case FL_CURSOR_INSERT:    i->cursor = GetCursor( iBeamCursor ); break;
  case FL_CURSOR_N:
  case FL_CURSOR_S:
  case FL_CURSOR_NS:        i->cursor = &crsrNSptr; break;
  case FL_CURSOR_HELP:	i->cursor = &crsrHELPptr; break;
  case FL_CURSOR_HAND:	i->cursor = &crsrHANDptr; break;
  case FL_CURSOR_MOVE:	i->cursor = &crsrMOVEptr; break;
  case FL_CURSOR_NE:
  case FL_CURSOR_SW:
  case FL_CURSOR_NESW:	i->cursor = &crsrNESWptr; break;
  case FL_CURSOR_E:
  case FL_CURSOR_W:
  case FL_CURSOR_WE:	i->cursor = &crsrWEptr; break;
  case FL_CURSOR_SE:
  case FL_CURSOR_NW:
  case FL_CURSOR_NWSE:	i->cursor = &crsrNWSEptr; break;
  case FL_CURSOR_NONE:	i->cursor = &crsrNONEptr; break;
  case FL_CURSOR_ARROW:   	i->cursor = &crsrARROWptr; break;
  case FL_CURSOR_DEFAULT:
  default:
    i->cursor = fl_default_cursor; break;
  }
  SetCursor( *i->cursor );
}

#else

// I like the MSWindows resize cursors, so I duplicate them here:

#define CURSORSIZE 16
#define HOTXY 7
static struct TableEntry {
  uchar bits[CURSORSIZE*CURSORSIZE/8];
  uchar mask[CURSORSIZE*CURSORSIZE/8];
  Cursor cursor;
} table[] = {
  {{	// FL_CURSOR_NS
   0x00, 0x00, 0x80, 0x01, 0xc0, 0x03, 0xe0, 0x07, 0x80, 0x01, 0x80, 0x01,
   0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
   0xe0, 0x07, 0xc0, 0x03, 0x80, 0x01, 0x00, 0x00},
   {
   0x80, 0x01, 0xc0, 0x03, 0xe0, 0x07, 0xf0, 0x0f, 0xf0, 0x0f, 0xc0, 0x03,
   0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xf0, 0x0f,
   0xf0, 0x0f, 0xe0, 0x07, 0xc0, 0x03, 0x80, 0x01}},
  {{	// FL_CURSOR_EW
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x10,
   0x0c, 0x30, 0xfe, 0x7f, 0xfe, 0x7f, 0x0c, 0x30, 0x08, 0x10, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x1c, 0x38,
   0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0x1c, 0x38, 0x18, 0x18,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
  {{	// FL_CURSOR_NWSE
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x38, 0x00, 0x78, 0x00,
   0xe8, 0x00, 0xc0, 0x01, 0x80, 0x03, 0x00, 0x17, 0x00, 0x1e, 0x00, 0x1c,
   0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   {
   0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0x7c, 0x00, 0xfc, 0x00,
   0xfc, 0x01, 0xec, 0x03, 0xc0, 0x37, 0x80, 0x3f, 0x00, 0x3f, 0x00, 0x3e,
   0x00, 0x3f, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00}},
  {{	// FL_CURSOR_NESW
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x1c, 0x00, 0x1e,
   0x00, 0x17, 0x80, 0x03, 0xc0, 0x01, 0xe8, 0x00, 0x78, 0x00, 0x38, 0x00,
   0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x3f, 0x00, 0x3e, 0x00, 0x3f,
   0x80, 0x3f, 0xc0, 0x37, 0xec, 0x03, 0xfc, 0x01, 0xfc, 0x00, 0x7c, 0x00,
   0xfc, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00}},
  {{0}, {0}} // FL_CURSOR_NONE & unknown
};

void Fl_Window::cursor(Fl_Cursor c, Fl_Color fg, Fl_Color bg) {
  if (!shown()) return;
  Cursor xc;
  int deleteit = 0;
  if (c == FL_CURSOR_DEFAULT) {
    c  = cursor_default;
    fg = cursor_fg;
    bg = cursor_bg;
  }

  if (!c) {
    xc = None;
  } else {
    if (c >= FL_CURSOR_NS) {
      TableEntry *q = (c > FL_CURSOR_NESW) ? table+4 : table+(c-FL_CURSOR_NS);
      if (!(q->cursor)) {
	XColor dummy;
	Pixmap p = XCreateBitmapFromData(fl_display,
	  RootWindow(fl_display, fl_screen), (const char*)(q->bits),
	  CURSORSIZE, CURSORSIZE);
	Pixmap m = XCreateBitmapFromData(fl_display,
	  RootWindow(fl_display, fl_screen), (const char*)(q->mask),
	  CURSORSIZE, CURSORSIZE);
	q->cursor = XCreatePixmapCursor(fl_display, p,m,&dummy, &dummy,
					HOTXY, HOTXY);
	XFreePixmap(fl_display, m);
	XFreePixmap(fl_display, p);
      }
      xc = q->cursor;
    } else {
      xc = XCreateFontCursor(fl_display, (c-1)*2);
      deleteit = 1;
    }
    XColor fgc;
    uchar r,g,b;
    Fl::get_color(fg,r,g,b);
    fgc.red = r<<8; fgc.green = g<<8; fgc.blue = b<<8;
    XColor bgc;
    Fl::get_color(bg,r,g,b);
    bgc.red = r<<8; bgc.green = g<<8; bgc.blue = b<<8;
    XRecolorCursor(fl_display, xc, &fgc, &bgc);
  }
  XDefineCursor(fl_display, fl_xid(this), xc);
  if (deleteit) XFreeCursor(fl_display, xc);
}

#endif

//
// End of "$Id: fl_cursor.cxx,v 1.6.2.6.2.8 2003/01/30 21:43:30 easysw Exp $".
//
