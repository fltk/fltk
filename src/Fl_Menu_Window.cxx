// Fl_Menu_Window.H

// This is the window type used by Fl_Menu to make the pop-ups.
// It draws in the overlay planes if possible.

// Also here is the implementation of the mouse & keyboard grab,
// which are used so that clicks outside the program's windows
// can be used to dismiss the menus.

#include <config.h>
#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Menu_Window.H>

// WIN32 note: HAVE_OVERLAY is false
#if HAVE_OVERLAY
extern XVisualInfo *fl_find_overlay_visual();
extern XVisualInfo *fl_overlay_visual;
extern Colormap fl_overlay_colormap;
extern unsigned long fl_transparent_pixel;
static GC gc;	// the GC used by all X windows
extern uchar fl_overlay; // changes how fl_color(x) works
#endif

#include <stdio.h>

void Fl_Menu_Window::show() {
#if HAVE_OVERLAY
  if (!shown() && overlay() && fl_find_overlay_visual()) {
    XInstallColormap(fl_display, fl_overlay_colormap);
    fl_background_pixel = int(fl_transparent_pixel);
    Fl_X::make_xid(this, fl_overlay_visual, fl_overlay_colormap);
    fl_background_pixel = -1;
  } else
#endif
    Fl_Single_Window::show();
}

void Fl_Menu_Window::flush() {
#if HAVE_OVERLAY
  if (!fl_overlay_visual || !overlay()) {Fl_Single_Window::flush(); return;}
  Fl_X *i = Fl_X::i(this);
  fl_window = i->xid;
  if (!gc) gc = XCreateGC(fl_display, i->xid, 0, 0);
  fl_gc = gc;
  fl_overlay = 1;
  fl_clip_region(i->region); i->region = 0;
  draw();
  fl_overlay = 0;
#else
  Fl_Single_Window::flush();
#endif
}

void Fl_Menu_Window::erase() {
#if HAVE_OVERLAY
  if (!gc || !shown()) return;
//XSetForeground(fl_display, gc, 0);
//XFillRectangle(fl_display, fl_xid(this), gc, 0, 0, w(), h());
  XClearWindow(fl_display, fl_xid(this));
#endif
}

// Fix the colormap flashing on Maximum Impact Graphics by erasing the
// menu before unmapping it:
void Fl_Menu_Window::hide() {
  erase();
  Fl_Single_Window::hide();
}

Fl_Menu_Window::~Fl_Menu_Window() {
  hide();
}

////////////////////////////////////////////////////////////////
// "Grab" is done while menu systems are up.  This has several effects:
// Events are all sent to the "grab window", which does not even
// have to be displayed (and in the case of Fl_Menu.C it isn't).
// Under X override_redirect and save_under is done to new windows.
// The system is also told to "grab" events and send them to this app.

extern void fl_fix_focus();

#ifdef WIN32
// We have to keep track of whether we have captured the mouse, since
// MSWindows shows little respect for this... Grep for fl_capture to
// see where and how this is used.
HWND fl_capture;
#endif

void Fl::grab(Fl_Window& w) {
  grab_ = &w;
  fl_fix_focus();
#ifdef WIN32
  SetActiveWindow(fl_capture = fl_xid(first_window()));
  SetCapture(fl_capture);
#else
  XGrabPointer(fl_display,
	       fl_xid(first_window()),
	       1,
	       ButtonPressMask|ButtonReleaseMask|
	       ButtonMotionMask|PointerMotionMask,
	       GrabModeAsync,
	       GrabModeAsync, 
	       None,
	       0,
	       fl_event_time);
  XGrabKeyboard(fl_display,
		fl_xid(first_window()),
		1,
		GrabModeAsync,
		GrabModeAsync, 
		fl_event_time);
#endif
}

void Fl::release() {
  grab_ = 0;
  fl_fix_focus();
#ifdef WIN32
  fl_capture = 0;
  ReleaseCapture();
#else
  XUngrabKeyboard(fl_display, fl_event_time);
  XUngrabPointer(fl_display, fl_event_time);
  // this flush is done in case the picked menu item goes into
  // an infinite loop, so we don't leave the X server locked up:
  XFlush(fl_display);
#endif
  return;
}

// end of Fl_Menu_Window.C
