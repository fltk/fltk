/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#if 1

//
// "$Id: color_chooser.cxx 12655 2018-02-09 14:39:42Z AlbrechtS $"
//
// Color chooser test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/fl_show_colormap.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Image.H>
#include <FL/platform.H>
#include <FL/fl_draw.H>

#include <stdlib.h>
#include <stdio.h>
#if !defined(_WIN32) && !defined(__APPLE__) && !defined(FL_PORTING) && !defined(__ANDROID__)
#include "list_visuals.cxx"
#endif

int width = 100;
int height = 100;
uchar *image;
Fl_Box *hint;

void make_image() {
  image = new uchar[3*width*height];
  uchar *p = image;
  for (int y = 0; y < height; y++) {
    double Y = double(y)/(height-1);
    for (int x = 0; x < width; x++) {
      double X = double(x)/(width-1);
      *p++ = uchar(255*((1-X)*(1-Y))); // red in upper-left
      *p++ = uchar(255*((1-X)*Y));	// green in lower-left
      *p++ = uchar(255*(X*Y));	// blue in lower-right
    }
  }
}

class Pens : public Fl_Box {
  void draw();
public:
  Pens(int X, int Y, int W, int H, const char* L)
  : Fl_Box(X,Y,W,H,L) {}
};
void Pens::draw() {
  // use every color in the gray ramp:
  for (int i = 0; i < 3*8; i++) {
    fl_color((Fl_Color)(FL_GRAY_RAMP+i));
    fl_line(x()+i, y(), x()+i, y()+h());
  }
}

Fl_Color c = FL_GRAY;
#define fullcolor_cell (FL_FREE_COLOR)

void cb1(Fl_Widget *, void *v) {
  c = fl_show_colormap(c);
  Fl_Box* b = (Fl_Box*)v;
  b->color(c);
  hint->labelcolor(fl_contrast(FL_BLACK,c));
  b->parent()->redraw();
}

void cb2(Fl_Widget *, void *v) {
  uchar r,g,b;
  Fl::get_color(c,r,g,b);
  if (!fl_color_chooser("New color:",r,g,b,3)) return;
  c = fullcolor_cell;
  Fl::set_color(fullcolor_cell,r,g,b);
  Fl_Box* bx = (Fl_Box*)v;
  bx->color(fullcolor_cell);
  hint->labelcolor(fl_contrast(FL_BLACK,fullcolor_cell));
  bx->parent()->redraw();
}

int main(int argc, char ** argv) {
  Fl::set_color(fullcolor_cell,145,159,170);
  Fl_Window window(400,400);
  Fl_Box box(30,30,340,340);
  box.box(FL_THIN_DOWN_BOX);
  c = fullcolor_cell;
  box.color(c);
  Fl_Box hintbox(40,40,320,30,"Pick background color with buttons:");
  hintbox.align(FL_ALIGN_INSIDE);
  hint = &hintbox;
  Fl_Button b1(120,80,180,30,"fl_show_colormap()");
  b1.callback(cb1,&box);
  Fl_Button b2(120,120,180,30,"fl_color_chooser()");
  b2.callback(cb2,&box);
  Fl_Box image_box(160,190,width,height,0);
  make_image();
  (new Fl_RGB_Image(image, width, height))->label(&image_box);
  Fl_Box b(160,310,120,30,"Example of fl_draw_image()");
  Pens p(60,180,3*8,120,"lines");
  p.align(FL_ALIGN_TOP);
  int i = 1;
  if (!Fl::args(argc,argv,i) || i < argc-1) {
    printf("usage: %s <switches> visual-number\n"
           " - : default visual\n"
           " r : call Fl::visual(FL_RGB)\n"
           " c : call Fl::own_colormap()\n",argv[0]);
#if !defined(_WIN32) && !defined(__APPLE__) && !defined(FL_PORTING) && !defined(__ANDROID__)
    printf(" # : use this visual with an empty colormap:\n");
    list_visuals();
#endif
    puts(Fl::help);
    exit(1);
  }
  if (i!=argc) {
    if (argv[i][0] == 'r') {
      if (!Fl::visual(FL_RGB)) printf("Fl::visual(FL_RGB) returned false.\n");
    } else if (argv[i][0] == 'c') {
      Fl::own_colormap();
    } else if (argv[i][0] != '-') {
#if !defined(_WIN32) && !defined(__APPLE__) && !defined(FL_PORTING) && !defined(__ANDROID__)
      int visid = atoi(argv[i]);
      fl_open_display();
      XVisualInfo templt; int num;
      templt.visualid = visid;
      fl_visual = XGetVisualInfo(fl_display, VisualIDMask, &templt, &num);
      if (!fl_visual) Fl::fatal("No visual with id %d",visid);
      fl_colormap = XCreateColormap(fl_display, RootWindow(fl_display,fl_screen),
                                    fl_visual->visual, AllocNone);
      fl_xpixel(FL_BLACK); // make sure black is allocated
#else
      Fl::fatal("Visual id's not supported on Windows or MacOS.");
#endif
    }
  }
  window.show(argc,argv);
  return Fl::run();
}

//
// End of "$Id: color_chooser.cxx 12655 2018-02-09 14:39:42Z AlbrechtS $".
//


#else

#include <src/drivers/Android/Fl_Android_Application.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>


Fl_Window *win, *win1, *win2;
Fl_Button *btn, *btn2;


class MyButton : public Fl_Button
{
public:
  MyButton(int x, int y, int w, int h, const char *l) : Fl_Button(x, y, w, h, l) { }
  void draw() {
    Fl_Button::draw();
    fl_push_clip(x(), y(), w(), h());
    fl_color(FL_BLUE);
    for (int i=0; i<h(); i+=5) {
      fl_line(x(), y()+h()/2, x()+w(), y()+i);
    }
    for (int i=0; i<w(); i+=5) {
      fl_line(x()+w()/2, y()-50, x()+i, y()+h()+50);
    }
    fl_pop_clip();
  }
};

void bye_cb(void*)
{
  btn2->color(FL_BLUE);
  btn2->redraw();
  Fl::remove_timeout(bye_cb, NULL);
}

void hello_cb(void*)
{
  btn2->color(FL_GREEN);
  btn2->redraw();
  Fl::add_timeout(1.0, bye_cb, NULL);
  Fl::remove_timeout(hello_cb, NULL);
}


int xmain(int argc, char **argv)
{
//  Fl::scheme("gleam");
  win1 = new Fl_Window(20+50, 10, 200, 200, "back");
  win1->color(FL_RED);
  win1->box(FL_DOWN_BOX);
  Fl_Button *b1 = new Fl_Button(10, 10, 180, 180, "back");
  b1->color(FL_DARK_RED);
  win1->end();
  win1->show();

  win = new Fl_Window(50, 150, 500, 400, "Hallo");

  btn2 = new Fl_Button(10, 10, 480, 100, "-@circle;-");
  btn2->color(FL_BLUE);

  btn = new MyButton((win->w()-280)/2, 200-45, 280, 80, "Hello, Android!\nWhere have you been so long?");
  btn->color(FL_LIGHT2);
  btn->labelsize(30);
  btn->align(FL_ALIGN_CLIP);
  btn->callback(
          [](Fl_Widget*, void*) {
            Fl::add_timeout(1.0, hello_cb, NULL);
          }
  );

  btn = new MyButton((win->w()-280)/2, 200+40, 280, 35, "Hello, \u00c4\u00e4\u00d6\u00f6\u00dc\u00fc\u00df \u2639 \u263a");
  btn->labelfont(FL_TIMES_BOLD_ITALIC);
  btn->labelsize(30);

  btn = new MyButton((win->w()-280)/2, 200+2*40, 280, 35, "Hello, Font!");
  btn->labelfont(FL_COURIER_BOLD_ITALIC);
  btn->labelsize(30);
  btn->callback(
          [](Fl_Widget *w, void*) {
//            FIXME: we can't seem to re-enter the event loop without locking
//                   the entire app!
//            fl_alert("This will change\nthe font for the entir\napplication");
            Fl::set_font(FL_COURIER_BOLD_ITALIC, "$DancingScript-Regular.ttf");
            w->redraw();
          }
  );

  btn = new MyButton((win->w()-280)/2, 200+3*40, 280, 35, "Hello, Android!");
  btn->box(FL_BORDER_BOX);
  btn->labelfont(FL_SCREEN);
  btn->labelsize(30);

  win->end();
  win->show(argc, argv);

  win2 = new Fl_Window(380-50, 10, 200, 200, "front");
  win2->color(FL_LIGHT3);
  win2->box(FL_UP_BOX);
  Fl_Button *b2 = new MyButton(10, 10, 180, 180, "front");
  b2->color(FL_DARK_BLUE);
  win2->end();
  win2->show();

  Fl::run();

  return 0;
}

#endif

/*

 Missing:
  - screen scale and size: most desktop apps expect to be in a draggable window
    on a larger desktop surface. For Android, there is usually no desktop, and
    screen resolution is often very high, so that a regular FLTK window would
    hide as a tiny gray spot in the top left corner
      * windows should probably be centered by default
      ? the screen resolution should adapt to the first opened window
      ? we should be able to hint at a prefered screen resolution
      * drawing call must scale at some point (line width!)
      * rotating the screen must call the app handler and(?) window resize
      * proportions: pixels should be square
 Need Work:
  - Fl_Android_Graphics_Driver::pie(int) needs refactoring
  - ...::line(...) has round ing issues (see rounded box type)
  - grab() not working when leaving window (adjuster...)


test/CubeMain.cxx			test/line_style.cxx
test/CubeView.cxx			test/list_visuals.cxx
  * test/adjuster.cxx     : + 'adjuster' works
                                        test/mandelbrot.cxx
test/animated.cxx			test/menubar.cxx
  * test/arc.cxx          : + 'arc' works as expected
                                        test/message.cxx
test/ask.cxx
  * test/minimum.cxx      : + 'minimum' works
test/bitmap.cxx				test/native-filechooser.cxx
test/blocks.cxx				test/navigation.cxx
  * test/boxtype.cxx      : + 'boxtype' works
                                        test/offscreen.cxx
test/browser.cxx			test/output.cxx
test/button.cxx				test/overlay.cxx
test/buttons.cxx			test/pack.cxx
test/cairo_test.cxx			test/pixmap.cxx
test/checkers.cxx			test/pixmap_browser.cxx
test/clock.cxx				test/resizebox.cxx
test/colbrowser.cxx			test/rotated_text.cxx
  * test/color_chooser.cxx:+ 'color_chooser' works
test/scroll.cxx
test/connect.cxx			test/shape.cxx
test/cube.cxx				test/subwindow.cxx
test/cursor.cxx				test/sudoku.cxx
test/curve.cxx
  * test/symbols.cxx      : + 'symbols' working as expected
test/demo.cxx				test/table.cxx
test/device.cxx				test/threads.cxx
test/doublebuffer.cxx			test/tile.cxx
test/editor.cxx				test/tiled_image.cxx
test/file_chooser.cxx			test/twowin.cxx
test/fonts.cxx				test/unittest_about.cxx
test/forms.cxx				test/unittest_circles.cxx
test/fractals.cxx			test/unittest_images.cxx
test/fracviewer.cxx			test/unittest_lines.cxx
test/fullscreen.cxx			test/unittest_points.cxx
test/gl_overlay.cxx			test/unittest_rects.cxx
test/glpuzzle.cxx			test/unittest_schemes.cxx
  * test/hello.cxx        : + 'hello' works fine, italics, shadow, etc.
                                        test/unittest_scrollbarsize.cxx
test/help_dialog.cxx			test/unittest_simple_terminal.cxx
test/icon.cxx				test/unittest_symbol.cxx
test/iconize.cxx			test/unittest_text.cxx
test/image.cxx				test/unittest_viewport.cxx
test/input.cxx				test/unittests.cxx
test/input_choice.cxx			test/utf8.cxx
test/keyboard.cxx			test/windowfocus.cxx
  * test/label.cxx        : + 'label' works

 */