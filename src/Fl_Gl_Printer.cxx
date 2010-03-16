/*
 *  Fl_Gl_Printer.cxx
 */

#include "FL/Fl_Gl_Printer.H"
#include "Fl_Gl_Choice.H"
#include "FL/Fl.H"
#ifndef __APPLE__
#include "FL/fl_draw.H"
#endif

#if defined(__APPLE__)
static void imgProviderReleaseData (void *info, const void *data, size_t size)
{
  free((void *)data);
}
#endif

void Fl_Gl_Printer::print_gl_window(Fl_Gl_Window *glw, int x, int y)
{
#ifdef WIN32
  HDC save_gc = fl_gc;
  const int bytesperpixel = 3;
#elif defined(__APPLE__)
  CGContextRef save_gc = fl_gc;
  const int bytesperpixel = 4;
#else
  _XGC *save_gc = fl_gc;
  const int bytesperpixel = 3;
#endif
  glw->redraw();
  fl_gc = NULL;
  Fl::check();
  glw->make_current();
  // select front buffer as our source for pixel data
  glReadBuffer(GL_FRONT);
  // Read OpenGL context pixels directly.
  // For extra safety, save & restore OpenGL states that are changed
  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
  glPixelStorei(GL_PACK_ALIGNMENT, 4); /* Force 4-byte alignment */
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
  // Read a block of pixels from the frame buffer
  int mByteWidth = glw->w() * bytesperpixel;                
  mByteWidth = (mByteWidth + 3) & ~3;    // Align to 4 bytes
  uchar *baseAddress = (uchar*)malloc(mByteWidth * glw->h());
  glReadPixels(0, 0, glw->w(), glw->h(), 
#ifdef WIN32
	       GL_RGB, GL_UNSIGNED_BYTE,
#elif defined(__APPLE__)
	       GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
#else // FIXME Linux/Unix
	       GL_RGB, GL_UNSIGNED_BYTE,
#endif
	       baseAddress);
  glPopClientAttrib();
  fl_gc = save_gc;
#if defined(__APPLE__)
#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_4
#define kCGBitmapByteOrder32Big 0
#define CGBitmapInfo CGImageAlphaInfo
#endif
  CGColorSpaceRef cSpace = CGColorSpaceCreateDeviceRGB();
  CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, baseAddress, mByteWidth * glw->h(), imgProviderReleaseData);
  CGImageRef image = CGImageCreate(glw->w(), glw->h(), 8, 8*bytesperpixel, mByteWidth, cSpace,
#if __BIG_ENDIAN__
		(CGBitmapInfo)(kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Big) /* XRGB Big Endian */
#else
		  kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little /* XRGB Little Endian */
#endif
   , provider, NULL, false, kCGRenderingIntentDefault);
  if(image == NULL) return;
  CGContextSaveGState(fl_gc);
  int w, h;
  this->printable_rect(&w, &h);
  CGContextTranslateCTM(fl_gc, 0, h);
  CGContextScaleCTM(fl_gc, 1.0f, -1.0f);
  CGRect rect = { { x, h - y - glw->h() }, { glw->w(), glw->h() } };
  Fl_X::q_begin_image(rect, 0, 0, glw->w(), glw->h());
  CGContextDrawImage(fl_gc, rect, image);
  Fl_X::q_end_image();
  CGContextRestoreGState(fl_gc);
  CGImageRelease(image);
  CGColorSpaceRelease(cSpace);
  CGDataProviderRelease(provider);  
#else
  fl_draw_image(baseAddress + (glw->h() - 1) * mByteWidth, x, y , glw->w(), glw->h(), bytesperpixel, - mByteWidth);
  free(baseAddress);
#endif // __APPLE__
}
