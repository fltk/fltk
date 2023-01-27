//
// Copy-to-clipboard code for the Fast Light Tool Kit (FLTK).
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
#include "Fl_GDI_Copy_Surface_Driver.H"
#include <FL/platform.H>
#include "Fl_GDI_Graphics_Driver.H"
#include "../WinAPI/Fl_WinAPI_Screen_Driver.H"
#include <FL/Fl_Image_Surface.H>
#include <windows.h>


Fl_GDI_Copy_Surface_Driver::Fl_GDI_Copy_Surface_Driver(int w, int h) : Fl_Copy_Surface_Driver(w, h) {
  driver(Fl_Graphics_Driver::newMainGraphicsDriver());
  oldgc = (HDC)Fl_Surface_Device::surface()->driver()->gc();
  // exact computation of factor from screen units to EnhMetaFile units (0.01 mm)
  HDC hdc = GetDC(NULL);
  int hmm = GetDeviceCaps(hdc, HORZSIZE);
  int hdots = GetDeviceCaps(hdc, HORZRES);
  int vmm = GetDeviceCaps(hdc, VERTSIZE);
  int vdots = GetDeviceCaps(hdc, VERTRES);
  ReleaseDC(NULL, hdc);
  float factorw = (100.f * hmm) / hdots;
  float factorh = (100.f * vmm) / vdots;
  // Global display scaling factor: 1, 1.25, 1.5, 1.75, etc...
  float scaling = Fl_Graphics_Driver::default_driver().scale();
  driver()->scale(scaling);
  RECT rect; rect.left = 0; rect.top = 0; rect.right = (LONG)((w*scaling) * factorw); rect.bottom = (LONG)((h*scaling) * factorh);
  gc = CreateEnhMetaFile (NULL, NULL, &rect, NULL);
  if (gc != NULL) {
    SetTextAlign(gc, TA_BASELINE|TA_LEFT);
    SetBkMode(gc, TRANSPARENT);
  }
}


Fl_GDI_Copy_Surface_Driver::~Fl_GDI_Copy_Surface_Driver() {
  if (oldgc == (HDC)Fl_Surface_Device::surface()->driver()->gc()) oldgc = NULL;
  HENHMETAFILE hmf = CloseEnhMetaFile (gc);
  if ( hmf != NULL ) {
    if ( OpenClipboard (NULL) ){
      EmptyClipboard ();
      // put first the vectorial form of the graphics in the clipboard
      SetClipboardData (CF_ENHMETAFILE, hmf);
      // then put a BITMAP version of the graphics in the clipboard
      float scaling = driver()->scale();
      int W = Fl_Scalable_Graphics_Driver::floor(width, scaling), H = Fl_Scalable_Graphics_Driver::floor(height, scaling);
      RECT rect = {0, 0, W, H};
      Fl_Image_Surface *surf = new Fl_Image_Surface(W, H);
      Fl_Surface_Device::push_current(surf);
      fl_color(FL_WHITE);    // draw white background
      fl_rectf(0, 0, W, H);
      PlayEnhMetaFile((HDC)surf->driver()->gc(), hmf, &rect); // draw metafile to offscreen buffer
      SetClipboardData(CF_BITMAP, (HBITMAP)surf->offscreen());
      Fl_Surface_Device::pop_current();
      delete surf;

      CloseClipboard ();
    }
    DeleteEnhMetaFile(hmf);
  }
  DeleteDC(gc);
  Fl_Surface_Device::surface()->driver()->gc(oldgc);
  delete driver();
}


void Fl_GDI_Copy_Surface_Driver::set_current() {
  driver()->gc(gc);
  fl_window = (HWND)1;
  Fl_Surface_Device::set_current();
}


void Fl_GDI_Copy_Surface_Driver::translate(int x, int y) {
  ((Fl_GDI_Graphics_Driver*)driver())->translate_all(x, y);
}


void Fl_GDI_Copy_Surface_Driver::untranslate() {
  ((Fl_GDI_Graphics_Driver*)driver())->untranslate_all();
}
