//
// Shortcut Button  code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

#include <FL/Fl_Shortcut_Button.H>

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_utf8.h>
#include "Fl_System_Driver.H"
#include "flstring.h"

#include <ctype.h>


/** \class Fl_Shortcut_Button
 A button that allows the user to type a key combination to create shortcuts.
 After clicked once, the button catches the following keyboard events and
 records the pressed keys and all modifiers. It draws a text representation of
 the shortcut.

 The backspace key deletes the current shortcut. A second click on the button
 or moving focus makes the last shortcut permanent.

 The Shortcut button calls the user callback after every change if
 FL_WHEN_CHANGED is set, and when the button is no longer recording
 shortcuts if FL_WHEN_RELEASE is set.
 */

/** Construct a shortcut button.
 \param X, Y, W, H position and size of the button
 \param l label text when no shortcut is set
 */
Fl_Shortcut_Button::Fl_Shortcut_Button(int X,int Y,int W,int H, const char* l)
: Fl_Button(X,Y,W,H,l),
  hot_(false),
  pre_hot_(false),
  default_set_(false),
  handle_default_button_(false),
  pre_esc_(0),
  default_shortcut_(0),
  shortcut_value(0)
{
  box(FL_DOWN_BOX);
  selection_color(FL_SELECTION_COLOR);
  type(FL_TOGGLE_BUTTON);
  // suppress warning on unused private members. I keep these around in case
  // we decide to activate this API again without changing the ABI.
  (void)default_shortcut_;
  (void)default_set_;

}

/**
 Set the displayed shortcut.
 \param[in] shortcut encoded as key and modifier
 */
void Fl_Shortcut_Button::value(Fl_Shortcut shortcut) {
  shortcut_value = shortcut;
  clear_changed();
  redraw();
}

/**
 Return the user selected shortcut.
 \return shortcut encoded as key and modifier
 */
Fl_Shortcut Fl_Shortcut_Button::value() {
  return shortcut_value;
}

#if 0
// Default shortcut settings are disabled until successful review of the UI
/* *
 Set the default shortcut.
 If set, and additional 'reverse' button apears that the user can click to
 reset the shortcut to some default value (including 0).
 \param[in] shortcut encoded as key and modifier
 */
void Fl_Shortcut_Button::default_value(Fl_Shortcut shortcut) {
  default_shortcut_ = shortcut;
  default_set_ = true;
  redraw();
}
#endif

#if 0
// Default shortcut settings are disabled until successful review of the UI
/* *
 Return the default shortcut.
 \return shortcut encoded as key and modifier
 */
Fl_Shortcut Fl_Shortcut_Button::default_value() {
  return default_shortcut_;
}
#endif

#if 0
// Default shortcut settings are disabled until successful review of the UI
/* *
 No longer show the button to reverse to a default shortcut.
 */
void Fl_Shortcut_Button::default_clear() {
  default_set_ = false;
  redraw();
}
#endif

/**
 Draw the textual representation of the shortcut button.

 When the button can receive shortcut key events, it's "hot". A hot button
 is drawn in selection color. A cold button is drawn as a regular text box
 containing a human readable version of the shortcut key.
 */
void Fl_Shortcut_Button::draw() {
  Fl_Color col = hot_ ? selection_color() : color();
  Fl_Boxtype b = box();
  if (hot_) {
    if (down_box())
      b = down_box();
    else if ((b > FL_FLAT_BOX) && (b < FL_BORDER_BOX))
      b = Fl_Boxtype(((int)b) ^ 1);
  }
  draw_box(b, col);
  draw_backdrop();

  int X = x() + Fl::box_dx(box());
  int Y = y() + Fl::box_dy(box());
  int W = w() - Fl::box_dw(box());
  int H = h() - Fl::box_dh(box());
  Fl_Color textcol = fl_contrast(labelcolor(), col);
  if (!active_r())
    textcol = fl_inactive(textcol);
  fl_color(textcol);
  fl_font(labelfont(), labelsize());
  const char *text = label();
  if (shortcut_value)
    text = fl_shortcut_label(shortcut_value);
#if 0
  if (default_set_) {
    fl_draw(text, X, Y, W-H, H, align() | FL_ALIGN_INSIDE);
    fl_draw_symbol("@-29undo", X+W-H, Y, H, H, textcol);
  } else {
    fl_draw(text, X, Y, W, H, align() | FL_ALIGN_INSIDE);
  }
#else
  fl_draw(text, X, Y, W, H, align() | FL_ALIGN_INSIDE);
#endif
  if (Fl::focus() == this) draw_focus();
}

/**
 Call the callback if the user is interested.
 */
void Fl_Shortcut_Button::do_end_hot_callback() {
  if (when() & FL_WHEN_RELEASE) {
    do_callback(FL_REASON_RELEASED);
  }
}

/**
 Handle keystrokes to catch the user's shortcut.
 */
int Fl_Shortcut_Button::handle(int e) {
  static int alt_modifier_extra_handler = Fl::system_driver()->need_test_shortcut_extra();
#if 0
  bool inside_default_button = false;
  if (default_set_ && ( (e == FL_PUSH) || (e == FL_DRAG) || (e == FL_RELEASE) ) ) {
    int X = x() + Fl::box_dx(box());
    int W = w() - Fl::box_dw(box());
    int H = h() - Fl::box_dh(box());
    if (Fl::event_inside(this) && (Fl::event_x() > X+W-H))
      inside_default_button = true;
  }
  if ((e == FL_PUSH) && default_set_ && inside_default_button) {
    if (Fl::visible_focus() && handle(FL_FOCUS)) Fl::focus(this);
    handle_default_button_ = true;
    return 1;
  }
  if (handle_default_button_) {
    if (e == FL_DRAG)
      return 1;
    if (e == FL_RELEASE) {
      if (inside_default_button && (shortcut_value != default_shortcut_)) {
        shortcut_value = default_shortcut_;
        set_changed();
        redraw();
        if (when() & FL_WHEN_CHANGED) do_callback(FL_REASON_CHANGED);
        clear_changed();
      }
      handle_default_button_ = false;
      return 1;
    }
  }
#endif
  switch (e) {
    case FL_PUSH:
      if (Fl::visible_focus() && handle(FL_FOCUS)) Fl::focus(this);
      pre_hot_ = hot_;
      /* FALLTHROUGH */
    case FL_DRAG:
    case FL_RELEASE:
      if (Fl::event_inside(this)) {
        hot_ = !pre_hot_;
      } else {
        hot_ = pre_hot_;
      }
      if ((e == FL_RELEASE) && pre_hot_ && !hot_)
        do_end_hot_callback();
      redraw();
      handle_default_button_ = false;
      return 1;
    case FL_UNFOCUS:
      if (hot_) do_end_hot_callback();
      hot_ = false;
      handle_default_button_ = false;
      /* FALLTHROUGH */
    case FL_FOCUS:
      redraw();
      return 1;
    case FL_KEYBOARD:
      if (hot_) {
        // Note: we can't really handle non-Latin shortcuts in the Fl_Shortcut
        //       type, so we don't handle them here either
        // Todo: use fl_utf_tolower and fl_utf_toupper
        int v = fl_utf8decode(Fl::event_text(), 0, 0);
        if (alt_modifier_extra_handler && Fl::event_state(FL_ALT)) {
          // MacOS returns special characters when the alt modifier is held down.
          // FLTK handles shortcuts as ASCII keys, so let's convert the keystroke.
          int c = Fl::event_key();
          if ( (c>32) && (c<128) && (isalnum(c)) ) {
            v = c;
            if (Fl::event_state(FL_SHIFT)) {
              v = toupper(c);
            }
          }
        }
        if ( (v > 32 && v < 0x7f) || (v > 0xa0 && v <= 0xff) ) {
          if (isupper(v)) {
            v = tolower(v);
            v |= FL_SHIFT;
          }
          v = v | (Fl::event_state()&(FL_META|FL_ALT|FL_CTRL));
        } else {
          v = (Fl::event_state() & (FL_META|FL_ALT|FL_CTRL|FL_SHIFT)) | Fl::event_key();
          if (v == FL_Escape) {
            if (shortcut_value == FL_Escape) {
              v = pre_esc_;
              do_end_hot_callback();
              hot_ = false;
            } else {
              pre_esc_ = shortcut_value;
            }
          }
          if ((v == FL_BackSpace) && shortcut_value) {
            v = 0;
          }
        }
        if (v != (int)shortcut_value) {
          shortcut_value = v;
          set_changed();
          redraw();
          if (when() & FL_WHEN_CHANGED) do_callback(FL_REASON_CHANGED);
          clear_changed();
        }
        return 1;
      } else {
        if ((Fl::event_key() == FL_Enter) || (strcmp(Fl::event_text(), " ") == 0)) {
          hot_ = true;
          redraw();
          return 1;
        }
      }
      break;
    case FL_SHORTCUT:
      if (hot_) return 1;
      break;
  }
  return Fl_Button::handle(e);
}

