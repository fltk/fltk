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


/**
 * @if AndroidDev
 * @defgroup AndroidDeveloper Android Developer Documentation
 * @{
 */


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


/**
 * @class Fl_Android_Screen_Driver
 *
 * Handle Android screen devices.
 *
 * @todo This class is in an early development stage
 */


static void nothing() {}
void (*fl_unlock_function)() = nothing;
void (*fl_lock_function)() = nothing;

static void timer_do_callback(int timerIndex);


/**
 * Creates a driver that manages all Android screen and display related calls.
 */
Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver()
{
  return new Fl_Android_Screen_Driver();
}


extern int fl_send_system_handlers(void *e);


/**
 * Create the screen driver.
 */
Fl_Android_Screen_Driver::Fl_Android_Screen_Driver() :
  super(),
  pContentChanged(false),
  pClearDesktop(false)
{
}


/**
 * Call the FLTK System handler with Android specific events.
 *
 * @return always 1, assuming the event was handled
 *
 * @see Fl_Android_Platform_Event
 */
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
//    Fl_Android_Application::log_i("Mouse push %x %d at %d, %d", win, Fl::event_button(), Fl::event_x(), Fl::event_y());
    if (win) Fl::handle(FL_PUSH, win); // do NOT send a push event into the "Desktop"
  } else if (AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_MOVE) {
    AInputQueue_finishEvent(queue, event, 1);
//    Fl_Android_Application::log_i("Mouse drag %x %d at %d, %d", win, Fl::event_button(), Fl::event_x(), Fl::event_y());
    if (win) Fl::handle(FL_DRAG, win);
  } else if (AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_UP) {
    AInputQueue_finishEvent(queue, event, 1);
    Fl::e_state = 0;
//    Fl_Android_Application::log_i("Mouse release %x %d at %d, %d", win, Fl::event_button(), Fl::event_x(), Fl::event_y());
    if (win) Fl::handle(FL_RELEASE, win);
  } else {
    AInputQueue_finishEvent(queue, event, 0);
  }
  return 1;
}


/**
 * Handle all events in the even queue.
 *
 * @todo what should this function return?
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

  int delay = Fl::damage() ? 0 : delay_millis;
  while (!done) {
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
        Fl_Android_Application::log_e("Someone woke up ALooper_pollOnce.");
        done = true;
        break;
      case ALOOPER_POLL_CALLBACK:
        Fl_Android_Application::log_e(
                "Someone added a callback to ALooper_pollOnce.");
        done = true;
        break;
      case ALOOPER_POLL_TIMEOUT:
        done = true; // timer expired, return to FLTK
        break;
      case ALOOPER_POLL_ERROR:
        Fl_Android_Application::log_e(
                "Something caused an ERROR in ALooper_pollOnce.");
        done = true; // return to the app to find the error
        break;
      default:
        Fl_Android_Application::log_e(
                "Unknown return value from ALooper_pollOnce.");
        done = true; // return to the app, just in case
        break;
    }
    // we need to repeat this as long as there are messages in the queue, or any
    // change in the graphical interface will trigger a redraw immediately. To
    // save time and energy, we want to collect graphics changes and execute
    // them as soon as no more events are pending.
    // Setting delay to zero on the second round makes sure that all events
    // are handled first, and the call returns only when no more
    // events are pending.
    delay = 0;
  }
  return ret;
}


/**
 * Wait for a maximum of `time_to_wait` until something happens.
 *
 * @param time_to_wait in seconds
 * @return We really do not know; check other platforms to see what is
 *        consistent here.
 *
 * @todo return the remaining time to reach 'time_to_wait'
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
    // FIXME: kludge to erase a window after it was hidden
    if (pClearDesktop && fl_graphics_driver) {
      ((Fl_Android_Graphics_Driver*)fl_graphics_driver)->make_current(nullptr);
      fl_rectf(0, 0, 600, 800, FL_BLACK);
      pClearDesktop = false;
      pContentChanged = true;
    }
    Fl::flush();
  } else {
    // if there is wait time, show the pending changes and then handle the events
    // FIXME: kludge to erase a window after it was hidden
    if (pClearDesktop && fl_graphics_driver) {
      ((Fl_Android_Graphics_Driver*)fl_graphics_driver)->make_current(nullptr);
      fl_rectf(0, 0, 600, 800, FL_BLACK);
      pClearDesktop = false;
      pContentChanged = true;
    }
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
 *
 * @see fl_flush()
 */
void Fl_Android_Screen_Driver::flush()
{
  Fl_Screen_Driver::flush();
  // FIXME: do this only if anything actually changed on screen (need to optimize)!
  if (pContentChanged) {
    if (Fl_Android_Application::copy_screen())
      pContentChanged = false;
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

  // if that didn't work either, we ran out of timers
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


/**
 * Play some system sound.
 *
 * This function plays some rather arbitrary system sounds.
 *
 * @param type
 *
 * @see Fl_Screen_Driver::beep(int)
 * @see fl_beep(int)
 */
void Fl_Android_Screen_Driver::beep(int type)
{
  int androidSoundID = 93;  // default to TONE_CDMA_ALERT_CALL_GUARD
  switch (type) {
    case FL_BEEP_DEFAULT:       androidSoundID = 92; break;
    case FL_BEEP_MESSAGE:       androidSoundID = 86; break;
    case FL_BEEP_ERROR:         androidSoundID = 87; break;
    case FL_BEEP_QUESTION:      androidSoundID = 91; break;
    case FL_BEEP_PASSWORD:      androidSoundID = 95; break;
    case FL_BEEP_NOTIFICATION:  androidSoundID = 93; break;
  }
  Fl_Android_Java java;
  if (java.is_attached()) {

    jclass class_tone_generator = java.env()->FindClass("android/media/ToneGenerator");

    jmethodID toneGeneratorConstructor = java.env()->GetMethodID(
            class_tone_generator, "<init>",
            "(II)V");

    jobject toneGeneratorObj = java.env()->NewObject(
            class_tone_generator, toneGeneratorConstructor,
            4,  // STREAM_ALARM
            100); // volume

    jmethodID method_start_tone = java.env()->GetMethodID(
            class_tone_generator,
            "startTone",
            "(II)Z");

    java.env()->CallBooleanMethod(
            toneGeneratorObj, method_start_tone,
            androidSoundID,
            1000);

    java.env()->DeleteLocalRef(class_tone_generator);
    java.env()->DeleteLocalRef(toneGeneratorObj);
  }

}


/**
 * Get the current mouse coordinates.
 *
 * This is used, among other things, to position the FLTK standard dialogs in
 * a way that makes it easy to click the most common button. For an Android
 * touch screen, this makes no sense at all, which is why we return the center
 * of the screen for now.
 *
 * @todo rethink the dialog positioning scheme for touch devices.
 *
 * @todo this method assumes a fixed screen resolution
 *
 * @param [out] x
 * @param [out] y
 * @return
 */
int Fl_Android_Screen_Driver::get_mouse(int &x, int &y)
{
  x = 600/2;
  y = 800/2;
  return 1;
}


void Fl_Android_Screen_Driver::grab(Fl_Window* win)
{
  if (win) {
    if (!Fl::grab_) {
      // TODO: will we need to fix any focus and/or direct the input stream to a window
    }
    Fl::grab_ = win;
  } else {
    if (Fl::grab_) {
      Fl::grab_ = 0;
    }
  }
}


/**
 * @}
 * @endif
 */


//
// End of "$Id$".
//
