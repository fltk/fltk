//
// "$Id: fl_cursor.cxx,v 1.6.2.6 2001/01/22 15:13:40 easysw Exp $"
//
// Mouse cursor support for the Fast Light Tool Kit (FLTK).
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

// Change the current cursor.
// Under X the cursor is attached to the X window.  I tried to hide
// this and pretend that changing the cursor is a drawing function.
// This avoids a field in the Fl_Window, and I suspect is more
// portable to other systems.

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/x.H>
#ifndef WIN32
#include <X11/cursorfont.h>
#endif
#include <FL/fl_draw.H>

void fl_cursor(Fl_Cursor c, Fl_Color fg, Fl_Color bg) {
  if (Fl::first_window()) Fl::first_window()->cursor(c,fg,bg);
}

#ifdef WIN32

#  ifndef IDC_HAND
#    define IDC_HAND	MAKEINTRESOURCE(32649)
#  endif // !IDC_HAND

void Fl_Window::cursor(Fl_Cursor c, Fl_Color, Fl_Color) {
  if (!shown()) return;
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
  Cursor cursor;
  int deleteit = 0;
  if (!c) {
    cursor = None;
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
      cursor = q->cursor;
    } else {
      cursor = XCreateFontCursor(fl_display, (c-1)*2);
      deleteit = 1;
    }
    XColor fgc;
    uchar r,g,b;
    Fl::get_color(fg,r,g,b);
    fgc.red = r<<8; fgc.green = g<<8; fgc.blue = b<<8;
    XColor bgc;
    Fl::get_color(bg,r,g,b);
    bgc.red = r<<8; bgc.green = g<<8; bgc.blue = b<<8;
    XRecolorCursor(fl_display, cursor, &fgc, &bgc);
  }
  XDefineCursor(fl_display, fl_xid(this), cursor);
  if (deleteit) XFreeCursor(fl_display, cursor);
}

#endif

//
// End of "$Id: fl_cursor.cxx,v 1.6.2.6 2001/01/22 15:13:40 easysw Exp $".
//
