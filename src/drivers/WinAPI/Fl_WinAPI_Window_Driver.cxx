//
// "$Id$"
//
// Definition of Apple Cocoa window driver.
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


#include "../../config_lib.h"
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Bitmap.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/x.H>
#include "Fl_WinAPI_Window_Driver.H"
#include "Fl_WinAPI_Screen_Driver.H"
#include <windows.h>

#if USE_COLORMAP
extern HPALETTE fl_select_palette(void); // in fl_color_win32.cxx
#endif


Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *w)
{
  return new Fl_WinAPI_Window_Driver(w);
}


Fl_WinAPI_Window_Driver::Fl_WinAPI_Window_Driver(Fl_Window *win)
: Fl_Window_Driver(win)
{
  icon_ = new icon_data;
  memset(icon_, 0, sizeof(icon_data));
}


Fl_WinAPI_Window_Driver::~Fl_WinAPI_Window_Driver()
{
  if (shape_data_) {
    delete shape_data_->todelete_;
    delete shape_data_;
  }
  delete icon_;
}


// --- private

RECT // frame of the decorated window in screen coordinates
  Fl_WinAPI_Window_Driver::border_width_title_bar_height(
                                                         int &bx, // left and right border width
                                                         int &by, // bottom border height (=bx)
                                                         int &bt, // height of window title bar
                                                         float *pscaling // display scaling factor
                                                         )
{
  Fl_Window *win = pWindow;
  RECT r = {0,0,0,0};
  bx = by = bt = 0;
  if (win->shown() && !win->parent() && win->border() && win->visible()) {
    static HMODULE dwmapi_dll = LoadLibrary("dwmapi.dll");
    typedef HRESULT (WINAPI* DwmGetWindowAttribute_type)(HWND hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute);
    static DwmGetWindowAttribute_type DwmGetWindowAttribute = dwmapi_dll ?
    (DwmGetWindowAttribute_type)GetProcAddress(dwmapi_dll, "DwmGetWindowAttribute") : NULL;
    int need_r = 1;
    float scaling = 1;
    if (DwmGetWindowAttribute) {
      const DWORD DWMWA_EXTENDED_FRAME_BOUNDS = 9;
      if ( DwmGetWindowAttribute(fl_xid(win), DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(RECT)) == S_OK ) {
        need_r = 0;
        scaling = Fl_WinAPI_Screen_Driver::desktop_scaling_factor();
      }
    }
    if (need_r) {
      GetWindowRect(fl_xid(win), &r);
    }
    if (pscaling) *pscaling = scaling;

    bx = (r.right - r.left - int(win->w() * scaling))/2;
    if (bx < 1) bx = 1;
    by = bx;
    bt = r.bottom - r.top - int(win->h() * scaling) - 2 * by;
  }
  return r;
}


// --- window data

int Fl_WinAPI_Window_Driver::decorated_w()
{
  int bt, bx, by;
  border_width_title_bar_height(bx, by, bt);
  return w() + 2 * bx;
}

int Fl_WinAPI_Window_Driver::decorated_h()
{
  int bt, bx, by;
  float scaling = 1;
  border_width_title_bar_height(bx, by, bt, &scaling);
  return h() + bt/scaling + 2 * by;
}


// --- window management



void Fl_WinAPI_Window_Driver::shape_bitmap_(Fl_Image* b) {
  shape_data_->shape_ = b;
}

void Fl_WinAPI_Window_Driver::shape_alpha_(Fl_Image* img, int offset) {
  int i, j, d = img->d(), w = img->w(), h = img->h(), bytesperrow = (w+7)/8;
  unsigned u;
  uchar byte, onebit;
  // build an Fl_Bitmap covering the non-fully transparent/black part of the image
  const uchar* bits = new uchar[h*bytesperrow]; // to store the bitmap
  const uchar* alpha = (const uchar*)*img->data() + offset; // points to alpha value of rgba pixels
  for (i = 0; i < h; i++) {
    uchar *p = (uchar*)bits + i * bytesperrow;
    byte = 0;
    onebit = 1;
    for (j = 0; j < w; j++) {
      if (d == 3) {
        u = *alpha;
        u += *(alpha+1);
        u += *(alpha+2);
      }
      else u = *alpha;
      if (u > 0) { // if the pixel is not fully transparent/black
        byte |= onebit; // turn on the corresponding bit of the bitmap
      }
      onebit = onebit << 1; // move the single set bit one position to the left
      if (onebit == 0 || j == w-1) {
        onebit = 1;
        *p++ = byte; // store in bitmap one pack of bits
        byte = 0;
      }
      alpha += d; // point to alpha value of next pixel
    }
  }
  Fl_Bitmap* bitmap = new Fl_Bitmap(bits, w, h);
  bitmap->alloc_array = 1;
  shape_bitmap_(bitmap);
  shape_data_->todelete_ = bitmap;
}

void Fl_WinAPI_Window_Driver::shape(const Fl_Image* img) {
  if (shape_data_) {
    if (shape_data_->todelete_) { delete shape_data_->todelete_; }
  }
  else {
    shape_data_ = new shape_data_type;
  }
  memset(shape_data_, 0, sizeof(shape_data_type));
  pWindow->border(false);
  int d = img->d();
  if (d && img->count() >= 2) shape_pixmap_((Fl_Image*)img);
  else if (d == 0) shape_bitmap_((Fl_Image*)img);
  else if (d == 2 || d == 4) shape_alpha_((Fl_Image*)img, d - 1);
  else if ((d == 1 || d == 3) && img->count() == 1) shape_alpha_((Fl_Image*)img, 0);
}


static inline BYTE bit(int x) { return (BYTE)(1 << (x%8)); }

static HRGN bitmap2region(Fl_Image* image) {
  HRGN hRgn = 0;
  /* Does this need to be dynamically determined, perhaps? */
  const int ALLOC_UNIT = 100;
  DWORD maxRects = ALLOC_UNIT;
  
  RGNDATA* pData = (RGNDATA*)malloc(sizeof(RGNDATAHEADER)+(sizeof(RECT)*maxRects));
  pData->rdh.dwSize = sizeof(RGNDATAHEADER);
  pData->rdh.iType = RDH_RECTANGLES;
  pData->rdh.nCount = pData->rdh.nRgnSize = 0;
  SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
  
  const int bytesPerLine = (image->w() + 7)/8;
  BYTE* p, *data = (BYTE*)*image->data();
  for (int y = 0; y < image->h(); y++) {
    // each row, left to right
    for (int x = 0; x < image->w(); x++) {
      int x0 = x;
      while (x < image->w()) {
        p = data + x / 8;
        if (!((*p) & bit(x))) break; // transparent pixel
        x++;
      }
      if (x > x0) {
        RECT *pr;
        /* Add the pixels (x0, y) to (x, y+1) as a new rectangle
         * in the region
         */
        if (pData->rdh.nCount >= maxRects) {
          maxRects += ALLOC_UNIT;
          pData = (RGNDATA*)realloc(pData, sizeof(RGNDATAHEADER)
                                    + (sizeof(RECT)*maxRects));
        }
        pr = (RECT*)&pData->Buffer;
        SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
        if (x0 < pData->rdh.rcBound.left)
          pData->rdh.rcBound.left = x0;
        if (y < pData->rdh.rcBound.top)
          pData->rdh.rcBound.top = y;
        if (x > pData->rdh.rcBound.right)
          pData->rdh.rcBound.right = x;
        if (y+1 > pData->rdh.rcBound.bottom)
          pData->rdh.rcBound.bottom = y+1;
        pData->rdh.nCount++;
        /* On Windows98, ExtCreateRegion() may fail if the
         * number of rectangles is too large (ie: >
         * 4000). Therefore, we have to create the region by
         * multiple steps.
         */
        if (pData->rdh.nCount == 2000) {
          HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER)
                                   + (sizeof(RECT)*maxRects), pData);
          if (hRgn) {
            CombineRgn(hRgn, hRgn, h, RGN_OR);
            DeleteObject(h);
          } else
            hRgn = h;
          pData->rdh.nCount = 0;
          SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
        }
      }
    }
    /* Go to next row */
    data += bytesPerLine;
  }
  /* Create or extend the region with the remaining rectangles*/
  HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER)
                           + (sizeof(RECT)*maxRects), pData);
  if (hRgn) {
    CombineRgn(hRgn, hRgn, h, RGN_OR);
    DeleteObject(h);
  } else hRgn = h;
  free(pData); // I've created the region so I can free this now, right?
  return hRgn;
}


void Fl_WinAPI_Window_Driver::draw_begin()
{
  if (shape_data_) {
    float s = Fl::screen_driver()->scale(screen_num());
    if ((shape_data_->lw_ != s*w() || shape_data_->lh_ != s*h()) && shape_data_->shape_) {
      // size of window has changed since last time
      shape_data_->lw_ = s*w();
      shape_data_->lh_ = s*h();
      Fl_Image* temp = shape_data_->shape_->copy(shape_data_->lw_, shape_data_->lh_);
      HRGN region = bitmap2region(temp);
      SetWindowRgn(fl_xid(pWindow), region, TRUE); // the system deletes the region when it's no longer needed
      delete temp;
    }
  }
}


void Fl_WinAPI_Window_Driver::flush_double()
{
  if (!shown()) return;
  pWindow->make_current(); // make sure fl_gc is non-zero
  Fl_X *i = Fl_X::i(pWindow);
  if (!i) return; // window not yet created

  if (!other_xid) {
    other_xid = fl_create_offscreen(w(), h());
    pWindow->clear_damage(FL_DAMAGE_ALL);
  }
  if (pWindow->damage() & ~FL_DAMAGE_EXPOSE) {
    fl_clip_region(i->region); i->region = 0;
    fl_begin_offscreen(other_xid);
    fl_graphics_driver->clip_region( 0 );
    draw();
    fl_end_offscreen();
  }

  int X,Y,W,H; fl_clip_box(0,0,w(),h(),X,Y,W,H);
  if (other_xid) fl_copy_offscreen(X, Y, W, H, other_xid, X, Y);
}


void Fl_WinAPI_Window_Driver::flush_overlay()
{
  Fl_Overlay_Window *oWindow = pWindow->as_overlay_window();

  if (!shown()) return;
  pWindow->make_current(); // make sure fl_gc is non-zero
  Fl_X *i = Fl_X::i(pWindow);
  if (!i) return; // window not yet created

  int eraseoverlay = (pWindow->damage()&FL_DAMAGE_OVERLAY);
  pWindow->clear_damage((uchar)(pWindow->damage()&~FL_DAMAGE_OVERLAY));

  if (!other_xid) {
    other_xid = fl_create_offscreen(w(), h());
    pWindow->clear_damage(FL_DAMAGE_ALL);
  }
  if (pWindow->damage() & ~FL_DAMAGE_EXPOSE) {
    fl_clip_region(i->region); i->region = 0;
    fl_begin_offscreen(other_xid);
    fl_graphics_driver->clip_region(0);
    draw();
    fl_end_offscreen();
  }

  if (eraseoverlay) fl_clip_region(0);
  int X, Y, W, H; fl_clip_box(0, 0, w(), h(), X, Y, W, H);
  if (other_xid) fl_copy_offscreen(X, Y, W, H, other_xid, X, Y);

  if (overlay() == oWindow) oWindow->draw_overlay();
}


void Fl_WinAPI_Window_Driver::icons(const Fl_RGB_Image *icons[], int count) {
  free_icons();
  
  if (count > 0) {
    icon_->icons = new Fl_RGB_Image*[count];
    icon_->count = count;
    // FIXME: Fl_RGB_Image lacks const modifiers on methods
    for (int i = 0;i < count;i++)
      icon_->icons[i] = (Fl_RGB_Image*)((Fl_RGB_Image*)icons[i])->copy();
  }
  
  if (Fl_X::i(pWindow))
    set_icons();
}

const void *Fl_WinAPI_Window_Driver::icon() const {
  return icon_->legacy_icon;
}

void Fl_WinAPI_Window_Driver::icon(const void * ic) {
  free_icons();
  icon_->legacy_icon = ic;
}

void Fl_WinAPI_Window_Driver::free_icons() {
  int i;
  icon_->legacy_icon = 0L;
  if (icon_->icons) {
    for (i = 0;i < icon_->count;i++)
      delete icon_->icons[i];
    delete [] icon_->icons;
    icon_->icons = 0L;
  }
  icon_->count = 0;
  if (icon_->big_icon)
    DestroyIcon(icon_->big_icon);
  if (icon_->small_icon)
    DestroyIcon(icon_->small_icon);
  icon_->big_icon = NULL;
  icon_->small_icon = NULL;
}


void Fl_WinAPI_Window_Driver::make_current() {
  fl_GetDC(fl_xid(pWindow));
  
#if USE_COLORMAP
  // Windows maintains a hardware and software color palette; the
  // SelectPalette() call updates the current soft->hard mapping
  // for all drawing calls, so we must select it here before any
  // code does any drawing...
  fl_select_palette();
#endif // USE_COLORMAP
  
  fl_graphics_driver->clip_region(0);
fl_graphics_driver->scale(Fl::screen_driver()->scale(0));
}

void Fl_WinAPI_Window_Driver::label(const char *name,const char *iname) {
  if (shown() && !parent()) {
    if (!name) name = "";
    size_t l = strlen(name);
    //  WCHAR *lab = (WCHAR*) malloc((l + 1) * sizeof(short));
    //  l = fl_utf2unicode((unsigned char*)name, l, (wchar_t*)lab);
    unsigned wlen = fl_utf8toUtf16(name, (unsigned) l, NULL, 0); // Pass NULL to query length
    wlen++;
    unsigned short * lab = (unsigned short*)malloc(sizeof(unsigned short)*wlen);
    wlen = fl_utf8toUtf16(name, (unsigned) l, lab, wlen);
    lab[wlen] = 0;
    SetWindowTextW(fl_xid(pWindow), (WCHAR *)lab);
    free(lab);
  }
}


extern void fl_clipboard_notify_retarget(HWND wnd);
extern void fl_update_clipboard(void);

void Fl_WinAPI_Window_Driver::hide() {
  Fl_X* ip = Fl_X::i(pWindow);
  // STR#3079: if there remains a window and a non-modal window, and the window is deleted,
  // the app remains running without any apparent window.
  // Bug mechanism: hiding an owner window unmaps the owned (non-modal) window(s)
  // but does not delete it(them) in FLTK.
  // Fix for it:
  // when hiding a window, build list of windows it owns, and do hide/show on them.
  int count = 0;
  Fl_Window *win, **doit = NULL;
  for (win = Fl::first_window(); win && ip; win = Fl::next_window(win)) {
    if (win->non_modal() && GetWindow(fl_xid(win), GW_OWNER) == ip->xid) {
      count++;
    }
  }
  if (count) {
    doit = new Fl_Window*[count];
    count = 0;
    for (win = Fl::first_window(); win && ip; win = Fl::next_window(win)) {
      if (win->non_modal() && GetWindow(fl_xid(win), GW_OWNER) == ip->xid) {
        doit[count++] = win;
      }
    }
  }

  if (hide_common()) return;
  
  // make sure any custom icons get freed
  icons(NULL, 0);
  // this little trick keeps the current clipboard alive, even if we are about
  // to destroy the window that owns the selection.
  if (GetClipboardOwner()==ip->xid)
    fl_update_clipboard();
  // Make sure we unlink this window from the clipboard chain
  fl_clipboard_notify_retarget(ip->xid);
  // Send a message to myself so that I'll get out of the event loop...
  PostMessage(ip->xid, WM_APP, 0, 0);
  if (private_dc) fl_release_dc(ip->xid, private_dc);
  if (ip->xid == fl_window && fl_graphics_driver->gc()) {
    fl_release_dc(fl_window, (HDC)fl_graphics_driver->gc());
    fl_window = (HWND)-1;
    fl_graphics_driver->gc(0);
# ifdef FLTK_USE_CAIRO
    if (Fl::cairo_autolink_context()) Fl::cairo_make_current((Fl_Window*) 0);
# endif
  }
  
  if (ip->region) Fl_Graphics_Driver::default_driver().XDestroyRegion(ip->region);
  
  // this little trickery seems to avoid the popup window stacking problem
  HWND p = GetForegroundWindow();
  if (p==GetParent(ip->xid)) {
    ShowWindow(ip->xid, SW_HIDE);
    ShowWindow(p, SW_SHOWNA);
  }
  DestroyWindow(ip->xid);
  // end of fix for STR#3079
  if (count) {
    int ii;
    for (ii = 0; ii < count; ii++)  doit[ii]->hide();
    for (ii = 0; ii < count; ii++)  {
      if (ii != 0) doit[0]->show(); // Fix for STR#3165
      doit[ii]->show();
    }
    delete[] doit;
  }
  // Try to stop the annoying "raise another program" behavior
  if (pWindow->non_modal() && Fl::first_window() && Fl::first_window()->shown())
    Fl::first_window()->show();
  delete ip;
}


void Fl_WinAPI_Window_Driver::map() {
  ShowWindow(fl_xid(pWindow), SW_RESTORE); // extra map calls are harmless
}


void Fl_WinAPI_Window_Driver::unmap() {
  ShowWindow(fl_xid(pWindow), SW_HIDE);
}

#if !defined(FL_DOXYGEN) // FIXME - silence Doxygen warning

void Fl_WinAPI_Window_Driver::make_fullscreen(int X, int Y, int W, int H) {
  Fl_Window *w = pWindow;
  int top, bottom, left, right;
  int sx, sy, sw, sh;
  
  top = fullscreen_screen_top();
  bottom = fullscreen_screen_bottom();
  left = fullscreen_screen_left();
  right = fullscreen_screen_right();
  
  if ((top < 0) || (bottom < 0) || (left < 0) || (right < 0)) {
    top = Fl::screen_num(X, Y, W, H);
    bottom = top;
    left = top;
    right = top;
  }
  
  Fl::screen_xywh(sx, sy, sw, sh, top);
  Y = sy;
  Fl::screen_xywh(sx, sy, sw, sh, bottom);
  H = sy + sh - Y;
  Fl::screen_xywh(sx, sy, sw, sh, left);
  X = sx;
  Fl::screen_xywh(sx, sy, sw, sh, right);
  W = sx + sw - X;
  
  DWORD flags = GetWindowLong(fl_xid(w), GWL_STYLE);
  flags = flags & ~(WS_THICKFRAME|WS_CAPTION);
  SetWindowLong(fl_xid(w), GWL_STYLE, flags);
  
  // SWP_NOSENDCHANGING is so that we can override size limits
  SetWindowPos(fl_xid(w), HWND_TOP, X, Y, W, H, SWP_NOSENDCHANGING | SWP_FRAMECHANGED);
}

#endif // !defined(FL_DOXYGEN) // FIXME - silence Doxygen warning


void Fl_WinAPI_Window_Driver::fullscreen_on() {
  pWindow->_set_fullscreen();
  make_fullscreen(x(), y(), w(), h());
  Fl::handle(FL_FULLSCREEN, pWindow);
}


void Fl_WinAPI_Window_Driver::fullscreen_off(int X, int Y, int W, int H) {
  pWindow->_clear_fullscreen();
  DWORD style = GetWindowLong(fl_xid(pWindow), GWL_STYLE);
  // Remove the xid temporarily so that Fl_WinAPI_Window_Driver::fake_X_wm() behaves like it
  // does in Fl_WinAPI_Window_Driver::makeWindow().
  HWND xid = fl_xid(pWindow);
  Fl_X::i(pWindow)->xid = NULL;
  int wx, wy, bt, bx, by;
  switch (fake_X_wm(wx, wy, bt, bx, by)) {
    case 0:
      break;
    case 1:
      style |= WS_CAPTION;
      break;
    case 2:
      if (border()) {
        style |= WS_THICKFRAME | WS_CAPTION;
      }
      break;
  }
  Fl_X::i(pWindow)->xid = xid;
  // Adjust for decorations (but not if that puts the decorations
  // outside the screen)
  if ((X != x()) || (Y != y())) {
    X -= bx;
    Y -= by+bt;
  }
  W += bx*2;
  H += by*2+bt;
  SetWindowLong(fl_xid(pWindow), GWL_STYLE, style);
  SetWindowPos(fl_xid(pWindow), 0, X, Y, W, H,
               SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
  Fl::handle(FL_FULLSCREEN, pWindow);
}


void Fl_WinAPI_Window_Driver::iconize() {
  ShowWindow(fl_xid(pWindow), SW_SHOWMINNOACTIVE);
}


void Fl_WinAPI_Window_Driver::decoration_sizes(int *top, int *left,  int *right, int *bottom) {
  if (size_range_set() && (maxw() != minw() || maxh() != minh())) {
    *left = *right = GetSystemMetrics(SM_CXSIZEFRAME);
    *top = *bottom = GetSystemMetrics(SM_CYSIZEFRAME);
  } else {
    *left = *right = GetSystemMetrics(SM_CXFIXEDFRAME);
    *top = *bottom = GetSystemMetrics(SM_CYFIXEDFRAME);
  }
  *top += GetSystemMetrics(SM_CYCAPTION);
}

int Fl_WinAPI_Window_Driver::scroll(int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y,
                   void (*draw_area)(void*, int,int,int,int), void* data)
{
  typedef int (WINAPI* fl_GetRandomRgn_func)(HDC, HRGN, INT);
  static fl_GetRandomRgn_func fl_GetRandomRgn = 0L;
  static char first_time = 1;
  // We will have to do some Region magic now, so let's see if the
  // required function is available (and it should be staring w/Win95)
  if (first_time) {
    HMODULE hMod = GetModuleHandle("GDI32.DLL");
    if (hMod) {
      fl_GetRandomRgn = (fl_GetRandomRgn_func)GetProcAddress(hMod, "GetRandomRgn");
    }
    first_time = 0;
  }
  // Now check if the source scrolling area is fully visible.
  // If it is, we will do a quick scroll and just update the
  // newly exposed area. If it is not, we go the safe route and
  // re-render the full area instead.
  // Note 1: we could go and find the areas that are actually
  // obscured and recursively call fl_scroll for the newly found
  // rectangles. However, this practice would rely on the
  // elements of the undocumented Rgn structure.
  // Note 2: although this method should take care of most
  // multi-screen solutions, it will not solve issues scrolling
  // from a different resolution screen onto another.
  // Note 3: this has been tested with image maps, too.
  HDC gc = (HDC)fl_graphics_driver->gc();
  if (fl_GetRandomRgn) {
    // get the DC region minus all overlapping windows
    HRGN sys_rgn = CreateRectRgn(0, 0, 0, 0);
    fl_GetRandomRgn(gc, sys_rgn, 4);
    // now get the source scrolling rectangle
    HRGN src_rgn = CreateRectRgn(src_x, src_y, src_x+src_w, src_y+src_h);
    POINT offset = { 0, 0 };
    if (GetDCOrgEx(gc, &offset)) {
      OffsetRgn(src_rgn, offset.x, offset.y);
    }
    // see if all source pixels are available in the system region
    // Note: we could be a bit more merciful and subtract the
    // scroll destination region as well.
    HRGN dst_rgn = CreateRectRgn(0, 0, 0, 0);
    int r = CombineRgn(dst_rgn, src_rgn, sys_rgn, RGN_DIFF);
    DeleteObject(dst_rgn);
    DeleteObject(src_rgn);
    DeleteObject(sys_rgn);
    if (r != NULLREGION) {
      return 1;
    }
  }
  // Great, we can do an accelerated scroll instead of re-rendering
  BitBlt(gc, dest_x, dest_y, src_w, src_h, gc, src_x, src_y,SRCCOPY);
  return 0;
}

//
// End of "$Id$".
//
