//
// "$Id: Fl_PicoAndroid_Screen_Driver.cxx 11253 2016-03-01 00:54:21Z matt $"
//
// Definition of Android Screen interface based on Pico
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

// http://developer.android.com/ndk/reference/group___native_activity.html


#include "../../config_lib.h"
#include "Fl_PicoAndroid_Screen_Driver.H"

#include <android/window.h>

#include <FL/x.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Window_Driver.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Graphics_Driver.H>
#include <FL/fl_draw.h>


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))



void Fl_PicoAndroid_Screen_Driver::initDisplay()
{
  // initialize OpenGL ES and EGL

  /*
   * Here specify the attributes of the desired configuration.
   * Below, we select an EGLConfig with at least 8 bits per color
   * component compatible with on-screen windows
   */
  const EGLint attribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_NONE
  };
  EGLint w, h, dummy, format;
  EGLint numConfigs;
  EGLConfig config;
  EGLSurface surface;
  EGLContext context;

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  eglInitialize(display, 0, 0);

  /* Here, the application chooses the configuration it desires. In this
   * sample, we have a very simplified selection process, where we pick
   * the first EGLConfig that matches our criteria */
  eglChooseConfig(display, attribs, &config, 1, &numConfigs);

  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
   * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
   * As soon as we picked a EGLConfig, we can safely reconfigure the
   * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

  ANativeWindow_setBuffersGeometry(pApp->window, 0, 0, format);

  surface = eglCreateWindowSurface(display, config, pApp->window, NULL);
  context = eglCreateContext(display, config, NULL, NULL);

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
    LOGW("Unable to eglMakeCurrent");
    return;
  }

  eglQuerySurface(display, surface, EGL_WIDTH, &w);
  eglQuerySurface(display, surface, EGL_HEIGHT, &h);

  this->pDisplay = display;
  pContext = context;
  pSurface = surface;
  pWidth = w;
  pHeight = h;

  // Initialize GL state.
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  glEnable(GL_CULL_FACE);
  glShadeModel(GL_SMOOTH);
  glDisable(GL_DEPTH_TEST);

  glViewport(0, 100, w, h-200);

  // make adjustments for screen ratio
  float ratio = 3.0 * (float) w / h;
  glMatrixMode(GL_PROJECTION);        // set matrix to projection mode
  glLoadIdentity();                        // reset the matrix to its default state
  // glFrustumf(-ratio, ratio, -3, 3, 3, 30);  // apply the projection matrix
  glOrthof(0, w/3, h/3, 0, -30, 30);  // apply the projection matrix
  glLineWidth(3);
}


void Fl_PicoAndroid_Screen_Driver::termDisplay()
{
  if (pDisplay != EGL_NO_DISPLAY) {
    eglMakeCurrent(pDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (pContext != EGL_NO_CONTEXT) {
      eglDestroyContext(pDisplay, pContext);
    }
    if (pSurface != EGL_NO_SURFACE) {
      eglDestroySurface(pDisplay, pSurface);
    }
    eglTerminate(pDisplay);
  }
  pDisplay = EGL_NO_DISPLAY;
  pContext = EGL_NO_CONTEXT;
  pSurface = EGL_NO_SURFACE;
}


void Fl_PicoAndroid_Screen_Driver::drawFrame()
{
  if (pDisplay == NULL) {
    return;
  }
  eglSwapBuffers(pDisplay, pSurface);
//  LOGI("Swapping buffers");
}


void Fl_PicoAndroid_Screen_Driver::handleAppCmdCB(struct android_app* app, int32_t cmd)
{
  Fl_PicoAndroid_Screen_Driver *This = (Fl_PicoAndroid_Screen_Driver*)(app->userData);
  This->handleAppCmd(app, cmd);
}


void Fl_PicoAndroid_Screen_Driver::handleAppCmd(struct android_app* app, int32_t cmd)
{
  LOGI("CMD %d", cmd);
//  struct engine* engine = (struct engine*)app->userData;
  switch (cmd) {
    case APP_CMD_SAVE_STATE:
      // The system has asked us to save our current state.  Do so.
//      engine->app->savedState = malloc(sizeof(struct saved_state));
//      *((struct saved_state*)engine->app->savedState) = engine->state;
//      engine->app->savedStateSize = sizeof(struct saved_state);
      break;
    case APP_CMD_INIT_WINDOW:
      // The window is being shown, get it ready.
      if (pApp->window != NULL) {
        // the flag below allow for easy development and should be removed when
        // distributing a final app
        ANativeActivity_setWindowFlags(pApp->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);
        initDisplay();
        drawFrame();
      }
      break;
    case APP_CMD_TERM_WINDOW:
      // The window is being hidden or closed, clean it up.
      termDisplay();
      break;
    case APP_CMD_GAINED_FOCUS:
      // When our app gains focus, we start monitoring the accelerometer.
//      if (engine->accelerometerSensor != NULL) {
//        ASensorEventQueue_enableSensor(engine->sensorEventQueue,
//                                       engine->accelerometerSensor);
//        // We'd like to get 60 events per second (in us).
//        ASensorEventQueue_setEventRate(engine->sensorEventQueue,
//                                       engine->accelerometerSensor, (1000L/60)*1000);
//      }
      break;
    case APP_CMD_LOST_FOCUS:
      // When our app loses focus, we stop monitoring the accelerometer.
      // This is to avoid consuming battery while not being used.
//      if (engine->accelerometerSensor != NULL) {
//        ASensorEventQueue_disableSensor(engine->sensorEventQueue,
//                                        engine->accelerometerSensor);
//      }
//      // Also stop animating.
//      engine->animating = 0;
//      engine_draw_frame(engine);
      break;
  }
}


int32_t Fl_PicoAndroid_Screen_Driver::handleInputEventCB(struct android_app* app, AInputEvent* event)
{
  Fl_PicoAndroid_Screen_Driver *This = (Fl_PicoAndroid_Screen_Driver*)(app->userData);
  This->handleInputEvent(app, event);
}


int32_t Fl_PicoAndroid_Screen_Driver::handleInputEvent(struct android_app* app, AInputEvent* event)
{
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
//    fl_lock_function();
    int x = AMotionEvent_getX(event, 0);
    int y = AMotionEvent_getY(event, 0);
    int action = AKeyEvent_getAction(event);
    Fl_Window *window = Fl::first_window();
    switch (action) {
      case AMOTION_EVENT_ACTION_DOWN:
        Fl::e_is_click = 1;
        Fl::e_x = Fl::e_x_root = x/3;
        Fl::e_y = Fl::e_y_root = (y-100)/3;
        if (!window) break;
        Fl::e_keysym = FL_Button+FL_LEFT_MOUSE;
        Fl::e_state = FL_BUTTON1;
        Fl::handle(FL_PUSH, window);
        break;
      case AMOTION_EVENT_ACTION_MOVE:
        Fl::e_is_click = 1;
        Fl::e_x = Fl::e_x_root = x/3;
        Fl::e_y = Fl::e_y_root = (y-100)/3;
        if (!window) break;
        Fl::e_keysym = FL_Button+FL_LEFT_MOUSE;
        Fl::e_state = FL_BUTTON1;
        Fl::handle(FL_DRAG, window);
        break;
      case AMOTION_EVENT_ACTION_UP:
      case AMOTION_EVENT_ACTION_CANCEL:
        Fl::e_is_click = 1;
        Fl::e_x = Fl::e_x_root = x/3;
        Fl::e_y = Fl::e_y_root = (y-100)/3;
        if (!window) break;
        Fl::e_keysym = FL_Button+FL_LEFT_MOUSE;
        Fl::e_state = 0;
        Fl::handle(FL_RELEASE, window);
        break;
//      case AMOTION_EVENT_ACTION_HOVER_MOVE:
//        Fl::e_is_click = 1;
//        Fl::e_x = Fl::e_x_root = x/3;
//        Fl::e_y = (y-100)/3;
//        if (!window) break;
//        Fl::e_keysym = 0;
//        Fl::e_state = 0;
//        Fl::handle(FL_MOVE, window);
//        break;
    }
//    AMOTION_EVENT_ACTION_MASK
//    LOGI("Motion at %d, %d", x, y);
//    fl_unlock_function();
    Fl_X::first->w->redraw();
    return 1;
  }
  return 0;
}


extern int main(int argc, const char **argv);

void android_main(struct android_app* state)
{
//  LOGI("Android Main call");
  Fl_PicoAndroid_Screen_Driver *This = (Fl_PicoAndroid_Screen_Driver*)Fl::screen_driver();
  This->android_main(state);
  static const char *argv[1] = { "native-activity" };
  main(1, argv);
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void Fl_PicoAndroid_Screen_Driver::android_main(struct android_app* state)
{
  app_dummy();

  pApp = state;
  pApp->userData = this;
  pApp->onAppCmd = handleAppCmdCB;
  pApp->onInputEvent = handleInputEventCB;


#if 0
  struct engine engine;

  // Make sure glue isn't stripped.
  app_dummy();

  memset(&engine, 0, sizeof(engine));
  state->userData = &engine;
  state->onAppCmd = engine_handle_cmd;
  state->onInputEvent = engine_handle_input;
  engine.app = state;

  // Prepare to monitor accelerometer
  engine.sensorManager = ASensorManager_getInstance();
  engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
                                                               ASENSOR_TYPE_ACCELEROMETER);
  engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
                                                            state->looper, LOOPER_ID_USER, NULL, NULL);

  if (state->savedState != NULL) {
    // We are starting with a previous saved state; restore from it.
    engine.state = *(struct saved_state*)state->savedState;
  }

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

      // If a sensor has data, process it now.
      if (ident == LOOPER_ID_USER) {
        if (engine.accelerometerSensor != NULL) {
          ASensorEvent event;
          while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
                                             &event, 1) > 0) {
            LOGI("accelerometer: x=%f y=%f z=%f",
                 event.acceleration.x, event.acceleration.y,
                 event.acceleration.z);
          }
        }
      }

      // Check if we are exiting.
      if (state->destroyRequested != 0) {
        engine_term_display(&engine);
        return;
      }
    }

    if (engine.animating) {
      // Done with events; draw next animation frame.
      engine.state.angle += .01f;
      if (engine.state.angle > 1) {
        engine.state.angle = 0;
      }

      // Drawing is throttled to the screen update rate, so there
      // is no need to do timing here.
      engine_draw_frame(&engine);
    }
  }
#endif
}


Fl_Screen_Driver* Fl_Screen_Driver::newScreenDriver()
{
  return new Fl_PicoAndroid_Screen_Driver();
}


Fl_PicoAndroid_Screen_Driver::Fl_PicoAndroid_Screen_Driver()
{
  pDisplay = EGL_NO_DISPLAY;
  pContext = EGL_NO_CONTEXT;
  pSurface = EGL_NO_SURFACE;
}

Fl_PicoAndroid_Screen_Driver::~Fl_PicoAndroid_Screen_Driver()
{
}


double Fl_PicoAndroid_Screen_Driver::wait(double time_to_wait)
{
  Fl::flush();
    // Read all pending events.
    int ident;
    int events;
    struct android_poll_source* source;

    // If not animating, we will block forever waiting for events.
    // If animating, we loop until all events are read, then continue
    // to draw the next frame of animation.
    // int ALooper_pollAll(int timeoutMillis, int* outFd, int* outEvents, void** outData)
    if ((ident=ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {

      // Process this event.
      if (source != NULL) {
        source->process(pApp, source);
      }

      // If a sensor has data, process it now.
//      if (ident == LOOPER_ID_USER) {
//        if (engine.accelerometerSensor != NULL) {
//          ASensorEvent event;
//          while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
//                                             &event, 1) > 0) {
//            LOGI("accelerometer: x=%f y=%f z=%f",
//                 event.acceleration.x, event.acceleration.y,
//                 event.acceleration.z);
//          }
//        }
//      }

      // Check if we are exiting.
//      if (state->destroyRequested != 0) {
//        engine_term_display(&engine);
//        return;
//      }
    }

//    if (engine.animating) {
//      // Done with events; draw next animation frame.
//      engine.state.angle += .01f;
//      if (engine.state.angle > 1) {
//        engine.state.angle = 0;
//      }
//
//      // Drawing is throttled to the screen update rate, so there
//      // is no need to do timing here.
//      engine_draw_frame(&engine);
//    }
//  }
  return 0.0;
}




/*
 * The following code should not be here! 
 * All this must be refactored into the driver system!
 */

Fl_Fontdesc* fl_fonts = NULL;

/*

 The following symbols are not found if we naively compile the core modules and
 no specific platform implementations. This list is a hint at all the functions
 and methods that probably need to be refactored into the driver system.

 Undefined symbols for architecture x86_64:
 */

void fl_set_spot(int, int, int, int, int, int, Fl_Window*) { }
void fl_reset_spot() { }
const char *fl_filename_name(char const*) { return 0; }
void fl_clipboard_notify_change() { }

//Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver() { return 0; }
//Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver() { return 0; }
void Fl_Graphics_Driver::global_gc() { }
int Fl::dnd() { return 0; }
void Fl::copy(char const*, int, int, char const*) { }
void Fl::paste(Fl_Widget&, int, char const*) { }
void Fl::get_mouse(int&, int&) { }
void Fl::set_color(unsigned int, unsigned int) { }
int Fl_X::set_cursor(Fl_Cursor) { return 0; }
int Fl_X::set_cursor(Fl_RGB_Image const*, int, int) { return 0; }
void Fl_X::set_default_icons(Fl_RGB_Image const**, int) { }

void Fl_X::set_icons() { }
void Fl_Window::size_range_() { }
void Fl_Window::fullscreen_x() { }

void Fl_Window::make_current()
{
  fl_window = i->xid;
  current_ = this;
}

void Fl_Window::fullscreen_off_x(int, int, int, int) { }

Window fl_xid(const Fl_Window* w)
{
  Fl_X *temp = Fl_X::i(w);
  return temp ? temp->xid : 0;
}

void Fl_Window::show() {
  if (!shown()) {
    Fl_X::make(this);
  }
}

Fl_X* Fl_X::make(Fl_Window *w)
{
  return w->driver()->makeWindow();
}

void Fl_Window::label(char const*, char const*) { }
void Fl_Window::resize(int, int, int, int) { }
Fl_Window *Fl_Window::current_;
char fl_show_iconic;
Window fl_window;
//void Fl_Image_Surface::translate(int x, int y) { }
//void Fl_Image_Surface::untranslate() { }

void Fl::add_fd(int, int, void (*)(int, void*), void*)
{
}

void Fl::add_fd(int, void (*)(int, void*), void*)
{
}

void Fl::remove_fd(int)
{
}

// these pointers are set by the Fl::lock() function:
static void nothing() {}
void (*fl_lock_function)() = nothing;
void (*fl_unlock_function)() = nothing;

Fl_Font_Descriptor::~Fl_Font_Descriptor()
{
}


//
// End of "$Id: Fl_PicoAndroid_Screen_Driver.cxx 11253 2016-03-01 00:54:21Z matt $".
//

