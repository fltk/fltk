//
// MacOS keyboard state routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2026 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

// Note: this file contains platform specific code and will therefore
// not be processed by doxygen (see Doxyfile.in).

// Return the current state of a key.  Keys are named by fltk symbols,
// which are actually X keysyms.  So this has to translate to macOS
// symbols.

#include <FL/Fl.H>
#include <FL/platform.H>
#include "drivers/Darwin/Fl_Darwin_System_Driver.H"
#include "drivers/Cocoa/Fl_Cocoa_Screen_Driver.H"

// The list of Mac OS virtual keycodes appears with OS 10.5 in
// ...../Carbon.framework/Frameworks/HIToolbox.framework/Headers/Events.h
#include <Carbon/Carbon.h>

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
unsigned short *Fl_Darwin_System_Driver::compute_macKeyLookUp()
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
int Fl_Cocoa_Screen_Driver::event_key(int k) {
  return get_key(k);
}

//: returns true, if that key is pressed right now
int Fl_Cocoa_Screen_Driver::get_key(int k) {
  return (int)CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState, fltk2mac(k) ); // 10.4
}
