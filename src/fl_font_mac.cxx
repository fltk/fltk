//
// "$Id: fl_font_mac.cxx,v 1.1.2.9 2002/06/27 04:29:39 matthiaswm Exp $"
//
// MacOS font selection routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2002 by Bill Spitzak and others.
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

//: MeasureText, FontMetrics, WidthTabHandle, GetSysFont, SysFontSize
//: TextSize, TextFont
//: GetFNum (theName: Str255; VAR familyID: Integer);
//: FUNCTION FMSwapFont (inRec: FMInput): FMOutPtr;

Fl_FontSize::Fl_FontSize(const char* name, int Size) {
  knowMetrics = 0;
  switch (*name++) {
  case 'I': face = italic; break;
  case 'P': face = italic | bold; break;
  case 'B': face = bold; break;
  default: face = 0; break;
  }
  unsigned char fn[80]; 
  fn[0] = strlen(name); strcpy((char*)(fn+1), name);
  GetFNum(fn, &font);
  size = Size;
  FMInput fIn = { font, size, face, 0, 0, { 1, 1}, { 1, 1} };
  FMOutput *fOut = FMSwapFont(&fIn);
  ascent = fOut->ascent;	//: the following three lines give only temporary aproimations
  descent = fOut->descent;
  for (int i=0; i<256; i++) width[i] = fOut->widMax;
#if HAVE_GL
  listbase = 0;
#endif
  minsize = maxsize = size;
}

Fl_FontSize* fl_fontsize = 0L;

Fl_FontSize::~Fl_FontSize() {
/*
#if HAVE_GL
 // ++ todo: remove OpenGL font alocations
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
  if (this == fl_fontsize) fl_fontsize = 0;
}

////////////////////////////////////////////////////////////////

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
{" Chicago"},
{"BChicago"},
{" Webdings"},
};

Fl_Fontdesc* fl_fonts = built_in_table;

void fl_font(Fl_FontSize* s) {
  fl_fontsize = s;
  if (fl_window) SetPort( GetWindowPort(fl_window) );
  TextFont(fl_fontsize->font);	//: select font into current QuickDraw GC
  TextFace(fl_fontsize->face);
  TextSize(fl_fontsize->size);
  if (!fl_fontsize->knowMetrics) {	//: get the true metrics for the currnet GC (fails on multiple monitors with different dpi's!)
    FontInfo fi; GetFontInfo(&fi);
    fl_fontsize->ascent = fi.ascent;
    fl_fontsize->descent = fi.descent;
    FMetricRec mr; FontMetrics(&mr);
    short *f = (short*)*mr.wTabHandle; //: get the char size table
    for (int i=0; i<256; i++) fl_fontsize->width[i] = f[2*i];
    fl_fontsize->knowMetrics = 1;
  }
}

static Fl_FontSize* find(int fnum, int size) {
  Fl_Fontdesc* s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // use 0 if fnum undefined
  Fl_FontSize* f;
  for (f = s->first; f; f = f->next)
    if (f->minsize <= size && f->maxsize >= size) return f;
  f = new Fl_FontSize(s->name, size);
  f->next = s->first;
  s->first = f;
  return f;
}

////////////////////////////////////////////////////////////////
// Public interface:

int fl_font_ = 0;
int fl_size_ = 0;

void fl_font(int fnum, int size) {
  if (fnum == fl_font_ && size == fl_size_) return;
  fl_font_ = fnum;
  fl_size_ = size;
  fl_font(find(fnum, size));
}

int fl_height() {
  return fl_fontsize->ascent+fl_fontsize->descent;
}

int fl_descent() {
  return fl_fontsize->descent;
}

double fl_width(const char* c, int n) {
  return (double)TextWidth( c, 0, n );
}

// todo : fl_width returns wrong results for OS X
double fl_width(uchar c) {
  return (double)TextWidth( &c, 0, 1 );
}

void fl_draw(const char* str, int n, int x, int y) {
  MoveTo(x, y);
  DrawText(str, 0, n);
}

//
// End of "$Id: fl_font_mac.cxx,v 1.1.2.9 2002/06/27 04:29:39 matthiaswm Exp $".
//
