/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//#include <android_native_app_glue.h>
#include <src/drivers/Android/Fl_Android_Application.H>
#include <src/drivers/Android/Fl_Android_Screen_Driver.H>

#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Enumerations.H>
Fl_Window *win;
Fl_Button *btn;

#include <errno.h>
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define  LOG_TAG    "HelloFLTK"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

/* Set to 1 to enable debug log traces. */
#define DEBUG 0

// ----------------------------------------------------------------------

struct engine {
    int animating;
};

struct engine engine = { 0 };

static void engine_draw_frame()
{
  //if (Fl_Android_Application::lock_screen()) {
    Fl::damage(FL_DAMAGE_ALL);
    win->redraw();
    Fl::flush();
  // Fl_Android_Application::unlock_and_post_screen();
  //}
}

static void engine_term_display() {
    engine.animating = 0;
}

static int32_t engine_handle_input(AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine.animating = 1;
        Fl::e_x = Fl::e_x_root = AMotionEvent_getX(event, 0) * 600 / ANativeWindow_getWidth(Fl_Android_Application::native_window());
        Fl::e_y = Fl::e_y_root = AMotionEvent_getY(event, 0) * 800 / ANativeWindow_getHeight(Fl_Android_Application::native_window());
        Fl::e_state = FL_BUTTON1;
        Fl::e_keysym = FL_Button+1;
        if (AMotionEvent_getAction(event)==AMOTION_EVENT_ACTION_DOWN) {
          Fl::e_is_click = 1;
          Fl::handle(FL_PUSH, Fl::first_window());
          LOGE("Mouse push %d at %d, %d", Fl::event_button(), Fl::event_x(), Fl::event_y());
        } else if (AMotionEvent_getAction(event)==AMOTION_EVENT_ACTION_MOVE) {
          Fl::handle(FL_DRAG, Fl::first_window());
        } else if (AMotionEvent_getAction(event)==AMOTION_EVENT_ACTION_UP) {
          Fl::e_state = 0;
          Fl::handle(FL_RELEASE, Fl::first_window());
        }
        return 1;
    } else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
        LOGI("Key event: action=%d keyCode=%d metaState=0x%x",
                AKeyEvent_getAction(event),
                AKeyEvent_getKeyCode(event),
                AKeyEvent_getMetaState(event));
    }

    return 0;
}

static void engine_handle_cmd(int32_t cmd) {
    static int32_t format = WINDOW_FORMAT_RGB_565;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (Fl_Android_Application::native_window() != NULL) {
                // fill_plasma() assumes 565 format, get it here
                format = ANativeWindow_getFormat(Fl_Android_Application::native_window());
                ANativeWindow_setBuffersGeometry(Fl_Android_Application::native_window(),
#if 1
                              600, //ANativeWindow_getWidth(app->window),
                              800, //ANativeWindow_getHeight(app->window),
#else
                              ANativeWindow_getWidth(app->window),
                              ANativeWindow_getHeight(app->window),
#endif
                              WINDOW_FORMAT_RGB_565);
                engine_draw_frame();
            }
            break;
        case APP_CMD_TERM_WINDOW:
            engine_term_display();
            ANativeWindow_setBuffersGeometry(Fl_Android_Application::native_window(),
#if 1
                          600, //ANativeWindow_getWidth(app->window),
                          800, //ANativeWindow_getHeight(app->window),
#else
                          ANativeWindow_getWidth(app->window),
                          ANativeWindow_getHeight(app->window),
#endif
                          format);
            break;
        case APP_CMD_LOST_FOCUS:
            engine.animating = 0;
            engine_draw_frame();
            break;
        default: break;
    }
}


int main(int argc, char **argv)
{
  Fl_Android_Application::log_e("App path is %s", argv[0]);

    memset(&engine, 0, sizeof(engine));
  Fl_Android_Application::set_on_app_cmd(engine_handle_cmd);
  Fl_Android_Application::set_on_input_event(engine_handle_input);

    win = new Fl_Window(10, 10, 600, 400, "Hallo");
    btn = new Fl_Button(190, 200, 280, 35, "Hello, Android!");
    btn->color(FL_LIGHT2);
    win->show();
  Fl::damage(FL_DAMAGE_ALL);
  win->redraw();


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
                source->process(source);
            }

            // Check if we are exiting.
            if (Fl_Android_Application::destroy_requested() != 0) {
                LOGI("Engine thread destroy requested!");
                engine_term_display();
                return 0;
            }
        }
        Fl::flush();
    }
  return 0;
}
