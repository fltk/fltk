//
// "$Id: Fluid_Image.cxx,v 1.3 1998/11/12 15:01:48 mike Exp $"
//
// Pixmap label support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include "Fl_Type.H"
#include "Fluid_Image.H"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h> // needed for DEC's Alpha cxx ?
#include <FL/filename.H>

extern void goto_source_dir(); // in fluid.C
extern void leave_source_dir(); // in fluid.C

////////////////////////////////////////////////////////////////
#include <FL/Fl_Pixmap.H>

class pixmap_image : public Fluid_Image {
protected:
  Fl_Pixmap *p;
  int *linelength;
public:
  pixmap_image(const char *name, FILE *);
  ~pixmap_image();
  virtual void label(Fl_Widget *); // set the label of this widget
  virtual void write_static();
  virtual void write_code();
  static int test_file(char *buffer);
};

int pixmap_image::test_file(char *buffer) {
  return (strstr(buffer,"/* XPM") != 0);
}

void pixmap_image::label(Fl_Widget *o) {
  if (p) p->label(o);
}

static int pixmap_header_written;

void pixmap_image::write_static() {
  if (!p) return;
  write_c("\n");
  if (pixmap_header_written != write_number) {
    write_c("#include <FL/Fl_Pixmap.H>\n");
    pixmap_header_written = write_number;
  }
  write_c("static unsigned char *%s[] = {\n",
	  unique_id(this, "image", filename_name(name()), 0));
  int l;
  for (l = 0; p->data[l]; l++) {
    if (l) write_c(",\n");
    write_c("(unsigned char *)");
    write_cstring(p->data[l],linelength[l]);
  }
  write_c("\n};\n");
  write_c("static Fl_Pixmap %s(%s);\n",
	  unique_id(this, "pixmap", filename_name(name()), 0),
	  unique_id(this, "image", filename_name(name()), 0));
}

void pixmap_image::write_code() {
  if (!p) return;
  write_c("%s%s.label(o);\n", indent(),
	  unique_id(this, "pixmap", filename_name(name()), 0));
}

static int hexdigit(int x) {
  if (isdigit(x)) return x-'0';
  if (isupper(x)) return x-'A'+10;
  if (islower(x)) return x-'a'+10;
  return 20;
}

#define MAXSIZE 1024

pixmap_image::pixmap_image(const char *name, FILE *f) : Fluid_Image(name) {
  if (!f) return; // for subclasses
  // read all the c-strings out of the file:
  char *data[MAXSIZE+1];
  int length[MAXSIZE+1];
  char buffer[MAXSIZE+20];
  int i = 0;
  while (i < MAXSIZE && fgets(buffer,MAXSIZE+20,f)) {
    if (buffer[0] != '\"') continue;
    char *p = buffer;
    char *q = buffer+1;
    while (*q != '\"') {
      if (*q == '\\') switch (*++q) {
      case '\n':
	fgets(q,(buffer+MAXSIZE+20)-q,f); break;
      case 0:
	break;
      case 'x': {
	q++;
	int n = 0;
	for (int x = 0; x < 3; x++) {
	  int d = hexdigit(*q);
	  if (d > 15) break;
	  n = (n<<4)+d;
	  q++;
	}
	*p++ = n;
      } break;
      default: {
	int c = *q++;
	if (c>='0' && c<='7') {
	  c -= '0';
	  for (int x=0; x<2; x++) {
	    int d = hexdigit(*q);
	    if (d>7) break;
	    c = (c<<3)+d;
	    q++;
	  }
	}
	*p++ = c;
      } break;
      } else {
	*p++ = *q++;
      }
    }
    *p++ = 0;
    data[i] = new char[p-buffer];
    memcpy(data[i],buffer,p-buffer);
    length[i] = p-buffer-1;
    i++;
  }
  data[i++] = 0; // put a null at the end

  char** real_data = new char*[i];
  linelength = new int[i];
  while (i--) {real_data[i] = data[i]; linelength[i] = length[i];}
  p = new Fl_Pixmap(real_data);
}

pixmap_image::~pixmap_image() {
  if (p && p->data) {
    char** real_data = (char**)(p->data);
    for (int i = 0; real_data[i]; i++) delete[] real_data[i];
    delete[] real_data;
  }
  delete[] linelength;
  delete p;
}

////////////////////////////////////////////////////////////////

class gif_image : public pixmap_image {
public:
  gif_image(const char *name, FILE *);
  ~gif_image();
  static int test_file(char *buffer);
};

int gif_image::test_file(char *buffer) {
  return !strncmp(buffer,"GIF",3);
}

// function in gif.C:
int gif2xpm(
    const char *infname,// filename for error messages
    FILE *GifFile,	// file to read
    char*** datap,	// return xpm data here
    int** lengthp,	// return line lengths here
    int inumber		// which image in movie (0 = first)
);

gif_image::gif_image(const char *name, FILE *f) : pixmap_image(name,0) {
  char** datap;
  if (gif2xpm(name,f,&datap,&linelength,0)) {
    p = new Fl_Pixmap(datap);
  } else
    p = 0;
}

gif_image::~gif_image() {
  if (p && p->data) {
    char** real_data = (char**)(p->data);
    for (int i = 0; i < 3; i++) delete[] real_data[i];
    delete[] real_data;
    p->data = 0;
  }
}

////////////////////////////////////////////////////////////////
#include <FL/Fl_Bitmap.H>

class bitmap_image : public Fluid_Image {
  Fl_Bitmap *p;
public:
  ~bitmap_image();
  bitmap_image(const char *name, FILE *);
  virtual void label(Fl_Widget *); // set the label of this widget
  virtual void write_static();
  virtual void write_code();
  static int test_file(char *buffer);
};

// bad test, always do this last!
int bitmap_image::test_file(char *buffer) {
  return (strstr(buffer,"#define ") != 0);
}

void bitmap_image::label(Fl_Widget *o) {
  if (p) p->label(o); else o->labeltype(FL_NORMAL_LABEL);
}

static int bitmap_header_written;

void bitmap_image::write_static() {
  if (!p) return;
  write_c("\n");
  if (bitmap_header_written != write_number) {
    write_c("#include <FL/Fl_Bitmap.H>\n");
    bitmap_header_written = write_number;
  }
  write_c("static unsigned char %s[] = {  \n",
	  unique_id(this, "bits", filename_name(name()), 0));
  int n = ((p->w+7)/8)*p->h;
  for (int i = 0; i < n; i++) {
    if (i) write_c(", ");
    write_c("%d",p->array[i]);
  }
  write_c("\n};\n");
  write_c("static Fl_Bitmap %s(%s, %d, %d);\n",
	  unique_id(this, "bitmap", filename_name(name()), 0),
	  unique_id(this, "bits", filename_name(name()), 0),
	  p->w, p->h);
}

void bitmap_image::write_code() {
  if (!p) return;
  write_c("%s%s.label(o);\n", indent(),
	  unique_id(this, "bitmap", filename_name(name()), 0));
}

bitmap_image::bitmap_image(const char *name, FILE *f) : Fluid_Image(name) {
  p = 0; // if any problems with parse we exit with this zero
  char buffer[1024];
  char junk[1024];
  int wh[2]; // width and height
  int i;
  for (i = 0; i<2; i++) {
    for (;;) {
      if (!fgets(buffer,1024,f)) return;
      int r = sscanf(buffer,"#define %s %d",junk,&wh[i]);
      if (r >= 2) break;
    }
  }
  // skip to data array:
  for (;;) {
    if (!fgets(buffer,1024,f)) return;
    if (!strncmp(buffer,"static ",7)) break;
  }
  int n = ((wh[0]+7)/8)*wh[1];
  uchar *data = new uchar[n];
  // read the data:
  i = 0;
  for (;i<n;) {
    if (!fgets(buffer,1024,f)) return;
    const char *a = buffer;
    while (*a && i<n) {
      int t;
      if (sscanf(a," 0x%x",&t)>0) data[i++] = t;
      while (*a && *a++ != ',');
    }
  }
  p = new Fl_Bitmap(data,wh[0],wh[1]);
}

bitmap_image::~bitmap_image() {
  if (p) {
    delete[] (uchar*)(p->array);
    delete p;
  }
}

////////////////////////////////////////////////////////////////

static Fluid_Image** images; // sorted list
static int numimages;
static int tablesize;

Fluid_Image* Fluid_Image::find(const char *name) {
  if (!name || !*name) return 0;

  // first search to see if it exists already:
  int a = 0;
  int b = numimages;
  while (a < b) {
    int c = (a+b)/2;
    int i = strcmp(name,images[c]->name_);
    if (i < 0) b = c;
    else if (i > 0) a = c+1;
    else return images[c];
  }

  // no, so now see if the file exists:

  goto_source_dir();
  FILE *f = fopen(name,"rb");
  if (!f) {
    read_error("%s : %s",name,strerror(errno));
    leave_source_dir();
    return 0;
  }

  Fluid_Image *ret;

  // now see if we can identify the type, by reading in some data
  // and asking all the types we know about:

  char buffer[1025];
  fread(buffer, 1, 1024, f);
  rewind(f);
  buffer[1024] = 0; // null-terminate so strstr() works

  if (pixmap_image::test_file(buffer)) {
    ret = new pixmap_image(name,f);
  } else if (gif_image::test_file(buffer)) {
    ret = new gif_image(name,f);
  } else if (bitmap_image::test_file(buffer)) {
    ret = new bitmap_image(name,f);
  } else {
    ret = 0;
    read_error("%s : unrecognized image format", name);
  }
  fclose(f);
  leave_source_dir();
  if (!ret) return 0;

  // make a new entry in the table:
  numimages++;
  if (numimages > tablesize) {
    tablesize = tablesize ? 2*tablesize : 16;
    images = (Fluid_Image**)realloc(images, tablesize*sizeof(Fluid_Image*));
  }
  for (b = numimages-1; b > a; b--) images[b] = images[b-1];
  images[a] = ret;

  return ret;
}

Fluid_Image::Fluid_Image(const char *name) {
  name_ = strdup(name);
  written = 0;
  refcount = 0;
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
  for (a = 0;; a++) if (images[a] == this) break;
  numimages--;
  for (; a < numimages; a++) images[a] = images[a+1];
  free((void*)name_);
}

////////////////////////////////////////////////////////////////

#include <FL/fl_file_chooser.H>

const char *ui_find_image_name;
Fluid_Image *ui_find_image(const char *oldname) {
  goto_source_dir();
  const char *name = fl_file_chooser("Image","*.{bm|xbm|xpm|gif}",oldname);
  ui_find_image_name = name;
  Fluid_Image *ret = (name && *name) ? Fluid_Image::find(name) : 0;
  leave_source_dir();
  return ret;
}

//
// End of "$Id: Fluid_Image.cxx,v 1.3 1998/11/12 15:01:48 mike Exp $".
//
