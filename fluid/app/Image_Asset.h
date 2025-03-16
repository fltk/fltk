//
// Image Helper header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

// This class stores the image labels for widgets in fluid.  This is
// not a class in FLTK itself, and will produce different types of
// code depending on what the image type is.


#ifndef APP_IMAGE_ASSET_H
#define APP_IMAGE_ASSET_H

#include "io/Code_Writer.h"

#include <FL/Fl_Shared_Image.H>

class Image_Asset {

private:  // member variables
  bool is_animated_gif_ = false;          ///< It's an animated gif.
  std::string filename_ { };              ///< Relative path to the image file
  int refcount_ = 0;                      ///< Reference count
  Fl_Shared_Image *image_ = nullptr;      ///< The actual image as managed by FLTK
  std::string initializer_function_ { };  ///< The name of the initializer function

private: // methods
  Image_Asset(const char *name); // no public constructor
  ~Image_Asset(); // no public destructor
  size_t write_static_binary(fld::io::Code_Writer& f, const char* fmt);
  size_t write_static_text(fld::io::Code_Writer& f, const char* fmt);
  void write_static_rgb(fld::io::Code_Writer& f, const char* idata_name);

public: // methods
  static Image_Asset* find(const char *);
  void dec_ref(); // reference counting & automatic free
  void inc_ref();
  Fl_Shared_Image *image() const { return image_; }
  void write_static(fld::io::Code_Writer& f, int compressed);
  void write_initializer(fld::io::Code_Writer& f, const char *type_name, const char *format, ...);
  void write_code(fld::io::Code_Writer& f, int bind, const char *var, int inactive = 0);
  void write_inline(fld::io::Code_Writer& f, int inactive = 0);
  void write_file_error(fld::io::Code_Writer& f, const char *fmt);
  const char *filename() const { return filename_.c_str(); }
};

// pop up file chooser and return a legal image selected by user,
// or zero for any errors:
Image_Asset *ui_find_image(const char *);
extern const char *ui_find_image_name;

#endif // APP_IMAGE_ASSET_H
