//
// Private header file for the Fast Light Tool Kit (FLTK).
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

/** \file src/Fl_Private.H
  \brief Fl::Private namespace.
*/

#ifndef Fl_Private_H
#define Fl_Private_H

#include <FL/fl_config.h> // build configuration
#include <FL/Fl.H>

/**
 FLTK private global variables and functions.
 */
namespace Fl {
namespace Private {

FL_EXPORT extern int use_high_res_GL_;
FL_EXPORT extern int draw_GL_text_with_textures_;
FL_EXPORT extern int box_shadow_width_;
FL_EXPORT extern int box_border_radius_max_;
FL_EXPORT extern int selection_to_clipboard_;

FL_EXPORT extern unsigned char options_[OPTION_LAST];
FL_EXPORT extern unsigned char options_read_;
FL_EXPORT extern int program_should_quit_; // non-zero means the program was asked to cleanly terminate

/**
  The currently executing idle callback function: DO NOT USE THIS DIRECTLY!

  This is now used as part of a higher level system allowing multiple
  idle callback functions to be called.
  \see add_idle(), remove_idle()
*/
FL_EXPORT extern void (*idle_)();

FL_EXPORT extern void run_idle();
FL_EXPORT extern void run_checks();

/**
  Sets an idle callback (internal use only).

  This method is now private and is used to store the idle callback.
  The old, public set_idle() method was deprecated since 1.3.x and
  is no longer available in FLTK 1.5.

  See documentation: use Fl::add_idle() instead.
*/
FL_EXPORT inline void set_idle_(Fl_Old_Idle_Handler cb) { idle_ = cb; }

#ifdef FLTK_HAVE_CAIRO

#if 0 // this non-public function appears not to be used anywhere in FLTK
  FL_EXPORT extern cairo_t *cairo_make_current(void *gc);
#endif
FL_EXPORT extern cairo_t *cairo_make_current(void *gc, int W, int H);
FL_EXPORT extern Fl_Cairo_State cairo_state_;

#endif // FLTK_HAVE_CAIRO

} // namespace Private
} // namespace Fl

#endif // !Fl_Private_H
