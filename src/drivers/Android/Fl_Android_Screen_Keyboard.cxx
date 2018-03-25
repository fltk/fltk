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


// convert an FLTK (X) keysym to a MacOS symbol:
// This table is in numeric order by FLTK symbol order for binary search.
static const struct {unsigned short vk, fltk;} vktab[] = {
        { AKEYCODE_SPACE, ' ' }, { AKEYCODE_APOSTROPHE, '\'' }, { AKEYCODE_COMMA, ',' }, { AKEYCODE_MINUS, '-' }, { AKEYCODE_PERIOD, '.' }, { AKEYCODE_SLASH, '/' },
        { AKEYCODE_0, '0' }, { AKEYCODE_1, '1' }, { AKEYCODE_2, '2' }, { AKEYCODE_3, '3' },
        { AKEYCODE_4, '4' }, { AKEYCODE_5, '5' }, { AKEYCODE_6, '6' }, { AKEYCODE_7, '7' },
        { AKEYCODE_8, '8' }, { AKEYCODE_9, '9' }, { AKEYCODE_SEMICOLON, ';' }, { AKEYCODE_EQUALS, '=' },
        { AKEYCODE_A, 'A' }, { AKEYCODE_B, 'B' }, { AKEYCODE_C, 'C' }, { AKEYCODE_D, 'D' },
        { AKEYCODE_E, 'E' }, { AKEYCODE_F, 'F' }, { AKEYCODE_G, 'G' }, { AKEYCODE_H, 'H' },
        { AKEYCODE_I, 'I' }, { AKEYCODE_J, 'J' }, { AKEYCODE_K, 'K' }, { AKEYCODE_L, 'L' },
        { AKEYCODE_M, 'M' }, { AKEYCODE_N, 'N' }, { AKEYCODE_O, 'O' }, { AKEYCODE_P, 'P' },
        { AKEYCODE_Q, 'Q' }, { AKEYCODE_R, 'R' }, { AKEYCODE_S, 'S' }, { AKEYCODE_T, 'T' },
        { AKEYCODE_U, 'U' }, { AKEYCODE_V, 'V' }, { AKEYCODE_W, 'W' }, { AKEYCODE_X, 'X' },
        { AKEYCODE_Y, 'Y' }, { AKEYCODE_Z, 'Z' },
        { AKEYCODE_LEFT_BRACKET, '[' }, { AKEYCODE_BACKSLASH, '\\' }, { AKEYCODE_RIGHT_BRACKET, ']' }, { AKEYCODE_GRAVE, '`' },
        { AKEYCODE_VOLUME_DOWN, FL_Volume_Down}, { AKEYCODE_MUTE, FL_Volume_Mute}, { AKEYCODE_VOLUME_UP, FL_Volume_Up},
#if 0
            #define FL_Volume_Down  0xEF11   /* Volume control down        */
        513 #define FL_Volume_Mute  0xEF12   /* Mute sound from the system */
        514 #define FL_Volume_Up    0xEF13   /* Volume control up          */
        515 #define FL_Media_Play   0xEF14   /* Start playing of audio     */
        516 #define FL_Media_Stop   0xEF15   /* Stop playing audio         */
        517 #define FL_Media_Prev   0xEF16   /* Previous track             */
        518 #define FL_Media_Next   0xEF17   /* Next track                 */
        519 #define FL_Home_Page    0xEF18   /* Display user's home page   */
        520 #define FL_Mail         0xEF19   /* Invoke user's mail program */
        521 #define FL_Search       0xEF1B   /* Search                     */
        522 #define FL_Back         0xEF26   /* Like back on a browser     */
        523 #define FL_Forward      0xEF27   /* Like forward on a browser  */
        524 #define FL_Stop         0xEF28   /* Stop current operation     */
        525 #define FL_Refresh      0xEF29   /* Refresh the page           */
        526 #define FL_Sleep        0xEF2F   /* Put system to sleep        */
        527 #define FL_Favorites    0xEF30   /* Show favorite locations    */
        528
#endif
        { AKEYCODE_DEL, FL_BackSpace }, { AKEYCODE_TAB, FL_Tab }, { AKEYCODE_POUND, FL_Iso_Key }, { AKEYCODE_ENTER, FL_Enter }, /*{ 0x7F, FL_Pause },
  { 0x7F, FL_Scroll_Lock },*/ { AKEYCODE_ESCAPE, FL_Escape },
        { AKEYCODE_KANA, FL_Kana}, { AKEYCODE_EISU, FL_Eisu}, { AKEYCODE_YEN, FL_Yen}, /*{ AKEYCODE_UND, FL_JIS_Underscore},*/
        { AKEYCODE_MOVE_HOME, FL_Home }, { AKEYCODE_DPAD_LEFT, FL_Left },
        { AKEYCODE_DPAD_UP, FL_Up }, { AKEYCODE_DPAD_RIGHT, FL_Right }, { AKEYCODE_DPAD_DOWN, FL_Down }, { AKEYCODE_PAGE_UP, FL_Page_Up },
        { AKEYCODE_PAGE_DOWN, FL_Page_Down },  { AKEYCODE_MOVE_END, FL_End }, { AKEYCODE_SYSRQ, FL_Print }, { AKEYCODE_INSERT, FL_Insert },
        { AKEYCODE_MENU, FL_Menu }, { AKEYCODE_HELP, FL_Help }, { AKEYCODE_NUM_LOCK, FL_Num_Lock },
        { AKEYCODE_NUMPAD_ENTER, FL_KP_Enter }, { AKEYCODE_NUMPAD_MULTIPLY, FL_KP+'*' }, { AKEYCODE_NUMPAD_ADD, FL_KP+'+'},
        { AKEYCODE_NUMPAD_COMMA, FL_KP+',' },
        { AKEYCODE_NUMPAD_SUBTRACT, FL_KP+'-' }, { AKEYCODE_NUMPAD_DOT, FL_KP+'.' }, { AKEYCODE_NUMPAD_DIVIDE, FL_KP+'/' },
        { AKEYCODE_NUMPAD_0, FL_KP+'0' }, { AKEYCODE_NUMPAD_1, FL_KP+'1' }, { AKEYCODE_NUMPAD_2, FL_KP+'2' }, { AKEYCODE_NUMPAD_3, FL_KP+'3' },
        { AKEYCODE_NUMPAD_4, FL_KP+'4' }, { AKEYCODE_NUMPAD_5, FL_KP+'5' }, { AKEYCODE_NUMPAD_6, FL_KP+'6' }, { AKEYCODE_NUMPAD_7, FL_KP+'7' },
        { AKEYCODE_NUMPAD_8, FL_KP+'8' }, { AKEYCODE_NUMPAD_9, FL_KP+'9' }, { AKEYCODE_NUMPAD_EQUALS, FL_KP+'=' },
        { AKEYCODE_F1, FL_F+1 }, { AKEYCODE_F2, FL_F+2 }, { AKEYCODE_F3, FL_F+3 }, { AKEYCODE_F4, FL_F+4 },
        { AKEYCODE_F5, FL_F+5 }, { AKEYCODE_F6, FL_F+6 }, { AKEYCODE_F7, FL_F+7 }, { AKEYCODE_F8, FL_F+8 },
        { AKEYCODE_F9, FL_F+9 }, { AKEYCODE_F10, FL_F+10 }, { AKEYCODE_F11, FL_F+11 }, { AKEYCODE_F12, FL_F+12 },
        //{ AKEYCODE_F13, FL_F+13 }, { AKEYCODE_F14, FL_F+14 }, { AKEYCODE_F15, FL_F+15 }, { AKEYCODE_F16, FL_F+16 },
        //{ AKEYCODE_F17, FL_F+17 }, { AKEYCODE_F18, FL_F+18 }, { AKEYCODE_F19, FL_F+19 }, { AKEYCODE_F20, FL_F+20 },
        { AKEYCODE_SHIFT_LEFT, FL_Shift_L }, { AKEYCODE_SHIFT_RIGHT, FL_Shift_R }, { AKEYCODE_CTRL_LEFT, FL_Control_L }, { AKEYCODE_CTRL_RIGHT, FL_Control_R },
        { AKEYCODE_CAPS_LOCK, FL_Caps_Lock }, { AKEYCODE_META_LEFT, FL_Meta_L }, { AKEYCODE_META_RIGHT, FL_Meta_R },
        { AKEYCODE_ALT_LEFT, FL_Alt_L }, { AKEYCODE_ALT_RIGHT, FL_Alt_R }, { AKEYCODE_FORWARD_DEL, FL_Delete }
};

#if 0


//    public static final int KEYCODE_ALL_APPS = 284;
//    private static final int LAST_KEYCODE = KEYCODE_ALL_APPS;

// Computes the macKeyLookUp table that transforms a Mac OS virtual keycode into an FLTK keysym
unsigned short *fl_compute_macKeyLookUp()
{
  static unsigned short macKeyLookUp[128];
  memset(macKeyLookUp, 0, sizeof(macKeyLookUp));
  for (unsigned i = 0; i < sizeof(vktab)/sizeof(*vktab); i++) {
    macKeyLookUp[vktab[i].vk] = vktab[i].fltk;
  }
  return macKeyLookUp;
}

static int fltk2mac(int fltk) {
  int a = 0;
  int b = sizeof(vktab)/sizeof(*vktab);
  while (a < b) {
    int c = (a+b)/2;
    if (vktab[c].fltk == fltk) return vktab[c].vk;
    if (vktab[c].fltk < fltk) a = c+1; else b = c;
  }
  return vktab[a].vk;
}

//: returns true, if that key was pressed during the last event
int Fl_Darwin_System_Driver::event_key(int k) {
  return get_key(k);
}

//: returns true, if that key is pressed right now
int Fl_Darwin_System_Driver::get_key(int k) {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (&CGEventSourceKeyState != NULL) {
    return (int)CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState, fltk2mac(k) );
  }
  else
#endif
  {
    typedef UInt32 fl_KeyMap[4];
    fl_KeyMap foo;
    // use the GetKeys Carbon function
    typedef void (*keymap_f)(fl_KeyMap);
    static keymap_f f = NULL;
    if (!f) f = ( keymap_f )Fl_Darwin_System_Driver::get_carbon_function("GetKeys");
    (*f)(foo);
#ifdef MAC_TEST_FOR_KEYCODES
    static int cnt = 0;
 if (cnt++>1024) {
  cnt = 0;
  printf("%08x %08x %08x %08x\n", (ulong*)(foo)[3], (ulong*)(foo)[2], (ulong*)(foo)[1], (ulong*)(foo)[0]);
 }
#endif
    unsigned char *b = (unsigned char*)foo;
    // KP_Enter can be at different locations for Powerbooks vs. desktop Macs
    if (k==FL_KP_Enter) {
      return (((b[0x34>>3]>>(0x34&7))&1)||((b[0x4c>>3]>>(0x4c&7))&1));
    }
    int i = fltk2mac(k);
    return (b[i>>3]>>(i&7))&1;
  }
}

#endif



int Fl_Android_Screen_Driver::compose(int &del)
{
  int condition;
  unsigned char ascii = (unsigned char)Fl::e_text[0];
  condition = (Fl::e_state & (FL_ALT | FL_META | FL_CTRL)) && !(ascii & 128) ;
  if (condition) { del = 0; return 0;} // this stuff is to be treated as a function key
  del = Fl::compose_state;
  Fl::compose_state = 0;
  // Only insert non-control characters:
  if ( (!Fl::compose_state) && ! (ascii & ~31 && ascii!=127)) { return 0; }
  return 1;
}


static unsigned short *key_lookup = nullptr;

// Computes the macKeyLookUp table that transforms a Mac OS virtual keycode into an FLTK keysym
static unsigned short *compute_key_lookup()
{
  static unsigned short AndroidKeyLookUp[AKEYCODE_ALL_APPS+1];
  memset(AndroidKeyLookUp, 0, sizeof(AndroidKeyLookUp));
  for (unsigned i = 0; i < sizeof(vktab)/sizeof(*vktab); i++) {
    AndroidKeyLookUp[vktab[i].vk] = vktab[i].fltk;
  }
  return AndroidKeyLookUp;
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
      //   getDeadChar()
      //
      // This seems to work for hardware keys only:
//      public static interface View.OnKeyListener
//      boolean onKey (View v,
//                     int keyCode,
//                     KeyEvent event)
      // public static interface KeyEvent.Callback
      //   onKeyDown(int keyCode, KeyEvent event)
      // public static interface Window.Callback
      //   abstract boolean dispatchKeyEvent(KeyEvent event)
      // view.setOnKeyListener(new OnKeyListener()
      //
      // NativeApplication.nativeApplication.addEventListener(KeyboardEvent.KEY_DOWN,checkKeypress);
      // public function CheckKeypress(event:KeyboardEvent):void
      //
      // https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/view/KeyEvent.java
    } else {
      // send keycode as many times as getRepeatCount() / AKeyEvent_getRepeatCount(event)
    }
  }

  int unicodeKey = 0;
  auto aKeyCode = AKeyEvent_getKeyCode(event);

  Fl_Android_Java java;
  if (java.is_attached()) {

    jclass class_key_event = java.env()->FindClass("android/view/KeyEvent");

    jmethodID eventConstructor = java.env()->GetMethodID(
            class_key_event, "<init>",
            "(JJIIIIIIII)V");

    jobject eventObj = java.env()->NewObject(
            class_key_event, eventConstructor,
            AKeyEvent_getDownTime(event),
            AKeyEvent_getEventTime(event),
            AKeyEvent_getAction(event),
            aKeyCode,
            AKeyEvent_getRepeatCount(event),
            AKeyEvent_getMetaState(event),
            AInputEvent_getDeviceId(event),
            AKeyEvent_getScanCode(event),
            AKeyEvent_getFlags(event),
            AInputEvent_getSource(event));

    jmethodID method_get_unicode_char = java.env()->GetMethodID(
            class_key_event,
            "getUnicodeChar",
            "(I)I");

    unicodeKey = java.env()->CallIntMethod(
            eventObj, method_get_unicode_char,
            AKeyEvent_getMetaState(event));

    java.env()->DeleteLocalRef(class_key_event);
    java.env()->DeleteLocalRef(eventObj);
  }

  static char buf[8];
  int len = fl_utf8encode(unicodeKey, buf);
  if (len >= 0 && len < 8)
    buf[len] = 0;
  else
    buf[0] = 0;
  Fl_Android_Application::log_i("Unicode: %d Text: %s", unicodeKey, buf);

  if (!key_lookup) key_lookup = compute_key_lookup();
  Fl::e_keysym = (aKeyCode>AKEYCODE_ALL_APPS) ? 0 : key_lookup[aKeyCode];

  AInputQueue_finishEvent(queue, event, 0);

  Fl_Widget *w = Fl::focus();
  if (w) {
    Fl_Window *win = w->window();
    if (keyAction==AKEY_EVENT_ACTION_DOWN) {
      if (unicodeKey>0) {
        Fl::e_text = buf;
        Fl::e_length = len;
      } else {
        Fl::e_text = (char*)"";
        Fl::e_length = 0;
      }
      Fl::handle(FL_KEYBOARD, win);
    }
  }

  return 0;
}


void Fl_Android_Screen_Driver::request_keyboard()
{
  if (pKeyboardCount==0) {
    /*
     * The following line does not work as of March 2018. The pseudo-Java
     * code that follows is needed to make the virtaul keyboard show.
     *
    ANativeActivity_showSoftInput(Fl_Android_Application::get_activity(),
                                  ANATIVEACTIVITY_SHOW_SOFT_INPUT_IMPLICIT);
     *
     * This is the actual Java code that we recreate in C++
     *
    InputMethodManager imm = ( InputMethodManager )getSystemService( Context.INPUT_METHOD_SERVICE );
    imm.showSoftInput( this.getWindow().getDecorView(), InputMethodManager.SHOW_FORCED );
     */

    Fl_Android_Java java;
    if (java.is_attached()) {

      jint lFlags = 0;

      // Retrieves Context.INPUT_METHOD_SERVICE.
      jclass ClassContext = java.env()->FindClass("android/content/Context");
      jfieldID FieldINPUT_METHOD_SERVICE = java.env()->GetStaticFieldID(
              ClassContext,
              "INPUT_METHOD_SERVICE",
              "Ljava/lang/String;");
      jobject INPUT_METHOD_SERVICE = java.env()->GetStaticObjectField(
              ClassContext,
              FieldINPUT_METHOD_SERVICE);

      // Runs getSystemService(Context.INPUT_METHOD_SERVICE).
      jclass ClassInputMethodManager = java.env()->FindClass(
              "android/view/inputmethod/InputMethodManager");
      jmethodID MethodGetSystemService = java.env()->GetMethodID(
              java.native_activity_class(), "getSystemService",
              "(Ljava/lang/String;)Ljava/lang/Object;");
      jobject lInputMethodManager = java.env()->CallObjectMethod(
              java.native_ativity(), MethodGetSystemService,
              INPUT_METHOD_SERVICE);

      // Runs getWindow().getDecorView().
      jmethodID MethodGetWindow = java.env()->GetMethodID(
              java.native_activity_class(), "getWindow",
              "()Landroid/view/Window;");
      jobject lWindow = java.env()->CallObjectMethod(
              java.native_ativity(),
              MethodGetWindow);
      jclass ClassWindow = java.env()->FindClass(
              "android/view/Window");
      jmethodID MethodGetDecorView = java.env()->GetMethodID(
              ClassWindow, "getDecorView", "()Landroid/view/View;");
      jobject lDecorView = java.env()->CallObjectMethod(
              lWindow,
              MethodGetDecorView);

      // Runs lInputMethodManager.showSoftInput(...).
      jmethodID MethodShowSoftInput = java.env()->GetMethodID(
              ClassInputMethodManager, "showSoftInput",
              "(Landroid/view/View;I)Z");
      jboolean lResult = java.env()->CallBooleanMethod(
              lInputMethodManager, MethodShowSoftInput,
              lDecorView, lFlags);

      java.env()->DeleteLocalRef(ClassContext);
      java.env()->DeleteLocalRef(ClassInputMethodManager);
      java.env()->DeleteLocalRef(ClassWindow);

      java.env()->DeleteLocalRef(INPUT_METHOD_SERVICE);
      java.env()->DeleteLocalRef(lInputMethodManager);
      java.env()->DeleteLocalRef(lWindow);
      java.env()->DeleteLocalRef(lDecorView);
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
