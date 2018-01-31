//
// "$Id$"
//
// MSWidnows' GDI color functions for the Fast Light Tool Kit (FLTK).
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

// The fltk "colormap".  This allows ui colors to be stored in 8-bit
// locations, and provides a level of indirection so that global color
// changes can be made.  Not to be confused with the X colormap, which
// I try to hide completely.

#include "Fl_GDI_Graphics_Driver.H"

#include <config.h>
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/fl_draw.H>

// FIXME: all the global functions in this file should probably be protected
// members of the driver class. Starting with 1.4 we will allow multiple drivers
// to co-exist, creating conflicts with multipe mapping.

// FIXME: maybe we can forget about color mapping and assume RGB?
// FIXME: ... but for now we still have it ...
extern unsigned fl_cmap[256]; // defined in fl_color.cxx

// Translations to win32 data structures:
Fl_XMap fl_xmap[256];

Fl_XMap* fl_current_xmap;

HPALETTE fl_palette;
static HGDIOBJ tmppen=0;
static HPEN savepen=0;

void fl_cleanup_pens(void) {
  for (int i=0; i<256; i++) {
    if (fl_xmap[i].pen) DeleteObject(fl_xmap[i].pen);
  }
}

void fl_save_pen(void) {
    if(!tmppen) tmppen = CreatePen(PS_SOLID, 1, 0);
  savepen = (HPEN)SelectObject((HDC)fl_graphics_driver->gc(), tmppen);
}

void fl_restore_pen(void) {
  if (savepen) SelectObject((HDC)fl_graphics_driver->gc(), savepen);
    DeleteObject(tmppen);
    tmppen = 0;
    savepen = 0;
}

static void clear_xmap(Fl_XMap& xmap) {
  if (xmap.pen) {
    HDC gc = (HDC)fl_graphics_driver->gc();
    HGDIOBJ tmppen = GetStockObject(BLACK_PEN);
    HGDIOBJ oldpen = SelectObject(gc, tmppen);       // Push out the current pen of the gc
    if(oldpen != xmap.pen) SelectObject(gc, oldpen); // Put it back if it is not the one we are about to delete
    DeleteObject((HGDIOBJ)(xmap.pen));
    xmap.pen = 0;
    xmap.brush = -1;
  }
}

static void set_xmap(Fl_XMap& xmap, COLORREF c, int lw) {
  xmap.rgb = c;
  if (xmap.pen) {
      HDC gc = (HDC)fl_graphics_driver->gc();
      HGDIOBJ oldpen = SelectObject(gc,GetStockObject(BLACK_PEN)); // replace current pen with safe one
      if (oldpen != xmap.pen)SelectObject(gc,oldpen);              // if old one not xmap.pen, need to put it back
      DeleteObject(xmap.pen);                                         // delete pen
  }
//  xmap.pen = CreatePen(PS_SOLID, 1, xmap.rgb);                        // get a pen into xmap.pen
  LOGBRUSH penbrush = {BS_SOLID, xmap.rgb, 0};
  xmap.pen = ExtCreatePen(PS_GEOMETRIC | PS_ENDCAP_FLAT | PS_JOIN_ROUND, lw, &penbrush, 0, 0);
#ifdef FLTK_HIDPI_SUPPORT
  xmap.pwidth = lw;
#endif
  xmap.brush = -1;
}

void Fl_GDI_Graphics_Driver::color(Fl_Color i) {
  if (i & 0xffffff00) {
    unsigned rgb = (unsigned)i;
    color((uchar)(rgb >> 24), (uchar)(rgb >> 16), (uchar)(rgb >> 8));
  } else {
    Fl_Graphics_Driver::color(i);
    Fl_XMap &xmap = fl_xmap[i];
    int tw = line_width_ ? line_width_ : int(scale_); if (!tw) tw = 1;
    if (!xmap.pen
#ifdef FLTK_HIDPI_SUPPORT
        || xmap.pwidth != tw
#endif
        ) {
#if USE_COLORMAP
      if (fl_palette) {
	set_xmap(xmap, PALETTEINDEX(i), tw);
      } else {
#endif
	unsigned c = fl_cmap[i];
	set_xmap(xmap, RGB(uchar(c>>24), uchar(c>>16), uchar(c>>8)), tw);
#if USE_COLORMAP
      }
#endif
    }
    fl_current_xmap = &xmap;
    SelectObject(gc_, (HGDIOBJ)(xmap.pen));
  }
}

void Fl_GDI_Graphics_Driver::color(uchar r, uchar g, uchar b) {
  static Fl_XMap xmap;
  COLORREF c = RGB(r,g,b);
  Fl_Graphics_Driver::color( fl_rgb_color(r, g, b) );
  int tw = line_width_ ? line_width_ : int(scale_); if (!tw) tw = 1;
  if (!xmap.pen || c != xmap.rgb
#ifdef FLTK_HIDPI_SUPPORT
      || tw != xmap.pwidth
#endif
      ) {
    clear_xmap(xmap);
    set_xmap(xmap, c, tw);
  }
  fl_current_xmap = &xmap;
  SelectObject(gc_, (HGDIOBJ)(xmap.pen));
}

HBRUSH fl_brush() {
  return fl_brush_action(0);
}

HBRUSH fl_brush_action(int action) {
  Fl_XMap *xmap = fl_current_xmap;
  HDC gc = (HDC)fl_graphics_driver->gc();
  // Wonko: we use some statistics to cache only a limited number
  // of brushes:
#define FL_N_BRUSH 16
  static struct Fl_Brush {
    HBRUSH brush;
    unsigned short usage;
    Fl_XMap* backref;
  } brushes[FL_N_BRUSH];

  if (action) {
    SelectObject(gc, GetStockObject(BLACK_BRUSH));  // Load stock object
    for (int i=0; i<FL_N_BRUSH; i++) {
      if (brushes[i].brush)
        DeleteObject(brushes[i].brush); // delete all brushes in array
    }
    return NULL;
  }

  int i = xmap->brush; // find the associated brush
  if (i != -1) { // if the brush was allready allocated
    if (brushes[i].brush == NULL) goto CREATE_BRUSH;
    if ( (++brushes[i].usage) > 32000 ) { // keep a usage statistic
      for (int j=0; j<FL_N_BRUSH; j++) {
	if (brushes[j].usage>16000)
	  brushes[j].usage -= 16000;
	else
	  brushes[j].usage = 0;
      }
    }
    return brushes[i].brush;
  } else {
    int umin = 32000, imin = 0;
    for (i=0; i<FL_N_BRUSH; i++) {
      if (brushes[i].brush == NULL) goto CREATE_BRUSH;
      if (brushes[i].usage<umin) {
	umin = brushes[i].usage;
	imin = i;
      }
    }
    i = imin;
    HGDIOBJ tmpbrush = GetStockObject(BLACK_BRUSH);  // get a stock brush
    HGDIOBJ oldbrush = SelectObject(gc,tmpbrush); // load in into current context
    if (oldbrush != brushes[i].brush) SelectObject(gc,oldbrush);  // reload old one
    DeleteObject(brushes[i].brush);      // delete the one in list
    brushes[i].brush = NULL;
    brushes[i].backref->brush = -1;
  }
CREATE_BRUSH:
  brushes[i].brush = CreateSolidBrush(xmap->rgb);
  brushes[i].usage = 0;
  brushes[i].backref = xmap;
  xmap->brush = i;
  return brushes[i].brush;
}

void Fl_GDI_Graphics_Driver::free_color(Fl_Color i, int overlay) {
  if (overlay) return; // do something about GL overlay?
  clear_xmap(fl_xmap[i]);
}

void Fl_GDI_Graphics_Driver::set_color(Fl_Color i, unsigned c) {
  if (fl_cmap[i] != c) {
    clear_xmap(fl_xmap[i]);
    fl_cmap[i] = c;
  }
}

#if USE_COLORMAP

// 'fl_select_palette()' - Make a color palette for 8-bit displays if necessary
// Thanks to Michael Sweet @ Easy Software Products for this

HPALETTE
fl_select_palette(void)
{
  static char beenhere;
  HDC gc = (HDC)fl_graphics_driver->gc();
  if (!beenhere) {
    beenhere = 1;

    int nColors = GetDeviceCaps(gc, SIZEPALETTE);
    if (nColors <= 0 || nColors > 256) return NULL;
    // this will try to work on < 256 color screens, but will probably
    // come out quite badly.

    // I lamely try to get this variable-sized object allocated on stack:
    ulong foo[(sizeof(LOGPALETTE)+256*sizeof(PALETTEENTRY))/sizeof(ulong)+1];
    LOGPALETTE *pPal = (LOGPALETTE*)foo;

    pPal->palVersion    = 0x300;
    pPal->palNumEntries = nColors;

    // Build 256 colors from the standard FLTK colormap...

    for (int i = 0; i < nColors; i ++) {
      pPal->palPalEntry[i].peRed   = (fl_cmap[i] >> 24) & 255;
      pPal->palPalEntry[i].peGreen = (fl_cmap[i] >> 16) & 255;
      pPal->palPalEntry[i].peBlue  = (fl_cmap[i] >>  8) & 255;
      pPal->palPalEntry[i].peFlags = 0;
    };

    // Create the palette:
    fl_palette = CreatePalette(pPal);
  }
  if (fl_palette) {
    SelectPalette(gc, fl_palette, FALSE);
    RealizePalette(gc);
  }
  return fl_palette;
}

#endif

//
// End of "$Id$".
//
