//
// "$Id$"
//
// Copy-to-clipboard code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2014 by Bill Spitzak and others.
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

#ifndef Fl_Copy_Surface_H
#define Fl_Copy_Surface_H

#include <FL/Fl_Paged_Device.H>
#include <FL/Fl_Printer.H>
#include <FL/x.H>

/** Supports copying of graphical data to the clipboard.
 
 <br> After creation of an Fl_Copy_Surface object, call set_current() on it, and all subsequent graphics requests
 will be recorded in the clipboard. It's possible to draw widgets (using Fl_Copy_Surface::draw()
 ) or to use any of the \ref fl_drawings or the \ref fl_attributes.
 Finally, delete the Fl_Copy_Surface object to load the clipboard with the graphical data.
 <br> Fl_GL_Window 's can be copied to the clipboard as well. 
 <br> Usage example:
 \code
 Fl_Widget *g = ...; // a widget you want to copy to the clipboard
 Fl_Copy_Surface *copy_surf = new Fl_Copy_Surface(g->w(), g->h()); // create an Fl_Copy_Surface object
 copy_surf->set_current(); // direct graphics requests to the clipboard
 fl_color(FL_WHITE); fl_rectf(0, 0, g->w(), g->h()); // draw a white background
 copy_surf->draw(g); // draw the g widget in the clipboard
 delete copy_surf; // after this, the clipboard is loaded
 Fl_Display_Device::display_device()->set_current();  // direct graphics requests back to the display
 \endcode
 Platform details:
 \li MSWindows: Transparent RGB images copy without transparency.
 The graphical data are copied to the clipboard as an 'enhanced metafile'.
 \li Mac OS: The graphical data are copied to the clipboard (a.k.a. pasteboard) in two 'flavors':
 1) in vectorial form as PDF data; 2) in bitmap form as a TIFF image. 
 Applications to which the clipboard content is pasted can use the flavor that suits them best.
 \li X11: the graphical data are copied to the clipboard as an image in BMP format.
*/
class FL_EXPORT Fl_Copy_Surface : public Fl_Widget_Surface {
private:
  class Helper;
  Helper *platform_surface;
  static Helper *newPlatformSurface(int w, int h);
protected:
  void translate(int x, int y);
  void untranslate();
public:
  Fl_Copy_Surface(int w, int h);
  ~Fl_Copy_Surface();
  void set_current();
  /** Returns the pixel width of the copy surface */
  int w();
  /** Returns the pixel height of the copy surface */
  int h();
  void origin(int *x, int *y);
};

#endif // Fl_Copy_Surface_H

//
// End of "$Id$".
//
