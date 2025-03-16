//
// Widget Bin Button code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

#include "widgets/Bin_Button.h"

#include "Fluid.h"
#include "nodes/factory.h"
#include "nodes/Window_Node.h"
#include "widgets/Node_Browser.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>

using namespace fld;
using namespace fld::widget;


/** \class fld::widget::Bin_Button
 The Bin_Button button is a button that can be used in the widget bin to
 allow the user to drag and drop widgets into a window or group. This feature
 makes it easy for the user to position a widget at a specific location within
 the window or group.
 */

/**
 Convert mouse dragging into a drag and drop event.
 */
int fld::widget::Bin_Button::handle(int inEvent)
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
        // fake a button release
        Fl_Button::handle(FL_RELEASE);
        // make it into a dnd event
        const char *type_name = (const char*)user_data();
        Fluid.proj.tree.current_dnd = Fluid.proj.tree.current;
        Fl::copy(type_name, (int)strlen(type_name)+1, 0);
        Fl::dnd();
        return 1;
      }
      return ret;
  }
  return Fl_Button::handle(inEvent);
}

/** \class fld::widget::Bin_Window_Button
 The Bin_Window_Button button is used in the widget bin to create new
 windows by dragging and dropping. When the button is dragged and dropped onto
 the desktop, a new window will be created at the drop location.

 This does not work in Wayland because Wayland does not allow client
 applications to control window placement.
 */

/**
 Convert mouse dragging into a drag and drop event.
 */
int fld::widget::Bin_Window_Button::handle(int inEvent)
{
  static Fl_Window *drag_win = nullptr;
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
          drag_win = new Fl_Window(0, 0, 480, 320);
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
        drag_win = nullptr;
        // create a new window here
        Node *prototype = typename_to_prototype((char*)user_data());
        if (prototype) {
          Node *new_type = add_new_widget_from_user(prototype, Strategy::AFTER_CURRENT);
          if (new_type && new_type->is_a(Type::Window)) {
            Window_Node *new_window = (Window_Node*)new_type;
            Fl_Window *w = (Fl_Window *)new_window->o;
            w->position(Fl::event_x_root(), Fl::event_y_root());
          }
        }
        widget_browser->display(Fluid.proj.tree.current);
        widget_browser->rebuild();
      }
      return Fl_Button::handle(inEvent);
  }
  return Fl_Button::handle(inEvent);
}

