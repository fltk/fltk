//
// "$Id: Fl_Window_hotspot.cxx,v 1.4 1998/11/05 16:04:48 mike Exp $"
//
// Common hotspot routines for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl.H>
#include <FL/Fl_Window.H>

#ifdef WIN32
#include <FL/win32.H>
#endif

void Fl_Window::hotspot(int X, int Y, int offscreen) {
  int mx,my; Fl::get_mouse(mx,my);
  X = mx-X; Y = my-Y;
  if (!offscreen) {
    if (border()) {
      // ensure border is on screen:
#ifdef WIN32
      int top, left, right, bottom;
      Fl_X::get_border(this, top, left, bottom);
      right = left; top += bottom;
#else
      const int top = 20;
      const int left = 1;
      const int right = 1;
      const int bottom = 1;
#endif
      if (X+w()+right > Fl::w()) X = Fl::w()-right-w();
      if (X-left < 0) X = left;
      if (Y+h()+bottom > Fl::h()) Y = Fl::h()-bottom-h();
      if (Y-top < 0) Y = top;
    }
    // now insure contents are on-screen (more important than border):
    if (X+w() > Fl::w()) X = Fl::w()-w();
    if (X < 0) X = 0;
    if (Y+h() > Fl::h()) Y = Fl::h()-h();
    if (Y < 0) Y = 0;
  }
  position(X,Y);
}

void Fl_Window::hotspot(const Fl_Widget *o, int offscreen) {
  int X = o->w()/2;
  int Y = o->h()/2;
  while (o != this) {
    X += o->x(); Y += o->y();
    o = o->window();
  }
  hotspot(X,Y,offscreen);
}

//
// End of "$Id: Fl_Window_hotspot.cxx,v 1.4 1998/11/05 16:04:48 mike Exp $".
//
