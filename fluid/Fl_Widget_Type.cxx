//
// "$Id$"
//
// Widget type code for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Group.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Input.H>
#include "Fl_Widget_Type.h"
#include "alignment_panel.h"
#include <FL/fl_message.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Window.H>
#include "../src/flstring.h"
#include <stdio.h>
#include <stdlib.h>

// Make an Fl_Widget_Type subclass instance.
// It figures out the automatic size and parent of the new widget,
// creates the Fl_Widget (by calling the virtual function _make),
// adds it to the Fl_Widget hierarchy, creates a new Fl_Type
// instance, sets the widget pointers, and makes all the display
// update correctly...

extern int reading_file;
int force_parent;
extern int gridx;
extern int gridy;
extern int i18n_type;
extern const char* i18n_include;
extern const char* i18n_function;
extern const char* i18n_file;
extern const char* i18n_set;

int Fl_Widget_Type::default_size = FL_NORMAL_SIZE;

int Fl_Widget_Type::is_widget() const {return 1;}
int Fl_Widget_Type::is_public() const {return public_;}

const char* subclassname(Fl_Type* l) {
  if (l->is_widget()) {
    Fl_Widget_Type* p = (Fl_Widget_Type*)l;
    const char* c = p->subclass();
    if (c) return c;
    if (l->is_class()) return "Fl_Group";
    if (p->o->type() == FL_WINDOW+1) return "Fl_Double_Window";
    if (strcmp(p->type_name(), "Fl_Input") == 0) {
      if (p->o->type() == FL_FLOAT_INPUT) return "Fl_Float_Input";
      if (p->o->type() == FL_INT_INPUT) return "Fl_Int_Input";
    }
  }
  return l->type_name();
}

// Return the ideal widget size...
void
Fl_Widget_Type::ideal_size(int &w, int &h) {
  h = o->labelsize();
  o->measure_label(w, h);

  w += Fl::box_dw(o->box());
  h += Fl::box_dh(o->box());

  if (w < 15) w = 15;
  if (h < 15) h = 15;
}

// Return the ideal widget spacing...
void
Fl_Widget_Type::ideal_spacing(int &x, int &y) {
  if (o->labelsize() < 10)
    x = y = 0;
  else if (o->labelsize() < 14)
    x = y = 5;
  else
    x = y = 10;
}

Fl_Type *Fl_Widget_Type::make() {
  // Find the current widget, or widget to copy:
  Fl_Type *qq = Fl_Type::current;
  while (qq && (!qq->is_widget() || qq->is_menu_item())) qq = qq->parent;
  if (!qq) {
    fl_message("Please select a widget");
    return 0;
  }
  Fl_Widget_Type* q = (Fl_Widget_Type*)qq;
  // find the parent widget:
  Fl_Widget_Type* p = q;
  if ((force_parent || !p->is_group()) && p->parent->is_widget())
    p = (Fl_Widget_Type*)(p->parent);
  force_parent = 0;

  // Figure out a border between widget and window:
  int B = p->o->w()/2; if (p->o->h()/2 < B) B = p->o->h()/2; if (B>25) B = 25;

  int ULX,ULY; // parent's origin in window
  if (!p->is_window()) { // if it is a group, add corner
    ULX = p->o->x(); ULY = p->o->y();
  } else {
    ULX = ULY = 0;
  }

  // Figure out a position and size for the widget
  int X,Y,W,H;
  if (is_group()) {	// fill the parent with the widget
    X = ULX+B;
    W = p->o->w()-B;
    Y = ULY+B;
    H = p->o->h()-B;
  } else if (q != p) {	// copy position and size of current widget
    W = q->o->w();
    H = q->o->h();
    X = q->o->x()+W;
    Y = q->o->y();
    if (X+W > ULX+p->o->w()) {
      X = q->o->x();
      Y = q->o->y()+H;
      if (Y+H > ULY+p->o->h()) Y = ULY+B;
    }
  } else {	// just make it small and square...
    X = ULX+B;
    Y = ULY+B;
    W = H = B;
  }

  // satisfy the grid requirements (otherwise it edits really strangely):
  if (gridx>1) {
    X = (X/gridx)*gridx;
    W = ((W-1)/gridx+1)*gridx;
  }
  if (gridy>1) {
    Y = (Y/gridy)*gridy;
    H = ((H-1)/gridy+1)*gridy;
  }

  // Construct the Fl_Type:
  Fl_Widget_Type *t = _make();
  if (!o) o = widget(0,0,100,100); // create template widget
  t->factory = this;
  // Construct the Fl_Widget:
  t->o = widget(X,Y,W,H);
  if (reading_file) t->o->label(0);
  else if (t->o->label()) t->label(t->o->label()); // allow editing
  t->o->user_data((void*)t);
  // Put it in the parent:
  //  ((Fl_Group *)(p->o))->add(t->o); (done by Fl_Type::add())
  // add to browser:
  t->add(p);
  t->redraw();
  return t;
}

#include "Fluid_Image.h"

void Fl_Widget_Type::setimage(Fluid_Image *i) {
  if (i == image || is_window()) return;
  if (image) image->decrement();
  if (i) i->increment();
  image = i;
  if (i) i->image(o);
  else o->image(0);
  redraw();
}

void Fl_Widget_Type::setinactive(Fluid_Image *i) {
  if (i == inactive || is_window()) return;
  if (inactive) inactive->decrement();
  if (i) i->increment();
  inactive = i;
  if (i) i->deimage(o);
  else o->deimage(0);
  redraw();
}

void Fl_Widget_Type::setlabel(const char *n) {
  o->label(n);
  redraw();
}

Fl_Widget_Type::Fl_Widget_Type() {
  for (int n=0; n<NUM_EXTRA_CODE; n++) {extra_code_[n] = 0; }
  subclass_ = 0;
  hotspot_ = 0;
  tooltip_ = 0;
  image_name_ = 0;
  inactive_name_ = 0;
  image = 0;
  inactive = 0;
  xclass = 0;
  o = 0;
  public_ = 1;
}

Fl_Widget_Type::~Fl_Widget_Type() {
  if (o) {
    o->hide();
    if (o->parent()) ((Fl_Group*)o->parent())->remove(*o);
    delete o;
  }
  if (subclass_) free((void*)subclass_);
  if (tooltip_) free((void*)tooltip_);
  if (image_name_) {
    free((void*)image_name_);
    if (image) image->decrement();
  }
  if (inactive_name_) {
    free((void*)inactive_name_);
    if (inactive) inactive->decrement();
  }
  for (int n=0; n<NUM_EXTRA_CODE; n++) {
    if (extra_code_[n]) free((void*) extra_code_[n]);
  }
}

void Fl_Widget_Type::extra_code(int m,const char *n) {
  storestring(n,extra_code_[m]);
}

extern void redraw_browser();
void Fl_Widget_Type::subclass(const char *n) {
  if (storestring(n,subclass_) && visible)
    redraw_browser();
}

void Fl_Widget_Type::tooltip(const char *n) {
  storestring(n,tooltip_);
  o->tooltip(n);
}

void Fl_Widget_Type::image_name(const char *n) {
  setimage(Fluid_Image::find(n));
  storestring(n,image_name_);
}

void Fl_Widget_Type::inactive_name(const char *n) {
  setinactive(Fluid_Image::find(n));
  storestring(n,inactive_name_);
}

void Fl_Widget_Type::redraw() {
  Fl_Type *t = this;
  if (is_menu_item()) {
    // find the menu button that parents this menu:
    do t = t->parent; while (t && t->is_menu_item());
    // kludge to cause build_menu to be called again:
    t->add_child(0,0);
  } else {
    while (t->parent && t->parent->is_widget()) t = t->parent;
    ((Fl_Widget_Type*)t)->o->redraw();
  }
}

// the recursive part sorts all children, returns pointer to next:
Fl_Type *sort(Fl_Type *parent) {
  Fl_Type *f,*n=0;
  for (f = parent ? parent->next : Fl_Type::first; ; f = n) {
    if (!f || (parent && f->level <= parent->level)) return f;
    n = sort(f);
    if (!f->selected || (!f->is_widget() || f->is_menu_item())) continue;
    Fl_Widget* fw = ((Fl_Widget_Type*)f)->o;
    Fl_Type *g; // we will insert before this
    for (g = parent->next; g != f; g = g->next) {
      if (!g->selected || g->level > f->level) continue;
      Fl_Widget* gw = ((Fl_Widget_Type*)g)->o;
      if (gw->y() > fw->y()) break;
      if (gw->y() == fw->y() && gw->x() > fw->x()) break;
    }
    if (g != f) f->move_before(g);
  }
}

////////////////////////////////////////////////////////////////
// The control panels!

#include "widget_panel.h"
#include <FL/fl_show_colormap.H>

static Fl_Window *the_panel;

// All the callbacks use the argument to indicate whether to load or store.
// This avoids the need for pointers to all the widgets, and keeps the
// code localized in the callbacks.
// A value of LOAD means to load.  The hope is that this will not collide
// with any actual useful values for the argument.  I also use this to
// initialized parts of the widget that are nyi by fluid.

Fl_Widget_Type *current_widget; // one of the selected ones
void* const LOAD = (void *)"LOAD"; // "magic" pointer to indicate that we need to load values into the dialog
static int numselected; // number selected
static int haderror;

void name_cb(Fl_Input* o, void *v) {
  if (v == LOAD) {
    static char buf[1024];
    if (numselected != 1) {
      snprintf(buf, sizeof(buf), "Widget Properties (%d widgets)", numselected);
      o->hide();
    } else {
      o->static_value(current_widget->name());
      o->show();
      snprintf(buf, sizeof(buf), "%s Properties", current_widget->title());
    }

    the_panel->label(buf);
  } else {
    if (numselected == 1) {
      current_widget->name(o->value());
      // I don't update window title, as it probably is being closed
      // and wm2 (a window manager) barfs if you retitle and then
      // hide a window:
      // ((Fl_Window*)(o->parent()->parent()->parent()))->label(current_widget->title());
    }
  }
}

void name_public_member_cb(Fl_Choice* i, void* v) {
  if (v == LOAD) {
    i->value(current_widget->public_);
    if (current_widget->is_in_class()) i->show(); else i->hide();
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        Fl_Widget_Type *w = ((Fl_Widget_Type*)o);
        if (w->is_in_class()) {
          w->public_ = i->value();
        } else {
          // if this is not in a class, it can be only private or public
          w->public_ = (i->value()>0);
        }
	mod = 1;
      }
    }
    if (mod) {
      set_modflag(1);
      redraw_browser();
    }
  }
}    

void name_public_cb(Fl_Choice* i, void* v) {
  if (v == LOAD) {
    i->value(current_widget->public_>0);
    if (current_widget->is_in_class()) i->hide(); else i->show();
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	((Fl_Widget_Type*)o)->public_ = i->value();
	mod = 1;
      }
    }
    if (mod) {
      set_modflag(1);
      redraw_browser();
    }
  }
}    

static char* oldlabel;
static unsigned oldlabellen;

void label_cb(Fl_Input* i, void *v) {
  if (v == LOAD) {
    i->static_value(current_widget->label());
    if (strlen(i->value()) >= oldlabellen) {
      oldlabellen = strlen(i->value())+128;
      oldlabel = (char*)realloc(oldlabel,oldlabellen);
    }
    strcpy(oldlabel,i->value());
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        o->label(i->value());
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

static Fl_Input *image_input;

void image_cb(Fl_Input* i, void *v) {
  if (v == LOAD) {
    image_input = i;
    if (current_widget->is_widget() && !current_widget->is_window()) {
      i->activate();
      i->static_value(((Fl_Widget_Type*)current_widget)->image_name());
    } else i->deactivate();
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        ((Fl_Widget_Type*)o)->image_name(i->value());
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void image_browse_cb(Fl_Button* b, void *v) {
  if (v == LOAD) {
    if (current_widget->is_widget() && !current_widget->is_window())
      b->activate();
    else 
      b->deactivate();
  } else {
    int mod = 0;
    if (ui_find_image(image_input->value())) {
      image_input->value(ui_find_image_name);
      for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
	if (o->selected && o->is_widget()) {
	  ((Fl_Widget_Type*)o)->image_name(ui_find_image_name);
	  mod = 1;
	}
      }
      if (mod) set_modflag(1);
    }
  }
}

static Fl_Input *inactive_input;

void inactive_cb(Fl_Input* i, void *v) {
  if (v == LOAD) {
    inactive_input = i;
    if (current_widget->is_widget() && !current_widget->is_window()) {
      i->activate();
      i->static_value(((Fl_Widget_Type*)current_widget)->inactive_name());
    } else i->deactivate();
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        ((Fl_Widget_Type*)o)->inactive_name(i->value());
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void inactive_browse_cb(Fl_Button* b, void *v) {
  if (v == LOAD) {
    if (current_widget->is_widget() && !current_widget->is_window()) 
      b->activate();
    else 
      b->deactivate();
  } else {
    int mod = 0;
    if (ui_find_image(inactive_input->value())) {
      inactive_input->value(ui_find_image_name);
      for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
	if (o->selected && o->is_widget()) {
	  ((Fl_Widget_Type*)o)->inactive_name(ui_find_image_name);
	  mod = 1;
	}
      }
      if (mod) set_modflag(1);
    }
  }
}

void tooltip_cb(Fl_Input* i, void *v) {
  if (v == LOAD) {
    if (current_widget->is_widget()) {
      i->activate();
      i->static_value(((Fl_Widget_Type*)current_widget)->tooltip());
    } else i->deactivate();
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        ((Fl_Widget_Type*)o)->tooltip(i->value());
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

Fl_Value_Input *x_input, *y_input, *w_input, *h_input;

void x_cb(Fl_Value_Input *i, void *v) {
  if (v == LOAD) {
    x_input = i;
    if (current_widget->is_widget()) {
      i->value(((Fl_Widget_Type *)current_widget)->o->x());
      x_input->activate();
    } else x_input->deactivate();
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
	w->resize((int)i->value(), w->y(), w->w(), w->h());
	if (w->window()) w->window()->redraw();
	if (o->is_window()) {
          ((Fl_Window *)w)->size_range(gridx, gridy, Fl::w(), Fl::h(),
                                       gridx, gridy, 0);
	}
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void y_cb(Fl_Value_Input *i, void *v) {
  if (v == LOAD) {
    y_input = i;
    if (current_widget->is_widget()) {
      i->value(((Fl_Widget_Type *)current_widget)->o->y());
      y_input->activate();
    } else y_input->deactivate();
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
	w->resize(w->x(), (int)i->value(), w->w(), w->h());
	if (w->window()) w->window()->redraw();
	if (o->is_window()) {
          ((Fl_Window *)w)->size_range(gridx, gridy, Fl::w(), Fl::h(),
                                       gridx, gridy, 0);
	}
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void w_cb(Fl_Value_Input *i, void *v) {
  if (v == LOAD) {
    w_input = i;
    if (current_widget->is_widget()) {
      i->value(((Fl_Widget_Type *)current_widget)->o->w());
      w_input->activate();
    } else w_input->deactivate();
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
	w->resize(w->x(), w->y(), (int)i->value(), w->h());
	if (w->window()) w->window()->redraw();
	if (o->is_window()) {
          ((Fl_Window *)w)->size_range(gridx, gridy, Fl::w(), Fl::h(),
                                       gridx, gridy, 0);
	}
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void h_cb(Fl_Value_Input *i, void *v) {
  if (v == LOAD) {
    h_input = i;
    if (current_widget->is_widget()) {
      i->value(((Fl_Widget_Type *)current_widget)->o->h());
      h_input->activate();
    } else h_input->deactivate();
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
	w->resize(w->x(), w->y(), w->w(), (int)i->value());
	if (w->window()) w->window()->redraw();
	if (o->is_window()) {
          ((Fl_Window *)w)->size_range(gridx, gridy, Fl::w(), Fl::h(),
                                       gridx, gridy, 0);
	}
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void wc_relative_cb(Fl_Light_Button *i, void *v) {
  if (v == LOAD) {
    if (!strcmp(current_widget->type_name(), "widget_class")) {
      i->show();
      i->value(((Fl_Widget_Class_Type *)current_widget)->wc_relative);
    } else {
      i->hide();
    }
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && !strcmp(current_widget->type_name(), "widget_class")) {
        Fl_Widget_Class_Type *t = (Fl_Widget_Class_Type *)o;
        t->wc_relative = i->value();
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

////////////////////////////////////////////////////////////////

// turn number to string or string to number for saving to file:
// does not work for hierarchial menus!

const char *item_name(Fl_Menu_Item* m, int i) {
  if (m) {
    while (m->label()) {
      if (m->argument() == i) return m->label();
      m++;
    }
  }
  static char buffer[20];
  sprintf(buffer, "%d", i);
  return buffer;
}
int item_number(Fl_Menu_Item* m, const char* i) {
  if (m && i) {
    if (i[0]=='F' && i[1]=='L' && i[2]=='_') i += 3;
    while (m->label()) {
      if (!strcmp(m->label(), i)) return int(m->argument());
      m++;
    }
  }
  return atoi(i);
}

#define ZERO_ENTRY 1000

Fl_Menu_Item boxmenu[] = {
{"NO_BOX",0,0,(void *)ZERO_ENTRY},
{"boxes",0,0,0,FL_SUBMENU},
{"UP_BOX",0,0,(void *)FL_UP_BOX},
{"DOWN_BOX",0,0,(void *)FL_DOWN_BOX},
{"FLAT_BOX",0,0,(void *)FL_FLAT_BOX},
{"BORDER_BOX",0,0,(void *)FL_BORDER_BOX},
{"THIN_UP_BOX",0,0,(void *)FL_THIN_UP_BOX},
{"THIN_DOWN_BOX",0,0,(void *)FL_THIN_DOWN_BOX},
{"ENGRAVED_BOX",0,0,(void *)FL_ENGRAVED_BOX},
{"EMBOSSED_BOX",0,0,(void *)FL_EMBOSSED_BOX},
{"ROUND_UP_BOX",0,0,(void *)FL_ROUND_UP_BOX},
{"ROUND_DOWN_BOX",0,0,(void *)FL_ROUND_DOWN_BOX},
{"DIAMOND_UP_BOX",0,0,(void *)FL_DIAMOND_UP_BOX},
{"DIAMOND_DOWN_BOX",0,0,(void *)FL_DIAMOND_DOWN_BOX},
{"SHADOW_BOX",0,0,(void *)FL_SHADOW_BOX},
{"ROUNDED_BOX",0,0,(void *)FL_ROUNDED_BOX},
{"RSHADOW_BOX",0,0,(void *)FL_RSHADOW_BOX},
{"RFLAT_BOX",0,0,(void *)FL_RFLAT_BOX},
{"OVAL_BOX",0,0,(void *)FL_OVAL_BOX},
{"OSHADOW_BOX",0,0,(void *)FL_OSHADOW_BOX},
{"OFLAT_BOX",0,0,(void *)FL_OFLAT_BOX},
{"PLASTIC_UP_BOX",0,0,(void *)FL_PLASTIC_UP_BOX},
{"PLASTIC_DOWN_BOX",0,0,(void *)FL_PLASTIC_DOWN_BOX},
{"PLASTIC_THIN_UP_BOX",0,0,(void *)FL_PLASTIC_THIN_UP_BOX},
{"PLASTIC_THIN_DOWN_BOX",0,0,(void *)FL_PLASTIC_THIN_DOWN_BOX},
{"PLASTIC_ROUND_UP_BOX",0,0,(void *)FL_PLASTIC_ROUND_UP_BOX},
{"PLASTIC_ROUND_DOWN_BOX",0,0,(void *)FL_PLASTIC_ROUND_DOWN_BOX},
{"GTK_UP_BOX",0,0,(void *)FL_GTK_UP_BOX},
{"GTK_DOWN_BOX",0,0,(void *)FL_GTK_DOWN_BOX},
{"GTK_THIN_UP_BOX",0,0,(void *)FL_GTK_THIN_UP_BOX},
{"GTK_THIN_DOWN_BOX",0,0,(void *)FL_GTK_THIN_DOWN_BOX},
{"GTK_ROUND_UP_BOX",0,0,(void *)FL_GTK_ROUND_UP_BOX},
{"GTK_ROUND_DOWN_BOX",0,0,(void *)FL_GTK_ROUND_DOWN_BOX},
{"GLEAM_UP_BOX",0,0,(void *)FL_GLEAM_UP_BOX},
{"GLEAM_DOWN_BOX",0,0,(void *)FL_GLEAM_DOWN_BOX},
{"GLEAM_THIN_UP_BOX",0,0,(void *)FL_GLEAM_THIN_UP_BOX},
{"GLEAM_THIN_DOWN_BOX",0,0,(void *)FL_GLEAM_THIN_DOWN_BOX},
{"GLEAM_ROUND_UP_BOX",0,0,(void *)FL_GLEAM_ROUND_UP_BOX},
{"GLEAM_ROUND_DOWN_BOX",0,0,(void *)FL_GLEAM_ROUND_DOWN_BOX},
{0},
{"frames",0,0,0,FL_SUBMENU},
{"UP_FRAME",0,0,(void *)FL_UP_FRAME},
{"DOWN_FRAME",0,0,(void *)FL_DOWN_FRAME},
{"THIN_UP_FRAME",0,0,(void *)FL_THIN_UP_FRAME},
{"THIN_DOWN_FRAME",0,0,(void *)FL_THIN_DOWN_FRAME},
{"ENGRAVED_FRAME",0,0,(void *)FL_ENGRAVED_FRAME},
{"EMBOSSED_FRAME",0,0,(void *)FL_EMBOSSED_FRAME},
{"BORDER_FRAME",0,0,(void *)FL_BORDER_FRAME},
{"SHADOW_FRAME",0,0,(void *)FL_SHADOW_FRAME},
{"ROUNDED_FRAME",0,0,(void *)FL_ROUNDED_FRAME},
{"OVAL_FRAME",0,0,(void *)FL_OVAL_FRAME},
{"PLASTIC_UP_FRAME",0,0,(void *)FL_PLASTIC_UP_FRAME},
{"PLASTIC_DOWN_FRAME",0,0,(void *)FL_PLASTIC_DOWN_FRAME},
{"GTK_UP_FRAME",0,0,(void *)FL_GTK_UP_FRAME},
{"GTK_DOWN_FRAME",0,0,(void *)FL_GTK_DOWN_FRAME},
{"GTK_THIN_UP_FRAME",0,0,(void *)FL_GTK_THIN_UP_FRAME},
{"GTK_THIN_DOWN_FRAME",0,0,(void *)FL_GTK_THIN_DOWN_FRAME},
{"GLEAM_UP_FRAME",0,0,(void *)FL_GLEAM_UP_FRAME},
{"GLEAM_DOWN_FRAME",0,0,(void *)FL_GLEAM_DOWN_FRAME},
{0},
{0}};

const char *boxname(int i) {
  if (!i) i = ZERO_ENTRY;
  for (int j = 0; j < int(sizeof(boxmenu)/sizeof(*boxmenu)); j++)
    if (boxmenu[j].argument() == i) return boxmenu[j].label();
  return 0;
}

int boxnumber(const char *i) {
  if (i[0]=='F' && i[1]=='L' && i[2]=='_') i += 3;
  for (int j = 0; j < int(sizeof(boxmenu)/sizeof(*boxmenu)); j++)
    if (boxmenu[j].label() && !strcmp(boxmenu[j].label(), i)) {
      return int(boxmenu[j].argument());
    }
  return 0;
}

void box_cb(Fl_Choice* i, void *v) {
  if (v == LOAD) {
    if (current_widget->is_menu_item()) {i->deactivate(); return;} else i->activate();
    int n = current_widget->o->box(); if (!n) n = ZERO_ENTRY;
    for (int j = 0; j < int(sizeof(boxmenu)/sizeof(*boxmenu)); j++)
      if (boxmenu[j].argument() == n) {i->value(j); break;}
  } else {
    int mod = 0;
    int m = i->value();
    int n = int(boxmenu[m].argument());
    if (!n) return; // should not happen
    if (n == ZERO_ENTRY) n = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
        q->o->box((Fl_Boxtype)n);
        q->redraw();
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void down_box_cb(Fl_Choice* i, void *v) {
  if (v == LOAD) {
    int n;
    if (current_widget->is_button() && !current_widget->is_menu_item())
      n = ((Fl_Button*)(current_widget->o))->down_box();
    else if (!strcmp(current_widget->type_name(), "Fl_Input_Choice"))
      n = ((Fl_Input_Choice*)(current_widget->o))->down_box();
    else if (current_widget->is_menu_button())
      n = ((Fl_Menu_*)(current_widget->o))->down_box();
    else {
      i->deactivate(); return;
    }
    i->activate();
    if (!n) n = ZERO_ENTRY;
    for (int j = 0; j < int(sizeof(boxmenu)/sizeof(*boxmenu)); j++)
      if (boxmenu[j].argument() == n) {i->value(j); break;}
  } else {
    int mod = 0;
    int m = i->value();
    int n = int(boxmenu[m].argument());
    if (!n) return; // should not happen
    if (n == ZERO_ENTRY) n = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected) {
	if (o->is_button() && !o->is_menu_item()) {
	  Fl_Widget_Type* q = (Fl_Widget_Type*)o;
          ((Fl_Button*)(q->o))->down_box((Fl_Boxtype)n);
          if (((Fl_Button*)(q->o))->value()) q->redraw();
	} else if (!strcmp(o->type_name(), "Fl_Input_Choice")) {
	  Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	  ((Fl_Input_Choice*)(q->o))->down_box((Fl_Boxtype)n);
	} else if (o->is_menu_button()) {
	  Fl_Widget_Type* q = (Fl_Widget_Type*)o;
          ((Fl_Menu_*)(q->o))->down_box((Fl_Boxtype)n);
	}
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

////////////////////////////////////////////////////////////////

Fl_Menu_Item whenmenu[] = {
  {"Never",0,0,(void*)ZERO_ENTRY},
  {"Release",0,0,(void*)FL_WHEN_RELEASE},
  {"Changed",0,0,(void*)FL_WHEN_CHANGED},
  {"Enter key",0,0,(void*)FL_WHEN_ENTER_KEY},
  //{"Release or Enter",0,0,(void*)(FL_WHEN_ENTER_KEY|FL_WHEN_RELEASE)},
  {0}};

static Fl_Menu_Item whensymbolmenu[] = {
  {"FL_WHEN_NEVER",0,0,(void*)(FL_WHEN_NEVER)},
  {"FL_WHEN_CHANGED",0,0,(void*)(FL_WHEN_CHANGED)},
  {"FL_WHEN_RELEASE",0,0,(void*)(FL_WHEN_RELEASE)},
  {"FL_WHEN_RELEASE_ALWAYS",0,0,(void*)(FL_WHEN_RELEASE_ALWAYS)},
  {"FL_WHEN_ENTER_KEY",0,0,(void*)(FL_WHEN_ENTER_KEY)},
  {"FL_WHEN_ENTER_KEY_ALWAYS",0,0,(void*)(FL_WHEN_ENTER_KEY_ALWAYS)},
  {0}};

void when_cb(Fl_Choice* i, void *v) {
  if (v == LOAD) {
    if (current_widget->is_menu_item()) {i->deactivate(); return;} else i->activate();
    int n = current_widget->o->when() & (~FL_WHEN_NOT_CHANGED);
    if (!n) n = ZERO_ENTRY;
    for (int j = 0; j < int(sizeof(whenmenu)/sizeof(*whenmenu)); j++)
      if (whenmenu[j].argument() == n) {i->value(j); break;}
  } else {
    int mod = 0;
    int m = i->value();
    int n = int(whenmenu[m].argument());
    if (!n) return; // should not happen
    if (n == ZERO_ENTRY) n = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	q->o->when(n|(q->o->when()&FL_WHEN_NOT_CHANGED));
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void when_button_cb(Fl_Light_Button* i, void *v) {
  if (v == LOAD) {
    if (current_widget->is_menu_item()) {i->deactivate(); return;} else i->activate();
    i->value(current_widget->o->when()&FL_WHEN_NOT_CHANGED);
  } else {
    int mod = 0;
    int n = i->value() ? FL_WHEN_NOT_CHANGED : 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	q->o->when(n|(q->o->when()&~FL_WHEN_NOT_CHANGED));
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

uchar Fl_Widget_Type::resizable() const {
  if (is_window()) return ((Fl_Window*)o)->resizable() != 0;
  Fl_Group* p = (Fl_Group*)o->parent();
  if (p) return p->resizable() == o;
  else return 0;
}

void Fl_Widget_Type::resizable(uchar v) {
  if (v) {
    if (resizable()) return;
    if (is_window()) ((Fl_Window*)o)->resizable(o);
    else {
      Fl_Group* p = (Fl_Group*)o->parent();
      if (p) p->resizable(o);
    }
  } else {
    if (!resizable()) return;
    if (is_window()) {
      ((Fl_Window*)o)->resizable(0);
    } else {
      Fl_Group* p = (Fl_Group*)o->parent();
      if (p) p->resizable(0);
    }
  }
}

void resizable_cb(Fl_Light_Button* i,void* v) {
  if (v == LOAD) {
    if (current_widget->is_menu_item()) {i->deactivate(); return;}
    if (numselected > 1) {i->deactivate(); return;}
    i->activate();
    i->value(current_widget->resizable());
  } else {
    current_widget->resizable(i->value());
    set_modflag(1);
  }
}

void hotspot_cb(Fl_Light_Button* i,void* v) {
  if (v == LOAD) {
    if (numselected > 1) {i->deactivate(); return;}
    if (current_widget->is_menu_item()) i->label("divider");
    else i->label("hotspot");
    i->activate();
    i->value(current_widget->hotspot());
  } else {
    current_widget->hotspot(i->value());
    if (current_widget->is_menu_item()) {current_widget->redraw(); return;}
    if (i->value()) {
      Fl_Type *p = current_widget->parent;
      if (!p || !p->is_widget()) return;
      while (!p->is_window()) p = p->parent;
      for (Fl_Type *o = p->next; o && o->level > p->level; o = o->next) {
	if (o->is_widget() && o != current_widget)
	  ((Fl_Widget_Type*)o)->hotspot(0);
      }
    }
    set_modflag(1);
  }
}

void visible_cb(Fl_Light_Button* i, void* v) {
  if (v == LOAD) {
    i->value(current_widget->o->visible());
    if (current_widget->is_window()) i->deactivate();
    else i->activate();
  } else {
    int mod = 0;
    int n = i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	n ? q->o->show() : q->o->hide();
	q->redraw();
	mod = 1;
	if (n && q->parent && q->parent->type_name()) {
	  if (!strcmp(q->parent->type_name(), "Fl_Tabs")) {
	    ((Fl_Tabs *)q->o->parent())->value(q->o);
	  } else if (!strcmp(q->parent->type_name(), "Fl_Wizard")) {
	    ((Fl_Wizard *)q->o->parent())->value(q->o);
	  }
	}
      }
    }
    if (mod) set_modflag(1);
  }
}

void active_cb(Fl_Light_Button* i, void* v) {
  if (v == LOAD) {
    i->value(current_widget->o->active());
    if (current_widget->is_window()) i->deactivate();
    else i->activate();
  } else {
    int mod = 0;
    int n = i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	n ? q->o->activate() : q->o->deactivate();
	q->redraw();
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

////////////////////////////////////////////////////////////////

Fl_Menu_Item fontmenu[] = {
{"Helvetica"},
{"Helvetica bold"},
{"Helvetica italic"},
{"Helvetica bold italic"},
{"Courier"},
{"Courier bold"},
{"Courier italic"},
{"Courier bold italic"},
{"Times"},
{"Times bold"},
{"Times italic"},
{"Times bold italic"},
{"Symbol"},
{"Terminal"},
{"Terminal Bold"},
{"Zapf Dingbats"},
{0}};

void labelfont_cb(Fl_Choice* i, void *v) {
  if (v == LOAD) {
    int n = current_widget->o->labelfont();
    if (n > 15) n = 0;
    i->value(n);
  } else {
    int mod = 0;
    int n = i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	q->o->labelfont(n);
	q->redraw();
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void labelsize_cb(Fl_Value_Input* i, void *v) {
  int n;
  if (v == LOAD) {
    n = current_widget->o->labelsize();
  } else {
    int mod = 0;
    n = int(i->value());
    if (n <= 0) n = Fl_Widget_Type::default_size;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	q->o->labelsize(n);
	q->redraw();
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
  i->value(n);
}

extern const char *ui_find_image_name;

Fl_Menu_Item labeltypemenu[] = {
  {"NORMAL_LABEL",0,0,(void*)0},
  {"SHADOW_LABEL",0,0,(void*)FL_SHADOW_LABEL},
  {"ENGRAVED_LABEL",0,0,(void*)FL_ENGRAVED_LABEL},
  {"EMBOSSED_LABEL",0,0,(void*)FL_EMBOSSED_LABEL},
  {"NO_LABEL",0,0,(void*)(FL_NO_LABEL)},
{0}};

void labeltype_cb(Fl_Choice* i, void *v) {
  if (v == LOAD) {
    int n;
    n = current_widget->o->labeltype();
    i->when(FL_WHEN_RELEASE);
    for (int j = 0; j < int(sizeof(labeltypemenu)/sizeof(*labeltypemenu)); j++)
      if (labeltypemenu[j].argument() == n) {i->value(j); break;}
  } else {
    int mod = 0;
    int m = i->value();
    int n = int(labeltypemenu[m].argument());
    if (n<0) return; // should not happen
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* p = (Fl_Widget_Type*)o;
	p->o->labeltype((Fl_Labeltype)n);
	p->redraw();
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

////////////////////////////////////////////////////////////////

void color_cb(Fl_Button* i, void *v) {
  Fl_Color c = current_widget->o->color();
  if (v == LOAD) {
    if (current_widget->is_menu_item()) {i->deactivate(); return;} else i->activate();
  } else {
    int mod = 0;
    Fl_Color d = fl_show_colormap(c);
    if (d == c) return;
    c = d;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	q->o->color(c); q->o->redraw();
        if (q->parent && q->parent->type_name() == tabs_type_name) {
          if (q->o->parent()) q->o->parent()->redraw();
        }
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
  i->color(c); i->labelcolor(fl_contrast(FL_BLACK,c)); i->redraw();
}

void color2_cb(Fl_Button* i, void *v) {
  Fl_Color c = current_widget->o->selection_color();
  if (v == LOAD) {
    if (current_widget->is_menu_item()) {i->deactivate(); return;} else i->activate();
  } else {
    int mod = 0;
    Fl_Color d = fl_show_colormap(c);
    if (d == c) return;
    c = d;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	q->o->selection_color(c); q->o->redraw();
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
  i->color(c); i->labelcolor(fl_contrast(FL_BLACK,c)); i->redraw();
}

void labelcolor_cb(Fl_Button* i, void *v) {
  Fl_Color c = current_widget->o->labelcolor();
  if (v != LOAD) {
    int mod = 0;
    Fl_Color d = fl_show_colormap(c);
    if (d == c) return;
    c = d;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	q->o->labelcolor(c); q->redraw();
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
  i->color(c); i->labelcolor(fl_contrast(FL_BLACK,c)); i->redraw();
}

static Fl_Button* relative(Fl_Widget* o, int i) {
  Fl_Group* g = (Fl_Group*)(o->parent());
  return (Fl_Button*)(g->child(g->find(*o)+i));
}

static Fl_Menu_Item alignmenu[] = {
  {"FL_ALIGN_CENTER",0,0,(void*)(fl_intptr_t)(FL_ALIGN_CENTER)},
  {"FL_ALIGN_TOP",0,0,(void*)(fl_intptr_t)(FL_ALIGN_TOP)},
  {"FL_ALIGN_BOTTOM",0,0,(void*)(fl_intptr_t)(FL_ALIGN_BOTTOM)},
  {"FL_ALIGN_LEFT",0,0,(void*)(fl_intptr_t)(FL_ALIGN_LEFT)},
  {"FL_ALIGN_RIGHT",0,0,(void*)(fl_intptr_t)(FL_ALIGN_RIGHT)},
  {"FL_ALIGN_INSIDE",0,0,(void*)(fl_intptr_t)(FL_ALIGN_INSIDE)},
  {"FL_ALIGN_CLIP",0,0,(void*)(fl_intptr_t)(FL_ALIGN_CLIP)},
  {"FL_ALIGN_WRAP",0,0,(void*)(fl_intptr_t)(FL_ALIGN_WRAP)},
  {"FL_ALIGN_TEXT_OVER_IMAGE",0,0,(void*)(fl_intptr_t)(FL_ALIGN_TEXT_OVER_IMAGE)},
  {"FL_ALIGN_TOP_LEFT",0,0,(void*)(fl_intptr_t)(FL_ALIGN_TOP_LEFT)},
  {"FL_ALIGN_TOP_RIGHT",0,0,(void*)(fl_intptr_t)(FL_ALIGN_TOP_RIGHT)},
  {"FL_ALIGN_BOTTOM_LEFT",0,0,(void*)(fl_intptr_t)(FL_ALIGN_BOTTOM_LEFT)},
  {"FL_ALIGN_BOTTOM_RIGHT",0,0,(void*)(fl_intptr_t)(FL_ALIGN_BOTTOM_RIGHT)},
  {"FL_ALIGN_LEFT_TOP",0,0,(void*)(fl_intptr_t)(FL_ALIGN_LEFT_TOP)},
  {"FL_ALIGN_RIGHT_TOP",0,0,(void*)(fl_intptr_t)(FL_ALIGN_RIGHT_TOP)},
  {"FL_ALIGN_LEFT_BOTTOM",0,0,(void*)(fl_intptr_t)(FL_ALIGN_LEFT_BOTTOM)},
  {"FL_ALIGN_RIGHT_BOTTOM",0,0,(void*)(fl_intptr_t)(FL_ALIGN_RIGHT_BOTTOM)},
{0}};

void align_cb(Fl_Button* i, void *v) {
  Fl_Align b = Fl_Align(fl_uintptr_t(i->user_data()));
  if (v == LOAD) {
    if (current_widget->is_menu_item()) {i->deactivate(); return;} else i->activate();
    i->value(current_widget->o->align() & b);
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	Fl_Align x = q->o->align();
	Fl_Align y;
	if (i->value()) {
	  y = x | b;
	  if (b == FL_ALIGN_LEFT || b == FL_ALIGN_TOP) {
	    Fl_Button *b1 = relative(i,+1);
	    b1->clear();
	    y = y & ~(b1->argument());
	  }
	  if (b == FL_ALIGN_RIGHT || b == FL_ALIGN_BOTTOM) {
	    Fl_Button *b1 = relative(i,-1);
	    b1->clear();
	    y = y & ~(b1->argument());
	  }
	} else {
	  y = x & ~b;
	}
	if (x != y) {
          q->o->align(y);
	  q->redraw();
	  mod = 1;
	}
      }
    }
    if (mod) set_modflag(1);
  }
}

void align_position_cb(Fl_Choice *i, void *v) {
  if (v == LOAD) {
    if (current_widget->is_menu_item()) {i->deactivate(); return;} else i->activate();
    Fl_Menu_Item *mi = (Fl_Menu_Item*)i->menu();
    Fl_Align b = current_widget->o->align() & FL_ALIGN_POSITION_MASK;
    for (;mi->text;mi++) {
      if ((Fl_Align)(mi->argument())==b)
        i->value(mi);
    }
  } else {
    const Fl_Menu_Item *mi = i->menu() + i->value();
    Fl_Align b = Fl_Align(fl_uintptr_t(mi->user_data()));
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	Fl_Align x = q->o->align();
	Fl_Align y = (x & ~FL_ALIGN_POSITION_MASK) | b;
	if (x != y) {
          q->o->align(y);
	  q->redraw();
	  mod = 1;
	}
      }
    }
    if (mod) set_modflag(1);
  }
}

void align_text_image_cb(Fl_Choice *i, void *v) {
  if (v == LOAD) {
    if (current_widget->is_menu_item()) {i->deactivate(); return;} else i->activate();
    Fl_Menu_Item *mi = (Fl_Menu_Item*)i->menu();
    Fl_Align b = current_widget->o->align() & FL_ALIGN_IMAGE_MASK;
    for (;mi->text;mi++) {
      if ((Fl_Align)(mi->argument())==b)
        i->value(mi);
    }
  } else {
    const Fl_Menu_Item *mi = i->menu() + i->value();
    Fl_Align b = Fl_Align(fl_uintptr_t(mi->user_data()));
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	Fl_Align x = q->o->align();
	Fl_Align y = (x & ~FL_ALIGN_IMAGE_MASK) | b;
	if (x != y) {
          q->o->align(y);
	  q->redraw();
	  mod = 1;
	}
      }
    }
    if (mod) set_modflag(1);
  }
}

////////////////////////////////////////////////////////////////

void callback_cb(CodeEditor* i, void *v) {
  if (v == LOAD) {
    const char *cbtext = current_widget->callback();
    i->buffer()->text( cbtext ? cbtext : "" );
  } else {
    int mod = 0;
    char *c = i->buffer()->text();
    const char *d = c_check(c);
    if (d) {
      fl_message("Error in callback: %s",d);
      if (i->window()) i->window()->make_current();
      haderror = 1;
    }
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected) {
        o->callback(c);
        mod = 1;
      }
    }
    if (mod) set_modflag(1);
    free(c);
  }
}

void comment_cb(Fl_Text_Editor* i, void *v) {
  if (v == LOAD) {
    const char *cmttext = current_widget->comment();
    i->buffer()->text( cmttext ? cmttext : "" );
  } else {
    int mod = 0;
    char *c = i->buffer()->text();
    const char *d = c_check(c);
    if (d) {
      fl_message("Error in comment: %s",d);
      if (i->window()) i->window()->make_current();
      haderror = 1;
    }
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected) {
        o->comment(c);
        mod = 1;
      }
    }
    if (mod) set_modflag(1);
    free(c);
  }
}

void user_data_cb(Fl_Input *i, void *v) {
  if (v == LOAD) {
    i->static_value(current_widget->user_data());
  } else {
    int mod = 0;
    const char *c = i->value();
    const char *d = c_check(c);
    if (d) {fl_message("Error in user_data: %s",d); haderror = 1; return;}
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected) {
        o->user_data(c);
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void user_data_type_cb(Fl_Input *i, void *v) {
  static const char *dflt = "void*";
  if (v == LOAD) {
    const char *c = current_widget->user_data_type();
    if (!c) c = dflt;
    i->static_value(c);
  } else {
    int mod = 0;
    const char *c = i->value();
    const char *d = c_check(c);
    if (!*c) i->value(dflt);
    else if (!strcmp(c,dflt)) c = 0;
    if (!d) {
      if (c && *c && c[strlen(c)-1] != '*' && strcmp(c,"long"))
	d = "must be pointer or long";
    }
    if (d) {fl_message("Error in type: %s",d); haderror = 1; return;}
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected) {
        o->user_data_type(c);
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

// "v_attributes" let user type in random code for attribute settings:

void v_input_cb(Fl_Input* i, void* v) {
  int n = fl_intptr_t(i->user_data());
  if (v == LOAD) {
    i->static_value(current_widget->extra_code(n));
  } else {
    int mod = 0;
    const char *c = i->value();
    const char *d = c_check(c&&c[0]=='#' ? c+1 : c);
    if (d) {fl_message("Error in %s: %s",i->label(),d); haderror = 1; return;}
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type *t = (Fl_Widget_Type*)o;
	t->extra_code(n,c);
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void subclass_cb(Fl_Input* i, void* v) {
  if (v == LOAD) {
    if (current_widget->is_menu_item()) {i->deactivate(); return;} else i->activate();
    i->static_value(current_widget->subclass());
  } else {
    int mod = 0;
    const char *c = i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type *t = (Fl_Widget_Type*)o;
	t->subclass(c);
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

////////////////////////////////////////////////////////////////

// textstuff: set textfont, textsize, textcolor attributes:

// default widget returns 0 to indicate not-implemented:
int Fl_Widget_Type::textstuff(int, Fl_Font&, int&, Fl_Color&) {return 0;}

void textfont_cb(Fl_Choice* i, void* v) {
  Fl_Font n; int s; Fl_Color c;
  if (v == LOAD) {
    if (!current_widget->textstuff(0,n,s,c)) {i->deactivate(); return;}
    i->activate();
    if (n > 15) n = FL_HELVETICA;
    i->value(n);
  } else {
    int mod = 0;
    n = (Fl_Font)i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	q->textstuff(1,n,s,c);
	q->o->redraw();
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void textsize_cb(Fl_Value_Input* i, void* v) {
  Fl_Font n; int s; Fl_Color c;
  if (v == LOAD) {
    if (!current_widget->textstuff(0,n,s,c)) {i->deactivate(); return;}
    i->activate();
  } else {
    int mod = 0;
    s = int(i->value());
    if (s <= 0) s = Fl_Widget_Type::default_size;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	q->textstuff(2,n,s,c);
	q->o->redraw();
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
  i->value(s);
}

void textcolor_cb(Fl_Button* i, void* v) {
  Fl_Font n; int s; Fl_Color c;
  if (v == LOAD) {
    if (!current_widget->textstuff(0,n,s,c)) {i->deactivate(); return;}
    i->activate();
  } else {
    int mod = 0;
    c = i->color();
    Fl_Color d = fl_show_colormap(c);
    if (d == c) return;
    c = d;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	q->textstuff(3,n,s,c); q->o->redraw();
	mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
  i->color(c); i->labelcolor(fl_contrast(FL_BLACK,c)); i->redraw();
}

////////////////////////////////////////////////////////////////
// Kludges to the panel for subclasses:

void min_w_cb(Fl_Value_Input* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_window()) {i->parent()->hide(); return;}
    i->parent()->show();
    i->value(((Fl_Window_Type*)current_widget)->sr_min_w);
  } else {
    int mod = 0;
    int n = (int)i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_window()) {
        ((Fl_Window_Type*)current_widget)->sr_min_w = n;
        mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void min_h_cb(Fl_Value_Input* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_window()) return;
    i->value(((Fl_Window_Type*)current_widget)->sr_min_h);
  } else {
    int mod = 0;
    int n = (int)i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_window()) {
        ((Fl_Window_Type*)current_widget)->sr_min_h = n;
        mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void max_w_cb(Fl_Value_Input* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_window()) return;
    i->value(((Fl_Window_Type*)current_widget)->sr_max_w);
  } else {
    int mod = 0;
    int n = (int)i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_window()) {
        ((Fl_Window_Type*)current_widget)->sr_max_w = n;
        mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void max_h_cb(Fl_Value_Input* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_window()) return;
    i->value(((Fl_Window_Type*)current_widget)->sr_max_h);
  } else {
    int mod = 0;
    int n = (int)i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_window()) {
        ((Fl_Window_Type*)current_widget)->sr_max_h = n;
        mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}

void set_min_size_cb(Fl_Button*, void* v) {
  if (v == LOAD) {
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_window()) {
        Fl_Window_Type *win = (Fl_Window_Type*)current_widget;
        win->sr_min_w = win->o->w();
        win->sr_min_h = win->o->h();
        mod = 1;
      }
    }
    propagate_load(the_panel, LOAD);
    if (mod) set_modflag(1);
  }
}

void set_max_size_cb(Fl_Button*, void* v) {
  if (v == LOAD) {
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_window()) {
        Fl_Window_Type *win = (Fl_Window_Type*)current_widget;
        win->sr_max_w = win->o->w();
        win->sr_max_h = win->o->h();
        mod = 1;
      }
    }
    propagate_load(the_panel, LOAD);
    if (mod) set_modflag(1);
  }
}

void slider_size_cb(Fl_Value_Input* i, void* v) {
  if (v == LOAD) {
    if (current_widget->is_window()) 
      i->parent()->hide(); 
    else
      i->parent()->show();
    if (current_widget->is_valuator()<2) {i->deactivate(); return;}
    i->activate();
    i->value(((Fl_Slider*)(current_widget->o))->slider_size());
  } else {
    int mod = 0;
    double n = i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	if (q->is_valuator()>=2) {
	  ((Fl_Slider*)(q->o))->slider_size(n);
	  q->o->redraw();
	  mod = 1;
	}
      }
    }
    if (mod) set_modflag(1);
  }
}

void min_cb(Fl_Value_Input* i, void* v) {
  if (v == LOAD) {
    if (current_widget->is_valuator()) {
      i->activate();
      i->value(((Fl_Valuator*)(current_widget->o))->minimum());
    } else if (current_widget->is_spinner()) {
      i->activate();
      i->value(((Fl_Spinner*)(current_widget->o))->minimum());
    } else {
      i->deactivate();
      return;
    }
  } else {
    int mod = 0;
    double n = i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	if (q->is_valuator()) {
	  ((Fl_Valuator*)(q->o))->minimum(n);
	  q->o->redraw();
	  mod = 1;
	} else if (q->is_spinner()) {
          ((Fl_Spinner*)(q->o))->minimum(n);
          q->o->redraw();
          mod = 1;
        }
      }
    }
    if (mod) set_modflag(1);
  }
}

void max_cb(Fl_Value_Input* i, void* v) {
  if (v == LOAD) {
    if (current_widget->is_valuator()) {
      i->activate();
      i->value(((Fl_Valuator*)(current_widget->o))->maximum());
    } else if (current_widget->is_spinner()) {
      i->activate();
      i->value(((Fl_Spinner*)(current_widget->o))->maximum());
    } else {
      i->deactivate();
      return;
    }
  } else {
    int mod = 0;
    double n = i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	if (q->is_valuator()) {
	  ((Fl_Valuator*)(q->o))->maximum(n);
	  q->o->redraw();
	  mod = 1;
        } else if (q->is_spinner()) {
          ((Fl_Spinner*)(q->o))->maximum(n);
          q->o->redraw();
          mod = 1;
	}
      }
    }
    if (mod) set_modflag(1);
  }
}

void step_cb(Fl_Value_Input* i, void* v) {
  if (v == LOAD) {
    if (current_widget->is_valuator()) {
      i->activate();
      i->value(((Fl_Valuator*)(current_widget->o))->step());
    } else if (current_widget->is_spinner()) {
      i->activate();
      i->value(((Fl_Spinner*)(current_widget->o))->step());
    } else {
      i->deactivate();
      return;
    }
  } else {
    int mod = 0;
    double n = i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        Fl_Widget_Type* q = (Fl_Widget_Type*)o;
        if (q->is_valuator()) {
          ((Fl_Valuator*)(q->o))->step(n);
          q->o->redraw();
          mod = 1;
        } else if (q->is_spinner()) {
          ((Fl_Spinner*)(q->o))->step(n);
          q->o->redraw();
          mod = 1;
        }
      }
    }
    if (mod) set_modflag(1);
  }
}

void value_cb(Fl_Value_Input* i, void* v) {
  if (v == LOAD) {
    if (current_widget->is_valuator()) {
      i->activate();
      i->value(((Fl_Valuator*)(current_widget->o))->value());
    } else if (current_widget->is_button()) {
      i->activate();
      i->value(((Fl_Button*)(current_widget->o))->value());
    } else if (current_widget->is_spinner()) {
      i->activate();
      i->value(((Fl_Spinner*)(current_widget->o))->value());
    } else 
      i->deactivate();
  } else {
    int mod = 0;
    double n = i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	if (q->is_valuator()) {
	  ((Fl_Valuator*)(q->o))->value(n);
	  mod = 1;
	} else if (q->is_button()) {
	  ((Fl_Button*)(q->o))->value(n != 0);
	  if (q->is_menu_item()) q->redraw();
	  mod = 1;
        } else if (q->is_spinner()) {
          ((Fl_Spinner*)(q->o))->value(n);
          mod = 1;
	}
      }
    }
    if (mod) set_modflag(1);
  }
}

////////////////////////////////////////////////////////////////

// subtypes:

Fl_Menu_Item *Fl_Widget_Type::subtypes() {return 0;}

void subtype_cb(Fl_Choice* i, void* v) {
  if (v == LOAD) {
    Fl_Menu_Item* m = current_widget->subtypes();
    if (!m) {i->deactivate(); return;}
    i->menu(m);
    int j;
    for (j = 0;; j++) {
      if (!m[j].text) {j = 0; break;}
      if (current_widget->is_spinner()) {
        if (m[j].argument() == ((Fl_Spinner*)current_widget->o)->type()) break;
      } else {
        if (m[j].argument() == current_widget->o->type()) break;
      }
    }
    i->value(j);
    i->activate();
    i->redraw();
  } else {
    int mod = 0;
    int n = int(i->mvalue()->argument());
    Fl_Menu_Item* m = current_widget->subtypes();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        Fl_Widget_Type* q = (Fl_Widget_Type*)o;
        if (q->subtypes()==m) {
          if (q->is_spinner())
            ((Fl_Spinner*)q->o)->type(n);
          else
            q->o->type(n);
          q->redraw();
          mod = 1;
        }
      }
    }
    if (mod) set_modflag(1);
  }
}

////////////////////////////////////////////////////////////////

void propagate_load(Fl_Group* g, void* v) {
  if (v == LOAD) {
    Fl_Widget*const* a = g->array();
    for (int i=g->children(); i--;) {
      Fl_Widget* o = *a++;
      o->do_callback(o,LOAD);
    }
  }
}

void set_cb(Fl_Button*, void*) {
  haderror = 0;
  Fl_Widget*const* a = the_panel->array();
  for (int i=the_panel->children(); i--;) {
    Fl_Widget* o = *a++;
    if (o->changed()) {
      o->do_callback();
      if (haderror) return;
      o->clear_changed();
    }
  }
}

void ok_cb(Fl_Return_Button* o, void* v) {
  set_cb(o,v);
  if (!haderror) the_panel->hide();
}

void revert_cb(Fl_Button*, void*) {
  // We have to revert all dynamically changing fields:
  // but for now only the first label works...
  if (numselected == 1) current_widget->label(oldlabel);
  propagate_load(the_panel, LOAD);
}

void cancel_cb(Fl_Button* o, void* v) {
  revert_cb(o,v);
  the_panel->hide();
}

void toggle_overlays(Fl_Widget *,void *); // in Fl_Window_Type.cxx
void overlay_cb(Fl_Button*o,void *v) {
  toggle_overlays(o,v);
}

void leave_live_mode_cb(Fl_Widget*, void*);

void live_mode_cb(Fl_Button*o,void *) {
  /// \todo live mode should end gracefully when the application quits
  ///       or when the user closes the live widget
  static Fl_Type *live_type = 0L;
  static Fl_Widget *live_widget = 0L;
  static Fl_Window *live_window = 0L;
  // if 'o' is 0, we must quit live mode
  if (!o) {
    o = wLiveMode;
    o->value(0);
  }
  if (o->value()) {
    if (numselected == 1) {
      Fl_Group::current(0L);
      live_widget = current_widget->enter_live_mode(1);
      if (live_widget) {
        live_type = current_widget;
        Fl_Group::current(0);
        int w = live_widget->w();
        int h = live_widget->h();
        live_window = new Fl_Double_Window(w+20, h+55, "Fluid Live Mode Widget");
        live_window->box(FL_FLAT_BOX);
        live_window->color(FL_GREEN);
        Fl_Group *rsz = new Fl_Group(0, h+20, 130, 35);
        rsz->box(FL_NO_BOX);
        Fl_Box *rsz_dummy = new Fl_Box(110, h+20, 1, 25);
        rsz_dummy->box(FL_NO_BOX);
        rsz->resizable(rsz_dummy);
        Fl_Button *btn = new Fl_Button(10, h+20, 100, 25, "Exit Live Mode");
        btn->labelsize(12);
        btn->callback(leave_live_mode_cb);
        rsz->end();
        live_window->add(live_widget);
        live_widget->position(10, 10);
        live_window->resizable(live_widget);
        live_window->set_modal(); // block all other UI
        live_window->callback(leave_live_mode_cb);
        if (current_widget->is_window()) {
          Fl_Window_Type *w = (Fl_Window_Type*)current_widget;
          int mw = w->sr_min_w; if (mw>0) mw += 20;
          int mh = w->sr_min_h; if (mh>0) mh += 55;
          int MW = w->sr_max_w; if (MW>0) MW += 20; 
          int MH = w->sr_max_h; if (MH>2) MH += 55;
          if (mw || mh || MW || MH)
            live_window->size_range(mw, mh, MW, MH);
        }
        live_window->show();
      } else o->value(0);
    } else o->value(0);
  } else {
    if (live_type)
      live_type->leave_live_mode();
    if (live_window) {
      live_window->hide();
      Fl::delete_widget(live_window);
    }
    live_type = 0L;
    live_widget = 0L;
    live_window = 0L;
  }
}

// update the panel according to current widget set:
static void load_panel() {
  if (!the_panel) return;

  // find all the Fl_Widget subclasses currently selected:
  numselected = 0;
  current_widget = 0;
  if (Fl_Type::current) {
    if (Fl_Type::current->is_widget())
      current_widget=(Fl_Widget_Type*)Fl_Type::current;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->is_widget() && o->selected) {
	numselected++;
	if (!current_widget) current_widget = (Fl_Widget_Type*)o;
      }
    }
  }
  if (numselected)
    propagate_load(the_panel, LOAD);
  else
    the_panel->hide();
}

// This is called when user double-clicks an item, open or update the panel:
void Fl_Widget_Type::open() {
  if (!the_panel) the_panel = make_widget_panel();
  load_panel();
  if (numselected) the_panel->show();
}

Fl_Type *Fl_Type::current;

extern void redraw_overlays();
extern void check_redraw_corresponding_parent(Fl_Type*);
extern void redraw_browser();
extern void update_sourceview_position();

// Called when ui changes what objects are selected:
// p is selected object, null for all deletions (we must throw away
// old panel in that case, as the object may no longer exist)
void selection_changed(Fl_Type *p) {
  // store all changes to the current selected objects:
  if (p && the_panel && the_panel->visible()) {
    set_cb(0,0);
    // if there was an error, we try to leave the selected set unchanged:
    if (haderror) {
      Fl_Type *q = 0;
      for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
	o->new_selected = o->selected;
	if (!q && o->selected) q = o;
      }
      if (!p || !p->selected) p = q;
      Fl_Type::current = p;
      redraw_browser();
      return;
    }
  }
  // update the selected flags to new set:
  Fl_Type *q = 0;
  for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
    o->selected = o->new_selected;
    if (!q && o->selected) q = o;
  }
  if (!p || !p->selected) p = q;
  Fl_Type::current = p;
  check_redraw_corresponding_parent(p);
  redraw_overlays();
  // load the panel with the new settings:
  load_panel();
  // update the source viewer to show the code for the selected object
  update_sourceview_position();
}

////////////////////////////////////////////////////////////////
// Writing the C code:

// test to see if user named a function, or typed in code:
int is_name(const char *c) {
  for (; *c; c++) if (ispunct(*c) && *c!='_' && *c!=':') return 0;
  return 1;
}

// Test to see if name() is an array entry.  If so, and this is the
// highest number, return name[num+1].  Return null if not the highest
// number or a field or function.  Return name() if not an array entry.
const char *array_name(Fl_Widget_Type *o) {
  const char *c = o->name();
  if (!c) return 0;
  const char *d;
  for (d = c; *d != '['; d++) {
    if (!*d) return c;
    if (ispunct(*d) && *d!='_') return 0;
  }
  int num = atoi(d+1);
  int sawthis = 0;
  Fl_Type *t = o->prev;
  Fl_Type *tp = o;
  const char *cn = o->class_name(1);
  for (; t && t->class_name(1) == cn; tp = t, t = t->prev) {/*empty*/}
  for (t = tp; t && t->class_name(1) == cn; t = t->next) {
    if (t == o) {sawthis=1; continue;}
    const char *e = t->name();
    if (!e) continue;
    if (strncmp(c,e,d-c)) continue;
    int n1 = atoi(e+(d-c)+1);
    if (n1 > num || (n1==num && sawthis)) return 0;
  }
  static char buffer[128];
  // MRS: we want strncpy() here...
  strncpy(buffer,c,d-c+1);
  snprintf(buffer+(d-c+1),sizeof(buffer) - (d-c+1), "%d]",num+1);
  return buffer;
}

// Test to see if extra code is a declaration:
int isdeclare(const char *c) {
  while (isspace(*c)) c++;
  if (*c == '#') return 1;
  if (!strncmp(c,"extern",6)) return 1;
  if (!strncmp(c,"typedef",7)) return 1;
  if (!strncmp(c,"using",5)) return 1;
  return 0;
}

void Fl_Widget_Type::write_static() {
  const char* t = subclassname(this);
  if (!subclass() || (is_class() && !strncmp(t, "Fl_", 3))) {
    write_declare("#include <FL/%s.H>", t);
  }
  for (int n=0; n < NUM_EXTRA_CODE; n++) {
    if (extra_code(n) && isdeclare(extra_code(n)))
      write_declare("%s", extra_code(n));
  }
  if (callback() && is_name(callback())) {
    int write_extern_declaration = 1;
    const Fl_Class_Type *cc = is_in_class();
    char buf[1024]; snprintf(buf, 1023, "%s(*)",  callback());
    if (cc) {
      if (cc->has_function("static void", buf))
        write_extern_declaration = 0;
    } else {
      if (has_toplevel_function(0L, buf))
        write_extern_declaration = 0;
    }
    if (write_extern_declaration)
      write_declare("extern void %s(%s*, %s);", callback(), t,
		    user_data_type() ? user_data_type() : "void*");
  }
  const char* k = class_name(1);
  const char* c = array_name(this);
  if (c && !k && !is_class()) {
    write_c("\n");
    if (!public_) write_c("static ");
    else write_h("extern %s *%s;\n", t, c);
    if (strchr(c, '[') == NULL) write_c("%s *%s=(%s *)0;\n", t, c, t);
    else write_c("%s *%s={(%s *)0};\n", t, c, t);
  }
  if (callback() && !is_name(callback())) {
    // see if 'o' or 'v' used, to prevent unused argument warnings:
    int use_o = 0;
    int use_v = 0;
    const char *d;
    for (d = callback(); *d;) {
      if (*d == 'o' && !is_id(d[1])) use_o = 1;
      if (*d == 'v' && !is_id(d[1])) use_v = 1;
      do d++; while (is_id(*d));
      while (*d && !is_id(*d)) d++;
    }
    const char* cn = callback_name();
    if (k) {
      write_c("\nvoid %s::%s_i(%s*", k, cn, t);
    } else {
      write_c("\nstatic void %s(%s*", cn, t);
    }
    if (use_o) write_c(" o");
    const char* ut = user_data_type() ? user_data_type() : "void*";
    write_c(", %s", ut);
    if (use_v) write_c(" v");
    write_c(") {\n  %s", callback());
    if (*(d-1) != ';') {
      const char *p = strrchr(callback(), '\n');
      if (p) p ++;
      else p = callback();
      // Only add trailing semicolon if the last line is not a preprocessor
      // statement...
      if (*p != '#' && *p) write_c(";");
    }
    write_c("\n}\n");
    if (k) {
      write_c("void %s::%s(%s* o, %s v) {\n", k, cn, t, ut);
      write_c("  ((%s*)(o", k);
      Fl_Type *q = 0;
      for (Fl_Type* p = parent; p && p->is_widget(); q = p, p = p->parent)
	write_c("->parent()");
      if (!q || strcmp(q->type_name(), "widget_class"))
        write_c("->user_data()");
      write_c("))->%s_i(o,v);\n}\n", cn);
    }
  }
  if (image) {
    if (image->written != write_number) {
      image->write_static();
      image->written = write_number;
    }
  }
  if (inactive) {
    if (inactive->written != write_number) {
      inactive->write_static();
      inactive->written = write_number;
    }
  }
}

const char *Fl_Type::callback_name() {
  if (is_name(callback())) return callback();
  return unique_id(this, "cb", name(), label());
}

extern int varused_test, varused;

void Fl_Widget_Type::write_code1() {
  const char* t = subclassname(this);
  const char *c = array_name(this);
  if (c) {
    if (class_name(1)) {
      write_public(public_);
      write_h("  %s *%s;\n", t, c);
    }
  }
  if (class_name(1) && callback() && !is_name(callback())) {
    const char* cn = callback_name();
    const char* ut = user_data_type() ? user_data_type() : "void*";
    write_public(0);
    write_h("  inline void %s_i(%s*, %s);\n", cn, t, ut);
    write_h("  static void %s(%s*, %s);\n", cn, t, ut);
  }
  // figure out if local variable will be used (prevent compiler warnings):
  int wused = !name() && is_window();
  const char *ptr;

  varused = wused;

  if (!name() && !varused) {
    varused |= is_parent();

    if (!varused) {
      varused_test = 1;
      write_widget_code();
      varused_test = 0;
    }
  }

  if (!varused) {
    for (int n=0; n < NUM_EXTRA_CODE; n++)
      if (extra_code(n) && !isdeclare(extra_code(n)))
      {
        int instring = 0;
	int inname = 0;
        for (ptr = extra_code(n); *ptr; ptr ++) {
	  if (instring) {
	    if (*ptr == '\\') ptr++;
	    else if (*ptr == '\"') instring = 0;
	  } else if (inname && !isalnum(*ptr & 255)) inname = 0;
          else if (*ptr == '\"') instring = 1;
	  else if (isalnum(*ptr & 255) || *ptr == '_') {
	    size_t len = strspn(ptr, "0123456789_"
	                             "abcdefghijklmnopqrstuvwxyz"
				     "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

            if (!strncmp(ptr, "o", len)) {
	      varused = 1;
	      break;
	    } else ptr += len - 1;
          }
	}
      }
  }

  write_c("%s{ ", indent());
  write_comment_inline_c();
  if (varused) write_c("%s* o = ", t);
  if (name()) write_c("%s = ", name());
  if (is_window()) {
    // Handle special case where user is faking a Fl_Group type as a window,
    // there is no 2-argument constructor in that case:
    if (!strstr(t, "Window"))
      write_c("new %s(0, 0, %d, %d", t, o->w(), o->h());
    else
      write_c("new %s(%d, %d", t, o->w(), o->h());
  } else {
    write_c("new %s(%d, %d, %d, %d", t, o->x(), o->y(), o->w(), o->h());
  }
  if (label() && *label()) {
    write_c(", ");
    switch (i18n_type) {
    case 0 : /* None */
        write_cstring(label());
        break;
    case 1 : /* GNU gettext */
        write_c("%s(", i18n_function);
        write_cstring(label());
	write_c(")");
        break;
    case 2 : /* POSIX catgets */
        write_c("catgets(%s,%s,%d,", i18n_file[0] ? i18n_file : "_catalog",
	        i18n_set, msgnum());
        write_cstring(label());
	write_c(")");
        break;
    }
  }
  write_c(");\n");

  indentation += 2;

  if (wused) write_c("%sw = o; if (w) {/* empty */}\n", indent());

  write_widget_code();
}

void Fl_Widget_Type::write_color(const char* field, Fl_Color color) {
  const char* color_name = 0;
  switch (color) {
  case FL_FOREGROUND_COLOR:	color_name = "FL_FOREGROUND_COLOR";	break;
  case FL_BACKGROUND2_COLOR:	color_name = "FL_BACKGROUND2_COLOR";	break;
  case FL_INACTIVE_COLOR:	color_name = "FL_INACTIVE_COLOR";	break;
  case FL_SELECTION_COLOR:	color_name = "FL_SELECTION_COLOR";	break;
  case FL_GRAY0:		color_name = "FL_GRAY0";		break;
  case FL_DARK3:		color_name = "FL_DARK3";		break;
  case FL_DARK2:		color_name = "FL_DARK2";		break;
  case FL_DARK1:		color_name = "FL_DARK1";		break;
  case FL_BACKGROUND_COLOR:	color_name = "FL_BACKGROUND_COLOR";	break;
  case FL_LIGHT1:		color_name = "FL_LIGHT1";		break;
  case FL_LIGHT2:		color_name = "FL_LIGHT2";		break;
  case FL_LIGHT3:		color_name = "FL_LIGHT3";		break;
  case FL_BLACK:		color_name = "FL_BLACK";		break;
  case FL_RED:			color_name = "FL_RED";			break;
  case FL_GREEN:		color_name = "FL_GREEN";		break;
  case FL_YELLOW:		color_name = "FL_YELLOW";		break;
  case FL_BLUE:			color_name = "FL_BLUE";			break;
  case FL_MAGENTA:		color_name = "FL_MAGENTA";		break;
  case FL_CYAN:			color_name = "FL_CYAN";			break;
  case FL_DARK_RED:		color_name = "FL_DARK_RED";		break;
  case FL_DARK_GREEN:		color_name = "FL_DARK_GREEN";		break;
  case FL_DARK_YELLOW:		color_name = "FL_DARK_YELLOW";		break;
  case FL_DARK_BLUE:		color_name = "FL_DARK_BLUE";		break;
  case FL_DARK_MAGENTA:		color_name = "FL_DARK_MAGENTA";		break;
  case FL_DARK_CYAN:		color_name = "FL_DARK_CYAN";		break;
  case FL_WHITE:		color_name = "FL_WHITE";		break;
  }
  const char *var = is_class() ? "this" : name() ? name() : "o";
  if (color_name) {
    write_c("%s%s->%s(%s);\n", indent(), var, field, color_name);
  } else {
    write_c("%s%s->%s((Fl_Color)%d);\n", indent(), var, field, color);
  }
}

// this is split from write_code1() for Fl_Window_Type:
void Fl_Widget_Type::write_widget_code() {
  Fl_Widget* tplate = ((Fl_Widget_Type*)factory)->o;
  const char *var = is_class() ? "this" : name() ? name() : "o";

  if (tooltip() && *tooltip()) {
    write_c("%s%s->tooltip(",indent(), var);
    switch (i18n_type) {
    case 0 : /* None */
        write_cstring(tooltip());
        break;
    case 1 : /* GNU gettext */
        write_c("%s(", i18n_function);
        write_cstring(tooltip());
	write_c(")");
        break;
    case 2 : /* POSIX catgets */
        write_c("catgets(%s,%s,%d,", i18n_file[0] ? i18n_file : "_catalog",
	        i18n_set, msgnum() + 1);
        write_cstring(tooltip());
	write_c(")");
        break;
    }
    write_c(");\n");
  }

  if (is_spinner() && ((Fl_Spinner*)o)->type() != ((Fl_Spinner*)tplate)->type())
    write_c("%s%s->type(%d);\n", indent(), var, ((Fl_Spinner*)o)->type());
  else if (o->type() != tplate->type() && !is_window())
    write_c("%s%s->type(%d);\n", indent(), var, o->type());
  if (o->box() != tplate->box() || subclass())
    write_c("%s%s->box(FL_%s);\n", indent(), var, boxname(o->box()));

  // write shortcut command if needed
  int shortcut = 0;
  if (is_button()) shortcut = ((Fl_Button*)o)->shortcut();
  else if (is_input()) shortcut = ((Fl_Input_*)o)->shortcut();
  else if (is_value_input()) shortcut = ((Fl_Value_Input*)o)->shortcut();
  else if (is_text_display()) shortcut = ((Fl_Text_Display*)o)->shortcut();
  if (shortcut) {
    if (use_FL_COMMAND && (shortcut & (FL_CTRL|FL_META))) {
      write_c("%s%s->shortcut(FL_COMMAND|0x%x);\n", indent(), var, shortcut & ~(FL_CTRL|FL_META));
    } else {
      write_c("%s%s->shortcut(0x%x);\n", indent(), var, shortcut);
    }
  }

  if (is_button()) {
    Fl_Button* b = (Fl_Button*)o;
    if (b->down_box()) write_c("%s%s->down_box(FL_%s);\n", indent(), var,
			       boxname(b->down_box()));
    if (b->value()) write_c("%s%s->value(1);\n", indent(), var);
  } else if (!strcmp(type_name(), "Fl_Input_Choice")) {
    Fl_Input_Choice* b = (Fl_Input_Choice*)o;
    if (b->down_box()) write_c("%s%s->down_box(FL_%s);\n", indent(), var,
			       boxname(b->down_box()));
  } else if (is_menu_button()) {
    Fl_Menu_* b = (Fl_Menu_*)o;
    if (b->down_box()) write_c("%s%s->down_box(FL_%s);\n", indent(), var,
			       boxname(b->down_box()));
  }
  if (o->color() != tplate->color() || subclass())
    write_color("color", o->color());
  if (o->selection_color() != tplate->selection_color() || subclass())
    write_color("selection_color", o->selection_color());
  if (image) image->write_code(var);
  if (inactive) inactive->write_code(var, 1);
  if (o->labeltype() != tplate->labeltype() || subclass())
    write_c("%s%s->labeltype(FL_%s);\n", indent(), var,
	    item_name(labeltypemenu, o->labeltype()));
  if (o->labelfont() != tplate->labelfont() || subclass())
    write_c("%s%s->labelfont(%d);\n", indent(), var, o->labelfont());
  if (o->labelsize() != tplate->labelsize() || subclass())
    write_c("%s%s->labelsize(%d);\n", indent(), var, o->labelsize());
  if (o->labelcolor() != tplate->labelcolor() || subclass())
    write_color("labelcolor", o->labelcolor());
  if (is_valuator()) {
    Fl_Valuator* v = (Fl_Valuator*)o;
    Fl_Valuator* f = (Fl_Valuator*)(tplate);
    if (v->minimum()!=f->minimum())
      write_c("%s%s->minimum(%g);\n", indent(), var, v->minimum());
    if (v->maximum()!=f->maximum())
      write_c("%s%s->maximum(%g);\n", indent(), var, v->maximum());
    if (v->step()!=f->step())
      write_c("%s%s->step(%g);\n", indent(), var, v->step());
    if (v->value()) {
      if (is_valuator()==3) { // Fl_Scrollbar::value(double) is nott available
        write_c("%s%s->Fl_Slider::value(%g);\n", indent(), var, v->value());
      } else {
        write_c("%s%s->value(%g);\n", indent(), var, v->value());
      }
    }
    if (is_valuator()>=2) {
      double x = ((Fl_Slider*)v)->slider_size();
      double y = ((Fl_Slider*)f)->slider_size();
      if (x != y) write_c("%s%s->slider_size(%g);\n", indent(), var, x);
    }
  }
  if (is_spinner()) {
    Fl_Spinner* v = (Fl_Spinner*)o;
    Fl_Spinner* f = (Fl_Spinner*)(tplate);
    if (v->minimum()!=f->minimum())
      write_c("%s%s->minimum(%g);\n", indent(), var, v->minimum());
    if (v->maximum()!=f->maximum())
      write_c("%s%s->maximum(%g);\n", indent(), var, v->maximum());
    if (v->step()!=f->step())
      write_c("%s%s->step(%g);\n", indent(), var, v->step());
    if (v->value()!=1.0f)
      write_c("%s%s->value(%g);\n", indent(), var, v->value());
  }

  {Fl_Font ff; int fs; Fl_Color fc; if (textstuff(4,ff,fs,fc)) {
    Fl_Font f; int s; Fl_Color c; textstuff(0,f,s,c);
    if (f != ff) write_c("%s%s->textfont(%d);\n", indent(), var, f);
    if (s != fs) write_c("%s%s->textsize(%d);\n", indent(), var, s);
    if (c != fc) write_color("textcolor", c);
  }}
  const char* ud = user_data();
  if (class_name(1) && !parent->is_widget()) ud = "this";
  if (callback()) {
    write_c("%s%s->callback((Fl_Callback*)%s", indent(), var, callback_name());
    if (ud)
      write_c(", (void*)(%s));\n", ud);
    else
      write_c(");\n");
  } else if (ud) {
    write_c("%s%s->user_data((void*)(%s));\n", indent(), var, ud);
  }
  if (o->align() != tplate->align() || subclass()) {
    int i = o->align();
    write_c("%s%s->align(Fl_Align(%s", indent(), var,
	    item_name(alignmenu, i & ~FL_ALIGN_INSIDE));
    if (i & FL_ALIGN_INSIDE) write_c("|FL_ALIGN_INSIDE");
    write_c("));\n");
  }
  // avoid the unsupported combination of flegs when user sets 
  // "when" to "FL_WHEN_NEVER", but keeps the "no change" set. 
  // FIXME: This could be reflected in the GUI by graying out the button.
  Fl_When ww = o->when();
  if (ww==FL_WHEN_NOT_CHANGED)
    ww = FL_WHEN_NEVER;
  if (ww != tplate->when() || subclass())
    write_c("%s%s->when(%s);\n", indent(), var,
            item_name(whensymbolmenu, ww));
  if (!o->visible() && o->parent())
    write_c("%s%s->hide();\n", indent(), var);
  if (!o->active())
    write_c("%s%s->deactivate();\n", indent(), var);
  if (!is_group() && resizable())
    write_c("%sFl_Group::current()->resizable(%s);\n", indent(), var);
  if (hotspot()) {
    if (is_class())
      write_c("%shotspot(%s);\n", indent(), var);
    else if (is_window())
      write_c("%s%s->hotspot(%s);\n", indent(), var, var);
    else
      write_c("%s%s->window()->hotspot(%s);\n", indent(), var, var);
  }
}

void Fl_Widget_Type::write_extra_code() {
  for (int n=0; n < NUM_EXTRA_CODE; n++)
    if (extra_code(n) && !isdeclare(extra_code(n)))
      write_c("%s%s\n", indent(), extra_code(n));
}

void Fl_Widget_Type::write_block_close() {
  indentation -= 2;
  write_c("%s} // %s* %s\n", indent(), subclassname(this),
          name() ? name() : "o");
}

void Fl_Widget_Type::write_code2() {
  write_extra_code();
  write_block_close();
}

////////////////////////////////////////////////////////////////

void Fl_Widget_Type::write_properties() {
  Fl_Type::write_properties();
  write_indent(level+1);
  switch (public_) {
    case 0: write_string("private"); break;
    case 1: break;
    case 2: write_string("protected"); break;
  }
  if (tooltip() && *tooltip()) {
    write_string("tooltip");
    write_word(tooltip());
  }
  if (image_name() && *image_name()) {
    write_string("image");
    write_word(image_name());
  }
  if (inactive_name() && *inactive_name()) {
    write_string("deimage");
    write_word(inactive_name());
  }
  write_string("xywh {%d %d %d %d}", o->x(), o->y(), o->w(), o->h());
  Fl_Widget* tplate = ((Fl_Widget_Type*)factory)->o;
  if (is_spinner() && ((Fl_Spinner*)o)->type() != ((Fl_Spinner*)tplate)->type()) {
    write_string("type");
    write_word(item_name(subtypes(), ((Fl_Spinner*)o)->type()));
  } else if (o->type() != tplate->type() || is_window()) {
    write_string("type");
    write_word(item_name(subtypes(), o->type()));
  }
  if (o->box() != tplate->box()) {
    write_string("box"); write_word(boxname(o->box()));}
  if (is_input()) {
    Fl_Input_* b = (Fl_Input_*)o;
    if (b->shortcut()) write_string("shortcut 0x%x", b->shortcut());
  }
  if (is_value_input()) {
    Fl_Value_Input* b = (Fl_Value_Input*)o;
    if (b->shortcut()) write_string("shortcut 0x%x", b->shortcut());
  }
  if (is_text_display()) {
    Fl_Text_Display* b = (Fl_Text_Display*)o;
    if (b->shortcut()) write_string("shortcut 0x%x", b->shortcut());
  }
  if (is_button()) {
    Fl_Button* b = (Fl_Button*)o;
    if (b->down_box()) {
      write_string("down_box"); write_word(boxname(b->down_box()));}
    if (b->shortcut()) write_string("shortcut 0x%x", b->shortcut());
    if (b->value()) write_string("value 1");
  } else if (!strcmp(type_name(), "Fl_Input_Choice")) {
    Fl_Input_Choice* b = (Fl_Input_Choice*)o;
    if (b->down_box()) {
      write_string("down_box"); write_word(boxname(b->down_box()));}
  } else if (is_menu_button()) {
    Fl_Menu_* b = (Fl_Menu_*)o;
    if (b->down_box()) {
      write_string("down_box"); write_word(boxname(b->down_box()));}
  }
  if (o->color()!=tplate->color())
    write_string("color %d", o->color());
  if (o->selection_color()!=tplate->selection_color())
    write_string("selection_color %d", o->selection_color());
  if (o->labeltype()!=tplate->labeltype()) {
    write_string("labeltype");
    write_word(item_name(labeltypemenu, o->labeltype()));
  }
  if (o->labelfont()!=tplate->labelfont())
    write_string("labelfont %d", o->labelfont());
  if (o->labelsize()!=tplate->labelsize())
    write_string("labelsize %d", o->labelsize());
  if (o->labelcolor()!=tplate->labelcolor())
    write_string("labelcolor %d", o->labelcolor());
  if (o->align()!=tplate->align())
    write_string("align %d", o->align());
  if (o->when() != tplate->when())
    write_string("when %d", o->when());
  if (is_valuator()) {
    Fl_Valuator* v = (Fl_Valuator*)o;
    Fl_Valuator* f = (Fl_Valuator*)(tplate);
    if (v->minimum()!=f->minimum()) write_string("minimum %g",v->minimum());
    if (v->maximum()!=f->maximum()) write_string("maximum %g",v->maximum());
    if (v->step()!=f->step()) write_string("step %g",v->step());
    if (v->value()!=0.0) write_string("value %g",v->value());
    if (is_valuator()>=2) {
      double x = ((Fl_Slider*)v)->slider_size();
      double y = ((Fl_Slider*)f)->slider_size();
      if (x != y) write_string("slider_size %g", x);
    }
  }
  if (is_spinner()) {
    Fl_Spinner* v = (Fl_Spinner*)o;
    Fl_Spinner* f = (Fl_Spinner*)(tplate);
    if (v->minimum()!=f->minimum()) write_string("minimum %g",v->minimum());
    if (v->maximum()!=f->maximum()) write_string("maximum %g",v->maximum());
    if (v->step()!=f->step()) write_string("step %g",v->step());
    if (v->value()!=1.0) write_string("value %g",v->value());
  }
  {Fl_Font ff; int fs; Fl_Color fc; if (textstuff(4,ff,fs,fc)) {
    Fl_Font f; int s; Fl_Color c; textstuff(0,f,s,c);
    if (f != ff) write_string("textfont %d", f);
    if (s != fs) write_string("textsize %d", s);
    if (c != fc) write_string("textcolor %d", c);
  }}
  if (!o->visible()) write_string("hide");
  if (!o->active()) write_string("deactivate");
  if (resizable()) write_string("resizable");
  if (hotspot()) write_string(is_menu_item() ? "divider" : "hotspot");
  for (int n=0; n < NUM_EXTRA_CODE; n++) if (extra_code(n)) {
    write_indent(level+1);
    write_string("code%d",n);
    write_word(extra_code(n));
  }
  if (subclass()) {
    write_indent(level+1);
    write_string("class");
    write_word(subclass());
  }
}

int pasteoffset;
extern double read_version;
void Fl_Widget_Type::read_property(const char *c) {
  int x,y,w,h; Fl_Font f; int s; Fl_Color cc;
  if (!strcmp(c,"private")) {
    public_ = 0;
  } else if (!strcmp(c,"protected")) {
    public_ = 2;
  } else if (!strcmp(c,"xywh")) {
    if (sscanf(read_word(),"%d %d %d %d",&x,&y,&w,&h) == 4) {
      x += pasteoffset;
      y += pasteoffset;
      // FIXME temporary change!
      if (read_version>=2.0 && o->parent() && o->parent()!=o->window()) {
        x += o->parent()->x();
        y += o->parent()->y();
      }
      o->resize(x,y,w,h);
    }
  } else if (!strcmp(c,"tooltip")) {
    tooltip(read_word());
  } else if (!strcmp(c,"image")) {
    image_name(read_word());
  } else if (!strcmp(c,"deimage")) {
    inactive_name(read_word());
  } else if (!strcmp(c,"type")) {
    if (is_spinner()) 
      ((Fl_Spinner*)o)->type(item_number(subtypes(), read_word()));
    else
      o->type(item_number(subtypes(), read_word()));
  } else if (!strcmp(c,"box")) {
    const char* value = read_word();
    if ((x = boxnumber(value))) {
      if (x == ZERO_ENTRY) x = 0;
      o->box((Fl_Boxtype)x);
    } else if (sscanf(value,"%d",&x) == 1) o->box((Fl_Boxtype)x);
  } else if (is_button() && !strcmp(c,"down_box")) {
    const char* value = read_word();
    if ((x = boxnumber(value))) {
      if (x == ZERO_ENTRY) x = 0;
      ((Fl_Button*)o)->down_box((Fl_Boxtype)x);
    }
  } else if (!strcmp(type_name(), "Fl_Input_Choice") && !strcmp(c,"down_box")) {
    const char* value = read_word();
    if ((x = boxnumber(value))) {
      if (x == ZERO_ENTRY) x = 0;
      ((Fl_Input_Choice*)o)->down_box((Fl_Boxtype)x);
    }
  } else if (is_menu_button() && !strcmp(c,"down_box")) {
    const char* value = read_word();
    if ((x = boxnumber(value))) {
      if (x == ZERO_ENTRY) x = 0;
      ((Fl_Menu_*)o)->down_box((Fl_Boxtype)x);
    }
  } else if (is_button() && !strcmp(c,"value")) {
    const char* value = read_word();
    ((Fl_Button*)o)->value(atoi(value));
  } else if (!strcmp(c,"color")) {
    const char *cw = read_word();
    if (cw[0]=='0' && cw[1]=='x') {
      sscanf(cw,"0x%x",&x);
      o->color(x);
    } else {
      int n = sscanf(cw,"%d %d",&x,&y);
      if (n == 2) { // back compatibility...
        if (x != 47) o->color(x);
        o->selection_color(y);
      } else {
        o->color(x);
      }
    }
  } else if (!strcmp(c,"selection_color")) {
    if (sscanf(read_word(),"%d",&x)) o->selection_color(x);
  } else if (!strcmp(c,"labeltype")) {
    c = read_word();
    if (!strcmp(c,"image")) {
      Fluid_Image *i = Fluid_Image::find(label());
      if (!i) read_error("Image file '%s' not found", label());
      else setimage(i);
      image_name(label());
      label("");
    } else {
      o->labeltype((Fl_Labeltype)item_number(labeltypemenu,c));
    }
  } else if (!strcmp(c,"labelfont")) {
    if (sscanf(read_word(),"%d",&x) == 1) o->labelfont(x);
  } else if (!strcmp(c,"labelsize")) {
    if (sscanf(read_word(),"%d",&x) == 1) o->labelsize(x);
  } else if (!strcmp(c,"labelcolor")) {
    if (sscanf(read_word(),"%d",&x) == 1) o->labelcolor(x);
  } else if (!strcmp(c,"align")) {
    if (sscanf(read_word(),"%d",&x) == 1) o->align(x);
  } else if (!strcmp(c,"when")) {
    if (sscanf(read_word(),"%d",&x) == 1) o->when(x);
  } else if (!strcmp(c,"minimum")) {
    if (is_valuator()) ((Fl_Valuator*)o)->minimum(strtod(read_word(),0));
    if (is_spinner()) ((Fl_Spinner*)o)->minimum(strtod(read_word(),0));
  } else if (!strcmp(c,"maximum")) {
    if (is_valuator()) ((Fl_Valuator*)o)->maximum(strtod(read_word(),0));
    if (is_spinner()) ((Fl_Spinner*)o)->maximum(strtod(read_word(),0));
  } else if (!strcmp(c,"step")) {
    if (is_valuator()) ((Fl_Valuator*)o)->step(strtod(read_word(),0));
    if (is_spinner()) ((Fl_Spinner*)o)->step(strtod(read_word(),0));
  } else if (!strcmp(c,"value")) {
    if (is_valuator()) ((Fl_Valuator*)o)->value(strtod(read_word(),0));
    if (is_spinner()) ((Fl_Spinner*)o)->value(strtod(read_word(),0));
  } else if ((!strcmp(c,"slider_size")||!strcmp(c,"size"))&&is_valuator()==2) {
    ((Fl_Slider*)o)->slider_size(strtod(read_word(),0));
  } else if (!strcmp(c,"textfont")) {
    if (sscanf(read_word(),"%d",&x) == 1) {f=(Fl_Font)x; textstuff(1,f,s,cc);}
  } else if (!strcmp(c,"textsize")) {
    if (sscanf(read_word(),"%d",&x) == 1) {s=x; textstuff(2,f,s,cc);}
  } else if (!strcmp(c,"textcolor")) {
    if (sscanf(read_word(),"%d",&x) == 1) {cc=(Fl_Color)x;textstuff(3,f,s,cc);}
  } else if (!strcmp(c,"hide")) {
    o->hide();
  } else if (!strcmp(c,"deactivate")) {
    o->deactivate();
  } else if (!strcmp(c,"resizable")) {
    resizable(1);
  } else if (!strcmp(c,"hotspot") || !strcmp(c, "divider")) {
    hotspot(1);
  } else if (!strcmp(c,"class")) {
    subclass(read_word());
  } else if (!strcmp(c,"shortcut")) {
    int shortcut = strtol(read_word(),0,0);
    if (is_button()) ((Fl_Button*)o)->shortcut(shortcut);
    else if (is_input()) ((Fl_Input_*)o)->shortcut(shortcut);
    else if (is_value_input()) ((Fl_Value_Input*)o)->shortcut(shortcut);
    else if (is_text_display()) ((Fl_Text_Display*)o)->shortcut(shortcut);
  } else {
    if (!strncmp(c,"code",4)) {
      int n = atoi(c+4);
      if (n >= 0 && n <= NUM_EXTRA_CODE) {
        extra_code(n,read_word());
        return;
      }
    } else if (!strcmp(c,"extra_code")) {
      extra_code(0,read_word());
      return;
    }
    Fl_Type::read_property(c);
  }
}

Fl_Menu_Item boxmenu1[] = {
  // these extra ones are for looking up fdesign saved strings:
  {"NO_FRAME",		0,0,(void *)FL_NO_BOX},
  {"ROUNDED3D_UPBOX",	0,0,(void *)_FL_ROUND_UP_BOX},
  {"ROUNDED3D_DOWNBOX",	0,0,(void *)_FL_ROUND_DOWN_BOX},
  {"OVAL3D_UPBOX",	0,0,(void *)_FL_ROUND_UP_BOX},
  {"OVAL3D_DOWNBOX",	0,0,(void *)_FL_ROUND_DOWN_BOX},
  {"0",			0,0,(void *)ZERO_ENTRY},
  {"1",			0,0,(void *)FL_UP_BOX},
  {"2",			0,0,(void *)FL_DOWN_BOX},
  {"3",			0,0,(void *)FL_FLAT_BOX},
  {"4",			0,0,(void *)FL_BORDER_BOX},
  {"5",			0,0,(void *)FL_SHADOW_BOX},
  {"6",			0,0,(void *)FL_FRAME_BOX},
  {"7",			0,0,(void *)FL_ROUNDED_BOX},
  {"8",			0,0,(void *)FL_RFLAT_BOX},
  {"9",			0,0,(void *)FL_RSHADOW_BOX},
  {"10",		0,0,(void *)FL_UP_FRAME},
  {"11",		0,0,(void *)FL_DOWN_FRAME},
{0}};

extern int fdesign_flip;
int lookup_symbol(const char *, int &, int numberok = 0);

int Fl_Widget_Type::read_fdesign(const char* propname, const char* value) {
  int v;
  if (!strcmp(propname,"box")) {
    float x,y,w,h;
    if (sscanf(value,"%f %f %f %f",&x,&y,&w,&h) == 4) {
      if (fdesign_flip) {
	Fl_Type *p;
	for (p = parent; p && !p->is_window(); p = p->parent) {/*empty*/}
	if (p && p->is_widget()) y = ((Fl_Widget_Type*)p)->o->h()-(y+h);
      }	  
      x += pasteoffset;
      y += pasteoffset;
      o->resize(int(x),int(y),int(w),int(h));
    }
  } else if (!strcmp(propname,"label")) {
    label(value);
  } else if (!strcmp(propname,"name")) {
    this->name(value);
  } else if (!strcmp(propname,"callback")) {
    callback(value); user_data_type("long");
  } else if (!strcmp(propname,"argument")) {
    user_data(value);
  } else if (!strcmp(propname,"shortcut")) {
    if (value[0]) {
      char buf[128]; sprintf(buf,"o->shortcut(\"%s\");",value);
      extra_code(0,buf);
    }
  } else if (!strcmp(propname,"style")) {
    if (!strncmp(value,"FL_NORMAL",9)) return 1;
    if (!lookup_symbol(value,v,1)) return 0;
    o->labelfont(v); o->labeltype((Fl_Labeltype)(v>>8));
  } else if (!strcmp(propname,"size")) {
    if (!lookup_symbol(value,v,1)) return 0;
    o->labelsize(v);
  } else if (!strcmp(propname,"type")) {
    if (!strncmp(value,"NORMAL",6)) return 1;
    if (lookup_symbol(value,v,1)) {o->type(v); return 1;}
    if (!strcmp(value+strlen(value)-5,"FRAME")) goto TRY_BOXTYPE;
    if (!strcmp(value+strlen(value)-3,"BOX")) goto TRY_BOXTYPE;
    return 0;
  } else if (!strcmp(propname,"lcol")) {
    if (!lookup_symbol(value,v,1)) return 0;
    o->labelcolor(v);
  } else if (!strcmp(propname,"return")) {
    if (!lookup_symbol(value,v,0)) return 0;
    o->when(v|FL_WHEN_RELEASE);
  } else if (!strcmp(propname,"alignment")) {
    if (!lookup_symbol(value,v)) {
      // convert old numeric values:
      int v1 = atoi(value); if (v1 <= 0 && strcmp(value,"0")) return 0;
      v = 0;
      if (v1 >= 5) {v = FL_ALIGN_INSIDE; v1 -= 5;}
      switch (v1) {
      case 0: v += FL_ALIGN_TOP; break;
      case 1: v += FL_ALIGN_BOTTOM; break;
      case 2: v += FL_ALIGN_LEFT; break;
      case 3: v += FL_ALIGN_RIGHT; break;
      case 4: v += FL_ALIGN_CENTER; break;
      default: return 0;
      }
    }
    o->align(v);
  } else if (!strcmp(propname,"resizebox")) {
    resizable(1);
  } else if (!strcmp(propname,"colors")) {
    char* p = (char*)value;
    while (*p != ' ') {if (!*p) return 0; p++;}
    *p = 0;
    int v1;
    if (!lookup_symbol(value,v,1) || !lookup_symbol(p+1,v1,1)) {
      *p=' '; return 0;}
    o->color(v,v1);
  } else if (!strcmp(propname,"resize")) {
    return !strcmp(value,"FL_RESIZE_ALL");
  } else if (!strcmp(propname,"gravity")) {
    return !strcmp(value,"FL_NoGravity FL_NoGravity");
  } else if (!strcmp(propname,"boxtype")) {
  TRY_BOXTYPE:
    int x = boxnumber(value);
    if (!x) {x = item_number(boxmenu1, value); if (x < 0) return 0;}
    if (x == ZERO_ENTRY) {
      x = 0;
      if (o->box() != ((Fl_Widget_Type*)factory)->o->box()) return 1; // kludge for frame
    }
    o->box((Fl_Boxtype)x);
  } else {
    return 0;
  }
  return 1;
}

void leave_live_mode_cb(Fl_Widget*, void*) {
  live_mode_cb(0, 0);
}

Fl_Widget *Fl_Widget_Type::enter_live_mode(int) {
  live_widget = widget(o->x(), o->y(), o->w(), o->h());
  if (live_widget)
    copy_properties();
  return live_widget;
}

void Fl_Widget_Type::leave_live_mode() {
}

/**
 * copy all properties from the edit widget to the live widget
 */
void Fl_Widget_Type::copy_properties() {
  if (!live_widget) 
    return;

  // copy all attributes common to all widget types
  Fl_Widget *w = live_widget;
  w->label(o->label());
  w->tooltip(tooltip());
  w->type(o->type());
  w->box(o->box());
  w->color(o->color());
  w->selection_color(o->selection_color());
  w->labeltype(o->labeltype());
  w->labelfont(o->labelfont());
  w->labelsize(o->labelsize());
  w->labelcolor(o->labelcolor());
  w->align(o->align());

  // copy all attributes specific to widgets derived from Fl_Button
  if (is_button()) {
    Fl_Button* d = (Fl_Button*)live_widget, *s = (Fl_Button*)o;
    d->down_box(s->down_box());
    d->shortcut(s->shortcut());
    d->value(s->value());
  }

  // copy all attributes specific to widgets derived from Fl_Input_
  if (is_input()) {
    Fl_Input_* d = (Fl_Input_*)live_widget, *s = (Fl_Input_*)o;
    d->shortcut(s->shortcut());
  }

  // copy all attributes specific to widgets derived from Fl_Value_Input
  if (is_value_input()) {
    Fl_Value_Input* d = (Fl_Value_Input*)live_widget, *s = (Fl_Value_Input*)o;
    d->shortcut(s->shortcut());
  }

  // copy all attributes specific to widgets derived from Fl_Text_Display
  if (is_text_display()) {
    Fl_Text_Display* d = (Fl_Text_Display*)live_widget, *s = (Fl_Text_Display*)o;
    d->shortcut(s->shortcut());
  }

  // copy all attributes specific to Fl_Valuator and derived classes
  if (is_valuator()) {
    Fl_Valuator* d = (Fl_Valuator*)live_widget, *s = (Fl_Valuator*)o;
    d->minimum(s->minimum());
    d->maximum(s->maximum());
    d->step(s->step());
    d->value(s->value());
    if (is_valuator()>=2) {
      Fl_Slider *d = (Fl_Slider*)live_widget, *s = (Fl_Slider*)o;
      d->slider_size(s->slider_size());
    }
  }

  // copy all attributes specific to Fl_Spinner and derived classes
  if (is_spinner()) {
    Fl_Spinner* d = (Fl_Spinner*)live_widget, *s = (Fl_Spinner*)o;
    d->minimum(s->minimum());
    d->maximum(s->maximum());
    d->step(s->step());
    d->value(s->value());
  }
 
/* TODO: implement this
  {Fl_Font ff; int fs; Fl_Color fc; if (textstuff(4,ff,fs,fc)) {
    Fl_Font f; int s; Fl_Color c; textstuff(0,f,s,c);
    if (f != ff) write_string("textfont %d", f);
    if (s != fs) write_string("textsize %d", s);
    if (c != fc) write_string("textcolor %d", c);
  }}*/

  if (!o->visible()) 
    w->hide();
  if (!o->active()) 
    w->deactivate();
  if (resizable() && w->parent()) 
    w->parent()->resizable(o);
}

void Fl_Pack_Type::copy_properties()
{
  Fl_Group_Type::copy_properties();
  Fl_Pack *d = (Fl_Pack*)live_widget, *s =(Fl_Pack*)o;
  d->spacing(s->spacing());
}

//
// End of "$Id$".
//
