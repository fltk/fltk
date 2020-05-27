//
// "$Id$"
//
// Mac header file for the Fast Light Tool Kit (FLTK).
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

// Do not directly include this file, instead use <FL/x.H>.  It will
// include this file if "__APPLE__" is defined.  This is to encourage
// portability of even the system-specific code...
#ifndef FL_DOXYGEN

#if !defined(Fl_X_H)
#  error "Never use <FL/mac.H> directly; include <FL/x.H> instead."
#endif // !Fl_X_H

#ifdef __OBJC__
@class FLWindow; // a subclass of the NSWindow Cocoa class
typedef FLWindow *Window;
#else
typedef class FLWindow *Window; // pointer to the FLWindow objective-c class
#endif // __OBJC__

#if !(defined(FL_LIBRARY) || defined(FL_INTERNALS)) // this part is used when compiling an application program
#  include <FL/Fl_Widget.H>

typedef struct flCocoaRegion* Fl_Region;
typedef struct CGContext* CGContextRef;
typedef struct OpaquePMPrintSettings*   PMPrintSettings;
typedef struct OpaquePMPageFormat*      PMPageFormat;
typedef struct OpaquePMPrintSession*    PMPrintSession;
typedef struct CGImage* CGImageRef;
typedef struct __CFData* CFMutableDataRef; // used in Fl_Copy_Surface.H
typedef CGContextRef Fl_Offscreen;

#else // this part must be compiled when building the FLTK libraries

// Standard MacOS C/C++ includes...
#include <ApplicationServices/ApplicationServices.h>
#undef check // because of Fl::check()

#ifndef MAC_OS_X_VERSION_10_4
#define MAC_OS_X_VERSION_10_4 1040
#endif
#ifndef MAC_OS_X_VERSION_10_5
#define MAC_OS_X_VERSION_10_5 1050
#endif
#ifndef MAC_OS_X_VERSION_10_6
#define MAC_OS_X_VERSION_10_6 1060
#endif
#ifndef MAC_OS_X_VERSION_10_7
#define MAC_OS_X_VERSION_10_7 1070
#endif
#ifndef MAC_OS_X_VERSION_10_8
#define MAC_OS_X_VERSION_10_8 1080
#endif
#ifndef MAC_OS_X_VERSION_10_9
#define MAC_OS_X_VERSION_10_9 1090
#endif
#ifndef MAC_OS_X_VERSION_10_10
#define MAC_OS_X_VERSION_10_10 101000
#endif
#ifndef MAC_OS_X_VERSION_10_11
#define MAC_OS_X_VERSION_10_11 101100
#endif
#ifndef MAC_OS_X_VERSION_10_12
#define MAC_OS_X_VERSION_10_12 101200
#endif
#ifndef MAC_OS_X_VERSION_10_13
#define MAC_OS_X_VERSION_10_13 101300
#endif
#ifndef MAC_OS_X_VERSION_10_14
#define MAC_OS_X_VERSION_10_14 101400
#endif

#ifndef NSINTEGER_DEFINED // appears with 10.5 in NSObjCRuntime.h
#if defined(__LP64__) && __LP64__
typedef long NSInteger;
typedef unsigned long NSUInteger;
#else
typedef int NSInteger;
typedef unsigned int NSUInteger;
#endif
#endif

#ifdef __OBJC__
@class NSCursor;
@class NSOpenGLPixelFormat;
@class NSOpenGLContext;
#else
class NSCursor;
class NSOpenGLPixelFormat;
class NSOpenGLContext;
#endif // __OBJC__

typedef CGContextRef Fl_Offscreen;
#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_4
typedef CGImageAlphaInfo CGBitmapInfo;
#endif

typedef struct flCocoaRegion {
  int count;
  CGRect *rects;
} *Fl_Region;  // a region is the union of a series of rectangles

#  include "Fl_Window.H"
#  include "../src/Fl_Font.H"

// Some random X equivalents
struct XPoint { int x, y; };
struct XRectangle {int x, y, width, height;};
#ifndef CGFLOAT_DEFINED //appears with 10.5 in CGBase.h
#if defined(__LP64__) && __LP64__
typedef double CGFloat;
#else
typedef float CGFloat;
#endif
#endif // CGFLOAT_DEFINED

extern CGRect fl_cgrectmake_cocoa(int x, int y, int w, int h);
inline Fl_Region XRectangleRegion(int x, int y, int w, int h) {
  Fl_Region R = (Fl_Region)malloc(sizeof(*R));
  R->count = 1;
  R->rects = (CGRect *)malloc(sizeof(CGRect));
  *(R->rects) = fl_cgrectmake_cocoa(x, y, w, h);
  return R;
}
inline void XDestroyRegion(Fl_Region r) {
  if(r) {
    free(r->rects);
    free(r);
  }
}
extern NSCursor *fl_default_cursor;

// This object contains all mac-specific stuff about a window:
// WARNING: this object is highly subject to change!
class Fl_X {
  
public:
  Window xid;              // pointer to the Cocoa window object (FLWindow*)
  Fl_Offscreen other_xid;  // pointer for offscreen bitmaps (overlay window)
  Fl_Window *w;            // FLTK window for 
  Fl_Region region;
#if FLTK_ABI_VERSION < 10304
  Fl_Region subRegion;     // for ABI compatibility, recycled to replace subRect_
#endif
  Fl_X *next;              // chain of mapped windows
#if FLTK_ABI_VERSION < 10304
  Fl_X *xidChildren;       // useless with true subwindows, recycled to replace mapped_to_retina_
  Fl_X *xidNext;           // useless with true subwindows
#endif
  int wait_for_expose;
  NSCursor *cursor;
  static Fl_X* first;
  static Fl_X* i(const Fl_Window* w) {return w->i;}
  static int fake_X_wm(const Fl_Window*,int&,int&,int&,int&,int&,int,int,int,int);
  static void make(Fl_Window*);
  void flush();
  static void set_high_resolution(bool);
#if FLTK_ABI_VERSION >= 10304
  CGRect* subRect() { return subRect_; } // getter
  void subRect(CGRect *r) { subRect_ = r; } // setter
#else
  CGRect* subRect() { return (CGRect*)subRegion; } // getter
  void subRect(CGRect *r) { subRegion = (Fl_Region)r; } // setter
#endif
  bool mapped_to_retina();      // is window mapped to retina display?
  void mapped_to_retina(bool);  // sets whether window is mapped to retina display
  bool changed_resolution();      // did window just moved to display with another resolution?
  void changed_resolution(bool);// sets whether window just moved to display with another resolution
  bool in_windowDidResize();      // is window performing windowDidResize?
  void in_windowDidResize(bool);  // sets whether window is performing windowDidResize
  // Quartz additions:
  CGContextRef gc;                 // graphics context (NULL when using QD)
  static void q_fill_context();    // fill a Quartz context with current FLTK state
  static void q_clear_clipping();  // remove all clipping from a Quartz context
  static void q_release_context(Fl_X *x=0); // free all resources associated with fl_gc
  static void q_begin_image(CGRect&, int x, int y, int w, int h);
  static void q_end_image();
  // Cocoa additions
  static NSOpenGLPixelFormat *mode_to_NSOpenGLPixelFormat(int mode, const int*); // computes NSOpenGLPixelFormat from Gl window's mode
  static NSOpenGLContext* create_GLcontext_for_window(NSOpenGLPixelFormat *pixelformat, NSOpenGLContext *shared_ctx, Fl_Window *window);
  static void GLcontext_update(NSOpenGLContext*);
  static void GLcontext_flushbuffer(NSOpenGLContext*);
  static void GLcontext_release(NSOpenGLContext*);
  static void GLcontext_makecurrent(NSOpenGLContext*);
  static void GL_cleardrawable(void);
  static void gl_start(NSOpenGLContext*);
  void destroy(void);
  void map(void);
  void unmap(void);
  void collapse(void);
  WindowRef window_ref(void); // useless with cocoa GL windows
  void set_key_window(void);
  // OS X doesn't have per window icons
  static void set_default_icons(const Fl_RGB_Image*[], int) {};
  void set_icons() {};
  int set_cursor(Fl_Cursor);
  int set_cursor(const Fl_RGB_Image*, int, int);
  static CGImageRef CGImage_from_window_rect(Fl_Window *win, int x, int y, int w, int h);
  static unsigned char *bitmap_from_window_rect(Fl_Window *win, int x, int y, int w, int h, int *bytesPerPixel);
  static Fl_Region intersect_region_and_rect(Fl_Region current, int x,int y,int w, int h);
  static void *get_carbon_function(const char *name);
  static void screen_work_area(int &X, int &Y, int &W, int &H, int n); // compute work area of a given screen
  static int next_marked_length; // next length of marked text after current marked text will have been replaced
  static int insertion_point_location(int *px, int *py, int *pheight); // computes window coordinates & height of insertion point
  static const int CoreText_threshold;  // Mac OS version from which the Core Text API is used to display text
  static Fl_Fontdesc* calc_fl_fonts(void); // computes the fl_fonts global variable
  static int dnd(int use_selection); // call Fl_X::dnd(1) to support text dragging
  static int calc_mac_os_version(void); // computes the fl_mac_os_version global variable
  static void clip_to_rounded_corners(CGContextRef gc, int w, int h);
  static void *get_titlebar_layer(Fl_Window *win);
  static void draw_layer_to_context(void *layer, CGContextRef ctxt, int w, int h);
private:
#if FLTK_ABI_VERSION >= 10304
  CGRect* subRect_;           // makes sure subwindow remains inside its parent window
  // stores 3 binary flags: whether window is mapped to retina display; whether resolution just changed;
  // whether window is OpenGL and is currently being resized.
  unsigned mapped_to_retina_;
#else
  bool subwindow;     // for ABI compatibility, useless with true subwindows
#endif
};

extern Window fl_window;

#endif // FL_LIBRARY || FL_INTERNALS

typedef CGImageRef Fl_Bitmask;

extern CGContextRef fl_gc;

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

#endif // FL_DOXYGEN
/** \file
 Mac OS X-specific symbols.
 */

/** \defgroup group_macosx Mac OS X-specific symbols
 Mac OS X-specific symbols declared in <FL/x.H> or <FL/gl.h>
 \sa \ref osissues_macos
 @{ */

/** @brief Register a function called for each file dropped onto an application icon.
 \e cb will be called with a single Unix-style file name and path.
 If multiple files were dropped, \e cb will be called multiple times.
 */
extern void fl_open_callback(void (*cb)(const char *));

/** 
 * \brief Attaches a callback to the "About myprog" item of the system application menu.
 *
 * \param cb   a callback that will be called by "About myprog" menu item
 *		   with NULL 1st argument.
 * \param user_data   a pointer transmitted as 2nd argument to the callback.
 * \param shortcut    optional shortcut to attach to the "About myprog" menu item (e.g., FL_META+'a')
 */
extern void fl_mac_set_about( Fl_Callback *cb, void *user_data, int shortcut = 0);

/** \brief The version number of the running Mac OS X (e.g., 100604 for 10.6.4)
 */
extern int fl_mac_os_version;

/** Determines whether cmd-Q or the "Quit xxx" item of application menu terminates the app or only the event loop.
 By default, fl_mac_quit_early = 1, and cmd-Q or "Quit xxx" terminate the app when all windows are closed
 without Fl::run() returning. If fl_mac_quit_early is set to 0, cmd-Q or "Quit xxx" terminate only the event loop
 when all windows are closed, and Fl::run() returns.
 \note This OS-specific variable will not be part of the API of FLTK 1.4.
 */
extern int fl_mac_quit_early;

/** The system menu bar.
 */
extern class Fl_Sys_Menu_Bar *fl_sys_menu_bar;

struct Fl_Menu_Item;

class Fl_Mac_App_Menu {
public:
  /** Localizable text for the "About xxx" application menu item */
  static const char *about;
  /** Localizable text for the "Print Front Window" application menu item.
   This menu item won't be displayed if Fl_Mac_App_Menu::print 
   is set to an empty string.
   */
  static const char *print;
  /** Localizable text for the "Services" application menu item */
  static const char *services;
  /** Localizable text for the "Hide xxx" application menu item */
  static const char *hide;
  /** Localizable text for the "Hide Others" application menu item */
  static const char *hide_others;
  /** Localizable text for the "Show All" application menu item */
  static const char *show;
  /** Localizable text for the "Quit xxx" application menu item */
  static const char *quit;
  static void custom_application_menu_items(const Fl_Menu_Item *m);
};

/** @} */

//
// End of "$Id$".
//

