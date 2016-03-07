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


double Fl_PicoSDL_Screen_Driver::wait(double time_to_wait)
{
  SDL_Event e;
  if (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      exit(0);
      // TODO: do something
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
Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver() { return 0; }
void Fl_Graphics_Driver::global_gc() { }
int Fl::dnd() { return 0; }
void Fl::copy(char const*, int, int, char const*) { }
void Fl::paste(Fl_Widget&, int, char const*) { }
void Fl::get_mouse(int&, int&) { }
void Fl::set_color(unsigned int, unsigned int) { }
int Fl_X::set_cursor(Fl_Cursor) { return 0; }
int Fl_X::set_cursor(Fl_RGB_Image const*, int, int) { return 0; }
void Fl_X::set_default_icons(Fl_RGB_Image const**, int) { }
void Fl_X::flush() { }
void Fl_X::set_icons() { }
void Fl_Window::size_range_() { }
void Fl_Window::fullscreen_x() { }
void Fl_Window::make_current() { }
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
  Fl_Group::current(0);
  if (w->parent() && !Fl_X::i(w->window())) {
    w->set_visible();
    return 0L;
  }
  Window parent;
  if (w->parent()) {
    parent = fl_xid(w->window());
  } else {
    parent = 0;
  }
  Fl_Window_Driver *x = Fl_Window_Driver::newWindowDriver(w);
  x->other_xid = 0;
  x->w = w;
  x->region = 0;
  if (!w->force_position()) {
    x->xid = SDL_CreateWindow(w->label(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w->w(), w->h(), 0);
  } else {
    x->xid = SDL_CreateWindow(w->label(), w->x(), w->y(), w->w(), w->h(), 0);
  }
  x->next = Fl_X::first;
  Fl_X::first = x;
  return x;
}

void Fl_Window::label(char const*, char const*) { }
void Fl_Window::resize(int, int, int, int) { }
Fl_Window *Fl_Window::current_;
char fl_show_iconic;
Window fl_window;
//void Fl_Image_Surface::translate(int x, int y) { }
//void Fl_Image_Surface::untranslate() { }

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

