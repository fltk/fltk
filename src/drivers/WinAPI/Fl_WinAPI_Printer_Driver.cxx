//
// "$Id$"
//
// Printing support for Windows for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2018 by Bill Spitzak and others.
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

#ifdef _WIN32

#include "../GDI/Fl_GDI_Graphics_Driver.H"
#include <FL/Fl_Printer.H>
#include <FL/fl_ask.H>
#include <FL/math.h>
#include <FL/fl_draw.H>
#include <commdlg.h>

extern HWND fl_window;

/** Support for printing on the Windows platform */
class Fl_WinAPI_Printer_Driver : public Fl_Paged_Device {
  friend class Fl_Paged_Device;
private:
  int   abortPrint;
  PRINTDLG      pd;
  HDC           hPr;
  int           prerr;
  int left_margin;
  int top_margin;
  void absolute_printable_rect(int *x, int *y, int *w, int *h);
  Fl_WinAPI_Printer_Driver(void);
  int start_job(int pagecount, int *frompage = NULL, int *topage = NULL);
  int start_page (void);
  int printable_rect(int *w, int *h);
  void margins(int *left, int *top, int *right, int *bottom);
  void origin(int *x, int *y);
  void origin(int x, int y);
  void scale (float scale_x, float scale_y = 0.);
  void rotate(float angle);
  void translate(int x, int y);
  void untranslate(void);
  int end_page (void);
  void end_job (void);
  ~Fl_WinAPI_Printer_Driver(void);
};

Fl_WinAPI_Printer_Driver::Fl_WinAPI_Printer_Driver(void) : Fl_Paged_Device() {
  hPr = NULL;
  driver(new Fl_GDI_Printer_Graphics_Driver);
}

Fl_Paged_Device* Fl_Paged_Device::newPrinterDriver(void)
{
  return new Fl_WinAPI_Printer_Driver();
}


Fl_WinAPI_Printer_Driver::~Fl_WinAPI_Printer_Driver(void) {
  if (hPr) end_job();
  delete driver();
}

static void WIN_SetupPrinterDeviceContext(HDC prHDC)
{
  if ( !prHDC ) return;
  
  fl_window = 0;
  SetGraphicsMode(prHDC, GM_ADVANCED); // to allow for rotations
  SetMapMode(prHDC, MM_ANISOTROPIC);
  SetTextAlign(prHDC, TA_BASELINE|TA_LEFT);
  SetBkMode(prHDC, TRANSPARENT);	
  // this matches 720 logical units to the number of device units in 10 inches of paper
  // thus the logical unit is the point (= 1/72 inch)
  SetWindowExtEx(prHDC, 720, 720, NULL);
  SetViewportExtEx(prHDC, 10*GetDeviceCaps(prHDC, LOGPIXELSX), 10*GetDeviceCaps(prHDC, LOGPIXELSY), NULL);
}


int Fl_WinAPI_Printer_Driver::start_job (int pagecount, int *frompage, int *topage)
// returns 0 iff OK
{
  if (pagecount == 0) pagecount = 10000;
  DWORD       commdlgerr;
  DOCINFO     di;
  char        docName [256];
  int err = 0;
  
  abortPrint = FALSE;
  memset (&pd, 0, sizeof (PRINTDLG));
  pd.lStructSize = sizeof (PRINTDLG);
  pd.hwndOwner = GetForegroundWindow();
  pd.Flags = PD_RETURNDC | PD_USEDEVMODECOPIESANDCOLLATE | PD_NOSELECTION;
  pd.nMinPage = 1;
  pd.nMaxPage = pagecount;
  BOOL b = PrintDlg (&pd);
  if (pd.hwndOwner) { // restore the correct state of mouse buttons and keyboard modifier keys (STR #3221)
    WNDPROC windproc = (WNDPROC)GetWindowLongPtrW(pd.hwndOwner, GWLP_WNDPROC);
    CallWindowProc(windproc, pd.hwndOwner, WM_ACTIVATEAPP, 1, 0);
  }
  if (b != 0) {
    hPr = pd.hDC;
    if (hPr != NULL) {
      strcpy (docName, "FLTK");
      memset(&di, 0, sizeof(DOCINFO));
      di.cbSize = sizeof (DOCINFO);
      di.lpszDocName = (LPCSTR) docName;
      prerr = StartDoc (hPr, &di);
      if (prerr < 1) {
        abortPrint = TRUE;
        //fl_alert ("StartDoc error %d", prerr);
        err = 1;
      }
    } else {
      commdlgerr = CommDlgExtendedError ();
      fl_alert ("Unable to create print context, error %lu",
                (unsigned long) commdlgerr);
      err = 1;
    }
  } else {
    err = 1;
  }
  if(!err) {
    if((pd.Flags & PD_PAGENUMS) != 0 ) {
      if (frompage) *frompage = pd.nFromPage;
      if (topage) *topage = pd.nToPage;
    }
    else {
      if (frompage) *frompage = 1;
      if (topage) *topage = pagecount;
    }
    x_offset = 0;
    y_offset = 0;
    WIN_SetupPrinterDeviceContext (hPr);
    driver()->gc(hPr);
    this->set_current();
  }
  return err;
}

void Fl_WinAPI_Printer_Driver::end_job (void)
{
  Fl_Display_Device::display_device()->set_current();
  if (hPr != NULL) {
    if (! abortPrint) {
      prerr = EndDoc (hPr);
      if (prerr < 0) {
	fl_alert ("EndDoc error %d", prerr);
      }
    }
    DeleteDC (hPr);
    if (pd.hDevMode != NULL) {
      GlobalFree (pd.hDevMode);
    }
    if (pd.hDevNames != NULL) {
      GlobalFree (pd.hDevNames);
    }
  }
  hPr = NULL;
}

void Fl_WinAPI_Printer_Driver::absolute_printable_rect(int *x, int *y, int *w, int *h)
{
  POINT         physPageSize;
  POINT         pixelsPerInch;
  XFORM		transform;
    
  if (hPr == NULL) return;
  HDC gc = (HDC)driver()->gc();
  GetWorldTransform(gc, &transform);
  ModifyWorldTransform(gc, NULL, MWT_IDENTITY);
  SetWindowOrgEx(gc, 0, 0, NULL);
  
  physPageSize.x = GetDeviceCaps(hPr, HORZRES);
  physPageSize.y = GetDeviceCaps(hPr, VERTRES);
  DPtoLP(hPr, &physPageSize, 1);
  *w = physPageSize.x + 1;
  *h = physPageSize.y + 1;
  pixelsPerInch.x = GetDeviceCaps(hPr, LOGPIXELSX);
  pixelsPerInch.y = GetDeviceCaps(hPr, LOGPIXELSY);
  DPtoLP(hPr, &pixelsPerInch, 1);
  left_margin = (pixelsPerInch.x / 4);
  *w -= (pixelsPerInch.x / 2);
  top_margin = (pixelsPerInch.y / 4);
  *h -= (pixelsPerInch.y / 2);
  
  *x = left_margin;
  *y = top_margin;
  origin(x_offset, y_offset);
  SetWorldTransform(gc, &transform);
}

void Fl_WinAPI_Printer_Driver::margins(int *left, int *top, int *right, int *bottom)
{
  int x = 0, y = 0, w = 0, h = 0;
  absolute_printable_rect(&x, &y, &w, &h);
  if (left) *left = x;
  if (top) *top = y;
  if (right) *right = x;
  if (bottom) *bottom = y;
}

int Fl_WinAPI_Printer_Driver::printable_rect(int *w, int *h)
{
  int x, y;
  absolute_printable_rect(&x, &y, w, h);
  return 0;
}

int Fl_WinAPI_Printer_Driver::start_page (void)
{
  int  rsult, w, h;
  
  rsult = 0;
  if (hPr != NULL) {
    WIN_SetupPrinterDeviceContext (hPr);
    prerr = StartPage (hPr);
    if (prerr < 0) {
      fl_alert ("StartPage error %d", prerr);
      rsult = 1;
    }
    printable_rect(&w, &h);
    origin(0, 0);
    fl_clip_region(0);
  }
  return rsult;
}

void Fl_WinAPI_Printer_Driver::origin (int deltax, int deltay)
{
  SetWindowOrgEx( (HDC)driver()->gc(), - left_margin - deltax, - top_margin - deltay, NULL);
  x_offset = deltax;
  y_offset = deltay;
}

void Fl_WinAPI_Printer_Driver::scale (float scalex, float scaley)
{
  if (scaley == 0.) scaley = scalex;
  int w, h;
  SetWindowExtEx((HDC)driver()->gc(), (int)(720 / scalex + 0.5), (int)(720 / scaley + 0.5), NULL);
  printable_rect(&w, &h);
  origin(0, 0);
}

void Fl_WinAPI_Printer_Driver::rotate (float rot_angle)
{
  XFORM mat;
  float angle;
  angle = (float) - (rot_angle * M_PI / 180.);
  mat.eM11 = (float)cos(angle);
  mat.eM12 = (float)sin(angle);
  mat.eM21 = - mat.eM12;
  mat.eM22 = mat.eM11;
  mat.eDx = mat.eDy = 0;
  SetWorldTransform((HDC)driver()->gc(), &mat);
}

int Fl_WinAPI_Printer_Driver::end_page (void)
{
  int  rsult;
  
  rsult = 0;
  if (hPr != NULL) {
    prerr = EndPage (hPr);
    if (prerr < 0) {
      abortPrint = TRUE;
      fl_alert ("EndPage error %d", prerr);
      rsult = 1;
    }
  }
  return rsult;
}

static int translate_stack_depth = 0;
const int translate_stack_max = 5;
static int translate_stack_x[translate_stack_max];
static int translate_stack_y[translate_stack_max];

static void do_translate(int x, int y, HDC gc)
{
  XFORM tr;
  tr.eM11 = tr.eM22 = 1;
  tr.eM12 = tr.eM21 = 0;
  tr.eDx =  (FLOAT) x;
  tr.eDy =  (FLOAT) y;
  ModifyWorldTransform(gc, &tr, MWT_LEFTMULTIPLY);
}

void Fl_WinAPI_Printer_Driver::translate (int x, int y)
{
  do_translate(x, y, (HDC)driver()->gc());
  if (translate_stack_depth < translate_stack_max) {
    translate_stack_x[translate_stack_depth] = x;
    translate_stack_y[translate_stack_depth] = y;
    translate_stack_depth++;
    }
}

void Fl_WinAPI_Printer_Driver::untranslate (void)
{
  if (translate_stack_depth > 0) {
    translate_stack_depth--;
    do_translate( - translate_stack_x[translate_stack_depth], - translate_stack_y[translate_stack_depth], (HDC)driver()->gc() );
    }
}

void Fl_WinAPI_Printer_Driver::origin(int *x, int *y)
{
  Fl_Paged_Device::origin(x, y);
}

#endif // _WIN32

//
// End of "$Id$".
//
