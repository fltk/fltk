//
// "$Id$"
//
// Radio round button header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

/* \file
   Fl_Radio_Round_Button widget . */

#ifndef Fl_Radio_Round_Button_H
#define Fl_Radio_Round_Button_H

#include "Fl_Round_Button.H"

class FL_EXPORT Fl_Radio_Round_Button : public Fl_Round_Button {
public:
    Fl_Radio_Round_Button(int x,int y,int w,int h,const char *l=0)
	: Fl_Round_Button(x,y,w,h,l) {type(FL_RADIO_BUTTON);}
};

#endif

//
// End of "$Id$".
//
