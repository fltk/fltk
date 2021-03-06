//
// Definition of SDL Window interface based on SDL
//
// Copyright 1998-2018 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//


#include <config.h>
#include "Fl_PicoSDL_Window_Driver.H"

#include <FL/platform.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>


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
  other_xid = 0;
  x->w = pWindow;
  x->region = 0;
  if (!pWindow->force_position()) {
    pNativeWindow = SDL_CreateWindow(pWindow->label(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w(), h(), 0);
  } else {
    pNativeWindow = SDL_CreateWindow(pWindow->label(), pWindow->x(), pWindow->y(), pWindow->w(), pWindow->h(), 0);
  }
  x->xid = SDL_CreateRenderer(pNativeWindow, -1, SDL_RENDERER_ACCELERATED);
  pNativeTexture = SDL_CreateTexture((SDL_Renderer*)x->xid, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, w(), h());
  x->next = Fl_X::first;
  wait_for_expose_value = 0;
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


void Fl_PicoSDL_Window_Driver::draw_end()
{
  Fl_X *i = Fl_X::i(pWindow);
  SDL_SetRenderTarget((SDL_Renderer*)pWindow->i->xid, 0L);
  //SDL_RenderClear((SDL_Renderer*)i->xid);
  SDL_RenderCopy((SDL_Renderer*)i->xid, pNativeTexture, 0L, 0L);
  SDL_RenderPresent((SDL_Renderer*)i->xid);
}


void Fl_PicoSDL_Window_Driver::make_current()
{
  fl_window = pWindow->i->xid;
  SDL_SetRenderTarget((SDL_Renderer*)pWindow->i->xid, pNativeTexture);
}


void Fl_PicoSDL_Window_Driver::show() {
  if (!shown()) {
    makeWindow();
  }
}
