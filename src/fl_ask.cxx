// fl_ask.C

// Implementation of fl_message, fl_ask, fl_choice, fl_input

// The three-message fl_show_x functions are for forms compatibility
// mostly.  In most cases it is easier to get a multi-line message
// by putting newlines in the message.

#include <FL/Fl.H>

#include <FL/fl_ask.H>

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
static Fl_Window *message_form;
static Fl_Box *message[3];
static Fl_Box *icon;
static Fl_Button *button[3];
static Fl_Input *input;
static char *iconlabel;
uchar fl_message_font_ = 0;
uchar fl_message_size_ = FL_NORMAL_SIZE;

static Fl_Window *makeform() {
 if (message_form) return message_form;
 Fl_Window *w = message_form = new Fl_Window(410,105);
 // w->clear_border();
 // w->box(FL_UP_BOX);
 (message[0] = new Fl_Box(60, 9, 340, 20))
   ->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
 (message[1] = new Fl_Box(60, 25, 340, 20))
   ->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
 (message[2] = new Fl_Box(60, 41, 340, 20))
   ->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
 (input = new Fl_Input(60,32,340,30))->hide();
 {Fl_Box* o = icon = new Fl_Box(10, 10, 50, 50);
  o->box(FL_THIN_UP_BOX);
  o->labelfont(FL_TIMES_BOLD);
  o->labelsize(34);
  o->color(FL_WHITE);
  o->labelcolor(FL_BLUE);
 }
 (button[0] = new Fl_Button(310, 70, 90, 25))->shortcut("^[");
 button[1] = new Fl_Return_Button(210, 70, 90, 25);
 button[2] = new Fl_Button(110, 70, 90, 25);
 w->end();
 w->set_modal();
 return w;
}

// back-compatable functions:

int fl_show_choice(
  const char *m0,
  const char *m1,
  const char *m2,
  int, // number of buttons, ignored
  const char *b0,
  const char *b1,
  const char *b2)
{
  makeform();
  message[0]->label(m0);
  message[1]->label(m1);
  message[2]->label(m2);
  Fl_Font f = (Fl_Font)fl_message_font_;
  if (!f) f = Fl_Input_::default_font();
  int s = fl_message_size_ + Fl_Input::default_size();
  for (int i=0; i<3; i++) {
    message[i]->labelfont(f);
    message[i]->labelsize(s);
  }
  if (b0) {button[0]->show();button[0]->label(b0);button[1]->position(210,70);}
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
  return r+1;
}

// pointers you can use to change fltk to a foreign language:
const char* fl_no = "No";
const char* fl_yes= "Yes";
const char* fl_ok = "OK";
const char* fl_cancel= "Cancel";

// back-compatable XForms functions:

void fl_show_message(const char *q1,const char *q2,const char *q3) {
  iconlabel = "i";
  fl_show_choice(q1, q2, q3, 1, 0, fl_ok, 0);
}

void fl_show_alert(const char *q1,const char *q2,const char *q3) {
  iconlabel = "!";
  fl_show_choice(q1, q2, q3, 1, 0, fl_ok, 0);
}

int fl_show_question(const char *q1,const char *q2,const char *q3) {
  iconlabel = "?";
  return fl_show_choice(q1, q2, q3, 2, fl_no, fl_yes, 0) - 1;
}

// fltk functions:

void fl_message(const char *question) {
  fl_show_message(0, question, 0);
}

void fl_alert(const char *question) {
  fl_show_alert(0, question, 0);
}

int fl_ask(const char *question) {
  return fl_show_question(0, question, 0);
}

int fl_choice(const char *q,const char *b0,const char *b1,const char *b2) {
  iconlabel = "?";
  return fl_show_choice(0,q,0,3,b0,b1,b2) - 1;
}

Fl_Widget *fl_message_icon() {makeform(); return icon;}

const char *fl_input(const char *str1, const char *defstr, uchar type) {
  makeform();
  input->type(type);
  input->show();
  input->value(defstr);
  iconlabel = "?";
  int r = fl_show_choice(str1,0,0,2,fl_cancel,fl_ok,0);
  input->hide();
  return r==2 ? input->value() : 0;
}

const char *fl_input(const char *str1, const char *defstr) {
  return fl_input(str1, defstr, FL_NORMAL_INPUT);
}

char *fl_show_simple_input(const char *str1, const char *defstr) {
  const char *r = fl_input(str1, defstr, FL_NORMAL_INPUT);
  return (char *)(r ? r : defstr);
}

// end of fl_ask.C
