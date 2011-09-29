//
// "$Id$"
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <FL/Fl_Tree.H>
#include <FL/Fl_Preferences.H>

//////////////////////
// Fl_Tree.cxx
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

// INTERNAL: scroller callback
static void scroll_cb(Fl_Widget*,void *data) {
  ((Fl_Tree*)data)->redraw();
}

// INTERNAL: Parse elements from path into an array of null terminated strings
//    Handles escape characters.
//    Path="/aa/bb"
//    Return: arr[0]="aa", arr[1]="bb", arr[2]=0
//    Caller must call free_path(arr).
//
static char **parse_path(const char *path) {
  while ( *path == '/' ) path++;	// skip leading '/' 
  // First pass: identify, null terminate, and count separators
  int seps = 1;				// separator count (1: first item)
  int arrsize = 1;			// array size (1: first item)
  char *save = strdup(path);		// make copy we can modify
  char *sin = save, *sout = save;
  while ( *sin ) {
    if ( *sin == '\\' ) {		// handle escape character
      *sout++ = *++sin;
      if ( *sin ) ++sin;
    } else if ( *sin == '/' ) {		// handle submenu
      *sout++ = 0;
      sin++;
      seps++;
      arrsize++;
    } else {				// all other chars
      *sout++ = *sin++;
    }
  }
  *sout = 0;
  arrsize++;				// (room for terminating NULL) 
  // Second pass: create array, save nonblank elements
  char **arr = (char**)malloc(sizeof(char*) * arrsize);
  int t = 0;
  sin = save;
  while ( seps-- > 0 ) {
    if ( *sin ) { arr[t++] = sin; }	// skips empty fields, e.g. '//'
    sin += (strlen(sin) + 1);
  }
  arr[t] = 0;
  return(arr);
}

// INTERNAL: Free the array returned by parse_path()
static void free_path(char **arr) {
  if ( arr ) {
    if ( arr[0] ) { free((void*)arr[0]); }
    free((void*)arr);
  }
}

// INTERNAL: Recursively descend tree hierarchy, accumulating total child count
static int find_total_children(Fl_Tree_Item *item, int count=0) {
  count++;
  for ( int t=0; t<item->children(); t++ ) {
    count = find_total_children(item->child(t), count);
  }
  return(count);
}

/// Constructor.
Fl_Tree::Fl_Tree(int X, int Y, int W, int H, const char *L) : Fl_Group(X,Y,W,H,L) { 
  _root = new Fl_Tree_Item(_prefs);
  _root->parent(0);				// we are root of tree
  _root->label("ROOT");
  _item_focus      = 0;
  _callback_item   = 0;
  _callback_reason = FL_TREE_REASON_NONE;
  _scrollbar_size  = 0;				// 0: uses Fl::scrollbar_size()
  box(FL_DOWN_BOX);
  color(FL_BACKGROUND2_COLOR, FL_SELECTION_COLOR);
  when(FL_WHEN_CHANGED);
  _vscroll = new Fl_Scrollbar(0,0,0,0);		// will be resized by draw()
  _vscroll->hide();
  _vscroll->type(FL_VERTICAL);
  _vscroll->step(1);
  _vscroll->callback(scroll_cb, (void*)this);
  end();
}

/// Destructor.
Fl_Tree::~Fl_Tree() {
  if ( _root ) { delete _root; _root = 0; }
}

/// Standard FLTK event handler for this widget.
int Fl_Tree::handle(int e) {
  int ret = 0;
  // Developer note: Fl_Browser_::handle() used for reference here..
  // #include <FL/names.h>	// for event debugging
  // fprintf(stderr, "DEBUG: %s (%d)\n", fl_eventnames[e], e);
  if (e == FL_ENTER || e == FL_LEAVE) return(1);
  switch (e) {
    case FL_FOCUS: {
      // FLTK tests if we want focus. 
      //     If a nav key was used to give us focus, and we've got no saved
      //     focus widget, determine which item gets focus depending on nav key.
      //
      if ( ! _item_focus ) {					// no focus established yet?
	switch (Fl::event_key()) {				// determine if focus was navigated..
	  case FL_Tab: {					// received focus via TAB?
	    if ( Fl::event_state(FL_SHIFT) ) {			// SHIFT-TAB similar to FL_Up
	      set_item_focus(next_visible_item(0, FL_Up));
	    } else {						// TAB similar to FL_Down
	      set_item_focus(next_visible_item(0, FL_Down));
	    }
	    break;
	  }
	  case FL_Left:		// received focus via LEFT or UP?
	  case FL_Up: { 	// XK_ISO_Left_Tab
	    set_item_focus(next_visible_item(0, FL_Up));
	    break;
	  }
	  case FL_Right: 	// received focus via RIGHT or DOWN?
	  case FL_Down:
	  default: {
	    set_item_focus(next_visible_item(0, FL_Down));
	    break;
	  }
	}
      }
      if ( visible_focus() ) redraw();	// draw focus change
      return(1);
    }
    case FL_UNFOCUS: {		// FLTK telling us some other widget took focus.
      if ( visible_focus() ) redraw();	// draw focus change
      return(1);
    }
    case FL_KEYBOARD: {		// keyboard shortcut
      // Do shortcuts first or scrollbar will get them...
      if (_prefs.selectmode() > FL_TREE_SELECT_NONE ) {
	if ( !_item_focus ) {
	  set_item_focus(first());
	}
	if ( _item_focus ) {
	  int ekey = Fl::event_key();
	  switch (ekey) {
	    case FL_Enter:	// ENTER: selects current item only
	    case FL_KP_Enter:
	      if ( when() & ~FL_WHEN_ENTER_KEY) {
		select_only(_item_focus);
		show_item(_item_focus);		// STR #2426
		return(1);
	      }
	      break;
	    case ' ':		// toggle selection state
	      switch ( _prefs.selectmode() ) {
		case FL_TREE_SELECT_NONE:
		  break;
		case FL_TREE_SELECT_SINGLE:
		  if ( ! _item_focus->is_selected() )		// not selected?
		    select_only(_item_focus);			// select only this
		  else
		    deselect_all();				// select nothing
		  break;
		case FL_TREE_SELECT_MULTI:
		  select_toggle(_item_focus);
		  break;
	      }
	      break;
	    case FL_Right:  	// open children (if any)
	    case FL_Left: {	// close children (if any)
	      if ( _item_focus ) {
		if ( ekey == FL_Right && _item_focus->is_close() ) {
		  // Open closed item
		  open(_item_focus);
		  redraw();
		  ret = 1;
		} else if ( ekey == FL_Left && _item_focus->is_open() ) {
		  // Close open item
		  close(_item_focus);
		  redraw();	
		  ret = 1;
		}
		return(1);
	      }
	      break;
	    }
	    case FL_Up:		// next item up
	    case FL_Down: {	// next item down
	      set_item_focus(next_visible_item(_item_focus, ekey));	// next item up|dn
	      if ( _item_focus ) {					// item in focus?
	        // Autoscroll
		int itemtop = _item_focus->y();
		int itembot = _item_focus->y()+_item_focus->h();
		if ( itemtop < y() ) { show_item_top(_item_focus); }
		if ( itembot > y()+h() ) { show_item_bottom(_item_focus); }
		// Extend selection
		if ( _prefs.selectmode() == FL_TREE_SELECT_MULTI &&	// multiselect on?
		     (Fl::event_state() & FL_SHIFT) &&			// shift key?
		     ! _item_focus->is_selected() ) {			// not already selected?
		    select(_item_focus);				// extend selection..
		}
		return(1);
	      }
	      break;
	    }
	  }
	}
      }
      break;
    }
  }

  // Let Fl_Group take a shot at handling the event
  if (Fl_Group::handle(e)) {
    return(1);			// handled? don't continue below
  }

  // Handle events the child FLTK widgets didn't need

  static Fl_Tree_Item *lastselect = 0;
  // fprintf(stderr, "ERCODEBUG: Fl_Tree::handle(): Event was %s (%d)\n", fl_eventnames[e], e); // DEBUGGING
  if ( ! _root ) return(ret);
  switch ( e ) {
    case FL_PUSH: {					// clicked on a tree item?
      if (Fl::visible_focus() && handle(FL_FOCUS)) {
        Fl::focus(this);
      }
      lastselect = 0;
      Fl_Tree_Item *o = _root->find_clicked(_prefs);
      if ( ! o ) break;
      set_item_focus(o);				// becomes new focus widget
      redraw();
      ret |= 1;						// handled
      if ( Fl::event_button() == FL_LEFT_MOUSE ) {
	if ( o->event_on_collapse_icon(_prefs) ) {	// collapse icon clicked?
	  open_toggle(o);
	} else if ( o->event_on_label(_prefs) && 	// label clicked?
		 (!o->widget() || !Fl::event_inside(o->widget())) &&		// not inside widget
		 (!_vscroll->visible() || !Fl::event_inside(_vscroll)) ) {	// not on scroller
	  switch ( _prefs.selectmode() ) {
	    case FL_TREE_SELECT_NONE:
	      break;
	    case FL_TREE_SELECT_SINGLE:
	      select_only(o);
	      break;
	    case FL_TREE_SELECT_MULTI: {
	      if ( Fl::event_state() & FL_SHIFT ) {		// SHIFT+PUSH?
	        select(o);					// add to selection
	      } else if ( Fl::event_state() & FL_CTRL ) {	// CTRL+PUSH?
		select_toggle(o);				// toggle selection state
		lastselect = o;					// save toggled item (prevent oscillation)
	      } else {
		select_only(o);
	      }
	      break;
	    }
	  }
	}
      }
      break;
    }
    case FL_DRAG: {
      // do the scrolling first:
      int my = Fl::event_y();
      if ( my < y() ) {				// above top?
        int p = vposition()-(y()-my);
	if ( p < 0 ) p = 0;
        vposition(p);
      } else if ( my > (y()+h()) ) {		// below bottom?
        int p = vposition()+(my-y()-h());
	if ( p > (int)_vscroll->maximum() ) p = (int)_vscroll->maximum();
        vposition(p);
      }
      if ( Fl::event_button() != FL_LEFT_MOUSE ) break;
      Fl_Tree_Item *o = _root->find_clicked(_prefs);
      if ( ! o ) break;
      set_item_focus(o);			// becomes new focus widget
      redraw();
      ret |= 1;
      // Item's label clicked?
      if ( o->event_on_label(_prefs) && 
	   (!o->widget() || !Fl::event_inside(o->widget())) &&
	   (!_vscroll->visible() || !Fl::event_inside(_vscroll)) ) {
	// Handle selection behavior
	switch ( _prefs.selectmode() ) {
	  case FL_TREE_SELECT_NONE: break;	// no selection changes
	  case FL_TREE_SELECT_SINGLE:
	    select_only(o);
	    break;
	  case FL_TREE_SELECT_MULTI:
	    if ( Fl::event_state() & FL_CTRL &&	// CTRL-DRAG: toggle?
	         lastselect != o ) {		// not already toggled from last microdrag?
	      select_toggle(o);			// toggle selection
	      lastselect = o;			// save we toggled it (prevents oscillation)
	    } else {
	      select(o);			// select this
	    }
	    break;
	}
      }
      break;
    }
  }
  return(ret);
}

/// Standard FLTK draw() method, handles draws the tree widget.
void Fl_Tree::draw() {
  // Let group draw box+label but *NOT* children.
  // We handle drawing children ourselves by calling each item's draw()
  //
  // Handle group's bg
  Fl_Group::draw_box();
  Fl_Group::draw_label();
  // Handle tree
  if ( ! _root ) return;
  int cx = x() + Fl::box_dx(box());
  int cy = y() + Fl::box_dy(box());
  int cw = w() - Fl::box_dw(box());
  int ch = h() - Fl::box_dh(box());
  // These values are changed during drawing
  // 'Y' will be the lowest point on the tree
  int X = cx + _prefs.marginleft();
  int Y = cy + _prefs.margintop() - (_vscroll->visible() ? _vscroll->value() : 0);
  int W = cw - _prefs.marginleft();			// - _prefs.marginright();
  int Ysave = Y;
  fl_push_clip(cx,cy,cw,ch);
  {
    fl_font(_prefs.labelfont(), _prefs.labelsize());
    _root->draw(X, Y, W, this,
                (Fl::focus()==this)?_item_focus:0,	// show focus item ONLY if Fl_Tree has focus
		_prefs);
  }
  fl_pop_clip();
  
  // Show vertical scrollbar?
  int ydiff = (Y+_prefs.margintop())-Ysave;		// ydiff=size of tree
  int ytoofar = (cy+ch) - Y;				// ytoofar -- scrolled beyond bottom (e.g. stow)
  
  //printf("ydiff=%d ch=%d Ysave=%d ytoofar=%d value=%d\n",
  //int(ydiff),int(ch),int(Ysave),int(ytoofar), int(_vscroll->value()));
  
  if ( ytoofar > 0 ) ydiff += ytoofar;
  if ( Ysave<cy || ydiff > ch || int(_vscroll->value()) > 1 ) {
    _vscroll->visible();

    int scrollsize = _scrollbar_size ? _scrollbar_size : Fl::scrollbar_size();
    int sx = x()+w()-Fl::box_dx(box())-scrollsize;
    int sy = y()+Fl::box_dy(box());
    int sw = scrollsize;
    int sh = h()-Fl::box_dh(box());
    _vscroll->show();
    _vscroll->range(0.0,ydiff-ch);
    _vscroll->resize(sx,sy,sw,sh);
    _vscroll->slider_size(float(ch)/float(ydiff));
  } else {
    _vscroll->Fl_Slider::value(0);
    _vscroll->hide();
  }
  fl_push_clip(cx,cy,cw,ch);
  Fl_Group::draw_children();	// draws any FLTK children set via Fl_Tree::widget()
  fl_pop_clip();
}

/// Print the tree as 'ascii art' to stdout.
/// Used mainly for debugging.
///
void Fl_Tree::show_self() {
  if ( ! _root ) return;
  _root->show_self();
}

/// Set the label for the root item. 
///
/// Makes an internally managed copy of 'new_label'.
///
void Fl_Tree::root_label(const char *new_label) {
  if ( ! _root ) return;
  _root->label(new_label);
}

/// Returns the root item.
Fl_Tree_Item* Fl_Tree::root() {
  return(_root);
}

/// Adds a new item, given a 'menu style' path, eg: "/Parent/Child/item".
/// Any parent nodes that don't already exist are created automatically.
/// Adds the item based on the value of sortorder().
///
/// To specify items or submenus that contain slashes ('/' or '\')
/// use an escape character to protect them, e.g.
///
/// \code
///     tree->add("/Holidays/Photos/12\\/25\\2010");          // Adds item "12/25/2010"
///     tree->add("/Pathnames/c:\\\\Program Files\\\\MyApp"); // Adds item "c:\Program Files\MyApp"
/// \endcode
///
/// \returns the child item created, or 0 on error.
///
Fl_Tree_Item* Fl_Tree::add(const char *path) {
  if ( ! _root ) {					// Create root if none
    _root = new Fl_Tree_Item(_prefs);
    _root->parent(0);
    _root->label("ROOT");
  }
  char **arr = parse_path(path);
  Fl_Tree_Item *item = _root->add(_prefs, arr);
  free_path(arr);
  return(item);
}

/// Add a new child to a tree-item.
///
/// \param[in] item The existing item to add new child to. Must not be NULL.
/// \param[in] name The label for the new item
/// \returns the item that was added.
///
Fl_Tree_Item* Fl_Tree::add(Fl_Tree_Item *item, const char *name) {
  return(item->add(_prefs, name));
}

/// Inserts a new item above the specified Fl_Tree_Item, with the label set to 'name'.
/// \param[in] above -- the item above which to insert the new item. Must not be NULL.
/// \param[in] name -- the name of the new item
/// \returns the item that was added, or 0 if 'above' could not be found.
/// 
Fl_Tree_Item* Fl_Tree::insert_above(Fl_Tree_Item *above, const char *name) {
  return(above->insert_above(_prefs, name));
}

/// Insert a new item into a tree-item's children at a specified position.
///
/// \param[in] item The existing item to insert new child into. Must not be NULL.
/// \param[in] name The label for the new item
/// \param[in] pos The position of the new item in the child list
/// \returns the item that was added.
///
Fl_Tree_Item* Fl_Tree::insert(Fl_Tree_Item *item, const char *name, int pos) {
  return(item->insert(_prefs, name, pos));
}

/// Remove the specified \p item from the tree.
/// \p item may not be NULL.
/// If it has children, all those are removed too.
/// \returns 0 if done, -1 if 'item' not found.
///
int Fl_Tree::remove(Fl_Tree_Item *item) {
  if ( item == _root ) {
    clear();
  } else {
    Fl_Tree_Item *parent = item->parent();		// find item's parent
    if ( ! parent ) return(-1);
    parent->remove_child(item);			// remove child + children
  }
  return(0);
} 

/// Clear all children from the tree.
/// The tree will be left completely empty.
///
void Fl_Tree::clear() {
  if ( ! _root ) return;
  _root->clear_children();
  delete _root; _root = 0;
} 
/// Clear all the children of a particular node in the tree specified by \p item.
/// Item may not be NULL.
///
void Fl_Tree::clear_children(Fl_Tree_Item *item) {
  if ( item->has_children() ) {
    item->clear_children();
    redraw();				// redraw only if there were children to clear
  }
} 

/// Find the item, given a menu style path, eg: "/Parent/Child/item".
/// There is both a const and non-const version of this method.
/// Const version allows pure const methods to use this method 
/// to do lookups without causing compiler errors.
///
/// To specify items or submenus that contain slashes ('/' or '\')
/// use an escape character to protect them, e.g.
///
/// \code
///     tree->add("/Holidays/Photos/12\\/25\\2010");          // Adds item "12/25/2010"
///     tree->add("/Pathnames/c:\\\\Program Files\\\\MyApp"); // Adds item "c:\Program Files\MyApp"
/// \endcode
///
/// \param[in] path -- the tree item's pathname to be found (e.g. "Flintstones/Fred")
/// \returns the item, or NULL if not found.
///
/// \see item_pathname()
///
Fl_Tree_Item *Fl_Tree::find_item(const char *path) {
  if ( ! _root ) return(NULL);
  char **arr = parse_path(path);
  Fl_Tree_Item *item = _root->find_item(arr);
  free_path(arr);
  return(item);
}

/// A const version of Fl_Tree::find_item(const char *path)
const Fl_Tree_Item *Fl_Tree::find_item(const char *path) const {
  if ( ! _root ) return(NULL);
  char **arr = parse_path(path);
  const Fl_Tree_Item *item = _root->find_item(arr);
  free_path(arr);
  return(item);
}

// Handle safe 'reverse string concatenation'.
//   In the following we build the pathname from right-to-left,
//   since we start at the child and work our way up to the root.
//
#define SAFE_RCAT(c) { \
  slen += 1; if ( slen >= pathnamelen ) { pathname[0] = '\0'; return(-2); } \
  *s-- = c; \
  }

/// Find the pathname for the specified \p item.
/// If \p item is NULL, root() is used.
/// The tree's root will be included in the pathname of showroot() is on.
/// Menu items or submenus that contain slashes ('/' or '\') in their names
/// will be escaped with a backslash. This is symmetrical with the add()
/// function which uses the same escape pattern to set names.
/// \param[in] pathname The string to use to return the pathname
/// \param[in] pathnamelen The maximum length of the string (including NULL). Must not be zero.
/// \param[in] item The item whose pathname is to be returned.
/// \returns
///	-   0 : OK (\p pathname returns the item's pathname)
///	-  -1 : item not found (pathname="")
///	-  -2 : pathname not large enough (pathname="")
/// \see find_item()
///
int Fl_Tree::item_pathname(char *pathname, int pathnamelen, const Fl_Tree_Item *item) const {
  pathname[0] = '\0';
  item = item ? item : _root;
  if ( !item ) return(-1);
  // Build pathname starting at end
  char *s = (pathname+pathnamelen-1);
  int slen = 0;			// length of string compiled so far (including NULL)
  SAFE_RCAT('\0');
  while ( item ) {
    if ( item->is_root() && showroot() == 0 ) break;		// don't include root in path if showroot() off
    // Find name of current item
    const char *name = item->label() ? item->label() : "???";	// name for this item
    int len = strlen(name);
    // Add name to end of pathname[]
    for ( --len; len>=0; len-- ) {
      SAFE_RCAT(name[len]);					// rcat name of item
      if ( name[len] == '/' || name[len] == '\\' ) {
        SAFE_RCAT('\\');					// escape front or back slashes within name
      }
    }
    SAFE_RCAT('/');						// rcat leading slash
    item = item->parent();					// move up tree (NULL==root)
  }
  if ( *(++s) == '/' ) ++s;				// leave off leading slash from pathname
  if ( s != pathname ) memmove(pathname, s, slen);	// Shift down right-aligned string
  return(0);
}

/// Find the item that was clicked.
/// You should use callback_item() instead, which is fast,
/// and is meant to be used within a callback to determine the item clicked.
///
/// This method walks the entire tree looking for the first item that is
/// under the mouse (ie. at Fl::event_x()/Fl:event_y().
///
/// Use this method /only/ if you've subclassed Fl_Tree, and are receiving
/// events before Fl_Tree has been able to process and update callback_item().
/// 
/// \returns the item clicked, or 0 if no item was under the current event.
///
const Fl_Tree_Item* Fl_Tree::find_clicked() const {
  if ( ! _root ) return(NULL);
  return(_root->find_clicked(_prefs));
}

/// Set the item that was last clicked.
/// Should only be used by subclasses needing to change this value.
/// Normally Fl_Tree manages this value.
///
/// Deprecated: use callback_item() instead.
///
void Fl_Tree::item_clicked(Fl_Tree_Item* val) {
  _callback_item = val;
}

/// Return the item that was last clicked.
///
/// Valid only from within the callback().
///
/// Deprecated: use callback_item() instead.
///
/// \returns the item clicked, or 0 if none.
///          0 may also be used to indicate several items were clicked/changed.
///
Fl_Tree_Item* Fl_Tree::item_clicked() {
  return(_callback_item);
}

/// Returns next visible item above (dir==Fl_Up) or below (dir==Fl_Down) the specified \p item.
/// If \p item is 0, returns first() if \p dir is Fl_Up, or last() if \p dir is FL_Down.
///
/// \param[in] item The item above/below which we'll find the next visible item
/// \param[in] dir The direction to search. Can be FL_Up or FL_Down.
/// \returns The item found, or 0 if there's no visible items above/below the specified \p item.
///
Fl_Tree_Item *Fl_Tree::next_visible_item(Fl_Tree_Item *item, int dir) {
  if ( ! item ) {				// no start item?
    item = ( dir == FL_Up ) ? last() : first();	// start at top or bottom
    if ( ! item ) return(0);
    if ( item->visible_r() ) return(item);	// return first/last visible item
  }
  switch ( dir ) {
    case FL_Up:   return(item->prev_displayed(_prefs));
    case FL_Down: return(item->next_displayed(_prefs));
    default:      return(item->next_displayed(_prefs));
  }
}

/// Returns the first item in the tree.
///
/// Use this to walk the tree in the forward direction, eg:
/// \code
/// for ( Fl_Tree_Item *item = tree->first(); item; item = tree->next(item) ) {
///     printf("Item: %s\n", item->label());
/// }
/// \endcode
///
/// \returns first item in tree, or 0 if none (tree empty).
/// \see first(),next(),last(),prev()
///
Fl_Tree_Item* Fl_Tree::first() {
  return(_root);					// first item always root
}

/// Return the next item after \p item, or 0 if no more items.
///
/// Use this code to walk the entire tree:
/// \code
/// for ( Fl_Tree_Item *item = tree->first(); item; item = tree->next(item) ) {
///     printf("Item: %s\n", item->label());
/// }
/// \endcode
///
/// \param[in] item The item to use to find the next item. If NULL, returns 0.
/// \returns Next item in tree, or 0 if at last item.
///
/// \see first(),next(),last(),prev()
///
Fl_Tree_Item *Fl_Tree::next(Fl_Tree_Item *item) {
  if ( ! item ) return(0);
  return(item->next());
}

/// Return the previous item before \p item, or 0 if no more items.
///
/// This can be used to walk the tree in reverse, eg:
///
/// \code
/// for ( Fl_Tree_Item *item = tree->first(); item; item = tree->prev(item) ) {
///     printf("Item: %s\n", item->label());
/// }
/// \endcode
///
/// \param[in] item The item to use to find the previous item. If NULL, returns 0.
/// \returns Previous item in tree, or 0 if at first item.
///
/// \see first(),next(),last(),prev()
///
Fl_Tree_Item *Fl_Tree::prev(Fl_Tree_Item *item) {
  if ( ! item ) return(0);
  return(item->prev());
}

/// Returns the last item in the tree.
///
/// This can be used to walk the tree in reverse, eg:
///
/// \code
/// for ( Fl_Tree_Item *item = tree->last(); item; item = tree->prev() ) {
///     printf("Item: %s\n", item->label());
/// }
/// \endcode
///
/// \returns last item in the tree, or 0 if none (tree empty).
///
/// \see first(),next(),last(),prev()
///
Fl_Tree_Item* Fl_Tree::last() {
  if ( ! _root ) return(0);
  Fl_Tree_Item *item = _root;
  while ( item->has_children() ) {
    item = item->child(item->children()-1);
  }
  return(item);
}

/// Returns the first selected item in the tree.
///
/// Use this to walk the tree looking for all the selected items, eg:
///
/// \code
/// for ( Fl_Tree_Item *item = tree->first_selected_item(); item; item = tree->next_selected_item(item) ) {
///     printf("Item: %s\n", item->label());
/// }
/// \endcode
///
/// \returns The next selected item, or 0 if there are no more selected items.
///     
Fl_Tree_Item *Fl_Tree::first_selected_item() {
  return(next_selected_item(0));
}

/// Returns the next selected item after \p item.
/// If \p item is 0, search starts at the first item (root).
///
/// Use this to walk the tree looking for all the selected items, eg:
/// \code
/// for ( Fl_Tree_Item *item = tree->first_selected_item(); item; item = tree->next_selected_item(item) ) {
///     printf("Item: %s\n", item->label());
/// }
/// \endcode
///
/// \param[in] item The item to use to find the next selected item. If NULL, first() is used.
/// \returns The next selected item, or 0 if there are no more selected items.
///     
Fl_Tree_Item *Fl_Tree::next_selected_item(Fl_Tree_Item *item) {
  if ( ! item ) {
    if ( ! (item = first()) ) return(0);
    if ( item->is_selected() ) return(item);
  }
  while ( (item = item->next()) )
    if ( item->is_selected() )
      return(item);
  return(0);
}

/// Open the specified 'item'.
/// This causes the item's children (if any) to be shown.
/// Handles redrawing if anything was actually changed.
/// Invokes the callback depending on the value of optional parameter \p docallback.
///
/// The callback can use callback_item() and callback_reason() respectively to determine 
/// the item changed and the reason the callback was called.
///
/// \param[in] item -- the item to be opened. Must not be NULL.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - callback() is not invoked
///     -   1 - callback() is invoked if item changed,
///             callback_reason() will be FL_TREE_REASON_OPENED
/// \returns
///     -   1 -- item was opened
///     -   0 -- item was already open, no change
///
/// \see open(), close(), is_open(), is_close(), callback_item(), callback_reason()
///
int Fl_Tree::open(Fl_Tree_Item *item, int docallback) {
  if ( item->is_open() ) return(0);
  item->open();
  redraw();
  if ( docallback ) {
    do_callback_for_item(item, FL_TREE_REASON_OPENED);
  }
  return(1);
}

/// Opens the item specified by \p path (eg: "Parent/child/item").
/// This causes the item's children (if any) to be shown.
/// Handles redrawing if anything was actually changed.
/// Invokes the callback depending on the value of optional parameter \p docallback.
///
/// Items or submenus that themselves contain slashes ('/' or '\')
/// should be escaped, e.g. open("Holidays/12\\/25\//2010").
///
/// The callback can use callback_item() and callback_reason() respectively to determine 
/// the item changed and the reason the callback was called.
///
/// \param[in] path -- the tree item's pathname (e.g. "Flintstones/Fred")
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - callback() is not invoked
///     -   1 - callback() is invoked if item changed,
///             callback_reason() will be FL_TREE_REASON_OPENED
/// \returns
///     -   1 -- OK: item opened
///     -   0 -- OK: item was already open, no change
///     -  -1 -- ERROR: item was not found
///
/// \see open(), close(), is_open(), is_close(), callback_item(), callback_reason()
///         
int Fl_Tree::open(const char *path, int docallback) {
  Fl_Tree_Item *item = find_item(path);
  if ( ! item ) return(-1);
  return(open(item, docallback));
}

/// Toggle the open state of \p item.
/// Handles redrawing if anything was actually changed.
/// Invokes the callback depending on the value of optional parameter \p docallback.
///
/// The callback can use callback_item() and callback_reason() respectively to determine 
/// the item changed and the reason the callback was called.
///
/// \param[in] item -- the item whose open state is to be toggled. Must not be NULL.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - callback() is not invoked
///     -   1 - callback() is invoked, callback_reason() will be either
///             FL_TREE_REASON_OPENED or FL_TREE_REASON_CLOSED
///
/// \see open(), close(), is_open(), is_close(), callback_item(), callback_reason()
///
void Fl_Tree::open_toggle(Fl_Tree_Item *item, int docallback) {
  if ( item->is_open() ) {
    close(item, docallback);
  } else {
    open(item, docallback);
  }
}

/// Closes the specified \p item.
/// Handles redrawing if anything was actually changed.
/// Invokes the callback depending on the value of optional parameter \p docallback.
///
/// The callback can use callback_item() and callback_reason() respectively to determine 
/// the item changed and the reason the callback was called.
///
/// \param[in] item -- the item to be closed. Must not be NULL.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - callback() is not invoked
///     -   1 - callback() is invoked if item changed,
///             callback_reason() will be FL_TREE_REASON_CLOSED
/// \returns
///     -   1 -- item was closed
///     -   0 -- item was already closed, no change
///
/// \see open(), close(), is_open(), is_close(), callback_item(), callback_reason()
///
int Fl_Tree::close(Fl_Tree_Item *item, int docallback) {
  if ( item->is_close() ) return(0);
  item->close();
  redraw();
  if ( docallback ) {
    do_callback_for_item(item, FL_TREE_REASON_CLOSED);
  }
  return(1);
}

/// Closes the item specified by \p path, eg: "Parent/child/item".
/// Handles redrawing if anything was actually changed.
/// Invokes the callback depending on the value of optional parameter \p docallback.
///
/// Items or submenus that themselves contain slashes ('/' or '\')
/// should be escaped, e.g. close("Holidays/12\\/25\//2010").
///
/// The callback can use callback_item() and callback_reason() respectively to determine 
/// the item changed and the reason the callback was called.
///
/// \param[in] path -- the tree item's pathname (e.g. "Flintstones/Fred")
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - callback() is not invoked
///     -   1 - callback() is invoked if item changed,
///             callback_reason() will be FL_TREE_REASON_CLOSED
/// \returns
///     -   1 -- OK: item closed
///     -   0 -- OK: item was already closed, no change
///     -  -1 -- ERROR: item was not found
///
/// \see open(), close(), is_open(), is_close(), callback_item(), callback_reason()
///         
int Fl_Tree::close(const char *path, int docallback) {
  Fl_Tree_Item *item = find_item(path);
  if ( ! item ) return(-1);
  return(close(item, docallback));
}

/// See if \p item is open.
///
/// Items that are 'open' are themselves not necessarily visible;
/// one of the item's parents might be closed.
///
/// \param[in] item -- the item to be tested. Must not be NULL.
/// \returns
///     -  1 : item is open
///     -  0 : item is closed
///
int Fl_Tree::is_open(Fl_Tree_Item *item) const {
  return(item->is_open()?1:0);
}

/// See if item specified by \p path (eg: "Parent/child/item") is open.
///
/// Items or submenus that themselves contain slashes ('/' or '\')
/// should be escaped, e.g. is_open("Holidays/12\\/25\//2010").
///
/// Items that are 'open' are themselves not necessarily visible;
/// one of the item's parents might be closed.
///
/// \param[in] path -- the tree item's pathname (e.g. "Flintstones/Fred")
/// \returns
///     -    1 - OK: item is open
///     -    0 - OK: item is closed
///     -   -1 - ERROR: item was not found
///
int Fl_Tree::is_open(const char *path) const {
  const Fl_Tree_Item *item = find_item(path);
  if ( ! item ) return(-1);
  return(item->is_open()?1:0);
}

/// See if the specified \p item is closed.
///
/// \param[in] item -- the item to be tested. Must not be NULL.
/// \returns
///     -   1 : item is open
///     -   0 : item is closed
///
int Fl_Tree::is_close(Fl_Tree_Item *item) const {
  return(item->is_close());
}

/// See if item specified by \p path (eg: "Parent/child/item") is closed.
///
/// Items or submenus that themselves contain slashes ('/' or '\')
/// should be escaped, e.g. is_close("Holidays/12\\/25\//2010").
///
/// \param[in] path -- the tree item's pathname (e.g. "Flintstones/Fred")
/// \returns
///     -   1 - OK: item is closed
///     -   0 - OK: item is open
///     -  -1 - ERROR: item was not found
///
int Fl_Tree::is_close(const char *path) const {
  const Fl_Tree_Item *item = find_item(path);
  if ( ! item ) return(-1);
  return(item->is_close()?1:0);
}

/// Select the specified \p item. Use 'deselect()' to de-select it.
/// Handles redrawing if anything was actually changed.
/// Invokes the callback depending on the value of optional parameter \p docallback.
///
/// The callback can use callback_item() and callback_reason() respectively to determine 
/// the item changed and the reason the callback was called.
///
/// \param[in] item -- the item to be selected. Must not be NULL.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - the callback() is not invoked
///     -   1 - the callback() is invoked if item changed state,
///             callback_reason() will be FL_TREE_REASON_SELECTED
/// \returns
///     -   1 - item's state was changed
///     -   0 - item was already selected, no change was made
///
int Fl_Tree::select(Fl_Tree_Item *item, int docallback) {
  if ( ! item->is_selected() ) {
    item->select();
    set_changed();
    if ( docallback ) {
      do_callback_for_item(item, FL_TREE_REASON_SELECTED);
    }
    redraw();
    return(1);
  }
  return(0);
}

/// Select the item specified by \p path (eg: "Parent/child/item").
/// Handles redrawing if anything was actually changed.
/// Invokes the callback depending on the value of optional parameter \p docallback.
///
/// Items or submenus that themselves contain slashes ('/' or '\')
/// should be escaped, e.g. select("Holidays/12\\/25\//2010").
///
/// The callback can use callback_item() and callback_reason() respectively to determine 
/// the item changed and the reason the callback was called.
///
/// \param[in] path -- the tree item's pathname (e.g. "Flintstones/Fred")
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - the callback() is not invoked
///     -   1 - the callback() is invoked if item changed state,
///             callback_reason() will be FL_TREE_REASON_SELECTED
/// \returns
///     -   1 : OK: item's state was changed
///     -   0 : OK: item was already selected, no change was made
///     -  -1 : ERROR: item was not found
///
int Fl_Tree::select(const char *path, int docallback) {
  Fl_Tree_Item *item = find_item(path);
  if ( ! item ) return(-1);
  return(select(item, docallback));
}

/// Toggle the select state of the specified \p item.
/// Handles redrawing if anything was actually changed.
/// Invokes the callback depending on the value of optional parameter \p docallback.
///
/// The callback can use callback_item() and callback_reason() respectively to determine 
/// the item changed and the reason the callback was called.
///
/// \param[in] item -- the item to be selected. Must not be NULL.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - the callback() is not invoked
///     -   1 - the callback() is invoked, callback_reason() will be
///             either FL_TREE_REASON_SELECTED or FL_TREE_REASON_DESELECTED
///
void Fl_Tree::select_toggle(Fl_Tree_Item *item, int docallback) {
  item->select_toggle();
  set_changed();
  if ( docallback ) {
    do_callback_for_item(item, item->is_selected() ? FL_TREE_REASON_SELECTED
						   : FL_TREE_REASON_DESELECTED);
  }
  redraw();
}

/// De-select the specified \p item.
/// Handles redrawing if anything was actually changed.
/// Invokes the callback depending on the value of optional parameter \p docallback.
///
/// The callback can use callback_item() and callback_reason() respectively to determine 
/// the item changed and the reason the callback was called.
///
/// \param[in] item -- the item to be selected. Must not be NULL.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - the callback() is not invoked
///     -   1 - the callback() is invoked if item changed state,
///             callback_reason() will be FL_TREE_REASON_DESELECTED
/// \returns
///     -   0 - item was already deselected, no change was made
///     -   1 - item's state was changed
///
int Fl_Tree::deselect(Fl_Tree_Item *item, int docallback) {
  if ( item->is_selected() ) {
    item->deselect();
    set_changed();
    if ( docallback ) {
      do_callback_for_item(item, FL_TREE_REASON_DESELECTED);
    }
    redraw();
    return(1);
  }
  return(0);
}

/// Deselect an item specified by \p path (eg: "Parent/child/item").
/// Handles redrawing if anything was actually changed.
/// Invokes the callback depending on the value of optional parameter \p docallback.
///
/// Items or submenus that themselves contain slashes ('/' or '\')
/// should be escaped, e.g. deselect("Holidays/12\\/25\//2010").
///
/// The callback can use callback_item() and callback_reason() respectively to determine 
/// the item changed and the reason the callback was called.
///
/// \param[in] path -- the tree item's pathname (e.g. "Flintstones/Fred")
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - the callback() is not invoked
///     -   1 - the callback() is invoked if item changed state,
///             callback_reason() will be FL_TREE_REASON_DESELECTED
///  \returns
///     -   1 - OK: item's state was changed
///     -   0 - OK: item was already deselected, no change was made
///     -  -1 - ERROR: item was not found
///
int Fl_Tree::deselect(const char *path, int docallback) {
  Fl_Tree_Item *item = find_item(path);
  if ( ! item ) return(-1);
  return(deselect(item, docallback));
}

/// Deselect \p item and all its children.
/// If item is NULL, first() is used.
/// Handles calling redraw() if anything was changed.
/// Invokes the callback depending on the value of optional parameter \p docallback.
///
/// The callback can use callback_item() and callback_reason() respectively to determine 
/// the item changed and the reason the callback was called.
///
/// \param[in] item The item that will be deselected (along with all its children).
///                 If NULL, first() is used.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - the callback() is not invoked
///     -   1 - the callback() is invoked for each item that changed state,
///             callback_reason() will be FL_TREE_REASON_DESELECTED
///
/// \returns count of how many items were actually changed to the deselected state.
///
int Fl_Tree::deselect_all(Fl_Tree_Item *item, int docallback) {
  item = item ? item : first();			// NULL? use first()
  if ( ! item ) return(0);
  int count = 0;
  // Deselect item
  if ( item->is_selected() )
    if ( deselect(item, docallback) )
      ++count;
  // Deselect its children
  for ( int t=0; t<item->children(); t++ ) {
    count += deselect_all(item->child(t), docallback);	// recurse
  }
  return(count);
}

/// Select only the specified \p item, deselecting all others that might be selected.
/// If item is 0, first() is used.
/// Handles calling redraw() if anything was changed.
/// Invokes the callback depending on the value of optional parameter \p docallback.
///
/// The callback can use callback_item() and callback_reason() respectively to determine 
/// the item changed and the reason the callback was called.
///
/// \param[in] selitem The item to be selected. If NULL, first() is used.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - the callback() is not invoked
///     -   1 - the callback() is invoked for each item that changed state, 
///             callback_reason() will be either FL_TREE_REASON_SELECTED or 
///             FL_TREE_REASON_DESELECTED
/// \returns the number of items whose selection states were changed, if any.
///
int Fl_Tree::select_only(Fl_Tree_Item *selitem, int docallback) {
  selitem = selitem ? selitem : first();	// NULL? use first()
  if ( ! selitem ) return(0);
  int changed = 0;
  for ( Fl_Tree_Item *item = first(); item; item = item->next() ) {
    if ( item == selitem ) {
      if ( item->is_selected() ) continue;	// don't count if already selected
      select(item, docallback);
      ++changed;
    } else {
      if ( item->is_selected() ) {
        deselect(item, docallback);
        ++changed;
      }
    }
  }
  return(changed);
}

/// Select \p item and all its children.
/// If item is NULL, first() is used.
/// Handles calling redraw() if anything was changed.
/// Invokes the callback depending on the value of optional parameter \p docallback.
///
/// The callback can use callback_item() and callback_reason() respectively to determine 
/// the item changed and the reason the callback was called.
///
/// \param[in] item The item that will be selected (along with all its children). 
///            If NULL, first() is used.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - the callback() is not invoked
///     -   1 - the callback() is invoked for each item that changed state,
///             callback_reason() will be FL_TREE_REASON_SELECTED
/// \returns count of how many items were actually changed to the selected state.
///
int Fl_Tree::select_all(Fl_Tree_Item *item, int docallback) {
  item = item ? item : first();			// NULL? use first()
  if ( ! item ) return(0);
  int count = 0;
  // Select item
  if ( !item->is_selected() )
    if ( select(item, docallback) )
      ++count;
  // Select its children
  for ( int t=0; t<item->children(); t++ ) {
    count += select_all(item->child(t), docallback);	// recurse
  }
  return(count);
}

/// Set the item that currently should have keyboard focus.
/// Handles calling redraw() to update the focus box (if it is visible).
///
/// \param[in] item The item that should take focus. If NULL, none will have focus.
///
void Fl_Tree::set_item_focus(Fl_Tree_Item *item) {
  if ( _item_focus != item ) {		// changed?
    _item_focus = item;			// update
    if ( visible_focus() ) redraw();	// redraw to update focus box
  }
}

/// See if the specified \p item is selected.
///
/// \param[in] item -- the item to be tested. Must not be NULL.
///
/// \return
///     -   1 : item selected
///     -   0 : item deselected
///
int Fl_Tree::is_selected(Fl_Tree_Item *item) const {
  return(item->is_selected()?1:0);
}

/// See if item specified by \p path (eg: "Parent/child/item") is selected.
///
/// Items or submenus that themselves contain slashes ('/' or '\')
/// should be escaped, e.g. is_selected("Holidays/12\\/25\//2010").
///
/// \param[in] path -- the tree item's pathname (e.g. "Flintstones/Fred")
/// \returns
///     -   1 : item selected
///     -   0 : item deselected
///     -  -1 : item was not found
///
int Fl_Tree::is_selected(const char *path) {
  Fl_Tree_Item *item = find_item(path);
  if ( ! item ) return(-1);
  return(is_selected(item));
}

/// Get the default label fontsize used for creating new items.
Fl_Fontsize Fl_Tree::item_labelsize() const {
  return(_prefs.labelsize());
}

/// Set the default label font size used for creating new items.
/// To change the font size on a per-item basis, use Fl_Tree_Item::labelsize(Fl_Fontsize)
///
void Fl_Tree::item_labelsize(Fl_Fontsize val) {
  _prefs.labelsize(val);
}

/// Get the default font face used for creating new items.
Fl_Font Fl_Tree::item_labelfont() const {
  return(_prefs.labelfont());
}

/// Set the default font face used for creating new items.
/// To change the font face on a per-item basis, use Fl_Tree_Item::labelfont(Fl_Font)
///
void Fl_Tree::item_labelfont(Fl_Font val) {
  _prefs.labelfont(val);
}

/// Get the default label foreground color used for creating new items.
Fl_Color Fl_Tree::item_labelfgcolor(void) const {
  return(_prefs.labelfgcolor());
}

/// Set the default label foreground color used for creating new items.
/// To change the foreground color on a per-item basis, use Fl_Tree_Item::labelfgcolor(Fl_Color)
///
void Fl_Tree::item_labelfgcolor(Fl_Color val) {
  _prefs.labelfgcolor(val);
}

/// Get the default label background color used for creating new items.
Fl_Color Fl_Tree::item_labelbgcolor(void) const {
  return(_prefs.labelbgcolor());
}

/// Set the default label background color used for creating new items.
/// To change the background color on a per-item basis, use Fl_Tree_Item::labelbgcolor(Fl_Color)
///
void Fl_Tree::item_labelbgcolor(Fl_Color val) {
  _prefs.labelbgcolor(val);
}

/// Get the connector color used for tree connection lines.
Fl_Color Fl_Tree::connectorcolor() const {
  return(_prefs.connectorcolor());
}

/// Set the connector color used for tree connection lines.
void Fl_Tree::connectorcolor(Fl_Color val) {
  _prefs.connectorcolor(val);
}

/// Get the amount of white space (in pixels) that should appear
/// between the widget's left border and the tree's contents.
///
int Fl_Tree::marginleft() const {
  return(_prefs.marginleft());
}

/// Set the amount of white space (in pixels) that should appear
/// between the widget's left border and the left side of the tree's contents.
///
void Fl_Tree::marginleft(int val) {
  _prefs.marginleft(val);
  redraw();
}

/// Get the amount of white space (in pixels) that should appear
/// between the widget's top border and the top of the tree's contents.
///
int Fl_Tree::margintop() const {
  return(_prefs.margintop());
}

/// Sets the amount of white space (in pixels) that should appear
/// between the widget's top border and the top of the tree's contents.
///
void Fl_Tree::margintop(int val) {
  _prefs.margintop(val);
  redraw();
}

/// Get the amount of white space (in pixels) that should appear
/// below an open child tree's contents.
///
int Fl_Tree::openchild_marginbottom() const {
  return(_prefs.openchild_marginbottom());
}

/// Set the amount of white space (in pixels) that should appear
/// below an open child tree's contents.
///
void Fl_Tree::openchild_marginbottom(int val) {
  _prefs.openchild_marginbottom(val);
  redraw();
}

/// Gets the width of the horizontal connection lines (in pixels) 
/// that appear to the left of each tree item's label.
///
int Fl_Tree::connectorwidth() const {
  return(_prefs.connectorwidth());
}

/// Sets the width of the horizontal connection lines (in pixels) 
/// that appear to the left of each tree item's label.
///
void Fl_Tree::connectorwidth(int val) {
  _prefs.connectorwidth(val);
  redraw();
}

/// Returns the Fl_Image being used as the default user icon for all
/// newly created items.
/// Returns zero if no icon has been set, which is the default.
///
Fl_Image* Fl_Tree::usericon() const {
  return(_prefs.usericon());
}

/// Sets the Fl_Image to be used as the default user icon for all
/// newly created items.
///
/// If you want to specify user icons on a per-item basis,
/// use Fl_Tree_Item::usericon() instead.
///
/// \param[in] val -- The new image to be used, or
///                   zero to disable user icons.
///
void Fl_Tree::usericon(Fl_Image *val) {
  _prefs.usericon(val);
  redraw();
}

/// Returns the icon to be used as the 'open' icon.
/// If none was set, the internal default is returned,
/// a simple '[+]' icon.
///
Fl_Image* Fl_Tree::openicon() const {
  return(_prefs.openicon());
}

/// Sets the icon to be used as the 'open' icon.
/// This overrides the built in default '[+]' icon.
///
/// \param[in] val -- The new image, or zero to use the default [+] icon.
///
void Fl_Tree::openicon(Fl_Image *val) {
  _prefs.openicon(val);
  redraw();
}

/// Returns the icon to be used as the 'close' icon.
/// If none was set, the internal default is returned,
/// a simple '[-]' icon.
///
Fl_Image* Fl_Tree::closeicon() const {
  return(_prefs.closeicon());
}

/// Sets the icon to be used as the 'close' icon.
/// This overrides the built in default '[-]' icon.
///
/// \param[in] val -- The new image, or zero to use the default [-] icon.
///
void Fl_Tree::closeicon(Fl_Image *val) {
  _prefs.closeicon(val);
  redraw();
}

/// Returns 1 if the collapse icon is enabled, 0 if not.
int Fl_Tree::showcollapse() const {
  return(_prefs.showcollapse());
}

/// Set if we should show the collapse icon or not.
/// If collapse icons are disabled, the user will not be able
/// to interactively collapse items in the tree, unless the application
/// provides some other means via open() and close().
///
/// \param[in] val 1: shows collapse icons (default),\n
///                0: hides collapse icons.
///
void Fl_Tree::showcollapse(int val) {
  _prefs.showcollapse(val);
  redraw();
}

/// Returns 1 if the root item is to be shown, or 0 if not.
int Fl_Tree::showroot() const {
  return(_prefs.showroot());
}

/// Set if the root item should be shown or not.
/// \param[in] val 1 -- show the root item (default)\n
///                0 -- hide the root item.
///
void Fl_Tree::showroot(int val) {
  _prefs.showroot(val);
  redraw();
}

/// Returns the line drawing style for inter-connecting items.
Fl_Tree_Connector Fl_Tree::connectorstyle() const {
  return(_prefs.connectorstyle());
}

/// Sets the line drawing style for inter-connecting items.
void Fl_Tree::connectorstyle(Fl_Tree_Connector val) {
  _prefs.connectorstyle(val);
  redraw();
}

/// Set the default sort order used when items are added to the tree.
///     See Fl_Tree_Sort for possible values.
///
Fl_Tree_Sort Fl_Tree::sortorder() const {
  return(_prefs.sortorder());
}

/// Gets the sort order used to add items to the tree.
void Fl_Tree::sortorder(Fl_Tree_Sort val) {
  _prefs.sortorder(val);
  // no redraw().. only affects new add()itions
}

/// Sets the style of box used to draw selected items.
/// This is an fltk Fl_Boxtype.
/// The default is influenced by FLTK's current Fl::scheme()
///
Fl_Boxtype Fl_Tree::selectbox() const {
  return(_prefs.selectbox());
}

/// Gets the style of box used to draw selected items.
/// This is an fltk Fl_Boxtype.
/// The default is influenced by FLTK's current Fl::scheme()
///
void Fl_Tree::selectbox(Fl_Boxtype val) {
  _prefs.selectbox(val);
  redraw();
}

/// Gets the tree's current selection mode.
Fl_Tree_Select Fl_Tree::selectmode() const {
  return(_prefs.selectmode());
}

/// Sets the tree's selection mode.
void Fl_Tree::selectmode(Fl_Tree_Select val) {
  _prefs.selectmode(val);
}

/// See if \p item is currently displayed on-screen (visible within the widget).
/// This can be used to detect if the item is scrolled off-screen.
/// Checks to see if the item's vertical position is within the top and bottom
/// edges of the display window. This does NOT take into account the hide()/show()
/// or open()/close() status of the item.
///
/// \param[in] item The item to be checked. If NULL, first() is used.
/// \returns 1 if displayed, 0 if scrolled off screen or no items are in tree.
///
int Fl_Tree::displayed(Fl_Tree_Item *item) {
  item = item ? item : first();
  if (!item) return(0);
  return( (item->y() >= y()) && (item->y() <= (y()+h()-item->h())) ? 1 : 0);
}

/// Adjust the vertical scroll bar so that \p item is visible
/// \p yoff pixels from the top of the Fl_Tree widget's display.
///
/// For instance, yoff=0 will position the item at the top.
///
/// If yoff is larger than the vertical scrollbar's limit,
/// the value will be clipped. So if yoff=100, but scrollbar's max
/// is 50, then 50 will be used.
///
/// \param[in] item The item to be shown. If NULL, first() is used.
/// \param[in] yoff The pixel offset from the top for the displayed position.
///
/// \see show_item_top(), show_item_middle(), show_item_bottom()
///
void Fl_Tree::show_item(Fl_Tree_Item *item, int yoff) {
  item = item ? item : first();
  if (!item) return;
  int newval = item->y() - y() - yoff + (int)_vscroll->value();
  if ( newval < _vscroll->minimum() ) newval = (int)_vscroll->minimum();
  if ( newval > _vscroll->maximum() ) newval = (int)_vscroll->maximum();
  _vscroll->value(newval);
  redraw();
}

/// Adjust the vertical scroll bar to show \p item at the top
/// of the display IF it is currently off-screen (e.g. show_item_top()).
/// If it is already on-screen, no change is made.
///
/// \param[in] item The item to be shown. If NULL, first() is used.
///
/// \see show_item_top(), show_item_middle(), show_item_bottom()
///
void Fl_Tree::show_item(Fl_Tree_Item *item) {
  item = item ? item : first();
  if (!item) return;
  if ( displayed(item) ) return;
  show_item_top(item);
}

/// Adjust the vertical scrollbar so that \p item is at the top of the display.
///
/// \param[in] item The item to be shown. If NULL, first() is used.
///
void Fl_Tree::show_item_top(Fl_Tree_Item *item) {
  item = item ? item : first();
  if (item) show_item(item, 0);
}

/// Adjust the vertical scrollbar so that \p item is in the middle of the display.
///
/// \param[in] item The item to be shown. If NULL, first() is used.
///
void Fl_Tree::show_item_middle(Fl_Tree_Item *item) {
  item = item ? item : first();
  if (item) show_item(item, (h()/2)-(item->h()/2));
}

/// Adjust the vertical scrollbar so that \p item is at the bottom of the display.
///
/// \param[in] item The item to be shown. If NULL, first() is used.
///
void Fl_Tree::show_item_bottom(Fl_Tree_Item *item) {
  item = item ? item : first();
  if (item) show_item(item, h()-item->h());
}

/// Displays \p item, scrolling the tree as necessary.
/// \param[in] item The item to be displayed. If NULL, first() is used.
///
void Fl_Tree::display(Fl_Tree_Item *item) {
  item = item ? item : first();
  if (item) show_item_middle(item);
}

/// Returns the vertical scroll position as a pixel offset.
/// The position returned is how many pixels of the tree are scrolled off the top edge
/// of the screen.  Example: A position of '3' indicates the top 3 pixels of 
/// the tree are scrolled off the top edge of the screen.
/// \see vposition(), hposition()
///
int Fl_Tree::vposition() const {
  return((int)_vscroll->value());
}

///  Sets the vertical scroll offset to position \p pos.
///  The position is how many pixels of the tree are scrolled off the top edge
///  of the screen. Example: A position of '3' scrolls the top three pixels of
///  the tree off the top edge of the screen.
///  \param[in] pos The vertical position (in pixels) to scroll the browser to.
///
void Fl_Tree::vposition(int pos) {
  if (pos < 0) pos = 0;
  if (pos > _vscroll->maximum()) pos = (int)_vscroll->maximum();
  if (pos == _vscroll->value()) return;
  _vscroll->value(pos);
  redraw();
}

/// See if widget \p w is one of the Fl_Tree widget's scrollbars.
/// Use this to skip over the scrollbars when walking the child() array. Example:
/// \code
/// for ( int i=0; i<tree->children(); i++ ) {    // walk children
///     Fl_Widget *w= tree->child(i);
///     if ( brow->is_scrollbar(w) ) continue;    // skip scrollbars
///     ..do work here..
/// }
/// \endcode
/// \param[in] w Widget to test
/// \returns 1 if \p w is a scrollbar, 0 if not.
///
int Fl_Tree::is_scrollbar(Fl_Widget *w) {
    return( ( w == _vscroll ) ? 1 : 0 );
}

/// Gets the current size of the scrollbars' troughs, in pixels.
///
/// If this value is zero (default), this widget will use the global
/// Fl::scrollbar_size() value as the scrollbar's width.
/// 
/// \returns Scrollbar size in pixels, or 0 if the global Fl::scrollsize() is being used.
/// \see Fl::scrollbar_size(int)
///
int Fl_Tree::scrollbar_size() const {
    return(_scrollbar_size);
}

/// Sets the pixel size of the scrollbars' troughs to the \p size, in pixels.
///
/// Normally you should not need this method, and should use the global
/// Fl::scrollbar_size(int) instead to manage the size of ALL 
/// your widgets' scrollbars. This ensures your application 
/// has a consistent UI, is the default behavior, and is normally
/// what you want.
///
/// Only use THIS method if you really need to override the global
/// scrollbar size. The need for this should be rare.
///   
/// Setting \p size to the special value of 0 causes the widget to
/// track the global Fl::scrollbar_size(), which is the default.
///   
/// \param[in] size Sets the scrollbar size in pixels.\n
///                 If 0 (default), scrollbar size tracks the global Fl::scrollbar_size()
/// \see Fl::scrollbar_size()
///
void Fl_Tree::scrollbar_size(int size) {
    _scrollbar_size = size;
    int scrollsize = _scrollbar_size ? _scrollbar_size : Fl::scrollbar_size();
    if ( _vscroll->w() != scrollsize ) {
      _vscroll->resize(x()+w()-scrollsize, h(), scrollsize, _vscroll->h());
    }
}   

/// Do the callback for the item, setting the item and reason
void Fl_Tree::do_callback_for_item(Fl_Tree_Item* item, Fl_Tree_Reason reason) {
  callback_reason(reason);
  callback_item(item);
  do_callback((Fl_Widget*)this, user_data());
}

/// Sets the item that was changed for this callback.
///    Used internally to pass the item that invoked the callback.
///
void Fl_Tree::callback_item(Fl_Tree_Item* item) {
  _callback_item = item;
}

/// Gets the item that caused the callback.
/// The callback() can use this value to see which item changed.
///
Fl_Tree_Item* Fl_Tree::callback_item() {
  return(_callback_item);
}

/// Sets the reason for this callback.
///    Used internally to pass the reason the callback was invoked.
///
void Fl_Tree::callback_reason(Fl_Tree_Reason reason) {
  _callback_reason = reason;
}

/// Gets the reason for this callback.
///
/// The callback() can use this value to see why it was called. Example:
/// \code
///     void MyTreeCallback(Fl_Widget *w, void *userdata) {
///         Fl_Tree *tree = (Fl_Tree*)w;
///         Fl_Tree_Item *item = tree->callback_item();    // the item changed (can be NULL if more than one item was changed!)
///         switch ( tree->callback_reason() ) {           // reason callback was invoked
///             case     FL_TREE_REASON_OPENED: ..item was opened..
///             case     FL_TREE_REASON_CLOSED: ..item was closed..
///             case   FL_TREE_REASON_SELECTED: ..item was selected..
///             case FL_TREE_REASON_DESELECTED: ..item was deselected..
///         }
///     }
/// \endcode
///
Fl_Tree_Reason Fl_Tree::callback_reason() const {
  return(_callback_reason);
}

/**
 * Read a preferences database into the tree widget.
 * A preferences database is a hierarchical collection of data which can be
 * directly loaded into the tree view for inspection.
 * \param[in] prefs the Fl_Preferences database
 */
void Fl_Tree::load(Fl_Preferences &prefs) 
{
  int i, j, n, pn = strlen(prefs.path());
  char *p;
  const char *path = prefs.path();
  if (strcmp(path, ".")==0)
    path += 1; // root path is empty
  else
    path += 2; // child path starts with "./"
  n = prefs.groups();
  for (i=0; i<n; i++) {
    Fl_Preferences prefsChild(prefs, i);
    add(prefsChild.path()+2); // children always start with "./"
    load(prefsChild);
  }
  n = prefs.entries();
  for (i=0; i<n; i++) {
    // We must remove all fwd slashes in the key and value strings. Replace with backslash.
    char *key = strdup(prefs.entry(i));
    int kn = strlen(key);
    for (j=0; j<kn; j++) {
      if (key[j]=='/') key[j]='\\'; 
    }
    char *val;  prefs.get(key, val, "");
    int vn = strlen(val);
    for (j=0; j<vn; j++) {
      if (val[j]=='/') val[j]='\\'; 
    }
    if (vn<40) {
      int sze = pn + strlen(key) + vn;
      p = (char*)malloc(sze+5);
      sprintf(p, "%s/%s = %s", path, key, val);
    } else {
      int sze = pn + strlen(key) + 40;
      p = (char*)malloc(sze+5);
      sprintf(p, "%s/%s = %.40s...", path, key, val);
    }
    add(p[0]=='/'?p+1:p);
    free(p);
    free(val);
    free(key);
  }
}

//
// End of "$Id$".
//
