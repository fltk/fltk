//
// "$Id$"
//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
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


#include <config.h>
#include "../../config_lib.h"
#include "Fl_GDI_Graphics_Driver.h"


const char *Fl_GDI_Graphics_Driver::class_id = "Fl_GDI_Graphics_Driver";

// FIXME: move to printer graphics driver
const char *Fl_GDI_Printer_Graphics_Driver::class_id = "Fl_GDI_Printer_Graphics_Driver";

/*
 * By linking this module, the following static method will instatiate the
 * MSWindows GDI Graphics driver as the main display driver.
 */
Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
  return new Fl_GDI_Graphics_Driver();
}

// Code used to switch output to an off-screen window.  See macros in
// win32.H which save the old state in local variables.

typedef struct { BYTE a; BYTE b; BYTE c; BYTE d; } FL_BLENDFUNCTION;
typedef BOOL (WINAPI* fl_alpha_blend_func)
(HDC,int,int,int,int,HDC,int,int,int,int,FL_BLENDFUNCTION);
static fl_alpha_blend_func fl_alpha_blend = NULL;
static FL_BLENDFUNCTION blendfunc = { 0, 0, 255, 1};

/*
 * This function checks if the version of MSWindows that we
 * curently run on supports alpha blending for bitmap transfers
 * and finds the required function if so.
 */
char Fl_GDI_Graphics_Driver::can_do_alpha_blending() {
  static char been_here = 0;
  static char can_do = 0;
  // do this test only once
  if (been_here) return can_do;
  been_here = 1;
  // load the library that implements alpha blending
  HMODULE hMod = LoadLibrary("MSIMG32.DLL");
  // give up if that doesn't exist (Win95?)
  if (!hMod) return 0;
  // now find the blending function inside that dll
  fl_alpha_blend = (fl_alpha_blend_func)GetProcAddress(hMod, "AlphaBlend");
  // give up if we can't find it (Win95)
  if (!fl_alpha_blend) return 0;
  // we have the call, but does our display support alpha blending?
  // get the desktop's device context
  HDC dc = GetDC(0L);
  if (!dc) return 0;
  // check the device capabilities flags. However GetDeviceCaps
  // does not return anything useful, so we have to do it manually:

  HBITMAP bm = CreateCompatibleBitmap(dc, 1, 1);
  HDC new_gc = CreateCompatibleDC(dc);
  int save = SaveDC(new_gc);
  SelectObject(new_gc, bm);
  /*COLORREF set = */ SetPixel(new_gc, 0, 0, 0x01010101);
  BOOL alpha_ok = fl_alpha_blend(dc, 0, 0, 1, 1, new_gc, 0, 0, 1, 1, blendfunc);
  RestoreDC(new_gc, save);
  DeleteDC(new_gc);
  DeleteObject(bm);
  ReleaseDC(0L, dc);

  if (alpha_ok) can_do = 1;
  return can_do;
}

HDC fl_makeDC(HBITMAP bitmap) {
  HDC new_gc = CreateCompatibleDC(fl_gc);
  SetTextAlign(new_gc, TA_BASELINE|TA_LEFT);
  SetBkMode(new_gc, TRANSPARENT);
#if USE_COLORMAP
  if (fl_palette) SelectPalette(new_gc, fl_palette, FALSE);
#endif
  SelectObject(new_gc, bitmap);
  return new_gc;
}

void Fl_GDI_Graphics_Driver::copy_offscreen(int x,int y,int w,int h,HBITMAP bitmap,int srcx,int srcy) {
  HDC new_gc = CreateCompatibleDC(fl_gc);
  int save = SaveDC(new_gc);
  SelectObject(new_gc, bitmap);
  BitBlt(fl_gc, x, y, w, h, new_gc, srcx, srcy, SRCCOPY);
  RestoreDC(new_gc, save);
  DeleteDC(new_gc);
}

void Fl_GDI_Graphics_Driver::copy_offscreen_with_alpha(int x,int y,int w,int h,HBITMAP bitmap,int srcx,int srcy) {
  HDC new_gc = CreateCompatibleDC(fl_gc);
  int save = SaveDC(new_gc);
  SelectObject(new_gc, bitmap);
  BOOL alpha_ok = 0;
  // first try to alpha blend
  if ( can_do_alpha_blending() ) {
    alpha_ok = fl_alpha_blend(fl_gc, x, y, w, h, new_gc, srcx, srcy, w, h, blendfunc);
  }
  // if that failed (it shouldn't), still copy the bitmap over, but now alpha is 1
  if (!alpha_ok) {
    BitBlt(fl_gc, x, y, w, h, new_gc, srcx, srcy, SRCCOPY);
  }
  RestoreDC(new_gc, save);
  DeleteDC(new_gc);
}

//
// End of "$Id$".
//
