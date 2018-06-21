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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Box.H>

#include <FL/fl_ask.H>

void update_input_text(Fl_Widget* o, const char *input) {
  if (input) {
    o->copy_label(input);
    o->redraw();
  }
}

void rename_me(Fl_Widget*o) {
  const char *input = fl_input("Input:", o->label());
  update_input_text(o, input);
}

void rename_me_pwd(Fl_Widget*o) {
  const char *input = fl_password("Input PWD:", o->label());
  update_input_text(o, input);
}

void window_callback(Fl_Widget*, void*) {
  int hotspot = fl_message_hotspot();
  fl_message_hotspot(0);
  fl_message_title("note: no hotspot set for this dialog");
  int rep = fl_choice("Are you sure you want to quit?",
                      "Cancel", "Quit", "Dunno");
  fl_message_hotspot(hotspot);
  if (rep==1)
    exit(0);
  else if (rep==2)
    fl_message("Well, maybe you should know before we quit.");
}
/*
  This timer callback shows a message dialog (fl_choice) window
  every 5 seconds to test "recursive" common dialogs.

  The timer can be stopped by clicking the button "Stop these funny popups"
  or pressing the Enter key. As it is currently implemented, clicking the
  "Close" button will reactivate the popups (only possible if "recursive"
  dialogs are enabled, see below).

  Note 1: This dialog box is blocked in FLTK 1.3.x if another common dialog
  is already open because the window used is a static (i.e. permanently
  allocated) Fl_Window instance. This should be fixed in FLTK 1.4.0.
  See STR #334 (sic !) and also STR #2751 ("Limit input field characters").
*/
void timer_cb(void *) {

  static int stop = 0;
  static const double delta = 5.0;

  Fl_Box *message_icon = (Fl_Box *)fl_message_icon();

  Fl::repeat_timeout(delta, timer_cb);

  if (stop == 1) {
    message_icon->color(FL_WHITE);
    return;
  }

  // Change the icon box color:
  Fl_Color c = message_icon->color();
  c = (c+1) % 32;
  if (c == message_icon->labelcolor()) c++;
  message_icon->color((Fl_Color)c);

  // pop up a message:
  stop = fl_choice("Timeout. Click the 'Close' button.\n"
                           "Note: this message was blocked in FLTK 1.3\n"
                           "if another message window is open.\n"
                           "This *should* be fixed in FLTK 1.4.0!\n"
                           "This message should pop up every 5 seconds.",
                   "Close", "Stop these funny popups", NULL);
}
int main(int argc, char **argv) {
  char buffer[128] = "Test text";
  char buffer2[128] = "MyPassword";

  // This is a test to make sure automatic destructors work. Pop up
  // the question dialog several times and make sure it doesn't crash.

  Fl_Double_Window window(200, 105);
  Fl_Return_Button b(20, 10, 160, 35, buffer);
  b.callback(rename_me);
  Fl_Button b2(20, 50, 160, 35, buffer2);
  b2.callback(rename_me_pwd);
  window.end();
  window.resizable(&b);
  window.show(argc, argv);

  // Also we test to see if the exit callback works:
  window.callback(window_callback);

  // Test: set default message window title:
  // fl_message_title_default("Default Window Title");

  // Test: multiple (nested, aka "recursive") popups
  Fl::add_timeout(5.0, timer_cb);

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
test/output.cxx
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

  * test/ask.cxx          :
    * fix popup position for dialogs
    * fix screen when keyboard pops up in fron of the text cursor or input field

  * test/pack.cxx         : + 'pack' works
  * test/adjuster.cxx     : + 'adjuster' works
  * test/arc.cxx          : + 'arc' works as expected
  * test/minimum.cxx      : + 'minimum' works
  * test/boxtype.cxx      : + 'boxtype' works
  * test/button.cxx       : + 'button' works, including beep
  * test/buttons.cxx      : + 'buttons' works
  * test/color_chooser.cxx:+ 'color_chooser' works
  * test/symbols.cxx      : + 'symbols' working as expected
  * test/hello.cxx        : + 'hello' works fine, italics, shadow, etc.
  * test/label.cxx        : + 'label' works

 */