//
// A shared image test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

#include <config.h>
#include <FL/platform.H>
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_GIF_Image.H>
#include <FL/Fl_Printer.H>
#include <string.h>
#include <errno.h>
#include <locale.h>     // setlocale()..
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_message.H>
#include <FL/Fl_SVG_File_Surface.H>
#include <FL/Fl_Native_File_Chooser.H>

Fl_Box *b;
Fl_Double_Window *w;
Fl_Shared_Image *img;


static char name[1024];

void cb_forced_redraw(void *) {
  Fl_Window *win = Fl::first_window();
  while (win) {
    if (!win->menu_window())
      win->redraw();
    win = Fl::next_window(win);
  }
  if (Fl::first_window())
    Fl::repeat_timeout(1./10, cb_forced_redraw);
}

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
  b->image(img);
  img->scale(b->w(), b->h());
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
  const char *fname = fl_file_chooser("Image file?","*.{bm,bmp,gif,ico,jpg,pbm,pgm,png,ppm,xbm,xpm"
#ifdef FLTK_USE_SVG
                                      ",svg,svgz"
#endif // FLTK_USE_SVG
                                      "}", name);
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

void svg_cb(Fl_Widget *widget, void *) {
  Fl_Native_File_Chooser fnfc;
  fnfc.title("Pick a .svg file");
  fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
  fnfc.filter("SVG\t*.svg\n");
  fnfc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM | Fl_Native_File_Chooser::USE_FILTER_EXT);
  if (fnfc.show() ) return;
  FILE *svg = fl_fopen(fnfc.filename(), "w");
  Fl_SVG_File_Surface surf(widget->window()->decorated_w(), widget->window()->decorated_h(), svg);
  surf.draw_decorated_window(widget->window());
  surf.close();
}

int dvisual = 0;
int animate = 1;
int arg(int, char **argv, int &i) {
  if (argv[i][1] == '8') {dvisual = 1; i++; return 1;}
  if (argv[i][1] == 'a') {animate = 1; i++; return 1;}
  return 0;
}

int main(int argc, char **argv) {
  int i = 1;

  setlocale(LC_ALL, "");    // enable multilanguage errors in file chooser
  fl_register_images();

  Fl::args_to_utf8(argc, argv); // enable multilanguage commandlines on Windows
  Fl::args(argc, argv, i, arg); // parse commandline

  if (animate)
    Fl_GIF_Image::animate = true; // create animated shared .GIF images (e.g. file chooser)

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
  Fl_Button svg(190,425,100,25,"save as SVG");
  svg.callback(svg_cb);

  window.show(argc,argv);
  if (animate)
    Fl::add_timeout(1./10, cb_forced_redraw); // force periodic redraw
  return Fl::run();
}
