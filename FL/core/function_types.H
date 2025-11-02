//
// Function types for the Fast Light Tool Kit (FLTK).
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

/** \file FL/core/function_types.H
  \brief FLTK typedefs for service and widget callbacks.
*/

#ifndef Fl_core_function_types_H
#define Fl_core_function_types_H


#include <FL/fl_config.h>             // build configuration
#include <FL/Fl_Export.H>             // for FL_EXPORT
#include <FL/platform_types.h>        // for FL_SOCKET
#include <FL/Enumerations.H>          // for Fl_Align, Fl_Color, etc.

class Fl_Window;
struct Fl_Label;


/** \defgroup  callback_functions  Callback Function Typedefs

  \brief Typedefs defined in <FL/core/function_types.H> for callback or handler functions passed as function parameters.

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

//
// Drawing and Rendering Function Types
//

/** Signature of some label drawing functions passed as parameters */
typedef void (Fl_Label_Draw_F)(const Fl_Label *label, int x, int y, int w, int h, Fl_Align align);

/** Signature of some label measurement functions passed as parameters */
typedef void (Fl_Label_Measure_F)(const Fl_Label *label, int &width, int &height);

/** Signature of some box drawing functions passed as parameters */
typedef void (Fl_Box_Draw_F)(int x, int y, int w, int h, Fl_Color color);

/** Signature of box focus frame drawing functions */
typedef void (Fl_Box_Draw_Focus_F)(Fl_Boxtype bt, int x, int y, int w, int h, Fl_Color fg, Fl_Color bg);

//
// Timer and Scheduling Function Types
//

/** Signature of timeout callback functions passed as parameters.
    Please see Fl::add_timeout() for details.
*/
typedef void (*Fl_Timeout_Handler)(void *data);

/** Signature of some wakeup callback functions passed as parameters */
typedef void (*Fl_Awake_Handler)(void *data);

//
// Idle Processing Function Types
//

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

//
// File Descriptor and I/O Function Types
//

/** Signature of add_fd functions passed as parameters */
typedef void (*Fl_FD_Handler)(FL_SOCKET fd, void *data);

//
// Event Handling Function Types
//

/** Signature of add_handler functions passed as parameters */
typedef int (*Fl_Event_Handler)(int event);

/** Signature of add_system_handler functions passed as parameters */
typedef int (*Fl_System_Handler)(void *event, void *data);

/** Signature of event_dispatch functions passed as parameters.
    \see Fl::event_dispatch(Fl_Event_Dispatch) */
typedef int (*Fl_Event_Dispatch)(int event, Fl_Window *w);

//
// Application Lifecycle Function Types
//

/** Signature of set_abort functions passed as parameters */
typedef void (*Fl_Abort_Handler)(const char *format,...);

/** Signature of set_atclose functions passed as parameters */
typedef void (*Fl_Atclose_Handler)(Fl_Window *window, void *data);

/** Signature of args functions passed as parameters */
typedef int (*Fl_Args_Handler)(int argc, char **argv, int &i);

//
// Clipboard and Data Transfer Function Types
//

/** Signature of add_clipboard_notify functions passed as parameters */
typedef void (*Fl_Clipboard_Notify_Handler)(int source, void *data);

/** @} */ /* group callback_functions */


#endif // !Fl_core_function_types_H
