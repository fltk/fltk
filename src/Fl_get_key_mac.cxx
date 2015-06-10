//
// "$Id$"
//
// MacOS keyboard state routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

// Return the current state of a key.  Keys are named by fltk symbols,
// which are actually X keysyms.  So this has to translate to macOS
// symbols.

#include <FL/Fl.H>
#include <FL/x.H>
#include <config.h>

// The list of Mac OS virtual keycodes appears with OS 10.5 in
// ...../Carbon.framework/Frameworks/HIToolbox.framework/Headers/Events.h
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
#include <Carbon/Carbon.h>
#else
/*
 *    These constants are the virtual keycodes defined originally in
 *    Inside Mac Volume V, pg. V-191. They identify physical keys on a
 *    keyboard. Those constants with "ANSI" in the name are labeled
 *    according to the key position on an ANSI-standard US keyboard.
 *    For example, kVK_ANSI_A indicates the virtual keycode for the key
 *    with the letter 'A' in the US keyboard layout. Other keyboard
 *    layouts may have the 'A' key label on a different physical key;
 *    in this case, pressing 'A' will generate a different virtual
 *    keycode.
 */
enum {
  kVK_ANSI_A                    = 0x00,
  kVK_ANSI_S                    = 0x01,
  kVK_ANSI_D                    = 0x02,
  kVK_ANSI_F                    = 0x03,
  kVK_ANSI_H                    = 0x04,
  kVK_ANSI_G                    = 0x05,
  kVK_ANSI_Z                    = 0x06,
  kVK_ANSI_X                    = 0x07,
  kVK_ANSI_C                    = 0x08,
  kVK_ANSI_V                    = 0x09,
  kVK_ANSI_B                    = 0x0B,
  kVK_ANSI_Q                    = 0x0C,
  kVK_ANSI_W                    = 0x0D,
  kVK_ANSI_E                    = 0x0E,
  kVK_ANSI_R                    = 0x0F,
  kVK_ANSI_Y                    = 0x10,
  kVK_ANSI_T                    = 0x11,
  kVK_ANSI_1                    = 0x12,
  kVK_ANSI_2                    = 0x13,
  kVK_ANSI_3                    = 0x14,
  kVK_ANSI_4                    = 0x15,
  kVK_ANSI_6                    = 0x16,
  kVK_ANSI_5                    = 0x17,
  kVK_ANSI_Equal                = 0x18,
  kVK_ANSI_9                    = 0x19,
  kVK_ANSI_7                    = 0x1A,
  kVK_ANSI_Minus                = 0x1B,
  kVK_ANSI_8                    = 0x1C,
  kVK_ANSI_0                    = 0x1D,
  kVK_ANSI_RightBracket         = 0x1E,
  kVK_ANSI_O                    = 0x1F,
  kVK_ANSI_U                    = 0x20,
  kVK_ANSI_LeftBracket          = 0x21,
  kVK_ANSI_I                    = 0x22,
  kVK_ANSI_P                    = 0x23,
  kVK_ANSI_L                    = 0x25,
  kVK_ANSI_J                    = 0x26,
  kVK_ANSI_Quote                = 0x27,
  kVK_ANSI_K                    = 0x28,
  kVK_ANSI_Semicolon            = 0x29,
  kVK_ANSI_Backslash            = 0x2A,
  kVK_ANSI_Comma                = 0x2B,
  kVK_ANSI_Slash                = 0x2C,
  kVK_ANSI_N                    = 0x2D,
  kVK_ANSI_M                    = 0x2E,
  kVK_ANSI_Period               = 0x2F,
  kVK_ANSI_Grave                = 0x32,
  kVK_ANSI_KeypadDecimal        = 0x41,
  kVK_ANSI_KeypadMultiply       = 0x43,
  kVK_ANSI_KeypadPlus           = 0x45,
  kVK_ANSI_KeypadClear          = 0x47,
  kVK_ANSI_KeypadDivide         = 0x4B,
  kVK_ANSI_KeypadEnter          = 0x4C,
  kVK_ANSI_KeypadMinus          = 0x4E,
  kVK_ANSI_KeypadEquals         = 0x51,
  kVK_ANSI_Keypad0              = 0x52,
  kVK_ANSI_Keypad1              = 0x53,
  kVK_ANSI_Keypad2              = 0x54,
  kVK_ANSI_Keypad3              = 0x55,
  kVK_ANSI_Keypad4              = 0x56,
  kVK_ANSI_Keypad5              = 0x57,
  kVK_ANSI_Keypad6              = 0x58,
  kVK_ANSI_Keypad7              = 0x59,
  kVK_ANSI_Keypad8              = 0x5B,
  kVK_ANSI_Keypad9              = 0x5C
};

/* keycodes for keys that are independent of keyboard layout*/
enum {
  kVK_Return                    = 0x24,
  kVK_Tab                       = 0x30,
  kVK_Space                     = 0x31,
  kVK_Delete                    = 0x33,
  kVK_Escape                    = 0x35,
  kVK_Command                   = 0x37,
  kVK_Shift                     = 0x38,
  kVK_CapsLock                  = 0x39,
  kVK_Option                    = 0x3A,
  kVK_Control                   = 0x3B,
  kVK_RightShift                = 0x3C,
  kVK_RightOption               = 0x3D,
  kVK_RightControl              = 0x3E,
  kVK_Function                  = 0x3F,
  kVK_F17                       = 0x40,
  kVK_VolumeUp                  = 0x48,
  kVK_VolumeDown                = 0x49,
  kVK_Mute                      = 0x4A,
  kVK_F18                       = 0x4F,
  kVK_F19                       = 0x50,
  kVK_F20                       = 0x5A,
  kVK_F5                        = 0x60,
  kVK_F6                        = 0x61,
  kVK_F7                        = 0x62,
  kVK_F3                        = 0x63,
  kVK_F8                        = 0x64,
  kVK_F9                        = 0x65,
  kVK_F11                       = 0x67,
  kVK_F13                       = 0x69,
  kVK_F16                       = 0x6A,
  kVK_F14                       = 0x6B,
  kVK_F10                       = 0x6D,
  kVK_F12                       = 0x6F,
  kVK_F15                       = 0x71,
  kVK_Help                      = 0x72,
  kVK_Home                      = 0x73,
  kVK_PageUp                    = 0x74,
  kVK_ForwardDelete             = 0x75,
  kVK_F4                        = 0x76,
  kVK_End                       = 0x77,
  kVK_F2                        = 0x78,
  kVK_PageDown                  = 0x79,
  kVK_F1                        = 0x7A,
  kVK_LeftArrow                 = 0x7B,
  kVK_RightArrow                = 0x7C,
  kVK_DownArrow                 = 0x7D,
  kVK_UpArrow                   = 0x7E
};

/* ISO keyboards only*/
enum {
  kVK_ISO_Section               = 0x0A
};

/* JIS keyboards only*/
enum {
  kVK_JIS_Yen                   = 0x5D,
  kVK_JIS_Underscore            = 0x5E,
  kVK_JIS_KeypadComma           = 0x5F,
  kVK_JIS_Eisu                  = 0x66,
  kVK_JIS_Kana                  = 0x68
};

#endif

// convert an FLTK (X) keysym to a MacOS symbol:
// This table is in numeric order by FLTK symbol order for binary search.
static const struct {unsigned short vk, fltk;} vktab[] = {
  { kVK_Space, ' ' }, { kVK_ANSI_Quote, '\'' }, { kVK_ANSI_Comma, ',' }, { kVK_ANSI_Minus, '-' }, { kVK_ANSI_Period, '.' }, { kVK_ANSI_Slash, '/' }, 
  { kVK_ANSI_0, '0' }, { kVK_ANSI_1, '1' }, { kVK_ANSI_2, '2' }, { kVK_ANSI_3, '3' }, 
  { kVK_ANSI_4, '4' }, { kVK_ANSI_5, '5' }, { kVK_ANSI_6, '6' }, { kVK_ANSI_7, '7' }, 
  { kVK_ANSI_8, '8' }, { kVK_ANSI_9, '9' }, { kVK_ANSI_Semicolon, ';' }, { kVK_ANSI_Equal, '=' },
  { kVK_ANSI_A, 'A' }, { kVK_ANSI_B, 'B' }, { kVK_ANSI_C, 'C' }, { kVK_ANSI_D, 'D' }, 
  { kVK_ANSI_E, 'E' }, { kVK_ANSI_F, 'F' }, { kVK_ANSI_G, 'G' }, { kVK_ANSI_H, 'H' }, 
  { kVK_ANSI_I, 'I' }, { kVK_ANSI_J, 'J' }, { kVK_ANSI_K, 'K' }, { kVK_ANSI_L, 'L' }, 
  { kVK_ANSI_M, 'M' }, { kVK_ANSI_N, 'N' }, { kVK_ANSI_O, 'O' }, { kVK_ANSI_P, 'P' }, 
  { kVK_ANSI_Q, 'Q' }, { kVK_ANSI_R, 'R' }, { kVK_ANSI_S, 'S' }, { kVK_ANSI_T, 'T' }, 
  { kVK_ANSI_U, 'U' }, { kVK_ANSI_V, 'V' }, { kVK_ANSI_W, 'W' }, { kVK_ANSI_X, 'X' }, 
  { kVK_ANSI_Y, 'Y' }, { kVK_ANSI_Z, 'Z' }, 
  { kVK_ANSI_LeftBracket, '[' }, { kVK_ANSI_Backslash, '\\' }, { kVK_ANSI_RightBracket, ']' }, { kVK_ANSI_Grave, '`' },  
  { kVK_VolumeDown, FL_Volume_Down}, { kVK_Mute, FL_Volume_Mute}, { kVK_VolumeUp, FL_Volume_Up},
  { kVK_Delete, FL_BackSpace }, { kVK_Tab, FL_Tab }, { kVK_ISO_Section, FL_Iso_Key }, { kVK_Return, FL_Enter }, /*{ 0x7F, FL_Pause },
  { 0x7F, FL_Scroll_Lock },*/ { kVK_Escape, FL_Escape }, 
  { kVK_JIS_Kana, FL_Kana}, { kVK_JIS_Eisu, FL_Eisu}, { kVK_JIS_Yen, FL_Yen}, { kVK_JIS_Underscore, FL_JIS_Underscore},
  { kVK_Home, FL_Home }, { kVK_LeftArrow, FL_Left },
  { kVK_UpArrow, FL_Up }, { kVK_RightArrow, FL_Right }, { kVK_DownArrow, FL_Down }, { kVK_PageUp, FL_Page_Up },
  { kVK_PageDown, FL_Page_Down },  { kVK_End, FL_End }, /*{ 0x7F, FL_Print }, { 0x7F, FL_Insert },*/
  { 0x6e, FL_Menu }, { kVK_Help, FL_Help }, { kVK_ANSI_KeypadClear, FL_Num_Lock },
  { kVK_ANSI_KeypadEnter, FL_KP_Enter }, { kVK_ANSI_KeypadMultiply, FL_KP+'*' }, { kVK_ANSI_KeypadPlus, FL_KP+'+'}, 
  { kVK_JIS_KeypadComma, FL_KP+',' },
  { kVK_ANSI_KeypadMinus, FL_KP+'-' }, { kVK_ANSI_KeypadDecimal, FL_KP+'.' }, { kVK_ANSI_KeypadDivide, FL_KP+'/' }, 
  { kVK_ANSI_Keypad0, FL_KP+'0' }, { kVK_ANSI_Keypad1, FL_KP+'1' }, { kVK_ANSI_Keypad2, FL_KP+'2' }, { kVK_ANSI_Keypad3, FL_KP+'3' }, 
  { kVK_ANSI_Keypad4, FL_KP+'4' }, { kVK_ANSI_Keypad5, FL_KP+'5' }, { kVK_ANSI_Keypad6, FL_KP+'6' }, { kVK_ANSI_Keypad7, FL_KP+'7' }, 
  { kVK_ANSI_Keypad8, FL_KP+'8' }, { kVK_ANSI_Keypad9, FL_KP+'9' }, { kVK_ANSI_KeypadEquals, FL_KP+'=' }, 
  { kVK_F1, FL_F+1 }, { kVK_F2, FL_F+2 }, { kVK_F3, FL_F+3 }, { kVK_F4, FL_F+4 }, 
  { kVK_F5, FL_F+5 }, { kVK_F6, FL_F+6 }, { kVK_F7, FL_F+7 }, { kVK_F8, FL_F+8 }, 
  { kVK_F9, FL_F+9 }, { kVK_F10, FL_F+10 }, { kVK_F11, FL_F+11 }, { kVK_F12, FL_F+12 }, 
  { kVK_F13, FL_F+13 }, { kVK_F14, FL_F+14 }, { kVK_F15, FL_F+15 }, { kVK_F16, FL_F+16 }, 
  { kVK_F17, FL_F+17 }, { kVK_F18, FL_F+18 }, { kVK_F19, FL_F+19 }, { kVK_F20, FL_F+20 }, 
  { kVK_Shift, FL_Shift_L }, { kVK_RightShift, FL_Shift_R }, { kVK_Control, FL_Control_L }, { kVK_RightControl, FL_Control_R }, 
  { kVK_CapsLock, FL_Caps_Lock }, { kVK_Command, FL_Meta_L }, { 0x36, FL_Meta_R },
  { kVK_Option, FL_Alt_L }, { kVK_RightOption, FL_Alt_R }, { kVK_ForwardDelete, FL_Delete }
};

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
int Fl::event_key(int k) {
  return get_key(k);
}

//: returns true, if that key is pressed right now
int Fl::get_key(int k) {
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
  if (!f) f = ( keymap_f )Fl_X::get_carbon_function("GetKeys");
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

//
// End of "$Id$".
//
