// Fl_cutpaste_win32.C

// Implementation of cut and paste.

// This is seperated from Fl.C mostly to test Fl::add_handler().
// But this will save a small amount of code size in a program that
// has no text editing fields or other things that call cut or paste.

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Widget.H>
#include <string.h>

static char *selection_buffer;
static int selection_length;
static int selection_buffer_length;
static char beenhere;

extern Fl_Widget *fl_selection_requestor; // widget doing request_paste()

static int selection_xevent_handler(int) {

  switch (fl_msg.message) {

  case WM_DESTROYCLIPBOARD:
    Fl::selection_owner(0);
    Fl::flush(); // get the redraw to happen
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
  if (!stuff || len<=0) return;
  if (len+1 > selection_buffer_length) {
    delete[] selection_buffer;
    selection_buffer = new char[len+100];
    selection_buffer_length = len+100;
  }
  memcpy(selection_buffer, stuff, len);
  selection_buffer[len] = 0; // needed for direct paste
  selection_length = len;
  if (OpenClipboard(fl_xid(Fl::first_window()))) {
    EmptyClipboard();
    SetClipboardData(CF_TEXT, NULL);
    CloseClipboard();
    if (!beenhere) {
      Fl::add_handler(selection_xevent_handler);
      beenhere = 1;
    }
  }
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
