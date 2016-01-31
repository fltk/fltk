//
// "$Id$"
//
// WIN32 font utilities for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#ifdef WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
/* We require Windows 2000 features such as GetGlyphIndices */
# if !defined(WINVER) || (WINVER < 0x0500)
#  ifdef WINVER
#   undef WINVER
#  endif
#  define WINVER 0x0500
# endif
# if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0500)
#  ifdef _WIN32_WINNT
#   undef _WIN32_WINNT
#  endif
#  define _WIN32_WINNT 0x0500
# endif
#endif

// Select fonts from the FLTK font table.
#include "../../flstring.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include "../../Fl_Font.H"

#include <stdio.h>
#include <stdlib.h>

// This function fills in the FLTK font table with all the fonts that
// are found on the X server.  It tries to place the fonts into families
// and to sort them so the first 4 in a family are normal, bold, italic,
// and bold italic.
#include <FL/fl_utf8.h>
#ifdef __CYGWIN__
# include <wchar.h>
#endif

// Bug: older versions calculated the value for *ap as a side effect of
// making the name, and then forgot about it. To avoid having to change
// the header files I decided to store this value in the last character
// of the font name array.
#define ENDOFBUFFER 127 // sizeof(Fl_Font.fontname)-1

// turn a stored font name into a pretty name:
const char* Fl::get_font_name(Fl_Font fnum, int* ap) {
  Fl_Fontdesc *f = fl_fonts + fnum;
  if (!f->fontname[0]) {
    const char* p = f->name;
    if (!p || !*p) {if (ap) *ap = 0; return "";}
    int type;
    switch (*p) {
    case 'B': type = FL_BOLD; break;
    case 'I': type = FL_ITALIC; break;
    case 'P': type = FL_BOLD | FL_ITALIC; break;
    default:  type = 0; break;
    }
    strlcpy(f->fontname, p+1, ENDOFBUFFER);
    if (type & FL_BOLD) strlcat(f->fontname, " bold", ENDOFBUFFER);
    if (type & FL_ITALIC) strlcat(f->fontname, " italic", ENDOFBUFFER);
    f->fontname[ENDOFBUFFER] = (char)type;
  }
  if (ap) *ap = f->fontname[ENDOFBUFFER];
  return f->fontname;
}

static int fl_free_font = FL_FREE_FONT;

static int CALLBACK
enumcbw(CONST LOGFONTW    *lpelf,
        CONST TEXTMETRICW * /*lpntm*/,
       DWORD            /*FontType*/,
       LPARAM           p) {
  if (!p && lpelf->lfCharSet != ANSI_CHARSET) return 1;
  char *n = NULL;
  size_t l = wcslen(lpelf->lfFaceName);
  unsigned dstlen = fl_utf8fromwc(n, 0, (xchar*)lpelf->lfFaceName, (unsigned) l) + 1; // measure the string
  n = (char*) malloc(dstlen);
//n[fl_unicode2utf((xchar*)lpelf->lfFaceName, l, n)] = 0;
  dstlen = fl_utf8fromwc(n, dstlen, (xchar*)lpelf->lfFaceName, (unsigned) l); // convert the string
  n[dstlen] = 0;
  for (int i=0; i<FL_FREE_FONT; i++) // skip if one of our built-in fonts
    if (!strcmp(Fl::get_font_name((Fl_Font)i),n)) {free(n);return 1;}
  char buffer[LF_FACESIZE + 1];
  strcpy(buffer+1, n);
  buffer[0] = ' '; Fl::set_font((Fl_Font)(fl_free_font++), strdup(buffer));
  if (lpelf->lfWeight <= 400)
    buffer[0] = 'B', Fl::set_font((Fl_Font)(fl_free_font++), strdup(buffer));
  buffer[0] = 'I'; Fl::set_font((Fl_Font)(fl_free_font++), strdup(buffer));
  if (lpelf->lfWeight <= 400)
    buffer[0] = 'P', Fl::set_font((Fl_Font)(fl_free_font++), strdup(buffer));
  free(n);
  return 1;
} /* enumcbw */

Fl_Font Fl::set_fonts(const char* xstarname) {
  if (fl_free_font == FL_FREE_FONT) {// if not already been called
    if (!fl_gc) fl_GetDC(0);

    EnumFontFamiliesW(fl_gc, NULL, (FONTENUMPROCW)enumcbw, xstarname != 0);

  }
  return (Fl_Font)fl_free_font;
}


static int nbSize;
static int cyPerInch;
static int sizes[128];
static int CALLBACK

EnumSizeCbW(CONST LOGFONTW    * /*lpelf*/,
           CONST TEXTMETRICW *lpntm,
           DWORD            fontType,
           LPARAM           /*p*/) {
  if ((fontType & RASTER_FONTTYPE) == 0) {
    sizes[0] = 0;
    nbSize = 1;

    // Scalable font
    return 0;
  }

  int add = lpntm->tmHeight - lpntm->tmInternalLeading;
  add = MulDiv(add, 72, cyPerInch);

  int start = 0;
  while ((start < nbSize) && (sizes[start] < add)) {
    start++;
  }

  if ((start < nbSize) && (sizes[start] == add)) {
    return 1;
  }

  for (int i=nbSize; i>start; i--) sizes[i] = sizes[i - 1];

  sizes[start] = add;
  nbSize++;

  // Stop enum if buffer overflow
  return nbSize < 128;
}


int
Fl::get_font_sizes(Fl_Font fnum, int*& sizep) {
  nbSize = 0;
  Fl_Fontdesc *s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // empty slot in table, use entry 0

  if (!fl_gc) fl_GetDC(0);
  cyPerInch = GetDeviceCaps(fl_gc, LOGPIXELSY);
  if (cyPerInch < 1) cyPerInch = 1;

//  int l = fl_utf_nb_char((unsigned char*)s->name+1, strlen(s->name+1));
//  unsigned short *b = (unsigned short*) malloc((l + 1) * sizeof(short));
//  fl_utf2unicode((unsigned char*)s->name+1, l, (xchar*)b);
  const char *nm = (const char*)s->name+1;
  size_t len = strlen(s->name+1);
  unsigned l = fl_utf8toUtf16(nm, (unsigned) len, NULL, 0); // Pass NULL to query length required
  unsigned short *b = (unsigned short*) malloc((l + 1) * sizeof(short));
  l = fl_utf8toUtf16(nm, (unsigned) len, b, (l+1)); // Now do the conversion
  b[l] = 0;
  EnumFontFamiliesW(fl_gc, (WCHAR*)b, (FONTENUMPROCW)EnumSizeCbW, 0);
  free(b);

  sizep = sizes;
  return nbSize;
}


//
// End of "$Id$".
//
//
// "$Id$"
//
// WIN32 font selection routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#include <FL/Fl_Printer.H>

static int fl_angle_ = 0;

#ifndef FL_DOXYGEN
Fl_Font_Descriptor::Fl_Font_Descriptor(const char* name, Fl_Fontsize fsize) {
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
    -fsize, // negative makes it use "char size"
    0,	            // logical average character width
    fl_angle_*10,	            // angle of escapement
    fl_angle_*10,	            // base-line orientation angle
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
  angle = fl_angle_;
  if (!fl_gc) fl_GetDC(0);
  SelectObject(fl_gc, fid);
  GetTextMetrics(fl_gc, &metr);
//  BOOL ret = GetCharWidthFloat(fl_gc, metr.tmFirstChar, metr.tmLastChar, font->width+metr.tmFirstChar);
// ...would be the right call, but is not implemented into Window95! (WinNT?)
  //GetCharWidth(fl_gc, 0, 255, width);
  int i;
  memset(width, 0, 64 * sizeof(int*));
#if HAVE_GL
  listbase = 0;
  for (i = 0; i < 64; i++) glok[i] = 0;
#endif
  size = fsize;
}

Fl_Font_Descriptor::~Fl_Font_Descriptor() {
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
  if (this == fl_graphics_driver->font_descriptor()) fl_graphics_driver->font_descriptor(NULL);
  DeleteObject(fid);
  for (int i = 0; i < 64; i++) {
    if ( width[i] ) free(width[i]);
    }
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

Fl_Fontdesc* fl_fonts = built_in_table;

static Fl_Font_Descriptor* find(Fl_Font fnum, Fl_Fontsize size, int angle) {
  Fl_Fontdesc* s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // use 0 if fnum undefined
  Fl_Font_Descriptor* f;
  for (f = s->first; f; f = f->next)
    if (f->size == size && f->angle == angle) return f;
  f = new Fl_Font_Descriptor(s->name, size);
  f->next = s->first;
  s->first = f;
  return f;
}

////////////////////////////////////////////////////////////////
// Public interface:

static void fl_font(Fl_Graphics_Driver *driver, Fl_Font fnum, Fl_Fontsize size, int angle) {
  if (fnum==-1) { // just make sure that we will load a new font next time
    fl_angle_ = 0;
    driver->Fl_Graphics_Driver::font(0, 0);
    return;
  }
  if (fnum == driver->Fl_Graphics_Driver::font() && size == driver->size() && angle == fl_angle_) return;
  fl_angle_ = angle;
  driver->Fl_Graphics_Driver::font(fnum, size);
  driver->font_descriptor( find(fnum, size, angle) );
}

void Fl_GDI_Graphics_Driver::font(Fl_Font fnum, Fl_Fontsize size) {
  fl_font(this, fnum, size, 0);
}

int Fl_GDI_Graphics_Driver::height() {
  Fl_Font_Descriptor *fl_fontsize = font_descriptor();
  if (fl_fontsize) return (fl_fontsize->metr.tmAscent + fl_fontsize->metr.tmDescent);
  else return -1;
}

int Fl_GDI_Graphics_Driver::descent() {
  Fl_Font_Descriptor *fl_fontsize = font_descriptor();
  if (fl_fontsize) return fl_fontsize->metr.tmDescent;
  else return -1;
}

// Unicode string buffer
static unsigned short *wstr = NULL;
static int wstr_len    = 0;


double Fl_GDI_Graphics_Driver::width(const char* c, int n) {
  int i = 0;
  if (!font_descriptor()) return -1.0;
  double w = 0.0;
  char *end = (char *)&c[n];
  while (i < n) {
    unsigned int ucs;
    int l;
    ucs = fl_utf8decode((const char*)(c + i), end, &l);
//  if (l < 1) l = 1;
    i += l;
    if (!fl_nonspacing(ucs)) {
      w += width(ucs);
    }
  }
  return w;
}

double Fl_GDI_Graphics_Driver::width(unsigned int c) {
  Fl_Font_Descriptor *fl_fontsize = font_descriptor();
  unsigned int r;
  SIZE s;
  // Special Case Handling of Unicode points over U+FFFF.
  // The logic (below) computes a lookup table for char widths
  // on-the-fly, but the table only covers codepoints up to
  // U+FFFF, which covers the basic multilingual plane, but
  // not any higher plane, or glyphs that require surrogate-pairs
  // to encode them in WinXX, which is UTF16.
  // This code assumes that these glyphs are rarely used and simply
  // measures them explicitly if they occur - This will be slow...
  if(c > 0x0000FFFF) { // UTF16 surrogate pair is needed
    if (!fl_gc) { // We have no valid gc, so nothing to measure - bail out
      return 0.0;
    }
    int cc; // cell count
    unsigned short u16[4]; // Array for UTF16 representation of c
    // Creates a UTF16 string from a UCS code point.
    cc = fl_ucs_to_Utf16(c, u16, 4);
    // Make sure the current font is selected before we make the measurement
    SelectObject(fl_gc, fl_fontsize->fid);
    // measure the glyph width
    GetTextExtentPoint32W(fl_gc, (WCHAR*)u16, cc, &s);
    return (double)s.cx;
  }
  // else - this falls through to the lookup-table for glyph widths
  // in the basic multilingual plane
  r = (c & 0xFC00) >> 10;
  if (!fl_fontsize->width[r]) {
    fl_fontsize->width[r] = (int*) malloc(sizeof(int) * 0x0400);
    for (int i = 0; i < 0x0400; i++) fl_fontsize->width[r][i] = -1;
  } else {
    if ( fl_fontsize->width[r][c&0x03FF] >= 0 ) { // already cached
	return (double) fl_fontsize->width[r][c & 0x03FF];
    }
  }
  unsigned short ii = r * 0x400;
  // The following code makes a best effort attempt to obtain a valid fl_gc.
  // If no fl_gc is available at the time we call fl_width(), then we first
  // try to obtain a gc from the first fltk window.
  // If that is null then we attempt to obtain the gc from the current screen
  // using (GetDC(NULL)).
  // This should resolve STR #2086
  HDC gc = fl_gc;
  HWND hWnd = 0;
  if (!gc) { // We have no valid gc, try and obtain one
    // Use our first fltk window, or fallback to using the screen via GetDC(NULL)
    hWnd = Fl::first_window() ? fl_xid(Fl::first_window()) : NULL;
    gc = GetDC(hWnd);
  }
  if (!gc) Fl::fatal("Invalid graphic context: fl_width() failed because no valid HDC was found!");
  SelectObject(gc, fl_fontsize->fid);
  ii += c &0x03FF;
  GetTextExtentPoint32W(gc, (WCHAR*)&ii, 1, &s);
  fl_fontsize->width[r][c&0x03FF] = s.cx;
  if (gc && gc!=fl_gc) ReleaseDC(hWnd, gc);
  return (double) fl_fontsize->width[r][c & 0x03FF];
}

/* Add function pointer to allow us to access GetGlyphIndicesW on systems that have it,
 * without crashing on systems that do not. */
/* DWORD WINAPI GetGlyphIndicesW(HDC,LPCWSTR,int,LPWORD,DWORD) */
typedef DWORD (WINAPI* fl_GetGlyphIndices_func)(HDC,LPCWSTR,int,LPWORD,DWORD);

static fl_GetGlyphIndices_func fl_GetGlyphIndices = NULL; // used to hold a proc pointer for GetGlyphIndicesW
static int have_loaded_GetGlyphIndices = 0; // Set this non-zero once we have tried to load GetGlyphIndices

// Function that tries to dynamically load GetGlyphIndicesW at runtime
static void GetGlyphIndices_init() {
  // Since not all versions of Windows include GetGlyphIndicesW support,
  // we do a run-time check for the required function.
  HMODULE hMod = GetModuleHandle("GDI32.DLL");
  if (hMod) {
    // check that GetGlyphIndicesW is available
    fl_GetGlyphIndices = (fl_GetGlyphIndices_func)GetProcAddress(hMod, "GetGlyphIndicesW");
  }
  have_loaded_GetGlyphIndices = -1; // set this non-zero when we have attempted to load GetGlyphIndicesW
} // GetGlyphIndices_init function

static void on_printer_extents_update(int &dx, int &dy, int &w, int &h)
// converts text extents from device coords to logical coords
{
  POINT pt[3] = { {0, 0}, {dx, dy}, {dx+w, dy+h} };
  DPtoLP(fl_gc, pt, 3);
  w = pt[2].x - pt[1].x;
  h = pt[2].y - pt[1].y;
  dx = pt[1].x - pt[0].x;
  dy = pt[1].y - pt[0].y;
}

// if printer context, extents shd be converted to logical coords
#define EXTENTS_UPDATE(x,y,w,h) \
  if (Fl_Surface_Device::surface() != Fl_Display_Device::display_device()) { on_printer_extents_update(x,y,w,h); }

// Function to determine the extent of the "inked" area of the glyphs in a string
void Fl_GDI_Graphics_Driver::text_extents(const char *c, int n, int &dx, int &dy, int &w, int &h) {

  Fl_Font_Descriptor *fl_fontsize = font_descriptor();
  if (!fl_fontsize) { // no valid font, nothing to measure
    w = 0; h = 0;
    dx = dy = 0;
    return;
  }

  static unsigned short *ext_buff = NULL; // UTF-16 converted version of input UTF-8 string
  static WORD  *w_buff = NULL; // glyph indices array
  static unsigned wc_len = 0;  // current string buffer dimensions
  static const MAT2 matrix = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } }; // identity mat for GetGlyphOutlineW
  GLYPHMETRICS metrics;
  int maxw = 0, maxh = 0, dh;
  int minx = 0, miny = -999999;
  unsigned len = 0, idx = 0;
  HWND hWnd = 0;
  HDC gc = fl_gc; // local copy of current gc - make a copy in case we change it...
  int has_surrogates; // will be set if the string contains surrogate pairs

  // Have we loaded the GetGlyphIndicesW function yet?
  if (have_loaded_GetGlyphIndices == 0) {
    GetGlyphIndices_init();
  }
  // Do we have a usable GetGlyphIndices function?
  if(!fl_GetGlyphIndices) goto exit_error; // No GetGlyphIndices function, use fallback mechanism instead

  // The following code makes a best effort attempt to obtain a valid fl_gc.
  // See description in fl_width() above for an explanation.
  if (!gc) { // We have no valid gc, try and obtain one
    // Use our first fltk window, or fallback to using the screen via GetDC(NULL)
    hWnd = Fl::first_window() ? fl_xid(Fl::first_window()) : NULL;
    gc = GetDC(hWnd);
  }
  if (!gc) goto exit_error; // no valid gc, attempt to use fallback measure

  // now convert the string to WCHAR and measure it
  len = fl_utf8toUtf16(c, n, ext_buff, wc_len);
  if(len >= wc_len) {
    if(ext_buff) {delete [] ext_buff;}
    if(w_buff) {delete [] w_buff;}
    wc_len = len + 64;
    ext_buff = new unsigned short[wc_len];
    w_buff = new WORD[wc_len];
    len = fl_utf8toUtf16(c, n, ext_buff, wc_len);
  }
  SelectObject(gc, fl_fontsize->fid);

  // Are there surrogate-pairs in this string? If so GetGlyphIndicesW will fail
  // since it can only handle the BMP range.
  // We ideally want to use GetGlyphIndicesW, as it is the Right Thing, but it
  // only works for the BMP, so we leverage GetCharacterPlacementW instead, which
  // is not ideal, but works adequately well, and does handle surrogate pairs.
  has_surrogates = 0;
  for(unsigned ll = 0; ll < len; ll++) {
    if((ext_buff[ll] >= 0xD800) && (ext_buff[ll] < 0xE000)) {
      has_surrogates = -1;
      break;
    }
  }
  if (has_surrogates) {
    // GetGlyphIndices will not work - use GetCharacterPlacementW() instead
    GCP_RESULTSW gcp_res;
    memset(w_buff, 0, (sizeof(WORD) * wc_len));
    memset(&gcp_res, 0, sizeof(GCP_RESULTSW));
    gcp_res.lpGlyphs = (LPWSTR)w_buff;
    gcp_res.nGlyphs = wc_len;
    gcp_res.lStructSize = sizeof(gcp_res);

    DWORD dr = GetCharacterPlacementW(gc, (WCHAR*)ext_buff, len, 0, &gcp_res, GCP_GLYPHSHAPE);
    if(dr) {
      len = gcp_res.nGlyphs;
    } else goto exit_error;
  } else {
    if (fl_GetGlyphIndices(gc, (WCHAR*)ext_buff, len, w_buff, GGI_MARK_NONEXISTING_GLYPHS) == GDI_ERROR) {
      // some error occured here - just return fl_measure values
      goto exit_error;
    }
  }

  // now we have the glyph array we measure each glyph in turn...
  for(idx = 0; idx < len; idx++){
    if (GetGlyphOutlineW (gc, w_buff[idx], GGO_METRICS | GGO_GLYPH_INDEX,
                          &metrics, 0, NULL, &matrix) == GDI_ERROR) {
      goto exit_error;
    }
    maxw += metrics.gmCellIncX;
    if(idx == 0) minx = metrics.gmptGlyphOrigin.x;
    dh = metrics.gmBlackBoxY - metrics.gmptGlyphOrigin.y;
    if(dh > maxh) maxh = dh;
    if(miny < metrics.gmptGlyphOrigin.y) miny = metrics.gmptGlyphOrigin.y;
  }
  // for the last cell, we only want the bounding X-extent, not the glyphs increment step
  maxw = maxw - metrics.gmCellIncX + metrics.gmBlackBoxX + metrics.gmptGlyphOrigin.x;
  w = maxw - minx;
  h = maxh + miny;
  dx = minx;
  dy = -miny;
  EXTENTS_UPDATE(dx, dy, w, h);
  return; // normal exit

exit_error:
  // some error here - just return fl_measure values
  w = (int)width(c, n);
  h = height();
  dx = 0;
  dy = descent() - h;
  EXTENTS_UPDATE(dx, dy, w, h);
  return;
} // fl_text_extents

void Fl_GDI_Graphics_Driver::draw(const char* str, int n, int x, int y) {
  COLORREF oldColor = SetTextColor(fl_gc, fl_RGB());
  // avoid crash if no font has been set yet
  if (!font_descriptor()) this->font(FL_HELVETICA, FL_NORMAL_SIZE);
  SelectObject(fl_gc, font_descriptor()->fid);
  int wn = fl_utf8toUtf16(str, n, wstr, wstr_len);
  if(wn >= wstr_len) {
    wstr = (unsigned short*) realloc(wstr, sizeof(unsigned short) * (wn + 1));
    wstr_len = wn + 1;
    wn = fl_utf8toUtf16(str, n, wstr, wstr_len);
  }
  TextOutW(fl_gc, x, y, (WCHAR*)wstr, wn);
  SetTextColor(fl_gc, oldColor); // restore initial state
}

void Fl_GDI_Graphics_Driver::draw(int angle, const char* str, int n, int x, int y) {
  fl_font(this, Fl_Graphics_Driver::font(), size(), angle);
  int wn = 0; // count of UTF16 cells to render full string
  COLORREF oldColor = SetTextColor(fl_gc, fl_RGB());
  SelectObject(fl_gc, font_descriptor()->fid);
  wn = fl_utf8toUtf16(str, n, wstr, wstr_len);
  if(wn >= wstr_len) { // Array too small
    wstr = (unsigned short*) realloc(wstr, sizeof(unsigned short) * (wn + 1));
    wstr_len = wn + 1;
    wn = fl_utf8toUtf16(str, n, wstr, wstr_len); // respin the translation
  }
  TextOutW(fl_gc, x, y, (WCHAR*)wstr, wn);
  SetTextColor(fl_gc, oldColor);
  fl_font(this, Fl_Graphics_Driver::font(), size(), 0);
}

void Fl_GDI_Graphics_Driver::rtl_draw(const char* c, int n, int x, int y) {
  int wn;
  wn = fl_utf8toUtf16(c, n, wstr, wstr_len);
  if(wn >= wstr_len) {
    wstr = (unsigned short*) realloc(wstr, sizeof(unsigned short) * (wn + 1));
    wstr_len = wn + 1;
    wn = fl_utf8toUtf16(c, n, wstr, wstr_len);
  }

  COLORREF oldColor = SetTextColor(fl_gc, fl_RGB());
  SelectObject(fl_gc, font_descriptor()->fid);
#ifdef RTL_CHAR_BY_CHAR
  int i = 0;
  int lx = 0;
  while (i < wn) { // output char by char is very bad for Arabic but coherent with fl_width()
    lx = (int) width(wstr[i]);
    x -= lx;
    TextOutW(fl_gc, x, y, (WCHAR*)wstr + i, 1);
    if (fl_nonspacing(wstr[i])) {
      x += lx;
    }
    i++;
  }
#else
  UINT old_align = SetTextAlign(fl_gc, TA_RIGHT | TA_RTLREADING);
  TextOutW(fl_gc, x, y - height() + descent(), (WCHAR*)wstr, wn);
  SetTextAlign(fl_gc, old_align);
#endif
  SetTextColor(fl_gc, oldColor);
}
#endif
//
// End of "$Id$".
//
