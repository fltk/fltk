//
// Common dialog implementation for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
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

/**
  \cond DriverDev
  \addtogroup DriverDeveloper
  \{
*/

/**
  \file Fl_Message.cxx - Base class for common dialogs.

  This is the base class for all common FLTK dialog windows used in
  fl_message(), fl_ask(), fl_choice(), fl_input(), and fl_password().

  \note <b>Internal use only. This class may be changed as required
    without notice.</b>\n
    This class is reserved for FLTK's internal usage in common dialogs
    in FLTK 1.4.x and later. The header file is "hidden" in the src/
    folder and not installed with the public header files.

  \since 1.4.0

  All common dialogs can be altered by changing the contents of some
  static class variables. This is done by accessor methods like
  fl_message_title() and others defined in FL/fl_ask.H. This is for
  backwards compatibility with FLTK 1.3.x and earlier.

  \note The documentation is only visible if the CMake option
    \c 'FLTK_INCLUDE_DRIVER_DOCS' is enabled.
*/

#include <FL/Fl.H>
#include "flstring.h"
#include <FL/fl_ask.H>
#include "Fl_Message.h"     // intentionally "hidden" in src/...
#include "FL/fl_string_functions.h"   // fl_strdup()

#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/fl_draw.H>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

// Fl_Message static variables

const char *Fl_Message::message_title_default_;
const char *Fl_Message::message_title_;

const char *Fl_Message::message_icon_label_;

Fl_Box *Fl_Message::message_icon_;

char *Fl_Message::input_buffer_;
int Fl_Message::input_size_;

int Fl_Message::enable_hotspot_ = 1;
int Fl_Message::form_x_ = 0;
int Fl_Message::form_y_ = 0;
int Fl_Message::form_position_ = 0; // 0 = not set, 1 = absolute, 2 = centered

/**
  Fl_Message's internal button callback.

  This callback function is used when the user pushes a button, presses the
  \c Return key, or uses a valid shortcut for one of the buttons.

  The internal return value \p retval_ is set according to the button that
  was invoked (0, 1, 2) and \p window_closed_ is set to 0 (dialog closed
  by dialog button). Then the dialog window is closed (hidden).
*/

void Fl_Message::button_cb_(Fl_Widget *w, void *d) {
  Fl_Window *window = w->window();
  Fl_Message *dialog = (Fl_Message *)window->user_data();
  dialog->window_closed_ = 0;
  dialog->retval_ = fl_int(d); // button
  window->hide();
} // button_cb_()

/**
  Fl_Message's internal window callback.

  This callback function is used internally when the dialog window is closed
  by the user with the window close button or by pressing \c Escape.

  The internal return value \p retval_ is set to 0 (i.e. button 0) and
  \p window_closed_ is set to -1 (Escape) or -2 (window close button).
  Then the dialog window is closed (hidden).
*/

void Fl_Message::window_cb_(Fl_Widget *w, void *d) {
  Fl_Window *window = (Fl_Window *)w;
  Fl_Message *dialog = (Fl_Message *)window->user_data();
  if ((Fl::event() == FL_KEYBOARD || Fl::event() == FL_SHORTCUT) &&
      (Fl::event_key() == FL_Escape))
    dialog->window_closed_ = -1;
  else
    dialog->window_closed_ = -2;
  dialog->retval_ = 0; // either window or button 0
  window->hide();
} // window_cb_()

/**
  Fl_Message constructor.

  The constructor creates a default message window and sets the icon type
  to the given \p iconlabel which can be any character (or string).

  If fl_message_icon_label() has been called before this label is used
  instead and reset to NULL after the message window has been created.

  Message text box (Fl_Box), icon (Fl_Box), and an input (Fl_Input) widgets
  are created and initialized. Three buttons are created and arranged right to
  left in the message window. The second (middle) button is an Fl_Return_Button.

  The message window is set to modal()
*/
Fl_Message::Fl_Message(const char *iconlabel)
  : window_(0)
  , retval_(0)
  , window_closed_(0) {

  // Make sure that the dialog does not become the child of some current group.

  Fl_Group *previous_group = Fl_Group::current();
  if (previous_group)
    Fl_Group::current(0);

  // create widgets

  window_ = new Fl_Window(400, 150, NULL);
  message_ = new Fl_Message_Box(60, 25, 340, 20);
  message_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);

  input_ = new Fl_Input(60, 37, 340, 23);
  input_->hide();

  Fl_Box *icon_template = message_icon(); // use template for icon box

  icon_ = new Fl_Box(10, 10, 50, 50);
  icon_->box(icon_template->box());
  icon_->labelfont(icon_template->labelfont());
  icon_->labelsize(icon_template->labelsize());
  icon_->color(icon_template->color());
  icon_->labelcolor(icon_template->labelcolor());
  icon_->image(icon_template->image());
  icon_->align(icon_template->align());

  if (message_icon_label_) {                // fl_message_icon_label() has been called
    icon_->copy_label(message_icon_label_);
    message_icon_label_ = 0;
  } else if (icon_template->label()) {      // sticky icon template label() has been set
    icon_->copy_label(icon_template->label());
  } else {                                  // default string (c'tor argument)
    icon_->label(iconlabel);
  }

  window_->end(); // don't add the buttons automatically

  // create the buttons (positions: right to left)
  // button 1 is a return button

  for (int b = 0, x = 310; b < 3; b++, x -= 100) {
    if (b == 1) {
      button_[b] = new Fl_Return_Button(x, 70, 90, 23);
    } else {
      button_[b] = new Fl_Button(x, 70, 90, 23);
    }
    button_[b]->align(FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    button_[b]->callback(button_cb_, fl_voidptr(b));
  }

  // add the buttons left to right for tab navigation

  for (int b = 2; b >= 0; b--) {
    window_->add(button_[b]);
  }

  window_->begin();
  window_->resizable(new Fl_Box(60, 10, 110 - 60, 27));
  window_->end();
  window_->callback(window_cb_, this);
  window_->set_modal();

  // restore previous group
  Fl_Group::current(previous_group);
}

/**
  Resizes the form and widgets so that they hold everything
  that is asked of them.
*/
void Fl_Message::resizeform() {
  int i;
  int message_w, message_h;
  int text_height;
  int button_w[3], button_h[3];
  int x, w, h, max_w, max_h;
  const int icon_size = 50;

  fl_font(message_->labelfont(), message_->labelsize());
  message_w = message_h = 0;
  fl_measure(message_->label(), message_w, message_h);

  message_w += 10;
  message_h += 10;
  if (message_w < 340)
    message_w = 340;
  if (message_h < 30)
    message_h = 30;

  fl_font(button_[0]->labelfont(), button_[0]->labelsize());

  memset(button_w, 0, sizeof(button_w));
  memset(button_h, 0, sizeof(button_h));

  for (max_h = 25, i = 0; i < 3; i++)
    if (button_[i]->visible()) {
      fl_measure(button_[i]->label(), button_w[i], button_h[i]);

      if (i == 1)
        button_w[1] += 20; // account for return button arrow

      button_w[i] += 30;
      button_h[i] += 10;

      if (button_h[i] > max_h)
        max_h = button_h[i];
    }

  if (input_->visible())
    text_height = message_h + 25;
  else
    text_height = message_h;

  max_w = message_w + 10 + icon_size;
  w = button_w[0] + button_w[1] + button_w[2] - 10;

  if (w > max_w)
    max_w = w;

  // if the button horizontally overlap the icon, make sure that they are drawn
  // below to icon by making the text part at least as tall as the icon.
  if (w > message_w && text_height < icon_size) {
    int pad_h = icon_size-text_height;
    message_h += pad_h;
    text_height += pad_h;
  }

  message_w = max_w - 10 - icon_size;

  w = max_w + 20;
  h = max_h + 30 + text_height;

  window_->size(w, h);
  window_->size_range(w, h, w, h);

  message_->resize(20 + icon_size, 10, message_w, message_h);
  icon_->resize(10, 10, icon_size, icon_size);
  icon_->labelsize(icon_size - 10);
  input_->resize(20 + icon_size, 10 + message_h, message_w, 25);

  for (x = w, i = 0; i < 3; i++) {
    if (button_w[i]) {
      x -= button_w[i];
      button_[i]->resize(x, h - 10 - max_h, button_w[i] - 10, max_h);
    }
  }
  window_->init_sizes();
}

/**
  Does all Fl_Message window internals for messages with and w/o an input field.

  This method finalizes the layout of the message window, arranges (shows or hides)
  the buttons and the optional text input widget, sets the message window title,
  pops up the window, and waits for user input.

  The message window is positioned according to the positioning options set before
  calling the message function.

  The private variables \p retval_ and \p window_closed_ are set depending
  on the user action (pushing a button or closing the window).

  \note The above mentioned variables must be evaluated before the event
    loop is called because they might be overwritten by another dialog.
    It is safe to evaluate the variables before the message function returns.

  \param[in] fmt  printf style format used in the user function call
  \param[in] ap   argument list provided by the user function call
  \param[in] b0   text of button 0 (right: usually the "cancel" button)
  \param[in] b1   text of button 1 (middle)
  \param[in] b2   text of button 2 (left)

  \returns    return code
  \retval  0  for Escape, window close, or button 0
  \retval  1  button 1 was pushed
  \retval  2  button 2 was pushed
*/

int Fl_Message::innards(const char *fmt, va_list ap, const char *b0, const char *b1, const char *b2) {
  Fl::pushed(0); // stop dragging (STR #2159)

  char buffer[1024];
  if (!strcmp(fmt, "%s")) {
    message_->label(va_arg(ap, const char *));
  } else {
    ::vsnprintf(buffer, sizeof(buffer) - 1, fmt, ap);
    message_->label(buffer);
  }

  message_->labelfont(fl_message_font_);
  if (fl_message_size_ == -1)
    message_->labelsize(FL_NORMAL_SIZE);
  else
    message_->labelsize(fl_message_size_);

  if (b0) {
    button_[0]->show();
    button_[0]->label(b0);
    button_[1]->position(210, 70);
  } else {
    button_[0]->hide();
    button_[1]->position(310, 70);
  }
  if (b1) {
    button_[1]->show();
    button_[1]->label(b1);
  } else
    button_[1]->hide();
  if (b2) {
    button_[2]->show();
    button_[2]->label(b2);
  } else
    button_[2]->hide();

  resizeform();

  if (button_[1]->visible() && !input_->visible())
    button_[1]->take_focus();

  if (form_position_) {
    if (form_position_ == 2) { // centered
      form_x_ -= window_->w() / 2;
      form_y_ -= window_->h() / 2;
    }
    window_->position(form_x_, form_y_);
    form_x_ = form_y_ = form_position_ = 0;
  } else if (enable_hotspot_)
    window_->hotspot(b0 ? button_[0] : button_[1]);
  else
    window_->free_position();

  if (b0 && Fl_Widget::label_shortcut(b0))
    button_[0]->shortcut(0);

  // set the one-time window title, if defined and a specific title is not set
  if (!window_->label() && message_title_) {
    window_->copy_label(message_title_);
    message_title(0); // reset global message title (compat.)
  }

  // set default window title, if defined and a specific title is not set
  if (!window_->label() && message_title_default_)
    window_->copy_label(message_title_default_);

  // deactivate Fl::grab() because it is incompatible with modal windows
  Fl_Window *g = Fl::grab();
  if (g)
    Fl::grab(0);
  Fl_Group *current_group = Fl_Group::current(); // make sure the dialog does not interfere with any active group
  Fl_Group::current(0);
  window_->show();
  Fl_Group::current(current_group);
  while (window_->shown())
    Fl::wait();
  if (g) // regrab the previous popup menu, if there was one
    Fl::grab(g);

  return retval_;
}

/**
  Gets the default icon container (Fl_Box) used in common dialogs.

  Many common dialogs like fl_message(), fl_alert(), fl_ask(),
  fl_choice(), fl_input(), and fl_password() display an icon.

  You can use this method to get the icon box (Fl_Box) and modify
  the icon's box type, font, fontsize etc.

  \since FLTK 1.4.0

  \note This method replaces the deprecated method fl_message_icon().
  Note that this new method correctly returns an Fl_Box * pointer,
  whereas fl_message_icon() returns Fl_Widget *.

  The current icon default values are:

    - width and height: 50 (currently not changeable)
    - box(FL_THIN_UP_BOX)
    - labelfont(FL_TIMES_BOLD)
    - labelsize(34)
    - color(FL_WHITE)
    - labelcolor(FL_BLUE)

  These values may be changed in a future FLTK version (although this
  is not very likely to happen).

  If you change this then the changed values are used for subsequently
  started common dialogs. This is intended to change the icon layout
  for your application at startup time. It is not recommended to change
  the default values for each message.
*/
Fl_Box *Fl_Message::message_icon() {

  if (!Fl_Message::message_icon_) { // not yet initialized

    Fl_Group *current_group = Fl_Group::current();
    Fl_Group::current(0);

    Fl_Box *o = Fl_Message::message_icon_ = new Fl_Box(10, 10, 50, 50);
    o->box(FL_THIN_UP_BOX);
    o->labelfont(FL_TIMES_BOLD);
    o->labelsize(34);
    o->color(FL_WHITE);
    o->labelcolor(FL_BLUE);
    Fl_Group::current(current_group);
  }
  return Fl_Message::message_icon_;
}

/**
  Does all Fl_Message window internals for messages with a text input field.

  \param[in] fmt      printf style format used in the user function call
  \param[in] ap       argument list provided by the user function call
  \param[in] defstr   default string given by the user
  \param[in] type     either FL_NORMAL_INPUT or FL_SECRET_INPUT (password)
  \param[in] maxchar  max. number of allowed characters (not bytes)
  \param[in] str      true: return type is string, false: internal buffer

  \returns    pointer to string or NULL if cancel or escape were hit

  \see innards()
*/
const char *Fl_Message::input_innards(const char *fmt, va_list ap, const char *defstr, uchar type, int maxchar, bool str) {
  message_->position(60, 10);
  input_->type(type);
  input_->show();
  input_->value(defstr);
  input_->take_focus();
  if (maxchar > 0)
    input_->maximum_size(maxchar);

  int r = innards(fmt, ap, fl_cancel, fl_ok, 0);

  if (!r)
    return 0;

  if (input_->value()) {

    int size = input_->size() + 1;

    if (!str) { // need to store the value in pre-allocated buffer

      // The allocated input buffer starts with size 0 and is allocated
      // in multiples of 128 bytes >= size. If both the size and the pointer
      // are 0 (NULL) then realloc() allocates a /new/ buffer.

      if (size > input_size_) {
        size += 127;
        size &= ~127;
        input_buffer_ = (char *)realloc(input_buffer_, size);
        input_size_ = size;
      }

      // Store the input. Note that value() can contain null bytes,
      // so we use memcpy and add a terminating null byte as well.

      memcpy(input_buffer_, input_->value(), input_->size());
      input_buffer_[input_->size()] = '\0';
      return (input_buffer_);

    } else { // string version: return value() which will be copied

      return input_->value();
    }

  } else
    return 0;
}

/** Sets the title of the dialog window used in many common dialogs.

  This window \p title will be used in the next call of one of the
  common dialogs like fl_message(), fl_alert(), fl_ask(), fl_choice(),
  fl_input(), fl_password().

  The \p title string is copied internally, so that you can use a
  local variable or free the string immediately after this call. It
  applies only to the \b next call of one of the common dialogs and
  will be reset to an empty title (the default for all dialogs) after
  that call.

  \param[in] title      window label, string copied internally
*/
void Fl_Message::message_title(const char *title) {
  if (message_title_) {
    free((void *)message_title_);
    message_title_ = 0;
  }
  if (title)
    message_title_ = fl_strdup(title);
}

/** Sets the default title of the dialog window used in many common dialogs.

  This window \p title will be used in all subsequent calls of one of the
  common dialogs like fl_message(), fl_alert(), fl_ask(), fl_choice(),
  fl_input(), fl_password(), unless a specific title has been set
  with fl_message_title(const char *title).

  The default is no title. You can override the default title for a
  single dialog with fl_message_title(const char *title).

  The \p title string is copied internally, so that you can use a
  local variable or free the string immediately after this call.

  \param[in] title      default window label, string copied internally
*/
void Fl_Message::message_title_default(const char *title) {
  if (message_title_default_) {
    free((void *)message_title_default_);
    message_title_default_ = 0;
  }
  if (title)
    message_title_default_ = fl_strdup(title);
}

void Fl_Message::icon_label(const char *str) {
  message_icon_label_ = str;
}

// handle ctrl-c (command-c on macOS) to copy message text

int Fl_Message_Box::handle(int e) {
  int mods = Fl::event_state() & (FL_META|FL_CTRL|FL_ALT);
  switch (e) {
    case FL_KEYBOARD:
    case FL_SHORTCUT:
      if (Fl::event_key() == 'c' && mods == FL_COMMAND) {
        Fl::copy(label(), int(strlen(label())), 1);
        return 1;
      }
      break;
    default:
      break;
  }
  return Fl_Box::handle(e);
}

/**
  \}
  \endcond
*/
