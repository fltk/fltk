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
#include <FL/Fl_Value_Input.H> // necessary for bug in mingw32?
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Input.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Multiline_Output.H>

Fl_Output *text;
Fl_Multiline_Output *text2;
Fl_Input *input;
Fl_Value_Slider *fonts;
Fl_Value_Slider *sizes;
Fl_Double_Window *window;

void font_cb(Fl_Widget *,void *) {
  text->textfont(int(fonts->value()));
  text->redraw();
  text2->textfont(int(fonts->value()));
  text2->redraw();
}

void size_cb(Fl_Widget *,void *) {
  text->textsize(int(sizes->value()));
  text->redraw();
  text2->textsize(int(sizes->value()));
  text2->redraw();
}

void input_cb(Fl_Widget *,void *) {
  text->value(input->value());
  text2->value(input->value());
}

int main(int argc, char **argv) {
  window = new Fl_Double_Window(400,400);

  input = new Fl_Input(50,375,350,25);
  input->static_value("The quick brown fox\njumped over\nthe lazy dog.");
  input->when(FL_WHEN_CHANGED);
  input->callback(input_cb);

  sizes = new Fl_Hor_Value_Slider(50,350,350,25,"Size");
  sizes->align(FL_ALIGN_LEFT);
  sizes->bounds(1,64);
  sizes->step(1);
  sizes->value(14);
  sizes->callback(size_cb);

  fonts = new Fl_Hor_Value_Slider(50,325,350,25,"Font");
  fonts->align(FL_ALIGN_LEFT);
  fonts->bounds(0,15);
  fonts->step(1);
  fonts->value(0);
  fonts->callback(font_cb);

  text2 = new Fl_Multiline_Output(100,150,200,100,"Fl_Multiline_Output");
  text2->value(input->value());
  text2->align(FL_ALIGN_BOTTOM);
  text2->tooltip("This is an Fl_Multiline_Output widget.");
  window->resizable(text2);

  text = new Fl_Output(100,90,200,30,"Fl_Output");
  text->value(input->value());
  text->align(FL_ALIGN_BOTTOM);
  text->tooltip("This is an Fl_Output widget.");

  window->end();
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
  - grab() not working when leaving window (adjuster...)
  - scrolling if implemented as a complete redraw. Must implement real scrolling
  - the 'hotspot' idea to position dialogs under the mouse cursor makes little sense on touch screen devices
  - fix screen when keyboard pops up in front of the text cursor or input field (temporarily shift up?)


test/CubeMain.cxx
test/line_style.cxx
test/CubeView.cxx
test/list_visuals.cxx
test/mandelbrot.cxx
test/animated.cxx
test/menubar.cxx
test/message.cxx
test/bitmap.cxx
test/native-filechooser.cxx
test/blocks.cxx
test/navigation.cxx
test/offscreen.cxx
test/browser.cxx
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