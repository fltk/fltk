//
// Pixmap (and other images) label support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
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

#include "Fluid_Image.h"

#include "fluid.h"
#include "Fl_Group_Type.h"
#include "Fl_Window_Type.h"
#include "file.h"
#include "code.h"

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>
#include "fluid_filename.h"
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

void Fluid_Image::image(Fl_Widget *o) {
  if (o->window() != o) o->image(img);
}

void Fluid_Image::deimage(Fl_Widget *o) {
  if (o->window() != o) o->deimage(img);
}

/** Write the contents of the name() file as binary source code.
 \param fmt short name of file contents for error message
 \return 0 if the file could not be opened or read */
size_t Fluid_Image::write_static_binary(Fd_Code_Writer& f, const char* fmt) {
  size_t nData = 0;
  enter_project_dir();
  FILE *in = fl_fopen(name(), "rb");
  leave_project_dir();
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

/** Write the contents of the name() file as textual source code.
 \param fmt short name of file contents for error message
 \return 0 if the file could not be opened or read */
size_t Fluid_Image::write_static_text(Fd_Code_Writer& f, const char* fmt) {
  size_t nData = 0;
  enter_project_dir();
  FILE *in = fl_fopen(name(), "rb");
  leave_project_dir();
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

void Fluid_Image::write_static_rgb(Fd_Code_Writer& f, const char* idata_name) {
  // Write image data...
  f.write_c("\n");
  f.write_c_once("#include <FL/Fl_Image.H>\n");
  f.write_c("static const unsigned char %s[] =\n", idata_name);
  const int extra_data = img->ld() ? (img->ld()-img->w()*img->d()) : 0;
  f.write_cdata(img->data()[0], (img->w() * img->d() + extra_data) * img->h());
  f.write_c(";\n");
  write_initializer(f, "Fl_RGB_Image", "%s, %d, %d, %d, %d", idata_name, img->w(), img->h(), img->d(), img->ld());
}

/**
 Write the static image data into the soutrce file.

 If \p compressed is set, write the original image format, which requires
 linking the matching image reader at runtime, or if we want to store the raw
 uncompressed pixels, which makes images fast, needs no reader, but takes a
 lot of memory (current default for PNG)

 \param compressed write data in the original compressed file format
 */
void Fluid_Image::write_static(Fd_Code_Writer& f, int compressed) {
  if (!img) return;
  const char *idata_name = f.unique_id(this, "idata", fl_filename_name(name()), 0);
  function_name_ = f.unique_id(this, "image", fl_filename_name(name()), 0);

  if (is_animated_gif_) {
    // Write animated gif image data...
    f.write_c("\n");
    f.write_c_once("#include <FL/Fl_Anim_GIF_Image.H>\n");
    f.write_c("static const unsigned char %s[] =\n", idata_name);
    size_t nData = write_static_binary(f, "AnimGIF");
    f.write_c(";\n");
    write_initializer(f, "Fl_Anim_GIF_Image", "\"%s\", %s, %d", fl_filename_name(name()), idata_name, nData);
  } else if (compressed && fl_ascii_strcasecmp(fl_filename_ext(name()), ".gif")==0) {
    // Write gif image data...
    f.write_c("\n");
    f.write_c_once("#include <FL/Fl_GIF_Image.H>\n");
    f.write_c("static const unsigned char %s[] =\n", idata_name);
    size_t nData = write_static_binary(f, "GIF");
    f.write_c(";\n");
    write_initializer(f, "Fl_GIF_Image", "\"%s\", %s, %d", fl_filename_name(name()), idata_name, nData);
  } else if (compressed && fl_ascii_strcasecmp(fl_filename_ext(name()), ".bmp")==0) {
    // Write bmp image data...
    f.write_c("\n");
    f.write_c_once("#include <FL/Fl_BMP_Image.H>\n");
    f.write_c("static const unsigned char %s[] =\n", idata_name);
    size_t nData = write_static_binary(f, "BMP");
    f.write_c(";\n");
    write_initializer(f, "Fl_BMP_Image", "\"%s\", %s, %d", fl_filename_name(name()), idata_name, nData);
  } else if (img->count() > 1) {
    // Write Pixmap data...
    f.write_c("\n");
    f.write_c_once("#include <FL/Fl_Pixmap.H>\n");
    f.write_c("static const char *%s[] = {\n", idata_name);
    f.write_cstring(img->data()[0], (int)strlen(img->data()[0]));

    int i;
    int ncolors, chars_per_color;
    sscanf(img->data()[0], "%*d%*d%d%d", &ncolors, &chars_per_color);

    if (ncolors < 0) {
      f.write_c(",\n");
      f.write_cstring(img->data()[1], ncolors * -4);
      i = 2;
    } else {
      for (i = 1; i <= ncolors; i ++) {
        f.write_c(",\n");
        f.write_cstring(img->data()[i], (int)strlen(img->data()[i]));
      }
    }
    for (; i < img->count(); i ++) {
      f.write_c(",\n");
      f.write_cstring(img->data()[i], img->w() * chars_per_color);
    }
    f.write_c("\n};\n");
    write_initializer(f, "Fl_Pixmap", "%s", idata_name);
  } else if (img->d() == 0) {
    // Write Bitmap data...
    f.write_c("\n");
    f.write_c_once("#include <FL/Fl_Bitmap.H>\n");
    f.write_c("static const unsigned char %s[] =\n", idata_name);
    f.write_cdata(img->data()[0], ((img->w() + 7) / 8) * img->h());
    f.write_c(";\n");
    write_initializer(f, "Fl_Bitmap", "%s, %d, %d, %d", idata_name, ((img->w() + 7) / 8) * img->h(), img->w(), img->h());
  } else if (compressed && fl_ascii_strcasecmp(fl_filename_ext(name()), ".jpg")==0) {
    // Write jpeg image data...
    f.write_c("\n");
    f.write_c_once("#include <FL/Fl_JPEG_Image.H>\n");
    f.write_c("static const unsigned char %s[] =\n", idata_name);
    size_t nData = write_static_binary(f, "JPEG");
    f.write_c(";\n");
    write_initializer(f, "Fl_JPEG_Image", "\"%s\", %s, %d", fl_filename_name(name()), idata_name, nData);
  } else if (compressed && fl_ascii_strcasecmp(fl_filename_ext(name()), ".png")==0) {
    // Write png image data...
    f.write_c("\n");
    f.write_c_once("#include <FL/Fl_PNG_Image.H>\n");
    f.write_c("static const unsigned char %s[] =\n", idata_name);
    size_t nData = write_static_binary(f, "PNG");
    f.write_c(";\n");
    write_initializer(f, "Fl_PNG_Image", "\"%s\", %s, %d", fl_filename_name(name()), idata_name, nData);
  }
#ifdef FLTK_USE_SVG
  else if (fl_ascii_strcasecmp(fl_filename_ext(name()), ".svg")==0 || fl_ascii_strcasecmp(fl_filename_ext(name()), ".svgz")==0) {
    bool gzipped = (strcmp(fl_filename_ext(name()), ".svgz") == 0);
    // Write svg image data...
    if (compressed) {
      f.write_c("\n");
      f.write_c_once("#include <FL/Fl_SVG_Image.H>\n");
      if (gzipped) {
        f.write_c("static const unsigned char %s[] =\n", idata_name);
        size_t nData = write_static_binary(f, "SVGZ");
        f.write_c(";\n");
        write_initializer(f, "Fl_SVG_Image", "\"%s\", %s, %ld", fl_filename_name(name()), idata_name, nData);
      } else {
        f.write_c("static const char %s[] =\n", idata_name);
        write_static_text(f, "SVG");
        f.write_c(";\n");
        write_initializer(f, "Fl_SVG_Image", "\"%s\", %s", fl_filename_name(name()), idata_name);
      }
    } else {
      // if FLUID runs from the command line, make sure that the image is not
      // only loaded but also rasterized, so we can write the RGB image data
      Fl_RGB_Image* rgb_image = NULL;
      Fl_SVG_Image* svg_image = NULL;
      if (img->d()>0)
        rgb_image = (Fl_RGB_Image*)img->image();
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

void Fluid_Image::write_file_error(Fd_Code_Writer& f, const char *fmt) {
  f.write_c("#warning Cannot read %s file \"%s\": %s\n", fmt, name(), strerror(errno));
  enter_project_dir();
  f.write_c("// Searching in path \"%s\"\n", fl_getcwd(0, FL_PATH_MAX));
  leave_project_dir();
}

void Fluid_Image::write_initializer(Fd_Code_Writer& f, const char *type_name, const char *format, ...) {
  /* Outputs code that returns (and initializes if needed) an Fl_Image as follows:
   static Fl_Image *'function_name_'() {
     static Fl_Image *image = NULL;
     if (!image)
       image = new 'type_name'('product of format and remaining args');
     return image;
   } */
  va_list ap;
  va_start(ap, format);
  f.write_c("static Fl_Image *%s() {\n", function_name_);
  if (is_animated_gif_)
    f.write_c("%sFl_GIF_Image::animate = true;\n", f.indent(1));
  f.write_c("%sstatic Fl_Image *image = NULL;\n", f.indent(1));
  f.write_c("%sif (!image)\n", f.indent(1));
  f.write_c("%simage = new %s(", f.indent(2), type_name);
  f.vwrite_c(format, ap);
  f.write_c(");\n");
  f.write_c("%sreturn image;\n", f.indent(1));
  f.write_c("}\n");
  va_end(ap);
}

void Fluid_Image::write_code(Fd_Code_Writer& f, int bind, const char *var, int inactive) {
  /* Outputs code that attaches an image to an Fl_Widget or Fl_Menu_Item.
   This code calls a function output before by Fluid_Image::write_initializer() */
  if (img) {
    f.write_c("%s%s->%s%s( %s() );\n", f.indent(), var, bind ? "bind_" : "", inactive ? "deimage" : "image", function_name_);
    if (is_animated_gif_)
      f.write_c("%s((Fl_Anim_GIF_Image*)(%s()))->canvas(%s, Fl_Anim_GIF_Image::DONT_RESIZE_CANVAS);\n", f.indent(), function_name_, var);
  }
}

void Fluid_Image::write_inline(Fd_Code_Writer& f, int inactive) {
  if (img)
    f.write_c("%s()", function_name_);
}


////////////////////////////////////////////////////////////////

static Fluid_Image** images = 0; // sorted list
static int numimages = 0;
static int tablesize = 0;

Fluid_Image* Fluid_Image::find(const char *iname) {
  if (!iname || !*iname) return 0;

  // first search to see if it exists already:
  int a = 0;
  int b = numimages;
  while (a < b) {
    int c = (a+b)/2;
    int i = strcmp(iname,images[c]->name_);
    if (i < 0) b = c;
    else if (i > 0) a = c+1;
    else return images[c];
  }

  // no, so now see if the file exists:

  enter_project_dir();
  FILE *f = fl_fopen(iname,"rb");
  if (!f) {
    if (batch_mode)
      fprintf(stderr, "Can't open image file:\n%s\n%s",iname,strerror(errno));
    else
      fl_message("Can't open image file:\n%s\n%s",iname,strerror(errno));
    leave_project_dir();
    return 0;
  }
  fclose(f);

  Fluid_Image *ret = new Fluid_Image(iname);

  if (!ret->img || !ret->img->w() || !ret->img->h()) {
    delete ret;
    ret = 0;
    if (batch_mode)
      fprintf(stderr, "Can't read image file:\n%s\nunrecognized image format",iname);
    else
      fl_message("Can't read image file:\n%s\nunrecognized image format",iname);
  }
  leave_project_dir();
  if (!ret) return 0;

  // make a new entry in the table:
  numimages++;
  if (numimages > tablesize) {
    tablesize = tablesize ? 2*tablesize : 16;
    if (images) images = (Fluid_Image**)realloc(images, tablesize*sizeof(Fluid_Image*));
    else images = (Fluid_Image**)malloc(tablesize*sizeof(Fluid_Image*));
  }
  for (b = numimages-1; b > a; b--) images[b] = images[b-1];
  images[a] = ret;

  return ret;
}

Fluid_Image::Fluid_Image(const char *iname)
  : is_animated_gif_(false)
{
  name_ = fl_strdup(iname);
  written = 0;
  refcount = 0;
  img = Fl_Shared_Image::get(iname);
  if (img && iname) {
    const char *ext = fl_filename_ext(iname);
    if (fl_ascii_strcasecmp(ext, ".gif")==0) {
      int fc = Fl_Anim_GIF_Image::frame_count(iname);
      if (fc > 0) is_animated_gif_ = true;
    }
  }
  function_name_ = NULL;
}

void Fluid_Image::increment() {
  ++refcount;
}

void Fluid_Image::decrement() {
  --refcount;
  if (refcount > 0) return;
  delete this;
}

Fluid_Image::~Fluid_Image() {
  int a;
  if (images) {
    for (a = 0; a<numimages; a++) {
      if (images[a] == this) {
        numimages--;
        for (; a < numimages; a++) {
          images[a] = images[a+1];
        }
        break;
      }
    }
  }
  if (img) img->release();
  free((void*)name_);
}

////////////////////////////////////////////////////////////////

const char *ui_find_image_name;
Fluid_Image *ui_find_image(const char *oldname) {
  enter_project_dir();
  fl_file_chooser_ok_label("Use Image");
  const char *name = fl_file_chooser("Image?",
            "Image Files (*.{bm,bmp,gif,jpg,pbm,pgm,png,ppm,xbm,xpm,svg"
#ifdef HAVE_LIBZ
                        ",svgz"
#endif
                                     "})",
            oldname,1);
  fl_file_chooser_ok_label(NULL);
  ui_find_image_name = name;
  Fluid_Image *ret = (name && *name) ? Fluid_Image::find(name) : 0;
  leave_project_dir();
  return ret;
}
