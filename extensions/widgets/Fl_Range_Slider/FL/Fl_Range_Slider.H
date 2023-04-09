//
// "$Id$"
//
// Flmm_Scalebar header file for the FLMM extension to FLTK.
//
// Copyright 2002-2004 by Matthias Melcher.
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
// Please report all bugs and problems to "flmm@matthiasm.com".
//

#ifndef Flmm_Scalebar_H
#define Flmm_Scalebar_H

#include <FL/Fl_Scrollbar.H>

class FL_EXPORT Fl_Range_Slider : public Fl_Scrollbar 
{
  short pushed_;
  short slider_size_i_;
  short min_sash_size_;
  
private:
  void drawSlider( int x, int y, int w, int h );
  void drawSliderBg( int x, int y, int w, int h );
  
protected:
  void draw();
  int handle(int);
  
public:  
  Fl_Range_Slider(int x,int y,int w,int h, const char *l = 0);
  int value();
  int value(int position, int size, int top, int total);
  int scrollvalue(int position, int size, int top, int total);
  void slider_size_i( int v );
  int slider_size_i();
};

#endif

//
// End of "$Id$".
//
