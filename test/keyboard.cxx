//
// "$Id: keyboard.cxx,v 1.5.2.3 2001/01/22 15:13:41 easysw Exp $"
//
// Keyboard/event test program for the Fast Light Tool Kit (FLTK).
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
// Copyright 1998-2001 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include "keyboard_ui.cxx"
#include <stdio.h>

// these are used to identify which buttons are which:
void key_cb(Fl_Button*, void*) {}
void shift_cb(Fl_Button*, void*) {}

// this is used to stop Esc from exiting the program:
int handle(int e) {
  return (e == FL_SHORTCUT); // eat all keystrokes
}

struct {int n; const char* text;} table[] = {
  {FL_Escape, "FL_Escape"},
  {FL_BackSpace, "FL_BackSpace"},
  {FL_Tab, "FL_Tab"},
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
  {FL_Num_Lock, "FL_Num_Lock"},
  {FL_KP_Enter, "FL_KP_Enter"}
};

int main(int argc, char** argv) {
  Fl::add_handler(handle);
  Fl_Window *window = make_window();
  window->show(argc,argv);
  while (Fl::wait()) {
    
    // update all the buttons with the current key and shift state:
    for (int i = 0; i < window->children(); i++) {
      Fl_Widget* b = window->child(i);
      if (b->callback() == (Fl_Callback*)key_cb) {
	int i = int(b->user_data());
	if (!i) i = b->label()[0];
	((Fl_Button*)b)->value(Fl::event_key(i));
      } else if (b->callback() == (Fl_Callback*)shift_cb) {
	int i = int(b->user_data());
	((Fl_Button*)b)->value(Fl::event_state(i));
      }
    }

    // figure out the keyname:
    char buffer[100];
    const char *keyname = buffer;
    int k = Fl::event_key();
    if (!k)
      keyname = "0";
    else if (k < 256) {
      sprintf(buffer, "'%c'", k);
    } else if (k >= FL_F && k <= FL_F_Last) {
      sprintf(buffer, "FL_F+%d", k - FL_F);
    } else if (k >= FL_KP && k <= FL_KP_Last) {
      sprintf(buffer, "FL_KP+'%c'", k-FL_KP);
    } else if (k >= FL_Button && k <= FL_Button+7) {
      sprintf(buffer, "FL_Button+%d", k-FL_Button);
    } else {
      sprintf(buffer, "0x%04x", k);
      for (int i = 0; i < int(sizeof(table)/sizeof(*table)); i++)
	if (table[i].n == k) {keyname = table[i].text; break;}
    }
    key_output->value(keyname);

    text_output->value(Fl::event_text());
  }
  return 0;
}

//
// End of "$Id: keyboard.cxx,v 1.5.2.3 2001/01/22 15:13:41 easysw Exp $".
//
