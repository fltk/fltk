// win32.H

// Do not directly include this file, instead use <FL/x.H>.  It will
// include this file if WIN32 is defined.  This is to encourage
// portability of even the system-specific code...

#include <windows.h>
#ifndef LPMINMAXINFO
// the gcc header files are missing some stuff:
#define LPMINMAXINFO MINMAXINFO*
#define VK_LWIN 0x5B
#define VK_RWIN 0x5C
#define VK_APPS 0x5D
#endif

// some random X equivalents
typedef HWND Window;
typedef POINT XPoint;
struct XRectangle {int x, y, width, height;};
typedef HRGN Region;
void fl_clip_region(Region);
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
  HDC private_dc; // used for OpenGL
  HCURSOR cursor;
  static Fl_X* first;
  static Fl_X* i(const Fl_Window* w) {return w->i;}
  void setwindow(Fl_Window* wi) {w=wi; wi->i=this;}
  void flush() {w->flush();}
  void set_minmax(LPMINMAXINFO minmax);
  void mapraise();
  static Fl_X* make(Fl_Window*);
};
extern HCURSOR fl_default_cursor;
inline Window fl_xid(const Fl_Window*w) {return Fl_X::i(w)->xid;}
Fl_Window* fl_find(Window xid);
extern char fl_override_redirect; // hack into Fl_Window::make_xid()
extern int fl_background_pixel;  // hack into Fl_Window::make_xid()

// most recent fl_color() or fl_rgbcolor() points at one of these:
extern struct Fl_XMap {
  COLORREF rgb;	// this should be the type the RGB() macro returns
  HPEN pen;	// pen, 0 if none created yet
  int brush;	// ref to solid brush, 0 if none created yet
} *fl_current_xmap;
inline COLORREF fl_RGB() {return fl_current_xmap->rgb;}
inline HPEN fl_pen() {return fl_current_xmap->pen;}
HBRUSH fl_brush(); // allocates a brush if necessary

extern HINSTANCE fl_display;
extern Window fl_window;
extern HDC fl_gc;
extern HPALETTE fl_palette; // non-zero only on 8-bit displays!
extern HDC fl_GetDC(Window);
extern MSG fl_msg;

// off-screen pixmaps: create, destroy, draw into, copy to window
#define Fl_Offscreen HBITMAP
#define fl_create_offscreen(w, h) CreateCompatibleBitmap(fl_gc, w, h)

extern HDC fl_makeDC(HBITMAP);
#define fl_begin_offscreen(b) \
  HDC _sgc=fl_gc; Window _sw=fl_window; fl_gc=fl_makeDC(b); fl_window=(HWND)b

#define fl_end_offscreen() \
  DeleteDC(fl_gc); fl_window=_sw; fl_gc = _sgc

void fl_copy_offscreen(int x,int y,int w,int h,HBITMAP pixmap,int srcx,int srcy);
#define fl_delete_offscreen(bitmap) DeleteObject(bitmap);
