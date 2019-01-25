//
// "$Id$"
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Tree_Item.H>
#include <FL/Fl_Tree_Prefs.H>
#include <FL/Fl_Tree.H>

//////////////////////
// Fl_Tree_Item.cxx
//////////////////////
//
// Fl_Tree -- This file is part of the Fl_Tree widget for FLTK
// Copyright (C) 2009-2010 by Greg Ercolano.
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
/////////////////////////////////////////////////////////////////////////// 80 /

// Was the last event inside the specified xywh?
static int event_inside(const int xywh[4]) {
  return(Fl::event_inside(xywh[0],xywh[1],xywh[2],xywh[3]));
}

/// Constructor.
/// Makes a new instance of Fl_Tree_Item using defaults from \p 'prefs'.
#if FLTK_ABI_VERSION >= 10303
/// \deprecated in 1.3.3 ABI -- you must use Fl_Tree_Item(Fl_Tree*) for proper horizontal scrollbar behavior.
#endif
///
Fl_Tree_Item::Fl_Tree_Item(const Fl_Tree_Prefs &prefs) {
  _Init(prefs, 0);
}

// Initialize the tree item
//    Used by constructors
//
void Fl_Tree_Item::_Init(const Fl_Tree_Prefs &prefs, Fl_Tree *tree) {
#if FLTK_ABI_VERSION >= 10303
  _tree         = tree;
#endif
  _label        = 0;
  _labelfont    = prefs.labelfont();
  _labelsize    = prefs.labelsize();
  _labelfgcolor = prefs.labelfgcolor();
  _labelbgcolor = prefs.labelbgcolor();
  _widget       = 0;
#if FLTK_ABI_VERSION >= 10301
  _flags        = OPEN|VISIBLE|ACTIVE;
#else /*FLTK_ABI_VERSION*/
  _open         = 1;
  _visible      = 1;
  _active       = 1;
  _selected     = 0;
#endif /*FLTK_ABI_VERSION*/
  _xywh[0]      = 0;
  _xywh[1]      = 0;
  _xywh[2]      = 0;
  _xywh[3]      = 0;
  _collapse_xywh[0] = 0;
  _collapse_xywh[1] = 0;
  _collapse_xywh[2] = 0;
  _collapse_xywh[3] = 0;
  _label_xywh[0]    = 0;
  _label_xywh[1]    = 0;
  _label_xywh[2]    = 0;
  _label_xywh[3]    = 0;
  _usericon         = 0;
#if FLTK_ABI_VERSION >= 10304
  _userdeicon       = 0;
#endif
  _userdata         = 0;
  _parent           = 0;
#if FLTK_ABI_VERSION >= 10303
  _children.manage_item_destroy(1);	// let array's dtor manage destroying Fl_Tree_Items
#endif
#if FLTK_ABI_VERSION >= 10301
  _prev_sibling     = 0;
  _next_sibling     = 0;
#endif /*FLTK_ABI_VERSION*/
}

#if FLTK_ABI_VERSION >= 10303
/// Constructor.
/// Makes a new instance of Fl_Tree_Item for \p 'tree'.
///
/// This must be used instead of the older, deprecated Fl_Tree_Item(Fl_Tree_Prefs)
/// constructor for proper horizontal scrollbar calculation.
///
/// \version 1.3.3 ABI feature
///
Fl_Tree_Item::Fl_Tree_Item(Fl_Tree *tree) {
  _Init(tree->_prefs, tree);
}
#endif

// DTOR
Fl_Tree_Item::~Fl_Tree_Item() {
  if ( _label ) { 
    free((void*)_label);
    _label = 0;
  }
  _widget = 0;			// Fl_Group will handle destruction
  _usericon = 0;		// user handled allocation
#if FLTK_ABI_VERSION >= 10304
  _userdeicon = 0;		// user handled allocation
#endif
#if FLTK_ABI_VERSION >= 10303
  // focus item? set to null
  if ( _tree && this == _tree->_item_focus )
    { _tree->_item_focus = 0; }
#endif
  //_children.clear();		// array's destructor handles itself
}

/// Copy constructor.
Fl_Tree_Item::Fl_Tree_Item(const Fl_Tree_Item *o) {
#if FLTK_ABI_VERSION >= 10303
  _tree             = o->_tree;
#endif
  _label        = o->label() ? strdup(o->label()) : 0;
  _labelfont    = o->labelfont();
  _labelsize    = o->labelsize();
  _labelfgcolor = o->labelfgcolor();
  _labelbgcolor = o->labelbgcolor();
  _widget       = o->widget();
#if FLTK_ABI_VERSION >= 10301
  _flags        = o->_flags;
#else /*FLTK_ABI_VERSION*/
  _open         = o->_open;
  _visible      = o->_visible;
  _active       = o->_active;
  _selected     = o->_selected;
#endif /*FLTK_ABI_VERSION*/
  _xywh[0]      = o->_xywh[0];
  _xywh[1]      = o->_xywh[1];
  _xywh[2]      = o->_xywh[2];
  _xywh[3]      = o->_xywh[3];
  _collapse_xywh[0] = o->_collapse_xywh[0];
  _collapse_xywh[1] = o->_collapse_xywh[1];
  _collapse_xywh[2] = o->_collapse_xywh[2];
  _collapse_xywh[3] = o->_collapse_xywh[3];
  _label_xywh[0]    = o->_label_xywh[0];
  _label_xywh[1]    = o->_label_xywh[1];
  _label_xywh[2]    = o->_label_xywh[2];
  _label_xywh[3]    = o->_label_xywh[3];
  _usericon         = o->usericon();
  _userdata         = o->user_data();
  _parent           = o->_parent;
#if FLTK_ABI_VERSION >= 10301
  _prev_sibling     = 0;		// do not copy ptrs! use update_prev_next()
  _next_sibling     = 0;		// do not copy ptrs! use update_prev_next()
#endif /*FLTK_ABI_VERSION*/
}

/// Print the tree as 'ascii art' to stdout.
/// Used mainly for debugging.
///
void Fl_Tree_Item::show_self(const char *indent) const {
  const char *thelabel = label() ? label() : "(NULL)";
#if FLTK_ABI_VERSION >= 10301
  printf("%s-%s (%d children, this=%p, parent=%p, prev=%p, next=%p, depth=%d)\n",
	 indent,thelabel,children(),(void*)this, (void*)_parent,
	 _prev_sibling, _next_sibling, depth());
#else /*FLTK_ABI_VERSION*/
  printf("%s-%s (%d children, this=%p, parent=%p depth=%d)\n",
	 indent,thelabel,children(),(void*)this, (void*)_parent, depth());
#endif /*FLTK_ABI_VERSION*/
  if ( children() ) {
    char *i2 = new char [strlen(indent)+2];
    strcpy(i2, indent);
    strcat(i2, " |");
    for ( int t=0; t<children(); t++ ) {
      child(t)->show_self(i2);
    }
    delete[] i2;
  }
  fflush(stdout);
}

/// Set the label to \p 'name'.
/// Makes and manages an internal copy of \p 'name'.
///
void Fl_Tree_Item::label(const char *name) {
  if ( _label ) { free((void*)_label); _label = 0; }
  _label = name ? strdup(name) : 0;
  recalc_tree();		// may change label geometry
}

/// Return the label.
const char *Fl_Tree_Item::label() const {
  return(_label);
}

/// Return const child item for the specified 'index'.
const Fl_Tree_Item *Fl_Tree_Item::child(int index) const {
  return(_children[index]);
}

/// Clear all the children for this item.
void Fl_Tree_Item::clear_children() {
  _children.clear();
  recalc_tree();		// may change tree geometry
}

/// Return the index of the immediate child of this item
/// that has the label \p 'name'.
///
/// \returns index of found item, or -1 if not found.
/// \version 1.3.0 release
///
int Fl_Tree_Item::find_child(const char *name) {
  if ( name ) {
    for ( int t=0; t<children(); t++ )
      if ( child(t)->label() )
        if ( strcmp(child(t)->label(), name) == 0 )
          return(t);
  }
  return(-1);
}

/// Return the /immediate/ child of current item
/// that has the label \p 'name'.
///
/// \returns const found item, or 0 if not found.
/// \version 1.3.3
///
const Fl_Tree_Item* Fl_Tree_Item::find_child_item(const char *name) const {
  if ( name )
    for ( int t=0; t<children(); t++ )
      if ( child(t)->label() )
        if ( strcmp(child(t)->label(), name) == 0 )
          return(child(t));
  return(0);
}

/// Non-const version of Fl_Tree_Item::find_child_item(const char *name) const.
Fl_Tree_Item* Fl_Tree_Item::find_child_item(const char *name) {
  // "Effective C++, 3rd Ed", p.23. Sola fide, Amen.
  return(const_cast<Fl_Tree_Item*>(
	 static_cast<const Fl_Tree_Item &>(*this).find_child_item(name)));
}

/// Find child item by descending array \p 'arr' of names.
/// Does not include self in search.
/// Only Fl_Tree should need this method.
///
/// \returns item, or 0 if not found
/// \version 1.3.0 release
///
const Fl_Tree_Item *Fl_Tree_Item::find_child_item(char **arr) const {
  for ( int t=0; t<children(); t++ ) {
    if ( child(t)->label() ) {
      if ( strcmp(child(t)->label(), *arr) == 0 ) {	// match?
        if ( *(arr+1) ) {				// more in arr? descend
          return(_children[t]->find_child_item(arr+1));
        } else {					// end of arr? done
          return(_children[t]);
        }
      }
    }
  }
  return(0);
}

/// Non-const version of Fl_Tree_Item::find_child_item(char **arr) const.
Fl_Tree_Item *Fl_Tree_Item::find_child_item(char **arr) {
  // "Effective C++, 3rd Ed", p.23. Sola fide, Amen.
  return(const_cast<Fl_Tree_Item*>(
	 static_cast<const Fl_Tree_Item &>(*this).find_child_item(arr)));
}

/// Find item by descending array of \p 'names'.
/// Includes self in search.
/// Only Fl_Tree should need this method. Use Fl_Tree::find_item() instead.
///
/// \returns const item, or 0 if not found
///
const Fl_Tree_Item *Fl_Tree_Item::find_item(char **names) const {
  if ( ! *names ) return(0);
  if ( label() && strcmp(label(), *names) == 0 ) {	// match self?
    ++names;						// skip self
    if ( *names == 0 ) return(this);			// end of names, found ourself
  }
  if ( children() ) {					// check children..
    return(find_child_item(names));
  }
  return(0);
}

/// Non-const version of Fl_Tree_Item::find_item(char **names) const.
Fl_Tree_Item *Fl_Tree_Item::find_item(char **names) {
  // "Effective C++, 3rd Ed", p.23. Sola fide, Amen.
  return(const_cast<Fl_Tree_Item*>(
	 static_cast<const Fl_Tree_Item &>(*this).find_item(names)));
}

/// Find the index number for the specified \p 'item'
/// in the current item's list of children.
///
/// \returns the index, or -1 if not found.
///
int Fl_Tree_Item::find_child(Fl_Tree_Item *item) {
  for ( int t=0; t<children(); t++ )
    if ( item == child(t) )
      return(t);
  return(-1);
}

/// Add a new child to this item with the name \p 'new_label'
/// and defaults from \p 'prefs'.
/// An internally managed copy is made of the label string.
/// Adds the item based on the value of prefs.sortorder().
/// \returns the item added
/// \version 1.3.0 release
///
Fl_Tree_Item *Fl_Tree_Item::add(const Fl_Tree_Prefs &prefs,
				const char *new_label) {
  return(add(prefs, new_label, (Fl_Tree_Item*)0));
}

/// Add \p 'item' as immediate child with \p 'new_label'
/// and defaults from \p 'prefs'.
/// If \p 'item' is NULL, a new item is created.
/// An internally managed copy is made of the label string.
/// Adds the item based on the value of prefs.sortorder().
/// \returns the item added
/// \version 1.3.3
///
Fl_Tree_Item *Fl_Tree_Item::add(const Fl_Tree_Prefs &prefs,
			        const char *new_label,
				Fl_Tree_Item *item) {
#if FLTK_ABI_VERSION >= 10303
  if ( !item )
    { item = new Fl_Tree_Item(_tree); item->label(new_label); }
#else
  if ( !item )
    { item = new Fl_Tree_Item(prefs); item->label(new_label); }
#endif
  recalc_tree();		// may change tree geometry
  item->_parent = this;
  switch ( prefs.sortorder() ) {
    case FL_TREE_SORT_NONE: {
      _children.add(item);
      return(item);
    }
    case FL_TREE_SORT_ASCENDING: {
      for ( int t=0; t<_children.total(); t++ ) {
        Fl_Tree_Item *c = _children[t];
        if ( c->label() && strcmp(c->label(), new_label) > 0 ) {
          _children.insert(t, item);
          return(item);
        }
      }
      _children.add(item);
      return(item);
    }
    case FL_TREE_SORT_DESCENDING: {
      for ( int t=0; t<_children.total(); t++ ) {
        Fl_Tree_Item *c = _children[t];
        if ( c->label() && strcmp(c->label(), new_label) < 0 ) {
          _children.insert(t, item);
          return(item);
        }
      }
      _children.add(item);
      return(item);
    }
  }
  return(item);
}

/// Descend into the path specified by \p 'arr', and add a new child there.
/// Should be used only by Fl_Tree's internals.
/// Adds the item based on the value of prefs.sortorder().
/// \returns the item added.
/// \version 1.3.0 release
///
Fl_Tree_Item *Fl_Tree_Item::add(const Fl_Tree_Prefs &prefs, char **arr) {
  return add(prefs, arr, 0);
}

/// Descend into path specified by \p 'arr' and add \p 'newitem' there.
/// Should be used only by Fl_Tree's internals.
/// If item is NULL, a new item is created.
/// Adds the item based on the value of prefs.sortorder().
/// \returns the item added.
/// \version 1.3.3 ABI feature
///
Fl_Tree_Item *Fl_Tree_Item::add(const Fl_Tree_Prefs &prefs,
			        char **arr,
				Fl_Tree_Item *newitem) {
  if ( !*arr ) return 0;
  // See if we can find an existing child with name requested.
  Fl_Tree_Item *child = find_child_item(*arr);
  if ( child ) {		// Child found?
    if ( *(arr+1) == 0 ) {	// ..and at end of path?
      if ( !newitem ) {		// ..and no item specified?
	return 0;		// ..error: child exists already
      } else {
        // Child found, end of path, item specified
	return child->add(prefs, newitem->label(), newitem);
      }
    }
    // Child found: more path elements to go or item specified?
    // Descend into child to handle add..
    return child->add(prefs, arr+1, newitem);	// recurse
  }
  // No child found, see if we reached end of path.
  //	If so, add as an immediate child, done
  if ( *(arr+1) == 0 )			// end of path?
    return add(prefs, *arr, newitem);	// add as immediate child

  // No child found, but more to path?
  // 	If so, create new child to handle add()
  Fl_Tree_Item *newchild;
  return (newchild=add(prefs, *arr))	      // create new immediate child
         ? newchild->add(prefs,arr+1,newitem) // it worked? recurse to add
	 : 0;				      // failed? error
}

/**
  Insert a new item named \p 'new_label' into current item's
  children at a specified position \p 'pos'.

  If \p pos is out of range the new item is
   - prepended if \p pos \< 0 or
   - appended  if \p pos \> item->children().

  \returns the new item inserted
  \see Fl_Tree::insert()
*/
Fl_Tree_Item *Fl_Tree_Item::insert(const Fl_Tree_Prefs &prefs, const char *new_label, int pos) {
#if FLTK_ABI_VERSION >= 10303
  Fl_Tree_Item *item = new Fl_Tree_Item(_tree);
#else
  Fl_Tree_Item *item = new Fl_Tree_Item(prefs);
#endif
  item->label(new_label);
  item->_parent = this;
  _children.insert(pos, item);
  recalc_tree();		// may change tree geometry
  return(item);
}

/// Insert a new item named \p 'new_label' above this item.
/// \returns the new item inserted, or 0 if an error occurred.
///
Fl_Tree_Item *Fl_Tree_Item::insert_above(const Fl_Tree_Prefs &prefs, const char *new_label) {
  Fl_Tree_Item *p = _parent;
  if ( ! p ) return(0);
  // Walk our parent's children to find ourself
  for ( int t=0; t<p->children(); t++ ) {
    Fl_Tree_Item *c = p->child(t);
    if ( this == c ) {
      return(p->insert(prefs, new_label, t));
    }
  }
  return(0);
}

/// Deparent child at index position \p 'pos'.
/// This creates an "orphaned" item that is still allocated,
/// but has no parent or siblings. Normally the caller would
/// want to immediately reparent the orphan elsewhere.
///
/// A successfully orphaned item will have its parent()
/// and prev_sibling()/next_sibling() set to NULL.
///
/// \returns
///     - pointer to orphaned item on success
///     - NULL on error (could not deparent the item)
///
Fl_Tree_Item* Fl_Tree_Item::deparent(int pos) {
  Fl_Tree_Item *orphan = _children[pos];
  if ( _children.deparent(pos) < 0 ) return NULL;
  return orphan;
}

/// Reparent specified item as a child of ourself at position \p 'pos'.
/// Typically 'newchild' was recently orphaned with deparent().
///
/// \returns
///    -  0: on success
///    - -1: on error (e.g. if \p 'pos' out of range) with no changes made.
///
int Fl_Tree_Item::reparent(Fl_Tree_Item *newchild, int pos) {
  int ret;
  if ( (ret = _children.reparent(newchild, this, pos)) < 0 ) return ret;
  newchild->parent(this);		// take custody
  return 0;
}

/// Move the item 'from' to sibling position of 'to'.
///
/// \returns
///    -  0: Success
///    - -1: range error (e.g. if \p 'to' or \p 'from' out of range).
///    - (Other return values reserved for future use)
///
int Fl_Tree_Item::move(int to, int from) {
  return _children.move(to, from);
}

/// Move the current item above/below/into the specified 'item',
/// where \p 'op' determines the type of move:
///
///    - 0: move above \p 'item' (\p 'pos' ignored)
///    - 1: move below \p 'item' (\p 'pos' ignored)
///    - 2: move into  \p 'item' as a child (at optional position \p 'pos')
///
/// \returns 0 on success. a negative number on error:
///     - -1: one of the items has no parent
///     - -2: item's index could not be determined
///     - -3: bad 'op'
///     - -4: index range error
///     - -5: could not deparent
///     - -6: could not reparent at \p 'pos'
///     - (Other return values reserved for future use.)
///
int Fl_Tree_Item::move(Fl_Tree_Item *item, int op, int pos) {
  Fl_Tree_Item *from_parent, *to_parent;
  int from, to;
  switch (op) {
    case 0:	// "above"
      from_parent = this->parent();
      to_parent   = item->parent();
      from        = from_parent->find_child(this);
      to          = to_parent->find_child(item);
      break;
    case 1:	// "below"
      from_parent = this->parent();
      to_parent   = item->parent();
      from        = from_parent->find_child(this);
      to          = to_parent->find_child(item);
      break;
    case 2:	// "into"
      from_parent = this->parent();
      to_parent   = item;
      from        = from_parent->find_child(this);
      to          = pos;
      break;
    default:
      return -3;
  }
  if ( !from_parent || !to_parent ) return -1;
  if ( from < 0 || to < 0 ) return -2;
  if ( from_parent == to_parent ) {		// same parent?
    switch (op) {				// 'to' offsets due to scroll
      case 0: if ( from < to && to > 0 ) --to; break;
      case 1: if ( from > to && to < to_parent->children() ) ++to; break;
    }
    if ( from_parent->move(to, from) < 0 )	// simple move among siblings
      return -4;
  } else {					// different parent?
    if ( to > to_parent->children() )		// try to prevent a reparent() error
      return -4;
    if ( from_parent->deparent(from) == NULL )	// deparent self from current parent
      return -5;
    if ( to_parent->reparent(this, to) < 0 ) {	// reparent self to new parent at position 'to'
      to_parent->reparent(this, 0);		// failed? shouldn't happen, reparent at 0
      return -6;
    }
  }
  return 0;
}

/// Move the current item above the specified 'item'.
/// This is the equivalent of calling move(item,0,0).
///
/// \returns 0 on success.<br>
///          On error returns a negative value;
///          see move(Fl_Tree_Item*,int,int) for possible error codes.
///
int Fl_Tree_Item::move_above(Fl_Tree_Item *item) {
  return move(item, 0, 0);
}

/// Move the current item below the specified 'item'.
/// This is the equivalent of calling move(item,1,0).
///
/// \returns 0 on success.<br>
///          On error returns a negative value;
///          see move(Fl_Tree_Item*,int,int) for possible error codes.
///
int Fl_Tree_Item::move_below(Fl_Tree_Item *item) {
  return move(item, 1, 0);
}

/// Parent the current item as a child of the specified \p 'item'.
/// This is the equivalent of calling move(item,2,pos).
///
/// \returns 0 on success.<br>
///          On error returns a negative value;
///          see move(Fl_Tree_Item*,int,int) for possible error codes.
///
int Fl_Tree_Item::move_into(Fl_Tree_Item *item, int pos) {
  return move(item, 2, pos);
}

#if FLTK_ABI_VERSION >= 10303
/// Return the parent tree's prefs.
/// \returns a reference to the parent tree's Fl_Tree_Prefs
/// \version 1.3.3 ABI feature
///
const Fl_Tree_Prefs& Fl_Tree_Item::prefs() const {
  return(_tree->_prefs);
}

/// Replace the current item with a new item.
///
/// The current item is destroyed if successful.
/// No checks are made to see if an item with the same name exists.
///
/// This method can be used to, for example, install 'custom' items
/// into the tree derived from Fl_Tree_Item; see draw_item_content().
///
/// \param[in] newitem The new item to replace the current item
/// \returns newitem on success, NULL if could not be replaced.
/// \see Fl_Tree_Item::draw_item_content(), Fl_Tree::root(Fl_Tree_Item*)
/// \version 1.3.3 ABI feature
///
Fl_Tree_Item *Fl_Tree_Item::replace(Fl_Tree_Item *newitem) {
  Fl_Tree_Item *p = parent();
  if ( !p ) {			// no parent? then we're the tree's root..
    _tree->root(newitem);	// ..tell tree to replace root
    return newitem;
  }
  // has parent? ask parent to replace us
  return p->replace_child(this, newitem);
}

/// Replace existing child \p 'olditem' with \p 'newitem'.
///
/// The \p 'olditem' is destroyed if successful.
/// Can be used to put custom items (derived from Fl_Tree_Item) into the tree.
/// No checks are made to see if an item with the same name exists.
///
/// \param[in] olditem The item to be found and replaced
/// \param[in] newitem The new item to take the place of \p 'olditem'
/// \returns newitem on success and \p 'olditem' is destroyed.
///          NULL on error if \p 'olditem' was not found
///          as an immediate child.
/// \see replace(), Fl_Tree_Item::draw()
/// \version 1.3.3 ABI feature
///
Fl_Tree_Item *Fl_Tree_Item::replace_child(Fl_Tree_Item *olditem,
				          Fl_Tree_Item *newitem) {
  int pos = find_child(olditem);	// find our index for olditem
  if ( pos == -1 ) return(NULL);
  newitem->_parent = this;
  // replace in array (handles stitching neighboring items)
  _children.replace(pos, newitem);
  recalc_tree();			// newitem may have changed tree geometry
  return newitem;
}
#endif

/// Remove \p 'item' from the current item's children.
/// \returns 0 if removed, -1 if item not an immediate child.
///
int Fl_Tree_Item::remove_child(Fl_Tree_Item *item) {
  for ( int t=0; t<children(); t++ ) {
    if ( child(t) == item ) {
      item->clear_children();
      _children.remove(t);
      recalc_tree();		// may change tree geometry
      return(0);
    }
  }
  return(-1);
}

/// Remove immediate child (and its children) by its label \p 'name'.
/// If more than one item matches \p 'name', only the first
/// matching item is removed.
/// \param[in] name The label name of the immediate child to remove
/// \returns 0 if removed, -1 if not found.
/// \version 1.3.3
///
int Fl_Tree_Item::remove_child(const char *name) {
  for ( int t=0; t<children(); t++ ) {
    if ( child(t)->label() ) {
      if ( strcmp(child(t)->label(), name) == 0 ) {
        _children.remove(t);
	recalc_tree();		// may change tree geometry
        return(0);
      }
    }
  }
  return(-1);
}

/// Swap two of our children, given two child index values \p 'ax' and \p 'bx'.
/// Use e.g. for sorting.<br>
/// This method is FAST, and does not involve lookups.<br>
/// No range checking is done on either index value.
/// \param[in] ax,bx the index of the items to swap
///
void Fl_Tree_Item::swap_children(int ax, int bx) {
  _children.swap(ax, bx);
}

/// Swap two of our immediate children, given item pointers.
/// Use e.g. for sorting. 
///
/// This method is SLOW because it involves linear lookups.<br>
/// For speed, use swap_children(int,int) instead.
///
/// \param[in] a,b The item ptrs of the two items to swap.
///                Both must be immediate children of the current item.
/// \returns
///    -    0 : OK
///    -   -1 : failed: item \p 'a' or \p 'b' is not our child.
///
int Fl_Tree_Item::swap_children(Fl_Tree_Item *a, Fl_Tree_Item *b) {
  int ax = -1, bx = -1;
  for ( int t=0; t<children(); t++ ) {	// find index for a and b
    if ( _children[t] == a ) { ax = t; if ( bx != -1 ) break; else continue; }
    if ( _children[t] == b ) { bx = t; if ( ax != -1 ) break; else continue; }
  }
  if ( ax == -1 || bx == -1 ) return(-1);	// not found? fail
  swap_children(ax,bx);
  return(0);
}

/// Internal: Horizontal connector line based on preference settings.
/// \param[in] x1 The left hand X position of the horizontal connector
/// \param[in] x2 The right hand X position of the horizontal connector
/// \param[in] y  The vertical position of the horizontal connector
/// \param[in] prefs The Fl_Tree prefs
///
void Fl_Tree_Item::draw_horizontal_connector(int x1, int x2, int y, const Fl_Tree_Prefs &prefs) {
  fl_color(prefs.connectorcolor());
  switch ( prefs.connectorstyle() ) {
    case FL_TREE_CONNECTOR_SOLID:
      y |= 1;				// force alignment w/dot pattern
      fl_line(x1,y,x2,y);
      return;
    case FL_TREE_CONNECTOR_DOTTED: 
    {
      y  |= 1;				// force alignment w/dot pattern
      x1 |= 1;
      for ( int xx=x1; xx<=x2; xx+=2 ) {
	fl_point(xx, y);
      }
      return;
    }
    case FL_TREE_CONNECTOR_NONE:
      return;
  }
}

/// Internal: Vertical connector line based on preference settings.
/// \param[in] x     The x position of the vertical connector
/// \param[in] y1    The top of the vertical connector
/// \param[in] y2    The bottom of the vertical connector
/// \param[in] prefs The Fl_Tree prefs
///
void Fl_Tree_Item::draw_vertical_connector(int x, int y1, int y2, const Fl_Tree_Prefs &prefs) {
  fl_color(prefs.connectorcolor());
  switch ( prefs.connectorstyle() ) {
    case FL_TREE_CONNECTOR_SOLID:
      y1 |= 1;				// force alignment w/dot pattern
      y2 |= 1;				// force alignment w/dot pattern
      fl_line(x,y1,x,y2);
      return;
    case FL_TREE_CONNECTOR_DOTTED:
    {
      y1 |= 1;				// force alignment w/dot pattern
      y2 |= 1;				// force alignment w/dot pattern
      for ( int yy=y1; yy<=y2; yy+=2 ) {
        fl_point(x, yy);
      }
      return;
    }
    case FL_TREE_CONNECTOR_NONE:
      return;
  }
}

#if FLTK_ABI_VERSION >= 10303
/// Find the item that the last event was over.
/// If \p 'yonly' is 1, only check event's y value, don't care about x.
/// \param[in] prefs The parent tree's Fl_Tree_Prefs
/// \param[in] yonly -- 0: check both event's X and Y values.
///                  -- 1: only check event's Y value, don't care about X.
/// \returns pointer to clicked item, or NULL if none found
/// \version 1.3.3 ABI feature
///
const Fl_Tree_Item *Fl_Tree_Item::find_clicked(const Fl_Tree_Prefs &prefs, int yonly) const {
  if ( ! is_visible() ) return(0);
  if ( is_root() && !prefs.showroot() ) {
    // skip event check if we're root but root not being shown
  } else {
    // See if event is over us
    if ( yonly ) {
      if ( Fl::event_y() >= _xywh[1] &&
           Fl::event_y() <= (_xywh[1]+_xywh[3]) ) {
        return(this);
      }
    } else {
      if ( event_inside(_xywh) ) {		// event within this item?
        return(this);				// found
      }
    }
  }
  if ( is_open() ) {				// open? check children of this item
    for ( int t=0; t<children(); t++ ) {
      const Fl_Tree_Item *item;
      if ( (item = _children[t]->find_clicked(prefs, yonly)) != NULL)  // recurse into child for descendents
        return(item);						       // found?
    }
  }
  return(0);
}

/// Non-const version of Fl_Tree_Item::find_clicked(const Fl_Tree_Prefs&,int) const
Fl_Tree_Item *Fl_Tree_Item::find_clicked(const Fl_Tree_Prefs &prefs, int yonly) {
  // "Effective C++, 3rd Ed", p.23. Sola fide, Amen.
  return(const_cast<Fl_Tree_Item*>(
	 static_cast<const Fl_Tree_Item &>(*this).find_clicked(prefs, yonly)));
}
#else
/// Find the item that the last event was over.
/// \param[in] prefs The parent tree's Fl_Tree_Prefs
/// \returns pointer to clicked item, or NULL if none found
/// \version 1.3.0
///
const Fl_Tree_Item *Fl_Tree_Item::find_clicked(const Fl_Tree_Prefs &prefs) const {
  if ( ! is_visible() ) return(0);
  if ( is_root() && !prefs.showroot() ) {
    // skip event check if we're root but root not being shown
  } else {
    // See if event is over us
    if ( event_inside(_xywh) ) {		// event within this item?
      return(this);				// found
    }
  }
  if ( is_open() ) {				// open? check children of this item
    for ( int t=0; t<children(); t++ ) {
      const Fl_Tree_Item *item;
      if ( (item = _children[t]->find_clicked(prefs)) != NULL)  // recurse into child for descendents
        return(item);						// found?
    }
  }
  return(0);
}

/// Non-const version of Fl_Tree_Item::find_clicked(const Fl_Tree_Prefs&) const.
Fl_Tree_Item *Fl_Tree_Item::find_clicked(const Fl_Tree_Prefs &prefs) {
  // "Effective C++, 3rd Ed", p.23. Sola fide, Amen.
  return(const_cast<Fl_Tree_Item*>(
	 static_cast<const Fl_Tree_Item &>(*this).find_clicked(prefs)));
}
#endif

static void draw_item_focus(Fl_Boxtype B, Fl_Color fg, Fl_Color bg, int X, int Y, int W, int H) {
  if (!Fl::visible_focus()) return;
  switch (B) {
    case FL_DOWN_BOX:
    case FL_DOWN_FRAME:
    case FL_THIN_DOWN_BOX:
    case FL_THIN_DOWN_FRAME:
      X ++;
      Y ++;
    default:
      break;
  }
  fl_color(fl_contrast(fg, bg));

#if defined(USE_X11) || defined(__APPLE_QUARTZ__)
  fl_line_style(FL_DOT);
  fl_rect(X + Fl::box_dx(B), Y + Fl::box_dy(B),
          W - Fl::box_dw(B) - 1, H - Fl::box_dh(B) - 1);
  fl_line_style(FL_SOLID);
#else
  // Some platforms don't implement dotted line style, so draw
  // every other pixel around the focus area...
  //
  // Also, QuickDraw (MacOS) does not support line styles specifically,
  // and the hack we use in fl_line_style() will not draw horizontal lines
  // on odd-numbered rows...
  int i, xx, yy;

  X += Fl::box_dx(B);
  Y += Fl::box_dy(B);
  W -= Fl::box_dw(B) + 2;
  H -= Fl::box_dh(B) + 2;

  for (xx = 0, i = 1; xx < W; xx ++, i ++) if (i & 1) fl_point(X + xx, Y);
  for (yy = 0; yy < H; yy ++, i ++) if (i & 1) fl_point(X + W, Y + yy);
  for (xx = W; xx > 0; xx --, i ++) if (i & 1) fl_point(X + xx, Y + H);
  for (yy = H; yy > 0; yy --, i ++) if (i & 1) fl_point(X, Y + yy);
#endif
}

/// Return the item's 'visible' height. Takes into account the item's:
///    - visibility (if !is_visible(), returns 0)
///    - labelfont() height: if label() != NULL
///    - widget() height: if widget() != NULL
///    - openicon() height (if not NULL)
///    - usericon() height (if not NULL)
/// Does NOT include Fl_Tree::linespacing();
/// \returns maximum pixel height
///
int Fl_Tree_Item::calc_item_height(const Fl_Tree_Prefs &prefs) const {
  if ( ! is_visible() ) return(0);
  int H = 0;
  if ( _label ) {
    fl_font(_labelfont, _labelsize);	// fl_descent() needs this :/
    H = _labelsize + fl_descent() + 1;	// at least one pixel space below descender
  }
#if FLTK_ABI_VERSION >= 10301
  if ( widget() && 
       (prefs.item_draw_mode() & FL_TREE_ITEM_HEIGHT_FROM_WIDGET) &&
       H < widget()->h()) {
    H = widget()->h();
  }
#endif /*FLTK_ABI_VERSION*/
  if ( has_children() && prefs.openicon() && H<prefs.openicon()->h() )
    H = prefs.openicon()->h();
  if ( usericon() && H<usericon()->h() )
    H = usericon()->h();
  return(H);
}

#if FLTK_ABI_VERSION >= 10303
// These methods held for 1.3.3 ABI: all need 'tree()' back-reference.

/// Returns the recommended foreground color used for drawing this item.
/// \see draw_item_content()
/// \version 1.3.3 ABI ABI
///
Fl_Color Fl_Tree_Item::drawfgcolor() const {
  return is_selected() ? fl_contrast(_labelfgcolor, tree()->selection_color())
		       : (is_active() && tree()->active_r()) ? _labelfgcolor
				                             : fl_inactive(_labelfgcolor);
}

/// Returns the recommended background color used for drawing this item.
/// \see draw_item_content()
/// \version 1.3.3 ABI
///
Fl_Color Fl_Tree_Item::drawbgcolor() const {
  const Fl_Color unspecified = 0xffffffff;
  return is_selected() ? is_active() && tree()->active_r() ? tree()->selection_color() 
				                           : fl_inactive(tree()->selection_color())
		       : _labelbgcolor == unspecified ? tree()->color()
						      : _labelbgcolor;
}

/// Draw the item content
///
/// This method can be overridden to implement custom drawing
/// by filling the label_[xywh]() area with content.
///
/// A minimal example of how to override draw_item_content()
/// and draw just a normal item's background and label ourselves:
///
/// \code
/// class MyTreeItem : public Fl_Tree_Item {
/// public:
///     MyTreeItem() { }
///     ~MyTreeItem() { }
///     // DRAW OUR CUSTOM CONTENT FOR THE ITEM
///     int draw_item_content(int render) {
///       // Our item's dimensions + text content
///       int X=label_x(), Y=label_y(), W=label_w(), H=label_h(); 
///       const char *text = label() ? label() : "";
///       // Rendering? Do any drawing that's needed
///       if ( render ) {
///         // Draw bg -- a filled rectangle
///         fl_color(drawbgcolor()); fl_rectf(X,Y,W,H);
///         // Draw label
///         fl_font(labelfont(), labelsize());      // use item's label font/size
///         fl_color(drawfgcolor());                // use recommended fg color
///         fl_draw(text, X,Y,W,H, FL_ALIGN_LEFT);  // draw the item's label
///       }
///       // Rendered or not, we must calculate content's max X position
///       int lw=0, lh=0;
///       fl_measure(text, lw, lh);                 // get width of label text
///       return X + lw;                            // return X + label width
///    }
/// };
/// \endcode
///
/// You can draw anything you want inside draw_item_content()
/// using any of the fl_draw.H functions, as long as it's 
/// within the label's xywh area.
///
/// To add instances of your custom item to the tree, you can use:
///
/// \code
///     // Example #1: using add()
///     MyTreeItem *bart = new MyTreeItem(..);  // class derived from Fl_Tree_Item
///     tree->add("/Simpsons/Bart", bart);      // Add item as /Simpsons/Bart
/// \endcode
///
/// ..or you can insert or replace existing items:
///
/// \code
///     // Example #2: using replace() 
///     MyTreeItem *marge = new MyTreeItem(..); // class derived from Fl_Tree_Item
///     item = tree->add("/Simpsons/Marge");    // create item
///     item->replace(mi);                      // replace it with our own
/// \endcode
///
/// \param[in] render Whether we should render content (1), or just tally
///            the geometry (0). Fl_Tree may want only to find the widest
///            item in the tree for scrollbar calculations.
///
/// \returns the right-most X coordinate, or 'xmax' of content we drew,
///          i.e. the "scrollable" content.
///          The tree uses the largest xmax to determine the maximum
///          width of the tree's content (needed for e.g. computing the
///          horizontal scrollbar's size).
/// \version 1.3.3 ABI feature
///
int Fl_Tree_Item::draw_item_content(int render) {
  Fl_Color fg = drawfgcolor();
  Fl_Color bg = drawbgcolor();
  const Fl_Tree_Prefs &prefs = tree()->prefs();
  int xmax = label_x();
  // Background for this item, only if different from tree's bg
  if ( render && (bg != tree()->color() || is_selected()) ) {
    if ( is_selected() ) {			// Selected? Use selectbox() style
      fl_draw_box(prefs.selectbox(),
      		  label_x(), label_y(), label_w(), label_h(), bg);
    } else {					// Not Selected? use plain filled rectangle
      fl_color(bg);
      fl_rectf(label_x(), label_y(), label_w(), label_h());
    }
    if ( widget() ) widget()->damage(FL_DAMAGE_ALL);	// if there's a child widget, we just damaged it
  }
  // Draw label
  if ( _label && 
       ( !widget() || 
	 (prefs.item_draw_mode() & FL_TREE_ITEM_DRAW_LABEL_AND_WIDGET) ) ) {
    if ( render ) {
      fl_color(fg);
      fl_font(_labelfont, _labelsize);
    }
    int lx = label_x()+(_label ? prefs.labelmarginleft() : 0);
    int ly = label_y()+(label_h()/2)+(_labelsize/2)-fl_descent()/2;
    int lw=0, lh=0;
    fl_measure(_label, lw, lh);		// get box around text (including white space)
    if ( render ) fl_draw(_label, lx, ly);
    xmax = lx + lw;			// update max width of drawn item
  }
  return xmax;
}

/// Draw this item and its children.
///
/// \param[in]     X              Horizontal position for item being drawn
/// \param[in,out] Y              Vertical position for item being drawn,
///                               returns new position for next item
/// \param[in]     W              Recommended width for item
/// \param[in]     itemfocus      The tree's current focus item (if any)
/// \param[in,out] tree_item_xmax The tree's running xmax (right-most edge so far).
///                               Mainly used by parent tree when render==0 to
///                               calculate tree's max width.
/// \param[in]     lastchild      Is this item the last child in a subtree?
/// \param[in]     render         Whether or not to render the item:
///                               0: no rendering, just calculate size w/out drawing.
///                               1: render item as well as size calc
///
/// \version 1.3.3 ABI feature: modified parameters
///
void Fl_Tree_Item::draw(int X, int &Y, int W, Fl_Tree_Item *itemfocus,
			int &tree_item_xmax, int lastchild, int render) {
  Fl_Tree_Prefs &prefs = _tree->_prefs;
  if ( !is_visible() ) return; 
  int tree_top = tree()->_tiy;
  int tree_bot = tree_top + tree()->_tih;
  int H = calc_item_height(prefs);	// height of item
  int H2 = H + prefs.linespacing();	// height of item with line spacing

  // Update the xywh of this item
  _xywh[0] = X;
  _xywh[1] = Y;
  _xywh[2] = W;
  _xywh[3] = H;

  // Determine collapse icon's xywh
  //   Note: calculate collapse icon's xywh for possible mouse click detection.
  //   We don't care about items clipped off the viewport; they won't get mouse events.
  //
  int item_y_center = Y+(H/2);
  _collapse_xywh[2] = prefs.openicon()->w();
  int &icon_w = _collapse_xywh[2];
  _collapse_xywh[0] = X + (icon_w + prefs.connectorwidth())/2 - 3;
  int &icon_x = _collapse_xywh[0];
  _collapse_xywh[1] = item_y_center - (prefs.openicon()->h()/2);
  int &icon_y = _collapse_xywh[1];
  _collapse_xywh[3] = prefs.openicon()->h();

  // Horizontal connector values
  //   Must calculate these even if(clipped) because 'draw children' code (below)
  //   needs hconn_x_center value. (Otherwise, these calculations could be 'clipped')
  //
  int hconn_x  = X+icon_w/2-1;
  int hconn_x2 = hconn_x + prefs.connectorwidth();
  int hconn_x_center = X + icon_w + ((hconn_x2 - (X + icon_w)) / 2);
  int cw1 = icon_w+prefs.connectorwidth()/2, cw2 = prefs.connectorwidth();
  int conn_w = cw1>cw2 ? cw1 : cw2;

  // Usericon position
  int uicon_x = X+(icon_w/2-1+conn_w) + ( (usericon() || prefs.usericon())
  					  ? prefs.usericonmarginleft() : 0);
  int uicon_w = usericon() ? usericon()->w()
                           : prefs.usericon() ? prefs.usericon()->w() : 0;

  // Label xywh
  _label_xywh[0] = uicon_x + uicon_w + prefs.labelmarginleft();
  _label_xywh[1] = Y;
  _label_xywh[2] = tree()->_tix + tree()->_tiw - _label_xywh[0];
  _label_xywh[3] = H;

  // Begin calc of this item's max width..
  //     It might not even be visible, so start at zero.
  //
  int xmax = 0;

  // Recalc widget position
  //   Do this whether clipped or not, so that when scrolled,
  //   the widgets move to appropriate 'offscreen' positions
  //   (so that they don't get mouse events, etc)
  //
  if ( widget() ) {
    int wx = uicon_x + uicon_w + (_label ? prefs.labelmarginleft() : 0);
    int wy = label_y();
    int ww = widget()->w();		// use widget's width
    int wh = (prefs.item_draw_mode() & FL_TREE_ITEM_HEIGHT_FROM_WIDGET)
             ? widget()->h() : H;
    if ( _label && 
         (prefs.item_draw_mode() & FL_TREE_ITEM_DRAW_LABEL_AND_WIDGET) ) {
      fl_font(_labelfont, _labelsize);	// fldescent() needs this
      int lw=0, lh=0;
      fl_measure(_label,lw,lh);		// get box around text (including white space)
      wx += (lw + prefs.widgetmarginleft());
    }
    if ( widget()->x() != wx || widget()->y() != wy ||
	 widget()->w() != ww || widget()->h() != wh ) {
      widget()->resize(wx,wy,ww,wh);		// we'll handle redraw below
    }
  }
  char clipped = ((Y+H) < tree_top) || (Y>tree_bot) ? 1 : 0;
  if (!render) clipped = 0;			// NOT rendering? Then don't clip, so we calc unclipped items
#if FLTK_ABI_VERSION >= 10304
  char active = (is_active() && tree()->active_r()) ? 1 : 0;
#endif
  char drawthis = ( is_root() && prefs.showroot() == 0 ) ? 0 : 1;
  if ( !clipped ) {
    Fl_Color fg = drawfgcolor();
    Fl_Color bg = drawbgcolor();
    // See if we should draw this item
    //    If this item is root, and showroot() is disabled, don't draw.
    //    'clipped' is an optimization to prevent drawing anything offscreen.
    //
    if ( drawthis ) {						// draw this item at all?
      if ( (tree()->damage() & ~FL_DAMAGE_CHILD) || !render ) {	// non-child damage?
	// Draw connectors
	if ( render && prefs.connectorstyle() != FL_TREE_CONNECTOR_NONE ) {
	  // Horiz connector between center of icon and text
	  // if this is root, the connector should not dangle in thin air on the left
	  if (is_root()) draw_horizontal_connector(hconn_x_center, hconn_x2, item_y_center, prefs);
	  else           draw_horizontal_connector(hconn_x, hconn_x2, item_y_center, prefs);
	  // Small vertical line down to children
	  if ( has_children() && is_open() )
	    draw_vertical_connector(hconn_x_center, item_y_center, Y+H2, prefs);
	  // Connectors for last child
	  if ( !is_root() ) {
	    if ( lastchild ) draw_vertical_connector(hconn_x, Y, item_y_center, prefs);
	    else             draw_vertical_connector(hconn_x, Y, Y+H2, prefs);
	  }
	}
	// Draw collapse icon
	if ( render && has_children() && prefs.showcollapse() ) {
	  // Draw icon image
#if FLTK_ABI_VERSION >= 10304
	  if ( is_open() ) {
	    if ( active ) prefs.closeicon()->draw(icon_x,icon_y);
	    else          prefs.closedeicon()->draw(icon_x,icon_y);
	  } else {
	    if ( active ) prefs.openicon()->draw(icon_x,icon_y);
	    else          prefs.opendeicon()->draw(icon_x,icon_y);
	  }
#else
	  if ( is_open() ) {
	    prefs.closeicon()->draw(icon_x,icon_y);
	  } else {
	    prefs.openicon()->draw(icon_x,icon_y);
	  }
#endif
	}
	// Draw user icon (if any)
#if FLTK_ABI_VERSION >= 10304
	if ( render && usericon() ) {
	  // Item has user icon? Use it
	  int uicon_y = item_y_center - (usericon()->h() >> 1);
	  if ( active ) usericon()->draw(uicon_x,uicon_y);
	  else if ( userdeicon() ) userdeicon()->draw(uicon_x,uicon_y);
	} else if ( render && prefs.usericon() ) {
	  // Prefs has user icon? Use it
	  int uicon_y = item_y_center - (prefs.usericon()->h() >> 1);
	  if ( active ) prefs.usericon()->draw(uicon_x,uicon_y);
	  else if ( prefs.userdeicon() ) prefs.userdeicon()->draw(uicon_x,uicon_y);
	}
#else
	if ( render && usericon() ) {
	  // Item has user icon? Use it
	  int uicon_y = item_y_center - (usericon()->h() >> 1);
	  usericon()->draw(uicon_x,uicon_y);
	} else if ( render && prefs.usericon() ) {
	  // Prefs has user icon? Use it
	  int uicon_y = item_y_center - (prefs.usericon()->h() >> 1);
	  prefs.usericon()->draw(uicon_x,uicon_y);
	}
#endif
	// Draw item's content
	xmax = draw_item_content(render);
      }			// end non-child damage
      // Draw child FLTK widget?
      if ( widget() ) {
        if (render)
	  tree()->draw_child(*widget());	// let group handle drawing child
	if ( widget()->label() && render )
	  tree()->draw_outside_label(*widget());// label too
        xmax = widget()->x() + widget()->w();	// update max width of widget
      }
      // Draw focus box around item's bg last
      if ( render &&
           this == itemfocus &&
           Fl::visible_focus() && 
	   Fl::focus() == tree() &&
	   prefs.selectmode() != FL_TREE_SELECT_NONE ) {
	draw_item_focus(FL_NO_BOX,fg,bg,label_x()+1,label_y()+1,label_w()-1,label_h()-1);
      }
    }			// end drawthis
  }			// end clipped
  if ( drawthis ) Y += H2;					// adjust Y (even if clipped)
  // Manage tree_item_xmax
  if ( xmax > tree_item_xmax )
    tree_item_xmax = xmax;
  // Draw child items (if any)
  if ( has_children() && is_open() ) {
    int child_x = drawthis ? (hconn_x_center - (icon_w/2) + 1)	// offset children to right,
                           : X;					// unless didn't drawthis
    int child_w = W - (child_x-X);
    int child_y_start = Y;
    for ( int t=0; t<children(); t++ ) {
      int lastchild = ((t+1)==children()) ? 1 : 0;
      _children[t]->draw(child_x, Y, child_w, itemfocus, tree_item_xmax, lastchild, render);
    }
    if ( has_children() && is_open() ) {
      Y += prefs.openchild_marginbottom();		// offset below open child tree
    }
    if ( ! lastchild ) {
      // Special 'clipped' calculation. (intentional variable shadowing)
      int clipped = ((child_y_start < tree_top) && (Y < tree_top)) ||
                    ((child_y_start > tree_bot) && (Y > tree_bot));
      if (render && !clipped )
        draw_vertical_connector(hconn_x, child_y_start, Y, prefs);
    }
  }
}

#else

/// Draw this item and its children.
///
/// \param[in]     X	     Horizontal position for item being drawn
/// \param[in,out] Y	     Vertical position for item being drawn,
///                          returns new position for next item
/// \param[in]     W	     Recommended width of item
/// \param[in]	   tree      The parent tree
/// \param[in]	   itemfocus The tree's current focus item (if any)
/// \param[in]	   prefs     The tree's preferences
/// \param[in]	   lastchild Is this item the last child in a subtree?
///
/// \version 1.3.0 release, removed 1.3.3 ABI
///
void Fl_Tree_Item::draw(int X, int &Y, int W, Fl_Widget *tree,
			Fl_Tree_Item *itemfocus,
                        const Fl_Tree_Prefs &prefs, int lastchild) {
  if ( ! is_visible() ) return; 
  int tree_top = tree->y();
  int tree_bot = tree_top + tree->h();
  int H = calc_item_height(prefs);	// height of item
  int H2 = H + prefs.linespacing();	// height of item with line spacing

  // Update the xywh of this item
  _xywh[0] = X;
  _xywh[1] = Y;
  _xywh[2] = W;
  _xywh[3] = H;

  // Determine collapse icon's xywh
  //   Note: calculate collapse icon's xywh for possible mouse click detection.
  //   We don't care about items clipped off the viewport; they won't get mouse events.
  //
  int item_y_center = Y+(H/2);
  _collapse_xywh[2] = prefs.openicon()->w();
  int &icon_w = _collapse_xywh[2];
  _collapse_xywh[0] = X + (icon_w + prefs.connectorwidth())/2 - 3;
  int &icon_x = _collapse_xywh[0];
  _collapse_xywh[1] = item_y_center - (prefs.openicon()->h()/2);
  int &icon_y = _collapse_xywh[1];
  _collapse_xywh[3] = prefs.openicon()->h();

  // Horizontal connector values
  //   XXX: Must calculate these even if(clipped) because 'draw children' code (below)
  //   needs hconn_x_center value. (Otherwise, these calculations could be 'clipped')
  //
  int hconn_x  = X+icon_w/2-1;
  int hconn_x2 = hconn_x + prefs.connectorwidth();
  int hconn_x_center = X + icon_w + ((hconn_x2 - (X + icon_w)) / 2);
  int cw1 = icon_w+prefs.connectorwidth()/2, cw2 = prefs.connectorwidth();
  int conn_w = cw1>cw2 ? cw1 : cw2;

  // Background position
  int &bg_x = _label_xywh[0] = X+(icon_w/2-1+conn_w);
  int &bg_y = _label_xywh[1] = Y;
  int &bg_w = _label_xywh[2] = W-(icon_w/2-1+conn_w);
  int &bg_h = _label_xywh[3] = H;

  // Usericon position
  int uicon_x = bg_x + ( (usericon() || prefs.usericon()) ? prefs.usericonmarginleft() : 0);
  int uicon_w = usericon() ? usericon()->w() : prefs.usericon() ? prefs.usericon()->w() : 0;

  // Label position
  int label_x = uicon_x + uicon_w + (_label ? prefs.labelmarginleft() : 0);

  // Recalc widget position
  //   Do this whether clipped or not, so that when scrolled,
  //   the widgets move to appropriate 'offscreen' positions
  //   (so that they don't get mouse events, etc)
  //
  if ( widget() ) {
    int wx = label_x;
    int wy = bg_y;
    int ww = widget()->w();		// use widget's width
#if FLTK_ABI_VERSION >= 10301
    int wh = (prefs.item_draw_mode() & FL_TREE_ITEM_HEIGHT_FROM_WIDGET)
             ? widget()->h() : H;
    if ( _label && 
         (prefs.item_draw_mode() & FL_TREE_ITEM_DRAW_LABEL_AND_WIDGET) ) {
#else /*FLTK_ABI_VERSION*/
    int wh = H;				// lock widget's height to item height
    if ( _label && !widget() ) {	// back compat: don't draw label if widget() present
#endif /*FLTK_ABI_VERSION*/
      fl_font(_labelfont, _labelsize);	// fldescent() needs this
      int lw=0, lh=0;
      fl_measure(_label,lw,lh);		// get box around text (including white space)
#if FLTK_ABI_VERSION >= 10301
      // NEW
      wx += (lw + prefs.widgetmarginleft());
#else /*FLTK_ABI_VERSION*/
      // OLD
      wx += (lw + 3);
#endif /*FLTK_ABI_VERSION*/
    }
    if ( widget()->x() != wx || widget()->y() != wy ||
	 widget()->w() != ww || widget()->h() != wh ) {
      widget()->resize(wx,wy,ww,wh);		// we'll handle redraw below
    }
  }
  char clipped = ((Y+H) < tree_top) || (Y>tree_bot) ? 1 : 0;
  char drawthis = ( is_root() && prefs.showroot() == 0 ) ? 0 : 1;
  char active = (is_active() && tree->active_r()) ? 1 : 0;
  if ( !clipped ) {
    const Fl_Color unspecified = 0xffffffff;

    Fl_Color fg = is_selected() ? fl_contrast(_labelfgcolor, tree->selection_color())
		                : active ? _labelfgcolor
				         : fl_inactive(_labelfgcolor);
    Fl_Color bg = is_selected() ? active ? tree->selection_color() 
				         : fl_inactive(tree->selection_color())
		                : _labelbgcolor == unspecified ? tree->color()
						               : _labelbgcolor;
    // See if we should draw this item
    //    If this item is root, and showroot() is disabled, don't draw.
    //    'clipped' is an optimization to prevent drawing anything offscreen.
    //
    if ( drawthis ) {					// draw this item at all?
      if ( tree->damage() & ~FL_DAMAGE_CHILD ) {	// non-child damage?
	// Draw connectors
	if ( prefs.connectorstyle() != FL_TREE_CONNECTOR_NONE ) {
	  // Horiz connector between center of icon and text
	  // if this is root, the connector should not dangle in thin air on the left
	  if (is_root()) draw_horizontal_connector(hconn_x_center, hconn_x2, item_y_center, prefs);
	  else           draw_horizontal_connector(hconn_x, hconn_x2, item_y_center, prefs);
	  // Small vertical line down to children
	  if ( has_children() && is_open() )
	    draw_vertical_connector(hconn_x_center, item_y_center, Y+H2, prefs);
	  // Connectors for last child
	  if ( !is_root() ) {
	    if ( lastchild ) draw_vertical_connector(hconn_x, Y, item_y_center, prefs);
	    else             draw_vertical_connector(hconn_x, Y, Y+H2, prefs);
	  }
	}
	// Draw collapse icon
	if ( has_children() && prefs.showcollapse() ) {
	  // Draw icon image
#if FLTK_ABI_VERSION >= 10304
	  if ( is_open() ) {
	    if ( active ) prefs.closeicon()->draw(icon_x,icon_y);
	    else          prefs.closedeicon()->draw(icon_x,icon_y);
	  } else {
	    if ( active ) prefs.openicon()->draw(icon_x,icon_y);
	    else          prefs.opendeicon()->draw(icon_x,icon_y);
	  }
#else
	  if ( is_open() ) {
	    prefs.closeicon()->draw(icon_x,icon_y);
	  } else {
	    prefs.openicon()->draw(icon_x,icon_y);
	  }
#endif
	}
	// Draw background for the item.. only if different from tree's bg color
	if ( bg != tree->color() || is_selected() ) {
	  if ( is_selected() ) {			// Selected? Use selectbox() style
	    fl_draw_box(prefs.selectbox(),bg_x,bg_y,bg_w,bg_h,bg);
	  } else {					// Not Selected? use plain filled rectangle
	    fl_color(bg);
	    fl_rectf(bg_x,bg_y,bg_w,bg_h);
	  }
	  if ( widget() ) widget()->damage(FL_DAMAGE_ALL);	// if there's a child widget, we just damaged it
	}
	// Draw user icon (if any)
#if FLTK_ABI_VERSION >= 10304
	if ( usericon() ) {
	  // Item has user icon? Use it
	  int uicon_y = item_y_center - (usericon()->h() >> 1);
	  if ( active ) usericon()->draw(uicon_x,uicon_y);
	  else if ( userdeicon() ) userdeicon()->draw(uicon_x,uicon_y);
	} else if ( prefs.usericon() ) {
	  // Prefs has user icon? Use it
	  int uicon_y = item_y_center - (prefs.usericon()->h() >> 1);
	  if ( active ) prefs.usericon()->draw(uicon_x,uicon_y);
	  else if ( userdeicon() ) prefs.userdeicon()->draw(uicon_x,uicon_y);
	}
#else
	if ( usericon() ) {
	  // Item has user icon? Use it
	  int uicon_y = item_y_center - (usericon()->h() >> 1);
	  usericon()->draw(uicon_x,uicon_y);
	} else if ( prefs.usericon() ) {
	  // Prefs has user icon? Use it
	  int uicon_y = item_y_center - (prefs.usericon()->h() >> 1);
	  prefs.usericon()->draw(uicon_x,uicon_y);
	}
#endif
	// Draw label
#if FLTK_ABI_VERSION >= 10301
	if ( _label && 
	     ( !widget() || 
	       (prefs.item_draw_mode() & FL_TREE_ITEM_DRAW_LABEL_AND_WIDGET) ) )
#else /*FLTK_ABI_VERSION*/
	if ( _label && !widget() )	// back compat: don't draw label if widget() present
#endif /*FLTK_ABI_VERSION*/
	{
	  fl_color(fg);
	  fl_font(_labelfont, _labelsize);
	  int label_y = Y+(H/2)+(_labelsize/2)-fl_descent()/2;
	  fl_draw(_label, label_x, label_y);
	}
      }			// end non-child damage
      // Draw child FLTK widget?
      if ( widget() ) {
        ((Fl_Tree*)tree)->draw_child(*widget());		// let group handle drawing child
	if ( widget()->label() )
	    ((Fl_Tree*)tree)->draw_outside_label(*widget());	// label too
      }
      // Draw focus box around item's bg last
      if ( this == itemfocus &&
           Fl::visible_focus() && 
	   Fl::focus() == tree &&
	   prefs.selectmode() != FL_TREE_SELECT_NONE ) {
	draw_item_focus(FL_NO_BOX,fg,bg,bg_x+1,bg_y+1,bg_w-1,bg_h-1);
      }
    }			// end drawthis
  }			// end clipped
  if ( drawthis ) Y += H2;			// adjust Y (even if clipped)
  // Draw child items (if any)
  if ( has_children() && is_open() ) {
    int child_x = drawthis ? (hconn_x_center - (icon_w/2) + 1)	// offset children to right,
                           : X;					// unless didn't drawthis
    int child_w = W - (child_x-X);
    int child_y_start = Y;
    for ( int t=0; t<children(); t++ ) {
      int lastchild = ((t+1)==children()) ? 1 : 0;
      _children[t]->draw(child_x, Y, child_w, tree, itemfocus, prefs, lastchild);
    }
    if ( has_children() && is_open() ) {
      Y += prefs.openchild_marginbottom();		// offset below open child tree
    }
    if ( ! lastchild ) {
      // Special 'clipped' calculation. (intentional variable shadowing)
      int clipped = ((child_y_start < tree_top) && (Y < tree_top)) ||
                    ((child_y_start > tree_bot) && (Y > tree_bot));
      if (!clipped) draw_vertical_connector(hconn_x, child_y_start, Y, prefs);
    }
  }
}
#endif

/// Was the event on the 'collapse' button of this item?
///
int Fl_Tree_Item::event_on_collapse_icon(const Fl_Tree_Prefs &prefs) const {
  if ( is_visible() && is_active() && has_children() && prefs.showcollapse() ) {
    return(event_inside(_collapse_xywh) ? 1 : 0);
  } else {
    return(0);
  }
}

/// Was event on the label() of this item?
///
int Fl_Tree_Item::event_on_label(const Fl_Tree_Prefs &prefs) const {
  if ( is_visible() && is_active() ) {
    return(event_inside(_label_xywh) ? 1 : 0);
  } else {
    return(0);
  }
}

/// Internal: Show the FLTK widget() for this item and all children.
/// Used by open() to re-show widgets that were hidden by a previous close()
///
void Fl_Tree_Item::show_widgets() {
  if ( _widget ) _widget->show();
  if ( is_open() ) {
    for ( int t=0; t<_children.total(); t++ ) {
      _children[t]->show_widgets();
    }
  }
}

/// Internal: Hide the FLTK widget() for this item and all children.
/// Used by close() to hide widgets.
///
void Fl_Tree_Item::hide_widgets() {
  if ( _widget ) _widget->hide();
  for ( int t=0; t<_children.total(); t++ ) {
    _children[t]->hide_widgets();
  }
}

/// Open this item and all its children.
void Fl_Tree_Item::open() {
  set_flag(OPEN,1);
  // Tell children to show() their widgets
  for ( int t=0; t<_children.total(); t++ ) {
    _children[t]->show_widgets();
  }
  recalc_tree();		// may change tree geometry
}

/// Close this item and all its children.
void Fl_Tree_Item::close() {
  set_flag(OPEN,0);
  // Tell children to hide() their widgets
  for ( int t=0; t<_children.total(); t++ ) {
    _children[t]->hide_widgets();
  }
  recalc_tree();		// may change tree geometry
}

/// Returns how many levels deep this item is in the hierarchy.
///
/// For instance; root has a depth of zero, and its immediate children
/// would have a depth of 1, and so on. Use e.g. for determining the
/// horizontal indent of this item during drawing.
///
int Fl_Tree_Item::depth() const {
  int count = 0;
  const Fl_Tree_Item *item = parent();
  while ( item ) {
    ++count;
    item = item->parent();
  }
  return(count);
}

/// Return the next item in the tree.
///
/// This method can be used to walk the tree forward.
/// For an example of how to use this method, see Fl_Tree::first().
/// 
/// \returns the next item in the tree, or 0 if there's no more items.
///
Fl_Tree_Item *Fl_Tree_Item::next() {
  Fl_Tree_Item *p, *c = this;
  if ( c->has_children() ) {
    return(c->child(0));
  }
#if FLTK_ABI_VERSION >= 10301
  // NEW
  while ( ( p = c->parent() ) != NULL ) {	// loop upwards through parents
    if ( c->_next_sibling )			// not last child?
      return(c->_next_sibling);			// return next child
    c = p;					// child becomes parent to move up generation
  }						// loop: moves up to next parent
#else /*FLTK_ABI_VERSION*/
  // OLD
  while ( ( p = c->parent() ) != NULL ) {	// loop upwards through parents
    int t = p->find_child(c);			// find our position in parent's children[] array
    if ( ++t < p->children() )			// not last child?
      return(p->child(t));			// return next child
    c = p;					// child becomes parent to move up generation
  }						// loop: moves up to next parent
#endif /*FLTK_ABI_VERSION*/
  return(0);					// hit root? done
}

/// Return the previous item in the tree.
///
/// This method can be used to walk the tree backwards.
/// For an example of how to use this method, see Fl_Tree::last().
/// 
/// \returns the previous item in the tree, 
///          or 0 if there's no item above this one (hit the root).
///
Fl_Tree_Item *Fl_Tree_Item::prev() {
#if FLTK_ABI_VERSION >= 10301
  // NEW
  if ( !parent() ) return(0);	// hit root? done
  if ( !_prev_sibling ) {	// are we first child?
    return(parent());		// return parent
  }
  // Tricky: in the following example our current position
  // in the tree is 'j', and we need to move "up one" to 'i':
  //
  //        ROOT
  //          |-- a
  //              b-- c
  //              |   d-- e
  //              |   |   f
  //              |   |
  //              |   g-- h
  //              |       i
  //              j
  //
  // We do this via b->g->i:
  //    1. Find j's prev_sibling (b)  _
  //    2. Find b's 'last child' (g)   |_ while loop
  //    3. Find g's 'last child' (i)  _|
  //
  Fl_Tree_Item *p = _prev_sibling;	// focus on our prev sibling
  while ( p->has_children() ) {		// item has children?
    p = p->child(p->children()-1);	// descend hierarchy finding deepest 'last child'
  }
  return(p);
#else /*FLTK_ABI_VERSION*/
  // OLD
  Fl_Tree_Item *p=parent();		// start with parent
  if ( ! p ) return(0);			// hit root? done
  int t = p->find_child(this);		// find our position in parent's children[] array
  if ( --t == -1 ) {	 		// are we first child?
    return(p);				// return immediate parent
  }
  p = p->child(t);			// take parent's previous child
  while ( p->has_children() ) {		// has children?
    p = p->child(p->children()-1);	// take last child
  }
  return(p);
#endif /*FLTK_ABI_VERSION*/
}

/// Return this item's next sibling.
///
/// Moves to the next item below us at the same level (sibling).
/// Use this to move down the tree without changing depth().
/// effectively skipping over this item's children/descendents.
/// 
/// \returns item's next sibling, or 0 if none.
///
Fl_Tree_Item *Fl_Tree_Item::next_sibling() {
#if FLTK_ABI_VERSION >= 10301
  // NEW
  return(_next_sibling);
#else /*FLTK_ABI_VERSION*/
  // OLD
  if ( !parent() ) return(0);			// No parent (root)? We have no siblings
  int index = parent()->find_child(this);	// find our position in parent's child() array
  if ( index == -1 ) return(0);			// parent doesn't know us? weird
  if ( (index+1) < parent()->children() )	// is there a next child?
    return(parent()->child(index+1));		// return next child if there's one below us
  return(0);					// no siblings below us
#endif /*FLTK_ABI_VERSION*/
}

/// Return this item's previous sibling.
///
/// Moves to the previous item above us at the same level (sibling).
/// Use this to move up the tree without changing depth().
/// 
/// \returns This item's previous sibling, or 0 if none.
///
Fl_Tree_Item *Fl_Tree_Item::prev_sibling() {
#if FLTK_ABI_VERSION >= 10301
  // NEW
  return(_prev_sibling);
#else /*FLTK_ABI_VERSION*/
  // OLD
  if ( !parent() ) return(0);				// No parent (root)? We have no siblings
  int index = parent()->find_child(this);		// find next position up in parent's child() array
  if ( index == -1 ) return(0);				// parent doesn't know us? weird
  if ( index > 0 ) return(parent()->child(index-1));	// return previous child if there's one above us
  return(0);						// no siblings above us
#endif /*FLTK_ABI_VERSION*/
}

/// Update our _prev_sibling and _next_sibling pointers to point to neighbors
/// given \p index as being our current position in the parent's item array.
/// Call this whenever items in the array are added/removed/moved/swapped/etc.
/// \param[in] index Our index# in the parent.<br>
///                  Special case if index=-1: become an orphan; null out all parent/sibling associations.
/// 
void Fl_Tree_Item::update_prev_next(int index) {
#if FLTK_ABI_VERSION >= 10301
  // NEW
  if ( index == -1 ) {	// special case: become an orphan
    _parent = 0;
    _prev_sibling = 0;
    _next_sibling = 0;
    return;
  }
  int pchildren = parent() ? parent()->children() : 0;
  int index_prev = index-1;
  int index_next = index+1;
  // Get pointers to prev+next items
  Fl_Tree_Item *item_prev = (index_prev>=0)&&(index_prev<pchildren) ? parent()->child(index_prev) : 0;
  Fl_Tree_Item *item_next = (index_next>=0)&&(index_next<pchildren) ? parent()->child(index_next) : 0;
  // Adjust our prev+next ptrs
  _prev_sibling = item_prev;
  _next_sibling = item_next;
  // Adjust neighbors to point to us
  if ( item_prev ) item_prev->_next_sibling = this;
  if ( item_next ) item_next->_prev_sibling = this;
#else /*FLTK_ABI_VERSION*/
  // OLD
  // -- does nothing --
#endif /*FLTK_ABI_VERSION*/
}
      
/// Return the next open(), visible() item. 
/// (If this item has children and is closed, children are skipped)
///
/// This method can be used to walk the tree forward, skipping items
/// that are not currently open/visible to the user.
/// 
/// \returns the next open() visible() item below us,
///          or 0 if there's no more items.
/// \version 1.3.3
///
Fl_Tree_Item *Fl_Tree_Item::next_visible(Fl_Tree_Prefs &prefs) {
  Fl_Tree_Item *item = this;
  while ( 1 ) {
    item = item->next();
    if ( !item ) return 0;
    if ( item->is_root() && !prefs.showroot() ) continue;
    if ( item->visible_r() ) return(item);
  }
}

/// Same as next_visible().
/// \deprecated in 1.3.3 for confusing name, use next_visible() instead
Fl_Tree_Item *Fl_Tree_Item::next_displayed(Fl_Tree_Prefs &prefs) {
  return next_visible(prefs);
}

/// Return the previous open(), visible() item. 
/// (If this item above us has children and is closed, its children are skipped)
///
/// This method can be used to walk the tree backward, 
/// skipping items that are not currently open/visible to the user.
/// 
/// \returns the previous open() visible() item above us,
///          or 0 if there's no more items.
///
Fl_Tree_Item *Fl_Tree_Item::prev_visible(Fl_Tree_Prefs &prefs) {
  Fl_Tree_Item *c = this;
  while ( c ) {
    c = c->prev();					// previous item
    if ( !c ) break;					// no more items? done
    if ( c->is_root() )					// root
      return((prefs.showroot()&&c->visible()) ? c : 0);	// return root if visible
    if ( !c->visible() ) continue;			// item not visible? skip
    // Check all parents to be sure none are closed.
    // If closed, move up to that level and repeat until sure none are closed.
    Fl_Tree_Item *p = c->parent();
    while (1) {
      if ( !p || p->is_root() ) return(c);		// hit top? then we're displayed, return c
      if ( p->is_close() ) c = p;			// found closed parent? make it current
      p = p->parent();					// continue up tree
    }
  }
  return(0);						// hit end: no more items
}

/// Same as prev_visible().
/// \deprecated in 1.3.3 for confusing name, use prev_visible()
///
Fl_Tree_Item *Fl_Tree_Item::prev_displayed(Fl_Tree_Prefs &prefs) {
  return prev_visible(prefs);
}

/// See if item and all its parents are open() and visible().
/// \returns
///    1 -- item and its parents are open() and visible()
///    0 -- item (or one of its parents) are invisible or close()ed.
///
int Fl_Tree_Item::visible_r() const {
  if ( !visible() ) return(0);
  for (const Fl_Tree_Item *p=parent(); p; p=p->parent())// move up through parents
    if (!p->visible() || p->is_close()) return(0);	// any parent not visible or closed?
  return(1);
}

/// Call this when our geometry is changed. (Font size, label contents, etc)
/// Schedules tree to recalculate itself, as changes to us may affect tree
/// widget's scrollbar visibility and tab sizes.
/// \version 1.3.3 ABI
///
void Fl_Tree_Item::recalc_tree() {
#if FLTK_ABI_VERSION >= 10303
  _tree->recalc_tree();
#endif
}

//
// End of "$Id$".
//
