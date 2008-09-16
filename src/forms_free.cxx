//
// "$Id$"
//
// Forms free widget routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2008 by Bill Spitzak and others.
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

// Emulation of the Forms "free" widget.
// This emulation allows the free demo to run, and has allowed
// me to port several other programs, but it is in no way
// complete.

#include <FL/Fl.H>
#include <FL/Fl_Free.H>

void Fl_Free::step(void *v) {
  Fl_Free *f = (Fl_Free *)v;
  int old_event = Fl::e_number;
  f->handle(Fl::e_number == FL_STEP);
  Fl::e_number = old_event;
  Fl::add_timeout(.01,step,v);
}

/**
    The constructor takes both the type and the handle
    function. The handle function should be declared as follows:
    \code
    int
    handle_function(Fl_Widget *w,
                   int       event,
    	        float     event_x,
    		float     event_y,
    		char      key)
    \endcode
    This function is called from the the handle() method in
    response to most events, and is called by the draw() method.
    The event argument contains the event type:
    \code
    // old event names for compatibility:
    #define FL_MOUSE		FL_DRAG
    #define FL_DRAW		0
    #define FL_STEP		9
    #define FL_FREEMEM		12
    #define FL_FREEZE		FL_UNMAP
    #define FL_THAW		FL_MAP
    \endcode
*/
Fl_Free::Fl_Free(uchar t,int X, int Y, int W, int H,const char *l,
		 FL_HANDLEPTR hdl) :
Fl_Widget(X,Y,W,H,l) {
  type(t);
  hfunc = hdl;
  if (t == FL_SLEEPING_FREE) set_flag(INACTIVE);
  if (t == FL_CONTINUOUS_FREE || t == FL_ALL_FREE)
    Fl::add_timeout(.01,step,this);
}

/** The destructor will call the handle function with the event FL_FREE_MEM. */
Fl_Free::~Fl_Free() {
  Fl::remove_timeout(step,this);
  hfunc(this,FL_FREEMEM,0,0,0);
}

void Fl_Free::draw() {hfunc(this,FL_DRAW,0,0,0);}

int Fl_Free::handle(int e) {
  char key = Fl::event_key();
  switch (e) {
  case FL_FOCUS:
    if (type()!=FL_INPUT_FREE && type()!=FL_ALL_FREE) return 0;
    break;
  case FL_PUSH:
  case FL_DRAG:
  case FL_RELEASE:
    key = 4-Fl::event_button();
    break;
  case FL_SHORTCUT:
    return 0;
  }
  if (hfunc(this, e, float(Fl::event_x()), float(Fl::event_y()), key)) do_callback();
  return 1;
}

//
// End of "$Id$".
//
