//
// "$Id: Fl_XBM_Image.H,v 1.1.2.2 2002/01/01 15:11:28 easysw Exp $"
//
// XBM image header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_XBM_Image_H
#define Fl_XBM_Image_H
#  include "Fl_Bitmap.H"

class FL_EXPORT Fl_XBM_Image : public Fl_Bitmap {

  public:

  Fl_XBM_Image(const char* filename);
};

#endif // !Fl_XBM_Image_H

//
// End of "$Id: Fl_XBM_Image.H,v 1.1.2.2 2002/01/01 15:11:28 easysw Exp $".
//
