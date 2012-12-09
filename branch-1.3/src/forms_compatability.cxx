//
// "$Id$"
//
// Forms compatibility functions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

// Forms library compatibility functions.
// Many more functions are defined as inlines in forms.h!

#include <FL/forms.H>
#include <stdlib.h>

char fl_flip = 2;
void fl_end_form() {
  while (Fl_Group::current()) Fl_Group::current()->forms_end();
}
void Fl_Group::forms_end() {
  // set the dimensions of a group to surround contents
  if (children() && !w()) {
    Fl_Widget*const* a = array();
    Fl_Widget* o = *a++;
    int rx = o->x();
    int ry = o->y();
    int rw = rx+o->w();
    int rh = ry+o->h();
    for (int i=children_-1; i--;) {
      o = *a++;
      if (o->x() < rx) rx = o->x();
      if (o->y() < ry) ry = o->y();
      if (o->x()+o->w() > rw) rw = o->x()+o->w();
      if (o->y()+o->h() > rh) rh = o->y()+o->h();
    }
    x(rx);
    y(ry);
    w(rw-rx);
    h(rh-ry);
  }
  // flip all the children's coordinate systems:
  if (fl_flip) {
    Fl_Widget* o = (type()>=FL_WINDOW) ? this : window();
    int Y = o->h();
    Fl_Widget*const* a = array();
    for (int i=children(); i--;) {
      Fl_Widget* ow = *a++;
      int newy = Y-ow->y()-ow->h();
      ow->y(newy);
    }
  }
  end();
}

static int initargc;
static char **initargv;

void fl_initialize(int *argc, char **argv, const char *, FL_CMD_OPT *, int) {
  initargc = *argc;
  initargv = new char*[*argc+1];
  int i,j;
  for (i=0; i<=*argc; i++) initargv[i] = argv[i];
  for (i=j=1; i<*argc; ) {
    if (Fl::arg(*argc,argv,i));
    else argv[j++] = argv[i++];
  }
  argv[j] = 0;
  *argc = j;
  if (fl_flip==2) fl_flip = 0;
}

char fl_modal_next; // set by fl_freeze_forms()

void fl_show_form(Fl_Window *f,int place,int b,const char *n) {

  f->label(n);
  if (!b) f->clear_border();
  if (fl_modal_next || b==FL_TRANSIENT) {f->set_modal(); fl_modal_next = 0;}

  if (place & FL_PLACE_MOUSE) f->hotspot(f);

  if (place & FL_PLACE_CENTER) {
    int scr_x, scr_y, scr_w, scr_h;
    Fl::screen_xywh(scr_x, scr_y, scr_w, scr_h);
    f->position(scr_x+(scr_w-f->w())/2, scr_y+(scr_h-f->h())/2);
  }

  if (place & FL_PLACE_FULLSCREEN)
    f->fullscreen();

  if (place & (FL_PLACE_POSITION | FL_PLACE_GEOMETRY))
    f->position(
      (f->x() < 0) ? Fl::w()-f->w()+f->x()-1 : f->x(),
      (f->y() < 0) ? Fl::h()-f->h()+f->y()-1 : f->y());

// if (place & FL_PLACE_ASPECT) {
// this is not yet implemented
// it can be done by setting size_range().

  if (place == FL_PLACE_FREE || place == FL_PLACE_SIZE)
    f->free_position();

  if (place == FL_PLACE_FREE || place & FL_FREE_SIZE)
    if (!f->resizable()) f->resizable(f);

  if (initargc) {f->show(initargc,initargv); initargc = 0;}
  else f->show();
}

Fl_Widget *fl_do_forms(void) {
  Fl_Widget *obj;
  while (!(obj = Fl::readqueue())) if (!Fl::wait()) exit(0);
  return obj;
}

Fl_Widget *fl_check_forms() {
  Fl::check();
  return Fl::readqueue();
}

void fl_set_graphics_mode(int /*r*/,int /*d*/) {}

#ifndef FL_DOXYGEN // FIXME: suppress doxygen warning
void Fl_FormsText::draw() {
  draw_box();
  align(align()|FL_ALIGN_INSIDE); // questionable method of compatibility
  draw_label();
}
#endif

// Create a forms button by selecting correct fltk subclass:

#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Repeat_Button.H>

Fl_Button *fl_add_button(uchar t,int x,int y,int w,int h,const char *l) {
  Fl_Button *b;
  switch (t) {
  case FL_RETURN_BUTTON:
  case FL_HIDDEN_RET_BUTTON:
    b = new Fl_Return_Button(x,y,w,h,l);
    break;
  case FL_TOUCH_BUTTON:
    b = new Fl_Repeat_Button(x,y,w,h,l);
    break;
  default:
    b = new Fl_Button(x,y,w,h,l);
  }
  switch (t) {
  case FL_TOGGLE_BUTTON:
  case FL_RADIO_BUTTON:
    b->type(t);
    break;
  case FL_HIDDEN_BUTTON:
  case FL_HIDDEN_RET_BUTTON:
    b->type(FL_HIDDEN_BUTTON);
    break;
  case FL_INOUT_BUTTON:
    b->when(FL_WHEN_CHANGED);
    break;
  }
  return b;
}

void fl_show_message(const char *q1,const char *q2,const char *q3) {
  fl_message("%s\n%s\n%s", q1?q1:"", q2?q2:"", q3?q3:"");
}

void fl_show_alert(const char *q1,const char *q2,const char *q3,int) {
  fl_alert("%s\n%s\n%s", q1?q1:"", q2?q2:"", q3?q3:"");
}

int fl_show_question(const char *q1,const char *q2,const char *q3) {
  return fl_choice("%s\n%s\n%s", "No", "Yes", 0L, q1?q1:"", q2?q2:"", q3?q3:"");
}

int fl_show_choice(
  const char *q1,
  const char *q2,
  const char *q3,
  int, // number of buttons, ignored
  const char *b0,
  const char *b1,
  const char *b2) {
  return fl_choice("%s\n%s\n%s", q1?q1:"", q2?q2:"", q3?q3:"", b0,b1,b2)+1;
}

char *fl_show_simple_input(const char *str1, const char *defstr) {
  const char *r = fl_input("%s", defstr, str1);
  return (char *)(r ? r : defstr);
}

//
// End of "$Id$".
//
