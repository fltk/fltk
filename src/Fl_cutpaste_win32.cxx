//
// "$Id: Fl_cutpaste_win32.cxx,v 1.5.2.8 2001/01/22 15:13:40 easysw Exp $"
//
// WIN32 cut/paste for the Fast Light Tool Kit (FLTK).
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

// Implementation of cut and paste.

// This is seperated from Fl.C mostly to test Fl::add_handler().
// But this will save a small amount of code size in a program that
// has no text editing fields or other things that call cut or paste.

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Widget.H>
#include <string.h>
#include <stdio.h>

static char *selection_buffer;
static int selection_length;
static int selection_buffer_length;
static char beenhere;
static char ignore_destroy;

extern Fl_Widget *fl_selection_requestor; // widget doing request_paste()

static int selection_xevent_handler(int) {

  switch (fl_msg.message) {

  case WM_DESTROYCLIPBOARD:
    if (!ignore_destroy) {
      Fl::selection_owner(0);
      Fl::flush(); // get the redraw to happen
    }
    return 1;

  case WM_RENDERALLFORMATS:
    if (!OpenClipboard(fl_xid(Fl::first_window()))) return 0;
    EmptyClipboard();
    // fall through...
  case WM_RENDERFORMAT: {
    HANDLE h = GlobalAlloc(GHND, selection_length+1);
    if (h) {
      LPSTR p = (LPSTR)GlobalLock(h);
      memcpy(p, selection_buffer, selection_length);
      p[selection_length] = 0;
      GlobalUnlock(h);
      SetClipboardData(CF_TEXT, h);
    }
    if (fl_msg.message == WM_RENDERALLFORMATS)
      CloseClipboard();
    return 1;}

  default:
    return 0;
  }
}

////////////////////////////////////////////////////////////////

// call this when you create a selection:
void Fl::selection(Fl_Widget &owner, const char *stuff, int len) {
  if (!stuff || len<0) return;
  if (len+1 > selection_buffer_length) {
    delete[] selection_buffer;
    selection_buffer = new char[len+100];
    selection_buffer_length = len+100;
  }
  memcpy(selection_buffer, stuff, len);
  selection_buffer[len] = 0; // needed for direct paste
  selection_length = len;
  ignore_destroy = 1;
  if (OpenClipboard(fl_xid(Fl::first_window()))) {
    EmptyClipboard();
    SetClipboardData(CF_TEXT, NULL);
    CloseClipboard();
    if (!beenhere) {
      Fl::add_handler(selection_xevent_handler);
      beenhere = 1;
    }
  }
  ignore_destroy = 0;
  selection_owner(&owner);
}

////////////////////////////////////////////////////////////////

// Call this when a "paste" operation happens:
void Fl::paste(Fl_Widget &receiver) {
  if (selection_owner()) {
    // We already have it, do it quickly without window server.
    // Notice that the text is clobbered if set_selection is
    // called in response to FL_PASTE!
    Fl::e_text = selection_buffer;
    Fl::e_length = selection_length;
    receiver.handle(FL_PASTE);
  } else {
    if (!OpenClipboard(fl_xid(Fl::first_window()))) return;
    HANDLE h = GetClipboardData(CF_TEXT);
    if (h) {
      Fl::e_text = (LPSTR)GlobalLock(h);
      LPSTR a,b;
      a = b = Fl::e_text;
      while (*a) { // strip the CRLF pairs ($%$#@^)
	if (*a == '\r' && a[1] == '\n') a++;
	else *b++ = *a++;
      }
      *b = 0;
      Fl::e_length = b - Fl::e_text;
      receiver.handle(FL_PASTE);
      GlobalUnlock(h);
    }
    CloseClipboard();
  }
}

//
// End of "$Id: Fl_cutpaste_win32.cxx,v 1.5.2.8 2001/01/22 15:13:40 easysw Exp $".
//
