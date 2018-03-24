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
          consumed = handle_keyboard_event(queue, event);
          break;
        case AINPUT_EVENT_TYPE_MOTION:
          consumed = handle_mouse_event(queue, event);
          break;
        default:
          // don't do anything. There may be additional event types in the future
          AInputQueue_finishEvent(queue, event, consumed);
          break;
      }
      // TODO: handle all events here
//      AInputQueue_finishEvent(queue, event, consumed);
    }
  }
  return 0;
}


int Fl_Android_Screen_Driver::compose(int &del)
{
  del = 0;
  return 1;
}


int Fl_Android_Screen_Driver::handle_keyboard_event(AInputQueue *queue, AInputEvent *event)
{
/*
int32_t 	AKeyEvent_getAction (const AInputEvent *key_event)
 { AKEY_EVENT_ACTION_DOWN = 0, AKEY_EVENT_ACTION_UP = 1, AKEY_EVENT_ACTION_MULTIPLE = 2 }
> Reading up on ACTION_MULTIPLE also explains how to deal with
> special or sequences of characters:
> "If the key code is not KEYCODE_UNKNOWN then the getRepeatCount()
> method returns the number of times the given key code should be
> executed. Otherwise, if the key code is KEYCODE_UNKNOWN, then this
> is a sequence of characters as returned by getCharacters()."

int32_t 	AKeyEvent_getFlags (const AInputEvent *key_event)
 {
  AKEY_EVENT_FLAG_WOKE_HERE = 0x1, AKEY_EVENT_FLAG_SOFT_KEYBOARD = 0x2, AKEY_EVENT_FLAG_KEEP_TOUCH_MODE = 0x4, AKEY_EVENT_FLAG_FROM_SYSTEM = 0x8,
  AKEY_EVENT_FLAG_EDITOR_ACTION = 0x10, AKEY_EVENT_FLAG_CANCELED = 0x20, AKEY_EVENT_FLAG_VIRTUAL_HARD_KEY = 0x40, AKEY_EVENT_FLAG_LONG_PRESS = 0x80,
  AKEY_EVENT_FLAG_CANCELED_LONG_PRESS = 0x100, AKEY_EVENT_FLAG_TRACKING = 0x200, AKEY_EVENT_FLAG_FALLBACK = 0x400
}

int32_t 	AKeyEvent_getKeyCode (const AInputEvent *key_event)
 {
  AKEYCODE_UNKNOWN = 0, AKEYCODE_SOFT_LEFT = 1, AKEYCODE_SOFT_RIGHT = 2, AKEYCODE_HOME = 3,
  AKEYCODE_BACK = 4, AKEYCODE_CALL = 5, AKEYCODE_ENDCALL = 6, AKEYCODE_0 = 7,
  AKEYCODE_1 = 8, AKEYCODE_2 = 9, AKEYCODE_3 = 10, AKEYCODE_4 = 11,
  AKEYCODE_5 = 12, AKEYCODE_6 = 13, AKEYCODE_7 = 14, AKEYCODE_8 = 15,
  AKEYCODE_9 = 16, AKEYCODE_STAR = 17, AKEYCODE_POUND = 18, AKEYCODE_DPAD_UP = 19,
  AKEYCODE_DPAD_DOWN = 20, AKEYCODE_DPAD_LEFT = 21, AKEYCODE_DPAD_RIGHT = 22, AKEYCODE_DPAD_CENTER = 23,
  AKEYCODE_VOLUME_UP = 24, AKEYCODE_VOLUME_DOWN = 25, AKEYCODE_POWER = 26, AKEYCODE_CAMERA = 27,
  AKEYCODE_CLEAR = 28, AKEYCODE_A = 29, AKEYCODE_B = 30, AKEYCODE_C = 31,
  AKEYCODE_D = 32, AKEYCODE_E = 33, AKEYCODE_F = 34, AKEYCODE_G = 35, ...

int32_t 	AKeyEvent_getScanCode (const AInputEvent *key_event)
  { AKEY_STATE_UNKNOWN = -1, AKEY_STATE_UP = 0, AKEY_STATE_DOWN = 1, AKEY_STATE_VIRTUAL = 2 }

int32_t 	AKeyEvent_getMetaState (const AInputEvent *key_event)
 {
  AMETA_NONE = 0, AMETA_ALT_ON = 0x02, AMETA_ALT_LEFT_ON = 0x10, AMETA_ALT_RIGHT_ON = 0x20,
  AMETA_SHIFT_ON = 0x01, AMETA_SHIFT_LEFT_ON = 0x40, AMETA_SHIFT_RIGHT_ON = 0x80, AMETA_SYM_ON = 0x04,
  AMETA_FUNCTION_ON = 0x08, AMETA_CTRL_ON = 0x1000, AMETA_CTRL_LEFT_ON = 0x2000, AMETA_CTRL_RIGHT_ON = 0x4000,
  AMETA_META_ON = 0x10000, AMETA_META_LEFT_ON = 0x20000, AMETA_META_RIGHT_ON = 0x40000, AMETA_CAPS_LOCK_ON = 0x100000,
  AMETA_NUM_LOCK_ON = 0x200000, AMETA_SCROLL_LOCK_ON = 0x400000
}

int32_t 	AKeyEvent_getRepeatCount (const AInputEvent *key_event)
int64_t 	AKeyEvent_getDownTime (const AInputEvent *key_event)
int64_t 	AKeyEvent_getEventTime (const AInputEvent *key_event)
*/
  Fl_Android_Application::log_i("Key event: action=%d keyCode=%d metaState=0x%x scanCode=%d",
                                AKeyEvent_getAction(event),
                                AKeyEvent_getKeyCode(event),
                                AKeyEvent_getMetaState(event),
                                AKeyEvent_getScanCode(event));

  auto keyAction = AKeyEvent_getAction(event);

  JavaVM *javaVM = Fl_Android_Application::get_activity()->vm;
  JNIEnv *jniEnv = Fl_Android_Application::get_activity()->env;

  JavaVMAttachArgs Args = { JNI_VERSION_1_6, "NativeThread", NULL };
  jint result = javaVM->AttachCurrentThread(&jniEnv, &Args);
  if (result == JNI_ERR) return 0;

  jclass class_key_event = jniEnv->FindClass("android/view/KeyEvent");

  jmethodID method_get_unicode_char = jniEnv->GetMethodID(class_key_event,
                                                          "getUnicodeChar",
                                                          "(I)I");
  jmethodID eventConstructor = jniEnv->GetMethodID(class_key_event, "<init>",
                                                   "(JJIIIIIIII)V");
  jobject eventObj = jniEnv->NewObject(class_key_event, eventConstructor,
                                       AKeyEvent_getDownTime(event),
                                       AKeyEvent_getEventTime(event),
                                       AKeyEvent_getAction(event),
                                       AKeyEvent_getKeyCode(event),
                                       AKeyEvent_getRepeatCount(event),
                                       AKeyEvent_getMetaState(event),
                                       AInputEvent_getDeviceId(event),
                                       AKeyEvent_getScanCode(event),
                                       AKeyEvent_getFlags(event),
                                       AInputEvent_getSource(event));
  int unicodeKey = jniEnv->CallIntMethod(eventObj, method_get_unicode_char,
                                         AKeyEvent_getMetaState(event));

  javaVM->DetachCurrentThread();

  static char buf[8];
  int len = fl_utf8encode(unicodeKey, buf);
  if (len >= 0 && len < 8)
    buf[len] = 0;
  else
    buf[0] = 0;
  Fl_Android_Application::log_i("Unicode: %d Text: %s", unicodeKey, buf);

  AInputQueue_finishEvent(queue, event, 0);

  Fl_Widget *w = Fl::focus();
  if (w) {
    Fl_Window *win = w->window();
    if (keyAction==AKEY_EVENT_ACTION_DOWN && unicodeKey>0) {
      Fl::e_text = buf;
      Fl::e_length = len;
      Fl::handle(FL_KEYBOARD, win);
    }
  }

/*
       case WM_KEYDOWN:
      case WM_SYSKEYDOWN:
      case WM_KEYUP:
      case WM_SYSKEYUP:
	// save the keysym until we figure out the characters:
	Fl::e_keysym = Fl::e_original_keysym = ms2fltk(wParam, lParam & (1 << 24));
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

 */


  return 0;
}


int Fl_Android_Screen_Driver::handle_mouse_event(AInputQueue *queue, AInputEvent *event)
{
  int ex = Fl::e_x_root = (int)(AMotionEvent_getX(event, 0) * 600 /
                                 ANativeWindow_getWidth(Fl_Android_Application::native_window()));
  int ey = Fl::e_y_root = (int)(AMotionEvent_getY(event, 0) * 800 /
                                 ANativeWindow_getHeight(Fl_Android_Application::native_window()));

  // FIXME: find the window in which the event happened
  Fl_Window *win = Fl::grab();
  if (!win) {
    win = Fl::first_window();
    if (win && !win->modal()) {
      while (win) {
        if (ex >= win->x() && ex < win->x() + win->w() && ey >= win->y() &&
            ey < win->y() + win->h())
          break;
        win = Fl::next_window(win);
      }
    }
  }
  if (!win) {
    AInputQueue_finishEvent(queue, event, 0);
    return 0;
  }

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
    AInputQueue_finishEvent(queue, event, 1);
    Fl::e_is_click = 1;
    if (win) Fl::handle(FL_PUSH, win); // do NOT send a push event into the "Desktop"
    Fl_Android_Application::log_i("Mouse push %d at %d, %d", Fl::event_button(), Fl::event_x(), Fl::event_y());
  } else if (AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_MOVE) {
    AInputQueue_finishEvent(queue, event, 1);
    Fl::handle(FL_DRAG, win);
  } else if (AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_UP) {
    AInputQueue_finishEvent(queue, event, 1);
    Fl::e_state = 0;
    Fl::handle(FL_RELEASE, win);
  } else {
    AInputQueue_finishEvent(queue, event, 0);
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
  int delay_millis = time_to_wait*1000;
  bool done = false;

//  while (!done) {
    int delay = Fl::damage() ? 0 : delay_millis;
    ident = ALooper_pollOnce(delay, nullptr, &events, nullptr);
    switch (ident) {
      case Fl_Android_Application::LOOPER_ID_MAIN:
        ret = handle_app_command();
        break;
      case Fl_Android_Application::LOOPER_ID_INPUT:
        ret = handle_input_event();
        break;
      case Fl_Android_Application::LOOPER_ID_TIMER:
        timer_do_callback(Fl_Android_Application::receive_timer_index());
        break;
      case ALOOPER_POLL_WAKE:
        Fl_Android_Application::log_e("Someone woke up ALooper_pollAll.");
        done = true;
        break;
      case ALOOPER_POLL_CALLBACK:
        Fl_Android_Application::log_e(
                "Someone added a callback to ALooper_pollAll.");
        done = true;
        break;
      case ALOOPER_POLL_TIMEOUT:
        // timer expired
        done = true;
        break;
      case ALOOPER_POLL_ERROR:
        Fl_Android_Application::log_e(
                "Something caused an ERROR in ALooper_pollAll.");
        done = true;
        break;
      default:
        Fl_Android_Application::log_e(
                "Unknown return value from ALooper_pollAll.");
        done = true;
        break;
    }
//  }
  return ret;
}

/**
 * Wait for a maximum of `time_to_wait` until something happens.
 * @param time_to_wait in seconds
 * @return We really do not know; check other platforms to see what is
 *        consistent here.
 * FIXME: return the remaining time to reach 'time_to_wait'
 */
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

  return 0.0;
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


void Fl_Android_Screen_Driver::request_keyboard()
{
  if (pKeyboardCount==0) {
    /*
    ANativeActivity_showSoftInput(Fl_Android_Application::get_activity(),
                                  ANATIVEACTIVITY_SHOW_SOFT_INPUT_IMPLICIT);
    */
//    void displayKeyboard(bool pShow)
    bool pShow = true;
    {
      // Attaches the current thread to the JVM.
      jint lResult;
      jint lFlags = 0;

      JavaVM* lJavaVM = Fl_Android_Application::get_activity()->vm;
      JNIEnv* lJNIEnv = Fl_Android_Application::get_activity()->env;

      JavaVMAttachArgs lJavaVMAttachArgs;
      lJavaVMAttachArgs.version = JNI_VERSION_1_6;
      lJavaVMAttachArgs.name = "NativeThread";
      lJavaVMAttachArgs.group = NULL;

      lResult=lJavaVM->AttachCurrentThread(&lJNIEnv, &lJavaVMAttachArgs);
      if (lResult == JNI_ERR) {
        return;
      }

      // Retrieves NativeActivity.
      jobject lNativeActivity = Fl_Android_Application::get_activity()->clazz;
      jclass ClassNativeActivity = lJNIEnv->GetObjectClass(lNativeActivity);

      // Retrieves Context.INPUT_METHOD_SERVICE.
      jclass ClassContext = lJNIEnv->FindClass("android/content/Context");
      jfieldID FieldINPUT_METHOD_SERVICE =
              lJNIEnv->GetStaticFieldID(ClassContext,
                                        "INPUT_METHOD_SERVICE", "Ljava/lang/String;");
      jobject INPUT_METHOD_SERVICE =
              lJNIEnv->GetStaticObjectField(ClassContext,
                                            FieldINPUT_METHOD_SERVICE);
//      jniCheck(INPUT_METHOD_SERVICE);

      // Runs getSystemService(Context.INPUT_METHOD_SERVICE).
      jclass ClassInputMethodManager = lJNIEnv->FindClass(
              "android/view/inputmethod/InputMethodManager");
      jmethodID MethodGetSystemService = lJNIEnv->GetMethodID(
              ClassNativeActivity, "getSystemService",
              "(Ljava/lang/String;)Ljava/lang/Object;");
      jobject lInputMethodManager = lJNIEnv->CallObjectMethod(
              lNativeActivity, MethodGetSystemService,
              INPUT_METHOD_SERVICE);

      // Runs getWindow().getDecorView().
      jmethodID MethodGetWindow = lJNIEnv->GetMethodID(
              ClassNativeActivity, "getWindow",
              "()Landroid/view/Window;");
      jobject lWindow = lJNIEnv->CallObjectMethod(lNativeActivity,
                                                  MethodGetWindow);
      jclass ClassWindow = lJNIEnv->FindClass(
              "android/view/Window");
      jmethodID MethodGetDecorView = lJNIEnv->GetMethodID(
              ClassWindow, "getDecorView", "()Landroid/view/View;");
      jobject lDecorView = lJNIEnv->CallObjectMethod(lWindow,
                                                     MethodGetDecorView);

      if (pShow) {
        // Runs lInputMethodManager.showSoftInput(...).
        jmethodID MethodShowSoftInput = lJNIEnv->GetMethodID(
                ClassInputMethodManager, "showSoftInput",
                "(Landroid/view/View;I)Z");
        jboolean lResult = lJNIEnv->CallBooleanMethod(
                lInputMethodManager, MethodShowSoftInput,
                lDecorView, lFlags);
      } else {
        // Runs lWindow.getViewToken()
        jclass ClassView = lJNIEnv->FindClass(
                "android/view/View");
        jmethodID MethodGetWindowToken = lJNIEnv->GetMethodID(
                ClassView, "getWindowToken", "()Landroid/os/IBinder;");
        jobject lBinder = lJNIEnv->CallObjectMethod(lDecorView,
                                                    MethodGetWindowToken);

        // lInputMethodManager.hideSoftInput(...).
        jmethodID MethodHideSoftInput = lJNIEnv->GetMethodID(
                ClassInputMethodManager, "hideSoftInputFromWindow",
                "(Landroid/os/IBinder;I)Z");
        jboolean lRes = lJNIEnv->CallBooleanMethod(
                lInputMethodManager, MethodHideSoftInput,
                lBinder, lFlags);
      }

      // Finished with the JVM.
      lJavaVM->DetachCurrentThread();
    }

  }
  pKeyboardCount++;
}


void Fl_Android_Screen_Driver::release_keyboard()
{
  pKeyboardCount--;
  if (pKeyboardCount==0) {
    ANativeActivity_hideSoftInput(Fl_Android_Application::get_activity(),
                                  ANATIVEACTIVITY_HIDE_SOFT_INPUT_NOT_ALWAYS);
  }
}



//
// End of "$Id$".
//
