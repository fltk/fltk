//
// "$Id$"
//
// Pixmap (and other images) label support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include "Fl_Type.h"
#include "Fluid_Image.h"
#include "../src/flstring.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <FL/filename.H>

extern void goto_source_dir(); // in fluid.cxx
extern void leave_source_dir(); // in fluid.cxx

void Fluid_Image::image(Fl_Widget *o) {
  if (o->window() != o) o->image(img);
}

void Fluid_Image::deimage(Fl_Widget *o) {
  if (o->window() != o) o->deimage(img);
}

static int pixmap_header_written = 0;
static int bitmap_header_written = 0;
static int image_header_written = 0;
static int jpeg_header_written = 0;

void Fluid_Image::write_static() {
  if (!img) return;
  const char *idata_name = unique_id(this, "idata", fl_filename_name(name()), 0);
  function_name_ = unique_id(this, "image", fl_filename_name(name()), 0);
  if (img->count() > 1) {
    // Write Pixmap data...
    write_c("\n");
    if (pixmap_header_written != write_number) {
      write_c("#include <FL/Fl_Pixmap.H>\n");
      pixmap_header_written = write_number;
    }
    write_c("static const char *%s[] = {\n", idata_name);
    write_cstring(img->data()[0], strlen(img->data()[0]));

    int i;
    int ncolors, chars_per_color;
    sscanf(img->data()[0], "%*d%*d%d%d", &ncolors, &chars_per_color);

    if (ncolors < 0) {
      write_c(",\n");
      write_cstring(img->data()[1], ncolors * -4);
      i = 2;
    } else {
      for (i = 1; i <= ncolors; i ++) {
        write_c(",\n");
        write_cstring(img->data()[i], strlen(img->data()[i]));
      }
    }
    for (; i < img->count(); i ++) {
      write_c(",\n");
      write_cstring(img->data()[i], img->w() * chars_per_color);
    }
    write_c("\n};\n");
    write_initializer("Fl_Pixmap", "%s", idata_name);
  } else if (img->d() == 0) {
    // Write Bitmap data...
    write_c("\n");
    if (bitmap_header_written != write_number) {
      write_c("#include <FL/Fl_Bitmap.H>\n");
      bitmap_header_written = write_number;
    }
    write_c("static const unsigned char %s[] =\n", idata_name);
    write_cdata(img->data()[0], ((img->w() + 7) / 8) * img->h());
    write_c(";\n");
    write_initializer( "Fl_Bitmap", "%s, %d, %d", idata_name, img->w(), img->h());
  } else if (strcmp(fl_filename_ext(name()), ".jpg")==0) {
    // Write jpeg image data...
    write_c("\n");
    if (jpeg_header_written != write_number) {
      write_c("#include <FL/Fl_JPEG_Image.H>\n");
      jpeg_header_written = write_number;
    }
    write_c("static const unsigned char %s[] =\n", idata_name);

    FILE *f = fl_fopen(name(), "rb");
    if (!f) {
      // message = "Can't include binary file. Can't open";
    } else {
      fseek(f, 0, SEEK_END);
      size_t nData = ftell(f);
      fseek(f, 0, SEEK_SET);
      if (nData) {
        char *data = (char*)calloc(nData, 1);
        if (fread(data, nData, 1, f)==0) { /* ignore */ }
        write_cdata(data, nData);
        free(data);
      }
      fclose(f);
    }
    
    write_c(";\n");
    write_initializer("Fl_JPEG_Image", "\"%s\", %s", fl_filename_name(name()), idata_name);
  } else {
    // Write image data...
    write_c("\n");
    if (image_header_written != write_number) {
      write_c("#include <FL/Fl_Image.H>\n");
      image_header_written = write_number;
    } 
    write_c("static const unsigned char %s[] =\n", idata_name);
    const int extra_data = img->ld() ? (img->ld()-img->w()*img->d()) : 0;
    write_cdata(img->data()[0], (img->w() * img->d() + extra_data) * img->h());
    write_c(";\n");
    write_initializer("Fl_RGB_Image", "%s, %d, %d, %d, %d", idata_name, img->w(), img->h(), img->d(), img->ld());
  }
}

void Fluid_Image::write_initializer(const char *type_name, const char *format, ...) {
  /* Outputs code that returns (and initializes if needed) an Fl_Image as follows:
   static Fl_Image *'function_name_'() {
     static Fl_Image *image = new 'type_name'('product of format and remaining args');
     return image;
   } */
  va_list ap;
  va_start(ap, format);
  write_c("static Fl_Image *%s() {\n  static Fl_Image *image = new %s(", function_name_, type_name);
  vwrite_c(format, ap);
  write_c(");\n  return image;\n}\n");
  va_end(ap);
}

void Fluid_Image::write_code(const char *var, int inactive) {
  /* Outputs code that attaches an image to an Fl_Widget or Fl_Menu_Item.
   This code calls a function output before by Fluid_Image::write_initializer() */
  if (img) write_c("%s%s->%s( %s() );\n", indent(), var, inactive ? "deimage" : "image", function_name_);
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

  goto_source_dir();
  FILE *f = fl_fopen(iname,"rb");
  if (!f) {
    read_error("%s : %s",iname,strerror(errno));
    leave_source_dir();
    return 0;
  }
  fclose(f);

  Fluid_Image *ret = new Fluid_Image(iname);

  if (!ret->img || !ret->img->w() || !ret->img->h()) {
    delete ret;
    ret = 0;
    read_error("%s : unrecognized image format", iname);
  }
  leave_source_dir();
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

Fluid_Image::Fluid_Image(const char *iname) {
  name_ = strdup(iname);
  written = 0;
  refcount = 0;
  img = Fl_Shared_Image::get(iname);
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
    for (a = 0;; a++) if (images[a] == this) break;
    numimages--;
    for (; a < numimages; a++) images[a] = images[a+1];
  }
  if (img) img->release();
  free((void*)name_);
}

////////////////////////////////////////////////////////////////

#include <FL/Fl_File_Chooser.H>

const char *ui_find_image_name;
Fluid_Image *ui_find_image(const char *oldname) {
  goto_source_dir();
  fl_file_chooser_ok_label("Use Image");
  const char *name = fl_file_chooser("Image?","Image Files (*.{bm,bmp,gif,jpg,pbm,pgm,png,ppm,xbm,xpm})",oldname,1);
  fl_file_chooser_ok_label(NULL);
  ui_find_image_name = name;
  Fluid_Image *ret = (name && *name) ? Fluid_Image::find(name) : 0;
  leave_source_dir();
  return ret;
}


//
// End of "$Id$".
//
