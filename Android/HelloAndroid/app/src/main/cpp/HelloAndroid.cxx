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
#include <src/drivers/Android/Fl_Android_Screen_Driver.H>

#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>

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
    struct android_app* app;
    int animating;
};

ANativeWindow_Buffer* gAGraphicsBuffer = 0;

static int64_t start_ms;
static void engine_draw_frame(struct engine* engine)
{
    if (engine->app->window == NULL) {
        // No window.
        return;
    }

    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(engine->app->window, &buffer, NULL) < 0) {
        LOGW("Unable to lock window buffer");
        return;
    }

    gAGraphicsBuffer = &buffer;
    Fl::damage(FL_DAMAGE_ALL);
    win->redraw();
    Fl::flush();

    ANativeWindow_unlockAndPost(engine->app->window);
    gAGraphicsBuffer = 0L;
}

static void engine_term_display(struct engine* engine) {
    engine->animating = 0;
}

static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;
        Fl::e_x = Fl::e_x_root = AMotionEvent_getX(event, 0) * 600 / ANativeWindow_getWidth(app->window);
        Fl::e_y = Fl::e_y_root = AMotionEvent_getY(event, 0) * 800 / ANativeWindow_getHeight(app->window);
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

static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    static int32_t format = WINDOW_FORMAT_RGB_565;
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (engine->app->window != NULL) {
                // fill_plasma() assumes 565 format, get it here
                format = ANativeWindow_getFormat(app->window);
                ANativeWindow_setBuffersGeometry(app->window,
#if 1
                              600, //ANativeWindow_getWidth(app->window),
                              800, //ANativeWindow_getHeight(app->window),
#else
                              ANativeWindow_getWidth(app->window),
                              ANativeWindow_getHeight(app->window),
#endif
                              WINDOW_FORMAT_RGB_565);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            engine_term_display(engine);
            ANativeWindow_setBuffersGeometry(app->window,
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
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
        default: break;
    }
}


void android_main(struct android_app* state)
{
    struct engine engine;

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    start_ms = (((int64_t)now.tv_sec)*1000000000LL + now.tv_nsec)/1000000;

    win = new Fl_Window(10, 10, 600, 400, "Hallo");
    btn = new Fl_Button(190, 200, 280, 35, "Hello, Android!");
    win->show();


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

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                LOGI("Engine thread destroy requested!");
                engine_term_display(&engine);
                return;
            }
        }

        if (engine.animating) {
            engine_draw_frame(&engine);
        }
    }
}
