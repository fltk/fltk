//
// "$Id$"
//
// A shared image test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
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
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Shared_Image.H>
#include <string.h>
#include <errno.h>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_message.H>

Fl_Box *b;
Fl_Window *w;
Fl_Shared_Image *img;


static char name[1024];

void load_file(const char *n) {
  if (img) {
    img->release();
    img = 0L;
  }
  if (fl_filename_isdir(n)) {
    b->label("@fileopen"); // show a generic folder
    b->labelsize(64);
    b->labelcolor(FL_LIGHT2);
    b->image(0);
    b->redraw();
    return;
  }
  img = Fl_Shared_Image::get(n);
  if (!img) {
    b->label("@filenew"); // show an empty document
    b->labelsize(64);
    b->labelcolor(FL_LIGHT2);
    b->image(0);
    b->redraw();
    return;
  }
  if (img->w() > b->w() || img->h() > b->h()) {
    Fl_Image *temp;
    if (img->w() > img->h()) temp = img->copy(b->w(), b->h() * img->h() / img->w());
    else temp = img->copy(b->w() * img->w() / img->h(), b->h());

    img->release();
    img = (Fl_Shared_Image *)temp;
  }
  b->label(name);
  b->labelsize(14);
  b->labelcolor(FL_FOREGROUND_COLOR);
  b->image(img);
  b->redraw();
}

void file_cb(const char *n) {
  if (!strcmp(name,n)) return;
  load_file(n);
  strcpy(name,n);
  w->label(name);
}

void button_cb(Fl_Widget *,void *) {
  fl_file_chooser_callback(file_cb);
  fl_file_chooser("Image file?","*.{bm,bmp,gif,jpg,pbm,pgm,png,ppm,xbm,xpm}", name);
  fl_file_chooser_callback(0);
}

int dvisual = 0;
int arg(int, char **argv, int &i) {
  if (argv[i][1] == '8') {dvisual = 1; i++; return 1;}
  return 0;
}

int main(int argc, char **argv) {
  int i = 1;

  fl_register_images();

  Fl::args(argc,argv,i,arg);

  Fl_Window window(400,435); ::w = &window;
  Fl_Box b(10,45,380,380); ::b = &b;
  b.box(FL_THIN_DOWN_BOX);
  b.align(FL_ALIGN_INSIDE|FL_ALIGN_CENTER);
  Fl_Button button(150,5,100,30,"load");
  button.callback(button_cb);
  if (!dvisual) Fl::visual(FL_RGB);
  if (argv[1]) load_file(argv[1]);
  window.resizable(window);
  window.show(argc,argv);
  return Fl::run();
}

//
// End of "$Id$".
//
