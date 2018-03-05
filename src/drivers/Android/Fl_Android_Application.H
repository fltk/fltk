//
// "$Id$"
//
// Android Native Application interface
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2018 by Bill Spitzak and others.
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
 \file Fl_Android_Application.H
 \brief Definition of Android Native Application interface
 */

#ifndef FL_ANDROID_APPLICATION_H
#define FL_ANDROID_APPLICATION_H

#include <FL/Fl.H>

extern void (*fl_unlock_function)();
extern void (*fl_lock_function)();


#include <poll.h>
#include <pthread.h>
#include <sched.h>

#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Static class that keeps often used data for global access.
 *
 * This class holds global variables that are needed by the Android
 * drivers.
 *
 * It also contains the interface to the Android Native Activity.
 * On launch, it creates a main thread and communication pipe to
 * the Activity. All FLTK code will run in that thread. Activity
 * events can be polled from the Screen driver using the provided
 * Android Looper, but must also be forwarded to this class.
 *
 * All other events are handled in Fl_Android_Screen_Driver
 *
 * This code is based on the native activity interface
 * provided by <android/native_activity.h>. It is based on a set
 * of application-provided callbacks that will be called
 * by the Activity's main thread when certain events occur.
 *
 * This means that each one of this callbacks _should_ _not_ block, or they
 * risk having the system force-close the application. This programming
 * model is direct, lightweight, but constraining.
 *
 * The 'Fl_Android_Application' interface is used to provide a different
 * execution model where the application can implement its own main event
 * loop in a different thread instead. Here's how it works:
 *
 * 1/ The application must provide a function named "int main(argc, argv)" that
 *    will be called when the activity is created, in a new thread that is
 *    distinct from the activity's main thread.
 *
 * 2/ android_main() receives a pointer to a valid "android_app" structure
 *    that contains references to other important objects, e.g. the
 *    ANativeActivity obejct instance the application is running in.
 *
 * 3/ the "android_app" object holds an ALooper instance that already
 *    listens to two important things:
 *
 *      - activity lifecycle events (e.g. "pause", "resume"). See APP_CMD_XXX
 *        declarations below.
 *
 *      - input events coming from the AInputQueue attached to the activity.
 *
 *    Each of these correspond to an ALooper identifier returned by
 *    ALooper_pollOnce with values of LOOPER_ID_MAIN and LOOPER_ID_INPUT,
 *    respectively.
 *
 *    Your application can use the same ALooper to listen to additional
 *    file-descriptors.  They can either be callback based, or with return
 *    identifiers starting with LOOPER_ID_USER.
 *
 * 4/ Whenever you receive a LOOPER_ID_MAIN or LOOPER_ID_INPUT event,
 *    the returned data will point to an android_poll_source structure.  You
 *    can call the process() function on it, and fill in android_app->onAppCmd
 *    and android_app->onInputEvent to be called for your own processing
 *    of the event.
 *
 *    Alternatively, you can call the low-level functions to read and process
 *    the data directly...  look at the process_cmd() and process_input()
 *    implementations in the glue to see how to do this.
 *
 * See the sample named "native-activity" that comes with the NDK with a
 * full usage example.  Also look at the JavaDoc of NativeActivity.

 */
class Fl_Android_Application
{
public:
  static void log_e(const char *text, ...);
  static void log_w(const char *text, ...);
  static void log_i(const char *text, ...);
  static void log_v(const char *text, ...);

  static int8_t read_cmd();
  static void pre_exec_cmd(int8_t cmd);
  static void post_exec_cmd(int8_t cmd);

  static inline ANativeWindow *native_window() { return pNativeWindow; }
  static inline ANativeWindow_Buffer &graphics_buffer() { return pApplicationWindowBuffer; }
  static int destroy_requested() { return pDestroyRequested; }
  static void set_on_app_cmd(void (*cmd)(int32_t cmd)) { pOnAppCmd = cmd; }
  static void set_on_input_event(int32_t (*cmd)(AInputEvent* event)) { pOnInputEvent = cmd; }

  static void copy_screen();

protected:
  static void free_saved_state();
  static void print_cur_config();
  static void destroy();
  static void process_input(struct android_poll_source* source);
  static void process_cmd(struct android_poll_source* source);
  static void* thread_entry(void* param);

  static void allocate_screen();
  static bool lock_screen();
  static void unlock_and_post_screen();
  static bool screen_is_locked();

  static ANativeActivity *pActivity;
  static AConfiguration *pConfig;
  static void *pSavedState;
  static size_t pSavedStateSize;
  static ALooper *pLooper;
  static AInputQueue *pInputQueue;
  static ANativeWindow *pNativeWindow;
  static ANativeWindow_Buffer pNativeWindowBuffer;
  static ANativeWindow_Buffer pApplicationWindowBuffer;
  static int pActivityState;
  static int pDestroyRequested;
  static void (*pOnAppCmd)(int32_t cmd);
  static int32_t (*pOnInputEvent)(AInputEvent* event);


  // ---- no need to make these visible to the outside ----
  static pthread_mutex_t pMutex;
  static pthread_cond_t pCond;
  static int pMsgRead;
  static int pMsgWrite;
  static pthread_t pThread;
  static struct android_poll_source pCmdPollSource;
  static struct android_poll_source pInputPollSource;
  static int pRunning;
  static int pStateSaved;
  static int pDestroyed;
  static int pRedrawNeeded;
  static AInputQueue* pPendingInputQueue;
  static ANativeWindow* pPendingWindow;
  static ARect pPendingContentRect;
};


class Fl_Android_Activity : public Fl_Android_Application
{
public:
  static void create(ANativeActivity* activity, void* savedState, size_t savedStateSize);

private:
  static void set_activity(ANativeActivity *a) { pActivity = a; }
  static void set_callbacks();

  // ---- Android Native Activity interface
  static void write_cmd(int8_t cmd);
  static void set_input(AInputQueue* inputQueue);
  static void set_window(ANativeWindow* window);
  static void set_activity_state(int8_t cmd);
  static void free();

  // ---- Android Native Activity callbacks ----
  static void onContentRectChanged(ANativeActivity *activity, const ARect *rect);
  static void onNativeWindowRedrawNeeded(ANativeActivity *activity, ANativeWindow *window);
  static void onNativeWindowResized(ANativeActivity *activity, ANativeWindow *window);
  static void onDestroy(ANativeActivity* activity);
  static void onStart(ANativeActivity* activity);
  static void onResume(ANativeActivity* activity);
  static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen);
  static void onPause(ANativeActivity* activity);
  static void onStop(ANativeActivity* activity);
  static void onConfigurationChanged(ANativeActivity* activity);
  static void onLowMemory(ANativeActivity* activity);
  static void onWindowFocusChanged(ANativeActivity* activity, int focused);
  static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window);
  static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window);
  static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue);
  static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue);
};

/**
 * Data associated with an ALooper fd that will be returned as the "outData"
 * when that source has data ready.
 */
struct android_poll_source {
  // The identifier of this source.  May be LOOPER_ID_MAIN or
  // LOOPER_ID_INPUT.
  int32_t id;

  // Function to call to perform the standard processing of data from
  // this source.
  void (*process)(struct android_poll_source* source);
};

enum {
  /**
   * Looper data ID of commands coming from the app's main thread, which
   * is returned as an identifier from ALooper_pollOnce().  The data for this
   * identifier is a pointer to an android_poll_source structure.
   * These can be retrieved and processed with android_app_read_cmd()
   * and android_app_exec_cmd().
   */
          LOOPER_ID_MAIN = 1,

  /**
   * Looper data ID of events coming from the AInputQueue of the
   * application's window, which is returned as an identifier from
   * ALooper_pollOnce().  The data for this identifier is a pointer to an
   * android_poll_source structure.  These can be read via the inputQueue
   * object of android_app.
   */
          LOOPER_ID_INPUT = 2,

  /**
   * Start of user-defined ALooper identifiers.
   */
          LOOPER_ID_USER = 3,
};

enum {
  /**
   * Command from main thread: the AInputQueue has changed.  Upon processing
   * this command, android_app->inputQueue will be updated to the new queue
   * (or NULL).
   */
          APP_CMD_INPUT_CHANGED,

  /**
   * Command from main thread: a new ANativeWindow is ready for use.  Upon
   * receiving this command, android_app->window will contain the new window
   * surface.
   */
          APP_CMD_INIT_WINDOW,

  /**
   * Command from main thread: the existing ANativeWindow needs to be
   * terminated.  Upon receiving this command, android_app->window still
   * contains the existing window; after calling android_app_exec_cmd
   * it will be set to NULL.
   */
          APP_CMD_TERM_WINDOW,

  /**
   * Command from main thread: the current ANativeWindow has been resized.
   * Please redraw with its new size.
   */
          APP_CMD_WINDOW_RESIZED,

  /**
   * Command from main thread: the system needs that the current ANativeWindow
   * be redrawn.  You should redraw the window before handing this to
   * android_app_exec_cmd() in order to avoid transient drawing glitches.
   */
          APP_CMD_WINDOW_REDRAW_NEEDED,

  /**
   * Command from main thread: the content area of the window has changed,
   * such as from the soft input window being shown or hidden.  You can
   * find the new content rect in android_app::contentRect.
   */
          APP_CMD_CONTENT_RECT_CHANGED,

  /**
   * Command from main thread: the app's activity window has gained
   * input focus.
   */
          APP_CMD_GAINED_FOCUS,

  /**
   * Command from main thread: the app's activity window has lost
   * input focus.
   */
          APP_CMD_LOST_FOCUS,

  /**
   * Command from main thread: the current device configuration has changed.
   */
          APP_CMD_CONFIG_CHANGED,

  /**
   * Command from main thread: the system is running low on memory.
   * Try to reduce your memory use.
   */
          APP_CMD_LOW_MEMORY,

  /**
   * Command from main thread: the app's activity has been started.
   */
          APP_CMD_START,

  /**
   * Command from main thread: the app's activity has been resumed.
   */
          APP_CMD_RESUME,

  /**
   * Command from main thread: the app should generate a new saved state
   * for itself, to restore from later if needed.  If you have saved state,
   * allocate it with malloc and place it in android_app.savedState with
   * the size in android_app.savedStateSize.  The will be freed for you
   * later.
   */
          APP_CMD_SAVE_STATE,

  /**
   * Command from main thread: the app's activity has been paused.
   */
          APP_CMD_PAUSE,

  /**
   * Command from main thread: the app's activity has been stopped.
   */
          APP_CMD_STOP,

  /**
   * Command from main thread: the app's activity is being destroyed,
   * and waiting for the app thread to clean up and exit before proceeding.
   */
          APP_CMD_DESTROY,
};

/**
 * This is the function that application code must implement, representing
 * the main entry to the app.
 */
extern int main(int argc, char **argv);

#ifdef __cplusplus
}
#endif



#endif // FL_ANDROID_APPLICATION_H

//
// End of "$Id$".
//
