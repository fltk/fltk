//
// "$Id: fl_read_image_win32.cxx,v 1.1.2.1 2002/05/30 15:09:03 easysw Exp $"
//
// WIN32 image reading routines for the Fast Light Tool Kit (FLTK).
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

//
// 'fl_read_image()' - Read an image from the current window.
//

uchar *				// O - Pixel buffer or NULL if failed
fl_read_image(uchar *p,		// I - Pixel buffer or NULL to allocate
              int   x,		// I - Left position
	      int   y,		// I - Top position
	      int   w,		// I - Width of area to read
	      int   h,		// I - Height of area to read
	      int   alpha) {	// I - Alpha value for image (0 for none)
  return 0;
}


//
// End of "$Id: fl_read_image_win32.cxx,v 1.1.2.1 2002/05/30 15:09:03 easysw Exp $".
//
