//
// "$Id$"
//
// Mac OS X-specific printing support (objective-c++) for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl_Printer.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Window_Driver.H>
#include "../Quartz/Fl_Quartz_Printer_Graphics_Driver.H"

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#import <Cocoa/Cocoa.h>

typedef OSStatus (*PMSessionSetDocumentFormatGeneration_type)(
                                     PMPrintSession   printSession,
                                     CFStringRef      docFormat,
                                     CFArrayRef       graphicsContextTypes,
                                     CFTypeRef        options);
typedef OSStatus (*PMSessionBeginDocumentNoDialog_type)(
                               PMPrintSession    printSession,
                               PMPrintSettings   printSettings,
                               PMPageFormat      pageFormat);
typedef OSStatus
(*PMSessionGetGraphicsContext_type)(
                            PMPrintSession   printSession,
                            CFStringRef      graphicsContextType,
                            void **          graphicsContext);

extern void fl_quartz_restore_line_style_(CGContextRef gc);


/** Support for printing on the Apple OS X platform */
class Fl_Cocoa_Printer_Driver : public Fl_Paged_Device {
  friend class Fl_Paged_Device;
private:
  float scale_x;
  float scale_y;
  float angle; // rotation angle in radians
  PMPrintSession  printSession;
  PMPageFormat    pageFormat;
  PMPrintSettings printSettings;
  Fl_Cocoa_Printer_Driver(void);
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
  void draw_decorated_window(Fl_Window *win, int x_offset, int y_offset);
  void print_window_part(Fl_Window *win, int x, int y, int w, int h, int delta_x, int delta_y);
  ~Fl_Cocoa_Printer_Driver(void);
};


Fl_Cocoa_Printer_Driver::Fl_Cocoa_Printer_Driver(void)
{
  x_offset = 0;
  y_offset = 0;
  scale_x = scale_y = 1.;
  driver(new Fl_Quartz_Printer_Graphics_Driver);
}

Fl_Paged_Device* Fl_Paged_Device::newPrinterDriver(void)
{
  return new Fl_Cocoa_Printer_Driver();
}

Fl_Cocoa_Printer_Driver::~Fl_Cocoa_Printer_Driver(void) {
  delete driver();
}


int Fl_Cocoa_Printer_Driver::start_job (int pagecount, int *frompage, int *topage)
//printing using a Quartz graphics context
//returns 0 iff OK
{
  OSStatus status = 0;
  fl_open_display();
  Fl_X::q_release_context();
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
  if (fl_mac_os_version >= 100500) {
    NSPrintInfo *info = [NSPrintInfo sharedPrintInfo];
    NSPrintPanel *panel = [NSPrintPanel printPanel];
    //from 10.5
    [panel setOptions:NSPrintPanelShowsCopies | NSPrintPanelShowsPageRange | NSPrintPanelShowsPageSetupAccessory];
    NSInteger retval = [panel runModalWithPrintInfo:info];//from 10.5
    if(retval != NSOKButton) {
      Fl_Window *w = Fl::first_window();
      if (w) w->show();
      return 1;
    }
    printSession = (PMPrintSession)[info PMPrintSession];//from 10.5
    pageFormat = (PMPageFormat)[info PMPageFormat];//from 10.5
    printSettings = (PMPrintSettings)[info PMPrintSettings];//from 10.5
    UInt32 from32, to32;
    PMGetFirstPage(printSettings, &from32); 
    if (frompage) *frompage = (int)from32;
    PMGetLastPage(printSettings, &to32); 
    if (topage) {
      *topage = (int)to32;
      if (*topage > pagecount && pagecount > 0) *topage = pagecount;
    }
    status = PMSessionBeginCGDocumentNoDialog(printSession, printSettings, pageFormat);//from 10.4
  }
  else
#endif
  {
#if !__LP64__
    Boolean accepted;
    status = PMCreateSession(&printSession);
    if (status != noErr) return 1;
    status = PMCreatePageFormat(&pageFormat);
    status = PMSessionDefaultPageFormat(printSession, pageFormat);
    if (status != noErr) return 1;
    // get pointer to the PMSessionPageSetupDialog Carbon function
    typedef OSStatus (*dialog_f)(PMPrintSession, PMPageFormat, Boolean *);
    static dialog_f f = NULL;
    if (!f) f = (dialog_f)Fl_X::get_carbon_function("PMSessionPageSetupDialog");
    status = (*f)(printSession, pageFormat, &accepted);
    if (status != noErr || !accepted) {
      Fl::first_window()->show();
      return 1;
    }
    status = PMCreatePrintSettings(&printSettings);
    if (status != noErr || printSettings == kPMNoPrintSettings) return 1;
    status = PMSessionDefaultPrintSettings (printSession, printSettings);
    if (status != noErr) return 1;
    PMSetPageRange(printSettings, 1, (UInt32)kPMPrintAllPages);
    // get pointer to the PMSessionPrintDialog Carbon function
    typedef OSStatus (*dialog_f2)(PMPrintSession, PMPrintSettings, PMPageFormat, Boolean *);
    static dialog_f2 f2 = NULL;
    if (!f2) f2 = (dialog_f2)Fl_X::get_carbon_function("PMSessionPrintDialog");
    status = (*f2)(printSession, printSettings, pageFormat, &accepted);
    if (!accepted) status = kPMCancel;
    if (status != noErr) {
      Fl::first_window()->show();
      return 1;
    }
    UInt32 from32, to32;
    PMGetFirstPage(printSettings, &from32); 
    if (frompage) *frompage = (int)from32;
    PMGetLastPage(printSettings, &to32); 
    if (topage) *topage = (int)to32;
    if(topage && *topage > pagecount) *topage = pagecount;
    CFStringRef mystring[1];
    mystring[0] = kPMGraphicsContextCoreGraphics;
    CFArrayRef array = CFArrayCreate(NULL, (const void **)mystring, 1, &kCFTypeArrayCallBacks);
    PMSessionSetDocumentFormatGeneration_type PMSessionSetDocumentFormatGeneration =
      (PMSessionSetDocumentFormatGeneration_type)Fl_X::get_carbon_function("PMSessionSetDocumentFormatGeneration");
    status = PMSessionSetDocumentFormatGeneration(printSession, kPMDocumentFormatDefault, array, NULL);
    CFRelease(array);
    PMSessionBeginDocumentNoDialog_type PMSessionBeginDocumentNoDialog =
      (PMSessionBeginDocumentNoDialog_type)Fl_X::get_carbon_function("PMSessionBeginDocumentNoDialog");
    status = PMSessionBeginDocumentNoDialog(printSession, printSettings, pageFormat);
#endif //__LP64__
  }

  if (status != noErr) return 1;
  y_offset = x_offset = 0;
  this->set_current();
  return 0;
}

void Fl_Cocoa_Printer_Driver::margins(int *left, int *top, int *right, int *bottom)
{
  PMPaper paper;
  PMGetPageFormatPaper(pageFormat, &paper);
  PMOrientation orientation;
  PMGetOrientation(pageFormat, &orientation);
  PMPaperMargins margins;
  PMPaperGetMargins(paper, &margins);
  if(orientation == kPMPortrait) {
    if (left) *left = (int)(margins.left / scale_x + 0.5);
    if (top) *top = (int)(margins.top / scale_y + 0.5);
    if (right) *right = (int)(margins.right / scale_x + 0.5);
    if (bottom) *bottom = (int)(margins.bottom / scale_y + 0.5);
    }
  else {
    if (left) *left = (int)(margins.top / scale_x + 0.5);
    if (top) *top = (int)(margins.left / scale_y + 0.5);
    if (right) *right = (int)(margins.bottom / scale_x + 0.5);
    if (bottom) *bottom = (int)(margins.right / scale_y + 0.5);
  }
}

int Fl_Cocoa_Printer_Driver::printable_rect(int *w, int *h)
//returns 0 iff OK
{
  OSStatus status;
  PMRect pmRect;
  int x, y;
  
  status = PMGetAdjustedPageRect(pageFormat, &pmRect);
  if (status != noErr) return 1;
  
  x = (int)pmRect.left;
  y = (int)pmRect.top;
  *w = int((int)(pmRect.right - x) / scale_x + 1);
  *h = int((int)(pmRect.bottom - y) / scale_y + 1);
  return 0;
}

void Fl_Cocoa_Printer_Driver::origin(int x, int y)
{
  x_offset = x;
  y_offset = y;
  CGContextRef gc = (CGContextRef)driver()->gc();
  CGContextRestoreGState(gc);
  CGContextRestoreGState(gc);
  CGContextSaveGState(gc);
  CGContextScaleCTM(gc, scale_x, scale_y);
  CGContextTranslateCTM(gc, x, y);
  CGContextRotateCTM(gc, angle);
  CGContextSaveGState(gc);
}

void Fl_Cocoa_Printer_Driver::scale (float s_x, float s_y)
{
  if (s_y == 0.) s_y = s_x;
  scale_x = s_x;
  scale_y = s_y;
  CGContextRef gc = (CGContextRef)driver()->gc();
  CGContextRestoreGState(gc);
  CGContextRestoreGState(gc);
  CGContextSaveGState(gc);
  CGContextScaleCTM(gc, scale_x, scale_y);
  CGContextRotateCTM(gc, angle);
  x_offset = y_offset = 0;
  CGContextSaveGState(gc);
}

void Fl_Cocoa_Printer_Driver::rotate (float rot_angle)
{
  angle = - rot_angle * M_PI / 180.;
  CGContextRef gc = (CGContextRef)driver()->gc();
  CGContextRestoreGState(gc);
  CGContextRestoreGState(gc);
  CGContextSaveGState(gc);
  CGContextScaleCTM(gc, scale_x, scale_y);
  CGContextTranslateCTM(gc, x_offset, y_offset);
  CGContextRotateCTM(gc, angle);
  CGContextSaveGState(gc);
}

void Fl_Cocoa_Printer_Driver::translate(int x, int y)
{
  CGContextRef gc = (CGContextRef)driver()->gc();
  CGContextSaveGState(gc);
  CGContextTranslateCTM(gc, x, y );
  CGContextSaveGState(gc);
}

void Fl_Cocoa_Printer_Driver::untranslate(void)
{
  CGContextRef gc = (CGContextRef)driver()->gc();
  CGContextRestoreGState(gc);
  CGContextRestoreGState(gc);
}

int Fl_Cocoa_Printer_Driver::start_page (void)
{	
  OSStatus status = PMSessionBeginPageNoDialog(printSession, pageFormat, NULL);
  CGContextRef gc;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if ( &PMSessionGetCGGraphicsContext != NULL ) {
    status = PMSessionGetCGGraphicsContext(printSession, &gc);
  }
  else
#endif
  {
#if ! __LP64_
    PMSessionGetGraphicsContext_type PMSessionGetGraphicsContext =
      (PMSessionGetGraphicsContext_type)Fl_X::get_carbon_function("PMSessionGetGraphicsContext");
    status = PMSessionGetGraphicsContext(printSession, NULL, (void **)&gc);
#endif
  }
  driver()->gc(gc);
  set_current();
  PMRect pmRect;
  float win_scale_x, win_scale_y;

  PMPaper paper;
  PMGetPageFormatPaper(pageFormat, &paper);
  PMPaperMargins margins;
  PMPaperGetMargins(paper, &margins);
  PMOrientation orientation;
  PMGetOrientation(pageFormat, &orientation);
  
  status = PMGetAdjustedPageRect(pageFormat, &pmRect);
  double h = pmRect.bottom - pmRect.top;
  x_offset = 0;
  y_offset = 0; 
  angle = 0;
  scale_x = scale_y = 1;
  win_scale_x = win_scale_y = 1;
  if(orientation == kPMPortrait)
    CGContextTranslateCTM(gc, margins.left, margins.bottom + h);
  else
    CGContextTranslateCTM(gc, margins.top, margins.right + h);
  CGContextScaleCTM(gc, win_scale_x, - win_scale_y);
  fl_quartz_restore_line_style_(gc);
  CGContextSetShouldAntialias(gc, false);
  CGContextSaveGState(gc);
  CGContextSaveGState(gc);
  fl_line_style(FL_SOLID);
  fl_window = (Window)1; // TODO: something better
  fl_clip_region(0);
  return status != noErr;
}

int Fl_Cocoa_Printer_Driver::end_page (void)
{	
  CGContextRef gc = (CGContextRef)driver()->gc();
  CGContextFlush(gc);
  CGContextRestoreGState(gc);
  CGContextRestoreGState(gc);
  OSStatus status = PMSessionEndPageNoDialog(printSession);
  gc = NULL;
  return status != noErr;
}

void Fl_Cocoa_Printer_Driver::end_job (void)
{
  OSStatus status;
  
  status = PMSessionError(printSession);
  if (status != noErr) {
    fl_alert ("PM Session error %d", (int)status);
  }
  PMSessionEndDocumentNoDialog(printSession);
#if !__LP64__
  if (fl_mac_os_version < 100500) {
    PMRelease(printSettings);
    PMRelease(pageFormat);
    PMRelease(printSession);
    }
#endif
  Fl_Display_Device::display_device()->set_current();
  driver()->gc(0);
  Fl_Window *w = Fl::first_window();
  if (w) w->show();
}

// version that prints at high res if using a retina display
void Fl_Cocoa_Printer_Driver::print_window_part(Fl_Window *win, int x, int y, int w, int h, int delta_x, int delta_y)
{
  Fl_Surface_Device *current = Fl_Surface_Device::surface();
  Fl_Display_Device::display_device()->set_current();
  Fl_Window *save_front = Fl::first_window();
  win->show();
  Fl::check();
  CGImageRef img = Fl_X::CGImage_from_window_rect(win, x, y, w, h);
  if (save_front != win) save_front->show();
  current->set_current();
  ((Fl_Quartz_Graphics_Driver*)driver())->draw_CGImage(img,delta_x, delta_y, w, h, 0,0,w,h);
  CFRelease(img);
}

void Fl_Cocoa_Printer_Driver::origin(int *x, int *y)
{
  Fl_Paged_Device::origin(x, y);
}

void Fl_Cocoa_Printer_Driver::draw_decorated_window(Fl_Window *win, int x_offset, int y_offset)
{
  if (!win->shown() || win->parent() || !win->border() || !win->visible()) {
    this->print_widget(win, x_offset, y_offset);
    return;
  }
  int bt = win->decorated_h() - win->h();
  BOOL to_quartz =  (this->driver()->has_feature(Fl_Graphics_Driver::NATIVE));
  CALayer *layer = nil;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
  if (fl_mac_os_version >= 101000) layer = [[[fl_xid(win) standardWindowButton:NSWindowCloseButton] superview] layer];
#endif
  
  if (layer) { // if title bar uses a layer
    if (to_quartz) { // to Quartz printer
      CGContextRef gc = (CGContextRef)driver()->gc();
      CGContextSaveGState(gc);
      CGContextTranslateCTM(gc, x_offset - 0.5, y_offset + bt - 0.5);
      CGContextScaleCTM(gc, 1, -1);
      Fl_X::draw_layer_to_context(layer, gc, win->w(), bt);
      CGContextRestoreGState(gc);
    }
    else {
      CGColorSpaceRef cspace = CGColorSpaceCreateDeviceRGB ();
      CGContextRef gc = CGBitmapContextCreate(NULL, 2*win->w(), 2*bt, 8, 0, cspace, kCGImageAlphaPremultipliedLast);
      CGColorSpaceRelease(cspace);
      CGContextScaleCTM(gc, 2, 2);
      Fl_X::draw_layer_to_context(layer, gc, win->w(), bt);
      Fl_RGB_Image *image = new Fl_RGB_Image((const uchar*)CGBitmapContextGetData(gc), 2*win->w(), 2*bt, 4,
                                             CGBitmapContextGetBytesPerRow(gc)); // 10.2
      int ori_x, ori_y;
      origin(&ori_x, &ori_y);
      scale(0.5);
      origin(2*ori_x, 2*ori_y);
      image->draw(2*x_offset, 2*y_offset); // draw title bar as double resolution image
      scale(1);
      origin(ori_x, ori_y);
      delete image;
      CGContextRelease(gc);
    }
    this->print_widget(win, x_offset, y_offset + bt);
    return;
  }
  Fl_Display_Device::display_device()->set_current(); // send win to front and make it current
  NSString *title = [fl_xid(win) title];
  [title retain];
  [fl_xid(win) setTitle:@""]; // temporarily set a void window title
  win->show();
  Fl::check();
  // capture the window title bar with no title
  Fl_Shared_Image *top, *left, *bottom, *right;
  win->driver()->capture_titlebar_and_borders(top, left, bottom, right);
  [fl_xid(win) setTitle:title]; // put back the window title
  this->set_current(); // back to the Fl_Paged_Device
  top->draw(x_offset, y_offset); // print the title bar
  top->release();
  if (win->label()) { // print the window title
    const int skip = 65; // approx width of the zone of the 3 window control buttons
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
    if (fl_mac_os_version >= 100400 && to_quartz) { // use Cocoa string drawing with exact title bar font
      // the exact font is LucidaGrande 13 pts (and HelveticaNeueDeskInterface-Regular with 10.10)
      NSGraphicsContext *current = [NSGraphicsContext currentContext];
      [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithGraphicsPort:driver()->gc() flipped:YES]];//10.4
      NSDictionary *attr = [NSDictionary dictionaryWithObject:[NSFont titleBarFontOfSize:0]
                                                       forKey:NSFontAttributeName];
      NSSize size = [title sizeWithAttributes:attr];
      int x = x_offset + win->w()/2 - size.width/2;
      if (x < x_offset+skip) x = x_offset+skip;
      NSRect r = NSMakeRect(x, y_offset+bt/2+4, win->w() - skip, bt);
      [[NSGraphicsContext currentContext] setShouldAntialias:YES];
      [title drawWithRect:r options:(NSStringDrawingOptions)0 attributes:attr]; // 10.4
      [[NSGraphicsContext currentContext] setShouldAntialias:NO];
      [NSGraphicsContext setCurrentContext:current];
    }
    else
#endif
    {
      fl_font(FL_HELVETICA, 14);
      fl_color(FL_BLACK);
      int x = x_offset + win->w()/2 - fl_width(win->label())/2;
      if (x < x_offset+skip) x = x_offset+skip;
      fl_push_clip(x_offset, y_offset, win->w(), bt);
      fl_draw(win->label(), x, y_offset+bt/2+4);
      fl_pop_clip();
    }
  }
  [title release];
  this->print_widget(win, x_offset, y_offset + bt); // print the window inner part
}

//
// End of "$Id$".
//
