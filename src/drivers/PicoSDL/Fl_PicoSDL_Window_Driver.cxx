//
// "$Id: Fl_PicoSDL_Window_Driver.cxx 11253 2016-03-01 00:54:21Z matt $"
//
// Definition of SDL Window interface based on SDL
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
#include "Fl_PicoSDL_Window_Driver.H"

#include <FL/x.H>
#include <FL/Fl.H>
#include <FL/Fl_WIndow.H>

void Fl_Window_Driver::default_icons(Fl_RGB_Image const**, int) { }

const char *fl_local_alt = "alt";
const char *fl_local_ctrl = "ctrl";
const char *fl_local_meta = "meta";
const char *fl_local_shift = "shift";


Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *win)
{
  return new Fl_PicoSDL_Window_Driver(win);
}


Fl_PicoSDL_Window_Driver::Fl_PicoSDL_Window_Driver(Fl_Window *win)
: Fl_Pico_Window_Driver(win)
{
}


Fl_PicoSDL_Window_Driver::~Fl_PicoSDL_Window_Driver()
{
}


Fl_X *Fl_PicoSDL_Window_Driver::makeWindow()
{
  Fl_Group::current(0);
  if (parent() && !Fl_X::i(pWindow->window())) {
    pWindow->set_visible();
    return 0L;
  }
  Window parent;
  if (this->parent()) {
    parent = fl_xid(pWindow->window());
  } else {
    parent = 0;
  }
  Fl_X *x = new Fl_X;
  x->other_xid = 0;
  x->w = pWindow;
  x->region = 0;
  if (!pWindow->force_position()) {
    pNativeWindow = SDL_CreateWindow(pWindow->label(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w(), h(), 0);
  } else {
    pNativeWindow = SDL_CreateWindow(pWindow->label(), pWindow->x(), pWindow->y(), pWindow->w(), pWindow->h(), 0);
  }
  x->xid = SDL_CreateRenderer(pNativeWindow, -1, SDL_RENDERER_ACCELERATED);
  x->next = Fl_X::first;
  x->wait_for_expose = 0;
  pWindow->i = x;
  Fl_X::first = x;

  pWindow->set_visible();
  pWindow->redraw();
  pWindow->flush();
  int old_event = Fl::e_number;
  pWindow->handle(Fl::e_number = FL_SHOW);
  Fl::e_number = old_event;

  return x;
}


#if 0
void Fl_PicoSDL_Window_Driver::flush_single()
{
  if (!shown()) return;
  pWindow->make_current();
  Fl_X *i = Fl_X::i(pWindow);
  if (!i) return;
  fl_clip_region(i->region);
  i->region = 0;
  //  SDL_RenderClear((SDL_Renderer*)i->xid);
  pWindow->draw();
  SDL_RenderPresent((SDL_Renderer*)i->xid);
}
#endif


void Fl_PicoSDL_Window_Driver::draw_end()
{
//  if (!shown()) return;
//  pWindow->make_current();
  Fl_X *i = Fl_X::i(pWindow);
//  if (!i) return;
//  fl_clip_region(i->region);
//  i->region = 0;
//  //  SDL_RenderClear((SDL_Renderer*)i->xid);
//  pWindow->draw();
  SDL_RenderPresent((SDL_Renderer*)i->xid);
}


void Fl_PicoSDL_Window_Driver::make_current()
{
  fl_window = pWindow->i->xid;
}

void Fl_PicoSDL_Window_Driver::show() {
  if (!shown()) {
    makeWindow();
  }
}



//
// End of "$Id: Fl_PicoSDL_Window_Driver.cxx 11253 2016-03-01 00:54:21Z matt $".
//