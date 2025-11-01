//
// Main header file for the Fast Light Tool Kit (FLTK).
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

/** \file FL/Fl.H
  \brief Fl namespace.
*/

#ifndef Fl_H
#  define Fl_H

#include <FL/fl_config.h> // build configuration
#include <FL/Fl_Export.H>
#include <FL/platform_types.h> // for FL_SOCKET
#include <FL/fl_casts.H>       // experimental

#ifdef FLTK_HAVE_CAIRO
# include <FL/Fl_Cairo.H>
#endif

#  include "fl_utf8.h"
#  include "Enumerations.H"
#  ifndef Fl_Object
#    define Fl_Object Fl_Widget /**< for back compatibility - use Fl_Widget! */
#  endif

#  ifdef check
#    undef check
#  endif

#  ifdef BSD
#    undef BSD
#  endif

#include <string.h> // FIXME: Fl::is_scheme(): strcmp needs string.h

class Fl_Widget;
class Fl_Window;
class Fl_Image;
struct Fl_Label;
class Fl_Screen_Driver;
class Fl_System_Driver;

// Pointers you can use to change FLTK to another language.
// Note: Similar pointers are defined in FL/fl_ask.H and src/fl_ask.cxx

extern FL_EXPORT const char* fl_local_alt;   ///< string pointer used in shortcuts, you can change it to another language
extern FL_EXPORT const char* fl_local_ctrl;  ///< string pointer used in shortcuts, you can change it to another language
extern FL_EXPORT const char* fl_local_meta;  ///< string pointer used in shortcuts, you can change it to another language
extern FL_EXPORT const char* fl_local_shift; ///< string pointer used in shortcuts, you can change it to another language

/** \defgroup  callback_functions  Callback Function Typedefs

  \brief Typedefs defined in <FL/Fl.H> for callback or handler functions passed as function parameters.

  FLTK uses callback functions as parameters for some function calls, e.g. to
  set up global event handlers (Fl::add_handler()), to add a timeout handler
  (Fl::add_timeout()), and many more.

  The typedefs defined in this group describe the function parameters used to set
  up or clear the callback functions and should also be referenced to define the
  callback function to handle such events in the user's code.

  \see  Fl::add_handler(), Fl::add_timeout(), Fl::repeat_timeout(),
        Fl::remove_timeout() and others

  @{
*/

/** Signature of some label drawing functions passed as parameters */
typedef void (Fl_Label_Draw_F)(const Fl_Label *label, int x, int y, int w, int h, Fl_Align align);

/** Signature of some label measurement functions passed as parameters */
typedef void (Fl_Label_Measure_F)(const Fl_Label *label, int &width, int &height);

/** Signature of some box drawing functions passed as parameters */
typedef void (Fl_Box_Draw_F)(int x, int y, int w, int h, Fl_Color color);

/** Signature of box focus frame drawing functions */
typedef void (Fl_Box_Draw_Focus_F)(Fl_Boxtype bt, int x, int y, int w, int h, Fl_Color fg, Fl_Color bg);

/** Signature of timeout callback functions passed as parameters.
  Please see Fl::add_timeout() for details.
*/
typedef void (*Fl_Timeout_Handler)(void *data);

/** Signature of some wakeup callback functions passed as parameters */
typedef void (*Fl_Awake_Handler)(void *data);

/** Signature of add_idle callback function passed as parameter.
  This signature allows an idle callback to use one parameter as an
  arbitrary `data` value.
*/
typedef void (*Fl_Idle_Handler)(void *data);

/** Signature of add_idle callback function passed as parameter.
  This signature allows an idle callback without parameters.
  When the callback is called it is called with an additional
  parameter (set to nullptr) which is not used by the callback.
*/
typedef void (*Fl_Old_Idle_Handler)();

/** Signature of add_fd functions passed as parameters */
typedef void (*Fl_FD_Handler)(FL_SOCKET fd, void *data);

/** Signature of add_handler functions passed as parameters */
typedef int (*Fl_Event_Handler)(int event);

/** Signature of add_system_handler functions passed as parameters */
typedef int (*Fl_System_Handler)(void *event, void *data);

/** Signature of set_abort functions passed as parameters */
typedef void (*Fl_Abort_Handler)(const char *format,...);

/** Signature of set_atclose functions passed as parameters */
typedef void (*Fl_Atclose_Handler)(Fl_Window *window, void *data);

/** Signature of args functions passed as parameters */
typedef int (*Fl_Args_Handler)(int argc, char **argv, int &i);

/** Signature of event_dispatch functions passed as parameters.
    \see Fl::event_dispatch(Fl_Event_Dispatch) */
typedef int (*Fl_Event_Dispatch)(int event, Fl_Window *w);

/** Signature of add_clipboard_notify functions passed as parameters */
typedef void (*Fl_Clipboard_Notify_Handler)(int source, void *data);

/** @} */ /* group callback_functions */


/**
  Fl is the FLTK global namespace containing state information and global
  methods for the current application.
*/
namespace Fl {

FL_EXPORT extern Fl_Screen_Driver *screen_driver();
FL_EXPORT extern Fl_System_Driver *system_driver();

#ifdef __APPLE__ // deprecated in 1.4 - only for compatibility with 1.3
FL_EXPORT extern void reset_marked_text();
FL_EXPORT extern void insertion_point_location(int x, int y, int height);
#endif

FL_EXPORT extern int box_shadow_width();
FL_EXPORT extern void box_shadow_width(int W);

FL_EXPORT extern int box_border_radius_max();
FL_EXPORT extern void box_border_radius_max(int R);

// should be private:

#ifndef FL_DOXYGEN
FL_EXPORT extern int e_number;
FL_EXPORT extern int e_x;
FL_EXPORT extern int e_y;
FL_EXPORT extern int e_x_root;
FL_EXPORT extern int e_y_root;
FL_EXPORT extern int e_dx;
FL_EXPORT extern int e_dy;
FL_EXPORT extern int e_state;
FL_EXPORT extern int e_clicks;
FL_EXPORT extern int e_is_click;
FL_EXPORT extern int e_keysym;
FL_EXPORT extern char* e_text;
FL_EXPORT extern int e_length;
FL_EXPORT extern void *e_clipboard_data;
FL_EXPORT extern const char *e_clipboard_type;
FL_EXPORT extern Fl_Event_Dispatch e_dispatch;
FL_EXPORT extern Fl_Callback_Reason callback_reason_;
FL_EXPORT extern Fl_Widget* belowmouse_;
FL_EXPORT extern Fl_Widget* pushed_;
FL_EXPORT extern Fl_Widget* focus_;
FL_EXPORT extern int damage_;
FL_EXPORT extern Fl_Widget* selection_owner_;
FL_EXPORT extern Fl_Window* modal_;
FL_EXPORT extern Fl_Window* grab_;
FL_EXPORT extern int compose_state; // used for dead keys (Windows) or marked text (MacOS)
FL_EXPORT extern void call_screen_init(); // recompute screen number and dimensions
#endif // FL_DOXYGEN


/**
  If true then flush() will do something.
*/
FL_EXPORT inline void damage(int d) {damage_ = d;}

/** Enumerator for global FLTK options.
    These options can be set system wide, per user, or for the running
    application only.
    \see Fl::option(Fl_Option, bool)
    \see Fl::option(Fl_Option)
*/
typedef enum {
    /// When switched on, moving the text cursor beyond the start or end of
    /// a text in a text widget will change focus to the next text widget.
    /// (This is considered 'old' behavior)
    ///
    /// When switched off (default), the cursor will stop at the end of the text.
    /// Pressing Tab or Ctrl-Tab will advance the keyboard focus.
    ///
    /// See also: Fl_Input_::tab_nav()
    ///
  OPTION_ARROW_FOCUS = 0,
    // When switched on, FLTK will use the file chooser dialog that comes
    // with your operating system whenever possible. When switched off, FLTK
    // will present its own file chooser.
    // \todo implement me
  // OPTION_NATIVE_FILECHOOSER,
    // When Filechooser Preview is enabled, the FLTK or native file chooser
    // will show a preview of a selected file (if possible) before the user
    // decides to choose the file.
    // \todo implement me
  //OPTION_FILECHOOSER_PREVIEW,
    /// If visible focus is switched on (default), FLTK will draw a dotted rectangle
    /// inside the widget that will receive the next keystroke. If switched
    /// off, no such indicator will be drawn and keyboard navigation
    /// is disabled.
  OPTION_VISIBLE_FOCUS,
    /// If text drag-and-drop is enabled (default), the user can select and drag text
    /// from any text widget. If disabled, no dragging is possible, however
    /// dropping text from other applications still works.
  OPTION_DND_TEXT,
    /// If tooltips are enabled (default), hovering the mouse over a widget with a
    /// tooltip text will open a little tooltip window until the mouse leaves
    /// the widget. If disabled, no tooltip is shown.
  OPTION_SHOW_TOOLTIPS,
    /// When switched on (default), Fl_Native_File_Chooser runs GTK file dialogs
    /// if the GTK library is available on the platform (linux/unix only).
    /// When switched off, GTK file dialogs aren't used even if the GTK library is available.
  OPTION_FNFC_USES_GTK,
    /// Meaningful for the Wayland/X11 platform only. When switched on, the library uses a Zenity-based file dialog.
    /// When switched off (default), no zenity-based file dialog is used.
  OPTION_FNFC_USES_ZENITY,
    /// Meaningful for the Wayland/X11 platform only.
    /// When switched on, the library uses a kdialog-based file dialog if command 'kdialog' is available on the running system.
    /// When switched off (default), no kdialog-based file dialog is used.
  OPTION_FNFC_USES_KDIALOG,
    /// When switched on (default), Fl_Printer runs the GTK printer dialog
    /// if the GTK library is available on the platform (linux/unix only).
    /// When switched off, the GTK printer dialog isn't used even if the GTK library is available.
  OPTION_PRINTER_USES_GTK,
    /// When switched on (default), the library shows in a transient yellow window the zoom factor
    /// value.
    /// When switched off, no such window gets displayed.
  OPTION_SHOW_SCALING,
    /// When switched on and when the keyboard in use has '+' in the shifted position of its key,
    /// pressing that key and ctrl triggers the zoom-in operation.
    /// When switched off (default), the zoom-in operation requires that also the shift key is pressed.
    /// Under macOS, this option has no effect because the OS itself generates ⌘= followed
    /// by ⌘+ when pressing ⌘ and the '=|+' key without pressing shift.
  OPTION_SIMPLE_ZOOM_SHORTCUT,
    // don't change this, leave it always as the last element
    /// For internal use only.
  OPTION_LAST
} Fl_Option;

/*
 Return a global setting for all FLTK applications, possibly overridden
 by a setting specifically for this application.
 */
FL_EXPORT extern bool option(Fl_Option opt);

/*
 Override an option while the application is running.
 */
FL_EXPORT extern void option(Fl_Option opt, bool val);

FL_EXPORT extern bool idle();

#ifndef FL_DOXYGEN
FL_EXPORT extern const char* scheme_;
FL_EXPORT extern Fl_Image* scheme_bg_;

FL_EXPORT extern int e_original_keysym; // late addition
FL_EXPORT extern int scrollbar_size_;
FL_EXPORT extern int menu_linespacing_; // STR #2927
#endif

// API version number
FL_EXPORT extern double version();
FL_EXPORT extern int api_version();

// ABI version number
FL_EXPORT extern int abi_version();

/**
  Returns whether the runtime library ABI version is correct.

  This enables you to check the ABI version of the linked FLTK
  library at runtime.

  Returns 1 (true) if the compiled ABI version (in the header files)
  and the linked library ABI version (used at runtime) are the same,
  0 (false) otherwise.

  Argument \p val can be used to query a particular library ABI version.
  Use for instance 10303 to query if the runtime library is compatible
  with FLTK ABI version 1.3.3. This is rarely useful.

  The default \p val argument is FL_ABI_VERSION, which checks the version
  defined at configure time (i.e. in the header files at program
  compilation time) against the linked library version used at runtime.
  This is particularly useful if you linked with a shared object library,
  but it also concerns static linking.

  \see Fl::abi_version()
*/
FL_EXPORT inline int abi_check(const int val = FL_ABI_VERSION) {
  return val == abi_version();
}

// argument parsers:
FL_EXPORT extern int arg(int argc, char **argv, int& i);
FL_EXPORT extern int args(int argc, char **argv, int& i, Fl_Args_Handler cb = 0);
FL_EXPORT extern void args(int argc, char **argv);

/**
  Usage string displayed if Fl::args() detects an invalid argument.
  This may be changed to point to customized text at run-time.
*/
FL_EXPORT extern const char* const help;

// things called by initialization:
FL_EXPORT extern void display(const char*);
FL_EXPORT extern int visual(int);

/**
  This does the same thing as Fl::visual(int) but also requires OpenGL
  drawing to work. This <I>must</I> be done if you want to draw in
  normal windows with OpenGL with gl_start() and gl_end().
  It may be useful to call this so your X windows use the same visual
  as an Fl_Gl_Window, which on some servers will reduce colormap flashing.

  See Fl_Gl_Window for a list of additional values for the argument.
*/
FL_EXPORT extern int gl_visual(int, int *alist=0); // platform dependent
FL_EXPORT extern void own_colormap();
FL_EXPORT extern void get_system_colors();
FL_EXPORT extern void foreground(uchar, uchar, uchar);
FL_EXPORT extern void background(uchar, uchar, uchar);
FL_EXPORT extern void background2(uchar, uchar, uchar);

// schemes:
FL_EXPORT extern int scheme(const char *name);
/** See void scheme(const char *name) */
FL_EXPORT inline const char* scheme() {return scheme_;}

/** Returns whether the current scheme is the given name.

  This is a fast inline convenience function to support scheme-specific
  code in widgets, e.g. in their draw() methods, if required.

  Use a valid scheme name, not \p NULL (although \p NULL is allowed,
  this is not a useful argument - see below).

  If Fl::scheme() has not been set or has been set to the default
  scheme ("none" or "base"), then this will always return 0 regardless
  of the argument, because Fl::scheme() is \p NULL in this case.

  \note The stored scheme name is always lowercase, and this method will
  do a case-sensitive compare, so you \b must provide a lowercase string to
  return the correct value. This is intentional for performance reasons.

  Example:
  \code
    if (Fl::is_scheme("gtk+")) { your_code_here(); }
  \endcode

  \param[in] name \b lowercase string of requested scheme name.

  \return 1 if the given scheme is active, 0 otherwise.

  \see Fl::scheme(const char *name)
*/
FL_EXPORT inline int is_scheme(const char *name) {
  return (scheme_ && name && !strcmp(name,scheme_));
}

FL_EXPORT extern int reload_scheme(); // defined in 'src/Fl_get_system_colors.cxx'

FL_EXPORT extern int scrollbar_size();
FL_EXPORT extern void scrollbar_size(int W);
FL_EXPORT extern int menu_linespacing();
FL_EXPORT extern void menu_linespacing(int H);

// execution:
FL_EXPORT extern int wait();
FL_EXPORT extern double wait(double time);
FL_EXPORT extern int check();
FL_EXPORT extern int ready();
FL_EXPORT extern int run();
FL_EXPORT extern int program_should_quit();
FL_EXPORT extern void program_should_quit(int should_i);
FL_EXPORT extern void hide_all_windows();

FL_EXPORT extern Fl_Widget* readqueue();

//
// cross-platform timer support
//

FL_EXPORT extern void add_timeout(double t, Fl_Timeout_Handler cb, void *data = 0);
FL_EXPORT extern void repeat_timeout(double t, Fl_Timeout_Handler cb, void *data = 0);
FL_EXPORT extern int  has_timeout(Fl_Timeout_Handler cb, void *data = 0);
FL_EXPORT extern void remove_timeout(Fl_Timeout_Handler cb, void *data = 0);
FL_EXPORT extern int remove_next_timeout(Fl_Timeout_Handler cb, void *data = 0, void **data_return = 0);

FL_EXPORT extern void add_check(Fl_Timeout_Handler, void* = 0);
FL_EXPORT extern int  has_check(Fl_Timeout_Handler, void* = 0);
FL_EXPORT extern void remove_check(Fl_Timeout_Handler, void* = 0);

FL_EXPORT extern Fl_Timestamp now(double offset = 0);
FL_EXPORT extern double seconds_since(Fl_Timestamp& then);
FL_EXPORT extern double seconds_between(Fl_Timestamp& back, Fl_Timestamp& further_back);
FL_EXPORT extern long ticks_since(Fl_Timestamp& then);
FL_EXPORT extern long ticks_between(Fl_Timestamp& back, Fl_Timestamp& further_back);

FL_EXPORT extern void add_fd(int fd, int when, Fl_FD_Handler cb, void* = 0); // platform dependent
FL_EXPORT extern void add_fd(int fd, Fl_FD_Handler cb, void* = 0); // platform dependent
/** Removes a file descriptor handler. */
FL_EXPORT extern void remove_fd(int, int when); // platform dependent
/** Removes a file descriptor handler. */
FL_EXPORT extern void remove_fd(int); // platform dependent

FL_EXPORT extern void add_idle(Fl_Idle_Handler cb, void* data = 0);
FL_EXPORT extern int  has_idle(Fl_Idle_Handler cb, void* data = 0);
FL_EXPORT extern void remove_idle(Fl_Idle_Handler cb, void* data = 0);
FL_EXPORT extern void add_idle(Fl_Old_Idle_Handler cb);
/** If true then flush() will do something. */
FL_EXPORT inline int damage() {return damage_;}
FL_EXPORT extern void redraw();
FL_EXPORT extern void flush();

/** \addtogroup group_comdlg
  @{ */

/**
  FLTK calls Fl::warning() to output a warning message.

  The default version on Windows returns \e without printing a warning
  message, because Windows programs normally don't have stderr (a console
  window) enabled.

  The default version on all other platforms prints the warning message to stderr.

  You can override the behavior by setting the function pointer to your
  own routine.

  Fl::warning() means that there was a recoverable problem, the display may
  be messed up, but the user can probably keep working - all X protocol
  errors call this, for example. The default implementation returns after
  displaying the message.
   \note \#include <FL/Fl.H>
 */
FL_EXPORT extern void (*warning)(const char*, ...);

/**
  FLTK calls Fl::error() to output a normal error message.

  The default version on Windows displays the error message in a MessageBox window.

  The default version on all other platforms prints the error message to stderr.

  You can override the behavior by setting the function pointer to your
  own routine.

  Fl::error() means there is a recoverable error such as the inability to read
  an image file. The default implementation returns after displaying the message.
   \note \#include <FL/Fl.H>
  */
FL_EXPORT extern void (*error)(const char*, ...);

/**
  FLTK calls Fl::fatal() to output a fatal error message.

  The default version on Windows displays the error message in a MessageBox window.

  The default version on all other platforms prints the error message to stderr.

  You can override the behavior by setting the function pointer to your
  own routine.

  Fl::fatal() must not return, as FLTK is in an unusable state, however your
  version may be able to use longjmp or an exception to continue, as long as
  it does not call FLTK again. The default implementation exits with status 1
  after displaying the message.
   \note \#include <FL/Fl.H>
*/
FL_EXPORT extern void (*fatal)(const char*, ...);

/** @} */

/** \defgroup  fl_windows Windows handling functions
    \brief Windows and standard dialogs handling declared in <FL/Fl.H>
  @{ */

FL_EXPORT extern Fl_Window* first_window();
FL_EXPORT extern void first_window(Fl_Window*);
FL_EXPORT extern Fl_Window* next_window(const Fl_Window*);

/**
  Returns the top-most modal() window currently shown.

  This is the most recently shown() window with modal() true, or NULL
  if there are no modal() windows shown().
  The modal() window has its handle() method called
  for all events, and no other windows will have handle()
  called (grab() overrides this).
*/
FL_EXPORT inline Fl_Window* modal() {return modal_;}

/** Returns the window that currently receives all events.

 \return The window that currently receives all events,
 or NULL if event grabbing is currently OFF.
*/
FL_EXPORT inline Fl_Window* grab() {return grab_;}

/** Selects the window to grab.
  This is used when pop-up menu systems are active.

  Send all events to the passed window no matter where the pointer or
  focus is (including in other programs). The window <I>does not have
  to be shown()</I> , this lets the handle() method of a
  "dummy" window override all event handling and allows you to
  map and unmap a complex set of windows (under both X and Windows
  <I>some</I> window must be mapped because the system interface needs a
  window id).

  If grab() is on it will also affect show() of windows by doing
  system-specific operations (on X it turns on override-redirect).
  These are designed to make menus popup reliably
  and faster on the system.

  To turn off grabbing do Fl::grab(0).

  <I>Be careful that your program does not enter an infinite loop
  while grab() is on.  On X this will lock up your screen!</I>
  To avoid this potential lockup, all newer operating systems seem to
  limit mouse pointer grabbing to the time during which a mouse button
  is held down. Some OS's may not support grabbing at all.
 */
FL_EXPORT extern void grab(Fl_Window*); // platform dependent

/** @} */

/** \defgroup fl_events Events handling functions
      Fl class events handling API declared in <FL/Fl.H>
      @{
*/

// event information:

/**
  Returns the last event that was processed. This can be used
  to determine if a callback is being done in response to a
  keypress, mouse click, etc.
*/
FL_EXPORT inline int event()            {return e_number;}

/**
  Returns the mouse position of the event relative to the Fl_Window
  it was passed to.
*/
FL_EXPORT inline int event_x()  {return e_x;}

/**
  Returns the mouse position of the event relative to the Fl_Window
  it was passed to.
*/
FL_EXPORT inline int event_y()  {return e_y;}

/**
  Returns the mouse position on the screen of the event.  To find the
  absolute position of an Fl_Window on the screen, use the
  difference between event_x_root(),event_y_root() and
  event_x(),event_y().
*/
FL_EXPORT inline int event_x_root()     {return e_x_root;}

/**
  Returns the mouse position on the screen of the event.  To find the
  absolute position of an Fl_Window on the screen, use the
  difference between event_x_root(),event_y_root() and
  event_x(),event_y().
*/
FL_EXPORT inline int event_y_root()     {return e_y_root;}

/**
  Returns the current horizontal mouse scrolling associated with the
  FL_MOUSEWHEEL event. Right is positive.
*/
FL_EXPORT inline int event_dx() {return e_dx;}

/**
  Returns the current vertical mouse scrolling associated with the
  FL_MOUSEWHEEL event. Down is positive.
*/
FL_EXPORT inline int event_dy() {return e_dy;}

/**
  Return where the mouse is on the screen by doing a round-trip query to
  the server.  You should use Fl::event_x_root() and
  Fl::event_y_root() if possible, but this is necessary if you are
  not sure if a mouse event has been processed recently (such as to
  position your first window).  If the display is not open, this will
  open it.
*/
FL_EXPORT extern void get_mouse(int &,int &);

/**
  Returns non zero if we had a double click event.
  \retval Non-zero if the most recent FL_PUSH or FL_KEYBOARD was a "double click".
  \retval  N-1 for  N clicks.
  A double click is counted if the same button is pressed
  again while event_is_click() is true.

 */
FL_EXPORT inline int event_clicks()     {return e_clicks;}

/**
  Manually sets the number returned by Fl::event_clicks().
  This can be used to set it to zero so that
  later code does not think an item was double-clicked.
  \param[in] i corresponds to no double-click if 0, i+1 mouse clicks otherwise
  \see int event_clicks()
*/
FL_EXPORT inline void event_clicks(int i) {e_clicks = i;}
/**
  Returns non-zero if the mouse has not moved far enough
  and not enough time has passed since the last FL_PUSH or
  FL_KEYBOARD event for it to be considered a "drag" rather than a
  "click".  You can test this on FL_DRAG, FL_RELEASE,
  and FL_MOVE events.
*/
FL_EXPORT inline int event_is_click()   {return e_is_click;}

/**
 Clears the value returned by Fl::event_is_click().
 Useful to prevent the <I>next</I>
 click from being counted as a double-click or to make a popup menu
 pick an item with a single click.  Don't pass non-zero to this.
*/
FL_EXPORT inline void event_is_click(int i) {e_is_click = i;}

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
FL_EXPORT inline int event_button() { return e_keysym - FL_Button; }

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
FL_EXPORT inline int event_state()      {return e_state;}

/** Returns non-zero if any of the passed event state bits are turned on.

  Use \p mask to pass the event states you're interested in.
  The legal event state bits are defined in Fl::event_state().
*/
FL_EXPORT inline int event_state(int mask) {return e_state&mask;}

/**
  Gets which key on the keyboard was last pushed.

  The returned integer 'key code' is not necessarily a text
  equivalent for the keystroke. For instance: if someone presses '5' on the
  numeric keypad with numlock on, Fl::event_key() may return the 'key code'
  for this key, and NOT the character '5'. To always get the '5', use Fl::event_text() instead.

  \returns an integer 'key code', or 0 if the last event was not a key press or release.
  \see int event_key(int), event_text(), compose(int&).
*/
FL_EXPORT inline int event_key()        {return e_keysym;}

/**
  Returns the keycode of the last key event, regardless of the NumLock state.

  If NumLock is deactivated, FLTK translates events from the
  numeric keypad into the corresponding arrow key events.
  event_key() returns the translated key code, whereas
  event_original_key() returns the keycode before NumLock translation.
*/
FL_EXPORT inline int event_original_key(){return e_original_keysym;}

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
FL_EXPORT inline const char* event_text() {return e_text;}

/**
  Returns the length of the text in Fl::event_text(). There
  will always be a nul at this position in the text. However there may
  be a nul before that if the keystroke translates to a nul character or
  you paste a nul character.
*/
FL_EXPORT inline int event_length() {return e_length;}

/**  Denotes plain textual data
 */
FL_EXPORT extern char const * const clipboard_plain_text;

/**  Denotes image data
 */
FL_EXPORT extern char const * const clipboard_image;

/** During an FL_PASTE event of non-textual data, returns a pointer to the pasted data.
 The returned data is an Fl_RGB_Image * when the result of Fl::event_clipboard_type() is Fl::clipboard_image.
 */
FL_EXPORT inline void *event_clipboard() { return e_clipboard_data; }

/** Returns the type of the pasted data during an FL_PASTE event.
 This type can be Fl::clipboard_plain_text or Fl::clipboard_image.
 */
FL_EXPORT inline const char *event_clipboard_type() {return e_clipboard_type; }


FL_EXPORT extern int compose(int &del);
FL_EXPORT extern void compose_reset();
FL_EXPORT extern int event_inside(int,int,int,int);
FL_EXPORT extern int event_inside(const Fl_Widget*);
FL_EXPORT extern int test_shortcut(Fl_Shortcut);

FL_EXPORT extern void enable_im();
FL_EXPORT extern void disable_im();

// event destinations:
FL_EXPORT extern int handle(int, Fl_Window*);
FL_EXPORT extern int handle_(int, Fl_Window*);
/**  Gets the widget that is below the mouse.
     \see  belowmouse(Fl_Widget*) */
FL_EXPORT inline Fl_Widget* belowmouse() {return belowmouse_;}
FL_EXPORT extern void belowmouse(Fl_Widget*);
/** Gets the widget that is being pushed.
    \see void pushed(Fl_Widget*) */
FL_EXPORT inline Fl_Widget* pushed()    {return pushed_;}
FL_EXPORT extern void pushed(Fl_Widget*);
/** Gets the current Fl::focus() widget. \sa Fl::focus(Fl_Widget*) */
FL_EXPORT inline Fl_Widget* focus()     {return focus_;}
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

/** @} */

/** \defgroup  fl_clipboard  Selection & Clipboard functions
      FLTK global copy/cut/paste functions declared in <FL/Fl.H>
 @{ */

/**
  Copies data to the selection buffer, the clipboard, or both.

  The \p destination can be:
   - 0: selection buffer (see note below)
   - 1: clipboard
   - 2: both

  The selection buffer exists only on the X11 platform and is used for middle-mouse
  pastes and for drag-and-drop selections. The clipboard is used for traditional
  copy/cut/paste operations. On all other platforms the selection buffer
  (\p destination = 0) is mapped to the clipboard, i.e. on platforms other than X11
  all \p destinations are equivalent and the data is always copied to the clipboard.

  \note Please see Fl::selection_to_clipboard() to enable duplication of the
    selection buffer to the clipboard on X11, i.e. if \p destination = 0
    (selection buffer) \b and Fl::selection_to_clipboard() is enabled, then
    the data is copied to both the selection buffer and the clipboard.
    This makes the X11 behavior similar to other platforms but keeps the
    selection buffer for X11 specific inter process communication.

  \p type should always be \p Fl::clipboard_plain_text which is the default.
  Other values are ignored and reserved for future extensions.

  \note This function is, at present, intended only to copy UTF-8 encoded
    textual data. To copy graphical data, use the Fl_Copy_Surface class.
    The \p type argument may allow to copy other kinds of data in the future.

  \param[in]  stuff       text data to be copied
  \param[in]  len         the number of relevant bytes in \p stuff
  \param[in]  destination 0 = selection, 1 = clipboard, 2 = both (see description)
  \param[in]  type        usually plain text (see description)

  \internal
    Documented here because it is platform dependent (calls the platform driver):
  \code
    Fl::screen_driver()->copy(stuff, len, clipboard, type);
  \endcode
*/
FL_EXPORT extern void copy(const char *stuff, int len, int destination = 0,
                 const char *type = Fl::clipboard_plain_text);

FL_EXPORT extern void selection_to_clipboard(int mode);
FL_EXPORT extern int selection_to_clipboard();

/**
 Pastes the data from the selection buffer (\p source is 0) or the clipboard
 (\p source is 1) into \p receiver.

 The selection buffer (\p source is 0) is used for middle-mouse pastes and for
 drag-and-drop selections. The clipboard (\p source is 1) is used for
 copy/cut/paste operations.

 If \p source is 1, the optional \p type argument indicates what type of data is requested from the clipboard.
 At present, Fl::clipboard_plain_text (requesting text data) and
 Fl::clipboard_image (requesting image data) are possible.
 Set things up so the handle function of the \p receiver widget will be called with an FL_PASTE event some
 time in the future if the clipboard does contain data of the requested type.

 The handle function of \p receiver can process the FL_PASTE event as follows:
 \li If the \p receiver widget is known to only receive text data, the text string
 from the specified \p source is in Fl::event_text() with UTF-8 encoding, and the
 number of bytes is in Fl::event_length(). If Fl::paste() gets called during the
 drop step of a files-drag-and-drop operation,
 Fl::event_text() contains a list of filenames (see \ref events_dnd).
 \li If the \p receiver widget can potentially receive non-text data, use
 Fl::event_clipboard_type() to determine what sort of data is being sent.
 If Fl::event_clipboard_type() returns Fl::clipboard_plain_text, proceed as above.
 It it returns Fl::clipboard_image, the pointer returned by Fl::event_clipboard()
 can be safely cast to type Fl_RGB_Image * to obtain a pointer to the pasted image.
 If \p receiver accepts the clipboard image, receiver.handle() should return 1 and the
 application should take ownership of this image (that is, delete it after use).
 Conversely, if receiver.handle() returns 0, the application must not use the image.

 The receiver should be prepared to be called \e directly by this, or for
 it to happen \e later, or possibly <i>not at all</i>.  This
 allows the window system to take as long as necessary to retrieve
 the paste buffer (or even to screw up completely) without complex
 and error-prone synchronization code in FLTK.

 \par Platform details for image data:
 \li Unix/Linux platform: Clipboard images in PNG or BMP formats are recognized. Requires linking with the fltk_images library.
 \li Windows platform: Both bitmap and vectorial (Enhanced metafile) data from clipboard
 can be pasted as image data.
 \li Mac OS X platform: Both bitmap (TIFF) and vectorial (PDF) data from clipboard
 can be pasted as image data.
 */
FL_EXPORT extern void paste(Fl_Widget &receiver, int source, const char *type = Fl::clipboard_plain_text);

/**
  FLTK will call the registered callback whenever there is a change to the
  selection buffer or the clipboard. The source argument indicates which
  of the two has changed. Only changes by other applications are reported.

  Example:
  \code
    void clip_callback(int source, void *data) {
        if ( source == 0 ) printf("CLIP CALLBACK: selection buffer changed\n");
        if ( source == 1 ) printf("CLIP CALLBACK: clipboard changed\n");
    }
    [..]
    int main() {
        [..]
        Fl::add_clipboard_notify(clip_callback);
        [..]
    }
  \endcode
  \note Some systems require polling to monitor the clipboard and may
  therefore have some delay in detecting changes.
  */
FL_EXPORT extern void add_clipboard_notify(Fl_Clipboard_Notify_Handler h, void *data = 0);

/**
  Stop calling the specified callback when there are changes to the selection
  buffer or the clipboard.
*/
FL_EXPORT extern void remove_clipboard_notify(Fl_Clipboard_Notify_Handler h);

/** Returns non 0 if the clipboard contains data matching \p type.
  The clipboard can contain both text and image data;  in that situation this function
  returns non 0 to both requests.
  This function is \e not meant to check whether the clipboard is empty.
  This function does not allow to query the selection buffer because FLTK
  allows to copy/paste non-textual data only from/to the clipboard.
  \param type can be Fl::clipboard_plain_text or Fl::clipboard_image.
 */
FL_EXPORT extern int clipboard_contains(const char *type);

/**
  Initiate a Drag And Drop operation. The selection buffer should be
  filled with relevant data before calling this method. FLTK will
  then initiate the system wide drag and drop handling. Dropped data
  will be marked as <i>text</i>.

  Create a selection first using:
  Fl::copy(const char *stuff, int len, 0)
*/
FL_EXPORT extern int dnd(); // platform dependent

// These are for back-compatibility only:
/**  back-compatibility only: Gets the widget owning the current selection
     \see Fl_Widget* selection_owner(Fl_Widget*) */
FL_EXPORT inline Fl_Widget* selection_owner() {return selection_owner_;}
FL_EXPORT extern void selection_owner(Fl_Widget*);
FL_EXPORT extern void selection(Fl_Widget &owner, const char*, int len);
FL_EXPORT extern void paste(Fl_Widget &receiver);

/** @} */


/** \defgroup  fl_screen  Screen functions
 Fl global screen functions declared in <FL/Fl.H>.

 FLTK supports high-DPI screens using a screen scaling factor.
 The scaling factor is initialized by the library to a value
 based on information obtained from the OS. If this initial value
 is not satisfactory, the FLTK_SCALING_FACTOR environment variable
 can be set to a value FLTK will multiply to the OS-given value.
 The 2 variants of functions Fl::screen_scale() allow to programmatically get and set
 scaling factor values. The scaling factor value can be further changed at runtime
 by typing \c Ctrl/+/-/0/ (\c Cmd/+/-/0/ under macOS).
 See \ref events_fl_shortcut for more details about these shortcuts.
     @{ */

FL_EXPORT extern int x(); // via screen driver
FL_EXPORT extern int y(); // via screen driver
FL_EXPORT extern int w(); // via screen driver
FL_EXPORT extern int h(); // via screen driver

// multi-head support:
FL_EXPORT extern int screen_count(); // via screen driver
FL_EXPORT extern void screen_xywh(int &X, int &Y, int &W, int &H); // via screen driver
FL_EXPORT extern void screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my); // via screen driver
FL_EXPORT extern void screen_xywh(int &X, int &Y, int &W, int &H, int n); // via screen driver
FL_EXPORT extern void screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my, int mw, int mh); // via screen driver
FL_EXPORT extern int screen_num(int x, int y); // via screen driver
FL_EXPORT extern int screen_num(int x, int y, int w, int h); // via screen driver
FL_EXPORT extern void screen_dpi(float &h, float &v, int n=0); // via screen driver
FL_EXPORT extern void screen_work_area(int &X, int &Y, int &W, int &H, int mx, int my); // via screen driver
FL_EXPORT extern void screen_work_area(int &X, int &Y, int &W, int &H, int n); // via screen driver
FL_EXPORT extern void screen_work_area(int &X, int &Y, int &W, int &H); // via screen driver
FL_EXPORT extern float screen_scale(int n); // via screen driver
FL_EXPORT extern void screen_scale(int n, float factor); // via screen driver
FL_EXPORT extern int screen_scaling_supported();
FL_EXPORT extern void keyboard_screen_scaling(int value);

/**   @} */


/** \defgroup  fl_attributes  Color & Font functions
      fl global color, font functions.
 These functions are declared in <FL/Fl.H> or <FL/fl_draw.H>.
   @{ */

// color map:
FL_EXPORT extern void   set_color(Fl_Color, uchar, uchar, uchar);
FL_EXPORT extern void   set_color(Fl_Color, uchar, uchar, uchar, uchar);

/**
  Sets an entry in the fl_color index table. You can set it to any
  8-bit RGB color. The color is not allocated until fl_color(i) is used.
*/
FL_EXPORT extern void   set_color(Fl_Color i, unsigned c); // platform dependent
FL_EXPORT extern unsigned get_color(Fl_Color i);
FL_EXPORT extern void   get_color(Fl_Color i, uchar &red, uchar &green, uchar &blue);
FL_EXPORT extern void   get_color(Fl_Color i, uchar &red, uchar &green, uchar &blue, uchar &alpha);

/**
  Frees the specified color from the colormap, if applicable.
  If overlay is non-zero then the color is freed from the
  overlay colormap.
*/
FL_EXPORT extern void   free_color(Fl_Color i, int overlay = 0); // platform dependent

// fonts:
FL_EXPORT extern const char* get_font(Fl_Font);

/**
  Get a human-readable string describing the family of this face.  This
  is useful if you are presenting a choice to the user.  There is no
  guarantee that each face has a different name.  The return value points
  to a static buffer that is overwritten each call.

  The integer pointed to by \p attributes (if the pointer is not
  zero) is set to zero, FL_BOLD or FL_ITALIC or
  FL_BOLD | FL_ITALIC.  To locate a "family" of fonts, search
  forward and back for a set with non-zero attributes, these faces along
  with the face with a zero attribute before them constitute a family.
*/
FL_EXPORT extern const char* get_font_name(Fl_Font, int* attributes = 0);

/**
  Return an array of sizes in \p sizep.  The return value is the
  length of this array.  The sizes are sorted from smallest to largest
  and indicate what sizes can be given to fl_font() that will
  be matched exactly (fl_font() will pick the closest size for
  other sizes).  A zero in the first location of the array indicates a
  scalable font, where any size works, although the array may list sizes
  that work "better" than others.  Warning: the returned array
  points at a static buffer that is overwritten each call.  Under X this
  will open the display.
*/
FL_EXPORT extern int get_font_sizes(Fl_Font, int*& sizep);
FL_EXPORT extern void set_font(Fl_Font, const char*);
FL_EXPORT extern void set_font(Fl_Font, Fl_Font);

/**
  FLTK will open the display, and add every fonts on the server to the
  face table.  It will attempt to put "families" of faces together, so
  that the normal one is first, followed by bold, italic, and bold
  italic.

  The only argument to this function is somewhat obsolete since FLTK and most
  underlying operating systems move to support Unicode. For completeness,
  following is the original documentation and a few updates:

  On X11, the optional argument is a string to describe the set of fonts to
  add.  Passing NULL will select only fonts that have the
  ISO8859-1 character set (and are thus usable by normal text).  Passing
  "-*" will select all fonts with any encoding as long as they have
  normal X font names with dashes in them.  Passing "*" will list every
  font that exists (on X this may produce some strange output).  Other
  values may be useful but are system dependent.

  With the Xft option on Linux, this parameter is ignored.

  With Windows, `NULL` selects fonts with ANSI_CHARSET encoding and non-NULL
  selects all fonts.

  On macOS, this parameter is ignored.

  The return value is how many faces are in the table after this is done.
*/
FL_EXPORT extern Fl_Font set_fonts(const char* = 0); // platform dependent

/**   @} */

/** \defgroup  fl_drawings  Drawing functions
  FLTK global graphics and GUI drawing functions.
  These functions are declared in <FL/fl_draw.H>,
  and in <FL/platform.H> for offscreen buffer-related ones.
   @{ */
// <Hack to re-order the 'Drawing functions' group>
/** @} */

// labeltypes:
FL_EXPORT extern void set_labeltype(Fl_Labeltype,Fl_Label_Draw_F*,Fl_Label_Measure_F*);
/** Sets the functions to call to draw and measure a specific labeltype. */
FL_EXPORT extern void set_labeltype(Fl_Labeltype, Fl_Labeltype from); // is it defined ?

// boxtypes:
FL_EXPORT extern Fl_Box_Draw_F *get_boxtype(Fl_Boxtype);
FL_EXPORT extern void set_boxtype(Fl_Boxtype, Fl_Box_Draw_F*,
                        uchar, uchar, uchar, uchar,
                        Fl_Box_Draw_Focus_F* =NULL);
FL_EXPORT extern void set_boxtype(Fl_Boxtype, Fl_Boxtype from);
FL_EXPORT extern int box_dx(Fl_Boxtype);
FL_EXPORT extern int box_dy(Fl_Boxtype);
FL_EXPORT extern int box_dw(Fl_Boxtype);
FL_EXPORT extern int box_dh(Fl_Boxtype);

FL_EXPORT extern int draw_box_active();
FL_EXPORT extern Fl_Color box_color(Fl_Color);
FL_EXPORT extern void set_box_color(Fl_Color);

// back compatibility:

/** \addtogroup fl_windows
  @{ */

/** For back compatibility, sets the void Fl::fatal handler callback */
FL_EXPORT inline void set_abort(Fl_Abort_Handler f) {fatal = f;}
FL_EXPORT extern void (*atclose)(Fl_Window*,void*);
FL_EXPORT extern void default_atclose(Fl_Window*,void*);

/** For back compatibility, sets the Fl::atclose handler callback. You
    can now simply change the callback for the window instead.
    \see Fl_Window::callback(Fl_Callback*) */
FL_EXPORT inline void set_atclose(Fl_Atclose_Handler f) {atclose = f;}

/**   @} */

/** \addtogroup fl_events
  @{ */

/** Returns non-zero if the Shift key is pressed. */
FL_EXPORT inline int event_shift() {return e_state&FL_SHIFT;}
/** Returns non-zero if the Control key is pressed. */
FL_EXPORT inline int event_ctrl() {return e_state&FL_CTRL;}
/** Returns non-zero if the FL_COMMAND key is pressed, either FL_CTRL or on OSX FL_META. */
FL_EXPORT inline int event_command() {return e_state&FL_COMMAND;}
/** Returns non-zero if the Alt key is pressed. */
FL_EXPORT inline int event_alt() {return e_state&FL_ALT;}

/**
  Returns the mouse buttons state bits; if non-zero, then at least one
  button is pressed now.  This function returns the button state at the
  time of the event. During an FL_RELEASE event, the state
  of the released button will be 0. To find out, which button
  caused an FL_RELEASE event, you can use Fl::event_button() instead.
  \return a bit mask value like { [FL_BUTTON1] | [FL_BUTTON2] | ... | [FL_BUTTON5] }
*/
FL_EXPORT inline int event_buttons() {return e_state & FL_BUTTONS;}

/**
  Returns non-zero if mouse button 1 is currently held down.
  For more details, see Fl::event_buttons().
*/
FL_EXPORT inline int event_button1() {return e_state & FL_BUTTON1;}

/**
  Returns non-zero if mouse button 2 is currently held down.
  For more details, see Fl::event_buttons().
*/
FL_EXPORT inline int event_button2() {return e_state & FL_BUTTON2;}

/**
  Returns non-zero if mouse button 3 is currently held down.
  For more details, see Fl::event_buttons().
*/
FL_EXPORT inline int event_button3() {return e_state & FL_BUTTON3;}

/**
  Returns non-zero if mouse button 4 is currently held down.
  For more details, see Fl::event_buttons().
*/
FL_EXPORT inline int event_button4() {return e_state & FL_BUTTON4;}

/**
  Returns non-zero if mouse button 5 is currently held down.
  For more details, see Fl::event_buttons().
*/
FL_EXPORT inline int event_button5() {return e_state & FL_BUTTON5;}

/**   @} */

/** See grab(Fl_Window*) */
FL_EXPORT inline void grab(Fl_Window& win) {grab(&win);}

// Visible focus methods...
/**
  Gets or sets the visible keyboard focus on buttons and other
  non-text widgets. The default mode is to enable keyboard focus
  for all widgets.
*/
FL_EXPORT inline void visible_focus(int v) { option(OPTION_VISIBLE_FOCUS, (v!=0)); }

/**
  Gets or sets the visible keyboard focus on buttons and other
  non-text widgets. The default mode is to enable keyboard focus
  for all widgets.
*/
FL_EXPORT inline int  visible_focus() { return option(OPTION_VISIBLE_FOCUS); }

// Drag-n-drop text operation methods...
/**
  Sets whether drag and drop text operations are supported.
  This specifically affects whether selected text can
  be dragged from text fields or dragged within a text field as a
  cut/paste shortcut.
*/
FL_EXPORT inline void dnd_text_ops(int v) { option(OPTION_DND_TEXT, (v!=0)); }

/**
  Gets whether drag and drop text operations are
  supported. This returns whether selected text can
  be dragged from text fields or dragged within a text field as a
  cut/paste shortcut.
*/
FL_EXPORT inline int  dnd_text_ops() { return option(OPTION_DND_TEXT); }

// --- FLTK Multithreading support functions ---

/** \defgroup fl_multithread Multithreading support functions
      fl multithreading support functions declared in <FL/Fl.H>
 @{ */

// Thread locking:
FL_EXPORT extern int lock();
FL_EXPORT extern void unlock();
// Thread wakup and defered calls:
FL_EXPORT extern void awake();
FL_DEPRECATED("since 1.5.0 - use Fl::awake() or Fl::awake(handler, user_data) instead",
FL_EXPORT extern void awake(void* message));
FL_EXPORT extern int awake(Fl_Awake_Handler handler, void* user_data=nullptr);
FL_EXPORT extern int awake_once(Fl_Awake_Handler handler, void* user_data=nullptr);
FL_DEPRECATED("since 1.5.0 - use Fl::awake() or Fl::awake(handler, user_data) instead",
FL_EXPORT extern void* thread_message()); // platform dependent

/** @} */

/** \defgroup fl_del_widget Safe widget deletion support functions

  These functions, declared in <FL/Fl.H>, support deletion of widgets inside callbacks.

  Fl::delete_widget() should be called when deleting widgets
  or complete widget trees (Fl_Group, Fl_Window, ...) inside
  callbacks.

  The other functions are intended for internal use. The preferred
  way to use them is by using the helper class Fl_Widget_Tracker.

  The following is to show how it works ...

  There are three groups of related methods:

    -# scheduled widget deletion
      - Fl::delete_widget() schedules widgets for deletion
      - Fl::do_widget_deletion() deletes all scheduled widgets
    -# widget watch list ("smart pointers")
      - Fl::watch_widget_pointer() adds a widget pointer to the watch list
      - Fl::release_widget_pointer() removes a widget pointer from the watch list
      - Fl::clear_widget_pointer() clears a widget pointer \e in the watch list
    -# the class Fl_Widget_Tracker:
      - the constructor calls Fl::watch_widget_pointer()
      - the destructor calls Fl::release_widget_pointer()
      - the access methods can be used to test, if a widget has been deleted
        \see Fl_Widget_Tracker.

 @{ */

// Widget deletion:
FL_EXPORT extern void delete_widget(Fl_Widget *w);
FL_EXPORT extern void do_widget_deletion();
FL_EXPORT extern void watch_widget_pointer(Fl_Widget *&w);
FL_EXPORT extern void release_widget_pointer(Fl_Widget *&w);
FL_EXPORT extern void clear_widget_pointer(Fl_Widget const *w);

/** @} */

FL_EXPORT extern void use_high_res_GL(int val);
FL_EXPORT extern int use_high_res_GL();

FL_EXPORT extern void draw_GL_text_with_textures(int val);
FL_EXPORT extern int draw_GL_text_with_textures();

FL_EXPORT extern int system(const char *command);

// Convert Windows commandline arguments to UTF-8 (documented in src/Fl.cxx)
FL_EXPORT extern int args_to_utf8(int argc, char ** &argv);

#ifdef FLTK_HAVE_CAIRO
/** \defgroup group_cairo Cairo Support Functions and Classes
    @{
*/

// Cairo support API

FL_EXPORT extern cairo_t *cairo_make_current(Fl_Window *w);

FL_EXPORT extern void cairo_autolink_context(bool alink);
FL_EXPORT extern bool cairo_autolink_context();
FL_EXPORT extern cairo_t *cairo_cc();
FL_EXPORT extern void cairo_cc(cairo_t *c, bool own=false);

/** Flush Cairo drawings on Cairo context \p c.
  This is \b required on Windows if you use the Cairo context provided
  by the "Cairo autolink" option. Call this when all your drawings on
  the Cairo context are finished. This is maybe not necessary on other
  platforms than Windows but it does no harm if you call it always.

  You don't need to use this if you use an Fl_Cairo_Window which does
  this automatically after the draw callback returns.

  Code example for "Cairo autolink" mode:

  In the overridden draw() method of your subclass of Fl_Window or any
  widget:
  \code
    cairo_t *cc = Fl::cairo_cc();   // get the "autolink" Cairo context
    // ... your Cairo drawings are here ...
    Fl::cairo_flush(cc);            // flush Cairo drawings to the device
  \endcode

  If you configure FLTK with CMake option
  \c 'FLTK_OPTION_CAIRO_WINDOW' (i.e. without CMake option
  \c 'FLTK_OPTION_CAIRO_EXT') or if you don't enable the \c 'autolink' Cairo
  context you may do the equivalent to use Cairo drawings in an
  overridden draw() method of derived classes by using
  \code
    // get the  Cairo context for the \c window
    cairo_t *cc = Fl::cairo_make_current(window);
    // ... your Cairo drawings are here ...
    Fl::cairo_flush(cc); // flush Cairo drawings to the device
  \endcode
  \see Fl::cairo_autolink_context(bool)
  \see Fl::cairo_make_current(Fl_Window*);
*/
FL_EXPORT extern void cairo_flush(cairo_t *c) {
  // flush Cairo drawings: necessary at least for Windows
  cairo_surface_t *s = cairo_get_target(c);
  cairo_surface_flush(s);
}

/** @} */

#endif // FLTK_HAVE_CAIRO

} // namespace Fl

/**
  This class should be used to control safe widget deletion.

  You can use an Fl_Widget_Tracker object to watch another widget, if you
  need to know whether this widget has been deleted during a callback.

  This simplifies the use of the "safe widget deletion" methods
  Fl::watch_widget_pointer() and Fl::release_widget_pointer() and
  makes their use more reliable, because the destructor automatically
  releases the widget pointer from the widget watch list.

  Fl_Widget_Tracker is intended to be used as an automatic (local/stack)
  variable, such that its destructor is called when the object's
  scope is left. This ensures that no stale widget pointers are
  left in the widget watch list (see example below).

  You can also create Fl_Widget_Tracker objects with \c new, but then it
  is your responsibility to delete the object (and thus remove the
  widget pointer from the watch list) when it is no longer needed.

  Example:

  \code
  int MyClass::handle (int event) {

    if (...) {
      Fl_Widget_Tracker wp(this);           // watch myself
      do_callback();                        // call the callback

      if (wp.deleted()) return 1;           // exit, if deleted

      // Now we are sure that the widget has not been deleted,
      // and it is safe to access the widget:

      box(FL_FLAT_BOX);
      color(FL_WHITE);
      redraw();
    }
  }
  \endcode

*/
class FL_EXPORT Fl_Widget_Tracker {

  Fl_Widget* wp_;

public:

  Fl_Widget_Tracker(Fl_Widget *wi);
  ~Fl_Widget_Tracker();

  /**
    Returns a pointer to the watched widget.

    This pointer is \c NULL, if the widget has been deleted.
  */
  Fl_Widget *widget() {return wp_;}

  /**
    Returns 1, if the watched widget has been deleted.

    This is a convenience method. You can also use something like

    <tt>  if (wp.widget() == 0) // ...</tt>

    where \p wp is an Fl_Widget_Tracker object.
  */
  int deleted() {return wp_ == 0;}

  /**
    Returns 1, if the watched widget exists (has not been deleted).

    This is a convenience method. You can also use something like

    <tt>  if (wp.widget() != 0) // ...</tt>

    where \p wp is an Fl_Widget_Tracker object.
  */
  int exists()  {return wp_ != 0;}

};

 /** \defgroup  fl_unicode  Unicode and UTF-8 functions
        fl global Unicode and UTF-8 handling functions declared in <FL/fl_utf8.h>
     @{ */
 /** @} */

#endif // !Fl_H
