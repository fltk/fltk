//
// "$Id: Fl_Value_Output.H,v 1.5.2.3 2001/01/22 15:13:38 easysw Exp $"
//
// Value output header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Value_Output_H
#define Fl_Value_Output_H

#ifndef Fl_Valuator_H
#include "Fl_Valuator.H"
#endif

class Fl_Value_Output : public Fl_Valuator {
  uchar textfont_, textsize_, textcolor_, soft_;
public:
  FL_EXPORT int handle(int);
  FL_EXPORT void draw();
  FL_EXPORT Fl_Value_Output(int x,int y,int w,int h,const char *l=0);

  void soft(uchar x) {soft_ = x;}
  uchar soft() const {return soft_;}

  Fl_Font textfont() const {return (Fl_Font)textfont_;}
  void textfont(uchar s) {textfont_ = s;}
  uchar textsize() const {return textsize_;}
  void textsize(uchar s) {textsize_ = s;}
  Fl_Color textcolor() const {return (Fl_Color)textcolor_;}
  void textcolor(uchar s) {textcolor_ = s;}
};

#endif

//
// End of "$Id: Fl_Value_Output.H,v 1.5.2.3 2001/01/22 15:13:38 easysw Exp $".
//
