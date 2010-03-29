//
// "$Id$"
//
// OpenGL drawing support routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

// Functions from <FL/gl.h>
// See also Fl_Gl_Window and gl_start.cxx

#include "flstring.h"
#if HAVE_GL

#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/x.H>
#include <FL/fl_draw.H>
#include "Fl_Gl_Choice.H"
#include "Fl_Font.H"
#include <FL/fl_utf8.h>

#if !defined(WIN32) && !defined(__APPLE__)
#include <FL/Xutf8.h>
#endif

#if USE_XFT
extern XFontStruct* fl_xxfont();
#endif // USE_XFT

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

#ifndef __APPLE__
#  define USE_OksiD_style_GL_font_selection 1  // Most hosts except OSX
#else
#  undef USE_OksiD_style_GL_font_selection  // OSX
#endif

#if USE_XFT
#  undef USE_OksiD_style_GL_font_selection  // turn this off for XFT also
#endif

/**
  Sets the current OpenGL font to the same font as calling fl_font()
  */
void  gl_font(int fontid, int size) {
  fl_font(fontid, size);
  if (!fl_fontsize->listbase) {

#ifdef  USE_OksiD_style_GL_font_selection
    fl_fontsize->listbase = glGenLists(0x10000);
#else // Fltk-1.1.8 style GL font selection

#if defined (USE_X11) // X-windows options follow, either XFT or "plain" X
#  if USE_XFT // XFT case
#    warning We really need a glXUseXftFont implementation here...
//    fl_xfont = fl_xxfont();
    XFontStruct *font = fl_xxfont();
    int base = font->min_char_or_byte2;
    int count = font->max_char_or_byte2-base+1;
    fl_fontsize->listbase = glGenLists(256);
    glXUseXFont(font->fid, base, count, fl_fontsize->listbase+base);
#  else // plain X
#    warning GL font selection is basically wrong here
/* OksiD has a fairly sophisticated scheme for storing multiple X fonts in a XUtf8FontStruct,
 * then sorting through them at draw time (for normal X rendering) to find which one can
 * render the current glyph... But for now, just use the first font in the list for GL...
 */
    XFontStruct *tmp_font = fl_fontsize->font->fonts[0];  // this is certainly wrong!
    int base = tmp_font->min_char_or_byte2;
    int count = tmp_font->max_char_or_byte2-base+1;
    fl_fontsize->listbase = glGenLists(256);
    glXUseXFont(tmp_font->fid, base, count, fl_fontsize->listbase+base);
#  endif // USE_XFT
# elif defined(WIN32)
    int base = fl_fontsize->metr.tmFirstChar;
    int count = fl_fontsize->metr.tmLastChar-base+1;
    HFONT oldFid = (HFONT)SelectObject(fl_gc, fl_fontsize->fid);
    fl_fontsize->listbase = glGenLists(256);
    wglUseFontBitmaps(fl_gc, base, count, fl_fontsize->listbase+base);
    SelectObject(fl_gc, oldFid);
# elif defined(__APPLE_QUARTZ__)
    /* FIXME: no OpenGL Font Selection in Cocoa!
//AGL is not supported for use in 64-bit applications:
//http://developer.apple.com/mac/library/documentation/Carbon/Conceptual/Carbon64BitGuide/OtherAPIChanges/OtherAPIChanges.html
    short font, face, size;
    uchar fn[256];
    fn[0]=strlen(fl_fontsize->q_name);
    strcpy((char*)(fn+1), fl_fontsize->q_name);
    GetFNum(fn, &font);
    face = 0;
    size = fl_fontsize->size;
    fl_fontsize->listbase = glGenLists(256);
	aglUseFont(aglGetCurrentContext(), font, face,
               size, 0, 256, fl_fontsize->listbase);
     */
# else 
#   error unsupported platform
# endif

#endif // USE_OksiD_style_GL_font_selection

  }
  gl_fontsize = fl_fontsize;
#ifndef __APPLE_COCOA__
  glListBase(fl_fontsize->listbase);
#endif
}

#ifndef __APPLE__
// The OSX build does not use this at present... It probbaly should, though...
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
    XGetUtf8FontAndGlyph(gl_fontsize->font, ii, &font, &id);
    if (font) glXUseXFont(font->fid, id, 1, gl_fontsize->listbase+ii);
    ii++;
   }
# endif
#elif defined(WIN32)
  unsigned int ii = r * 0x400;
  HFONT oldFid = (HFONT)SelectObject(fl_gc, gl_fontsize->fid);
  wglUseFontBitmapsW(fl_gc, ii, ii + 0x03ff, gl_fontsize->listbase+ii);
  SelectObject(fl_gc, oldFid);
#elif defined(__APPLE_QUARTZ__)
// FIXME:
#else
#  error unsupported platform
#endif
} // get_list
#endif

void gl_remove_displaylist_fonts()
{
# if HAVE_GL

  // clear variables used mostly in fl_font
  fl_font_ = 0;
  fl_size_ = 0;

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

        // It would be nice if this next line was in a desctructor somewhere
        glDeleteLists(f->listbase, 256);

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

/**
  Draws an array of n characters of the string in the current font
  at the current position.
  */
#ifdef __APPLE__
static void gl_draw_cocoa(const char* str, int n);
#endif

void gl_draw(const char* str, int n) {
#ifdef __APPLE__  
  gl_draw_cocoa(str, n);  
#else
  static xchar *buf = NULL;
  static int l = 0;
//  if (n > l) {
//    buf = (xchar*) realloc(buf, sizeof(xchar) * (n + 20));
//    l = n + 20;
//  }
//  n = fl_utf2unicode((const unsigned char*)str, n, buf);

  int wn = fl_utf8toUtf16(str, n, (unsigned short*)buf, l);
  if(wn >= l) {
    buf = (xchar*) realloc(buf, sizeof(xchar) * (wn + 1));
    l = wn + 1;
    wn = fl_utf8toUtf16(str, n, (unsigned short*)buf, l);
  }
  n = wn;

  int i;
  for (i = 0; i < n; i++) {
    unsigned int r;
    r = (str[i] & 0xFC00) >> 10;
    if (!gl_fontsize->glok[r]) get_list(r);
  }
  glCallLists(n, GL_UNSIGNED_SHORT, buf);
#endif
}

/**
  Draws n characters of the string in the current font at the given position 
  */
void gl_draw(const char* str, int n, int x, int y) {
  glRasterPos2i(x, y);
  gl_draw(str, n);
}

/**
  Draws n characters of the string in the current font at the given position 
  */
void gl_draw(const char* str, int n, float x, float y) {
  glRasterPos2f(x, y);
  gl_draw(str, n);
}

/**
  Draws a nul-terminated string in the current font at the current position
  */
void gl_draw(const char* str) {
  gl_draw(str, strlen(str));
}

/**
  Draws a nul-terminated string in the current font at the given position 
  */
void gl_draw(const char* str, int x, int y) {
  gl_draw(str, strlen(str), x, y);
}

/**
  Draws a nul-terminated string in the current font at the given position 
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
  fl_draw(str, x, -y-h, w, h, align, gl_draw_invert);
}

void gl_measure(const char* str, int& x, int& y) {fl_measure(str,x,y);}

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

#ifdef __APPLE__

#include <FL/glu.h>

static void gl_draw_cocoa(const char* str, int n) 
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
  float winw = Fl_Window::current()->w();
  float winh = Fl_Window::current()->h();
  glScalef (2.0f / winw, 2.0f /  winh, 1.0f);
  glTranslatef (-winw / 2.0f, -winh / 2.0f, 0.0f);
//write str to a bitmap just big enough  
  int w = 0, h = 0;
  fl_measure(str, w, h, 0);
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  void *base = calloc(4*w, h);
  if(base == NULL) return;
  fl_gc = CGBitmapContextCreate(base, w, h, 8, w*4, lut, kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(lut);
  fl_fontsize = gl_fontsize;
  fl_draw(str, 0, h - fl_descent());
//put this bitmap in a texture  
  static GLuint texName = 0;
  glPushAttrib(GL_TEXTURE_BIT);
  if (0 == texName) glGenTextures (1, &texName);
  glBindTexture (GL_TEXTURE_RECTANGLE_EXT, texName);
  glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, w, h, 0,  GL_RGBA, GL_UNSIGNED_BYTE, base);
  glPopAttrib();
  CGContextRelease(fl_gc);
  fl_gc = NULL;
  free(base);
  GLfloat pos[4];
  glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
  if (texName) {//write the texture on screen
	CGRect bounds = CGRectMake (pos[0], pos[1] - fl_descent(), w, h);
	glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT); // GL_COLOR_BUFFER_BIT for glBlendFunc, GL_ENABLE_BIT for glEnable / glDisable
	
	glDisable (GL_DEPTH_TEST); // ensure text is not removed by depth buffer test.
	glEnable (GL_BLEND); // for text fading
	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // ditto
	glEnable (GL_TEXTURE_RECTANGLE_EXT);	
	
	glBindTexture (GL_TEXTURE_RECTANGLE_EXT, texName);
	glBegin (GL_QUADS);
	glTexCoord2f (0.0f, 0.0f); // draw lower left in world coordinates
	glVertex2f (bounds.origin.x, bounds.origin.y);
	
	glTexCoord2f (0.0f, h); // draw upper left in world coordinates
	glVertex2f (bounds.origin.x, bounds.origin.y + bounds.size.height);
	
	glTexCoord2f (w, h); // draw upper right in world coordinates
	glVertex2f (bounds.origin.x + bounds.size.width, bounds.origin.y + bounds.size.height);
	
	glTexCoord2f (w, 0.0f); // draw lower right in world coordinates
	glVertex2f (bounds.origin.x + bounds.size.width, bounds.origin.y);
	glEnd ();
	
	glPopAttrib();
	glDeleteTextures(1, &texName);
  }
  // reset original matrices
  glPopMatrix(); // GL_MODELVIEW
  glMatrixMode (GL_PROJECTION);
  glPopMatrix();
  glMatrixMode (matrixMode);
//set the raster position to end of string
  pos[0] += w;
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
#endif

#endif

//
// End of "$Id$".
//
