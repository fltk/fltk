//
// "$Id$"
//
// Definition of Android screen interface
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2018 by Bill Spitzak and others.
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

/**
 \file Fl_Android_Screen_Driver.H
 \brief Definition of Android screen interface.
 */

#ifndef FL_ANDROID_SCREEN_DRIVER_H
#define FL_ANDROID_SCREEN_DRIVER_H

#include <FL/Fl_Screen_Driver.H>
#include <FL/Fl_Rect.H>
#include <android/input.h>
//#include <windows.h>

extern void (*fl_unlock_function)();
extern void (*fl_lock_function)();


class Fl_Window;


class FL_EXPORT Fl_Android_Screen_Driver : public Fl_Screen_Driver 
{
  // easy access to the super class
  typedef Fl_Screen_Driver super;

#if 0
  // No, this is not how we will implement this.
  enum {
    /// request attribute for a specific screen; default to screen 0
    INDEX = 0x0001,     // add an integer in the vararg list
    /// allow or lock screen rotation
    ROTATION_MASK = 0x0006, ROTATION_KEEP = 0x0000,
    MAY_ROTATE = 0x0006, LOCK_TO_PORTRAIT = 0x0002, LOCK_TO_LANDSCAPE = 0x0004,
    /// screen size
    SIZE_MASK = 0x0038, SIZE_KEEP = 0x0000,
    NATIVE_SIZE = 0x0008, // keep the full native screen size
    FIRST_WINDOW = 0x0010, // adapt the screen to the size of the first window
    ALL_WINDOWS = 0x0018, // adapt the screen to show all windows
    FIXED_DIAGONAL = 0x0020, // keep aspect ration at 1:1, add an int for the length of the diagonal
    // fixed size override the previous flags for portrait and/or landscape
    FIXED_PORTRAIT_SIZE = 0x0040, // add two int width and height
    NO_FIXED_PORTRAIT_SIZE = 0x80000040, // release fixed portrait size
    FIXED_LANDSCAPE_SIZE = 0x0080, // add two int width and height
    NO_FIXED_LANDSCAPE_SIZE = 0x80000080, // release fixed landscape size
    // show or hide mobile device screen items
    SHOW_STATUS_BAR = 0x0080, // top of the screen
    HIDE_STATUS_BAR = 0x80000080, // top of the screen
    SHOW_NAVIGATION_BAR = 0x0100, // bottom of the screen
    HIDE_NAVIGATION_BAR = 0x0100, // bottom of the screen
  };
  /// request some attributes from a screen that may or may not be met
  virtual hint(unsigned int flags, ...);
#endif


private:
  int handle_queued_events(double time_to_wait);
  int handle_app_command();
  int handle_input_event();
  int handle_keyboard_event(AInputQueue*, AInputEvent*);
  int handle_mouse_event(AInputQueue*, AInputEvent*);

public:
  Fl_Android_Screen_Driver();

  virtual void add_timeout(double time, Fl_Timeout_Handler cb, void *argp) override;
  virtual void repeat_timeout(double time, Fl_Timeout_Handler cb, void *argp) override;
  virtual int has_timeout(Fl_Timeout_Handler cb, void *argp) override;
  virtual void remove_timeout(Fl_Timeout_Handler cb, void *argp) override;

  virtual int compose(int &del) override;

  virtual void request_keyboard() override;
  virtual void release_keyboard() override;
  int pKeyboardCount = 0;

#if 0
  Fl_WinAPI_Screen_Driver() : Fl_Screen_Driver() {
    for (int i = 0; i < MAX_SCREENS; i++) scale_of_screen[i] = 1;
  }
  // --- display management
  virtual int visual(int flags);
  // --- screen configuration
  virtual void init();
  virtual int x();
  virtual int y();
  virtual int w();
  virtual int h();
  virtual void screen_xywh(int &X, int &Y, int &W, int &H, int n);
  virtual void screen_dpi(float &h, float &v, int n=0);
  int screen_num_unscaled(int x, int y);
  virtual void screen_work_area(int &X, int &Y, int &W, int &H, int n);
#endif
  // --- audible output
  virtual void beep(int type) override;
  // --- global events
  virtual void flush() override;
  virtual double wait(double time_to_wait) override;
#if 0
  virtual int ready();
  virtual void grab(Fl_Window* win);
  // --- global colors
  virtual void get_system_colors();
  virtual const char *get_system_scheme();
  // --- global timers
  virtual int dnd(int unused);
  virtual int compose(int &del);
  virtual Fl_RGB_Image *read_win_rectangle(int X, int Y, int w, int h);
  Fl_RGB_Image *read_win_rectangle_unscaled(int X, int Y, int w, int h);
  virtual int get_mouse(int &x, int &y);
  virtual void enable_im();
  virtual void disable_im();
  virtual void open_display_platform();
  virtual void offscreen_size(Fl_Offscreen off, int &width, int &height);
#if defined(FLTK_HIDPI_SUPPORT)  
  virtual APP_SCALING_CAPABILITY rescalable() {
    return PER_SCREEN_APP_SCALING;
  }
  virtual float scale(int n) {
    return scale_of_screen[n];
  }
  virtual void scale(int n, float f) {
    scale_of_screen[n] = f;
  }
#else
  float DWM_scaling_factor();
#endif
  virtual float desktop_scale_factor();

#endif
#if 0

  protected:
  RECT screens[MAX_SCREENS];
  RECT work_area[MAX_SCREENS];
  float dpi[MAX_SCREENS][2];
  float scale_of_screen[MAX_SCREENS];

  static BOOL CALLBACK screen_cb(HMONITOR mon, HDC, LPRECT r, LPARAM);
  BOOL screen_cb(HMONITOR mon, HDC, LPRECT r);
  int get_mouse_unscaled(int &mx, int &my);
#ifdef FLTK_HIDPI_SUPPORT
  void init_screen_scale_factors();
#endif

#endif
  bool pContentChanged;
  bool pClearDesktop;
};

#endif // FL_ANDROID_SCREEN_DRIVER_H

//
// End of "$Id$".
//
