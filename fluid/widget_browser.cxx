//
// Widget Browser code for the Fast Light Tool Kit (FLTK).
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

#include "widget_browser.h"

#include "fluid.h"
#include "Fl_Widget_Type.h"
#include "pixmaps.h"

#include <FL/Fl.H>
#include <FL/Fl_Browser_.H>
#include <FL/fl_draw.H>

/**
 \class Widget_Browser

 A widget that displays the nodes in the widget tree.

 The Widget Browser is derived from the FLTK basic browser, extending
 tree browsing functionality by using the \c depth component of the double
 linked list of \c Fl_Type items.

 \see Fl_Type
 */

// ---- global variables

/// Global access to the widget browser.
Widget_Browser *widget_browser = NULL;

// ---- global functions

/**
 Shortcut to have the widget browser graphics refreshed soon.
 */
void redraw_browser() {
  widget_browser->redraw();
}

/**
 Shortcut to create the widget browser.
 */
Fl_Widget *make_widget_browser(int x,int y,int w,int h) {
  return (widget_browser = new Widget_Browser(x,y,w,h));
}

/**
 Make sure thet the caller is visible in the widget browser.
 \param[in] caller scroll the browser in y so that caller
 is visible (may be NULL)
 */
void redraw_widget_browser(Fl_Type *caller)
{
  if (caller)
    widget_browser->display(caller);
  widget_browser->redraw();
}

/**
 Select or deselect a node in the widget browser.
 \param[in] o (de)select this node
 \param[in] v the new selection state (1=select, 0=de-select)
 */
void select(Fl_Type *o, int v) {
  widget_browser->select(o,v,1);
}

/**
 Select a single node in the widget browser, deselect all others.
 \param[in] o select this node
 */
void select_only(Fl_Type *o) {
  widget_browser->select_only(o,1);
}

/**
 Deselect all nodes in the widget browser.
 */
void deselect() {
  widget_browser->deselect();
}

/**
 Show the selected item in the browser window.

 Make sure that the given item is visible in the browser by opening
 all parent groups and moving the item into the visible space.

 \param[in] t show this ite
 */
void reveal_in_browser(Fl_Type *t) {
  Fl_Type *p = t->parent;
  if (p) {
    for (;;) {
      if (!p->open_)
        p->open_ = 1;
      if (!p->parent) break;
      p = p->parent;
    }
    fixvisible(p);
  }
  widget_browser->display(t);
  widget_browser->redraw();
}

// ---- local functions

/**
 Copy the given string str to buffer p with no more than maxl characters.

 Add "..." if string was truncated.

 Quote characters are NOT counted.

 \param[out] p return the resulting string in this buffer, terminated with
    a NUL byte
 \param[in] str copy this string; utf8 aware
 \param[in] maxl maximum number of letter to copy until we print
    the elipsis (...)
 \param[in] quote if set, the resulting string is embedded in double quotes
 \returns pointer to end of string (before terminating null byte).
 \note the buffer p must be large enough to hold (4 * (maxl+1) + 1) bytes
    or (4 * (maxl+1) + 3) bytes if quoted, e.g. "123..." because each UTF-8
    character can consist of 4 bytes, "..." adds 3 bytes, quotes '""' add two
    bytes, and the terminating null byte adds another byte.
    This supports Unicode code points up to U+10FFFF (standard as of 10/2016).
    Sanity checks for illegal UTF-8 sequences are included.
 */
static char *copy_trunc(char *p, const char *str, int maxl, int quote)
{
  int size = 0;                         // truncated string size in characters
  int bs;                               // size of UTF-8 character in bytes
  const char *end = str + strlen(str);  // end of input string
  if (quote) *p++ = '"';                // opening quote
  while (size < maxl) {                 // maximum <maxl> characters
    if (!(*str & (-32))) break;         // end of string (0 or control char)
    bs = fl_utf8len(*str);              // size of next character
    if (bs <= 0) break;                 // some error - leave
    if (str + bs > end) break;          // UTF-8 sequence beyond end of string
    while (bs--) *p++ = *str++;         // copy that character into the buffer
    size++;                             // count copied characters
  }
  if (*str) {                           // string was truncated
    strcpy(p,"..."); p += 3;
  }
  if (quote) *p++ = '"';                // closing quote
  *p = 0;                               // terminating null byte
  return p;
}

// ---- Widget_Browser implementation

/**
 Create a new instance of the Widget_Browser widget.

 Fluid currently generates only one instance of this browser. If we want
 to use multiple browser at some point, we need to refactor a few global
 variables, i.e. Fl_Type::first and Fl_Type::last .

 \param[in] X, Y, W, H position and size of widget
 \param[in] l optional label
 \todo It would be nice to be able to grab one or more nodes and mmove them
    within the hierarchy.
 */
Widget_Browser::Widget_Browser(int X,int Y,int W,int H,const char*l) :
  Fl_Browser_(X,Y,W,H,l),
  pushedtitle(NULL),
  saved_h_scroll_(0),
  saved_v_scroll_(0)
{
  type(FL_MULTI_BROWSER);
  Fl_Widget::callback(callback_stub);
  when(FL_WHEN_RELEASE);
}

/**
 Override the method to find the first item in the list of elements.
 \return the first item
 */
void *Widget_Browser::item_first() const {
  return Fl_Type::first;
}

/**
 Override the method to find the next item in the list of elements.
 \param l this item
 \return the next item, irregardless of tree depth, or NULL at the end
 */
void *Widget_Browser::item_next(void *l) const {
  return ((Fl_Type*)l)->next;
}

/**
 Override the method to find the previous item in the list of elements.
 \param l this item
 \return the previous item, irregardless of tree depth, or NULL at the start
 */
void *Widget_Browser::item_prev(void *l) const {
  return ((Fl_Type*)l)->prev;
}

/**
 Override the method to check if an item was selected.
 \param l this item
 \return 1 if selected, 0 if not
 \todo what is the difference between selected and new_selected, and why do we do this?
 */
int Widget_Browser::item_selected(void *l) const {
  return ((Fl_Type*)l)->new_selected;
}

/**
 Override the method to mark an item selected.
 \param l this item
 \param[in] v 1 if selecting, 0 if not
 */
void Widget_Browser::item_select(void *l,int v) {
  ((Fl_Type*)l)->new_selected = v;
}

/**
 Override the method to return the height of an item representation in Flixels.
 \param l this item
 \return height in FLTK units (used to be pixels before high res screens)
 */
int Widget_Browser::item_height(void *l) const {
  Fl_Type *t = (Fl_Type*)l;
  if (t->visible) {
    if (show_comments && t->comment())
      return textsize()*2+4;
    else
      return textsize()+5;
  }
  return 0;
}

/**
 Override the method to return the estimated height of all items.
 \return height in FLTK units
 */
int Widget_Browser::incr_height() const {
  return textsize()+2;
}

/**
 Draw an item in the widget browser.

 A browser line starts with a variable size space. This space directly
 relates to the level of the type entry.

 If this type has the ability to store children, a triangle follows,
 pointing right (closed) or pointing down (open, children shown).

 Next follows an icon that is specific to the type. This makes it easy to
 spot certain types.

 Now follows some text. For classes and widgets, this is the type itself,
 followed by the name of the object. Other objects show their content as
 text, possibly abbreviated with an ellipsis.

 \param v      v is a pointer to the actual widget type and can be cast safely
    to Fl_Type
 \param X,Y    these give the position in window coordinates of the top left
    corner of this line
 */
void Widget_Browser::item_draw(void *v, int X, int Y, int, int) const {
  // cast to a more general type
  Fl_Type *l = (Fl_Type *)v;

  char buf[340]; // edit buffer: large enough to hold 80 UTF-8 chars + nul

  // calculate the horizontal start position of this item
  // 3 is the edge of the browser
  // 13 is the width of the arrow that indicates children for the item
  // 18 is the width of the icon
  // 12 is the indent per level
  X += 3 + 13 + 18 + l->level * 12;

  // calculate the horizontal start position and width of the separator line
  int x1 = X;
  int w1 = w() - x1;

  // items can contain a comment. If they do, the comment gets a second text
  // line inside this browser line
  int comment_incr = 0;
  if (show_comments && l->comment()) {
    copy_trunc(buf, l->comment(), 80, 0);
    comment_incr = textsize()-1;
    Fl_Color comment_color = fl_color_average(FL_DARK_GREEN, FL_BLACK, 0.9f);
    if (l->new_selected) fl_color(fl_contrast(comment_color, FL_SELECTION_COLOR));
    else fl_color(fl_contrast(comment_color, color()));
    fl_font(textfont()+FL_ITALIC, textsize()-2);
    fl_draw(buf, X, Y+12);
    Y += comment_incr/2;
    comment_incr -= comment_incr/2;
  }

  if (l->new_selected) fl_color(fl_contrast(FL_FOREGROUND_COLOR,FL_SELECTION_COLOR));
  else fl_color(FL_FOREGROUND_COLOR);

  // Width=10: Draw the triangle that indicates possible children
  if (l->is_parent()) {
    X = X - 18 - 13;
    if (!l->next || l->next->level <= l->level) {
      if (l->open_!=(l==pushedtitle)) {
        // an outlined triangle to the right indicates closed item, no children
        fl_loop(X,Y+7,X+5,Y+12,X+10,Y+7);
      } else {
        // an outlined triangle to the bottom indicates open item, no children
        fl_loop(X+2,Y+2,X+7,Y+7,X+2,Y+12);
      }
    } else {
      if (l->open_!=(l==pushedtitle)) {
        // a filled triangle to the right indicates closed item, with children
        fl_polygon(X,Y+7,X+5,Y+12,X+10,Y+7);
      } else {
        // a filled triangle to the bottom indicates open item, with children
        fl_polygon(X+2,Y+2,X+7,Y+7,X+2,Y+12);
      }
    }
    X = X + 13 + 18;
  }

  // Width=18: Draw the icon associated with the type.
  Fl_Pixmap *pm = pixmap[l->pixmapID()];
  if (pm) pm->draw(X-18, Y);

  // Add tags on top of the icon for locked and protected types.
  switch (l->is_public()) {
    case 0: lock_pixmap->draw(X - 17, Y); break;
    case 2: protected_pixmap->draw(X - 17, Y); break;
  }

  if (   l->is_widget()
      && !l->is_window()
      && ((Fl_Widget_Type*)l)->o
      && !((Fl_Widget_Type*)l)->o->visible()
      && (!l->parent || (   strcmp(l->parent->type_name(),"Fl_Tabs")
                         && strcmp(l->parent->type_name(),"Fl_Wizard")) )
      )
  {
    invisible_pixmap->draw(X - 17, Y);
  }

  // Indent=12 per level: Now write the text that comes after the graphics representation
  Y += comment_incr;
  if (l->is_widget() || l->is_class()) {
    const char* c = subclassname(l);
    if (!strncmp(c,"Fl_",3)) c += 3;
    fl_font(textfont(), textsize());
    fl_draw(c, X, Y+13);
    X += int(fl_width(c)+fl_width('n'));
    c = l->name();
    if (c) {
      fl_font(textfont()|FL_BOLD, textsize());
      fl_draw(c, X, Y+13);
    } else if ((c = l->label())) {
      copy_trunc(buf, c, 20, 1); // quoted string
      fl_draw(buf, X, Y+13);
    }
  } else {
    copy_trunc(buf, l->title(), 55, 0);
    fl_font(textfont() | (l->is_code_block() && (l->level==0 || l->parent->is_class())?0:FL_BOLD), textsize());
    fl_draw(buf, X, Y+13);
  }

  // draw a thin line below the item if this item is not selected
  // (if it is selected this additional line would look bad)
  if (!l->new_selected) {
    fl_color(fl_lighter(FL_GRAY));
    fl_line(x1,Y+16,x1+w1,Y+16);
  }
}

/**
 Override the method to return the width of an item representation in Flixels.
 \param v this item
 \return width in FLTK units
 */
int Widget_Browser::item_width(void *v) const {

  char buf[340]; // edit buffer: large enough to hold 80 UTF-8 chars + nul

  Fl_Type *l = (Fl_Type *)v;

  if (!l->visible) return 0;

  int W = 3 + 13 + 18 + l->level * 12;

  if (l->is_widget() || l->is_class()) {
    const char* c = l->type_name();
    if (!strncmp(c,"Fl_",3)) c += 3;
    fl_font(textfont(), textsize());
    W += int(fl_width(c) + fl_width('n'));
    c = l->name();
    if (c) {
      fl_font(textfont()|FL_BOLD, textsize());
      W += int(fl_width(c));
    } else if (l->label()) {
      copy_trunc(buf, l->label(), 20, 1); // quoted string
      W += int(fl_width(buf));
    }
  } else {
    copy_trunc(buf, l->title(), 55, 0);
    fl_font(textfont() | (l->is_code_block() && (l->level==0 || l->parent->is_class())?0:FL_BOLD), textsize());
    W += int(fl_width(buf));
  }

  return W;
}

/**
 Callback to tell the FLuid UI when the list of selected items changed.
 */
void Widget_Browser::callback() {
  selection_changed((Fl_Type*)selection());
}

/**
 Override the event handling for this browser.

 The vertical mouse position corresponds to an entry in the type tree.
 The horizontal position has the following hot zones:
 - 0-3 is the widget frame and ignored
 - the next hot zone starts 12*indent pixels further to the right
 - the next 13 pixels refer to the arrow that indicates children for the item
 - 18 pixels follow for the icon
 - the remaining part is filled with text

 \param[in] e the incoming event type
 \return 0 if the event is not supported, and 1 if the event was "used up"
 */
int Widget_Browser::handle(int e) {
  static Fl_Type *title;
  Fl_Type *l;
  int X,Y,W,H; bbox(X,Y,W,H);
  switch (e) {
  case FL_PUSH:
    if (!Fl::event_inside(X,Y,W,H)) break;
    l = (Fl_Type*)find_item(Fl::event_y());
    if (l) {
      X += 3 + 12*l->level - hposition();
      if (l->is_parent() && Fl::event_x()>X && Fl::event_x()<X+13) {
        title = pushedtitle = l;
        redraw_line(l);
        return 1;
      }
    }
    break;
  case FL_DRAG:
    if (!title) break;
    l = (Fl_Type*)find_item(Fl::event_y());
    if (l) {
      X += 3 + 12*l->level - hposition();
      if (l->is_parent() && Fl::event_x()>X && Fl::event_x()<X+13) ;
      else l = 0;
    }
    if (l != pushedtitle) {
      if (pushedtitle) redraw_line(pushedtitle);
      if (l) redraw_line(l);
      pushedtitle = l;
    }
    return 1;
  case FL_RELEASE:
    if (!title) {
      l = (Fl_Type*)find_item(Fl::event_y());
      if (l && l->new_selected && (Fl::event_clicks() || Fl::event_state(FL_CTRL)))
        l->open();
      break;
    }
    l = pushedtitle;
    title = pushedtitle = 0;
    if (l) {
      if (l->open_) {
        l->open_ = 0;
        for (Fl_Type*k = l->next; k&&k->level>l->level; k = k->next)
          k->visible = 0;
      } else {
        l->open_ = 1;
        for (Fl_Type*k=l->next; k&&k->level>l->level;) {
          k->visible = 1;
          if (k->is_parent() && !k->open_) {
            Fl_Type *j;
            for (j = k->next; j && j->level>k->level; j = j->next) {/*empty*/}
            k = j;
          } else
            k = k->next;
        }
      }
      redraw();
    }
    return 1;
  }
  return Fl_Browser_::handle(e);
}

/**
 Save the current scrollbar postion during rebuild.
 */
void Widget_Browser::save_scroll_position() {
  saved_h_scroll_ = hposition();
  saved_v_scroll_ = vposition();
}

/**
 Restore the previous scrollbar postion after rebuild.
 */
void Widget_Browser::restore_scroll_position() {
  hposition(saved_h_scroll_);
  vposition(saved_v_scroll_);
}

/**
 Rebuild the browser layout to reflect multiple changes.
 This clears internal caches, recalculates the scroll bar sizes, and
 sends a redraw() request to the widget.
 */
void Widget_Browser::rebuild() {
  save_scroll_position();
  new_list();
  damage(FL_DAMAGE_SCROLL);
  redraw();
  restore_scroll_position();
}

/**
 Rebuild the browser layout and make sure that the given item is visible.
 \param[in] inNode pointer to a widget node derived from Fl_Type.
 */
void Widget_Browser::display(Fl_Type *inNode) {
  if (!inNode) {
    // Alternative: find the first (last?) visible selected item.
    return;
  }
  // remeber our current scroll position
  int currentV = vposition(), newV = currentV;
  int nodeV = 0;
  // find the inNode in the tree and check, if it is already visible
  Fl_Type *p=Fl_Type::first;
  for ( ; p && p!=inNode; p=p->next) {
    if (p->visible)
      nodeV += item_height(p);
  }
  if (p) {
    int xx, yy, ww, hh;
    bbox(xx, yy, ww, hh);
    int frame_top = xx-x();
    int frame_bottom = frame_top + hh;
    int node_height = item_height(inNode);
    int margin_height = 2 * item_quick_height(inNode);
    if (margin_height>hh/2) margin_height = hh/2;
    // is the inNode above the current scroll position?
    if (nodeV<currentV+margin_height)
      newV = nodeV - margin_height;
    else if (nodeV>currentV+frame_bottom-margin_height-node_height)
      newV = nodeV - frame_bottom + margin_height + node_height;
    if (newV<0)
      newV = 0;
  }
  if (newV!=currentV)
    vposition(newV);
}

