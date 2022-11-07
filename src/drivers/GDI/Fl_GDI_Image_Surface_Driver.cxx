//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
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


#include "Fl_GDI_Graphics_Driver.H"
#include "../WinAPI/Fl_WinAPI_Screen_Driver.H"
#include "Fl_GDI_Image_Surface_Driver.H"
#include <FL/platform.H>
#include <windows.h>


Fl_GDI_Image_Surface_Driver::Fl_GDI_Image_Surface_Driver(int w, int h, int high_res, Fl_Offscreen off) : Fl_Image_Surface_Driver(w, h, high_res, off) {
  Fl_Display_Device::display_device(); // make sure fl_graphics_driver was initialized
  float d =  fl_graphics_driver->scale();
  if (!off && d != 1 && high_res) {
    w = int(w*d);
    h = int(h*d);
  }
  HDC gc = (HDC)Fl_Graphics_Driver::default_driver().gc();
  offscreen = off ? off : (Fl_Offscreen)CreateCompatibleBitmap( (gc ? gc : fl_GetDC(0) ) , w, h);
  if (!offscreen) offscreen = (Fl_Offscreen)CreateCompatibleBitmap(fl_GetDC(0), w, h);
  driver(Fl_Graphics_Driver::newMainGraphicsDriver());
  if (d != 1 && high_res) ((Fl_GDI_Graphics_Driver*)driver())->scale(d);
  origin.x = origin.y = 0;
}


Fl_GDI_Image_Surface_Driver::~Fl_GDI_Image_Surface_Driver() {
  if (offscreen && !external_offscreen) DeleteObject((HBITMAP)offscreen);
  delete driver();
}


void Fl_GDI_Image_Surface_Driver::set_current() {
  HDC gc = fl_makeDC((HBITMAP)offscreen);
  driver()->gc(gc);
  SetWindowOrgEx(gc, origin.x, origin.y, NULL);
  Fl_Surface_Device::set_current();
  pre_window = fl_window;
  _savedc = SaveDC(gc);
  fl_window=(HWND)offscreen;
}


void Fl_GDI_Image_Surface_Driver::translate(int x, int y) {
  ((Fl_GDI_Graphics_Driver*)driver())->translate_all(x, y);
}


void Fl_GDI_Image_Surface_Driver::untranslate() {
  ((Fl_GDI_Graphics_Driver*)driver())->untranslate_all();
}


Fl_RGB_Image* Fl_GDI_Image_Surface_Driver::image()
{
  Fl_RGB_Image *image = Fl::screen_driver()->read_win_rectangle( 0, 0, width, height, 0);
  return image;
}


void Fl_GDI_Image_Surface_Driver::end_current()
{
  HDC gc = (HDC)driver()->gc();
  GetWindowOrgEx(gc, &origin);
  RestoreDC(gc, _savedc);
  DeleteDC(gc);
  fl_window = pre_window;
  Fl_Surface_Device::end_current();
}
