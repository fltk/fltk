//
// "$Id: Fluid_Image.h,v 1.3.2.4 2001/01/22 15:13:38 easysw Exp $"
//
// Pixmap image header file for the Fast Light Tool Kit (FLTK).
//
// This class stores the image labels for widgets in fluid.  This is
// not a class in fltk itself, and this will produce different types of
// code depending on what the image type is.  There are private subclasses
// in Fluid_Image.C for each type of image format.  Right now only xpm
// files are supported.
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

#ifndef FLUID_IMAGE_H
#define FLUID_IMAGE_H

class Fluid_Image {
  const char *name_;
  int refcount;
protected:
  Fluid_Image(const char *name); // no public constructor
  virtual ~Fluid_Image(); // no public destructor
public:
  int written;
  static Fluid_Image* find(const char *);
  void decrement(); // reference counting & automatic free
  void increment();
  virtual void label(Fl_Widget *) = 0; // set the label of this widget
  virtual void write_static() = 0;
  virtual void write_code() = 0;
  const char *name() const {return name_;}
};

// pop up file chooser and return a legal image selected by user,
// or zero for any errors:
Fluid_Image *ui_find_image(const char *);

#endif

//
// End of "$Id: Fluid_Image.h,v 1.3.2.4 2001/01/22 15:13:38 easysw Exp $".
//
