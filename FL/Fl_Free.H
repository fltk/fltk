//
// "$Id: Fl_Free.H,v 1.5.2.4 2001/01/22 15:13:37 easysw Exp $"
//
// Forms free header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Free_H
#define Fl_Free_H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

#define FL_NORMAL_FREE		1
#define FL_SLEEPING_FREE	2
#define FL_INPUT_FREE		3
#define FL_CONTINUOUS_FREE	4
#define FL_ALL_FREE		5

typedef int (*FL_HANDLEPTR)(Fl_Widget *, int , float, float, char);

class Fl_Free : public Fl_Widget {
    FL_HANDLEPTR hfunc;
    static FL_EXPORT void step(void *);
protected:
    FL_EXPORT void draw();
public:
    FL_EXPORT int handle(int);
    FL_EXPORT Fl_Free(uchar t,int x,int y,int w,int h,const char *l,FL_HANDLEPTR hdl);
    FL_EXPORT ~Fl_Free();
};

// old event names for compatability:
#define FL_MOUSE		FL_DRAG
#define FL_DRAW			0
#define FL_STEP			9
#define FL_FREEMEM		12
#define FL_FREEZE		FL_UNMAP
#define FL_THAW			FL_MAP

#endif

//
// End of "$Id: Fl_Free.H,v 1.5.2.4 2001/01/22 15:13:37 easysw Exp $".
//
