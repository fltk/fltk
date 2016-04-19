//
// "$Id$"
//
// Mac header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
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

// Do not directly include this file, instead use <FL/x.H>.

// This file can be used as a template when porting FLTK to a new platform

#if !defined(Fl_X_H)
#  error "Never use <FL/porting.H> directly; include <FL/x.H> instead."
#endif // !Fl_X_H

typedef void *Window;

#if !(defined(FL_LIBRARY) || defined(FL_INTERNALS)) // this part is used when compiling an application program
#  include <FL/Fl_Widget.H>

typedef void *Fl_Region;
typedef void *Fl_Offscreen;

#else // this part must be compiled when building the FLTK libraries

typedef void *Fl_Region;
typedef void *Fl_Offscreen;

#  include "Fl_Window.H"

// Some random X equivalents
struct XPoint { int x, y; };
struct XRectangle {int x, y, width, height;};

inline void fl_open_callback(void (*)(const char *)) {}

// This object contains all platform-specific stuff about a window:
// WARNING: this object is highly subject to change!
class Fl_X {
public:
  Window xid;              // pointer to the native window object (FLWindow*)
  Fl_Offscreen other_xid;  // pointer for offscreen bitmaps (overlay window)
  Fl_Window *w;            // FLTK window for
  Fl_Region region;
  Fl_X *next;              // chain of mapped windows
  int wait_for_expose;
  static Fl_X* first;
  static Fl_X* i(const Fl_Window* w) {return w->i;}
  static int fake_X_wm(const Fl_Window*,int&,int&,int&,int&,int&);
  static Fl_X* make(Fl_Window*);
  void flush();
  static void set_default_icons(const Fl_RGB_Image*[], int);
  void set_icons();
  int set_cursor(Fl_Cursor);
  int set_cursor(const Fl_RGB_Image*, int, int);
};

extern Window fl_window;

#endif // FL_LIBRARY || FL_INTERNALS

typedef void *Fl_Bitmask;

extern void *fl_gc;

extern Window fl_xid(const Fl_Window*);
extern Fl_Window* fl_find(Window xid);
void fl_clip_region(Fl_Region);

extern Fl_Bitmask fl_create_bitmask(int w, int h, const uchar *data);
extern Fl_Bitmask fl_create_alphamask(int w, int h, int d, int ld, const uchar *data);
extern void fl_delete_bitmask(Fl_Bitmask bm);
extern Fl_Offscreen fl_create_offscreen(int w, int h);
extern void fl_copy_offscreen(int x,int y,int w,int h, Fl_Offscreen gWorld, int srcx,int srcy);
extern void fl_delete_offscreen(Fl_Offscreen gWorld);
extern void fl_begin_offscreen(Fl_Offscreen gWorld);
extern void fl_end_offscreen();

extern int fl_parse_color(const char* p, uchar& r, uchar& g, uchar& b);
extern void fl_open_display();


//
// End of "$Id$".
//

