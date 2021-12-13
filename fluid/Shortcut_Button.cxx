//
// Widget type code for the Fast Light Tool Kit (FLTK).
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

/// \defgroup fl_type Basic Node for all Widgets and Functions
/// \{

/** \class Fl_Type
Each object described by Fluid is one of these objects.  They
are all stored in a double-linked list.

The "type" of the object is covered by the virtual functions.
There will probably be a lot of these virtual functions.

The type browser is also a list of these objects, but they
are "factory" instances, not "real" ones.  These objects exist
only so the "make" method can be called on them.  They are
not in the linked list and are not written to files or
copied or otherwise examined.
*/

#include "Shortcut_Button.h"

#include "fluid.h"
#include "Fl_Window_Type.h"
#include "factory.h"
#include "widget_panel.h"

#include <FL/platform.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Menu_.H>
#include "../src/flstring.h"

/** \class Shortcut_Button
 A button that allows the user to type a key combination to create shortcuts.
 After clicked once, the button catches the following keyboard events and
 records the pressed keys and all modifiers. It draws a text representation of
 the shortcut. The backspace key deletes the current shortcut.
 */

/**
 Draw the textual representation of the shortcut.
 */
void Shortcut_Button::draw() {
  if (value()) draw_box(FL_DOWN_BOX, (Fl_Color)9);
  else draw_box(FL_UP_BOX, FL_WHITE);
  fl_font(FL_HELVETICA,14); fl_color(FL_FOREGROUND_COLOR);
  if (use_FL_COMMAND && (svalue & (FL_CTRL|FL_META))) {
    char buf[1024];
    fl_snprintf(buf, 1023, "Command+%s", fl_shortcut_label(svalue&~(FL_CTRL|FL_META)));
    fl_draw(buf,x()+6,y(),w(),h(),FL_ALIGN_LEFT);
  } else {
    fl_draw(fl_shortcut_label(svalue),x()+6,y(),w(),h(),FL_ALIGN_LEFT);
  }
}

/**
 Handle keystrokes to catch the user's shortcut.
 */
int Shortcut_Button::handle(int e) {
  when(0); type(FL_TOGGLE_BUTTON);
  if (e == FL_KEYBOARD) {
    if (!value()) return 0;
    int v = Fl::event_text()[0];
    if ( (v > 32 && v < 0x7f) || (v > 0xa0 && v <= 0xff) ) {
      if (isupper(v)) {
        v = tolower(v);
        v |= FL_SHIFT;
      }
      v = v | (Fl::event_state()&(FL_META|FL_ALT|FL_CTRL));
    } else {
      v = (Fl::event_state()&(FL_META|FL_ALT|FL_CTRL|FL_SHIFT)) | Fl::event_key();
      if (v == FL_BackSpace && svalue) v = 0;
    }
    if (v != svalue) {svalue = v; set_changed(); redraw(); do_callback(); }
    return 1;
  } else if (e == FL_UNFOCUS) {
    int c = changed(); value(0); if (c) set_changed();
    return 1;
  } else if (e == FL_FOCUS) {
    return value();
  } else {
    int r = Fl_Button::handle(e);
    if (e == FL_RELEASE && value() && Fl::focus() != this) take_focus();
    return r;
  }
}

/** \class Widget_Bin_Button
 A button for the widget bin that allows the user to drag widgets into a window.
 Dragging and dropping a new widget makes it easy for the user to position
 a widget inside a window or group.
 */

/**
 Convert mouse dragging into a drag and drop event.
 */
int Widget_Bin_Button::handle(int inEvent)
{
  int ret = 0;
  switch (inEvent) {
    case FL_PUSH:
      Fl_Button::handle(inEvent);
      return 1; // make sure that we get drag events
    case FL_DRAG:
      ret = Fl_Button::handle(inEvent);
      if (!user_data())
        return ret;
      if (!Fl::event_is_click()) { // make it a dnd event
        // fake a drag outside of the widget
        Fl::e_x = x()-1;
        Fl_Button::handle(inEvent);
        // fake a buttton release
        Fl_Button::handle(FL_RELEASE);
        // make it into a dnd event
        const char *type_name = (const char*)user_data();
        Fl_Type::current_dnd = Fl_Type::current;
        Fl::copy(type_name, (int)strlen(type_name)+1, 0);
        Fl::dnd();
        return 1;
      }
      return ret;
  }
  return Fl_Button::handle(inEvent);
}

/** \class Widget_Bin_Window_Button
 This button is used by the widget bin to create new windows by drag'n'drop.
 The new window will be created wherever the user drops it on the desktop.
 */

/**
 Convert mouse dragging into a drag and drop event.
 */
int Widget_Bin_Window_Button::handle(int inEvent)
{
  static Fl_Window *drag_win = NULL;
  int ret = 0;
  switch (inEvent) {
    case FL_PUSH:
      Fl_Button::handle(inEvent);
      return 1; // make sure that we get drag events
    case FL_DRAG:
      ret = Fl_Button::handle(inEvent);
      if (!user_data())
        return ret;
      if (!Fl::event_is_click()) {
        if (!drag_win) {
          drag_win = new Fl_Window(0, 0, 100, 100);
          drag_win->border(0);
          drag_win->set_non_modal();
        }
        if (drag_win) {
          drag_win->position(Fl::event_x_root()+1, Fl::event_y_root()+1);
          drag_win->show();
        }
        // Does not work outside window: fl_cursor(FL_CURSOR_HAND);
      }
      return ret;
    case FL_RELEASE:
      if (drag_win) {
        Fl::delete_widget(drag_win);
        drag_win = NULL;
        // create a new window here
        Fl_Type *prototype = typename_to_prototype((char*)user_data());
        if (prototype) {
          Fl_Type *new_type = add_new_widget_from_user(prototype, kAddAfterCurrent);
          if (new_type && new_type->is_window()) {
            Fl_Window_Type *new_window = (Fl_Window_Type*)new_type;
            Fl_Window *w = (Fl_Window *)new_window->o;
            w->position(Fl::event_x_root(), Fl::event_y_root());
          }
        }
      }
      return Fl_Button::handle(inEvent);
  }
  return Fl_Button::handle(inEvent);
}
