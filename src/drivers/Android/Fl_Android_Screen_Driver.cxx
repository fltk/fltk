//
// "$Id$"
//
// Windows screen interface for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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
#include "Fl_Android_Screen_Driver.H"
#include "./Fl_Font.H"
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/Fl_Graphics_Driver.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/fl_ask.H>
#include <stdio.h>

#include <android/log.h>

#define  LOG_TAG    "FLTK"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGV(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

static struct android_app* gAndroidApp = 0L;

static void nothing() {}
void (*fl_unlock_function)() = nothing;
void (*fl_lock_function)() = nothing;

#if 0

// these are set by Fl::args() and override any system colors: from Fl_get_system_colors.cxx
extern const char *fl_fg;
extern const char *fl_bg;
extern const char *fl_bg2;
// end of extern additions workaround


#if !defined(HMONITOR_DECLARED) && (_WIN32_WINNT < 0x0500)
#  define COMPILE_MULTIMON_STUBS
#  include <multimon.h>
#endif // !HMONITOR_DECLARED && _WIN32_WINNT < 0x0500

#endif

/*
 Creates a driver that manages all screen and display related calls.

 This function must be implemented once for every platform.
 */
Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver()
{
  return new Fl_Android_Screen_Driver();
}

int Fl_Android_Screen_Driver::handle_queued_events(double time_to_wait)
{
/*
  int ALooper_pollAll	(	int 	timeoutMillis,
                          int * 	outFd,
                          int * 	outEvents,
                          void ** 	outData
  )

  struct engine engine;

  memset(&engine, 0, sizeof(engine));
  state->userData = &engine;
  state->onAppCmd = engine_handle_cmd;
  state->onInputEvent = engine_handle_input;
  engine.app = state;

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  start_ms = (((int64_t)now.tv_sec)*1000000000LL + now.tv_nsec)/1000000;

  win = new Fl_Window(10, 10, 600, 400, "Hallo");
  btn = new Fl_Button(190, 200, 280, 35, "Hello, Android!");
  win->show();


  // loop waiting for stuff to do.

  while (1) {
    // Read all pending events.
    int ident;
    int events;
    struct android_poll_source* source;

    // If not animating, we will block forever waiting for events.
    // If animating, we loop until all events are read, then continue
    // to draw the next frame of animation.
    while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                                  (void**)&source)) >= 0) {

      // Process this event.
      if (source != NULL) {
        source->process(state, source);
      }

      // Check if we are exiting.
      if (state->destroyRequested != 0) {
        LOGI("Engine thread destroy requested!");
        engine_term_display(&engine);
        return;
      }
    }

    if (engine.animating) {
      engine_draw_frame(&engine);
    }
  }
  */
  return -1;
}


double Fl_Android_Screen_Driver::wait(double time_to_wait)
{
  Fl::run_checks();
  static int in_idle = 0;
  if (Fl::idle) {
    if (!in_idle) {
      in_idle = 1;
      Fl::idle();
      in_idle = 0;
    }
    // the idle function may turn off idle, we can then wait:
    if (Fl::idle) time_to_wait = 0.0;
  }

  if (time_to_wait==0.0) {
    // if there is no wait time, handle the event and show the results right away
    fl_unlock_function();
    handle_queued_events(time_to_wait);
    fl_lock_function();
    Fl::flush();
  } else {
    // if there is wait time, show the pending changes and then handle the events
    Fl::flush();
    if (Fl::idle && !in_idle) // 'idle' may have been set within flush()
      time_to_wait = 0.0;
    fl_unlock_function();
    handle_queued_events(time_to_wait);
    fl_lock_function();
  }

  return 0.0; // FIXME: return the remaining time to reach 'time_to_wait'
}


#if 0

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
//extern FILE*LOG;fprintf(LOG,"screen_cb ns=%d\n",num_screens);fflush(LOG);
    /*fl_alert("screen %d %d,%d,%d,%d work %d,%d,%d,%d",num_screens,
    screens[num_screens].left,screens[num_screens].right,screens[num_screens].top,screens[num_screens].bottom,
    work_area[num_screens].left,work_area[num_screens].right,work_area[num_screens].top,work_area[num_screens].bottom);
    */
    // find the pixel size
    if (mi.cbSize == sizeof(mi)) {
      HDC screen = CreateDC(mi.szDevice, NULL, NULL, NULL);
      if (screen) {
        dpi[num_screens][0] = (float)GetDeviceCaps(screen, LOGPIXELSX);
        dpi[num_screens][1] = (float)GetDeviceCaps(screen, LOGPIXELSY);
      }
      DeleteDC(screen);
    }

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
  scale_of_screen[0] = 1;
}


float Fl_WinAPI_Screen_Driver::desktop_scale_factor() {
  return 0; //indicates each screen has already been assigned its scale factor value
}


void Fl_WinAPI_Screen_Driver::screen_work_area(int &X, int &Y, int &W, int &H, int n)
{
  if (num_screens < 0) init();
  if (n < 0 || n >= num_screens) n = 0;
  X = work_area[n].left/scale_of_screen[n];
  Y = work_area[n].top/scale_of_screen[n];
  W = (work_area[n].right - X)/scale_of_screen[n];
  H = (work_area[n].bottom - Y)/scale_of_screen[n];
}


void Fl_WinAPI_Screen_Driver::screen_xywh(int &X, int &Y, int &W, int &H, int n)
{
  if (num_screens < 0) init();

  if ((n < 0) || (n >= num_screens))
    n = 0;

  if (num_screens > 0) {
    X = screens[n].left/scale_of_screen[n];
    Y = screens[n].top/scale_of_screen[n];
    W = (screens[n].right - screens[n].left)/scale_of_screen[n];
    H = (screens[n].bottom - screens[n].top)/scale_of_screen[n];
  } else {
    /* Fallback if something is broken... */
    X = 0;
    Y = 0;
    W = GetSystemMetrics(SM_CXSCREEN);
    H = GetSystemMetrics(SM_CYSCREEN);
  }
}


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
  RECT r;

  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.left;
}


int Fl_WinAPI_Screen_Driver::y()
{
  RECT r;

  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.top;
}


int Fl_WinAPI_Screen_Driver::h()
{
  RECT r;

  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.bottom - r.top;
}


int Fl_WinAPI_Screen_Driver::w()
{
  RECT r;

  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.right - r.left;
}


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
  if (!bg2_set) getsyscolor(COLOR_WINDOW,	fl_bg2,Fl::background2);
  if (!fg_set) getsyscolor(COLOR_WINDOWTEXT,	fl_fg, Fl::foreground);
  if (!bg_set) getsyscolor(COLOR_BTNFACE,	fl_bg, Fl::background);
  getsyscolor(COLOR_HIGHLIGHT,	0,     set_selection_color);
}


const char *Fl_WinAPI_Screen_Driver::get_system_scheme()
{
  return fl_getenv("FLTK_SCHEME");
}


// ---- timers


struct Win32Timer
{
  UINT_PTR handle;
  Fl_Timeout_Handler callback;
  void *data;
};
static Win32Timer* win32_timers;
static int win32_timer_alloc;
static int win32_timer_used;
static HWND s_TimerWnd;


static void realloc_timers()
{
  if (win32_timer_alloc == 0) {
    win32_timer_alloc = 8;
  }
  win32_timer_alloc *= 2;
  Win32Timer* new_timers = new Win32Timer[win32_timer_alloc];
  memset(new_timers, 0, sizeof(Win32Timer) * win32_timer_used);
  memcpy(new_timers, win32_timers, sizeof(Win32Timer) * win32_timer_used);
  Win32Timer* delete_me = win32_timers;
  win32_timers = new_timers;
  delete [] delete_me;
}


static void delete_timer(Win32Timer& t)
{
  KillTimer(s_TimerWnd, t.handle);
  memset(&t, 0, sizeof(Win32Timer));
}


static LRESULT CALLBACK s_TimerProc(HWND hwnd, UINT msg,
                                    WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
    case WM_TIMER:
    {
      unsigned int id = (unsigned) (wParam - 1);
      if (id < (unsigned int)win32_timer_used && win32_timers[id].handle) {
        Fl_Timeout_Handler cb   = win32_timers[id].callback;
        void*              data = win32_timers[id].data;
        delete_timer(win32_timers[id]);
        if (cb) {
          (*cb)(data);
        }
      }
    }
      return 0;

    default:
      break;
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}


void Fl_WinAPI_Screen_Driver::add_timeout(double time, Fl_Timeout_Handler cb, void* data)
{
  repeat_timeout(time, cb, data);
}


void Fl_WinAPI_Screen_Driver::repeat_timeout(double time, Fl_Timeout_Handler cb, void* data)
{
  int timer_id = -1;
  for (int i = 0;  i < win32_timer_used;  ++i) {
    if ( !win32_timers[i].handle ) {
      timer_id = i;
      break;
    }
  }
  if (timer_id == -1) {
    if (win32_timer_used == win32_timer_alloc) {
      realloc_timers();
    }
    timer_id = win32_timer_used++;
  }
  unsigned int elapsed = (unsigned int)(time * 1000);

  if ( !s_TimerWnd ) {
    const char* timer_class = "FLTimer";
    WNDCLASSEX wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof (wc);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = (WNDPROC)s_TimerProc;
    wc.hInstance = fl_display;
    wc.lpszClassName = timer_class;
    /*ATOM atom =*/ RegisterClassEx(&wc);
    // create a zero size window to handle timer events
    s_TimerWnd = CreateWindowEx(WS_EX_LEFT | WS_EX_TOOLWINDOW,
                                timer_class, "",
                                WS_POPUP,
                                0, 0, 0, 0,
                                NULL, NULL, fl_display, NULL);
    // just in case this OS won't let us create a 0x0 size window:
    if (!s_TimerWnd)
      s_TimerWnd = CreateWindowEx(WS_EX_LEFT | WS_EX_TOOLWINDOW,
                                  timer_class, "",
                                  WS_POPUP,
                                  0, 0, 1, 1,
                                  NULL, NULL, fl_display, NULL);
    ShowWindow(s_TimerWnd, SW_SHOWNOACTIVATE);
  }

  win32_timers[timer_id].callback = cb;
  win32_timers[timer_id].data     = data;

  win32_timers[timer_id].handle =
  SetTimer(s_TimerWnd, timer_id + 1, elapsed, NULL);
}


int Fl_WinAPI_Screen_Driver::has_timeout(Fl_Timeout_Handler cb, void* data)
{
  for (int i = 0;  i < win32_timer_used;  ++i) {
    Win32Timer& t = win32_timers[i];
    if (t.handle  &&  t.callback == cb  &&  t.data == data) {
      return 1;
    }
  }
  return 0;
}


void Fl_WinAPI_Screen_Driver::remove_timeout(Fl_Timeout_Handler cb, void* data)
{
  int i;
  for (i = 0;  i < win32_timer_used;  ++i) {
    Win32Timer& t = win32_timers[i];
    if (t.handle  &&  t.callback == cb  &&
        (t.data == data  ||  data == NULL)) {
      delete_timer(t);
    }
  }
}

int Fl_WinAPI_Screen_Driver::compose(int &del) {
  unsigned char ascii = (unsigned char)Fl::e_text[0];
  int condition = (Fl::e_state & (FL_ALT | FL_META)) && !(ascii & 128) ;
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
                                            int   X,		// I - Left position
                                            int   Y,		// I - Top position
                                            int   w,		// I - Width of area to read
                                            int   h)		// I - Height of area to read
{
  float s = Fl_Surface_Device::surface()->driver()->scale();
  return read_win_rectangle_unscaled(X*s, Y*s, w*s, h*s);
}

Fl_RGB_Image *Fl_WinAPI_Screen_Driver::read_win_rectangle_unscaled(int X, int Y, int w, int h)
{
  int	d = 3;			// Depth of image
  int alpha = 0; uchar *p = NULL;
  // Allocate the image data array as needed...  
  const uchar *oldp = p;
  if (!p) p = new uchar[w * h * d];
  
  // Initialize the default colors/alpha in the whole image...
  memset(p, alpha, w * h * d);
  
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
  
  if (h < 1 || w < 1) return 0/*p*/;		// nothing to copy
  
  int line_size = ((3*w+3)/4) * 4;	// each line is aligned on a DWORD (4 bytes)
  uchar *dib = new uchar[line_size*h];	// create temporary buffer to read DIB
  
  // fill in bitmap info for GetDIBits
  
  BITMAPINFO   bi;
  bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi.bmiHeader.biWidth = w;
  bi.bmiHeader.biHeight = -h;		// negative => top-down DIB
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = 24;		// 24 bits RGB
  bi.bmiHeader.biCompression = BI_RGB;
  bi.bmiHeader.biSizeImage = 0;
  bi.bmiHeader.biXPelsPerMeter = 0;
  bi.bmiHeader.biYPelsPerMeter = 0;
  bi.bmiHeader.biClrUsed = 0;
  bi.bmiHeader.biClrImportant = 0;
  
  // copy bitmap from original DC (Window, Fl_Offscreen, ...)
  HDC gc = (HDC)fl_graphics_driver->gc();
  HDC hdc = CreateCompatibleDC(gc);
  HBITMAP hbm = CreateCompatibleBitmap(gc,w,h);
  
  int save_dc = SaveDC(hdc);			// save context for cleanup
  SelectObject(hdc,hbm);			// select bitmap
  BitBlt(hdc,0,0,w,h,gc,X,Y,SRCCOPY);	// copy image section to DDB
  
  // copy RGB image data to the allocated DIB
  
  GetDIBits(hdc, hbm, 0, h, dib, (BITMAPINFO *)&bi, DIB_RGB_COLORS);
  
  // finally copy the image data to the user buffer
  
  for (int j = 0; j<h; j++) {
    const uchar *src = dib + j * line_size;			// source line
    uchar *tg = p + (j + shift_y) * d * ww + shift_x * d;	// target line
    for (int i = 0; i<w; i++) {
      uchar b = *src++;
      uchar g = *src++;
      *tg++ = *src++;	// R
      *tg++ = g;	// G
      *tg++ = b;	// B
      if (alpha)
        *tg++ = alpha;	// alpha
    }
  }
  
  // free used GDI and other structures
  
  RestoreDC(hdc,save_dc);	// reset DC
  DeleteDC(hdc);
  DeleteObject(hbm);
  delete[] dib;		// delete DIB temporary buffer
  
  Fl_RGB_Image *rgb = new Fl_RGB_Image(p, w, h, d);
  if (!oldp) rgb->alloc_array = 1;
  return rgb;
}

#ifndef FLTK_HIDPI_SUPPORT
/* Returns the current desktop scaling factor for screen_num (1.75 for example)
 */
float Fl_WinAPI_Screen_Driver::DWM_scaling_factor() {
  // Compute the global desktop scaling factor: 1, 1.25, 1.5, 1.75, etc...
  // This factor can be set in Windows 10 by
  // "Change the size of text, apps and other items" in display settings.
  // We don't cache this value because it can change while the app is running.
  HDC hdc = GetDC(NULL);
  int hr = GetDeviceCaps(hdc, HORZRES); // pixels visible to the app
#ifndef DESKTOPHORZRES
#define DESKTOPHORZRES 118
  /* As of 27 august 2016, the DESKTOPHORZRES flag for GetDeviceCaps()
   has disappeared from Microsoft online doc, but is quoted in numerous coding examples
   e.g., https://social.msdn.microsoft.com/Forums/en-US/6acc3b21-23a4-4a00-90b4-968a43e1ccc8/capture-screen-with-high-dpi?forum=vbgeneral
   It is necessary for the computation of the scaling factor at runtime as done here.
   */
#endif
  int dhr = GetDeviceCaps(hdc, DESKTOPHORZRES); // true number of pixels on display
  ReleaseDC(NULL, hdc);
  float scaling = dhr/float(hr);
  scaling = int(scaling * 100 + 0.5)/100.; // round to 2 digits after decimal point
  return scaling;
}

#endif // ! FLTK_HIDPI_SUPPORT

void Fl_WinAPI_Screen_Driver::offscreen_size(Fl_Offscreen off, int &width, int &height)
{
  BITMAP bitmap;
  if ( GetObject(off, sizeof(BITMAP), &bitmap) ) {
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

#endif


/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <jni.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

static void free_saved_state(struct android_app* android_app) {
    pthread_mutex_lock(&android_app->mutex);
    if (android_app->savedState != NULL) {
        free(android_app->savedState);
        android_app->savedState = NULL;
        android_app->savedStateSize = 0;
    }
    pthread_mutex_unlock(&android_app->mutex);
}

int8_t android_app_read_cmd(struct android_app* android_app) {
    int8_t cmd;
    if (read(android_app->msgread, &cmd, sizeof(cmd)) == sizeof(cmd)) {
        switch (cmd) {
            case APP_CMD_SAVE_STATE:
                free_saved_state(android_app);
                break;
        }
        return cmd;
    } else {
        LOGE("No data on command pipe!");
    }
    return -1;
}

static void print_cur_config(struct android_app* android_app) {
    char lang[2], country[2];
    AConfiguration_getLanguage(android_app->config, lang);
    AConfiguration_getCountry(android_app->config, country);

    LOGV("Config: mcc=%d mnc=%d lang=%c%c cnt=%c%c orien=%d touch=%d dens=%d "
            "keys=%d nav=%d keysHid=%d navHid=%d sdk=%d size=%d long=%d "
            "modetype=%d modenight=%d",
            AConfiguration_getMcc(android_app->config),
            AConfiguration_getMnc(android_app->config),
            lang[0], lang[1], country[0], country[1],
            AConfiguration_getOrientation(android_app->config),
            AConfiguration_getTouchscreen(android_app->config),
            AConfiguration_getDensity(android_app->config),
            AConfiguration_getKeyboard(android_app->config),
            AConfiguration_getNavigation(android_app->config),
            AConfiguration_getKeysHidden(android_app->config),
            AConfiguration_getNavHidden(android_app->config),
            AConfiguration_getSdkVersion(android_app->config),
            AConfiguration_getScreenSize(android_app->config),
            AConfiguration_getScreenLong(android_app->config),
            AConfiguration_getUiModeType(android_app->config),
            AConfiguration_getUiModeNight(android_app->config));
}

void android_app_pre_exec_cmd(struct android_app* android_app, int8_t cmd) {
    switch (cmd) {
        case APP_CMD_INPUT_CHANGED:
            LOGV("APP_CMD_INPUT_CHANGED\n");
            pthread_mutex_lock(&android_app->mutex);
            if (android_app->inputQueue != NULL) {
                AInputQueue_detachLooper(android_app->inputQueue);
            }
            android_app->inputQueue = android_app->pendingInputQueue;
            if (android_app->inputQueue != NULL) {
                LOGV("Attaching input queue to looper");
                AInputQueue_attachLooper(android_app->inputQueue,
                        android_app->looper, LOOPER_ID_INPUT, NULL,
                        &android_app->inputPollSource);
            }
            pthread_cond_broadcast(&android_app->cond);
            pthread_mutex_unlock(&android_app->mutex);
            break;

        case APP_CMD_INIT_WINDOW:
            LOGV("APP_CMD_INIT_WINDOW\n");
            pthread_mutex_lock(&android_app->mutex);
            android_app->window = android_app->pendingWindow;
            pthread_cond_broadcast(&android_app->cond);
            pthread_mutex_unlock(&android_app->mutex);
            break;

        case APP_CMD_TERM_WINDOW:
            LOGV("APP_CMD_TERM_WINDOW\n");
            pthread_cond_broadcast(&android_app->cond);
            break;

        case APP_CMD_RESUME:
        case APP_CMD_START:
        case APP_CMD_PAUSE:
        case APP_CMD_STOP:
            LOGV("activityState=%d\n", cmd);
            pthread_mutex_lock(&android_app->mutex);
            android_app->activityState = cmd;
            pthread_cond_broadcast(&android_app->cond);
            pthread_mutex_unlock(&android_app->mutex);
            break;

        case APP_CMD_CONFIG_CHANGED:
            LOGV("APP_CMD_CONFIG_CHANGED\n");
            AConfiguration_fromAssetManager(android_app->config,
                    android_app->activity->assetManager);
            print_cur_config(android_app);
            break;

        case APP_CMD_DESTROY:
            LOGV("APP_CMD_DESTROY\n");
            android_app->destroyRequested = 1;
            break;
    }
}

void android_app_post_exec_cmd(struct android_app* android_app, int8_t cmd) {
    switch (cmd) {
        case APP_CMD_TERM_WINDOW:
            LOGV("APP_CMD_TERM_WINDOW\n");
            pthread_mutex_lock(&android_app->mutex);
            android_app->window = NULL;
            pthread_cond_broadcast(&android_app->cond);
            pthread_mutex_unlock(&android_app->mutex);
            break;

        case APP_CMD_SAVE_STATE:
            LOGV("APP_CMD_SAVE_STATE\n");
            pthread_mutex_lock(&android_app->mutex);
            android_app->stateSaved = 1;
            pthread_cond_broadcast(&android_app->cond);
            pthread_mutex_unlock(&android_app->mutex);
            break;

        case APP_CMD_RESUME:
            free_saved_state(android_app);
            break;
    }
}

void app_dummy() {

}

static void android_app_destroy(struct android_app* android_app) {
    LOGV("android_app_destroy!");
    free_saved_state(android_app);
    pthread_mutex_lock(&android_app->mutex);
    if (android_app->inputQueue != NULL) {
        AInputQueue_detachLooper(android_app->inputQueue);
    }
    AConfiguration_delete(android_app->config);
    android_app->destroyed = 1;
    pthread_cond_broadcast(&android_app->cond);
    pthread_mutex_unlock(&android_app->mutex);
    // Can't touch android_app object after this.
}

static void process_input(struct android_app* app, struct android_poll_source* source) {
    AInputEvent* event = NULL;
    while (AInputQueue_getEvent(app->inputQueue, &event) >= 0) {
        LOGV("New input event: type=%d\n", AInputEvent_getType(event));
        if (AInputQueue_preDispatchEvent(app->inputQueue, event)) {
            continue;
        }
        int32_t handled = 0;
        if (app->onInputEvent != NULL) handled = app->onInputEvent(app, event);
        AInputQueue_finishEvent(app->inputQueue, event, handled);
    }
}

static void process_cmd(struct android_app* app, struct android_poll_source* source) {
    int8_t cmd = android_app_read_cmd(app);
    android_app_pre_exec_cmd(app, cmd);
    if (app->onAppCmd != NULL) app->onAppCmd(app, cmd);
    android_app_post_exec_cmd(app, cmd);
}

static void* android_app_entry(void* param) {
    struct android_app* android_app = (struct android_app*)param;

    android_app->config = AConfiguration_new();
    AConfiguration_fromAssetManager(android_app->config, android_app->activity->assetManager);

    print_cur_config(android_app);

    android_app->cmdPollSource.id = LOOPER_ID_MAIN;
    android_app->cmdPollSource.app = android_app;
    android_app->cmdPollSource.process = process_cmd;
    android_app->inputPollSource.id = LOOPER_ID_INPUT;
    android_app->inputPollSource.app = android_app;
    android_app->inputPollSource.process = process_input;

    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(looper, android_app->msgread, LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL,
            &android_app->cmdPollSource);
    android_app->looper = looper;

    pthread_mutex_lock(&android_app->mutex);
    android_app->running = 1;
    pthread_cond_broadcast(&android_app->cond);
    pthread_mutex_unlock(&android_app->mutex);

    android_main(android_app);

    android_app_destroy(android_app);
    return NULL;
}

// --------------------------------------------------------------------
// Native activity interaction (called from main thread)
// --------------------------------------------------------------------

static struct android_app* android_app_create(ANativeActivity* activity,
        void* savedState, size_t savedStateSize) {
    struct android_app* android_app = (struct android_app*)malloc(sizeof(struct android_app));
    gAndroidApp = android_app;
    memset(android_app, 0, sizeof(struct android_app));
    android_app->activity = activity;

    pthread_mutex_init(&android_app->mutex, NULL);
    pthread_cond_init(&android_app->cond, NULL);

    if (savedState != NULL) {
        android_app->savedState = malloc(savedStateSize);
        android_app->savedStateSize = savedStateSize;
        memcpy(android_app->savedState, savedState, savedStateSize);
    }

    int msgpipe[2];
    if (pipe(msgpipe)) {
        LOGE("could not create pipe: %s", strerror(errno));
        return NULL;
    }
    android_app->msgread = msgpipe[0];
    android_app->msgwrite = msgpipe[1];

    pthread_attr_t attr; 
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&android_app->thread, &attr, android_app_entry, android_app);

    // Wait for thread to start.
    pthread_mutex_lock(&android_app->mutex);
    while (!android_app->running) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }
    pthread_mutex_unlock(&android_app->mutex);

    return android_app;
}

static void android_app_write_cmd(struct android_app* android_app, int8_t cmd) {
    if (write(android_app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd)) {
        LOGE("Failure writing android_app cmd: %s\n", strerror(errno));
    }
}

static void android_app_set_input(struct android_app* android_app, AInputQueue* inputQueue) {
    pthread_mutex_lock(&android_app->mutex);
    android_app->pendingInputQueue = inputQueue;
    android_app_write_cmd(android_app, APP_CMD_INPUT_CHANGED);
    while (android_app->inputQueue != android_app->pendingInputQueue) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }
    pthread_mutex_unlock(&android_app->mutex);
}

static void android_app_set_window(struct android_app* android_app, ANativeWindow* window) {
    pthread_mutex_lock(&android_app->mutex);
    if (android_app->pendingWindow != NULL) {
        android_app_write_cmd(android_app, APP_CMD_TERM_WINDOW);
    }
    android_app->pendingWindow = window;
    if (window != NULL) {
        android_app_write_cmd(android_app, APP_CMD_INIT_WINDOW);
    }
    while (android_app->window != android_app->pendingWindow) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }
    pthread_mutex_unlock(&android_app->mutex);
}

static void android_app_set_activity_state(struct android_app* android_app, int8_t cmd) {
    pthread_mutex_lock(&android_app->mutex);
    android_app_write_cmd(android_app, cmd);
    while (android_app->activityState != cmd) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }
    pthread_mutex_unlock(&android_app->mutex);
}

static void android_app_free(struct android_app* android_app) {
    pthread_mutex_lock(&android_app->mutex);
    android_app_write_cmd(android_app, APP_CMD_DESTROY);
    while (!android_app->destroyed) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }
    pthread_mutex_unlock(&android_app->mutex);

    close(android_app->msgread);
    close(android_app->msgwrite);
    pthread_cond_destroy(&android_app->cond);
    pthread_mutex_destroy(&android_app->mutex);
    free(android_app);
}

static void onDestroy(ANativeActivity* activity) {
    LOGV("Destroy: %p\n", activity);
    android_app_free((struct android_app*)activity->instance);
}

static void onStart(ANativeActivity* activity) {
    LOGV("Start: %p\n", activity);
    android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_START);
}

static void onResume(ANativeActivity* activity) {
    LOGV("Resume: %p\n", activity);
    android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_RESUME);
}

static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
    struct android_app* android_app = (struct android_app*)activity->instance;
    void* savedState = NULL;

    LOGV("SaveInstanceState: %p\n", activity);
    pthread_mutex_lock(&android_app->mutex);
    android_app->stateSaved = 0;
    android_app_write_cmd(android_app, APP_CMD_SAVE_STATE);
    while (!android_app->stateSaved) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }

    if (android_app->savedState != NULL) {
        savedState = android_app->savedState;
        *outLen = android_app->savedStateSize;
        android_app->savedState = NULL;
        android_app->savedStateSize = 0;
    }

    pthread_mutex_unlock(&android_app->mutex);

    return savedState;
}

static void onPause(ANativeActivity* activity) {
    LOGV("Pause: %p\n", activity);
    android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_PAUSE);
}

static void onStop(ANativeActivity* activity) {
    LOGV("Stop: %p\n", activity);
    android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_STOP);
}

static void onConfigurationChanged(ANativeActivity* activity) {
    struct android_app* android_app = (struct android_app*)activity->instance;
    LOGV("ConfigurationChanged: %p\n", activity);
    android_app_write_cmd(android_app, APP_CMD_CONFIG_CHANGED);
}

static void onLowMemory(ANativeActivity* activity) {
    struct android_app* android_app = (struct android_app*)activity->instance;
    LOGV("LowMemory: %p\n", activity);
    android_app_write_cmd(android_app, APP_CMD_LOW_MEMORY);
}

static void onWindowFocusChanged(ANativeActivity* activity, int focused) {
    LOGV("WindowFocusChanged: %p -- %d\n", activity, focused);
    android_app_write_cmd((struct android_app*)activity->instance,
            focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
}

static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
    LOGV("NativeWindowCreated: %p -- %p\n", activity, window);
    android_app_set_window((struct android_app*)activity->instance, window);
}

static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
    LOGV("NativeWindowDestroyed: %p -- %p\n", activity, window);
    android_app_set_window((struct android_app*)activity->instance, NULL);
}

static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
    LOGV("InputQueueCreated: %p -- %p\n", activity, queue);
    android_app_set_input((struct android_app*)activity->instance, queue);
}

static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
    LOGV("InputQueueDestroyed: %p -- %p\n", activity, queue);
    android_app_set_input((struct android_app*)activity->instance, NULL);
}

JNIEXPORT
void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState,
                              size_t savedStateSize) {
    LOGV("Creating: %p\n", activity);
    activity->callbacks->onDestroy = onDestroy;
    activity->callbacks->onStart = onStart;
    activity->callbacks->onResume = onResume;
    activity->callbacks->onSaveInstanceState = onSaveInstanceState;
    activity->callbacks->onPause = onPause;
    activity->callbacks->onStop = onStop;
    activity->callbacks->onConfigurationChanged = onConfigurationChanged;
    activity->callbacks->onLowMemory = onLowMemory;
    activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
    activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;

    activity->instance = android_app_create(activity, savedState, savedStateSize);
}


//
// End of "$Id$".
//
