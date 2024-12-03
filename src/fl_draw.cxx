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
#include <FL/platform.H>        // fl_open_display()

#include "flstring.h"
#include "fl_oxy.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>

char fl_draw_shortcut;  // set by fl_labeltypes.cxx

static char* underline_at;

/*
 Extract the part of text that fits into the given maximum width.

 If called with maxbuf==0, use an internally allocated buffer and enlarge it as needed.
 Otherwise, use buf as buffer but don't go beyond its length of maxbuf.

 \param[in] from input text, can contain '\n' and single or double '\@' characters
 \param[out] buf buffer for the output text segment
 \param[in] maxbuf size of the buffer, or 0 to use the internal buffer allocated in this function
 \param[in] maxw maximum width in pixels of the output text
 \param[out] n number of characters in the output text segment
 \param[out] width actual width in pixels of the output text
 \param[in] wrap if true, wrap at maxw, else wrap at newlines
 \param[in] draw_symbols if true, double '\@' characters are escaped into a single '@'

 \return pointer to the next character in the input text
 */
static const char* expand_text_(const char* from, char*& buf, int maxbuf, double maxw, int& n,
               double &width, int wrap, int draw_symbols) {
  // Reset underline_at to null
  underline_at = NULL;

  // Initialize the total width to 0
  double w = 0;

  // Check if the caller wants to use the internal buffer
  static int l_local_buff = 500;
  static char *local_buf = (char*)malloc(l_local_buff); // initial buffer allocation

  // Calculate the end pointer of the buffer
  char* e;
  if (maxbuf == 0) {
    buf = local_buf;
    e = buf + l_local_buff - 4;
  } else {
    e = buf+(maxbuf-4);
  }

  // Initialize the output pointer to the buffer
  char* o = buf;

  // Initialize the word end pointer that points into the `out` buffer
  char* word_end = o;

  // Initialize the word start pointer that points into the `from` buffer
  const char* word_start = from;

  // Iterate over the input text
  const char* p = from;
  for (;; p++) {
    int c = *p & 255;

    // Check for end of line, space, or '\n'
    if (!c || c == ' ' || c == '\n') {
      // Check for word wrap
      if (word_start < p && wrap) {
        // Calculate the new width
        double newwidth = w + fl_width(word_end, (int) (o-word_end) );

        // Check if the new width exceeds the maximum width
        if (word_end > buf && int(newwidth) > maxw) { // break before this word
          o = word_end;
          p = word_start;
          break;
        }
        // Update the word end pointer
        word_end = o;
        w = newwidth;
      }

      // Check for end of line
      if (!c) break;
      else if (c == '\n') {p++; break;}

      // Update the word start pointer
      word_start = p+1;
    }

    // Check if the buffer needs to be enlarged
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

    // Process the character based on its type
    if (c == '\t') {
      // Process tab character
      for (c = fl_utf_nb_char((uchar*)buf, (int) (o-buf) )%8; c<8 && o<e; c++)
           *o++ = ' ';
    } else if (c == '&' && fl_draw_shortcut && *(p+1)) {
      // Process '&' character
      if (*(p+1) == '&') {p++; *o++ = '&';}
      else if (fl_draw_shortcut != 2) underline_at = o;
    } else if (c < ' ' || c == 127) { // ^X
      // Process control characters
      *o++ = '^';
      *o++ = c ^ 0x40;
    } else if (c == '@' && draw_symbols) { // Symbol???
      // Process '@' character
      if (p[1] && p[1] != '@')  break;
      *o++ = c;
      if (p[1]) p++;
    } else {
      // Process regular characters
      *o++ = c;
    }
  }

  // Calculate the final width
  width = w + fl_width(word_end, (int) (o-word_end));

  // Add the null terminator and set the number of characters
  *o = 0;
  n = (int)(o-buf);

  return p;
}

/**
 Copy \p from to \p buf, replacing control characters with ^X.

 Stop at a newline or if \p maxbuf characters written to buffer.
 Also word-wrap if width exceeds maxw.
 Returns a pointer to the start of the next line of characters.
 Sets n to the number of characters put into the buffer.
 Sets width to the width of the string in the \ref drawing_fl_font "current font".

 \param[in] from input text, can contain '\n' and single or double '\@' characters
 \param[out] buf buffer for the output text segment
 \param[in] maxbuf size of the buffer, or 0 to use the internal buffer allocated in this function
 \param[in] maxw maximum width in pixels of the output text
 \param[out] n number of characters in the output text segment
 \param[out] width actual width in pixels of the output text
 \param[in] wrap if true, wrap at maxw, else wrap at newlines
 \param[in] draw_symbols if true, double '\@' characters are escaped into a single '@'

 \return pointer to the next character in the input text
 */
const char*
fl_expand_text(const char* from, char* buf, int maxbuf, double maxw, int& n,
               double &width, int wrap, int draw_symbols) {
  return expand_text_(from,  buf, maxbuf, maxw,  n, width,  wrap,  draw_symbols);
}

// Caution: put the documentation next to the function's declaration in fl_draw.H for Doxygen
// to see default argument values.
void fl_draw(
    const char* str,    // the (multi-line) string
    int x, int y, int w, int h, // bounding box
    Fl_Align align,
    void (*callthis)(const char*,int,int,int),
    Fl_Image* img, int draw_symbols, int spacing)
{
  char *linebuf = NULL;       // Pointer to a buffer managed by expand_text_
  const char* p;              // Scratch pointer into text, multiple use
  const char* e;              // Scratch pointer into text, multiple use
  int buflen;                 // Number of bytes copied into linebuf
                              // by expand_text_
  char symbol[2][255];        // Copy of symbol text at start and end of str
  int symwidth[2];            // Width and height of symbols (always square)
  int symoffset;
  int symtotal;               // Combined width of both sybols
  int imgtotal;               // Width of image if image is to the left or right
  int imgvert = ((align&FL_ALIGN_IMAGE_NEXT_TO_TEXT)==0); // True if image is
                              // above or below text
  int lines;                  // Number of text lines including '\n' and wrapping
  double width;               // width of the longest text line
  int height = fl_height();   // Height of a line of text


  // If the image is set as a backdrop, ignore it in this function
  if (img && (align & FL_ALIGN_IMAGE_BACKDROP)) img = 0;

  symbol[0][0] = '\0';
  symwidth[0]  = 0;
  symbol[1][0] = '\0';
  symwidth[1]  = 0;

  // Find the symbol at the start and end of the string
  if (draw_symbols) {
    if (str && str[0] == '@' && str[1] && str[1] != '@') {
      char *symptr;
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

  // Width and height of both symbols combined
  symtotal = symwidth[0] + symwidth[1];
  // Image width if image is to the left or right of the text, else 0
  imgtotal = (img && (align&FL_ALIGN_IMAGE_NEXT_TO_TEXT)) ? img->w() + spacing : 0;

  int strw = 0;               // Width of text only without symbols
  int strh;                   // Height of text only without symbols

  // Count how many lines and put the last one into the buffer:
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

  // Fix the size of the symbols if there is at least one line of text to print
  if (lines) {
    if (symwidth[0]) symwidth[0] = lines * height;
    if (symwidth[1]) symwidth[1] = lines * height;
  }

  // Width and height of both symbols combined
  symtotal = symwidth[0] + symwidth[1];
  // Height of text only
  strh = lines * fl_height();

  // Figure out vertical position of the first element
  int xpos;                   // Position of image or text
  int ypos;                   // Position of image or text
  int imgh = img && imgvert ? img->h() + spacing : 0; // Height of image if image is above or below text
  int imgw[2] = {0, 0};       // Width of image on the left and right side of the text

  symoffset = 0;

  // Figure out vertical position of the first line of text or top image
  if (align & FL_ALIGN_BOTTOM) {
    ypos = y+h-(lines-1)*height-imgh;
  } else if (align & FL_ALIGN_TOP) {
    ypos = y+height;
  } else {
    ypos = y+(h-lines*height-imgh)/2+height;
  }

  // Draw the image if located *above* the text
  if (img && imgvert && !(align & FL_ALIGN_TEXT_OVER_IMAGE)) {
    if (img->w() > symoffset) symoffset = img->w();

    if (align & FL_ALIGN_LEFT) {
      xpos = x + symwidth[0];
    } else if (align & FL_ALIGN_RIGHT) {
      xpos = x + w - img->w() - symwidth[1];
    } else {
      xpos = x + (w - img->w() - symtotal) / 2 + symwidth[0];
    }

    img->draw(xpos, ypos - height);
    ypos += img->h() + spacing;
  }

  // Draw the image if either on the *left* or *right* side of the text
  if (img && !imgvert) {
    if (align & FL_ALIGN_TEXT_OVER_IMAGE) {
      // Image is to the right of the text
      imgw[1] = img->w() + spacing;
      // Find the horizontal position of the image
      if (align & FL_ALIGN_LEFT) {
        xpos = x + symwidth[0] + strw + 1;
      } else if (align & FL_ALIGN_RIGHT) {
        xpos = x + w - symwidth[1] - imgw[1] + 1;
      } else {
        xpos = x + (w - strw - symtotal - imgw[1]) / 2 + symwidth[0] + strw + 1;
      }
      xpos += spacing;
    } else {
      // Image is to the left of the text
      imgw[0] = img->w() + spacing;
      // Find the horizontal position of the image
      if (align & FL_ALIGN_LEFT) {
        xpos = x + symwidth[0] - 1;
      } else if (align & FL_ALIGN_RIGHT) {
        xpos = x + w - symwidth[1] - strw - imgw[0] - 1;
      } else {
        xpos = x + (w - strw - symtotal - imgw[0]) / 2 - 1;
      }
    }
    // Find the vertical position of the image
    int yimg;
    if (align & FL_ALIGN_TOP) {
      yimg = ypos - height;
    } else if (align & FL_ALIGN_BOTTOM) {
      yimg = ypos - height + strh - img->h() - 1;
    } else {
      yimg = ypos - height + (strh - img->h() - 1) / 2;
    }
    // Draw the image
    img->draw(xpos, yimg);
  }

  // Now draw all the text lines
  if (str) {
    int desc = fl_descent();
    for (p=str; ; ypos += height) {
      if (lines>1) {
        e = expand_text_(p, linebuf, 0, w - symtotal - imgtotal, buflen,
                         width, align&FL_ALIGN_WRAP, draw_symbols);
      } else {
        e = "";
      }

      if (width > symoffset) symoffset = (int)(width + 0.5);

      if (align & FL_ALIGN_LEFT) {
        xpos = x + symwidth[0] + imgw[0];
      } else if (align & FL_ALIGN_RIGHT) {
        xpos = x + w - (int)(width + .5) - symwidth[1] - imgw[1];
      } else {
        xpos = x + (w - (int)(width + .5) - symtotal - imgw[0] - imgw[1]) / 2 + symwidth[0] + imgw[0];
      }

      callthis(linebuf,buflen,xpos,ypos-desc);

      if (underline_at && underline_at >= linebuf && underline_at < (linebuf + buflen))
        callthis("_",1,xpos+int(fl_width(linebuf,(int) (underline_at-linebuf))),ypos-desc);

      if (!*e || (*e == '@' && e[1] != '@')) break;
      p = e;
    }
  }

  // Draw the image if the image is *below* the text
  if (img && imgvert && (align & FL_ALIGN_TEXT_OVER_IMAGE)) {
    if (img->w() > symoffset) symoffset = img->w();

    if (align & FL_ALIGN_LEFT) {
      xpos = x + symwidth[0];
    } else if (align & FL_ALIGN_RIGHT) {
      xpos = x + w - img->w() - symwidth[1];
    } else {
      xpos = x + (w - img->w() - symtotal) / 2 + symwidth[0];
    }

    img->draw(xpos, ypos + spacing);
  }

  // Draw the symbols, if any...
  if (symwidth[0]) {
    // Draw the leading symbol to the left of the text
    if (align & FL_ALIGN_LEFT) {
      xpos = x;
    } else if (align & FL_ALIGN_RIGHT) {
      xpos = x + w - symtotal - symoffset;
    } else {
      xpos = x + (w - symoffset - symtotal) / 2;
    }

    if (align & FL_ALIGN_BOTTOM) {
      ypos = y + h - symwidth[0];
    } else if (align & FL_ALIGN_TOP) {
      ypos = y;
    } else {
      ypos = y + (h - symwidth[0]) / 2;
    }

    fl_draw_symbol(symbol[0], xpos, ypos, symwidth[0], symwidth[0], fl_color());
  }

  if (symwidth[1]) {
    // Draw the trailing symbol to the right of the text
    if (align & FL_ALIGN_LEFT) {
      xpos = x + symoffset + symwidth[0];
    } else if (align & FL_ALIGN_RIGHT) {
      xpos = x + w - symwidth[1];
    } else {
      xpos = x + (w - symoffset - symtotal) / 2 + symoffset + symwidth[0];
    }

    if (align & FL_ALIGN_BOTTOM) {
      ypos = y + h - symwidth[1];
    } else if (align & FL_ALIGN_TOP) {
      ypos = y;
    } else {
      ypos = y + (h - symwidth[1]) / 2;
    }

    fl_draw_symbol(symbol[1], xpos, ypos, symwidth[1], symwidth[1], fl_color());
  }
}

// Caution: put the documentation next to the function's declaration in fl_draw.H for Doxygen
// to see default argument values.
void fl_draw(
  const char* str,
  int x, int y, int w, int h,
  Fl_Align align,
  Fl_Image* img,
  int draw_symbols,
  int spacing)
{
  if ((!str || !*str) && !img) return;
  if (w && h && !fl_not_clipped(x, y, w, h) && (align & FL_ALIGN_INSIDE)) return;
  if (align & FL_ALIGN_CLIP)
    fl_push_clip(x, y, w, h);
  fl_draw(str, x, y, w, h, align, fl_draw, img, draw_symbols, spacing);
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
  \param[in,out] w call with w=0, or with the prefered width for word wrapping,
        returns with the width of the string in current font
  \param[out] h height of string in current font
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

  symwidth[0] = 0;      // size of symbol at beginning of string (if any)
  symwidth[1] = 0;      // size of symbol at end of string (if any)

  if (draw_symbols) {
    // Symbol at beginning of string?
    const char *sym2 = (str[0]=='@' && str[1]=='@') ? str+2 : str;      // sym2 check will skip leading @@
    if (str[0] == '@' && str[1] != '@') {
      while (*str && !isspace(*str)) { ++str; }         // skip over symbol
      if (isspace(*str)) ++str;                         // skip over trailing space
      sym2 = str;                                       // sym2 check will skip leading symbol
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
  Sets the current font, which is then used in various drawing routines.
  You may call this outside a draw context if necessary to measure text,
  for instance by calling fl_width(), fl_measure(), or fl_text_extents(),
  but on X this will open the display.

  The font is identified by a \p face and a \p size.
  The size of the font is measured in pixels and not "points".
  Lines should be spaced \p size pixels apart or more.
*/
void fl_font(Fl_Font face, Fl_Fontsize fsize) {
  if (!fl_graphics_driver)
    fl_open_display();
  fl_graphics_driver->font(face, fsize);
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

/** Removes any GUI scaling factor in subsequent drawing operations.
 This must be matched by a later call to fl_restore_scale().
 This function can be used to transiently perform drawing operations
 that are not rescaled by the current value of the GUI scaling factor.
 It's not supported to call fl_push_clip() while transiently removing scaling.
 \return The GUI scaling factor value that was in place when the function started.
 */
float fl_override_scale() {
  return fl_graphics_driver->override_scale();
}

/** Restores the GUI scaling factor in subsequent drawing operations.
 \param s Value returned by a previous call to fl_override_scale(). */
void fl_restore_scale(float s) {
  fl_graphics_driver->restore_scale(s);
}

/**
  Draw a check mark inside the given bounding box.

  The check mark is allowed to fill the entire box but the algorithm used
  makes sure that a 1-pixel border is kept free if the box is large enough.
  You need to calculate margins for box borders etc. yourself.

  The check mark size is limited (minimum and maximum size) and the check
  mark is always centered in the given box.

  \note If the box is too small (bad GUI design) the check mark will be drawn
    over the box borders. This is intentional for better user experience.
    Otherwise users might not be able to recognize if a box is checked.

  The size limits are implementation details and may be changed at any time.

  \param[in]  bb   rectangle that defines the bounding box
  \param[in]  col  Fl_Color to draw the check mark

  \since 1.4.0
*/

void fl_draw_check(Fl_Rect bb, Fl_Color col) {

  const int md = 6; // max. d1 value: 3 * md + 1 pixels wide
  int tx = bb.x();
  int ty = bb.y();
  int tw = bb.w();
  int th = bb.h();
  int lh = 3;       // line height 3 means 4 pixels
  int d1, d2;

  Fl_Color saved_color = fl_color();

  // make sure there's a free 1-pixel border if the area is large enough
  if (tw > 10) {
    tx++;
    tw -= 2;
  }
  if (th > 10) {
    ty++;
    th -= 2;
  }
  // calculate d1, the width and height of the left part of the check mark
  d1 = tw / 3;
  d2 = 2 * d1;
  if (d1 > md) {
    d1 = md;
    d2 = 2 * d1;
  }
  // make sure the height fits
  if (d2 + lh + 1 > th) {
    d2 = th - lh - 1;
    d1 = (d2+1) / 2;
  }
  // check minimal size (box too small)
  if (d1 < 2) {
    d1 = 2;
    d2 = 4;
  }
  // reduce line height (width) for small sizes
  if (d1 < 3)
    lh = 2;

  tw = d1 + d2 + 1; // total width
  th = d2 + lh + 1; // total height

  tx = bb.x() + (bb.w() - tw + 1) / 2;  // x position (centered)
  ty = bb.y() + (bb.h() - th + 1) / 2;  // y position (centered)

  // Set DEBUG_FRAME to 1 - 3 for debugging (0 otherwise)
  // Bit 1 set: draws a green background (the entire given box)
  // Bit 2 set: draws a red frame around the check mark (the bounding box)
  // The background (1) can be used to test correct positioning by the widget code

#define DEBUG_FRAME (0)
#if (DEBUG_FRAME)
  if (DEBUG_FRAME & 1) {    // 1 = background
    fl_color(0x88dd8800);
    fl_rectf(bb.x(), bb.y(), bb.w(), bb.h());
  }
  if (DEBUG_FRAME & 2) {    // 2 = bounding box
    fl_color(0xff000000);
    fl_rect(tx, ty, tw, th);
  }
#endif

  // draw the check mark

  fl_color(col);

  fl_begin_complex_polygon();

    ty += d2 - d1;            // upper border of check mark: left to right
    fl_vertex(tx, ty);
    fl_vertex(tx + d1, ty + d1);
    fl_vertex(tx + d1 + d2, ty + d1 - d2);
    ty += lh;                 // lower border of check mark: right to left
    fl_vertex(tx + d1 + d2, ty + d1 - d2);
    fl_vertex(tx + d1, ty + d1);
    fl_vertex(tx, ty);

  fl_end_complex_polygon();

  fl_color(saved_color);

} // fl_draw_check()

/**
  Draw a potentially small, filled circle using a given color.

  This function draws a filled circle bounded by rectangle
  <tt>(x, y, d, d)</tt> using color \p color

  This function is the same as <tt>fl_pie(x, y, d, d, 0, 360)</tt> except
  with some systems that don't draw small circles well. In that situation,
  the circle diameter \p d is converted from FLTK units to pixels and this
  function approximates a filled circle by drawing several filled
  rectangles if the converted diameter is ≤ 6 pixels.

  The current drawing color fl_color() is preserved across the call.

  \param[in]  x,y     coordinates of top left of the bounding box
  \param[in]  d       diameter == width and height of the bounding box in FLTK units
  \param[in]  color   the color used to draw the circle

  \since 1.4.0
 */
void fl_draw_circle(int x, int y, int d, Fl_Color color) {
#define DEBUG_DRAW_CIRCLE (0) // bit 1 = draw bounding box (green)
#if (DEBUG_DRAW_CIRCLE & 0)
  Fl_Color current = fl_color();
  fl_rectf(x, y, d, d, FL_GREEN);
  fl_color(current);
#endif
  fl_graphics_driver->draw_circle(x, y, d, color);
}

/**
  Draw a round check mark (circle) of a radio button.

  This draws only the round "radio button mark", it does not draw the
  (also typically round) box of the radio button.

  Call this only if the radio button is \c ON.

  This method draws a scheme specific "circle" with a particular light effect
  if the scheme is gtk+. For all other schemes this function draws a simple,
  small circle.

  The \c color must be chosen by the caller so it has enough contrast with
  the background.

  The bounding box of the circle is the rectangle <tt>(x, y, d, d)</tt>.

  The current drawing color fl_color() is preserved across the call.

  \param[in]  x,y     coordinates of top left of the bounding box
  \param[in]  d       diameter == width and height of the bounding box in FLTK units
  \param[in]  color   the base color used to draw the circle

  \since 1.4.0
*/
void fl_draw_radio(int x, int y, int d, Fl_Color color) {

  Fl_Color current = fl_color();

#if (0) // DEBUG: draw bounding box
  fl_color(fl_lighter(FL_RED));
  fl_rectf(x, y, d, d);
#endif

  if (Fl::is_scheme("gtk+")) {
    fl_color(color);
    fl_pie(x, y, d, d, 0.0, 360.0);
    Fl_Color icol = fl_color_average(FL_WHITE, color, 0.2f);
    fl_draw_circle(x + 2, y + 2, d - 4, icol);
    fl_color(fl_color_average(FL_WHITE, color, 0.5));
    fl_arc(x + 1, y + 1, d - 1, d - 1, 60.0, 180.0);
  } else {
    fl_draw_circle(x + 1, y + 1, d - 2, color);
  }
  fl_color(current);
}
