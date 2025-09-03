//
// Windows-specific code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

// Note: this file contains platform specific code and will therefore
// not be processed by doxygen (see Doxyfile.in).

// This file contains Windows-specific code for FLTK which is always linked
// in.  Search other files for "_WIN32" or filenames ending in _win32.cxx
// for other system-specific code.

/* We require Windows 2000 features (e.g. VK definitions) */
# if !defined(WINVER) || (WINVER < 0x0500)
#  ifdef WINVER
#   undef WINVER
#  endif
#  define WINVER 0x0500
# endif
# if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0500)
#  ifdef _WIN32_WINNT
#   undef _WIN32_WINNT
#  endif
#  define _WIN32_WINNT 0x0500
# endif

// recent versions of MinGW warn: "Please include winsock2.h before windows.h"
#if !defined(__CYGWIN__)
#  include <winsock2.h>
#endif
#include <windows.h>
#include <ole2.h>
#include <shellapi.h>
// Some versions of MinGW now require us to explicitly include winerror to get S_OK defined
#include <winerror.h>
#include <math.h> // for ceil() and round()

void fl_free_fonts(void);
void fl_release_dc(HWND, HDC);
void fl_cleanup_dc_list(void);

#include <config.h>
#include <FL/Fl.H>
#include <FL/platform.H>
#include "Fl_Window_Driver.H"
#include "Fl_Screen_Driver.H"
#include "Fl_Timeout.h"
#include "print_button.h"
#include <FL/Fl_Graphics_Driver.H> // for fl_graphics_driver
#include "drivers/WinAPI/Fl_WinAPI_Window_Driver.H"
#include "drivers/WinAPI/Fl_WinAPI_System_Driver.H"
#include "drivers/WinAPI/Fl_WinAPI_Screen_Driver.H"
#include "drivers/GDI/Fl_GDI_Graphics_Driver.H"
#include <FL/fl_utf8.h>
#include <FL/fl_string_functions.h>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Paged_Device.H>
#include <FL/Fl_Image_Surface.H>
#include "flstring.h"
#include "drivers/GDI/Fl_Font.H"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#ifdef __CYGWIN__
#  include <sys/time.h>
#  include <unistd.h>
#endif

#if !defined(NO_TRACK_MOUSE)
#  include <commctrl.h> // TrackMouseEvent
#endif

#if defined(__GNUC__)
# include <wchar.h>
#endif

// old versions of MinGW lack definition of GET_XBUTTON_WPARAM:

#ifndef GET_XBUTTON_WPARAM
#define GET_XBUTTON_WPARAM(wParam) (HIWORD(wParam))
#endif

static bool is_dpi_aware = false;

extern bool fl_clipboard_notify_empty(void);
extern void fl_trigger_clipboard_notify(int source);
extern HBRUSH fl_brush_action(int action);
extern void fl_cleanup_pens(void);

// MSVC 2010 can't find round() although <math.h> is included above,
// which is surprising because ceil() works fine.
// We could (should?) probably add configure/CMake feature tests for
// round() and ceil() rather than depending on MSVC version numbers.
// AlbrechtS, 02/2010 - Note: we don't know about MSVC 2012 - 2015, see
// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros

#if defined(_MSC_VER) && _MSC_VER <= 1600
#define round(A) int((A) + 0.5)
#endif // _MSC_VER <= 1600

// Internal functions
static void fl_clipboard_notify_target(HWND wnd);
static void fl_clipboard_notify_untarget(HWND wnd);

// Internal variables
static HWND clipboard_wnd = 0;
static HWND next_clipboard_wnd = 0;

static bool initial_clipboard = true;

// dynamic wsock dll handling api:
#if defined(__CYGWIN__) && !defined(SOCKET)
# define SOCKET int
#endif

/*
  Dynamic linking of imm32.dll
  This library is only needed for a hand full (four ATM) functions relating to
  international text rendering and locales. Dynamically loading reduces initial
  size and link dependencies.
*/
static HMODULE s_imm_module = 0;
typedef BOOL(WINAPI *flTypeImmAssociateContextEx)(HWND, HIMC, DWORD);
static flTypeImmAssociateContextEx flImmAssociateContextEx = 0;
typedef HIMC(WINAPI *flTypeImmGetContext)(HWND);
static flTypeImmGetContext flImmGetContext = 0;
typedef BOOL(WINAPI *flTypeImmSetCompositionWindow)(HIMC, LPCOMPOSITIONFORM);
static flTypeImmSetCompositionWindow flImmSetCompositionWindow = 0;
typedef BOOL(WINAPI *flTypeImmReleaseContext)(HWND, HIMC);
static flTypeImmReleaseContext flImmReleaseContext = 0;

static void get_imm_module() {
  s_imm_module = LoadLibrary("IMM32.DLL");
  if (!s_imm_module)
    Fl::fatal("FLTK Lib Error: IMM32.DLL file not found!\n\n"
              "Please check your input method manager library accessibility.");
  flImmAssociateContextEx = (flTypeImmAssociateContextEx)GetProcAddress(s_imm_module, "ImmAssociateContextEx");
  flImmGetContext = (flTypeImmGetContext)GetProcAddress(s_imm_module, "ImmGetContext");
  flImmSetCompositionWindow = (flTypeImmSetCompositionWindow)GetProcAddress(s_imm_module, "ImmSetCompositionWindow");
  flImmReleaseContext = (flTypeImmReleaseContext)GetProcAddress(s_imm_module, "ImmReleaseContext");
}

// USE_TRACK_MOUSE - define NO_TRACK_MOUSE if you don't have
// TrackMouseEvent()...
//
// Now (Dec. 2008) we can assume that current Cygwin/MinGW versions
// support the TrackMouseEvent() function, but WinCE obviously doesn't
// support it (STR 2095). Therefore, USE_TRACK_MOUSE is enabled by
// default, but you can disable it by defining NO_TRACK_MOUSE.
//
// TrackMouseEvent is only used to support window leave notifications
// under Windows. It can be used to get FL_LEAVE events, when the
// mouse leaves the _main_ application window (FLTK detects subwindow
// leave events by using normal move events).
//
// Implementation note: If the mouse cursor leaves one subwindow and
// enters another window, then Windows sends a WM_MOUSEMOVE message to
// the new window before it sends a WM_MOUSELEAVE message to the old
// (just left) window. We save the current mouse window in a static variable,
// and if we get a WM_MOUSELEAVE event for the current mouse window, this
// means that the top level window has been left (otherwise we would have
// got another WM_MOUSEMOVE message before).

// #define NO_TRACK_MOUSE

#if !defined(NO_TRACK_MOUSE)
# define USE_TRACK_MOUSE
#endif // NO_TRACK_MOUSE

static Fl_Window *track_mouse_win = 0; // current TrackMouseEvent() window

// USE_CAPTURE_MOUSE_WIN - this must be defined for TrackMouseEvent to work
// correctly with subwindows - otherwise a single mouse click and release
// (without a move) would generate phantom leave events.
// This defines, if the current mouse window (maybe a subwindow) or the
// main window should get mouse events after pushing (and holding) a mouse
// button, i.e. when dragging the mouse. This is done by calling SetCapture
// (see below).

#ifdef USE_TRACK_MOUSE
#define USE_CAPTURE_MOUSE_WIN
#endif // USE_TRACK_MOUSE

//
// WM_SYNCPAINT is an "undocumented" message, which is finally defined in
// VC++ 6.0.
//

#ifndef WM_SYNCPAINT
#  define WM_SYNCPAINT 0x0088
#endif

#ifndef WM_MOUSELEAVE
#  define WM_MOUSELEAVE 0x02a3
#endif

#ifndef WM_MOUSEWHEEL
#  define WM_MOUSEWHEEL 0x020a
#endif

#ifndef WHEEL_DELTA
#  define WHEEL_DELTA 120       // according to MSDN.
#endif

// This is only defined on Vista and upwards...
#ifndef WM_MOUSEHWHEEL
#  define WM_MOUSEHWHEEL 0x020E
#endif

#ifndef SM_CXPADDEDBORDER
#  define SM_CXPADDEDBORDER (92) // STR #3061
#endif

// https://msdn.microsoft.com/en-us/library/windows/desktop/dn312083(v=vs.85).aspx
#ifndef WM_DPICHANGED
#  define WM_DPICHANGED 0x02E0
#endif

//
// WM_FLSELECT is the user-defined message that we get when one of
// the sockets has pending data, etc.
//

#define WM_FLSELECT (WM_APP + 1) // WM_APP is used for hide-window


////////////////////////////////////////////////////////////////
// interface to poll/select call:

// fd's are only implemented for sockets.  Microsoft Windows does not
// have a unified IO system, so it doesn't support select() on files,
// devices, or pipes...
//
// Microsoft provides the Berkeley select() call and an asynchronous
// select function that sends a Windows message when the select condition
// exists. However, we don't try to use the asynchronous WSAAsyncSelect()
// any more for good reasons (see above).
//
// A.S. Dec 2009: We got reports that current winsock2.h files define
// POLLIN, POLLOUT, and POLLERR with conflicting values WRT what we
// used before (STR #2301).  Therefore we must not use these values
// for our internal purposes, but use FL_READ, FL_WRITE, and
// FL_EXCEPT, as defined for use in Fl::add_fd().
//
static int maxfd = 0;
static fd_set fdsets[3];

extern IDropTarget *flIDropTarget;

static int nfds = 0;
static int fd_array_size = 0;
static struct FD {
  int fd;
  short events;
  void (*cb)(FL_SOCKET, void *); // keep socket api opaque at this level to reduce multiplatform deps headaches
  void *arg;
} *fd = 0;

extern unsigned int fl_codepage;

void Fl_WinAPI_System_Driver::add_fd(int n, int events, void (*cb)(FL_SOCKET, void *), void *v) {
  remove_fd(n, events);
  int i = nfds++;
  if (i >= fd_array_size) {
    fd_array_size = 2 * fd_array_size + 1;
    fd = (FD *)realloc(fd, fd_array_size * sizeof(FD));
  }
  fd[i].fd = n;
  fd[i].events = (short)events;
  fd[i].cb = cb;
  fd[i].arg = v;

  if (events & FL_READ)
    FD_SET((unsigned)n, &fdsets[0]);
  if (events & FL_WRITE)
    FD_SET((unsigned)n, &fdsets[1]);
  if (events & FL_EXCEPT)
    FD_SET((unsigned)n, &fdsets[2]);
  if (n > maxfd)
    maxfd = n;
}

void Fl_WinAPI_System_Driver::add_fd(int fd, void (*cb)(FL_SOCKET, void *), void *v) {
  add_fd(fd, FL_READ, cb, v);
}

void Fl_WinAPI_System_Driver::remove_fd(int n, int events) {
  int i, j;
  for (i = j = 0; i < nfds; i++) {
    if (fd[i].fd == n) {
      short e = fd[i].events & ~events;
      if (!e)
        continue; // if no events left, delete this fd
      fd[i].events = e;
    }
    // move it down in the array if necessary:
    if (j < i) {
      fd[j] = fd[i];
    }
    j++;
  }
  nfds = j;

  if (events & FL_READ)
    FD_CLR(unsigned(n), &fdsets[0]);
  if (events & FL_WRITE)
    FD_CLR(unsigned(n), &fdsets[1]);
  if (events & FL_EXCEPT)
    FD_CLR(unsigned(n), &fdsets[2]);
}

void Fl_WinAPI_System_Driver::remove_fd(int n) {
  remove_fd(n, -1);
}

// these pointers are set by the Fl::lock() function:
static void nothing() {}
void (*fl_lock_function)() = nothing;
void (*fl_unlock_function)() = nothing;

static void *thread_message_;
void *Fl_WinAPI_System_Driver::thread_message() {
  void *r = thread_message_;
  thread_message_ = 0;
  return r;
}

extern int fl_send_system_handlers(void *e);

MSG fl_msg;

// A local helper function to flush any pending callback requests
// from the awake ring-buffer
static void process_awake_handler_requests(void) {
  Fl_Awake_Handler func;
  void *data;
  while (Fl::get_awake_handler_(func, data) == 0) {
    func(data);
  }
}

// This is never called with time_to_wait < 0.0.
// It *should* return negative on error, 0 if nothing happens before
// timeout, and >0 if any callbacks were done.  This version
// always returns 1.
double Fl_WinAPI_System_Driver::wait(double time_to_wait) {

  time_to_wait = Fl_System_Driver::wait(time_to_wait);

  int have_message = 0;

  if (nfds) {
    // For Windows we need to poll for socket input FIRST, since
    // the event queue is not something we can select() on...
    timeval t;
    t.tv_sec = 0;
    t.tv_usec = 0;

    fd_set fdt[3];
    memcpy(fdt, fdsets, sizeof fdt); // one shot faster fdt init
    if (select(maxfd + 1, &fdt[0], &fdt[1], &fdt[2], &t)) {
      // We got something - do the callback!
      for (int i = 0; i < nfds; i++) {
        SOCKET f = fd[i].fd;
        short revents = 0;
        if (FD_ISSET(f, &fdt[0]))
          revents |= FL_READ;
        if (FD_ISSET(f, &fdt[1]))
          revents |= FL_WRITE;
        if (FD_ISSET(f, &fdt[2]))
          revents |= FL_EXCEPT;
        if (fd[i].events & revents)
          fd[i].cb(f, fd[i].arg);
      }
      time_to_wait = 0.0; // just peek for any messages
    } else {
      // we need to check them periodically, so set a short timeout:
      if (time_to_wait > .001)
        time_to_wait = .001;
    }
  }

  if (Fl::idle || Fl::damage())
    time_to_wait = 0.0;

  // if there are no more windows and this timer is set
  // to FOREVER, continue through or look up indefinitely
  if (!Fl::first_window() && time_to_wait == 1e20)
    time_to_wait = 0.0;

  fl_unlock_function();

  time_to_wait = (time_to_wait > 10000 ? 10000 : time_to_wait);

  time_to_wait = Fl_Timeout::time_to_wait(time_to_wait);

  int t_msec = (int)(time_to_wait * 1000.0 + 0.5);
  MsgWaitForMultipleObjects(0, NULL, FALSE, t_msec, QS_ALLINPUT);

  fl_lock_function();

  // Execute the message we got, and all other pending messages:
  // have_message = PeekMessage(&fl_msg, NULL, 0, 0, PM_REMOVE);
  while ((have_message = PeekMessageW(&fl_msg, NULL, 0, 0, PM_REMOVE)) > 0) {
    if (fl_send_system_handlers(&fl_msg))
      continue;

    // Let applications treat WM_QUIT identical to SIGTERM on *nix
    if (fl_msg.message == WM_QUIT)
      raise(SIGTERM);

    if (fl_msg.message == fl_wake_msg) {
      // Used for awaking wait() from another thread
      thread_message_ = (void *)fl_msg.wParam;
      process_awake_handler_requests();
    }

    TranslateMessage(&fl_msg);
    DispatchMessageW(&fl_msg);
  }

  // The following conditional test: !Fl_System_Driver::awake_ring_empty()
  //  equivalent to:
  //    (Fl::awake_ring_head_ != Fl::awake_ring_tail_)
  // is a workaround / fix for STR #3143. This works, but a better solution
  // would be to understand why the PostThreadMessage() messages are not
  // seen by the main window if it is being dragged/ resized at the time.
  // If a worker thread posts an awake callback to the ring buffer
  // whilst the main window is unresponsive (if a drag or resize operation
  // is in progress) we may miss the PostThreadMessage(). So here, we check if
  // there is anything pending in the awake ring buffer and if so process
  // it. This is not strictly thread safe (for speed it compares the head
  // and tail indices without first locking the ring buffer) but is intended
  // only as a fall-back recovery mechanism if the awake processing stalls.
  // If the test erroneously returns true (may happen if we test the indices
  // whilst they are being modified) we will call process_awake_handler_requests()
  // unnecessarily, but this has no harmful consequences so is safe to do.
  // Note also that if we miss the PostThreadMessage(), then thread_message_
  // will not be updated, so this is not a perfect solution, but it does
  // recover and process any pending awake callbacks.
  // Normally the ring buffer head and tail indices will match and this
  // comparison will do nothing. Addresses STR #3143
  if (!Fl_System_Driver::awake_ring_empty()) {
    process_awake_handler_requests();
  }

  Fl::flush();

  // This should return 0 if only timer events were handled:
  return 1;
}

// just like Fl_WinAPI_System_Driver::wait(0.0) except no callbacks are done:
int Fl_WinAPI_System_Driver::ready() {
  if (PeekMessage(&fl_msg, NULL, 0, 0, PM_NOREMOVE))
    return 1;
  if (!nfds)
    return 0;
  timeval t;
  t.tv_sec = 0;
  t.tv_usec = 0;
  fd_set fdt[3];
  memcpy(fdt, fdsets, sizeof fdt);
  return select(0, &fdt[0], &fdt[1], &fdt[2], &t);
}

static void delayed_create_print_window(void *) {
  Fl::remove_check(delayed_create_print_window);
  fl_create_print_window();
}

void Fl_WinAPI_Screen_Driver::open_display_platform() {
  static char beenHereDoneThat = 0;

  if (beenHereDoneThat)
    return;

  beenHereDoneThat = 1;
  // test whether DpiAwareness has been set before via a manifest
  /*enum PROCESS_DPI_AWARENESS { // in shellscalingapi.h from Window 8.1
    PROCESS_DPI_UNAWARE,
    PROCESS_SYSTEM_DPI_AWARE,
    PROCESS_PER_MONITOR_DPI_AWARE
  };*/
  typedef HRESULT(WINAPI * GetProcessDpiAwareness_type)(HANDLE, int *);
  GetProcessDpiAwareness_type fl_GetProcessDpiAwareness =
      (GetProcessDpiAwareness_type)GetProcAddress(LoadLibrary("Shcore.DLL"), "GetProcessDpiAwareness");
  int awareness;
  if (!fl_GetProcessDpiAwareness || fl_GetProcessDpiAwareness(NULL, &awareness) != S_OK) {
    awareness = 0; //corresponds to PROCESS_DPI_UNAWARE;
  }
  if (awareness == 2 /*PROCESS_PER_MONITOR_DPI_AWARE*/) is_dpi_aware = true;
  if (awareness == 0 /*PROCESS_DPI_UNAWARE*/) { // DpiAwareness has not been set via a manifest
    typedef void *fl_DPI_AWARENESS_CONTEXT;
    typedef BOOL(WINAPI * SetProcessDpiAwarenessContext_type)(fl_DPI_AWARENESS_CONTEXT);
    SetProcessDpiAwarenessContext_type fl_SetProcessDpiAwarenessContext =
    (SetProcessDpiAwarenessContext_type)GetProcAddress(LoadLibrary("User32.DLL"), "SetProcessDpiAwarenessContext");
    if (fl_SetProcessDpiAwarenessContext) {
      const fl_DPI_AWARENESS_CONTEXT fl_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = (fl_DPI_AWARENESS_CONTEXT)(-4);
      is_dpi_aware = fl_SetProcessDpiAwarenessContext(fl_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    }
    if (!is_dpi_aware) {
      typedef HRESULT(WINAPI * SetProcessDpiAwareness_type)(int);
      SetProcessDpiAwareness_type fl_SetProcessDpiAwareness =
      (SetProcessDpiAwareness_type)GetProcAddress(LoadLibrary("Shcore.DLL"), "SetProcessDpiAwareness");
      if (fl_SetProcessDpiAwareness) {
        const int fl_PROCESS_PER_MONITOR_DPI_AWARE = 2;
        if (fl_SetProcessDpiAwareness(fl_PROCESS_PER_MONITOR_DPI_AWARE) == S_OK) is_dpi_aware = true;
      }
    }
  }
  OleInitialize(0L);
  get_imm_module();
  Fl::add_check(delayed_create_print_window);
}


void Fl_WinAPI_Screen_Driver::update_scaling_capability() {
  scaling_capability = SYSTEMWIDE_APP_SCALING;
  for (int ns = 1; ns < screen_count(); ns++) {
    if (scale(ns) != scale(0)) {
      scaling_capability = PER_SCREEN_APP_SCALING;
      break;
    }
  }
}

void Fl_WinAPI_Screen_Driver::desktop_scale_factor() {
  typedef HRESULT(WINAPI * GetDpiForMonitor_type)(HMONITOR, int, UINT *, UINT *);
  typedef HMONITOR(WINAPI * MonitorFromRect_type)(LPCRECT, DWORD);
  GetDpiForMonitor_type fl_GetDpiForMonitor = NULL;
  MonitorFromRect_type fl_MonitorFromRect = NULL;
  if (is_dpi_aware) {
      fl_GetDpiForMonitor = (GetDpiForMonitor_type)GetProcAddress(LoadLibrary("Shcore.DLL"), "GetDpiForMonitor");
      if (fl_GetDpiForMonitor)
        fl_MonitorFromRect = (MonitorFromRect_type)GetProcAddress(LoadLibrary("User32.DLL"), "MonitorFromRect");
    }
  for (int ns = 0; ns < screen_count(); ns++) {
    UINT dpiX, dpiY;
    HRESULT r = E_INVALIDARG;
    if (fl_GetDpiForMonitor && fl_MonitorFromRect) {
       HMONITOR hm = fl_MonitorFromRect(&screens[ns], MONITOR_DEFAULTTONEAREST);
       r =  fl_GetDpiForMonitor(hm, 0, &dpiX, &dpiY);
    }
    if (r != S_OK) { dpiX = dpiY = 96; }
    dpi[ns][0] = float(dpiX);
    dpi[ns][1] = float(dpiY);
    scale(ns, dpiX / 96.f);
    // fprintf(LOG, "desktop_scale_factor ns=%d factor=%.2f dpi=%.1f\n", ns, scale(ns), dpi[ns][0]);
  }
  update_scaling_capability();
}


class Fl_Win32_At_Exit {
public:
  Fl_Win32_At_Exit() {}
  ~Fl_Win32_At_Exit() {
    fl_free_fonts(); // do some Windows cleanup
    fl_cleanup_pens();
    OleUninitialize();
    if (fl_graphics_driver) fl_brush_action(1);
    fl_cleanup_dc_list();
    // This is actually too late in the cleanup process to remove the
    // clipboard notifications, but we have no earlier hook so we try
    // to work around it anyway.
    if (clipboard_wnd != NULL)
      fl_clipboard_notify_untarget(clipboard_wnd);
#if USE_GDIPLUS
    Fl_GDIplus_Graphics_Driver::shutdown();
#endif
  }
};
static Fl_Win32_At_Exit win32_at_exit;

static char im_enabled = 1;

void Fl_WinAPI_Screen_Driver::enable_im() {
  open_display();

  Fl_X *i = Fl_X::first;
  while (i) {
    flImmAssociateContextEx((HWND)i->xid, 0, IACE_DEFAULT);
    i = i->next;
  }

  im_enabled = 1;
}

void Fl_WinAPI_Screen_Driver::disable_im() {
  open_display();

  Fl_X *i = Fl_X::first;
  while (i) {
    flImmAssociateContextEx((HWND)i->xid, 0, 0);
    i = i->next;
  }

  im_enabled = 0;
}

void Fl_WinAPI_Screen_Driver::set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win)
{
  if (!win) return;
  Fl_Window* tw = win->top_window();

  if (!tw->shown())
    return;

  HIMC himc = flImmGetContext(fl_xid(tw));

  if (himc) {
    COMPOSITIONFORM cfs;
    float s = Fl_Graphics_Driver::default_driver().scale();
    cfs.dwStyle = CFS_POINT;
    cfs.ptCurrentPos.x = int(X * s);
    cfs.ptCurrentPos.y = int(Y * s) - int(tw->labelsize() * s);
    // Attempt to have temporary text entered by input method use scaled font.
    // Does good, but still not always effective.
    Fl_GDI_Font_Descriptor *desc = (Fl_GDI_Font_Descriptor*)Fl_Graphics_Driver::default_driver().font_descriptor();
    if (desc) SelectObject((HDC)Fl_Graphics_Driver::default_driver().gc(), desc->fid);
    MapWindowPoints(fl_xid(win), fl_xid(tw), &cfs.ptCurrentPos, 1);
    flImmSetCompositionWindow(himc, &cfs);
    flImmReleaseContext(fl_xid(tw), himc);
  }
}


////////////////////////////////////////////////////////////////

int Fl_WinAPI_Screen_Driver::get_mouse_unscaled(int &mx, int &my) {
  POINT p;
  GetCursorPos(&p);
  mx = p.x;
  my = p.y;
  int screen = screen_num_unscaled(mx, my);
  return screen >= 0 ? screen : 0;
}

int Fl_WinAPI_Screen_Driver::get_mouse(int &x, int &y) {
  int n = get_mouse_unscaled(x, y);
  float s = scale(n);
  x = int(x / s);
  y = int(y / s);
  return n;
}

////////////////////////////////////////////////////////////////
// code used for selections:

char *fl_selection_buffer[2];
int fl_selection_length[2];
int fl_selection_buffer_length[2];
char fl_i_own_selection[2];

UINT fl_get_lcid_codepage(LCID id) {
  char buf[8];
  buf[GetLocaleInfo(id, LOCALE_IDEFAULTANSICODEPAGE, buf, 8)] = 0;
  return atol(buf);
}

// Convert \n -> \r\n
class Lf2CrlfConvert {
  char *out;
  int outlen;

public:
  Lf2CrlfConvert(const char *in, int inlen) {
    outlen = 0;
    const char *i;
    char *o;
    int lencount;
    // Predict size of \r\n conversion buffer
    for (i = in, lencount = inlen; lencount > 0; lencount--) {
      if (*i == '\r' && *(i + 1) == '\n' && lencount >= 2) { // leave \r\n untranslated
        i += 2;
        outlen += 2;
        lencount--;
      } else if (*i == '\n') { // \n by itself? leave room to insert \r
        i++;
        outlen += 2;
      } else {
        ++i;
        ++outlen;
      }
    }
    // Alloc conversion buffer + NULL
    out = new char[outlen + 1];
    // Handle \n -> \r\n conversion
    for (i = in, o = out, lencount = inlen; lencount > 0; lencount--) {
      if (*i == '\r' && *(i + 1) == '\n' && lencount >= 2) { // leave \r\n untranslated
        *o++ = *i++;
        *o++ = *i++;
        lencount--;
      } else if (*i == '\n') { // \n by itself? insert \r
        *o++ = '\r';
        *o++ = *i++;
      } else {
        *o++ = *i++;
      }
    }
    *o++ = 0;
  }
  ~Lf2CrlfConvert() {
    delete[] out;
  }
  int GetLength() const { return (outlen); }
  const char *GetValue() const { return (out); }
};

void fl_update_clipboard(void) {
  Fl_Window *w1 = Fl::first_window();
  if (!w1)
    return;

  HWND hwnd = fl_xid(w1);

  if (!OpenClipboard(hwnd))
    return;

  EmptyClipboard();

  int utf16_len = fl_utf8toUtf16(fl_selection_buffer[1], fl_selection_length[1], 0, 0);

  HGLOBAL hMem = GlobalAlloc(GHND, utf16_len * 2 + 2); // moveable and zero'ed mem alloc.
  LPVOID memLock = GlobalLock(hMem);

  fl_utf8toUtf16(fl_selection_buffer[1], fl_selection_length[1], (unsigned short *)memLock, utf16_len + 1);

  GlobalUnlock(hMem);
  SetClipboardData(CF_UNICODETEXT, hMem);

  CloseClipboard();

  // In case Windows managed to lob of a WM_DESTROYCLIPBOARD during
  // the above.
  fl_i_own_selection[1] = 1;
}

// call this when you create a selection:
void Fl_WinAPI_Screen_Driver::copy(const char *stuff, int len, int clipboard, const char *type) {
  if (!stuff || len < 0)
    return;
  if (clipboard >= 2)
    clipboard = 1; // Only on X11 do multiple clipboards make sense.

  // Convert \n -> \r\n (for old apps like Notepad, DOS)
  Lf2CrlfConvert buf(stuff, len);
  len = buf.GetLength();
  stuff = buf.GetValue();

  if (len + 1 > fl_selection_buffer_length[clipboard]) {
    delete[] fl_selection_buffer[clipboard];
    fl_selection_buffer[clipboard] = new char[len + 100];
    fl_selection_buffer_length[clipboard] = len + 100;
  }
  memcpy(fl_selection_buffer[clipboard], stuff, len);
  fl_selection_buffer[clipboard][len] = 0; // needed for direct paste
  fl_selection_length[clipboard] = len;
  fl_i_own_selection[clipboard] = 1;
  if (clipboard)
    fl_update_clipboard();
}

// Call this when a "paste" operation happens:
void Fl_WinAPI_Screen_Driver::paste(Fl_Widget &receiver, int clipboard, const char *type) {
  if (!clipboard || (fl_i_own_selection[clipboard] && strcmp(type, Fl::clipboard_plain_text) == 0)) {
    // We already have it, do it quickly without window server.
    // Notice that the text is clobbered if set_selection is
    // called in response to FL_PASTE!
    char *i = fl_selection_buffer[clipboard];
    if (i == 0L) {
      Fl::e_text = 0;
      return;
    }
    char *clip_text = new char[fl_selection_length[clipboard] + 1];
    char *o = clip_text;
    while (*i) { // Convert \r\n -> \n
      if (*i == '\r' && *(i + 1) == '\n')
        i++;
      else
        *o++ = *i++;
    }
    *o = 0;
    Fl::e_text = clip_text;
    Fl::e_length = (int)(o - Fl::e_text);
    Fl::e_clipboard_type = Fl::clipboard_plain_text;
    receiver.handle(FL_PASTE); // this may change Fl::e_text
    delete[] clip_text;
    Fl::e_text = 0;
  } else if (clipboard) {
    HANDLE h;
    if (!OpenClipboard(NULL))
      return;
    if (strcmp(type, Fl::clipboard_plain_text) == 0) { // we want plain text from clipboard
      if ((h = GetClipboardData(CF_UNICODETEXT))) {    // there's text in the clipboard
        wchar_t *memLock = (wchar_t *)GlobalLock(h);
        size_t utf16_len = wcslen(memLock);
        char *clip_text = new char[utf16_len * 4 + 1];
        unsigned utf8_len = fl_utf8fromwc(clip_text, (unsigned)(utf16_len * 4), memLock, (unsigned)utf16_len);
        *(clip_text + utf8_len) = 0;
        GlobalUnlock(h);
        LPSTR a, b;
        a = b = clip_text;
        while (*a) { // strip the CRLF pairs ($%$#@^)
          if (*a == '\r' && a[1] == '\n')
            a++;
          else
            *b++ = *a++;
        }
        *b = 0;
        Fl::e_text = clip_text;
        Fl::e_length = (int)(b - Fl::e_text);
        Fl::e_clipboard_type = Fl::clipboard_plain_text; // indicates that the paste event is for plain UTF8 text
        receiver.handle(FL_PASTE);                       // send the FL_PASTE event to the widget. May change Fl::e_text
        delete[] clip_text;
        Fl::e_text = 0;
      }
    } else if (strcmp(type, Fl::clipboard_image) == 0) { // we want an image from clipboard
      uchar *rgb = NULL;
      Fl_RGB_Image *image = NULL;
      int width = 0, height = 0, depth = 0;
      if ((h = GetClipboardData(CF_DIB))) { // if there's a DIB in clipboard
        LPBITMAPINFO lpBI = (LPBITMAPINFO)GlobalLock(h);
        width = lpBI->bmiHeader.biWidth; // bitmap width & height
        height = lpBI->bmiHeader.biHeight; // is < 0 for top-down DIB
        if ((lpBI->bmiHeader.biBitCount == 24 || lpBI->bmiHeader.biBitCount == 32) &&
            lpBI->bmiHeader.biCompression == BI_RGB &&
            lpBI->bmiHeader.biClrUsed == 0) {      // direct use of the DIB data if it's RGB or RGBA
          int linewidth;                           // row length
          depth = lpBI->bmiHeader.biBitCount / 8;  // 3 or 4
          if (depth == 3)
            linewidth = 4 * ((3 * width + 3) / 4); // row length: series of groups of 3 bytes, rounded to multiple of 4 bytes
          else
            linewidth = 4 * width;
          rgb = new uchar[width * abs(height) * depth]; // will hold the image data
          uchar *p = rgb, *r, rr, gg, bb;
          int step = (height > 0 ? -1 : +1);
          int from = (height > 0 ? height-1 : 0);
          int to = (height > 0 ? 0 : -height-1);
          for (int i = from; (height > 0 ? i>=to : i <=to); i += step) {// for each row, from last to first
            r = (uchar *)(lpBI->bmiColors) + i * linewidth; // beginning of pixel data for the ith row
            for (int j = 0; j < width; j++) {               // for each pixel in a row
              bb = *r++;                                    // BGR is in DIB
              gg = *r++;
              rr = *r++;
              *p++ = rr; // we want RGB
              *p++ = gg;
              *p++ = bb;
              if (depth == 4)
                *p++ = *r++; // copy alpha if present
            }
          }
        } else { // the system will decode a complex DIB
          void *pDIBBits = (void *)(lpBI->bmiColors + 256);
          if (lpBI->bmiHeader.biCompression == BI_BITFIELDS)
            pDIBBits = (void *)(lpBI->bmiColors + 3);
          else if (lpBI->bmiHeader.biClrUsed > 0)
            pDIBBits = (void *)(lpBI->bmiColors + lpBI->bmiHeader.biClrUsed);
          Fl_Image_Surface *surf = new Fl_Image_Surface(width, abs(height));
          Fl_Surface_Device::push_current(surf);
          SetDIBitsToDevice((HDC)fl_graphics_driver->gc(), 0, 0, width, abs(height), 0, 0, 0, abs(height), pDIBBits, lpBI, DIB_RGB_COLORS);
          rgb = fl_read_image(NULL, 0, 0, width, abs(height));
          depth = 3;
          Fl_Surface_Device::pop_current();
          delete surf;
        }
        GlobalUnlock(h);
      } else if ((h = GetClipboardData(CF_ENHMETAFILE))) { // if there's an enhanced metafile in clipboard
        ENHMETAHEADER header;
        GetEnhMetaFileHeader((HENHMETAFILE)h, sizeof(header), &header); // get structure containing metafile dimensions
        width = (header.rclFrame.right - header.rclFrame.left + 1);     // in .01 mm units
        height = (header.rclFrame.bottom - header.rclFrame.top + 1);
        HDC hdc = GetDC(NULL); // get unit correspondance between .01 mm and screen pixels
        int hmm = GetDeviceCaps(hdc, HORZSIZE);
        int hdots = GetDeviceCaps(hdc, HORZRES);
        ReleaseDC(NULL, hdc);
        float factor = (100.f * hmm) / hdots;
        float scaling = Fl::screen_driver()->scale(Fl_Window_Driver::driver(receiver.top_window())->screen_num());
        if (!Fl_Window::current()) {
          Fl_GDI_Graphics_Driver *d = (Fl_GDI_Graphics_Driver*)&Fl_Graphics_Driver::default_driver();
          d->scale(scaling);// may run early at app startup before Fl_Window::make_current() scales d
        }
        width = int(width / (scaling * factor)); // convert to screen pixel unit
        height = int(height / (scaling * factor));
        RECT rect = {0, 0, width, height};
        Fl_Image_Surface *surf = new Fl_Image_Surface(width, height, 1);
        Fl_Surface_Device::push_current(surf);
        fl_color(FL_WHITE);             // draw white background
        fl_rectf(0, 0, width, height);
        rect.right = LONG(rect.right * scaling);          // apply scaling to the metafile draw operation
        rect.bottom = LONG(rect.bottom * scaling);
        PlayEnhMetaFile((HDC)fl_graphics_driver->gc(), (HENHMETAFILE)h, &rect); // draw metafile to offscreen buffer
        image = surf->image();
        Fl_Surface_Device::pop_current();
        delete surf;
      }
      if (rgb || image) {
        if (!image) {
          image = new Fl_RGB_Image(rgb, width, abs(height), depth); // create new image from pixel data
          image->alloc_array = 1;
        }
        Fl::e_clipboard_data = image;
        Fl::e_clipboard_type = Fl::clipboard_image; // indicates that the paste event is for image data
        int done = receiver.handle(FL_PASTE);       // send FL_PASTE event to widget
        Fl::e_clipboard_type = "";
        if (done == 0) { // if widget did not handle the event, delete the image
          Fl::e_clipboard_data = NULL;
          delete image;
        }
      }
    }
    CloseClipboard();
  }
}

int Fl_WinAPI_Screen_Driver::clipboard_contains(const char *type) {
  int retval = 0;
  if (!OpenClipboard(NULL))
    return 0;
  if (strcmp(type, Fl::clipboard_plain_text) == 0 || type[0] == 0) {
    retval = IsClipboardFormatAvailable(CF_UNICODETEXT);
  } else if (strcmp(type, Fl::clipboard_image) == 0) {
    retval = IsClipboardFormatAvailable(CF_DIB) || IsClipboardFormatAvailable(CF_ENHMETAFILE);
  }
  CloseClipboard();
  return retval;
}

static void fl_clipboard_notify_target(HWND wnd) {
  if (clipboard_wnd)
    return;

  // We get one fake WM_DRAWCLIPBOARD immediately, which we therefore
  // need to ignore.
  initial_clipboard = true;

  clipboard_wnd = wnd;
  next_clipboard_wnd = SetClipboardViewer(wnd);
}

static void fl_clipboard_notify_untarget(HWND wnd) {
  if (wnd != clipboard_wnd)
    return;

  // We might be called late in the cleanup where Windows has already
  // implicitly destroyed our clipboard window. At that point we need
  // to do some extra work to manually repair the clipboard chain.
  if (IsWindow(wnd))
    ChangeClipboardChain(wnd, next_clipboard_wnd);
  else {
    HWND tmp, head;

    tmp = CreateWindow("STATIC", "Temporary FLTK Clipboard Window", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
    if (tmp == NULL)
      return;

    head = SetClipboardViewer(tmp);
    if (head == NULL)
      ChangeClipboardChain(tmp, next_clipboard_wnd);
    else {
      SendMessage(head, WM_CHANGECBCHAIN, (WPARAM)wnd, (LPARAM)next_clipboard_wnd);
      ChangeClipboardChain(tmp, head);
    }

    DestroyWindow(tmp);
  }

  clipboard_wnd = next_clipboard_wnd = 0;
}

void fl_clipboard_notify_retarget(HWND wnd) {
  // The given window is getting destroyed. If it's part of the
  // clipboard chain then we need to unregister it and find a
  // replacement window.
  if (wnd != clipboard_wnd)
    return;

  fl_clipboard_notify_untarget(wnd);

  if (Fl::first_window())
    fl_clipboard_notify_target(fl_xid(Fl::first_window()));
}

void Fl_WinAPI_Screen_Driver::clipboard_notify_change() {
  // untarget clipboard monitor if no handlers are registered
  if (clipboard_wnd != NULL && fl_clipboard_notify_empty()) {
    fl_clipboard_notify_untarget(clipboard_wnd);
    return;
  }

  // if there are clipboard notify handlers but no window targeted
  // target first window if available
  if (clipboard_wnd == NULL && Fl::first_window())
    fl_clipboard_notify_target(fl_xid(Fl::first_window()));
}

////////////////////////////////////////////////////////////////
void fl_get_codepage() {
  HKL hkl = GetKeyboardLayout(0);
  TCHAR ld[8];

  GetLocaleInfo(LOWORD(hkl), LOCALE_IDEFAULTANSICODEPAGE, ld, 6);
  DWORD ccp = atol(ld);
  fl_codepage = ccp;
}

HWND fl_capture;

static int mouse_event(Fl_Window *window, int what, int button,
                       WPARAM wParam, LPARAM lParam) {
  static int px, py, pmx, pmy;
  POINT pt;
  float scale = Fl::screen_driver()->scale(window->screen_num());
  Fl::e_x = pt.x = (signed short)LOWORD(lParam);
  Fl::e_y = pt.y = (signed short)HIWORD(lParam);
  Fl::e_x = int(Fl::e_x / scale);
  Fl::e_y = int(Fl::e_y / scale);
  ClientToScreen(fl_xid(window), &pt);
  Fl::e_x_root = int(pt.x / scale);
  Fl::e_y_root = int(pt.y / scale);
#ifdef USE_CAPTURE_MOUSE_WIN
  Fl_Window *mouse_window = window; // save "mouse window"
#endif
  while (window->parent()) {
    Fl::e_x += window->x();
    Fl::e_y += window->y();
    window = window->window();
  }

  ulong state = Fl::e_state & 0xff0000; // keep shift key states
#if 0
  // mouse event reports some shift flags, perhaps save them?
  if (wParam & MK_SHIFT) state |= FL_SHIFT;
  if (wParam & MK_CONTROL) state |= FL_CTRL;
#endif
  if (wParam & MK_LBUTTON)  state |= FL_BUTTON1;  // left
  if (wParam & MK_MBUTTON)  state |= FL_BUTTON2;  // right
  if (wParam & MK_RBUTTON)  state |= FL_BUTTON3;  // middle
  if (wParam & MK_XBUTTON1) state |= FL_BUTTON4;  // side button 1 (back)
  if (wParam & MK_XBUTTON2) state |= FL_BUTTON5;  // side button 2 (forward)

  Fl::e_state = state;

  switch (what) {
    case 1: // double-click
      if (Fl::e_is_click) {
        Fl::e_clicks++;
        goto J1;
      }
    case 0: // single-click
      Fl::e_clicks = 0;
    J1:
#ifdef USE_CAPTURE_MOUSE_WIN
      if (!fl_capture)
        SetCapture(fl_xid(mouse_window)); // use mouse window
#else
      if (!fl_capture)
        SetCapture(fl_xid(window)); // use main window
#endif
      Fl::e_keysym = FL_Button + button;
      Fl::e_is_click = 1;
      px = pmx = Fl::e_x_root;
      py = pmy = Fl::e_y_root;
      return Fl::handle(FL_PUSH, window);

    case 2: // release:
      if (!fl_capture)
        ReleaseCapture();
      Fl::e_keysym = FL_Button + button;
      return Fl::handle(FL_RELEASE, window);

    case 3:  // move:
    default: // avoid compiler warning
      // Windows produces extra events even if the mouse does not move, ignore em:
      if (Fl::e_x_root == pmx && Fl::e_y_root == pmy)
        return 1;
      pmx = Fl::e_x_root;
      pmy = Fl::e_y_root;
      if (abs(Fl::e_x_root - px) > 5 || abs(Fl::e_y_root - py) > 5)
        Fl::e_is_click = 0;
      return Fl::handle(FL_MOVE, window);
  }
}

// Convert a Windows VK_x to an FLTK (X) Keysym:
// See also the inverse converter in Fl_get_key_win32.cxx
// This table is in numeric order by VK:
static const struct {
  unsigned short vk, fltk, extended;
} vktab[] = {
  {VK_BACK,     FL_BackSpace},
  {VK_TAB,      FL_Tab},
  {VK_CLEAR,    FL_KP+'5',      0xff0b/*XK_Clear*/},
  {VK_RETURN,   FL_Enter,       FL_KP_Enter},
  {VK_SHIFT,    FL_Shift_L,     FL_Shift_R},
  {VK_CONTROL,  FL_Control_L,   FL_Control_R},
  {VK_MENU,     FL_Alt_L,       FL_Alt_R},
  {VK_PAUSE,    FL_Pause},
  {VK_CAPITAL,  FL_Caps_Lock},
  {VK_ESCAPE,   FL_Escape},
  {VK_SPACE,    ' '},
  {VK_PRIOR,    FL_KP+'9',      FL_Page_Up},
  {VK_NEXT,     FL_KP+'3',      FL_Page_Down},
  {VK_END,      FL_KP+'1',      FL_End},
  {VK_HOME,     FL_KP+'7',      FL_Home},
  {VK_LEFT,     FL_KP+'4',      FL_Left},
  {VK_UP,       FL_KP+'8',      FL_Up},
  {VK_RIGHT,    FL_KP+'6',      FL_Right},
  {VK_DOWN,     FL_KP+'2',      FL_Down},
  {VK_SNAPSHOT, FL_Print},      // does not work on NT
  {VK_INSERT,   FL_KP+'0',      FL_Insert},
  {VK_DELETE,   FL_KP+'.',      FL_Delete},
  {VK_LWIN,     FL_Meta_L},
  {VK_RWIN,     FL_Meta_R},
  {VK_APPS,     FL_Menu},
  {VK_SLEEP, FL_Sleep},
  {VK_MULTIPLY, FL_KP+'*'},
  {VK_ADD,      FL_KP+'+'},
  {VK_SUBTRACT, FL_KP+'-'},
  {VK_DECIMAL,  FL_KP+'.'},
  {VK_DIVIDE,   FL_KP+'/'},
  {VK_NUMLOCK,  FL_Num_Lock},
  {VK_SCROLL,   FL_Scroll_Lock},
#if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0500)
  {VK_BROWSER_BACK,     FL_Back},
  {VK_BROWSER_FORWARD,  FL_Forward},
  {VK_BROWSER_REFRESH,  FL_Refresh},
  {VK_BROWSER_STOP,     FL_Stop},
  {VK_BROWSER_SEARCH,   FL_Search},
  {VK_BROWSER_FAVORITES, FL_Favorites},
  {VK_BROWSER_HOME,     FL_Home_Page},
  {VK_VOLUME_MUTE,      FL_Volume_Mute},
  {VK_VOLUME_DOWN,      FL_Volume_Down},
  {VK_VOLUME_UP,        FL_Volume_Up},
  {VK_MEDIA_NEXT_TRACK, FL_Media_Next},
  {VK_MEDIA_PREV_TRACK, FL_Media_Prev},
  {VK_MEDIA_STOP,       FL_Media_Stop},
  {VK_MEDIA_PLAY_PAUSE, FL_Media_Play},
  {VK_LAUNCH_MAIL,      FL_Mail},
#endif
  {0xba,        ';'},
  {0xbb,        '='},   // 0xbb == VK_OEM_PLUS (see #1086)
  {0xbc,        ','},
  {0xbd,        '-'},
  {0xbe,        '.'},
  {0xbf,        '/'},
  {0xc0,        '`'},
  {0xdb,        '['},
  {0xdc,        '\\'},
  {0xdd,        ']'},
  {0xde,        '\''},
  {VK_OEM_102,  FL_Iso_Key}
};
static int ms2fltk(WPARAM vk, int extended) {
  static unsigned short vklut[256];
  static unsigned short extendedlut[256];
  if (!vklut[1]) { // init the table
    unsigned int i;
    for (i = 0; i < 256; i++)
      vklut[i] = tolower(i);
    for (i = VK_F1; i <= VK_F16; i++)
      vklut[i] = i + (FL_F - (VK_F1 - 1));
    for (i = VK_NUMPAD0; i <= VK_NUMPAD9; i++)
      vklut[i] = i + (FL_KP + '0' - VK_NUMPAD0);
    for (i = 0; i < sizeof(vktab) / sizeof(*vktab); i++) {
      vklut[vktab[i].vk] = vktab[i].fltk;
      extendedlut[vktab[i].vk] = vktab[i].extended;
    }
    for (i = 0; i < 256; i++)
      if (!extendedlut[i])
        extendedlut[i] = vklut[i];
  }
  return extended ? extendedlut[vk] : vklut[vk];
}

#if USE_COLORMAP
extern HPALETTE fl_select_palette(void); // in fl_color_win32.cxx
#endif


static Fl_Window *resize_bug_fix;

extern void fl_save_pen(void);
extern void fl_restore_pen(void);

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

  // Copy the message to fl_msg so add_handler code can see it.
  // It is already there if this is called by DispatchMessage,
  // but not if Windows calls this directly.

  fl_msg.hwnd = hWnd;
  fl_msg.message = uMsg;
  fl_msg.wParam = wParam;
  fl_msg.lParam = lParam;
  // fl_msg.time = ???
  // fl_msg.pt = ???
  // fl_msg.lPrivate = ???

  Fl_Window *window = fl_find(hWnd);
  float scale = (window ? Fl::screen_driver()->scale(Fl_Window_Driver::driver(window)->screen_num()) : 1);

  if (window) {
    switch (uMsg) {

      case WM_DPICHANGED: { // 0x02E0, after display re-scaling and followed by WM_DISPLAYCHANGE
        if (is_dpi_aware && !Fl_WinAPI_Window_Driver::data_for_resize_window_between_screens_.busy) {
          RECT r, *lParam_rect = (RECT*)lParam;
          Fl_WinAPI_Screen_Driver *sd = (Fl_WinAPI_Screen_Driver*)Fl::screen_driver();
          int centerX = (lParam_rect->left + lParam_rect->right)/2;
          int centerY = (lParam_rect->top + lParam_rect->bottom)/2;
          int ns = sd->screen_num_unscaled(centerX, centerY);
          int old_ns = Fl_Window_Driver::driver(window)->screen_num();
          if (sd->dpi[ns][0] != HIWORD(wParam) && ns == old_ns) { // change DPI of a screen
            sd->dpi[ns][0] = sd->dpi[ns][1] = HIWORD(wParam);
            float f = HIWORD(wParam) / 96.f;
            GetClientRect(hWnd, &r);
            float old_f = float(r.right) / window->w();
            Fl::screen_driver()->scale(ns, f);
            Fl_Window_Driver::driver(window)->resize_after_scale_change(ns, old_f, f);
            sd->update_scaling_capability();
          } else if (ns != old_ns) {
            // jump window with Windows+Shift+L|R-arrow to other screen with other DPI
            float scale = Fl::screen_driver()->scale(ns);
            int bt, bx, by;
            Fl_WinAPI_Window_Driver *wdr = (Fl_WinAPI_Window_Driver*)Fl_Window_Driver::driver(window);
            wdr->border_width_title_bar_height(bx, by, bt);
            window->position(int(round(lParam_rect->left/scale)),
                                        int(round((lParam_rect->top + bt)/scale)));
            wdr->resize_after_scale_change(ns, scale, scale);
          }
        }
        return 0;
      }

      case WM_QUIT: // this should not happen?
        Fl::fatal("WM_QUIT message");

      case WM_CLOSE: // user clicked close box
        Fl::handle(FL_CLOSE, window);
        return 0;

      case WM_SYNCPAINT:
      case WM_NCPAINT:
      case WM_ERASEBKGND:
        // Andreas Weitl - WM_SYNCPAINT needs to be passed to DefWindowProc
        // so that Windows can generate the proper paint messages...
        // Similarly, WM_NCPAINT and WM_ERASEBKGND need this, too...
        break;

      case WM_PAINT: {
        HRGN R, R2;
        Fl_X *i = Fl_X::flx(window);
        Fl_Window_Driver::driver(window)->wait_for_expose_value = 0;
        char redraw_whole_window = false;
        if (!i->region && window->damage()) {
          // Redraw the whole window...
          i->region = CreateRectRgn(0, 0, window->w(), window->h());
          redraw_whole_window = true;
        }

        // We need to merge Windows' damage into FLTK's damage.
        R = CreateRectRgn(0, 0, 0, 0);
        int r = GetUpdateRgn(hWnd, R, 0);
        if (r == NULLREGION && !redraw_whole_window) {
          DeleteObject(R);
          break;
        }

        // convert i->region in FLTK units to R2 in drawing units
        R2 = Fl_GDI_Graphics_Driver::scale_region((HRGN)i->region, scale, NULL);

        RECT r_box;
        if (scale != 1 && GetRgnBox(R, &r_box) != NULLREGION) {
          // add de-scaled update region to i->region in FLTK units
          r_box.left = LONG(r_box.left / scale);
          r_box.right = LONG(r_box.right / scale);
          r_box.top = LONG(r_box.top / scale);
          r_box.bottom = LONG(r_box.bottom / scale);
          HRGN R3 = CreateRectRgn(r_box.left, r_box.top, r_box.right + 1, r_box.bottom + 1);
          if (!i->region) i->region = R3;
          else {
            CombineRgn((HRGN)i->region, (HRGN)i->region, R3, RGN_OR);
            DeleteObject(R3);
          }
        }
        if (R2) {
          // Also tell Windows that we are drawing someplace else as well...
          CombineRgn(R2, R2, R, RGN_OR);
          DeleteObject(R);
        } else {
          R2 = R;
        }
        if (window->type() == FL_DOUBLE_WINDOW)
          ValidateRgn(hWnd, 0);
        else {
          ValidateRgn(hWnd, R2);
        }

        if (scale != 1) DeleteObject(R2);

        window->clear_damage((uchar)(window->damage() | FL_DAMAGE_EXPOSE));
        // These next two statements should not be here, so that all update
        // is deferred until Fl::flush() is called during idle.  However Windows
        // apparently is very unhappy if we don't obey it and draw right now.
        // Very annoying!
        fl_GetDC(hWnd); // Make sure we have a DC for this window...
        fl_save_pen();
        Fl_Window_Driver::driver(window)->flush();
        fl_restore_pen();
        window->clear_damage();
        return 0;
      } // case WM_PAINT

      case WM_LBUTTONDOWN:
        mouse_event(window, 0, 1, wParam, lParam);
        return 0;
      case WM_LBUTTONDBLCLK:
        mouse_event(window, 1, 1, wParam, lParam);
        return 0;
      case WM_LBUTTONUP:
        mouse_event(window, 2, 1, wParam, lParam);
        return 0;
      case WM_MBUTTONDOWN:
        mouse_event(window, 0, 2, wParam, lParam);
        return 0;
      case WM_MBUTTONDBLCLK:
        mouse_event(window, 1, 2, wParam, lParam);
        return 0;
      case WM_MBUTTONUP:
        mouse_event(window, 2, 2, wParam, lParam);
        return 0;
      case WM_RBUTTONDOWN:
        mouse_event(window, 0, 3, wParam, lParam);
        return 0;
      case WM_RBUTTONDBLCLK:
        mouse_event(window, 1, 3, wParam, lParam);
        return 0;
      case WM_RBUTTONUP:
        mouse_event(window, 2, 3, wParam, lParam);
        return 0;
      case WM_XBUTTONDOWN: {
        int xbutton = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 4 : 5;
        mouse_event(window, 0, xbutton, wParam, lParam);
        return 0;
      }
      case WM_XBUTTONDBLCLK: {
        int xbutton = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 4 : 5;
        mouse_event(window, 1, xbutton, wParam, lParam);
        return 0;
      }
      case WM_XBUTTONUP: {
        int xbutton = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 4 : 5;
        mouse_event(window, 2, xbutton, wParam, lParam);
        return 0;
      }

      case WM_MOUSEMOVE:
#ifdef USE_TRACK_MOUSE
        if (track_mouse_win != window) {
          TRACKMOUSEEVENT tme;
          tme.cbSize    = sizeof(TRACKMOUSEEVENT);
          tme.dwFlags   = TME_LEAVE;
          tme.hwndTrack = hWnd;
          _TrackMouseEvent(&tme);
          track_mouse_win = window;
        }
#endif  // USE_TRACK_MOUSE
        mouse_event(window, 3, 0, wParam, lParam);
        return 0;

      case WM_MOUSELEAVE:
        if (track_mouse_win == window) { // we left the top level window !
          Fl_Window *tw = window;
          while (tw->parent()) // find top level window
            tw = tw->window();
          Fl::belowmouse(0);
          Fl::handle(FL_LEAVE, tw);
        }
        track_mouse_win = 0; // force TrackMouseEvent() restart
        break;

      case WM_SETFOCUS:
        if ((Fl::modal_) && (Fl::modal_ != window)) {
          SetFocus(fl_xid(Fl::modal_));
          return 0;
        }
        Fl::handle(FL_FOCUS, window);
        break;

      case WM_KILLFOCUS:
        if (Fl::grab() && (Fl::grab() != window) && Fl::grab()->menu_window()) {
          // simulate click at remote location (see issue #1166)
          mouse_event(Fl::grab(), 0, 1, MK_LBUTTON, MAKELPARAM(100000, 0));
        }
        Fl::handle(FL_UNFOCUS, window);
        Fl::flush(); // it never returns to main loop when deactivated...
        break;

      case WM_SHOWWINDOW:
        if (!window->parent()) {
          Fl::handle(wParam ? FL_SHOW : FL_HIDE, window);
        }
        break;

      case WM_ACTIVATEAPP:
        // From eric@vfx.sel.sony.com, we should process WM_ACTIVATEAPP
        // messages to restore the correct state of the shift/ctrl/alt/lock
        // keys...  Added control, shift, alt, and meta keys, and changed
        // to use GetAsyncKeyState and do it when wParam is 1
        // (that means we have focus...)
        if (wParam) {
          ulong state = 0;
          if (GetAsyncKeyState(VK_CAPITAL))
            state |= FL_CAPS_LOCK;
          if (GetAsyncKeyState(VK_NUMLOCK))
            state |= FL_NUM_LOCK;
          if (GetAsyncKeyState(VK_SCROLL))
            state |= FL_SCROLL_LOCK;
          if (GetAsyncKeyState(VK_CONTROL) & ~1)
            state |= FL_CTRL;
          if (GetAsyncKeyState(VK_SHIFT) & ~1)
            state |= FL_SHIFT;
          if (GetAsyncKeyState(VK_MENU))
            state |= FL_ALT;
          if ((GetAsyncKeyState(VK_LWIN) | GetAsyncKeyState(VK_RWIN)) & ~1)
            state |= FL_META;
          Fl::e_state = state;
          return 0;
        }
        break;

      case WM_INPUTLANGCHANGE:
        fl_get_codepage();
        break;
      case WM_IME_COMPOSITION:
        // if (!fl_is_nt4() && lParam & GCS_RESULTCLAUSE) {
        //   HIMC himc = ImmGetContext(hWnd);
        //     wlen = ImmGetCompositionStringW(himc, GCS_RESULTSTR,
        //                                     wbuf, sizeof(wbuf)) / sizeof(short);
        //     if (wlen < 0) wlen = 0;
        //       wbuf[wlen] = 0;
        //       ImmReleaseContext(hWnd, himc);
        //     }
        break;

      case WM_KEYDOWN:
      case WM_SYSKEYDOWN:
      case WM_KEYUP:
      case WM_SYSKEYUP:
        // save the keysym until we figure out the characters:
        Fl::e_keysym = Fl::e_original_keysym = ms2fltk(wParam, lParam & (1 << 24));
        // Kludge to allow recognizing ctrl+'-' on keyboards with digits in uppercase positions (e.g. French)
        if (Fl::e_keysym == '6' && (VkKeyScanA('-') & 0xff) == '6') {
          Fl::e_keysym = '-';
        }
        // See if TranslateMessage turned it into a WM_*CHAR message:
        if (PeekMessageW(&fl_msg, hWnd, WM_CHAR, WM_SYSDEADCHAR, PM_REMOVE)) {
          uMsg = fl_msg.message;
          wParam = fl_msg.wParam;
          lParam = fl_msg.lParam;
        }
        // FALLTHROUGH ...

      case WM_DEADCHAR:
      case WM_SYSDEADCHAR:
      case WM_CHAR:
      case WM_SYSCHAR: {
        ulong state = Fl::e_state & 0xff000000; // keep the mouse button state
        // if GetKeyState is expensive we might want to comment some of these out:
        if (GetKeyState(VK_SHIFT) & ~1)
          state |= FL_SHIFT;
        if (GetKeyState(VK_CAPITAL))
          state |= FL_CAPS_LOCK;
        if (GetKeyState(VK_CONTROL) & ~1)
          state |= FL_CTRL;
        // Alt gets reported for the Alt-GR switch on non-English keyboards.
        // so we need to check the event as well to get it right:
        if ((lParam & (1 << 29)) // same as GetKeyState(VK_MENU)
            && uMsg != WM_CHAR)
          state |= FL_ALT;
        if (GetKeyState(VK_NUMLOCK))
          state |= FL_NUM_LOCK;
        if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & ~1) {
          // Windows bug?  GetKeyState returns garbage if the user hit the
          // meta key to pop up start menu.  Sigh.
          if ((GetAsyncKeyState(VK_LWIN) | GetAsyncKeyState(VK_RWIN)) & ~1)
            state |= FL_META;
        }
        if (GetKeyState(VK_SCROLL))
          state |= FL_SCROLL_LOCK;
        Fl::e_state = state;
        static char buffer[1024];
        if (uMsg == WM_CHAR || uMsg == WM_SYSCHAR) {
          wchar_t u = (wchar_t)wParam;
          Fl::e_length = fl_utf8fromwc(buffer, 1024, &u, 1);
          buffer[Fl::e_length] = 0;
        } else if (Fl::e_keysym >= FL_KP && Fl::e_keysym <= FL_KP_Last) {
          if (state & FL_NUM_LOCK) {
            // Convert to regular keypress...
            buffer[0] = Fl::e_keysym - FL_KP;
            Fl::e_length = 1;
          } else {
            // Convert to special keypress...
            buffer[0] = 0;
            Fl::e_length = 0;
            switch (Fl::e_keysym) {
              case FL_KP + '0':
                Fl::e_keysym = FL_Insert;
                break;
              case FL_KP + '1':
                Fl::e_keysym = FL_End;
                break;
              case FL_KP + '2':
                Fl::e_keysym = FL_Down;
                break;
              case FL_KP + '3':
                Fl::e_keysym = FL_Page_Down;
                break;
              case FL_KP + '4':
                Fl::e_keysym = FL_Left;
                break;
              case FL_KP + '6':
                Fl::e_keysym = FL_Right;
                break;
              case FL_KP + '7':
                Fl::e_keysym = FL_Home;
                break;
              case FL_KP + '8':
                Fl::e_keysym = FL_Up;
                break;
              case FL_KP + '9':
                Fl::e_keysym = FL_Page_Up;
                break;
              case FL_KP + '.':
                Fl::e_keysym = FL_Delete;
                break;
              case FL_KP + '/':
              case FL_KP + '*':
              case FL_KP + '-':
              case FL_KP + '+':
                buffer[0] = Fl::e_keysym - FL_KP;
                Fl::e_length = 1;
                break;
            }
          }
        } else if ((lParam & (1 << 31)) == 0) {
#ifdef FLTK_PREVIEW_DEAD_KEYS
          if ((lParam & (1 << 24)) == 0) { // clear if dead key (always?)
            wchar_t u = (wchar_t)wParam;
            Fl::e_length = fl_utf8fromwc(buffer, 1024, &u, 1);
            buffer[Fl::e_length] = 0;
          } else { // set if "extended key" (never printable?)
            buffer[0] = 0;
            Fl::e_length = 0;
          }
#else
          buffer[0] = 0;
          Fl::e_length = 0;
#endif
        }
        Fl::e_text = buffer;

        // Kludge to process the +-containing key in cross-platform way when used with Ctrl
/* Table of how Windows processes the '+'-containing key by keyboard layout
  key  virtual
content  key    keyboard layout
  =|+   0xbb    US/UK/Fr/Arabic/Chinese/Hebrew/Brazil/Russian/Vietnam/Japan/Korean/Persian
  +|*   0xbb    German/Spanish/Italy/Greek/Portugal
  +|?   0xbb    Swedish/Finish/Norway
  +|±   0xbb    Dutch
  1|+   '1'     Swiss/Luxemburg
  3|+   '3'     Hungarian
  4|+   '4'     Turkish
*/
        if ((Fl::e_state & FL_CTRL) && !(GetAsyncKeyState(VK_MENU) >> 15)) {
          // extra processing necessary only when Ctrl is down and Alt is up
          int vk_plus_key = (VkKeyScanA('+') & 0xff); // virtual key of '+'-containing key
          bool plus_shift_pos = ((VkKeyScanA('+') & 0x100) != 0); // true means '+' in shifted position
          int plus_other_char;  // the other char on same key as '+'
          if (plus_shift_pos) plus_other_char = ms2fltk(vk_plus_key, 0);
          else if ((VkKeyScanA('*') & 0xff) == vk_plus_key) plus_other_char = '*'; // German
          else if ((VkKeyScanA('?') & 0xff) == vk_plus_key) plus_other_char = '?'; // Swedish
          else if ((VkKeyScanW(L'±') & 0xff) == vk_plus_key) plus_other_char = L'±'; // Dutch
          else plus_other_char = '='; // fallback
//fprintf(stderr, "plus_shift_pos=%d plus_other_char='%c' vk+=0x%x\n", plus_shift_pos,
//        plus_other_char, vk_plus_key);
          if ( (vk_plus_key == 0xbb && Fl::e_keysym == '=') || // the '+'-containing key is down
                (plus_shift_pos && Fl::e_keysym == plus_other_char) ) {
            Fl::e_keysym = (plus_shift_pos ? plus_other_char : '+');
            static char plus_other_char_utf8[4];
            int lutf8 = fl_utf8encode(plus_other_char, plus_other_char_utf8);
            plus_other_char_utf8[lutf8] = 0;
            if (plus_shift_pos) {
              Fl::e_text = ( (Fl::e_state & FL_SHIFT) ? (char*)"+" : plus_other_char_utf8 );
            } else {
              Fl::e_text = ( (Fl::e_state & FL_SHIFT) ? plus_other_char_utf8 : (char*)"+" );
            }
            Fl::e_length = (int)strlen(Fl::e_text);
          }
        }
        // end of processing of the +-containing key

        if (lParam & (1 << 31)) { // key up events.
          if (Fl::handle(FL_KEYUP, window))
            return 0;
          break;
        }
        while (window->parent())
          window = window->window();
        if (Fl::handle(FL_KEYBOARD, window)) {
          if (uMsg == WM_DEADCHAR || uMsg == WM_SYSDEADCHAR)
            Fl::compose_state = 1;
          return 0;
        }
        break; // WM_KEYDOWN ... WM_SYSKEYUP, WM_DEADCHAR ... WM_SYSCHAR
      } // case WM_DEADCHAR ... WM_SYSCHAR

      case WM_MOUSEWHEEL: {
        static int delta = 0; // running total of all vertical mousewheel motion
        delta += (SHORT)(HIWORD(wParam));
        int dy = -delta / WHEEL_DELTA;
        delta += dy * WHEEL_DELTA;
        if (dy == 0) // nothing to do
          return 0;
        if (Fl::event_shift()) { // shift key pressed: send horizontal mousewheel event
          Fl::e_dx = dy;
          Fl::e_dy = 0;
        } else { // shift key not pressed (normal behavior): send vertical mousewheel event
          Fl::e_dx = 0;
          Fl::e_dy = dy;
        }
        Fl::handle(FL_MOUSEWHEEL, window);
        return 0;
      }

      case WM_MOUSEHWHEEL: {
        static int delta = 0; // running total of all horizontal mousewheel motion
        delta += (SHORT)(HIWORD(wParam));
        int dx = delta / WHEEL_DELTA;
        delta -= dx * WHEEL_DELTA;
        if (dx == 0) // nothing to do
          return 0;
        if (Fl::event_shift()) { // shift key pressed: send *vertical* mousewheel event
          Fl::e_dx = 0;
          Fl::e_dy = dx;
        } else { // shift key not pressed (normal behavior): send horizontal mousewheel event
          Fl::e_dx = dx;
          Fl::e_dy = 0;
        }
        Fl::handle(FL_MOUSEWHEEL, window);
        return 0;
      }

      case WM_GETMINMAXINFO:
        Fl_WinAPI_Window_Driver::driver(window)->set_minmax((LPMINMAXINFO)lParam);
        break;

      case WM_SIZE:
        if (!window->parent()) {
          Fl_Window_Driver::driver(window)->is_maximized(wParam == SIZE_MAXIMIZED);
          if (wParam == SIZE_MINIMIZED || wParam == SIZE_MAXHIDE) {
            Fl::handle(FL_HIDE, window);
          } else {
            Fl::handle(FL_SHOW, window);
            resize_bug_fix = window;
            window->size(int(ceil(LOWORD(lParam) / scale)), int(ceil(HIWORD(lParam) / scale)));
            // fprintf(LOG,"WM_SIZE size(%.0f,%.0f) graph(%d,%d) s=%.2f\n",
            //         ceil(LOWORD(lParam)/scale),ceil(HIWORD(lParam)/scale),
            //         LOWORD(lParam),HIWORD(lParam),scale);
          }
        }
        break;

      case WM_MOVE: {
        if (IsIconic(hWnd)) {
          break;
        }
        resize_bug_fix = window;
        int nx = LOWORD(lParam);
        int ny = HIWORD(lParam);
        if (nx & 0x8000) nx -= 65536;
        if (ny & 0x8000) ny -= 65536;
        // fprintf(LOG,"WM_MOVE position(%d,%d) s=%.2f\n",int(nx/scale),int(ny/scale),scale);
        // detect when window centre changes screen
        Fl_WinAPI_Screen_Driver *sd = (Fl_WinAPI_Screen_Driver *)Fl::screen_driver();
        Fl_WinAPI_Window_Driver *wd = Fl_WinAPI_Window_Driver::driver(window);
        int olds = wd->screen_num();
        // Issue #1097: when a fullscreen window is restored to its size, it receives first a WM_MOVE
        // and then a WM_SIZE, so it still has its fullscreen size at the WM_MOVE event, which defeats
        // using window->w()|h() to compute the center of the (small) window. We detect this situation
        // with condition: !window->fullscreen_active() && *wd->no_fullscreen_w()
        // and use *wd->no_fullscreen_w()|h() instead of window->w()|h().
        int trueW = window->w(), trueH = window->h();
        if (!window->fullscreen_active() && *wd->no_fullscreen_w()) {
          trueW = *wd->no_fullscreen_w(); trueH = *wd->no_fullscreen_h();
        }
        int news = sd->screen_num_unscaled(nx + int(trueW * scale / 2), ny + int(trueH * scale / 2));
        if (news == -1)
          news = olds;
        float s = sd->scale(news);
        // fprintf(LOG,"WM_MOVE olds=%d(%.2f) news=%d(%.2f) busy=%d\n",olds,
        //             sd->scale(olds),news, s,
        //             Fl_WinAPI_Window_Driver::data_for_resize_window_between_screens_.busy);
        // fflush(LOG);
        if (!window->parent()) {
          if (olds != news) {
            if (s != sd->scale(olds) &&
                !Fl_WinAPI_Window_Driver::data_for_resize_window_between_screens_.busy &&
                window->user_data() != &Fl_WinAPI_Screen_Driver::transient_scale_display) {
              Fl_WinAPI_Window_Driver::data_for_resize_window_between_screens_.busy = true;
              Fl_WinAPI_Window_Driver::data_for_resize_window_between_screens_.screen = news;
              Fl::add_timeout(1, Fl_WinAPI_Window_Driver::resize_after_screen_change, window);
            }
            else if (!Fl_WinAPI_Window_Driver::data_for_resize_window_between_screens_.busy)
              wd->screen_num(news);
          } else if (Fl_WinAPI_Window_Driver::data_for_resize_window_between_screens_.busy) {
            Fl::remove_timeout(Fl_WinAPI_Window_Driver::resize_after_screen_change, window);
            Fl_WinAPI_Window_Driver::data_for_resize_window_between_screens_.busy = false;
          }
        }
        window->position(int(round(nx/scale)), int(round(ny/scale)));
        break;
      } // case WM_MOVE

      case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT) {
          while (window->parent())
            window = window->window();
          SetCursor(Fl_WinAPI_Window_Driver::driver(window)->cursor);
          return 0;
        }
        break;

#if USE_COLORMAP
      case WM_QUERYNEWPALETTE:
        fl_GetDC(hWnd);
        if (fl_select_palette())
          InvalidateRect(hWnd, NULL, FALSE);
        break;

      case WM_PALETTECHANGED:
        if ((HWND)wParam != hWnd && fl_select_palette())
          UpdateColors(fl_GetDC(hWnd));
        break;

      case WM_CREATE:
        fl_GetDC(hWnd);
        fl_select_palette();
        break;
#endif

      case WM_DESTROYCLIPBOARD:
        fl_i_own_selection[1] = 0;
        return 1;

      case WM_DISPLAYCHANGE: {// when screen configuration (number, size, position) changes
        Fl::call_screen_init();
        Fl::handle(FL_SCREEN_CONFIGURATION_CHANGED, NULL);
        return 0;
      }
      case WM_CHANGECBCHAIN:
        if ((hWnd == clipboard_wnd) && (next_clipboard_wnd == (HWND)wParam))
          next_clipboard_wnd = (HWND)lParam;
        else
          SendMessage(next_clipboard_wnd, WM_CHANGECBCHAIN, wParam, lParam);
        return 0;

      case WM_DRAWCLIPBOARD:
        // When the clipboard moves between two FLTK windows,
        // fl_i_own_selection will temporarily be false as we are
        // processing this message. Hence the need to use fl_find().
        if (!initial_clipboard && !fl_find(GetClipboardOwner()))
          fl_trigger_clipboard_notify(1);
        initial_clipboard = false;

        if (next_clipboard_wnd)
          SendMessage(next_clipboard_wnd, WM_DRAWCLIPBOARD, wParam, lParam);

        return 0;

      default:
        if (Fl::handle(0, 0))
          return 0;
        break;
    } // switch (uMsg)
  } // if (window)
  return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

/* Implementation note about the API to get the dimensions of the top/left borders and the title bar

 Function fake_X_wm() below is used before calling CreateWindowExW() to create
 a window and before calling SetWindowPos(). Both of these Windows functions need the window size
 including borders and title bar. Function fake_X_wm() uses AdjustWindowRectExForDpi() or
 AdjustWindowRectEx() to get the sizes of borders and title bar. The gotten values don't always match
 what is seen on the display, but they are the **required** values so the subsequent calls to
 CreateWindowExW() or SetWindowPos() correctly size the window.
 The Windows doc of AdjustWindowRectExForDpi/AdjustWindowRectEx makes this very clear:
    Calculates the required size of the window rectangle, based on the desired size of the client
    rectangle [and the provided DPI]. This window rectangle can then be passed to the CreateWindowEx
    function to create a window with a client area of the desired size.

 Conversely, Fl_WinAPI_Window_Driver::border_width_title_bar_height() is used to get
 the true sizes of borders and title bar of a mapped window. The correct API for that is
 DwmGetWindowAttribute().
 */

// /////////////////////////////////////////////////////////////////
// This function gets the dimensions of the top/left borders and
// the title bar, if there is one, based on the FL_BORDER, FL_MODAL
// and FL_NONMODAL flags, and on the window's size range.
// It returns the following values:
//
// value | border | title bar
//   0   |  none  |   no
//   1   |  fix   |   yes
//   2   |  size  |   yes

int Fl_WinAPI_Window_Driver::fake_X_wm(int &X, int &Y, int &bt, int &bx, int &by, DWORD style, DWORD styleEx) {

  const Fl_Window *w = pWindow;

  int W = 0, H = 0, xoff = 0, yoff = 0, dx = 0, dy = 0;
  int ret = bx = by = bt = 0;

  int fallback = 1;
  float s = Fl::screen_driver()->scale(screen_num());
  int minw, minh, maxw, maxh;
  pWindow->get_size_range(&minw, &minh, &maxw, &maxh, NULL, NULL, NULL);
  if (!w->parent()) {
    if (fl_xid(w) || style) {
      // The block below calculates the window borders by requesting the
      // required decorated window rectangle for a desired client rectangle.
      // If any part of the function above fails, we will drop to a
      // fallback to get the best guess which is always available.

      if (!style) {
        HWND hwnd = fl_xid(w);
        // request the style flags of this window, as Windows sees them
        style = GetWindowLong(hwnd, GWL_STYLE);
        styleEx = GetWindowLong(hwnd, GWL_EXSTYLE);
      }

      RECT r;
      int drawingX, drawingY; // drawing coordinates of window top-left
      r.left = drawingX = int(round(w->x() * s));
      r.top = drawingY = int(round(w->y() * s));
      r.right = drawingX + int(w->w() * s);
      r.bottom = drawingY + int(w->h() * s);
      // get the decoration rectangle for the desired client rectangle

      typedef BOOL(WINAPI* AdjustWindowRectExForDpi_type)(LPRECT, DWORD, BOOL, DWORD, UINT);
      static AdjustWindowRectExForDpi_type fl_AdjustWindowRectExForDpi =
        (AdjustWindowRectExForDpi_type)GetProcAddress(LoadLibrary("User32.DLL"), "AdjustWindowRectExForDpi");
      BOOL ok;
      if (is_dpi_aware && fl_AdjustWindowRectExForDpi) {
        Fl_WinAPI_Screen_Driver *sd = (Fl_WinAPI_Screen_Driver*)Fl::screen_driver();
        UINT dpi = UINT(sd->dpi[screen_num()][0]);
        ok = fl_AdjustWindowRectExForDpi(&r, style, FALSE, styleEx, dpi);
      } else
        ok = AdjustWindowRectEx(&r, style, FALSE, styleEx);
      if (ok) {
        X = r.left;
        Y = r.top;
        W = r.right - r.left;
        H = r.bottom - r.top;
        bx = drawingX - r.left;
        by = r.bottom - int(drawingY + w->h() * s); // height of the bottom frame
        bt = drawingY - r.top - by;                 // height of top caption bar
        xoff = bx;
        yoff = by + bt;
        dx = W - int(w->w() * s);
        dy = H - int(w->h() * s);
        if (maxw != minw || maxh != minh)
          ret = 2;
        else
          ret = 1;
        fallback = 0;
      }
    }
  }
  // This is the original (pre 1.1.7) routine to calculate window border sizes.
  if (fallback) {
    if (w->border() && !w->parent()) {
      if (maxw != minw || maxh != minh) {
        ret = 2;
        bx = GetSystemMetrics(SM_CXSIZEFRAME);
        by = GetSystemMetrics(SM_CYSIZEFRAME);
      } else {
        ret = 1;
        int padding = GetSystemMetrics(SM_CXPADDEDBORDER);
        NONCLIENTMETRICS ncm;
        ncm.cbSize = sizeof(NONCLIENTMETRICS);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
        bx = GetSystemMetrics(SM_CXFIXEDFRAME) + (padding ? padding + ncm.iBorderWidth : 0);
        by = GetSystemMetrics(SM_CYFIXEDFRAME) + (padding ? padding + ncm.iBorderWidth : 0);
      }
      bt = GetSystemMetrics(SM_CYCAPTION);
    }
    // The coordinates of the whole window, including non-client area
    xoff = bx;
    yoff = by + bt;
    dx = 2 * bx;
    dy = 2 * by + bt;
    X = w->x() - xoff;
    Y = w->y() - yoff;
    W = w->w() + dx;
    H = w->h() + dy;
  }

  // Proceed to positioning the window fully inside the screen, if possible
  // Find screen that contains most of the window
  // FIXME: this ought to be the "work area" instead of the entire screen !
  int scr_x = 0, scr_y = 0, scr_w = 0, scr_h = 0;
  int ns = Fl::screen_num(int(round(X / s)), int(round(Y / s)), int(W / s), int(H / s));
  ((Fl_WinAPI_Screen_Driver*)Fl::screen_driver())->screen_xywh_unscaled(scr_x, scr_y, scr_w, scr_h, ns);
  // Make border's lower right corner visible
  if (scr_x + scr_w < X + W)
    X = scr_x + scr_w - W;
  if (scr_y + scr_h < Y + H)
    Y = scr_y + scr_h - H;
  // Make border's upper left corner visible
  if (X < scr_x)
    X = scr_x;
  if (Y < scr_y)
    Y = scr_y;
  // Make client area's lower right corner visible
  if (scr_x + scr_w < X + dx + w->w())
    X = scr_x + scr_w - int(w->w() * s) - dx;
  if (scr_y + scr_h < Y + dy + w->h())
    Y = scr_y + scr_h - int(w->h() * s) - dy;
  // Make client area's upper left corner visible
  if (X + xoff < scr_x)
    X = scr_x - xoff;
  if (Y + yoff < scr_y)
    Y = scr_y - yoff;
  // Return the client area's top left corner in (X,Y)
  X += xoff;
  Y += yoff;

  if (w->fullscreen_active()) {
    bx = by = bt = 0;
  }

  return ret;
}

////////////////////////////////////////////////////////////////

static void delayed_fullscreen(Fl_Window *win) {
  Fl::remove_check((Fl_Timeout_Handler)delayed_fullscreen, win);
  win->fullscreen_off();
  win->fullscreen();
}


static void delayed_maximize(Fl_Window *win) {
  Fl::remove_check((Fl_Timeout_Handler)delayed_maximize, win);
  win->un_maximize();
  win->maximize();
}


void Fl_WinAPI_Window_Driver::resize(int X, int Y, int W, int H) {
//fprintf(stderr, "resize w()=%d W=%d h()=%d H=%d\n",pWindow->w(), W,pWindow->h(), H);
  if (Fl_Window::is_a_rescale() && pWindow->fullscreen_active()) {
    Fl::add_check((Fl_Timeout_Handler)delayed_fullscreen, pWindow);
  } else if (Fl_Window::is_a_rescale() && pWindow->maximize_active()) {
    Fl::add_check((Fl_Timeout_Handler)delayed_maximize, pWindow);
  }
  UINT flags = SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER;
  int is_a_resize = (W != w() || H != h() || Fl_Window::is_a_rescale());
  int resize_from_program = (pWindow != resize_bug_fix);
  if (!resize_from_program)
    resize_bug_fix = 0;
  if (X != x() || Y != y() || Fl_Window::is_a_rescale()) {
    force_position(1);
  } else {
    if (!is_a_resize)
      return;
    flags |= SWP_NOMOVE;
  }
  if (is_a_resize) {
    if (resize_from_program && shown()) {
      // don't obey "resize from program" when window is maximized
      WINDOWPLACEMENT wplace;
      wplace.length = sizeof(WINDOWPLACEMENT);
      BOOL ok = GetWindowPlacement(fl_xid(pWindow), &wplace);
      if (ok && wplace.showCmd == SW_SHOWMAXIMIZED) return;
    }
    pWindow->Fl_Group::resize(X, Y, W, H);
    if (visible_r()) {
      pWindow->redraw();
      // only wait for exposure if this window has a size - a window
      // with no width or height will never get an exposure event
      Fl_X *i = Fl_X::flx(pWindow);
      if (i && W > 0 && H > 0)
        wait_for_expose_value = 1;
    }
  } else {
    x(X);
    y(Y);
    flags |= SWP_NOSIZE;
  }
  if (!border())
    flags |= SWP_NOACTIVATE;
  if (resize_from_program && shown()) {
    int dummy_x, dummy_y, bt, bx, by;
    // compute window position and size in scaled units
    float s = Fl::screen_driver()->scale(screen_num());
    int scaledX = int(round(X * s)), scaledY = int(round(Y * s)), scaledW = int(W * s), scaledH = int(H * s);
    // Ignore window managing when resizing, so that windows (and more
    // specifically menus) can be moved offscreen.
    if (fake_X_wm(dummy_x, dummy_y, bt, bx, by)) {
      scaledX -= bx;
      scaledY -= by + bt;
      scaledW += 2 * bx;
      scaledH += 2 * by + bt;
    }
    // avoid zero size windows. A zero sized window on Win32
    // will cause continouly  new redraw events.
    if (scaledW <= 0)
      scaledW = 1;
    if (scaledH <= 0)
      scaledH = 1;
    SetWindowPos(fl_xid(pWindow), 0, scaledX, scaledY, scaledW, scaledH, flags);
  }
}


////////////////////////////////////////////////////////////////

/*
 This silly little class remembers the name of all window classes
 we register to avoid double registration. It has the added bonus
 of freeing everything on application close as well.
 */
class NameList {
public:
  NameList() {
    name = (char **)malloc(sizeof(char **));
    NName = 1;
    nName = 0;
  }
  ~NameList() {
    int i;
    for (i = 0; i < nName; i++)
      free(name[i]);
    if (name)
      free(name);
  }
  void add_name(const char *n) {
    if (NName == nName) {
      NName += 5;
      name = (char **)realloc(name, NName * sizeof(char *));
    }
    name[nName++] = fl_strdup(n);
  }
  char has_name(const char *n) {
    int i;
    for (i = 0; i < nName; i++) {
      if (strcmp(name[i], n) == 0)
        return 1;
    }
    return 0;
  }

private:
  char **name;
  int nName, NName;
};

void fl_fix_focus(); // in Fl.cxx

UINT fl_wake_msg = 0;
int fl_disable_transient_for; // secret method of removing TRANSIENT_FOR

void Fl_WinAPI_Window_Driver::makeWindow() {
  Fl_Group::current(0); // get rid of very common user bug: forgot end()

  fl_open_display();

  // if the window is a subwindow and our parent is not mapped yet, we
  // mark this window visible, so that mapping the parent at a later
  // point in time will call this function again to finally map the subwindow.
  Fl_Window *w = pWindow;
  if (w->parent() && !Fl_X::flx(w->window())) {
    w->set_visible();
    return;
  }

  static NameList class_name_list;
  static const char *first_class_name = 0L;
  const char *class_name = w->xclass();
  if (!class_name)
    class_name = first_class_name; // reuse first class name used
  if (!class_name)
    class_name = "FLTK"; // default to create a "FLTK" WNDCLASS
  if (!first_class_name) {
    first_class_name = class_name;
  }

  wchar_t class_namew[100]; // (limited) buffer for Windows class name

  // convert UTF-8 class_name to wchar_t for RegisterClassExW and CreateWindowExW

  fl_utf8toUtf16(class_name,
                 (unsigned)strlen(class_name),                     // in
                 (unsigned short *)class_namew,                    // out
                 (unsigned)sizeof(class_namew) / sizeof(wchar_t)); // max. size

  if (!class_name_list.has_name(class_name)) {
    WNDCLASSEXW wcw;
    memset(&wcw, 0, sizeof(wcw));
    wcw.cbSize = sizeof(WNDCLASSEXW);

    // Documentation states a device context consumes about 800 bytes
    // of memory... so who cares? If 800 bytes per window is what it
    // takes to speed things up, I'm game.
    wcw.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
    wcw.lpfnWndProc = (WNDPROC)WndProc;
    wcw.cbClsExtra = wcw.cbWndExtra = 0;
    wcw.hInstance = fl_display;
    if (!w->icon() && !icon_->count)
      w->icon((void *)LoadIcon(NULL, IDI_APPLICATION));
    wcw.hIcon = wcw.hIconSm = (HICON)w->icon();
    wcw.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcw.hbrBackground = NULL;
    wcw.lpszMenuName = NULL;
    wcw.lpszClassName = class_namew;
    RegisterClassExW(&wcw);
    class_name_list.add_name(class_name);
  }

  const wchar_t *message_namew = L"FLTK::ThreadWakeup";
  if (!fl_wake_msg)
    fl_wake_msg = RegisterWindowMessageW(message_namew);

  HWND parent;
  DWORD style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
  DWORD styleEx = WS_EX_LEFT;

  // compute adequate screen where to put the window
  int nscreen = 0;
  if (w->parent()) {
    nscreen = Fl_Window_Driver::driver(w->top_window())->screen_num();
  } else if (Fl_Window_Driver::driver(w)->force_position() && Fl_WinAPI_Window_Driver::driver(w)->screen_num_ >= 0) {
    nscreen = Fl_Window_Driver::driver(w)->screen_num();
  } else {
    Fl_Window *hint = Fl::first_window();
    if (hint) {
      nscreen = Fl_Window_Driver::driver(hint->top_window())->screen_num();
    } else {
      int mx, my;
      nscreen = Fl::screen_driver()->get_mouse(mx, my);
    }
  }
  Fl_Window_Driver::driver(w)->screen_num(nscreen);
  float s = Fl::screen_driver()->scale(nscreen);
  int xp = int(round(w->x() * s)); // these are in graphical units
  int yp = int(round(w->y() * s));
  int wp = int(w->w() * s);
  int hp = int(w->h() * s);

  int showit = 1;

  if (w->parent()) {
    style |= WS_CHILD;
    styleEx |= WS_EX_WINDOWEDGE | WS_EX_CONTROLPARENT;
    parent = fl_xid(w->window());
  } else { // top level window
    styleEx |= WS_EX_WINDOWEDGE | WS_EX_CONTROLPARENT;

    int wintype = 0;
    if (w->border()) {
      if (is_resizable())
        wintype = 2;
      else
        wintype = 1;
    }

    switch (wintype) {
      // No border (used for menus)
      case 0:
        style |= WS_POPUP;
        styleEx |= WS_EX_TOOLWINDOW;
        break;

      // Thin border and title bar
      case 1:
        style |= WS_DLGFRAME | WS_CAPTION;
        if (!w->modal())
          style |= WS_SYSMENU | WS_MINIMIZEBOX;
        break;

      // Thick, resizable border and title bar, with maximize button
      case 2:
        style |= WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_CAPTION;
        if (!w->modal())
          style |= WS_MINIMIZEBOX;
        break;
    }

    int xwm = xp, ywm = yp, bt, bx, by; // these are in graphical units

    fake_X_wm(xwm, ywm, bt, bx, by, style, styleEx);

    if (by + bt) {
      wp += 2 * bx;
      hp += 2 * by + bt;
    }
    if (!force_position()) {
      xp = yp = CW_USEDEFAULT;
    } else {
      if (!Fl::grab()) {
        xp = xwm;
        yp = ywm;
        x(int(round(xp / s)));
        y(int(round(yp / s)));
      }
      xp -= bx;
      yp -= by + bt;
    }

    parent = 0;
    if (w->non_modal() && Fl_X::first && !fl_disable_transient_for) {
      // find some other window to be "transient for":
      Fl_Window *w = Fl_X::first->w;
      while (w->parent())
        w = w->window();
      parent = fl_xid(w);
      if (!w->visible())
        showit = 0;
//      https://www.fltk.org/str.php?L1115
//      Mike added the code below to fix issues with tooltips that unfortunately
//      he does not specify in detail. After extensive testing, I can't see
//      how this fixes things, but I do see how a window opened by a timer will
//      link that window to the current popup, which is wrong.
//      Matt, Apr 30th, 2023
//    } else if (Fl::grab()) {
//      parent = fl_xid(Fl::grab());
    }
  }

  Fl_X *x = new Fl_X;
  other_xid = 0;
  x->w = w;
  flx(x);
  x->region = 0;
  Fl_WinAPI_Window_Driver::driver(w)->private_dc = 0;
  cursor = LoadCursor(NULL, IDC_ARROW);
  custom_cursor = 0;
  if (!fl_codepage)
    fl_get_codepage();

  WCHAR *lab = NULL;
  if (w->label()) {
    size_t l = strlen(w->label());
    unsigned wlen = fl_utf8toUtf16(w->label(), (unsigned)l, NULL, 0); // Pass NULL to query length
    wlen++;
    lab = (WCHAR *)malloc(sizeof(WCHAR) * wlen);
    wlen = fl_utf8toUtf16(w->label(), (unsigned)l, (unsigned short *)lab, wlen);
    lab[wlen] = 0;
  }
  x->xid = (fl_uintptr_t)CreateWindowExW(styleEx,
                           class_namew, lab, style,
                           xp, yp, wp, hp,
                           parent,
                           NULL, // menu
                           fl_display,
                           NULL // creation parameters
                          );
  if (lab)
    free(lab);

  x->next = Fl_X::first;
  Fl_X::first = x;

  set_icons();

  if (w->fullscreen_active()) {
    /* We need to make sure that the fullscreen is created on the
       default monitor, ie the desktop where the shortcut is located
       etc. This requires that CreateWindow is called with CW_USEDEFAULT
       for x and y. We can then use GetWindowRect to determine which
       monitor the window was placed on. */
    RECT rect;
    GetWindowRect((HWND)x->xid, &rect);
    make_fullscreen(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
  }

  // Setup clipboard monitor target if there are registered handlers and
  // no window is targeted.
  if (!fl_clipboard_notify_empty() && clipboard_wnd == NULL)
    fl_clipboard_notify_target((HWND)x->xid);

  wait_for_expose_value = ((wp == 0 || hp == 0) && !w->border() && !w->parent() ? 0 : 1); // issue #985
  if (show_iconic()) {
    showit = 0;
    show_iconic(0);
  }
  if (showit) {
    w->set_visible();
    int old_event = Fl::e_number;
    w->handle(Fl::e_number = FL_SHOW); // get child windows to appear
    Fl::e_number = old_event;
    w->redraw(); // force draw to happen
  }

  // Needs to be done before ShowWindow() to get the correct behavior
  // when we get WM_SETFOCUS.
  if (w->modal()) {
    Fl::modal_ = w;
    fl_fix_focus();
  }

  // If we've captured the mouse, we don't want to activate any
  // other windows from the code, or we lose the capture.
  ShowWindow((HWND)x->xid, !showit ? SW_SHOWMINNOACTIVE :
             (Fl::grab() || (styleEx & WS_EX_TOOLWINDOW)) ? SW_SHOWNOACTIVATE : SW_SHOWNORMAL);

  // Register all windows for potential drag'n'drop operations
  RegisterDragDrop((HWND)x->xid, flIDropTarget);

  if (!im_enabled)
    flImmAssociateContextEx((HWND)x->xid, 0, 0);
}


////////////////////////////////////////////////////////////////

HINSTANCE fl_display = GetModuleHandle(NULL);

HINSTANCE fl_win32_display() { return fl_display; }

void Fl_WinAPI_Window_Driver::set_minmax(LPMINMAXINFO minmax) {
  int td, wd, hd, dummy_x, dummy_y;

  fake_X_wm(dummy_x, dummy_y, td, wd, hd);
  wd *= 2;
  hd *= 2;
  hd += td;

  int minw, minh, maxw, maxh;
  pWindow->get_size_range(&minw, &minh, &maxw, &maxh, NULL, NULL, NULL);
  float s = Fl::screen_driver()->scale(screen_num());
  minmax->ptMinTrackSize.x = LONG(s * minw) + wd;
  minmax->ptMinTrackSize.y = LONG(s * minh) + hd;
  if (maxw) {
    minmax->ptMaxTrackSize.x = LONG(s * maxw) + wd;
    minmax->ptMaxSize.x = LONG(s * maxw) + wd;
  }
  if (maxh) {
    minmax->ptMaxTrackSize.y = LONG(s * maxh) + hd;
    minmax->ptMaxSize.y = LONG(s * maxh) + hd;
  }
}


////////////////////////////////////////////////////////////////

// returns pointer to the filename, or null if name ends with '/'
const char *Fl_WinAPI_System_Driver::filename_name(const char *name) {
  const char *p, *q;
  if (!name)
    return (0);
  q = name;
  if (q[0] && q[1] == ':')
    q += 2; // skip leading drive letter
  for (p = q; *p; p++) {
    if (*p == '/' || *p == '\\')
      q = p + 1;
  }
  return q;
}


////////////////////////////////////////////////////////////////

static HICON image_to_icon(const Fl_RGB_Image *image, bool is_icon, int hotx, int hoty) {
  BITMAPV5HEADER bi;
  HBITMAP bitmap, mask;
  DWORD *bits;
  HICON icon;

  if (!is_icon) {
    if ((hotx < 0) || (hotx >= image->data_w()))
      return NULL;
    if ((hoty < 0) || (hoty >= image->data_h()))
      return NULL;
  }

  memset(&bi, 0, sizeof(BITMAPV5HEADER));

  bi.bV5Size        = sizeof(BITMAPV5HEADER);
  bi.bV5Width       = image->data_w();
  bi.bV5Height      = -image->data_h(); // Negative for top-down
  bi.bV5Planes      = 1;
  bi.bV5BitCount    = 32;
  bi.bV5Compression = BI_BITFIELDS;
  bi.bV5RedMask     = 0x00FF0000;
  bi.bV5GreenMask   = 0x0000FF00;
  bi.bV5BlueMask    = 0x000000FF;
  bi.bV5AlphaMask   = 0xFF000000;

  HDC hdc;

  hdc = GetDC(NULL);
  bitmap = CreateDIBSection(hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&bits, NULL, 0);
  ReleaseDC(NULL, hdc);

  if (bits == NULL)
    return NULL;

  const uchar *i = (const uchar *)*image->data();
  const int extra_data = image->ld() ? (image->ld() - image->data_w() * image->d()) : 0;

  for (int y = 0; y < image->data_h(); y++) {
    for (int x = 0; x < image->data_w(); x++) {
      switch (image->d()) {
        case 1:
          *bits = (0xff << 24) | (i[0] << 16) | (i[0] << 8) | i[0];
          break;
        case 2:
          *bits = (i[1] << 24) | (i[0] << 16) | (i[0] << 8) | i[0];
          break;
        case 3:
          *bits = (0xff << 24) | (i[0] << 16) | (i[1] << 8) | i[2];
          break;
        case 4:
          *bits = (i[3] << 24) | (i[0] << 16) | (i[1] << 8) | i[2];
          break;
      }
      i += image->d();
      bits++;
    }
    i += extra_data;
  }

  // A mask bitmap is still needed even though it isn't used
  mask = CreateBitmap(image->data_w(), image->data_h(), 1, 1, NULL);
  if (mask == NULL) {
    DeleteObject(bitmap);
    return NULL;
  }

  ICONINFO ii;

  ii.fIcon    = is_icon;
  ii.xHotspot = hotx;
  ii.yHotspot = hoty;
  ii.hbmMask  = mask;
  ii.hbmColor = bitmap;

  icon = CreateIconIndirect(&ii);

  DeleteObject(bitmap);
  DeleteObject(mask);

  return icon;
}

////////////////////////////////////////////////////////////////

static HICON default_big_icon = NULL;
static HICON default_small_icon = NULL;

static const Fl_RGB_Image *find_best_icon(int ideal_width, const Fl_RGB_Image *icons[], int count) {
  const Fl_RGB_Image *best;

  best = NULL;

  for (int i = 0; i < count; i++) {
    if (best == NULL)
      best = icons[i];
    else {
      if (best->w() < ideal_width) {
        if (icons[i]->w() > best->w())
          best = icons[i];
      } else {
        if ((icons[i]->w() >= ideal_width) && (icons[i]->w() < best->w()))
          best = icons[i];
      }
    }
  }

  return best;
}

void Fl_WinAPI_Screen_Driver::default_icons(const Fl_RGB_Image *icons[], int count) {
  const Fl_RGB_Image *best_big, *best_small;

  if (default_big_icon != NULL)
    DestroyIcon(default_big_icon);
  if (default_small_icon != NULL)
    DestroyIcon(default_small_icon);

  default_big_icon = NULL;
  default_small_icon = NULL;

  best_big = find_best_icon(GetSystemMetrics(SM_CXICON), icons, count);
  best_small = find_best_icon(GetSystemMetrics(SM_CXSMICON), icons, count);

  bool need_delete;
  if (best_big != NULL) {
    need_delete = false;
    if (best_big->w() != best_big->data_w() || best_big->h() != best_big->data_h()) {
      best_big = (Fl_RGB_Image *)best_big->copy();
      need_delete = true;
    }
    default_big_icon = image_to_icon(best_big, true, 0, 0);
    if (need_delete) delete best_big;
  }

  if (best_small != NULL) {
    need_delete = false;
    if (best_small->w() != best_small->data_w() ||
        best_small->h() != best_small->data_h()) {
      best_small = (Fl_RGB_Image *)best_small->copy();
      need_delete = true;
    }
    default_small_icon = image_to_icon(best_small, true, 0, 0);
    if (need_delete) delete best_small;
  }
}


void Fl_Window::icons(HICON big_icon, HICON small_icon) {
  free_icons();
  if (big_icon != NULL)
    Fl_WinAPI_Window_Driver::driver(this)->icon_->big_icon = CopyIcon(big_icon);
  if (small_icon != NULL)
    Fl_WinAPI_Window_Driver::driver(this)->icon_->small_icon = CopyIcon(small_icon);
  if (Fl_X::flx(this))
    Fl_WinAPI_Window_Driver::driver(this)->set_icons();
}

void Fl_Window::default_icons(HICON big_icon, HICON small_icon) {
  if (default_big_icon != NULL)
    DestroyIcon(default_big_icon);
  if (default_small_icon != NULL)
    DestroyIcon(default_small_icon);

  default_big_icon = NULL;
  default_small_icon = NULL;

  if (big_icon != NULL)
    default_big_icon = CopyIcon(big_icon);
  if (small_icon != NULL)
    default_small_icon = CopyIcon(small_icon);
}

void Fl_WinAPI_Window_Driver::set_icons() {
  HICON big_icon, small_icon;

  // Windows doesn't copy the icons, so we have to "leak" them when
  // setting, and clean up when we change to some other icons.
  big_icon = (HICON)SendMessage(fl_xid(pWindow), WM_GETICON, ICON_BIG, 0);
  if ((big_icon != NULL) && (big_icon != default_big_icon))
    DestroyIcon(big_icon);
  small_icon = (HICON)SendMessage(fl_xid(pWindow), WM_GETICON, ICON_SMALL, 0);
  if ((small_icon != NULL) && (small_icon != default_small_icon))
    DestroyIcon(small_icon);

  big_icon = NULL;
  small_icon = NULL;

  if (icon_->count) {
    const Fl_RGB_Image *best_big, *best_small;

    best_big = find_best_icon(GetSystemMetrics(SM_CXICON),
                              (const Fl_RGB_Image **)icon_->icons,
                              icon_->count);
    best_small = find_best_icon(GetSystemMetrics(SM_CXSMICON),
                                (const Fl_RGB_Image **)icon_->icons,
                                icon_->count);

    if (best_big != NULL)
      big_icon = image_to_icon(best_big, true, 0, 0);
    if (best_small != NULL)
      small_icon = image_to_icon(best_small, true, 0, 0);
  } else {
    if ((icon_->big_icon != NULL) || (icon_->small_icon != NULL)) {
      big_icon = icon_->big_icon;
      small_icon = icon_->small_icon;
    } else {
      big_icon = default_big_icon;
      small_icon = default_small_icon;
    }
  }

  SendMessage(fl_xid(pWindow), WM_SETICON, ICON_BIG, (LPARAM)big_icon);
  SendMessage(fl_xid(pWindow), WM_SETICON, ICON_SMALL, (LPARAM)small_icon);
}


////////////////////////////////////////////////////////////////

#ifndef IDC_HAND
#define IDC_HAND MAKEINTRESOURCE(32649)
#endif // !IDC_HAND

int Fl_WinAPI_Window_Driver::set_cursor(Fl_Cursor c) {
  LPSTR n;
  HCURSOR new_cursor;

  if (c == FL_CURSOR_NONE)
    new_cursor = NULL;
  else {
    switch (c) {
      case FL_CURSOR_ARROW:
        n = IDC_ARROW;
        break;
      case FL_CURSOR_CROSS:
        n = IDC_CROSS;
        break;
      case FL_CURSOR_WAIT:
        n = IDC_WAIT;
        break;
      case FL_CURSOR_INSERT:
        n = IDC_IBEAM;
        break;
      case FL_CURSOR_HAND:
        n = IDC_HAND;
        break;
      case FL_CURSOR_HELP:
        n = IDC_HELP;
        break;
      case FL_CURSOR_MOVE:
        n = IDC_SIZEALL;
        break;
      case FL_CURSOR_N:
      case FL_CURSOR_S:
      // FIXME: Should probably have fallbacks for these instead
      case FL_CURSOR_NS:
        n = IDC_SIZENS;
        break;
      case FL_CURSOR_NE:
      case FL_CURSOR_SW:
      // FIXME: Dito.
      case FL_CURSOR_NESW:
        n = IDC_SIZENESW;
        break;
      case FL_CURSOR_E:
      case FL_CURSOR_W:
      // FIXME: Dito.
      case FL_CURSOR_WE:
        n = IDC_SIZEWE;
        break;
      case FL_CURSOR_SE:
      case FL_CURSOR_NW:
      // FIXME: Dito.
      case FL_CURSOR_NWSE:
        n = IDC_SIZENWSE;
        break;
      default:
        return 0;
    }

    new_cursor = LoadCursor(NULL, n);
    if (new_cursor == NULL)
      return 0;
  }

  if ((cursor != NULL) && custom_cursor)
    DestroyIcon(cursor);

  cursor = new_cursor;
  custom_cursor = 0;

  SetCursor(cursor);

  return 1;
}

int Fl_WinAPI_Window_Driver::set_cursor(const Fl_RGB_Image *image, int hotx, int hoty) {
  HCURSOR new_cursor;
  Fl_RGB_Image *scaled_image = (Fl_RGB_Image*)image->copy();
  new_cursor = image_to_icon(scaled_image, false, hotx, hoty);
  delete scaled_image;
  if (new_cursor == NULL)
    return 0;

  if ((cursor != NULL) && custom_cursor)
    DestroyIcon(cursor);

  cursor = new_cursor;
  custom_cursor = 1;

  SetCursor(cursor);

  return 1;
}


////////////////////////////////////////////////////////////////
// Implement the virtual functions for the base Fl_Window class:

void Fl_WinAPI_Window_Driver::show() {
  if (!shown()) {
    makeWindow();
  } else {
    // Once again, we would lose the capture if we activated the window.
    Fl_X *i = Fl_X::flx(pWindow);
    if (IsIconic((HWND)i->xid))
      OpenIcon((HWND)i->xid);
    if (!fl_capture)
      BringWindowToTop((HWND)i->xid);
    // ShowWindow(i->xid,fl_capture?SW_SHOWNOACTIVATE:SW_RESTORE);
  }
}

// the current context
// the current window handle, initially set to -1 so we can correctly
// allocate fl_GetDC(0)
HWND fl_window = NULL;

// Here we ensure only one GetDC is ever in place.
HDC fl_GetDC(HWND w) {
  HDC gc = (HDC)Fl_Graphics_Driver::default_driver().gc();
  if (gc) {
    if (w == fl_window && fl_window != NULL)
      return gc;
    if (fl_window)
      fl_release_dc(fl_window, gc); // ReleaseDC
  }
  gc = GetDC(w);
  Fl_Graphics_Driver::default_driver().gc(gc);
  fl_save_dc(w, gc);
  fl_window = w;
  // calling GetDC seems to always reset these: (?)
  SetTextAlign(gc, TA_BASELINE | TA_LEFT);
  SetBkMode(gc, TRANSPARENT);

  return gc;
}


/* Make sure that all allocated fonts are released. This works only if
   Fl::run() is allowed to exit by closing all windows. Calling 'exit(int)'
   will not automatically free any fonts. */
void fl_free_fonts(void) {
  // remove the Fl_Font_Descriptor chains
  int i;
  Fl_Fontdesc *s;
  Fl_Font_Descriptor *f;
  Fl_Font_Descriptor *ff;
  for (i = 0; i < FL_FREE_FONT; i++) {
    s = fl_fonts + i;
    for (f = s->first; f; f = ff) {
      ff = f->next;
      delete (Fl_GDI_Font_Descriptor*)f;
      s->first = ff;
    }
  }
}


///////////////////////////////////////////////////////////////////////
//
//  The following routines help fix a problem with the leaking of Windows
//  Device Context (DC) objects. The 'proper' protocol is for a program to
//  acquire a DC, save its state, do the modifications needed for drawing,
//  perform the drawing, restore the initial state, and release the DC. In
//  FLTK, the save and restore steps have previously been omitted and DCs are
//  not properly released, leading to a great number of DC leaks. As some
//  Windows "OSs" will hang when any process exceeds roughly 10,000 GDI objects,
//  it is important to control GDI leaks, which are much more important than memory
//  leaks. The following struct, global variable, and routines help implement
//  the above protocol for those cases where the GetDC and RestoreDC are not in
//  the same routine. For each GetDC, fl_save_dc is used to create an entry in
//  a linked list that saves the window handle, the DC handle, and the initial
//  state. When the DC is to be released, 'fl_release_dc' is called. It restores
//  the initial state and releases the DC. When the program exits, 'fl_cleanup_dc_list'
//  frees any remaining nodes in the list.

struct Win_DC_List { // linked list
  HWND window;       // window handle
  HDC dc;            // device context handle
  int saved_dc;      // initial state of DC
  Win_DC_List *next; // pointer to next item
};

static Win_DC_List *win_DC_list = 0;

void fl_save_dc(HWND w, HDC dc) {
  Win_DC_List *t;
  t = new Win_DC_List;
  t->window = w;
  t->dc = dc;
  t->saved_dc = SaveDC(dc);
  if (win_DC_list)
    t->next = win_DC_list;
  else
    t->next = NULL;
  win_DC_list = t;
}

void fl_release_dc(HWND w, HDC dc) {
  Win_DC_List *t = win_DC_list;
  Win_DC_List *prev = 0;
  if (!t)
    return;
  do {
    if (t->dc == dc) {
      RestoreDC(dc, t->saved_dc);
      ReleaseDC(w, dc);
      if (!prev) {
        win_DC_list = t->next; // delete first item
      } else {
        prev->next = t->next; // one in the middle
      }
      delete (t);
      return;
    }
    prev = t;
    t = t->next;
  } while (t);
}

void fl_cleanup_dc_list(void) { // clean up the list
  Win_DC_List *t = win_DC_list;
  if (!t)
    return;
  do {
    RestoreDC(t->dc, t->saved_dc);
    ReleaseDC(t->window, t->dc);
    win_DC_list = t->next;
    delete (t);
    t = win_DC_list;
  } while (t);
}

/* Returns images of the captures of the window title-bar, and the left, bottom and right window borders.
 This function exploits a feature of Fl_WinAPI_Screen_Driver::read_win_rectangle() which,
 when fl_gc is set to the screen device context, captures the window decoration.
 */
void Fl_WinAPI_Window_Driver::capture_titlebar_and_borders(Fl_RGB_Image *&top, Fl_RGB_Image *&left,
                                                           Fl_RGB_Image *&bottom, Fl_RGB_Image *&right) {
  top = left = bottom = right = NULL;
  if (!shown() || parent() || !border() || !visible())
    return;
  int wsides, hbottom, bt;
  float scaling = Fl::screen_driver()->scale(screen_num());
  RECT r = border_width_title_bar_height(wsides, hbottom, bt);
  int htop = bt + hbottom;
  Fl_Surface_Device::push_current(Fl_Display_Device::display_device());
  pWindow->show();
  while (Fl::ready())
    Fl::check();
  HDC save_gc = (HDC)fl_graphics_driver->gc();
  fl_graphics_driver->gc(GetDC(NULL));
  int ww = int(w() * scaling) + 2 * wsides;
  wsides = int(wsides / scaling);
  if (wsides < 1)
    wsides = 1;
  ww = int(ww / scaling);
  if (wsides <= 1)
    ww = w() + 2 * wsides;
  // capture the 4 window sides from screen
  int offset = r.left < 0 ? -r.left : 0;
  Fl_WinAPI_Screen_Driver *dr = (Fl_WinAPI_Screen_Driver *)Fl::screen_driver();
  if (htop && r.right - r.left > offset) {
    top = dr->read_win_rectangle_unscaled(r.left+offset, r.top, r.right - r.left-offset, htop, 0);
    if (scaling != 1 && top)
      top->scale(ww, int(htop / scaling), 0, 1);
  }
  if (wsides) {
    left = dr->read_win_rectangle_unscaled(r.left + offset, r.top + htop, wsides, int(h() * scaling), 0);
    right = dr->read_win_rectangle_unscaled(r.right - wsides, r.top + htop, wsides, int(h() * scaling), 0);
    bottom = dr->read_win_rectangle_unscaled(r.left+offset, r.bottom - hbottom, ww, hbottom, 0);
    if (scaling != 1) {
      if (left) left->scale(wsides, h(), 0, 1);
      if (right) right->scale(wsides, h(), 0, 1);
      if (bottom) bottom->scale(ww, hbottom, 0, 1);
    }
  }
  ReleaseDC(NULL, (HDC)fl_graphics_driver->gc());
  fl_graphics_driver->gc(save_gc);
  Fl_Surface_Device::pop_current();
}
