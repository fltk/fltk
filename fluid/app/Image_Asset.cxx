//
// Image Helper code for the Fast Light Tool Kit (FLTK).
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

#include "app/Image_Asset.h"

#include "Fluid.h"
#include "io/Project_Reader.h"
#include "io/Project_Writer.h"
#include "io/Code_Writer.h"
#include "nodes/Group_Node.h"
#include "nodes/Window_Node.h"
#include "tools/filename.h"

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>
#include <FL/fl_string_functions.h>
#include <FL/fl_utf8.h>     // fl_fopen()
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_SVG_Image.H>
#include <FL/Fl_Anim_GIF_Image.H>
#include "../src/flstring.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <map>
#include <string>


/**
 \brief A map of all image assets.
 \todo This is a global variable, but should be associated 
    with a project instead.
 */
static std::map<std::string, Image_Asset*> image_asset_map;


/** 
 \brief Write the contents of the image file as binary source code.

 Write the contents of the image file as C++ code, so the image is available in 
 the target app in the original binary format, for example:
 ```
 { 1, 2, 3, ...}
 ```
 \param f Write code to this C++ source code file
 \param fmt short name of file contents for error message
 \return 0 if the file could not be opened or read 
 */
size_t Image_Asset::write_static_binary(fld::io::Code_Writer& f, const char* fmt) {
  size_t nData = 0;
  Fluid.proj.enter_project_dir();
  FILE *in = fl_fopen(filename(), "rb");
  Fluid.proj.leave_project_dir();
  if (!in) {
    write_file_error(f, fmt);
    return 0;
  } else {
    fseek(in, 0, SEEK_END);
    nData = ftell(in);
    fseek(in, 0, SEEK_SET);
    if (nData) {
      char *data = (char*)calloc(nData, 1);
      if (fread(data, nData, 1, in)==0) { /* ignore */ }
      f.write_cdata(data, (int)nData);
      free(data);
    }
    fclose(in);
  }
  return nData;
}


/** 
 \brief Write the contents of the image file as text with escaped special characters.

 This function is only useful for writing out image formats that are ASCII text
 based, like svg and pixmaps. Other formats should use write_static_binary().

 \param f Write code to this C++ source code file
 \param fmt short name of file contents for error message
 \return 0 if the file could not be opened or read 
 */
size_t Image_Asset::write_static_text(fld::io::Code_Writer& f, const char* fmt) {
  size_t nData = 0;
  Fluid.proj.enter_project_dir();
  FILE *in = fl_fopen(filename(), "rb");
  Fluid.proj.leave_project_dir();
  if (!in) {
    write_file_error(f, fmt);
    return 0;
  } else {
    fseek(in, 0, SEEK_END);
    nData = ftell(in);
    fseek(in, 0, SEEK_SET);
    if (nData) {
      char *data = (char*)calloc(nData+1, 1);
      if (fread(data, nData, 1, in)==0) { /* ignore */ }
      f.write_cstring(data, (int)nData);
      free(data);
    }
    fclose(in);
  }
  return nData;
}


/** 
 \brief Write the contents of the image file as uncompressed RGB.

 Write source code that generates the uncompressed RGB image data at compile
 time, and an initializer that creates the image at run time.

 \todo If the ld() value is not 0 and not d()*w(), the image data is not 
 written correctly. There is no check if the data is actually in RGB format.

 \param f Write code to this C++ source code file
 \param fmt short name of file contents for error message
 \return 0 if the file could not be opened or read 
 */
void Image_Asset::write_static_rgb(fld::io::Code_Writer& f, const char* idata_name) {
  // Write image data...
  f.write_c("\n");
  f.write_c_once("#include <FL/Fl_Image.H>\n");
  f.write_c("static const unsigned char %s[] =\n", idata_name);
  const int extra_data = image_->ld() ? (image_->ld()-image_->w()*image_->d()) : 0;
  f.write_cdata(image_->data()[0], (image_->w() * image_->d() + extra_data) * image_->h());
  f.write_c(";\n");
  write_initializer(f, "Fl_RGB_Image", "%s, %d, %d, %d, %d", idata_name, image_->w(), image_->h(), image_->d(), image_->ld());
}


/**
 \brief Write the static image data into the source file.

 Write source code that generates the image data at compile time, and an
 initializer that creates the image at run time.

 If \p compressed is set, write the original image format, which requires
 linking the matching image reader at runtime, or if we want to store the raw
 uncompressed pixels, which makes images fast, needs no reader, but takes a
 lot of memory (current default for PNG)

 \param f Write code to this C++ source code file
 \param compressed write data in the original compressed file format
 */
void Image_Asset::write_static(fld::io::Code_Writer& f, int compressed) {
  if (!image_) return;
  const char *idata_name = f.unique_id(this, "idata", fl_filename_name(filename()), nullptr);
  initializer_function_ = f.unique_id(this, "image", fl_filename_name(filename()), nullptr);

  if (is_animated_gif_) {
    // Write animated gif image data...
    f.write_c("\n");
    f.write_c_once("#include <FL/Fl_Anim_GIF_Image.H>\n");
    f.write_c("static const unsigned char %s[] =\n", idata_name);
    size_t nData = write_static_binary(f, "AnimGIF");
    f.write_c(";\n");
    write_initializer(f, "Fl_Anim_GIF_Image", "\"%s\", %s, %d", fl_filename_name(filename()), idata_name, nData);
  } else if (compressed && fl_ascii_strcasecmp(fl_filename_ext(filename()), ".gif")==0) {
    // Write gif image data...
    f.write_c("\n");
    f.write_c_once("#include <FL/Fl_GIF_Image.H>\n");
    f.write_c("static const unsigned char %s[] =\n", idata_name);
    size_t nData = write_static_binary(f, "GIF");
    f.write_c(";\n");
    write_initializer(f, "Fl_GIF_Image", "\"%s\", %s, %d", fl_filename_name(filename()), idata_name, nData);
  } else if (compressed && fl_ascii_strcasecmp(fl_filename_ext(filename()), ".bmp")==0) {
    // Write bmp image data...
    f.write_c("\n");
    f.write_c_once("#include <FL/Fl_BMP_Image.H>\n");
    f.write_c("static const unsigned char %s[] =\n", idata_name);
    size_t nData = write_static_binary(f, "BMP");
    f.write_c(";\n");
    write_initializer(f, "Fl_BMP_Image", "\"%s\", %s, %d", fl_filename_name(filename()), idata_name, nData);
  } else if (image_->count() > 1) {
    // Write Pixmap data...
    f.write_c("\n");
    f.write_c_once("#include <FL/Fl_Pixmap.H>\n");
    f.write_c("static const char *%s[] = {\n", idata_name);
    f.write_cstring(image_->data()[0], (int)strlen(image_->data()[0]));

    int i;
    int ncolors, chars_per_color;
    sscanf(image_->data()[0], "%*d%*d%d%d", &ncolors, &chars_per_color);

    if (ncolors < 0) {
      f.write_c(",\n");
      f.write_cstring(image_->data()[1], ncolors * -4);
      i = 2;
    } else {
      for (i = 1; i <= ncolors; i ++) {
        f.write_c(",\n");
        f.write_cstring(image_->data()[i], (int)strlen(image_->data()[i]));
      }
    }
    for (; i < image_->count(); i ++) {
      f.write_c(",\n");
      f.write_cstring(image_->data()[i], image_->w() * chars_per_color);
    }
    f.write_c("\n};\n");
    write_initializer(f, "Fl_Pixmap", "%s", idata_name);
  } else if (image_->d() == 0) {
    // Write Bitmap data...
    f.write_c("\n");
    f.write_c_once("#include <FL/Fl_Bitmap.H>\n");
    f.write_c("static const unsigned char %s[] =\n", idata_name);
    f.write_cdata(image_->data()[0], ((image_->w() + 7) / 8) * image_->h());
    f.write_c(";\n");
    write_initializer(f, "Fl_Bitmap", "%s, %d, %d, %d", idata_name, ((image_->w() + 7) / 8) * image_->h(), image_->w(), image_->h());
  } else if (compressed && fl_ascii_strcasecmp(fl_filename_ext(filename()), ".jpg")==0) {
    // Write jpeg image data...
    f.write_c("\n");
    f.write_c_once("#include <FL/Fl_JPEG_Image.H>\n");
    f.write_c("static const unsigned char %s[] =\n", idata_name);
    size_t nData = write_static_binary(f, "JPEG");
    f.write_c(";\n");
    write_initializer(f, "Fl_JPEG_Image", "\"%s\", %s, %d", fl_filename_name(filename()), idata_name, nData);
  } else if (compressed && fl_ascii_strcasecmp(fl_filename_ext(filename()), ".png")==0) {
    // Write png image data...
    f.write_c("\n");
    f.write_c_once("#include <FL/Fl_PNG_Image.H>\n");
    f.write_c("static const unsigned char %s[] =\n", idata_name);
    size_t nData = write_static_binary(f, "PNG");
    f.write_c(";\n");
    write_initializer(f, "Fl_PNG_Image", "\"%s\", %s, %d", fl_filename_name(filename()), idata_name, nData);
  }
#ifdef FLTK_USE_SVG
  else if (fl_ascii_strcasecmp(fl_filename_ext(filename()), ".svg")==0 || fl_ascii_strcasecmp(fl_filename_ext(filename()), ".svgz")==0) {
    bool gzipped = (strcmp(fl_filename_ext(filename()), ".svgz") == 0);
    // Write svg image data...
    if (compressed) {
      f.write_c("\n");
      f.write_c_once("#include <FL/Fl_SVG_Image.H>\n");
      if (gzipped) {
        f.write_c("static const unsigned char %s[] =\n", idata_name);
        size_t nData = write_static_binary(f, "SVGZ");
        f.write_c(";\n");
        write_initializer(f, "Fl_SVG_Image", "\"%s\", %s, %ld", fl_filename_name(filename()), idata_name, nData);
      } else {
        f.write_c("static const char %s[] =\n", idata_name);
        write_static_text(f, "SVG");
        f.write_c(";\n");
        write_initializer(f, "Fl_SVG_Image", "\"%s\", %s", fl_filename_name(filename()), idata_name);
      }
    } else {
      // if FLUID runs from the command line, make sure that the image is not
      // only loaded but also rasterized, so we can write the RGB image data
      Fl_RGB_Image* rgb_image = nullptr;
      Fl_SVG_Image* svg_image = nullptr;
      if (image_->d()>0)
        rgb_image = (Fl_RGB_Image*)image_->image();
      if (rgb_image)
        svg_image = rgb_image->as_svg_image();
      if (svg_image) {
        svg_image->resize(svg_image->w(), svg_image->h());
        write_static_rgb(f, idata_name);
      } else {
        write_file_error(f, "RGB_from_SVG");
      }
    }
  }
#endif // FLTK_USE_SVG
  else {
    write_static_rgb(f, idata_name);
  }
}


/**
 \brief Write a warning message to the generated code file that the image asset
 can't be read.

 This writes a #warning directive to the generated code file which contains the
 filename of the image asset and the error message as returned by strerror(errno).
 The current working directory is also printed for debugging purposes.
 \param f The C++ source code file to write the warning message to.
 \param fmt The format string for the image file type.
 */
void Image_Asset::write_file_error(fld::io::Code_Writer& f, const char *fmt) {
  f.write_c("#warning Cannot read %s file \"%s\": %s\n", fmt, filename(), strerror(errno));
  Fluid.proj.enter_project_dir();
  f.write_c("// Searching in path \"%s\"\n", fl_getcwd(nullptr, FL_PATH_MAX));
  Fluid.proj.leave_project_dir();
}


/**
 \brief Outputs code that loads and returns an Fl_Image.

 The generated code loads the image if it hasn't been loaded yet, and then
 returns a pointer to the image.

 \code 
  static Fl_Image *'initializer_function_'() {
    static Fl_Image *image = 0L;
    if (!image)
      image = new 'type_name'('product of format and remaining args');
    return image;
  }
 \endcode  
 
 \param f Write the C++ code to this file.
 \param image_class Name of the Fl_Image class, for example Fl_GIF_Image.
 \param format Format string for additional parameters for the constructor.
  */
void Image_Asset::write_initializer(fld::io::Code_Writer& f, const char *image_class, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  f.write_c("static Fl_Image *%s() {\n", initializer_function_.c_str());
  if (is_animated_gif_)
    f.write_c("%sFl_GIF_Image::animate = true;\n", f.indent(1));
  f.write_c("%sstatic Fl_Image *image = 0L;\n", f.indent(1));
  f.write_c("%sif (!image)\n", f.indent(1));
  f.write_c("%simage = new %s(", f.indent(2), image_class);
  f.vwrite_c(format, ap);
  f.write_c(");\n");
  f.write_c("%sreturn image;\n", f.indent(1));
  f.write_c("}\n");
  va_end(ap);
}


/**
 \brief Outputs code that attaches an image to an Fl_Widget or Fl_Menu_Item.

 The generated code will call the image initializer function and assign
 the resulting image to the widget.
 
 \param f Write the C++ code to this file.
 \param bind If true, use bind_image() instead of image().
 \param var Name of the Fl_Widget or Fl_Menu_Item to attach the image to.
 \param inactive If true, use deimage() instead of image().
 */
void Image_Asset::write_code(fld::io::Code_Writer& f, int bind, const char *var, int inactive) {
  if (image_) {
    f.write_c("%s%s->%s%s( %s() );\n", 
      f.indent(), 
      var, 
      bind ? "bind_" : "", 
      inactive ? "deimage" : "image", 
      initializer_function_.c_str());
    if (is_animated_gif_)
      f.write_c("%s((Fl_Anim_GIF_Image*)(%s()))->canvas(%s, Fl_Anim_GIF_Image::DONT_RESIZE_CANVAS);\n", 
        f.indent(), 
        initializer_function_.c_str(), 
        var);
  }
}


/**
 \brief Outputs code that calls the image initializer function.

 The generated code calls the image initializer function, loading an image and
 assigning it to a Fl_Menu_Item.

 \param f Write the C++ code to this file.
 \param inactive Unused.
 */
void Image_Asset::write_inline(fld::io::Code_Writer& f, int inactive) {
  (void)inactive;
  if (image_) {
    f.write_c("%s()", initializer_function_.c_str());
  }
}


/**
 \brief Finds an image asset by filename.

 If the image asset has already been loaded, it is returned from the cache.
 If the image asset has not been loaded, it is loaded from the file system.
 If the image asset cannot be loaded, nullptr is returned.

 \param iname The filename of the image asset to find.
 \returns The image asset, or nullptr if it cannot be loaded.
 */
Image_Asset* Image_Asset::find(const char *iname) {
  if (!iname || !*iname) return nullptr;

  // First search to see if it exists already. If it does, return it.
  auto result = image_asset_map.find(iname);
  if (result != image_asset_map.end())
    return result->second;

  // Check if a file by that name exists.
  Fluid.proj.enter_project_dir();
  FILE *f = fl_fopen(iname,"rb");
  if (!f) {
    if (Fluid.batch_mode)
      fprintf(stderr, "Can't open image file:\n%s\n%s",iname,strerror(errno));
    else
      fl_message("Can't open image file:\n%s\n%s",iname,strerror(errno));
    Fluid.proj.leave_project_dir();
    return nullptr;
  }
  fclose(f);

  // We found the file. Create the asset.
  Image_Asset *asset = new Image_Asset(iname);
  if (!asset->image_ || !asset->image_->w() || !asset->image_->h()) {
    delete asset;
    if (Fluid.batch_mode)
      fprintf(stderr, "Can't read image file:\n%s\nunrecognized image format",iname);
    else
      fl_message("Can't read image file:\n%s\nunrecognized image format",iname);
    Fluid.proj.leave_project_dir();
    return nullptr;
  }

  // Add the new asset to our image asset map and return it to the caller.
  image_asset_map[iname] = asset;
  return asset;
}

/**
 \brief Construct an image asset from a file in the project directory.

 This constructor creates an image asset from a file in the project
 directory. The image file is loaded and stored in the map of image
 assets. The image asset is given a reference count of 1, which means
 that it will be destroyed when all references to it have been released.
 The constructor also sets the flag to indicate if the image is an
 animated GIF. This information is used when generating code for the
 image asset.

 \param iname The name of the image file in the project directory.
*/
Image_Asset::Image_Asset(const char *iname)
{
  filename_ = iname;
  image_ = Fl_Shared_Image::get(iname);
  if (image_ && iname) {
    const char *ext = fl_filename_ext(iname);
    if (fl_ascii_strcasecmp(ext, ".gif")==0) {
      int fc = Fl_Anim_GIF_Image::frame_count(iname);
      if (fc > 0) is_animated_gif_ = true;
    }
  }
}

/**
 \brief Increments the reference count of the image asset.

 This method increments the reference count of the image asset. The
 reference count is used to keep track of how many times the image asset
 is referenced in the user interface. When the reference count reaches zero,
 the image asset is destroyed.
*/
void Image_Asset::inc_ref() {
  ++refcount_;
}

/**
 \brief Decrements the reference count of the image asset.

 This method decrements the reference count of the image asset. The
 reference count is used to keep track of how many times the image asset
 is referenced in the user interface. If the reference count reaches zero,
 the image asset is destroyed.
*/
void Image_Asset::dec_ref() {
  --refcount_;
  if (refcount_ > 0) return;
  delete this;
}

/**
 \brief Destructor for the Image_Asset class.

 This destructor removes the image asset from the global image asset map
 and releases the associated shared image if it exists. It ensures that 
 any resources associated with the Image_Asset are properly cleaned up 
 when the object is destroyed.
*/
Image_Asset::~Image_Asset() {
  image_asset_map.erase(filename_);
  if (image_) image_->release();
}

////////////////////////////////////////////////////////////////

/**
 \brief Displays a file chooser for the user to select an image file.

 This function displays a file chooser dialog with the title "Image?" and
 the current working directory set to the project directory. The file
 chooser displays files with the extensions .bm, .bmp, .gif, .jpg, .pbm,
 .pgm, .png, .ppm, .xbm, .xpm, and .svg (and .svgz if zlib support is
 enabled). The function returns a pointer to an Image_Asset object that
 references the selected image. If the user cancels the file chooser or
 selects a file that does not exist, the function returns nullptr.

 \param oldname The default filename to display in the file chooser.

 \return A pointer to an Image_Asset object that references the selected
 image, or nullptr if the user cancels the file chooser or selects a file
 that does not exist. The asset is automaticly added to the global image
 asset map.
*/
Image_Asset *ui_find_image(const char *oldname) {
  Fluid.proj.enter_project_dir();
  fl_file_chooser_ok_label("Use Image");
  const char *name = fl_file_chooser("Image?",
            "Image Files (*.{bm,bmp,gif,jpg,pbm,pgm,png,ppm,xbm,xpm,svg"
#ifdef HAVE_LIBZ
                        ",svgz"
#endif
                                     "})",
            oldname,1);
  fl_file_chooser_ok_label(nullptr);
  Image_Asset *ret = (name && *name) ? Image_Asset::find(name) : nullptr;
  Fluid.proj.leave_project_dir();
  return ret;
}

