//
// Standard dialog functions for the Fast Light Tool Kit (FLTK).
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
  \addtogroup group_comdlg
  @{
*/

/**
  \file fl_ask.cxx

  \brief Utility functions for common dialogs.

  This file defines the functions

  - fl_alert()
  - fl_beep()
  - fl_message()
  - fl_ask()
  - fl_choice()
  - fl_input()
  - fl_input_str()
  - fl_password()
  - fl_password_str()

  and some more functions to change their behavior (positioning,
  window title, and more).

  Since FLTK 1.4.0 a big part of these functions is
    implemented in class Fl_Message.
*/

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input_.H>
#include "flstring.h"
#include "Fl_Screen_Driver.H"
#include <FL/fl_ask.H>
#include "Fl_Message.h" // intentionally "hidden" in src/...

#include <stdio.h>
#include <stdarg.h>

// static, configurable variables

Fl_Font fl_message_font_ = FL_HELVETICA;
Fl_Fontsize fl_message_size_ = -1;

// pointers you can use to change FLTK to another language:
const char *fl_no = "No";         ///< string pointer used in common dialogs, you can change it to another language
const char *fl_yes = "Yes";       ///< string pointer used in common dialogs, you can change it to another language
const char *fl_ok = "OK";         ///< string pointer used in common dialogs, you can change it to another language
const char *fl_cancel = "Cancel"; ///< string pointer used in common dialogs, you can change it to another language
const char *fl_close = "Close";   ///< string pointer used in common dialogs, you can change it to another language

// fltk functions:

/**
  Emits a system beep.

  This function is platform specific. Depending on the input \p type a different
  sound may be played or the system speaker may beep with a different volume.

  On X the system speaker is used which may not work at all on newer systems
  that don't have a speaker. Since 1.4.0 \c FL_BEEP_DEFAULT and other types
  honor the system or user settings whereas \c FL_BEEP_ERROR uses 100% volume.
  This may be changed in a future version.

  On Wayland an ASCII \p BEL (0x07) is output to stderr.

  On Windows the \c MessageBeep() function is used to play different sounds
  depending on the \p type argument.

  On macOS the system beep function \c NSBeep() is used for \c FL_BEEP_DEFAULT
  and \c FL_BEEP_ERROR. Other types are ignored.

  On other platforms the behavior is undefined and may change in the future.

  \param[in] type The beep type from the \ref Fl_Beep enumeration (optional)

  \code #include <FL/fl_ask.H> \endcode
*/
void fl_beep(int type) {
  Fl::screen_driver()->beep(type);
}

/** Shows an information message dialog box.

  \code #include <FL/fl_ask.H> \endcode

  \param[in] fmt can be used as an sprintf-like format and variables for the message text
*/
void fl_message(const char *fmt, ...) {

  Fl_Message msg("i");
  va_list ap;

  va_start(ap, fmt);
  msg.innards(fmt, ap, 0, fl_close, 0);
  va_end(ap);
}

/** Shows an alert message dialog box.

   \code #include <FL/fl_ask.H> \endcode

   \param[in] fmt can be used as an sprintf-like format and variables for the message text
*/
void fl_alert(const char *fmt, ...) {

  Fl_Message msg("!");
  va_list ap;

  va_start(ap, fmt);
  msg.innards(fmt, ap, 0, fl_close, 0);
  va_end(ap);
}

/** Shows a dialog displaying the \p fmt message,
    this dialog features 2 yes/no buttons.

   \code #include <FL/fl_ask.H> \endcode

   \param[in] fmt can be used as an sprintf-like format and variables for the message text
   \retval 0 if the no button is selected
   \retval 1 if yes is selected

   \deprecated fl_ask() is deprecated since it uses "Yes" and "No" for the buttons which
               does not conform to the current FLTK Human Interface Guidelines.
               Use fl_choice() with the appropriate verbs instead.
*/
int fl_ask(const char *fmt, ...) {

  Fl_Message msg("?");
  va_list ap;

  va_start(ap, fmt);
  int r = msg.innards(fmt, ap, fl_no, fl_yes, 0);
  va_end(ap);

  return r;
}

/** Shows a dialog displaying the printf style \p fmt message.

  This dialog features up to 3 customizable choice buttons
  which are specified in order of *right-to-left* in the dialog, e.g.
  \image html  fl_choice_left_middle_right.png
  \image latex fl_choice_left_middle_right.png  "fl_choice() button ordering" width=4cm

  \code #include <FL/fl_ask.H> \endcode

  Three choices with printf() style formatting:
  \image html  fl_choice_three_fmt.png
  \image latex fl_choice_three_fmt.png  "fl_choice() three choices with printf formatting" width=4cm
  \code
    int num_msgs = GetNumberOfMessages();
    switch ( fl_choice("What to do with %d messages?", "Send", "Save", "Delete", num_msgs) ) {
      case 0: .. // Send
      case 1: .. // Save (default)
      case 2: .. // Delete
      ..
    }
  \endcode

  Three choice example:
  \image html  fl_choice_three.png
  \image latex fl_choice_three.png  "fl_choice() three choices" width=4cm
  \code
  switch ( fl_choice("How many bedrooms?", "Zero", "One", "Two") ) {
    case 0: .. // "Zero"
    case 1: .. // "One" (default)
    case 2: .. // "Two"
  }
  \endcode

  Two choice example:
  \image html  fl_choice_two.png
  \image latex fl_choice_two.png  "fl_choice() two choices" width=4cm
  \code
    switch ( fl_choice("Empty trash?", "Yes", "No", 0) ) {
      case 0: .. // Yes
      case 1: .. // No (default)
    }
  \endcode

  One choice example:
  \image html  fl_choice_one.png
  \image latex fl_choice_one.png  "fl_choice() one choice" width=4cm
  \code
    fl_choice("All hope is lost.", "OK", 0, 0);   // "OK" default
  \endcode

  \param[in] fmt can be used as an sprintf-like format and variables for the message text
  \param[in] b0 text label for right button 0
  \param[in] b1 text label for middle button 1 (can be 0)
  \param[in] b2 text label for left button 2 (can be 0)
  \retval 0 if the button with \p b0 text is pushed or the user pressed
      the \c Escape key or clicked the window close button
  \retval 1 if the button with \p b1 text is pushed or the user pressed
      the \c Return key
  \retval 2 if the button with \p b2 text is pushed
*/
int fl_choice(const char *fmt, const char *b0, const char *b1, const char *b2, ...) {

  Fl_Message msg("?");
  va_list ap;

  va_start(ap, b2);
  int r = msg.innards(fmt, ap, b0, b1, b2);
  va_end(ap);
  return r;
}

/** Shows a dialog displaying the printf style \p fmt message.

  This function is like fl_choice() but returns \c -1 if the dialog window
  was closed by pressing the \c Escape key or the window close button
  rather than pushing one of the dialog buttons.

  \see fl_choice()

  \param[in] fmt can be used as an sprintf-like format and variables for the message text
  \param[in] b0 text label for right button 0
  \param[in] b1 text label for middle button 1 (can be 0)
  \param[in] b2 text label for left button 2 (can be 0)

  \retval -3 reserved, FLTK 1.3 only: another dialog is still open (not possible in 1.4)
  \retval -2 if the dialog was closed by pushing the window close button
  \retval -1 if the dialog was closed by hitting Escape
  \retval  0 if the button with \p b0 text is pushed
  \retval  1 if the button with \p b1 text is pushed
  \retval  2 if the button with \p b2 text is pushed
*/
int fl_choice_n(const char *fmt, const char *b0, const char *b1, const char *b2, ...) {

  Fl_Message msg("?");
  va_list ap;

  va_start(ap, b2);
  int r = msg.innards(fmt, ap, b0, b1, b2);
  va_end(ap);
  if (msg.window_closed() != 0)
    return msg.window_closed();
  return r;
}

/**
  Gets the Fl_Box icon container of the current default dialog used in
  many common dialogs like fl_message(), fl_alert(),
  fl_ask(), fl_choice(), fl_input(), fl_password().

  The return value cannot be Null. The object pointed to is an Fl_Box widget.
  The returned pointer (Fl_Widget *) can be safely cast to an Fl_Box* pointer.

  \note You can set some attributes of this \b default icon box. These attributes
    are sticky, i.e. they will be used in all subsequent common dialogs unless
    overridden by specific "one shot" variables. Setting any attribute except
    those mentioned below causes undefined behavior.

  Supported icon attributes:

    - box()
    - labelfont()
    - labelsize()
    - color()
    - labelcolor()
    - image()
    - align()

  The icon size can not be changed. If you set an image() you should scale it
  to the available size, i.e. \p w() and \p h() of the icon box.

  \code #include <FL/fl_ask.H> \endcode
*/
Fl_Widget *fl_message_icon() {
  return Fl_Message::message_icon();
}

/** Shows an input dialog displaying the \p fmt message with variable arguments.

  Returns the string in an internally allocated buffer that may be changed later.
  You \b must copy the string immediately after return from this method - at least
  before the next execution of the event loop.

  \code #include <FL/fl_ask.H> \endcode

  \param[in] fmt can be used as an sprintf-like format and variables for the message text
  \param[in] defstr defines the default returned string if no text is entered

  \return the user string input if OK was pushed
  \retval NULL if Cancel was pushed or the window was closed by the user
*/
const char *fl_input(const char *fmt, const char *defstr, ...) {

  Fl_Message msg("?");
  va_list ap;
  va_start(ap, defstr);
  const char *r = msg.input_innards(fmt, ap, defstr, FL_NORMAL_INPUT, 0, false);
  va_end(ap);
  return r;
}


/** Shows an input dialog displaying the \p fmt message with variable arguments.

  This is the same as const char *fl_input(const char *fmt, const char *defstr, ...)
  except that it has an additional parameter to limit the number of characters
  the user can input.

  Returns the string in an internally allocated buffer that may be changed later.
  You \b must copy the string immediately after return from this method - at least
  before the next execution of the event loop.

  \code #include <FL/fl_ask.H> \endcode

  \param[in] maxchar maximum number of characters the user can input (UTF-8 aware)
  \param[in] fmt can be used as an sprintf-like format and variables for the message text
  \param[in] defstr defines the default returned string if no text is entered

  \return the user string input if OK was pushed
  \retval NULL if Cancel was pushed or the window was closed by the user
*/
const char *fl_input(int maxchar, const char *fmt, const char *defstr, ...) {

  Fl_Message msg("?");
  if (maxchar < 0) maxchar = 0;
  va_list ap;
  va_start(ap, defstr);
  const char *r = msg.input_innards(fmt, ap, defstr, FL_NORMAL_INPUT, maxchar, false);
  va_end(ap);
  return r;
}



#if (FLTK_USE_STD)

/** Shows an input dialog displaying the \p fmt message with variable arguments.

  Like fl_input(), but this method has the additional argument \p maxchar
  that limits the number of \b characters that can be input. Since the
  string is encoded in UTF-8 it is possible that the number of bytes
  in the string is larger than \p maxchar.

  Other than the deprecated fl_input() method w/o the \p maxchar argument, this one
  returns the string in an std::string object that must be released after use. This
  can be a local/automatic variable.

  The \p ret variable is set to 0 if the user clicked OK, and to a negative
  value if the user canceled the dialog. If the dialog was canceled, the returned
  string will be empty.

  \code #include <FL/fl_ask.H> \endcode

  Example:
  \code
    { int ret;
      std::string str = fl_input_str(ret, 0, "Enter text:", "");
      if (ret < 0)
        printf("Text input was canceled.\n");
      else
        printf("Text is: '%s'\n", str.c_str());
    } // (str goes out of scope)
  \endcode

  \param[out] ret    0 if user clicked OK, negative if dialog was canceled
  \param[in] maxchar input size limit in characters (not bytes), use 0 for no limit
  \param[in] fmt     can be used as an sprintf-like format and variables for the message text
  \param[in] defstr  defines the default returned string if no text is entered

  \return the user string input if OK was clicked which can be empty
  \return an empty string and set \p ret to a negative value if the user canceled the dialog

  \since 1.4.0
*/
std::string fl_input_str(int &ret, int maxchar, const char *fmt, const char *defstr, ...) {
  Fl_Message msg("?");
  if (maxchar < 0) maxchar = 0;
  va_list ap;
  va_start(ap, defstr);
  const char *r = msg.input_innards(fmt, ap, defstr, FL_NORMAL_INPUT, maxchar, true);
  va_end(ap);
  ret = (r == NULL) ? -1 : 0;
  return (r == NULL) ? std::string("") : std::string(r);
}

/** Shows an input dialog displaying the \p fmt message with variable arguments.
 \note No information is given if the user canceled the dialog or clicked OK.
 \see fl_input_str(int &ret, int maxchar, const char *label, const char *deflt = 0, ...)
 */
std::string fl_input_str(int maxchar, const char *fmt, const char *defstr, ...) {
  Fl_Message msg("?");
  if (maxchar < 0) maxchar = 0;
  va_list ap;
  va_start(ap, defstr);
  const char *r = msg.input_innards(fmt, ap, defstr, FL_NORMAL_INPUT, maxchar, true);
  va_end(ap);
  return (r == NULL) ? std::string("") : std::string(r);
}

#endif // FLTK_USE_STD

/** Shows an input dialog displaying the \p fmt message with variable arguments.

  Like fl_input() except the input text is not shown,
  '*' or similar replacement characters are displayed instead.

  \code #include <FL/fl_ask.H> \endcode

  \param[in] fmt can be used as an sprintf-like format and variables for the message text
  \param[in] defstr defines the default returned string if no text is entered

  \return the user string input if OK was pushed
  \retval NULL if Cancel was pushed or the window was closed by the user
*/
const char *fl_password(const char *fmt, const char *defstr, ...) {
  Fl_Message msg("?");
  va_list ap;
  va_start(ap, defstr);
  const char *r = msg.input_innards(fmt, ap, defstr, FL_SECRET_INPUT, 0, false);
  va_end(ap);
  return r;
}

/** Shows an input dialog displaying the \p fmt message with variable arguments.

  Like fl_input() except the input text is not shown,
  '*' or similar replacement characters are displayed instead.

  \code #include <FL/fl_ask.H> \endcode

  \param[in] maxchar  input lenght limit in chars, 0 = no limit
  \param[in] fmt can be used as an sprintf-like format and variables for the message text
  \param[in] defstr defines the default returned string if no text is entered

  \return the user string input if OK was pushed
  \retval NULL if Cancel was pushed or the window was closed by the user
*/
const char *fl_password(int maxchar, const char *fmt, const char *defstr, ...) {
  Fl_Message msg("?");
  if (maxchar < 0) maxchar = 0;
  va_list ap;
  va_start(ap, defstr);
  const char *r = msg.input_innards(fmt, ap, defstr, FL_SECRET_INPUT, maxchar, false);
  va_end(ap);
  return r;
}

#if (FLTK_USE_STD)

/** Shows an input dialog displaying the \p fmt message with variable arguments.

  Like fl_input_str() except the input text is not shown,
  '*' or similar replacement characters are displayed instead.

  Other than the fl_password() method w/o the \p maxchar argument, this one
  returns the string in an std::string object that must be released after use.
  This can be a local/automatic variable.

  For an example see fl_input_str()

  \code #include <FL/fl_ask.H> \endcode

  \param[out] ret    0 if user clicked OK, negative if dialog was canceled
  \param[in] maxchar input size limit in characters (not bytes), use 0 for no limit
  \param[in] fmt     can be used as an sprintf-like format and variables for the message text
  \param[in] defstr  defines the default returned string if no text is entered

  \return the user string input if OK was clicked which can be empty
  \return an empty string and set \p ret to a negative value if the user canceled the dialog

  \since 1.4.0
*/
std::string fl_password_str(int &ret, int maxchar, const char *fmt, const char *defstr, ...) {
  Fl_Message msg("?");
  if (maxchar < 0) maxchar = 0;
  va_list ap;
  va_start(ap, defstr);
  const char *r = msg.input_innards(fmt, ap, defstr, FL_SECRET_INPUT, maxchar, true);
  va_end(ap);
  ret = (r == NULL) ? -1 : 0;
  return (r == NULL) ? std::string("") : std::string(r);
}

/** Shows an input dialog displaying the \p fmt message with variable arguments.
 \note No information is given if the user canceled the dialog or clicked OK.
 \see fl_password_str(int &ret, int maxchar, const char *label, const char *deflt = 0, ...)
 */
std::string fl_password_str(int maxchar, const char *fmt, const char *defstr, ...) {
  Fl_Message msg("?");
  if (maxchar < 0) maxchar = 0;
  va_list ap;
  va_start(ap, defstr);
  const char *r = msg.input_innards(fmt, ap, defstr, FL_SECRET_INPUT, maxchar, true);
  va_end(ap);
  return (r == NULL) ? std::string("") : std::string(r);
}

#endif // FLTK_USE_STD


/** Sets the preferred position for the message box used in
  many common dialogs like fl_message(), fl_alert(),
  fl_ask(), fl_choice(), fl_input(), fl_password().

  The position set with this method overrides the hotspot setting,
  i.e. setting a position has higher priority than the hotspot mode
  set by fl_message_hotspot(int).

  The preferred position set by any of the fl_message_position() variants
  affects only the next call of one of the common dialogs. The preferred
  position is reset to 0 (unset) as soon as the dialog is shown.

  If the optional argument \p center is non-zero (true) the message box
  will be centered at the given coordinates rather than using the X/Y
  position as the window position (top left corner).

  \code #include <FL/fl_ask.H> \endcode

  \param[in] x        Preferred X position
  \param[in] y        Preferred Y position
  \param[in] center   1 = centered, 0 = absolute

  \see int fl_message_position(int *x, int *y)
*/
void fl_message_position(const int x, const int y, const int center) {
  Fl_Message::message_position(x, y, center);
}

/** Sets the preferred position for the message box used in
  many common dialogs like fl_message(), fl_alert(),
  fl_ask(), fl_choice(), fl_input(), fl_password().

  The message box will be centered over the given widget
  or window extensions.

  Everything else is like fl_message_position(int, int, int) with
  argument 'center' set to 1.

  \code #include <FL/fl_ask.H> \endcode

  \param[in] widget   Widget or window to position the message box over.

  \see int fl_message_position(int x, int y, int center)
*/
void fl_message_position(Fl_Widget *widget) {
  Fl_Message::message_position(widget);
}

/** Gets the preferred position for the message box used in
  many common dialogs like fl_message(), fl_alert(),
  fl_ask(), fl_choice(), fl_input(), fl_password().

  \code #include <FL/fl_ask.H> \endcode

  The position set with this method overrides the hotspot setting,
  i.e. setting a position has higher priority than the hotspot mode
  set by fl_message_hotspot(int).

  The preferred position set by any of the fl_message_position() variants
  affects only the next call of one of the common dialogs. The preferred
  position is reset to 0 (unset) as soon as the dialog is shown.

  \param[out] x  Preferred X position, returns -1 if not set
  \param[out] y  Preferred Y position, returns -1 if not set

  \returns   whether position is currently set or not
  \retval 0  position is not set (hotspot may be enabled or not)
  \retval 1  position is set (window position)
  \retval 2  position is set (message box centered)

  \see fl_message_hotspot()
  \see fl_message_hotspot(int)
  \see fl_message_position(int, int)
  \see fl_message_position(const int x, const int y, const int center)
  \see fl_message_position(Fl_Widget *)
*/
int fl_message_position(int *x, int *y) {
  return Fl_Message::message_position(x, y);
}

/** Sets whether or not to move the message box used in
  many common dialogs like fl_message(), fl_alert(),
  fl_ask(), fl_choice(), fl_input(), fl_password() to follow
  the mouse pointer.

  The default is \e enabled, so that the default button is the
  hotspot and appears at the mouse position.

  \code #include <FL/fl_ask.H> \endcode

  \param[in]  enable  non-zero enables hotspot behavior,
                      0 disables hotspot
*/
void fl_message_hotspot(int enable) {
  Fl_Message::message_hotspot(enable);
}

/** Gets whether or not to move the message box used in
  many common dialogs like fl_message(), fl_alert(),
  fl_ask(), fl_choice(), fl_input(), fl_password() to follow
  the mouse pointer.

  This is a permanent setting. It remains active and affects the window
  position unless overridden by an explicit positioning request by means
  of one of the fl_message_position() variants.

  \code #include <FL/fl_ask.H> \endcode

  \return  0 if disabled, non-zero otherwise

  \see void fl_message_hotspot(int)
  \see int fl_message_position(int *x, int *y)
  \see void fl_message_position(Fl_Widget *)
  \see fl_message_position()
*/
int fl_message_hotspot() {
  return Fl_Message::message_hotspot();
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

  \code #include <FL/fl_ask.H> \endcode
  \param[in] title    window label, string copied internally
*/
void fl_message_title(const char *title) {
  Fl_Message::message_title(title);
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

  \code #include <FL/fl_ask.H> \endcode

  \param[in] title default window label, string copied internally
*/
void fl_message_title_default(const char *title) {
  Fl_Message::message_title_default(title);
}

/** Sets the icon label of the dialog window used in many common dialogs.

  This icon label will be used in the next call of one of the
  common dialogs like fl_message(), fl_alert(), fl_ask(), fl_choice(),
  fl_input(), fl_password().

  The label \p str is stored internally as a reference, it must be
  in scope until the dialog function (e.g. fl_choice) is called.

  It applies only to the \b next call of one of the common dialogs and
  will be reset after that call so the next dialog will use its default
  label unless set again.

  \note This label string must be short, usually only one character so
    it fits in the icon box. You can use any valid UTF-8 character, e.g.
    the Euro sign ("€") which is three bytes in UTF-8 encoding.

  \code #include <FL/fl_ask.H> \endcode
  \param[in] str    icon label
*/
void fl_message_icon_label(const char *str) {
  Fl_Message::icon_label(str);
}

/** @} */
