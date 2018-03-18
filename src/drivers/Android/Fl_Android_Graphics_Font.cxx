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
#include <FL/filename.H>

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"


//struct Fl_Fontdesc {
//  const char *name;
//  char fontname[128];  // "Pretty" font name
//  Fl_Font_Descriptor *first;  // linked list of sizes of this style
//};


/**
 * - font names starting with a $ will have the system font path inserted
 * - font names starting with an @ will be loaded via the Asset Manager
 * - all other names will be used verbatim
 */
static Fl_Fontdesc built_in_table[] = {
        {"$Roboto-Regular.ttf"},
        {"$Roboto-Bold.ttf"},
        {"$Roboto-Italic.ttf"},
        {"$Roboto-BoldItalic.ttf"},
        {"$CutiveMono.ttf"},
        {"$CutiveMono.ttf"}, // sorry no bold
        {"$CutiveMono.ttf"}, // sorry no italic
        {"$CutiveMono.ttf"}, // sorry no bold-italic
        {"$NotoSerif-Regular.ttf"},
        {"$NotoSerif-Bold.ttf"},
        {"$NotoSerif-Italic.ttf"},
        {"$NotoSerif-BoldItalic.ttf"},
        {"$Roboto-Regular.ttf"},
        {"$DroidSansMono.ttf"},
        {"$DroidSansMono.ttf"}, // sorry no bold
        {"$Roboto-Regular.ttf"},
};

Fl_Fontdesc* fl_fonts = built_in_table;

static const char *old_font_names[] = {
        "$DroidSans.ttf", "$DroidSerif-Regular.ttf",
        "$DroidSansMono.ttf", "$DroidSansMono.ttf"
};

/**
 * Create an empty Bytemap.
 */
Fl_Android_Bytemap::Fl_Android_Bytemap() :
        pBytes(nullptr)
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
        pFileBuffer(nullptr),
        pName(fname),
        pFontIndex(fnum),
        pError(false)
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
 * Attempt to find an load a font file.
 * @param name file or asset name
 * @return
 */
bool Fl_Android_Font_Source::load_font(const char *name)
{
  if (pFileBuffer) return true;
  if (!name) return false;
  bool ret = false;
  if (name[0]=='@')
    ret = load_font_asset(name+1);
  else
    ret = load_font_file(name);
  return ret;
}

/**
 * Attempt to load a font through the asset manager.
 * @param name file or asset name
 * @return
 */
bool Fl_Android_Font_Source::load_font_asset(const char *name)
{
  errno = 0;
  AAssetManager *aMgr = Fl_Android_Application::get_asset_manager();
  AAsset *aFile = AAssetManager_open(aMgr, name, AASSET_MODE_STREAMING);
  if (aFile == nullptr) {
    Fl_Android_Application::log_w("Can't open font asset at '%s': ",
                                  name, strerror(errno));
    return false;
  }
  size_t fsize = (size_t)AAsset_getLength(aFile);
  if (fsize == 0) {
    Fl_Android_Application::log_w("Can't read font asset at '%s': file is empty",
                                  name);
    AAsset_close(aFile);
    return false;
  }
  pFileBuffer = (uint8_t *)malloc(fsize);
  if (AAsset_read(aFile, pFileBuffer, fsize)<=0) {
    Fl_Android_Application::log_w("Can't read font asset at '%s': ",
                                  name, strerror(errno));
    free(pFileBuffer);
    pFileBuffer = 0;
    AAsset_close(aFile);
    return false;
  }
  AAsset_close(aFile);
  return true;
}

/**
 * Attempt to load a font through the asset manager.
 * @param name file or asset name
 * @return
 */
bool Fl_Android_Font_Source::load_font_file(const char *name)
{
  char buf[2048];
  if (name[0] == '$') {
    // use the system path for fonts
    snprintf(buf, 2048, "/system/fonts/%s", name + 1);
  } else {
    strcpy(buf, name);
  }
  FILE *f = fopen(buf, "rb");
  if (f == nullptr) {
    Fl_Android_Application::log_w("Can't open font file at '%s': ",
                                  name, strerror(errno));
    return false;
  }
  fseek(f, 0, SEEK_END);
  size_t fsize = (size_t)ftell(f);
  fseek(f, 0, SEEK_SET);
  if (fsize == 0) {
    Fl_Android_Application::log_w(
            "Can't read font file at '%s': file is empty",
            name);
    fclose(f);
    return false;
  }
  pFileBuffer = (uint8_t *)malloc(fsize);
  if (fread(pFileBuffer, 1, fsize, f)<=0) {
    Fl_Android_Application::log_w("Can't read font file at '%s': ",
                                  name, strerror(errno));
    free(pFileBuffer);
    pFileBuffer = 0;
    fclose(f);
    return false;
  }
  fclose(f);
  return true;
}


/**
 * Load a True Type font file and initialize the TTF interpreter.
 * A copy of the font file must remain in memory for the interpreter to work.
 */
void Fl_Android_Font_Source::load_font()
{
  if (pError) return;
  if (pFileBuffer==0) {
    const char *name = fl_fonts[pFontIndex].name;

    // first attempt, try to read a font from wherever the user wishes
    bool ret = load_font(name);

    // if that did not work, read the old style Android fonts
    if (!ret && pFontIndex<16)
      ret = load_font(old_font_names[pFontIndex/4]);

    // if that still didn't work, see if we have the default font asset
    if (!ret)
      ret = load_font("@fonts/Roboto-Regular.ttf");

    // still no luck? Well, I guess we can't render anything in this font.
    if (!ret) {
      Fl_Android_Application::log_e("Giving up. Can't load font '%s'", name);
      pError = true;
      return;
    }
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
  if (pError) return nullptr;

  Fl_Android_Bytemap *bm = new Fl_Android_Bytemap();

  float hgt = stbtt_ScaleForPixelHeight(&pFont, size);
  bm->pBytes = stbtt_GetCodepointBitmap(&pFont, 0, hgt, c,
                                             &bm->pWidth, &bm->pHeight,
                                             &bm->pXOffset, &bm->pYOffset);
  bm->pStride = bm->pWidth;

  int advance, lsb;
  stbtt_GetCodepointHMetrics(&pFont, c, &advance, &lsb);
  float scale = stbtt_ScaleForPixelHeight(&pFont, size);
  bm->pAdvance = (int)((scale * advance)+0.5f);

  return bm;
}

/**
 * Get the width of the character in pixels.
 * This is not a good function because character advance also depends on kerning
 * which takes the next character in a text line into account. Also, FLTK is
 * limited to integer character positions, and so is the Android driver.
 * @param c unicode character
 * @param size height in pixels
 * @return width in pixels to the start of the next character
 */
float Fl_Android_Font_Source::get_advance(uint32_t c, Fl_Fontsize size)
{
  int advance, lsb;

  if (pFileBuffer==0) load_font();
  if (pError) return 0.0f;

  stbtt_GetCodepointHMetrics(&pFont, c, &advance, &lsb);
  float scale = stbtt_ScaleForPixelHeight(&pFont, size);
  return scale * advance;
}


int Fl_Android_Font_Source::get_descent(Fl_Fontsize size)
{
  if (pFileBuffer==0) load_font();
  if (pError) return 0.0f;

  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(&pFont, &ascent, &descent, &lineGap);
  float scale = stbtt_ScaleForPixelHeight(&pFont, size);

  return -(descent*scale-0.5f);
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
  descent = -1;
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

/**
 * Release resources, including all cached unicode character shapes.
 */
Fl_Android_Font_Descriptor::~Fl_Android_Font_Descriptor()
{
  // Life is easy in C++11.
  for (auto &i: pBytemapTable) {
    delete i.second; i.second = nullptr;
  }
}

/*
 * Get the width of the character in pixels.
 * @param c unicode character
 * @return width in pixels to the start of the next character
 */
float Fl_Android_Font_Descriptor::get_advance(uint32_t c)
{
  // TODO: should we use the cached value in the Bytemap?
  // Yes, we should, because if FLTK requests the width of a character, it is
  // more than likely to render that character soon after.
  return pFontSource->get_advance(c, size);
}

/**
 * Get the pixels for a given Unicode character.
 *
 * Calculating a bitmap is relatively expensive. This class will cache every
 * bitmap ever generated. Currently, this is pretty much brute force because
 * none of the bitmaps are ever released.
 *
 * @param c unicode character
 * @return a bytemap
 */
Fl_Android_Bytemap *Fl_Android_Font_Descriptor::get_bytemap(uint32_t c)
{
  Fl_Android_Bytemap *bm = 0;
  auto it = pBytemapTable.find(c);
  if (it==pBytemapTable.end()) {
    bm = pFontSource->get_bytemap(c, size);
    if (bm)
      pBytemapTable[c] = bm;
  } else {
    bm = it->second;
  }
  return bm;
}


int Fl_Android_Font_Descriptor::get_descent()
{
  if (descent==-1)
    descent = (short)pFontSource->get_descent(size);
  return descent;
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


void Fl_Android_Graphics_Driver::render_bytemap(int xx, int yy, Fl_Android_Bytemap *bm, Fl_Rect_Region &r)
{
  xx += bm->pXOffset; yy += bm->pYOffset;

  if (xx>r.right()) return;
  if (yy>r.bottom()) return;
  if (xx+bm->pWidth < r.left()) return;
  if (yy+bm->pHeight < r.top()) return;

  uint16_t cc = make565(fl_color()), cc12 = (cc&0xf7de)>>1, cc14 = (cc12&0xf7de)>>1, cc34 = cc12+cc14;
  int32_t ss = pStride;
  uint16_t *bits = pBits;
  uint32_t ww = bm->pWidth;
  uint32_t hh = bm->pHeight;
  unsigned char *srcBytes = bm->pBytes;

  int dx = r.left()-xx;
  int dy = r.top()-yy;
  int dr = (xx+ww)-r.right();
  int db = (yy+hh)-r.bottom();
  if (dx>0) { xx+=dx; ww-=dx; srcBytes+=dx; }
  if (dy>0) { yy+=dy; hh-=dy; srcBytes+=dy*bm->pStride; }
  if (dr>0) { ww-=dr; }
  if (db>0) { hh-=db; }

  for (uint32_t iy = 0; iy<hh; ++iy) {
    uint16_t *d = bits + (yy+iy)*ss + xx;
    unsigned char *s = srcBytes + iy*bm->pStride;
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

}


/**
 * Copy a single letter to the screen.
 * @param xx, yy position of character on screen
 * @param c unicode character
 * @return x position of next character on screen
 */
int Fl_Android_Graphics_Driver::render_letter(int xx, int yy, uint32_t c, Fl_Rect_Region &r)
{
  int oxx = xx;

  // find the font descriptor
  Fl_Android_Font_Descriptor *fd = (Fl_Android_Font_Descriptor*)font_descriptor();
  if (!fd) return xx; // this should not happen

  Fl_Android_Bytemap *bm = fd->get_bytemap(c);
  if (!bm) return oxx;

  render_bytemap(xx, yy, bm, r);

  return oxx + bm->pAdvance;
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
    int dx, dy, w, h;
    text_extents_unscaled(str, n, dx, dy, w, h);
    pClippingRegion.print("<---- clip text to this");
    Fl_Rect_Region(x+dx, y+dy, w, h).print(str);
    for (const auto &it: pClippingRegion.overlapping(Fl_Rect_Region(x+dx, y+dy, w, h))) {
      Fl_Rect_Region &r = it->clipped_rect();
      r.print("Clip");
      const char *e = str + n;
      for (int i = 0; i < n;) {
        int incr = 1;
        unsigned uniChar = fl_utf8decode(str + i, e, &incr);
        int x1 = x;
        x = render_letter(x, y, uniChar, r);
#if 0
        // use this to make the character baseline visible
        Fl_Color old = fl_color();
        fl_color(FL_RED);
        fl_xyline(x1, y, x);
        fl_yxline(x1, y-5, y+5);
        fl_color(old);
#endif
        i += incr;
      }
    }
  }
}


double Fl_Android_Graphics_Driver::width_unscaled(const char *str, int n)
{
  Fl_Android_Font_Descriptor *fd = (Fl_Android_Font_Descriptor*)font_descriptor();
  if (!fd) return 0;

  int width = 0;
  const char *e = str+n;
  for (int i=0; i<n; ) {
    int incr = 1;
    unsigned uniChar = fl_utf8decode(str + i, e, &incr);
    width += ((int)(fd->get_advance(uniChar)+0.5f));
    i += incr;
  }
  return width;
}


double Fl_Android_Graphics_Driver::width_unscaled(unsigned int uniChar)
{
  Fl_Android_Font_Descriptor *fd = (Fl_Android_Font_Descriptor*)font_descriptor();
  if (!fd) return 0;
  return ((int)(fd->get_advance(uniChar)+0.5f));
}


Fl_Fontsize Fl_Android_Graphics_Driver::size_unscaled()
{
  Fl_Android_Font_Descriptor *fd = (Fl_Android_Font_Descriptor*)font_descriptor();
  if (!fd) return 0;
  return fd->size;
}


void Fl_Android_Graphics_Driver::text_extents_unscaled(const char *str, int n, int &dx, int &dy, int &w, int &h)
{
  w = width_unscaled(str, n);
  h = height_unscaled();
  dx = 0;
  dy = descent_unscaled() - h;
}


int Fl_Android_Graphics_Driver::height_unscaled()
{
  // This should really be "ascent - descent + lineGap"
  Fl_Android_Font_Descriptor *fd = (Fl_Android_Font_Descriptor*)font_descriptor();
  if (!fd) return 0;
  return fd->size;
}


int Fl_Android_Graphics_Driver::descent_unscaled()
{
  Fl_Android_Font_Descriptor *fd = (Fl_Android_Font_Descriptor*)font_descriptor();
  if (!fd) return 0;
  if (fd->descent==-1) fd->get_descent();
  return fd->descent;
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
