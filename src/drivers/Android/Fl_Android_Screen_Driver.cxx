//
// "$Id$"
//
// Android screen interface for the Fast Light Tool Kit (FLTK).
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
#include "Fl_Android_Application.H"
#include "Fl_Android_Graphics_Font.H"
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/Fl_Graphics_Driver.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/fl_ask.H>
#include <stdio.h>
#include <errno.h>
#include <math.h>


static void nothing() {}
void (*fl_unlock_function)() = nothing;
void (*fl_lock_function)() = nothing;

static void timer_do_callback(int timerIndex);


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


extern int fl_send_system_handlers(void *e);

int Fl_Android_Screen_Driver::handle_app_command()
{
  // get the command
  int8_t cmd = Fl_Android_Application::read_cmd();

  // setup the Android glue and prepare all settings for calling into FLTK
  Fl_Android_Application::pre_exec_cmd(cmd);

  // call all registered FLTK system handlers
  Fl::e_number = ((uint32_t)(cmd-Fl_Android_Application::APP_CMD_INPUT_CHANGED)) + FL_ANDROID_EVENT_INPUT_CHANGED;
  fl_send_system_handlers(nullptr);

  // fixup and finalize application wide command handling
  Fl_Android_Application::post_exec_cmd(cmd);
  return 1;
}

int Fl_Android_Screen_Driver::handle_input_event()
{
  AInputQueue *queue = Fl_Android_Application::input_event_queue();
  AInputEvent *event = nullptr;

  if (AInputQueue_getEvent(queue, &event) >= 0) {
    if (AInputQueue_preDispatchEvent(queue, event)==0) {
      int consumed = 0;
      switch (AInputEvent_getType(event)) {
        case  AINPUT_EVENT_TYPE_KEY:
          consumed = handle_keyboard_event(event);
          break;
        case AINPUT_EVENT_TYPE_MOTION:
          consumed = handle_mouse_event(event);
          break;
        default:
          // don't do anything. There may be additional event types in the future
          break;
      }
      // TODO: handle all events here
      AInputQueue_finishEvent(queue, event, consumed);
    }
  }
  return 0;
}

int Fl_Android_Screen_Driver::handle_keyboard_event(AInputEvent *event)
{
  Fl_Android_Application::log_i("Key event: action=%d keyCode=%d metaState=0x%x",
                                AKeyEvent_getAction(event),
                                AKeyEvent_getKeyCode(event),
                                AKeyEvent_getMetaState(event));
  return 0;
}

int Fl_Android_Screen_Driver::handle_mouse_event(AInputEvent *event)
{
  int ex = Fl::e_x_root = (int)(AMotionEvent_getX(event, 0) * 600 /
                                 ANativeWindow_getWidth(Fl_Android_Application::native_window()));
  int ey = Fl::e_y_root = (int)(AMotionEvent_getY(event, 0) * 800 /
                                 ANativeWindow_getHeight(Fl_Android_Application::native_window()));

  // FIXME: find the window in which the event happened
  Fl_Window *win = Fl::grab();
  if (!win) {
    win = Fl::first_window();
    while (win) {
      if (ex >= win->x() && ex < win->x() + win->w() && ey >= win->y() &&
          ey < win->y() + win->h())
        break;
      win = Fl::next_window(win);
    }
  }
  if (!win) return 0;

  if (win) {
    Fl::e_x = ex-win->x();
    Fl::e_y = ey-win->y();
  } else {
    Fl::e_x = ex;
    Fl::e_y = ey;
  }

  Fl::e_state = FL_BUTTON1;
  Fl::e_keysym = FL_Button + 1;
  if (AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_DOWN) {
    Fl::e_is_click = 1;
    if (win) Fl::handle(FL_PUSH, win); // do NOT send a push event into the "Desktop"
    Fl_Android_Application::log_i("Mouse push %d at %d, %d", Fl::event_button(), Fl::event_x(), Fl::event_y());
  } else if (AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_MOVE) {
    Fl::handle(FL_DRAG, win);
  } else if (AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_UP) {
    Fl::e_state = 0;
    Fl::handle(FL_RELEASE, win);
  }
  return 1;
}

/**
 * Handle all events in the even queue.
 *
 * FIXME: what should this function return?
 *
 * @param time_to_wait
 * @return we do not know
 */
int Fl_Android_Screen_Driver::handle_queued_events(double time_to_wait)
{
  int ret = 0;
  // Read all pending events.
  int ident;
  int events;
  struct android_poll_source *source;

  for (;;) {
    ident = ALooper_pollAll(Fl::damage() ? 0 : -1, nullptr, &events, (void **) &source);
    switch (ident) {
      // FIXME:  ALOOPER_POLL_WAKE = -1, ALOOPER_POLL_CALLBACK = -2, ALOOPER_POLL_TIMEOUT = -3, ALOOPER_POLL_ERROR = -4
      case Fl_Android_Application::LOOPER_ID_MAIN:
        ret = handle_app_command();
        break;
      case Fl_Android_Application::LOOPER_ID_INPUT:
        ret = handle_input_event();
        break;
      case Fl_Android_Application::LOOPER_ID_TIMER:
        timer_do_callback(Fl_Android_Application::receive_timer_index());
        break;
      case -3: return ret;
      default: return ret;
    }
  }
  return ret;
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
#endif

/**
 * On Android, we currently write into a memory buffer and copy
 * the content to the screen.
 */
void Fl_Android_Screen_Driver::flush()
{
  Fl_Screen_Driver::flush();
  // FIXME: do this only if anything actually changed on screen (need to optimize)!
  if (pScreenContentChanged) {
    if (Fl_Android_Application::copy_screen())
      pScreenContentChanged = false;
  }
}

#if 0
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


// ---- timers

struct TimerData
{
  timer_t handle;
  struct sigevent sigevent;
  Fl_Timeout_Handler callback;
  void *data;
  bool used;
  bool triggered;
  struct itimerspec timeout;
};
static TimerData* timerData = nullptr;
static int NTimerData = 0;
static int nTimerData = 0;


static int allocate_more_timers()
{
  if (NTimerData == 0) {
    NTimerData = 8;
  }
  if (NTimerData>256) { // out of timers
    return -1;
  }
  NTimerData *= 2;
  timerData = (TimerData*)realloc(timerData, sizeof(TimerData) * NTimerData);
  return nTimerData;
}


static void timer_signal_handler(union sigval data)
{
  int timerIndex = data.sival_int;
  Fl_Android_Application::send_timer_index(timerIndex);
}


static void timer_do_callback(int timerIndex)
{
  TimerData& t = timerData[timerIndex];
  t.triggered = false;
  if (t.callback) {
    t.callback(t.data);
    // TODO: should we release the timer at this point?
  }
}


void Fl_Android_Screen_Driver::add_timeout(double time, Fl_Timeout_Handler cb, void *data)
{
  repeat_timeout(time, cb, data);
}


void Fl_Android_Screen_Driver::repeat_timeout(double time, Fl_Timeout_Handler cb, void *data)
{
  int ret = -1;
  int timerIndex = -1;

  // first, find the timer associated with this handler
  for (int i = 0; i < nTimerData; ++i) {
    TimerData& t = timerData[i];
    if ( (t.used) && (t.callback==cb) && (t.data==data) ) {
      timerIndex = i;
      break;
    }
  }

  // if we did not have a timer yet, find a free slot
  if (timerIndex==-1) {
    for (int i = 0; i < nTimerData; ++i) {
      if (!timerData[i].used)
        timerIndex = i;
      break;
    }
  }

  // if that didn't work, allocate more timers
  if (timerIndex==-1) {
    if (nTimerData==NTimerData)
      allocate_more_timers();
    timerIndex = nTimerData++;
  }

  // if that didn;t work either, we ran out of timers
  if (timerIndex==-1) {
    Fl::error("FLTK ran out of timer slots.");
    return;
  }

  TimerData& t = timerData[timerIndex];
  if (!t.used) {
    t.data = data;
    t.callback = cb;
    memset(&t.sigevent, 0, sizeof(struct sigevent));
    t.sigevent.sigev_notify = SIGEV_THREAD;
    t.sigevent.sigev_notify_function = timer_signal_handler;
    t.sigevent.sigev_value.sival_int = timerIndex;
    ret = timer_create(CLOCK_MONOTONIC, &t.sigevent, &t.handle);
    if (ret==-1) {
      Fl_Android_Application::log_e("Can't create timer: %s", strerror(errno));
      return;
    }
    t.used = true;
  }

  double ff;
  t.timeout = {
          { 0, 0 },
          { (time_t)floor(time), (long)(modf(time, &ff)*1000000000) }
  };
  ret = timer_settime(t.handle, 0, &t.timeout, nullptr);
  if (ret==-1) {
    Fl_Android_Application::log_e("Can't launch timer: %s", strerror(errno));
    return;
  }
  t.triggered = true;
}


int Fl_Android_Screen_Driver::has_timeout(Fl_Timeout_Handler cb, void *data)
{
  for (int i = 0; i < nTimerData; ++i) {
    TimerData& t = timerData[i];
    if ( (t.used) && (t.callback==cb) && (t.data==data) ) {
      return 1;
    }
  }
  return 0;
}


void Fl_Android_Screen_Driver::remove_timeout(Fl_Timeout_Handler cb, void *data)
{
  for (int i = 0; i < nTimerData; ++i) {
    TimerData& t = timerData[i];
    if ( t.used && (t.callback==cb) && ( (t.data==data) || (data==nullptr) ) ) {
      if (t.used)
        timer_delete(t.handle);
      t.triggered = t.used = false;
    }
  }
}


//
// End of "$Id$".
//
