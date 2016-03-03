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


#if !(defined(FL_LIBRARY) || defined(FL_INTERNALS)) // this part is used when compiling an application program
#  include <FL/Fl_Widget.H>

typedef void *Fl_Region;
typedef void *Fl_Offscreen;

#else // this part must be compiled when building the FLTK libraries

typedef void *Fl_Region;
typedef void *Fl_Offscreen;

#  include "Fl_Window.H"
#  include "../src/Fl_Font.H"

// Some random X equivalents
struct XPoint { int x, y; };
struct XRectangle {int x, y, width, height;};

typedef void *Window;

//typedef float CGFloat;

inline Fl_Region XRectangleRegion(int x, int y, int w, int h) {
  // write code here
  return 0L;
}

inline void XDestroyRegion(Fl_Region r) {
  // write code here
}

extern void *fl_default_cursor;

// This object contains all platform-specific stuff about a window:
// WARNING: this object is highly subject to change!
class Fl_X {
  
public:
  Window xid;              // pointer to the native window object (FLWindow*)
  Fl_Offscreen other_xid;  // pointer for offscreen bitmaps (overlay window)
  Fl_Window *w;            // FLTK window for
  Fl_Region region;
//#if FLTK_ABI_VERSION < 10304
//  Fl_Region subRegion;     // for ABI compatibility, recycled to replace subRect_
//#endif
  Fl_X *next;              // chain of mapped windows
//#if FLTK_ABI_VERSION < 10304
//  Fl_X *xidChildren;       // useless with true subwindows, recycled to replace mapped_to_retina_
//  Fl_X *xidNext;           // useless with true subwindows
//#endif
  int wait_for_expose;
//  NSCursor *cursor;
  static Fl_X* first;
  static Fl_X* i(const Fl_Window* w) {return (Fl_X*)w->i;}
  static int fake_X_wm(const Fl_Window*,int&,int&,int&,int&,int&);
  static Fl_X* make(Fl_Window*);
  void flush();
//#if FLTK_ABI_VERSION >= 10304
//  CGRect* subRect() { return subRect_; } // getter
//  void subRect(CGRect *r) { subRect_ = r; } // setter
//#else
//  CGRect* subRect() { return (CGRect*)subRegion; } // getter
//  void subRect(CGRect *r) { subRegion = (Fl_Region)r; } // setter
//#endif
//  bool mapped_to_retina();      // is window mapped to retina display?
//  void mapped_to_retina(bool);  // sets whether window is mapped to retina display
//  bool changed_resolution();      // did window just moved to display with another resolution?
//  void changed_resolution(bool);// sets whether window just moved to display with another resolution
//  bool in_windowDidResize();      // is window performing windowDidResize?
//  void in_windowDidResize(bool);  // sets whether window is performing windowDidResize
//  // Quartz additions:
//  CGContextRef gc;                 // graphics context (NULL when using QD)
//  static void q_release_context(Fl_X *x=0); // free all resources associated with fl_gc
//  // Cocoa additions
//  static NSOpenGLPixelFormat *mode_to_NSOpenGLPixelFormat(int mode, const int*); // computes NSOpenGLPixelFormat from Gl window's mode
//  static NSOpenGLContext* create_GLcontext_for_window(NSOpenGLPixelFormat *pixelformat, NSOpenGLContext *shared_ctx, Fl_Window *window);
//  static void GLcontext_update(NSOpenGLContext*);
//  static void GLcontext_flushbuffer(NSOpenGLContext*);
//  static void GLcontext_release(NSOpenGLContext*);
//  static void GLcontext_makecurrent(NSOpenGLContext*);
//  static void GL_cleardrawable(void);
//  void destroy(void);
//  void map(void);
//  void unmap(void);
//  void collapse(void);
//  WindowRef window_ref(void); // useless with cocoa GL windows
//  void set_key_window(void);
//  // OS X doesn't have per window icons
  static void set_default_icons(const Fl_RGB_Image*[], int);
  void set_icons();
  int set_cursor(Fl_Cursor);
  int set_cursor(const Fl_RGB_Image*, int, int);
//  static CGImageRef CGImage_from_window_rect(Fl_Window *win, int x, int y, int w, int h);
//  static unsigned char *bitmap_from_window_rect(Fl_Window *win, int x, int y, int w, int h, int *bytesPerPixel);
//  static Fl_Region intersect_region_and_rect(Fl_Region current, int x,int y,int w, int h);
//  static void *get_carbon_function(const char *name);
//  static void screen_work_area(int &X, int &Y, int &W, int &H, int n); // compute work area of a given screen
//  static int next_marked_length; // next length of marked text after current marked text will have been replaced
//  static int insertion_point_location(int *px, int *py, int *pheight); // computes window coordinates & height of insertion point
//  static const int CoreText_threshold;  // Mac OS version from which the Core Text API is used to display text
//  static Fl_Fontdesc* calc_fl_fonts(void); // computes the fl_fonts global variable
//  static int dnd(int use_selection); // call Fl_X::dnd(1) to support text dragging
//  static int calc_mac_os_version(void); // computes the fl_mac_os_version global variable
//  static void clip_to_rounded_corners(CGContextRef gc, int w, int h);
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

