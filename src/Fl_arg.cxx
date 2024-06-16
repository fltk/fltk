//
// Optional argument initialization code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

// OPTIONAL initialization code for a program using FLTK.
// You do not need to call this!  Feel free to make up your own switches.

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tooltip.H>
#include "Fl_Window_Driver.H"
#include "Fl_System_Driver.H"
#include "Fl_Screen_Driver.H"
#include <ctype.h>

static int fl_match(const char *a, const char *s, int atleast = 1) {
  const char *b = s;
  while (*a && (*a == *b || tolower(*a) == *b)) {a++; b++;}
  return !*a && b >= s+atleast;
}

// flags set by previously parsed arguments:
static char arg_called;
static char return_i;
static const char *name;
static const char *geometry;
static const char *title;
// these are in Fl_get_system_colors and are set by the switches:
extern const char *fl_fg;
extern const char *fl_bg;
extern const char *fl_bg2;

/**
  Parse a single switch from \p argv, starting at word \p i.
  Returns the number of words eaten (1 or 2, or 0 if it is not
  recognized) and adds the same value to \p i.

  This is the default argument handler used internally by Fl::args(...),
  but you can use this function if you prefer to step through the
  standard FLTK switches yourself.

  All standard FLTK switches except -bg2 may be abbreviated to just
  one letter and case is ignored:

  \li -bg color or -background color
  <br>
  Sets the background color using Fl::background().

  \li -bg2 color or -background2 color
  <br>
  Sets the secondary background color using Fl::background2().

  \li -display host:n.n
  <br>
  Sets the X display to use; this option is silently
  ignored under Windows and MacOS.

  \li -dnd and -nodnd
  <br>
  Enables or disables drag and drop text operations
  using Fl::dnd_text_ops().

  \li -fg color or -foreground color
  <br>
  Sets the foreground color using Fl::foreground().

  \li -geometry WxH+X+Y
  <br>
  Sets the initial window position and size according
  to the standard X geometry string.

  \li -iconic
  <br>
  Iconifies the window using Fl_Window::iconize().

  \li -kbd and -nokbd
  <br>
  Enables or disables visible keyboard focus for
  non-text widgets using Fl::visible_focus().

  \li -name string
  <br>
  Sets the window class using Fl_Window::xclass().

  \li -scheme string
  <br>
  Sets the widget scheme using Fl::scheme().

  \li -title string
  <br>
  Sets the window title using Fl_Window::label().

  \li -tooltips and -notooltips
  <br>
  Enables or disables tooltips using Fl_Tooltip::enable().

  Color values are commonly given as three digit or six digit hex numbers.
  - The order of fg, bg, and bg2 in the command line does not matter
  - There is no way at the moment to set the selection color.
  - Setting the bg2 color also changes the fg color to have sufficient contrast
  - Explicitly setting fg color overrides the bg2/contrast constraint.
  - Setting the bg color will update the color lookup table for the gray ramp,
    so color index values  can stay the same for all apps, it's just mapped to
    different RGB values.
  - The calculation of the gray ramp is only based on the bg color, so there is
    no way at the moment to create an inverted (dark mode) ramp.
  - Consequently, setting bg to black creates a an all-black ramp, setting a
    somewhat dark bg color creates a extremely dark ramp.
  - Setting the bg has no influence on bg2 or fg.

  If your program requires other switches in addition to the standard
  FLTK options, you will need to pass your own argument handler to
  Fl::args(int,char**,int&,Fl_Args_Handler) explicitly.

  \see \ref fl_parse_color(const char* p, uchar& r, uchar& g, uchar& b) to see how
  color values can be defined
*/
int Fl::arg(int argc, char **argv, int &i) {
  arg_called = 1;
  const char *s = argv[i];

  if (!s) {i++; return 1;}      // something removed by calling program?

  // a word that does not start with '-', or a word after a '--', or
  // the word '-' by itself all start the "non-switch arguments" to
  // a program.  Return 0 to indicate that we don't understand the
  // word, but set a flag (return_i) so that args() will return at
  // that point:
  if (s[0] != '-' || s[1] == '-' || !s[1]) {return_i = 1; return 0;}
  s++; // point after the dash

  if (fl_match(s, "iconic")) {
    Fl_Window::show_next_window_iconic(1);
    i++;
    return 1;
  } else if (fl_match(s, "kbd")) {
    Fl::visible_focus(1);
    i++;
    return 1;
  } else if (fl_match(s, "nokbd", 3)) {
    Fl::visible_focus(0);
    i++;
    return 1;
  } else if (fl_match(s, "dnd", 2)) {
    Fl::dnd_text_ops(1);
    i++;
    return 1;
  } else if (fl_match(s, "nodnd", 3)) {
    Fl::dnd_text_ops(0);
    i++;
    return 1;
  } else if (fl_match(s, "tooltips", 2)) {
    Fl_Tooltip::enable();
    i++;
    return 1;
  } else if (fl_match(s, "notooltips", 3)) {
    Fl_Tooltip::disable();
    i++;
    return 1;
  } else if (Fl::system_driver()->single_arg(s)) {
    i++;
    return 1;
  }

  const char *v = argv[i+1];
  if (i >= argc-1 || !v)
    return 0;   // all the rest need an argument, so if missing it is an error

  if (fl_match(s, "geometry")) {

    int flags, gx, gy; unsigned int gw, gh;
    flags = Fl::screen_driver()->XParseGeometry(v, &gx, &gy, &gw, &gh);
    if (!flags) return 0;
    geometry = v;

  } else if (fl_match(s, "display", 2)) {
    Fl::screen_driver()->display(v);

  } else if (Fl::system_driver()->arg_and_value(s, v)) {
    // nothing to do

  } else if (fl_match(s, "title", 2)) {
    title = v;

  } else if (fl_match(s, "name", 2)) {
    name = v;

  } else if (fl_match(s, "bg2", 3) || fl_match(s, "background2", 11)) {
    fl_bg2 = v;

  } else if (fl_match(s, "bg", 2) || fl_match(s, "background", 10)) {
    fl_bg = v;

  } else if (fl_match(s, "fg", 2) || fl_match(s, "foreground", 10)) {
    fl_fg = v;

  } else if (fl_match(s, "scheme", 1)) {
    Fl::scheme(v);

  } else {
    return 0; // unrecognized
  }

  i += 2;
  return 2;
}


/**
  Parse command line switches using the \p cb argument handler.

  Returns 0 on error, or the number of words processed.

  FLTK provides this as an <i>entirely optional</i> command line
  switch parser. You don't have to call it if you don't want to.
  Everything it can do can be done with other calls to FLTK.

  To use the switch parser, call Fl::args(...) near the start
  of your program.  This does \b not open the display, instead
  switches that need the display open are stashed into static
  variables. Then you \b must display your first window by calling
  <tt>window->show(argc,argv)</tt>, which will do anything stored
  in the static variables.

  Providing an argument handler callback \p cb lets you define
  your own switches. It is called with the same \p argc and \p argv,
  and with \p i set to the index of the switch to be processed.
  The \p cb handler should return zero if the switch is unrecognized,
  and not change \p i. It should return non-zero to indicate the
  number of words processed if the switch is recognized, i.e. 1 for
  just the switch, and more than 1 for the switch plus associated
  parameters. \p i should be incremented by the same amount.

  The \p cb handler is called \b before any other tests, so
  <i>you can also override any standard FLTK switch</i>
  (this is why FLTK can use very short switches instead of
  the long ones all other toolkits force you to use).
  See Fl::arg() for descriptions of the standard switches.

  On return \p i is set to the index of the first non-switch.
  This is either:

  \li The first word that does not start with '-'.
  \li The word '-' (used by many programs to name stdin as a file)
  \li The first unrecognized switch (return value is 0).
  \li \p argc

  The return value is \p i unless an unrecognized switch is found,
  in which case it is zero. If your program takes no arguments other
  than switches you should produce an error if the return value is less
  than \p argc.


  A usage string is displayed if Fl::args() detects an invalid argument
  on the command-line. You can change the message by setting the
  Fl::help pointer.

  A very simple command line parser can be found in <tt>examples/howto-parse-args.cxx</tt>

  The simpler Fl::args(int argc, char **argv) form is useful if your program
  does not have command line switches of its own.
*/

int Fl::args(int argc, char** argv, int& i, Fl_Args_Handler cb) {
  arg_called = 1;
  i = 1; // skip argv[0]
  while (i < argc) {
    if (cb && cb(argc,argv,i)) continue;
    if (!arg(argc,argv,i)) return return_i ? i : 0;
  }
  return i;
}

// show a main window, use any parsed arguments
void Fl_Window::show(int argc, char **argv) {
  if (argc && !arg_called) Fl::args(argc,argv);

  Fl::get_system_colors();

  pWindowDriver->show_with_args_begin();

  // note: background_pixel is no longer used since 1.4.0, but anyway ...
  // set colors first, so background_pixel is correct:
  static char beenhere = 0;
  if (!beenhere) {
    if (geometry) {
      int fl = 0, gx = x(), gy = y(); unsigned int gw = w(), gh = h();
      fl = Fl::screen_driver()->XParseGeometry(geometry, &gx, &gy, &gw, &gh);
      if (fl & Fl_Screen_Driver::fl_XNegative) gx = Fl::w()-w()+gx;
      if (fl & Fl_Screen_Driver::fl_YNegative) gy = Fl::h()-h()+gy;
      //  int mw,mh; minsize(mw,mh);
      //  if (mw > gw) gw = mw;
      //  if (mh > gh) gh = mh;
      Fl_Widget *r = resizable();
      if (!r) resizable(this);
      // for Windows we assume window is not mapped yet:
      if (fl & (Fl_Screen_Driver::fl_XValue | Fl_Screen_Driver::fl_YValue))
        x(-1), resize(gx,gy,gw,gh);
      else
        size(gw,gh);
      resizable(r);
    }
  }

  // set the class, which is used by X version of get_system_colors:
  if (name) {xclass(name); name = 0;}
  else if (!xclass() || !strcmp(xclass(),"FLTK")) xclass(fl_filename_name(argv[0]));

  if (title) {label(title); title = 0;}
  else if (!label()) label(xclass());

  if (!beenhere) {
    beenhere = 1;
    Fl::scheme(Fl::scheme()); // opens display!  May call Fl::fatal()
  }

  // Show the window AFTER we have set the colors and scheme.
  show();

  pWindowDriver->show_with_args_end(argc, argv);
}

// Calls useful for simple demo programs, with automatic help message:

static const char * const helpmsg =
"options are:\n"
" -bg2 color\n"
" -bg color\n"
" -di[splay] host:n.n\n"
" -dn[d]\n"
" -fg color\n"
" -g[eometry] WxH+X+Y\n"
" -i[conic]\n"
" -k[bd]\n"
" -na[me] classname\n"
" -nod[nd]\n"
" -nok[bd]\n"
" -not[ooltips]\n"
" -s[cheme] scheme\n"
" -ti[tle] windowtitle\n"
" -to[oltips]";

const char * const Fl::help = helpmsg+13;

/**
 Parse all command line switches matching standard FLTK options only.

 It parses all the switches, and if any are not recognized it calls
 Fl::abort(Fl::help), i.e. unlike the long form, an unrecognized
 switch generates an error message and causes the program to exit.

 */
void Fl::args(int argc, char **argv) {
  int i; if (Fl::args(argc,argv,i) < argc) Fl::error(helpmsg);
}
