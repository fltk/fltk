//
// "$Id: Fl_Output.cxx,v 1.6.2.3 2001/01/22 15:13:40 easysw Exp $"
//
// Output widget for the Fast Light Tool Kit (FLTK).
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

// This subclass of Fl_Input_ does not allow user to edit the output.
// Used to display output.

#include <FL/Fl.H>
#include <FL/Fl_Output.H>
#include <FL/fl_draw.H>

void Fl_Output::draw() {
  Fl_Boxtype b = box() ? box() : FL_DOWN_BOX;
  if (damage() & FL_DAMAGE_ALL) draw_box(b, color());
  Fl_Input_::drawtext(x()+Fl::box_dx(b)+3, y()+Fl::box_dy(b),
		      w()-Fl::box_dw(b)-6, h()-Fl::box_dh(b));
}

int Fl_Output::handle(int event) {
  if (event == FL_FOCUS) return 0;
  Fl_Boxtype b = box() ? box() : FL_DOWN_BOX;
  return Fl_Input_::handletext(event,
	x()+Fl::box_dx(b)+3, y()+Fl::box_dy(b),
	w()-Fl::box_dw(b)-6, h()-Fl::box_dh(b));
}

//
// End of "$Id: Fl_Output.cxx,v 1.6.2.3 2001/01/22 15:13:40 easysw Exp $".
//
