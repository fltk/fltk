//
// "$Id: fl_font_xft.cxx,v 1.4 2001/12/16 22:32:03 spitzak Exp $"
//
// Copyright 2001 Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

// Draw fonts using Keith Packard's Xft and Xrender extension. Yow!
// Many thanks to Carl for making the original version of this.
//
// This code is included in fltk if it is compiled with -DUSE_XFT=1
// It should also be possible to compile this file as a seperate
// shared library, and by using LD_PRELOAD you can insert it between
// any fltk program and the fltk shared library.
//
// This plugin only requires libXft to work. Contrary to popular
// belief there is no need to have freetype, or the Xrender extension
// available to use this code. You will just get normal Xlib fonts
// (Xft calls them "core" fonts) The Xft algorithims for choosing
// these is about as good as the fltk ones (I hope to fix it so it is
// exactly as good...), plus it can cache it's results and share them
// between programs, so using this should be a win in all cases. Also
// it should be obvious by comparing this file and fl_font_x.cxx that
// it is a lot easier to program to Xft than to Xlib.
//
// To actually get antialiasing you need the following:
//
// 1. You have XFree86 4
// 2. You have the XRender extension
// 3. Your X device driver supports the render extension
// 4. You have libXft
// 5. Your libXft has freetype2 support compiled in
// 6. You have the freetype2 library
//
// Distributions that have XFree86 4.0.3 or later should have all of this...
//
// Unlike some other Xft packages, I tried to keep this simple and not
// to work around the current problems in Xft by making the "patterns"
// complicated. I belive doing this defeats our ability to improve Xft
// itself. You should edit the ~/.xftconfig file to "fix" things, there
// are several web pages of information on how to do this.

#include <fltk/Fl.h>
#include <fltk/Fl_Font.h>
#include <fltk/fl_draw.h>
#include <fltk/x.h>
#include <X11/Xft/Xft.h>
#include <string.h>
#include <stdlib.h>

class Fl_FontSize {
public:
  Fl_FontSize *next;	// linked list for this Fl_Fontdesc
  XftFont* font;
  const char* encoding;
  Fl_FontSize(const char* xfontname);
  unsigned size;
  //~Fl_FontSize();
};

static Fl_FontSize *fl_fontsize;
#define current_font (fl_fontsize->font)
  
// Change the encoding to use for the next font selection.
void fl_encoding(const char* f) {
  fl_encoding_ = f;
}

void fl_font(Fl_Font font, unsigned size) {
  if (font == fl_font_ && size == fl_size_ &&
      !strcasecmp(fl_fontsize->encoding, fl_encoding_))
    return;
  fl_font_ = font; fl_size_ = size;
  Fl_FontSize* f;
  // search the fontsizes we have generated already
  for (f = font->first; f; f = f->next) {
    if (f->size == size && !strcasecmp(f->encoding, fl_encoding_))
      break;
  }
  if (!f) {
    f = new Fl_FontSize(font->name_);
    f->next = font->first;
    ((Fl_Font_*)font)->first = f;
  }
  fl_fontsize = f;
}

static XftFont* fontopen(const char* name, bool core) {
  int slant = XFT_SLANT_ROMAN;
  int weight = XFT_WEIGHT_MEDIUM;
  // may be efficient, but this is non-obvious
  switch (*name++) {
  case 'I': slant = XFT_SLANT_ITALIC; break;
  case 'P': slant = XFT_SLANT_ITALIC;
  case 'B': weight = XFT_WEIGHT_BOLD; break;
  case ' ': break;
  default: name--;
  }
  // this call is extremely slow...
  return XftFontOpen(fl_display, fl_screen,
		     XFT_FAMILY, XftTypeString, name,
		     XFT_WEIGHT, XftTypeInteger, weight,
		     XFT_SLANT, XftTypeInteger, slant,
		     XFT_ENCODING, XftTypeString, fl_encoding_,
		     XFT_PIXEL_SIZE, XftTypeDouble, (double)fl_size_,
		     core ? XFT_CORE : 0, XftTypeBool, true,
		     XFT_RENDER, XftTypeBool, false,
		     0);
}

Fl_FontSize::Fl_FontSize(const char* name) {
  encoding = fl_encoding_;
  size = fl_size_;
  font = fontopen(name, false);
}

// This call is used by opengl to get a bitmapped font. Xft actually does
// a pretty good job of selecting X fonts...
XFontStruct* fl_xfont() {
  if (current_font->core) return current_font->u.core.font;
  static XftFont* xftfont;
  if (xftfont) XftFontClose (fl_display, xftfont);
  xftfont = fontopen(fl_font_->name_, true);
  return xftfont->u.core.font;
}

#if 0 // this is never called!
Fl_FontSize::~Fl_FontSize() {
  if (this == fl_fontsize) fl_fontsize = 0;
  XftFontClose(fl_display, font);
}
#endif

#if 1
// Some of the line spacings these return are insanely big!
int fl_height() { return current_font->height; }
int fl_descent() { return current_font->descent; }
#else
int fl_height() { return fl_size_;}
int fl_descent() { return fl_size_/4;}
#endif

int fl_width(const char *str, int n) {
  XGlyphInfo i;
  XftTextExtents8(fl_display, current_font, (XftChar8 *)str, n, &i);
  return i.xOff;
}

#if USE_OVERLAY
// Currently Xft does not work with colormapped visuals, so this probably
// does not work unless you have a true-color overlay.
extern bool fl_overlay;
extern Colormap fl_overlay_colormap;
extern XVisualInfo* fl_overlay_visual;
#endif

void fl_draw(const char *str, int n, int x, int y) {
#if USE_OVERLAY
  static XftDraw* draw_main, * draw_overlay;
  XftDraw*& draw = fl_overlay ? draw_overlay : draw_main;
  if (!draw)
    draw = XftDrawCreate(fl_display, fl_window,
			 (fl_overlay?fl_overlay_visual:fl_visual)->visual,
			 fl_overlay ? fl_overlay_colormap : fl_colormap);
  else
    XftDrawChange(draw, fl_window);
#else
  static XftDraw *draw = 0;
  if (!draw)
    draw = XftDrawCreate(fl_display, fl_window, fl_visual->visual, fl_colormap);
  else
    XftDrawChange(draw, fl_window);
#endif
  Region region = fl_clip_region();
  if (region) {
    if (XEmptyRegion(region)) return;
    XftDrawSetClip(draw, region);
  }

  // Use fltk's color allocator, copy the results to match what
  // XftCollorAllocValue returns:
  XftColor color;
  color.pixel = fl_pixel;
  uchar r,g,b; fl_get_color(fl_color_, r,g,b);
  color.color.red   = r*0x101;
  color.color.green = g*0x101;
  color.color.blue  = b*0x101;
  color.color.alpha = 0xffff;

  XftDrawString8(draw, &color, current_font, x+fl_x_, y+fl_y_,
                    (XftChar8 *)str, n);
}

////////////////////////////////////////////////////////////////

// The predefined fonts that fltk has:  bold:       italic:
Fl_Font_
fl_fonts[] = {
{" sans",		fl_fonts+1, fl_fonts+2},
{"Bsans",		fl_fonts+1, fl_fonts+3},
{"Isans",		fl_fonts+3, fl_fonts+2},
{"Psans",		fl_fonts+3, fl_fonts+3},
{" mono",		fl_fonts+5, fl_fonts+6},
{"Bmono",		fl_fonts+5, fl_fonts+7},
{"Imono",		fl_fonts+7, fl_fonts+6},
{"Pmono",		fl_fonts+7, fl_fonts+7},
{" serif",		fl_fonts+9, fl_fonts+10},
{"Bserif",		fl_fonts+9, fl_fonts+11},
{"Iserif",		fl_fonts+11,fl_fonts+10},
{"Pserif",		fl_fonts+11,fl_fonts+11},
{" symbol",		fl_fonts+12,fl_fonts+12},
{" screen",		fl_fonts+14,fl_fonts+14},
{"Bscreen",		fl_fonts+14,fl_fonts+14},
{" dingbats",		fl_fonts+15,fl_fonts+15},
};

////////////////////////////////////////////////////////////////
// The rest of this is for listing fonts:

// turn a stored font name into a pretty name:
const char* Fl_Font_::name(int* ap) const {
  int type;
  switch (name_[0]) {
  case 'B': type = FL_BOLD; break;
  case 'I': type = FL_ITALIC; break;
  case 'P': type = FL_BOLD | FL_ITALIC; break;
  default:  type = 0; break;
  }
  if (ap) {*ap = type; return name_+1;}
  if (!type) {return name_+1;}
  static char *buffer = new char[128];
  strcpy(buffer, name_+1);
  if (type & FL_BOLD) strcat(buffer, " bold");
  if (type & FL_ITALIC) strcat(buffer, " italic");
  return buffer;
}

extern "C" {
static int sort_function(const void *aa, const void *bb) {
  const char* name_a = (*(Fl_Font_**)aa)->name_;
  const char* name_b = (*(Fl_Font_**)bb)->name_;
  int ret = strcasecmp(name_a+1, name_b+1); if (ret) return ret;
  return name_a[0]-name_b[0]; // sort by attribute
}
}

static Fl_Font_* make_a_font(char attrib, const char* name) {
  Fl_Font_* newfont = new Fl_Font_;
  char *n = new char[strlen(name)+2];
  n[0] = attrib;
  strcpy(n+1, name);
  newfont->name_ = n;
  newfont->bold_ = newfont;
  newfont->italic_ = newfont;
  newfont->first = 0;
  return newfont;
}

int fl_list_fonts(Fl_Font*& arrayp) {
  static Fl_Font *font_array = 0;
  static int num_fonts = 0;

  if (font_array) { arrayp = font_array; return num_fonts; }

  XftFontSet *fs;
  char *name;
  fl_open_display();
  fs = XftListFonts(fl_display, fl_screen, 0, XFT_FAMILY, 0);
  num_fonts = fs->nfont;
  font_array = (Fl_Font *)calloc(num_fonts, sizeof(Fl_Font *));
  for (int i = 0; i < num_fonts; i++) {
    if (XftPatternGetString(fs->fonts[i], XFT_FAMILY, 0, &name) == XftResultMatch) {
      Fl_Font_* base = make_a_font(' ', name);
      base->italic_ = make_a_font('I', name);
      //if (bNeedBold) {
	base->bold_ = make_a_font('B', name);
	base->italic_->bold_ = base->bold_->italic_ = make_a_font('P', name);
	//}
      font_array[i] = base;
    }
  }
  XftFontSetDestroy(fs);

  qsort(font_array, num_fonts, sizeof(Fl_Font), sort_function);

  arrayp = font_array;
  return num_fonts;
}

extern "C" {
static int int_sort(const void *aa, const void *bb) {
  return (*(int*)aa)-(*(int*)bb);
}
}

////////////////////////////////////////////////////////////////

// Return all the point sizes supported by this font:
// Suprisingly enough Xft works exactly like fltk does and returns
// the same list. Except there is no way to tell if the font is scalable.
int Fl_Font_::sizes(int*& sizep) const {
  fl_open_display();
  XftFontSet* fs = XftListFonts(fl_display, fl_screen,
				XFT_FAMILY, XftTypeString, name_+1, 0,
				XFT_PIXEL_SIZE, 0);
  static int* array = 0;
  static int array_size = 0;
  if (fs->nfont >= array_size) {
    delete[] array;
    array = new int[array_size = fs->nfont+1];
  }
  array[0] = 0; int j = 1; // claim all fonts are scalable
  for (int i = 0; i < fs->nfont; i++) {
    double v;
    if (XftPatternGetDouble(fs->fonts[i], XFT_PIXEL_SIZE, 0, &v) == XftResultMatch) {
      array[j++] = int(v);
    }
  }
  qsort(array+1, j-1, sizeof(int), int_sort);
  XftFontSetDestroy(fs);
  sizep = array;
  return j;
}

////////////////////////////////////////////////////////////////
// Return all the encodings for this font:

int Fl_Font_::encodings(const char**& arrayp) const {
  fl_open_display();
  // we need to keep the previous return value around so the strings are
  // not destroyed.
  static XftFontSet* fs;
  if (fs) XftFontSetDestroy(fs);
  fs = XftListFonts(fl_display, fl_screen,
		    XFT_FAMILY, XftTypeString, name_+1, 0,
		    XFT_ENCODING, 0);
  static const char** array = 0;
  static int array_size = 0;
  if (fs->nfont > array_size) {
    delete[] array;
    array = new (const char*)[array_size = fs->nfont];
  }
  int j = 0;
  for (int i = 0; i < fs->nfont; i++) {
    char* v;
    if (XftPatternGetString(fs->fonts[i], XFT_ENCODING, 0, &v) == XftResultMatch) {
      array[j++] = v;
    }
  }
  arrayp = array;
  return j;
}

//
// End of "$Id: fl_font_xft.cxx,v 1.4 2001/12/16 22:32:03 spitzak Exp $"
//
