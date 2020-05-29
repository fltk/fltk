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

#include <FL/Fl_Copy_Surface.H>
#include <FL/Fl.H>


#if defined(__APPLE__)
#include <ApplicationServices/ApplicationServices.h>

Fl_Quartz_Surface_::Fl_Quartz_Surface_(int w, int h) : Fl_System_Printer(), width(w), height(h) {
}

int Fl_Quartz_Surface_::printable_rect(int *w, int *h) {
  *w = width;
  *h = height;
  return 0;
}

const char *Fl_Quartz_Surface_::class_id = "Fl_Quartz_Surface_";

#elif defined(WIN32)

Fl_GDI_Surface_::Fl_GDI_Surface_() : Fl_Paged_Device() {
  driver(new Fl_GDI_Graphics_Driver);
  depth = 0;
}

Fl_GDI_Surface_::~Fl_GDI_Surface_() {
  delete driver();
}

void Fl_GDI_Surface_::translate(int x, int y) {
  GetWindowOrgEx(fl_gc, origins+depth);
  SetWindowOrgEx(fl_gc, origins[depth].x - x, origins[depth].y - y, NULL);
  if (depth < sizeof(origins)/sizeof(POINT)) depth++;
  else Fl::warning("Fl_GDI_Surface_: translate stack overflow!");
}

void Fl_GDI_Surface_::untranslate() {
  if (depth > 0) depth--;
  SetWindowOrgEx(fl_gc, origins[depth].x, origins[depth].y, NULL);
}
  
const char *Fl_GDI_Surface_::class_id = "Fl_GDI_Surface_";

#endif


const char *Fl_Copy_Surface::class_id = "Fl_Copy_Surface";

/** Constructor. 
 \param w and \param h are the width and height of the clipboard surface
 in pixels where drawing will occur.
 */
Fl_Copy_Surface::Fl_Copy_Surface(int w, int h) :  Fl_Surface_Device(NULL)
{
  width = w;
  height = h;
#ifdef __APPLE__
  helper = new Fl_Quartz_Surface_(width, height);
  driver(helper->driver());
  prepare_copy_pdf_and_tiff(w, h);
  oldgc = fl_gc;
#elif defined(WIN32)
  helper = new Fl_GDI_Surface_();
  driver(helper->driver());
  oldgc = fl_gc;
  // exact computation of factor from screen units to EnhMetaFile units (0.01 mm)
  HDC hdc = GetDC(NULL);
  int hmm = GetDeviceCaps(hdc, HORZSIZE);
  int hdots = GetDeviceCaps(hdc, HORZRES);
  int vmm = GetDeviceCaps(hdc, VERTSIZE);
  int vdots = GetDeviceCaps(hdc, VERTRES);
  int dhr = GetDeviceCaps(hdc, DESKTOPHORZRES); // true number of pixels on display
  ReleaseDC(NULL, hdc);
  float factorw = (100.f * hmm) / hdots;
  float factorh = (100.f * vmm) / vdots;
  // Global display scaling factor: 1, 1.25, 1.5, 1.75, etc...
  float scaling = dhr/float(hdots);
  
  RECT rect; rect.left = 0; rect.top = 0; rect.right = (LONG)(w * factorw / scaling); rect.bottom = (LONG)(h * factorh / scaling);
  gc = CreateEnhMetaFile (NULL, NULL, &rect, NULL);
  if (gc != NULL) {
    SetTextAlign(gc, TA_BASELINE|TA_LEFT);
    SetBkMode(gc, TRANSPARENT);
  } 
#else // Xlib
  helper = new Fl_Xlib_Surface_();
  driver(helper->driver());
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
#ifdef __APPLE__
  complete_copy_pdf_and_tiff();
  fl_gc = oldgc;
  delete (Fl_Quartz_Surface_*)helper;
#elif defined(WIN32)
  if(oldgc == fl_gc) oldgc = NULL;
  HENHMETAFILE hmf = CloseEnhMetaFile (gc);
  if ( hmf != NULL ) {
    if ( OpenClipboard (NULL) ){
      EmptyClipboard ();
      // first, put vectorial version of graphics in the clipboard
      SetClipboardData (CF_ENHMETAFILE, hmf);
      // next, put BITMAP version of the graphics in the clipboard
      RECT rect = {0, 0, width, height};
      Fl_Offscreen of = CreateCompatibleBitmap(fl_GetDC(0), width, height);
      fl_begin_offscreen(of);
      fl_color(FL_WHITE);    // draw white background
      fl_rectf(0, 0, width, height);
      PlayEnhMetaFile((HDC)fl_gc, hmf, &rect); // draw metafile to offscreen buffer
      fl_end_offscreen();
      SetClipboardData(CF_BITMAP, (HBITMAP)of);
      fl_delete_offscreen(of);
      CloseClipboard ();
    }
    DeleteEnhMetaFile(hmf);
  }
  DeleteDC(gc);
  fl_gc = oldgc;
  delete (Fl_GDI_Surface_*)helper;
#else // Xlib
  fl_pop_clip(); 
  unsigned char *data = fl_read_image(NULL,0,0,width,height,0);
  fl_window = oldwindow; 
  _ss->set_current();
  Fl::copy_image(data,width,height,1);
  delete[] data;
  fl_delete_offscreen(xid);
  delete (Fl_Xlib_Surface_*)helper;
#endif
}

/** Copies a widget in the clipboard
 
 \param widget any FLTK widget (e.g., standard, custom, window, GL view) to copy
 \param delta_x and \param delta_y give 
 the position in the clipboard of the top-left corner of the widget
 */
void Fl_Copy_Surface::draw(Fl_Widget* widget, int delta_x, int delta_y) 
{
  helper->print_widget(widget, delta_x, delta_y);
}

void Fl_Copy_Surface::set_current()
{
#if defined(__APPLE__) || defined(WIN32)
  fl_gc = gc;
  fl_window = (Window)1;
  Fl_Surface_Device::set_current();
#else
  fl_window=xid; 
  _ss = Fl_Surface_Device::surface(); 
  Fl_Surface_Device::set_current();
  fl_push_no_clip();
#endif
}


#if defined(__APPLE__)

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
  CGContextTranslateCTM(gc, 0.5, h-0.5);
  CGContextScaleCTM(gc, 1.0f, -1.0f);
  CGContextSaveGState(gc);
}

void Fl_Copy_Surface::draw_decorated_window(Fl_Window* win, int delta_x, int delta_y)
{
  int bt = win->decorated_h() - win->h();
  draw(win, delta_x, bt + delta_y ); // draw the window content
  if (win->border()) {
    // draw the window title bar
    CGContextSaveGState(gc);
    CGContextTranslateCTM(gc, delta_x, bt + delta_y);
    CGContextScaleCTM(gc, 1, -1);
    Fl_X::clip_to_rounded_corners(gc, win->w(), bt);
    void *layer = Fl_X::get_titlebar_layer(win);
    if (layer) {
      CGColorSpaceRef cspace = CGColorSpaceCreateDeviceRGB();
      if (fl_mac_os_version < 101300) {
        // for unknown reason, rendering the layer to the Fl_Copy_Surface pdf graphics context does not work;
        // we use an auxiliary bitmap context
        CGContextRef auxgc = CGBitmapContextCreate(NULL, win->w(), bt, 8, 0, cspace, kCGImageAlphaPremultipliedLast);
        CGColorSpaceRelease(cspace);
        CGContextTranslateCTM(auxgc, 0, bt);
        CGContextScaleCTM(auxgc, 1, -1);
        Fl_X::draw_layer_to_context(layer, auxgc, win->w(), bt);
        Fl_RGB_Image *image = new Fl_RGB_Image((const uchar*)CGBitmapContextGetData(auxgc), win->w(), bt, 4,
                                               CGBitmapContextGetBytesPerRow(auxgc)); // 10.2
        image->draw(0, 0);
        delete image;
        CGContextRelease(auxgc);
      } else {
        Fl_X::draw_layer_to_context(layer, gc, win->w(), bt);
      }
    } else {
      CGImageRef img = Fl_X::CGImage_from_window_rect(win, 0, -bt, win->w(), bt);
      CGContextDrawImage(gc, CGRectMake(0, 0, win->w(), bt), img);
      CGImageRelease(img);
    }
    CGContextRestoreGState(gc);
  }
}

#else

/** Copies a window and its borders and title bar to the clipboard. 
 \param win an FLTK window to copy
 \param delta_x and \param delta_y give
 the position in the clipboard of the top-left corner of the window's title bar
*/
void Fl_Copy_Surface::draw_decorated_window(Fl_Window* win, int delta_x, int delta_y)
{
  helper->draw_decorated_window(win, delta_x, delta_y, this);
}

#endif // __APPLE__


#if !(defined(__APPLE__) || defined(WIN32) || defined(FL_DOXYGEN))
/* graphics driver that translates all graphics coordinates before calling Xlib */
class Fl_translated_Xlib_Graphics_Driver_ : public Fl_Xlib_Graphics_Driver {
  int offset_x, offset_y; // translation between user and graphical coordinates: graphical = user + offset
  unsigned depth; // depth of translation stack
  int stack_x[20], stack_y[20]; // translation stack allowing cumulative translations
public:
  static const char *class_id;
  const char *class_name() {return class_id;};
  Fl_translated_Xlib_Graphics_Driver_() {
    offset_x = 0; offset_y = 0;
    depth = 0;
    }
  virtual ~Fl_translated_Xlib_Graphics_Driver_() {};
  void translate_all(int dx, int dy) { // reversibly adds dx,dy to the offset between user and graphical coordinates
    stack_x[depth] = offset_x;
    stack_y[depth] = offset_y;
    offset_x = stack_x[depth] + dx;
    offset_y = stack_y[depth] + dy;
    push_matrix();
    translate(dx, dy);
    if (depth < sizeof(stack_x)/sizeof(int)) depth++;
    else Fl::warning("%s: translate stack overflow!", class_id);
  }
  void untranslate_all() { // undoes previous translate_all()
    if (depth > 0) depth--;
    offset_x = stack_x[depth];
    offset_y = stack_y[depth];
    pop_matrix();
  }
  void rect(int x, int y, int w, int h) { Fl_Xlib_Graphics_Driver::rect(x+offset_x, y+offset_y, w, h); }
  void rectf(int x, int y, int w, int h) { Fl_Xlib_Graphics_Driver::rectf(x+offset_x, y+offset_y, w, h); }
  void xyline(int x, int y, int x1) { Fl_Xlib_Graphics_Driver::xyline(x+offset_x, y+offset_y, x1+offset_x); }
  void xyline(int x, int y, int x1, int y2) { Fl_Xlib_Graphics_Driver::xyline(x+offset_x, y+offset_y, x1+offset_x, y2+offset_y); }
  void xyline(int x, int y, int x1, int y2, int x3) { Fl_Xlib_Graphics_Driver::xyline(x+offset_x, y+offset_y, x1+offset_x, y2+offset_y, x3+offset_x); }
  void yxline(int x, int y, int y1) { Fl_Xlib_Graphics_Driver::yxline(x+offset_x, y+offset_y, y1+offset_y); }
  void yxline(int x, int y, int y1, int x2) { Fl_Xlib_Graphics_Driver::yxline(x+offset_x, y+offset_y, y1+offset_y, x2+offset_x); }
  void yxline(int x, int y, int y1, int x2, int y3) { Fl_Xlib_Graphics_Driver::yxline(x+offset_x, y+offset_y, y1+offset_y, x2+offset_x, y3+offset_y); }
  void line(int x, int y, int x1, int y1) { Fl_Xlib_Graphics_Driver::line(x+offset_x, y+offset_y, x1+offset_x, y1+offset_y); }
  void line(int x, int y, int x1, int y1, int x2, int y2) { Fl_Xlib_Graphics_Driver::line(x+offset_x, y+offset_y, x1+offset_x, y1+offset_y, x2+offset_x, y2+offset_y); }
  void draw(const char* str, int n, int x, int y) { 
    Fl_Xlib_Graphics_Driver::draw(str, n, x+offset_x, y+offset_y); 
 }
  void draw(int angle, const char *str, int n, int x, int y) { 
    Fl_Xlib_Graphics_Driver::draw(angle, str, n, x+offset_x, y+offset_y); 
  }
  void rtl_draw(const char* str, int n, int x, int y) { 
    Fl_Xlib_Graphics_Driver::rtl_draw(str, n, x+offset_x, y+offset_y); 
  }
  void draw(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) { 
    XP += offset_x; YP += offset_y;
    translate_all(-offset_x, -offset_y);
    Fl_Xlib_Graphics_Driver::draw(pxm, XP, YP, WP,HP,cx,cy); 
    untranslate_all();
  }
  void draw(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) { 
    XP += offset_x; YP += offset_y;
    translate_all(-offset_x, -offset_y);
    Fl_Xlib_Graphics_Driver::draw(bm, XP, YP, WP,HP,cx,cy); 
    untranslate_all();
  }
  void draw(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy) { 
    XP += offset_x; YP += offset_y;
    translate_all(-offset_x, -offset_y);
    Fl_Xlib_Graphics_Driver::draw(img, XP, YP, WP,HP,cx,cy); 
    untranslate_all();
  }
  void draw_image(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0) { 
    X += offset_x; Y += offset_y;
    translate_all(-offset_x, -offset_y);
    Fl_Xlib_Graphics_Driver::draw_image(buf, X, Y, W,H,D,L); 
    untranslate_all();
  }
  void draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3) { 
    X += offset_x; Y += offset_y;
    translate_all(-offset_x, -offset_y);
    Fl_Xlib_Graphics_Driver::draw_image(cb, data, X, Y, W,H,D); 
    untranslate_all();
  }
  void draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0) { 
    X += offset_x; Y += offset_y;
    translate_all(-offset_x, -offset_y);
    Fl_Xlib_Graphics_Driver::draw_image_mono(buf, X, Y, W,H,D,L); 
    untranslate_all();
  }
  void draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1) { 
    X += offset_x; Y += offset_y;
    translate_all(-offset_x, -offset_y);
    Fl_Xlib_Graphics_Driver::draw_image_mono(cb, data, X, Y, W,H,D); 
    untranslate_all();
  }
  void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy) { 
    Fl_Xlib_Graphics_Driver::copy_offscreen(x+offset_x, y+offset_y, w, h,pixmap,srcx,srcy); 
  }
  void push_clip(int x, int y, int w, int h) {
    Fl_Xlib_Graphics_Driver::push_clip(x+offset_x, y+offset_y, w, h); 
  }
  int not_clipped(int x, int y, int w, int h) { return Fl_Xlib_Graphics_Driver::not_clipped(x + offset_x, y + offset_y, w, h); };
  int clip_box(int x, int y, int w, int h, int& X, int& Y, int& W, int& H) { 
    int retval = Fl_Xlib_Graphics_Driver::clip_box(x + offset_x, y + offset_y, w,h,X,Y,W,H);
    X -= offset_x;
    Y -= offset_y;
    return retval;
  }
  void pie(int x, int y, int w, int h, double a1, double a2) { Fl_Xlib_Graphics_Driver::pie(x+offset_x,y+offset_y,w,h,a1,a2); }
  void arc(int x, int y, int w, int h, double a1, double a2) { Fl_Xlib_Graphics_Driver::arc(x+offset_x,y+offset_y,w,h,a1,a2); }
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2) { Fl_Xlib_Graphics_Driver::polygon(x0+offset_x,y0+offset_y,x1+offset_x,y1+offset_y,x2+offset_x,y2+offset_y);}
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
    Fl_Xlib_Graphics_Driver::polygon(x0+offset_x,y0+offset_y,x1+offset_x,y1+offset_y,x2+offset_x,y2+offset_y,x3+offset_x,y3+offset_y);
  }
  void loop(int x0, int y0, int x1, int y1, int x2, int y2) {Fl_Xlib_Graphics_Driver::loop(x0+offset_x,y0+offset_y,x1+offset_x,y1+offset_y,x2+offset_x,y2+offset_y);}
  void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
    Fl_Xlib_Graphics_Driver::loop(x0+offset_x,y0+offset_y,x1+offset_x,y1+offset_y,x2+offset_x,y2+offset_y,x3+offset_x,y3+offset_y);
  }
  void point(int x, int y) { Fl_Xlib_Graphics_Driver::point(x+offset_x, y+offset_y); }
};

const char *Fl_translated_Xlib_Graphics_Driver_::class_id = "Fl_translated_Xlib_Graphics_Driver_";

void Fl_Xlib_Surface_::translate(int x, int y) { 
  ((Fl_translated_Xlib_Graphics_Driver_*)driver())->translate_all(x, y); 
}
void Fl_Xlib_Surface_::untranslate() { 
  ((Fl_translated_Xlib_Graphics_Driver_*)driver())->untranslate_all(); 
}

Fl_Xlib_Surface_::Fl_Xlib_Surface_() : Fl_Paged_Device() {
  driver(new Fl_translated_Xlib_Graphics_Driver_());
}
Fl_Xlib_Surface_::~Fl_Xlib_Surface_() {
  delete driver();
}

const char *Fl_Xlib_Surface_::class_id = "Fl_Xlib_Surface_";

#endif

//
// End of "$Id$".
//
