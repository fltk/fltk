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
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#ifndef FLUID_IMAGE_H
#define FLUID_IMAGE_H

#include <FL/Fl_Shared_Image.H>

#include "code.h"

class Fluid_Image {
  bool is_animated_gif_;
  const char *name_;
  int refcount;
  Fl_Shared_Image *img;
  const char *function_name_;
protected:
  Fluid_Image(const char *name); // no public constructor
  ~Fluid_Image(); // no public destructor
  size_t write_static_binary(Fd_Code_Writer& f, const char* fmt);
  size_t write_static_text(Fd_Code_Writer& f, const char* fmt);
  void write_static_rgb(Fd_Code_Writer& f, const char* idata_name);
public:
  int written;
  static Fluid_Image* find(const char *);
  void decrement(); // reference counting & automatic free
  void increment();
  void image(Fl_Widget *); // set the image of this widget
  void deimage(Fl_Widget *); // set the deimage of this widget
  void write_static(Fd_Code_Writer& f, int compressed);
  void write_initializer(Fd_Code_Writer& f, const char *type_name, const char *format, ...);
  void write_code(Fd_Code_Writer& f, int bind, const char *var, int inactive = 0);
  void write_inline(Fd_Code_Writer& f, int inactive = 0);
  void write_file_error(Fd_Code_Writer& f, const char *fmt);
  const char *name() const {return name_;}
};

// pop up file chooser and return a legal image selected by user,
// or zero for any errors:
Fluid_Image *ui_find_image(const char *);
extern const char *ui_find_image_name;

#endif
