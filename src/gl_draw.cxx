// gl_draw.C

// Functions from <FL/gl.h>
// See also Fl_Gl_Window and gl_start.C

#include <config.h>
#if HAVE_GL

#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/x.H>
#include "Fl_Gl_Choice.H"
#include "Fl_Font.H"
#include <string.h>

// stuff from fl_draw.H:
void  fl_font(int fontid, int size);
int   fl_height();	// using "size" should work ok
int   fl_descent();
void fl_measure(const char*, int& x, int& y);
double fl_width(const char*);
double fl_width(const char*, int n);
double fl_width(uchar);
unsigned long fl_xpixel(uchar i);

void  gl_font(int fontid, int size) {fl_font(fontid, size);}
int   gl_height() {return fl_height();}
int   gl_descent() {return fl_descent();}
double gl_width(const char* s) {return fl_width(s);}
double gl_width(const char* s, int n) {return fl_width(s,n);}
double gl_width(uchar c) {return fl_width(c);}

void gl_draw(const char* str, int n) {
#ifdef WIN32
  if (!fl_current_xfont->listbase) {
    int base = fl_current_xfont->metr.tmFirstChar;
    int size = fl_current_xfont->metr.tmLastChar-base+1;
    HFONT oldFid = (HFONT)SelectObject(fl_gc, fl_current_xfont->fid);
    fl_current_xfont->listbase = glGenLists(size)-base;
    wglUseFontBitmaps(fl_gc, base, size, fl_current_xfont->listbase+base); 
    SelectObject(fl_gc, oldFid);
  }
#else
  if (!fl_current_xfont->listbase) {
    int base = fl_current_xfont->font->min_char_or_byte2;
    int size = fl_current_xfont->font->max_char_or_byte2-base+1;
//  int base = 0; int size = 256;
    fl_current_xfont->listbase = glGenLists(size)-base;
    glXUseXFont(fl_current_xfont->font->fid, base, size, 
		fl_current_xfont->listbase+base);
  }
#endif
  glListBase(fl_current_xfont->listbase);
  glCallLists(n, GL_UNSIGNED_BYTE, str);
}

void gl_draw(const char* str, int n, int x, int y) {
  glRasterPos2i(x, y);
  gl_draw(str, n);
}

void gl_draw(const char* str) {
  gl_draw(str, strlen(str));
}

void gl_draw(const char* str, int x, int y) {
  gl_draw(str, strlen(str), x, y);
}

static void gl_draw_invert(const char* str, int n, int x, int y) {
  glRasterPos2i(x, -y);
  gl_draw(str, n);
}

void gl_draw(
  const char* str, 	// the (multi-line) string
  int x, int y, int w, int h, 	// bounding box
  Fl_Align align) {
  fl_draw(str, x, -y-h, w, h, align, gl_draw_invert);
}

void gl_measure(const char* str, int& x, int& y) {fl_measure(str,x,y);}

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
#endif

void gl_color(Fl_Color i) {
#if HAVE_GL_OVERLAY
#ifdef WIN32
  if (fl_overlay) {glIndexi(i ? i : FL_GRAY_RAMP); return;}
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

#endif

