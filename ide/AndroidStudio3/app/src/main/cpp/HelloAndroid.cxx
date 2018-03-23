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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/fl_draw.H>

int N = 0;
#define W 70
#define H 70
#define ROWS 7
#define COLS 6

Fl_Double_Window *window;
Fl_Value_Slider *orientation;
Fl_Value_Slider *size;

void slider_cb(Fl_Widget *, void *) {
  static char buf[80];
  int val = (int)orientation->value();
  int sze = (int)size->value();
  for (int i = window->children(); i--; ) {          // all window children
    Fl_Widget *wc = window->child(i);
    const char *l = (const char *)(wc->user_data());
    if ( l && *l == '@' ) {                       // all children with '@'
      l ++;
      if ( wc->box() == FL_NO_BOX ) {             // ascii legend?
        if (val&&sze) sprintf(buf, "@@%+d%d%s", sze, val, l);
        else if (val) sprintf(buf, "@@%d%s", val, l);
        else if (sze) sprintf(buf, "@@%+d%s", sze, l);
        else          sprintf(buf, "@@%s", l);
      } else {                                    // box with symbol
        if (val&&sze) sprintf(buf, "@%+d%d%s", sze, val, l);
        else if (val) sprintf(buf, "@%d%s", val, l);
        else if (sze) sprintf(buf, "@%+d%s", sze, l);
        else          sprintf(buf, "@%s", l);
      }
      wc->copy_label(buf);
    }
  }
  window->redraw();
}

void bt(const char *name) {
  int x = N%COLS;
  int y = N/COLS;
  char buf[255];
  N++;
  x = x*W+10;
  y = y*H+10;
  sprintf(buf, "@%s", name);
  Fl_Box *a = new Fl_Box(x,y,W-20,H-20);
  a->box(FL_NO_BOX);
  a->copy_label(buf);
  a->align(FL_ALIGN_BOTTOM);
  a->labelsize(11);
  a->user_data((void *)name);
  Fl_Box *b = new Fl_Box(x,y,W-20,H-20);
  b->box(FL_UP_BOX);
  b->copy_label(name);
  b->labelcolor(FL_DARK3);
  b->user_data((void *)name);
}

int main(int argc, char ** argv) {
  window = new Fl_Double_Window(COLS*W,ROWS*H+60);
  bt("@->");
  bt("@>");
  bt("@>>");
  bt("@>|");
  bt("@>[]");
  bt("@|>");
  bt("@<-");
  bt("@<");
  bt("@<<");
  bt("@|<");
  bt("@[]<");
  bt("@<|");
  bt("@<->");
  bt("@-->");
  bt("@+");
  bt("@->|");
  bt("@||");
  bt("@<|");
  bt("@<->");
  bt("@-->");
  bt("@+");
  bt("@->|");
  bt("@||");
  bt("@arrow");
  bt("@returnarrow");
  bt("@square");
  bt("@circle");
  bt("@line");
  bt("@menu");
  bt("@UpArrow");
  bt("@DnArrow");
  bt("@search");
  bt("@FLTK");
  bt("@filenew");
  bt("@fileopen");
  bt("@filesave");
  bt("@filesaveas");
  bt("@fileprint");
  bt("@refresh");
  bt("@reload");
  bt("@undo");
  bt("@redo");

  orientation = new Fl_Value_Slider(
          (int)(window->w()*.05+.5), window->h()-40,
          (int)(window->w()*.42+.5), 16, "Orientation");
  orientation->type(FL_HORIZONTAL);
  orientation->range(0.0, 9.0);
  orientation->value(0.0);
  orientation->step(1);
  orientation->callback(slider_cb, 0);

  size = new Fl_Value_Slider(
          (int)(window->w()*.53+.5), window->h()-40,
          (int)(window->w()*.42+.5), 16, "Size");
  size->type(FL_HORIZONTAL);
  size->range(-3.0, 9.0);
  size->value(0.0);
  size->step(1);
  size->callback(slider_cb, 0);

  window->resizable(window);
  window->show(argc,argv);
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
  - ...::line(...) has round ing issues (see rounded box type)


test/CubeMain.cxx			test/line_style.cxx
test/CubeView.cxx			test/list_visuals.cxx
  * test/adjuster.cxx     : - missing bitmap drawing
                                        test/mandelbrot.cxx
test/animated.cxx			test/menubar.cxx
  * test/arc.cxx          : + 'arc' works as expected
                                        test/message.cxx
test/ask.cxx
  * test/minimum.cxx      : + 'minimum' works
test/bitmap.cxx				test/native-filechooser.cxx
test/blocks.cxx				test/navigation.cxx
  * test/boxtype.cxx      : + 'boxtype': works
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
test/curve.cxx
  * test/symbols.cxx      : !! working this out: implement circle drawing
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
test/label.cxx

 */