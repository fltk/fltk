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
#include <FL/Fl_Bitmap.H>
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
  shape_data_ = NULL;
}


Fl_GDI_Image_Surface_Driver::~Fl_GDI_Image_Surface_Driver() {
  if (shape_data_ && shape_data_->background) {
    DeleteObject(shape_data_->background);
    delete shape_data_->mask;
    free(shape_data_);
  }
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
  if (shape_data_ && shape_data_->background) {
    // get the offscreen size in pixels
    HDC gc = fl_makeDC((HBITMAP)offscreen);
    BITMAPINFO bmi;
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 0;
    bmi.bmiHeader.biSizeImage = 0;
    GetDIBits(gc, (HBITMAP)offscreen, 0, 0, NULL, &bmi, DIB_RGB_COLORS);
    int W = bmi.bmiHeader.biWidth;
    int H = bmi.bmiHeader.biHeight;
    int line_size = ((3*W+3)/4) * 4;

    // read bits of main offscreen
    uchar *dib_src = new uchar[line_size * H];
    bmi.bmiHeader.biWidth = W;
    bmi.bmiHeader.biHeight = H;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biBitCount = 24;
    GetDIBits(gc, (HBITMAP)offscreen, 0, H,
                       dib_src, &bmi, DIB_RGB_COLORS);

    // draw above the secondary offscreen the main offscreen masked by shape_data_->mask
    GdiFlush();
    Fl_Image_Surface_Driver::copy_with_mask(shape_data_->mask, shape_data_->vBits, dib_src, ((3*W+3)/4) * 4, true);
    delete shape_data_->mask;
    delete[] dib_src;

    // write bits of main offscreen
    SetDIBits(gc, (HBITMAP)offscreen, 0, H, shape_data_->vBits, &bmi, DIB_RGB_COLORS);
    DeleteDC(gc);
    DeleteObject(shape_data_->background);
    shape_data_->background = NULL;
    free(shape_data_);
    shape_data_ = NULL;
   }
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


void Fl_GDI_Image_Surface_Driver::mask(const Fl_RGB_Image *mask) {
  shape_data_ =  (struct shape_data_type*)calloc(1, sizeof(struct shape_data_type));
  // get the offscreen size in pixels
  HDC gc = fl_makeDC((HBITMAP)offscreen);
  BITMAPINFO bmi;
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 0;
  bmi.bmiHeader.biSizeImage = 0;

  GetDIBits(gc, (HBITMAP)offscreen, 0, 0, NULL, &bmi, DIB_RGB_COLORS);
  int W = bmi.bmiHeader.biWidth;
  int H = bmi.bmiHeader.biHeight;

  shape_data_->mask = Fl_Image_Surface_Driver::RGB3_to_RGB1(mask, W, H);

  // duplicate current offscreen content to new offscreen
  int line_size = ((3*W+3)/4) * 4;
  uchar *dib = new uchar[line_size * H];  // create temporary buffer to read DIB
  bmi.bmiHeader.biWidth = W;
  bmi.bmiHeader.biHeight = H;
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biBitCount = 24;

  GetDIBits(gc, (HBITMAP)offscreen, 0, H, dib, &bmi, DIB_RGB_COLORS);

  HDC background_gc = CreateCompatibleDC(gc);
  shape_data_->background =
    CreateDIBSection(background_gc, &bmi, DIB_RGB_COLORS,
                     (void**)&shape_data_->vBits, NULL, 0);
  if (!shape_data_->background) {
    Fl::error("CreateDIBSection error=%lu", GetLastError());
  }
  memcpy(shape_data_->vBits, dib, H * line_size);
  delete[] dib;
  DeleteDC(background_gc);
  DeleteDC(gc);
}

