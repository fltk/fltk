//
// "$Id: Fl_win32.cxx,v 1.33.2.37 2001/04/27 15:43:38 easysw Exp $"
//
// WIN32-specific code for the Fast Light Tool Kit (FLTK).
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

// This file contains win32-specific code for fltk which is always linked
// in.  Search other files for "WIN32" or filenames ending in _win32.C
// for other system-specific code.

#include <config.h>
#include <FL/Fl.H>
#include <FL/win32.H>
#include <FL/Fl_Window.H>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <winsock.h>
#include <ctype.h>

//
// USE_ASYNC_SELECT - define it if you have WSAAsyncSelect()...
//

// MRS - this currently doesn't appear to work anymore; needs to be fixed!
//#define USE_ASYNC_SELECT

//
// WM_SYNCPAINT is an "undocumented" message, which is finally defined in
// VC++ 6.0.
//
#ifndef WM_SYNCPAINT
#  define WM_SYNCPAINT 0x0088
#endif /* !WM_SYNCPAINT */

#ifndef WM_MOUSELEAVE
#  define WM_MOUSELEAVE 0x02a3
#endif

//
// WM_FLSELECT is the user-defined message that we get when one of
// the sockets has pending data, etc.
//

#define WM_FLSELECT	(WM_USER+0x0400)

////////////////////////////////////////////////////////////////
// interface to poll/select call:

// fd's are only implemented for sockets.  Microsoft Windows does not
// have a unified IO system, so it doesn't support select() on files,
// devices, or pipes...
//
// Microsoft provides the Berkeley select() call and an asynchronous
// select function that sends a WIN32 message when the select condition
// exists...

#ifndef USE_ASYNC_SELECT
static fd_set fdsets[3];
#endif // !USE_ASYNC_SELECT

#define POLLIN 1
#define POLLOUT 4
#define POLLERR 8

static int nfds = 0;
static int fd_array_size = 0;
static struct FD {
  int fd;
  short events;
  void (*cb)(int, void*);
  void* arg;
} *fd = 0;

void Fl::add_fd(int n, int events, void (*cb)(int, void*), void *v) {
  remove_fd(n,events);
  int i = nfds++;
  if (i >= fd_array_size) {
    fd_array_size = 2*fd_array_size+1;
    fd = (FD*)realloc(fd, fd_array_size*sizeof(FD));
  }
  fd[i].fd = n;
  fd[i].events = events;
  fd[i].cb = cb;
  fd[i].arg = v;

#ifdef USE_ASYNC_SELECT
  int mask = 0;
  if (events & POLLIN) mask |= FD_READ;
  if (events & POLLOUT) mask |= FD_WRITE;
  if (events & POLLERR) mask |= FD_CLOSE;
  WSAAsyncSelect(n, fl_window, WM_FLSELECT, mask);
#else
  if (events & POLLIN) FD_SET(n, &fdsets[0]);
  if (events & POLLOUT) FD_SET(n, &fdsets[1]);
  if (events & POLLERR) FD_SET(n, &fdsets[2]);
#endif // USE_ASYNC_SELECT
}

void Fl::add_fd(int fd, void (*cb)(int, void*), void* v) {
  Fl::add_fd(fd, POLLIN, cb, v);
}

void Fl::remove_fd(int n, int events) {
  int i,j;
  for (i=j=0; i<nfds; i++) {
    if (fd[i].fd == n) {
      int e = fd[i].events & ~events;
      if (!e) continue; // if no events left, delete this fd
      fd[i].events = e;
    }
    // move it down in the array if necessary:
    if (j<i) {
      fd[j]=fd[i];
    }
    j++;
  }
  nfds = j;

#ifdef USE_ASYNC_SELECT
  WSAAsyncSelect(n, 0, 0, 0);
#else
  if (events & POLLIN) FD_CLR(unsigned(n), &fdsets[0]);
  if (events & POLLOUT) FD_CLR(unsigned(n), &fdsets[1]);
  if (events & POLLERR) FD_CLR(unsigned(n), &fdsets[2]);
#endif // USE_ASYNC_SELECT
}

void Fl::remove_fd(int n) {
  remove_fd(n, -1);
}

MSG fl_msg;

// This is never called with time_to_wait < 0.0.
// It *should* return negative on error, 0 if nothing happens before
// timeout, and >0 if any callbacks were done.  This version only
// returns zero if nothing happens during a 0.0 timeout, otherwise
// it returns 1.
int fl_wait(double time_to_wait) {
  int have_message = 0;
  int timerid;

#ifndef USE_ASYNC_SELECT
  if (nfds) {
    // For WIN32 we need to poll for socket input FIRST, since
    // the event queue is not something we can select() on...
    timeval t;
    t.tv_sec = 0;
    t.tv_usec = 0;

    fd_set fdt[3];
    fdt[0] = fdsets[0];
    fdt[1] = fdsets[1];
    fdt[2] = fdsets[2];

    if (::select(0,&fdt[0],&fdt[1],&fdt[2],&t)) {
      // We got something - do the callback!
      for (int i = 0; i < nfds; i ++) {
	int f = fd[i].fd;
	short revents = 0;
	if (FD_ISSET(f,&fdt[0])) revents |= POLLIN;
	if (FD_ISSET(f,&fdt[1])) revents |= POLLOUT;
	if (FD_ISSET(f,&fdt[2])) revents |= POLLERR;
	if (fd[i].events & revents) fd[i].cb(f, fd[i].arg);
      }
      time_to_wait = 0.0; // just peek for any messages
    } else {
      // we need to check them periodically, so set a short timeout:
      if (time_to_wait > .001) time_to_wait = .001;
    }
  }
#endif // USE_ASYNC_SELECT

  if (time_to_wait < 2147483.648) {
    // Perform the requested timeout...
    have_message = PeekMessage(&fl_msg, NULL, 0, 0, PM_REMOVE);
    if (!have_message) {
      int t = (int)(time_to_wait * 1000.0 + .5);
      if (t <= 0) return 0; // too short to measure
      timerid = SetTimer(NULL, 0, t, NULL);
      have_message = GetMessage(&fl_msg, NULL, 0, 0);
      KillTimer(NULL, timerid);
    }
  } else {
    have_message = GetMessage(&fl_msg, NULL, 0, 0);
  }

  // Execute the message we got, and all other pending messages:
  while (have_message) {
#ifdef USE_ASYNC_SELECT
    if (fl_msg.message == WM_FLSELECT) {
      // Got notification for socket
      for (int i = 0; i < nfds; i ++)
        if (fd[i].fd == (int)fl_msg.wParam) {
	  (fd[i].cb)(fd[i].fd, fd[i].arg);
	  break;
	}
      // looks like it is best to do the dispatch-message anyway:
    }
#endif
    TranslateMessage(&fl_msg);
    DispatchMessage(&fl_msg);
    have_message = PeekMessage(&fl_msg, NULL, 0, 0, PM_REMOVE);
  }

  // This should return 0 if only timer events were handled:
  return 1;
}

// fl_ready() is just like fl_wait(0.0) except no callbacks are done:
int fl_ready() {
  if (PeekMessage(&fl_msg, NULL, 0, 0, PM_NOREMOVE)) return 1;
#ifdef USE_ASYNC_SELECT
  return 0;
#else
  timeval t;
  t.tv_sec = 0;
  t.tv_usec = 0;
  fd_set fdt[3];
  fdt[0] = fdsets[0];
  fdt[1] = fdsets[1];
  fdt[2] = fdsets[2];
  return ::select(0,&fdt[0],&fdt[1],&fdt[2],&t);
#endif // USE_ASYNC_SELECT
}

////////////////////////////////////////////////////////////////

int Fl::x()
{
  RECT r;

  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.left;
}

int Fl::y()
{
  RECT r;

  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.top;
}

int Fl::h()
{
  RECT r;

  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.bottom - r.top;
}

int Fl::w()
{
  RECT r;

  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.right - r.left;
}

void Fl::get_mouse(int &x, int &y) {
  POINT p;
  GetCursorPos(&p);
  x = p.x;
  y = p.y;
}

////////////////////////////////////////////////////////////////

HWND fl_capture;

static int mouse_event(Fl_Window *window, int what, int button,
			WPARAM wParam, LPARAM lParam)
{
  static int px, py, pmx, pmy;
  POINT pt;
  Fl::e_x = pt.x = (signed short)LOWORD(lParam);
  Fl::e_y = pt.y = (signed short)HIWORD(lParam);
  ClientToScreen(fl_xid(window), &pt);
  Fl::e_x_root = pt.x;
  Fl::e_y_root = pt.y;
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
  if (wParam & MK_LBUTTON) state |= FL_BUTTON1;
  if (wParam & MK_MBUTTON) state |= FL_BUTTON2;
  if (wParam & MK_RBUTTON) state |= FL_BUTTON3;
  Fl::e_state = state;

  switch (what) {
  case 1: // double-click
    if (Fl::e_is_click) {Fl::e_clicks++; goto J1;}
  case 0: // single-click
    Fl::e_clicks = 0;
  J1:
    if (!fl_capture) SetCapture(fl_xid(window));
    Fl::e_keysym = FL_Button + button;
    Fl::e_is_click = 1;
    px = pmx = Fl::e_x_root; py = pmy = Fl::e_y_root;
    return Fl::handle(FL_PUSH,window);

  case 2: // release:
    if (!fl_capture) ReleaseCapture();
    Fl::e_keysym = FL_Button + button;
    return Fl::handle(FL_RELEASE,window);

  case 3: // move:
  default: // avoid compiler warning
    // MSWindows produces extra events even if mouse does not move, ignore em:
    if (Fl::e_x_root == pmx && Fl::e_y_root == pmy) return 1;
    pmx = Fl::e_x_root; pmy = Fl::e_y_root;
    if (abs(Fl::e_x_root-px)>5 || abs(Fl::e_y_root-py)>5) Fl::e_is_click = 0;
    return Fl::handle(FL_MOVE,window);

  }
}

// convert a MSWindows VK_x to an Fltk (X) Keysym:
// See also the inverse converter in Fl_get_key_win32.C
// This table is in numeric order by VK:
static const struct {unsigned short vk, fltk, extended;} vktab[] = {
  {VK_BACK,	FL_BackSpace},
  {VK_TAB,	FL_Tab},
  {VK_CLEAR,	FL_KP+'5',	0xff0b/*XK_Clear*/},
  {VK_RETURN,	FL_Enter,	FL_KP_Enter},
  {VK_SHIFT,	FL_Shift_L,	FL_Shift_R},
  {VK_CONTROL,	FL_Control_L,	FL_Control_R},
  {VK_MENU,	FL_Alt_L,	FL_Alt_R},
  {VK_PAUSE,	FL_Pause},
  {VK_CAPITAL,	FL_Caps_Lock},
  {VK_ESCAPE,	FL_Escape},
  {VK_SPACE,	' '},
  {VK_PRIOR,	FL_KP+'9',	FL_Page_Up},
  {VK_NEXT,	FL_KP+'3',	FL_Page_Down},
  {VK_END,	FL_KP+'1',	FL_End},
  {VK_HOME,	FL_KP+'7',	FL_Home},
  {VK_LEFT,	FL_KP+'4',	FL_Left},
  {VK_UP,	FL_KP+'8',	FL_Up},
  {VK_RIGHT,	FL_KP+'6',	FL_Right},
  {VK_DOWN,	FL_KP+'2',	FL_Down},
  {VK_SNAPSHOT,	FL_Print},	// does not work on NT
  {VK_INSERT,	FL_KP+'0',	FL_Insert},
  {VK_DELETE,	FL_KP+'.',	FL_Delete},
  {VK_LWIN,	FL_Meta_L},
  {VK_RWIN,	FL_Meta_R},
  {VK_APPS,	FL_Menu},
  {VK_MULTIPLY,	FL_KP+'*'},
  {VK_ADD,	FL_KP+'+'},
  {VK_SUBTRACT,	FL_KP+'-'},
  {VK_DECIMAL,	FL_KP+'.'},
  {VK_DIVIDE,	FL_KP+'/'},
  {VK_NUMLOCK,	FL_Num_Lock},
  {VK_SCROLL,	FL_Scroll_Lock},
  {0xba,	';'},
  {0xbb,	'='},
  {0xbc,	','},
  {0xbd,	'-'},
  {0xbe,	'.'},
  {0xbf,	'/'},
  {0xc0,	'`'},
  {0xdb,	'['},
  {0xdc,	'\\'},
  {0xdd,	']'},
  {0xde,	'\''}
};
static int ms2fltk(int vk, int extended) {
  static unsigned short vklut[256];
  static unsigned short extendedlut[256];
  if (!vklut[1]) { // init the table
    unsigned int i;
    for (i = 0; i < 256; i++) vklut[i] = tolower(i);
    for (i=VK_F1; i<=VK_F16; i++) vklut[i] = i+(FL_F-(VK_F1-1));
    for (i=VK_NUMPAD0; i<=VK_NUMPAD9; i++) vklut[i] = i+(FL_KP+'0'-VK_NUMPAD0);
    for (i = 0; i < sizeof(vktab)/sizeof(*vktab); i++) {
      vklut[vktab[i].vk] = vktab[i].fltk;
      extendedlut[vktab[i].vk] = vktab[i].extended;
    }
    for (i = 0; i < 256; i++) if (!extendedlut[i]) extendedlut[i] = vklut[i];
  }
  return extended ? extendedlut[vk] : vklut[vk];
}

#if USE_COLORMAP
extern HPALETTE fl_select_palette(void); // in fl_color_win32.C
#endif

static Fl_Window* resize_bug_fix;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // Matt: When dragging a full window, MSWindows on 'slow'
  // machines can lose track of the window refresh area. It sends some kind
  // of panic message to the desktop that in turn sends this message on to
  // all applications.
  static int cnt=0;
  if (uMsg == WM_SYNCPAINT) {
    MSG msg;
    if ( PeekMessage( &msg, hWnd, WM_PAINT, WM_PAINT, false )==0 )
      InvalidateRect(hWnd,0,FALSE);
    if (cnt) {
      InvalidateRect(fl_window,0,FALSE);
      cnt = 0;
    } else cnt = 1;
  } else if (uMsg == WM_PAINT) cnt = 0;

  fl_msg.message = uMsg;

  Fl_Window *window = fl_find(hWnd);

  if (window) switch (uMsg) {

  case WM_QUIT: // this should not happen?
    Fl::fatal("WM_QUIT message");

  case WM_CLOSE: // user clicked close box
    Fl::handle(FL_CLOSE, window);
    return 0;

  case WM_PAINT: {

    Fl_X *i = Fl_X::i(window);
    i->wait_for_expose = 0;
    // We need to merge this damage into fltk's damage.  I do this in
    // reverse, telling Win32 about fltk's damage and then reading back
    // the new accumulated region.
    if (window->damage()) {
      // If there is no region the entire window is damaged
      if (i->region) {
	InvalidateRgn(hWnd,i->region,FALSE);
	GetUpdateRgn(hWnd,i->region,0);
      }
    } else {
      if (!i->region) i->region = CreateRectRgn(0,0,0,0);
      GetUpdateRgn(hWnd,i->region,0);
    }
    window->clear_damage(window->damage()|FL_DAMAGE_EXPOSE);
    // These next two statements should not be here, so that all update
    // is deferred until Fl::flush() is called during idle.  However Win32
    // apparently is very unhappy if we don't obey it and draw right now.
    // Very annoying!
    i->flush();
    window->clear_damage();
    // This convinces MSWindows we have painted whatever they wanted
    // us to paint, and stops it from sending WM_PAINT messages:
    ValidateRgn(hWnd,NULL);
    } break;

  case WM_LBUTTONDOWN:  mouse_event(window, 0, 1, wParam, lParam); return 0;
  case WM_LBUTTONDBLCLK:mouse_event(window, 1, 1, wParam, lParam); return 0;
  case WM_LBUTTONUP:    mouse_event(window, 2, 1, wParam, lParam); return 0;
  case WM_MBUTTONDOWN:  mouse_event(window, 0, 2, wParam, lParam); return 0;
  case WM_MBUTTONDBLCLK:mouse_event(window, 1, 2, wParam, lParam); return 0;
  case WM_MBUTTONUP:    mouse_event(window, 2, 2, wParam, lParam); return 0;
  case WM_RBUTTONDOWN:  mouse_event(window, 0, 3, wParam, lParam); return 0;
  case WM_RBUTTONDBLCLK:mouse_event(window, 1, 3, wParam, lParam); return 0;
  case WM_RBUTTONUP:    mouse_event(window, 2, 3, wParam, lParam); return 0;
  case WM_MOUSEMOVE:    mouse_event(window, 3, 0, wParam, lParam); return 0;

#ifdef WM_MOUSELEAVE
  case WM_MOUSELEAVE:
    Fl::handle(FL_LEAVE, window);
    break;
#endif /* WM_MOUSELEAVE */

  case WM_SETFOCUS:
    Fl::handle(FL_FOCUS, window);
    break;

  case WM_KILLFOCUS:
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
    if (wParam)
    {
      ulong state = 0;
      if (GetAsyncKeyState(VK_CAPITAL)) state |= FL_CAPS_LOCK;
      if (GetAsyncKeyState(VK_NUMLOCK)) state |= FL_NUM_LOCK;
      if (GetAsyncKeyState(VK_SCROLL)) state |= FL_SCROLL_LOCK;
      if (GetAsyncKeyState(VK_CONTROL)&~1) state |= FL_CTRL;
      if (GetAsyncKeyState(VK_SHIFT)&~1) state |= FL_SHIFT;
      if (GetAsyncKeyState(VK_MENU)) state |= FL_ALT;
      if ((GetAsyncKeyState(VK_LWIN)|GetAsyncKeyState(VK_RWIN))&~1) state |= FL_META;
      Fl::e_state = state;
      return 0;
    }
    break;

  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP:
    // save the keysym until we figure out the characters:
    Fl::e_keysym = ms2fltk(wParam,lParam&(1<<24));
    // See if TranslateMessage turned it into a WM_*CHAR message:
    if (PeekMessage(&fl_msg, hWnd, WM_CHAR, WM_SYSDEADCHAR, 1)) {
      uMsg = fl_msg.message;
      wParam = fl_msg.wParam;
      lParam = fl_msg.lParam;
    }
  case WM_DEADCHAR:
  case WM_SYSDEADCHAR:
  case WM_CHAR:
  case WM_SYSCHAR: {
    ulong state = Fl::e_state & 0xff000000; // keep the mouse button state
    // if GetKeyState is expensive we might want to comment some of these out:
    if (GetKeyState(VK_SHIFT)&~1) state |= FL_SHIFT;
    if (GetKeyState(VK_CAPITAL)) state |= FL_CAPS_LOCK;
    if (GetKeyState(VK_CONTROL)&~1) state |= FL_CTRL;
    // Alt gets reported for the Alt-GR switch on foreign keyboards.
    // so we need to check the event as well to get it right:
    if ((lParam&(1<<29)) //same as GetKeyState(VK_MENU)
	&& uMsg != WM_CHAR) state |= FL_ALT;
    if (GetKeyState(VK_NUMLOCK)) state |= FL_NUM_LOCK;
    if ((GetKeyState(VK_LWIN)|GetKeyState(VK_RWIN))&~1) {
      // WIN32 bug?  GetKeyState returns garbage if the user hit the
      // meta key to pop up start menu.  Sigh.
      if ((GetAsyncKeyState(VK_LWIN)|GetAsyncKeyState(VK_RWIN))&~1)
	state |= FL_META;
    }
    if (GetKeyState(VK_SCROLL)) state |= FL_SCROLL_LOCK;
    Fl::e_state = state;
    if (lParam & (1<<31)) goto DEFAULT; // ignore up events after fixing shift
    static char buffer[2];
    if (uMsg == WM_CHAR || uMsg == WM_SYSCHAR) {
      buffer[0] = char(wParam);
      Fl::e_length = 1;
    } else if (Fl::e_keysym >= FL_KP && Fl::e_keysym <= FL_KP_Last) {
      buffer[0] = Fl::e_keysym-FL_KP;
      Fl::e_length = 1;
    } else {
      buffer[0] = 0;
      Fl::e_length = 0;
    }
    Fl::e_text = buffer;
    // for (int i = lParam&0xff; i--;)
    while (window->parent()) window = window->window();
    if (Fl::handle(FL_KEYBOARD,window)) return 0;
    break;}

  case WM_GETMINMAXINFO:
    Fl_X::i(window)->set_minmax((LPMINMAXINFO)lParam);
    break;

  case WM_SIZE:
    if (!window->parent()) {
      if (wParam == SIZE_MINIMIZED || wParam == SIZE_MAXHIDE) {
	Fl::handle(FL_HIDE, window);
      } else {
	Fl::handle(FL_SHOW, window);
	resize_bug_fix = window;
	window->size(LOWORD(lParam), HIWORD(lParam));
      }
    }
    break;

  case WM_MOVE:
    resize_bug_fix = window;
    window->position(LOWORD(lParam), HIWORD(lParam));
    break;

  case WM_SETCURSOR:
    if (LOWORD(lParam) == HTCLIENT) {
      while (window->parent()) window = window->window();
      SetCursor(Fl_X::i(window)->cursor);
      return 0;
    }
    break;

#if USE_COLORMAP
  case WM_QUERYNEWPALETTE :
    fl_GetDC(hWnd);
    if (fl_select_palette()) InvalidateRect(hWnd, NULL, FALSE);
    break;

  case WM_PALETTECHANGED:
    fl_GetDC(hWnd);
    if ((HWND)wParam != hWnd && fl_select_palette()) UpdateColors(fl_gc);
    break;

  case WM_CREATE :
    fl_GetDC(hWnd);
    fl_select_palette();
    break;

#endif

  default:
  DEFAULT:
    if (Fl::handle(0,0)) return 0;
    break;
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

////////////////////////////////////////////////////////////////
// This function gets the dimensions of the top/left borders and
// the title bar, if there is one, based on the FL_BORDER, FL_MODAL
// and FL_NONMODAL flags, and on the window's size range.
// It returns the following values:
//
// value | border | title bar
//   0   |  none  |   no
//   1   |  fix   |   yes
//   2   |  size  |   yes

int Fl_X::fake_X_wm(const Fl_Window* w,int &X,int &Y, int &bt,int &bx, int &by) {
  int W, H, xoff, yoff, dx, dy;
  int ret = bx = by = bt = 0;
  if (w->border() && !w->parent()) {
    if (w->maxw != w->minw || w->maxh != w->minh) {
      ret = 2;
      bx = GetSystemMetrics(SM_CXSIZEFRAME);
      by = GetSystemMetrics(SM_CYSIZEFRAME);
    } else {
      ret = 1;
      bx = GetSystemMetrics(SM_CXFIXEDFRAME);
      by = GetSystemMetrics(SM_CYFIXEDFRAME);
    }
    bt = GetSystemMetrics(SM_CYCAPTION);
  }
  //The coordinates of the whole window, including non-client area
  xoff = bx;
  yoff = by + bt;
  dx = 2*bx;
  dy = 2*by + bt;
  X = w->x()-xoff;
  Y = w->y()-yoff;
  W = w->w()+dx;
  H = w->h()+dy;

  //Proceed to positioning the window fully inside the screen, if possible
  //Make border's lower right corner visible
  if (Fl::w() < X+W) X = Fl::w() - W;
  if (Fl::h() < Y+H) Y = Fl::h() - H;
  //Make border's upper left corner visible
  if (X<0) X = 0;
  if (Y<0) Y = 0;
  //Make client area's lower right corner visible
  if (Fl::w() < X+dx+ w->w()) X = Fl::w() - w->w() - dx;
  if (Fl::h() < Y+dy+ w->h()) Y = Fl::h() - w->h() - dy;
  //Make client area's upper left corner visible
  if (X+xoff < 0) X = -xoff;
  if (Y+yoff < 0) Y = -yoff;
  //Return the client area's top left corner in (X,Y)
  X+=xoff;
  Y+=yoff;

  return ret;
}

////////////////////////////////////////////////////////////////

void Fl_Window::resize(int X,int Y,int W,int H) {
  UINT flags = SWP_NOSENDCHANGING | SWP_NOZORDER;
  int is_a_resize = (W != w() || H != h());
  int resize_from_program = (this != resize_bug_fix);
  if (!resize_from_program) resize_bug_fix = 0;
  if (X != x() || Y != y()) set_flag(FL_FORCE_POSITION);
    else {if (!is_a_resize) return; flags |= SWP_NOMOVE;}
  if (is_a_resize) {
    Fl_Group::resize(X,Y,W,H);
    if (shown()) {redraw(); i->wait_for_expose = 1;}
  } else {
    x(X); y(Y);
    flags |= SWP_NOSIZE;
  }
  if (resize_from_program && shown()) {
    int dummy, bt, bx, by;
    //Ignore window managing when resizing, so that windows (and more
    //specifically menus) can be moved offscreen.
    if (Fl_X::fake_X_wm(this, dummy, dummy, bt, bx, by)) {
      X -= bx;
      Y -= by+bt;
      W += 2*bx;
      H += 2*by+bt;
    }
    SetWindowPos(i->xid, 0, X, Y, W, H, flags);
  }
}

////////////////////////////////////////////////////////////////

void fl_fix_focus(); // in Fl.C

char fl_show_iconic;	// hack for Fl_Window::iconic()
// int fl_background_pixel = -1; // color to use for background
HCURSOR fl_default_cursor;
int fl_disable_transient_for; // secret method of removing TRANSIENT_FOR

Fl_X* Fl_X::make(Fl_Window* w) {
  Fl_Group::current(0); // get rid of very common user bug: forgot end()

  const char* class_name = /*w->xclass();
  if (!class_name) class_name =*/ "FLTK"; // create a "FLTK" WNDCLASS

  WNDCLASSEX wc;
  // Documentation states a device context consumes about 800 bytes
  // of memory... so who cares? If 800 bytes per window is what it
  // takes to speed things up, I'm game.
  //wc.style = CS_HREDRAW | CS_VREDRAW | CS_CLASSDC | CS_DBLCLKS;
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
  wc.lpfnWndProc = (WNDPROC)WndProc;
  wc.cbClsExtra = wc.cbWndExtra = 0;
  wc.hInstance = fl_display;
  if (!w->icon())
    w->icon((void *)LoadIcon(NULL, IDI_APPLICATION));
  wc.hIcon = wc.hIconSm = (HICON)w->icon();
  wc.hCursor = fl_default_cursor = LoadCursor(NULL, IDC_ARROW);
  //uchar r,g,b; Fl::get_color(FL_GRAY,r,g,b);
  //wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(r,g,b));
  wc.hbrBackground = NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = class_name;
  wc.cbSize = sizeof(WNDCLASSEX);
  RegisterClassEx(&wc);

  HWND parent;
  DWORD style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
  DWORD styleEx = WS_EX_LEFT;

  int xp = w->x();
  int yp = w->y();
  int wp = w->w();
  int hp = w->h();

  int showit = 1;

  if (w->parent()) {
    style |= WS_CHILD;
    styleEx |= WS_EX_WINDOWEDGE | WS_EX_CONTROLPARENT;
    parent = fl_xid(w->window());
  } else {
    if (!w->size_range_set) {
      if (w->resizable()) {
	Fl_Widget *o = w->resizable();
	int minw = o->w(); if (minw > 100) minw = 100;
	int minh = o->h(); if (minh > 100) minh = 100;
	w->size_range(w->w() - o->w() + minw, w->h() - o->h() + minh, 0, 0);
      } else {
	w->size_range(w->w(), w->h(), w->w(), w->h());
      }
    }
    styleEx |= WS_EX_WINDOWEDGE | WS_EX_CONTROLPARENT;
    int xwm = xp , ywm = yp , bt, bx, by;
    switch (fake_X_wm(w, xwm, ywm, bt, bx, by)) {
      // No border (user for menus)
      case 0: style |= WS_POPUP; break;

      // Thin border and title bar
      case 1: style |= WS_DLGFRAME | WS_CAPTION; break;

      // Thick, resizable border and title bar, with maximize button
      case 2: style |= WS_THICKFRAME | WS_MAXIMIZEBOX | WS_CAPTION ; break;
    }
    if (by+bt) {
      if (!w->modal()) style |= WS_SYSMENU | WS_MINIMIZEBOX;
      wp += 2*bx;
      hp += 2*by+bt;
    }
    if (!(w->flags() & Fl_Window::FL_FORCE_POSITION)) {
      xp = yp = CW_USEDEFAULT;
    } else {
      if (!Fl::grab()) {
	xp = xwm; yp = ywm;
        w->x(xp);w->y(yp);
      }
      xp -= bx;
      yp -= by+bt;
    }

    parent = 0;
    if (w->non_modal() && Fl_X::first && !fl_disable_transient_for) {
      // find some other window to be "transient for":
      Fl_Window* w = Fl_X::first->w;
      while (w->parent()) w = w->window();
      parent = fl_xid(w);
      if (!w->visible()) showit = 0;
    }
  }

  Fl_X* x = new Fl_X;
  x->other_xid = 0;
  x->setwindow(w);
  x->region = 0;
  x->private_dc = 0;
  x->cursor = fl_default_cursor;
  x->xid = CreateWindowEx(
    styleEx,
    class_name, w->label(), style,
    xp, yp, wp, hp,
    parent,
    NULL, // menu
    fl_display,
    NULL // creation parameters
    );
  x->next = Fl_X::first;
  Fl_X::first = x;

  x->wait_for_expose = 1;
  if (fl_show_iconic) {showit = 0; fl_show_iconic = 0;}
  if (showit) {
    w->set_visible();
    w->handle(FL_SHOW); // get child windows to appear
    w->redraw(); // force draw to happen
  }
  // If we've captured the mouse, we dont want do activate any
  // other windows from the code, or we loose the capture.
  ShowWindow(x->xid, !showit ? SW_SHOWMINNOACTIVE :
	     fl_capture? SW_SHOWNOACTIVATE : SW_SHOWNORMAL);
  if (w->modal()) {Fl::modal_ = w; fl_fix_focus();}
  return x;
}

////////////////////////////////////////////////////////////////

HINSTANCE fl_display = GetModuleHandle(NULL);

void Fl_Window::size_range_() {
  size_range_set = 1;
}

void Fl_X::set_minmax(LPMINMAXINFO minmax)
{
  int td, wd, hd, dummy;

  fake_X_wm(w, dummy, dummy, td, wd, hd);
  wd *= 2;
  hd *= 2;
  hd += td;

  minmax->ptMinTrackSize.x = w->minw + wd;
  minmax->ptMinTrackSize.y = w->minh + hd;
  if (w->maxw) {
    minmax->ptMaxTrackSize.x = w->maxw + wd;
    minmax->ptMaxSize.x = w->maxw + wd;
  }
  if (w->maxh) {
    minmax->ptMaxTrackSize.y = w->maxh + hd;
    minmax->ptMaxSize.y = w->maxh + hd;
  }
}

////////////////////////////////////////////////////////////////

#include <FL/filename.H> // need so FL_EXPORT filename_name works

// returns pointer to the filename, or null if name ends with '/'
const char *filename_name(const char *name) {
  const char *p,*q;
  q = name;
  if (q[0] && q[1]==':') q += 2; // skip leading drive letter
  for (p = q; *p; p++) if (*p == '/' || *p == '\\') q = p+1;
  return q;
}

void Fl_Window::label(const char *name,const char *iname) {
  Fl_Widget::label(name);
  iconlabel_ = iname;
  if (shown() && !parent()) {
    if (!name) name = "";
    SetWindowText(i->xid, name);
    // if (!iname) iname = filename_name(name);
    // should do something with iname here...
  }
}

////////////////////////////////////////////////////////////////
// Implement the virtual functions for the base Fl_Window class:

// If the box is a filled rectangle, we can make the redisplay *look*
// faster by using X's background pixel erasing.  We can make it
// actually *be* faster by drawing the frame only, this is done by
// setting fl_boxcheat, which is seen by code in fl_drawbox.C:
// For WIN32 it looks like all windows share a background color, so
// I use FL_GRAY for this and only do this cheat for windows that are
// that color.
// Actually it is totally disabled.
// Fl_Widget *fl_boxcheat;
//static inline int can_boxcheat(uchar b) {return (b==1 || (b&2) && b<=15);}

void Fl_Window::show() {
  if (!shown()) {
    // if (can_boxcheat(box())) fl_background_pixel = fl_xpixel(color());
    Fl_X::make(this);
  } else {
    // Once again, we would lose the capture if we activated the window.
    if (IsIconic(i->xid)) OpenIcon(i->xid);
    if (!fl_capture) BringWindowToTop(i->xid);
    //ShowWindow(i->xid,fl_capture?SW_SHOWNOACTIVATE:SW_RESTORE);
  }
}

Fl_Window *Fl_Window::current_;
// the current context
HDC fl_gc = 0;
// the current window handle, initially set to -1 so we can correctly
// allocate fl_GetDC(0)
HWND fl_window = (HWND)-1;

// Here we ensure only one GetDC is ever in place.
HDC fl_GetDC(HWND w) {
  if (fl_gc) {
    if (w == fl_window) return fl_gc;
    ReleaseDC(fl_window, fl_gc);
  }
  fl_gc = GetDC(w);
  fl_window = w;
  // calling GetDC seems to always reset these: (?)
  SetTextAlign(fl_gc, TA_BASELINE|TA_LEFT);
  SetBkMode(fl_gc, TRANSPARENT);
  return fl_gc;
}

// make X drawing go into this window (called by subclass flush() impl.)
void Fl_Window::make_current() {
  fl_GetDC(fl_xid(this));

#if USE_COLORMAP
  // Windows maintains a hardware and software color palette; the
  // SelectPalette() call updates the current soft->hard mapping
  // for all drawing calls, so we must select it here before any
  // code does any drawing...

  fl_select_palette();
#endif // USE_COLORMAP

  current_ = this;
  fl_clip_region(0);
}

//
// End of "$Id: Fl_win32.cxx,v 1.33.2.37 2001/04/27 15:43:38 easysw Exp $".
//
