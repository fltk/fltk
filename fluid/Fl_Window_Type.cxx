/*	Fl_Window_Type.C

	The widget describing an Fl_Window.  This is also all the code
	for interacting with the overlay, which allows the user to
	select, move, and resize the children widgets.

*/

#include <FL/Fl.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/fl_message.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Menu_Item.H>
#include "Fl_Widget_Type.H"
#include <math.h>
#include <stdlib.h>
#include "alignment_panel.H"
#include <stdio.h>

int gridx = 5;
int gridy = 5;
int snap = 3;

int include_H_from_C;

void alignment_cb(Fl_Input *i, long v) {
  int n = atoi(i->value());
  if (n < 0) n = 0;
  switch (v) {
  case 1: gridx = n; break;
  case 2: gridy = n; break;
  case 3: snap  = n; break;
  }
}

extern const char* header_file_name;
extern const char* code_file_name;

void show_alignment_cb(Fl_Widget *, void *) {
  if(alignment_window==0) make_alignment_window();
  include_H_from_C_button->value(include_H_from_C);
  header_file_input->value(header_file_name);
  code_file_input->value(code_file_name);
  char buf[128];
  sprintf(buf,"%d",gridx); horizontal_input->value(buf);
  sprintf(buf,"%d",gridy); vertical_input->value(buf);
  sprintf(buf,"%d",snap); snap_input->value(buf);
  alignment_window->show();
}

void header_input_cb(Fl_Input* i, void*) {
  header_file_name = i->value();
}
void code_input_cb(Fl_Input* i, void*) {
  code_file_name = i->value();
}

void include_H_from_C_button_cb(Fl_Light_Button* b, void*) {
  include_H_from_C = b->value();
}

////////////////////////////////////////////////////////////////

static Fl_Menu_Item window_type_menu[] = {
  {"Single",0,0,(void*)FL_WINDOW},
  {"Double",0,0,(void*)(FL_WINDOW+1)},
  {0}};
class Fl_Window_Type : public Fl_Widget_Type {
  Fl_Menu_Item* subtypes() {return window_type_menu;}

  friend class Overlay_Window;
  int mx,my;		// mouse position during dragging
  int x1,y1;		// initial position of selection box
  int bx,by,br,bt;	// bounding box of selection
  int dx,dy;
  int drag;		// which parts of bbox are being moved
  int numselected;	// number of children selected
  enum {LEFT=1,RIGHT=2,BOTTOM=4,TOP=8,DRAG=16,BOX=32};
  void draw_overlay();
  void newdx();
  void newposition(Fl_Widget_Type *,int &x,int &y,int &w,int &h);
  int handle(int);
  virtual void setlabel(const char *);
  void write_code1();
  void write_code2();
  Fl_Widget_Type *_make() {return 0;} // we don't call this
  Fl_Widget *widget(int,int,int,int) {return 0;}
  int recalc;		// set by fix_overlay()

public:

  uchar modal, non_modal;

  Fl_Type *make();
  virtual const char *type_name() {return "Fl_Window";}

  void open();

  void fix_overlay();	// update the bounding box, etc

  virtual void write_properties();
  virtual void read_property(const char *);
  virtual int read_fdesign(const char*, const char*);

  void add_child(Fl_Type*, Fl_Type*);
  void move_child(Fl_Type*, Fl_Type*);
  void remove_child(Fl_Type*);

  int is_parent() const {return 1;}
  int is_group() const {return 1;}
  int is_window() const {return 1;}
};

static int overlays_invisible;

// The following Fl_Widget is used to simulate the windows.  It has
// an overlay for the fluid ui, and special-cases the FL_NO_BOX.

class Overlay_Window : public Fl_Overlay_Window {
  void draw();
  void draw_overlay();
public:
  Fl_Window_Type *window;
  int handle(int);
  Overlay_Window(int w,int h) : Fl_Overlay_Window(w,h) {Fl_Group::current(0);}
  void resize(int,int,int,int);
};
void Overlay_Window::draw() {
  const int CHECKSIZE = 8;
  // see if box is clear or a frame or rounded:
  if ((damage()&FL_DAMAGE_ALL) &&
      (!box() || (box()>=4&&!(box()&2)) || box()>=_FL_ROUNDED_BOX)) {
    // if so, draw checkerboard so user can see what areas are clear:
    for (int y = 0; y < h(); y += CHECKSIZE) 
      for (int x = 0; x < w(); x += CHECKSIZE) {
	fl_color(((y/(2*CHECKSIZE))&1) != ((x/(2*CHECKSIZE))&1) ?
		 FL_WHITE : FL_BLACK);
	fl_rectf(x,y,CHECKSIZE,CHECKSIZE);
      }
  }
  Fl_Overlay_Window::draw();
}

void Overlay_Window::draw_overlay() {
  window->draw_overlay();
}
int Overlay_Window::handle(int e) {
  return window->handle(e);
}

Fl_Type *Fl_Window_Type::make() {
  Fl_Type *p = Fl_Type::current;
  while (p && !p->is_code_block()) p = p->parent;
  if (!p) {
    fl_message("Please select a function");
    return 0;
  }
  Fl_Window_Type *o = new Fl_Window_Type();
  if (!this->o) {// template widget
    this->o = new Fl_Window(100,100);
    Fl_Group::current(0);
  }
  o->factory = this;
  o->drag = 0;
  o->numselected = 0;
  Overlay_Window *w = new Overlay_Window(100,100);
  w->window = o;
  o->o = w;
  o->add(p);
  o->modal = 0;
  o->non_modal = 0;
  return o;
}

void Fl_Window_Type::add_child(Fl_Type* cc, Fl_Type* before) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  Fl_Widget* b = before ? ((Fl_Widget_Type*)before)->o : 0;
  ((Fl_Window*)o)->insert(*(c->o), b);
  o->redraw();
}

void Fl_Window_Type::remove_child(Fl_Type* cc) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  ((Fl_Window*)o)->remove(c->o);
  o->redraw();
}

void Fl_Window_Type::move_child(Fl_Type* cc, Fl_Type* before) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  ((Fl_Window*)o)->remove(c->o);
  Fl_Widget* b = before ? ((Fl_Widget_Type*)before)->o : 0;
  ((Fl_Window*)o)->insert(*(c->o), b);
  o->redraw();
}

////////////////////////////////////////////////////////////////

// Double-click on window widget shows the window, or if already shown,
// it shows the control panel.
void Fl_Window_Type::open() {
  Overlay_Window *w = (Overlay_Window *)o;
  if (w->shown()) {
    w->show();
    Fl_Widget_Type::open();
  } else {
    Fl_Widget *p = w->resizable();
    if (!p) w->resizable(w);
    w->show();
    w->resizable(p);
  }
}

// control panel items:
#include "widget_panel.H"

void modal_cb(Fl_Light_Button* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_window()) {i->hide(); return;}
    i->show();
    i->value(((Fl_Window_Type *)current_widget)->modal);
  } else {
    ((Fl_Window_Type *)current_widget)->modal = i->value();
  }
}

void non_modal_cb(Fl_Light_Button* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_window()) {i->hide(); return;}
    i->show();
    i->value(((Fl_Window_Type *)current_widget)->non_modal);
  } else {
    ((Fl_Window_Type *)current_widget)->non_modal = i->value();
  }
}

void border_cb(Fl_Light_Button* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_window()) {i->hide(); return;}
    i->show();
    i->value(((Fl_Window*)(current_widget->o))->border());
  } else {
    ((Fl_Window*)(current_widget->o))->border(i->value());
  }
}

void xclass_cb(Fl_Input* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_window()) {i->hide(); return;}
    i->show();
    i->value(((Fl_Widget_Type *)current_widget)->xclass);
  } else {
    for (Fl_Type *o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* w = (Fl_Widget_Type*)o;
	if (w->is_window() || w->is_button())
	  storestring(i->value(),w->xclass);
	if (w->is_window()) ((Fl_Window*)(w->o))->xclass(w->xclass);
	else if (w->is_menu_item()) w->redraw();
      }
  }
}

////////////////////////////////////////////////////////////////

void Fl_Window_Type::setlabel(const char *n) {
  if (o) ((Fl_Window *)o)->label(n);
}

// make() is called on this widget when user picks window off New menu:
Fl_Window_Type Fl_Window_type;

// Resize from window manager, try to resize it back to a legal size.
// This is not proper X behavior, but works on 4DWM and fvwm
void Overlay_Window::resize(int X,int Y,int W,int H) {
  if (!visible() || W==w() && H==h()) {
    Fl_Overlay_Window::resize(X,Y,W,H);
    return;
  }
  int nw = gridx&&W!=w() ? ((W+gridx/2)/gridx)*gridx : W;
  int nh = gridy&&H!=h() ? ((H+gridy/2)/gridy)*gridy : H;
  Fl_Widget* t = resizable(); resizable(0);
  Fl_Overlay_Window::resize(X,Y,W,H);
  resizable(t);
  // make sure new window size surrounds the widgets:
  int b = 0;
  int r = 0;
  for (Fl_Type *o=window->next; o && o->level>window->level; o=o->next)
    if (o->is_widget() && !o->is_menu_item()) {
      Fl_Widget* w = ((Fl_Widget_Type*)o)->o;
      if (w->x()+w->w() > r) r = w->x()+w->w();
      if (w->y()+w->h() > b) b = w->y()+w->h();
    }
  if (nh < b) nh = b;
  if (nw < r) nw = r;
  // If changed, tell the window manager.  Skip really big windows
  // that might be bigger than screen:
  if (nw != W && nw < Fl::w()-100 || nh != H && nh < Fl::h()-100) size(nw,nh);
}

// calculate actual move by moving mouse position (mx,my) to
// nearest multiple of gridsize, and snap to original position
void Fl_Window_Type::newdx() {
  int dx, dy;
  if (Fl::event_state(FL_ALT)) {
    dx = mx-x1;
    dy = my-y1;
  } else {
    int dx0 = mx-x1;
    int ix = (drag&RIGHT) ? br : bx;
    dx = gridx ? ((ix+dx0+gridx/2)/gridx)*gridx - ix : dx0;
    if (dx0 > snap) {
      if (dx < 0) dx = 0;
    } else if (dx0 < -snap) {
      if (dx > 0) dx = 0;
    } else 
      dx = 0;
    int dy0 = my-y1;
    int iy = (drag&BOTTOM) ? by : bt;
    dy = gridy ? ((iy+dy0+gridy/2)/gridy)*gridy - iy : dy0;
    if (dy0 > snap) {
      if (dy < 0) dy = 0;
    } else if (dy0 < -snap) {
      if (dy > 0) dy = 0;
    } else 
      dy = 0;
  }
  if (this->dx != dx || this->dy != dy) {
    this->dx = dx; this->dy = dy;
    ((Overlay_Window *)(this->o))->redraw_overlay();
  }
}

// Move a widget according to dx and dy calculated above
void Fl_Window_Type::newposition(Fl_Widget_Type *o,int &X,int &Y,int &R,int &T) {
  X = o->o->x();
  Y = o->o->y();
  R = X+o->o->w();
  T = Y+o->o->h();
  if (!drag) return;
  if (drag&DRAG) {
    X += dx;
    Y += dy;
    R += dx;
    T += dy;
  } else {
    if (drag&LEFT) if (X==bx) X += dx; else if (X<bx+dx) X = bx+dx;
    if (drag&BOTTOM) if (Y==by) Y += dy; else if (Y<by+dy) Y = by+dy;
    if (drag&RIGHT) if (R==br) R += dx; else if (R>br+dx) R = br+dx;
    if (drag&TOP) if (T==bt) T += dy; else if (T>bt+dx) T = bt+dx;
  }
  if (R<X) {int n = X; X = R; R = n;}
  if (T<Y) {int n = Y; Y = T; T = n;}
}

void Fl_Window_Type::draw_overlay() {
  if (recalc) {
    bx = o->w(); by = o->h(); br = 0; bt = 0;
    numselected = 0;
    for (Fl_Type *q=next; q && q->level>level; q=q->next)
      if (q->selected && q->is_widget() && !q->is_menu_item()) {
	numselected++;
	Fl_Widget_Type* o = (Fl_Widget_Type*)q;
	if (o->o->x() < bx) bx = o->o->x();
	if (o->o->y() < by) by = o->o->y();
	if (o->o->x()+o->o->w() > br) br = o->o->x()+o->o->w();
	if (o->o->y()+o->o->h() > bt) bt = o->o->y()+o->o->h();
      }
    recalc = 0;
  }
  fl_color(FL_RED);
  if (drag==BOX && (x1 != mx || y1 != my)) {
    int x = x1; int r = mx; if (x > r) {x = mx; r = x1;}
    int y = y1; int b = my; if (y > b) {y = my; b = y1;}
    fl_rect(x,y,r-x,b-y);
  }
  if (overlays_invisible) return;
  if (selected) fl_rect(0,0,o->w(),o->h());
  if (!numselected) return;
  int bx,by,br,bt;
  bx = o->w(); by = o->h(); br = 0; bt = 0;
  for (Fl_Type *q=next; q && q->level>level; q = q->next)
    if (q->selected && q->is_widget() && !q->is_menu_item()) {
      Fl_Widget_Type* o = (Fl_Widget_Type*)q;
      int x,y,r,t;
      newposition(o,x,y,r,t);
      fl_rect(x,y,r-x,t-y);
      if (x < bx) bx = x;
      if (y < by) by = y;
      if (r > br) br = r;
      if (t > bt) bt = t;
    }
  if (selected) return;
  if (numselected>1) fl_rect(bx,by,br-bx,bt-by);
  fl_rectf(bx,by,5,5);
  fl_rectf(br-5,by,5,5);
  fl_rectf(br-5,bt-5,5,5);
  fl_rectf(bx,bt-5,5,5);
}

// Calculate new bounding box of selected widgets:
void Fl_Window_Type::fix_overlay() {
  overlays_invisible = 0;
  recalc = 1;
  ((Overlay_Window *)(this->o))->redraw_overlay();
}

// do that for every window (when selected set changes):
void redraw_overlays() {
  for (Fl_Type *o=Fl_Type::first; o; o=o->next)
    if (o->is_window()) ((Fl_Window_Type*)o)->fix_overlay();
}

void toggle_overlays(Fl_Widget *,void *) {
  overlays_invisible = !overlays_invisible;
  for (Fl_Type *o=Fl_Type::first; o; o=o->next)
    if (o->is_window()) {
      Fl_Widget_Type* w = (Fl_Widget_Type*)o;
      ((Overlay_Window*)(w->o))->redraw_overlay();
    }
}

extern void select(Fl_Type *,int);
extern void select_only(Fl_Type *);
extern void deselect();
extern Fl_Type* in_this_only;
extern void fix_group_size(Fl_Type *t);

extern Fl_Menu_Item Main_Menu[];
extern Fl_Menu_Item New_Menu[];

int Fl_Window_Type::handle(int event) {
  static Fl_Type* selection;
  switch (event) {
  case FL_PUSH:
    x1 = mx = Fl::event_x();
    y1 = my = Fl::event_y();
    drag = 0;
    // test for popup menu:
    if (Fl::event_button() >= 3) {
      in_this_only = this; // modifies how some menu items work.
      static const Fl_Menu_Item* prev;
      const Fl_Menu_Item* m = New_Menu->popup(mx,my,"New",prev);
      if (m && m->callback()) {prev = m; m->do_callback(this->o);}
      in_this_only = 0;
      return 1;
    }
    // find the innermost item clicked on:
    selection = this;
    {for (Fl_Type* i=next; i && i->level>level; i=i->next)
      if (i->is_widget() && !i->is_menu_item()) {
      Fl_Widget_Type* o = (Fl_Widget_Type*)i;
      for (Fl_Widget *o1 = o->o; o1; o1 = o1->parent())
	if (!o1->visible()) goto CONTINUE2;
      if (Fl::event_inside(o->o)) selection = o;
    CONTINUE2:;
    }}
    // do object-specific selection of other objects:
    {Fl_Type* t = selection->click_test(mx, my);
    if (t) {
      //if (t == selection) return 1; // indicates mouse eaten w/o change
      if (Fl::event_state(FL_SHIFT)) {
	Fl::event_is_click(0);
	select(t, !t->selected);
      } else {
	deselect();
	select(t, 1);
	if (t->is_menu_item()) t->open();
      }
      selection = t;
      return 1;
    }}
    // see if user grabs edges of selected region:
    if (numselected && !overlays_invisible && !(Fl::event_state(FL_SHIFT)) &&
	mx<=br+snap && mx>=bx-snap && my<=bt+snap && my>=by-snap) {
      int snap1 = snap>5 ? snap : 5;
      int w1 = (br-bx)/4; if (w1 > snap1) w1 = snap1;
      if (mx>=br-w1) drag |= RIGHT;
      else if (mx<bx+w1) drag |= LEFT;
      w1 = (bt-by)/4; if (w1 > snap1) w1 = snap1;
      if (my<=by+w1) drag |= BOTTOM;
      else if (my>bt-w1) drag |= TOP;
      if (!drag) drag = DRAG;
    } else {
      drag = BOX; // start a new selection region
    }
    return 1;
  case FL_DRAG:
    if (!drag) return 0;
    mx = Fl::event_x();
    my = Fl::event_y();
    newdx();
    return 1;
  case FL_RELEASE:
    if (!drag) return 0;
    mx = Fl::event_x();
    my = Fl::event_y();
    newdx();
    if (drag != BOX && (dx || dy || !Fl::event_is_click())) {
      if (dx || dy) {
	Fl_Type *i;
	for (i=next; i && i->level>level;) {
	  if (i->selected && i->is_widget() && !i->is_menu_item()) {
	    Fl_Widget_Type* o = (Fl_Widget_Type*)i;
	    int x,y,r,t;
	    newposition(o,x,y,r,t);
	    o->o->resize(x,y,r-x,t-y);
	    // move all the children, whether selected or not:
	    Fl_Type* p;
	    for (p = o->next; p && p->level>o->level; p = p->next)
	      if (p->is_widget() && !p->is_menu_item()) {
		Fl_Widget_Type* o = (Fl_Widget_Type*)p;
		int x,y,r,t;
		newposition(o,x,y,r,t);
		o->o->resize(x,y,r-x,t-y);
	      }
	    i = p;
	  } else {
	    i = i->next;
	  }
	}
	for (i=next; i && i->level>level; i=i->next) fix_group_size(i);
	this->o->redraw();
	fix_overlay();
	modflag = 1;
      }
    } else if ((Fl::event_clicks() || Fl::event_state(FL_CTRL))) {
      Fl_Widget_Type::open();
    } else {
      if (mx<x1) {int t = x1; x1 = mx; mx = t;}
      if (my<y1) {int t = y1; y1 = my; my = t;}
      int n = 0;
      int toggle = Fl::event_state(FL_SHIFT);
      // clear selection on everything:
      if (!toggle) deselect(); else Fl::event_is_click(0);
      // select everything in box:
      for (Fl_Type*i=next; i&&i->level>level; i=i->next)
	if (i->is_widget() && !i->is_menu_item()) {
	Fl_Widget_Type* o = (Fl_Widget_Type*)i;
	for (Fl_Widget *o1 = o->o; o1; o1 = o1->parent())
	  if (!o1->visible()) goto CONTINUE;
	if (Fl::event_inside(o->o)) selection = o;
	if (o->o->x()>=x1 && o->o->y()>y1 &&
	    o->o->x()+o->o->w()<mx && o->o->y()+o->o->h()<my) {
	  n++;
	  select(o, toggle ? !o->selected : 1);
	}
      CONTINUE:;
      }
      // if nothing in box, select what was clicked on:
      if (!n) {
	select(selection, toggle ? !selection->selected : 1);
      }
    }
    drag = 0;
    return 1;

  case FL_KEYBOARD: {
    if (Fl::event_key() == FL_Escape) {((Fl_Window*)o)->hide(); return 1;}
    // find current child:
    Fl_Type *i = Fl_Type::current;
    while (i && (!i->is_widget() || i->is_menu_item())) i = i->parent;
    if (!i) return 0;
    Fl_Type *p = i->parent;
    while (p && p != this) p = p->parent;
    if (!p || !p->is_widget()) {i=next; if (!i || i->level <= level) return 0;}
    p = i;
    // try to navigate to another child:
    for (;;) {
      switch (Fl::event_key()) {
      case FL_Tab:
	if (Fl::event_state(FL_SHIFT)) goto LEFT;
      case FL_Right:
      case FL_Down:
	i = i->next; break;
      case FL_Left:
      case FL_Up:
      LEFT:
	i = i->prev; break;
      default:
	return 0;
      }
      if (!i || i->level <= level) {i = p; break;}
      if (!i->is_widget() || i->is_menu_item()) continue;
      switch (Fl::event_key()) {
      case FL_Up:
      case FL_Down: if (p->is_widget() && !p->is_menu_item()) {
	Fl_Widget* w = ((Fl_Widget_Type*)i)->o;
	Fl_Widget* pw = ((Fl_Widget_Type*)p)->o;
	if (w->x() >= pw->x()+pw->w() ||
	    w->x()+w->w() <= pw->x()) continue;
      }}
      break;
    }
    // select it:
    deselect(); select(i,1);
    } return 1;

  case FL_SHORTCUT: {
    in_this_only = this; // modifies how some menu items work.
    const Fl_Menu_Item* m = Main_Menu->test_shortcut();
    if (m && m->callback()) m->do_callback(this->o);
    in_this_only = 0;
    return (m != 0);
  }
  default:
    return 0;
  }
}

////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>

void Fl_Window_Type::write_code1() {
  Fl_Widget_Type::write_code1();
}

void Fl_Window_Type::write_code2() {
  if (modal) write_c("%so->set_modal();\n", indent());
  else if (non_modal) write_c("%so->set_non_modal();\n", indent());
  if (!((Fl_Window*)o)->border()) write_c("%so->clear_border();\n", indent());
  write_c("%so->end();\n", indent());
  if (((Fl_Window*)o)->resizable() == o)
    write_c("%so->resizable(o);\n", indent());
  Fl_Widget_Type::write_code2();
}

void Fl_Window_Type::write_properties() {
  Fl_Widget_Type::write_properties();
  if (modal) write_string("modal");
  else if (non_modal) write_string("non_modal");
  if (!((Fl_Window*)o)->border()) write_string("noborder");
  if (xclass) {write_string("xclass"); write_word(xclass);}
  if (o->visible()) write_string("visible");
}

void Fl_Window_Type::read_property(const char *c) {
  if (!strcmp(c,"modal")) {
    modal = 1;
  } else if (!strcmp(c,"non_modal")) {
    non_modal = 1;
  } else if (!strcmp(c, "visible")) {
    if (Fl::first_window()) open(); // only if we are using user interface
  } else if (!strcmp(c,"noborder")) {
    ((Fl_Window*)o)->border(0);
  } else if (!strcmp(c,"xclass")) {
    storestring(read_word(),xclass);
    ((Fl_Window*)o)->xclass(xclass);
  } else {
    Fl_Widget_Type::read_property(c);
  }
}

int Fl_Window_Type::read_fdesign(const char* name, const char* value) {
  int x;
  o->box(FL_NO_BOX); // because fdesign always puts an Fl_Box next
  if (!strcmp(name,"Width")) {
    if (sscanf(value,"%d",&x) == 1) o->size(x,o->h());
  } else if (!strcmp(name,"Height")) {
    if (sscanf(value,"%d",&x) == 1) o->size(o->w(),x);
  } else if (!strcmp(name,"NumberofWidgets")) {
    return 1; // we can figure out count from file
  } else if (!strcmp(name,"border")) {
    if (sscanf(value,"%d",&x) == 1) ((Fl_Window*)o)->border(x);
  } else if (!strcmp(name,"title")) {
    label(value);
  } else {
    return Fl_Widget_Type::read_fdesign(name,value);
  }
  return 1;
}
