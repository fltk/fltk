//
// "$Id: Fl_Shared_Image.cxx,v 1.23 2001/09/10 01:16:17 spitzak Exp $"
//
// Image drawing code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

// Draw an image that is stored compressed in a file or in memory. 
// Keep uncompressed images in memory for later use. 

#include <config.h>
#include <fltk/Fl.h>
#include <fltk/fl_draw.h>
#include <fltk/Fl_Shared_Image.h>
#include <fltk/Fl_Bitmap.h>
#include <fltk/x.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


FL_IMAGES_API const char *Fl_Shared_Image::fl_shared_image_root=0;

FL_IMAGES_API Fl_Shared_Image  *Fl_Shared_Image::first_image = 0;

FL_IMAGES_API int Fl_Shared_Image::image_used=0;
FL_IMAGES_API size_t Fl_Shared_Image::mem_usage_limit=0;

FL_IMAGES_API size_t Fl_Shared_Image::mem_used=0;
FL_IMAGES_API int Fl_Shared_Image::forbid_delete = 1;

//     static size_t mem_used=0; (now moved to Fl.cxx !)
// This contains the total number of pixmap pixels in the cache
// WARNING : this is updated incrementally, so beware that it keeps balanced
// when deleting or creating pixmaps !

Fl_Shared_Image::~Fl_Shared_Image()
{
  if(forbid_delete)
    fprintf(stderr, 
      "FLTK user error : deleting an Fl_Shared_Image object is forbiden !\n");
  if(id) mem_used -= w*h;
}

void Fl_Shared_Image::set_cache_size(size_t l)
{
  mem_usage_limit = l;
}

static Fl_Shared_Image *limage; // used to find the less used image
void Fl_Shared_Image::find_less_used() {
  if(l1) l1->find_less_used();
  if(l2) l2->find_less_used();
  if(id && (limage->id == 0 || used<limage->used)) limage=this;
}
void Fl_Shared_Image::check_mem_usage()
{
  if(mem_usage_limit==0 || first_image==NULL || mem_used < mem_usage_limit) 
    return;

  do {
    limage=first_image;
    first_image->find_less_used();
    if(limage->id) {
      mem_used -= limage->w*limage->h;
      fl_delete_offscreen(Pixmap(limage->id));
      limage->id=0;
      if(limage->mask) {
	fl_delete_bitmap(Pixmap(limage->mask));
	limage->mask = 0;
      }
    } else return;
  } while(mem_used >= mem_usage_limit);
}


class fl_shared_image_destructor_class {
  int dummy;
public:
  fl_shared_image_destructor_class() { dummy = 0; }
  ~fl_shared_image_destructor_class() {
    if (Fl_Shared_Image::first_image) Fl_Shared_Image::first_image->clear_cache();
  }
};

fl_shared_image_destructor_class fl_shared_image_destructor;

void Fl_Shared_Image::clear_cache()
{
  if(id) {
    mem_used -= w*h;
    fl_delete_offscreen((Pixmap)id);
    id=0;
    if(mask) {
      fl_delete_bitmap((Pixmap)mask);
      mask = 0;
    }
  }
  if (l1) l1->clear_cache();
  if (l2) l2->clear_cache();
}

void Fl_Shared_Image::set_root_directory(const char *d) {
  fl_shared_image_root = d;
}

void Fl_Shared_Image::insert(Fl_Shared_Image*& p, Fl_Shared_Image* image) {
  if(p == 0)
    p = image;
  else {
    int c = strcmp(image->name, p->name);
    if(c<0) insert(p->l1, image);
    else insert(p->l2, image);
  }
}

Fl_Shared_Image* Fl_Shared_Image::find(Fl_Shared_Image* image, const char* name) {
  if(image == 0) return 0;
  int c = strcmp(name, image->name);
  if(c == 0) return image;
  else if(c<0) return find(image->l1, name);
  else return find(image->l2, name);
}


const char* Fl_Shared_Image::get_filename() {
  return get_filename(name);
}

const char* Fl_Shared_Image::get_filename(const char* name)
{
  if (name[0] == '/' || !fl_shared_image_root || !*fl_shared_image_root)
    return name;
  int m = strlen(fl_shared_image_root);
  int n = strlen(name) + m + 2;
  static char *s;
  if (s) free(s);
  s = (char*) malloc(n+1);
  strcpy(s, fl_shared_image_root);
  if (s[m-1] != '/') s[m++] = '/';
  strcpy(s+m, name);
  return s;
}


Fl_Shared_Image* Fl_Shared_Image::get(Fl_Shared_Image* (*create)(),
				      const char* name, const uchar *datas)
{
  Fl_Shared_Image *image=Fl_Shared_Image::find(first_image, name);
  if(!image)
  {
    image=create();
    image->refcount = 1;
    image->name = strdup(name);
    image->datas=datas;
    image->w = -1; // We mark the fact the it has never been measured yet
    image->l1 = image->l2 = 0;
    image->id=image->mask=0;
    Fl_Shared_Image::insert(first_image, image);
  } else {
    if(image->datas==NULL) image->datas=datas;
    image->refcount++;
  }
  image->used = image_used++;
  return image;
}

void Fl_Shared_Image::reload(const uchar* pdatas)
{
  if (id) {
    mem_used -= w*h;
    fl_delete_offscreen((Pixmap)id);
    id=0;
    if (mask) {
      fl_delete_bitmap((Pixmap)mask);
      mask = 0;
    }
  }
  if (pdatas) datas = pdatas;
  measure(w, h);
}
void Fl_Shared_Image::reload(const char* name, const uchar* pdatas)
{
  Fl_Shared_Image *image=Fl_Shared_Image::find(first_image, name);
  if (image) image->reload(pdatas);
}

void Fl_Shared_Image::remove_from_tree(Fl_Shared_Image*& p, Fl_Shared_Image* image) {
  if (p) {
    int c = strcmp(image->name, p->name);
    if (c == 0) {
      if (image->l1) {
	p = image->l1;
	if (image->l2) insert(first_image, image->l2);
      } else
	p = image->l2;
    } else if (c<0) remove_from_tree(p->l1, image);
    else remove_from_tree(p->l2, image);
  }
}

int Fl_Shared_Image::remove()
{
  if (--refcount) return 0;
  remove_from_tree(first_image, this);
  forbid_delete = 0;
  delete this;
  forbid_delete = 1;
  return 1;
}
int Fl_Shared_Image::remove(const char* name)
{
  Fl_Shared_Image *image=Fl_Shared_Image::find(first_image, name);
  if (image) return image->remove();
  else return 0;
}

void Fl_Shared_Image::draw(int X, int Y, Fl_Flags flags)
{
  if (w<0) measure(w, h);
  if (w==0) return;
  if (!id) // Need to uncompress the image ?
  {
    used = image_used++; // do this before check_mem_usage
    mem_used += w*h;
    check_mem_usage();

    read();
    if (!id) { // could not read the image for some reason ?
      mem_used -= w*h;
      w = 0; // Will never try again ...
      return; 
    }
  }
  else
    used = image_used++;
  _draw(X, Y, flags);
}

//
// End of "$Id: Fl_Shared_Image.cxx,v 1.23 2001/09/10 01:16:17 spitzak Exp $"
//
