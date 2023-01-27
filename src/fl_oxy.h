//
// "Oxy" Scheme drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 2011 by Dmitrij K. aka "kdiman"
// Copyright 2012-2022 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//   https://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//   https://www.fltk.org/str.php
//

#ifndef fl_oxy_h
#define fl_oxy_h

#include <FL/Fl.H>

// draw an arrow GUI element for the 'oxy' scheme
//
// bb   bounding box
// t    arrow type
// o    orientation
// c    arrow color

extern FL_EXPORT void oxy_arrow(Fl_Rect bb,
                                Fl_Arrow_Type t, Fl_Orientation o,
                                Fl_Color col);

#endif // fl_oxy_h
