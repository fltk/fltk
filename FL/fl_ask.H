//
// "$Id: fl_ask.H,v 1.7.2.3 2001/01/22 15:13:38 easysw Exp $"
//
// Standard dialog header file for the Fast Light Tool Kit (FLTK).
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

#ifndef fl_ask_H
#define fl_ask_H

#include "Enumerations.H"

class Fl_Widget;

FL_EXPORT void fl_message(const char *,...);
FL_EXPORT void fl_alert(const char *,...);
FL_EXPORT int fl_ask(const char *,...);
FL_EXPORT int fl_choice(const char *q,const char *b0,const char *b1,const char *b2,...);
FL_EXPORT const char *fl_input(const char *label, const char *deflt = 0, ...);
FL_EXPORT const char *fl_password(const char *label, const char *deflt = 0, ...);

FL_EXPORT Fl_Widget *fl_message_icon();
extern FL_EXPORT unsigned char fl_message_font_;
extern FL_EXPORT unsigned char fl_message_size_;
inline void fl_message_font(unsigned char f,unsigned char s) {
  fl_message_font_ = f; fl_message_size_ = s;}

// pointers you can use to change FLTK to a foreign language:
extern FL_EXPORT const char* fl_no;
extern FL_EXPORT const char* fl_yes;
extern FL_EXPORT const char* fl_ok;
extern FL_EXPORT const char* fl_cancel;

#endif

//
// End of "$Id: fl_ask.H,v 1.7.2.3 2001/01/22 15:13:38 easysw Exp $".
//
