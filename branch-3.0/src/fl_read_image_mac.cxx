//
// "$Id$"
//
// WIN32 image reading routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
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
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <config.h>
extern unsigned char *MACbitmapFromRectOfWindow(Fl_Window *win, int x, int y, int w, int h, int *bytesPerPixel);

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
  Fl_Window *window = Fl_Window::current();
  while(window->window()) window = window->window();
  int delta;
  uchar *base = MACbitmapFromRectOfWindow(window,x,y,w,h,&delta);
  int rowBytes = delta*w;
  // Allocate the image data array as needed...
  int d = alpha ? 4 : 3;
  if (!p) p = new uchar[w * h * d];
  // Initialize the default colors/alpha in the whole image...
  memset(p, alpha, w * h * d);
  // Copy the image from the off-screen buffer to the memory buffer.
  int           idx, idy;	// Current X & Y in image
  uchar *pdst, *psrc;
  for (idy = 0, pdst = p; idy < h; idy ++) {
    for (idx = 0, psrc = base + idy * rowBytes; idx < w; idx ++, psrc += delta, pdst += d) {
      pdst[0] = psrc[0];  // R
      pdst[1] = psrc[1];  // G
      pdst[2] = psrc[2];  // B
    }
  }
  delete base;
  return p;
}


//
// End of "$Id$".
//
