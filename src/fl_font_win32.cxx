// fl_font_win32.C

#include <config.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/win32.H>
#include "Fl_Font.H"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

Fl_XFont::Fl_XFont(const char *name, int size, int num) {
  int weight = FW_NORMAL;
  int italic = 0;
  switch (*name++) {
  case 'I': italic = 1; break;
  case 'P': italic = 1;
  case 'B': weight = FW_BOLD; break;
  case ' ': break;
  default: name--;
  }
  fid = CreateFont(
    -size, // negative makes it use "char size"
    0,	            // logical average character width 
    0,	            // angle of escapement 
    0,	            // base-line orientation angle 
    weight,
    italic,
    FALSE,	        // underline attribute flag 
    FALSE,	        // strikeout attribute flag 
    DEFAULT_CHARSET,    // character set identifier 
    OUT_DEFAULT_PRECIS,	// output precision 
    CLIP_DEFAULT_PRECIS,// clipping precision 
    DEFAULT_QUALITY,	// output quality 
    DEFAULT_PITCH,	// pitch and family 
    name	        // pointer to typeface name string 
    );
  if (!fl_gc) fl_gc = fl_GetDC(0);
  SelectObject(fl_gc, fid);
  GetTextMetrics(fl_gc, &metr);
//  BOOL ret = GetCharWidthFloat(fl_gc, metr.tmFirstChar, metr.tmLastChar, font->width+metr.tmFirstChar);
// ...would be the right call, but is not implemented into Window95! (WinNT?)
  GetCharWidth(fl_gc, 0, 255, width);
#if HAVE_GL
  listbase = 0;
#endif
  number = num;
  minsize = maxsize = size;
}

Fl_XFont *fl_current_xfont;

Fl_XFont::~Fl_XFont() {
#if HAVE_GL
// Delete list created by gl_draw().  This is not done by this code
// as it will link in GL unnecessarily.  There should be some kind
// of "free" routine pointer, or a subclass?
// if (listbase) {
//  int base = font->min_char_or_byte2;
//  int size = font->max_char_or_byte2-base+1;
//  int base = 0; int size = 256;
//  glDeleteLists(listbase+base,size);
// }
#endif
  if (this == fl_current_xfont) fl_current_xfont = 0;
  DeleteObject(fid);
}

////////////////////////////////////////////////////////////////

// WARNING: if you add to this table, you must redefine FL_FREE_FONT
// in Enumerations.H & recompile!!
static Fl_Fontdesc built_in_table[] = {
{" Arial"},
{"BArial"},
{"IArial"},
{"PArial"},
{" Courier New"},
{"BCourier New"},
{"ICourier New"},
{"PCourier New"},
{" Times New Roman"},
{"BTimes New Roman"},
{"ITimes New Roman"},
{"PTimes New Roman"},
{" Symbol"},
{" Terminal"},
{"BTerminal"},
{" Wingdings"},
};

Fl_Fontdesc *fl_fonts = built_in_table;

static Fl_XFont *find(int fnum, int size) {
  Fl_Fontdesc *s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // use 0 if fnum undefined
  Fl_XFont *f;
  for (f = s->first; f; f = f->next)
    if (f->minsize <= size && f->maxsize >= size) return f;
  f = new Fl_XFont(s->name, size, fnum);
  f->next = s->first;
  s->first = f;
  return f;
}

////////////////////////////////////////////////////////////////
// Public interface:

int fl_font_;
int fl_size_;
static HDC font_gc;

void fl_font(int fnum, int size) {
  if (fnum == fl_font_ && size == fl_size_) return;
  fl_font_ = fnum; fl_size_ = size;
  fl_current_xfont = find(fnum, size);
}

void fl_font(int fnum, int size, Fl_Font default_font, int default_size) {
  if (fnum<4) fnum |= default_font;
  fl_font(fnum, size + default_size);
}

int fl_height() {
  return (fl_current_xfont->metr.tmAscent + fl_current_xfont->metr.tmDescent);
}

int fl_descent() {
  return fl_current_xfont->metr.tmDescent;
}

double fl_width(const char *c) {
  double w = 0.0;
  while (*c) w += fl_current_xfont->width[uchar(*c++)];
  return w;
}

double fl_width(const char *c, int n) {
  double w = 0.0;
  while (n--) w += fl_current_xfont->width[uchar(*c++)];
  return w;
}

double fl_width(uchar c) {
  return fl_current_xfont->width[c];
}

void fl_draw(const char *str, int n, int x, int y) {
  SetTextColor(fl_gc, fl_RGB());
  SelectObject(fl_gc, fl_current_xfont->fid);
  TextOut(fl_gc, x, y, str, n);
}

void fl_draw(const char *str, int x, int y) {
  fl_draw(str, strlen(str), x, y);
}

// end of fl_font_win32.C
