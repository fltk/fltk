// Fl_Menu_.C

// This is a base class for all items that have a menu:
//	Fl_Menu_Bar, Fl_Menu_Button, Fl_Choice
// This provides storage for a menu item, functions to add/modify/delete
// items, and a call for when the user picks a menu item.

// More code in Fl_Menu_add.C

#include <FL/Fl.H>
#include <FL/Fl_Menu_.H>
#include <stdlib.h>

Fl_Font Fl_Menu_::default_font_;
int Fl_Menu_::default_size_;

int Fl_Menu_::value(const Fl_Menu_Item* m) {
  clear_changed();
  if (value_ != m) {value_ = m; return 1;}
  return 0;
}

// When user picks a menu item, call this.  It will do the callback.
// Unfortunatly this also casts away const for the checkboxes, but this
// was necessary so non-checkbox menus can really be declared const...
const Fl_Menu_Item* Fl_Menu_::picked(const Fl_Menu_Item* v) {
  if (v) {
    if (v->radio()) {
      if (!v->value()) { // they are turning on a radio item
	set_changed();
	((Fl_Menu_Item*)v)->setonly();
      }
    } else if (v->flags & FL_MENU_TOGGLE) {
      set_changed();
      ((Fl_Menu_Item*)v)->flags ^= FL_MENU_VALUE;
    } else if (v != value_) { // normal item
      set_changed();
    }
    value_ = v;
    if (when()&(FL_WHEN_CHANGED|FL_WHEN_RELEASE)) {
      if (changed() || when()&FL_WHEN_NOT_CHANGED) {
	clear_changed();
	if (value_ && value_->callback_) value_->do_callback((Fl_Widget*)this);
	else do_callback();
      }
    }
  }
  return v;
}

// turn on one of a set of radio buttons
void Fl_Menu_Item::setonly() {
  flags |= FL_MENU_RADIO | FL_MENU_VALUE;
  Fl_Menu_Item* j;
  for (j = this; ; ) {	// go down
    if (j->flags & FL_MENU_DIVIDER) break; // stop on divider lines
    j++;
    if (!j->text || !j->radio()) break; // stop after group
    j->clear();
  }
  for (j = this-1; ; j--) { // go up
    if (!j->text || (j->flags&FL_MENU_DIVIDER) || !j->radio()) break;
    j->clear();
  }
}

Fl_Menu_::Fl_Menu_(int X,int Y,int W,int H,const char* l)
: Fl_Widget(X,Y,W,H,l) {
  set_flag(SHORTCUT_LABEL);
  box(FL_UP_BOX);
  when(FL_WHEN_RELEASE_ALWAYS);
  value_ = menu_ = 0;
  alloc = 0;
  selection_color(FL_SELECTION_COLOR);
  textfont(FL_HELVETICA);
  textsize(FL_NORMAL_SIZE);
  textcolor(FL_BLACK);
  down_box(FL_NO_BOX);
}

int Fl_Menu_::size() const {
  if (!menu_) return 0;
  return menu_->size();
}

void Fl_Menu_::menu(const Fl_Menu_Item* m) {
  // if (alloc) clear();
  alloc = 0;
  value_ = menu_ = (Fl_Menu_Item*)m;
}

Fl_Menu_::~Fl_Menu_() {
  // if (alloc) clear();
}

// end of Fl_Menu_.C
