//
// "$Id: win32.H,v 1.15.2.3 2001/01/22 15:13:38 easysw Exp $"
//
// WIN32 header file for the Fast Light Tool Kit (FLTK).
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

// Do not directly include this file, instead use <FL/x.H>.  It will
// include this file if WIN32 is defined.  This is to encourage
// portability of even the system-specific code...

#include <windows.h>
// In some of the distributions, the gcc header files are missing some stuff:
#ifndef LPMINMAXINFO
#define LPMINMAXINFO MINMAXINFO*
#endif
#ifndef VK_LWIN
#define VK_LWIN 0x5B
#define VK_RWIN 0x5C
#define VK_APPS 0x5D
#endif

// some random X equivalents
typedef HWND Window;
typedef POINT XPoint;
struct XRectangle {int x, y, width, height;};
typedef HRGN Region;
FL_EXPORT void fl_clip_region(Region);
inline Region XRectangleRegion(int x, int y, int w, int h) {
    return CreateRectRgn(x,y,x+w,y+h);
}
inline void XDestroyRegion(Region r) {DeleteObject(r);}
inline void XClipBox(Region r,XRectangle* rect) {
    RECT win_rect; GetRgnBox(r,&win_rect);
    rect->x=win_rect.left;
    rect->y=win_rect.top;
    rect->width=win_rect.right-win_rect.left;
    rect->height=win_rect.bottom-win_rect.top;
}
#define XDestroyWindow(a,b) DestroyWindow(b)
#define XMapWindow(a,b) ShowWindow(b, SW_RESTORE)
#define XUnmapWindow(a,b) ShowWindow(b, SW_HIDE)

#include "Fl_Window.H"
// this object contains all win32-specific stuff about a window:
// Warning: this object is highly subject to change!
class Fl_X {
public:
  Window xid;
  HBITMAP other_xid; // for double-buffered windows
  Fl_Window* w;
  Region region;
  Fl_X *next;
  int wait_for_expose;
  HDC private_dc; // used for OpenGL
  HCURSOR cursor;
  static FL_EXPORT Fl_X* first;
  static FL_EXPORT Fl_X* i(const Fl_Window* w) {return w->i;}
  static FL_EXPORT int fake_X_wm(const Fl_Window* w,int &X, int &Y,
		                 int &bt,int &bx,int &by);
  FL_EXPORT void setwindow(Fl_Window* wi) {w=wi; wi->i=this;}
  FL_EXPORT void flush() {w->flush();}
  FL_EXPORT void set_minmax(LPMINMAXINFO minmax);
  FL_EXPORT void mapraise();
  static FL_EXPORT Fl_X* make(Fl_Window*);
};
extern FL_EXPORT HCURSOR fl_default_cursor;
inline Window fl_xid(const Fl_Window*w) {return Fl_X::i(w)->xid;}
FL_EXPORT Fl_Window* fl_find(Window xid);
extern FL_EXPORT char fl_override_redirect; // hack into Fl_Window::make_xid()
extern FL_EXPORT int fl_background_pixel;  // hack into Fl_Window::make_xid()

// most recent fl_color() or fl_rgbcolor() points at one of these:
extern FL_EXPORT struct Fl_XMap {
  COLORREF rgb;	// this should be the type the RGB() macro returns
  HPEN pen;	// pen, 0 if none created yet
  int brush;	// ref to solid brush, 0 if none created yet
} *fl_current_xmap;
inline COLORREF fl_RGB() {return fl_current_xmap->rgb;}
inline HPEN fl_pen() {return fl_current_xmap->pen;}
FL_EXPORT HBRUSH fl_brush(); // allocates a brush if necessary

extern FL_EXPORT HINSTANCE fl_display;
extern FL_EXPORT Window fl_window;
extern FL_EXPORT HDC fl_gc;
extern FL_EXPORT HPALETTE fl_palette; // non-zero only on 8-bit displays!
extern FL_EXPORT HDC fl_GetDC(Window);
extern FL_EXPORT MSG fl_msg;

// off-screen pixmaps: create, destroy, draw into, copy to window
#define Fl_Offscreen HBITMAP
#define fl_create_offscreen(w, h) CreateCompatibleBitmap(fl_gc, w, h)

extern FL_EXPORT HDC fl_makeDC(HBITMAP);
#define fl_begin_offscreen(b) \
  HDC _sgc=fl_gc; Window _sw=fl_window; \
  fl_gc=fl_makeDC(b); fl_window=(HWND)b; fl_push_no_clip()

#define fl_end_offscreen() \
  fl_pop_clip(); DeleteDC(fl_gc); fl_window=_sw; fl_gc = _sgc

FL_EXPORT void fl_copy_offscreen(int x,int y,int w,int h,HBITMAP pixmap,int srcx,int srcy);
#define fl_delete_offscreen(bitmap) DeleteObject(bitmap);

//
// End of "$Id: win32.H,v 1.15.2.3 2001/01/22 15:13:38 easysw Exp $".
//
