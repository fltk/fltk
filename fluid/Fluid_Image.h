//
// "$Id$"
//
// Image header file for the Fast Light Tool Kit (FLTK).
//
// This class stores the image labels for widgets in fluid.  This is
// not a class in FLTK itself, and will produce different types of
// code depending on what the image type is.
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

#ifndef FLUID_IMAGE_H
#  define FLUID_IMAGE_H

#  include <FL/Fl_Shared_Image.H>


class Fluid_Image {
  const char *name_;
  int refcount;
  Fl_Shared_Image *img;
  const char *function_name_;
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
  void write_initializer(const char *type_name, const char *format, ...);
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
