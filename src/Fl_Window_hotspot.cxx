//
// "$Id: Fl_Window_hotspot.cxx,v 1.7.2.3.2.3 2002/04/15 20:30:06 easysw Exp $"
//
// Common hotspot routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2002 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/x.H>

void Fl_Window::hotspot(int X, int Y, int offscreen) {
  int mx,my;

  // Update the screen position based on the mouse position.
  Fl::get_mouse(mx,my);
  X = mx-X; Y = my-Y;

  // If offscreen is 0 (the default), make sure that the window
  // stays on the screen, if possible.
  if (!offscreen) {
#if defined(WIN32) || defined(__APPLE__)
    // These will be used by reference, so we must passed different variables
    int bt,bx,by;
    x(X);y(Y);
    Fl_X::fake_X_wm(this, X, Y, bt, bx, by);
    //force FL_FORCE_POSITION to be set in Fl_Window::resize()
    if (X==x()) x(X-1);
#else
    if (border()) {
      // Ensure border is on screen; these values are generic enough
      // to work with many window managers, and are based on KDE defaults.
      const int top = 20;
      const int left = 4;
      const int right = 4;
      const int bottom = 8;
      if (X+w()+right > Fl::w()-Fl::x()) X = Fl::w()-Fl::x()-right-w();
      if (X-left < Fl::x()) X = left;
      if (Y+h()+bottom > Fl::h()-Fl::y()) Y = Fl::h()-Fl::y()-bottom-h();
      if (Y-top < Fl::y()) Y = top;
    }
    // now insure contents are on-screen (more important than border):
    if (X+w() > Fl::w()-Fl::x()) X = Fl::w()-Fl::x()-w();
    if (X < Fl::x()) X = Fl::x();
    if (Y+h() > Fl::h()-Fl::y()) Y = Fl::h()-Fl::y()-h();
    if (Y < Fl::y()) Y = Fl::y();
#endif
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
// End of "$Id: Fl_Window_hotspot.cxx,v 1.7.2.3.2.3 2002/04/15 20:30:06 easysw Exp $".
//
