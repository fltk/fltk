//
// "$Id: fl_font_mac.cxx,v 1.1.2.22 2004/09/09 21:34:47 matthiaswm Exp $"
//
// MacOS font selection routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2004 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <config.h>

//: MeasureText, FontMetrics, WidthTabHandle, GetSysFont, SysFontSize
//: TextSize, TextFont
//: GetFNum (theName: Str255; VAR familyID: Integer);
//: FUNCTION FMSwapFont (inRec: FMInput): FMOutPtr;
//: SetFractEnable

Fl_FontSize::Fl_FontSize(const char* name, int Size) {
  next = 0;
#  if HAVE_GL
  listbase = 0;
#  endif
#ifdef __APPLE_QD__
  knowMetrics = 0;
  switch (*name++) {
  case 'I': face = italic; break;
  case 'P': face = italic | bold; break;
  case 'B': face = bold; break;
  default: face = 0; break;
  }
  unsigned char fn[80]; 
  fn[0] = strlen(name); strcpy((char*)(fn+1), name);
  GetFNum(fn, &font);
  size = Size;
  FMInput fIn = { font, size, face, 0, 0, { 1, 1}, { 1, 1} };
  FMOutput *fOut = FMSwapFont(&fIn);
  ascent = fOut->ascent;	//: the following three lines give only temporary aproimations
  descent = fOut->descent;
  for (int i=0; i<256; i++) width[i] = fOut->widMax;
  minsize = maxsize = size;
#elif defined(__APPLE_QUARTZ__)
  q_name = strdup(name);
  size = Size;
  ascent = Size*3/4;
  descent = Size-ascent;
  q_width = Size*2/3;
  minsize = maxsize = Size;
  // Using ATS to get the genral Glyph size information
  CFStringRef cfname = CFStringCreateWithCString(0L, q_name, kCFStringEncodingASCII);
  ATSFontRef font = ATSFontFindFromName(cfname, kATSOptionFlagsDefault);
  if (font) {
    ATSFontMetrics m = { 0 };
    ATSFontGetHorizontalMetrics(font, kATSOptionFlagsDefault, &m);
    if (m.avgAdvanceWidth) q_width = int(m.avgAdvanceWidth*size);
    // playing with the offsets a little to make standard sizes fit
    if (m.ascent) ascent  = int(m.ascent*size-0.5f);
    if (m.descent) descent = -int(m.descent*size-1.5f);
  }
  CFRelease(cfname);
#endif
}

Fl_FontSize* fl_fontsize = 0L;

Fl_FontSize::~Fl_FontSize() {
/*
#if HAVE_GL
 // ++ todo: remove OpenGL font alocations
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
  */
  if (this == fl_fontsize) fl_fontsize = 0;
#ifdef __APPLE_QUARTZ__
  free(q_name);
#endif
}

////////////////////////////////////////////////////////////////

static Fl_Fontdesc built_in_table[] = {
#ifdef __APPLE_QD__
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
{" Chicago"},
{"BChicago"},
{" Webdings"},
#elif defined(__APPLE_QUARTZ__)
{"Arial"},
{"Arial Bold"},
{"Arial Italic"},
{"Arial Bold Italic"},
{"Courier New"},
{"Courier New Bold"},
{"Courier New Italic"},
{"Courier New Bold Italic"},
{"Times New Roman"},
{"Times New Roman Bold"},
{"Times New Roman Italic"},
{"Times New Roman Bold Italic"},
{"Symbol"},
{"Monaco"},
{"Andale Mono"}, // there is no bold Monaco font on standard Mac
{"Webdings"},
#endif
};

Fl_Fontdesc* fl_fonts = built_in_table;

void fl_font(Fl_FontSize* s) {
  fl_fontsize = s;
#ifdef __APPLE_QD__
  if (fl_window) SetPort( GetWindowPort(fl_window) );
  TextFont(fl_fontsize->font);	//: select font into current QuickDraw GC
  TextFace(fl_fontsize->face);
  TextSize(fl_fontsize->size);
  if (!fl_fontsize->knowMetrics) {	//: get the true metrics for the currnet GC 
                                        //: (fails on multiple monitors with different dpi's!)
    FontInfo fi; GetFontInfo(&fi);
    fl_fontsize->ascent = fi.ascent;
    fl_fontsize->descent = fi.descent;
    FMetricRec mr; FontMetrics(&mr);
    short *f = (short*)*mr.wTabHandle; //: get the char size table
    for (int i=0; i<256; i++) fl_fontsize->width[i] = f[2*i];
    fl_fontsize->knowMetrics = 1;
  }
#elif defined(__APPLE_QUARTZ__)
  if (!s) return;
  if (!fl_gc) return; // no worries, we will assign the font to the context later
  CGContextSelectFont(fl_gc, s->q_name, (float)s->size, kCGEncodingMacRoman);
#else
# error : need to defined either Quartz or Quickdraw
#endif
}

static Fl_FontSize* find(int fnum, int size) {
  Fl_Fontdesc* s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // use 0 if fnum undefined
  Fl_FontSize* f;
  for (f = s->first; f; f = f->next)
    if (f->minsize <= size && f->maxsize >= size) return f;
  f = new Fl_FontSize(s->name, size);
  f->next = s->first;
  s->first = f;
  return f;
}

////////////////////////////////////////////////////////////////
// Public interface:

int fl_font_ = 0;
int fl_size_ = 0;

void fl_font(int fnum, int size) {
  fl_font_ = fnum;
  fl_size_ = size;
  fl_font(find(fnum, size));
}

int fl_height() {
  if (fl_fontsize) return fl_fontsize->ascent+fl_fontsize->descent;
  else return -1;
}

int fl_descent() {
  if (fl_fontsize) return fl_fontsize->descent;
  else return -1;
}

// MRS: The default character set is MacRoman, which is different from
//      ISO-8859-1; in FLTK 2.0 we'll use UTF-8 with Quartz...

static uchar macroman_lut[256] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
  32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
  48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
  64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
  80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
  96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
  112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
  128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
  144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
  202, 193, 162, 163, 164, 180, 166, 164, 172, 169, 187, 199, 194, 173, 168, 248,
  161, 177, 178, 179, 171, 181, 166, 225, 252, 185, 188, 200, 188, 189, 190, 192,
  203, 231, 229, 204, 128, 129, 174, 130, 233, 131, 230, 232, 237, 234, 235, 236,
  208, 132, 241, 238, 239, 205, 133, 215, 175, 244, 242, 243, 134, 221, 222, 167,
  136, 135, 137, 139, 138, 140, 190, 141, 143, 142, 144, 145, 147, 146, 148, 149,
  240, 150, 152, 151, 153, 155, 154, 214, 191, 157, 156, 158, 159, 253, 254, 216
};

static char *iso_buf = 0;
static int n_iso_buf = 0;

// this function must be available for OpenGL character drawing as well
const char *fl_iso2macRoman(const char *s, int n) {
  if (n>n_iso_buf) {
    if (iso_buf) free(iso_buf);
    iso_buf = (char*)malloc(n+500);
    n_iso_buf = n;
  }
  uchar *src = (uchar*)s;
  uchar *dst = (uchar*)iso_buf;
  for (;n--;) {
    *dst++ = macroman_lut[*src++];
  }
  return iso_buf;
}

double fl_width(const char* c, int n) {
#ifdef __APPLE_QD__
  return (double)TextWidth( c, 0, n );
#else
  if (!fl_gc) {
    Fl_Window *w = Fl::first_window();
    if (w) w->make_current();
    if (!fl_gc) return -1;
  }
  const char *txt = fl_iso2macRoman(c, n);
  // according to the Apple developer docs, this is the correct way to
  // find the length of a rendered text...
  CGContextSetTextPosition(fl_gc, 0, 0);
  CGContextSetTextDrawingMode(fl_gc, kCGTextInvisible);
  CGContextShowText(fl_gc, txt, n);
  CGContextSetTextDrawingMode(fl_gc, kCGTextFill);
  CGPoint p = CGContextGetTextPosition(fl_gc);
  return p.x;
#endif
}

double fl_width(uchar c) {
#ifdef __APPLE_QD__
  return (double)TextWidth((const char*)(macroman_lut + c), 0, 1 );
#else
  return fl_width((const char*)(&c), 1);
#endif
}

void fl_draw(const char *str, int n, float x, float y);

void fl_draw(const char* str, int n, int x, int y) {
#ifdef __APPLE_QD__
  const char *txt = fl_iso2macRoman(str, n);
  MoveTo(x, y);
  DrawText((const char *)buf, 0, n);
#elif defined(__APPLE_QUARTZ__)
  fl_draw(str, n, (float)x, (float)y);
#else
# error : neither Quartz no Quickdraw chosen
#endif
}

void fl_draw(const char *str, int n, float x, float y) {
#ifdef __APPLE_QD__
  fl_draw(str, n, (int)x, (int)y);
#elif defined(__APPLE_QUARTZ__)
  const char *txt = fl_iso2macRoman(str, n);
  CGContextShowTextAtPoint(fl_gc, x, y, txt, n);
#else
# error : neither Quartz no Quickdraw chosen
#endif
}

//
// End of "$Id: fl_font_mac.cxx,v 1.1.2.22 2004/09/09 21:34:47 matthiaswm Exp $".
//
