//
// "$Id: Fl_Double_Window.H,v 1.7.2.3 2001/01/22 15:13:37 easysw Exp $"
//
// Double-buffered window header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Double_Window_H
#define Fl_Double_Window_H

#include "Fl_Window.H"

class Fl_Double_Window : public Fl_Window {
protected:
  FL_EXPORT void flush(int eraseoverlay);
public:
  FL_EXPORT void show();
  void show(int a, char **b) {Fl_Window::show(a,b);}
  FL_EXPORT void flush();
  FL_EXPORT void resize(int,int,int,int);
  FL_EXPORT void hide();
  FL_EXPORT ~Fl_Double_Window();
  Fl_Double_Window(int W, int H, const char *l = 0) : Fl_Window(W,H,l) {}
  Fl_Double_Window(int X, int Y, int W, int H, const char *l = 0)
    : Fl_Window(X,Y,W,H,l) {}
};

#endif

//
// End of "$Id: Fl_Double_Window.H,v 1.7.2.3 2001/01/22 15:13:37 easysw Exp $".
//
