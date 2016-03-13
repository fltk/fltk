//
// "$Id: Fl_PicoAndroid_Window_Driver.cxx 11253 2016-03-01 00:54:21Z matt $"
//
// Definition of Android Window interface based on SDL
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
#include "Fl_PicoAndroid_Window_Driver.H"

#include "Fl_PicoAndroid_Screen_Driver.H"

#include <jni.h>
#include <errno.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#include <FL/x.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Window_Driver.H>


Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *win)
{
  return new Fl_PicoAndroid_Window_Driver(win);
}


Fl_PicoAndroid_Window_Driver::Fl_PicoAndroid_Window_Driver(Fl_Window *win)
: Fl_Pico_Window_Driver(win)
{
}


Fl_PicoAndroid_Window_Driver::~Fl_PicoAndroid_Window_Driver()
{
}


Fl_X *Fl_PicoAndroid_Window_Driver::makeWindow()
{
  Fl_PicoAndroid_Screen_Driver *scr = (Fl_PicoAndroid_Screen_Driver*)Fl::screen_driver();

  Fl_Group::current(0);
  if (pWindow->parent() && !Fl_X::i(pWindow->window())) {
    pWindow->set_visible();
    return 0L;
  }
  Window parent;
  if (pWindow->parent()) {
    parent = fl_xid(pWindow->window());
  } else {
    parent = 0;
  }
  Fl_X *x = new Fl_X;
  x->other_xid = 0;
  x->w = pWindow;
  x->region = 0;
  if (!pWindow->force_position()) {
//    pNativeWindow = SDL_CreateWindow(pWindow->label(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, pWindow->w(), pWindow->h(), 0);
  } else {
//    pNativeWindow = SDL_CreateWindow(pWindow->label(), pWindow->x(), pWindow->y(), pWindow->w(), pWindow->h(), 0);
  }
  pNativeWindow = scr->pApp->window;
//  x->xid = SDL_CreateRenderer(pNativeWindow, -1, SDL_RENDERER_ACCELERATED);
  x->next = Fl_X::first;
  x->wait_for_expose = 0;
  pWindow->i = x;
  Fl_X::first = x;

  pWindow->set_visible();
  pWindow->redraw();
  flush();
  int old_event = Fl::e_number;
  pWindow->handle(Fl::e_number = FL_SHOW);
  Fl::e_number = old_event;

  return x;
}

#include <FL/fl_draw.h>


void Fl_Window_Driver::draw_begin()
{
}


void Fl_Window_Driver::draw_end()
{
}


void Fl_PicoAndroid_Window_Driver::flush()
{
  Fl_PicoAndroid_Screen_Driver *scr = (Fl_PicoAndroid_Screen_Driver*)Fl::screen_driver();
//  LOGI("Flush...");
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  pWindow->flush();
//  fl_color(FL_RED);
//  fl_rectf(10, 10, 300, 400);
  scr->drawFrame();
}


//
// End of "$Id: Fl_PicoSDL_Window_Driver.cxx 11253 2016-03-01 00:54:21Z matt $".
//