//
// "$Id: fl_ask.cxx,v 1.8.2.8 2001/01/22 15:13:40 easysw Exp $"
//
// Standard dialog functions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

// Implementation of fl_message, fl_ask, fl_choice, fl_input
// The three-message fl_show_x functions are for forms compatibility
// mostly.  In most cases it is easier to get a multi-line message
// by putting newlines in the message.

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <config.h>

#include <FL/Fl.H>

#include <FL/fl_ask.H>

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/x.H>

static Fl_Window *message_form;
static Fl_Box *message;
static Fl_Box *icon;
static Fl_Button *button[3];
static Fl_Input *input;
static const char *iconlabel = "?";
uchar fl_message_font_ = 0;
uchar fl_message_size_ = 14;

static Fl_Window *makeform() {
 if (message_form) {
   message_form->size(410,103);
   return message_form;
 }
 Fl_Window *w = message_form = new Fl_Window(410,103);
 // w->clear_border();
 // w->box(FL_UP_BOX);
 (message = new Fl_Box(60, 25, 340, 20))
   ->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
 (input = new Fl_Input(60, 37, 340, 23))->hide();
 {Fl_Box* o = icon = new Fl_Box(10, 10, 50, 50);
  o->box(FL_THIN_UP_BOX);
  o->labelfont(FL_TIMES_BOLD);
  o->labelsize(34);
  o->color(FL_WHITE);
  o->labelcolor(FL_BLUE);
 }
 (button[0] = new Fl_Button(310, 70, 90, 23))->shortcut("^[");
 button[1] = new Fl_Return_Button(210, 70, 90, 23);
 button[2] = new Fl_Button(110, 70, 90, 23);
 w->resizable(new Fl_Box(60,10,110-60,27));
 w->end();
 w->set_modal();
 return w;
}

#if !HAVE_VSNPRINTF || defined(__hpux)
extern "C" {
int vsnprintf(char* str, size_t size, const char* fmt, va_list ap);
}
#endif

static int innards(const char* fmt, va_list ap,
  const char *b0,
  const char *b1,
  const char *b2)
{
  makeform();
  char buffer[1024];
  if (!strcmp(fmt,"%s")) {
    message->label(va_arg(ap, const char*));
  } else {
    vsnprintf(buffer, 1024, fmt, ap);
    message->label(buffer);
  }
  Fl_Font f = (Fl_Font)fl_message_font_;
  message->labelfont(f);
  message->labelsize(fl_message_size_);
  if (b0) {button[0]->show(); button[0]->label(b0); button[1]->position(210,70);}
  else {button[0]->hide(); button[1]->position(310,70);}
  if (b1) {button[1]->show(); button[1]->label(b1);}
  else button[1]->hide();
  if (b2) {button[2]->show(); button[2]->label(b2);}
  else button[2]->hide();
  const char* prev_icon_label = icon->label();
  if (!prev_icon_label) icon->label(iconlabel);
  message_form->hotspot(button[0]);
  message_form->show();
  int r;
  for (;;) {
    Fl_Widget *o = Fl::readqueue();
    if (!o) Fl::wait();
    else if (o == button[0]) {r = 0; break;}
    else if (o == button[1]) {r = 1; break;}
    else if (o == button[2]) {r = 2; break;}
    else if (o == message_form) {r = 0; break;}
  }
  message_form->hide();
  icon->label(prev_icon_label);
  return r;
}

// pointers you can use to change fltk to a foreign language:
const char* fl_no = "No";
const char* fl_yes= "Yes";
const char* fl_ok = "OK";
const char* fl_cancel= "Cancel";

// fltk functions:

void fl_message(const char *fmt, ...) {
  va_list ap;

#ifdef WIN32
  MessageBeep(MB_ICONASTERISK);
#endif // WIN32

  va_start(ap, fmt);
  iconlabel = "i";
  innards(fmt, ap, 0, fl_ok, 0);
  va_end(ap);
  iconlabel = "?";
}

void fl_alert(const char *fmt, ...) {
  va_list ap;

#ifdef WIN32
  MessageBeep(MB_ICONERROR);
#else
  if (!fl_display) fl_open_display();
  XBell(fl_display, 100);
#endif // WIN32

  va_start(ap, fmt);
  iconlabel = "!";
  innards(fmt, ap, 0, fl_ok, 0);
  va_end(ap);
  iconlabel = "?";
}

int fl_ask(const char *fmt, ...) {
  va_list ap;

#ifdef WIN32
  MessageBeep(MB_ICONQUESTION);
#endif // WIN32

  va_start(ap, fmt);
  int r = innards(fmt, ap, fl_no, fl_yes, 0);
  va_end(ap);

  return r;
}

int fl_choice(const char*fmt,const char *b0,const char *b1,const char *b2,...){
  va_list ap;

#ifdef WIN32
  MessageBeep(MB_ICONQUESTION);
#endif // WIN32

  va_start(ap, b2);
  int r = innards(fmt, ap, b0, b1, b2);
  va_end(ap);
  return r;
}

Fl_Widget *fl_message_icon() {makeform(); return icon;}

static const char* input_innards(const char* fmt, va_list ap,
				 const char* defstr, uchar type) {
  makeform();
  message->position(60,10);
  input->type(type);
  input->show();
  input->value(defstr);

#ifdef WIN32
  MessageBeep(MB_ICONQUESTION);
#endif // WIN32

  int r = innards(fmt, ap, fl_cancel, fl_ok, 0);
  input->hide();
  message->position(60,25);
  return r ? input->value() : 0;
}

const char* fl_input(const char *fmt, const char *defstr, ...) {
  va_list ap;
  va_start(ap, defstr);
  const char* r = input_innards(fmt, ap, defstr, FL_NORMAL_INPUT);
  va_end(ap);
  return r;
}

const char *fl_password(const char *fmt, const char *defstr, ...) {
  va_list ap;
  va_start(ap, defstr);
  const char* r = input_innards(fmt, ap, defstr, FL_SECRET_INPUT);
  va_end(ap);
  return r;
}

//
// End of "$Id: fl_ask.cxx,v 1.8.2.8 2001/01/22 15:13:40 easysw Exp $".
//
