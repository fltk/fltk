//
// "$Id: Fl_PicoSDL_Screen_Driver.cxx 11253 2016-03-01 00:54:21Z matt $"
//
// Definition of SDL Screen interface based on Pico
//
// Copyright 1998-2016 by Bill Spitzak and others.
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


#include "../../config_lib.h"
#include "Fl_PicoSDL_Screen_Driver.H"

#include <FL/x.H>
#include <FL/Fl_Window_Driver.H>

#define __APPLE__
#include <SDL2/SDL.h>
#undef __APPLE__


Fl_Screen_Driver* Fl_Screen_Driver::newScreenDriver()
{
  return new Fl_PicoSDL_Screen_Driver();
}


Fl_PicoSDL_Screen_Driver::Fl_PicoSDL_Screen_Driver()
{
}

Fl_PicoSDL_Screen_Driver::~Fl_PicoSDL_Screen_Driver()
{
}


#if 0
if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
  //    fl_lock_function();
  int x = AMotionEvent_getX(event, 0);
  int y = AMotionEvent_getY(event, 0);
  int action = AKeyEvent_getAction(event);
  Fl_Window *window = Fl::first_window();
  switch (action) {
    case AMOTION_EVENT_ACTION_DOWN:
      Fl::e_is_click = 1;
      Fl::e_x = Fl::e_x_root = x/3;
      Fl::e_y = (y-100)/3;
      if (!window) break;
      Fl::e_keysym = FL_Button+FL_LEFT_MOUSE;
      Fl::e_state = FL_BUTTON1;
      Fl::handle(FL_PUSH, window);
      break;
    case AMOTION_EVENT_ACTION_MOVE:
      Fl::e_is_click = 1;
      Fl::e_x = Fl::e_x_root = x/3;
      Fl::e_y = (y-100)/3;
      if (!window) break;
      Fl::e_keysym = FL_Button+FL_LEFT_MOUSE;
      Fl::e_state = FL_BUTTON1;
      Fl::handle(FL_DRAG, window);
      break;
    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_CANCEL:
      Fl::e_is_click = 1;
      Fl::e_x = Fl::e_x_root = x/3;
      Fl::e_y = (y-100)/3;
      if (!window) break;
      Fl::e_keysym = FL_Button+FL_LEFT_MOUSE;
      Fl::e_state = 0;
      Fl::handle(FL_RELEASE, window);
      break;
      //      case AMOTION_EVENT_ACTION_HOVER_MOVE:
      //        Fl::e_is_click = 1;
      //        Fl::e_x = Fl::e_x_root = x/3;
      //        Fl::e_y = (y-100)/3;
      //        if (!window) break;
      //        Fl::e_keysym = 0;
      //        Fl::e_state = 0;
      //        Fl::handle(FL_MOVE, window);
      //        break;
  }
  //    AMOTION_EVENT_ACTION_MASK
  LOGI("Motion at %d, %d", x, y);
  //    fl_unlock_function();
  Fl_X::first->w->redraw();
  return 1;
}
return 0;
#endif

double Fl_PicoSDL_Screen_Driver::wait(double time_to_wait)
{
  Fl::flush();
  SDL_Event e;
  Fl_Window *window = Fl::first_window();
  if (SDL_PollEvent(&e)) {
    switch (e.type) {
      case SDL_QUIT:
        exit(0);
      case SDL_WINDOWEVENT_EXPOSED:
      case SDL_WINDOWEVENT_SHOWN:
      { // not happening!
        //event->window.windowID
        if ( !window ) break;;
        Fl_X *i = Fl_X::i(Fl::first_window());
        i->wait_for_expose = 0;

        if ( i->region ) {
          XDestroyRegion(i->region);
          i->region = 0;
        }
        window->clear_damage(FL_DAMAGE_ALL);
        i->flush();
        window->clear_damage();
        Fl_X::first->wait_for_expose = 0;
      }
        break;
      case SDL_MOUSEBUTTONDOWN:
        if (!window) break;
        Fl::e_is_click = e.button.clicks;
        Fl::e_x = e.button.x;
        Fl::e_y = e.button.y;
        Fl::e_x_root = e.button.x + window->x();
        Fl::e_y_root = e.button.y + window->y();
        switch (e.button.button) {
          case SDL_BUTTON_LEFT:   Fl::e_keysym = FL_Button+FL_LEFT_MOUSE;   Fl::e_state |= FL_BUTTON1; break;
          case SDL_BUTTON_MIDDLE: Fl::e_keysym = FL_Button+FL_MIDDLE_MOUSE; Fl::e_state |= FL_BUTTON2; break;
          case SDL_BUTTON_RIGHT:  Fl::e_keysym = FL_Button+FL_RIGHT_MOUSE;  Fl::e_state |= FL_BUTTON3; break;
        }
        Fl::handle(FL_PUSH, window);
        break;
      case SDL_MOUSEBUTTONUP:
        if (!window) break;
        Fl::e_is_click = e.button.clicks;
        Fl::e_x = e.button.x;
        Fl::e_y = e.button.y;
        Fl::e_x_root = e.button.x + window->x();
        Fl::e_y_root = e.button.y + window->y();
        switch (e.button.button) {
          case SDL_BUTTON_LEFT:   Fl::e_keysym = FL_Button+FL_LEFT_MOUSE;   Fl::e_state &= ~FL_BUTTON1; break;
          case SDL_BUTTON_MIDDLE: Fl::e_keysym = FL_Button+FL_MIDDLE_MOUSE; Fl::e_state &= ~FL_BUTTON2; break;
          case SDL_BUTTON_RIGHT:  Fl::e_keysym = FL_Button+FL_RIGHT_MOUSE;  Fl::e_state &= ~FL_BUTTON3; break;
        }
        Fl::handle(FL_RELEASE, window);
        break;
      case SDL_MOUSEMOTION: // SDL_BUTTON_LMASK
        if (!window) break;
        Fl::e_is_click = e.motion.state;
        Fl::e_x = e.motion.x;
        Fl::e_y = e.motion.y;
        Fl::e_x_root = e.motion.x + window->x();
        Fl::e_y_root = e.motion.y + window->y();
        if (e.motion.state & SDL_BUTTON_LMASK) Fl::e_state |= FL_BUTTON1; else Fl::e_state &= ~FL_BUTTON1; break;
        if (e.motion.state & SDL_BUTTON_MMASK) Fl::e_state |= FL_BUTTON2; else Fl::e_state &= ~FL_BUTTON2; break;
        if (e.motion.state & SDL_BUTTON_RMASK) Fl::e_state |= FL_BUTTON3; else Fl::e_state &= ~FL_BUTTON3; break;
        if ((e.motion.state & (SDL_BUTTON_LMASK|SDL_BUTTON_MMASK|SDL_BUTTON_RMASK)) == 0 )
          Fl::handle(FL_MOVE, window);
        else
          Fl::handle(FL_DRAG, window);
        break;
      case SDL_MOUSEWHEEL:
        break;
    }
  }
  return 0.0;
}



#include <FL/x.H>
#include <FL/Fl.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Graphics_Driver.H>

/*
 * The following code should not be here! 
 * All this must be refactored into the driver system!
 */

/*

 The following symbols are not found if we naively compile the core modules and
 no specific platform implementations. This list is a hint at all the functions
 and methods that probably need to be refactored into the driver system.

 Undefined symbols for architecture x86_64:
 */

void fl_set_spot(int, int, int, int, int, int, Fl_Window*) { }
void fl_reset_spot() { }
const char *fl_filename_name(char const*) { return 0; }
void fl_clipboard_notify_change() { }

//Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver() { return 0; }
//Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver() { return 0; }
void Fl_Graphics_Driver::global_gc() { }
int Fl::dnd() { return 0; }
void Fl::copy(char const*, int, int, char const*) { }
void Fl::paste(Fl_Widget&, int, char const*) { }
void Fl::get_mouse(int&, int&) { }
void Fl::set_color(unsigned int, unsigned int) { }
int Fl_X::set_cursor(Fl_Cursor) { return 0; }
int Fl_X::set_cursor(Fl_RGB_Image const*, int, int) { return 0; }
void Fl_X::set_default_icons(Fl_RGB_Image const**, int) { }

void Fl_X::flush()
{
  w->driver()->flush();
}

void Fl_X::set_icons() { }
void Fl_Window::size_range_() { }
void Fl_Window::fullscreen_x() { }

void Fl_Window::make_current()
{
  fl_window = i->xid;
  current_ = this;
}

void Fl_Window::fullscreen_off_x(int, int, int, int) { }

Window fl_xid(const Fl_Window* w)
{
  Fl_X *temp = Fl_X::i(w);
  return temp ? temp->xid : 0;
}

void Fl_Window::show() {
  if (!shown()) {
    Fl_X::make(this);
  }
}

Fl_X* Fl_X::make(Fl_Window *w)
{
  return w->driver()->makeWindow();
}

void Fl_Window::label(char const*, char const*) { }
void Fl_Window::resize(int, int, int, int) { }
Fl_Window *Fl_Window::current_;
char fl_show_iconic;
Window fl_window;
//void Fl_Image_Surface::translate(int x, int y) { }
//void Fl_Image_Surface::untranslate() { }

void Fl::add_fd(int, void (*)(int, void*), void*)
{
}

void Fl::remove_fd(int)
{
}

int Fl_Window::decorated_h()
{
}

int Fl_Window::decorated_w()
{
}

/*
 #define __APPLE__
 #include <SDL2/SDL.h>
 #undef __APPLE__
 
 SDL_Window *win = NULL;
 SDL_Renderer *renderer = NULL;
 SDL_Texture *bitmapTex = NULL;
 SDL_Surface *bitmapSurface = NULL;
 int posX = 100, posY = 100, width = 320, height = 240;

 SDL_Init(SDL_INIT_VIDEO);

 win = SDL_CreateWindow("Hello World", posX, posY, width, height, 0);

 renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

 bitmapSurface = SDL_LoadBMP("img/hello.bmp");
 bitmapTex = SDL_CreateTextureFromSurface(renderer, bitmapSurface);
 SDL_FreeSurface(bitmapSurface);

 while (1) {
 SDL_Event e;
 if (SDL_PollEvent(&e)) {
 if (e.type == SDL_QUIT) {
 break;
 }
 }

 SDL_RenderClear(renderer);
 SDL_RenderCopy(renderer, bitmapTex, NULL, NULL);
 SDL_RenderPresent(renderer);
 }

 SDL_DestroyTexture(bitmapTex);
 SDL_DestroyRenderer(renderer);
 SDL_DestroyWindow(win);

 SDL_Quit();

 return 0;
*/


//
// End of "$Id: Fl_PicoSDL_Screen_Driver.cxx 11253 2016-03-01 00:54:21Z matt $".
//

