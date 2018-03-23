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

#if 0


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Hor_Slider.H>
#include <FL/fl_draw.H>

class MyBox : public Fl_Box
{
public:
  MyBox(int x, int y, int w, int h) :
    Fl_Box(x, y, w, h)
  {
    box(FL_DOWN_BOX);
  }
  void draw()
  {
    Fl_Box::draw();
    fl_color(FL_BLUE);
    fl_pie(x()+20, y()+20, w()-40, h()-40, a1, a2);
  }
  double a1 = 0.0;
  double a2 = 240.0;
};

Fl_Window *win;
MyBox *box = 0L;
char s1Label[80] = { 0 };
char s2Label[80] = { 0 };

int main(int argc, char **argv)
{
  win = new Fl_Window(0, 0, 600, 800);

  box = new MyBox(10, 10, 580, 580);

  auto s1 = new Fl_Hor_Slider(210, 600, 380, 45, s1Label);
  s1->range(-360, 360);
  s1->value(0.0);
  s1->labelsize(35);
  s1->align(FL_ALIGN_LEFT);
  s1->increment(1, 0);
  s1->callback(
    [](Fl_Widget *w, void*) {
      auto s = (Fl_Slider*)w;
      box->a1 = s->value();
      sprintf(s1Label, "%g", s->value());
      win->redraw();
    }
  );

  auto s2 = new Fl_Hor_Slider(210, 660, 380, 45, s2Label);
  s2->range(-360, 360);
  s2->value(240.0);
  s2->labelsize(35);
  s2->align(FL_ALIGN_LEFT);
  s2->callback(
          [](Fl_Widget *w, void*) {
            auto s = (Fl_Slider*)w;
            box->a2 = s->value();
            sprintf(s2Label, "%g", s->value());
            win->redraw();
          }
  );

  win->show(argc, argv);
  Fl::run();
  return 0;
}

#elif 1

#include <stdlib.h>
#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

int N = 0;
#define W 200
#define H 50
#define ROWS 14

// Note: Run the program with command line '-s abc' to view boxtypes
// with scheme 'abc'.

// class BoxGroup - minimal class to enable visible box size debugging
//
// Set the following static variables to 0 (default) to disable
// or 1 to enable the given feature.
//
// If you enable the 'outline' variable, then a red frame should be drawn
// around each box, and it should ideally be fully visible.
//
// The white background is optional (otherwise you see the window background).
//
// Set BOTH variables = 0 to show the default image for the FLTK manual.
//
// Note: As of FLTK 1.3.3 (probably since FLTK 1.0/1.1) there are some
// shadows 'leaking' outside their widget bounding boxes (x,y,w,h).
// Is this intentional?

static const int outline = 0; // draw 1-px red frame around all boxes
static const int box_bg  = 0; // draw white background inside all boxes
static const int inactive  = 0; // deactivate boxes and use green background
class BoxGroup : public Fl_Group {
public:
  BoxGroup(int x, int y, int w, int h) : Fl_Group(x,y,w,h) {};
  void draw() {
    draw_box();
    if (outline + box_bg) { // outline or box_bg or both
      Fl_Widget*const* a = array();
      for (int i=children(); i--;) {
        Fl_Widget& o = **a++;
        if (outline) {
          fl_color(FL_RED);
          fl_rect(o.x()-1,o.y()-1,o.w()+2,o.h()+2);
        }
        if (box_bg) {
          fl_color(FL_WHITE);
          fl_rectf(o.x(),o.y(),o.w(),o.h());
        }
        fl_color(FL_BLACK);
      }
    } // outline or box_bg or both
    Fl_Group::draw_children();
  } // draw()
}; // class BoxGroup

Fl_Double_Window *window;

void bt(const char *name, Fl_Boxtype type, int square=0) {
  int x = N%4;
  int y = N/4;
  N++;
  x = x*W+10;
  y = y*H+10;
  Fl_Box *b = new Fl_Box(type,x,y,square ? H-20 : W-20,H-20,name);
  b->labelsize(11);
  if (inactive) {
    b->color(FL_GREEN);
    b->deactivate();
  }
  if (square) b->align(FL_ALIGN_RIGHT);
}

int main(int argc, char ** argv) {
  window = new Fl_Double_Window(-W, 0, 4*W,ROWS*H);
  window->box(FL_FLAT_BOX);
#if 0 // this code uses the command line arguments to set arbitrary color schemes
  Fl::args(argc, argv);
  Fl::get_system_colors();
#elif 0 // this code uses a single color to define a scheme
  Fl::args(argc, argv);
  Fl::get_system_colors();
  Fl::background(113,113,198);
#else // this code uses the nice bright blue background to show box vs. frame types
  Fl::args(argc, argv);
  Fl::get_system_colors();
  window->color(12);// light blue
#endif

  // set window title to show active scheme
  Fl::scheme(Fl::scheme()); // init scheme
  char title[100];
  sprintf(title,"FLTK boxtypes: scheme = '%s'",Fl::scheme()?Fl::scheme():"none");
  window->label(title);

  // create special container group for box size debugging
  BoxGroup *bg = new BoxGroup(0,0,window->w(),window->h());
  bg->box(FL_NO_BOX);

  // create demo boxes
  bt("FL_NO_BOX",FL_NO_BOX);
  bt("FL_FLAT_BOX",FL_FLAT_BOX);
  N += 2; // go to start of next row to line up boxes & frames
  bt("FL_UP_BOX",FL_UP_BOX);
  bt("FL_DOWN_BOX",FL_DOWN_BOX);
  bt("FL_UP_FRAME",FL_UP_FRAME);
  bt("FL_DOWN_FRAME",FL_DOWN_FRAME);
  bt("FL_THIN_UP_BOX",FL_THIN_UP_BOX);
  bt("FL_THIN_DOWN_BOX",FL_THIN_DOWN_BOX);
  bt("FL_THIN_UP_FRAME",FL_THIN_UP_FRAME);
  bt("FL_THIN_DOWN_FRAME",FL_THIN_DOWN_FRAME);
  bt("FL_ENGRAVED_BOX",FL_ENGRAVED_BOX);
  bt("FL_EMBOSSED_BOX",FL_EMBOSSED_BOX);
  bt("FL_ENGRAVED_FRAME",FL_ENGRAVED_FRAME);
  bt("FL_EMBOSSED_FRAME",FL_EMBOSSED_FRAME);
  bt("FL_BORDER_BOX",FL_BORDER_BOX);
  bt("FL_SHADOW_BOX",FL_SHADOW_BOX);
  bt("FL_BORDER_FRAME",FL_BORDER_FRAME);
  bt("FL_SHADOW_FRAME",FL_SHADOW_FRAME);
  bt("FL_ROUNDED_BOX",FL_ROUNDED_BOX);
  bt("FL_RSHADOW_BOX",FL_RSHADOW_BOX);
  bt("FL_ROUNDED_FRAME",FL_ROUNDED_FRAME);
  bt("FL_RFLAT_BOX",FL_RFLAT_BOX);
  bt("FL_OVAL_BOX",FL_OVAL_BOX);
  bt("FL_OSHADOW_BOX",FL_OSHADOW_BOX);
  bt("FL_OVAL_FRAME",FL_OVAL_FRAME);
  bt("FL_OFLAT_BOX",FL_OFLAT_BOX);
  bt("FL_ROUND_UP_BOX",FL_ROUND_UP_BOX);
  bt("FL_ROUND_DOWN_BOX",FL_ROUND_DOWN_BOX);
  bt("FL_DIAMOND_UP_BOX",FL_DIAMOND_UP_BOX);
  bt("FL_DIAMOND_DOWN_BOX",FL_DIAMOND_DOWN_BOX);

  bt("FL_PLASTIC_UP_BOX",FL_PLASTIC_UP_BOX);
  bt("FL_PLASTIC_DOWN_BOX",FL_PLASTIC_DOWN_BOX);
  bt("FL_PLASTIC_UP_FRAME",FL_PLASTIC_UP_FRAME);
  bt("FL_PLASTIC_DOWN_FRAME",FL_PLASTIC_DOWN_FRAME);
  bt("FL_PLASTIC_THIN_UP_BOX",FL_PLASTIC_THIN_UP_BOX);
  bt("FL_PLASTIC_THIN_DOWN_BOX",FL_PLASTIC_THIN_DOWN_BOX);
  N += 2;
  bt("FL_PLASTIC_ROUND_UP_BOX",FL_PLASTIC_ROUND_UP_BOX);
  bt("FL_PLASTIC_ROUND_DOWN_BOX",FL_PLASTIC_ROUND_DOWN_BOX);
  N += 2;

  bt("FL_GTK_UP_BOX",FL_GTK_UP_BOX);
  bt("FL_GTK_DOWN_BOX",FL_GTK_DOWN_BOX);
  bt("FL_GTK_UP_FRAME",FL_GTK_UP_FRAME);
  bt("FL_GTK_DOWN_FRAME",FL_GTK_DOWN_FRAME);
  bt("FL_GTK_THIN_UP_BOX",FL_GTK_THIN_UP_BOX);
  bt("FL_GTK_THIN_DOWN_BOX",FL_GTK_THIN_DOWN_BOX);
  bt("FL_GTK_THIN_UP_FRAME",FL_GTK_THIN_UP_FRAME);
  bt("FL_GTK_THIN_DOWN_FRAME",FL_GTK_THIN_DOWN_FRAME);
  bt("FL_GTK_ROUND_UP_BOX",FL_GTK_ROUND_UP_BOX);
  bt("FL_GTK_ROUND_DOWN_BOX",FL_GTK_ROUND_DOWN_BOX);
  bg->end();
  window->resizable(window);
  window->end();
  window->show();
  return Fl::run();
}





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

test/CubeMain.cxx			test/line_style.cxx
test/CubeView.cxx			test/list_visuals.cxx
test/adjuster.cxx			test/mandelbrot.cxx
test/animated.cxx			test/menubar.cxx
  * test/arc.cxx          : + 'arc' works as expected
                                        test/message.cxx
test/ask.cxx				test/minimum.cxx
test/bitmap.cxx				test/native-filechooser.cxx
test/blocks.cxx				test/navigation.cxx
  * test/boxtype.cxx      : !! 'boxtype': must fix diamond box, plastic up frame, see fourth column
                                        test/offscreen.cxx
test/browser.cxx			test/output.cxx
test/button.cxx				test/overlay.cxx
test/buttons.cxx			test/pack.cxx
test/cairo_test.cxx			test/pixmap.cxx
test/checkers.cxx			test/pixmap_browser.cxx
test/clock.cxx				test/resizebox.cxx
test/colbrowser.cxx			test/rotated_text.cxx
test/color_chooser.cxx			test/scroll.cxx
test/connect.cxx			test/shape.cxx
test/cube.cxx				test/subwindow.cxx
test/cursor.cxx				test/sudoku.cxx
test/curve.cxx				test/symbols.cxx
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
test/hello.cxx        : + 'hello' works fine, italics, shadow, etc.
                                        test/unittest_scrollbarsize.cxx
test/help_dialog.cxx			test/unittest_simple_terminal.cxx
test/icon.cxx				test/unittest_symbol.cxx
test/iconize.cxx			test/unittest_text.cxx
test/image.cxx				test/unittest_viewport.cxx
test/input.cxx				test/unittests.cxx
test/input_choice.cxx			test/utf8.cxx
test/keyboard.cxx			test/windowfocus.cxx
test/label.cxx

 */