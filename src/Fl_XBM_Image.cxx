//
// "$Id: Fl_XBM_Image.cxx,v 1.1.2.3 2002/01/01 15:11:31 easysw Exp $"
//
// Fl_XBM_Image routines.
//
// Copyright 1997-2002 by Bill Spitzak and others.
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
// Contents:
//
//   Fl_XBM_Image::Fl_XBM_Image() - Load an XBM file.
//

//
// Include necessary header files...
//

#include <FL/Fl.H>
#include <FL/Fl_XBM_Image.H>
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// 'Fl_XBM_Image::Fl_XBM_Image()' - Load an XBM file.
//

Fl_XBM_Image::Fl_XBM_Image(const char *name) : Fl_Bitmap((const char *)0,0,0) {
  FILE	*f;
  uchar	*data;

  if ((f = fopen(name, "rb")) == NULL) return;

  char buffer[1024];
  char junk[1024];
  int wh[2]; // width and height
  int i;
  for (i = 0; i<2; i++) {
    for (;;) {
      if (!fgets(buffer,1024,f)) {
        fclose(f);
	return;
      }
      int r = sscanf(buffer,"#define %s %d",junk,&wh[i]);
      if (r >= 2) break;
    }
  }

  // skip to data array:
  for (;;) {
    if (!fgets(buffer,1024,f)) {
      fclose(f);
      return;
    }
    if (!strncmp(buffer,"static ",7)) break;
  }

  // Allocate memory...
  w(wh[0]);
  h(wh[1]);

  int n = ((wh[0]+7)/8)*wh[1];
  array = new uchar[((w() + 7) / 8) * h()];

  // read the data:
  for (i = 0, data = (uchar *)array; i < n;) {
    if (!fgets(buffer,1024,f)) {
      fclose(f);
      return;
    }
    const char *a = buffer;
    while (*a && i<n) {
      int t;
      if (sscanf(a," 0x%x",&t)>0) *data++ = t;
      while (*a && *a++ != ',');
    }
  }

  fclose(f);
}


//
// End of "$Id: Fl_XBM_Image.cxx,v 1.1.2.3 2002/01/01 15:11:31 easysw Exp $".
//
