//
// Image Helper header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2026 by Bill Spitzak and others.
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

#include <map>
#include <memory>
#include <string>


class Image_Asset;

/**
 \brief Cache of all active Image_Asset objects, keyed by filename.

 Holds weak references so assets are released automatically when no
 Widget_Node holds a shared_ptr to them anymore.
 */
class Image_Asset_Map {
  std::map<std::string, std::weak_ptr<Image_Asset>> map_;
public:
  std::shared_ptr<Image_Asset> find(const std::string& name) const;
  void insert(const std::string& name, std::shared_ptr<Image_Asset> asset);
  void erase(const std::string& name);
};


class Image_Asset
{
private:  // member variables
  std::string filename_;                  ///< Relative path to the image file
  std::string initializer_function_;      ///< The name of the initializer function
  std::unique_ptr<Fl_Shared_Image, Fl_Shared_Image::Deleter> image_; ///< The actual image as managed by FLTK
  bool is_animated_gif_ = false;          ///< It's an animated gif.

private: // methods
  Image_Asset(const std::string& name); // use find() to obtain instances

  size_t write_static_binary(fld::io::Code_Writer& f, const char* fmt);
  size_t write_static_text(fld::io::Code_Writer& f, const char* fmt);
  void write_static_rgb(fld::io::Code_Writer& f, const char* idata_name);

public: // methods
  ~Image_Asset();

  static std::shared_ptr<Image_Asset> find(const std::string&);
  Fl_Shared_Image *image() const { return image_.get(); }
  const char *filename() const { return filename_.c_str(); }

  void write_static(fld::io::Code_Writer& f, int compressed);
  void write_initializer(fld::io::Code_Writer& f, const char *type_name, const char *format, ...);
  void write_code(fld::io::Code_Writer& f, int bind, const char *var, int inactive = 0);
  void write_inline(fld::io::Code_Writer& f, int inactive = 0);
  void write_file_error(fld::io::Code_Writer& f, const char *fmt);
};

// pop up file chooser and return a legal image selected by user,
// or nullptr for any errors:
std::shared_ptr<Image_Asset> ui_find_image(const char *);

#endif // APP_IMAGE_ASSET_H
