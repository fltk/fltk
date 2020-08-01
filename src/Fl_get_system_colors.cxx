//
// System color support for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl.H>
#include "Fl_Screen_Driver.H"
#include "Fl_System_Driver.H"
#include <FL/fl_draw.H>
#include <FL/platform.H>
#include <FL/math.h>
#include <FL/fl_utf8.h>
#include <FL/fl_string.h>
#include "flstring.h"
#include <stdio.h>
#include <stdlib.h>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Tiled_Image.H>
#include "tile.xpm"

/**
    Changes fl_color(FL_BACKGROUND_COLOR) to the given color,
    and changes the gray ramp from 32 to 56 to black to white.  These are
    the colors used as backgrounds by almost all widgets and used to draw
    the edges of all the boxtypes.
*/
void Fl::background(uchar r, uchar g, uchar b) {
  Fl_Screen_Driver::bg_set = 1;

  // replace the gray ramp so that FL_GRAY is this color
  if (!r) r = 1; else if (r==255) r = 254;
  double powr = log(r/255.0)/log((FL_GRAY-FL_GRAY_RAMP)/(FL_NUM_GRAY-1.0));
  if (!g) g = 1; else if (g==255) g = 254;
  double powg = log(g/255.0)/log((FL_GRAY-FL_GRAY_RAMP)/(FL_NUM_GRAY-1.0));
  if (!b) b = 1; else if (b==255) b = 254;
  double powb = log(b/255.0)/log((FL_GRAY-FL_GRAY_RAMP)/(FL_NUM_GRAY-1.0));
  for (int i = 0; i < FL_NUM_GRAY; i++) {
    double gray = i/(FL_NUM_GRAY-1.0);
    Fl::set_color(fl_gray_ramp(i),
                  uchar(pow(gray,powr)*255+.5),
                  uchar(pow(gray,powg)*255+.5),
                  uchar(pow(gray,powb)*255+.5));
  }
}


/** Changes fl_color(FL_FOREGROUND_COLOR). */
void Fl::foreground(uchar r, uchar g, uchar b) {
  Fl_Screen_Driver::fg_set = 1;

  Fl::set_color(FL_FOREGROUND_COLOR,r,g,b);
}


/**
    Changes the alternative background color. This color is used as a
    background by Fl_Input and other text widgets.
    <P>This call may change fl_color(FL_FOREGROUND_COLOR) if it
    does not provide sufficient contrast to FL_BACKGROUND2_COLOR.
*/
void Fl::background2(uchar r, uchar g, uchar b) {
  Fl_Screen_Driver::bg2_set = 1;

  Fl::set_color(FL_BACKGROUND2_COLOR,r,g,b);
  Fl::set_color(FL_FOREGROUND_COLOR,
                get_color(fl_contrast(FL_FOREGROUND_COLOR,FL_BACKGROUND2_COLOR)));
}


// these are set by Fl::args() and override any system colors:
const char *fl_fg = NULL;
const char *fl_bg = NULL;
const char *fl_bg2 = NULL;


int fl_parse_color(const char* p, uchar& r, uchar& g, uchar& b) {
  return Fl::screen_driver()->parse_color(p, r, g, b);
}


/** \fn Fl::get_system_colors()
    Read the user preference colors from the system and use them to call
    Fl::foreground(), Fl::background(), and Fl::background2().

    This is done by Fl_Window::show(argc,argv) before applying
    the -fg and -bg switches.

    On X this reads some common values from the Xdefaults database.
    KDE users can set these values by running the "krdb" program, and
    newer versions of KDE set this automatically if you check the "apply
    style to other X programs" switch in their control panel.
*/
void Fl::get_system_colors()
{
  Fl::screen_driver()->get_system_colors();
}


//// Simple implementation of 2.0 Fl::scheme() interface...
#define D1 BORDER_WIDTH
#define D2 (BORDER_WIDTH+BORDER_WIDTH)

extern void     fl_up_box(int, int, int, int, Fl_Color);
extern void     fl_down_box(int, int, int, int, Fl_Color);
extern void     fl_thin_up_box(int, int, int, int, Fl_Color);
extern void     fl_thin_down_box(int, int, int, int, Fl_Color);
extern void     fl_round_up_box(int, int, int, int, Fl_Color);
extern void     fl_round_down_box(int, int, int, int, Fl_Color);

extern void     fl_up_frame(int, int, int, int, Fl_Color);
extern void     fl_down_frame(int, int, int, int, Fl_Color);
extern void     fl_thin_up_frame(int, int, int, int, Fl_Color);
extern void     fl_thin_down_frame(int, int, int, int, Fl_Color);

#ifndef FL_DOXYGEN
const char      *Fl::scheme_ = (const char *)0;     // current scheme
Fl_Image        *Fl::scheme_bg_ = (Fl_Image *)0;    // current background image for the scheme
#endif

static Fl_Pixmap        tile(tile_xpm);


/**
    Sets the current widget scheme. NULL will use the scheme defined
    in the FLTK_SCHEME environment variable or the scheme resource
    under X11. Otherwise, any of the following schemes can be used:

        - "none" - This is the default look-n-feel which resembles old
                   Windows (95/98/Me/NT/2000) and old GTK/KDE

        - "base" - This is an alias for "none"

        - "plastic" - This scheme is inspired by the Aqua user interface
                      on Mac OS X

        - "gtk+" - This scheme is inspired by the Red Hat Bluecurve theme

        - "gleam" - This scheme is inspired by the Clearlooks Glossy scheme.
                    (Colin Jones and Edmanuel Torres).

    Uppercase scheme names are equivalent, but the stored scheme name will
    always be lowercase and Fl::scheme() will return this lowercase name.

    If the resulting scheme name is not defined, the default scheme will
    be used and Fl::scheme() will return NULL.

    \see Fl::is_scheme()
*/
int Fl::scheme(const char *s) {
  if (!s) {
    s = screen_driver()->get_system_scheme();
  }

  if (s) {
    if (!fl_ascii_strcasecmp(s, "none") || !fl_ascii_strcasecmp(s, "base") || !*s) s = 0;
    else if (!fl_ascii_strcasecmp(s, "gtk+")) s = fl_strdup("gtk+");
    else if (!fl_ascii_strcasecmp(s, "plastic")) s = fl_strdup("plastic");
    else if (!fl_ascii_strcasecmp(s, "gleam")) s = fl_strdup("gleam");
    else s = 0;
  }
  if (scheme_) free((void*)scheme_);
  scheme_ = s;

  // Save the new scheme in the FLTK_SCHEME env var so that child processes
  // inherit it...
  static char e[1024];
  strcpy(e,"FLTK_SCHEME=");
  if (s) strlcat(e,s,sizeof(e));
  Fl::system_driver()->putenv(e);

  // Load the scheme...
  return reload_scheme();
}


int Fl::reload_scheme() {
  Fl_Window *win;

  if (scheme_ && !fl_ascii_strcasecmp(scheme_, "plastic")) {
    // Update the tile image to match the background color...
    uchar r, g, b;
    int nr, ng, nb;
    int i;
//    static uchar levels[3] = { 0xff, 0xef, 0xe8 };
    // OSX 10.3 and higher use a background with less contrast...
    static uchar levels[3] = { 0xff, 0xf8, 0xf4 };

    get_color(FL_GRAY, r, g, b);

//    printf("FL_GRAY = 0x%02x 0x%02x 0x%02x\n", r, g, b);

    for (i = 0; i < 3; i ++) {
      nr = levels[i] * r / 0xe8;
      if (nr > 255) nr = 255;

      ng = levels[i] * g / 0xe8;
      if (ng > 255) ng = 255;

      nb = levels[i] * b / 0xe8;
      if (nb > 255) nb = 255;

      sprintf(tile_cmap[i], "%c c #%02x%02x%02x", "Oo."[i], nr, ng, nb);
//      puts(tile_cmap[i]);
    }

    tile.uncache();

    if (!scheme_bg_) scheme_bg_ = new Fl_Tiled_Image(&tile, 0, 0);

    // Load plastic buttons, etc...
    set_boxtype(FL_UP_FRAME,        FL_PLASTIC_UP_FRAME);
    set_boxtype(FL_DOWN_FRAME,      FL_PLASTIC_DOWN_FRAME);
    set_boxtype(FL_THIN_UP_FRAME,   FL_PLASTIC_UP_FRAME);
    set_boxtype(FL_THIN_DOWN_FRAME, FL_PLASTIC_DOWN_FRAME);

    set_boxtype(FL_UP_BOX,          FL_PLASTIC_UP_BOX);
    set_boxtype(FL_DOWN_BOX,        FL_PLASTIC_DOWN_BOX);
    set_boxtype(FL_THIN_UP_BOX,     FL_PLASTIC_THIN_UP_BOX);
    set_boxtype(FL_THIN_DOWN_BOX,   FL_PLASTIC_THIN_DOWN_BOX);
    set_boxtype(_FL_ROUND_UP_BOX,   FL_PLASTIC_ROUND_UP_BOX);
    set_boxtype(_FL_ROUND_DOWN_BOX, FL_PLASTIC_ROUND_DOWN_BOX);

    // Use standard size scrollbars...
    Fl::scrollbar_size(16);
  } else if (scheme_ && !fl_ascii_strcasecmp(scheme_, "gtk+")) {
    // Use a GTK+ inspired look-n-feel...
    if (scheme_bg_) {
      delete scheme_bg_;
      scheme_bg_ = (Fl_Image *)0;
    }

    set_boxtype(FL_UP_FRAME,        FL_GTK_UP_FRAME);
    set_boxtype(FL_DOWN_FRAME,      FL_GTK_DOWN_FRAME);
    set_boxtype(FL_THIN_UP_FRAME,   FL_GTK_THIN_UP_FRAME);
    set_boxtype(FL_THIN_DOWN_FRAME, FL_GTK_THIN_DOWN_FRAME);

    set_boxtype(FL_UP_BOX,          FL_GTK_UP_BOX);
    set_boxtype(FL_DOWN_BOX,        FL_GTK_DOWN_BOX);
    set_boxtype(FL_THIN_UP_BOX,     FL_GTK_THIN_UP_BOX);
    set_boxtype(FL_THIN_DOWN_BOX,   FL_GTK_THIN_DOWN_BOX);
    set_boxtype(_FL_ROUND_UP_BOX,   FL_GTK_ROUND_UP_BOX);
    set_boxtype(_FL_ROUND_DOWN_BOX, FL_GTK_ROUND_DOWN_BOX);

    // Use slightly thinner scrollbars...
    Fl::scrollbar_size(15);
  } else if (scheme_ && !fl_ascii_strcasecmp(scheme_, "gleam")) {
    // Use a GTK+ inspired look-n-feel...
    if (scheme_bg_) {
      delete scheme_bg_;
      scheme_bg_ = (Fl_Image *)0;
    }

    set_boxtype(FL_UP_FRAME,        FL_GLEAM_UP_FRAME);
    set_boxtype(FL_DOWN_FRAME,      FL_GLEAM_DOWN_FRAME);
    set_boxtype(FL_THIN_UP_FRAME,   FL_GLEAM_UP_FRAME);
    set_boxtype(FL_THIN_DOWN_FRAME, FL_GLEAM_DOWN_FRAME);

    set_boxtype(FL_UP_BOX,          FL_GLEAM_UP_BOX);
    set_boxtype(FL_DOWN_BOX,        FL_GLEAM_DOWN_BOX);
    set_boxtype(FL_THIN_UP_BOX,     FL_GLEAM_THIN_UP_BOX);
    set_boxtype(FL_THIN_DOWN_BOX,   FL_GLEAM_THIN_DOWN_BOX);
    set_boxtype(_FL_ROUND_UP_BOX,   FL_GLEAM_ROUND_UP_BOX);
    set_boxtype(_FL_ROUND_DOWN_BOX, FL_GLEAM_ROUND_DOWN_BOX);

    // Use slightly thinner scrollbars...
    Fl::scrollbar_size(15);
  } else {
    // Use the standard FLTK look-n-feel...
    if (scheme_bg_) {
      delete scheme_bg_;
      scheme_bg_ = (Fl_Image *)0;
    }

    set_boxtype(FL_UP_FRAME,        fl_up_frame, D1, D1, D2, D2);
    set_boxtype(FL_DOWN_FRAME,      fl_down_frame, D1, D1, D2, D2);
    set_boxtype(FL_THIN_UP_FRAME,   fl_thin_up_frame, 1, 1, 2, 2);
    set_boxtype(FL_THIN_DOWN_FRAME, fl_thin_down_frame, 1, 1, 2, 2);

    set_boxtype(FL_UP_BOX,          fl_up_box, D1, D1, D2, D2);
    set_boxtype(FL_DOWN_BOX,        fl_down_box, D1, D1, D2, D2);
    set_boxtype(FL_THIN_UP_BOX,     fl_thin_up_box, 1, 1, 2, 2);
    set_boxtype(FL_THIN_DOWN_BOX,   fl_thin_down_box, 1, 1, 2, 2);
    set_boxtype(_FL_ROUND_UP_BOX,   fl_round_up_box, 3, 3, 6, 6);
    set_boxtype(_FL_ROUND_DOWN_BOX, fl_round_down_box, 3, 3, 6, 6);

    // Use standard size scrollbars...
    Fl::scrollbar_size(16);
  }

  // Set (or clear) the background tile for all windows...

  // FIXME: This makes it impossible to assign a background image
  // and/or a label to a window. IMHO we should be able to assign a
  // background image to a window. Currently (as of FLTK 1.3.3) there
  // is the workaround to use a group inside the window to achieve this.
  // See also STR #3075.
  // AlbrechtS, 01 Mar 2015
  //
  // If there is already an image assigned that is not the scheme_bg_,
  // then don't change the labeltype or assign another image. Will that
  // fix it?

  for (win = first_window(); win; win = next_window(win)) {
    win->labeltype(scheme_bg_ ? FL_NORMAL_LABEL : FL_NO_LABEL);
    win->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    win->image(scheme_bg_);
    win->redraw();
  }

  return 1;
}
