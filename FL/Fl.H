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
#define Fl_H

#include <FL/fl_config.h>             // build configuration
#include <FL/Fl_Export.H>             // for FL_EXPORT
#include <FL/platform_types.h>        // for FL_SOCKET
#include <FL/fl_casts.H>              // type casting
#include <FL/core/function_types.H>   // widget callbacks and services
#include <FL/core/events.H>           // global event handling
#include <FL/core/options.H>          // system and application setting
#include <FL/core/pen_events.H>       // pen and tablet events
#include <FL/Fl_Widget_Tracker.H>     // historically included here

#ifdef FLTK_HAVE_CAIRO
# include <FL/Fl_Cairo.H>
#endif

#include "fl_utf8.h"
#include "Enumerations.H"
#ifndef Fl_Object
#  define Fl_Object Fl_Widget /**< for back compatibility - use Fl_Widget! */
#endif

#ifdef check
#  undef check
#endif

#ifdef BSD
#  undef BSD
#endif

#include <string.h> // FIXME: Fl::is_scheme(): strcmp needs string.h
#include <vector>

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

FL_EXPORT extern bool idle();

#ifndef FL_DOXYGEN
FL_EXPORT extern const char* scheme_;
FL_EXPORT extern Fl_Image* scheme_bg_;

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
typedef struct { double t; Fl_Timeout_Handler cb; void *data; } TimeoutData;
FL_EXPORT extern std::vector<TimeoutData> timeout_list();

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

FL_EXPORT extern bool box_bg(Fl_Boxtype);

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

/** See grab(Fl_Window*) */
FL_EXPORT inline void grab(Fl_Window& win) {grab(&win);}

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
FL_EXPORT extern void cairo_flush(cairo_t *c);

/** @} */

#endif // FLTK_HAVE_CAIRO

} // namespace Fl


/** \defgroup  fl_unicode  Unicode and UTF-8 functions
      fl global Unicode and UTF-8 handling functions declared in <FL/fl_utf8.h>
   @{ */
/** @} */

#endif // !Fl_H
