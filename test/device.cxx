//
// Device test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Roman Kantor and others.
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

#include <math.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Clock.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Bitmap.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Printer.H>
#include <FL/Fl_PostScript.H>
#include <FL/Fl_Copy_Surface.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_SVG_File_Surface.H>
#include <FL/Fl_PDF_File_Surface.H>

#include "pixmaps/porsche.xpm"
#include "pixmaps/sorceress.xbm"

class MyWidget: public Fl_Box{
protected:
  void draw() FL_OVERRIDE {
    Fl_Box::draw();
    fl_color(FL_RED);
    fl_rectf(x()+5,y()+5,w()-10,h()-10);
    fl_push_clip(x()+6,y()+6,w()-12,h()-12);
    fl_color(FL_DARK_GREEN);
    fl_rectf(x()+5,y()+5,w()-10,h()-10);
    fl_pop_clip();
    fl_color(FL_YELLOW);
    fl_rectf(x()+7,y()+7,w()-14,h()-14);
    fl_color(FL_BLUE);

    fl_rect(x()+8,y()+8,w()-16,h()-16);
    fl_push_clip(x()+25,y()+25,w()-50, h()-50);
    fl_color(FL_BLACK);
    fl_rect(x()+24,y()+24,w()-48,h()-48);
    fl_line(x()+27,y()+27,x()+w()-27,y()+h()-27);
    fl_line(x()+27,y()+h()-27,x()+w()-27,y()+27);
    //fl_rect(x()+30,y()+30,w()-60,h()-60);
    fl_pop_clip();
  }
public:
  MyWidget(int x, int y):Fl_Box(x,y,100,100, "Clipping and rect(f):\nYellow rect.framed\nby B-Y-G-R rect. 1 p.\nthick. Your printer may \nrender very thin lines\nsurrounding \"X\""){
    align(FL_ALIGN_TOP);
    labelsize(10);
  }
};

class MyWidget2: public Fl_Box {
protected:
  void draw() FL_OVERRIDE {
    Fl_Box::draw();
    int d;
    //    fl_line_style(0);
    for(d=y()+5;d<48+y();d+=2){
      fl_xyline(x()+5,d,x()+48);
    }

    fl_push_clip(x()+52,y()+5,45,43);
    for(d=y()+5;d<150+y();d+=3){
      fl_line(x()+52,d,x()+92,d-40);
    }
    fl_pop_clip();

    fl_line_style(FL_DASH);
    fl_xyline(x()+5,y()+55,x()+48);
    fl_line_style(FL_DOT);
    fl_xyline(x()+5,y()+58,x()+48);
    fl_line_style(FL_DASHDOT);
    fl_xyline(x()+5,y()+61,x()+48);
    fl_line_style(FL_DASHDOTDOT);
    fl_xyline(x()+5,y()+64,x()+48);
    fl_line_style(0,0,(char*)"\7\3\7\2");
    fl_xyline(x()+5,y()+67,x()+48);

    fl_line_style(0);

    fl_line(x()+5,y()+72,x()+25,y()+95);
    fl_line(x()+8,y()+72,x()+28,y()+95,x()+31,y()+72);

    fl_color(FL_YELLOW);
    fl_polygon(x()+11, y()+72,x()+27,y()+91,x()+29,y()+72);
    fl_color(FL_RED);
    fl_loop(x()+11, y()+72,x()+27,y()+91,x()+29,y()+72);

    fl_color(FL_BLUE); ////
    fl_line_style(FL_SOLID, 6);
    fl_loop(x()+31, y()+12,x()+47,y()+31,x()+49,y()+12);
    fl_line_style(0);

    fl_color(200,0,200);
    fl_polygon(x()+35,y()+72,x()+33,y()+95,x()+48,y()+95,x()+43,y()+72);
    fl_color(FL_GREEN);
    fl_loop(x()+35,y()+72,x()+33,y()+95,x()+48,y()+95,x()+43,y()+72);

    fl_color(FL_BLUE);
    fl_yxline(x()+65,y()+63,y()+66);
    fl_color(FL_GREEN);
    fl_yxline(x()+66,y()+66,y()+63);

    fl_color(FL_BLUE);
    fl_rect(x()+80,y()+55,5,5);
    fl_color(FL_YELLOW);
    fl_rectf(x()+81,y()+56,3,3);
    fl_color(FL_BLACK);
    fl_point(x()+82,y()+57);

    fl_color(FL_BLUE);
    fl_rect(x()+56, y()+79, 24, 17);
    fl_color(FL_CYAN);
    fl_rectf(x()+57, y()+80, 22 , 15 );
    fl_color(FL_RED);
    fl_arc(x()+57, y()+80, 22 ,15 ,40, 270);
    fl_color(FL_YELLOW);
    fl_pie(x()+58, y()+81, 20 ,13 ,40, 270);

    fl_line_style(0);

    fl_color(FL_BLACK);
    fl_point(x()+58,y()+58);
    fl_color(FL_RED);
    fl_yxline(x()+59,y()+58,y()+59);
    fl_color(FL_GREEN);
    fl_yxline(x()+60,y()+59,y()+58);
    fl_color(FL_BLACK);
    fl_xyline(x()+61,y()+58,x()+62);
    fl_color(FL_RED);
    fl_xyline(x()+62,y()+59,x()+61);

    fl_color(FL_GREEN);
    fl_yxline(x()+57,y()+58,y()+59,x()+58);
    fl_color(FL_BLUE);
    fl_xyline(x()+58,y()+60,x()+56,y()+58);
    fl_color(FL_RED);
    fl_xyline(x()+58,y()+61,x()+56,y()+63);
    fl_color(FL_GREEN);
    fl_yxline(x()+57,y()+63,y()+62,x()+58);

    fl_color(FL_BLUE);
    fl_line(x()+58,y()+63, x()+60, y()+65);
    fl_color(FL_BLACK);
    fl_line(x()+61,y()+65, x()+59, y()+63);

    fl_color(FL_BLACK);
  }

public:
  MyWidget2(int x, int y):Fl_Box(x,y,100,100, "Integer primitives") {
    labelsize(10);
    align(FL_ALIGN_TOP);
  }
};

class MyWidget3: public Fl_Box {
protected:
  void draw() FL_OVERRIDE {
    Fl_Box::draw();
    double d;
    //    fl_line_style(0);
    fl_push_clip(x()+5,y()+5,45,43);
    for(d=y()+5;d<95+y();d+=1.63){
      fl_begin_line();
      fl_vertex(x()+5,d);
      fl_vertex(x()+48,d);
      fl_end_line();
    }
    fl_pop_clip();

    fl_push_clip(x()+52,y()+5,45,43);
    for(d=y()+5;d<150+y();d+=2.3052){
      fl_begin_line();
      fl_vertex(x()+52,d);
      fl_vertex(x()+92,d-43);
      fl_end_line();
    }
    fl_pop_clip();

  }
public:
  MyWidget3(int x, int y):Fl_Box(x,y,100,100, "Sub-pixel drawing of\nlines 1.63 points apart\nOn the screen you\ncan see aliasing, the\nprinter should render\nthem properly") {
    labelsize(10);
    align(FL_ALIGN_TOP);
  }
};

class MyWidget4: public Fl_Box{
protected:
  void draw() FL_OVERRIDE {
    Fl_Box::draw();
    fl_push_matrix();
    fl_translate(x(),y());
    fl_scale(.75,.75);

    fl_line_style(FL_SOLID , 5);
    fl_begin_line();
    fl_vertex(10, 160);
    fl_vertex(40, 160);
    fl_vertex(40, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_RED);
    fl_line_style(FL_SOLID | FL_CAP_FLAT |FL_JOIN_MITER , 5);
    fl_begin_line();
    fl_vertex(10, 150);
    fl_vertex(50, 150);
    fl_vertex(50, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_GREEN);
    fl_line_style(FL_SOLID | FL_CAP_ROUND |FL_JOIN_ROUND , 5);
    fl_begin_line();
    fl_vertex(10, 140);
    fl_vertex(60, 140);
    fl_vertex(60, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_BLUE);
    fl_line_style(FL_SOLID | FL_CAP_SQUARE |FL_JOIN_BEVEL , 5);
    fl_begin_line();
    fl_vertex(10, 130);
    fl_vertex(70, 130);
    fl_vertex(70, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_BLACK);
    fl_line_style(FL_DASH , 5);
    fl_begin_line();
    fl_vertex(10, 120);
    fl_vertex(80, 120);
    fl_vertex(80, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_RED);
    fl_line_style(FL_DASH |FL_CAP_FLAT , 5);
    fl_begin_line();
    fl_vertex(10, 110);
    fl_vertex(90, 110);
    fl_vertex(90, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_GREEN);
    fl_line_style(FL_DASH |FL_CAP_ROUND , 5);
    fl_begin_line();
    fl_vertex(10, 100);
    fl_vertex(100, 100);
    fl_vertex(100, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_BLUE);
    fl_line_style(FL_DASH |FL_CAP_SQUARE , 5);
    fl_begin_line();
    fl_vertex(10, 90);
    fl_vertex(110, 90);
    fl_vertex(110, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_BLACK);
    fl_line_style(FL_DOT, 5);
    fl_begin_line();
    fl_vertex(10, 80);
    fl_vertex(120, 80);
    fl_vertex(120, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_RED);
    fl_line_style(FL_DOT | FL_CAP_FLAT, 5);
    fl_begin_line();
    fl_vertex(10, 70);
    fl_vertex(130, 70);
    fl_vertex(130, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_GREEN);
    fl_line_style(FL_DOT | FL_CAP_ROUND, 5);
    fl_begin_line();
    fl_vertex(10, 60);
    fl_vertex(140, 60);
    fl_vertex(140, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_BLUE);
    fl_line_style(FL_DOT | FL_CAP_SQUARE, 5);
    fl_begin_line();
    fl_vertex(10, 50);
    fl_vertex(150, 50);
    fl_vertex(150, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_BLACK);
    fl_line_style(FL_DASHDOT |FL_CAP_ROUND |FL_JOIN_ROUND , 5);
    fl_begin_line();
    fl_vertex(10, 40);
    fl_vertex(160, 40);
    fl_vertex(160, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_RED);
    fl_line_style(FL_DASHDOTDOT |FL_CAP_SQUARE |FL_JOIN_BEVEL , 5);
    fl_begin_line();
    fl_vertex(10, 30);
    fl_vertex(170, 30);
    fl_vertex(170, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_GREEN);
    fl_line_style(FL_DASHDOTDOT |FL_CAP_ROUND |FL_JOIN_ROUND , 5);
    fl_begin_line();
    fl_vertex(10, 20);
    fl_vertex(180, 20);
    fl_vertex(180, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_BLUE);
    fl_line_style(0, 5, (char*)"\12\3\4\2\2\1");
    fl_begin_line();
    fl_vertex(10, 10);
    fl_vertex(190, 10);
    fl_vertex(190, 190);

    fl_end_line();
    fl_line_style(0);
    fl_pop_matrix();

    fl_color(FL_BLACK);
  }
public:
  MyWidget4(int x, int y):Fl_Box(x,y+10,150,150, "Line styles"){
    labelsize(10);
    align(FL_ALIGN_TOP);
  }
};

class MyWidget5: public Fl_Box {
protected:
  void draw() FL_OVERRIDE {
    Fl_Box::draw();
    fl_push_matrix();

    fl_translate(x(),y());
    fl_push_matrix();
    fl_mult_matrix(1,3,0,1,0,-20);
    fl_color(FL_GREEN);
    fl_begin_polygon();
    fl_vertex(10,10);
    fl_vertex(100,-80);
    fl_vertex(100,-190);
    fl_end_polygon();

    fl_color(FL_RED);
    fl_line_style(FL_DASHDOT, 7);
    fl_begin_loop();

    fl_vertex(10,10);
    fl_vertex(100,-80);
    fl_vertex(100,-190);
    fl_end_loop();
    fl_line_style(0);

    fl_color(FL_BLUE);
    fl_line_style(FL_SOLID, 3);
    fl_begin_loop();
    fl_circle(60,-50,30);
    fl_end_loop();
    fl_line_style(0);

    fl_pop_matrix();
    fl_scale(1.8,1);

    fl_color(FL_YELLOW);
    fl_begin_polygon();
    fl_arc(30,90,20,-45,200);
    fl_end_polygon();

    fl_color(FL_BLACK);
    fl_line_style(FL_DASH, 3);
    fl_begin_line();
    fl_arc(30,90,20,-45,200);
    fl_end_line();
    fl_line_style(0);

    fl_translate(15,0);
    fl_scale(1.5,3);
    fl_begin_complex_polygon();
    fl_vertex(30,70);
    fl_arc(45,55,10,200,90);
    fl_arc(55,45,8,-170,20);
    fl_vertex(60,40);
    fl_vertex(30,20);
    fl_vertex(40,5);
    fl_vertex(60,25);
    //fl_vertex(50,50);
    fl_curve(35,30,30,53,0,35,65,65);
    fl_gap();
    fl_vertex(50,25);
    fl_vertex(40,10);
    fl_vertex(35,20);
    fl_end_complex_polygon();

    fl_pop_matrix();
  }
public:
  MyWidget5(int x, int y):Fl_Box(x,y,230,250, "Complex (double) drawings:\nBlue ellipse may not be\ncorrectly transformed\ndue to non-orthogonal\ntransformation"){
    labelsize(10);
    align(FL_ALIGN_TOP);
  }
};

uchar *image;
int width = 80;
int height = 80;

void make_image() {
  image = new uchar[4*width*height];
  uchar *p = image;
  for (int y = 0; y < height; y++) {
    double Y = double(y)/(height-1);
    for (int x = 0; x < width; x++) {
      double X = double(x)/(width-1);
      *p++ = uchar(255*((1-X)*(1-Y))); // red in upper-left
      *p++ = uchar(255*((1-X)*Y));     // green in lower-left
      *p++ = uchar(255*(X*Y));         // blue in lower-right
      X -= 0.5;
      Y -= 0.5;
      int alpha = (int)(350 * sqrt(X * X + Y * Y));
      if (alpha < 255) *p++ = uchar(alpha);     // alpha transparency
      else *p++ = 255;
      Y += 0.5;
    }
  }
}

void close_tmp_win(Fl_Widget *win, void *data) {
  ((Fl_Shared_Image*)data)->release();
  Fl::delete_widget(win);
}

Fl_Widget *target;
const  char *operation;

void copy(Fl_Widget *, void *data) {
  if (strcmp(operation, "Fl_Image_Surface") == 0) {
    Fl_Image_Surface *rgb_surf;
    int W, H, decorated;
    if (target->as_window() && !target->parent()) {
      W = target->as_window()->decorated_w();
      H = target->as_window()->decorated_h();
      decorated = 1;
    }
    else {
      W = target->w();
      H = target->h();
      decorated = 0;
    }
    rgb_surf = new Fl_Image_Surface(W, H, 1);
    Fl_Surface_Device::push_current(rgb_surf);
    fl_color(FL_YELLOW);fl_rectf(0,0,W,H);
    if (decorated)
      rgb_surf->draw_decorated_window(target->as_window());
    else
      rgb_surf->draw(target);
    Fl_Image *img = rgb_surf->image();
    delete rgb_surf;
    Fl_Surface_Device::pop_current();
    if (img) {
      Fl_Window* g2 = new Fl_Window(img->w()+10, img->h()+10, "Fl_Image_Surface");
      g2->color(FL_YELLOW);
      Fl_Box *b = new Fl_Box(FL_NO_BOX,5,5,img->w(), img->h(),0);
      b->image(img);
      g2->end();
      g2->callback(close_tmp_win, img);
      g2->show();
    }
    return;
  }

  if (strcmp(operation, "Fl_Copy_Surface") == 0) {
    Fl_Copy_Surface *copy_surf;
    if (target->as_window() && !target->parent()) {
      copy_surf = new Fl_Copy_Surface(target->as_window()->decorated_w(), target->as_window()->decorated_h());
      Fl_Surface_Device::push_current(copy_surf);
      copy_surf->draw_decorated_window(target->as_window(), 0, 0);
    }
    else {
      copy_surf = new Fl_Copy_Surface(target->w()+10, target->h()+20);
      Fl_Surface_Device::push_current(copy_surf);
      fl_color(FL_YELLOW);fl_rectf(0,0,copy_surf->w(), copy_surf->h());
      copy_surf->draw(target, 5, 10);
    }
    delete copy_surf;
    Fl_Surface_Device::pop_current();
  }

  if (strcmp(operation, "Fl_Printer") == 0 || strcmp(operation, "Fl_PostScript_File_Device") == 0
      || strcmp(operation, "Fl_PDF_File_Surface") == 0) {
    Fl_Paged_Device *p;
    int err;
    char *err_message = NULL;
    if (strcmp(operation, "Fl_Printer") == 0) {
      p = new Fl_Printer();
      err = p->begin_job(1, NULL, NULL, &err_message);
    } else if (strcmp(operation, "Fl_PDF_File_Surface") == 0) {
      p = new Fl_PDF_File_Surface();
      err = ((Fl_PDF_File_Surface*)p)->begin_job("FLTK.pdf", &err_message);
    } else {
      p = new Fl_PostScript_File_Device();
      err = ((Fl_PostScript_File_Device*)p)->start_job(1);
    }
    if (!err) {
      p->begin_page();
      Fl_Window *win = target->as_window();
      int target_w = win ? win->decorated_w() : target->w();
      int target_h = win ? win->decorated_h() : target->h();
      int w, h;
      p->printable_rect(&w, &h);
      float s = 1, s_aux = 1;
      if (target_w > w)
        s_aux = float(w) / target_w;
      if (target_h > h)
        s = float(h) / target_h;
      if (s_aux < s) s = s_aux;
      p->scale(s);
      p->printable_rect(&w, &h);
      p->origin(w/2, h/2);
      if (win) p->draw_decorated_window(win, - target_w/2, - target_h/2);
      else p->draw(target, - target_w/2, - target_h/2);
      p->end_page();
      p->end_job();
    } else if (err > 1 && err_message) {fl_alert("%s", err_message); delete[] err_message;}
    delete p;
  }

  if (strcmp(operation, "Fl_SVG_File_Surface") == 0) {
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Save a .svg file");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fnfc.filter("SVG\t*.svg\n");
    fnfc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM | Fl_Native_File_Chooser::USE_FILTER_EXT);
    if (!fnfc.show() ) {
      FILE *svg = fl_fopen(fnfc.filename(), "w");
      if (svg) {
        int ww, wh;
        if (target->as_window())  {
          ww = target->as_window()->decorated_w();
          wh = target->as_window()->decorated_h();
        } else {
          ww = target->w();
          wh = target->h();
        }
        Fl_SVG_File_Surface surface(ww, wh, svg);
        if (surface.file()) {
          if (target->as_window()) surface.draw_decorated_window(target->as_window());
          else surface.draw(target);
          if (surface.close()) fl_message("Error while writing to SVG file %s", fnfc.filename());
        }
      }
    }
  }

  if (strcmp(operation, "fl_capture_window()") == 0) {
    Fl_Window *win = target->as_window() ? target->as_window() : target->window();
    int X = target->as_window() ? 0 : target->x();
    int Y = target->as_window() ? 0 : target->y();
    Fl_RGB_Image *img = fl_capture_window(win, X, Y, target->w(), target->h());
    if (img) {
      Fl_Window* g2 = new Fl_Window(img->w()+10, img->h()+10, "fl_capture_window()");
      g2->color(FL_YELLOW);
      Fl_Box *b = new Fl_Box(FL_NO_BOX,5,5,img->w(), img->h(),0);
      b->image(img);
      g2->end();
      g2->callback(close_tmp_win, img);
      g2->show();
    }
  }

  if (strcmp(operation, "Fl_Image_Surface::mask()") == 0) {
    Fl_Image_Surface *surf = new Fl_Image_Surface(target->w(), target->h(), 1);
    Fl_Surface_Device::push_current(surf);
    fl_color(FL_BLACK);
    fl_rectf(0, 0, target->w(), target->h());
    fl_color(FL_WHITE);
    fl_pie(0, 0, target->w(), target->h(), 0, 360);
    if (target->top_window() == target) {
      fl_color(FL_BLACK);
      int mini = int((target->w() < target->h() ? target->w() : target->h()) * 0.66);
      fl_pie(target->w()/2 - mini/2, target->h()/2 - mini/2, mini, mini, 0, 360);
      fl_color(FL_WHITE);
      fl_font(FL_TIMES_BOLD, 120);
      int dx, dy, l, h;
      fl_text_extents("FLTK", dx, dy, l, h);
      fl_draw("FLTK", target->w()/2 - l/2, target->h()/2 + h/2);
    }
    Fl_RGB_Image *mask = surf->image();
    fl_color(FL_YELLOW);
    fl_rectf(0, 0, target->w(), target->h());
    Fl_Surface_Device::pop_current();
    surf->mask(mask);
    delete mask;
    Fl_Surface_Device::push_current(surf);
    surf->draw(target, 0, 0);
    mask = surf->image();
    Fl_Surface_Device::pop_current();
    delete surf;
    Fl_Window *win = new Fl_Window(mask->w(), mask->h(), operation);
    Fl_Box *box = new Fl_Box(0, 0, mask->w(), mask->h());
    box->bind_image(mask);
    win->end();
    win->show();
  }

}

class My_Button:public Fl_Button {
protected:
  void draw() FL_OVERRIDE {
    if (type() == FL_HIDDEN_BUTTON) return;
    Fl_Color col = value() ? selection_color() : color();
    draw_box(value() ? (down_box()?down_box():fl_down(box())) : box(), col);
    fl_color(FL_WHITE);
    fl_line_style(FL_SOLID,5);
    fl_line(x()+15,y()+10,x()+w()-15,y()+h()-23);
    fl_line(x()+w()-15,y()+10,x()+15,y()+h()-23);
    fl_line_style(0);
    draw_label();

  }
public:
  My_Button(int x, int y, int w, int h, const char * label = 0):Fl_Button(x,y,w,h,label){}
};

void target_cb(Fl_Widget* wid, void *data)
{
  target = (Fl_Widget*)data;
}

void operation_cb(Fl_Widget* wid, void *data)
{
  operation = wid->label();
}

int main(int argc, char ** argv) {

  //Fl::scheme("plastic");

  Fl_Window * w2 = new Fl_Window(500,568,"Graphics test");

  Fl_Group *c2 =new Fl_Group(3, 56, 494, 514 );

  new MyWidget(10,140+16);
  new MyWidget2(110,80+16);
  new MyWidget3(220,140+16);
  new MyWidget4(330,70+16);
  new MyWidget5(140,270+16);

  make_image();
  Fl_RGB_Image *rgb = new Fl_RGB_Image(image, width, height, 4);
  My_Button b_rgb(10,245+16,100,100,"RGB with alpha");
  b_rgb.image(rgb);

  My_Button b_pixmap(10,345+16,100,100,"Pixmap");
  Fl_Pixmap *pixmap = new Fl_Pixmap(porsche_xpm);
  b_pixmap.image(pixmap);

  My_Button b_bitmap(10,445+16,100,100,"Bitmap");
  b_bitmap.labelcolor(FL_GREEN);
  b_bitmap.image(new Fl_Bitmap(sorceress_bits,sorceress_width,sorceress_height));

  new Fl_Clock(360,230+16,120,120);
  Fl_Return_Button * ret = new Fl_Return_Button (360, 360+16, 120,30, "Return");
  ret->deactivate();
  Fl_Button but1(360, 390+16, 30, 30, "@->|");
  but1.labelcolor(FL_DARK3);
  Fl_Button but2(390, 390+16, 30, 30, "@UpArrow");
  but2.labelcolor(FL_DARK3);
  Fl_Button but3(420, 390+16, 30, 30, "@DnArrow");
  but3.labelcolor(FL_DARK3);
  Fl_Button but4(450, 390+16, 30, 30, "@+");
  but4.labelcolor(FL_DARK3);
  Fl_Button but5(360, 425+16, 120, 30, "Hello, World");
  but5.labelfont(FL_BOLD|FL_ITALIC);
  but5.labeltype(FL_SHADOW_LABEL);
  but5.box(FL_ROUND_UP_BOX);

  Fl_Button but6(360, 460+16, 120, 30, "Plastic");
  but6.box(FL_PLASTIC_UP_BOX);

  Fl_Group *group;
  { Fl_Group* o = new Fl_Group(360, 495+16, 120, 40); group=o;
    o->box(FL_UP_BOX);
    { Fl_Group* o = new Fl_Group(365, 500+16, 110, 30);
      o->box(FL_THIN_UP_FRAME);
      { Fl_Round_Button* o = new Fl_Round_Button(365, 500+16, 40, 30, "rad");
        o->value(1);
      }
      { Fl_Check_Button* o = new Fl_Check_Button(410, 500+16, 60, 30, "check");
        o->value(1);
      }
      o->end();
    }
    o->end();
    o->deactivate();
  }
  Fl_Box tx(120,492+16,230,50,"Background is not printed because\nencapsulating group, which we are\n printing, has not set the box type");
  tx.box(FL_SHADOW_BOX);
  tx.labelsize(12);

  tx.hide();

  c2->end();

  Fl_Radio_Round_Button *rb;
  Fl_Window *w3 = new Fl_Window(2,5,w2->w()-10,73);
  w3->box(FL_DOWN_BOX);
  Fl_Group *g1 = new Fl_Group(0,0,w3->w(),w3->h());
  rb = new Fl_Radio_Round_Button(5,4,150,12, "Fl_Image_Surface");
  rb->set(); rb->callback(operation_cb, NULL); operation = rb->label(); rb->labelsize(12);
  rb = new Fl_Radio_Round_Button(170,4,150,12, "Fl_Copy_Surface"); rb->callback(operation_cb, NULL); rb->labelsize(12);
  rb = new Fl_Radio_Round_Button(5,17,150,12, "Fl_Printer"); rb->callback(operation_cb, NULL); rb->labelsize(12);
  rb = new Fl_Radio_Round_Button(170,17,150,12, "Fl_PostScript_File_Device"); rb->callback(operation_cb, NULL); rb->labelsize(12);
  rb = new Fl_Radio_Round_Button(5,30,150,12, "Fl_PDF_File_Surface"); rb->callback(operation_cb, NULL); rb->labelsize(12);
  rb = new Fl_Radio_Round_Button(170,30,150,12, "Fl_SVG_File_Surface"); rb->callback(operation_cb, NULL); rb->labelsize(12);
  rb = new Fl_Radio_Round_Button(5,43,150,12, "fl_capture_window()"); rb->callback(operation_cb, NULL); rb->labelsize(12);
  rb = new Fl_Radio_Round_Button(170,43,150,12, "Fl_Image_Surface::mask()"); rb->callback(operation_cb, NULL); rb->labelsize(12);
  g1->end();

  Fl_Group *g2 = new Fl_Group(0,0,w3->w(),w3->h());
  Fl_Box *box = new Fl_Box(FL_BORDER_BOX, 4, 55, 340, 16, NULL);
  box->color(FL_LIGHT3);
  rb = new Fl_Radio_Round_Button(5,57,140,12, "Decorated window");
  rb->labelsize(12); rb->set(); rb->callback(target_cb, w2); target = w2;
  rb = new Fl_Radio_Round_Button(160,57,100,12, "Sub-window");
  rb->labelsize(12); rb->callback(target_cb, w3);
  rb = new Fl_Radio_Round_Button(275,57,60,12, "Group");
  rb->labelsize(12);rb->callback(target_cb, group);
  g2->end();
  Fl_Button *b4 = new Fl_Button(380, (w3->h() - 25)/2, 100, 25, "GO");
  b4->callback((Fl_Callback*)copy,NULL);
  w3->end();

  w2->end();
  Fl_RGB_Image *rgba_icon = new Fl_RGB_Image(pixmap);
  Fl_Window::default_icon(rgba_icon);
  // w2->icon(rgba_icon);
  delete rgba_icon;
  w2->show(argc, argv);

  Fl::run();
  delete pixmap;
  delete b_bitmap.image();
  delete rgb;

  return 0;
}
