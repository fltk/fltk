//
// "$Id$"
//
// Common menu code for the Fast Light Tool Kit (FLTK).
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

// This is a base class for all items that have a menu:
//	Fl_Menu_Bar, Fl_Menu_Button, Fl_Choice
// This provides storage for a menu item, functions to add/modify/delete
// items, and a call for when the user picks a menu item.

// More code in Fl_Menu_add.cxx

#include <FL/Fl.H>
#include <FL/Fl_Menu_.H>
#include "flstring.h"
#include <stdio.h>
#include <stdlib.h>

#define SAFE_STRCAT(s) { len += (int) strlen(s); if ( len >= namelen ) { *name='\0'; return(-2); } else strcat(name,(s)); }

/** Get the menu 'pathname' for the specified menuitem.

    If finditem==NULL, mvalue() is used (the most recently picked menuitem).

    \b Example:
    \code
      Fl_Menu_Bar *menubar = 0;
      void my_menu_callback(Fl_Widget*,void*) {
        char name[80];
        if ( menubar->item_pathname(name, sizeof(name)-1) == 0 ) {   // recently picked item
          if ( strcmp(name, "File/&Open") == 0 ) { .. }              // open invoked
          if ( strcmp(name, "File/&Save") == 0 ) { .. }              // save invoked
          if ( strcmp(name, "Edit/&Copy") == 0 ) { .. }              // copy invoked
        }
      }
      int main() {
        [..]
        menubar = new Fl_Menu_Bar(..);
        menubar->add("File/&Open",  0, my_menu_callback);
        menubar->add("File/&Save",  0, my_menu_callback);
        menubar->add("Edit/&Copy",  0, my_menu_callback);
        [..]
      }
    \endcode

    \returns
	-   0 : OK (name has menuitem's pathname)
	-  -1 : item not found (name="")
	-  -2 : 'name' not large enough (name="")
    \see find_item()
*/
int Fl_Menu_::item_pathname(char *name, int namelen, const Fl_Menu_Item *finditem) const {
  name[0] = '\0';
  return item_pathname_(name, namelen, finditem, menu_);
}

// INTERNAL: Descend into a specific menu hierarchy
int Fl_Menu_::item_pathname_(char *name, 
			     int namelen,
			     const Fl_Menu_Item *finditem,
			     const Fl_Menu_Item *menu) const {
  int len = 0;
  int level = 0;
  finditem = finditem ? finditem : mvalue();    
  menu = menu ? menu : this->menu();
  for ( int t=0; t<size(); t++ ) {
    const Fl_Menu_Item *m = menu + t;
    if (m->submenu()) {				// submenu? descend
      if (m->flags & FL_SUBMENU_POINTER) {
        // SUBMENU POINTER? Recurse to descend
        int slen = strlen(name);
	const Fl_Menu_Item *submenu = (const Fl_Menu_Item*)m->user_data();
        if (m->label()) {
          if (*name) SAFE_STRCAT("/");
	  SAFE_STRCAT(m->label());
        }
        if (item_pathname_(name, len, finditem, submenu) == 0)
	  return 0;
	name[slen] = 0;				// continue from where we were
      } else {
        // REGULAR SUBMENU? DESCEND
	++level;
        if (*name) SAFE_STRCAT("/");
        if (m->label()) SAFE_STRCAT(m->label());
        if (m == finditem) return(0);		// found? done.
      }
    } else {
      if (m->label()) {				// menu item?
	if ( m == finditem ) {			// found? tack on itemname, done.
	  SAFE_STRCAT("/");
	  SAFE_STRCAT(m->label());
	  return(0);
	}
      } else {					// end of submenu? pop
        if ( --level < 0 ) {
	  *name = '\0';
	  return -1;
	}
	char *ss = strrchr(name, '/');
	if ( ss ) { *ss = 0; len = (int) strlen(name); }	// "File/Edit" -> "File"
	else { name[0] = '\0'; len = 0; }	// "File" -> ""
	continue;
      }
    }
  }
  *name = '\0';
  return(-1);					// item not found
}

/**
 Find the menu item for a given menu \p pathname, such as "Edit/Copy".
 
 This method finds a menu item in the menu array, also traversing submenus, but
 not submenu pointers (FL_SUBMENU_POINTER).

 To get the menu item's index, use find_index(const char*)

  \b Example:
  \code
    Fl_Menu_Bar *menubar = new Fl_Menu_Bar(..);
    menubar->add("File/&Open");
    menubar->add("File/&Save");
    menubar->add("Edit/&Copy");
    // [..]
    Fl_Menu_Item *item;
    if ( ( item = (Fl_Menu_Item*)menubar->find_item("File/&Open") ) != NULL ) {
	item->labelcolor(FL_RED);
    }
    if ( ( item = (Fl_Menu_Item*)menubar->find_item("Edit/&Copy") ) != NULL ) {
	item->labelcolor(FL_GREEN);
    }
  \endcode

  \param pathname The path and name of the menu item
  \returns The item found, or NULL if not found
  \see find_index(const char*), find_item(Fl_Callback*), item_pathname() 
*/
const Fl_Menu_Item * Fl_Menu_::find_item(const char *pathname) {
  int i = find_index(pathname);
  return( (i==-1) ? 0 : (const Fl_Menu_Item*)(menu_+i));
}

/**
 Find the index into the menu array for a given \p item.

 A way to convert a menu item pointer into an index.

 Does \b not handle items that are in submenu pointers (FL_SUBMENU_POINTER).

 -1 is returned if the item is not in this menu
 or is part of an FL_SUBMENU_POINTER submenu.

 Current implementation is fast and not expensive.

 \code
   // Convert an index-to-item
   int index = 12;
   const Fl_Menu_Item *item = mymenu->menu() + index;

   // Convert an item-to-index
   int index = mymenu->find_index(item);
   if ( index == -1 ) { ..error.. }
 \endcode

 \param[in]  item  The item to be found
 \returns    The index of the item, or -1 if not found.
 \see        menu()
*/
int Fl_Menu_::find_index(const Fl_Menu_Item *item) const {
  Fl_Menu_Item *max = menu_+size();
  if (item<menu_ || item>=max) return(-1);
  return (int) (item-menu_);
}

/**
 Find the index into the menu array for a given callback \p cb.
 
 This method finds a menu item's index position, also traversing submenus, but
 \b not submenu pointers (FL_SUBMENU_POINTER). This is useful if an
 application uses internationalisation and a menu item can not be found
 using its label. This search is also much faster.

 \param cb Find the first item with this callback
 \returns  The index of the item with the specific callback, or -1 if not found
 \see      find_index(const char*)
 */
int Fl_Menu_::find_index(Fl_Callback *cb) const {
  for ( int t=0; t < size(); t++ )
    if (menu_[t].callback_==cb)
      return(t);
  return(-1);
}

/**
 Find the menu item index for a given menu \p pathname, such as "Edit/Copy".
 
 This method finds a menu item's index position for the given menu pathname,
 also traversing submenus, but \b not submenu pointers (FL_SUBMENU_POINTER).

 To get the menu item pointer for a pathname, use find_item()

 \param[in] pathname The path and name of the menu item to find
 \returns        The index of the matching item, or -1 if not found.
 \see            item_pathname()

*/
int Fl_Menu_::find_index(const char *pathname) const {
  char menupath[1024] = "";	// File/Export
  for ( int t=0; t < size(); t++ ) {
    Fl_Menu_Item *m = menu_ + t;
    if (m->flags&FL_SUBMENU) {
      // IT'S A SUBMENU
      // we do not support searches through FL_SUBMENU_POINTER links
      if (menupath[0]) strlcat(menupath, "/", sizeof(menupath));
      strlcat(menupath, m->label(), sizeof(menupath));
      if (!strcmp(menupath, pathname)) return(t);
    } else {
      if (!m->label()) {
	// END OF SUBMENU? Pop back one level.
	char *ss = strrchr(menupath, '/');
	if ( ss ) *ss = 0;
	else menupath[0] = '\0';
	continue;
      }
      // IT'S A MENU ITEM
      char itempath[1024];	// eg. Edit/Copy
      strcpy(itempath, menupath);
      if (itempath[0]) strlcat(itempath, "/", sizeof(itempath));
      strlcat(itempath, m->label(), sizeof(itempath));
      if (!strcmp(itempath, pathname)) return(t);
    }
  }
  return(-1);
}

/**
 Find the menu item for the given callback \p cb.
 
 This method finds a menu item in a menu array, also traversing submenus, but
 not submenu pointers. This is useful if an application uses 
 internationalisation and a menu item can not be found using its label. This
 search is also much faster.
 
 \param[in] cb find the first item with this callback
 \returns The item found, or NULL if not found
 \see find_item(const char*)
 */
const Fl_Menu_Item * Fl_Menu_::find_item(Fl_Callback *cb) {
  for ( int t=0; t < size(); t++ ) {
    const Fl_Menu_Item *m = menu_ + t;
    if (m->callback_==cb) {
      return m;
    }
  }
  return (const Fl_Menu_Item *)0;
}

/**
  The value is the index into menu() of the last item chosen by
  the user.  It is zero initially.  You can set it as an integer, or set
  it with a pointer to a menu item.  The set routines return non-zero if
  the new value is different than the old one.
*/
int Fl_Menu_::value(const Fl_Menu_Item* m) {
  clear_changed();
  if (value_ != m) {value_ = m; return 1;}
  return 0;
}

/** 
 When user picks a menu item, call this.  It will do the callback.
 Unfortunately this also casts away const for the checkboxes, but this
 was necessary so non-checkbox menus can really be declared const...
*/
const Fl_Menu_Item* Fl_Menu_::picked(const Fl_Menu_Item* v) {
  if (v) {
    if (v->radio()) {
      if (!v->value()) { // they are turning on a radio item
        set_changed();
        setonly((Fl_Menu_Item*)v);
      }
      redraw();
    } else if (v->flags & FL_MENU_TOGGLE) {
      set_changed();
      ((Fl_Menu_Item*)v)->flags ^= FL_MENU_VALUE;
      redraw();
    } else if (v != value_) { // normal item
      set_changed();
    }
    value_ = v;
    if (when()&(FL_WHEN_CHANGED|FL_WHEN_RELEASE)) {
      if (changed() || when()&FL_WHEN_NOT_CHANGED) {
	if (value_ && value_->callback_) value_->do_callback((Fl_Widget*)this);
	else do_callback();
      }
    }
  }
  return v;
}

/* Scans an array of Fl_Menu_Item's that begins at start, searching for item.
 Returns NULL if item is not found.
 If item is present, returns start, unless item belongs to an
 FL_SUBMENU_POINTER-adressed array of items, in which case the first item of this array is returned.
 */
static Fl_Menu_Item *first_submenu_item(Fl_Menu_Item *item, Fl_Menu_Item *start)
{
  Fl_Menu_Item* m = start;
  int nest = 0; // will indicate submenu nesting depth
  while (1) { // loop over all menu items
    if (!m->text) { // m is a null item
      if (!nest) return NULL; // item was not found
      nest--; // m marks the end of a submenu -> decrement submenu nesting depth
    } else { // a true item
      if (m == item) return start; // item is found, return menu start item
      if (m->flags & FL_SUBMENU_POINTER) {
        // scan the detached submenu which begins at m->user_data()
        Fl_Menu_Item *first = first_submenu_item(item, (Fl_Menu_Item*)m->user_data());
        if (first) return first; // if item was found in the submenu, return
      }
      else if (m->flags & FL_SUBMENU) { // a direct submenu
        nest++; // increment submenu nesting depth
      }
    }
    m++; // step to next menu item
  }
}


/** Turns the radio item "on" for the menu item and turns "off" adjacent radio items of the same group. */
void Fl_Menu_::setonly(Fl_Menu_Item* item) {
  // find the first item of the (sub)menu containing item
  Fl_Menu_Item* first = first_submenu_item(item, menu_);
  if (!first) return; // item does not belong to our menu
  item->flags |= FL_MENU_RADIO | FL_MENU_VALUE;
  Fl_Menu_Item* j;
  for (j = item; ; ) {	// go down
    if (j->flags & FL_MENU_DIVIDER) break; // stop on divider lines
    j++;
    if (!j->text || !j->radio()) break; // stop after group
    j->clear();
  }
  for (j = item-1; j>=first; j--) { // go up
    //DEBUG printf("GO UP: WORKING ON: item='%s', flags=%x\n", j->text, j->flags);
    if (!j->text || (j->flags&FL_MENU_DIVIDER) || !j->radio()) break;
    j->clear();
  }
}

/** Turns the radio item "on" for the menu item and turns "off" adjacent radio items set.
 \deprecated This method is dangerous if radio items are first in the menu.
 Use Fl_Menu_::setonly(Fl_Menu_Item*) instead.
 */
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

/**
 Creates a new Fl_Menu_ widget using the given position, size,
 and label string.  menu() is initialized to null.
 */
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
  textcolor(FL_FOREGROUND_COLOR);
  down_box(FL_NO_BOX);
}

/**
  This returns the number of Fl_Menu_Item structures that make up the
  menu, correctly counting submenus.  This includes the "terminator"
  item at the end.  To copy a menu array you need to copy
  size()*sizeof(Fl_Menu_Item) bytes.  If the menu is
  NULL this returns zero (an empty menu will return 1).
*/
int Fl_Menu_::size() const {
  if (!menu_) return 0;
  return menu_->size();
}

/**
    Sets the menu array pointer directly.  If the old menu is private it is
    deleted.  NULL is allowed and acts the same as a zero-length
    menu.  If you try to modify the array (with add(), replace(), or
    remove()) a private copy is automatically done.
*/
void Fl_Menu_::menu(const Fl_Menu_Item* m) {
  clear();
  value_ = menu_ = (Fl_Menu_Item*)m;
}

// this version is ok with new Fl_Menu_add code with fl_menu_array_owner:

/** 
  Sets the menu array pointer with a copy of m that will be automatically deleted. 
  If userdata \p ud is not NULL, then all user data pointers are changed in the menus as well.
  See void Fl_Menu_::menu(const Fl_Menu_Item* m). 
*/
void Fl_Menu_::copy(const Fl_Menu_Item* m, void* ud) {
  int n = m->size();
  Fl_Menu_Item* newMenu = new Fl_Menu_Item[n];
  memcpy(newMenu, m, n*sizeof(Fl_Menu_Item));
  menu(newMenu);
  alloc = 1; // make destructor free array, but not strings
  // for convenience, provide way to change all the user data pointers:
  if (ud) for (; n--;) {
    if (newMenu->callback_) newMenu->user_data_ = ud;
    newMenu++;
  }
}

Fl_Menu_::~Fl_Menu_() {
  clear();
}

// Fl_Menu::add() uses this to indicate the owner of the dynamically-
// expanding array.  We must not free this array:
Fl_Menu_* fl_menu_array_owner = 0;

/**
  Same as menu(NULL), set the array pointer to null, indicating
  a zero-length menu.
  
  Menus must not be cleared during a callback to the same menu.
*/
void Fl_Menu_::clear() {
  if (alloc) {
    if (alloc>1) for (int i = size(); i--;)
      if (menu_[i].text) free((void*)menu_[i].text);
    if (this == fl_menu_array_owner)
      fl_menu_array_owner = 0;
    else
      delete[] menu_;
    menu_ = 0;
    value_ = 0;
    alloc = 0;
  }
}

/**
 Clears the specified submenu pointed to by \p index of all menu items.

 This method is useful for clearing a submenu so that it can be
 re-populated with new items. Example: a "File/Recent Files/..." submenu
 that shows the last few files that have been opened.

 The specified \p index must point to a submenu.
 
 The submenu is cleared with remove().
 If the menu array was directly set with menu(x), then copy() 
 is done to make a private array.

 \warning Since this method can change the internal menu array, any menu
 item pointers or indices the application may have cached can become
 stale, and should be recalculated/refreshed.

 \b Example:
 \code
   int index = menubar->find_index("File/Recent");    // get index of "File/Recent" submenu
   if ( index != -1 ) menubar->clear_submenu(index);  // clear the submenu
   menubar->add("File/Recent/Aaa");
   menubar->add("File/Recent/Bbb");
   [..]
 \endcode

 \param index The index of the submenu to be cleared
 \returns 0 on success, -1 if the index is out of range or not a submenu
 \see remove(int)
 */
int Fl_Menu_::clear_submenu(int index) {
  if ( index < 0 || index >= size() ) return(-1);
  if ( ! (menu_[index].flags & FL_SUBMENU) ) return(-1);
  ++index;					// advance to first item in submenu
  while ( index < size() ) {                    // keep remove()ing top item until end is reached
    if ( menu_[index].text == 0 ) break;	// end of this submenu? done
    remove(index);				// remove items/submenus
  }
  return(0);
}

//
// End of "$Id$".
//
