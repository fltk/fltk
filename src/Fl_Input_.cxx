//
// Common input widget routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2011 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Input_.H>
#include <FL/Fl_Window.H>
#include "Fl_Screen_Driver.H"
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <math.h>
#include <FL/fl_utf8.h>
#include "flstring.h"
#include <stdlib.h>
#include <ctype.h>

#define MAXBUF 1024
static int l_secret;

extern void fl_draw(const char*, int, float, float);

////////////////////////////////////////////////////////////////

// see: Fl_Text_Undo_Action
class Fl_Input_Undo_Action {
public:
  Fl_Input_Undo_Action() :
  undobuffer(NULL),
  undobufferlength(0),
  undoat(0),
  undocut(0),
  undoinsert(0),
  undoyankcut(0)
  { }
  ~Fl_Input_Undo_Action() {
    if (undobuffer)
      ::free(undobuffer);
  }

  char *undobuffer;
  int undobufferlength;
  int undoat;              // points after insertion
  int undocut;             // number of characters deleted there
  int undoinsert;          // number of characters inserted
  int undoyankcut;         // length of valid contents of buffer, even if undocut=0

  /*
   Resize the undo buffer to match at least the requested size.
   */
  void undobuffersize(int n)
  {
    if (n > undobufferlength) {
      undobufferlength = n + 128;
      undobuffer = (char *)realloc(undobuffer, undobufferlength);
    }
  }

  void clear() {
    undocut = undoinsert = 0;
  }
};

// see: Fl_Text_Undo_Action_List
class Fl_Input_Undo_Action_List {
  Fl_Input_Undo_Action** list_;
  int list_size_;
  int list_capacity_;
public:
  Fl_Input_Undo_Action_List() :
    list_(NULL),
    list_size_(0),
    list_capacity_(0)
  { }

  ~Fl_Input_Undo_Action_List() {
    clear();
  }

  int size() const {
    return list_size_;
  }

  void push(Fl_Input_Undo_Action* action) {
    if (list_size_ == list_capacity_) {
      list_capacity_ += 25;
      list_ = (Fl_Input_Undo_Action**)realloc(list_, list_capacity_ * sizeof(Fl_Input_Undo_Action*));
    }
    list_[list_size_++] = action;
  }

  Fl_Input_Undo_Action* pop() {
    if (list_size_ > 0)
      return list_[--list_size_];
    else
      return NULL;
  }

  void clear() {
    if (list_) {
      for (int i=0; i<list_size_; i++) {
        delete list_[i];
      }
      ::free(list_);
    }
    list_ = NULL;
    list_size_ = 0;
    list_capacity_ = 0;
  }
};


/** \internal
  Converts a given text segment into the text that will be rendered on screen.

  This copies the text from \p p to \p buf, replacing characters with <tt>^X</tt>
  and <tt>\\nnn</tt> as necessary.

  The destination buffer is limited to \c MAXBUF (currently at 1024). All
  following text is truncated.

  \param [in] p pointer to source buffer
  \param [in] buf pointer to destination buffer
  \return pointer to the end of the destination buffer
*/
const char* Fl_Input_::expand(const char* p, char* buf) const {
  char* o = buf;
  char* e = buf+(MAXBUF-4);
  const char* lastspace = p;
  char* lastspace_out = o;
  int width_to_lastspace = 0;
  int word_count = 0;
  int word_wrap;
//  const char *pe = p + strlen(p);

  if (input_type()==FL_SECRET_INPUT) {
    while (o<e && p < value_+size_) {
      if (fl_utf8len((char)p[0]) >= 1) {
        l_secret = fl_utf8encode(Fl_Screen_Driver::secret_input_character, o);
        o += l_secret;
      }
      p++;
    }

  } else while (o<e) {
    if (wrap() && (p >= value_+size_ || isspace(*p & 255))) {
      word_wrap = w() - Fl::box_dw(box()) - 2;
      width_to_lastspace += (int)fl_width(lastspace_out, (int) (o-lastspace_out));
      if (p > lastspace+1) {
        if (word_count && width_to_lastspace > word_wrap) {
          p = lastspace; o = lastspace_out; break;
        }
        word_count++;
      }
      lastspace = p;
      lastspace_out = o;
    }

    if (p >= value_+size_) break;
    int c = *p++ & 255;
    if (c < ' ' || c == 127) {
      if (c=='\n' && input_type()==FL_MULTILINE_INPUT) {p--; break;}
      if (c == '\t' && input_type()==FL_MULTILINE_INPUT) {
        for (c = fl_utf_nb_char((uchar*)buf, (int) (o-buf))%8; c<8 && o<e; c++) {
          *o++ = ' ';
        }
      } else {
        *o++ = '^';
        *o++ = c ^ 0x40;
      }
    } else {
      *o++ = c;
    }
  }
  *o = 0;
  return p;
}

/** \internal
  Calculates the width in pixels of part of a text buffer.

  This call takes a string, usually created by expand, and calculates
  the width of the string when rendered with the given font.

  \param [in] p pointer to the start of the original string
  \param [in] e pointer to the end of the original string
  \param [in] buf pointer to the buffer as returned by expand()
  \return width of string in pixels
*/
double Fl_Input_::expandpos(
  const char* p,        // real string
  const char* e,        // pointer into real string
  const char* buf,      // conversion of real string by expand()
  int* returnn          // return offset into buf here
) const {
  int n = 0;
  int chr = 0;
  int l;
  if (input_type()==FL_SECRET_INPUT) {
    while (p<e) {
      l = fl_utf8len((char)p[0]);
      if (l >= 1) n += l_secret;
      p += l;
    }
  } else while (p<e) {
    int c = *p & 255;
    if (c < ' ' || c == 127) {
      if (c == '\t' && input_type()==FL_MULTILINE_INPUT) {
         n += 8-(chr%8);
         chr += 7-(chr%8);
      } else n += 2;
    } else {
      n += fl_utf8len1(*p);
    }
    chr += fl_utf8len((char)p[0]) >= 1;
    p += fl_utf8len1(*p);
  }
  if (returnn) *returnn = n;
  return fl_width(buf, n);
}

////////////////////////////////////////////////////////////////

/** \internal
  Marks a range of characters for update.

  This call marks all characters from \p p to the end of the
  text buffer for update. At least these characters
  will be redrawn in the next update cycle.

  Characters from \p mu_p to end of widget are redrawn.
  If \p erase_cursor_only, small part at \p mu_p is redrawn.
  Right now minimal update just keeps unchanged characters from
  being erased, so they don't blink.

  \param [in] p start of update range
*/
void Fl_Input_::minimal_update(int p) {
  if (damage() & FL_DAMAGE_ALL) return; // don't waste time if it won't be done
  if (damage() & FL_DAMAGE_EXPOSE) {
    if (p < mu_p) mu_p = p;
  } else {
    mu_p = p;
  }

  damage(FL_DAMAGE_EXPOSE);
  erase_cursor_only = 0;
}

/** \internal
  Marks a range of characters for update.

  This call marks a text range for update. At least all characters
  from \p p to \p q will be redrawn in the next update cycle.

  \param [in] p start of update range
  \param [in] q end of update range
*/
void Fl_Input_::minimal_update(int p, int q) {
  if (q < p) p = q;
  minimal_update(p);
}

////////////////////////////////////////////////////////////////

/* Horizontal cursor position in pixels while moving up or down. */
double Fl_Input_::up_down_pos = 0;

/* Flag to remember last cursor move. */
int Fl_Input_::was_up_down = 0;

/**
  Sets the current font and font size.
*/
void Fl_Input_::setfont() const {
  fl_font(textfont(), textsize());
}

/**
 Draws the text in the passed bounding box.

 If <tt>damage() & FL_DAMAGE_ALL</tt> is true, this assumes the
 area has already been erased to color(). Otherwise it does
 minimal update and erases the area itself.

 \param X, Y, W, H area that must be redrawn
 */
void Fl_Input_::drawtext(int X, int Y, int W, int H) {
  drawtext(X, Y, W, H, (Fl::focus()==this));
}

/**
  Draws the text in the passed bounding box.

  This version of `drawtext` allows the user to control whether the widget is
  drawn as acitive, i.e. with the text cursor, or inactive. This is useful for
  compound widgets where the input should be shown as active when actually
  the container widget is the active one.

  A caller should not draw the widget with `active` set if another text
  widget may indeed be the active widget.

  \param X, Y, W, H area that must be redrawn
  \param draw_active if set, the cursor will be drawn, even if the widget is not active
  \see Fl_Input_::drawtext(int X, int Y, int W, int H)
*/
void Fl_Input_::drawtext(int X, int Y, int W, int H, bool draw_active) {
  int do_mu = !(damage()&FL_DAMAGE_ALL);

  if (!draw_active && !size()) {
    if (do_mu) { // we have to erase it if cursor was there
      draw_box(box(), X-Fl::box_dx(box()), Y-Fl::box_dy(box()),
               W+Fl::box_dw(box()), H+Fl::box_dh(box()), color());
    }
    return;
  }

  int selstart, selend;
  if (!draw_active && /*Fl::selection_owner()!=this &&*/ Fl::pushed()!=this)
    selstart = selend = 0;
  else if (insert_position() <= mark()) {
    selstart = insert_position(); selend = mark();
  } else {
    selend = insert_position(); selstart = mark();
  }

  setfont();
  const char *p, *e;
  char buf[MAXBUF];

  // count how many lines and put the last one into the buffer:
  // And figure out where the cursor is:
  int height = fl_height();
  int threshold = height/2;
  int lines;
  int curx, cury;
  for (p=value(), curx=cury=lines=0; ;) {
    e = expand(p, buf);
    if (insert_position() >= p-value() && insert_position() <= e-value()) {
      curx = int(expandpos(p, value()+insert_position(), buf, 0)+.5);
      if (draw_active && !was_up_down) up_down_pos = curx;
      cury = lines*height;
      int newscroll = xscroll_;
      if (curx > newscroll+W-threshold) {
        // figure out scrolling so there is space after the cursor:
        newscroll = curx+threshold-W;
        // figure out the furthest left we ever want to scroll:
        int ex = int(expandpos(p, e, buf, 0))+4-W;
        // use minimum of both amounts:
        if (ex < newscroll) newscroll = ex;
      } else if (curx < newscroll+threshold) {
        newscroll = curx-threshold;
      }
      if (newscroll < 0) newscroll = 0;
      if (newscroll != xscroll_) {
        xscroll_ = newscroll;
        mu_p = 0; erase_cursor_only = 0;
      }
    }
    lines++;
    if (e >= value_+size_) break;
    p = e+1;
  }

  // adjust the scrolling:
  if (input_type()==FL_MULTILINE_INPUT) {
    int newy = yscroll_;
    if (cury < newy) newy = cury;
    if (cury > newy+H-height) newy = cury-H+height;
    if (newy < -1) newy = -1;
    if (newy != yscroll_) {yscroll_ = newy; mu_p = 0; erase_cursor_only = 0;}
  } else {
    yscroll_ = -(H-height)/2;
  }

  fl_push_clip(X, Y, W, H);
  Fl_Color tc = active_r() ? textcolor() : fl_inactive(textcolor());

  p = value();
  // visit each line and draw it:
  int desc = height-fl_descent();
  float xpos = (float)(X - xscroll_ + 1);
  int ypos = -yscroll_;
  int ypos_cur = 0; //fix issue #270
  for (; ypos < H;) {

    // re-expand line unless it is the last one calculated above:
    if (lines>1) e = expand(p, buf);

    if (ypos <= -height) goto CONTINUE; // clipped off top

    if (do_mu) {        // for minimal update:
      const char* pp = value()+mu_p; // pointer to where minimal update starts
      if (e < pp) goto CONTINUE2; // this line is before the changes
      if (readonly()) erase_cursor_only = 0; // this isn't the most efficient way
      if (erase_cursor_only && p > pp) goto CONTINUE2; // this line is after
      // calculate area to erase:
      float r = (float)(X+W);
      float xx;
      if (p >= pp) {
        xx = (float)X;
        if (erase_cursor_only) r = xpos+2;
        else if (readonly()) xx -= 3;
      } else {
        xx = xpos + (float)expandpos(p, pp, buf, 0);
        if (erase_cursor_only) r = xx+2;
        else if (readonly()) xx -= 3;
      }
      // clip to and erase it:
      fl_push_clip((int)xx-1-height/8, Y+ypos, (int)(r-xx+2+height/4), height);
      draw_box(box(), X-Fl::box_dx(box()), Y-Fl::box_dy(box()),
               W+Fl::box_dw(box()), H+Fl::box_dh(box()), color());
      // it now draws entire line over it
      // this should not draw letters to left of erased area, but
      // that is nyi.
    }

    // Draw selection area if required:
    if (selstart < selend && selstart <= e-value() && selend > p-value()) {
      const char* pp = value()+selstart;
      float x1 = xpos;
      int offset1 = 0;
      if (pp > p) {
        fl_color(tc);
        x1 += (float)expandpos(p, pp, buf, &offset1);
        fl_draw(buf, offset1, xpos, (float)(Y+ypos+desc));
      }
      pp = value()+selend;
      float x2 = (float)(X+W);
      int offset2;
      if (pp <= e) x2 = xpos + (float)expandpos(p, pp, buf, &offset2);
      else offset2 = (int) strlen(buf);
      if (Fl::screen_driver()->has_marked_text() && Fl::compose_state) {
        fl_color(textcolor());
      }
      else
      {
        fl_color(selection_color());
        fl_rectf((int)(x1+0.5), Y+ypos, (int)(x2-x1+0.5), height);
        fl_color(fl_contrast(textcolor(), selection_color()));
      }
      fl_draw(buf+offset1, offset2-offset1, x1, (float)(Y+ypos+desc));
      if (Fl::screen_driver()->has_marked_text() && Fl::compose_state) {
        fl_color( fl_color_average(textcolor(), color(), 0.6f) );
        float width = (float)fl_width(buf+offset1, offset2-offset1);
        fl_line((int)x1, Y+ypos+height-1, (int)(x1+width), Y+ypos+height-1);
      }
      if (pp < e) {
        fl_color(tc);
        fl_draw(buf+offset2, (int) strlen(buf+offset2), x2, (float)(Y+ypos+desc));
      }
    } else {
      // draw unselected text
      fl_color(tc);
      fl_draw(buf, (int) strlen(buf), xpos, (float)(Y+ypos+desc));
    }

    if (do_mu) fl_pop_clip();

  CONTINUE2:
    // draw the cursor:
    if ( draw_active
         && ( (Fl::screen_driver()->has_marked_text() && Fl::compose_state)
              || selstart == selend
             )
         && insert_position() >= p-value()
         && insert_position() <= e-value()
        ) {
      fl_color(cursor_color());
      // cursor position may need to be recomputed (see STR #2486)
      curx = int(expandpos(p, value()+insert_position(), buf, 0)+.5);
      if (readonly()) {
        // Draw '^' caret cursor
        fl_line((int)(xpos+curx-2.5f), Y+ypos+height-1,
                (int)(xpos+curx+0.5f), Y+ypos+height-4,
                (int)(xpos+curx+3.5f), Y+ypos+height-1);
      } else {
        fl_rectf((int)(xpos+curx+0.5), Y+ypos, 2, height);
      }
      ypos_cur = ypos+height; //fix issue #270
    }

  CONTINUE:
    ypos += height;
    if (e >= value_+size_) break;
    if (*e == '\n' || *e == ' ') e++;
    p = e;
  }

  // for minimal update, erase all lines below last one if necessary:
  if (input_type()==FL_MULTILINE_INPUT && do_mu && ypos<H
      && (!erase_cursor_only || p <= value()+mu_p)) {
    if (ypos < 0) ypos = 0;
    fl_push_clip(X, Y+ypos, W, H-ypos);
    draw_box(box(), X-Fl::box_dx(box()), Y-Fl::box_dy(box()),
             W+Fl::box_dw(box()), H+Fl::box_dh(box()), color());
    fl_pop_clip();
  }

  fl_pop_clip();
  if (draw_active) {
    fl_set_spot(textfont(), textsize(),
                (int)xpos+curx, Y+ypos_cur-fl_descent(), W, H, window()); //fix issue #270
  }
}

/** \internal
  Simple function that determines if a character could be part of a word.
  \todo This function is not UTF-8-aware.
*/
static int isword(char c) {
  return (c&128 || isalnum(c) || strchr("#%-@_~", c));
}

/**
  Finds the end of a word.

  Returns the index after the last byte of a word.
  If the index is already at the end of a word, it will find the
  end of the following word, so if you call it repeatedly you will
  move forwards to the end of the text.

  Note that this is inconsistent with line_end().

  \param [in] i starting index for the search
  \return end of the word
*/
int Fl_Input_::word_end(int i) const {
  if (input_type() == FL_SECRET_INPUT) return size();
  while (i < size() && !isword(index(i))) i++;
  while (i < size() && isword(index(i))) i++;
  return i;
}

/**
  Finds the start of a word.

  Returns the index of the first byte of a word.
  If the index is already at the beginning of a word, it will find the
  beginning of the previous word, so if you call it repeatedly you will
  move backwards to the beginning of the text.

  Note that this is inconsistent with line_start().

  \param [in] i starting index for the search
  \return start of the word, or previous word
*/
int Fl_Input_::word_start(int i) const {
  if (input_type() == FL_SECRET_INPUT) return 0;
  while (i > 0 && !isword(index(i-1))) i--;
  while (i > 0 && isword(index(i-1))) i--;
  return i;
}

/**
  Finds the end of a line.

  This call calculates the end of a line based on the given
  index \p i.

  \param [in] i starting index for the search
  \return end of the line
*/
int Fl_Input_::line_end(int i) const {
  if (input_type() != FL_MULTILINE_INPUT) return size();

  if (wrap()) {
    // go to the start of the paragraph:
    int j = i;
    while (j > 0 && index(j-1) != '\n') j--;
    // now measure lines until we get past i, end of that line is real eol:
    setfont();
    for (const char* p=value()+j; ;) {
      char buf[MAXBUF];
      p = expand(p, buf);
      int k = (int) (p-value());
      if (k >= i) return k;
      p++;
    }
  } else {
    while (i < size() && index(i) != '\n') i++;
    return i;
  }
}

/**
  Finds the start of a line.

  This call calculates the start of a line based on the given
  index \p i.

  \param [in] i starting index for the search
  \return start of the line
*/
int Fl_Input_::line_start(int i) const {
  if (input_type() != FL_MULTILINE_INPUT) return 0;
  int j = i;
  while (j > 0 && index(j-1) != '\n') j--;
  if (wrap()) {
    // now measure lines until we get past i, start of that line is real eol:
    setfont();
    for (const char* p=value()+j; ;) {
      char buf[MAXBUF];
      const char* e = expand(p, buf);
      if ((int) (e-value()) >= i) return (int) (p-value());
      p = e+1;
    }
  } else return j;
}

static int strict_word_start(const char *s, int i, int itype) {
  if (itype == FL_SECRET_INPUT) return 0;
  while (i > 0 && !isspace(s[i-1]))
    i--;
  return i;
}

static int strict_word_end(const char *s, int len, int i, int itype) {
  if (itype == FL_SECRET_INPUT) return len;
  while (i < len && !isspace(s[i]))
    i++;
  return i;
}

/**
  Handles mouse clicks and mouse moves.
  \todo Add comment and parameters
*/
void Fl_Input_::handle_mouse(int X, int Y, int /*W*/, int /*H*/, int drag) {
  was_up_down = 0;
  if (!size()) return;
  setfont();

  const char *p, *e;
  char buf[MAXBUF];

  int theline = (input_type()==FL_MULTILINE_INPUT) ?
    (Fl::event_y()-Y+yscroll_)/fl_height() : 0;

  int newpos = 0;
  for (p=value();; ) {
    e = expand(p, buf);
    theline--; if (theline < 0) break;
    if (e >= value_+size_) break;
    p = e+1;
  }
  const char *l, *r, *t; double f0 = Fl::event_x()-X+xscroll_;
  for (l = p, r = e; l<r; ) {
    double f;
    int cw = fl_utf8len((char)l[0]);
    if (cw < 1) cw = 1;
    t = l+cw;
    f = X-xscroll_+expandpos(p, t, buf, 0);
    if (f <= Fl::event_x()) {l = t; f0 = Fl::event_x()-f;}
    else r = t-cw;
  }
  if (l < e) { // see if closer to character on right:
    double f1;
    int cw = fl_utf8len((char)l[0]);
    if (cw > 0) {
      f1 = X-xscroll_+expandpos(p, l + cw, buf, 0) - Fl::event_x();
      if (f1 < f0) l = l+cw;
    }
  }
  newpos = (int) (l-value());

  int newmark = drag ? mark() : newpos;
  if (Fl::event_clicks()) {
    if (newpos >= newmark) {
      if (newpos == newmark) {
        if (newpos < size()) newpos++;
        else newmark--;
      }
      if (Fl::event_clicks() > 1) {
        newpos = line_end(newpos);
        newmark = line_start(newmark);
      } else {
        newpos = strict_word_end(value(), size(), newpos, input_type());
        newmark = strict_word_start(value(), newmark, input_type());
      }
    } else {
      if (Fl::event_clicks() > 1) {
        newpos = line_start(newpos);
        newmark = line_end(newmark);
      } else {
        newpos = strict_word_start(value(), newpos, input_type());
        newmark = strict_word_end(value(), size(), newmark, input_type());
      }
    }
    // if the multiple click does not increase the selection, revert
    // to single-click behavior:
    if (!drag && (mark() > insert_position() ?
                  (newmark >= insert_position() && newpos <= mark()) :
                  (newmark >= mark() && newpos <= insert_position()))) {
      Fl::event_clicks(0);
      newmark = newpos = (int) (l-value());
    }
  }
  insert_position(newpos, newmark);
}

/**
  Sets the index for the cursor and mark.

  The input widget maintains two pointers into the string. The
  \e position (\c p) is where the cursor is. The
  \e mark (\c m) is the other end of the selected text. If they
  are equal then there is no selection. Changing this does not
  affect the clipboard (use copy() to do that).

  Changing these values causes a redraw(). The new
  values are bounds checked.

  \param p index for the cursor position
  \param m index for the mark
  \return 0 if no positions changed
  \see position(int), position(), mark(int)
*/
int Fl_Input_::insert_position(int p, int m) {
  int is_same = 0;
  was_up_down = 0;
  if (p<0) p = 0;
  if (p>size()) p = size();
  if (m<0) m = 0;
  if (m>size()) m = size();
  if (p == m) is_same = 1;

  while (p < position_ && p > 0 && (size() - p) > 0 &&
       (fl_utf8len((char)(value() + p)[0]) < 1)) { p--; }
  int ul = fl_utf8len((char)(value() + p)[0]);
  while (p < size() && p > position_ && ul < 0) {
       p++;
       ul = fl_utf8len((char)(value() + p)[0]);
  }

  while (m < mark_ && m > 0 && (size() - m) > 0 &&
       (fl_utf8len((char)(value() + m)[0]) < 1)) { m--; }
  ul = fl_utf8len((char)(value() + m)[0]);
  while (m < size() && m > mark_ && ul < 0) {
       m++;
       ul = fl_utf8len((char)(value() + m)[0]);
  }
  if (is_same) m = p;
  if (p == position_ && m == mark_) return 0;


  //if (Fl::selection_owner() == this) Fl::selection_owner(0);
  if (p != m) {
    if (p != position_) minimal_update(position_, p);
    if (m != mark_) minimal_update(mark_, m);
  } else {
    // new position is a cursor
    if (position_ == mark_) {
      // old position was just a cursor
      if (Fl::focus() == this && !(damage()&FL_DAMAGE_EXPOSE)) {
        minimal_update(position_); erase_cursor_only = 1;
      }
    } else { // old position was a selection
      minimal_update(position_, mark_);
    }
  }
  position_ = p;
  mark_ = m;
  return 1;
}

/**
  Moves the cursor to the column given by \p up_down_pos.

  This function is helpful when implementing up and down
  cursor movement. It moves the cursor from the beginning
  of a line to the column indicated by the global variable
  \p up_down_pos in pixel units.

  \param [in] i index into the beginning of a line of text
  \param [in] keepmark if set, move only the cursor, but not the mark
  \return index to new cursor position
*/
int Fl_Input_::up_down_position(int i, int keepmark) {
  // unlike before, i must be at the start of the line already!

  setfont();
  char buf[MAXBUF];
  const char* p = value()+i;
  const char* e = expand(p, buf);
  const char *l, *r, *t;
  for (l = p, r = e; l<r; ) {
    t = l+(r-l+1)/2;
    int f = (int)expandpos(p, t, buf, 0);
    if (f <= up_down_pos) l = t; else r = t-1;
  }
  int j = (int) (l-value());
  j = insert_position(j, keepmark ? mark_ : j);
  was_up_down = 1;
  return j;
}

/**
  Put the current selection into the clipboard.

  This function copies the current selection between mark() and
  position() into the specified \c clipboard. This does not
  replace the old clipboard contents if position() and
  mark() are equal. Clipboard 0 maps to the current text
  selection and clipboard 1 maps to the cut/paste clipboard.

  \param clipboard the clipboard destination 0 or 1
  \return 0 if no text is selected, 1 if the selection was copied
  \see Fl::copy(const char *, int, int)
*/
int Fl_Input_::copy(int clipboard) {
  int b = insert_position();
  int e = mark();
  if (b != e) {
    if (b > e) {b = mark(); e = insert_position();}
    if (input_type() == FL_SECRET_INPUT) e = b;
    Fl::copy(value()+b, e-b, clipboard);
    return 1;
  }
  return 0;
}

#define MAXFLOATSIZE 40

/**
 Append text at the end.

 This function appends the string in \p t to the end of the text.
 It does not moves the new position or mark.

 \param [in] t text that will be appended
 \param [in] l length of text, or 0 if the string is terminated by \c nul.
 \param [in] keep_selection if this is 1, the current text selection will
    remain, if 0, the cursor will move to the end of the inserted text.
 \return 0 if no text was appended
 */
int Fl_Input_::append(const char* t, int l, char keep_selection)
{
  int end = size();
  int om = mark_, op = position_;
  int ret = replace(end, end, t, l);
  if (keep_selection) {
    insert_position(op, om);
  }
  return ret;
}


/**
  Deletes text from \p b to \p e and inserts the new string \p text.

  All changes to the text buffer go through this function.
  It deletes the region between \p b and \p e (either one may be less or
  equal to the other), and then inserts the string \p text
  at that point and moves the mark() and
  position() to the end of the insertion. Does the callback if
  <tt>when() & FL_WHEN_CHANGED</tt> and there is a change.

  Set \p b and \p e equal to not delete anything.
  Set \p text to \c NULL to not insert anything.

  \p ilen can be zero or <tt>strlen(text)</tt>, which
  saves a tiny bit of time if you happen to already know the
  length of the insertion, or can be used to insert a portion of a
  string. If \p ilen is zero, <tt>strlen(text)</tt> is used instead.

  \p b and \p e are clamped to the <tt>0..size()</tt> range, so it is
  safe to pass any values. \p b, \p e, and \p ilen are used as numbers
  of bytes (not characters), where \p b and \p e count from 0 to
  size() (end of buffer).

  If \p b and/or \p e don't point to a valid UTF-8 character boundary,
  they are adjusted to the previous (\p b) or the next (\p e) valid
  UTF-8 character boundary, resp..

  If the current number of characters in the buffer minus deleted
  characters plus inserted characters in \p text would overflow the
  number of allowed characters (maximum_size()), then only the first
  characters of the string are inserted, so that maximum_size()
  is not exceeded.

  cut() and insert() are just inline functions that call replace().

  \param [in] b beginning index of text to be deleted
  \param [in] e ending index of text to be deleted and insertion position
  \param [in] text string that will be inserted
  \param [in] ilen length of \p text or 0 for \c nul terminated strings
  \return 0 if nothing changed

  \note If \p text does not point to a valid UTF-8 character or includes
  invalid UTF-8 sequences, the text is inserted nevertheless (counting
  invalid UTF-8 bytes as one character each).
*/
int Fl_Input_::replace(int b, int e, const char* text, int ilen) {
  int ul, om, op;
  was_up_down = 0;

  if (b<0) b = 0;
  if (e<0) e = 0;
  if (b>size_) b = size_;
  if (e>size_) e = size_;
  if (e<b) {int t=b; b=e; e=t;}
  while (b != e && b > 0 && (size_ - b) > 0 &&
       (fl_utf8len((value_ + b)[0]) < 1)) { b--; }
  ul = fl_utf8len((char)(value_ + e)[0]);
  while (e < size_ && e > 0 && ul < 0) {
       e++;
       ul = fl_utf8len((char)(value_ + e)[0]);
  }
  if (text && !ilen) ilen = (int) strlen(text);
  if (e<=b && !ilen) return 0; // don't clobber undo for a null operation

  // we must count UTF-8 *characters* to determine whether we can insert
  // the full text or only a part of it (and how much this would be)

  int nchars = 0;       // characters in value() - deleted + inserted
  const char *p = value_;
  while (p < (char *)(value_+size_)) {
    if (p == (char *)(value_+b)) { // skip removed part
      p = (char *)(value_+e);
      if (p >= (char *)(value_+size_)) break;
    }
    int ulen = fl_utf8len(*p);
    if (ulen < 1) ulen = 1; // invalid UTF-8 character: count as 1
    nchars++;
    p += ulen;
  }
  int nlen = 0;         // length (in bytes) to be inserted
  p = text;
  while (p < (char *)(text+ilen) && nchars < maximum_size()) {
    int ulen = fl_utf8len(*p);
    if (ulen < 1) ulen = 1; // invalid UTF-8 character: count as 1
    nchars++;
    p += ulen;
    nlen += ulen;
  }
  ilen = nlen;

  put_in_buffer(size_+ilen);

  if (e>b) {
    if (b == undo_->undoat) {
      undo_->undobuffersize(undo_->undocut+(e-b));
      memcpy(undo_->undobuffer+undo_->undocut, value_+b, e-b);
      undo_->undocut += e-b;
    } else if (e == undo_->undoat && !undo_->undoinsert) {
      undo_->undobuffersize(undo_->undocut+(e-b));
      memmove(undo_->undobuffer+(e-b), undo_->undobuffer, undo_->undocut);
      memcpy(undo_->undobuffer, value_+b, e-b);
      undo_->undocut += e-b;
    } else if (e == undo_->undoat && (e-b)<undo_->undoinsert) {
      undo_->undoinsert -= e-b;
    } else {
      redo_list_->clear();
      undo_list_->push(undo_);
      undo_ = new Fl_Input_Undo_Action();
      undo_->undobuffersize(e-b);
      memcpy(undo_->undobuffer, value_+b, e-b);
      undo_->undocut = e-b;
      undo_->undoinsert = 0;
    }
    memmove(buffer+b, buffer+e, size_-e+1);
    size_ -= e-b;
    undo_->undoat = b;
    if (input_type() == FL_SECRET_INPUT) undo_->undoyankcut = 0; else undo_->undoyankcut = undo_->undocut;
  }

  if (ilen) {
    if (b == undo_->undoat) {
      undo_->undoinsert += ilen;
    } else {
      redo_list_->clear();
      undo_list_->push(undo_);
      undo_ = new Fl_Input_Undo_Action();
      undo_->undocut = 0;
      undo_->undoinsert = ilen;
    }
    memmove(buffer+b+ilen, buffer+b, size_-b+1);
    memcpy(buffer+b, text, ilen);
    size_ += ilen;
  }
  om = mark_;
  op = position_;
  mark_ = position_ = undo_->undoat = b+ilen;

  // Insertions into the word at the end of the line will cause it to
  // wrap to the next line, so we must indicate that the changes may start
  // right after the whitespace before the current word.  This will
  // result in sub-optimal update when such wrapping does not happen
  // but it is too hard to figure out for now...
  if (wrap()) {
    // if there is a space in the pasted text, the whole line may have rewrapped
    int i;
    for (i=0; i<ilen; i++)
      if (text[i]==' ') break;
    if (i==ilen)
      while (b > 0 && !isspace(index(b) & 255) && index(b)!='\n') b--;
    else
      while (b > 0 && index(b)!='\n') b--;
  }

  // make sure we redraw the old selection or cursor:
  if (om < b) b = om;
  if (op < b) b = op;

  minimal_update(b);

  mark_ = position_ = undo_->undoat;

  set_changed();
  if (when()&FL_WHEN_CHANGED) do_callback(FL_REASON_CHANGED);
  return 1;
}

/**
 Apply the current undo/redo operation

 It's up to undo() and redo() to push and pop actions to and from the lists.

 \return 1 if the current action changed any text.
 \see undo(), redo() */
int Fl_Input_::apply_undo() {
  was_up_down = 0;
  if (!undo_->undocut && !undo_->undoinsert) return 0;

  int ilen = undo_->undocut;
  int xlen = undo_->undoinsert;
  int b = undo_->undoat-xlen;
  int b1 = b;

  minimal_update(position_);

  put_in_buffer(size_+ilen);

  if (ilen) {
    memmove(buffer+b+ilen, buffer+b, size_-b+1);
    memcpy(buffer+b, undo_->undobuffer, ilen);
    size_ += ilen;
    b += ilen;
  }

  if (xlen) {
    undo_->undobuffersize(xlen);
    memcpy(undo_->undobuffer, buffer+b, xlen);
    memmove(buffer+b, buffer+b+xlen, size_-xlen-b+1);
    size_ -= xlen;
  }

  undo_->undocut = xlen;
  if (xlen) undo_->undoyankcut = xlen;
  undo_->undoinsert = ilen;
  undo_->undoat = b;
  mark_ = b /* -ilen */;
  position_ = b;

  if (wrap())
    while (b1 > 0 && index(b1)!='\n') b1--;
  minimal_update(b1);
  set_changed();

  return 1;
}

/**
 Undoes previous changes to the text buffer.

 This call undoes a number of previous calls to replace().

 \return non-zero if any change was made.
 */
int Fl_Input_::undo() {
  if (apply_undo() == 0)
    return 0;

  redo_list_->push(undo_);
  undo_ = undo_list_->pop();
  if (!undo_) undo_ = new Fl_Input_Undo_Action();

  if (when()&FL_WHEN_CHANGED) do_callback(FL_REASON_CHANGED);

  return 1;
}

/**
 Check if the last operation can be undone.

 \return true if the widget can undo the last change
 */
bool Fl_Input_::can_undo() const {
  return (undo_->undocut || undo_->undoinsert);
}

/**
 Redo previous undo operation.

 This call reapplies previously executed undo operations.

 \return non-zero if any change was made.
 */
int Fl_Input_::redo() {
  Fl_Input_Undo_Action *redo_action = redo_list_->pop();
  if (!redo_action)
    return 0;

  if (undo_->undocut || undo_->undoinsert)
    undo_list_->push(undo_);
  else
    delete undo_;
  undo_ = redo_action;

  int ret = apply_undo();
  if (ret && (when()&FL_WHEN_CHANGED)) do_callback(FL_REASON_CHANGED);

  return ret;
}

/**
 Check if there is a redo action available.

 \return true if the widget can redo the last undo action
 */
bool Fl_Input_::can_redo() const {
  return (redo_list_->size() > 0);
}

/**
  Copies the \e yank buffer to the clipboard.

  This method copies all the previous contiguous cuts from the undo
  information to the clipboard. This function implements
  the \c ^K shortcut key.

  \return 0 if the operation did not change the clipboard
  \see copy(int), cut()
*/
int Fl_Input_::copy_cuts() {
  // put the yank buffer into the X clipboard
  if (!undo_->undoyankcut || input_type()==FL_SECRET_INPUT) return 0;
  Fl::copy(undo_->undobuffer, undo_->undoyankcut, 1);
  return 1;
}

/** \internal
  Checks the when() field and does a callback if indicated.
*/
void Fl_Input_::maybe_do_callback(Fl_Callback_Reason reason) {
  if (changed() || (when()&FL_WHEN_NOT_CHANGED)) {
    do_callback(reason);
  }
}

/**
  Handles all kinds of text field related events.

  This is called by derived classes.
  \todo Add comment and parameters
*/
int Fl_Input_::handletext(int event, int X, int Y, int W, int H) {
  switch (event) {

  case FL_ENTER:
  case FL_MOVE:
    if (active_r() && window()) window()->cursor(FL_CURSOR_INSERT);
    return 1;

  case FL_LEAVE:
    if (active_r() && window()) window()->cursor(FL_CURSOR_DEFAULT);
    return 1;

  case FL_FOCUS:
    fl_set_spot(textfont(), textsize(), x(), y(), w(), h(), window());
    if (mark_ == position_) {
      minimal_update(size()+1);
    } else //if (Fl::selection_owner() != this)
      minimal_update(mark_, position_);
    return 1;

  case FL_UNFOCUS:
    if (active_r() && window()) window()->cursor(FL_CURSOR_DEFAULT);
    if (mark_ == position_) {
      if (!(damage()&FL_DAMAGE_EXPOSE)) {minimal_update(position_); erase_cursor_only = 1;}
    } else //if (Fl::selection_owner() != this)
      minimal_update(mark_, position_);
      // FALLTHROUGH
  case FL_HIDE:
    fl_reset_spot();
    if (!readonly() && (when() & FL_WHEN_RELEASE))
      maybe_do_callback(FL_REASON_LOST_FOCUS);
    return 1;

  case FL_PUSH:
    if (active_r() && window()) window()->cursor(FL_CURSOR_INSERT);

    handle_mouse(X, Y, W, H, Fl::event_state(FL_SHIFT));

    if (Fl::focus() != this) {
      Fl::focus(this);
      handle(FL_FOCUS);
    }
    return 1;

  case FL_DRAG:
    handle_mouse(X, Y, W, H, 1);
    return 1;

  case FL_RELEASE:
    copy(0);
    return 1;

  case FL_PASTE: {
    // Don't allow pastes into readonly widgets...
    if (readonly()) {
      fl_beep(FL_BEEP_ERROR);
      return 1;
    }

    // See if we have anything to paste...
    if (!Fl::event_text() || !Fl::event_length()) return 1;

    // strip trailing control characters and spaces before pasting:
    const char* t = Fl::event_text();
    const char* e = t+Fl::event_length();
    if (input_type() != FL_MULTILINE_INPUT) while (e > t && isspace(*(e-1) & 255)) e--;
    if (!t || e <= t) return 1; // Int/float stuff will crash without this test
    if (input_type() == FL_INT_INPUT) {
      while (isspace(*t & 255) && t < e) t ++;
      const char *p = t;
      if (*p == '+' || *p == '-') p ++;
      if (strncmp(p, "0x", 2) == 0) {
        p += 2;
        while (isxdigit(*p & 255) && p < e) p ++;
      } else {
        while (isdigit(*p & 255) && p < e) p ++;
      }
      if (p < e) {
        fl_beep(FL_BEEP_ERROR);
        return 1;
      } else return replace(0, size(), t, (int) (e-t));
    } else if (input_type() == FL_FLOAT_INPUT) {
      while (isspace(*t & 255) && t < e) t ++;
      const char *p = t;
      if (*p == '+' || *p == '-') p ++;
      while (isdigit(*p & 255) && p < e) p ++;
      if (*p == '.') {
        p ++;
        while (isdigit(*p & 255) && p < e) p ++;
        if (*p == 'e' || *p == 'E') {
          p ++;
          if (*p == '+' || *p == '-') p ++;
          while (isdigit(*p & 255) && p < e) p ++;
        }
      }
      if (p < e) {
        fl_beep(FL_BEEP_ERROR);
        return 1;
      } else return replace(0, size(), t, (int) (e-t));
    }
    return replace(insert_position(), mark(), t, (int) (e-t));}

  case FL_SHORTCUT:
    if (!(shortcut() ? Fl::test_shortcut(shortcut()) : test_shortcut()))
      return 0;
    if (Fl::visible_focus() && handle(FL_FOCUS)) {
      Fl::focus(this);
      return 1;
    } // else fall through

  default:
    return 0;
  }
}

/*------------------------------*/

/**
  Creates a new Fl_Input_ widget.

  This function creates a new Fl_Input_ widget and adds it to the current
  Fl_Group. The value() is set to \c NULL.
  The default boxtype is \c FL_DOWN_BOX.

  \param X, Y, W, H the dimensions of the new widget
  \param l an optional label text
*/
Fl_Input_::Fl_Input_(int X, int Y, int W, int H, const char* l)
: Fl_Widget(X, Y, W, H, l) {
  box(FL_DOWN_BOX);
  color(FL_BACKGROUND2_COLOR, FL_SELECTION_COLOR);
  align(FL_ALIGN_LEFT);
  textsize_ = FL_NORMAL_SIZE;
  textfont_ = FL_HELVETICA;
  textcolor_ = FL_FOREGROUND_COLOR;
  cursor_color_ = FL_FOREGROUND_COLOR; // was FL_BLUE
  mark_ = position_ = size_ = 0;
  bufsize = 0;
  buffer  = 0;
  value_ = "";
  xscroll_ = yscroll_ = 0;
  maximum_size_ = 32767;
  shortcut_ = 0;
  undo_list_ = new Fl_Input_Undo_Action_List();
  redo_list_ = new Fl_Input_Undo_Action_List();
  undo_ = new Fl_Input_Undo_Action();
  set_flag(SHORTCUT_LABEL);
  set_flag(MAC_USE_ACCENTS_MENU);
  set_flag(NEEDS_KEYBOARD);
  tab_nav(1);
}

/**
 Copies the value from a possibly static entry into the internal buffer.

 \param [in] len size of the current text
*/
void Fl_Input_::put_in_buffer(int len) {
  if (value_ == buffer && bufsize > len) {
    buffer[size_] = 0;
    return;
  }
  if (!bufsize) {
    if (len > size_) len += 9; // let a few characters insert before realloc
    bufsize = len+1;
    buffer = (char*)malloc(bufsize);
  } else if (bufsize <= len) {
    // we may need to move old value in case it points into buffer:
    int moveit = (value_ >= buffer && value_ < buffer+bufsize);
    // enlarge current buffer
    if (len > size_) {
      do {bufsize *= 2;} while (bufsize <= len);
    } else {
      bufsize = len+1;
    }
    // Note: the following code is equivalent to:
    //
    //   if (moveit) value_ = value_ - buffer;
    //   char* nbuffer = (char*)realloc(buffer, bufsize);
    //   if (moveit) value_ = value_ + nbuffer;
    //   buffer = nbuffer;
    //
    // We just optimized the pointer arithmetic for value_...
    //
    char* nbuffer = (char*)realloc(buffer, bufsize);
    if (moveit) value_ += (nbuffer-buffer);
    buffer = nbuffer;
  }
  memmove(buffer, value_, size_); buffer[size_] = 0;
  value_ = buffer;
}

/**
  Changes the widget text.

  This function changes the text and sets the mark and the point to
  the end of it. The string is \e not copied. If the user edits the
  string it is copied to the internal buffer then. This can save a
  great deal of time and memory if your program is rapidly
  changing the values of text fields, but this will only work if
  the passed string remains unchanged until either the
  Fl_Input is destroyed or value() is called again.

  You can use the \p len parameter to directly set the length
  if you know it already or want to put \c nul characters in the text.

  \param [in] str the new text
  \param [in] len the length of the new text
  \return non-zero if the new value is different than the current one
*/
int Fl_Input_::static_value(const char* str, int len) {
  clear_changed();
  undo_->clear();
  undo_list_->clear();
  redo_list_->clear();
  if (str == value_ && len == size_) return 0;
  if (len) { // non-empty new value:
    if (xscroll_ || yscroll_) {
      xscroll_ = yscroll_ = 0;
      minimal_update(0);
    } else {
      int i = 0;
      // find first different character:
      if (value_) {
        for (; i<size_ && i<len && str[i]==value_[i]; i++) {/*empty*/}
        if (i==size_ && i==len) return 0;
      }
      minimal_update(i);
    }
    value_ = str;
    size_ = len;
  } else { // empty new value:
    if (!size_) return 0; // both old and new are empty.
    size_ = 0;
    value_ = "";
    xscroll_ = yscroll_ = 0;
    minimal_update(0);
  }
  insert_position(readonly() ? 0 : size());
  return 1;
}

/**
  Changes the widget text.

  This function changes the text and sets the mark and the point to
  the end of it. The string is \e not copied. If the user edits the
  string it is copied to the internal buffer then. This can save a
  great deal of time and memory if your program is rapidly
  changing the values of text fields, but this will only work if
  the passed string remains unchanged until either the
  Fl_Input is destroyed or value() is called again.

  \param [in] str the new text
  \return non-zero if the new value is different than the current one
*/
int Fl_Input_::static_value(const char* str) {
  return static_value(str, str ? (int) strlen(str) : 0);
}

/**
  Changes the widget text.

  This function changes the text and sets the mark and the
  point to the end of it. The string is copied to the internal
  buffer. Passing \c NULL is the same as "".

  You can use the \p length parameter to directly set the length
  if you know it already or want to put \c nul characters in the text.

  \param [in] str the new text
  \param [in] len the length of the new text
  \return non-zero if the new value is different than the current one
  \see Fl_Input_::value(const char* str), Fl_Input_::value()
*/
int Fl_Input_::value(const char* str, int len) {
  int r = static_value(str, len);
  if (len) put_in_buffer(len);
  return r;
}

/**
  Changes the widget text.

  This function changes the text and sets the mark and the
  point to the end of it. The string is copied to the internal
  buffer. Passing \c NULL is the same as \c "".

  \param [in] str the new text
  \return non-zero if the new value is different than the current one
  \see Fl_Input_::value(const char* str, int len), Fl_Input_::value()
*/
int Fl_Input_::value(const char* str) {
  return value(str, str ? (int) strlen(str) : 0);
}

/**
 Changes the widget text to a signed integer number.

 \param [in] v the new value
 \return non-zero if the new value is different than the current one
 \see Fl_Input_::value(const char* str), Fl_Input_::ivalue()
 */
int Fl_Input_::value(int v) {
  char buf[64];
  snprintf(buf, sizeof(buf)-1, "%d", v);
  return value(buf);
}

/**
 Changes the widget text to a floating point number ("%g").

 \param [in] v the new value
 \return non-zero if the new value is different than the current one
 \see Fl_Input_::value(const char* str), Fl_Input_::ivalue()
 */
int Fl_Input_::value(double v) {
  char buf[64];
  snprintf(buf, sizeof(buf)-1, "%g", v);
  return value(buf);
}

/**
 Returns the widget text interpreted as a signed integer.

 \return signed integer value
 \see Fl_Input_::dvalue()
 \see Fl_Input_::value(int)
 */
int Fl_Input_::ivalue() const {
  return atoi(value());
}

/**
 Returns the widget text interpreted as a floating point number.

 \return double precision floating point value
 \see Fl_Input_::ivalue()
 \see Fl_Input_::value(double)
 */
double Fl_Input_::dvalue() const {
  return atof(value());
}

/**
  Changes the size of the widget.
  This call updates the text layout so that the cursor is visible.
  \param [in] X, Y, W, H new size of the widget
  \see Fl_Widget::resize(int, int, int, int)
*/
void Fl_Input_::resize(int X, int Y, int W, int H) {
  if (W != w()) xscroll_ = 0;
  if (H != h()) yscroll_ = 0;
  Fl_Widget::resize(X, Y, W, H);
}

/**
  Destroys the widget.

  The destructor clears all allocated buffers and removes the widget
  from the parent Fl_Group.
*/
Fl_Input_::~Fl_Input_() {
  delete undo_list_;
  delete redo_list_;
  delete undo_;
  if (bufsize) free((void*)buffer);
}

/** \internal
  Returns the number of lines displayed on a single page.
  \return widget height divided by the font height
*/
int Fl_Input_::linesPerPage() {
  int n = 1;
  if (input_type() == FL_MULTILINE_INPUT) {
    fl_font(textfont(),textsize()); //ensure current font is set to ours
    n = h()/fl_height(); // number of lines to scroll
    if (n<=0) n = 1;
  }
  return n;
}

/**
  Returns the character at index \p i.

  This function returns the UTF-8 character at \p i
  as a ucs4 character code.

  \param [in] i index into the value field
  \return the character at index \p i
*/
unsigned int Fl_Input_::index(int i) const
{
  int len = 0;
  return fl_utf8decode(value_+i, value_+size_, &len);
}
