//
// Private colormap support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

// Using the default system colormap can be a bad idea on PseudoColor
// visuals, since typically every application uses the default colormap and
// you can run out of colormap entries easily.
//
// The solution is to always create a new colormap on PseudoColor displays
// and copy the first 16 colors from the default colormap so that we won't
// get huge color changes when switching windows.

#include <FL/Fl.H>
#include "Fl_Screen_Driver.H"

/** \fn Fl::own_colormap()
 Makes FLTK use its <a href="fltk-colormap.png">own colormap</a>.  This may make FLTK display better
 and will reduce conflicts with other programs that want lots of colors.
 However the colors may flash as you move the cursor between windows.

 <P>This does nothing if the current visual is not colormapped.
 */
void Fl::own_colormap() {
  Fl::screen_driver()->own_colormap();
}
