//
// "$Id$"
//
// MacOS font selection routines for the Fast Light Tool Kit (FLTK).
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

#include "../../config_lib.h"
#ifdef FL_CFG_GFX_QUARTZ

#include "Fl_Quartz_Graphics_Driver.h"
#include "../Cocoa/Fl_Cocoa_Screen_Driver.H"
#include "Fl_Font.H"
#include <math.h>
#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/fl_utf8.h> // for fl_utf8toUtf16()

Fl_Fontdesc* fl_fonts = NULL;

static CGAffineTransform font_mx = { 1, 0, 0, -1, 0, 0 };

const int Fl_Quartz_Graphics_Driver::CoreText_threshold = 100500; // this represents Mac OS 10.5

static int fl_free_font = FL_FREE_FONT;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5

static CFMutableDictionaryRef attributes = NULL;

static Fl_Fontdesc built_in_table_PS[] = { // PostScript font names preferred when Mac OS ≥ 10.5
  {"ArialMT"},
  {"Arial-BoldMT"},
  {"Arial-ItalicMT"},
  {"Arial-BoldItalicMT"},
  {"Courier"},
  {"Courier-Bold"},
  {"Courier-Oblique"},
  {"Courier-BoldOblique"},
  {"TimesNewRomanPSMT"},
  {"TimesNewRomanPS-BoldMT"},
  {"TimesNewRomanPS-ItalicMT"},
  {"TimesNewRomanPS-BoldItalicMT"},
  {"Symbol"},
  {"Monaco"},
  {"AndaleMono"}, // there is no bold Monaco font on standard Mac
  {"ZapfDingbatsITC"}
};
#endif

#if HAS_ATSU
static Fl_Fontdesc built_in_table_full[] = { // full font names used before 10.5
  {"Arial"},
  {"Arial Bold"},
  {"Arial Italic"},
  {"Arial Bold Italic"},
  {"Courier"},
  {"Courier Bold"},
  {"Courier New Italic"},
  {"Courier New Bold Italic"},
  {"Times New Roman"},
  {"Times New Roman Bold"},
  {"Times New Roman Italic"},
  {"Times New Roman Bold Italic"},
  {"Symbol"},
  {"Monaco"},
  {"Andale Mono"}, // there is no bold Monaco font on standard Mac
  {"Webdings"}
};
#endif

// Bug: older versions calculated the value for *ap as a side effect of
// making the name, and then forgot about it. To avoid having to change
// the header files I decided to store this value in the last character
// of the font name array.
#define ENDOFBUFFER  sizeof(fl_fonts->fontname)-1

// turn a stored font name into a pretty name:
const char* Fl_Quartz_Graphics_Driver::get_font_name(Fl_Font fnum, int* ap) {
  if (!fl_fonts) fl_fonts = calc_fl_fonts();
  Fl_Fontdesc *f = fl_fonts + fnum;
  if (!f->fontname[0]) {
    this->set_fontname_in_fontdesc(f);
    const char* p = f->name;
    if (!p || !*p) {if (ap) *ap = 0; return "";}
    int type = 0;
    if (strstr(f->name, "Bold")) type |= FL_BOLD;
    if (strstr(f->name, "Italic") || strstr(f->name, "Oblique")) type |= FL_ITALIC;
    f->fontname[ENDOFBUFFER] = (char)type;
  }
  if (ap) *ap = f->fontname[ENDOFBUFFER];
  return f->fontname;
}


int Fl_Quartz_Graphics_Driver::get_font_sizes(Fl_Font fnum, int*& sizep) {
  static int array[128];
  if (!fl_fonts) fl_fonts = calc_fl_fonts();
  Fl_Fontdesc *s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // empty slot in table, use entry 0
  int cnt = 0;

  // ATS supports all font size 
  array[0] = 0;
  sizep = array;
  cnt = 1;

  return cnt;
}

Fl_Font_Descriptor::Fl_Font_Descriptor(const char* name, Fl_Fontsize Size) {
  next = 0;
#  if HAVE_GL
  listbase = 0;
#  endif
  // OpenGL needs those for its font handling
  size = Size;
  fontref = NULL;
  layout = NULL;
  Fl_Quartz_Graphics_Driver *driver = (Fl_Quartz_Graphics_Driver*)Fl_Display_Device::display_device()->driver();
  driver->descriptor_init(name, size, this);
}


Fl_Font_Descriptor::~Fl_Font_Descriptor() {
/*
#if HAVE_GL
 // ++ todo: remove OpenGL font allocations
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
  if (this == fl_graphics_driver->font_descriptor()) fl_graphics_driver->font_descriptor(NULL);
  if (fontref) {
    CFRelease(fontref);
    for (unsigned i = 0; i < sizeof(width)/sizeof(float*); i++) {
      if (width[i]) free(width[i]);
    }
  }
#if HAS_ATSU
  if (layout) {
    ATSUDisposeTextLayout(layout);
    ATSUDisposeStyle(style);
  }
#endif
}


static UniChar *utfWbuf = 0;
static unsigned utfWlen = 0;

static UniChar *mac_Utf8_to_Utf16(const char *txt, int len, int *new_len)
{
  unsigned wlen = fl_utf8toUtf16(txt, len, (unsigned short*)utfWbuf, utfWlen);
  if (wlen >= utfWlen)
  {
    utfWlen = wlen + 100;
	if (utfWbuf) free(utfWbuf);
    utfWbuf = (UniChar*)malloc((utfWlen)*sizeof(UniChar));
	wlen = fl_utf8toUtf16(txt, len, (unsigned short*)utfWbuf, utfWlen);
  }
  *new_len = wlen;
  return utfWbuf;
}


static Fl_Font_Descriptor* find(Fl_Font fnum, Fl_Fontsize size) {
  if (!fl_fonts) fl_fonts = Fl_Graphics_Driver::default_driver().calc_fl_fonts();
  Fl_Fontdesc* s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // use 0 if fnum undefined
  Fl_Font_Descriptor* f;
  for (f = s->first; f; f = f->next)
    if (f->size == size) return f;
  f = new Fl_Font_Descriptor(s->name, size);
  f->next = s->first;
  s->first = f;
  return f;
}


void Fl_Quartz_Graphics_Driver::font(Fl_Font fnum, Fl_Fontsize size) {
  if (fnum==-1) {
    Fl_Graphics_Driver::font(0, 0);
    return;
  }
  Fl_Graphics_Driver::font(fnum, size);
  this->font_descriptor( find(fnum, size) );
}

int Fl_Quartz_Graphics_Driver::height() {
  if (!font_descriptor()) font(FL_HELVETICA, FL_NORMAL_SIZE);
  Fl_Font_Descriptor *fl_fontsize = font_descriptor();
  return fl_fontsize->ascent + fl_fontsize->descent;
}

int Fl_Quartz_Graphics_Driver::descent() {
  if (!font_descriptor()) font(FL_HELVETICA, FL_NORMAL_SIZE);
  Fl_Font_Descriptor *fl_fontsize = font_descriptor();
  return fl_fontsize->descent+1;
}

void Fl_Quartz_Graphics_Driver::draw(const char *str, int n, float x, float y) {
  // avoid a crash if no font has been selected by user yet !
  if (!font_descriptor()) font(FL_HELVETICA, FL_NORMAL_SIZE);
  draw_float(x, y, str, n);
}

void Fl_Quartz_Graphics_Driver::draw(const char* str, int n, int x, int y) {
  // avoid a crash if no font has been selected by user yet !
  if (!font_descriptor()) font(FL_HELVETICA, FL_NORMAL_SIZE);
  //fl_mac_draw(str, n, (float)x-0.0f, (float)y+0.5f, this);
  draw_float((float)x, y+0.5f, str, n);
}

void Fl_Quartz_Graphics_Driver::draw(int angle, const char *str, int n, int x, int y) {
  CGContextSaveGState(gc_);
  CGContextTranslateCTM(gc_, x, y);
  CGContextRotateCTM(gc_, - angle*(M_PI/180) );
  draw(str, n, 0, 0);
  CGContextRestoreGState(gc_);
}

void Fl_Quartz_Graphics_Driver::rtl_draw(const char* c, int n, int x, int y) {
  int dx, dy, w, h;
  text_extents(c, n, dx, dy, w, h);
  draw(c, n, x - w - dx, y);
}

double Fl_Quartz_Graphics_Driver::width(const char* txt, int n) {
  int wc_len = n;
  UniChar *uniStr = mac_Utf8_to_Utf16(txt, n, &wc_len);
  if (!font_descriptor()) font(FL_HELVETICA, FL_NORMAL_SIZE);
  return width(uniStr, wc_len, font_descriptor());
}

double Fl_Quartz_Graphics_Driver::width(unsigned int wc) {
  if (!font_descriptor()) font(FL_HELVETICA, FL_NORMAL_SIZE);
  
  UniChar utf16[3];
  int l = 1;
  if (wc <= 0xFFFF) {
    *utf16 = wc;
  }
  else {
    l = (int)fl_ucs_to_Utf16(wc, utf16, 3);
  }
  return width(utf16, l, font_descriptor());
}


void Fl_Quartz_Graphics_Driver::set_fontname_in_fontdesc(Fl_Fontdesc *f) {
  strlcpy(f->fontname, f->name, ENDOFBUFFER);
}


unsigned Fl_Quartz_Graphics_Driver::font_desc_size() {
  return (unsigned)sizeof(Fl_Fontdesc);
}

const char *Fl_Quartz_Graphics_Driver::font_name(int num) {
  if (!fl_fonts) fl_fonts = calc_fl_fonts();
  return fl_fonts[num].name;
}

void Fl_Quartz_Graphics_Driver::font_name(int num, const char *name) {
  Fl_Fontdesc *s = fl_fonts + num;
  if (s->name) {
    if (!strcmp(s->name, name)) {s->name = name; return;}
    for (Fl_Font_Descriptor* f = s->first; f;) {
      Fl_Font_Descriptor* n = f->next; delete f; f = n;
    }
    s->first = 0;
  }
  s->name = name;
  s->fontname[0] = 0;
  s->first = 0;
}



#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
/// CoreText-based code

Fl_Fontdesc* Fl_CoreText_Graphics_Driver::calc_fl_fonts(void)
{
  return  built_in_table_PS;
}


void Fl_CoreText_Graphics_Driver::descriptor_init(const char* name, Fl_Fontsize size, Fl_Font_Descriptor *d) {
  CFStringRef str = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
  d->fontref = CTFontCreateWithName(str, size, NULL);
  CGGlyph glyph[2];
  const UniChar A[2]={'W','.'};
  CTFontGetGlyphsForCharacters(d->fontref, A, glyph, 2);
  CGSize advances[2];
  double w;
  CTFontGetAdvancesForGlyphs(d->fontref, kCTFontHorizontalOrientation, glyph, advances, 2);
  w = advances[0].width;
  if ( fabs(advances[0].width - advances[1].width) < 1E-2 ) {//this is a fixed-width font
    // slightly rescale fixed-width fonts so the character width has an integral value
    CFRelease(d->fontref);
    CGFloat fsize = size / ( w/floor(w + 0.5) );
    d->fontref = CTFontCreateWithName(str, fsize, NULL);
    w = CTFontGetAdvancesForGlyphs(d->fontref, kCTFontHorizontalOrientation, glyph, NULL, 1);
  }
  CFRelease(str);
  d->ascent = (short)(CTFontGetAscent(d->fontref) + 0.5);
  d->descent = (short)(CTFontGetDescent(d->fontref) + 0.5);
  d->q_width = w + 0.5;
  for (unsigned i = 0; i < sizeof(d->width)/sizeof(float*); i++) d->width[i] = NULL;
  if (!attributes) {
    static CFNumberRef zero_ref;
    float zero = 0.;
    zero_ref = CFNumberCreate(NULL, kCFNumberFloat32Type, &zero);
    // deactivate kerning for all fonts, so that string width = sum of character widths
    // which allows fast fl_width() implementation.
    attributes = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                           3,
                                           &kCFTypeDictionaryKeyCallBacks,
                                           &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue (attributes, kCTKernAttributeName, zero_ref);
  }
  if (d->ascent == 0) { // this may happen with some third party fonts
    CFDictionarySetValue (attributes, kCTFontAttributeName, d->fontref);
    CFAttributedStringRef mastr = CFAttributedStringCreate(kCFAllocatorDefault, CFSTR("Wj"), attributes);
    CTLineRef ctline = CTLineCreateWithAttributedString(mastr);
    CFRelease(mastr);
    CGFloat fascent, fdescent;
    CTLineGetTypographicBounds(ctline, &fascent, &fdescent, NULL);
    CFRelease(ctline);
    d->ascent = (short)(fascent + 0.5);
    d->descent = (short)(fdescent + 0.5);
  }
}

// returns width of a pair of UniChar's in the surrogate range
static CGFloat surrogate_width(const UniChar *txt, Fl_Font_Descriptor *fl_fontsize)
{
  CTFontRef font2 = fl_fontsize->fontref;
  bool must_release = false;
  CGGlyph glyphs[2];
  bool b = CTFontGetGlyphsForCharacters(font2, txt, glyphs, 2);
  CGSize a;
  if(!b) { // the current font doesn't contain this char
    CFStringRef str = CFStringCreateWithCharactersNoCopy(NULL, txt, 2, kCFAllocatorNull);
    // find a font that contains it
    font2 = CTFontCreateForString(font2, str, CFRangeMake(0,2));
    must_release = true;
    CFRelease(str);
    b = CTFontGetGlyphsForCharacters(font2, txt, glyphs, 2);
  }
  if (b) CTFontGetAdvancesForGlyphs(font2, kCTFontHorizontalOrientation, glyphs, &a, 1);
  else a.width = fl_fontsize->q_width;
  if(must_release) CFRelease(font2);
  return a.width;
}

static CGFloat variation_selector_width(CFStringRef str16, Fl_Font_Descriptor *fl_fontsize)
{
  CGFloat retval;
  CFDictionarySetValue(attributes, kCTFontAttributeName, fl_fontsize->fontref);
  CFAttributedStringRef mastr = CFAttributedStringCreate(kCFAllocatorDefault, str16, attributes);
  CTLineRef ctline = CTLineCreateWithAttributedString(mastr);
  CFRelease(mastr);
  retval = CTLineGetOffsetForStringIndex(ctline, 2, NULL);
  CFRelease(ctline);
  return retval;
}

double Fl_CoreText_Graphics_Driver::width(const UniChar* txt, int n, Fl_Font_Descriptor *fl_fontsize) {
  double retval = 0;
  UniChar uni;
  int i;
  for (i = 0; i < n; i++) { // loop over txt
    uni = txt[i];
    if (uni >= 0xD800 && uni <= 0xDBFF) { // handles the surrogate range
      retval += surrogate_width(&txt[i], fl_fontsize);
      i++; // because a pair of UniChar's represent a single character
      continue;
    }
    if (i+1 < n && txt[i+1] >= 0xFE00 && txt[i+1] <= 0xFE0F) { // handles variation selectors
      CFStringRef substr = CFStringCreateWithCharacters(NULL, txt + i, 2);
      retval += variation_selector_width(substr, fl_fontsize);
      CFRelease(substr);
      i++;
      continue;
    }
    const int block = 0x10000 / (sizeof(fl_fontsize->width)/sizeof(float*)); // block size
    // r: index of the character block containing uni
    unsigned int r = uni >> 7; // change 7 if sizeof(width) is changed
    if (!fl_fontsize->width[r]) { // this character block has not been hit yet
      //fprintf(stderr,"r=%d size=%d name=%s\n",r,fl_fontsize->size,fl_fonts[fl_font()].name);
      // allocate memory to hold width of each character in the block
      fl_fontsize->width[r] = (float*) malloc(sizeof(float) * block);
      UniChar ii = r * block;
      CGSize advance_size;
      CGGlyph glyph;
      for (int j = 0; j < block; j++) { // loop over the block
	// ii spans all characters of this block
	bool b = CTFontGetGlyphsForCharacters(fl_fontsize->fontref, &ii, &glyph, 1);
	if (b) 
	  CTFontGetAdvancesForGlyphs(fl_fontsize->fontref, kCTFontHorizontalOrientation, &glyph, &advance_size, 1);
	else 
	  advance_size.width = -1e9; // calculate this later
	// the width of one character of this block of characters
	fl_fontsize->width[r][j] = advance_size.width;
	ii++;
      }
    }
    // sum the widths of all characters of txt
    double wdt = fl_fontsize->width[r][uni & (block-1)];
    if (wdt == -1e9) {
      CGSize advance_size;
      CGGlyph glyph;
      CTFontRef font2 = fl_fontsize->fontref;
      bool must_release = false;
      bool b = CTFontGetGlyphsForCharacters(font2, &uni, &glyph, 1);
      if (!b) { // the current font doesn't contain this char
	CFStringRef str = CFStringCreateWithCharactersNoCopy(NULL, &uni, 1, kCFAllocatorNull);
	// find a font that contains it
	font2 = CTFontCreateForString(font2, str, CFRangeMake(0,1));
	must_release = true;
	CFRelease(str);
	b = CTFontGetGlyphsForCharacters(font2, &uni, &glyph, 1);
      }
      if (b) CTFontGetAdvancesForGlyphs(font2, kCTFontHorizontalOrientation, &glyph, &advance_size, 1);
      else advance_size.width = 0.;
      // the width of the 'uni' character
      wdt = fl_fontsize->width[r][uni & (block-1)] = advance_size.width;
      if (must_release) CFRelease(font2);
    }
    retval += wdt;
  }
  return retval;
}


// text extent calculation
void Fl_CoreText_Graphics_Driver::text_extents(const char *str8, int n, int &dx, int &dy, int &w, int &h) {
  if (!font_descriptor()) font(FL_HELVETICA, FL_NORMAL_SIZE);
  Fl_Font_Descriptor *fl_fontsize = font_descriptor();
  UniChar *txt = mac_Utf8_to_Utf16(str8, n, &n);
  CFStringRef str16 = CFStringCreateWithCharactersNoCopy(NULL, txt, n,  kCFAllocatorNull);
  CFDictionarySetValue (attributes, kCTFontAttributeName, fl_fontsize->fontref);
  CFAttributedStringRef mastr = CFAttributedStringCreate(kCFAllocatorDefault, str16, attributes);
  CFRelease(str16);
  CTLineRef ctline = CTLineCreateWithAttributedString(mastr);
  CFRelease(mastr);
  CGContextSetTextPosition(gc_, 0, 0);
  CGContextSetShouldAntialias(gc_, true);
  CGRect rect = CTLineGetImageBounds(ctline, gc_);
  CGContextSetShouldAntialias(gc_, false);
  CFRelease(ctline);
  dx = floor(rect.origin.x + 0.5);
  dy = floor(- rect.origin.y - rect.size.height + 0.5);
  w = rect.size.width + 0.5;
  h = rect.size.height + 0.5;
}


static CGColorRef flcolortocgcolor(Fl_Color i)
{
  uchar r, g, b;
  Fl::get_color(i, r, g, b);
  CGFloat components[4] = {r/255.0f, g/255.0f, b/255.0f, 1.};
  static CGColorSpaceRef cspace = NULL;
  if (cspace == NULL) {
    cspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    }
  return CGColorCreate(cspace, components);
}

void Fl_CoreText_Graphics_Driver::draw_float(float x, float y, const char *str, int n) {
  // convert to UTF-16 first
  UniChar *uniStr = mac_Utf8_to_Utf16(str, n, &n);
  CGContextRef gc = (CGContextRef)this->gc();
  CFMutableStringRef str16 = CFStringCreateMutableWithExternalCharactersNoCopy(NULL, uniStr, n,  n, kCFAllocatorNull);
  if (str16 == NULL) return; // shd not happen
  CGColorRef color = flcolortocgcolor(this->color());
  CFDictionarySetValue (attributes, kCTFontAttributeName, font_descriptor()->fontref);
  CFDictionarySetValue (attributes, kCTForegroundColorAttributeName, color);
  CFAttributedStringRef mastr = CFAttributedStringCreate(kCFAllocatorDefault, str16, attributes);
  CFRelease(str16);
  CFRelease(color);
  CTLineRef ctline = CTLineCreateWithAttributedString(mastr);
  CFRelease(mastr);
  CGContextSetTextMatrix(gc, font_mx);
  CGContextSetTextPosition(gc, x, y);
  CGContextSetShouldAntialias(gc, true);
  CTLineDraw(ctline, gc);
  CGContextSetShouldAntialias(gc, false);
  CFRelease(ctline);
}

static char *skip(char *p, int& derived)
{
  if (memcmp(p, "-BoldItalic", 11) == 0) { p += 11; derived = 3; }
  else if (memcmp(p, "-BoldOblique", 12) == 0) { p += 12; derived = 3; }
  else if (memcmp(p, "-Bold", 5) == 0) {p += 5; derived = 1; }
  else if (memcmp(p, "-Italic", 7) == 0) {p += 7; derived = 2; }
  else if (memcmp(p, "-Oblique", 8) == 0) {p += 8; derived = 2; }
  else if (memcmp(p, "-Regular", 8) == 0) {p += 8; }
  else if (memcmp(p, "-Roman", 6) == 0) {p += 6; }
  return p;
}

static int name_compare(const void *a, const void *b)
{
  /* Compare PostScript font names.
   First compare font family names ignoring bold, italic and oblique qualifiers.
   When families are identical, order them according to regular, bold, italic, bolditalic.
   */
  char *n1 = *(char**)a;
  char *n2 = *(char**)b;
  int derived1 = 0;
  int derived2 = 0;
  while (true) {
    if (*n1 == '-') n1 = skip(n1, derived1);
    if (*n2 == '-') n2 = skip(n2, derived2);
    if (*n1 < *n2) return -1;
    if (*n1 > *n2) return +1;
    if (*n1 == 0) {
      return derived1 - derived2;
    }
    n1++; n2++;
  }
}

Fl_Font Fl_CoreText_Graphics_Driver::set_fonts(const char* xstarname) {
#pragma unused ( xstarname )
  if (fl_free_font > FL_FREE_FONT) return (Fl_Font)fl_free_font; // if already called
  
  int value[1] = {1};
  CFDictionaryRef dict = CFDictionaryCreate(NULL,
                                            (const void **)kCTFontCollectionRemoveDuplicatesOption,
                                            (const void **)&value, 1, NULL, NULL);
  CTFontCollectionRef fcref = CTFontCollectionCreateFromAvailableFonts(dict);
  CFRelease(dict);
  CFArrayRef arrayref = CTFontCollectionCreateMatchingFontDescriptors(fcref);
  CFRelease(fcref);
  CFIndex count = CFArrayGetCount(arrayref);
  CFIndex i;
  char **tabfontnames = new char*[count];
  for (i = 0; i < count; i++) {
    CTFontDescriptorRef fdesc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(arrayref, i);
    CTFontRef font = CTFontCreateWithFontDescriptor(fdesc, 0., NULL);
    CFStringRef cfname = CTFontCopyPostScriptName(font);
    CFRelease(font);
    static char fname[200];
    CFStringGetCString(cfname, fname, sizeof(fname), kCFStringEncodingUTF8);
    tabfontnames[i] = strdup(fname); // never free'ed
    CFRelease(cfname);
  }
  CFRelease(arrayref);
  qsort(tabfontnames, count, sizeof(char*), name_compare);
  for (i = 0; i < count; i++) {
    Fl::set_font((Fl_Font)(fl_free_font++), tabfontnames[i]);
  }
  delete[] tabfontnames;
  return (Fl_Font)fl_free_font;
}

void Fl_CoreText_Graphics_Driver::set_fontname_in_fontdesc(Fl_Fontdesc *f) {
  CFStringRef cfname = CFStringCreateWithCString(NULL, f->name, kCFStringEncodingUTF8);
  CTFontRef ctfont = CTFontCreateWithName(cfname, 0, NULL);
  CFRelease(cfname);
  cfname = CTFontCopyFullName(ctfont);
  CFRelease(ctfont);
  CFStringGetCString(cfname, f->fontname, ENDOFBUFFER, kCFStringEncodingUTF8);
  CFRelease(cfname);
}

#endif // >= 10.5


#if HAS_ATSU
/// ATSU-based code to support Mac OS < 10.5

void Fl_ATSU_Graphics_Driver::descriptor_init(const char* name, Fl_Fontsize size, Fl_Font_Descriptor *d) {
  OSStatus err;
  // fill our structure with a few default values
  d->ascent = size*3/4.;
  d->descent = size-d->ascent;
  d->q_width = size*2/3.;
  // now we allocate everything needed to render text in this font later
  // get us the default layout and style
  err = ATSUCreateTextLayout(&d->layout);
  UniChar mTxt[2] = { 65, 0 };
  err = ATSUSetTextPointerLocation(d->layout, mTxt, kATSUFromTextBeginning, 1, 1);
  err = ATSUCreateStyle(&d->style);
  err = ATSUSetRunStyle(d->layout, d->style, kATSUFromTextBeginning, kATSUToTextEnd);
  // now set the actual font, size and attributes. We also set the font matrix to
  // render our font up-side-down, so when rendered through our inverted CGContext,
  // text will appear normal again.
  Fixed fsize = IntToFixed(size);
  ATSUFontID fontID;
  ATSUFindFontFromName(name, strlen(name), kFontFullName, kFontMacintoshPlatform, kFontNoScriptCode, kFontEnglishLanguage, &fontID);
  
  // draw the font upside-down... Compensate for fltk/OSX origin differences
  ATSUAttributeTag sTag[] = { kATSUFontTag, kATSUSizeTag, kATSUFontMatrixTag };
  ByteCount sBytes[] = { sizeof(ATSUFontID), sizeof(Fixed), sizeof(CGAffineTransform) };
  ATSUAttributeValuePtr sAttr[] = { &fontID, &fsize, &font_mx };
  if (fontID != kATSUInvalidFontID) err = ATSUSetAttributes(d->style, 1, sTag, sBytes, sAttr); // set the font attribute
  err = ATSUSetAttributes(d->style, 2, sTag + 1, sBytes + 1, sAttr + 1); // then the size and matrix attributes
  // next, make sure that Quartz will only render at integer coordinates
  ATSLineLayoutOptions llo = kATSLineUseDeviceMetrics | kATSLineDisableAllLayoutOperations;
  ATSUAttributeTag aTag[] = { kATSULineLayoutOptionsTag };
  ByteCount aBytes[] = { sizeof(ATSLineLayoutOptions) };
  ATSUAttributeValuePtr aAttr[] = { &llo };
  err = ATSUSetLineControls (d->layout, kATSUFromTextBeginning, 1, aTag, aBytes, aAttr);
  // now we are finally ready to measure some letter to get the bounding box
  Fixed bBefore, bAfter, bAscent, bDescent;
  err = ATSUGetUnjustifiedBounds(d->layout, kATSUFromTextBeginning, 1, &bBefore, &bAfter, &bAscent, &bDescent);
  // Requesting a certain height font on Mac does not guarantee that ascent+descent
  // equal the requested height. fl_height will reflect the actual height that we got.
  // The font "Apple Chancery" is a pretty extreme example of overlapping letters.
  float fa = -FixedToFloat(bAscent), fd = -FixedToFloat(bDescent);
  if (fa>0.0f && fd>0.0f) {
    //float f = Size/(fa+fd);
    d->ascent = int(fa); //int(fa*f+0.5f);
    d->descent = int(fd); //Size - ascent;
  }
  int w = FixedToInt(bAfter);
  if (w)
    d->q_width = FixedToInt(bAfter);
  
  // Now, by way of experiment, try enabling Transient Font Matching, this will
  // cause ATSU to find a suitable font to render any chars the current font can't do...
  ATSUSetTransientFontMatching (d->layout, true);
}

void Fl_ATSU_Graphics_Driver::draw_float(float x, float y, const char *str, int n) {
  // convert to UTF-16 first
  UniChar *uniStr = mac_Utf8_to_Utf16(str, n, &n);
  CGContextRef gc = (CGContextRef)this->gc();
  OSStatus err;
  // now collect our ATSU resources
  ATSUTextLayout layout = font_descriptor()->layout;
  
  ByteCount iSize = sizeof(CGContextRef);
  ATSUAttributeTag iTag = kATSUCGContextTag;
  ATSUAttributeValuePtr iValuePtr=&gc;
  ATSUSetLayoutControls(layout, 1, &iTag, &iSize, &iValuePtr);
  
  err = ATSUSetTextPointerLocation(layout, uniStr, kATSUFromTextBeginning, n, n);
  CGContextSetShouldAntialias(gc, true);
  err = ATSUDrawText(layout, kATSUFromTextBeginning, n, FloatToFixed(x), FloatToFixed(y));
  CGContextSetShouldAntialias(gc, false);
}

double Fl_ATSU_Graphics_Driver::width(const UniChar* txt, int n, Fl_Font_Descriptor *fl_fontsize) {
  OSStatus err;
  Fixed bBefore, bAfter, bAscent, bDescent;
  ATSUTextLayout layout;
  ByteCount iSize;
  ATSUAttributeTag iTag;
  ATSUAttributeValuePtr iValuePtr;
  
  // Here's my ATSU text measuring attempt... This seems to do the Right Thing
  // now collect our ATSU resources and measure our text string
  layout = fl_fontsize->layout;
  // activate the current GC
  iSize = sizeof(CGContextRef);
  iTag = kATSUCGContextTag;
  CGContextRef value = (CGContextRef)fl_graphics_driver->gc();
  iValuePtr = &value;
  ATSUSetLayoutControls(layout, 1, &iTag, &iSize, &iValuePtr);
  // now measure the bounding box
  err = ATSUSetTextPointerLocation(layout, txt, kATSUFromTextBeginning, n, n);
  err = ATSUGetUnjustifiedBounds(layout, kATSUFromTextBeginning, n, &bBefore, &bAfter, &bAscent, &bDescent);
  // If err is OK then return length, else return 0. Or something...
  int len = FixedToInt(bAfter);
  return len;
}

void Fl_ATSU_Graphics_Driver::text_extents(const char *str8, int n, int &dx, int &dy, int &w, int &h) {
  if (!font_descriptor()) font(FL_HELVETICA, FL_NORMAL_SIZE);
  Fl_Font_Descriptor *fl_fontsize = font_descriptor();
  UniChar *txt = mac_Utf8_to_Utf16(str8, n, &n);
  OSStatus err;
  ATSUTextLayout layout;
  ByteCount iSize;
  ATSUAttributeTag iTag;
  ATSUAttributeValuePtr iValuePtr;
  
  // Here's my ATSU text measuring attempt... This seems to do the Right Thing
  // now collect our ATSU resources and measure our text string
  layout = fl_fontsize->layout;
  // activate the current GC
  iSize = sizeof(CGContextRef);
  iTag = kATSUCGContextTag;
  CGContextRef value = (CGContextRef)gc();
  iValuePtr = &value;
  ATSUSetLayoutControls(layout, 1, &iTag, &iSize, &iValuePtr);
  // now measure the bounding box
  err = ATSUSetTextPointerLocation(layout, txt, kATSUFromTextBeginning, n, n);
  Rect bbox;
  err = ATSUMeasureTextImage(layout, kATSUFromTextBeginning, n, 0, 0, &bbox);
  w = bbox.right - bbox.left;
  h = bbox.bottom - bbox.top;
  dx = bbox.left;
  dy = -bbox.bottom;
  //printf("r: %d l: %d t: %d b: %d w: %d h: %d\n", bbox.right, bbox.left, bbox.top, bbox.bottom, w, h);
}

Fl_Font Fl_ATSU_Graphics_Driver::set_fonts(const char* xstarname) {
#pragma unused ( xstarname )
  if (fl_free_font > FL_FREE_FONT) return (Fl_Font)fl_free_font; // if already called
  
  ItemCount oFontCount, oCountAgain;
  ATSUFontID *oFontIDs;
  // How many fonts?
  ATSUFontCount (&oFontCount);
  // now allocate space for them...
  oFontIDs = (ATSUFontID *)malloc((oFontCount+1) * sizeof(ATSUFontID));
  ATSUGetFontIDs (oFontIDs, oFontCount, &oCountAgain);
  // Now oFontIDs should contain a list of all the available Unicode fonts
  // Iterate through the list to get each font name
  for (ItemCount idx = 0; idx < oFontCount; idx++)
  {
    //  ByteCount actualLength = 0;
    //	Ptr oName;
    // How to get the name - Apples docs say call this twice, once to get the length, then again
    // to get the actual name...
    //    ATSUFindFontName (oFontIDs[idx], kFontFullName, kFontMacintoshPlatform, kFontRomanScript, kFontEnglishLanguage,
    //                      0, NULL, &actualLength, NULL);
    // Now actualLength tells us the length of buffer we need
    //	oName = (Ptr)malloc(actualLength + 8);
    // But who's got time for that nonsense? Let's just hard code a fixed buffer (urgh!)
    ByteCount actualLength = 511;
    char oName[512];
    ATSUFindFontName (oFontIDs[idx], kFontFullName, kFontMacintoshPlatform, kFontRomanScript, kFontEnglishLanguage,
                      actualLength, oName, &actualLength, &oCountAgain);
    // bounds check and terminate the returned name
    if(actualLength > 511)
      oName[511] = 0;
    else
      oName[actualLength] = 0;
    Fl::set_font((Fl_Font)(fl_free_font++), strdup(oName));
    //	free(oName);
  }
  free(oFontIDs);
  return (Fl_Font)fl_free_font;
}

Fl_Fontdesc* Fl_ATSU_Graphics_Driver::calc_fl_fonts(void)
{
  return built_in_table_full;
}

#endif // HAS_ATSU

#endif // FL_CFG_GFX_QUARTZ

//
// End of "$Id$".
//
