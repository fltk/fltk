//
// "$Id: fl_set_fonts.cxx,v 1.6.2.5.2.4 2002/03/06 18:11:01 easysw Exp $"
//
// More font utilities for the Fast Light Tool Kit (FLTK).
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

#include <config.h>
#include <FL/Fl.H>
#include <FL/x.H>
#include "Fl_Font.H"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#  include "fl_set_fonts_win32.cxx"
#elif defined(__APPLE__)
#  include "fl_set_fonts_mac.cxx"
#elif USE_XFT
#  include "fl_set_fonts_xft.cxx"
#else
#  include "fl_set_fonts_x.cxx"
#endif // WIN32

//
// End of "$Id: fl_set_fonts.cxx,v 1.6.2.5.2.4 2002/03/06 18:11:01 easysw Exp $".
//
