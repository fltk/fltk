/*
 * Author: Jean-Marc Lienher ( http://oksid.ch )
 * Copyright 2000-2003 by O'ksi'D.
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file.  If this
 * file is missing or damaged, see the license at:
 *
 *     https://www.fltk.org/COPYING.php
 *
 * Please see the following page on how to report bugs and issues:
 *
 *     https://www.fltk.org/bugs.php
 */

/*
 * X11 UTF-8 text drawing functions.
 *
 * This file is compiled and linked only for X11 w/o Xft.
 */

#include "../Xutf8.h"
#include <X11/Xlib.h>
#include <FL/fl_string_functions.h>  // fl_strdup()
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../utf8_internal.h"

/* External auto generated functions : */
#include "ucs2fontmap.c"
/*
 * extern int ucs2fontmap(char *s, unsigned int ucs, int enc);
 * extern int encoding_number(const char *enc);
 * extern const char *encoding_name(int num);
 */

/* The ARM header files have a bug by not taking into account that ARM cpu
 * likes pacing to 4 bytes. This little trick defines our own version of
 * XChar2b which does not have this problem
 */

#if defined(__GNUC__) && defined(__arm__) && !defined(__ARM_EABI__)
typedef struct {
  unsigned char byte1;
  unsigned char byte2;
}
__attribute__ ((packed))
Fl_XChar2b;
#else
#define Fl_XChar2b XChar2b
#endif


/*********************************************************************/
/** extract a list of font from the base font name list             **/
/*********************************************************************/
static int
get_font_list(
        const char      *base_font_name_list,
        char            ***flist) {
  const char *ptr;
  const char *p;
  int nb;
  int nb_name;

  ptr = base_font_name_list;
  p = NULL;
  nb = 0;
  nb_name = 1;

  while (*ptr) {
    if (*ptr == ',') nb_name++;
    ptr++;
  }

  *flist = (char **) malloc(sizeof(char*) * nb_name);
  ptr = base_font_name_list;

  while (*ptr) {
    int l = 0, i = 0;

    while(isspace((int)(unsigned char)*ptr)) ptr++;
    p = ptr;
    while (*ptr && *ptr != ',') { ptr++; l++; }
    if (l > 2) {
      (*flist)[nb] = (char*) malloc((unsigned)l + 2);
      while (p != ptr) { ((*flist)[nb])[i] = *p; i++; p++; }
      (*flist)[nb][i] = '\0';
      nb++;
    }
    if (*ptr) ptr++;
  }
  if (nb < 1) {
    free(*flist);
    *flist = (char**)NULL;
  }
  return nb;
}

/*********************************************************************/
/** get the font name used as encoding for "fontspecific" encoding  **/
/** (mainly used for adobe-symbol and adobe-zapfdingbats)           **/
/*********************************************************************/
static int
font_spec_enc(char *font) {
  int ret;
  char *enc;
  char *end;

  enc = font;
  while (*enc != '-') enc++;
  enc++;
  while (*enc != '-') enc++;
  enc++;
  end = enc;
  while (*end != '-') end++;
  *end = '\0';

  ret = encoding_number(enc);
  *end = '-';

  return ret;
}


/*********************************************************************/
/** get the sub range of a iso10646-1 font                          **/
/*********************************************************************/
static void
get_range(const char    *enc,
          int           *min,
          int           *max) {

  const char *ptr = enc;
  const char *ptr1;

  if (!enc) return;

  while (*ptr && *ptr != '-') ptr++;
  if (!*ptr) return;
  while (*ptr && *ptr != '[') ptr++;
  if (!*ptr) return;
  *min = 0xFFFF;
  *max = 0;
  while (*ptr && *ptr != ']') {
    int val;
    ptr++;
    ptr1 = ptr;
    while (*ptr && *ptr != ']' && *ptr != ' ' && *ptr != '_') ptr++;
    val = strtol(ptr1, NULL, 0);
    if (val < *min) *min = val;
    if (val > *max) *max = val;
  }
}

/*********************************************************************/
/** get the internal encoding number of each fonts                  **/
/*********************************************************************/
static int *
get_encodings(char      **font_name_list,
              int       *ranges,
              int       nb_font) {

  int *font_encoding_list;
  int i;
  i = 0;

  font_encoding_list = (int *) malloc(sizeof(int) * nb_font);
  while (i < nb_font) {
    char *ptr;
    int ec;
    ptr = font_name_list[i];
    ec = 0;
    font_encoding_list[i] = -1;
    ranges[i * 2] = 0;
    ranges[i * 2 + 1] = 0xFFFF;

    if (ptr && strstr(ptr, "fontspecific")) {
      font_encoding_list[i] = font_spec_enc(ptr);
      ptr = NULL;
    }
    while (ptr && *ptr) {
      if (*ptr == '-') {
        ec++;
        if (ec == 13) {
          font_encoding_list[i] = encoding_number(ptr + 1);
          if (font_encoding_list[i] == 0) {
            get_range(ptr + 1,
                      ranges + i * 2,
                      ranges + i * 2 + 1);
          }
          break;
        }
      }
      ptr++;
    }
    if (font_encoding_list[i] < 0) font_encoding_list[i] = 1;
    i++;
  }
  return font_encoding_list;
}

/*********************************************************************/
/** find the first font which matches the name and load it.         **/
/*********************************************************************/
static XFontStruct *
find_best_font(Display  *dpy,
               char     **name) {

  char **list;
  int cnt;
  XFontStruct *s;

  list = XListFonts(dpy, *name, 1, &cnt);
  if (cnt && list) {
    free(*name);
    *name = fl_strdup(list[0]);
    s = XLoadQueryFont(dpy, *name);
    XFreeFontNames(list);
    return s;
  }
  return NULL;
}

/*********************************************************************/
/** load all fonts                                                  **/
/*********************************************************************/
static void
load_fonts(Display         *dpy,
           XUtf8FontStruct *font_set) {

  int i = 0;

  font_set->fonts = (XFontStruct**) malloc(sizeof(XFontStruct*) *
                                           font_set->nb_font);

  font_set->ranges = (int*) malloc(sizeof(int) *
                                   font_set->nb_font * 2);

  font_set->descent = 0;
  font_set->ascent = 0;
  font_set->fid = 0;

  while (i < font_set->nb_font) {
    XFontStruct *fnt;

    fnt = font_set->fonts[i] =
      find_best_font(dpy, &(font_set->font_name_list[i]));
    if (fnt) {
      font_set->fid = fnt->fid;
      if (fnt->ascent > font_set->ascent) {
        font_set->ascent = fnt->ascent;
      }
      if (fnt->descent > font_set->descent) {
        font_set->descent = fnt->descent;
      }
    } else {
      free(font_set->font_name_list[i]);
      font_set->font_name_list[i] = NULL;
    }
    i++;
  }

  font_set->encodings =
    get_encodings(font_set->font_name_list,
      font_set->ranges, font_set->nb_font);

  /* unload fonts with same encoding */
  for (i = 0; i < font_set->nb_font; i++) {
    if (font_set->font_name_list[i]) {
      int j;
      for (j = 0; j < i; j++) {
        if (font_set->font_name_list[j] &&
            font_set->encodings[j] ==
            font_set->encodings[i] &&
            font_set->ranges[2*j] ==
            font_set->ranges[2*i] &&
            font_set->ranges[(2*j)+1] &&
            font_set->ranges[(2*i)+1]) {
          XFreeFont(dpy, font_set->fonts[i]);
          free(font_set->font_name_list[i]);
          font_set->font_name_list[i] = NULL;
          font_set->fonts[i] = 0;
        }
      }
    }
  }
}

/*********************************************************************/
/** Creates an array of XFontStruct acording to the comma separated **/
/** list of fonts. XLoad all fonts.                                 **/
/*********************************************************************/
XUtf8FontStruct *
XCreateUtf8FontStruct(Display    *dpy,
                      const char *base_font_name_list) {

  XUtf8FontStruct *font_set;

  font_set = (XUtf8FontStruct*)malloc(sizeof(XUtf8FontStruct));

  if (!font_set) {
    return NULL;
  }

  font_set->nb_font = get_font_list(base_font_name_list,
                                    &font_set->font_name_list);

  if (font_set->nb_font < 1) {
    free(font_set);
    return NULL;
  }

  load_fonts(dpy, font_set);

  return font_set;
}


/*****************************************************************************/
/** draw a Right To Left UTF-8 string using multiple fonts as needed.       **/
/*****************************************************************************/
void
XUtf8DrawRtlString(Display              *display,
                   Drawable             d,
                   XUtf8FontStruct      *font_set,
                   GC                   gc,
                   int                  x,
                   int                  y,
                   const char           *string,
                   int                  num_bytes) {

  int           *encodings;     /* encodings array */
  XFontStruct   **fonts;        /* fonts array */
  Fl_XChar2b    buf[128];       /* drawing buffer */
  Fl_XChar2b    *ptr;           /* pointer to the drawing buffer */
  int           fnum;           /* index of the current font in the fonts array*/
  int           i;              /* current byte in the XChar2b buffer */
  int           first;          /* first valid font index */
  int           last_fnum;      /* font index of the previous char */
  int           nb_font;        /* quantity of fonts in the font array */
  char          glyph[2];       /* byte1 and byte2 value of the UTF-8 char */
  int           *ranges;        /* sub range of iso10646 */

  nb_font = font_set->nb_font;

  if (nb_font < 1) {
    /* there is no font in the font_set :-( */
    return;
  }

  ranges = font_set->ranges;
  fonts = font_set->fonts;
  encodings = font_set->encodings;
  i = 0;
  fnum = 0;
  ptr = buf + 128;

  while(fnum < nb_font && !fonts[fnum]) fnum++;
  if (fnum >= nb_font) {
    /* there is no valid font for the X server */
    return;
  }

  first = fnum;
  last_fnum = fnum;

  while (num_bytes > 0) {
    int          ulen;   /* byte length of the UTF-8 char */
    unsigned int ucs;    /* Unicode value of the UTF-8 char */
    unsigned int no_spc; /* Spacing char equivalent of a non-spacing char */

    if (i > 120) {
      /*** draw the buffer **/
      XSetFont(display, gc, fonts[fnum]->fid);
      x -= XTextWidth16(fonts[fnum], ptr, i);
      XDrawString16(display, d, gc, x, y, ptr, i);
      i = 0;
      ptr = buf + 128;
    }

    ulen = XFastConvertUtf8ToUcs((const unsigned char*)string, num_bytes, &ucs);

    if (ulen < 1) ulen = 1;

    no_spc = XUtf8IsNonSpacing(ucs);
    if (no_spc) ucs = no_spc;

    /*
     * find the first encoding which can be used to
     * draw the glyph
     */
    fnum = first;
    while (fnum < nb_font) {
      if (fonts[fnum] && ucs2fontmap(glyph, ucs, encodings[fnum]) >= 0) {
        if (encodings[fnum] != 0 ||
            ((int)ucs >= ranges[fnum * 2] && (int)ucs <= ranges[fnum * 2 + 1])) {
          break;
        }
      }
      fnum++;
    }
    if (fnum == nb_font) {
      /* the char is not valid in all encodings ->
       * draw it using the first font :-(
       */
      fnum = first;
      ucs2fontmap(glyph, '?', encodings[fnum]);
    }

    if (last_fnum != fnum || no_spc) {
      XSetFont(display, gc, fonts[last_fnum]->fid);
      x -= XTextWidth16(fonts[last_fnum], ptr, i);
      XDrawString16(display, d, gc, x, y, ptr, i);
      i = 0;
      ptr = buf + 127;
      (*ptr).byte1 = glyph[0];
      (*ptr).byte2 = glyph[1];
      if (no_spc) {
        x += XTextWidth16(fonts[fnum], ptr, 1);
      }
    } else {
      ptr--;
      (*ptr).byte1 = glyph[0];
      (*ptr).byte2 = glyph[1];
    }
    last_fnum = fnum;
    i++;
    string += ulen;
    num_bytes -= ulen;
  }

  if (i < 1) return;

  XSetFont(display, gc, fonts[fnum]->fid);
  x -= XTextWidth16(fonts[last_fnum], ptr, i);
  XDrawString16(display, d, gc, x, y, ptr, i);
}


/*****************************************************************************/
/** draw an UTF-8 string using multiple fonts as needed.                    **/
/*****************************************************************************/
void
XUtf8DrawString(Display         *display,
                Drawable        d,
                XUtf8FontStruct *font_set,
                GC              gc,
                int             x,
                int             y,
                const char      *string,
                int             num_bytes) {

  int           *encodings; /* encodings array */
  XFontStruct   **fonts;    /* fonts array */
  Fl_XChar2b    buf[128];   /* drawing buffer */
  int           fnum;       /* index of the current font in the fonts array*/
  int           i;          /* current byte in the XChar2b buffer */
  int           first;      /* first valid font index */
  int           last_fnum;  /* font index of the previous char */
  int           nb_font;    /* quantity of fonts in the font array */
  char          glyph[2];   /* byte1 and byte2 value of the UTF-8 char */
  int           *ranges;    /* sub range of iso10646 */

  nb_font = font_set->nb_font;

  if (nb_font < 1) {
    /* there is no font in the font_set :-( */
    return;
  }
  ranges = font_set->ranges;
  fonts = font_set->fonts;
  encodings = font_set->encodings;
  i = 0;
  fnum = 0;

  while(fnum < nb_font && !fonts[fnum]) fnum++;
  if (fnum >= nb_font) {
    /* there is no valid font for the X server */
    return;
  }

  first = fnum;
  last_fnum = fnum;

  while (num_bytes > 0) {
    int          ulen;   /* byte length of the UTF-8 char */
    unsigned int ucs;    /* Unicode value of the UTF-8 char */
    unsigned int no_spc; /* Spacing char equivalent of a non-spacing char */

    if (i > 120) {
      /*** draw the buffer **/
      XSetFont(display, gc, fonts[fnum]->fid);
      XDrawString16(display, d, gc, x, y, buf, i);
      x += XTextWidth16(fonts[fnum], buf, i);
      i = 0;
    }

    ulen = XFastConvertUtf8ToUcs((const unsigned char*)string, num_bytes, &ucs);

    if (ulen < 1) ulen = 1;

    no_spc = XUtf8IsNonSpacing(ucs);
    if (no_spc) ucs = no_spc;

    /*
     * find the first encoding which can be used to
     * draw the glyph
     */
    fnum = first;
    while (fnum < nb_font) {
      if (fonts[fnum] && ucs2fontmap(glyph, ucs, encodings[fnum]) >= 0) {
        if (encodings[fnum] != 0 ||
            ((int)ucs >= ranges[fnum * 2] &&
             (int)ucs <= ranges[fnum * 2 + 1])) {
          break;
        }
      }
      fnum++;
    }
    if (fnum == nb_font) {
      /* the char is not valid in all encodings ->
       * draw it using the first font :-(
       */
      fnum = first;
      ucs2fontmap(glyph, '?', encodings[fnum]);
    }

    if (last_fnum != fnum || no_spc) {
      XSetFont(display, gc, fonts[last_fnum]->fid);
      XDrawString16(display, d, gc, x, y, buf, i);
      x += XTextWidth16(fonts[last_fnum], buf, i);
      i = 0;
      (*buf).byte1 = glyph[0];
      (*buf).byte2 = glyph[1];
      if (no_spc) {
        x -= XTextWidth16(fonts[fnum], buf, 1);
      }
    } else {
      (*(buf + i)).byte1 = glyph[0];
      (*(buf + i)).byte2 = glyph[1];
    }
    last_fnum = fnum;
    i++;
    string += ulen;
    num_bytes -= ulen;
  }

  XSetFont(display, gc, fonts[fnum]->fid);
  XDrawString16(display, d, gc, x, y, buf, i);
}


/*****************************************************************************/
/** Measure the inked extents of a UTF-8 string using multiple fonts as     **/
/** needed. Tries to mirror the behaviour of the draw function              **/
/** XUtf8DrawString() as closely as possible to get consistent sizes.       **/
/*****************************************************************************/
void
XUtf8_measure_extents(
        Display                 *display,
        Drawable                d,
        XUtf8FontStruct  *font_set,
        GC                      gc,
        int                     *xx,     /* x-offset from origin */
        int                     *yy,     /* y-offset from origin */
        int                     *ww,     /* overall inked width  */
        int                     *hh,     /* maximum inked height */
        const char              *string, /* text to measure */
        int                     num_bytes) {
  int           *encodings; /* encodings array */
  XFontStruct   **fonts;    /* fonts array */
  Fl_XChar2b    buf[128];   /* drawing buffer */
  int           fnum;       /* index of the current font in the fonts array*/
  int           i;          /* current byte in the XChar2b buffer */
  int           first;      /* first valid font index */
  int           last_fnum;  /* font index of the previous char */
  int           nb_font;    /* quantity of fonts in the font array */
  char          glyph[2];   /* byte1 and byte2 value of the UTF-8 char */
  int           *ranges;    /* sub range of iso10646 */

  int wd = 0; /* accumulates the width of the text */
  int ht = 0; /* used to find max height in text */
  int hs;     /* "height sum" of current text segment */
  int yt = 0x7FFFFFFF; /* used to find bounding rectangle delta-y */
  /* int res; */ /* result from calling XTextExtents16() - we should test this is OK! */
  /* FC: the man does not specify error codes for it, but X will generate X errors like BadGC or BadFont. */

  XCharStruct sizes;
  int dir_ret = 0;
  int fnt_asc = 0;
  int fnt_dsc = 0;

  nb_font = font_set->nb_font;

  if (nb_font < 1) {
    /* there is no font in the font_set :-( */
    return;
  }
  ranges = font_set->ranges;
  fonts = font_set->fonts;
  encodings = font_set->encodings;
  i = 0;
  fnum = 0;

  while(fnum < nb_font && !fonts[fnum]) fnum++;
  if (fnum >= nb_font) {
    /* there is no valid font for the X server */
    return;
  }

  first = fnum;
  last_fnum = fnum;

  while (num_bytes > 0) {
    int          ulen;   /* byte length of the UTF-8 char */
    unsigned int ucs;    /* Unicode value of the UTF-8 char */
    unsigned int no_spc; /* Spacing char equivalent of a non-spacing char */

    if (i > 120) {
      /*** draw the buffer **/
      XSetFont(display, gc, fonts[fnum]->fid);
      /* res = */ XTextExtents16(fonts[fnum], buf, i, &dir_ret, &fnt_asc, &fnt_dsc, &sizes);
      /* recover the dimensions - should verify that res == 0 first! */
      wd += sizes.width; /* accumulate the width */
      hs = sizes.ascent + sizes.descent; /* total height */
      if(hs > ht) ht = hs; /* new height exceeds previous height */
      if(yt > (-sizes.ascent)) yt = -sizes.ascent; /* delta y offset */
      i = 0;
    }

    ulen = XFastConvertUtf8ToUcs((const unsigned char*)string, num_bytes, &ucs);

    if (ulen < 1) ulen = 1;

    no_spc = XUtf8IsNonSpacing(ucs);
    if (no_spc) ucs = no_spc;

    /*
     * find the first encoding which can be used to
     * draw the glyph
     */
    fnum = first;
    while (fnum < nb_font) {
      if (fonts[fnum] && ucs2fontmap(glyph, ucs, encodings[fnum]) >= 0) {
        if (encodings[fnum] != 0 ||
            ((int)ucs >= ranges[fnum * 2] &&
             (int)ucs <= ranges[fnum * 2 + 1])) {
          break;
        }
      }
      fnum++;
    }
    if (fnum == nb_font) {
      /* the char is not valid in all encodings ->
       * draw it using the first font :-(
       */
      fnum = first;
      ucs2fontmap(glyph, '?', encodings[fnum]);
    }

    if (last_fnum != fnum || no_spc) {
      XSetFont(display, gc, fonts[last_fnum]->fid);
      /* res = */ XTextExtents16(fonts[last_fnum], buf, i, &dir_ret, &fnt_asc, &fnt_dsc, &sizes);
      /* recover the dimensions - should verify that res == 0 first! */
      wd += sizes.width; /* accumulate the width */
      hs = sizes.ascent + sizes.descent; /* total height */
      if(hs > ht) ht = hs; /* new height exceeds previous height */
      if(yt > (-sizes.ascent)) yt = -sizes.ascent; /* delta y offset */
      i = 0;
      (*buf).byte1 = glyph[0];
      (*buf).byte2 = glyph[1];
      if (no_spc) {
        wd -= XTextWidth16(fonts[fnum], buf, 1);
      }
    } else {
      (*(buf + i)).byte1 = glyph[0];
      (*(buf + i)).byte2 = glyph[1];
    }
    last_fnum = fnum;
    i++;
    string += ulen;
    num_bytes -= ulen;
  }

  XSetFont(display, gc, fonts[fnum]->fid);
  /* res = */ XTextExtents16(fonts[fnum], buf, i, &dir_ret, &fnt_asc, &fnt_dsc, &sizes);
  /* recover the dimensions - should verify that res == 0 first! */
  wd += sizes.width; /* accumulate the width */
  hs = sizes.ascent + sizes.descent; /* total height */
  if(hs > ht) ht = hs; /* new height exceeds previous height */
  if(yt > (-sizes.ascent)) yt = -sizes.ascent; /* delta y offset */
  /* return values */
  *ww = wd; /* width of inked area rectangle */
  *hh = ht; /* max height of inked area rectangle */
  *xx = 0;  /* x-offset from origin to start of inked area - this is wrong! */
  *yy = yt; /* y-offset from origin to start of inked rectangle */
}


/*****************************************************************************/
/** returns the pixel width of a UTF-8 string                               **/
/*****************************************************************************/
int
XUtf8TextWidth(XUtf8FontStruct  *font_set,
               const char       *string,
               int              num_bytes) {

  int           x;
  int           *encodings; /* encodings array */
  XFontStruct   **fonts;    /* fonts array */
  Fl_XChar2b    buf[128];   /* drawing buffer */
  int           fnum;       /* index of the current font in the fonts array*/
  int           i;          /* current byte in the XChar2b buffer */
  int           first;      /* first valid font index */
  int           last_fnum;  /* font index of the previous char */
  int           nb_font;    /* quantity of fonts in the font array */
  char          glyph[2];   /* byte1 and byte2 value of the UTF-8 char */
  int           *ranges;    /* sub range of iso10646 */

  nb_font = font_set->nb_font;
  x = 0;

  if (nb_font < 1) {
    /* there is no font in the font_set :-( */
    return x;
  }

  ranges = font_set->ranges;
  fonts = font_set->fonts;
  encodings = font_set->encodings;
  i = 0;
  fnum = 0;

  while(fnum < nb_font && !fonts[fnum]) fnum++;
  if (fnum >= nb_font) {
    /* there is no valid font for the X server */
    return x;
  }

  first = fnum;
  last_fnum = fnum;

  while (num_bytes > 0) {
    int          ulen;   /* byte length of the UTF-8 char */
    unsigned int ucs;    /* Unicode value of the UTF-8 char */
    unsigned int no_spc; /* Spacing char equivalent of a non-spacing char */

    if (i > 120) {
      /*** measure the buffer **/
      x += XTextWidth16(fonts[fnum], buf, i);
      i = 0;
    }

    ulen = XFastConvertUtf8ToUcs((const unsigned char*)string, num_bytes, &ucs);

    if (ulen < 1) ulen = 1;

    no_spc = XUtf8IsNonSpacing(ucs);
    if (no_spc) {
      ucs = no_spc;
    }

    /*
     * find the first encoding which can be used to
     * draw the glyph
     */
    fnum = first;
    while (fnum < nb_font) {
      if (fonts[fnum] && ucs2fontmap(glyph, ucs, encodings[fnum]) >= 0) {
        if (encodings[fnum] != 0 ||
            ((int)ucs >= ranges[fnum * 2] &&
             (int)ucs <= ranges[fnum * 2 + 1])) {
          break;
        }
      }
      fnum++;
    }
    if (fnum == nb_font) {
      /* the char is not valid in all encodings ->
       * draw it using the first font :-(
       */
      fnum = first;
      ucs2fontmap(glyph, '?', encodings[fnum]);
    }

    if (last_fnum != fnum || no_spc) {
      x += XTextWidth16(fonts[last_fnum], buf, i);
      i = 0;
      (*buf).byte1 = glyph[0];
      (*buf).byte2 = glyph[1];
      if (no_spc) {
        /* go back to draw the non-spacing char over the previous char */
        x -= XTextWidth16(fonts[fnum], buf, 1);
      }
    } else {
      (*(buf + i)).byte1 = glyph[0];
      (*(buf + i)).byte2 = glyph[1];
    }
    last_fnum = fnum;
    i++;
    string += ulen;
    num_bytes -= ulen;
  }

  x += XTextWidth16(fonts[last_fnum], buf, i);

  return x;
}

/*****************************************************************************/
/**  get the X font and glyph ID of a UCS char                              **/
/*****************************************************************************/
int
fl_XGetUtf8FontAndGlyph(XUtf8FontStruct  *font_set,
                        unsigned int     ucs,
                        XFontStruct      **fnt,
                        unsigned short   *id) {

  int             *encodings; /* encodings array */
  XFontStruct     **fonts;    /* fonts array */
  int             fnum;       /* index of the current font in the fonts array*/
  int             first;      /* first valid font index */
  int             nb_font;    /* quantity of fonts in the font array */
  char            glyph[2];   /* byte1 and byte2 value of the UTF-8 char */
  int             *ranges;    /* sub range of iso10646 */

  nb_font = font_set->nb_font;

  if (nb_font < 1) {
    /* there is no font in the font_set :-( */
    return -1;
  }

  ranges = font_set->ranges;
  fonts = font_set->fonts;
  encodings = font_set->encodings;
  fnum = 0;

  while(fnum < nb_font && !fonts[fnum]) fnum++;
  if (fnum >= nb_font) {
    /* there is no valid font for the X server */
    return -1;
  }

  first = fnum;

  /*
   * find the first encoding which can be used to draw the glyph
   */
  fnum = first;
  while (fnum < nb_font) {
    if (fonts[fnum] && ucs2fontmap(glyph, ucs, encodings[fnum]) >= 0) {
      if (encodings[fnum] != 0 ||
          ((int)ucs >= ranges[fnum * 2] &&
           (int)ucs <= ranges[fnum * 2 + 1])) {
        break;
      }
    }
    fnum++;
  }
  if (fnum == nb_font) {
    /* the char is not valid in all encodings ->
     * draw it using the first font :-(
     */
    fnum = first;
    ucs2fontmap(glyph, '?', encodings[fnum]);
  }

  *id = ((unsigned char)glyph[0] << 8) | (unsigned char)glyph[1] ;
  *fnt = fonts[fnum];
  return 0;
}

/*****************************************************************************/
/** returns the pixel width of a UCS char                                   **/
/*****************************************************************************/
int
XUtf8UcsWidth(XUtf8FontStruct  *font_set,
              unsigned int     ucs) {

  int           x;
  int           *encodings; /* encodings array */
  XFontStruct   **fonts;    /* fonts array */
  Fl_XChar2b    buf[8];     /* drawing buffer */
  int           fnum;       /* index of the current font in the fonts array*/
  int           first;      /* first valid font index */
  int           nb_font;    /* quantity of fonts in the font array */
  char          glyph[2];   /* byte1 and byte2 value of the UTF-8 char */
  int           *ranges;    /* sub range of iso10646 */
  unsigned int  no_spc;

  nb_font = font_set->nb_font;
  x = 0;

  if (nb_font < 1) {
    /* there is no font in the font_set :-( */
    return x;
  }

  ranges = font_set->ranges;
  fonts = font_set->fonts;
  encodings = font_set->encodings;
  fnum = 0;

  while(fnum < nb_font && !fonts[fnum]) fnum++;
  if (fnum >= nb_font) {
    /* there is no valid font for the X server */
    return x;
  }

  first = fnum;

  no_spc = XUtf8IsNonSpacing(ucs);
  if (no_spc) ucs = no_spc;

  /*
   * find the first encoding which can be used to
   * draw the glyph
   */
  fnum = first;
  while (fnum < nb_font) {
    if (fonts[fnum] &&
        ucs2fontmap(glyph, ucs, encodings[fnum]) >= 0) {
      if (encodings[fnum] != 0 || ((int)ucs >= ranges[fnum * 2] &&
          (int)ucs <= ranges[fnum * 2 + 1])) {
        break;
      }
    }
    fnum++;
  }
  if (fnum == nb_font) {
    /* the char is not valid in all encodings ->
     * draw it using the first font :-(
     */
    fnum = first;
    ucs2fontmap(glyph, '?', encodings[fnum]);
  }

  (*buf).byte1 = glyph[0];
  (*buf).byte2 = glyph[1];

  x += XTextWidth16(fonts[fnum], buf, 1);

  return x;
}

/*****************************************************************************/
/** draw an UTF-8 string and clear the background.                          **/
/*****************************************************************************/
void
XUtf8DrawImageString(Display         *display,
                     Drawable        d,
                     XUtf8FontStruct *font_set,
                     GC              gc,
                     int             x,
                     int             y,
                     const char      *string,
                     int             num_bytes) {

  /* FIXME: must be improved ! */
  int w;
  int fill_style;
  unsigned long foreground;
  unsigned long background;
  int function;
  XGCValues xgcv;

  w = XUtf8TextWidth(font_set, string, num_bytes);

  XGetGCValues(display, gc,
               GCFunction|GCForeground|GCBackground|GCFillStyle, &xgcv);

  function = xgcv.function;
  fill_style = xgcv.fill_style;
  foreground = xgcv.foreground;
  background = xgcv.background;

  xgcv.function = GXcopy;
  xgcv.foreground = background;
  xgcv.background = foreground;
  xgcv.fill_style = FillSolid;

  XChangeGC(display, gc,
            GCFunction|GCForeground|GCBackground|GCFillStyle, &xgcv);

  XFillRectangle(display, d, gc, x, y - font_set->ascent,
                 (unsigned)w, (unsigned)(font_set->ascent + font_set->descent));

  xgcv.function = function;
  xgcv.foreground = foreground;
  xgcv.background = background;
  xgcv.fill_style = fill_style;

  XChangeGC(display, gc,
            GCFunction|GCForeground|GCBackground|GCFillStyle, &xgcv);

  XUtf8DrawString(display, d, font_set, gc, x, y, string, num_bytes);
}

/*****************************************************************************/
/** free the XFontSet and others things created by XCreateUtf8FontSet       **/
/*****************************************************************************/
void
XFreeUtf8FontStruct(Display         *dpy,
                    XUtf8FontStruct *font_set) {

  int i;
  i = 0;
  while (i < font_set->nb_font) {
    if (font_set->fonts[i]) {
        XFreeFont(dpy, font_set->fonts[i]);
        free(font_set->font_name_list[i]);
    }
    i++;
  }
  free(font_set->ranges);
  free(font_set->font_name_list);
  free(font_set->fonts);
  free(font_set->encodings);
  free(font_set);
}
