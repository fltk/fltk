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


Fl_Android_Font_Descriptor::Fl_Android_Font_Descriptor(const char* fontname, Fl_Font fnum, Fl_Fontsize size) :
        Fl_Font_Descriptor(fontname, size),
        pFontIndex(fnum),
        pFileBuffer(0)
{
  // --- This is what we have to live with:
  //  Fl_Font_Descriptor *next;
  //  Fl_Fontsize size; /**< font size */
  //  Fl_Font_Descriptor(const char* fontname, Fl_Fontsize size);
  //          FL_EXPORT ~Fl_Font_Descriptor() {}
  //  short ascent, descent, q_width;
  //  unsigned int listbase; // base of display list, 0 = none


}


unsigned char *Fl_Android_Font_Descriptor::get_bitmap(uint32_t c, int *w, int *h, int *dx, int *dy)
{
  unsigned char *bitmap;

  if (pFileBuffer==0) {
    char buf[1024];
    sprintf(buf, "/system/fonts/%s.ttf", fl_fonts[pFontIndex].name);
//    FILE *f = fopen("/system/fonts/DroidSans.ttf", "rb");
    FILE *f = fopen(buf, "rb");
    if (f==NULL) {
      Fl_Android_Application::log_e("ERROR reading font %d!", errno);
      return 0;
    }
    fseek(f, 0, SEEK_END);
    size_t fsize = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    pFileBuffer = (uint8_t*)malloc(fsize);
    fread(pFileBuffer, 1, fsize, f);
    fclose(f);

    stbtt_InitFont(&pFont, pFileBuffer, stbtt_GetFontOffsetForIndex(pFileBuffer,0));
  }

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
  int ww, hh, ddx, ddy;
  float hgt = stbtt_ScaleForPixelHeight(&pFont, size);
  bitmap = stbtt_GetCodepointBitmap(&pFont, 0, hgt, c, w, h, dx, dy);
  return bitmap;

}


void Fl_Android_Font_Descriptor::free_bitmap(uint8_t *bitmap)
{
  stbtt_FreeBitmap(bitmap, 0L);
}


float Fl_Android_Font_Descriptor::get_advance(uint32_t c)
{
  int advance, lsb;
  stbtt_GetCodepointHMetrics(&pFont, c, &advance, &lsb);
  float scale = stbtt_ScaleForPixelHeight(&pFont, size);

  return scale * advance;
}


static Fl_Android_Font_Descriptor* find(Fl_Font fnum, Fl_Fontsize size)
{
  Fl_Fontdesc *s = fl_fonts + fnum;
  if (!s->name) s = fl_fonts; // use 0 if fnum undefined

  Fl_Android_Font_Descriptor *f;
  for (f = (Fl_Android_Font_Descriptor *) s->first; f; f = (Fl_Android_Font_Descriptor *) f->next) {
    if (f->size == size) return f;
  }

  f = new Fl_Android_Font_Descriptor(s->name, fnum, size);
  f->next = s->first;
  s->first = f;
  return f;
}


// =============================================================================


void Fl_Android_Graphics_Driver::font_unscaled(Fl_Font fnum, Fl_Fontsize size) {
  Fl_Android_Application::log_e("FLTK is requesting font %d at %d pixels (%08x)", fnum, size, fl_fonts);
  font_descriptor( find(fnum, size) );
  size_ = size;
  font_ = fnum;
}


// -- fun with text rendering

int Fl_Android_Graphics_Driver::render_letter(int xx, int yy, uint32_t c)
{
  unsigned char *bitmap;
  int w, h, size = 30;
  int dx, dy;

  // find the font descriptor
  Fl_Android_Font_Descriptor *fd = (Fl_Android_Font_Descriptor*)font_descriptor();
  if (!fd) return xx; // this should not happen

  bitmap = fd->get_bitmap(c, &w, &h, &dx, &dy);

  // rrrr.rggg.gggb.bbbb
  xx += dx; yy += dy;
  uint16_t cc = make565(fl_color()), cc12 = (cc&0xf7de)>>1, cc14 = (cc12&0xf7de)>>1, cc34 = cc12+cc14;
  int32_t ss = pStride;
  uint16_t *bits = pBits;
  uint32_t ww = w;
  uint32_t hh = h;
  unsigned char *s = bitmap;
  for (uint32_t iy = 0; iy<hh; ++iy) {
    uint16_t *d = bits + (yy+iy)*ss + xx;
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
  fd->free_bitmap(bitmap);
  return xx+fd->get_advance(c);
}

void Fl_Android_Graphics_Driver::draw_unscaled(const char* str, int n, int x, int y)
{
  if (!str) return;

  if (str) {
    x = x+16*(-n/2);
    for (int i=0; i<n; i++)
      x = render_letter(x, y+5, str[i]);
  }
}


#if 0

//"DroidSans.ttf", "DroidSans-Bold.ttf" -> links to "Roboto-Regular.ttf""
//
//"CutiveMono.ttf"
//"DroidSansMono.ttf"
//
//"Roboto-*.ttf", Regular Bold Italic BoldItalic
//
//"NotoSerif-*.ttf", Regular Bold Italic BoldItalic
//
//"NotoSansSymbols-Regular-Subsetted.ttf"
//"NotoSansSymbols-Regular-Subsetted2.ttf"
//
//"NotoColorEmoji.ttf"

#include "../../config_lib.h"
#include "Fl_Android_Application.H"
#include "Fl_Android_Graphics_Driver.H"
#include "Fl_Android_Screen_Driver.H"
#include <FL/Fl.H>
#include <FL/platform.H>
#include <errno.h>


#endif

//
// End of "$Id$".
//
