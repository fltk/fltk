//
// OpenGL overlay code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

#include <config.h>
#if HAVE_GL

#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>
#include "Fl_Gl_Window_Driver.H"
#include <stdlib.h>

/**
 Returns true if the hardware overlay is possible.  If this is false,
 FLTK will try to simulate the overlay, with significant loss of update
 speed.  Calling this will cause FLTK to open the display.
 */
int Fl_Gl_Window::can_do_overlay() {
  return pGlWindowDriver->can_do_overlay();
}

/**
 * @cond DriverDev
 * @addtogroup DriverDeveloper
 * @{
 */

void Fl_Gl_Window_Driver::make_overlay(void *&o) {
  o = pWindow;
}

/**
 * @}
 * @endcond
 */

/**
 Causes draw_overlay() to be called at a later time.
 Initially the overlay is clear. If you want the window to display
 something in the overlay when it first appears, you must call this
 immediately after you show() your window.
 */
void Fl_Gl_Window::redraw_overlay() {
  if (!shown()) return;
  pGlWindowDriver->make_overlay(overlay);
  pGlWindowDriver->redraw_overlay();
}

/**
 Selects the OpenGL context for the widget's overlay.
 This method is called automatically prior to the
 draw_overlay() method being called and can also be used to
 implement feedback and/or selection within the handle()
 method.
 */
void Fl_Gl_Window::make_overlay_current() {
  pGlWindowDriver->make_overlay(overlay);
  pGlWindowDriver->make_overlay_current();
}

/** Hides the window if it is not this window, does nothing in Windows. */
void Fl_Gl_Window::hide_overlay() {
  pGlWindowDriver->hide_overlay();
}

#endif // HAVE_GL
