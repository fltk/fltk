//
// Label drawing code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

// Implementation of fl_draw(const char*,int,int,int,int,Fl_Align)
// Used to draw all the labels and text, this routine:
// Word wraps the labels to fit into their bounding box.
// Breaks them into lines at the newlines.
// Expands all unprintable characters to ^X or \nnn notation
// Aligns them against the inside of the box.

#include <FL/fl_utf8.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Image.H>

#include "flstring.h"
#include <ctype.h>
#include <math.h>


char fl_draw_shortcut;	// set by fl_labeltypes.cxx

static char* underline_at;

/* If called with maxbuf==0, use an internally allocated buffer and enlarge it as needed.
 Otherwise, use buf as buffer but don't go beyond its length of maxbuf.
 */
static const char* expand_text_(const char* from, char*& buf, int maxbuf, double maxw, int& n,
	       double &width, int wrap, int draw_symbols) {
  char* e = buf+(maxbuf-4);
  underline_at = 0;
  double w = 0;
  static int l_local_buff = 500;
  static char *local_buf = (char*)malloc(l_local_buff); // initial buffer allocation
  if (maxbuf == 0) {
    buf = local_buf;
    e = buf + l_local_buff - 4;
    }
  char* o = buf;
  char* word_end = o;
  const char* word_start = from;

  const char* p = from;
  for (;; p++) {

    int c = *p & 255;

    if (!c || c == ' ' || c == '\n') {
      // test for word-wrap:
      if (word_start < p && wrap) {
	double newwidth = w + fl_width(word_end, (int) (o-word_end) );
	if (word_end > buf && int(newwidth) > maxw) { // break before this word
	  o = word_end;
	  p = word_start;
	  break;
	}
	word_end = o;
	w = newwidth;
      }
      if (!c) break;
      else if (c == '\n') {p++; break;}
      word_start = p+1;
    }

    if (o > e) {
      if (maxbuf) break; // don't overflow buffer
      l_local_buff += int(o - e) + 200; // enlarge buffer
      size_t delta_o = (o - local_buf);
      size_t delta_end = (word_end - local_buf);
      local_buf = (char*)realloc(local_buf, l_local_buff);
      buf = local_buf;
      e = local_buf + l_local_buff - 4; // update pointers to buffer content
      o = local_buf + delta_o;
      word_end = local_buf + delta_end;
    }

    if (c == '\t') {
      for (c = fl_utf_nb_char((uchar*)buf, (int) (o-buf) )%8; c<8 && o<e; c++)
           *o++ = ' ';
    } else if (c == '&' && fl_draw_shortcut && *(p+1)) {
      if (*(p+1) == '&') {p++; *o++ = '&';}
      else if (fl_draw_shortcut != 2) underline_at = o;
    } else if (c < ' ' || c == 127) { // ^X
      *o++ = '^';
      *o++ = c ^ 0x40;
    } else if (c == '@' && draw_symbols) { // Symbol???
      if (p[1] && p[1] != '@')  break;
      *o++ = c;
      if (p[1]) p++;
    } else {
      *o++ = c;
    }
  }

  width = w + fl_width(word_end, (int) (o-word_end));
  *o = 0;
  n = (int) (o-buf);
  return p;
}

/**
 Copy \p from to \p buf, replacing control characters with ^X.

 Stop at a newline or if \p maxbuf characters written to buffer.
 Also word-wrap if width exceeds maxw.
 Returns a pointer to the start of the next line of characters.
 Sets n to the number of characters put into the buffer.
 Sets width to the width of the string in the \ref drawing_fl_font "current font".
 */
const char*
fl_expand_text(const char* from, char* buf, int maxbuf, double maxw, int& n,
	       double &width, int wrap, int draw_symbols) {
  return expand_text_(from,  buf, maxbuf, maxw,  n, width,  wrap,  draw_symbols);
}

/**
  The same as fl_draw(const char*,int,int,int,int,Fl_Align,Fl_Image*,int) with
  the addition of the \p callthis parameter, which is a pointer to a text drawing
  function such as fl_draw(const char*, int, int, int) to do the real work
*/
void fl_draw(
    const char* str,	// the (multi-line) string
    int x, int y, int w, int h,	// bounding box
    Fl_Align align,
    void (*callthis)(const char*,int,int,int),
    Fl_Image* img, int draw_symbols)
{
  char *linebuf = NULL;
  const char* p;
  const char* e;
  int buflen;
  char symbol[2][255], *symptr;
  int symwidth[2], symoffset, symtotal, imgtotal;

  // count how many lines and put the last one into the buffer:
  int lines;
  double width;

  // if the image is set as a backdrop, ignore it here
  if (img && (align & FL_ALIGN_IMAGE_BACKDROP)) img = 0;

  symbol[0][0] = '\0';
  symwidth[0]  = 0;

  symbol[1][0] = '\0';
  symwidth[1]  = 0;

  if (draw_symbols) {
    if (str && str[0] == '@' && str[1] && str[1] != '@') {
      // Start with a symbol...
      for (symptr = symbol[0];
           *str && !isspace(*str) && symptr < (symbol[0] + sizeof(symbol[0]) - 1);
           *symptr++ = *str++) {/*empty*/}
      *symptr = '\0';
      if (isspace(*str)) str++;
      symwidth[0] = (w < h ? w : h);
    }

    if (str && (p = strrchr(str, '@')) != NULL && p > (str + 1) && p[-1] != '@') {
      strlcpy(symbol[1], p, sizeof(symbol[1]));
      symwidth[1] = (w < h ? w : h);
    }
  }

  symtotal = symwidth[0] + symwidth[1];
  imgtotal = (img && (align&FL_ALIGN_IMAGE_NEXT_TO_TEXT)) ? img->w() : 0;

  int strw = 0;
  int strh;

  if (str) {
    for (p = str, lines=0; p;) {
      e = expand_text_(p, linebuf, 0, w - symtotal - imgtotal, buflen, width,
                         align&FL_ALIGN_WRAP, draw_symbols);
      if (strw<width) strw = (int)width;
      lines++;
      if (!*e || (*e == '@' && e[1] != '@' && draw_symbols)) break;
      p = e;
    }
  } else lines = 0;

  if ((symwidth[0] || symwidth[1]) && lines) {
    if (symwidth[0]) symwidth[0] = lines * fl_height();
    if (symwidth[1]) symwidth[1] = lines * fl_height();
  }

  symtotal = symwidth[0] + symwidth[1];
  strh = lines * fl_height();

  // figure out vertical position of the first line:
  int xpos;
  int ypos;
  int height = fl_height();
  int imgvert = ((align&FL_ALIGN_IMAGE_NEXT_TO_TEXT)==0);
  int imgh = img && imgvert ? img->h() : 0;
  int imgw[2] = {0, 0};

  symoffset = 0;

  if (align & FL_ALIGN_BOTTOM) ypos = y+h-(lines-1)*height-imgh;
  else if (align & FL_ALIGN_TOP) ypos = y+height;
  else ypos = y+(h-lines*height-imgh)/2+height;

  // draw the image unless the "text over image" alignment flag is set...
  if (img && imgvert && !(align & FL_ALIGN_TEXT_OVER_IMAGE)) {
    if (img->w() > symoffset) symoffset = img->w();

    if (align & FL_ALIGN_LEFT) xpos = x + symwidth[0];
    else if (align & FL_ALIGN_RIGHT) xpos = x + w - img->w() - symwidth[1];
    else xpos = x + (w - img->w() - symtotal) / 2 + symwidth[0];

    img->draw(xpos, ypos - height);
    ypos += img->h();
  }

  // draw the image to the side of the text
  if (img && !imgvert /* && (align & !FL_ALIGN_TEXT_NEXT_TO_IMAGE)*/ ) {
    if (align & FL_ALIGN_TEXT_OVER_IMAGE) { // image is right of text
      imgw[1] = img->w();
      if (align & FL_ALIGN_LEFT) xpos = x + symwidth[0] + strw + 1;
      else if (align & FL_ALIGN_RIGHT) xpos = x + w - symwidth[1] - imgw[1] + 1;
      else xpos = x + (w - strw - symtotal - imgw[1]) / 2 + symwidth[0] + strw + 1;
    } else { // image is to the left of the text
      imgw[0] = img->w();
      if (align & FL_ALIGN_LEFT) xpos = x + symwidth[0] - 1;
      else if (align & FL_ALIGN_RIGHT) xpos = x + w - symwidth[1] - strw - imgw[0] - 1;
      else xpos = x + (w - strw - symtotal - imgw[0]) / 2 - 1;
    }
    int yimg = ypos - height;
    if (align & FL_ALIGN_TOP) ;
    else if (align & FL_ALIGN_BOTTOM) yimg += strh - img->h() - 1;
    else yimg += (strh - img->h() - 1) / 2;
    img->draw(xpos, yimg);
  }

  // now draw all the lines:
  if (str) {
    int desc = fl_descent();
    for (p=str; ; ypos += height) {
      if (lines>1)
        e = expand_text_(p, linebuf, 0, w - symtotal - imgtotal, buflen,
			 width, align&FL_ALIGN_WRAP, draw_symbols);
      else e = "";

      if (width > symoffset) symoffset = (int)(width + 0.5);

      if (align & FL_ALIGN_LEFT) xpos = x + symwidth[0] + imgw[0];
      else if (align & FL_ALIGN_RIGHT) xpos = x + w - (int)(width + .5) - symwidth[1] - imgw[1];
      else xpos = x + (w - (int)(width + .5) - symtotal - imgw[0] - imgw[1]) / 2 + symwidth[0] + imgw[0];

      callthis(linebuf,buflen,xpos,ypos-desc);

      if (underline_at && underline_at >= linebuf && underline_at < (linebuf + buflen))
	callthis("_",1,xpos+int(fl_width(linebuf,(int) (underline_at-linebuf))),ypos-desc);

      if (!*e || (*e == '@' && e[1] != '@')) break;
      p = e;
    }
  }

  // draw the image if the "text over image" alignment flag is set...
  if (img && imgvert && (align & FL_ALIGN_TEXT_OVER_IMAGE)) {
    if (img->w() > symoffset) symoffset = img->w();

    if (align & FL_ALIGN_LEFT) xpos = x + symwidth[0];
    else if (align & FL_ALIGN_RIGHT) xpos = x + w - img->w() - symwidth[1];
    else xpos = x + (w - img->w() - symtotal) / 2 + symwidth[0];

    img->draw(xpos, ypos);
  }

  // draw the symbols, if any...
  if (symwidth[0]) {
    // draw to the left
    if (align & FL_ALIGN_LEFT) xpos = x;
    else if (align & FL_ALIGN_RIGHT) xpos = x + w - symtotal - symoffset;
    else xpos = x + (w - symoffset - symtotal) / 2;

    if (align & FL_ALIGN_BOTTOM) ypos = y + h - symwidth[0];
    else if (align & FL_ALIGN_TOP) ypos = y;
    else ypos = y + (h - symwidth[0]) / 2;

    fl_draw_symbol(symbol[0], xpos, ypos, symwidth[0], symwidth[0], fl_color());
  }

  if (symwidth[1]) {
    // draw to the right
    if (align & FL_ALIGN_LEFT) xpos = x + symoffset + symwidth[0];
    else if (align & FL_ALIGN_RIGHT) xpos = x + w - symwidth[1];
    else xpos = x + (w - symoffset - symtotal) / 2 + symoffset + symwidth[0];

    if (align & FL_ALIGN_BOTTOM) ypos = y + h - symwidth[1];
    else if (align & FL_ALIGN_TOP) ypos = y;
    else ypos = y + (h - symwidth[1]) / 2;

    fl_draw_symbol(symbol[1], xpos, ypos, symwidth[1], symwidth[1], fl_color());
  }
}

/**
  Fancy string drawing function which is used to draw all the labels.

  The string is formatted and aligned inside the passed box.
  Handles '\\t' and '\\n', expands all other control characters to '^X',
  and aligns inside or against the edges of the box.
  See Fl_Widget::align() for values of \p align. The value FL_ALIGN_INSIDE
  is ignored, as this function always prints inside the box.
  If \p img is provided and is not \p NULL, the image is drawn above or
  below the text as specified by the \p align value.
  The \p draw_symbols argument specifies whether or not to look for symbol
  names starting with the '\@' character'
*/
void fl_draw(
  const char* str,
  int x, int y, int w, int h,
  Fl_Align align,
  Fl_Image* img,
  int draw_symbols)
{
  if ((!str || !*str) && !img) return;
  if (w && h && !fl_not_clipped(x, y, w, h) && (align & FL_ALIGN_INSIDE)) return;
  if (align & FL_ALIGN_CLIP)
    fl_push_clip(x, y, w, h);
  fl_draw(str, x, y, w, h, align, fl_draw, img, draw_symbols);
  if (align & FL_ALIGN_CLIP)
    fl_pop_clip();
}

/**
  Measure how wide and tall the string will be when printed by the
  fl_draw() function with \p align parameter. If the incoming \p w
  is non-zero it will wrap to that width.

  The \ref drawing_fl_font "current font" is used to do the width/height
  calculations, so unless its value is known at the time fl_measure() is
  called, it is advised to first set the current font with fl_font().
  With event-driven GUI programming you can never be sure which
  widget was exposed and redrawn last, nor which font it used.
  If you have not called fl_font() explicitly in your own code,
  the width and height may be set to unexpected values, even zero!

  \b Note: In the general use case, it's a common error to forget to set
  \p w to 0 before calling fl_measure() when wrap behavior isn't needed.

  \param[in] str nul-terminated string
  \param[out] w,h width and height of string in current font
  \param[in] draw_symbols non-zero to enable @@symbol handling [default=1]

  \code
  // Example: Common use case for fl_measure()
  const char *s = "This is a test";
  int wi=0, hi=0;              // initialize to zero before calling fl_measure()
  fl_font(FL_HELVETICA, 14);   // set current font face/size to be used for measuring
  fl_measure(s, wi, hi);       // returns pixel width/height of string in current font
  \endcode
*/
void fl_measure(const char* str, int& w, int& h, int draw_symbols) {
  if (!str || !*str) {w = 0; h = 0; return;}
  h = fl_height();
  char *linebuf = NULL;
  const char* p;
  const char* e;
  int buflen;
  int lines;
  double width=0;
  int W = 0;
  int symwidth[2], symtotal;

  symwidth[0] = 0;	// size of symbol at beginning of string (if any)
  symwidth[1] = 0;	// size of symbol at end of string (if any)

  if (draw_symbols) {
    // Symbol at beginning of string?
    const char *sym2 = (str[0]=='@' && str[1]=='@') ? str+2 : str;	// sym2 check will skip leading @@
    if (str[0] == '@' && str[1] != '@') {
      while (*str && !isspace(*str)) { ++str; }		// skip over symbol
      if (isspace(*str)) ++str;				// skip over trailing space
      sym2 = str;					// sym2 check will skip leading symbol
      symwidth[0] = h;
    }
    // Symbol at end of string?
    if ((p=strchr(sym2,'@')) != NULL && p[1] != '@') {
      symwidth[1] = h;
    }
  }

  symtotal = symwidth[0] + symwidth[1];

  for (p = str, lines=0; p;) {
//    e = expand(p, linebuf, w - symtotal, buflen, width, w != 0, draw_symbols);
    e = expand_text_(p, linebuf, 0, w - symtotal, buflen, width,
			w != 0, draw_symbols);
    if ((int)ceil(width) > W) W = (int)ceil(width);
    lines++;
    if (!*e || (*e == '@' && e[1] != '@' && draw_symbols)) break;
    p = e;
  }

  if ((symwidth[0] || symwidth[1]) && lines) {
    if (symwidth[0]) symwidth[0] = lines * fl_height();
    if (symwidth[1]) symwidth[1] = lines * fl_height();
  }

  symtotal = symwidth[0] + symwidth[1];

  w = W + symtotal;
  h = lines*h;
}

/**
  This function returns the actual height of the specified \p font
  and \p size. Normally the font height should always be 'size',
  but with the advent of XFT, there are (currently) complexities
  that seem to only be solved by asking the font what its actual
  font height is. (See STR#2115)

  This function was originally undocumented in 1.1.x, and was used
  only by Fl_Text_Display. We're now documenting it in 1.3.x so that
  apps that need precise height info can get it with this function.

  \returns the height of the font in pixels.

  \todo  In the future, when the XFT issues are resolved, this function
         should simply return the 'size' value.
*/
int fl_height(int font, int size) {
    if ( font == fl_font() && size == fl_size() ) return(fl_height());
    int tf = fl_font(), ts = fl_size();   // save
    fl_font(font,size);
    int height = fl_height();
    fl_font(tf,ts);                       // restore
    return(height);
}
