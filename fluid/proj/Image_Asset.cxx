//
// Image Helper code for the Fast Light Tool Kit (FLTK).
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

#include "proj/Image_Asset.h"

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
 \brief Write the contents of the image file as a binary data array.

 Emits the raw bytes of the image file as a C++ initializer list, so the
 image is embedded in the target application in its original compressed
 format (e.g. GIF, BMP, JPEG, PNG, SVGZ).

 \param f   Code writer to emit the data into.
 \param fmt Short image type name used in the error message (e.g. "GIF", "PNG").
 \return Number of bytes written, or 0 if the file could not be opened or read.
 */
size_t Image_Asset::write_static_binary(fld::io::Code_Writer& f, const char* fmt) {
  size_t nData = 0;
  map_->project().enter_project_dir();
  FILE *in = fl_fopen(filename(), "rb");
  map_->project().leave_project_dir();
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
 \brief Write the contents of the image file as an escaped C string literal.

 Intended for text-based image formats (SVG, XPM). The file content is written
 as a C string with special characters escaped, rather than as raw binary.
 Use write_static_binary() for binary formats.

 \param f   Code writer to emit the string into.
 \param fmt Short image type name used in the error message (e.g. "SVG").
 \return Number of bytes written, or 0 if the file could not be opened or read.
 */
size_t Image_Asset::write_static_text(fld::io::Code_Writer& f, const char* fmt) {
  size_t nData = 0;
  map_->project().enter_project_dir();
  FILE *in = fl_fopen(filename(), "rb");
  map_->project().leave_project_dir();
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
 \brief Write the image's decoded pixel data as an uncompressed RGB array.

 Emits a static byte array containing the raw pixel data already held in
 memory by the Fl_Shared_Image, then calls write_initializer() to emit an
 Fl_RGB_Image constructor that references it. This avoids the need to link
 an image-format reader in the target application at the cost of larger code.

 \todo If ld() is non-zero and differs from w()*d(), the row stride is not
 handled correctly. There is also no check that the data is truly in RGB format.

 \param f          Code writer to emit the array and initializer into.
 \param idata_name Variable name to use for the generated static data array.
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
 \brief Write static image data and its runtime initializer into the source file.

 Chooses the appropriate output strategy based on the image type and the
 \p compressed flag:
 - If \p compressed is set, the image is written in its original file format
   (GIF, BMP, JPEG, PNG, SVG/SVGZ), which produces compact code but requires
   the matching FLTK image reader to be linked into the target application.
 - If \p compressed is not set, the decoded pixel data is written as a raw RGB
   array via write_static_rgb(), which needs no reader but uses more memory.
 Pixmap and Bitmap images always use their native in-memory representation.

 \param f          Code writer to emit the data and initializer into.
 \param compressed If non-zero, embed the original compressed file bytes.
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
    f.write_c("static const char* %s[] = {\n", idata_name);
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
 \brief Write a \c \#warning and a path comment into the generated source file.

 Emits a compiler warning directive containing the filename and the system
 error string (strerror(errno)), so build failures are visible at compile
 time. Also writes a comment with the current working directory to help
 diagnose path resolution problems.

 \param f   Code writer to emit the diagnostic lines into.
 \param fmt Short image type name shown in the warning (e.g. "GIF", "PNG").
 */
void Image_Asset::write_file_error(fld::io::Code_Writer& f, const char *fmt) {
  f.write_c("#warning Cannot read %s file \"%s\": %s\n", fmt, filename(), strerror(errno));
  map_->project().enter_project_dir();
  f.write_c("// Searching in path \"%s\"\n", fl_getcwd(nullptr, FL_PATH_MAX));
  map_->project().leave_project_dir();
}


/**
 \brief Emit a static loader function that constructs the image on first call.

 Writes a file-scoped function named \c initializer_function_ that lazily
 constructs the image the first time it is called and caches it in a local
 static variable. The generated pattern is:

 \code
  static Fl_Image* <initializer_function_>() {
    static Fl_Image* image = nullptr;
    if (!image)
      image = new <image_class>(<format args>);
    return image;
  }
 \endcode

 \p initializer_function_ must have been set by write_static() before this
 method is called.

 \param f           Code writer to emit the function into.
 \param image_class FLTK image class name (e.g. "Fl_GIF_Image", "Fl_RGB_Image").
 \param format      printf-style format string for the constructor arguments,
                    followed by the matching variadic arguments.
 */
void Image_Asset::write_initializer(fld::io::Code_Writer& f, const char *image_class, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  f.write_c("static Fl_Image* %s() {\n", initializer_function_.c_str());
  if (is_animated_gif_)
    f.write_c("%sFl_GIF_Image::animate = true;\n", f.indent(1));
  f.write_c("%sstatic Fl_Image* image = nullptr;\n", f.indent(1));
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
 \brief Emit a bare call expression for the image initializer function.

 Writes the initializer function name followed by `()` with no statement
 terminator, so the call can be placed inline inside a larger expression —
 for example as the image field of an Fl_Menu_Item initializer list.

 \param f        Code writer to emit the expression into.
 \param inactive Unused; present for API symmetry with write_code().
 */
void Image_Asset::write_inline(fld::io::Code_Writer& f, int inactive) {
  (void)inactive;
  if (image_) {
    f.write_c("%s()", initializer_function_.c_str());
  }
}


/**
 \brief Construct an image asset from a file in the project directory.

 The caller (Image_Asset_Map::find_or_create) must have entered the project
 directory before constructing, so Fl_Shared_Image::get resolves the path.

 \param iname The name of the image file in the project directory.
 \param map   The owning map; stored for use by the destructor and write methods.
*/
Image_Asset::Image_Asset(const std::string& iname, Image_Asset_Map& map)
  : filename_(iname), map_(&map)
{
  image_.reset(Fl_Shared_Image::get(iname.c_str()));
  if (image_ && !iname.empty()) {
    std::string ext = fl_filename_ext_str(iname);
    if (fl_ascii_strcasecmp(ext.c_str(), ".gif")==0) {
      int fc = Fl_Anim_GIF_Image::frame_count(iname.c_str());
      if (fc > 0) is_animated_gif_ = true;
    }
  }
}

/**
 \brief Destructor: removes this asset from its owning map.
*/
Image_Asset::~Image_Asset() {
  map_->erase(filename_);
}

////////////////////////////////////////////////////////////////

/**
 \brief Show a file chooser and return the image asset selected by the user.

 Opens the file chooser in the current project's directory (Fluid.proj).
 If the user picks a file, it is loaded via the project's image asset cache;
 an existing cached asset is reused rather than reloaded.

 \param oldname Filename pre-filled in the chooser, or nullptr for none.
 \return The selected asset, or nullptr if the user cancelled or the file
         could not be loaded.
 */
std::shared_ptr<Image_Asset> ui_find_image(const char *oldname) {
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
  std::shared_ptr<Image_Asset> ret = (name && *name) ? Fluid.proj.image_assets.find_or_create(name) : nullptr;
  Fluid.proj.leave_project_dir();
  return ret;
}

