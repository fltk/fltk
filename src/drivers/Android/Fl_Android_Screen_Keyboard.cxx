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
