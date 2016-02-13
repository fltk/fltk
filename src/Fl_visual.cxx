//
// "$Id$"
//
// Visual support for the Fast Light Tool Kit (FLTK).
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

// Set the default visual according to passed switches:

#include <config.h>
#include <FL/Fl.H>
#include <FL/Fl_Screen_Driver.H>

/** \fn  Fl::visual(int flags)
    Selects a visual so that your graphics are drawn correctly.  This is
    only allowed before you call show() on any windows.  This does nothing
    if the default visual satisfies the capabilities, or if no visual
    satisfies the capabilities, or on systems that don't have such
    brain-dead notions.
    
    <P>Only the following combinations do anything useful:
    
    <UL>
    <LI>Fl::visual(FL_RGB)
    <BR>Full/true color (if there are several depths FLTK chooses  the
    largest).  Do this if you use fl_draw_image
    for much better (non-dithered)  output.
    <BR>&nbsp; </LI>
    <LI>Fl::visual(FL_RGB8)
    <BR>Full color with at least 24 bits of color. FL_RGB will
    always  pick this if available, but if not it will happily return a
    less-than-24 bit deep visual.  This call fails if 24 bits are not
    available.
    <BR>&nbsp; </LI>
    <LI>Fl::visual(FL_DOUBLE|FL_INDEX)
    <BR>Hardware double buffering.  Call this if you are going to use 
    Fl_Double_Window.
    <BR>&nbsp; </LI>
    <LI>Fl::visual(FL_DOUBLE|FL_RGB)</LI>
    <LI>Fl::visual(FL_DOUBLE|FL_RGB8)
    <BR>Hardware double buffering and full color.
    </UL>
    
    <P>This returns true if the system has the capabilities by default or
    FLTK suceeded in turing them on.  Your program will still work even if
    this returns false (it just won't look as good).
*/
int Fl::visual(int flags)
{
  return screen_driver()->visual(flags);
}


//
// End of "$Id$".
//
