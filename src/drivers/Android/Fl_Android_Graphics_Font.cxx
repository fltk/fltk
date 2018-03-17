//
// "$Id$"
//
// Graphics routines for the Fast Light Tool Kit (FLTK).
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


#include "Fl_Android_Graphics_Font.H"
#include "Fl_Android_Application.H"
#include <FL/fl_draw.H>
#include <errno.h>

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"


//struct Fl_Fontdesc {
//  const char *name;
//  char fontname[128];  // "Pretty" font name
//  Fl_Font_Descriptor *first;  // linked list of sizes of this style
//};


// TODO: font names starting with a $ will have the system font path inserted
// TODO: font names starting with a . or / have a known path and will not change
// TODO: font names starting with a ~ will have the package resource path for fonts added
static Fl_Fontdesc built_in_table[] = {
        {"Roboto-Regular"},
        {"Roboto-Bold"},
        {"Roboto-Italic"},
        {"Roboto-BoldItalic"},
        {"CutiveMono"},
        {"CutiveMono"}, // sorry no bold
        {"CutiveMono"}, // sorry no italic
        {"CutiveMono"}, // sorry no bold-italic
        {"NotoSerif-Regular"},
        {"NotoSerif-Bold"},
        {"NotoSerif-Italic"},
        {"NotoSerif-BoldItalic"},
        {"Roboto-Regular"},
        {"DroidSansMono"},
        {"DroidSansMono"}, // sorry no bold
        {"Roboto-Regular"},
};

Fl_Fontdesc* fl_fonts = built_in_table;


/**
 * Create an empty Bytemap.
 */
Fl_Android_Bytemap::Fl_Android_Bytemap() :
        pBytes(0L)
{
}

/**
 * Destroy the Bytemap and its allocated resources.
 */
Fl_Android_Bytemap::~Fl_Android_Bytemap()
{
  if (pBytes) ::free(pBytes);
}


// -----------------------------------------------------------------------------

/**
 * Create a True Type font manager.
 * @param fname the name of the font as it appears in the fl_fonts table.
 * @param fnum the index into the fl_fonts table
 */
Fl_Android_Font_Source::Fl_Android_Font_Source(const char *fname, Fl_Font fnum) :
        pFileBuffer(0L),
        pName(fname),
        pFontIndex(fnum)
{
}

/**
 * Release all resources.
 */
Fl_Android_Font_Source::~Fl_Android_Font_Source()
{
  if (pFileBuffer) ::free(pFileBuffer);
  // pFont does not allocate any buffers and needs no destructor
}

/**
 * Load a True Type font file and initialize the TTF interpreter.
 * A copy of the font file must remain in memory for the interpreter to work.
 */
void Fl_Android_Font_Source::load_font()
{
  if (pFileBuffer==0) {
    char buf[1024];
    sprintf(buf, "/system/fonts/%s.ttf", fl_fonts[pFontIndex].name);
    FILE *f = fopen(buf, "rb");
    if (f==NULL) {
      Fl_Android_Application::log_e("ERROR reading font %d from '%s'!", errno, buf);
      return;
    }
    fseek(f, 0, SEEK_END);
    size_t fsize = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    pFileBuffer = (uint8_t*)malloc(fsize);
    fread(pFileBuffer, 1, fsize, f);
    fclose(f);
    stbtt_InitFont(&pFont, pFileBuffer, stbtt_GetFontOffsetForIndex(pFileBuffer,0));
  }
}

/**
 * Return a bytemap for the give unicode character.
 * @param c unicode character
 * @param size height in pixels
 * @return a bytemap
 */
Fl_Android_Bytemap *Fl_Android_Font_Source::get_bytemap(uint32_t c, int size)
{
  if (pFileBuffer==0) load_font();

  Fl_Android_Bytemap *byteMap = new Fl_Android_Bytemap();

#if 0
  scale = stbtt_ScaleForPixelHeight(&font, 15);
   stbtt_GetFontVMetrics(&font, &ascent,0,0);
   baseline = (int) (ascent*scale);

   while (text[ch]) {
      int advance,lsb,x0,y0,x1,y1;
      float x_shift = xpos - (float) floor(xpos);
      stbtt_GetCodepointHMetrics(&font, text[ch], &advance, &lsb);
      stbtt_GetCodepointBitmapBoxSubpixel(&font, text[ch], scale,scale,x_shift,0, &x0,&y0,&x1,&y1);
      stbtt_MakeCodepointBitmapSubpixel(&font, &screen[baseline + y0][(int) xpos + x0], x1-x0,y1-y0, 79, scale,scale,x_shift,0, text[ch]);
      // note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
      // because this API is really for baking character bitmaps into textures. if you want to render
      // a sequence of characters, you really need to render each bitmap to a temp buffer, then
      // "alpha blend" that into the working buffer
      xpos += (advance * scale);
      if (text[ch+1])
         xpos += scale*stbtt_GetCodepointKernAdvance(&font, text[ch],text[ch+1]);
      ++ch;
   }

#endif

  float hgt = stbtt_ScaleForPixelHeight(&pFont, size);
  byteMap->pBytes = stbtt_GetCodepointBitmap(&pFont, 0, hgt, c,
                                             &byteMap->pWidth, &byteMap->pHeight,
                                             &byteMap->pXOffset, &byteMap->pYOffset);
  byteMap->pStride = byteMap->pWidth;

  return byteMap;
}

/**
 * Get the width of the character in pixels.
 * This is not a good function because character advance also depends on kerning
 * which takes the next character in a text line into account. Also, FLTK is
 * limited to interger character positions, and so is the Android driver.
 * @param c unicode character
 * @param size height in pixels
 * @return width in pixels to the start of the next character
 */
float Fl_Android_Font_Source::get_advance(uint32_t c, Fl_Fontsize size)
{
  int advance, lsb;

  if (pFileBuffer==0) load_font();
  stbtt_GetCodepointHMetrics(&pFont, c, &advance, &lsb);
  float scale = stbtt_ScaleForPixelHeight(&pFont, size);
  return scale * advance;
}


// -----------------------------------------------------------------------------

/**
 * Create a new font descriptor.
 * @param fname name of this font as in fl_fonts
 * @param fsrc the font source for this font; there is one single font source
 *        for all hights of a single font
 * @param fnum index into the fl_fonts array
 * @param fsize height of font in pixels
 */
Fl_Android_Font_Descriptor::Fl_Android_Font_Descriptor(const char *fname, Fl_Android_Font_Source *fsrc, Fl_Font fnum, Fl_Fontsize fsize) :
        Fl_Font_Descriptor(fname, fsize),
        pFontSource(fsrc),
        pFontIndex(fnum)
{
  if (!pFontSource) {
    pFontSource = new Fl_Android_Font_Source(fname, fnum);
  }
  // --- We probably must fill these values in:
  //  Fl_Font_Descriptor *next;
  //  Fl_Fontsize size; /**< font size */
  //  Fl_Font_Descriptor(const char* fontname, Fl_Fontsize size);
  //          FL_EXPORT ~Fl_Font_Descriptor() {}
  //  short ascent, descent, q_width;
  //  unsigned int listbase; // base of display list, 0 = none
}

/*
 * Get the width of the character in pixels.
 * @param c unicode character
 * @return width in pixels to the start of the next character
 */
float Fl_Android_Font_Descriptor::get_advance(uint32_t c)
{
  return pFontSource->get_advance(c, size);
}

/**
 * Get the pixels for a given Unicode character.
 * @param c unicode character
 * @return a bytemap
 */
Fl_Android_Bytemap *Fl_Android_Font_Descriptor::get_bytemap(uint32_t c)
{
  // TODO: cache bytemaps here for fast access
  return pFontSource->get_bytemap(c, size);
}


/**
 * Find or create a font descriptor for a given font and height.
 * @param fnum index into fl_fonts
 * @param size height in pixels
 * @return an existing oder newly created descriptor
 */
Fl_Android_Font_Descriptor* Fl_Android_Font_Descriptor::find(Fl_Font fnum, Fl_Fontsize size)
{
  Fl_Fontdesc &s = fl_fonts[fnum];
  if (!s.name) s = fl_fonts[0]; // use 0 if fnum undefined

  Fl_Font_Descriptor *f;
  for (f = s.first; f; f = f->next) {
    if (f->size==size) return (Fl_Android_Font_Descriptor*)f;
  }

  Fl_Android_Font_Source *fsrc = nullptr;
  if (s.first) fsrc = ((Fl_Android_Font_Descriptor*)s.first)->get_font_source();

  Fl_Android_Font_Descriptor *af = new Fl_Android_Font_Descriptor(s.name, fsrc, fnum, size);
  af->next = s.first;
  s.first = af;
  return af;
}


// =============================================================================

/**
 * Set a font for future use in text rendering calls.
 * @param fnum index into fl_fonts
 * @param size height in pixels
 */
void Fl_Android_Graphics_Driver::font_unscaled(Fl_Font fnum, Fl_Fontsize size) {
  font_descriptor( Fl_Android_Font_Descriptor::find(fnum, size) );
  size_ = size;
  font_ = fnum;
}

/**
 * Copy a single letter to the screen.
 * @param xx, yy position of character on screen
 * @param c unicode character
 * @return x position of next character on screen
 */
int Fl_Android_Graphics_Driver::render_letter(int xx, int yy, uint32_t c)
{
  int oxx = xx;

  // find the font descriptor
  Fl_Android_Font_Descriptor *fd = (Fl_Android_Font_Descriptor*)font_descriptor();
  if (!fd) return xx; // this should not happen

  Fl_Android_Bytemap *bm = fd->get_bytemap(c);

  // rrrr.rggg.gggb.bbbb
  xx += bm->pXOffset; yy += bm->pYOffset;
  uint16_t cc = make565(fl_color()), cc12 = (cc&0xf7de)>>1, cc14 = (cc12&0xf7de)>>1, cc34 = cc12+cc14;
  int32_t ss = pStride;
  uint16_t *bits = pBits;
  uint32_t ww = bm->pWidth;
  uint32_t hh = bm->pHeight;
  for (uint32_t iy = 0; iy<hh; ++iy) {
    uint16_t *d = bits + (yy+iy)*ss + xx;
    unsigned char *s = bm->pBytes + iy*bm->pStride;
    for (uint32_t ix = 0; ix<ww; ++ix) {
#if 1
      // 5 step antialiasing
      unsigned char v = *s++;
      if (v>200) { // 100% black
        *d = cc;
      } else if (v<50) { // 0%
      } else if (v>150) { // 75%
        uint16_t nn = *d, nn14 = (nn&(uint16_t(0xe79c)))>>2;
        *d = nn14 + cc34;
      } else if (v<100) { // 25%
        uint16_t nn = *d, nn12 = (nn&(uint16_t(0xf7de)))>>1, nn14 = (nn12&(uint16_t(0xf7de)))>>1, nn34 = nn12+nn14;
        *d = nn34 + cc14;
      } else { // 50%
        uint16_t nn = *d, nn12 = (nn&(uint16_t(0xf7de)))>>1;
        *d = nn12 + cc12;
      }
#else
      // pure black and white
      if (*s++ > 128)
        *d = cc;
#endif
      d++;
    }
  }
  delete bm;
  return oxx + (int)(fd->get_advance(c)+0.5f);
}

/**
 * Render a string to screen.
 * @param str text in utf-8 encoding
 * @param n number of bytes to render
 * @param x, y position on screen
 */
void Fl_Android_Graphics_Driver::draw_unscaled(const char* str, int n, int x, int y)
{
  if (str) {
    const char *e = str+n;
    for (int i=0; i<n; ) {
      int incr = 1;
      unsigned uniChar = fl_utf8decode(str + i, e, &incr);
      int x1 = x;
      x = render_letter(x, y, uniChar);
      Fl_Color old = fl_color();
      fl_color(FL_RED);
      fl_xyline(x1, y, x);
      fl_color(old);
      i += incr;
    }
  }
}

#if 0

// TODO: do we need that?
const char* Fl_Xlib_Graphics_Driver::get_font_name(Fl_Font fnum, int* ap) {
  Fl_Xlib_Fontdesc *f = ((Fl_Xlib_Fontdesc*)fl_fonts) + fnum;
  if (!f->fontname[0]) {
    int type = 0;
    const char* p = f->name;
    if (!p) {
      if (ap) *ap = 0;
      return "";
    }
    char *o = f->fontname;

    if (*p != '-') { // non-standard font, just replace * with spaces:
      if (strstr(p,"bold")) type = FL_BOLD;
      if (strstr(p,"ital")) type |= FL_ITALIC;
      for (;*p; p++) {
	if (*p == '*' || *p == ' ' || *p == '-') {
	  do p++; while (*p == '*' || *p == ' ' || *p == '-');
	  if (!*p) break;
	  if (o < (f->fontname + ENDOFBUFFER - 1)) *o++ = ' ';
	}
	if (o < (f->fontname + ENDOFBUFFER - 1)) *o++ = *p;
      }
      *o = 0;

    } else { // standard dash-separated font:

      // get the family:
      const char *x = fl_font_word(p,2); if (*x) x++; if (*x=='*') x++;
      if (!*x) {
	if (ap) *ap = 0;
	return p;
      }
      const char *e = fl_font_word(x,1);
      if ((e - x) < (int)(ENDOFBUFFER - 1)) {
	// MRS: we want strncpy here, not strlcpy...
	strncpy(o,x,e-x);
	o += e-x;
      } else {
	strlcpy(f->fontname, x, ENDOFBUFFER);
	o = f->fontname+ENDOFBUFFER-1;
      }

      // collect all the attribute words:
      for (int n = 3; n <= 6; n++) {
	// get the next word:
	if (*e) e++; x = e; e = fl_font_word(x,1);
	int t = attribute(n,x);
	if (t < 0) {
	  if (o < (f->fontname + ENDOFBUFFER - 1)) *o++ = ' ';
	  if ((e - x) < (int)(ENDOFBUFFER - (o - f->fontname) - 1)) {
	    // MRS: we want strncpy here, not strlcpy...
	    strncpy(o,x,e-x);
	    o += e-x;
	  } else {
	    strlcpy(o,x, ENDOFBUFFER - (o - f->fontname) - 1);
	    o = f->fontname+ENDOFBUFFER-1;
	  }
	} else type |= t;
      }

      // skip over the '*' for the size and get the registry-encoding:
      x = fl_font_word(e,2);
      if (*x) {x++; *o++ = '('; while (*x) *o++ = *x++; *o++ = ')';}

      *o = 0;
      if (type & FL_BOLD) strlcat(f->fontname, " bold", ENDOFBUFFER);
      if (type & FL_ITALIC) strlcat(f->fontname, " italic", ENDOFBUFFER);
    }
    f->fontname[ENDOFBUFFER] = (char)type;
  }
  if (ap) *ap = f->fontname[ENDOFBUFFER];
  return f->fontname;
}

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

#endif

//
// End of "$Id$".
//
