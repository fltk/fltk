//
// Scheme Choice widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022 by Bill Spitzak and others.
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
#include <FL/Fl_Window.H>
#include <FL/Fl_Scheme_Choice.H>


/**
  The constructor initializes the Fl_Scheme_Choice object with all known schemes.

  \param[in]  X,Y   Widget coordinates
  \param[in]  W,H   Widget size (width, height)
  \param[in]  L     Widget label (default: NULL, no label)
*/
Fl_Scheme_Choice::Fl_Scheme_Choice(int X, int Y, int W, int H, const char *L)
  : Fl_Choice(X, Y, W, H, L) {

  const char * const *names = Fl_Scheme::names();

  // Add all known schemes in the order defined by the list of scheme names
  while (*names) {
    add(*names);
    names++;
  }

  callback(scheme_cb_);   // internal callback
  init_value();           // set choice value to current scheme
}

/**
  Public method to initialize the value of the Fl_Scheme_Choice widget.

  Normally you don't need to call this unless you change the current scheme
  by calling Fl::scheme(const char *).

  The Fl_Scheme_Choice widget does this automatically when the widget is
  shown (when receiving the FL_SHOW event) which should always be after
  Fl_Window::show(argc, argv) which may set the current scheme by interpreting
  the commandline.

  \since 1.4.0
*/
void Fl_Scheme_Choice::init_value() {
  const char *current = Fl::scheme();

  value(0);
  if (!current)
    return;

  const char * const * names = Fl_Scheme::names();
  int i = 0;
  while (names[i]) {
    if (!strcmp(current, names[i])) {
      value(i);
      break;
    }
    i++;
  }
} // init_value()


/**
  Internal Fl_Scheme_Choice callback function (protected).

  You don't need to set a callback for this widget. The default callback
  changes the scheme (Fl::scheme()) and redraws all open windows.

  You may override the callback if changing the scheme shall redraw other
  windows or don't redraw the window at all.

  \param[in]  w   The Fl_Scheme_Choice widget
*/
void Fl_Scheme_Choice::scheme_cb_(Fl_Widget *w, void *) {
  Fl_Choice *c = reinterpret_cast<Fl_Choice *>(w);
  // set the new scheme only if the scheme was changed
  const char *new_scheme = c->text(c->value());
  if (!Fl::is_scheme(new_scheme)) {
    Fl::scheme(new_scheme);
  }
}

/**
  \brief Handle FLTK events.

  This widget uses FL_SHOW and some other events to initialize its value()
  according to the current scheme.

  All events are also handled by the base class Fl_Choice.

  \param[in]  event
  \return     1 if the event was used, 0 otherwise

  \internal
    Usually the FL_SHOW event is used to initialize the value, and this should
    in most cases be sufficient. However, if the scheme is changed after show()
    the widget doesn't "know" this and can't update itself. Therefore the enter
    and push events are also used to update the displayed value.

    In the future we will be able to register a callback that will be triggered
    when the scheme is changed. This will make the special handling of FL_PUSH
    and FL_ENTER obsolete, but FL_SHOW is still required.
*/
int Fl_Scheme_Choice::handle(int event) {
  int ret = 0;
  switch (event) {
    case FL_SHOW:
    case FL_PUSH:
    case FL_ENTER:
      init_value();
      ret = 1;
      break;
    default:
      break;
  }
  ret |= Fl_Choice::handle(event);
  return ret;
}
