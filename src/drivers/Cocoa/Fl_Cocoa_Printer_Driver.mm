//
// Mac OS X-specific printing support (objective-c++) for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2024 by Bill Spitzak and others.
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

#include <FL/Fl_Paged_Device.H>
#include <FL/Fl_Printer.H>
#include "../../Fl_Window_Driver.H"
#include "../../Fl_Screen_Driver.H"
#include "../Quartz/Fl_Quartz_Graphics_Driver.H"
#include "../Darwin/Fl_Darwin_System_Driver.H"
#include <FL/Fl_PDF_File_Surface.H>
#include "Fl_Cocoa_Window_Driver.H"

#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/fl_string_functions.h>
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


/** Support for printing on the Apple OS X platform */
class Fl_Cocoa_Printer_Driver : public Fl_Paged_Device {
  friend class Fl_Printer;
protected:
  float scale_x;
  float scale_y;
  float angle; // rotation angle in radians
  PMPrintSession  printSession;
  PMPageFormat    pageFormat;
  PMPrintSettings printSettings;
  Fl_Cocoa_Printer_Driver(void);
  int begin_job(int pagecount = 0, int *frompage = NULL, int *topage = NULL, char **perr_message = NULL) FL_OVERRIDE;
  int begin_page (void) FL_OVERRIDE;
  int printable_rect(int *w, int *h) FL_OVERRIDE;
  void margins(int *left, int *top, int *right, int *bottom) FL_OVERRIDE;
  void origin(int *x, int *y) FL_OVERRIDE;
  void origin(int x, int y) FL_OVERRIDE;
  void scale (float scale_x, float scale_y = 0.) FL_OVERRIDE;
  void rotate(float angle) FL_OVERRIDE;
  void translate(int x, int y) FL_OVERRIDE;
  void untranslate(void) FL_OVERRIDE;
  int end_page (void) FL_OVERRIDE;
  void end_job (void) FL_OVERRIDE;
  ~Fl_Cocoa_Printer_Driver(void);
};


Fl_Cocoa_Printer_Driver::Fl_Cocoa_Printer_Driver(void)
{
  x_offset = 0;
  y_offset = 0;
  scale_x = scale_y = 1.;
  driver(new Fl_Quartz_Printer_Graphics_Driver);
}

Fl_Paged_Device* Fl_Printer::newPrinterDriver(void)
{
  return new Fl_Cocoa_Printer_Driver();
}

Fl_Cocoa_Printer_Driver::~Fl_Cocoa_Printer_Driver(void) {
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

int Fl_Cocoa_Printer_Driver::begin_job (int pagecount, int *frompage, int *topage, char **perr_message)
//printing using a Quartz graphics context
//returns 0 iff OK
{
  OSStatus status = 0;
  fl_open_display();
  Fl_Cocoa_Window_Driver::q_release_context();
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
  if (fl_mac_os_version >= 100500) {
    NSPrintInfo *info = [NSPrintInfo sharedPrintInfo];
    NSPrintPanel *panel = [NSPrintPanel printPanel];
    //from 10.5
    [panel setOptions:NSPrintPanelShowsCopies | NSPrintPanelShowsPageRange |
      NSPrintPanelShowsPageSetupAccessory | NSPrintPanelShowsOrientation | NSPrintPanelShowsPaperSize];
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
#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_9
    const NSInteger NSModalResponseOK = NSOKButton;
#endif
    if (retval != NSModalResponseOK) return 1;
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
    if (!f) f = (dialog_f)Fl_Darwin_System_Driver::get_carbon_function("PMSessionPageSetupDialog");
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
    if (!f2) f2 = (dialog_f2)Fl_Darwin_System_Driver::get_carbon_function("PMSessionPrintDialog");
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
      (PMSessionSetDocumentFormatGeneration_type)Fl_Darwin_System_Driver::get_carbon_function("PMSessionSetDocumentFormatGeneration");
    status = PMSessionSetDocumentFormatGeneration(printSession, kPMDocumentFormatDefault, array, NULL);
    CFRelease(array);
    PMSessionBeginDocumentNoDialog_type PMSessionBeginDocumentNoDialog =
      (PMSessionBeginDocumentNoDialog_type)Fl_Darwin_System_Driver::get_carbon_function("PMSessionBeginDocumentNoDialog");
    status = PMSessionBeginDocumentNoDialog(printSession, printSettings, pageFormat);
#endif //__LP64__
  }

  if (status != noErr) {
    if (perr_message) {
      NSError *nserr = [NSError errorWithDomain:NSCocoaErrorDomain code:status userInfo:nil];
      NSString *s = [nserr localizedDescription];
      if (s) *perr_message = fl_strdup([s UTF8String]);
    }
    return 2;
  }
  y_offset = x_offset = 0;
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

int Fl_Cocoa_Printer_Driver::begin_page (void)
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
#if !defined(__LP64__) || !__LP64__
    PMSessionGetGraphicsContext_type PMSessionGetGraphicsContext =
      (PMSessionGetGraphicsContext_type)Fl_Darwin_System_Driver::get_carbon_function("PMSessionGetGraphicsContext");
    status = PMSessionGetGraphicsContext(printSession, NULL, (void **)&gc);
#endif
  }
  driver()->gc(gc);
  Fl_Surface_Device::push_current(this);
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
  ((Fl_Quartz_Graphics_Driver*)driver())->quartz_restore_line_style();
  CGContextSetShouldAntialias(gc, false);
  CGContextSaveGState(gc);
  CGContextSaveGState(gc);
  fl_line_style(FL_SOLID);
  fl_window = (FLWindow*)1; // TODO: something better
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
  Fl_Surface_Device::pop_current();
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
#if !defined(__LP64__) || !__LP64__
  if (fl_mac_os_version < 100500) {
    PMRelease(printSettings);
    PMRelease(pageFormat);
    PMRelease(printSession);
    }
#endif
  Fl_Window *w = Fl::first_window();
  if (w) w->show();
}

void Fl_Cocoa_Printer_Driver::origin(int *x, int *y)
{
  Fl_Paged_Device::origin(x, y);
}


class Fl_PDF_Cocoa_File_Surface : public Fl_Cocoa_Printer_Driver
{
public:
  char *doc_fname;
  Fl_PDF_Cocoa_File_Surface();
  ~Fl_PDF_Cocoa_File_Surface() { if (doc_fname) free(doc_fname); }
  int begin_job(const char *defaultname,
                char **perr_message = NULL);
  int begin_job(int, int*, int *, char **) FL_OVERRIDE {return 1;} // don't use
  int begin_document(const char* outname,
                     enum Fl_Paged_Device::Page_Format format,
                     enum Fl_Paged_Device::Page_Layout layout,
                     char **perr_message);
};


Fl_PDF_Cocoa_File_Surface::Fl_PDF_Cocoa_File_Surface() {
  driver(new Fl_Quartz_Graphics_Driver());
  doc_fname = NULL;
}


int Fl_PDF_Cocoa_File_Surface::begin_job(const char* defaultfilename,
                                      char **perr_message) {
  OSStatus status = 0;
  if (fl_mac_os_version < 100900) return 1;
  Fl_Window *top = Fl::first_window();
  NSWindow *main = (top ? (NSWindow*)fl_xid(top->top_window()) : nil);
  if (!main) return 1;
  Fl_Cocoa_Window_Driver::q_release_context();
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_9 && defined(__BLOCKS__)
  NSPDFInfo *pdf_info = [[NSPDFInfo alloc] init]; // 10.9
  NSPDFPanel *pdf_panel = [NSPDFPanel panel]; // 10.9
  char buf[FL_PATH_MAX];
  strcpy(buf, defaultfilename);
  fl_filename_setext(buf, sizeof(buf), NULL);
  [pdf_panel setDefaultFileName:[NSString stringWithUTF8String:buf]];
  [pdf_panel setOptions: NSPrintPanelShowsOrientation | NSPrintPanelShowsPaperSize];
  NSInteger retval = -1;
  __block NSInteger complete = -1;
  [pdf_panel beginSheetWithPDFInfo:pdf_info
                    modalForWindow:main
                 completionHandler:^(NSInteger returnCode) {
    // this block runs after OK or Cancel was triggered in file dialog
    complete = returnCode;
  }
  ];
  while (complete == -1) Fl::wait(100); // loop until end of file dialog
  retval = complete;
  [main makeKeyAndOrderFront:nil];
  if (retval != NSModalResponseOK) return 1;
  NSURL *url = [pdf_info URL];
  doc_fname = fl_strdup([url fileSystemRepresentation]);
  NSPrintInfo *pr_info = [NSPrintInfo sharedPrintInfo];
  [pr_info takeSettingsFromPDFInfo:pdf_info];
  [pdf_info release];
  printSession = (PMPrintSession)[pr_info PMPrintSession];
  printSettings = (PMPrintSettings)[pr_info PMPrintSettings];
  pageFormat = (PMPageFormat)[pr_info PMPageFormat];
  status = PMSessionBeginCGDocumentNoDialog(printSession, printSettings, pageFormat);//from 10.4
#endif
  if (status != noErr) {
    if (perr_message) {
      NSError *nserr = [NSError errorWithDomain:NSCocoaErrorDomain code:status userInfo:nil];
      NSString *s = [nserr localizedDescription];
      if (s) *perr_message = fl_strdup([s UTF8String]);
    }
    free(doc_fname);
    doc_fname = NULL;
    return 2;
  }
  y_offset = x_offset = 0;
  return 0;
}


int Fl_PDF_Cocoa_File_Surface::begin_document(const char* outfname,
                                      enum Fl_Paged_Device::Page_Format format,
                                      enum Fl_Paged_Device::Page_Layout layout,
                                      char **perr_message) {
  OSStatus status = 0;
  fl_open_display();
  if (fl_mac_os_version < 100900) return 1;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_9
  NSPDFInfo *pdf_info = [[NSPDFInfo alloc] init]; // 10.9
  doc_fname = fl_strdup(outfname);
  NSURL *url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:doc_fname]];
  [pdf_info setURL:url];
  NSSize psize = {(CGFloat)Fl_Paged_Device::page_formats[format].width, (CGFloat)Fl_Paged_Device::page_formats[format].height};
  [pdf_info setPaperSize:psize];
  [pdf_info setOrientation:(layout == PORTRAIT ? NSPaperOrientationPortrait : NSPaperOrientationLandscape)];
  NSPrintInfo *pr_info = [NSPrintInfo sharedPrintInfo];
  [pr_info takeSettingsFromPDFInfo:pdf_info];
  [pdf_info release];
  printSession = (PMPrintSession)[pr_info PMPrintSession];
  printSettings = (PMPrintSettings)[pr_info PMPrintSettings];
  pageFormat = (PMPageFormat)[pr_info PMPageFormat];
  status = PMSessionBeginCGDocumentNoDialog(printSession, printSettings, pageFormat);//from 10.4
#endif
  if (status != noErr) {
    if (perr_message) {
      NSError *nserr = [NSError errorWithDomain:NSCocoaErrorDomain code:status userInfo:nil];
      NSString *s = [nserr localizedDescription];
      if (s) *perr_message = fl_strdup([s UTF8String]);
    }
    free(doc_fname);
    doc_fname = NULL;
    return 2;
  }
  y_offset = x_offset = 0;
  return 0;
}


Fl_Paged_Device *Fl_PDF_File_Surface::new_platform_pdf_surface_(const char ***pfname) {
  Fl_PDF_Cocoa_File_Surface *surf = new Fl_PDF_Cocoa_File_Surface();
  *pfname = (const char**)&surf->doc_fname;
  return surf;
}


int Fl_PDF_File_Surface::begin_job(const char* defaultfilename,
                                char **perr_message) {
  return ((Fl_PDF_Cocoa_File_Surface*)platform_surface_)->begin_job(defaultfilename, perr_message);
}


int Fl_PDF_File_Surface::begin_document(const char* defaultfilename,
                                     enum Fl_Paged_Device::Page_Format format,
                                     enum Fl_Paged_Device::Page_Layout layout,
                                     char **perr_message) {
  return ((Fl_PDF_Cocoa_File_Surface*)platform_surface_)->begin_document(defaultfilename, format, layout, perr_message);
}
