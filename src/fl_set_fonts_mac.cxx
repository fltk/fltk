//
// "$Id: fl_set_fonts_mac.cxx,v 1.1.2.8 2003/01/30 21:44:16 easysw Exp $"
//
// MacOS font utilities for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2003 by Bill Spitzak and others.
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

// This function fills in the fltk font table with all the fonts that
// are found on the X server.  It tries to place the fonts into families
// and to sort them so the first 4 in a family are normal, bold, italic,
// and bold italic.

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
    if (ap) *ap = type;
    if (!type) return p+1;
    strlcpy(f->fontname, p+1, sizeof(f->fontname));
    if (type & FL_BOLD) strlcat(f->fontname, " bold", sizeof(f->fontname));
    if (type & FL_ITALIC) strlcat(f->fontname, " italic", sizeof(f->fontname));
  }
  return f->fontname;
}

static int fl_free_font = FL_FREE_FONT;

Fl_Font Fl::set_fonts(const char* xstarname) {
#pragma unused ( xstarname )
  if (fl_free_font != FL_FREE_FONT) 
    return (Fl_Font)fl_free_font;
  static char styleLU[] = " BIP";
  FMFontFamilyInstanceIterator ffiIterator;
  FMFontFamilyIterator ffIterator;
  FMFontFamily family;
  FMFont font;
  FMFontStyle style; // bits 0..6: bold, italic underline, outline, shadow, condens, extended (FLTK supports 0 and 1 )
  FMFontSize size;
  //FMFilter filter; // do we need to set a specific (or multiple) filter(s) to get ALL fonts?
  
  Str255 buf;
  //filter.format = kFMCurrentFilterFormat;
  //filter.selector = kFMGenerationFilterSelector;
  //filter.filter.generationFilter = 
  FMCreateFontFamilyIterator( NULL, NULL, kFMUseGlobalScopeOption, &ffIterator );
  OSStatus listFamilies, listInstances;
  for (;;)
  {
    listFamilies = FMGetNextFontFamily( &ffIterator, &family );
    if ( listFamilies != 0 ) break;
    FMGetFontFamilyName( family, buf );
    buf[ buf[0]+1 ] = 0;
    //printf( "Font Family: %s\n", buf+1 );
    int i;
    for (i=0; i<FL_FREE_FONT; i++) // skip if one of our built-in fonts
      if (!strcmp(Fl::get_font_name((Fl_Font)i),(char*)buf+1)) break;
    if ( i < FL_FREE_FONT ) continue;
    FMCreateFontFamilyInstanceIterator( family, &ffiIterator );
    char pStyle = 0, nStyle;
    for (;;)
    {
      listInstances = FMGetNextFontFamilyInstance( &ffiIterator, &font, &style, &size );
      if ( listInstances != 0 ) break;
      // printf(" %d %d %d\n", font, style, size );
      nStyle = styleLU[style&0x03];
      if ( ( pStyle & ( 1<<(style&0x03) ) ) == 0 )
      {
        buf[0] = nStyle;
        Fl::set_font((Fl_Font)(fl_free_font++), strdup((char*)buf));
        pStyle |= ( 1<<(style&0x03) );
      }
    }
    FMDisposeFontFamilyInstanceIterator( &ffiIterator );
  }
  FMDisposeFontFamilyIterator( &ffIterator );
  return (Fl_Font)fl_free_font;
}

static int array[128];
int Fl::get_font_sizes(Fl_Font fnum, int*& sizep) {
  Fl_Fontdesc *s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // empty slot in table, use entry 0

  Str255 name;
  int len = strlen( s->name );
  memcpy(((char*)name)+1, s->name+1, len );
  name[0] = len-1;
  FMFontFamily family = FMGetFontFamilyFromName( name );
  if ( family == kInvalidFontFamily ) return 0;

  sizep = array;
  FMFont font;
  FMFontStyle style, fStyle;
  switch ( s->name[0] ) {
    default :
      fStyle=0;
      break;
    case 'B' : 
      fStyle=1;
      break;
    case 'I' : 
      fStyle=2;
      break;
    case 'P' :
      fStyle=3;
      break;
  }
  FMFontSize size, pSize = -1;
  FMFontFamilyInstanceIterator ffiIterator;
  FMCreateFontFamilyInstanceIterator( family, &ffiIterator );
  int cnt = 0;
  OSStatus listInstances;
  for (;;)
  {
    listInstances = FMGetNextFontFamilyInstance( &ffiIterator, &font, &style, &size );
    if ( listInstances != 0 ) break;
    if ( style==fStyle )
    {
      if ( size>pSize ) 
      {
        array[ cnt++ ] = size;
        pSize = size;
      }
    }
  }
  FMDisposeFontFamilyInstanceIterator( &ffiIterator );

  return cnt;
}

//
// End of "$Id: fl_set_fonts_mac.cxx,v 1.1.2.8 2003/01/30 21:44:16 easysw Exp $".
//
