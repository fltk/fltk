//
// "$Id$"
//
// X11 font utilities for the Fast Light Tool Kit (FLTK).
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

// Select fonts from the FLTK font table.
#include "../../flstring.h"
#include "Fl_Xlib_Graphics_Driver.H"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include "Fl_Font.H"

#include <stdio.h>
#include <stdlib.h>

static char* fl_find_fontsize(char* name);
static const char* fl_font_word(const char* p, int n);

// This function fills in the fltk font table with all the fonts that
// are found on the X server.  It tries to place the fonts into families
// and to sort them so the first 4 in a family are normal, bold, italic,
// and bold italic.

// Standard X fonts are matched by a pattern that is always of
// this form, and this pattern is put in the table:
// "-*-family-weight-slant-width1-style-*-registry-encoding"

// Non-standard font names (those not starting with '-') are matched
// by a pattern of the form "prefix*suffix", where the '*' is where
// fltk thinks the point size is, or by the actual font name if no
// point size is found.

// Fltk knows how to pull an "attribute" out of a font name, such as
// bold or italic, by matching known x font field values.  All words
// that don't match a known attribute are combined into the "name"
// of the font.  Names are compared before attributes for sorting, this
// makes the bold and plain version of a font come out next to each
// other despite the poor X font naming scheme.

// By default fl_set_fonts() only does iso8859-1 encoded fonts.  You can
// do all normal X fonts by passing "-*" or every possible font with "*".

// Fl::set_font will take strings other than the ones this stores
// and can identify any font on X that way.  You may want to write your
// own system of font management and not use this code.

// turn word N of a X font name into either some attribute bits
// (right now 0, FL_BOLD, or FL_ITALIC), or into -1 indicating that
// the word should be put into the name:

static int attribute(int n, const char *p) {
  // don't put blank things into name:
  if (!*p || *p=='-' || *p=='*') return 0;
  if (n == 3) { // weight
    if (!strncmp(p,"normal",6) ||
	!strncmp(p,"light",5) ||
	!strncmp(p,"medium",6) ||
	!strncmp(p,"book",4)) return 0;
    if (!strncmp(p,"bold",4) || !strncmp(p,"demi",4)) return FL_BOLD;
  } else if (n == 4) { // slant
    if (*p == 'r') return 0;
    if (*p == 'i' || *p == 'o') return FL_ITALIC;
  } else if (n == 5) { // sWidth
    if (!strncmp(p,"normal",6)) return 0;
  }
  return -1;
}

// return non-zero if the registry-encoding should be used:
extern const char* fl_encoding;
static int use_registry(const char *p) {
  return *p && *p!='*' && strcmp(p,fl_encoding);
}

// Bug: older versions calculated the value for *ap as a side effect of
// making the name, and then forgot about it. To avoid having to change
// the header files I decided to store this value in the last character
// of the font name array.
#define ENDOFBUFFER 127 // sizeof(Fl_Font.fontname)-1

// turn a stored (with *'s) X font name into a pretty name:
const char* Fl_Xlib_Graphics_Driver::get_font_name(Fl_Font fnum, int* ap) {
  Fl_Fontdesc *f = fl_fonts + fnum;
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

extern "C" {
// sort raw (non-'*') X font names into perfect order:

static int ultrasort(const void *aa, const void *bb) {
  const char *a = *(char **)aa;
  const char *b = *(char **)bb;

  // sort all non x-fonts at the end:
  if (*a != '-') {
    if (*b == '-') return 1;
    // 2 non-x fonts are matched by "numeric sort"
    int ret = 0;
    for (;;) {
      if (isdigit(*a) && isdigit(*b)) {
	int na = strtol(a, (char **)&a, 10);
	int nb = strtol(b, (char **)&b, 10);
	if (!ret) ret = na-nb;
      } else if (*a != *b) {
	return (*a-*b);
      } else if (!*a) {
	return ret;
      } else {
	a++; b++;
      }
    }
  } else {
    if (*b != '-') return -1;
  }

  // skip the foundry (assume equal):
  for (a++; *a && *a++!='-';);
  for (b++; *b && *b++!='-';);

  // compare the family and all the attribute words:
  int atype = 0;
  int btype = 0;
  for (int n = 2; n <= 6; n++) {
    int at = attribute(n,a);
    int bt = attribute(n,b);
    if (at < 0) {
      if (bt >= 0) return 1;
      for (;;) {if (*a!=*b) return *a-*b; b++; if (!*a || *a++=='-') break;}
    } else {
      if (bt < 0) return -1;
      a = fl_font_word(a,1); if (*a) a++;
      b = fl_font_word(b,1); if (*b) b++;
      atype |= at; btype |= bt;
    }
  }

  // remember the pixel size:
  int asize = atoi(a);
  int bsize = atoi(b);

  // compare the registry/encoding:
  a = fl_font_word(a,6); if (*a) a++;
  b = fl_font_word(b,6); if (*b) b++;
  if (use_registry(a)) {
    if (!use_registry(b)) return 1;
    int r = strcmp(a,b); if (r) return r;
  } else {
    if (use_registry(b)) return -1;
  }

  if (atype != btype) return atype-btype;
  if (asize != bsize) return asize-bsize;

  // something wrong, just do a string compare...
  return strcmp(*(char**)aa, *(char**)bb);
}
}

// converts a X font name to a standard starname, returns point size:
static int to_canonical(char *to, const char *from, size_t tolen) {
  char* c = fl_find_fontsize((char*)from);
  if (!c) return -1; // no point size found...
  const char* endptr;
  int size = strtol(c,(char**)&endptr,10);
  if (from[0] == '-') {
    // replace the "foundry" with -*-:
    *to++ = '-'; *to++ = '*';
    for (from++; *from && *from != '-'; from++);
    // skip to the registry-encoding:
    endptr = (char*)fl_font_word(endptr,6);
    if (*endptr && !use_registry(endptr+1)) endptr = "";
  }
  int n = c-from;
  // MRS: we want strncpy here, not strlcpy...
  if (n > (int)(tolen - 1)) return -1;
  strncpy(to,from,n);
  to[n++] = '*';
  strlcpy(to+n,endptr, tolen - n);
  return size;
}

static unsigned int fl_free_font = FL_FREE_FONT;


// This function fills in the fltk font table with all the fonts that
// are found on the X server.  It tries to place the fonts into families
// and to sort them so the first 4 in a family are normal, bold, italic,
// and bold italic.
Fl_Font Fl_Xlib_Graphics_Driver::set_fonts(const char* xstarname) {
  if (fl_free_font > (unsigned)FL_FREE_FONT) // already been here
    return (Fl_Font)fl_free_font;
  fl_open_display();
  int xlistsize;
  char buf[20];
  if (!xstarname) {
    strcpy(buf,"-*-"); strcpy(buf+3,fl_encoding);
    xstarname = buf;
  }
  char **xlist = XListFonts(fl_display, xstarname, 10000, &xlistsize);
  if (!xlist) return (Fl_Font)fl_free_font;
  qsort(xlist, xlistsize, sizeof(*xlist), ultrasort);
  int used_xlist = 0;
  for (int i=0; i<xlistsize;) {
    int first_xlist = i;
    const char *p = xlist[i++];
    char canon[1024];
    int size = to_canonical(canon, p, sizeof(canon));
    if (size >= 0) {
      for (;;) { // find all matching fonts:
	if (i >= xlistsize) break;
	const char *q = xlist[i];
	char this_canon[1024];
	if (to_canonical(this_canon, q, sizeof(this_canon)) < 0) break;
	if (strcmp(canon, this_canon)) break;
	i++;
      }
      /*if (*p=='-' || i > first_xlist+1)*/ p = canon;
    }
    unsigned int j;
    for (j = 0;; j++) {
      /*if (j < FL_FREE_FONT) {
	// see if it is one of our built-in fonts:
	// if so, set the list of x fonts, since we have it anyway
	if (fl_fonts[j].name && !strcmp(fl_fonts[j].name, p)) break;
      } else */{
	j = fl_free_font++;
	if (p == canon) p = strdup(p); else used_xlist = 1;
	Fl::set_font((Fl_Font)j, p);
	break;
      }
    }
    if (!fl_fonts[j].xlist) {
      fl_fonts[j].xlist = xlist+first_xlist;
      fl_fonts[j].n = -(i-first_xlist);
      used_xlist = 1;
    }
  }
  if (!used_xlist) XFreeFontNames(xlist);
  return (Fl_Font)fl_free_font;
}

int Fl_Xlib_Graphics_Driver::get_font_sizes(Fl_Font fnum, int*& sizep) {
  Fl_Fontdesc *s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // empty slot in table, use entry 0
  if (!s->xlist) {
    fl_open_display();
    s->xlist = XListFonts(fl_display, s->name, 100, &(s->n));
    if (!s->xlist) return 0;
  }
  int listsize = s->n; if (listsize<0) listsize = -listsize;
  static int sizes[128];
  int numsizes = 0;
  for (int i = 0; i < listsize; i++) {
    char *q = s->xlist[i];
    char *d = fl_find_fontsize(q);
    if (!d) continue;
    int s = strtol(d,0,10);
    if (!numsizes || sizes[numsizes-1] < s) {
      sizes[numsizes++] = s;
    } else {
      // insert-sort the new size into list:
      int n;
      for (n = numsizes-1; n > 0; n--) if (sizes[n-1] < s) break;
      if (sizes[n] != s) {
	for (int m = numsizes; m > n; m--) sizes[m] = sizes[m-1];
	sizes[n] = s;
	numsizes++;
      }
    }
  }
  sizep = sizes;
  return numsizes;
}


#ifndef FL_DOXYGEN

Fl_Font_Descriptor::Fl_Font_Descriptor(const char* name) {
  font = XCreateUtf8FontStruct(fl_display, name);
  if (!font) {
    Fl::warning("bad font: %s", name);
    font = XCreateUtf8FontStruct(fl_display, "fixed");
  }
#  if HAVE_GL
  listbase = 0;
  for (int u = 0; u < 64; u++) glok[u] = 0;
#  endif
}

Fl_XFont_On_Demand fl_xfont;

Fl_Font_Descriptor::~Fl_Font_Descriptor() {
#  if HAVE_GL
// Delete list created by gl_draw().  This is not done by this code
// as it will link in GL unnecessarily.  There should be some kind
// of "free" routine pointer, or a subclass?
// if (listbase) {
//  int base = font->min_char_or_byte2;
//  int size = font->max_char_or_byte2-base+1;
//  int base = 0; int size = 256;
//  glDeleteLists(listbase+base,size);
// }
#  endif
  if (this == fl_graphics_driver->font_descriptor()) {
    fl_graphics_driver->font_descriptor(NULL);
    fl_xfont = 0;
  }
  XFreeUtf8FontStruct(fl_display, font);
}

////////////////////////////////////////////////////////////////

// WARNING: if you add to this table, you must redefine FL_FREE_FONT
// in Enumerations.H & recompile!!
static Fl_Fontdesc built_in_table[] = {
{"-*-helvetica-medium-r-normal--*"},
{"-*-helvetica-bold-r-normal--*"},
{"-*-helvetica-medium-o-normal--*"},
{"-*-helvetica-bold-o-normal--*"},
{"-*-courier-medium-r-normal--*"},
{"-*-courier-bold-r-normal--*"},
{"-*-courier-medium-o-normal--*"},
{"-*-courier-bold-o-normal--*"},
{"-*-times-medium-r-normal--*"},
{"-*-times-bold-r-normal--*"},
{"-*-times-medium-i-normal--*"},
{"-*-times-bold-i-normal--*"},
{"-*-symbol-*"},
{"-*-lucidatypewriter-medium-r-normal-sans-*"},
{"-*-lucidatypewriter-bold-r-normal-sans-*"},
{"-*-*zapf dingbats-*"}
};

Fl_Fontdesc* fl_fonts = built_in_table;

#define MAXSIZE 32767

// return dash number N, or pointer to ending null if none:
static const char* fl_font_word(const char* p, int n) {
  while (*p) {if (*p=='-') {if (!--n) break;} p++;}
  return p;
}

// return a pointer to a number we think is "point size":
static char* fl_find_fontsize(char* name) {
  char* c = name;
  // for standard x font names, try after 7th dash:
  if (*c == '-') {
    c = (char*)fl_font_word(c,7);
    if (*c++ && isdigit(*c)) return c;
    return 0; // malformed x font name?
  }
  char* r = 0;
  // find last set of digits:
  for (c++;* c; c++)
    if (isdigit(*c) && !isdigit(*(c-1))) r = c;
  return r;
}

//const char* fl_encoding = "iso8859-1";
const char* fl_encoding = "iso10646-1";

// return true if this matches fl_encoding:
int fl_correct_encoding(const char* name) {
  if (*name != '-') return 0;
  const char* c = fl_font_word(name,13);
  return (*c++ && !strcmp(c,fl_encoding));
}

static const char *find_best_font(const char *fname, int size) {
  int cnt;
  static char **list = NULL;
// locate or create an Fl_Font_Descriptor for a given Fl_Fontdesc and size:
  if (list) XFreeFontNames(list);
  list = XListFonts(fl_display, fname, 100, &cnt);
  if (!list) return "fixed";

  // search for largest <= font size:
  char* name = list[0]; int ptsize = 0;     // best one found so far
  int matchedlength = 32767;
  char namebuffer[1024];        // holds scalable font name
  int found_encoding = 0;
  int m = cnt; if (m<0) m = -m;
  for (int n=0; n < m; n++) {
    char* thisname = list[n];
    if (fl_correct_encoding(thisname)) {
      if (!found_encoding) ptsize = 0; // force it to choose this
      found_encoding = 1;
    } else {
      if (found_encoding) continue;
    }
    char* c = (char*)fl_find_fontsize(thisname);
    int thissize = c ? atoi(c) : MAXSIZE;
    int thislength = strlen(thisname);
    if (thissize == size && thislength < matchedlength) {
      // exact match, use it:
      name = thisname;
      ptsize = size;
      matchedlength = thislength;
    } else if (!thissize && ptsize!=size) {
      // whoa!  A scalable font!  Use unless exact match found:
      int l = c-thisname;
      memcpy(namebuffer,thisname,l);
      l += sprintf(namebuffer+l,"%d",size);
      while (*c == '0') c++;
      strcpy(namebuffer+l,c);
      name = namebuffer;
      ptsize = size;
    } else if (!ptsize ||	// no fonts yet
	       (thissize < ptsize && ptsize > size) || // current font too big
	       (thissize > ptsize && thissize <= size) // current too small
      ) {
      name = thisname;
      ptsize = thissize;
      matchedlength = thislength;
    }
  }

//  if (ptsize != size) { // see if we already found this unscalable font:
//    for (f = s->first; f; f = f->next) {
//      if (f->minsize <= ptsize && f->maxsize >= ptsize) {
//	if (f->minsize > size) f->minsize = size;
//	if (f->maxsize < size) f->maxsize = size;
//	return f;
//      }
//    }
//  }
//
//  // okay, we definately have some name, make the font:
//  f = new Fl_Font_Descriptor(name);
//  if (ptsize < size) {f->minsize = ptsize; f->maxsize = size;}
//  else {f->minsize = size; f->maxsize = ptsize;}
//  f->next = s->first;
//  s->first = f;
//  return f;

  return name;
}

static char *put_font_size(const char *n, int size)
{
        int i = 0;
        char *buf;
        const char *ptr;
        const char *f;
        char *name;
        int nbf = 1;
        name = strdup(n);
        while (name[i]) {
                if (name[i] == ',') {nbf++; name[i] = '\0';}
                i++;
        }

        buf = (char*) malloc(nbf * 256);
        buf[0] = '\0';
        ptr = name;
        i = 0;
        while (ptr && nbf > 0) {
                f = find_best_font(ptr, size);
                while (*f) {
                        buf[i] = *f;
                        f++; i++;
                }
                nbf--;
                while (*ptr) ptr++;
                if (nbf) {
                        ptr++;
                        buf[i] = ',';
                        i++;
                }
                while(isspace(*ptr)) ptr++;
        }
        buf[i] = '\0';
        free(name);
        return buf;
}


char *fl_get_font_xfld(int fnum, int size) {
  Fl_Fontdesc* s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // use font 0 if still undefined
  fl_open_display();
  return put_font_size(s->name, size);
}

// locate or create an Fl_Font_Descriptor for a given Fl_Fontdesc and size:
static Fl_Font_Descriptor* find(int fnum, int size) {
  char *name;
  Fl_Fontdesc* s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // use font 0 if still undefined
  Fl_Font_Descriptor* f;
  for (f = s->first; f; f = f->next)
    if (f->size == size) return f;
  fl_open_display();

  name = put_font_size(s->name, size);
  f = new Fl_Font_Descriptor(name);
  f->size = size;
  f->next = s->first;
  s->first = f;
  free(name);
  return f;
}


////////////////////////////////////////////////////////////////
// Public interface:

void *fl_xftfont = 0;
static GC font_gc;

XFontStruct* Fl_XFont_On_Demand::value() {
  return ptr;
}

void Fl_Xlib_Graphics_Driver::font(Fl_Font fnum, Fl_Fontsize size) {
  if (fnum==-1) {
    Fl_Graphics_Driver::font(0, 0);
    return;
  }
  if (fnum == Fl_Graphics_Driver::font() && size == Fl_Graphics_Driver::size()) return;
  Fl_Graphics_Driver::font(fnum, size);
  Fl_Font_Descriptor* f = find(fnum, size);
  if (f != this->font_descriptor()) {
    this->font_descriptor(f);
    fl_xfont = f->font->fonts[0];
    font_gc = 0;
  }
}

int Fl_Xlib_Graphics_Driver::height() {
  if (font_descriptor()) return font_descriptor()->font->ascent + font_descriptor()->font->descent;
  else return -1;
}

int Fl_Xlib_Graphics_Driver::descent() {
  if (font_descriptor()) return font_descriptor()->font->descent;
  else return -1;
}

double Fl_Xlib_Graphics_Driver::width(const char* c, int n) {
  if (font_descriptor()) return (double) XUtf8TextWidth(font_descriptor()->font, c, n);
  else return -1;
}

double Fl_Xlib_Graphics_Driver::width(unsigned int c) {
  if (font_descriptor()) return (double) XUtf8UcsWidth(font_descriptor()->font, c);
  else return -1;
}

void Fl_Xlib_Graphics_Driver::text_extents(const char *c, int n, int &dx, int &dy, int &W, int &H) {
  if (font_gc != gc_) {
    if (!font_descriptor()) font(FL_HELVETICA, FL_NORMAL_SIZE);
    font_gc = gc_;
    XSetFont(fl_display, gc_, font_descriptor()->font->fid);
  }
  int xx, yy, ww, hh;
  xx = yy = ww = hh = 0;
  if (gc_) XUtf8_measure_extents(fl_display, fl_window, font_descriptor()->font, gc_, &xx, &yy, &ww, &hh, c, n);

  W = ww; H = hh; dx = xx; dy = yy;
// This is the safe but mostly wrong thing we used to do...
//  W = 0; H = 0;
//  fl_measure(c, W, H, 0);
//  dx = 0;
//  dy = fl_descent() - H;
}

void Fl_Xlib_Graphics_Driver::draw(const char* c, int n, int x, int y) {
  if (font_gc != gc_) {
    if (!font_descriptor()) this->font(FL_HELVETICA, FL_NORMAL_SIZE);
    font_gc = gc_;
    XSetFont(fl_display, gc_, font_descriptor()->font->fid);
  }
  if (gc_) XUtf8DrawString(fl_display, fl_window, font_descriptor()->font, gc_, x, y, c, n);
}

void Fl_Xlib_Graphics_Driver::draw(int angle, const char *str, int n, int x, int y) {
  static char warning = 0; // issue warning only once
  if (!warning && angle != 0) {
    warning = 1;
    fprintf(stderr,
	    "libfltk: rotated text not implemented by X backend.\n"
	    "  You should use the Xft backend. Check USE_XFT in config.h.\n");
  }
  this->draw(str, n, (int)x, (int)y);
}

void Fl_Xlib_Graphics_Driver::rtl_draw(const char* c, int n, int x, int y) {
  if (font_gc != gc_) {
    if (!font_descriptor()) this->font(FL_HELVETICA, FL_NORMAL_SIZE);
    font_gc = gc_;
  }
  if (gc_) XUtf8DrawRtlString(fl_display, fl_window, font_descriptor()->font, gc_, x, y, c, n);
}

float Fl_Xlib_Graphics_Driver::scale_font_for_PostScript(Fl_Font_Descriptor *desc, int s) {
  float ps_size = (float) s;
  // Non-Xft fonts can be smaller than required.
  // Set the PostScript font size to the display font height
  char *name = desc->font->font_name_list[0];
  char *p = strstr(name, "--");
  if (p) {
    sscanf(p + 2, "%f", &ps_size);
  }
  return ps_size;
}

float Fl_Xlib_Graphics_Driver::scale_bitmap_for_PostScript() {
  return 1;
}

#endif // FL_DOXYGEN

//
// End of "$Id$".
//
