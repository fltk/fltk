//
// Windows screen interface for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
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


#include <config.h>
#include "Fl_WinAPI_Screen_Driver.H"
#include "../GDI/Fl_Font.H"
#include <FL/Fl.H>
#include <FL/platform.H>
#include "../GDI/Fl_GDI_Graphics_Driver.H"
#include <FL/Fl_RGB_Image.H>
#include <FL/fl_ask.H>
#include <stdio.h>


// these are set by Fl::args() and override any system colors: from Fl_get_system_colors.cxx
extern const char *fl_fg;
extern const char *fl_bg;
extern const char *fl_bg2;
// end of extern additions workaround


#if !defined(HMONITOR_DECLARED) && (_WIN32_WINNT < 0x0500)
#  define COMPILE_MULTIMON_STUBS
#  include <multimon.h>
#endif // !HMONITOR_DECLARED && _WIN32_WINNT < 0x0500

static Fl_Text_Editor::Key_Binding extra_bindings[] =  {
  // Define Windows specific accelerators...
  { 'y',          FL_CTRL,                  Fl_Text_Editor::kf_redo       ,0},
  { 0,            0,                        0                             ,0}
};


Fl_WinAPI_Screen_Driver::Fl_WinAPI_Screen_Driver() : Fl_Screen_Driver() {
  text_editor_extra_key_bindings =  extra_bindings;
  for (int i = 0; i < MAX_SCREENS; i++) scale_of_screen[i] = 1;
  scaling_capability = SYSTEMWIDE_APP_SCALING;
}

int Fl_WinAPI_Screen_Driver::visual(int flags)
{
  fl_GetDC(0);
  if (flags & FL_DOUBLE) return 0;
  HDC gc = (HDC)Fl_Graphics_Driver::default_driver().gc();
  if (!(flags & FL_INDEX) &&
      GetDeviceCaps(gc,BITSPIXEL) <= 8) return 0;
  if ((flags & FL_RGB8) && GetDeviceCaps(gc,BITSPIXEL)<24) return 0;
  return 1;
}


// We go the much more difficult route of individually picking some multi-screen
// functions from the USER32.DLL . If these functions are not available, we
// will gracefully fall back to single monitor support.
//
// If we were to insist on the existence of "EnumDisplayMonitors" and
// "GetMonitorInfoA", it would be impossible to use FLTK on Windows 2000
// before SP2 or earlier.

// BOOL EnumDisplayMonitors(HDC, LPCRECT, MONITORENUMPROC, LPARAM)
typedef BOOL(WINAPI* fl_edm_func)(HDC, LPCRECT, MONITORENUMPROC, LPARAM);
// BOOL GetMonitorInfo(HMONITOR, LPMONITORINFO)
typedef BOOL(WINAPI* fl_gmi_func)(HMONITOR, LPMONITORINFO);

static fl_gmi_func fl_gmi = NULL; // used to get a proc pointer for GetMonitorInfoA


BOOL Fl_WinAPI_Screen_Driver::screen_cb(HMONITOR mon, HDC hdc, LPRECT r, LPARAM d)
{
  Fl_WinAPI_Screen_Driver *drv = (Fl_WinAPI_Screen_Driver*)d;
  return drv->screen_cb(mon, hdc, r);
}


BOOL Fl_WinAPI_Screen_Driver::screen_cb(HMONITOR mon, HDC, LPRECT r)
{
  if (num_screens >= MAX_SCREENS) return TRUE;

  MONITORINFOEX mi;
  mi.cbSize = sizeof(mi);

  //  GetMonitorInfo(mon, &mi);
  //  (but we use our self-acquired function pointer instead)
  if (fl_gmi(mon, &mi)) {
    screens[num_screens] = mi.rcMonitor;
    // If we also want to record the work area, we would also store mi.rcWork at this point
    work_area[num_screens] = mi.rcWork;
    num_screens++;
  }
  return TRUE;
}


void Fl_WinAPI_Screen_Driver::init()
{
  open_display();
  // Since not all versions of Windows include multiple monitor support,
  // we do a run-time check for the required functions...
  HMODULE hMod = GetModuleHandle("USER32.DLL");

  if (hMod) {
    // check that EnumDisplayMonitors is available
    fl_edm_func fl_edm = (fl_edm_func)GetProcAddress(hMod, "EnumDisplayMonitors");

    if (fl_edm) {
      // we have EnumDisplayMonitors - do we also have GetMonitorInfoA ?
      fl_gmi = (fl_gmi_func)GetProcAddress(hMod, "GetMonitorInfoA");
      if (fl_gmi) {
        // We have GetMonitorInfoA, enumerate all the screens...
        //      EnumDisplayMonitors(0,0,screen_cb,0);
        //      (but we use our self-acquired function pointer instead)
        //      NOTE: num_screens is incremented in screen_cb so we must first reset it here...
        num_screens = 0;
        fl_edm(0, 0, screen_cb, (LPARAM)this);
        return;
      }
    }
  }

  // If we get here, assume we have 1 monitor...
  num_screens = 1;
  screens[0].top = 0;
  screens[0].left = 0;
  screens[0].right = GetSystemMetrics(SM_CXSCREEN);
  screens[0].bottom = GetSystemMetrics(SM_CYSCREEN);
  work_area[0] = screens[0];
}


void Fl_WinAPI_Screen_Driver::screen_work_area(int &X, int &Y, int &W, int &H, int n)
{
  if (num_screens < 0) init();
  if (n < 0 || n >= num_screens) n = 0;
  X = int(work_area[n].left/scale_of_screen[n]);
  Y = int(work_area[n].top/scale_of_screen[n]);
  W = int((work_area[n].right - work_area[n].left)/scale_of_screen[n]);
  H = int((work_area[n].bottom - work_area[n].top)/scale_of_screen[n]);
}


void Fl_WinAPI_Screen_Driver::screen_xywh(int &X, int &Y, int &W, int &H, int n)
{
  if (num_screens < 0) init();

  if ((n < 0) || (n >= num_screens))
    n = 0;

  if (num_screens > 0) {
    X = int(screens[n].left/scale_of_screen[n]);
    Y = int(screens[n].top/scale_of_screen[n]);
    W = int((screens[n].right - screens[n].left)/scale_of_screen[n]);
    H = int((screens[n].bottom - screens[n].top)/scale_of_screen[n]);
  } else {
    /* Fallback if something is broken... */
    X = 0;
    Y = 0;
    W = GetSystemMetrics(SM_CXSCREEN);
    H = GetSystemMetrics(SM_CYSCREEN);
  }
}


void Fl_WinAPI_Screen_Driver::screen_xywh_unscaled(int &X, int &Y, int &W, int &H, int n) {
  if (num_screens < 0) init();
  if ((n < 0) || (n >= num_screens)) n = 0;
  X = screens[n].left;
  Y = screens[n].top;
  W = screens[n].right - screens[n].left;
  H = screens[n].bottom - screens[n].top;
};


void Fl_WinAPI_Screen_Driver::screen_dpi(float &h, float &v, int n)
{
  if (num_screens < 0) init();
  h = v = 0.0f;
  if (n >= 0 && n < num_screens) {
    h = float(dpi[n][0]);
    v = float(dpi[n][1]);
  }
}


int Fl_WinAPI_Screen_Driver::x()
{
  /*RECT r;

  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.left;*/
  int X, Y, W, H;
  screen_work_area(X, Y, W, H, 0);
  return X;
}


int Fl_WinAPI_Screen_Driver::y()
{
  /*RECT r;

  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.top;*/
  int X, Y, W, H;
  screen_work_area(X, Y, W, H, 0);
  return Y;
}


int Fl_WinAPI_Screen_Driver::h()
{
  /*RECT r;

  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.bottom - r.top;*/
  int X, Y, W, H;
  screen_work_area(X, Y, W, H, 0);
  return H;
}


int Fl_WinAPI_Screen_Driver::w()
{
  /*RECT r;

  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.right - r.left;*/
  int X, Y, W, H;
  screen_work_area(X, Y, W, H, 0);
  return W;
}


// Implements fl_beep(). See documentation in src/fl_ask.cxx.
void Fl_WinAPI_Screen_Driver::beep(int type)
{
  switch (type) {
    case FL_BEEP_QUESTION :
    case FL_BEEP_PASSWORD :
      MessageBeep(MB_ICONQUESTION);
      break;
    case FL_BEEP_MESSAGE :
      MessageBeep(MB_ICONASTERISK);
      break;
    case FL_BEEP_NOTIFICATION :
      MessageBeep(MB_ICONASTERISK);
      break;
    case FL_BEEP_ERROR :
      MessageBeep(MB_ICONERROR);
      break;
    default :
      MessageBeep(0xFFFFFFFF);
      break;
  }
}


void Fl_WinAPI_Screen_Driver::flush()
{
  GdiFlush();
}


extern void fl_fix_focus(); // in Fl.cxx

// We have to keep track of whether we have captured the mouse, since
// Windows shows little respect for this... Grep for fl_capture to
// see where and how this is used.
extern HWND fl_capture;


void Fl_WinAPI_Screen_Driver::grab(Fl_Window* win)
{
  if (win) {
    if (!Fl::grab_) {
      SetActiveWindow(fl_capture = fl_xid(Fl::first_window()));
      SetCapture(fl_capture);
    }
    Fl::grab_ = win;
  } else {
    if (Fl::grab_) {
      fl_capture = 0;
      ReleaseCapture();
      Fl::grab_ = 0;
      fl_fix_focus();
    }
  }
}


static void set_selection_color(uchar r, uchar g, uchar b)
{
  Fl::set_color(FL_SELECTION_COLOR,r,g,b);
}


static void getsyscolor(int what, const char* arg, void (*func)(uchar,uchar,uchar))
{
  if (arg) {
    uchar r,g,b;
    if (!fl_parse_color(arg, r,g,b))
      Fl::error("Unknown color: %s", arg);
    else
      func(r,g,b);
  } else {
    DWORD x = GetSysColor(what);
    func(uchar(x&255), uchar(x>>8), uchar(x>>16));
  }
}


void Fl_WinAPI_Screen_Driver::get_system_colors()
{
  if (!bg2_set) getsyscolor(COLOR_WINDOW,       fl_bg2,Fl::background2);
  if (!fg_set) getsyscolor(COLOR_WINDOWTEXT,    fl_fg, Fl::foreground);
  if (!bg_set) getsyscolor(COLOR_BTNFACE,       fl_bg, Fl::background);
  getsyscolor(COLOR_HIGHLIGHT,  0,     set_selection_color);
}


int Fl_WinAPI_Screen_Driver::compose(int &del) {
  unsigned char ascii = (unsigned char)Fl::e_text[0];
  /* WARNING: The [AltGr] key on international keyboards sets FL_CTRL.
   2nd line in condition below asks [AltGr] key (a.k.a. VK_RMENU) not to be down.
   */
  int condition = (Fl::e_state & (FL_ALT | FL_META  | FL_CTRL)) && !(ascii & 128) &&
    !( (Fl::e_state & FL_CTRL) && (GetAsyncKeyState(VK_RMENU) >> 15) );
  if (condition) { // this stuff is to be treated as a function key
    del = 0;
    return 0;
  }
  del = Fl::compose_state;
  Fl::compose_state = 0;
  // Only insert non-control characters:
  if ( (!Fl::compose_state) && ! (ascii & ~31 && ascii!=127)) {
    return 0;
  }
  return 1;
}


Fl_RGB_Image *                                                  // O - image or NULL if failed
Fl_WinAPI_Screen_Driver::read_win_rectangle(
                                            int   X,            // I - Left position
                                            int   Y,            // I - Top position
                                            int   w,            // I - Width of area to read
                                            int   h,            // I - Height of area to read
                                            Fl_Window *win,     // I - window to capture from or NULL to capture from current offscreen
                                            bool may_capture_subwins, bool *did_capture_subwins)
{
  float s = Fl_Surface_Device::surface()->driver()->scale();
  int ws, hs;
  if (int(s) == s) { ws = w * int(s); hs = h * int(s);}
  else {
    ws = Fl_Scalable_Graphics_Driver::floor(X+w, s) - Fl_Scalable_Graphics_Driver::floor(X, s),
    hs = Fl_Scalable_Graphics_Driver::floor(Y+h, s) - Fl_Scalable_Graphics_Driver::floor(Y, s);
    if (ws < 1) ws = 1;
    if (hs < 1) hs = 1;
  }
  return read_win_rectangle_unscaled(Fl_Scalable_Graphics_Driver::floor(X, s), Fl_Scalable_Graphics_Driver::floor(Y, s), ws, hs, win);
}

Fl_RGB_Image *Fl_WinAPI_Screen_Driver::read_win_rectangle_unscaled(int X, int Y, int w, int h, Fl_Window *win)
{
  // Depth of image is always 3 here

  // Grab all of the pixels in the image...

  // Assure that we are not trying to read non-existing data. If it is so, the
  // function should still work, but the out-of-bounds part of the image is
  // untouched (initialized with the alpha value or 0 (black), resp.).

  int ww = w; // We need the original width for output data line size

  int shift_x = 0; // X target shift if X modified
  int shift_y = 0; // Y target shift if X modified

  if (X < 0) {
    shift_x = -X;
    w += X;
    X = 0;
  }
  if (Y < 0) {
    shift_y = -Y;
    h += Y;
    Y = 0;
  }

  if (h < 1 || w < 1) return 0;            // nothing to copy

  // Allocate and initialize the image data array
  size_t arraySize = ((size_t)w * h) * 3;
  uchar *p = new uchar[arraySize];
  memset(p, 0, arraySize);

  int line_size = ((3*w+3)/4) * 4;      // each line is aligned on a DWORD (4 bytes)
  uchar *dib = new uchar[line_size*h];  // create temporary buffer to read DIB

  // fill in bitmap info for GetDIBits

  BITMAPINFO   bi;
  bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi.bmiHeader.biWidth = w;
  bi.bmiHeader.biHeight = -h;           // negative => top-down DIB
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = 24;         // 24 bits RGB
  bi.bmiHeader.biCompression = BI_RGB;
  bi.bmiHeader.biSizeImage = 0;
  bi.bmiHeader.biXPelsPerMeter = 0;
  bi.bmiHeader.biYPelsPerMeter = 0;
  bi.bmiHeader.biClrUsed = 0;
  bi.bmiHeader.biClrImportant = 0;

  // copy bitmap from original DC (Window, Fl_Offscreen, ...)
  if (win && Fl_Window::current() != win) win->make_current();
  HDC gc = (HDC)fl_graphics_driver->gc();
  HDC hdc = CreateCompatibleDC(gc);
  HBITMAP hbm = CreateCompatibleBitmap(gc,w,h);

  int save_dc = SaveDC(hdc);                    // save context for cleanup
  SelectObject(hdc,hbm);                        // select bitmap
  BitBlt(hdc,0,0,w,h,gc,X,Y,SRCCOPY);   // copy image section to DDB

  // copy RGB image data to the allocated DIB

  GetDIBits(hdc, hbm, 0, h, dib, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

  // finally copy the image data to the user buffer

  for (int j = 0; j<h; j++) {
    const uchar *src = dib + j * line_size;                     // source line
    uchar *tg = p + (j + shift_y) * 3 * ww + shift_x * 3;       // target line
    for (int i = 0; i<w; i++) {
      uchar b = *src++;
      uchar g = *src++;
      *tg++ = *src++;   // R
      *tg++ = g;        // G
      *tg++ = b;        // B
    }
  }

  // free used GDI and other structures

  RestoreDC(hdc,save_dc);       // reset DC
  DeleteDC(hdc);
  DeleteObject(hbm);
  delete[] dib;         // delete DIB temporary buffer

  Fl_RGB_Image *rgb = new Fl_RGB_Image(p, w, h, 3);
  rgb->alloc_array = 1;
  return rgb;
}


void Fl_WinAPI_Screen_Driver::offscreen_size(Fl_Offscreen off, int &width, int &height)
{
  BITMAP bitmap;
  if ( GetObject((HBITMAP)off, sizeof(BITMAP), &bitmap) ) {
    width = bitmap.bmWidth;
    height = bitmap.bmHeight;
  }
}

//NOTICE: returns -1 if x,y is not in any screen
int Fl_WinAPI_Screen_Driver::screen_num_unscaled(int x, int y)
{
  int screen = -1;
  if (num_screens < 0) init();
  for (int i = 0; i < num_screens; i ++) {
    if (x >= screens[i].left && x < screens[i].right &&
        y >= screens[i].top && y < screens[i].bottom) {
      screen = i;
      break;
    }
  }
  return screen;
}


float Fl_WinAPI_Screen_Driver::base_scale(int numscreen) {
  return float(dpi[numscreen][0] / 96.);
}
