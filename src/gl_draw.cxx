//
// "$Id$"
//
// OpenGL text drawing support routines for the Fast Light Tool Kit (FLTK).
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

/* Note about implementing GL text support for a new platform
 
 1) if the GL_EXT_texture_rectangle (a.k.a. GL_ARB_texture_rectangle) GL extension
 is available, no platform-specific code is needed, besides support for fl_draw() and Fl_Image_Surface for the platform.
 
 2) if the GL_EXT_texture_rectangle GL extension is not available,
 a rudimentary support through GLUT is obtained without any platform-specific code.

 3) A more elaborate support can be obtained implementing
 get_list(), gl_bitmap_font() and draw_string_legacy() for the platform's Fl_XXX_Gl_Window_Driver.
 */

#include "config_lib.h"
#if defined(FL_PORTING)
#  pragma message "FL_PORTING: implement OpenGL text rendering here"
#endif // defined(FL_PORTING)

#if HAVE_GL || defined(FL_DOXYGEN)

#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/gl_draw.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Gl_Window_Driver.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/glu.h>  // for gluUnProject()
#include <FL/glut.H> // for glutStrokeString() and glutStrokeLength()

#ifndef GL_TEXTURE_RECTANGLE_ARB
#  define GL_TEXTURE_RECTANGLE_ARB 0x84F5
#endif


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
static int has_texture_rectangle = 0; // true means GL_EXT_texture_rectangle is available

extern float gl_start_scale; // in gl_start.cxx

/**
  Sets the current OpenGL font to the same font as calling fl_font().
 \see Fl::draw_GL_text_with_textures(int val)
  */
void  gl_font(int fontid, int size) {
  static bool once = true;
  if (once) {
    once = false;
    if (Fl::draw_GL_text_with_textures()) {
      // For the font texture pile to work, we need a texture rectangle extension, so check for
      // one here. First we check for GL_EXT_texture_rectangle and if that fails we try
      // for GL_ARB_texture_rectangle instead. If that also fails, we fall back to the
      // legacy methods used by fltk-1.3 and earlier.
      has_texture_rectangle = (strstr((const char*)glGetString(GL_EXTENSIONS), "GL_EXT_texture_rectangle") != NULL);
      if (!has_texture_rectangle) has_texture_rectangle =
        (strstr((const char*)glGetString(GL_EXTENSIONS), "GL_ARB_texture_rectangle") != NULL);
      Fl::draw_GL_text_with_textures(has_texture_rectangle);
    }
  }
  fl_font(fontid, size);
  Fl_Font_Descriptor *fl_fontsize = fl_graphics_driver->font_descriptor();
  if (!has_texture_rectangle) Fl_Gl_Window_Driver::global()->gl_bitmap_font(fl_fontsize);
  gl_fontsize = fl_fontsize;
}

void gl_remove_displaylist_fonts()
{
  // clear variables used mostly in fl_font
  fl_graphics_driver->font(0, 0);

  for (int j = 0 ; j < FL_FREE_FONT ; ++j)
  {
    Fl_Font_Descriptor* past = 0;
    Fl_Font_Descriptor** s_first = Fl_Gl_Window_Driver::global()->fontnum_to_fontdescriptor(j);
    Fl_Font_Descriptor* f = *s_first;
    while (f != 0) {
      if(f->listbase) {
        if(f == *s_first) {
          *s_first = f->next;
        }
        else {
          past->next = f->next;
        }
        // It would be nice if this next line was in a destructor somewhere
        glDeleteLists(f->listbase, Fl_Gl_Window_Driver::global()->genlistsize());
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
}


/**
  Draws an array of n characters of the string in the current font at the current position.
 \see  gl_texture_pile_height(int)
  */
void gl_draw(const char* str, int n) {
  if (has_texture_rectangle)  Fl_Gl_Window_Driver::draw_string_with_texture(str, n);
  else Fl_Gl_Window_Driver::global()->draw_string_legacy(str, n);
}


/**
  Draws n characters of the string in the current font at the given position
 \see  gl_texture_pile_height(int)
  */
void gl_draw(const char* str, int n, int x, int y) {
  glRasterPos2i(x, y);
  gl_draw(str, n);
}

/**
  Draws n characters of the string in the current font at the given position
 \see  gl_texture_pile_height(int)
  */
void gl_draw(const char* str, int n, float x, float y) {
  glRasterPos2f(x, y);
  gl_draw(str, n);
}

/**
  Draws a nul-terminated string in the current font at the current position
 \see  gl_texture_pile_height(int)
  */
void gl_draw(const char* str) {
  gl_draw(str, strlen(str));
}

/**
  Draws a nul-terminated string in the current font at the given position
 \see  gl_texture_pile_height(int)
  */
void gl_draw(const char* str, int x, int y) {
  gl_draw(str, strlen(str), x, y);
}

/**
  Draws a nul-terminated string in the current font at the given position
 \see  gl_texture_pile_height(int)
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
  Fl_Align align)
{
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

void gl_draw_image(const uchar* b, int x, int y, int w, int h, int d, int ld) {
  if (!ld) ld = w*d;
  glPixelStorei(GL_UNPACK_ROW_LENGTH, ld/d);
  glRasterPos2i(x,y);
  glDrawPixels(w,h,d<4?GL_RGB:GL_RGBA,GL_UNSIGNED_BYTE,(const ulong*)b);
}


/**
  Sets the curent OpenGL color to an FLTK color.

  For color-index modes it will use fl_xpixel(c), which is only
  right if the window uses the default colormap!
  */
void gl_color(Fl_Color i) {
  if (Fl_Gl_Window_Driver::global()->overlay_color(i)) return;
  uchar red, green, blue;
  Fl::get_color(i, red, green, blue);
  glColor3ub(red, green, blue);
}


#if ! defined(FL_DOXYGEN) // do not want too much of the gl_texture_fifo internals in the documentation

/* Implement the gl_texture_fifo mechanism:
 Strings to be drawn are memorized in a fifo pile (which max size can
 be increased calling gl_texture_pile_height(int)).
 Each pile element contains the string, the font, the GUI scale, and
 an image of the string in the form of a GL texture.
 Strings are drawn in 2 steps:
    1) compute the texture for that string if it was not computed before;
    2) draw the texture using the current GL color.
*/

static float gl_scale = 1; // scaling factor between FLTK and GL drawing units: GL = FLTK * gl_scale

// manages a fifo pile of pre-computed string textures
class gl_texture_fifo {
  friend class Fl_Gl_Window_Driver;
private:
  typedef struct { // information for a pre-computed texture
    GLuint texName; // its name
    char *utf8; //its text
    Fl_Font_Descriptor *fdesc; // its font
    float scale; // scaling factor of the GUI
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

// returns rank of pre-computed texture for a string if it exists
int gl_texture_fifo::already_known(const char *str, int n)
{
  int rank;
  for ( rank = 0; rank <= last; rank++) {
    if ( (memcmp(str, fifo[rank].utf8, n) == 0) && (fifo[rank].utf8[n] == 0) &&
      (fifo[rank].fdesc == gl_fontsize) && (fifo[rank].scale == gl_scale) ) {
        return rank;
    }
  }
  return -1; // means no texture exists yet for that string
}

static gl_texture_fifo *gl_fifo = NULL; // points to the texture pile class instance

void gl_texture_reset()
{
  if (gl_fifo) gl_texture_pile_height(gl_texture_pile_height());
}


// Cross-platform implementation of the texture mechanism for text rendering
// using textures with the alpha channel only.

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
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_LIGHTING);
  GLfloat pos[4];
  glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
  if (gl_start_scale != 1) { // using gl_start() / gl_finish()
    pos[0] /= gl_start_scale;
    pos[1] /= gl_start_scale;
  }
  
  float R = 2;
  glScalef (R/winw, R/winh, 1.0f);
  glTranslatef (-winw/R, -winh/R, 0.0f);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, fifo[rank].texName);
  GLint width, height;
  glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE_ARB, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE_ARB, 0, GL_TEXTURE_HEIGHT, &height);
  //write the texture on screen
  glBegin (GL_QUADS);
  float ox = pos[0];
  float oy = pos[1] + height - gl_scale * fl_descent();
  glTexCoord2f (0.0f, 0.0f); // draw lower left in world coordinates
  glVertex2f (ox, oy);
  glTexCoord2f (0.0f, height); // draw upper left in world coordinates
  glVertex2f (ox, oy - height);
  glTexCoord2f (width, height); // draw upper right in world coordinates
  glVertex2f (ox + width, oy - height);
  glTexCoord2f (width, 0.0f); // draw lower right in world coordinates
  glVertex2f (ox + width, oy);
  glEnd ();
  glPopAttrib();
  
  // reset original matrices
  glPopMatrix(); // GL_MODELVIEW
  glMatrixMode (GL_PROJECTION);
  glPopMatrix();
  glMatrixMode (matrixMode);
  
  //set the raster position to end of string
  pos[0] += width;
  GLdouble modelmat[16];
  glGetDoublev (GL_MODELVIEW_MATRIX, modelmat);
  GLdouble projmat[16];
  glGetDoublev (GL_PROJECTION_MATRIX, projmat);
  GLdouble objX, objY, objZ;
  GLint viewport[4];
  glGetIntegerv (GL_VIEWPORT, viewport);
  gluUnProject(pos[0], pos[1], pos[2], modelmat, projmat, viewport, &objX, &objY, &objZ);

  if (gl_start_scale != 1) { // using gl_start() / gl_finish()
    objX *= gl_start_scale;
    objY *= gl_start_scale;
  }
  glRasterPos2d(objX, objY);
} // display_texture


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
  int w, h;
  w = fl_width(fifo[current].utf8, n) * gl_scale;
  // Hack - make w be aligned
  w = (w + 3) & 0xFFFFFFC;
  h = fl_height() * gl_scale;
  
  fifo[current].scale = gl_scale;
  fifo[current].fdesc = gl_fontsize;
  char *txt_buf = Fl_Gl_Window_Driver::global()->alpha_mask_for_string(str, n, w, h);
  
  // put the bitmap in an alpha-component-only texture
  glPushAttrib(GL_TEXTURE_BIT);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, fifo[current].texName);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //glPixelStorei(GL_UNPACK_ROW_LENGTH, w);
  // GL_ALPHA8 is defined in GL/gl.h of X11 and of MinGW32 and of MinGW64 and of OpenGL.framework for MacOS
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_ALPHA8, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, txt_buf);
  /* For the record: texture construction if an alpha-only-texture is not possible
   glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA8, w, h, 0, GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV, rgba_buf);
   and also, replace GL_SRC_ALPHA by GL_ONE in glBlendFunc() call of display_texture()
   */
  delete[] txt_buf; // free the buffer now we have copied it into the Gl texture
  glPopAttrib();
  return current;
}

#endif  // ! defined(FL_DOXYGEN)

/**
 Returns the current maximum height of the pile of pre-computed string textures.
 The default value is 100
 \see Fl::draw_GL_text_with_textures(int)
 */
int gl_texture_pile_height(void)
{
  if (! gl_fifo) gl_fifo = new gl_texture_fifo();
  return gl_fifo->size();
}

/**
 Changes the maximum height of the pile of pre-computed string textures

 Strings that are often re-displayed can be processed much faster if
 this pile is set high enough to hold all of them.
 \param max Maximum height of the texture pile
 \see Fl::draw_GL_text_with_textures(int)
*/
void gl_texture_pile_height(int max)
{
  if (gl_fifo) delete gl_fifo;
  gl_fifo = new gl_texture_fifo(max);
}


/**
  * @cond DriverDev
  * @addtogroup DriverDeveloper
  * @{
  */

void Fl_Gl_Window_Driver::draw_string_legacy(const char* str, int n)
{
  draw_string_legacy_glut(str,  n);
}


/** draws a utf8 string using an OpenGL texture */
void Fl_Gl_Window_Driver::draw_string_with_texture(const char* str, int n)
{
  Fl_Gl_Window *gwin = Fl_Window::current()->as_gl_window();
  gl_scale = (gwin ? gwin->pixels_per_unit() : 1);
  if (!gl_fifo) gl_fifo = new gl_texture_fifo();
  if (!gl_fifo->textures_generated) {
    if (has_texture_rectangle) for (int i = 0; i < gl_fifo->size_; i++) glGenTextures(1, &(gl_fifo->fifo[i].texName));
    gl_fifo->textures_generated = 1;
  }
  int index = gl_fifo->already_known(str, n);
  if (index == -1) {
    index = gl_fifo->compute_texture(str, n);
  }
  gl_fifo->display_texture(index);
}


char *Fl_Gl_Window_Driver::alpha_mask_for_string(const char *str, int n, int w, int h)
{
  // write str to a bitmap that is just big enough
  // create an Fl_Image_Surface object
  Fl_Image_Surface *image_surface = new Fl_Image_Surface(w, h);
  Fl_Font fnt = fl_font(); // get the current font
  // direct all further graphics requests to the image
  Fl_Surface_Device::push_current(image_surface);
  // fill the background with black, which we will interpret as transparent
  fl_color(0,0,0);
  fl_rectf(0, 0, w, h);
  // set up the text colour as white, which we will interpret as opaque
  fl_color(255,255,255);
  // Fix the font scaling
  fl_font (fnt, gl_fontsize->size); // resize "fltk" font to current GL view scaling
  int desc = fl_descent();
  // Render the text to the buffer
  fl_draw(str, n, 0, h - desc);
  // get the resulting image
  Fl_RGB_Image* image = image_surface->image();
  // direct graphics requests back to previous state
  Fl_Surface_Device::pop_current();
  delete image_surface;
  // This gives us an RGB rendering of the text. We build an alpha channel from that.
  char *txt_buf = new char [w * h];
  for (int idx = 0; idx < w * h; ++idx)
  { // Fake up the alpha component using the green component's value
    txt_buf[idx] = image->array[idx * 3 + 1];
  }
  delete image;
  return txt_buf;
}


// platform-independent, no-texture GL text drawing procedure
// when Fl_XXX_Gl_Window_Driver::get_list() and gl_bitmap_font() are implemented
void Fl_Gl_Window_Driver::draw_string_legacy_get_list(const char* str, int n) {
  static unsigned short *buf = NULL;
  static unsigned l = 0;
  unsigned wn = fl_utf8toUtf16(str, n, buf, l);
  if (wn >= l) {
    buf = (unsigned short*) realloc(buf, sizeof(unsigned short) * (wn + 1));
    l = wn + 1;
    wn = fl_utf8toUtf16(str, n, buf, l);
  }
  int size = 0;
  if (gl_start_scale != 1) { // using gl_start() / gl_finish()
    size = fl_graphics_driver->font_descriptor()->size;
    gl_font(fl_font(), size * gl_start_scale);
  }
  for (unsigned i = 0; i < wn; i++) {
    unsigned int r;
    r = (buf[i] & 0xFC00) >> 10;
    get_list(gl_fontsize, r);
  }
  glCallLists(wn, GL_UNSIGNED_SHORT, buf);
  if (gl_start_scale != 1) { // using gl_start() / gl_finish()
    gl_font(fl_font(), size);
  }
}

/* Platform-independent, no-texture, GL text drawing procedure when there's no OS support whatsoever:
 glutStrokeString() is used to draw text. It's possible to vary text size, but not text font,
 and only ASCII characters can be drawn.
 */
void Fl_Gl_Window_Driver::draw_string_legacy_glut(const char* str, int n)
{
  uchar *str_nul = new uchar[n + 1];
  int m = 0;
  for (int i = 0; i < n; i++) {
    if ((uchar)str[i] < 128) str_nul[m++] = str[i];
  }
  str_nul[m] = 0;
  n = m;
  Fl_Surface_Device::push_current(Fl_Display_Device::display_device());
  fl_graphics_driver->font_descriptor(gl_fontsize);
  Fl_Gl_Window *gwin = Fl_Window::current()->as_gl_window();
  gl_scale = (gwin ? gwin->pixels_per_unit() : 1);
  float ratio = fl_width((char*)str_nul, n) * gl_scale/glutStrokeLength(GLUT_STROKE_ROMAN, str_nul);
  Fl_Surface_Device::pop_current();
  
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
  
  GLfloat pos[4];
  glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
  if (gl_start_scale != 1) { // using gl_start() / gl_finish()
    pos[0] /= gl_start_scale;
    pos[1] /= gl_start_scale;
  }
  float R = 2 * ratio;
  glScalef (R/winw, R/winh, 1.0f);
  glTranslatef (-winw/R, -winh/R, 0.0f);
  glTranslatef(pos[0]*2/R, pos[1]*2/R, 0.0);
  glutStrokeString(GLUT_STROKE_ROMAN, str_nul);
  float width = fl_width((char*)str_nul);
  delete[] str_nul;
  glPopAttrib();
  // reset original matrices
  glPopMatrix(); // GL_MODELVIEW
  glMatrixMode (GL_PROJECTION);
  glPopMatrix();
  glMatrixMode (matrixMode);
  //set the raster position to end of string
  pos[0] += width;
  GLdouble modelmat[16];
  glGetDoublev (GL_MODELVIEW_MATRIX, modelmat);
  GLdouble projmat[16];
  glGetDoublev (GL_PROJECTION_MATRIX, projmat);
  GLdouble objX, objY, objZ;
  GLint viewport[4];
  glGetIntegerv (GL_VIEWPORT, viewport);
  gluUnProject(pos[0], pos[1], pos[2], modelmat, projmat, viewport, &objX, &objY, &objZ);
  if (gl_start_scale != 1) { // using gl_start() / gl_finish()
    objX *= gl_start_scale;
    objY *= gl_start_scale;
  }
  glRasterPos2d(objX, objY);
}


#if  defined(FL_CFG_GFX_XLIB)
#  include "drivers/Xlib/Fl_Font.H"
#  include <FL/platform.H>
#  include <GL/glx.h>

void Fl_X11_Gl_Window_Driver::draw_string_legacy(const char* str, int n) {
  draw_string_legacy_get_list(str, n);
}

int Fl_X11_Gl_Window_Driver::genlistsize() {
#if USE_XFT
  return 256;
#else
  return 0x10000;
#endif
}

void Fl_X11_Gl_Window_Driver::gl_bitmap_font(Fl_Font_Descriptor *fl_fontsize) {
  /* This method should ONLY be triggered if our GL font texture pile mechanism
   * is not working on this platform. This code might not reliably render glyphs
   * from higher codepoints. */
  if (!fl_fontsize->listbase) {
#if USE_XFT
    /* Ideally, for XFT, we need a glXUseXftFont implementation here... But we
     * do not have such a thing. Instead, we try to find a legacy Xlib font that
     * matches the current XFT font and use that.
     * Ideally, we never come here - we hope the texture pile implementation
     * will work correctly so that XFT can render the face directly without the
     * need for this workaround. */
    XFontStruct *font = fl_xfont.value();
    int base = font->min_char_or_byte2;
    int count = font->max_char_or_byte2 - base + 1;
    fl_fontsize->listbase = glGenLists(genlistsize());
    glXUseXFont(font->fid, base, count, fl_fontsize->listbase+base);
#else
    /* Not using XFT to render text - the legacy Xlib fonts can usually be rendered
     * directly by using glXUseXFont mechanisms. */
    fl_fontsize->listbase = glGenLists(genlistsize());
#endif // !USE_XFT
  }
  glListBase(fl_fontsize->listbase);
}


void Fl_X11_Gl_Window_Driver::get_list(Fl_Font_Descriptor *fd, int r) {
  Fl_Xlib_Font_Descriptor *gl_fd = (Fl_Xlib_Font_Descriptor*)fd;
  if (gl_fd->glok[r]) return;
  gl_fd->glok[r] = 1;
# if USE_XFT
  /* We hope not to come here: We hope that any system using XFT will also
   * have sufficient GL capability to support our font texture pile mechansim,
   * allowing XFT to render the face directly. */
  // Face already set by gl_bitmap_font in this case.
# else
  unsigned int ii = r * 0x400;
  for (int i = 0; i < 0x400; i++) {
    XFontStruct *font = NULL;
    unsigned short id;
    fl_XGetUtf8FontAndGlyph(gl_fd->font, ii, &font, &id);
    if (font) glXUseXFont(font->fid, id, 1, gl_fd->listbase+ii);
    ii++;
  }
# endif
}

#if !USE_XFT
Fl_Font_Descriptor** Fl_X11_Gl_Window_Driver::fontnum_to_fontdescriptor(int fnum) {
  Fl_Xlib_Fontdesc *s = ((Fl_Xlib_Fontdesc*)fl_fonts) + fnum;
  return &(s->first);
}
#endif

#if HAVE_GL_OVERLAY
extern uchar fl_overlay;
int Fl_X11_Gl_Window_Driver::overlay_color(Fl_Color i) {
  if (fl_overlay) {glIndexi(int(fl_xpixel(i))); return 1;}
  return 0;
}
#endif // HAVE_GL_OVERLAY

#endif // FL_CFG_GFX_XLIB


#if defined(FL_CFG_GFX_GDI)
#  include "drivers/GDI/Fl_Font.H"

void Fl_WinAPI_Gl_Window_Driver::draw_string_legacy(const char* str, int n) {
  draw_string_legacy_get_list(str, n);
}

int Fl_WinAPI_Gl_Window_Driver::genlistsize() {
  return 0x10000;
}

void Fl_WinAPI_Gl_Window_Driver::gl_bitmap_font(Fl_Font_Descriptor *fl_fontsize) {
  if (!fl_fontsize->listbase) {
    fl_fontsize->listbase = glGenLists(genlistsize());
  }
  glListBase(fl_fontsize->listbase);
}

void Fl_WinAPI_Gl_Window_Driver::get_list(Fl_Font_Descriptor *fd, int r) {
  Fl_GDI_Font_Descriptor* gl_fd = (Fl_GDI_Font_Descriptor*)fd;
  if (gl_fd->glok[r]) return;
  gl_fd->glok[r] = 1;
  unsigned int ii = r * 0x400;
  HFONT oldFid = (HFONT)SelectObject((HDC)fl_graphics_driver->gc(), gl_fd->fid);
  wglUseFontBitmapsW((HDC)fl_graphics_driver->gc(), ii, 0x400, gl_fd->listbase+ii);
  SelectObject((HDC)fl_graphics_driver->gc(), oldFid);
}

#if HAVE_GL_OVERLAY
extern uchar fl_overlay;
extern int fl_overlay_depth;
int Fl_WinAPI_Gl_Window_Driver::overlay_color(Fl_Color i) {
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
    return 1;
  }
  return 0;
}
#endif // HAVE_GL_OVERLAY
#endif // FL_CFG_GFX_GDI


#if defined(FL_CFG_GFX_QUARTZ)
#  include "drivers/Quartz/Fl_Font.H"

#  if !defined(kCGBitmapByteOrder32Host) // doc says available 10.4 but some 10.4 don't have it
#   define kCGBitmapByteOrder32Host 0
#  endif // !defined(kCGBitmapByteOrder32Host)

/* Some old Apple hardware doesn't implement the GL_EXT_texture_rectangle extension.
 For it, draw_string_legacy_glut() is used to draw text.
 */

char *Fl_Cocoa_Gl_Window_Driver::alpha_mask_for_string(const char *str, int n, int w, int h)
{
  // write str to a bitmap just big enough
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  void *base = NULL;
  if (fl_mac_os_version < 100600) base = calloc(4*w, h);
  void* save_gc = fl_graphics_driver->gc();
  CGContextRef gc = CGBitmapContextCreate(base, w, h, 8, w*4, lut,
                                          (CGBitmapInfo)(kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host));
  CGColorSpaceRelease(lut);
  fl_graphics_driver->gc(gc);
  fl_color(FL_WHITE);
  CGContextScaleCTM(gc, gl_scale, -gl_scale);
  CGContextTranslateCTM(gc, 0,  -fl_descent());
  fl_draw(str, n, 0, 0);
  // get the alpha channel only of the bitmap
  char *txt_buf = new char[w*h], *r = txt_buf, *q;
  q = (char*)CGBitmapContextGetData(gc);
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      *r++ = *(q+3);
      q += 4;
    }
  }
  CGContextRelease(gc);
  fl_graphics_driver->gc(save_gc);
  if (base) free(base);
  return txt_buf;
}

#endif // FL_CFG_GFX_QUARTZ

/**
 * @}
 * @endcond
 */

#endif // HAVE_GL || defined(FL_DOXYGEN)

//
// End of "$Id$".
//
