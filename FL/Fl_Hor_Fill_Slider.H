//
// "$Id$"
//
// Horizontal fill slider header file for the Fast Light Tool Kit (FLTK).
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
   Fl_Hor_Fill_Slider widget . */

#ifndef Fl_Hor_Fill_Slider_H
#define Fl_Hor_Fill_Slider_H

#include "Fl_Slider.H"

class FL_EXPORT Fl_Hor_Fill_Slider : public Fl_Slider {
public:
    Fl_Hor_Fill_Slider(int x,int y,int w,int h,const char *l=0)
	: Fl_Slider(x,y,w,h,l) {type(FL_HOR_FILL_SLIDER);}
};

#endif

//
// End of "$Id$".
//
