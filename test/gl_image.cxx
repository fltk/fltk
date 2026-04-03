//
// OpenGL image-drawing test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 2026 by Bill Spitzak and others.
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

#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/gl.h>

class image_background_box : public Fl_Widget {
public:
  image_background_box(int x, int y, int w, int h) : Fl_Widget(x, y, w, h, NULL) {}
  void draw() override {
    gl_color(FL_WHITE);
    gl_rectf(x(), y(), w(), h());
    gl_color(color());
    int startX;
    for (int Y = y(); Y < y()+h(); Y += 5) {
      startX = Y % 10;
      for (int X = startX; X < w(); X += 10) {
        gl_rectf(X, Y, 5, 5);
      }
    }
    fl_color(FL_DARK_BLUE); // Effective only when the drawn image is an Fl_Bitmap
    Fl_Image *img = image();
    if (img) img->draw((w() - img->w())/2, y() + (h() - img->h())/2);
  }
};


void chooser_cb(Fl_Widget *, class Gl_Image_Window *mainwin);

void close_cb(Fl_Widget *, void *) {
  Fl::first_window()->hide();
}


Fl_Menu_Item items[] = {
  { "File", 0, 0, 0, FL_SUBMENU},
  { "Choose image file…", 0, (Fl_Callback*)chooser_cb},
  { "Quit", 0, close_cb},
  {0},
  {0},
};


class Gl_Image_Window : public Fl_Gl_Window {
  const Fl_Image *img_;
  image_background_box *box_;
public:
  Gl_Image_Window(int w, int h, const char *t) : Fl_Gl_Window(w, h, t) {
    img_ = NULL;
    begin();
    Fl_Menu_Bar *bar = new Fl_Menu_Bar(0, 0, w, 30);
    bar->menu(items);
    items[1].user_data(this);
    box_ = new image_background_box(0, 30, w, h - 30);
    box_->color(FL_GRAY);
    end();
    resizable(box_);
  }

  void set_image(const Fl_Image *img) {
    if (img_) ((Fl_Image*)img_)->release();
    img_ = img;
    box_->image((Fl_Image*)img);
    ((Fl_Image*)img)->scale(box_->w(), box_->h());
    redraw();
  }

  void resize(int x, int y, int w, int h) override {
    Fl_Gl_Window::resize(x,y,w,h);
    if (img_) ((Fl_Image*)img_)->scale(box_->w(), box_->h());
  }

  void draw() override {
    Fl_Gl_Window::draw(); // Draw FLTK child widgets.
  }
};


void chooser_cb(Fl_Widget *, Gl_Image_Window *mainwin) {
  char *fname = fl_file_chooser("select image file", "*.{png,jpg,gif,svg,svgz,xbm,xpm,ico}", NULL, 0);
  if (fname) {
    Fl_Shared_Image *shared = Fl_Shared_Image::get(fname);
    if (shared && !shared->fail()) {
      mainwin->copy_label(fname);
      mainwin->set_image(shared);
    }
  }
}


int main(int argc, char **argv) {
  Fl::use_high_res_GL(1);
  fl_register_images();
  Gl_Image_Window mainwin(600, 600, "GL Image Viewer");
  mainwin.show(argc, argv);
  Fl::run();
  delete mainwin.child(1)->image();
  return 0;
}
