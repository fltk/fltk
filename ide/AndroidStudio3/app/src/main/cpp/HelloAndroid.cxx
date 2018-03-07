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

Fl_Window *win;
Fl_Button *btn;

int h(void*, void*)
{
  Fl_Android_Application::log_w("App global even %p", Fl::event());
  return 0;
}

int main(int argc, char **argv)
{
  Fl::add_system_handler(h, 0);
  win = new Fl_Window(10, 10, 600, 400, "Hallo");
  btn = new Fl_Button(190, 200, 280, 35, "Hello, Android!");
  btn->color(FL_LIGHT2);
  win->show(argc, argv);

  Fl::run();

  return 0;
}
