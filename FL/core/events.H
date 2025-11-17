//
// Global event header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2025 by Bill Spitzak and others.
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

/** \file FL/core/events.H
  \brief Global event handling variables and functions.
*/

#ifndef Fl_core_events_H
#define Fl_core_events_H

#include <FL/fl_config.h>             // build configuration
#include <FL/Fl_Export.H>             // for FL_EXPORT
#include <FL/core/function_types.H>   // widget callbacks and services

class Fl_Widget;

namespace Fl {

//
// Event Variables (Internal)
// Note: These variables should be private, but would harm back compatibility.
//

#ifndef FL_DOXYGEN

// Core event information
FL_EXPORT extern int                e_number;           ///< Current event type
FL_EXPORT extern int                e_state;            ///< Keyboard/mouse button states

// Mouse position and movement
FL_EXPORT extern int                e_x;                ///< Mouse X position (window relative)
FL_EXPORT extern int                e_y;                ///< Mouse Y position (window relative)
FL_EXPORT extern int                e_x_root;           ///< Mouse X position (screen absolute)
FL_EXPORT extern int                e_y_root;           ///< Mouse Y position (screen absolute)
FL_EXPORT extern int                e_dx;               ///< Mouse wheel horizontal delta
FL_EXPORT extern int                e_dy;               ///< Mouse wheel vertical delta

// Mouse click handling
FL_EXPORT extern int                e_clicks;           ///< Number of consecutive clicks
FL_EXPORT extern int                e_is_click;         ///< True if event qualifies as click

// Keyboard event data
FL_EXPORT extern int                e_keysym;           ///< Key symbol for current event
FL_EXPORT extern int                e_original_keysym;  ///< Original key before NumLock translation
FL_EXPORT extern char*              e_text;             ///< Text associated with key event
FL_EXPORT extern int                e_length;           ///< Length of text in e_text

// Clipboard data (for paste events)
FL_EXPORT extern void*              e_clipboard_data;   ///< Pasted data pointer
FL_EXPORT extern const char*        e_clipboard_type;   ///< Type of pasted data

// Event handling infrastructure
FL_EXPORT extern Fl_Event_Dispatch  e_dispatch;         ///< Custom event dispatcher
FL_EXPORT extern Fl_Callback_Reason callback_reason_;   ///< Reason for current callback

// Widget state tracking
FL_EXPORT extern Fl_Widget*         belowmouse_;        ///< Widget under mouse cursor
FL_EXPORT extern Fl_Widget*         pushed_;            ///< Widget receiving drag events
FL_EXPORT extern Fl_Widget*         focus_;             ///< Widget with keyboard focus

// Event variables should be private, but would harm back compatibility.

#endif // FL_DOXYGEN


/** \defgroup fl_events Events handling functions
      Fl class events handling API declared in <FL/Fl.H>
      @{
*/

//
// Event Information Functions
//

/**
  Returns the last event that was processed. This can be used
  to determine if a callback is being done in response to a
  keypress, mouse click, etc.
*/
FL_EXPORT inline int event()                    { return e_number; }

//
// Mouse Position Functions
//

/**
  Returns the mouse position of the event relative to the Fl_Window
  it was passed to.
*/
FL_EXPORT inline int event_x()                  { return e_x; }

/**
  Returns the mouse position of the event relative to the Fl_Window
  it was passed to.
*/
FL_EXPORT inline int event_y()                  { return e_y; }

/**
  Returns the mouse position on the screen of the event.  To find the
  absolute position of an Fl_Window on the screen, use the
  difference between event_x_root(),event_y_root() and
  event_x(),event_y().
*/
FL_EXPORT inline int event_x_root()             { return e_x_root; }

/**
  Returns the mouse position on the screen of the event.  To find the
  absolute position of an Fl_Window on the screen, use the
  difference between event_x_root(),event_y_root() and
  event_x(),event_y().
*/
FL_EXPORT inline int event_y_root()             { return e_y_root; }

//
// Mouse Wheel Functions
//

/**
  Returns the current horizontal mouse scrolling associated with the
  FL_MOUSEWHEEL event. Right is positive.
*/
FL_EXPORT inline int event_dx()                 { return e_dx; }

/**
  Returns the current vertical mouse scrolling associated with the
  FL_MOUSEWHEEL event. Down is positive.
*/
FL_EXPORT inline int event_dy()                 { return e_dy; }

//
// Mouse Query Functions
//

/**
  Return where the mouse is on the screen by doing a round-trip query to
  the server.  You should use Fl::event_x_root() and
  Fl::event_y_root() if possible, but this is necessary if you are
  not sure if a mouse event has been processed recently (such as to
  position your first window).  If the display is not open, this will
  open it.
*/
FL_EXPORT extern void get_mouse(int &,int &);

//
// Mouse Click Functions
//

/**
  Returns non zero if we had a double click event.
  \retval Non-zero if the most recent FL_PUSH or FL_KEYBOARD was a "double click".
  \retval  N-1 for  N clicks.
  A double click is counted if the same button is pressed
  again while event_is_click() is true.

 */
FL_EXPORT inline int event_clicks()             { return e_clicks; }

/**
  Manually sets the number returned by Fl::event_clicks().
  This can be used to set it to zero so that
  later code does not think an item was double-clicked.
  \param[in] i corresponds to no double-click if 0, i+1 mouse clicks otherwise
  \see int event_clicks()
*/
FL_EXPORT inline void event_clicks(int i)       { e_clicks = i; }

/**
  Returns non-zero if the mouse has not moved far enough
  and not enough time has passed since the last FL_PUSH or
  FL_KEYBOARD event for it to be considered a "drag" rather than a
  "click".  You can test this on FL_DRAG, FL_RELEASE,
  and FL_MOVE events.
*/
FL_EXPORT inline int event_is_click()           { return e_is_click; }

/**
 Clears the value returned by Fl::event_is_click().
 Useful to prevent the <I>next</I>
 click from being counted as a double-click or to make a popup menu
 pick an item with a single click.  Don't pass non-zero to this.
*/
FL_EXPORT inline void event_is_click(int i)     { e_is_click = i; }

//
// Mouse Button Functions
//

/**
  Gets which particular mouse button caused the current event.

  This returns garbage if the most recent event was not a FL_PUSH or FL_RELEASE event.
  \retval FL_LEFT_MOUSE
  \retval FL_MIDDLE_MOUSE
  \retval FL_RIGHT_MOUSE
  \retval FL_BACK_MOUSE
  \retval FL_FORWARD_MOUSE
  \see Fl::event_buttons(), Fl::event_state()
*/
FL_EXPORT inline int event_button()             { return e_keysym - FL_Button; }

//
// Event State Functions
//

/**
  Returns the keyboard and mouse button states of the last event.

  This is a bitfield of what shift states were on and what mouse buttons
  were held down during the most recent event.

  \note FLTK platforms differ in what Fl::event_state() returns when it is called
    while a modifier key or mouse button is being pressed or released.

  - Under X11 and Wayland, Fl::event_state() indicates the state of the modifier keys and
    mouse buttons just \b prior to the event. Thus, during the \c FL_KEYDOWN event generated
    when pressing the shift key, for example, the \c FL_SHIFT bit of event_state() is 0 and
    becomes 1 only at the next event which can be any other event, including e.g. \c FL_MOVE.
  - Under other platforms the reported state of modifier keys or mouse buttons includes that
    of the key or button being pressed or released.
  - Fl::event_state() returns the same value under all platforms when it's called while a
    non-modifier key (e.g. a letter or function key) is being pressed or released.
  - X servers do not agree on shift states, and \c FL_NUM_LOCK, \c FL_META, and \c FL_SCROLL_LOCK
    may not work.
  - The values were selected to match the XFree86 server on Linux.

  \note This inconsistency \b may be fixed (on X11 and Wayland) in a later release.

  The legal event state bits are:

    | Device   | State Bit      | Key or Button           | Since  |
    |----------|----------------|-------------------------|--------|
    | Keyboard | FL_SHIFT       | Shift                   |        |
    | Keyboard | FL_CAPS_LOCK   | Caps Lock               |        |
    | Keyboard | FL_CTRL        | Ctrl                    |        |
    | Keyboard | FL_ALT         | Alt                     |        |
    | Keyboard | FL_NUM_LOCK    | Num Lock                |        |
    | Keyboard | FL_META        | Meta, e.g. "Windows"    |        |
    | Keyboard | FL_SCROLL_LOCK | Scroll Lock             |        |
    | Mouse    | FL_BUTTON1     | left button             |        |
    | Mouse    | FL_BUTTON2     | middle button           |        |
    | Mouse    | FL_BUTTON3     | right button            |        |
    | Mouse    | FL_BUTTON4     | side button 1 (back)    | 1.3.10 |
    | Mouse    | FL_BUTTON5     | side button 2 (forward) | 1.3.10 |
*/
FL_EXPORT inline int event_state()              { return e_state; }

/** Returns non-zero if any of the passed event state bits are turned on.

  Use \p mask to pass the event states you're interested in.
  The legal event state bits are defined in Fl::event_state().
*/
FL_EXPORT inline int event_state(int mask)      { return e_state & mask; }

//
// Keyboard Event Functions
//

/**
  Gets which key on the keyboard was last pushed.

  The returned integer 'key code' is not necessarily a text
  equivalent for the keystroke. For instance: if someone presses '5' on the
  numeric keypad with numlock on, Fl::event_key() may return the 'key code'
  for this key, and NOT the character '5'. To always get the '5', use Fl::event_text() instead.

  \returns an integer 'key code', or 0 if the last event was not a key press or release.
  \see int event_key(int), event_text(), compose(int&).
*/
FL_EXPORT inline int event_key()                { return e_keysym; }

/**
  Returns the keycode of the last key event, regardless of the NumLock state.

  If NumLock is deactivated, FLTK translates events from the
  numeric keypad into the corresponding arrow key events.
  event_key() returns the translated key code, whereas
  event_original_key() returns the keycode before NumLock translation.
*/
FL_EXPORT inline int event_original_key()       { return e_original_keysym; }

/**
  Returns true if the given \p key was held
  down (or pressed) <I>during</I> the last event.  This is constant until
  the next event is read from the server.

  Fl::get_key(int) returns true if the given key is held down <I>now</I>.
  Under X this requires a round-trip to the server and is <I>much</I>
  slower than Fl::event_key(int).

  Keys are identified by the <I>unshifted</I> values. FLTK defines a
  set of symbols that should work on most modern machines for every key
  on the keyboard:

  \li All keys on the main keyboard producing a printable ASCII
      character use the value of that ASCII character (as though shift,
      ctrl, and caps lock were not on). The space bar is 32.
  \li All keys on the numeric keypad producing a printable ASCII
      character use the value of that ASCII character plus FL_KP
      (e.g., FL_KP + '4', FL_KP + '/').
      The highest possible value is FL_KP_Last so you can
      range-check to see if something is  on the keypad.
  \li All numbered function keys use the number on the function key plus
      FL_F.  The highest possible number is FL_F_Last, so you
      can range-check a value.
  \li Buttons on the mouse are considered keys, and use the button
      number (where the left button is 1) plus FL_Button.
  \li All other keys on the keypad have a symbol: FL_Escape,
      FL_BackSpace, FL_Tab, FL_Enter, FL_Print, FL_Scroll_Lock, FL_Pause,
      FL_Insert, FL_Home, FL_Page_Up, FL_Delete, FL_End, FL_Page_Down,
      FL_Left, FL_Up, FL_Right, FL_Down, FL_Iso_Key, FL_Shift_L, FL_Shift_R,
      FL_Control_L, FL_Control_R, FL_Caps_Lock, FL_Alt_L, FL_Alt_R,
      FL_Meta_L, FL_Meta_R, FL_Menu, FL_Num_Lock, FL_KP_Enter.  Be
      careful not to confuse these with the very similar, but all-caps,
      symbols used by Fl::event_state().

  On X Fl::get_key(FL_Button+n) does not work.

  On Windows Fl::get_key(FL_KP_Enter) and Fl::event_key(FL_KP_Enter) do not work.
*/
FL_EXPORT extern int event_key(int key);

/**
  Returns true if the given \p key is held down <I>now</I>.
  Under X this requires a round-trip to the server and is <I>much</I>
  slower than Fl::event_key(int). \see event_key(int)
*/
FL_EXPORT extern int get_key(int key); // platform dependent

//
// Text and Clipboard Functions
//

/**
  Returns the text associated with the current event, including FL_PASTE or FL_DND_RELEASE events.
  This can be used in response to FL_KEYUP, FL_KEYDOWN, FL_PASTE, and FL_DND_RELEASE.

  When responding to FL_KEYUP/FL_KEYDOWN, use this function instead of Fl::event_key()
  to get the text equivalent of keystrokes suitable for inserting into strings
  and text widgets.

  The returned string is guaranteed to be NULL terminated.
  However, see Fl::event_length() for the actual length of the string,
  in case the string itself contains NULLs that are part of the text data.

  \returns A NULL terminated text string equivalent of the last keystroke.
 */
FL_EXPORT inline const char* event_text()       { return e_text; }

/**
  Returns the length of the text in Fl::event_text(). There
  will always be a nul at this position in the text. However there may
  be a nul before that if the keystroke translates to a nul character or
  you paste a nul character.
*/
FL_EXPORT inline int event_length()             { return e_length; }

/**  Denotes plain textual data
 */
FL_EXPORT extern char const * const clipboard_plain_text;

/**  Denotes image data
 */
FL_EXPORT extern char const * const clipboard_image;

/** During an FL_PASTE event of non-textual data, returns a pointer to the pasted data.
 The returned data is an Fl_RGB_Image * when the result of Fl::event_clipboard_type() is Fl::clipboard_image.
 */
FL_EXPORT inline void *event_clipboard()        { return e_clipboard_data; }

/** Returns the type of the pasted data during an FL_PASTE event.
 This type can be Fl::clipboard_plain_text or Fl::clipboard_image.
 */
FL_EXPORT inline const char *event_clipboard_type() { return e_clipboard_type; }


//
// Advanced Event Functions
//

FL_EXPORT extern int compose(int &del);
FL_EXPORT extern void compose_reset();
FL_EXPORT extern int event_inside(int,int,int,int);
FL_EXPORT extern int event_inside(const Fl_Widget*);
FL_EXPORT extern int test_shortcut(Fl_Shortcut);

FL_EXPORT extern void enable_im();
FL_EXPORT extern void disable_im();

//
// Event Handling and Widget Management
//

FL_EXPORT extern int handle(int, Fl_Window*);
FL_EXPORT extern int handle_(int, Fl_Window*);

/**  Gets the widget that is below the mouse.
     \see  belowmouse(Fl_Widget*) */
FL_EXPORT inline Fl_Widget* belowmouse()        { return belowmouse_; }
FL_EXPORT extern void belowmouse(Fl_Widget*);

/** Gets the widget that is being pushed.
    \see void pushed(Fl_Widget*) */
FL_EXPORT inline Fl_Widget* pushed()            { return pushed_; }
FL_EXPORT extern void pushed(Fl_Widget*);

/** Gets the current Fl::focus() widget. \sa Fl::focus(Fl_Widget*) */
FL_EXPORT inline Fl_Widget* focus()             { return focus_; }
FL_EXPORT extern void focus(Fl_Widget*);

FL_EXPORT extern void add_handler(Fl_Event_Handler ha);
FL_EXPORT extern void add_handler(Fl_Event_Handler ha, Fl_Event_Handler before);
FL_EXPORT extern Fl_Event_Handler last_handler();
FL_EXPORT extern void remove_handler(Fl_Event_Handler h);
FL_EXPORT extern void add_system_handler(Fl_System_Handler h, void *data);
FL_EXPORT extern void remove_system_handler(Fl_System_Handler h);
FL_EXPORT extern void event_dispatch(Fl_Event_Dispatch d);
FL_EXPORT extern Fl_Event_Dispatch event_dispatch();
FL_EXPORT extern Fl_Callback_Reason callback_reason();

//
// Convenience State Check Functions
//

/** Returns non-zero if the Shift key is pressed. */
FL_EXPORT inline int event_shift()              { return e_state & FL_SHIFT; }

/** Returns non-zero if the Control key is pressed. */
FL_EXPORT inline int event_ctrl()               { return e_state & FL_CTRL; }

/** Returns non-zero if the FL_COMMAND key is pressed, either FL_CTRL or on OSX FL_META. */
FL_EXPORT inline int event_command()            { return e_state & FL_COMMAND; }

/** Returns non-zero if the Alt key is pressed. */
FL_EXPORT inline int event_alt()                { return e_state & FL_ALT; }

//
// Mouse Button State Functions
//

/**
  Returns the mouse buttons state bits; if non-zero, then at least one
  button is pressed now.  This function returns the button state at the
  time of the event. During an FL_RELEASE event, the state
  of the released button will be 0. To find out, which button
  caused an FL_RELEASE event, you can use Fl::event_button() instead.
  \return a bit mask value like { [FL_BUTTON1] | [FL_BUTTON2] | ... | [FL_BUTTON5] }
*/
FL_EXPORT inline int event_buttons()            { return e_state & FL_BUTTONS; }

/**
  Returns non-zero if mouse button 1 is currently held down.
  For more details, see Fl::event_buttons().
*/
FL_EXPORT inline int event_button1()            { return e_state & FL_BUTTON1; }

/**
  Returns non-zero if mouse button 2 is currently held down.
  For more details, see Fl::event_buttons().
*/
FL_EXPORT inline int event_button2()            { return e_state & FL_BUTTON2; }

/**
  Returns non-zero if mouse button 3 is currently held down.
  For more details, see Fl::event_buttons().
*/
FL_EXPORT inline int event_button3()            { return e_state & FL_BUTTON3; }

/**
  Returns non-zero if mouse button 4 is currently held down.
  For more details, see Fl::event_buttons().
*/
FL_EXPORT inline int event_button4()            { return e_state & FL_BUTTON4; }

/**
  Returns non-zero if mouse button 5 is currently held down.
  For more details, see Fl::event_buttons().
*/
FL_EXPORT inline int event_button5()            { return e_state & FL_BUTTON5; }

/**   @} */

} // namespace Fl

#endif // !Fl_core_events_H
