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

#include <src/drivers/Android/Fl_Android_Application.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Enumerations.H>
#include <FL/fl_draw.H>


Fl_Window *win, *win1, *win2;
Fl_Button *btn, *btn2;


class MyButton : public Fl_Button
{
public:
  MyButton(int x, int y, int w, int h, const char *l) : Fl_Button(x, y, w, h, l) { }
  void draw() {
    //fl_push_clip(x(), y(), w()*2/3, h()*2/3);
    Fl_Button::draw();
    //fl_pop_clip();
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


int main(int argc, char **argv)
{
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

  btn = new MyButton((win->w()-280)/2, 200, 280, 35, "Hello, Android!");
  btn->color(FL_LIGHT2);
  btn->labelsize(30);
  btn->callback(
          [](Fl_Widget*, void*) {
            Fl::add_timeout(1.0, hello_cb, NULL);
          }
  );

  btn = new MyButton((win->w()-280)/2, 200+40, 280, 35, "Hello, Android!");
  btn->labelfont(FL_TIMES_BOLD_ITALIC);
  btn->labelsize(30);

  btn = new MyButton((win->w()-280)/2, 200+2*40, 280, 35, "Hello, Android!");
  btn->labelfont(FL_COURIER_BOLD_ITALIC);
  btn->labelsize(30);

  btn = new MyButton((win->w()-280)/2, 200+3*40, 280, 35, "Hello, Android!");
  btn->labelfont(FL_SCREEN);
  btn->labelsize(30);

  win->end();
  win->show(argc, argv);

  win2 = new Fl_Window(380-50, 10, 200, 200, "front");
  win2->color(FL_BLUE);
  win2->box(FL_UP_BOX);
  Fl_Button *b2 = new Fl_Button(10, 10, 180, 180, "front");
  b2->color(FL_DARK_BLUE);
  win2->end();
  win2->show();

  Fl::run();

  return 0;
}

