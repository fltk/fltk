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

static int			num_screens = 0;
static XineramaScreenInfo	*screens;

static void xinerama_init() {
  if (!fl_display) fl_open_display();

  if (XineramaIsActive(fl_display)) {
    screens = XineramaQueryScreens(fl_display, &num_screens);
  } else num_screens = 1;
}
#endif // HAVE_XINERAMA


// Return the number of screens...
int Fl::screen_count() {
#ifdef WIN32
#elif defined(__APPLE__)
#elif defined(HAVE_XINERAMA);
  if (!num_screens) xinerama_init();
  return num_screens;
#endif // WIN32
}

// Return the screen bounding rect for the given mouse position...
void Fl::screen_xywh(int &x, int &y, int &w, int &h, int mx, int my) {
#ifdef WIN32
#elif defined(__APPLE__)
#elif defined(HAVE_XINERAMA)
  if (!num_screens) xinerama_init();

  if (num_screens > 0) {
    int i;

    for (i = 0; i < num_screens; i ++) {
      if (mx >= screens[i].x_org &&
	  mx < (screens[i].x_org + screens[i].width) &&
	  my >= screens[i].y_org &&
	  my < (screens[i].y_org + screens[i].height)) {
	x = screens[i].x_org;
	y = screens[i].y_org;
	w = screens[i].width;
	h = screens[i].height;
	return;
      }
    }
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
  if (!num_screens) xinerama_init();

  if (num_screens > 0 && n < num_screens) {
    x = screens[n].x_org;
    y = screens[n].y_org;
    w = screens[n].width;
    h = screens[n].height;
    return;
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
