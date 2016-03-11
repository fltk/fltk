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


#include "../../config_lib.h"
#include "Fl_PicoAndroid_Screen_Driver.H"

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <FL/x.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Window_Driver.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Graphics_Driver.H>


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))


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
//      if (engine->app->window != NULL) {
//        engine_init_display(engine);
//        engine_draw_frame(engine);
//      }
      break;
    case APP_CMD_TERM_WINDOW:
      // The window is being hidden or closed, clean it up.
//      engine_term_display(engine);
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
    int x = AMotionEvent_getX(event, 0);
    int y = AMotionEvent_getY(event, 0);
    LOGI("Motion at %d, %d", x, y);
    return 1;
  }
  return 0;
}


extern int main(int argc, const char **argv);

void android_main(struct android_app* state)
{
  LOGI("Android Main call");
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
}

Fl_PicoAndroid_Screen_Driver::~Fl_PicoAndroid_Screen_Driver()
{
}


double Fl_PicoAndroid_Screen_Driver::wait(double time_to_wait)
{
  Fl::flush();
//  SDL_Event e;
//  if (SDL_PollEvent(&e)) {
//    switch (e.type) {
//      case SDL_QUIT:
//        exit(0);
//      case SDL_WINDOWEVENT_EXPOSED:
//      case SDL_WINDOWEVENT_SHOWN:
//      { // not happening!
//        //event->window.windowID
//        Fl_Window *window = Fl::first_window();
//        if ( !window ) break;;
//        Fl_X *i = Fl_X::i(Fl::first_window());
//        i->wait_for_expose = 0;
//
//        if ( i->region ) {
//          XDestroyRegion(i->region);
//          i->region = 0;
//        }
//        window->clear_damage(FL_DAMAGE_ALL);
//        i->flush();
//        window->clear_damage();
//        Fl_X::first->wait_for_expose = 0;
//      }
//        break;
//
//    }
//  }
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

void Fl_X::flush()
{
  w->driver()->flush();
}

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

int Fl_Window::decorated_h()
{
  return h();
}

int Fl_Window::decorated_w()
{
  return w();
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

