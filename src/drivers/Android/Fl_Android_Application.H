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


/**
 * A class to make Java calls from C++ easier.
 */
class Fl_Android_Java
{
  JavaVM *pJavaVM = nullptr;
  JNIEnv *pJNIEnv = nullptr;
  jobject pNativeActivity;
  jclass pNativeActivityClass;
  bool pAttached = false;

public:
  Fl_Android_Java();
  ~Fl_Android_Java();
  bool is_attached() { return pAttached; }
  JavaVM *VM() { return pJavaVM; }
  JNIEnv *env() { return pJNIEnv; }
  jobject native_ativity() { return pNativeActivity; }
  jclass native_activity_class() { return pNativeActivityClass; }
};


/**
 * Static class that manages all interaction between the Android Native Activity
 * and the FLTK library. It also keeps often used data for global access.
 *
 * On launch, it creates a main thread and communication pipe to
 * the Activity. All FLTK code will run in that thread. Activity
 * events will be polled by the Screen driver using the provided
 * Android Looper, and will also be routed back to this class as needed.
 *
 * This code is based on the native activity interface
 * provided by <android/native_activity.h>. It is based on a set
 * of application-provided callbacks that will be called
 * by the Activity's main thread when certain events occur.
 *
 * 1/ The application must provide a function named "int main(argc, argv)" that
 *    will be called when the activity is created, in a new thread that is
 *    distinct from the activity's main thread.
 *
 * 2/ The application has access to a static "Fl_Android_Application" class
 *    that contains references to other important objects, e.g. the
 *    ANativeActivity object instance the application is running in.
 *
 * 3/ the "Fl_Android_Application" class holds an ALooper instance that already
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
 *    FLTK will add more items to the looper for timers and file and socket
 *    communication. (fl_add_timeout, Fl::add_fd(), ...
 */
class Fl_Android_Application
{
public:

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
            LOOPER_ID_INPUT,

    /**
     * Timer data ID of all timer events coming from the Unix timer_create()
     * and friends, used in fl_add_timeout() and colleagues.
     */
            LOOPER_ID_TIMER,

    /**
     * Start of user-defined ALooper identifiers.
     */
            LOOPER_ID_USER,
  };

  /**
   * @see android.H Fl_Android_Platform_Event
   */
  enum {
            APP_CMD_INPUT_CHANGED,
            APP_CMD_INIT_WINDOW,
            APP_CMD_TERM_WINDOW,
            APP_CMD_WINDOW_RESIZED,
            APP_CMD_WINDOW_REDRAW_NEEDED,
            APP_CMD_CONTENT_RECT_CHANGED,
            APP_CMD_GAINED_FOCUS,
            APP_CMD_LOST_FOCUS,
            APP_CMD_CONFIG_CHANGED,
            APP_CMD_LOW_MEMORY,
            APP_CMD_START,
            APP_CMD_RESUME,
            APP_CMD_SAVE_STATE,
            APP_CMD_PAUSE,
            APP_CMD_STOP,
            APP_CMD_DESTROY,
  };

public:
  // --- logging
  static void log_e(const char *text, ...);
  static void log_w(const char *text, ...);
  static void log_i(const char *text, ...);
  static void log_v(const char *text, ...);

  // --- application state stuff
  static int8_t read_cmd();
  static void pre_exec_cmd(int8_t cmd);
  static void post_exec_cmd(int8_t cmd);
  static int destroy_requested() { return pDestroyRequested; }
  static const char *get_internal_data_path() { return pActivity->internalDataPath; }
  static const char *get_external_data_path() { return pActivity->externalDataPath; }
  static AAssetManager *get_asset_manager() { return pActivity->assetManager; }
  static ANativeActivity *get_activity() { return pActivity; }

  // --- event handling
  static AInputQueue *input_event_queue() { return pInputQueue; }

  // --- screen stuff
  static bool copy_screen();
  static inline ANativeWindow *native_window() { return pNativeWindow; }
  static inline ANativeWindow_Buffer &graphics_buffer() { return pApplicationWindowBuffer; }

  // --- timer stuff
  static void send_timer_index(uint8_t ix);
  static uint8_t receive_timer_index();


protected:
  static void free_saved_state();
  static void print_cur_config();
  static void destroy();
  static void* thread_entry(void* param);

  // --- screen handling stuff
  static void allocate_screen();
  static bool lock_screen();
  static void unlock_and_post_screen();
  static bool screen_is_locked();

  // --- timer stuff
  static void create_timer_handler();
  static void destroy_timer_handler();

  static ANativeActivity *pActivity;
  static AConfiguration *pConfig;
  static void *pSavedState;
  static size_t pSavedStateSize;
  static ALooper *pAppLooper;
  static AInputQueue *pInputQueue;
  static ANativeWindow *pNativeWindow;
  static ANativeWindow_Buffer pNativeWindowBuffer;
  static ANativeWindow_Buffer pApplicationWindowBuffer;
  static int pActivityState;
  static int pDestroyRequested;

  // ---- no need to make these visible to the outside ----
  static pthread_mutex_t pMutex;
  static pthread_cond_t pCond;
  static int pMsgReadPipe;
  static int pMsgWritePipe;
  static pthread_t pThread;
  static int pRunning;
  static int pStateSaved;
  static int pDestroyed;
  static AInputQueue* pPendingInputQueue;
  static ANativeWindow* pPendingWindow;

  // --- timer variables
  static int pTimerReadPipe;
  static int pTimerWritePipe;

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
  static void close_activity();

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


#endif // FL_ANDROID_APPLICATION_H

//
// End of "$Id$".
//
