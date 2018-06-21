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


#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Box.H>
#include <string.h>
#include <stdio.h>
#include <FL/fl_draw.H>
#include <FL/math.h>

class Drawing : public Fl_Widget {
  void draw();
public:
  Drawing(int X,int Y,int W,int H,const char* L) : Fl_Widget(X,Y,W,H,L) {
    align(FL_ALIGN_TOP);
    box(FL_FLAT_BOX);
    color(FL_WHITE);
  }
};

void Drawing::draw() {
  draw_box();
  fl_push_matrix();
  fl_translate(x()+w()/2, y()+h()/2);
  fl_scale(w()/2, h()/2);
  fl_color(FL_BLACK);
  for (int i = 0; i < 20; i++) {
    for (int j = i+1; j < 20; j++) {
      fl_begin_line();
      fl_vertex(cos(M_PI*i/10+.1), sin(M_PI*i/10+.1));
      fl_vertex(cos(M_PI*j/10+.1), sin(M_PI*j/10+.1));
      fl_end_line();
    }
  }
  fl_pop_matrix();
}

Fl_Scroll* thescroll;

void box_cb(Fl_Widget* o, void*) {
  thescroll->box(((Fl_Button*)o)->value() ? FL_DOWN_FRAME : FL_NO_BOX);
  thescroll->redraw();
}

void type_cb(Fl_Widget*, void* v) {
  thescroll->type((uchar)((fl_intptr_t)v));
  thescroll->redraw();
}
Fl_Menu_Item choices[] = {
        {"0", 0, type_cb, (void*)0},
        {"HORIZONTAL", 0, type_cb, (void*)Fl_Scroll::HORIZONTAL},
        {"VERTICAL", 0, type_cb, (void*)Fl_Scroll::VERTICAL},
        {"BOTH", 0, type_cb, (void*)Fl_Scroll::BOTH},
        {"HORIZONTAL_ALWAYS", 0, type_cb, (void*)Fl_Scroll::HORIZONTAL_ALWAYS},
        {"VERTICAL_ALWAYS", 0, type_cb, (void*)Fl_Scroll::VERTICAL_ALWAYS},
        {"BOTH_ALWAYS", 0, type_cb, (void*)Fl_Scroll::BOTH_ALWAYS},
        {0}
};

void align_cb(Fl_Widget*, void* v) {
  thescroll->scrollbar.align((uchar)((fl_intptr_t)v));
  thescroll->redraw();
}

Fl_Menu_Item align_choices[] = {
        {"left+top", 0, align_cb, (void*)(FL_ALIGN_LEFT+FL_ALIGN_TOP)},
        {"left+bottom", 0, align_cb, (void*)(FL_ALIGN_LEFT+FL_ALIGN_BOTTOM)},
        {"right+top", 0, align_cb, (void*)(FL_ALIGN_RIGHT+FL_ALIGN_TOP)},
        {"right+bottom", 0, align_cb, (void*)(FL_ALIGN_RIGHT+FL_ALIGN_BOTTOM)},
        {0}
};

int main(int argc, char** argv) {
  Fl_Window window(5*75,400);
  window.box(FL_NO_BOX);
  Fl_Scroll scroll(0,0,5*75,300);

  int n = 0;
  for (int y=0; y<16; y++) for (int x=0; x<5; x++) {
      char buf[20]; sprintf(buf,"%d",n++);
      Fl_Button* b = new Fl_Button(x*75,y*25+(y>=8?5*75:0),75,25);
      b->copy_label(buf);
      b->color(n);
      b->labelcolor(FL_WHITE);
    }
  Drawing drawing(0,8*25,5*75,5*75,0);
  scroll.end();
  window.resizable(scroll);

  Fl_Box box(0,300,5*75,window.h()-300); // gray area below the scroll
  box.box(FL_FLAT_BOX);

  Fl_Light_Button but1(150, 310, 200, 25, "box");
  but1.callback(box_cb);

  Fl_Choice choice(150, 335, 200, 25, "type():");
  choice.menu(choices);
  choice.value(3);

  Fl_Choice achoice(150, 360, 200, 25, "scrollbar.align():");
  achoice.menu(align_choices);
  achoice.value(3);

  thescroll = &scroll;

  //scroll.box(FL_DOWN_BOX);
  //scroll.type(Fl_Scroll::VERTICAL);
  window.end();
  window.show(argc,argv);
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
  - grab() not working when leaving window (adjuster...)
  - scrolling if implemented as a complete redraw. Must implement real scrolling
  - the 'hotspot' idea to position dialogs under the mouse cursor makes little sense on touch screen devices
  - fix screen when keyboard pops up in front of the text cursor or input field (temporarily shift up?)
  - ending 'message' will not quit the app right away, but wait for some timeout


test/CubeMain.cxx
test/line_style.cxx
test/CubeView.cxx
test/list_visuals.cxx
test/mandelbrot.cxx
test/animated.cxx
test/native-filechooser.cxx
test/blocks.cxx
test/navigation.cxx
test/offscreen.cxx
test/overlay.cxx
test/cairo_test.cxx
test/pixmap.cxx
test/checkers.cxx
test/pixmap_browser.cxx
test/clock.cxx
test/resizebox.cxx
test/colbrowser.cxx
test/rotated_text.cxx
test/scroll.cxx
test/connect.cxx
test/shape.cxx
test/cube.cxx
test/subwindow.cxx
test/cursor.cxx
test/sudoku.cxx
test/curve.cxx
test/demo.cxx
test/table.cxx
test/device.cxx
test/threads.cxx
test/doublebuffer.cxx
test/tile.cxx
test/editor.cxx
test/tiled_image.cxx
test/file_chooser.cxx
test/twowin.cxx
test/fonts.cxx
test/unittest_about.cxx
test/forms.cxx
test/unittest_circles.cxx
test/fractals.cxx
test/unittest_images.cxx
test/fracviewer.cxx
test/unittest_lines.cxx
test/fullscreen.cxx
test/unittest_points.cxx
test/gl_overlay.cxx
test/unittest_rects.cxx
test/glpuzzle.cxx
test/unittest_schemes.cxx
test/unittest_scrollbarsize.cxx
test/help_dialog.cxx
test/unittest_simple_terminal.cxx
test/icon.cxx
test/unittest_symbol.cxx
test/iconize.cxx
test/unittest_text.cxx
test/image.cxx
test/unittest_viewport.cxx
test/input.cxx
test/unittests.cxx
test/input_choice.cxx
test/utf8.cxx
test/keyboard.cxx
test/windowfocus.cxx
test/browser.cxx

  * test/scroll.cxx       : - works ok
                            - some dirt when a popup draws over another menu button!?
                            - on touch-screens, menuitem should be selected when released
                            - on touch-screens, scroll groups should scroll on multitouch, or when not causing any other action
  * test/bitmap.cxx       : + 'bitmap' works
  * test/message.cxx      : - 'message' mostly works
                            - when ending the app, it will not close right away but instead hang around for a few seconds
  * test/menubar.cxx      : - 'menubar' mostly works including unicode
                            ! pressing 'button' will hang the app
                            - shortcut modifiers don't work
                            - right-click does not work (should this be emulated via click-and-hold?)
  * test/output.cxx       : + 'output' works
  * test/ask.cxx          : + 'ask' works
  * test/button.cxx       : + 'button' works, including beep
  * test/pack.cxx         : + 'pack' works
  * test/adjuster.cxx     : + 'adjuster' works
  * test/arc.cxx          : + 'arc' works as expected
  * test/minimum.cxx      : + 'minimum' works
  * test/boxtype.cxx      : + 'boxtype' works
  * test/buttons.cxx      : + 'buttons' works
  * test/color_chooser.cxx: + 'color_chooser' works
  * test/symbols.cxx      : + 'symbols' working as expected
  * test/hello.cxx        : + 'hello' works fine, italics, shadow, etc.
  * test/label.cxx        : + 'label' works

 */