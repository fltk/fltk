//
// "$Id: fl_font_xft.cxx,v 1.4.2.5 2002/05/15 23:20:51 easysw Exp $"
//
// Xft font code for the Fast Light Tool Kit (FLTK).
//
// Copyright 2001-2002 Bill Spitzak and others.
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

//
// Draw fonts using Keith Packard's Xft library to provide anti-
// aliased text. Yow!
//
// Many thanks to Carl for making the original version of this.
//
// This font code only requires libXft to work.  Contrary to popular
// belief there is no need to have FreeType, or the Xrender extension
// available to use this code.  You will just get normal Xlib fonts
// (Xft calls them "core" fonts) The Xft algorithms for choosing
// these is about as good as the FLTK ones (I hope to fix it so it is
// exactly as good...), plus it can cache it's results and share them
// between programs, so using this should be a win in all cases. Also
// it should be obvious by comparing this file and fl_font_x.cxx that
// it is a lot easier to program with Xft than with Xlib.
//
// Also, Xft supports UTF-8 text rendering directly, which will allow
// us to support UTF-8 on all platforms more easily.
//
// To actually get antialiasing you need the following:
//
//     1. You have XFree86 4
//     2. You have the XRender extension
//     3. Your X device driver supports the render extension
//     4. You have libXft
//     5. Your libXft has FreeType2 support compiled in
//     6. You have the FreeType2 library
//
// Distributions that have XFree86 4.0.3 or later should have all of this...
//
// Unlike some other Xft packages, I tried to keep this simple and not
// to work around the current problems in Xft by making the "patterns"
// complicated. I belive doing this defeats our ability to improve Xft
// itself. You should edit the ~/.xftconfig file to "fix" things, there
// are several web pages of information on how to do this.
//

#include <X11/Xft/Xft.h>

// The predefined fonts that FLTK has:
static Fl_Fontdesc built_in_table[] = {
{" sans"},
{"Bsans"},
{"Isans"},
{"Psans"},
{" mono"},
{"Bmono"},
{"Imono"},
{"Pmono"},
{" serif"},
{"Bserif"},
{"Iserif"},
{"Pserif"},
{" symbol"},
{" screen"},
{"Bscreen"},
{" dingbats"},
};

Fl_Fontdesc* fl_fonts = built_in_table;

#define current_font (fl_fontsize->font)

int fl_font_;
int fl_size_;
XFontStruct* fl_xfont;
const char* fl_encoding_ = "iso8859-1";
Fl_FontSize* fl_fontsize;

void fl_font(int fnum, int size) {
  if (fnum == fl_font_ && size == fl_size_ &&
      !strcasecmp(fl_fontsize->encoding, fl_encoding_))
    return;
  fl_font_ = fnum; fl_size_ = size;
  Fl_Fontdesc *font = fl_fonts + fnum;
  Fl_FontSize* f;
  // search the fontsizes we have generated already
  for (f = font->first; f; f = f->next) {
    if (f->size == size && !strcasecmp(f->encoding, fl_encoding_))
      break;
  }
  if (!f) {
    f = new Fl_FontSize(font->name);
    f->next = font->first;
    font->first = f;
  }
  fl_fontsize = f;
  fl_xfont    = f->font->u.core.font;
}

static XftFont* fontopen(const char* name, bool core) {
  fl_open_display();
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

Fl_FontSize::~Fl_FontSize() {
  if (this == fl_fontsize) fl_fontsize = 0;
//  XftFontClose(fl_display, font);
}

int fl_height() { return current_font->ascent + current_font->descent; }
int fl_descent() { return current_font->descent; }

double fl_width(const char *str, int n) {
  XGlyphInfo i;
  XftTextExtents8(fl_display, current_font, (XftChar8 *)str, n, &i);
  return i.xOff;
}

double fl_width(uchar c) {
  return fl_width((const char *)(&c), 1);
}

#if USE_OVERLAY
// Currently Xft does not work with colormapped visuals, so this probably
// does not work unless you have a true-color overlay.
extern bool fl_overlay;
extern Colormap fl_overlay_colormap;
extern XVisualInfo* fl_overlay_visual;
#endif

// For some reason Xft produces errors if you destroy a window whose id
// still exists in an XftDraw structure. It would be nice if this is not
// true, a lot of junk is needed to try to stop this:

static XftDraw* draw;
static Window draw_window;
#if USE_OVERLAY
static XftDraw* draw_overlay;
static Window draw_overlay_window;
#endif

void fl_destroy_xft_draw(Window id) {
  if (id == draw_window)
    XftDrawChange(draw, draw_window = fl_message_window);
#if USE_OVERLAY
  if (id == draw_overlay_window)
    XftDrawChange(draw_overlay, draw_overlay_window = fl_message_window);
#endif
}

void fl_draw(const char *str, int n, int x, int y) {
#if USE_OVERLAY
  XftDraw*& draw = fl_overlay ? draw_overlay : ::draw;
  if (fl_overlay) {
    if (!draw) 
      draw = XftDrawCreate(fl_display, draw_overlay_window = fl_window,
			   fl_overlay_visual->visual, fl_overlay_colormap);
    else //if (draw_overlay_window != fl_window)
      XftDrawChange(draw, draw_overlay_window = fl_window);
  } else
#endif
  if (!draw)
    draw = XftDrawCreate(fl_display, draw_window = fl_window,
			 fl_visual->visual, fl_colormap);
  else //if (draw_window != fl_window)
    XftDrawChange(draw, draw_window = fl_window);

  Region region = fl_clip_region();
  if (region) {
    if (XEmptyRegion(region)) return;
    XftDrawSetClip(draw, region);
  }

  // Use fltk's color allocator, copy the results to match what
  // XftCollorAllocValue returns:
  XftColor color;
  color.pixel = fl_xpixel(fl_color_);
  uchar r,g,b; Fl::get_color(fl_color_, r,g,b);
  color.color.red   = (int)r*0x101;
  color.color.green = (int)g*0x101;
  color.color.blue  = (int)b*0x101;
  color.color.alpha = 0xffff;

  XftDrawString8(draw, &color, current_font, x, y, (XftChar8 *)str, n);
}

//
// End of "$Id: fl_font_xft.cxx,v 1.4.2.5 2002/05/15 23:20:51 easysw Exp $"
//
