//
// "$Id$"
//
// Image header file for the Fast Light Tool Kit (FLTK).
//
// This class stores the image labels for widgets in fluid.  This is
// not a class in FLTK itself, and will produce different types of
// code depending on what the image type is.
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

#ifndef FLUID_IMAGE_H
#  define FLUID_IMAGE_H

#  include <FL/Fl_Shared_Image.H>


class Fluid_Image {
  const char *name_;
  int refcount;
  Fl_Shared_Image *img;
protected:
  Fluid_Image(const char *name); // no public constructor
  ~Fluid_Image(); // no public destructor
public:
  int written;
  static Fluid_Image* find(const char *);
  void decrement(); // reference counting & automatic free
  void increment();
  void image(Fl_Widget *); // set the image of this widget
  void deimage(Fl_Widget *); // set the deimage of this widget
  void write_static();
  void write_code(const char *var, int inactive = 0);
  const char *name() const {return name_;}
};

// pop up file chooser and return a legal image selected by user,
// or zero for any errors:
Fluid_Image *ui_find_image(const char *);
extern const char *ui_find_image_name;

#endif

//
// End of "$Id$".
//
