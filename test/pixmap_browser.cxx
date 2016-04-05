//
// "$Id$"
//
// A shared image test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
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
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Printer.H>
#include <string.h>
#include <errno.h>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_message.H>

Fl_Box *b;
Fl_Double_Window *w;
Fl_Shared_Image *img;


static char name[1024];

void load_file(const char *n) {
  if (img) {
    ((Fl_Shared_Image*)b->image())->release();
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
  Fl_Shared_Image *img2 = Fl_Shared_Image::get(n);
 
  if (!img2) {
    b->label("@filenew"); // show an empty document
    b->labelsize(64);
    b->labelcolor(FL_LIGHT2);
    b->image(0);
    b->redraw();
    return;
  }
  img = img2;
  b->labelsize(14);
  b->labelcolor(FL_FOREGROUND_COLOR);
#if FLTK_ABI_VERSION >= 10304
  b->image(img);
  img->scale(b->w(), b->h());
#else
  if (img->w() <= b->w() && img->h() <= b->h()) b->image(img);
  else {
    float fw = img->w() / float(b->w());
    float fh = img->h() / float(b->h());
    float f = fw > fh ? fw : fh;
    b->image(img->copy(int(img->w()/f), int(img->h()/f)));
    img->release();
  }
#endif
  b->label(NULL);
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
  const char *fname = fl_file_chooser("Image file?","*.{bm,bmp,gif,jpg,pbm,pgm,png,ppm,xbm,xpm}", name);
  puts(fname ? fname : "(null)"); fflush(stdout);
  fl_file_chooser_callback(0);
}
void print_cb(Fl_Widget *widget, void *) {
  Fl_Printer printer;
  int width, height;
  if (printer.start_job(1)) return;
  printer.start_page();
  printer.printable_rect(&width, &height);
  float fw = widget->window()->decorated_w() / float(width);
  float fh = widget->window()->decorated_h() / float(height);
  if (fh > fw) fw = fh;
  printer.scale(1/fw);
  printer.print_window(widget->window());
  printer.end_page();
  printer.end_job();
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

  Fl_Double_Window window(400,450); ::w = &window;
  Fl_Box b(10,45,380,380); ::b = &b;
  b.box(FL_THIN_DOWN_BOX);
  b.align(FL_ALIGN_INSIDE|FL_ALIGN_CENTER|FL_ALIGN_CLIP);
  Fl_Button button(150,5,100,30,"load");
  button.callback(button_cb);
  if (!dvisual) Fl::visual(FL_RGB);
  if (argv[1]) load_file(argv[1]);
  window.resizable(b);
  Fl_Button print(300,425,50,25,"Print");
  print.callback(print_cb);

  window.show(argc,argv);
  return Fl::run();
}

//
// End of "$Id$".
//
