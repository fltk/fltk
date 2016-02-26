//
// "$Id$"
//
// Copy-to-clipboard code for the Fast Light Tool Kit (FLTK).
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

#include "config_lib.h"
#include <FL/Fl_Copy_Surface.H>
#include <FL/Fl.H>
#ifdef FL_CFG_GFX_QUARTZ
#include "drivers/Quartz/Fl_Quartz_Graphics_Driver.h"
#endif
#ifdef FL_CFG_GFX_XLIB
#include "drivers/Xlib/Fl_Translated_Xlib_Graphics_Driver.H"
#endif
#ifdef FL_CFG_GFX_GDI
#include "drivers/GDI/Fl_GDI_Graphics_Driver.h"
#endif


#if defined(__APPLE__) // PORTME: Fl_Surface_Driver - platform copy surface

#elif defined(WIN32)


#else

#endif


/** Constructor.
 \param w and \param h are the width and height of the clipboard surface
 in pixels where drawing will occur.
 */
Fl_Copy_Surface::Fl_Copy_Surface(int w, int h) :  Fl_Widget_Surface(NULL)
{
  width = w;
  height = h;
#ifdef __APPLE__ // PORTME: Fl_Surface_Driver - platform copy surface
  driver(new Fl_Quartz_Graphics_Driver);
  prepare_copy_pdf_and_tiff(w, h);
#elif defined(WIN32)
  driver(new Fl_Translated_GDI_Graphics_Driver);
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

  RECT rect; rect.left = 0; rect.top = 0; rect.right = (LONG)(w * factorw); rect.bottom = (LONG)(h * factorh);
  gc = CreateEnhMetaFile (NULL, NULL, &rect, NULL);
  if (gc != NULL) {
    SetTextAlign(gc, TA_BASELINE|TA_LEFT);
    SetBkMode(gc, TRANSPARENT);
  }
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: initialize members of Fl_Copy_Surface"
#else // Xlib
  driver(new Fl_Translated_Xlib_Graphics_Driver());
  Fl::first_window()->make_current();
  oldwindow = fl_xid(Fl::first_window());
  xid = fl_create_offscreen(w,h);
  Fl_Surface_Device *present_surface = Fl_Surface_Device::surface();
  set_current();
  fl_color(FL_WHITE);
  fl_rectf(0, 0, w, h);
  present_surface->set_current();
#endif
}

/** Destructor.
 */
Fl_Copy_Surface::~Fl_Copy_Surface()
{
#ifdef __APPLE__ // PORTME: Fl_Surface_Driver - platform copy surface
  complete_copy_pdf_and_tiff();
#elif defined(WIN32)
  if (oldgc == (HDC)Fl_Surface_Device::surface()->driver()->gc()) oldgc = NULL;
  HENHMETAFILE hmf = CloseEnhMetaFile (gc);
  if ( hmf != NULL ) {
    if ( OpenClipboard (NULL) ){
      EmptyClipboard ();
      SetClipboardData (CF_ENHMETAFILE, hmf);
      CloseClipboard ();
    }
    DeleteEnhMetaFile(hmf);
  }
  DeleteDC(gc);
  Fl_Surface_Device::surface()->driver()->gc(oldgc);
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: free resources in destructor of Fl_Copy_Surface"
#else // Xlib
  fl_pop_clip();
  unsigned char *data = fl_read_image(NULL,0,0,width,height,0);
  fl_window = oldwindow;
  _ss->set_current();
  Fl::copy_image(data,width,height,1);
  delete[] data;
  fl_delete_offscreen(xid);
#endif
}


void Fl_Copy_Surface::set_current()
{
#if defined(__APPLE__) || defined(WIN32) // PORTME: Fl_Surface_Driver - platform copy surface
  driver()->gc(gc);
  fl_window = (Window)1;
  Fl_Surface_Device::set_current();
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement Fl_Copy_Surface::set_current"
#else
  fl_window=xid;
  _ss = Fl_Surface_Device::surface();
  Fl_Surface_Device::set_current();
  fl_push_no_clip();
#endif
}


#if defined(__APPLE__) // PORTME: Fl_Surface_Driver - platform copy surface

size_t Fl_Copy_Surface::MyPutBytes(void* info, const void* buffer, size_t count)
  {
  CFDataAppendBytes ((CFMutableDataRef) info, (const UInt8 *)buffer, count);
  return count;
}

void Fl_Copy_Surface::init_PDF_context(int w, int h)
{
  CGRect bounds = CGRectMake(0, 0, w, h );
  pdfdata = CFDataCreateMutable(NULL, 0);
  CGDataConsumerRef myconsumer;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1040
  if (&CGDataConsumerCreateWithCFData != NULL) {
    myconsumer = CGDataConsumerCreateWithCFData(pdfdata); // 10.4
  }
  else
#endif
  {
    static CGDataConsumerCallbacks callbacks = { Fl_Copy_Surface::MyPutBytes, NULL };
    myconsumer = CGDataConsumerCreate ((void*) pdfdata, &callbacks);
  }
  gc = CGPDFContextCreate (myconsumer, &bounds, NULL);
  CGDataConsumerRelease (myconsumer);
}

void Fl_Copy_Surface::prepare_copy_pdf_and_tiff(int w, int h)
{
  init_PDF_context(w, h);
  if (gc == NULL) return;
  CGRect bounds = CGRectMake(0, 0, w, h );
  CGContextBeginPage (gc, &bounds);
  CGContextTranslateCTM(gc, 0, h);
  CGContextScaleCTM(gc, 1.0f, -1.0f);
  CGContextSaveGState(gc);
}

void Fl_Copy_Surface::translate(int x, int y) {
  CGContextRef gc = (CGContextRef)driver()->gc();
  CGContextRestoreGState(gc);
  CGContextSaveGState(gc);
  CGContextTranslateCTM(gc, x, y);
  CGContextSaveGState(gc);
}

void Fl_Copy_Surface::untranslate() {
  CGContextRestoreGState((CGContextRef)driver()->gc());
}

#elif defined(WIN32)

void Fl_Copy_Surface::translate(int x, int y) {
  ((Fl_Translated_GDI_Graphics_Driver*)driver())->translate_all(x, y);
}

void Fl_Copy_Surface::untranslate() {
  ((Fl_Translated_GDI_Graphics_Driver*)driver())->untranslate_all();
}

#else
void Fl_Copy_Surface::translate(int x, int y) {
  ((Fl_Translated_Xlib_Graphics_Driver*)driver())->translate_all(x, y);
}

void Fl_Copy_Surface::untranslate() {
  ((Fl_Translated_Xlib_Graphics_Driver*)driver())->untranslate_all();
}

#endif  // __APPLE__ // PORTME: Fl_Surface_Driver - platform copy surface

//
// End of "$Id$".
//
