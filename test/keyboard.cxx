//
// Keyboard/event test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

//
// Continuously display FLTK's event state.
//
// Known bugs:
//
// X insists on reporting the state *before* the shift key was
// pressed, rather than after, on shift key events.  I fixed this for
// the mouse buttons, but it did not seem worth it for shift.
//
// X servers do not agree about any shift flags after except shift, ctrl,
// lock, and alt.  They may also not agree about the symbols for the extra
// keys Micro$oft put on the keyboard.
//
// On IRIX the backslash key does not work.  A bug in XKeysymToKeycode?
//

#include "keyboard_ui.h"
#include <string.h>

// these are used to identify which buttons are which:
void key_cb(Fl_Button*, void*) {}
void shift_cb(Fl_Button*, void*) {}
void wheel_cb(Fl_Dial*, void*) {}

// this is used to stop Esc from exiting the program:
int handle(int e) {
  return (e == FL_SHORTCUT); // eat all keystrokes
}

int MyWindow::handle(int event) {
  static int r = 0;
  switch (event) {
    case FL_MOUSEWHEEL: {
      int x = (int)(w_scroll->xvalue() - Fl::event_dx());
      int y = (int)(w_scroll->yvalue() - Fl::event_dy());
      w_scroll->value( (double)(x&31), (double)(y&31) );
      return 1; }
    case FL_ZOOM_GESTURE: {
      int z = (int)(w_zoom->yvalue() + Fl::event_dy());
      w_zoom->value( (double)(z&255), (double)(z&255) );
      return 1; }
    case FL_ROTATE_GESTURE: {
      r = r - (Fl::event_dy()/100.0);
      w_rotate->value( (double)(r&1023) );
      return 1; }
  }
  return 0;
}

struct keycode_table{int n; const char* text;} table[] = {
  {FL_Escape, "FL_Escape"},
  {FL_BackSpace, "FL_BackSpace"},
  {FL_Tab, "FL_Tab"},
  {FL_Iso_Key, "FL_Iso_Key"},
  {FL_Enter, "FL_Enter"},
  {FL_Print, "FL_Print"},
  {FL_Scroll_Lock, "FL_Scroll_Lock"},
  {FL_Pause, "FL_Pause"},
  {FL_Insert, "FL_Insert"},
  {FL_Home, "FL_Home"},
  {FL_Page_Up, "FL_Page_Up"},
  {FL_Delete, "FL_Delete"},
  {FL_End, "FL_End"},
  {FL_Page_Down, "FL_Page_Down"},
  {FL_Left, "FL_Left"},
  {FL_Up, "FL_Up"},
  {FL_Right, "FL_Right"},
  {FL_Down, "FL_Down"},
  {FL_Shift_L, "FL_Shift_L"},
  {FL_Shift_R, "FL_Shift_R"},
  {FL_Control_L, "FL_Control_L"},
  {FL_Control_R, "FL_Control_R"},
  {FL_Caps_Lock, "FL_Caps_Lock"},
  {FL_Alt_L, "FL_Alt_L"},
  {FL_Alt_R, "FL_Alt_R"},
  {FL_Meta_L, "FL_Meta_L"},
  {FL_Meta_R, "FL_Meta_R"},
  {FL_Menu, "FL_Menu"},
  {FL_Help, "FL_Help"},
  {FL_Num_Lock, "FL_Num_Lock"},
  {FL_KP_Enter, "FL_KP_Enter"}
};

int main(int argc, char** argv) {
  Fl::add_handler(handle);
  MyWindow *window = make_window();
  window->show(argc,argv);
  while (Fl::wait()) {
    const char *str;

    // update all the buttons with the current key and shift state:
    for (int c = 0; c < window->children(); c++) {
      Fl_Widget* b = window->child(c);
      if (b->callback() == (Fl_Callback*)key_cb) {
        int i = (int)b->argument();
        if (!i) i = b->label()[0];
        Fl_Button *btn = ((Fl_Button*)b);
        int state = Fl::event_key(i);
        if (btn->value()!=state)
          btn->value(state);
      } else if (b->callback() == (Fl_Callback*)shift_cb) {
        int i = (int)b->argument();
        Fl_Button *btn = ((Fl_Button*)b);
        int state = Fl::event_state(i);
        if (btn->value()!=state)
          btn->value(state);
      }
    }

    // figure out the keyname:
    char buffer[100];
    const char *keyname = buffer;
    int k = Fl::event_key();
    if (!k)
      keyname = "0";
    else if (k < 128) { // ASCII
      snprintf(buffer, sizeof(buffer), "'%c'", k);
    } else if (k >= 0xa0 && k <= 0xff) { // ISO-8859-1 (international keyboards)
      char key[8];
      int kl = fl_utf8encode((unsigned)k, key);
      key[kl] = '\0';
      snprintf(buffer, sizeof(buffer), "'%s'", key);
    } else if (k > FL_F && k <= FL_F_Last) {
      snprintf(buffer, sizeof(buffer), "FL_F+%d", k - FL_F);
    } else if (k >= FL_KP && k <= FL_KP_Last) {
      snprintf(buffer, sizeof(buffer), "FL_KP+'%c'", k-FL_KP);
    } else if (k >= FL_Button && k <= FL_Button+7) {
      snprintf(buffer, sizeof(buffer), "FL_Button+%d", k-FL_Button);
    } else {
      snprintf(buffer, sizeof(buffer), "0x%04x", k);
      for (int i = 0; i < int(sizeof(table)/sizeof(*table)); i++)
        if (table[i].n == k) {keyname = table[i].text; break;}
    }
    if (strcmp(key_output->value(), keyname))
      key_output->value(keyname);

    str = Fl::event_text();
    if (strcmp(text_output->value(), str))
      text_output->value(str);
  }
  return 0;
}
