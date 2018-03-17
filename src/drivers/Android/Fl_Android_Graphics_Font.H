//
// "$Id$"
//
// Font definitions for the Fast Light Tool Kit (FLTK).
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

#ifndef FL_ANDROID_GRAPHICS_FONT_H
#define FL_ANDROID_GRAPHICS_FONT_H


#include "Fl_Android_Graphics_Driver.H"

// We violate FLTKs avoidance of STL because we live in a defined driver space
#include <map>

#include "stb_truetype.h"


/**
 * A bytemap is an array of bytes, used as an alpha channel when redering glyphs
 * in a given color.
 */
class Fl_Android_Bytemap
{
public:
  Fl_Android_Bytemap();
  ~Fl_Android_Bytemap();

public:
  int pWidth, pHeight, pStride, pXOffset, pYOffset, pAdvance;
  unsigned char *pBytes;
};


/**
 * This class reads True Type Font files and creates Bytemaps for glyphs at the
 * requested height.
 */
class Fl_Android_Font_Source
{
private:
  stbtt_fontinfo pFont;
  uint8_t *pFileBuffer;
  const char *pName;
  Fl_Font pFontIndex;
  bool pError;

  bool load_font(const char *name);
  bool load_font_file(const char *name);
  bool load_font_asset(const char *name);

public:
  Fl_Android_Font_Source(const char *fname, Fl_Font fnum);
  ~Fl_Android_Font_Source();
  void load_font();
  Fl_Android_Bytemap *get_bytemap(uint32_t c, int size);
  float get_advance(uint32_t c, Fl_Fontsize size);
};


/**
 * This class caches glyphs of a font for a specified height.
 */
class Fl_Android_Font_Descriptor : public Fl_Font_Descriptor
{
private:
  Fl_Android_Font_Source *pFontSource;
  Fl_Font pFontIndex;
  std::map<uint32_t, Fl_Android_Bytemap*> pBytemapTable;

public:
  Fl_Android_Font_Descriptor(const char *fname, Fl_Android_Font_Source *fsrc, Fl_Font fnum, Fl_Fontsize size);
  ~Fl_Android_Font_Descriptor();
  float get_advance(uint32_t c);
  Fl_Android_Bytemap *get_bytemap(uint32_t c);
  Fl_Android_Font_Source *get_font_source() { return pFontSource; }

  static Fl_Android_Font_Descriptor* find(Fl_Font fnum, Fl_Fontsize size);
};


#endif // FL_ANDROID_GRAPHICS_FONT_H

//
// End of "$Id$".
//
