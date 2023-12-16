//
// Common dialog header file for the Fast Light Tool Kit (FLTK).
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

#ifndef _src_Fl_Message_h_
#define _src_Fl_Message_h_

#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>

class Fl_Button;
class Fl_Input;

/**
  \cond DriverDev

  \addtogroup DriverDeveloper

  \{
*/

/**
  Fl_Message_Box is a tiny class that features copying the message text.

  This class is used in Fl_Message and handles ctrl-c or command-c
  (on macOS) to copy the given message text to the clipboard.
*/

/* Note: Do not FL_EXPORT this class, it's for internal use only */

class Fl_Message_Box : public Fl_Box {
public:
  Fl_Message_Box(int X, int Y, int W, int H)
    : Fl_Box(X, Y, W, H) {}
  int handle(int e) FL_OVERRIDE;
}; // class Fl_Message_Box

/**
  This is the base class for all common FLTK dialog windows used in
  fl_message(), fl_ask(), fl_choice(), fl_input(), and fl_password().

  \note <b>Internal use only. This class may be changed as required
    without notice.</b>\n

  This class is reserved for FLTK's internal usage in common dialogs
  in FLTK 1.4.x and later. The header file is "hidden" in the src/
  folder and not installed with the public header files.

  This class uses some static variables used to let the user code change
  the behavior and look of the \b next call of some of the message related
  functions. This is necessary to support the existing fl_message() and
  similar functions and is safe as long as the variables are reset before
  the function pops up (shows) the message window and enters the internal
  (i.e. nested) event loop.

  \since 1.4.0
*/

/* Note: Do not FL_EXPORT this class, it's for internal use only */

class Fl_Message {

  // static variables and methods

private:
  static Fl_Box *message_icon_; // returned by Fl_Message::message_icon()

  static const char *message_title_;
  static const char *message_title_default_;

  // icon label for next dialog (STR #2762)
  static const char *message_icon_label_;

  // Note: since Fl_Message objects are destroyed before fl_input()
  // and fl_password() return their input text, we *need* to store
  // the text in an internal (static) buffer. :-(

  static char *input_buffer_; // points to the allocated text buffer
  static int input_size_;     // size of allocated text buffer

  // the callback for all buttons:
  static void button_cb_(Fl_Widget *w, void *d);

  // the window callback:
  static void window_cb_(Fl_Widget *w, void *d);

  // resize to make text and buttons fit
  void resizeform();

public:
  static Fl_Box *message_icon();
  static void message_title(const char *title);
  static void message_title_default(const char *title);
  static void icon_label(const char *str);

  /** Implements fl_message_position(const int, const int y, const int center). */
  static void message_position(const int x, const int y, const int center) {
    form_x_ = x;
    form_y_ = y;
    form_position_ = center ? 2 : 1;
  }

  /** Implements fl_message_position(Fl_Widget *widget). */
  static void message_position(Fl_Widget *widget) {
    int xo, yo;
    Fl_Window *win = widget->top_window_offset(xo, yo);
    form_x_ = xo + widget->w() / 2;
    form_y_ = yo + widget->h() / 2;
    if (win) {
      form_x_ += win->x();
      form_y_ += win->y();
    }
    form_position_ = 2;
  }

  /** Implements fl_message_position(int *x, int *y). */
  static int message_position(int *x, int *y) {
    if (x)
      *x = form_position_ ? form_x_ : -1;
    if (y)
      *y = form_position_ ? form_y_ : -1;
    return form_position_;
  }

  /** Implements void fl_message_hotspot(int). */
  static void message_hotspot(int enable) { enable_hotspot_ = enable ? 1 : 0; }

  /** Implements int fl_message_hotspot(). */
  static int message_hotspot() { return enable_hotspot_; }

  int window_closed() const {
    return window_closed_;
  }

  // member variables and methods

private:
  Fl_Window *window_;         ///< message window
  Fl_Message_Box *message_;   ///< message text (handles ctrl-c)
  Fl_Box *icon_;              ///< contains the icon
  Fl_Button *button_[3];      ///< buttons used internally
  Fl_Input *input_;           ///< normal text or secret input
  int retval_;                ///< internally used to store the return value
  int window_closed_;         ///< window close flag (-1 = Escape, -2 = close button)

  // static (private) variables

  static int enable_hotspot_; ///< follow the mouse pointer (hotspot)
  static int form_x_;         ///< x position for next dialog
  static int form_y_;         ///< y position for next dialog
  static int form_position_;  ///< 0 = not set (may be hotspot), 1 = absolute, 2 = centered

public:
  // Constructor
  Fl_Message(const char *iconlabel);
  /** Destructor. */
  ~Fl_Message() { delete window_; }

  int innards(const char *fmt, va_list ap, const char *b0, const char *b1, const char *b2);

  const char *input_innards(const char *fmt, va_list ap, const char *defstr, uchar type, int maxchar = -1, bool str = false);
};

/**
  \}
  \endcond
*/

#endif // _src_Fl_Message_h_
