//
// "$Id$"
//
// Screen/monitor bounding box API for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2005 by Bill Spitzak and others.
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


#include <FL/Fl.H>
#include <FL/x.H>
#include <config.h>

#ifdef HAVE_XINERAMA
#  include <X11/extensions/Xinerama.h>
#endif // HAVE_XINERAMA


// Return the number of screens...
int Fl::screen_count() {
}

// Return the screen bounding rect for the given mouse position...
void Fl::screen_xywh(int &x, int &y, int &w, int &h, int mx, int my) {
#ifdef WIN32
#elif defined(__APPLE__)
#elif defined(HAVE_XINERAMA)
  if (!fl_display) fl_open_display();

  if (XineramaIsActive(fl_display)) {
    int			i,
			num_rects;
    XineramaScreenInfo	*rects;


    rects = XineramaQueryScreens(fl_display, &num_rects);

#  ifdef DEBUG
    printf("num_rects = %d\n", num_rects);
    printf("window_->x_root() = %d, y_root() = %d\n",
           window_->x_root(), window_->y_root());
#  endif // DEBUG

    for (i = 0; i < num_rects; i ++) {
#  ifdef DEBUG
      printf("rects[%d] = [%d %d %d %d]\n", i,
	     rects[i].x_org, rects[i].y_org, rects[i].width,
	     rects[i].height);
#  endif // DEBUG

      if (mx >= rects[i].x_org &&
	  mx < (rects[i].x_org + rects[i].width) &&
	  my >= rects[i].y_org &&
	  my < (rects[i].y_org + rects[i].height))
      {
	x = rects[i].x_org;
	y = rects[i].y_org;
	w = rects[i].width;
	h = rects[i].height;
	break;
      }
    }

    XFree(rects);
  }
#endif // WIN32

  x = Fl::x();
  y = Fl::y();
  w = Fl::w();
  h = Fl::h();
}

// Return the screen bounding rect for the given screen...
void Fl::screen_xywh(int &x, int &y, int &w, int &h, int n) {
#ifdef WIN32
#elif defined(__APPLE__)
#elif defined(HAVE_XINERAMA)
  if (!fl_display) fl_open_display();

  if (XineramaIsActive(fl_display)) {
    int			i,
			num_rects;
    XineramaScreenInfo	*rects;


    rects = XineramaQueryScreens(fl_display, &num_rects);

#  ifdef DEBUG
    printf("num_rects = %d\n", num_rects);
    printf("window_->x_root() = %d, y_root() = %d\n",
           window_->x_root(), window_->y_root());
#  endif // DEBUG

    for (i = 0; i < num_rects; i ++) {
#  ifdef DEBUG
      printf("rects[%d] = [%d %d %d %d]\n", i,
	     rects[i].x_org, rects[i].y_org, rects[i].width,
	     rects[i].height);
#  endif // DEBUG

      if (mx >= rects[i].x_org &&
	  mx < (rects[i].x_org + rects[i].width) &&
	  my >= rects[i].y_org &&
	  my < (rects[i].y_org + rects[i].height))
      {
	x = rects[i].x_org;
	y = rects[i].y_org;
	w = rects[i].width;
	h = rects[i].height;
	break;
      }
    }

    XFree(rects);
  }
#endif // WIN32

  x = Fl::x();
  y = Fl::y();
  w = Fl::w();
  h = Fl::h();
}


//
// End of "$Id$".
//
