//
// "$Id$"
//
// Pixmap label support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2008 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include "Fl_Type.h"
#include "Fluid_Image.h"
#include "../src/flstring.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <FL/filename.H>

extern void goto_source_dir(); // in fluid.C
extern void leave_source_dir(); // in fluid.C

void Fluid_Image::image(Fl_Widget *o) {
  if (o->window() != o) o->image(img);
}

void Fluid_Image::deimage(Fl_Widget *o) {
  if (o->window() != o) o->deimage(img);
}

static int pixmap_header_written = 0;
static int bitmap_header_written = 0;
static int image_header_written = 0;

void Fluid_Image::write_static() {
  if (!img) return;
  if (img->count() > 1) {
    // Write Pixmap data...
    write_c("\n");
    if (pixmap_header_written != write_number) {
      write_c("#include <FL/Fl_Pixmap.H>\n");
      pixmap_header_written = write_number;
    }
    write_c("static const char *%s[] = {\n",
	    unique_id(this, "idata", fl_filename_name(name()), 0));
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
    write_c("static Fl_Pixmap %s(%s);\n",
	    unique_id(this, "image", fl_filename_name(name()), 0),
	    unique_id(this, "idata", fl_filename_name(name()), 0));
  } else if (img->d() == 0) {
    // Write Bitmap data...
    write_c("\n");
    if (bitmap_header_written != write_number) {
      write_c("#include <FL/Fl_Bitmap.H>\n");
      bitmap_header_written = write_number;
    }
    write_c("static unsigned char %s[] =\n",
	    unique_id(this, "idata", fl_filename_name(name()), 0));
    write_cdata(img->data()[0], ((img->w() + 7) / 8) * img->h());
    write_c(";\n");
    write_c("static Fl_Bitmap %s(%s, %d, %d);\n",
	    unique_id(this, "image", fl_filename_name(name()), 0),
	    unique_id(this, "idata", fl_filename_name(name()), 0),
	    img->w(), img->h());
  } else {
    // Write image data...
    write_c("\n");
    if (image_header_written != write_number) {
      write_c("#include <FL/Fl_Image.H>\n");
      image_header_written = write_number;
    }
    write_c("static unsigned char %s[] =\n",
	    unique_id(this, "idata", fl_filename_name(name()), 0));
    write_cdata(img->data()[0], (img->w() * img->d() + img->ld()) * img->h());
    write_c(";\n");
    write_c("static Fl_RGB_Image %s(%s, %d, %d, %d, %d);\n",
	    unique_id(this, "image", fl_filename_name(name()), 0),
	    unique_id(this, "idata", fl_filename_name(name()), 0),
	    img->w(), img->h(), img->d(), img->ld());
  }
}

void Fluid_Image::write_code(const char *var, int inactive) {
  if (!img) return;
  write_c("%s%s->%s(%s);\n", indent(), var, inactive ? "deimage" : "image",
	  unique_id(this, "image", fl_filename_name(name()), 0));
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
  FILE *f = fopen(iname,"rb");
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
