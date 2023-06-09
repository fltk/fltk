//
// Drag & Drop code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

// Note: this file contains platform specific code and will therefore
// not be processed by doxygen (see Doxyfile.in).

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/platform.H>
#include "flstring.h"
#include "drivers/X11/Fl_X11_Screen_Driver.H"
#include "Fl_Window_Driver.H"

extern Atom fl_XdndAware;
extern Atom fl_XdndSelection;
extern Atom fl_XdndEnter;
extern Atom fl_XdndTypeList;
extern Atom fl_XdndPosition;
extern Atom fl_XdndLeave;
extern Atom fl_XdndDrop;
extern Atom fl_XdndStatus;
extern Atom fl_XdndActionCopy;
extern Atom fl_XdndFinished;
extern Atom fl_XdndURIList;
extern Atom fl_XaUtf8String;

extern char fl_i_own_selection[2];
extern char *fl_selection_buffer[2];

extern void fl_sendClientMessage(Window window, Atom message,
                                 unsigned long d0,
                                 unsigned long d1=0,
                                 unsigned long d2=0,
                                 unsigned long d3=0,
                                 unsigned long d4=0);

// return version # of Xdnd this window supports.  Also change the
// window to the proxy if it uses a proxy:
static int dnd_aware(Window& window) {
  Atom actual; int format; unsigned long count, remaining;
  unsigned char *data = 0;
  XGetWindowProperty(fl_display, window, fl_XdndAware,
                     0, 4, False, XA_ATOM,
                     &actual, &format,
                     &count, &remaining, &data);
  int ret = 0;
  if (actual == XA_ATOM && format==32 && count && data)
    ret = int(*(Atom*)data);
  if (data) { XFree(data); data = 0; }
  return ret;
}

static int grabfunc(int event) {
  if (event == FL_RELEASE) Fl::pushed(0);
  return 0;
}

extern int (*fl_local_grab)(int); // in Fl.cxx

// send an event to an fltk window belonging to this program:
static int local_handle(int event, Fl_Window* window) {
  fl_local_grab = 0;
  Fl::e_x = Fl::e_x_root-window->x();
  Fl::e_y = Fl::e_y_root-window->y();
  int ret = Fl::handle(event,window);
  fl_local_grab = grabfunc;
  return ret;
}

int Fl_X11_Screen_Driver::dnd(int unused) {
  Fl_Window *source_fl_win = Fl::first_window();
  Fl::first_window()->cursor(FL_CURSOR_MOVE);
  Window source_window = fl_xid(Fl::first_window());
  fl_local_grab = grabfunc;
  Window target_window = 0;
  Fl_Window* local_window = 0;
  int dndversion = 4; int dest_x, dest_y;
  XSetSelectionOwner(fl_display, fl_XdndSelection, source_window, fl_event_time);

  while (Fl::pushed()) {
    // figure out what window we are pointing at:
    Window new_window = 0; int new_version = 0;
    Fl_Window* new_local_window = 0;
    for (Window child = RootWindow(fl_display, fl_screen);;) {
      Window root; unsigned int junk3;
      XQueryPointer(fl_display, child, &root, &child,
                    &Fl::e_x_root, &Fl::e_y_root, &dest_x, &dest_y, &junk3);
      if (!child) {
        if (!new_window && (new_version = dnd_aware(root))) new_window = root;
        break;
      }
      new_window = child;
      if ((new_local_window = fl_find(child))) break;
      if ((new_version = dnd_aware(new_window))) break;
    }
#if USE_XFT
    if (new_local_window) {
      float s = Fl::screen_driver()->scale(Fl_Window_Driver::driver(new_local_window)->screen_num());
      Fl::e_x_root /= s;
      Fl::e_y_root /= s;
    }
#endif

    if (new_window != target_window) {
      if (local_window) {
        local_handle(FL_DND_LEAVE, local_window);
      } else if (dndversion) {
        fl_sendClientMessage(target_window, fl_XdndLeave, source_window);
      }
      dndversion = new_version;
      target_window = new_window;
      local_window = new_local_window;
      if (local_window) {
        local_handle(FL_DND_ENTER, local_window);
      } else if (dndversion) {
        // Send an X-DND message to the target window.  In order to
        // support dragging of files/URLs as well as arbitrary text,
        // we look at the selection buffer - if the buffer starts
        // with a common URI scheme, does not contain spaces, and
        // contains at least one CR LF, then we flag the data as
        // both a URI list (MIME media type "text/uri-list") and
        // plain text.  Otherwise, we just say it is plain text.
        if ((!strncmp(fl_selection_buffer[0], "file:///", 8) ||
             !strncmp(fl_selection_buffer[0], "ftp://", 6) ||
             !strncmp(fl_selection_buffer[0], "http://", 7) ||
             !strncmp(fl_selection_buffer[0], "https://", 8) ||
             !strncmp(fl_selection_buffer[0], "ipp://", 6) ||
             !strncmp(fl_selection_buffer[0], "ldap:", 5) ||
             !strncmp(fl_selection_buffer[0], "mailto:", 7) ||
             !strncmp(fl_selection_buffer[0], "news:", 5) ||
             !strncmp(fl_selection_buffer[0], "smb://", 6)) &&
            !strchr(fl_selection_buffer[0], ' ') &&
            strstr(fl_selection_buffer[0], "\r\n")) {
          // Send file/URI list...
          fl_sendClientMessage(target_window, fl_XdndEnter, source_window, dndversion<<24,
                               fl_XdndURIList, fl_XaUtf8String, XA_STRING);
        } else {
          // Send plain text...
          fl_sendClientMessage(target_window, fl_XdndEnter, source_window, dndversion<<24,
                               fl_XaUtf8String, XA_STRING, 0);
        }
      }
    }
    if (local_window) {
      local_handle(FL_DND_DRAG, local_window);
    } else if (dndversion) {
      int exroot = Fl::e_x_root, eyroot = Fl::e_y_root;
#if USE_XFT
      Fl_Window *target = fl_find(target_window);
      if (target) {
        float s = Fl::screen_driver()->scale(Fl_Window_Driver::driver(target)->screen_num());
        exroot *= s; eyroot *= s;
      }
#endif
      fl_sendClientMessage(target_window, fl_XdndPosition, source_window,
                           0, (exroot<<16)|eyroot, fl_event_time,
                           fl_XdndActionCopy);
    }
    Fl::wait();
  }

  if (local_window) {
    fl_i_own_selection[0] = 1;
    if (local_handle(FL_DND_RELEASE, local_window)) Fl::paste(*Fl::belowmouse(), 0);
  } else if (dndversion) {
    fl_sendClientMessage(target_window, fl_XdndDrop, source_window,
                         0, fl_event_time);
  } else if (target_window) {
    // fake a drop by clicking the middle mouse button:
    XButtonEvent msg;
    msg.type = ButtonPress;
    msg.window = target_window;
    msg.root = RootWindow(fl_display, fl_screen);
    msg.subwindow = 0;
    msg.time = fl_event_time+1;
    msg.x = dest_x;
    msg.y = dest_y;
    float s = 1;
#if USE_XFT
    Fl_Window *target = fl_find(target_window);
    if (target) s = Fl::screen_driver()->scale(Fl_Window_Driver::driver(target)->screen_num());
#endif
    msg.x_root = Fl::e_x_root * s;
    msg.y_root = Fl::e_y_root * s;
    msg.state = 0x0;
    msg.button = Button2;
    XSendEvent(fl_display, target_window, False, 0L, (XEvent*)&msg);
    msg.time++;
    msg.state = 0x200;
    msg.type = ButtonRelease;
    XSendEvent(fl_display, target_window, False, 0L, (XEvent*)&msg);
  }

  fl_local_grab = 0;
  Fl::handle(FL_RELEASE, source_fl_win);
  source_fl_win->cursor(FL_CURSOR_DEFAULT);
  return 1;
}
