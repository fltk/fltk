//
// "$Id$"
//
// All screen related calls in a driver style class.
//
// Copyright 1998-2017 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#ifndef FL_SCREEN_DRIVER_H
#define FL_SCREEN_DRIVER_H

#include <FL/Fl_Device.H>
#include <FL/fl_types.h>
#include <FL/Fl.H> // for Fl_Timeout_Handler
#include <FL/Fl_Text_Editor.H>


// TODO: add text composition?
// TODO: add Fl::display
// TODO: add copy/paste, drag/drop?
// TODO: get key/get mouse?
// TODO: system colors/colormaps
// TODO: system menu?
// TODO: native filechooser
// TODO: native message boxes
// TODO: read screen to image
// TODO: application shortcuts

class Fl_Window;
class Fl_RGB_Image;
class Fl_Group;
class Fl_Input;

/** A base class describing the interface between FLTK and screen-related operations.
This class is only for internal use by the FLTK library.
Each supported platform implements several of the virtual methods of this class.
*/
class FL_EXPORT Fl_Screen_Driver {

protected:
  Fl_Screen_Driver();
  virtual ~Fl_Screen_Driver();

  static const int MAX_SCREENS = 16;

  int num_screens;
  static  float fl_intersection(int x1, int y1, int w1, int h1,
                                      int x2, int y2, int w2, int h2);

public:
  static char bg_set;
  static char bg2_set;
  static char fg_set;

public:
  virtual float scale(int n) {return 1;}
  virtual void scale(int n, float f) { }
  static Fl_Screen_Driver *newScreenDriver();
  // --- display management
  virtual void display(const char *disp);
  virtual int visual(int flags);
  // --- screen configuration
  virtual void init() = 0;
  virtual int x() = 0;
  virtual int y() = 0;
  virtual int w() = 0;
  virtual int h() = 0;
  virtual int screen_count();
  virtual void screen_xywh(int &X, int &Y, int &W, int &H);
  virtual void screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my);
  virtual void screen_xywh(int &X, int &Y, int &W, int &H, int n) = 0;
  virtual void screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my, int mw, int mh);
  virtual int screen_num(int x, int y);
  virtual int screen_num(int x, int y, int w, int h);
  virtual void screen_dpi(float &h, float &v, int n=0) = 0;
  virtual void screen_work_area(int &X, int &Y, int &W, int &H, int mx, int my);
  virtual void screen_work_area(int &X, int &Y, int &W, int &H, int n) = 0;
  virtual void screen_work_area(int &X, int &Y, int &W, int &H);
  // --- audible output
  virtual void beep(int type) = 0;
  // --- global events
  virtual void flush() = 0;
  virtual double wait(double time_to_wait) = 0;
  virtual int ready() = 0;
  virtual void grab(Fl_Window* win) = 0;
  // --- global colors
  virtual int parse_color(const char* p, uchar& r, uchar& g, uchar& b) = 0;
  virtual void get_system_colors() = 0;
  virtual const char *get_system_scheme();
  // --- global timers
  virtual void add_timeout(double time, Fl_Timeout_Handler cb, void *argp) = 0;
  virtual void repeat_timeout(double time, Fl_Timeout_Handler cb, void *argp) = 0;
  virtual int has_timeout(Fl_Timeout_Handler cb, void *argp) = 0;
  virtual void remove_timeout(Fl_Timeout_Handler cb, void *argp) = 0;

  static int secret_input_character;
  /* Implement to indicate whether complex text input may involve marked text.
   When it does, has_marked_text returns non zero and reset_marked_text() and
   insertion_point_location() must also be implemented.
   */
  virtual int has_marked_text() { return 0; }
  virtual void reset_marked_text() {}
  virtual void insertion_point_location(int x, int y, int height) {}
  // implement so text-editing widgets support dead keys
  virtual int compose(int &del) {del = 0; return 0;}
  // default implementation may be enough
  virtual void compose_reset();
  // implement to support drag-n-drop. use_selection = 1 means the GUI is welcome to display
  // the selected text during the D&D operation
  virtual int dnd(int use_selection = 0) {return 0;}
  // null means no platform-specific key bindings for Fl_Text_Editor
  Fl_Text_Editor::Key_Binding *text_editor_extra_key_bindings;
  // default implementation may be enough
  virtual int text_display_can_leak() { return 0; }

  // read raw image from a window or an offscreen buffer
#if defined(FL_PORTING)
#  pragma message "FL_PORTING: implement code to read RGB data from screen"
#endif
  /* Both member functions read_image() and read_win_rectangle() support
   the public function fl_read_image() which captures pixel data either from
   the current window or from an offscreen buffer.

   A platform re-implements either read_image() or read_win_rectangle().
   In the 1st case and for capture from a window, the returned pixel array 
   also contains data from any embedded sub-window.
   In the 2nd case and for capture from a window, only data from the current 
   window is collected, and read_image()'s default implementation captures 
   pixels from any subwindow.
   
   A platform may also use its read_win_rectangle() implementation to capture
   window decorations (e.g., title bar). In that case, it is called by
   Fl_XXX_Window_Driver::capture_titlebar_and_borders().
   */
  virtual uchar *read_image(uchar *p, int x, int y, int w, int h, int alpha);
  virtual Fl_RGB_Image *read_win_rectangle(uchar *p, int X, int Y, int w, int h, int alpha) {return NULL;}
  static void write_image_inside(Fl_RGB_Image *to, Fl_RGB_Image *from, int to_x, int to_y);
  static Fl_RGB_Image *traverse_to_gl_subwindows(Fl_Group *g, uchar *p, int x, int y, int w, int h, int alpha,
                                                 Fl_RGB_Image *full_img);
  // optional platform-specific key handling for Fl_Input widget
  // the default implementation may be enough
  virtual int input_widget_handle_key(int key, unsigned mods, unsigned shift, Fl_Input *input);
  // implement to support Fl::get_mouse()
  virtual void get_mouse(int &x, int &y) {}
  // optional methods to enable/disable input methods for complex scripts
  virtual void enable_im() {}
  virtual void disable_im() {}
  // implement to open access to the display
  virtual void open_display() {}
  // optional method to close display access
  virtual void close_display() {}
  // compute dimensions of an Fl_Offscreen
  virtual void offscreen_size(Fl_Offscreen off, int &width, int &height) {}
  
  void rescale_all_windows_from_screen(int screen, float f);
  static void transient_scale_display(float f, int nscreen);
  static int scale_handler(int event);
  virtual void init_workarea() {}
  virtual float desktop_scale_factor() {return 1;}
  float default_scale_factor();
  enum APP_SCALING_CAPABILITY {
    NO_APP_SCALING = 0, ///< The platform does not support rescaling.
    SYSTEMWIDE_APP_SCALING, ///< The platform supports rescaling with the same factor for all screens.
    PER_SCREEN_APP_SCALING ///< The platform supports rescaling with one factor for each screen.
  };
  /** Returns the platform's support for rescaling the application with ctrl-/+/-/0/ keys.
   */
  virtual APP_SCALING_CAPABILITY rescalable() { return NO_APP_SCALING; }
};


#endif // !FL_SCREEN_DRIVER_H

//
// End of "$Id$".
//
