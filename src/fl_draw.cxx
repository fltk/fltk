//
// "$Id: fl_draw.cxx,v 1.6.2.4.2.1 2001/08/05 23:58:54 easysw Exp $"
//
// Label drawing code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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

// Implementation of fl_draw(const char*,int,int,int,int,Fl_Align)
// Used to draw all the labels and text, this routine:
// Word wraps the labels to fit into their bounding box.
// Breaks them into lines at the newlines.
// Expands all unprintable characters to ^X or \nnn notation
// Aligns them against the inside of the box.

#include <FL/fl_draw.H>
#include <FL/Fl_Image.H>

#include <string.h>

#define MAXBUF 1024

char fl_draw_shortcut;	// set by fl_labeltypes.cxx

static char* underline_at;

// Copy p to buf, replacing unprintable characters with ^X and \nnn
// Stop at a newline of if MAXBUF characters written to buffer.
// Also word-wrap if width exceeds maxw.
// Returns a pointer to the start of the next line of caharcters.
// Sets n to the number of characters put into the buffer.
// Sets width to the width of the string in the current font.

static const char*
expand(const char* from, char* buf, double maxw, int& n, double &width, int wrap) {

  char* o = buf;
  char* e = buf+(MAXBUF-4);
  underline_at = 0;
  char* word_end = o;
  const char* word_start = from;
  double w = 0;

  const char* p = from;
  for (;; p++) {

    int c = *p & 255;

    if (!c || c == ' ' || c == '\n') {
      // test for word-wrap:
      if (word_start < p && wrap) {
	double newwidth = w + fl_width(word_end, o-word_end);
	if (word_end > buf && newwidth > maxw) { // break before this word
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

    if (o > e) break; // don't overflow buffer

    if (c == '\t') {
      for (c = (o-buf)%8; c<8 && o<e; c++) *o++ = ' ';

    } else if (c == '&' && fl_draw_shortcut && *(p+1)) {
      if (*(p+1) == '&') {p++; *o++ = '&';}
      else if (fl_draw_shortcut != 2) underline_at = o;

    } else if (c < ' ' || c == 127) { // ^X
      *o++ = '^';
      *o++ = c ^ 0x40;

    } else if (c == 0xA0) { // non-breaking space
      *o++ = ' ';

    } else {
      *o++ = c;

    }
  }

  width = w + fl_width(word_end, o-word_end);
  *o = 0;
  n = o-buf;
  return p;
}

void fl_draw(
    const char* str,	// the (multi-line) string
    int x, int y, int w, int h,	// bounding box
    Fl_Align align,
    void (*callthis)(const char*,int,int,int),
    Fl_Image* img) {
  const char* p;
  const char* e;
  char buf[MAXBUF];
  int buflen;

  // count how many lines and put the last one into the buffer:
  int lines;
  double width;
  for (p=str,lines=0; ;) {
    e = expand(p, buf, w, buflen, width, align&FL_ALIGN_WRAP);
    lines++;
    if (!*e) break;
    p = e;
  }

  // figure out vertical position of the first line:
  int xpos;
  int ypos;
  int height = fl_height();
  int imgh = img ? img->h() : 0;

  if (align & FL_ALIGN_BOTTOM) ypos = y+h-(lines-1)*height-imgh;
  else if (align & FL_ALIGN_TOP) ypos = y+height;
  else ypos = y+(h-lines*height-imgh)/2+height;

  // draw the image unless the "text over image" alignment flag is set...
  if (img && !(align & FL_ALIGN_TEXT_OVER_IMAGE)) {
    if (align & FL_ALIGN_LEFT) xpos = x;
    else if (align & FL_ALIGN_RIGHT) xpos = x+w-img->w();
    else xpos = x+(w-img->w())/2;

    img->draw(xpos, ypos - height);
    ypos += img->h();
  }

  // now draw all the lines:
  int desc = fl_descent();
  for (p=str; ; ypos += height) {
    if (lines>1) e = expand(p, buf, w, buflen, width, align&FL_ALIGN_WRAP);

    if (align & FL_ALIGN_LEFT) xpos = x;
    else if (align & FL_ALIGN_RIGHT) xpos = x+w-int(width+.5);
    else xpos = x+int((w-width)/2);

    callthis(buf,buflen,xpos,ypos-desc);

    if (underline_at)
      callthis("_",1,xpos+int(fl_width(buf,underline_at-buf)),ypos-desc);

    if (!*e) break;
    p = e;
  }

  // draw the image if the "text over image" alignment flag is set...
  if (img && (align & FL_ALIGN_TEXT_OVER_IMAGE)) {
    if (align & FL_ALIGN_LEFT) xpos = x;
    else if (align & FL_ALIGN_RIGHT) xpos = x+w-img->w();
    else xpos = x+(w-img->w())/2;

    img->draw(xpos, ypos);
  }
}

void fl_draw(
  const char* str,	// the (multi-line) string
  int x, int y, int w, int h,	// bounding box
  Fl_Align align,
  Fl_Image* img) {
  if (!str || !*str) return;
  if (w && h && !fl_not_clipped(x, y, w, h)) return;
  if (align & FL_ALIGN_CLIP) fl_clip(x, y, w, h);
  fl_draw(str, x, y, w, h, align, fl_draw, img);
  if (align & FL_ALIGN_CLIP) fl_pop_clip();
}

void fl_measure(const char* str, int& w, int& h) {
  h = fl_height();
  if (!str || !*str) {w = 0; return;}
  const char* p;
  const char* e;
  char buf[MAXBUF];
  int buflen;
  int lines;
  double width;
  int W = 0;
  for (p=str,lines=0; ;) {
    e = expand(p, buf, w, buflen, width, w!=0);
    if (int(width) > W) W = int(width);
    lines++;
    if (!*e) break;
    p = e;
  }
  w = W;
  h = lines*h;
}

//
// End of "$Id: fl_draw.cxx,v 1.6.2.4.2.1 2001/08/05 23:58:54 easysw Exp $".
//
