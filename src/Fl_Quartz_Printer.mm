//
// "$Id$"
//
// Mac OS X-specific printing support (objective-c++) for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2014 by Bill Spitzak and others.
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

#ifdef __APPLE__
#include <FL/Fl_Printer.H>

#include <FL/Fl.H>
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

extern void fl_quartz_restore_line_style_();

Fl_System_Printer::Fl_System_Printer(void)
{
  x_offset = 0;
  y_offset = 0;
  scale_x = scale_y = 1.;
  gc = 0;
  driver(new Fl_Quartz_Graphics_Driver);
}

Fl_System_Printer::~Fl_System_Printer(void) {
  delete driver();
}

@interface print_panel_delegate : NSObject
- (void)printPanelDidEnd:(NSPrintPanel *)printPanel returnCode:(NSInteger)returnCode contextInfo:(NSInteger *)contextInfo;
@end
@implementation print_panel_delegate
- (void)printPanelDidEnd:(NSPrintPanel *)printPanel returnCode:(NSInteger)returnCode contextInfo:(NSInteger *)contextInfo
{
  *contextInfo = returnCode;
}
@end

int Fl_System_Printer::start_job (int pagecount, int *frompage, int *topage)
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
    [panel setOptions:NSPrintPanelShowsCopies | NSPrintPanelShowsPageRange | NSPrintPanelShowsPageSetupAccessory | NSPrintPanelShowsOrientation | NSPrintPanelShowsPaperSize];
    NSInteger retval = -1;
    Fl_Window *top = Fl::first_window();
    NSWindow *main = (top ? (NSWindow*)fl_xid(top->top_window()) : nil);
    if (main) {
      [panel beginSheetWithPrintInfo:info
                      modalForWindow:main
                            delegate:[[[print_panel_delegate alloc] init] autorelease]
                      didEndSelector:@selector(printPanelDidEnd:returnCode:contextInfo:)
                         contextInfo:&retval];
      while (retval < 0) Fl::wait(100);
      [main makeKeyAndOrderFront:nil];
    } else
      retval = [panel runModalWithPrintInfo:info]; //from 10.5
    if (retval != NSOKButton) return 1;
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
#if !defined(__LP64__) || !__LP64__
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

void Fl_System_Printer::margins(int *left, int *top, int *right, int *bottom)
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

int Fl_System_Printer::printable_rect(int *w, int *h)
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

void Fl_System_Printer::origin(int x, int y)
{
  x_offset = x;
  y_offset = y;
  CGContextRestoreGState(fl_gc);
  CGContextRestoreGState(fl_gc);
  CGContextSaveGState(fl_gc);
  CGContextScaleCTM(fl_gc, scale_x, scale_y);
  CGContextTranslateCTM(fl_gc, x, y);
  CGContextRotateCTM(fl_gc, angle);
  CGContextSaveGState(fl_gc);
}

void Fl_System_Printer::scale (float s_x, float s_y)
{
  if (s_y == 0.) s_y = s_x;
  scale_x = s_x;
  scale_y = s_y;
  CGContextRestoreGState(fl_gc);
  CGContextRestoreGState(fl_gc);
  CGContextSaveGState(fl_gc);
  CGContextScaleCTM(fl_gc, scale_x, scale_y);
  CGContextRotateCTM(fl_gc, angle);
  x_offset = y_offset = 0;
  CGContextSaveGState(fl_gc);
}

void Fl_System_Printer::rotate (float rot_angle)
{
  angle = - rot_angle * M_PI / 180.;
  CGContextRestoreGState(fl_gc);
  CGContextRestoreGState(fl_gc);
  CGContextSaveGState(fl_gc);
  CGContextScaleCTM(fl_gc, scale_x, scale_y);
  CGContextTranslateCTM(fl_gc, x_offset, y_offset);
  CGContextRotateCTM(fl_gc, angle);
  CGContextSaveGState(fl_gc);
}

void Fl_System_Printer::translate(int x, int y)
{
  CGContextSaveGState(fl_gc);
  CGContextTranslateCTM(fl_gc, x, y );
  CGContextSaveGState(fl_gc);
}

void Fl_System_Printer::untranslate(void)
{
  CGContextRestoreGState(fl_gc);
  CGContextRestoreGState(fl_gc);
}

int Fl_System_Printer::start_page (void)
{	
  OSStatus status = PMSessionBeginPageNoDialog(printSession, pageFormat, NULL);
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if ( &PMSessionGetCGGraphicsContext != NULL ) {
    status = PMSessionGetCGGraphicsContext(printSession, &fl_gc);
  }
  else
#endif
  {
#if !defined(__LP64__) || !__LP64__
    PMSessionGetGraphicsContext_type PMSessionGetGraphicsContext =
      (PMSessionGetGraphicsContext_type)Fl_X::get_carbon_function("PMSessionGetGraphicsContext");
    status = PMSessionGetGraphicsContext(printSession, NULL, (void **)&fl_gc);
#endif
  }
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
    CGContextTranslateCTM(fl_gc, margins.left, margins.bottom + h);
  else
    CGContextTranslateCTM(fl_gc, margins.top, margins.right + h);
  CGContextScaleCTM(fl_gc, win_scale_x, - win_scale_y);
  fl_quartz_restore_line_style_();
  CGContextSetShouldAntialias(fl_gc, false);
  CGContextSaveGState(fl_gc);
  CGContextSaveGState(fl_gc);
  fl_line_style(FL_SOLID);
  fl_window = (Window)1; // TODO: something better
  fl_clip_region(0);
  if( status == noErr) gc = fl_gc;
  return status != noErr;
}

int Fl_System_Printer::end_page (void)
{	
  CGContextFlush(fl_gc);
  CGContextRestoreGState(fl_gc);
  CGContextRestoreGState(fl_gc);
  OSStatus status = PMSessionEndPageNoDialog(printSession);
  gc = NULL;
  return status != noErr;
}

void Fl_System_Printer::end_job (void)
{
  OSStatus status;
  
  status = PMSessionError(printSession);
  if (status != noErr) {
    fl_alert ("PM Session error %d", (int)status);
  }
  PMSessionEndDocumentNoDialog(printSession);
#if !defined(__LP64__) || !__LP64__
  if (fl_mac_os_version < 100500) {
    PMRelease(printSettings);
    PMRelease(pageFormat);
    PMRelease(printSession);
    }
#endif
  Fl_Display_Device::display_device()->set_current();
  fl_gc = 0;
  Fl_Window *w = Fl::first_window();
  if (w) w->show();
}

// version that prints at high res if using a retina display
void Fl_System_Printer::print_window_part(Fl_Window *win, int x, int y, int w, int h, int delta_x, int delta_y)
{
  Fl_Surface_Device *current = Fl_Surface_Device::surface();
  Fl_Display_Device::display_device()->set_current();
  Fl_Window *save_front = Fl::first_window();
  win->show();
  fl_gc = NULL;
  Fl::check();
  CGImageRef img = Fl_X::CGImage_from_window_rect(win, x, y, w, h);
  if (save_front != win) save_front->show();
  current->set_current();
  CGRect rect = CGRectMake(delta_x, delta_y, w, h);
  Fl_X::q_begin_image(rect, 0, 0, w, h);
  CGContextDrawImage(fl_gc, rect, img);
  Fl_X::q_end_image();
  CFRelease(img);
}

#endif // __APPLE__

//
// End of "$Id$".
//
