//
// "$Id"
//
// Standard dialog header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#ifndef fl_ask_H
#define fl_ask_H

class Fl_Widget;

void fl_message(const char *);
void fl_alert(const char *);
int fl_ask(const char *);
int fl_choice(const char *q,const char *b0,const char *b1,const char *b2);
const char *fl_input(const char *label, const char *deflt, unsigned char type);
const char *fl_input(const char *label, const char *deflt = 0);
inline const char *fl_password(const char *label, const char *deflt = 0) {
  return fl_input(label, deflt, 5);
}
Fl_Widget *fl_message_icon();
extern unsigned char fl_message_font_;
extern unsigned char fl_message_size_;
inline void fl_message_font(unsigned char f,unsigned char s) {
  fl_message_font_ = f; fl_message_size_ = s;}

// pointers you can use to change fltk to a foreign language:
extern const char* fl_no;
extern const char* fl_yes;
extern const char* fl_ok;
extern const char* fl_cancel;

// back compatability and XForms compatability functions:

inline int fl_show_question(const char *c) {return fl_ask(c);}
void fl_show_message(const char *,const char *,const char *);
void fl_show_alert(const char *,const char *,const char *);
int fl_show_question(const char *,const char *,const char *);
inline const char *fl_show_input(const char *l,const char*d=0) {return fl_input(l,d);}
/*const*/ char *fl_show_simple_input(const char *label, const char *deflt = 0);

// all are implemented using this:
int fl_show_choice(
    const char *m1,
    const char *m2,
    const char *m3,
    int numb,
    const char *b0,
    const char *b1,
    const char *b2);

#endif

//
// End of "$Id: fl_ask.H,v 1.2 1998/10/19 21:39:20 mike Exp $".
//
