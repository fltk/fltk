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
  if (keyAction==AKEY_EVENT_ACTION_MULTIPLE) {
    if (AKeyEvent_getKeyCode(event)==AKEYCODE_UNKNOWN) {
      // characters are in getCharacters()
      // String class KeyEvent::getCharacters() [Java]
      // is there a way to get the true Java event somehow?
      // override dispatchKeyEvent(android.view.KeyEvent event)
    } else {
      // send keycode as many times as getRepeatCount() / AKeyEvent_getRepeatCount(event)
    }
  }

  JavaVM *javaVM = Fl_Android_Application::get_activity()->vm;
  JNIEnv *jniEnv = Fl_Android_Application::get_activity()->env;

  JavaVMAttachArgs Args = { JNI_VERSION_1_6, "NativeThread", NULL };
  jint result = javaVM->AttachCurrentThread(&jniEnv, &Args);
  if (result == JNI_ERR) return 0;

  jclass class_key_event = jniEnv->FindClass("android/view/KeyEvent");
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

  jmethodID method_get_unicode_char = jniEnv->GetMethodID(class_key_event,
                                                          "getUnicodeChar",
                                                          "(I)I");
  int unicodeKey = jniEnv->CallIntMethod(eventObj, method_get_unicode_char,
                                         AKeyEvent_getMetaState(event));

  jniEnv->DeleteLocalRef(class_key_event);
  jniEnv->DeleteLocalRef(eventObj);

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


// ---- timers -----------------------------------------------------------------

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


// ---- keyboard ---------------------------------------------------------------


void Fl_Android_Screen_Driver::request_keyboard()
{
  if (pKeyboardCount==0) {
    /*
    ANativeActivity_showSoftInput(Fl_Android_Application::get_activity(),
                                  ANATIVEACTIVITY_SHOW_SOFT_INPUT_IMPLICIT);
    */
//    void displayKeyboard(bool pShow)
//    InputMethodManager imm = ( InputMethodManager )getSystemService( Context.INPUT_METHOD_SERVICE );
//    imm.showSoftInput( this.getWindow().getDecorView(), InputMethodManager.SHOW_FORCED );

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
