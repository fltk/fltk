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


Fl_Android_Bytemap::Fl_Android_Bytemap() :
        pBytes(0L)
{
}


Fl_Android_Bytemap::~Fl_Android_Bytemap()
{
  if (pBytes) ::free(pBytes);
}


// -----------------------------------------------------------------------------


Fl_Android_Font_Source::Fl_Android_Font_Source(const char *fname, Fl_Font fnum) :
        pFileBuffer(0L),
        pName(fname),
        pFontIndex(fnum)
{
}


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


float Fl_Android_Font_Source::get_advance(uint32_t c, Fl_Fontsize size)
{
  int advance, lsb;

  if (pFileBuffer==0) load_font();
  stbtt_GetCodepointHMetrics(&pFont, c, &advance, &lsb);
  float scale = stbtt_ScaleForPixelHeight(&pFont, size);
  return scale * advance;
}


// -----------------------------------------------------------------------------


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


float Fl_Android_Font_Descriptor::get_advance(uint32_t c)
{
  return pFontSource->get_advance(c, size);
}


Fl_Android_Bytemap *Fl_Android_Font_Descriptor::get_bytemap(uint32_t c)
{
  // TODO: cache bytemaps here for fast access
  return pFontSource->get_bytemap(c, size);
}


static Fl_Android_Font_Descriptor* find(Fl_Font fnum, Fl_Fontsize size)
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


void Fl_Android_Graphics_Driver::font_unscaled(Fl_Font fnum, Fl_Fontsize size) {
  Fl_Android_Application::log_e("FLTK is requesting font %d at %d pixels (%08x)", fnum, size, fl_fonts);
  font_descriptor( find(fnum, size) );
  size_ = size;
  font_ = fnum;
}


int Fl_Android_Graphics_Driver::render_letter(int xx, int yy, uint32_t c)
{
//  unsigned char *bitmap;
  int oxx = xx;
//  int w, h, size = 30;
//  int dx, dy;

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
        uint16_t nn = *d, nn14 = (nn&0xe79c)>>2;
        *d = nn14 + cc34;
      } else if (v<100) { // 25%
        uint16_t nn = *d, nn12 = (nn&0xf7de)>>1, nn14 = (nn12&0xf7de)>>1, nn34 = nn12+nn14;
        *d = nn34 + cc14;
      } else { // 50%
        uint16_t nn = *d, nn12 = (nn&0xf7de)>>1;
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
  return oxx + fd->get_advance(c);
}


void Fl_Android_Graphics_Driver::draw_unscaled(const char* str, int n, int x, int y)
{
  if (str) {
    x = x+16*(-n/2);
    for (int i=0; i<n; i++)
      x = render_letter(x, y+5, str[i]);
  }
}


//
// End of "$Id$".
//
