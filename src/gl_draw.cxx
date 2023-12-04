//
// "$Id$"
//
// OpenGL drawing support routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

// Functions from <FL/gl.h>
// See also Fl_Gl_Window and gl_start.cxx

#include "flstring.h"
#if HAVE_GL || defined(FL_DOXYGEN)

#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/gl_draw.H>
#include <FL/x.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Device.H>
#include "Fl_Gl_Choice.H"
#include "Fl_Font.H"
#include <FL/fl_utf8.h>

#if !defined(WIN32) && !defined(__APPLE__)
#include "Xutf8.h"
#endif

#if defined(__APPLE__)

#if !defined(kCGBitmapByteOrder32Host) // doc says available 10.4 but some 10.4 don't have it
#  define kCGBitmapByteOrder32Host 0
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_4
#  include <OpenGL/glext.h>
#  define GL_TEXTURE_RECTANGLE_ARB GL_TEXTURE_RECTANGLE_EXT
#  include <FL/Fl_Gl_Window.H>
#endif // < MAC_OS_X_VERSION_10_4

#endif // __APPLE__

/** Returns the current font's height */
int   gl_height() {return fl_height();}
/** Returns the current font's descent */
int   gl_descent() {return fl_descent();}
/** Returns the width of the string in the current fnt */
double gl_width(const char* s) {return fl_width(s);}
/** Returns the width of n characters of the string in the current font */
double gl_width(const char* s, int n) {return fl_width(s,n);}
/** Returns the width of the character in the current font */
double gl_width(uchar c) {return fl_width(c);}

static Fl_Font_Descriptor *gl_fontsize;

#define GENLISTSIZE 256
#ifndef __APPLE__
#  define USE_OksiD_style_GL_font_selection 1  // Most hosts except OSX
#endif

#if USE_XFT
#  undef USE_OksiD_style_GL_font_selection  // turn this off for XFT also
#endif

/**
  Sets the current OpenGL font to the same font as calling fl_font()
  */
void  gl_font(int fontid, int size) {
  fl_font(fontid, size);
  Fl_Font_Descriptor *fl_fontsize = fl_graphics_driver->font_descriptor();
#ifndef __APPLE__
  if (!fl_fontsize->listbase) {

#ifdef  USE_OksiD_style_GL_font_selection
#undef GENLISTSIZE
#define GENLISTSIZE 0x10000
    fl_fontsize->listbase = glGenLists(GENLISTSIZE);
#else // Fltk-1.1.8 style GL font selection

#if defined (USE_X11) // X-windows options follow, either XFT or "plain" X
// FIXME:  warning Ideally, for XFT, we really need a glXUseXftFont implementation here...
// FIXME:  warning GL font selection is basically wrong here
/* OksiD had a fairly sophisticated scheme for storing multiple X fonts in a XUtf8FontStruct,
 * then sorting through them at draw time (for normal X rendering) to find which one can
 * render the current glyph... But for now, just use the first font in the list for GL...
 */
    XFontStruct *font = fl_X_core_font();
    int base = font->min_char_or_byte2;
    int count = font->max_char_or_byte2-base+1;
    fl_fontsize->listbase = glGenLists(GENLISTSIZE);
    glXUseXFont(font->fid, base, count, fl_fontsize->listbase+base);
# elif defined(WIN32)
    // this is unused because USE_OksiD_style_GL_font_selection == 1
    int base = fl_fontsize->metr.tmFirstChar;
    int count = fl_fontsize->metr.tmLastChar-base+1;
    HFONT oldFid = (HFONT)SelectObject(fl_gc, fl_fontsize->fid);
    fl_fontsize->listbase = glGenLists(GENLISTSIZE);
    wglUseFontBitmaps(fl_gc, base, count, fl_fontsize->listbase+base);
    SelectObject(fl_gc, oldFid);
# endif

#endif // USE_OksiD_style_GL_font_selection
  }
  glListBase(fl_fontsize->listbase);
#endif // !__APPLE__
  gl_fontsize = fl_fontsize;
}

#if !defined(__APPLE__)
#if !(defined(USE_X11) || USE_XFT)
static void get_list(int r) {
  gl_fontsize->glok[r] = 1;
#if defined(USE_X11)
# if USE_XFT
// FIXME
# else
  unsigned int ii = r * 0x400;
  for (int i = 0; i < 0x400; i++) {
    XFontStruct *font = NULL;
    unsigned short id;
    fl_XGetUtf8FontAndGlyph(gl_fontsize->font, ii, &font, &id);
    if (font) glXUseXFont(font->fid, id, 1, gl_fontsize->listbase+ii);
    ii++;
   }
# endif
#elif defined(WIN32)
  unsigned int ii = r * 0x400;
  HFONT oldFid = (HFONT)SelectObject(fl_gc, gl_fontsize->fid);
  wglUseFontBitmapsW(fl_gc, ii, 0x400, gl_fontsize->listbase+ii);
  SelectObject(fl_gc, oldFid);
#else
#  error unsupported platform
#endif
} // get_list
#endif // !(defined(USE_X11) || USE_XFT)
#endif // !defined(__APPLE__)

void gl_remove_displaylist_fonts()
{
# if HAVE_GL

  // clear variables used mostly in fl_font
  fl_graphics_driver->font(0, 0);

  for (int j = 0 ; j < FL_FREE_FONT ; ++j)
  {
    Fl_Font_Descriptor* past = 0;
    Fl_Fontdesc* s    = fl_fonts + j ;
    Fl_Font_Descriptor* f    = s->first;
    while (f != 0) {
      if(f->listbase) {
        if(f == s->first) {
          s->first = f->next;
        }
        else {
          past->next = f->next;
        }

        // It would be nice if this next line was in a destructor somewhere
        glDeleteLists(f->listbase, GENLISTSIZE);

        Fl_Font_Descriptor* tmp = f;
        f = f->next;
        delete tmp;
      }
      else {
        past = f;
        f = f->next;
      }
    }
  }

#endif
}

#ifdef __APPLE__
static void gl_draw_textures(const char* str, int n);
#endif

/**
  Draws an array of n characters of the string in the current font
  at the current position.
 \see On the Mac OS X platform, see gl_texture_pile_height(int)
  */
void gl_draw(const char* str, int n) {
#ifdef __APPLE__
  gl_draw_textures(str, n);
#else
  static unsigned short *buf = NULL;
  static unsigned l = 0;
  unsigned wn = fl_utf8toUtf16(str, n, buf, l);
  if (wn >= l) {
    buf = (unsigned short*) realloc(buf, sizeof(unsigned short) * (wn + 1));
    l = wn + 1;
    wn = fl_utf8toUtf16(str, n, buf, l);
  }

#if !(defined(USE_X11) || USE_XFT)
  for (unsigned i = 0; i < wn; i++) {
    unsigned int r;
    r = (buf[i] & 0xFC00) >> 10;
    if (!gl_fontsize->glok[r]) get_list(r);
  }
#endif
  glCallLists(wn, GL_UNSIGNED_SHORT, buf);
#endif
}

/**
  Draws n characters of the string in the current font at the given position
 \see On the Mac OS X platform, see gl_texture_pile_height(int)
  */
void gl_draw(const char* str, int n, int x, int y) {
  glRasterPos2i(x, y);
  gl_draw(str, n);
}

/**
  Draws n characters of the string in the current font at the given position
 \see On the Mac OS X platform, see gl_texture_pile_height(int)
  */
void gl_draw(const char* str, int n, float x, float y) {
  glRasterPos2f(x, y);
  gl_draw(str, n);
}

/**
  Draws a nul-terminated string in the current font at the current position
 \see On the Mac OS X platform, see gl_texture_pile_height(int)
  */
void gl_draw(const char* str) {
  gl_draw(str, strlen(str));
}

/**
  Draws a nul-terminated string in the current font at the given position
 \see On the Mac OS X platform, see gl_texture_pile_height(int)
  */
void gl_draw(const char* str, int x, int y) {
  gl_draw(str, strlen(str), x, y);
}

/**
  Draws a nul-terminated string in the current font at the given position
 \see On the Mac OS X platform, see gl_texture_pile_height(int)
  */
void gl_draw(const char* str, float x, float y) {
  gl_draw(str, strlen(str), x, y);
}

static void gl_draw_invert(const char* str, int n, int x, int y) {
  glRasterPos2i(x, -y);
  gl_draw(str, n);
}

/**
  Draws a string formatted into a box, with newlines and tabs expanded,
  other control characters changed to ^X. and aligned with the edges or
  center. Exactly the same output as fl_draw().
  */
void gl_draw(
  const char* str, 	// the (multi-line) string
  int x, int y, int w, int h, 	// bounding box
  Fl_Align align) {
  fl_draw(str, x, -y-h, w, h, align, gl_draw_invert, NULL, 0);
}

/** Measure how wide and tall the string will be when drawn by the gl_draw() function */
void gl_measure(const char* str, int& x, int& y) {
  fl_measure(str,x,y,0);
}

/**
  Outlines the given rectangle with the current color.
  If Fl_Gl_Window::ortho() has been called, then the rectangle will
  exactly fill the given pixel rectangle.
  */
void gl_rect(int x, int y, int w, int h) {
  if (w < 0) {w = -w; x = x-w;}
  if (h < 0) {h = -h; y = y-h;}
  glBegin(GL_LINE_STRIP);
  glVertex2i(x+w-1, y+h-1);
  glVertex2i(x+w-1, y);
  glVertex2i(x, y);
  glVertex2i(x, y+h-1);
  glVertex2i(x+w, y+h-1);
  glEnd();
}

#if HAVE_GL_OVERLAY
extern uchar fl_overlay;
extern int fl_overlay_depth;
#endif

/**
  Sets the curent OpenGL color to an FLTK color.

  For color-index modes it will use fl_xpixel(c), which is only
  right if the window uses the default colormap!
  */
void gl_color(Fl_Color i) {
#if HAVE_GL_OVERLAY
#if defined(WIN32)
  if (fl_overlay && fl_overlay_depth) {
    if (fl_overlay_depth < 8) {
      // only black & white produce the expected colors.  This could
      // be improved by fixing the colormap set in Fl_Gl_Overlay.cxx
      int size = 1<<fl_overlay_depth;
      if (!i) glIndexi(size-2);
      else if (i >= size-2) glIndexi(size-1);
      else glIndexi(i);
    } else {
      glIndexi(i ? i : FL_GRAY_RAMP);
    }
    return;
  }
#else
  if (fl_overlay) {glIndexi(int(fl_xpixel(i))); return;}
#endif
#endif
  uchar red, green, blue;
  Fl::get_color(i, red, green, blue);
  glColor3ub(red, green, blue);
}

void gl_draw_image(const uchar* b, int x, int y, int w, int h, int d, int ld) {
  if (!ld) ld = w*d;
  glPixelStorei(GL_UNPACK_ROW_LENGTH, ld/d);
  glRasterPos2i(x,y);
  glDrawPixels(w,h,d<4?GL_RGB:GL_RGBA,GL_UNSIGNED_BYTE,(const ulong*)b);
}

#if defined(__APPLE__) || defined(FL_DOXYGEN)
/* Text drawing to an OpenGL scene under Mac OS X is implemented using textures, as recommended by Apple.
 This allows to use any font at any size, and any Unicode character.
 Some old Apple hardware doesn't implement the required GL_EXT_texture_rectangle extension.
 For these, glutStrokeString() is used to draw text. In that case, it's possible to vary text size,
 but not text font, and only ASCII characters can be drawn.
 */

static int gl_scale = 1; // set to 2 for high resolution Fl_Gl_Window
static int has_texture_rectangle = 0; // true means GL_EXT_texture_rectangle is available

#include <FL/glu.h>  // for gluUnProject() and gluCheckExtension()
#include <FL/glut.H> // for glutStrokeString() and glutStrokeLength()

// manages a fifo pile of pre-computed string textures
class gl_texture_fifo {
  friend void gl_draw_textures(const char *, int);
private:
  typedef struct { // information for a pre-computed texture
    GLuint texName; // its name
    char *utf8; //its text
    Fl_Font_Descriptor *fdesc; // its font
    int width; // its width
    float ratio; // used without rectangle texture
    int scale; // 1 or 2 for low/high resolution
  } data;
  data *fifo; // array of pile elements
  int size_; // pile height
  int current; // the oldest texture to have entered the pile
  int last; // pile top
  int textures_generated; // true after glGenTextures has been called
  void display_texture(int rank);
  int compute_texture(const char* str, int n);
  int already_known(const char *str, int n);
public:
  gl_texture_fifo(int max = 100); // 100 = default height of texture pile
  inline int size(void) {return size_; };
  ~gl_texture_fifo(void);
};

gl_texture_fifo::gl_texture_fifo(int max)
{
  size_ = max;
  last = current = -1;
  textures_generated = 0;
  fifo = (data*)calloc(size_, sizeof(data));
}

gl_texture_fifo::~gl_texture_fifo()
{
  for (int i = 0; i < size_; i++) {
    if (fifo[i].utf8) free(fifo[i].utf8);
    if (textures_generated) glDeleteTextures(1, &fifo[i].texName);
    }
  free(fifo);
}

// displays a pre-computed texture on the GL scene
void gl_texture_fifo::display_texture(int rank)
{
  //setup matrices
  GLint matrixMode;
  glGetIntegerv (GL_MATRIX_MODE, &matrixMode);
  glMatrixMode (GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity ();
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity ();
  float winw = gl_scale * Fl_Window::current()->w();
  float winh = gl_scale * Fl_Window::current()->h();
  // GL_COLOR_BUFFER_BIT for glBlendFunc, GL_ENABLE_BIT for glEnable / glDisable
  glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT);
  glDisable (GL_DEPTH_TEST); // ensure text is not removed by depth buffer test.
  glEnable (GL_BLEND); // for text fading
  glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // ditto
  glDisable(GL_LIGHTING);
  GLfloat pos[4];
  glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
  float R = 2;
  if (!has_texture_rectangle) {
    R *= fifo[rank].ratio;
  }
  glScalef (R/winw, R/winh, 1.0f);
  glTranslatef (-winw/R, -winh/R, 0.0f);
  if (has_texture_rectangle) {
    glEnable (GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture (GL_TEXTURE_RECTANGLE_ARB, fifo[rank].texName);
    GLint height;
    glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE_ARB, 0, GL_TEXTURE_HEIGHT, &height);
    CGRect bounds = CGRectMake (pos[0], pos[1] - gl_scale*fl_descent(), fifo[rank].width, height);
    //write the texture on screen
    glBegin (GL_QUADS);
    glTexCoord2f (0.0f, 0.0f); // draw lower left in world coordinates
    glVertex2f (bounds.origin.x, bounds.origin.y);

    glTexCoord2f (0.0f, height); // draw upper left in world coordinates
    glVertex2f (bounds.origin.x, bounds.origin.y + bounds.size.height);

    glTexCoord2f (fifo[rank].width, height); // draw upper right in world coordinates
    glVertex2f (bounds.origin.x + bounds.size.width, bounds.origin.y + bounds.size.height);

    glTexCoord2f (fifo[rank].width, 0.0f); // draw lower right in world coordinates
    glVertex2f (bounds.origin.x + bounds.size.width, bounds.origin.y);
    glEnd ();
  } else {
    glTranslatef(pos[0]*2/R, pos[1]*2/R, 0.0);
    glutStrokeString(GLUT_STROKE_ROMAN, (uchar*)fifo[rank].utf8);
  }
  glPopAttrib();

  // reset original matrices
  glPopMatrix(); // GL_MODELVIEW
  glMatrixMode (GL_PROJECTION);
  glPopMatrix();
  glMatrixMode (matrixMode);

  //set the raster position to end of string
  pos[0] += fifo[rank].width;
  GLdouble modelmat[16];
  glGetDoublev (GL_MODELVIEW_MATRIX, modelmat);
  GLdouble projmat[16];
  glGetDoublev (GL_PROJECTION_MATRIX, projmat);
  GLdouble objX, objY, objZ;
  GLint viewport[4];
  glGetIntegerv (GL_VIEWPORT, viewport);
  gluUnProject(pos[0], pos[1], pos[2], modelmat, projmat, viewport, &objX, &objY, &objZ);
  glRasterPos2d(objX, objY);
}

// pre-computes a string texture
int gl_texture_fifo::compute_texture(const char* str, int n)
{
  current = (current + 1) % size_;
  if (current > last) last = current;
  if ( fifo[current].utf8 ) free(fifo[current].utf8);
  fifo[current].utf8 = (char *)malloc(n + 1);
  memcpy(fifo[current].utf8, str, n);
  fifo[current].utf8[n] = 0;
  fl_graphics_driver->font_descriptor(gl_fontsize);
  int h;
  fl_measure(fifo[current].utf8, fifo[current].width, h, 0);
  fifo[current].width *= gl_scale;
  h *= gl_scale;
  fifo[current].scale = gl_scale;
  fifo[current].fdesc = gl_fontsize;
  if (has_texture_rectangle) {
    //write str to a bitmap just big enough
    CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
    void *base = NULL;
    if (fl_mac_os_version < 100600) base = calloc(4*fifo[current].width, h);
    CGContextRef save_gc = fl_gc;
    fl_gc = CGBitmapContextCreate(base, fifo[current].width, h, 8, fifo[current].width*4, lut,
                                  (CGBitmapInfo)(kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host));
    CGColorSpaceRelease(lut);
    GLfloat colors[4];
    glGetFloatv(GL_CURRENT_COLOR, colors);
    fl_color((uchar)(colors[0]*255), (uchar)(colors[1]*255), (uchar)(colors[2]*255));
    CGContextTranslateCTM(fl_gc, 0, h - gl_scale*fl_descent());
    CGContextScaleCTM(fl_gc, gl_scale, gl_scale);
    fl_draw(str, n, 0, 0);
    //put this bitmap in a texture
    glPushAttrib(GL_TEXTURE_BIT);
    glBindTexture (GL_TEXTURE_RECTANGLE_ARB, fifo[current].texName);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, fifo[current].width);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA8, fifo[current].width, h, 0,  GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV, CGBitmapContextGetData(fl_gc));
    glPopAttrib();
    CGContextRelease(fl_gc);
    fl_gc = save_gc;
    if (base) free(base);
  } else {
    fifo[current].ratio = float(fifo[current].width)/glutStrokeLength(GLUT_STROKE_ROMAN, (uchar*)fifo[current].utf8);
  }
  return current;
}

// returns rank of pre-computed texture for a string if it exists
int gl_texture_fifo::already_known(const char *str, int n)
{
  int rank;
  for ( rank = 0; rank <= last; rank++) {
    if ( memcmp(str, fifo[rank].utf8, n) == 0 && fifo[rank].utf8[n] == 0 &&
      fifo[rank].fdesc == gl_fontsize && fifo[rank].scale == gl_scale) return rank;
  }
  return -1;
}

static gl_texture_fifo *gl_fifo = NULL; // points to the texture pile class instance

// draws a utf8 string using pre-computed texture if available
static void gl_draw_textures(const char* str, int n)
{
  Fl_Gl_Window *gwin = Fl_Window::current()->as_gl_window();
  gl_scale = (gwin ? gwin->pixels_per_unit() : 1);
  //fprintf(stderr,"gl_scale=%d\n",gl_scale);
  if (! gl_fifo) gl_fifo = new gl_texture_fifo();
  if (!gl_fifo->textures_generated) {
    has_texture_rectangle = gluCheckExtension((GLubyte*)"GL_EXT_texture_rectangle", glGetString(GL_EXTENSIONS));
    if (has_texture_rectangle) for (int i = 0; i < gl_fifo->size_; i++) glGenTextures(1, &(gl_fifo->fifo[i].texName));
    gl_fifo->textures_generated = 1;
  }
  int rank = gl_fifo->already_known(str, n);
  if (rank == -1) {
    rank = gl_fifo->compute_texture(str, n);
  }
  gl_fifo->display_texture(rank);
}

/** \addtogroup group_macosx
 @{ */

/**
 \brief Returns the current height of the pile of pre-computed string textures
 *
 The default value is 100
 */
int gl_texture_pile_height(void)
{
  if (! gl_fifo) gl_fifo = new gl_texture_fifo();
  return gl_fifo->size();
}

/**
 \brief Changes the height of the pile of pre-computed string textures
 *
 Strings that are often re-displayed can be processed much faster if
 this pile is set high enough to hold all of them.
 \param max Height of the texture pile
 */
void gl_texture_pile_height(int max)
{
  if (gl_fifo) delete gl_fifo;
  gl_fifo = new gl_texture_fifo(max);
}

/** @} */

void gl_texture_reset()
{
  if (gl_fifo) gl_texture_pile_height(gl_texture_pile_height());
}
#endif // __APPLE__

#endif // HAVE_GL

//
// End of "$Id$".
//
