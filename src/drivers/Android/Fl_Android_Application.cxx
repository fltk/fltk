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

#include "Fl_Android_Application.H"

#include <jni.h>

#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#include <android/log.h>

#define  LOG_TAG    "FLTK"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGV(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

// The ANativeActivity object instance that this app is running in.
ANativeActivity *Fl_Android_Application::pActivity = 0L;

// The current configuration the app is running in.
AConfiguration* Fl_Android_Application::pConfig = 0L;

// This is the last instance's saved state, as provided at creation time.
// It is NULL if there was no state.  You can use this as you need; the
// memory will remain around until you call android_app_exec_cmd() for
// APP_CMD_RESUME, at which point it will be freed and savedState set to NULL.
// These variables should only be changed when processing a APP_CMD_SAVE_STATE,
// at which point they will be initialized to NULL and you can malloc your
// state and place the information here.  In that case the memory will be
// freed for you later.
void* Fl_Android_Application::pSavedState = 0;
size_t Fl_Android_Application::pSavedStateSize = 0;

// The ALooper associated with the app's thread.
ALooper* Fl_Android_Application::pMsgPipeLooper = 0;

// The ALooper tht interrupts the main loop when FLTK requests a redraw.
ALooper* Fl_Android_Application::pRedrawLooper = 0;

// When non-NULL, this is the input queue from which the app will
// receive user input events.
AInputQueue* Fl_Android_Application::pInputQueue = 0;

// When non-NULL, this is the window surface that the app can draw in.
ANativeWindow* Fl_Android_Application::pNativeWindow = 0;

// Use this buffer for direct drawing access
ANativeWindow_Buffer Fl_Android_Application::pNativeWindowBuffer = { 0 };
ANativeWindow_Buffer Fl_Android_Application::pApplicationWindowBuffer = { 0 };

// Current state of the app's activity.  May be either APP_CMD_START,
// APP_CMD_RESUME, APP_CMD_PAUSE, or APP_CMD_STOP; see below.
int Fl_Android_Application::pActivityState = 0;

// This is non-zero when the application's NativeActivity is being
// destroyed and waiting for the app thread to complete.
int Fl_Android_Application::pDestroyRequested = 0;

// Fill this in with the function to process main app commands (APP_CMD_*)
void (*Fl_Android_Application::pOnAppCmd)(int32_t cmd) = 0;

// Fill this in with the function to process input events.  At this point
// the event has already been pre-dispatched, and it will be finished upon
// return.  Return 1 if you have handled the event, 0 for any default
// dispatching.
int32_t (*Fl_Android_Application::pOnInputEvent)(AInputEvent* event) = 0;

pthread_mutex_t Fl_Android_Application::pMutex = { 0 };
pthread_cond_t Fl_Android_Application::pCond = { 0 };
int Fl_Android_Application::pMsgReadPipe = 0;
int Fl_Android_Application::pMsgWritePipe = 0;
pthread_t Fl_Android_Application::pThread = 0;
struct android_poll_source Fl_Android_Application::pCmdPollSource = { 0 };
struct android_poll_source Fl_Android_Application::pInputPollSource = { 0 };
int Fl_Android_Application::pRunning = 0;
int Fl_Android_Application::pStateSaved = 0;
int Fl_Android_Application::pDestroyed = 0;
int Fl_Android_Application::pRedrawNeeded = 0;
AInputQueue *Fl_Android_Application::pPendingInputQueue = 0;
ANativeWindow *Fl_Android_Application::pPendingWindow = 0;
ARect Fl_Android_Application::pPendingContentRect = { 0 };



void Fl_Android_Application::log_e(const char *text, ...)
{
  va_list args;
  va_start (args, text);
  __android_log_vprint(ANDROID_LOG_ERROR, LOG_TAG, text, args);
  va_end (args);
}


void Fl_Android_Application::log_w(const char *text, ...)
{
  va_list args;
  va_start (args, text);
  __android_log_vprint(ANDROID_LOG_WARN, LOG_TAG, text, args);
  va_end (args);
}


void Fl_Android_Application::log_i(const char *text, ...)
{
  va_list args;
  va_start (args, text);
  __android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, text, args);
  va_end (args);
}


void Fl_Android_Application::log_v(const char *text, ...)
{
#ifdef _DEBUG
  va_list args;
  va_start (args, text);
  __android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, text, args);
  va_end (args);
#else
  text = 0;
#endif
}


void Fl_Android_Application::free_saved_state()
{
  pthread_mutex_lock(&pMutex);
  if (pSavedState != NULL) {
    free(pSavedState);
    pSavedState = NULL;
    pSavedStateSize = 0;
  }
  pthread_mutex_unlock(&pMutex);
}

/**
 * Call when ALooper_pollAll() returns LOOPER_ID_MAIN, reading the next
 * app command message.
 */
int8_t Fl_Android_Application::read_cmd()
{
  int8_t cmd;
  if (read(pMsgReadPipe, &cmd, sizeof(cmd)) == sizeof(cmd)) {
    switch (cmd) {
      case APP_CMD_SAVE_STATE:
        free_saved_state();
        break;
    }
    return cmd;
  } else {
    LOGE("No data on command pipe!");
  }
  return -1;
}


void Fl_Android_Application::print_cur_config()
{
  char lang[2], country[2];
  AConfiguration_getLanguage(pConfig, lang);
  AConfiguration_getCountry(pConfig, country);

  LOGV("Config: mcc=%d mnc=%d lang=%c%c cnt=%c%c orien=%d touch=%d dens=%d "
               "keys=%d nav=%d keysHid=%d navHid=%d sdk=%d size=%d long=%d "
               "modetype=%d modenight=%d",
       AConfiguration_getMcc(pConfig),
       AConfiguration_getMnc(pConfig),
       lang[0], lang[1], country[0], country[1],
       AConfiguration_getOrientation(pConfig),
       AConfiguration_getTouchscreen(pConfig),
       AConfiguration_getDensity(pConfig),
       AConfiguration_getKeyboard(pConfig),
       AConfiguration_getNavigation(pConfig),
       AConfiguration_getKeysHidden(pConfig),
       AConfiguration_getNavHidden(pConfig),
       AConfiguration_getSdkVersion(pConfig),
       AConfiguration_getScreenSize(pConfig),
       AConfiguration_getScreenLong(pConfig),
       AConfiguration_getUiModeType(pConfig),
       AConfiguration_getUiModeNight(pConfig));
}

/**
 * Call with the command returned by android_app_read_cmd() to do the
 * initial pre-processing of the given command.  You can perform your own
 * actions for the command after calling this function.
 */
void Fl_Android_Application::pre_exec_cmd(int8_t cmd)
{
  switch (cmd) {
    case APP_CMD_INPUT_CHANGED:
      LOGV("APP_CMD_INPUT_CHANGED\n");
      pthread_mutex_lock(&pMutex);
      if (pInputQueue != NULL) {
        AInputQueue_detachLooper(pInputQueue);
      }
      pInputQueue = pPendingInputQueue;
      if (pInputQueue != NULL) {
        LOGV("Attaching input queue to looper");
        AInputQueue_attachLooper(pInputQueue,
                                 pMsgPipeLooper, LOOPER_ID_INPUT, NULL,
                                 &pInputPollSource);
      }
      pthread_cond_broadcast(&pCond);
      pthread_mutex_unlock(&pMutex);
      break;

    case APP_CMD_INIT_WINDOW:
      LOGV("APP_CMD_INIT_WINDOW\n");
      pthread_mutex_lock(&pMutex);
      pNativeWindow = pPendingWindow;
      pthread_cond_broadcast(&pCond);
      pthread_mutex_unlock(&pMutex);
      break;

    case APP_CMD_TERM_WINDOW:
      LOGV("APP_CMD_TERM_WINDOW\n");
      pthread_cond_broadcast(&pCond);
      break;

    case APP_CMD_RESUME:
    case APP_CMD_START:
    case APP_CMD_PAUSE:
    case APP_CMD_STOP:
      LOGV("activityState=%d\n", cmd);
      pthread_mutex_lock(&pMutex);
      pActivityState = cmd;
      pthread_cond_broadcast(&pCond);
      pthread_mutex_unlock(&pMutex);
      break;

    case APP_CMD_CONFIG_CHANGED:
      LOGV("APP_CMD_CONFIG_CHANGED\n");
      AConfiguration_fromAssetManager(pConfig,
                                      pActivity->assetManager);
      print_cur_config();
      break;

    case APP_CMD_DESTROY:
      LOGV("APP_CMD_DESTROY\n");
      pDestroyRequested = 1;
      break;
  }
}

/**
 * Call with the command returned by read_cmd() to do the
 * final post-processing of the given command.  You must have done your own
 * actions for the command before calling this function.
 */
void Fl_Android_Application::post_exec_cmd(int8_t cmd)
{
  switch (cmd) {
    case APP_CMD_TERM_WINDOW:
      LOGV("APP_CMD_TERM_WINDOW\n");
      pthread_mutex_lock(&pMutex);
      pNativeWindow = NULL;
      pthread_cond_broadcast(&pCond);
      pthread_mutex_unlock(&pMutex);
      break;

    case APP_CMD_SAVE_STATE:
      LOGV("APP_CMD_SAVE_STATE\n");
      pthread_mutex_lock(&pMutex);
      pStateSaved = 1;
      pthread_cond_broadcast(&pCond);
      pthread_mutex_unlock(&pMutex);
      break;

    case APP_CMD_RESUME:
      free_saved_state();
      break;
  }
}


void Fl_Android_Application::destroy()
{
  log_v("android_app_destroy!");
  free_saved_state();
  pthread_mutex_lock(&pMutex);
  if (pInputQueue != NULL) {
    AInputQueue_detachLooper(pInputQueue);
  }
  AConfiguration_delete(pConfig);
  pDestroyed = 1;
  pthread_cond_broadcast(&pCond);
  pthread_mutex_unlock(&pMutex);
  // Can't touch android_app object after this.
}


void Fl_Android_Application::process_input(struct android_poll_source* source)
{
  AInputEvent* event = NULL;
  while (AInputQueue_getEvent(pInputQueue, &event) >= 0) {
    //LOGV("New input event: type=%d\n", AInputEvent_getType(event));
    if (AInputQueue_preDispatchEvent(pInputQueue, event)) {
      continue;
    }
    int32_t handled = 0;
    if (pOnInputEvent != NULL) handled = pOnInputEvent(event);
    AInputQueue_finishEvent(pInputQueue, event, handled);
  }
}


void Fl_Android_Application::process_cmd(struct android_poll_source* source)
{
  int8_t cmd = read_cmd();
  pre_exec_cmd(cmd);
  if (pOnAppCmd != NULL) pOnAppCmd(cmd);
  post_exec_cmd(cmd);
}


void *Fl_Android_Application::thread_entry(void* param)
{
  pConfig = AConfiguration_new();
  AConfiguration_fromAssetManager(pConfig, pActivity->assetManager);

  print_cur_config();

  pCmdPollSource.id = LOOPER_ID_MAIN;
  pCmdPollSource.process = process_cmd;
  pInputPollSource.id = LOOPER_ID_INPUT;
  pInputPollSource.process = process_input;

  ALooper *looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
  ALooper_addFd(looper, pMsgReadPipe, LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL,
                &pCmdPollSource);
  pMsgPipeLooper = looper;

  pthread_mutex_lock(&pMutex);
  pRunning = 1;
  pthread_cond_broadcast(&pCond);
  pthread_mutex_unlock(&pMutex);

  char *argv[] = { strdup(pActivity->obbPath), 0 };
  main(1, argv);

  destroy();
  return NULL;
}

/**
 * Allocate memory for our internal screen buffer.
 *
 * FIXME: everything is currently hardcoded to an 600x800 resolution
 * TODO: react to screen changes
 */
void Fl_Android_Application::allocate_screen() {
  pApplicationWindowBuffer.bits = calloc(600*800, 2); // one uint16_t per pixel
  pApplicationWindowBuffer.width = 600;
  pApplicationWindowBuffer.height = 800;
  pApplicationWindowBuffer.stride = 600;
  pApplicationWindowBuffer.format = WINDOW_FORMAT_RGB_565;
}


#include <FL/fl_draw.H>

bool Fl_Android_Application::copy_screen()
{
  bool ret = false;
  if (lock_screen()) {

    static int i = 0;
    fl_color( (i&1) ? FL_RED : FL_GREEN);
    fl_rectf(i*10, 600+i*10, 50, 50);
    i++;
    if (i>10) i = 0;

    // TODO: there are endless possibilities to optimize the following code
    // We can identify previously written buffers and copy only those pixels
    // that actually changed.
    const uint16_t *src = (uint16_t*)pApplicationWindowBuffer.bits;
    int srcStride = pApplicationWindowBuffer.stride;
    int ww = pApplicationWindowBuffer.width;
    int hh = pApplicationWindowBuffer.height;

    uint16_t *dst = (uint16_t*)pNativeWindowBuffer.bits;
    int dstStride = pNativeWindowBuffer.stride;
    if (pNativeWindowBuffer.width<ww) ww = pNativeWindowBuffer.width;
    if (pNativeWindowBuffer.height<ww) ww = pNativeWindowBuffer.height;

    for (int row=hh; row>0; --row) {
      memcpy(dst, src, size_t(ww * 2));
      src += srcStride;
      dst += dstStride;
    }
    unlock_and_post_screen();
    ret = true;
  } else {
    Fl::damage(FL_DAMAGE_EXPOSE);
  }
  return ret;
}

/**
 * Take ownership of screen memory for gaining write access.
 *
 * If the screen is already locked, it will not be locked again
 * and a value of true will be returned.
 *
 * @return true if we gaines access, false if no access was granted and screen memory must not be writte to
 */
bool Fl_Android_Application::lock_screen()
{
  if (screen_is_locked())
    return true;

  // TODO: or should we wait until the window is mapped?
  // TODO: see also Fl_Window_Driver::wait_for_expose_value
  if (!pNativeWindow) {
    log_w("Unable to lock window buffer: no native window found.");
    return false;
  }

  if (ANativeWindow_lock(pNativeWindow, &pNativeWindowBuffer, 0L) < 0) {
    log_w("Unable to lock window buffer: Android won't lock.");
    return false;
  }
  return true;
}

/**
 * Release screen memory ownership and give it back to the system.
 *
 * The memory content will be copied to the physical screen next.
 * If the screen is not locked, this call will have no effect.
 */
void Fl_Android_Application::unlock_and_post_screen()
{
  if (!screen_is_locked())
    return;

  ANativeWindow_unlockAndPost(pNativeWindow);
  pNativeWindowBuffer.bits = 0L; // avoid any misunderstandings...
}

/**
 * Is the screen currently locked?
 * @return true if it is locked and the app has write access.
 */
bool Fl_Android_Application::screen_is_locked()
{
  return (pNativeWindowBuffer.bits!=0L);
}


// --------------------------------------------------------------------
// Native activity interaction (called from main thread)
// --------------------------------------------------------------------


void Fl_Android_Activity::write_cmd(int8_t cmd)
{
  if (write(pMsgWritePipe, &cmd, sizeof(cmd)) != sizeof(cmd)) {
    LOGE("Failure writing android_app cmd: %s\n", strerror(errno));
  }
}


void Fl_Android_Activity::set_input(AInputQueue* inputQueue)
{
  pthread_mutex_lock(&pMutex);
  pPendingInputQueue = inputQueue;
  write_cmd(APP_CMD_INPUT_CHANGED);
  while (pInputQueue != pPendingInputQueue) {
    pthread_cond_wait(&pCond, &pMutex);
  }
  pthread_mutex_unlock(&pMutex);
}


void Fl_Android_Activity::set_window(ANativeWindow* window)
{
  pthread_mutex_lock(&pMutex);
  if (pPendingWindow != NULL) {
    write_cmd(APP_CMD_TERM_WINDOW);
  }
  pPendingWindow = window;
  if (window != NULL) {
    write_cmd(APP_CMD_INIT_WINDOW);
  }
  while (pNativeWindow != pPendingWindow) {
    pthread_cond_wait(&pCond, &pMutex);
  }
  pthread_mutex_unlock(&pMutex);
}


void Fl_Android_Activity::set_activity_state(int8_t cmd)
{
  pthread_mutex_lock(&pMutex);
  write_cmd(cmd);
  while (pActivityState != cmd) {
    pthread_cond_wait(&pCond, &pMutex);
  }
  pthread_mutex_unlock(&pMutex);
}


void Fl_Android_Activity::free()
{
  pthread_mutex_lock(&pMutex);
  write_cmd(APP_CMD_DESTROY);
  while (!pDestroyed) {
    pthread_cond_wait(&pCond, &pMutex);
  }
  pthread_mutex_unlock(&pMutex);

  close(pMsgReadPipe);
  close(pMsgWritePipe);
  pthread_cond_destroy(&pCond);
  pthread_mutex_destroy(&pMutex);
}


// ---- Android Native Activity callbacks ----

/**
 * The rectangle in the window in which content should be placed has changed.
 */
void Fl_Android_Activity::onContentRectChanged(ANativeActivity *activity, const ARect *rect)
{
  // TODO: implement me
}

/**
 * The drawing window for this native activity needs to be redrawn. To avoid transient artifacts during screen changes (such resizing after rotation), applications should not return from this function until they have finished drawing their window in its current state.
 */
void Fl_Android_Activity::onNativeWindowRedrawNeeded(ANativeActivity *activity, ANativeWindow *window)
{
  // TODO: implement me
}

/**
 * The drawing window for this native activity has been resized. You should retrieve the new size from the window and ensure that your rendering in it now matches.
 */
void Fl_Android_Activity::onNativeWindowResized(ANativeActivity *activity, ANativeWindow *window)
{
  // TODO: implement me
}

/**
 * NativeActivity is being destroyed. See Java documentation for Activity.onDestroy() for more information.
 */
void Fl_Android_Activity::onDestroy(ANativeActivity* activity)
{
  log_v("Destroy: %p\n", activity);
  // FIXME: use the correct free()
  free();
}

/**
 * NativeActivity has started. See Java documentation for Activity.onStart() for more information.
 */
void Fl_Android_Activity::onStart(ANativeActivity* activity)
{
  log_v("Start: %p\n", activity);
  set_activity_state(APP_CMD_START);
}

/**
 * NativeActivity has resumed. See Java documentation for Activity.onResume() for more information.
 */
void Fl_Android_Activity::onResume(ANativeActivity* activity)
{
  log_v("Resume: %p\n", activity);
  set_activity_state(APP_CMD_RESUME);
}

/**
 * Framework is asking NativeActivity to save its current instance state. See Java documentation for Activity.onSaveInstanceState() for more information. The returned pointer needs to be created with malloc(); the framework will call free() on it for you. You also must fill in outSize with the number of bytes in the allocation. Note that the saved state will be persisted, so it can not contain any active entities (pointers to memory, file descriptors, etc).
 */
void *Fl_Android_Activity::onSaveInstanceState(ANativeActivity* activity, size_t* outLen)
{
  struct android_app* android_app = (struct android_app*)activity->instance;
  void* savedState = NULL;

  log_v("SaveInstanceState: %p\n", activity);
  pthread_mutex_lock(&pMutex);
  pStateSaved = 0;
  write_cmd(APP_CMD_SAVE_STATE);
  while (!pStateSaved) {
    pthread_cond_wait(&pCond, &pMutex);
  }

  if (pSavedState != NULL) {
    savedState = pSavedState;
    *outLen = pSavedStateSize;
    pSavedState = NULL;
    pSavedStateSize = 0;
  }

  pthread_mutex_unlock(&pMutex);

  return savedState;
}

/**
 * NativeActivity has paused. See Java documentation for Activity.onPause() for more information.
 */
void Fl_Android_Activity::onPause(ANativeActivity* activity)
{
  log_v("Pause: %p\n", activity);
  set_activity_state(APP_CMD_PAUSE);
}

/**
 * NativeActivity has stopped. See Java documentation for Activity.onStop() for more information.
 */
void Fl_Android_Activity::onStop(ANativeActivity* activity)
{
  log_v("Stop: %p\n", activity);
  set_activity_state(APP_CMD_STOP);
}

/**
 * The current device AConfiguration has changed. The new configuration can be retrieved from assetManager.
 */
void Fl_Android_Activity::onConfigurationChanged(ANativeActivity* activity)
{
  struct android_app* android_app = (struct android_app*)activity->instance;
  log_v("ConfigurationChanged: %p\n", activity);
  write_cmd(APP_CMD_CONFIG_CHANGED);
}

/**
 * The system is running low on memory. Use this callback to release resources you do not need, to help the system avoid killing more important processes.
 */
void Fl_Android_Activity::onLowMemory(ANativeActivity* activity)
{
  struct android_app* android_app = (struct android_app*)activity->instance;
  log_v("LowMemory: %p\n", activity);
  write_cmd(APP_CMD_LOW_MEMORY);
}

/**
 * Focus has changed in this NativeActivity's window. This is often used, for example, to pause a game when it loses input focus.
 */
void Fl_Android_Activity::onWindowFocusChanged(ANativeActivity* activity, int focused)
{
  log_v("WindowFocusChanged: %p -- %d\n", activity, focused);
  write_cmd(focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
}

/**
 * The drawing window for this native activity has been created. You can use the given native window object to start drawing.
 */
void Fl_Android_Activity::onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window)
{
  log_v("NativeWindowCreated: %p -- %p\n", activity, window);
  set_window(window);
}

/**
 * The drawing window for this native activity is going to be destroyed. You MUST ensure that you do not touch the window object after returning from this function: in the common case of drawing to the window from another thread, that means the implementation of this callback must properly synchronize with the other thread to stop its drawing before returning from here.
 */
void Fl_Android_Activity::onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window)
{
  log_v("NativeWindowDestroyed: %p -- %p\n", activity, window);
  set_window(NULL);
}

/**
 * The input queue for this native activity's window has been created. You can use the given input queue to start retrieving input events.
 */
void Fl_Android_Activity::onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue)
{
  log_v("InputQueueCreated: %p -- %p\n", activity, queue);
  set_input(queue);
}

/**
 * The input queue for this native activity's window is being destroyed. You should no longer try to reference this object upon returning from this function.
 */
void Fl_Android_Activity::onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue)
{
  log_v("InputQueueDestroyed: %p -- %p\n", activity, queue);
  set_input(NULL);
}

/**
 * Create a thread that will run our FLTK code and the required communications and locks.
 * @param activity the interface to the Java end of Android
 * @param savedState if this app is relaunched, this is a memory block with the state of the app when it was interrupted
 * @param savedStateSize size of that block
 */
void Fl_Android_Activity::create(ANativeActivity* activity, void* savedState,
                                 size_t savedStateSize)
{
  static const char *FLTK = "FLTK";
  activity->instance = (void*)FLTK;

  set_activity(activity);
  set_callbacks();

  allocate_screen(); // TODO: we may need to change this to when the actual screen is allocated

  pthread_mutex_init(&pMutex, NULL);
  pthread_cond_init(&pCond, NULL);

  if (savedState != NULL) {
    pSavedState = malloc(savedStateSize);
    pSavedStateSize = savedStateSize;
    memcpy(pSavedState, savedState, savedStateSize);
  }

  int msgpipe[2];
  if (pipe(msgpipe)) {
    LOGE("could not create pipe: %s", strerror(errno));
    return;
  }
  pMsgReadPipe = msgpipe[0];
  pMsgWritePipe = msgpipe[1];

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&pThread, &attr, thread_entry, 0L);

  // Wait for thread to start.
  pthread_mutex_lock(&pMutex);
  while (!pRunning) {
    pthread_cond_wait(&pCond, &pMutex);
  }
  pthread_mutex_unlock(&pMutex);
}

/**
 * Set all callbacks from the Native Activity.
 */
void Fl_Android_Activity::set_callbacks()
{
  ANativeActivityCallbacks *cb = pActivity->callbacks;
  cb->onContentRectChanged = onContentRectChanged;
  cb->onNativeWindowRedrawNeeded = onNativeWindowRedrawNeeded;
  cb->onNativeWindowResized = onNativeWindowResized;
  cb->onDestroy = onDestroy;
  cb->onStart = onStart;
  cb->onResume = onResume;
  cb->onSaveInstanceState = onSaveInstanceState;
  cb->onPause = onPause;
  cb->onStop = onStop;
  cb->onConfigurationChanged = onConfigurationChanged;
  cb->onLowMemory = onLowMemory;
  cb->onWindowFocusChanged = onWindowFocusChanged;
  cb->onNativeWindowCreated = onNativeWindowCreated;
  cb->onNativeWindowDestroyed = onNativeWindowDestroyed;
  cb->onInputQueueCreated = onInputQueueCreated;
  cb->onInputQueueDestroyed = onInputQueueDestroyed;
}

/**
 * This is the main entry point from the Android JavaVM into the native world.
 */
JNIEXPORT void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize)
{
  // TODO: can we return an error message her is creation of the app failed?
  Fl_Android_Activity::create(activity, savedState, savedStateSize);
}

//
// End of "$Id$".
//
