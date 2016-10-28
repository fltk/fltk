//
// "$Id$"
//
// Widget type code for the Fast Light Tool Kit (FLTK).
//
// Each object described by Fluid is one of these objects.  They
// are all stored in a double-linked list.
//
// They "type" of the object is covered by the virtual functions.
// There will probably be a lot of these virtual functions.
//
// The type browser is also a list of these objects, but they
// are "factory" instances, not "real" ones.  These objects exist
// only so the "make" method can be called on them.  They are
// not in the linked list and are not written to files or
// copied or otherwise examined.
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

#include <FL/Fl.H>
#include <FL/Fl_Browser_.H>
#include <FL/fl_draw.H>
#include <stdlib.h>
#include "../src/flstring.h"
#include <stdio.h>

#include "Fl_Type.h"
#include "undo.h"

#include <FL/Fl_Pixmap.H>
#include "pixmaps/lock.xpm"
#include "pixmaps/protected.xpm"
//#include "pixmaps/unlock.xpm"

static Fl_Pixmap	lock_pixmap(lock_xpm);
static Fl_Pixmap	protected_pixmap(protected_xpm);
//static Fl_Pixmap	unlock_pixmap(unlock_xpm);

#include "pixmaps/flWindow.xpm"
#include "pixmaps/flButton.xpm"
#include "pixmaps/flCheckButton.xpm"
#include "pixmaps/flRoundButton.xpm"
#include "pixmaps/flBox.xpm"
#include "pixmaps/flGroup.xpm"
#include "pixmaps/flFunction.xpm"
#include "pixmaps/flCode.xpm"
#include "pixmaps/flCodeBlock.xpm"
#include "pixmaps/flComment.xpm"
#include "pixmaps/flData.xpm"
#include "pixmaps/flDeclaration.xpm"
#include "pixmaps/flDeclarationBlock.xpm"
#include "pixmaps/flClass.xpm"
#include "pixmaps/flTabs.xpm"
#include "pixmaps/flInput.xpm"
#include "pixmaps/flChoice.xpm"
#include "pixmaps/flMenuitem.xpm"
#include "pixmaps/flMenubar.xpm"
#include "pixmaps/flSubmenu.xpm"
#include "pixmaps/flScroll.xpm"
#include "pixmaps/flTile.xpm"
#include "pixmaps/flWizard.xpm"
#include "pixmaps/flPack.xpm"
#include "pixmaps/flReturnButton.xpm"
#include "pixmaps/flLightButton.xpm"
#include "pixmaps/flRepeatButton.xpm"
#include "pixmaps/flMenuButton.xpm"
#include "pixmaps/flOutput.xpm"
#include "pixmaps/flTextDisplay.xpm"
#include "pixmaps/flTextEdit.xpm"
#include "pixmaps/flFileInput.xpm"
#include "pixmaps/flBrowser.xpm"
#include "pixmaps/flCheckBrowser.xpm"
#include "pixmaps/flFileBrowser.xpm"
#include "pixmaps/flClock.xpm"
#include "pixmaps/flHelp.xpm"
#include "pixmaps/flProgress.xpm"
#include "pixmaps/flSlider.xpm"
#include "pixmaps/flScrollBar.xpm"
#include "pixmaps/flValueSlider.xpm"
#include "pixmaps/flAdjuster.xpm"
#include "pixmaps/flCounter.xpm"
#include "pixmaps/flDial.xpm"
#include "pixmaps/flRoller.xpm"
#include "pixmaps/flValueInput.xpm"
#include "pixmaps/flValueOutput.xpm"
#include "pixmaps/flSpinner.xpm"
#include "pixmaps/flWidgetClass.xpm"
#include "pixmaps/flTree.xpm"
#include "pixmaps/flTable.xpm"

static Fl_Pixmap	window_pixmap(flWindow_xpm);
static Fl_Pixmap	button_pixmap(flButton_xpm);
static Fl_Pixmap	checkbutton_pixmap(flCheckButton_xpm);
static Fl_Pixmap	roundbutton_pixmap(flRoundButton_xpm);
static Fl_Pixmap	box_pixmap(flBox_xpm);
static Fl_Pixmap	group_pixmap(flGroup_xpm);
static Fl_Pixmap	function_pixmap(flFunction_xpm);
static Fl_Pixmap	code_pixmap(flCode_xpm);
static Fl_Pixmap	codeblock_pixmap(flCodeBlock_xpm);
static Fl_Pixmap	comment_pixmap(flComment_xpm);
static Fl_Pixmap	declaration_pixmap(flDeclaration_xpm);
static Fl_Pixmap	declarationblock_pixmap(flDeclarationBlock_xpm);
static Fl_Pixmap	class_pixmap(flClass_xpm);
static Fl_Pixmap	tabs_pixmap(flTabs_xpm);
static Fl_Pixmap	input_pixmap(flInput_xpm);
static Fl_Pixmap	choice_pixmap(flChoice_xpm);
static Fl_Pixmap	menuitem_pixmap(flMenuitem_xpm);
static Fl_Pixmap	menubar_pixmap(flMenubar_xpm);
static Fl_Pixmap	submenu_pixmap(flSubmenu_xpm);
static Fl_Pixmap	scroll_pixmap(flScroll_xpm);
static Fl_Pixmap	tile_pixmap(flTile_xpm);
static Fl_Pixmap	wizard_pixmap(flWizard_xpm);
static Fl_Pixmap	pack_pixmap(flPack_xpm);
static Fl_Pixmap	returnbutton_pixmap(flReturnButton_xpm);
static Fl_Pixmap	lightbutton_pixmap(flLightButton_xpm);
static Fl_Pixmap	repeatbutton_pixmap(flRepeatButton_xpm);
static Fl_Pixmap	menubutton_pixmap(flMenuButton_xpm);
static Fl_Pixmap	output_pixmap(flOutput_xpm);
static Fl_Pixmap	textdisplay_pixmap(flTextDisplay_xpm);
static Fl_Pixmap	textedit_pixmap(flTextEdit_xpm);
static Fl_Pixmap	fileinput_pixmap(flFileInput_xpm);
static Fl_Pixmap	browser_pixmap(flBrowser_xpm);
static Fl_Pixmap	checkbrowser_pixmap(flCheckBrowser_xpm);
static Fl_Pixmap	filebrowser_pixmap(flFileBrowser_xpm);
static Fl_Pixmap	clock_pixmap(flClock_xpm);
static Fl_Pixmap	help_pixmap(flHelp_xpm);
static Fl_Pixmap	progress_pixmap(flProgress_xpm);
static Fl_Pixmap	slider_pixmap(flSlider_xpm);
static Fl_Pixmap	scrollbar_pixmap(flScrollBar_xpm);
static Fl_Pixmap	valueslider_pixmap(flValueSlider_xpm);
static Fl_Pixmap	adjuster_pixmap(flAdjuster_xpm);
static Fl_Pixmap	counter_pixmap(flCounter_xpm);
static Fl_Pixmap	dial_pixmap(flDial_xpm);
static Fl_Pixmap	roller_pixmap(flRoller_xpm);
static Fl_Pixmap	valueinput_pixmap(flValueInput_xpm);
static Fl_Pixmap	valueoutput_pixmap(flValueOutput_xpm);
static Fl_Pixmap	spinner_pixmap(flSpinner_xpm);
static Fl_Pixmap	widgetclass_pixmap(flWidgetClass_xpm);
static Fl_Pixmap	data_pixmap(flData_xpm);
static Fl_Pixmap	tree_pixmap(flTree_xpm);
static Fl_Pixmap	table_pixmap(flTable_xpm);

Fl_Pixmap *pixmap[] = { 0, &window_pixmap, &button_pixmap, &checkbutton_pixmap, &roundbutton_pixmap, /* 0..4 */
 &box_pixmap, &group_pixmap, &function_pixmap, &code_pixmap, &codeblock_pixmap, &declaration_pixmap, /* 5..10 */ 
 &declarationblock_pixmap, &class_pixmap, &tabs_pixmap, &input_pixmap, &choice_pixmap,               /* 11..15 */
 &menuitem_pixmap, &menubar_pixmap, &submenu_pixmap, &scroll_pixmap, &tile_pixmap, &wizard_pixmap,   /* 16..21 */
 &pack_pixmap, &returnbutton_pixmap, &lightbutton_pixmap, &repeatbutton_pixmap, &menubutton_pixmap,  /* 22..26 */
 &output_pixmap, &textdisplay_pixmap, &textedit_pixmap, &fileinput_pixmap, &browser_pixmap,          /* 27..32 */
 &checkbrowser_pixmap, &filebrowser_pixmap, &clock_pixmap, &help_pixmap, &progress_pixmap,	     /* 33..36 */
 &slider_pixmap, &scrollbar_pixmap, &valueslider_pixmap, &adjuster_pixmap, &counter_pixmap,          /* 37..41 */
 &dial_pixmap, &roller_pixmap, &valueinput_pixmap, &valueoutput_pixmap, &comment_pixmap,             /* 42..46 */
 &spinner_pixmap, &widgetclass_pixmap, &data_pixmap, &tree_pixmap, &table_pixmap };                  /* 47..51 */

extern int show_comments;

////////////////////////////////////////////////////////////////

// Copy the given string str to buffer p with no more than maxl characters.
// Add "..." if string was truncated.
// If parameter quote is true (not 0) the string is quoted with "".
// Quote characters are NOT counted.
// The returned buffer (string) is terminated with a null byte.
// Returns pointer to end of string (before terminating null byte).
// Note: the buffer p must be large enough to hold (4 * (maxl+1) + 1) bytes
// or (4 * (maxl+1) + 3) bytes if quoted, e.g. "123..." because each UTF-8
// character can consist of 4 bytes, "..." adds 3 bytes, quotes '""' add two
// bytes, and the terminating null byte adds another byte.
// This supports Unicode code points up to U+10FFFF (standard as of 10/2016).
// Sanity checks for illegal UTF-8 sequences are included.

static char *copy_trunc(char *p, const char *str, int maxl, int quote) {

  int size = 0;				// truncated string size in characters
  int bs;				// size of UTF-8 character in bytes
  const char *end = str + strlen(str);	// end of input string
  if (quote) *p++ = '"';		// opening quote
  while (size < maxl) {			// maximum <maxl> characters
    if (!(*str & (-32))) break;		// end of string (0 or control char)
    bs = fl_utf8len(*str);		// size of next character
    if (bs <= 0) break;			// some error - leave
    if (str + bs > end) break;		// UTF-8 sequence beyond end of string
    while (bs--) *p++ = *str++;		// copy that character into the buffer
    size++;				// count copied characters
  }
  if (*str) {				// string was truncated
    strcpy(p,"..."); p += 3;
  }
  if (quote) *p++ = '"';		// closing quote
  *p = 0;				// terminating null byte
  return p;
}

////////////////////////////////////////////////////////////////

class Widget_Browser : public Fl_Browser_ {
  friend class Fl_Type;

  // required routines for Fl_Browser_ subclass:
  void *item_first() const ;
  void *item_next(void *) const ;
  void *item_prev(void *) const ;
  int item_selected(void *) const ;
  void item_select(void *,int);
  int item_width(void *) const ;
  int item_height(void *) const ;
  void item_draw(void *,int,int,int,int) const ;
  int incr_height() const ;

public:	

  int handle(int);
  void callback();
  Widget_Browser(int,int,int,int,const char * =0);
};

static Widget_Browser *widget_browser;
Fl_Widget *make_widget_browser(int x,int y,int w,int h) {
  return (widget_browser = new Widget_Browser(x,y,w,h));
}

void redraw_widget_browser(Fl_Type *caller)
{
  if (caller) {
    widget_browser->display(caller);
  }
  widget_browser->redraw();
}

void select(Fl_Type *o, int v) {
  widget_browser->select(o,v,1);
  //  Fl_Type::current = o;
}

void select_only(Fl_Type *o) {
  widget_browser->select_only(o,1);
}

void deselect() {
  widget_browser->deselect();
  //Fl_Type::current = 0; // this breaks the paste & merge functions
}

Fl_Type *Fl_Type::first;
Fl_Type *Fl_Type::last;

static void Widget_Browser_callback(Fl_Widget *o,void *) {
  ((Widget_Browser *)o)->callback();
}

Widget_Browser::Widget_Browser(int X,int Y,int W,int H,const char*l)
: Fl_Browser_(X,Y,W,H,l) {
  type(FL_MULTI_BROWSER);
  Fl_Widget::callback(Widget_Browser_callback);
  when(FL_WHEN_RELEASE);
}

void *Widget_Browser::item_first() const {return Fl_Type::first;}

void *Widget_Browser::item_next(void *l) const {return ((Fl_Type*)l)->next;}

void *Widget_Browser::item_prev(void *l) const {return ((Fl_Type*)l)->prev;}

int Widget_Browser::item_selected(void *l) const {return ((Fl_Type*)l)->new_selected;}

void Widget_Browser::item_select(void *l,int v) {((Fl_Type*)l)->new_selected = v;}

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

int Widget_Browser::incr_height() const {return textsize()+2;}

static Fl_Type* pushedtitle;

// Generate a descriptive text for this item, to put in browser & window titles
const char* Fl_Type::title() {
  const char* c = name(); if (c) return c;
  return type_name();
}

extern const char* subclassname(Fl_Type*);


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

  \param v	v is a pointer to the actual widget type and can be cast safely
		to Fl_Type
  \param X,Y	these give the position in window coordinates of the top left 
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
    if (l->new_selected) fl_color(fl_contrast(FL_DARK_GREEN,FL_SELECTION_COLOR));
    else fl_color(fl_contrast(FL_DARK_GREEN,color()));
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
    case 0: lock_pixmap.draw(X - 17, Y); break;
    case 2: protected_pixmap.draw(X - 17, Y); break;
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

void redraw_browser() {
  widget_browser->redraw();
}

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

Fl_Type::Fl_Type() {
  factory = 0;
  parent = 0;
  next = prev = 0;
  selected = new_selected = 0;
  visible = 0;
  name_ = 0;
  label_ = 0;
  user_data_ = 0;
  user_data_type_ = 0;
  callback_ = 0;
  comment_ = 0;
  rtti = 0;
  level = 0;
  code_position = header_position = -1;
  code_position_end = header_position_end = -1;
}

static void fixvisible(Fl_Type *p) {
  Fl_Type *t = p;
  for (;;) {
    if (t->parent) t->visible = t->parent->visible && t->parent->open_;
    else t->visible = 1;
    t = t->next;
    if (!t || t->level <= p->level) break;
  }
}

// turn a click at x,y on this into the actual picked object:
Fl_Type* Fl_Type::click_test(int,int) {return 0;}
void Fl_Type::add_child(Fl_Type*, Fl_Type*) {}
void Fl_Type::move_child(Fl_Type*, Fl_Type*) {}
void Fl_Type::remove_child(Fl_Type*) {}

// add a list of widgets as a new child of p:
void Fl_Type::add(Fl_Type *p) {
  if (p && parent == p) return;
  undo_checkpoint();
  parent = p;
  Fl_Type *end = this;
  while (end->next) end = end->next;
  Fl_Type *q;
  int newlevel;
  if (p) {
    for (q = p->next; q && q->level > p->level; q = q->next) {/*empty*/}
    newlevel = p->level+1;
  } else {
    q = 0;
    newlevel = 0;
  }
  for (Fl_Type *t = this->next; t; t = t->next) t->level += (newlevel-level);
  level = newlevel;
  if (q) {
    prev = q->prev;
    prev->next = this;
    q->prev = end;
    end->next = q;
  } else if (first) {
    prev = last;
    prev->next = this;
    end->next = 0;
    last = end;
  } else {
    first = this;
    last = end;
    prev = end->next = 0;
  }
  if (p) p->add_child(this,0);
  open_ = 1;
  fixvisible(this);
  set_modflag(1);
  widget_browser->redraw();
}

// add to a parent before another widget:
void Fl_Type::insert(Fl_Type *g) {
  Fl_Type *end = this;
  while (end->next) end = end->next;
  parent = g->parent;
  int newlevel = g->level;
  visible = g->visible;
  for (Fl_Type *t = this->next; t; t = t->next) t->level += newlevel-level;
  level = newlevel;
  prev = g->prev;
  if (prev) prev->next = this; else first = this;
  end->next = g;
  g->prev = end;
  fixvisible(this);
  if (parent) parent->add_child(this, g);
  widget_browser->redraw();
}

// Return message number for I18N...
int
Fl_Type::msgnum() {
  int		count;
  Fl_Type	*p;

  for (count = 0, p = this; p;) {
    if (p->label()) count ++;
    if (p != this && p->is_widget() && ((Fl_Widget_Type *)p)->tooltip()) count ++;

    if (p->prev) p = p->prev;
    else p = p->parent;
  }

  return count;
}


// delete from parent:
Fl_Type *Fl_Type::remove() {
  Fl_Type *end = this;
  for (;;) {
    if (!end->next || end->next->level <= level) break;
    end = end->next;
  }
  if (prev) prev->next = end->next;
  else first = end->next;
  if (end->next) end->next->prev = prev;
  else last = prev;
  Fl_Type *r = end->next;
  prev = end->next = 0;
  if (parent) parent->remove_child(this);
  parent = 0;
  widget_browser->redraw();
  selection_changed(0);
  return r;
}

// update a string member:
int storestring(const char *n, const char * & p, int nostrip) {
  if (n == p) return 0;
  undo_checkpoint();
  int length = 0;
  if (n) { // see if blank, strip leading & trailing blanks
    if (!nostrip) while (isspace((int)(unsigned char)*n)) n++;
    const char *e = n + strlen(n);
    if (!nostrip) while (e > n && isspace((int)(unsigned char)*(e-1))) e--;
    length = e-n;
    if (!length) n = 0;
  }    
  if (n == p) return 0;
  if (n && p && !strncmp(n,p,length) && !p[length]) return 0;
  if (p) free((void *)p);
  if (!n || !*n) {
    p = 0;
  } else {
    char *q = (char *)malloc(length+1);
    strlcpy(q,n,length+1);
    p = q;
  }
  set_modflag(1);
  return 1;
}

void Fl_Type::name(const char *n) {
  int nostrip = is_comment();
  if (storestring(n,name_,nostrip)) {
    if (visible) widget_browser->redraw();
  }
}

void Fl_Type::label(const char *n) {
  if (storestring(n,label_,1)) {
    setlabel(label_);
    if (visible && !name_) widget_browser->redraw();
  }
}

void Fl_Type::callback(const char *n) {
  storestring(n,callback_);
}

void Fl_Type::user_data(const char *n) {
  storestring(n,user_data_);
}

void Fl_Type::user_data_type(const char *n) {
  storestring(n,user_data_type_);
}

void Fl_Type::comment(const char *n) {
  if (storestring(n,comment_,1)) {
    if (visible) widget_browser->redraw();
  }
}

void Fl_Type::open() {
  printf("Open of '%s' is not yet implemented\n",type_name());
}

void Fl_Type::setlabel(const char *) {}

Fl_Type::~Fl_Type() {
  // warning: destructor only works for widgets that have been add()ed.
  if (widget_browser) widget_browser->deleting(this);
  if (prev) prev->next = next; else first = next;
  if (next) next->prev = prev; else last = prev;
  if (current == this) current = 0;
  if (parent) parent->remove_child(this);
  if (name_) free((void*)name_);
  if (label_) free((void*)label_);
  if (callback_) free((void*)callback_);
  if (user_data_) free((void*)user_data_);
  if (user_data_type_) free((void*)user_data_type_);
  if (comment_) free((void*)comment_);
}

int Fl_Type::is_parent() const {return 0;}
int Fl_Type::is_widget() const {return 0;}
int Fl_Type::is_valuator() const {return 0;}
int Fl_Type::is_spinner() const {return 0;}
int Fl_Type::is_button() const {return 0;}
int Fl_Type::is_input() const {return 0;}
int Fl_Type::is_value_input() const {return 0;}
int Fl_Type::is_text_display() const {return 0;}
int Fl_Type::is_menu_item() const {return 0;}
int Fl_Type::is_menu_button() const {return 0;}
int Fl_Type::is_group() const {return 0;}
int Fl_Type::is_window() const {return 0;}
int Fl_Type::is_code() const {return 0;}
int Fl_Type::is_code_block() const {return 0;}
int Fl_Type::is_decl_block() const {return 0;}
int Fl_Type::is_comment() const {return 0;}
int Fl_Type::is_class() const {return 0;}
int Fl_Type::is_public() const {return 1;}

int Fl_Code_Type::is_public()const { return -1; }
int Fl_CodeBlock_Type::is_public()const { return -1; }


////////////////////////////////////////////////////////////////

Fl_Type *in_this_only; // set if menu popped-up in window

void select_all_cb(Fl_Widget *,void *) {
  Fl_Type *p = Fl_Type::current ? Fl_Type::current->parent : 0;
  if (in_this_only) {
    Fl_Type *t = p;
    for (; t && t != in_this_only; t = t->parent) {/*empty*/}
    if (t != in_this_only) p = in_this_only;
  }
  for (;;) {
    if (p) {
      int foundany = 0;
      for (Fl_Type *t = p->next; t && t->level>p->level; t = t->next) {
	if (!t->new_selected) {widget_browser->select(t,1,0); foundany = 1;}
      }
      if (foundany) break;
      p = p->parent;
    } else {
      for (Fl_Type *t = Fl_Type::first; t; t = t->next)
	widget_browser->select(t,1,0);
      break;
    }
  }
  selection_changed(p);
}

void select_none_cb(Fl_Widget *,void *) {
  Fl_Type *p = Fl_Type::current ? Fl_Type::current->parent : 0;
  if (in_this_only) {
    Fl_Type *t = p;
    for (; t && t != in_this_only; t = t->parent) {/*empty*/}
    if (t != in_this_only) p = in_this_only;
  }
  for (;;) {
    if (p) {
      int foundany = 0;
      for (Fl_Type *t = p->next; t && t->level>p->level; t = t->next) {
	if (t->new_selected) {widget_browser->select(t,0,0); foundany = 1;}
      }
      if (foundany) break;
      p = p->parent;
    } else {
      for (Fl_Type *t = Fl_Type::first; t; t = t->next)
	widget_browser->select(t,0,0);
      break;
    }
  }
  selection_changed(p);
}

static void delete_children(Fl_Type *p) {
  Fl_Type *f;
  for (f = p; f && f->next && f->next->level > p->level; f = f->next) {/*empty*/}
  for (; f != p; ) {
    Fl_Type *g = f->prev;
    delete f;
    f = g;
  }
}

void delete_all(int selected_only) {
  for (Fl_Type *f = Fl_Type::first; f;) {
    if (f->selected || !selected_only) {
      delete_children(f);
      Fl_Type *g = f->next;
      delete f;
      f = g;
    } else f = f->next;
  }
  if(!selected_only) {
		include_H_from_C=1;
		use_FL_COMMAND=0;
	}

  selection_changed(0);
}

// move f (and it's children) into list before g:
// returns pointer to whatever is after f & children
void Fl_Type::move_before(Fl_Type* g) {
  if (level != g->level) printf("move_before levels don't match! %d %d\n",
				level, g->level);
  Fl_Type* n;
  for (n = next; n && n->level > level; n = n->next) {/*empty*/}
  if (n == g) return;
  Fl_Type *l = n ? n->prev : Fl_Type::last;
  prev->next = n;
  if (n) n->prev = prev; else Fl_Type::last = prev;
  prev = g->prev;
  l->next = g;
  if (prev) prev->next = this; else Fl_Type::first = this;
  g->prev = l;
  if (parent && is_widget()) parent->move_child(this,g);
  widget_browser->inserting(g, this);
  widget_browser->display(this);
  widget_browser->redraw();
}


// move selected widgets in their parent's list:
void earlier_cb(Fl_Widget*,void*) {
  Fl_Type *f;
  int mod = 0;
  for (f = Fl_Type::first; f; ) {
    Fl_Type* nxt = f->next;
    if (f->selected) {
      Fl_Type* g;
      for (g = f->prev; g && g->level > f->level; g = g->prev) {/*empty*/}
      if (g && g->level == f->level && !g->selected) {
        f->move_before(g);
        mod = 1;
      }
    }
    f = nxt;
  }
  if (mod) set_modflag(1);
}

void later_cb(Fl_Widget*,void*) {
  Fl_Type *f;
  int mod = 0;
  for (f = Fl_Type::last; f; ) {
    Fl_Type* prv = f->prev;
    if (f->selected) {
      Fl_Type* g;
      for (g = f->next; g && g->level > f->level; g = g->next) {/*empty*/}
      if (g && g->level == f->level && !g->selected) {
        g->move_before(f);
        mod = 1;
      }
    }
    f = prv;
  }
  if (mod) set_modflag(1);
}

////////////////////////////////////////////////////////////////

// write a widget and all it's children:
void Fl_Type::write() {
    write_indent(level);
    write_word(type_name());
    
    if (is_class()) {
      const char * p = 	((Fl_Class_Type*)this)->prefix();
      if (p &&	strlen(p))
        write_word(p);
    }

    write_word(name());
    write_open(level);
    write_properties();
    write_close(level);
    if (!is_parent()) return;
    // now do children:
    write_open(level);
    Fl_Type *child;
    for (child = next; child && child->level > level; child = child->next)
	if (child->level == level+1) child->write();
    write_close(level);
}

void Fl_Type::write_properties() {
  // repeat this for each attribute:
  if (label()) {
    write_indent(level+1);
    write_word("label");
    write_word(label());
  }
  if (user_data()) {
    write_indent(level+1);
    write_word("user_data");
    write_word(user_data());
  }
  if (user_data_type()) {
    write_word("user_data_type");
    write_word(user_data_type());
  }
  if (callback()) {
    write_indent(level+1);
    write_word("callback");
    write_word(callback());
  }
  if (comment()) {
    write_indent(level+1);
    write_word("comment");
    write_word(comment());
  }
  if (is_parent() && open_) write_word("open");
  if (selected) write_word("selected");
}

void Fl_Type::read_property(const char *c) {
  if (!strcmp(c,"label"))
    label(read_word());
  else if (!strcmp(c,"user_data"))
    user_data(read_word());
  else if (!strcmp(c,"user_data_type"))
    user_data_type(read_word());
  else if (!strcmp(c,"callback"))
    callback(read_word());
  else if (!strcmp(c,"comment"))
    comment(read_word());
  else if (!strcmp(c,"open"))
    open_ = 1;
  else if (!strcmp(c,"selected"))
    select(this,1);
  else
    read_error("Unknown property \"%s\"", c);
}

int Fl_Type::read_fdesign(const char*, const char*) {return 0;}

/**
  Return 1 if the list contains a function with the given signature at the top level.
 */
int has_toplevel_function(const char *rtype, const char *sig) {
  Fl_Type *child;
  for (child = Fl_Type::first; child; child = child->next) {
    if (!child->is_in_class() && strcmp(child->type_name(), "Function")==0) {
      const Fl_Function_Type *fn = (const Fl_Function_Type*)child;
      if (fn->has_signature(rtype, sig))
        return 1;
    }
  }
  return 0;
}

/**
  Write a comment into the header file.
*/
void Fl_Type::write_comment_h(const char *pre)
{
  if (comment() && *comment()) {
    write_h("%s/**\n", pre);
    const char *s = comment();
    write_h("%s ", pre);
    while(*s) {
      if (*s=='\n') {
        if (s[1]) {
          write_h("\n%s ", pre);
        }
      } else {
        write_h("%c", *s); // FIXME this is much too slow!
      }
      s++;
    }
    write_h("\n%s*/\n", pre);
  }
}

/**
  Write a comment into the source file.
*/
void Fl_Type::write_comment_c(const char *pre)
{
  if (comment() && *comment()) {
    write_c("%s/**\n", pre);
    const char *s = comment();
    write_c("%s ", pre);
    while(*s) {
      if (*s=='\n') {
        if (s[1]) {
          write_c("\n%s ", pre);
        }
      } else {
        write_c("%c", *s); // FIXME this is much too slow!
      }
      s++;
    }
    write_c("\n%s*/\n", pre);
  }
}

/**
  Write a comment into the source file.
*/
void Fl_Type::write_comment_inline_c(const char *pre)
{
  if (comment() && *comment()) {
    const char *s = comment();
    if (strchr(s, '\n')==0L) {
      // single line comment
      if (pre) write_c("%s", pre);
      write_c("// %s\n", s);
      if (!pre) write_c("%s  ", indent());
    } else {
      write_c("%s/*\n", pre?pre:"");
      if (pre) write_c("%s ", pre); else write_c("%s   ", indent());
      while(*s) {
        if (*s=='\n') {
          if (s[1]) {
            if (pre) write_c("\n%s ", pre); else write_c("\n%s   ", indent());
          }
        } else {
          write_c("%c", *s); // FIXME this is much too slow!
        }
        s++;
      }
      if (pre) write_c("\n%s */\n", pre); else write_c("\n%s   */\n", indent());
      if (!pre) write_c("%s  ", indent());
    }
  }
}

/**
  Make sure that the given item is visible in the browser by opening
  all parent groups and moving the item into the visible space.
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
  redraw_browser();
}

/**
  Build widgets and dataset needed in live mode.
  \return a widget pointer that the live mode initiator can 'show()'
  \see leave_live_mode()
*/
Fl_Widget *Fl_Type::enter_live_mode(int) {
  return 0L;
}

/**
  Release all resources created when entering live mode.
  \see enter_live_mode()
*/
void Fl_Type::leave_live_mode() {
}

/**
  Copy all needed properties for this type into the live object.
*/
void Fl_Type::copy_properties() {
}

/**
  Check whether callback \p cbname is declared anywhere else by the user.

  \b Warning: this just checks that the name is declared somewhere,
  but it should probably also check that the name corresponds to a
  plain function or a member function within the same class and that
  the parameter types match.
 */
int Fl_Type::user_defined(const char* cbname) const {
  for (Fl_Type* p = Fl_Type::first; p ; p = p->next)
    if (strcmp(p->type_name(), "Function") == 0 && p->name() != 0)
      if (strncmp(p->name(), cbname, strlen(cbname)) == 0)
        if (p->name()[strlen(cbname)] == '(')
          return 1;
  return 0;
}


//
// End of "$Id$".
//
