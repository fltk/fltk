//
// "$Id: Fl_Wizard.H,v 1.1.2.2 2002/01/01 15:11:28 easysw Exp $"
//
// Fl_Wizard widget definitions.
//
// Copyright 1999-2002 by Easy Software Products.
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

//
// Include necessary header files...
//

#ifndef _Fl_Wizard_H_
#  define _Fl_Wizard_H_

#  include <FL/Fl_Group.H>


//
// Fl_Wizard class...
//

class Fl_Wizard : public Fl_Group
{
  Fl_Widget *value_;

  void draw();

  public:

  Fl_Wizard(int, int, int, int, const char * = 0);

  void		next();
  void		prev();
  Fl_Widget	*value();
  void		value(Fl_Widget *);
};

#endif // !_Fl_Wizard_H_

//
// End of "$Id: Fl_Wizard.H,v 1.1.2.2 2002/01/01 15:11:28 easysw Exp $".
//
