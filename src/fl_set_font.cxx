//
// Font utilities for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

// Add a font to the internal table.
// Also see fl_set_fonts.cxx which adds all possible fonts.

#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/fl_draw.H>
#include "Fl_Screen_Driver.H"
#include "flstring.h"
#include <stdlib.h>

struct Fl_Fontdesc;
extern FL_EXPORT Fl_Fontdesc *fl_fonts; // the table

static int table_size;
/**
  Changes a face.
 \param fnum The font number to be assigned a new face
 \param name Name of the font to assign. The string pointer is simply stored,
 the string is not copied, so the string must be in static memory. The exact name to be used
 depends on the platform :

 \li Windows, X11, Xft: use the family name prefixed by one character to indicate the desired font variant.
 Characters <tt>' ', 'I', 'B', 'P' </tt>denote plain, italic, bold, and bold-italic variants, respectively. For example,
 string \c "IGabriola" is to be used to denote the <tt>"Gabriola italic"</tt> font. The \c "Oblique" suffix,
 in whatever case, is to be treated  as \c "italic", that is, prefix the family name with \c 'I'.
 \li Other platforms, i.e., X11 + Pango, Wayland, macOS: use the full font name as returned by
 function Fl::get_font_name() or as listed by applications test/fonts or test/utf8. No prefix is to be added.
*/
void Fl::set_font(Fl_Font fnum, const char* name) {
  Fl_Graphics_Driver &d = Fl_Graphics_Driver::default_driver();
  unsigned width = d.font_desc_size();
  if (!fl_fonts) fl_fonts = d.calc_fl_fonts();
  while (fnum >= table_size) {
    int i = table_size;
    if (!i) {   // don't realloc the built-in table
      table_size = 2*FL_FREE_FONT;
      i = FL_FREE_FONT;
      Fl_Fontdesc* t = (Fl_Fontdesc*)malloc(table_size*width);
      memcpy(t, fl_fonts, FL_FREE_FONT*width);
      fl_fonts = t;
    } else {
      table_size = 2*table_size;
      fl_fonts=(Fl_Fontdesc*)realloc(fl_fonts, table_size*width);
    }
    for (; i < table_size; i++) {
      memset((char*)fl_fonts + i * width, 0, width);
    }
  }
  d.font_name(fnum, name);
  d.font(-1, 0);
}

/** Copies one face to another. */
void Fl::set_font(Fl_Font fnum, Fl_Font from) {
  Fl::set_font(fnum, get_font(from));
}

/**
  Loads a TrueType or OpenType font file and assigns it to a font number.

  The font is available to this application only and remains loaded until the
  application exits. The style of the font (regular, bold, italic, or bold italic)
  is read from the font file. To use a font family with the FLTK font attributes
  FL_BOLD and FL_ITALIC, load each style into the matching font number:
  \code
    Fl::load_font(FL_FREE_FONT,                  "MyFont-Regular.ttf");
    Fl::load_font(FL_FREE_FONT + FL_BOLD,        "MyFont-Bold.ttf");
    Fl::load_font(FL_FREE_FONT + FL_ITALIC,      "MyFont-Italic.ttf");
    Fl::load_font(FL_FREE_FONT + FL_BOLD_ITALIC, "MyFont-BoldItalic.ttf");
  \endcode
  If the file is a font collection (.ttc), its first face is assigned to \p fnum.

  This is not supported by X11 builds that use neither Xft nor Pango, and
  loading fonts from memory requires macOS 10.8 or later.

  \param fnum The font number to be assigned the new face
  \param filename The UTF-8 encoded path of the font file
  \return \p fnum, or -1 if the font could not be loaded
  \see Fl::load_font(Fl_Font, const unsigned char *, size_t)
  \since 1.5.0
*/
Fl_Font Fl::load_font(Fl_Font fnum, const char *filename) {
  FILE *f = filename ? fl_fopen(filename, "rb") : NULL;
  if (!f) return -1;
  unsigned char *data = NULL;
  long size = -1;
  if (fseek(f, 0, SEEK_END) == 0) size = ftell(f);
  if (size > 0 && fseek(f, 0, SEEK_SET) == 0) {
    data = (unsigned char *)malloc(size);
    if (data && fread(data, 1, size, f) != (size_t)size) { free(data); data = NULL; }
  }
  fclose(f);
  if (!data) return -1;
  const char *name = Fl_Graphics_Driver::default_driver().load_font(filename, data, size);
  free(data);
  if (!name) return -1;
  Fl::set_font(fnum, name);
  return fnum;
}

/**
  Loads a TrueType or OpenType font from memory and assigns it to a font number.

  Use this function to load a font that is compiled into the application, for
  instance with the Data node in FLUID. The data is copied, so the buffer can be
  released when this function returns.

  \param fnum The font number to be assigned the new face
  \param data The content of a TrueType or OpenType font file
  \param size The size of \p data in bytes
  \return \p fnum, or -1 if the font could not be loaded
  \see Fl::load_font(Fl_Font, const char *) for details
  \since 1.5.0
*/
Fl_Font Fl::load_font(Fl_Font fnum, const unsigned char *data, size_t size) {
  const char *name = Fl_Graphics_Driver::default_driver().load_font(NULL, data, size);
  if (!name) return -1;
  Fl::set_font(fnum, name);
  return fnum;
}

/**
    Gets the string for this face.  This string is different for each
    face. Under X this value is passed to XListFonts to get all the sizes
    of this face.
*/
const char* Fl::get_font(Fl_Font fnum) {
  return Fl_Graphics_Driver::default_driver().font_name(fnum);
}
