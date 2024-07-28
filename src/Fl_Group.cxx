//
// Group widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

// Fl_Group is the basic container type in FLTK. Other container types
// (classes) are usually subclasses of Fl_Group.

// Fl_Window itself is a subclass of this, and most of the event
// handling is designed so windows themselves work correctly.

#include <FL/Fl_Group.H>
#include "Fl_Window_Driver.H"
#include <FL/Fl_Rect.H>
#include <FL/fl_draw.H>

#include <stdlib.h> // malloc etc.

Fl_Group* Fl_Group::current_;

// Hack: A single child is stored in the pointer to the array, while
// multiple children are stored in an allocated array:

/**
  Returns a pointer to the array of children.

  \note This pointer is only valid until the next time a child
        is added or removed.
*/
Fl_Widget*const* Fl_Group::array() const {
  return children_ <= 1 ? &child1_ : array_;
}

/**
  Searches the child array for the widget and returns the index.

  Returns children() if the widget is NULL or not found.
*/
int Fl_Group::find(const Fl_Widget* o) const {
  Fl_Widget*const* a = array();
  int i; for (i=0; i < children_; i++) if (*a++ == o) break;
  return i;
}

// Some (* which? *) compilers / toolchains can't export the static
// class member: current_, so these methods can't be inlined...

/**
  Sets the current group so you can build the widget
  tree by just constructing the widgets.

  begin() is automatically called by the constructor for Fl_Group (and thus for
  Fl_Window as well). begin() <I>is exactly the same as</I> current(this).
  <I>Don't forget to end() the group or window!</I>
*/
void Fl_Group::begin() {current_ = this;}

/**
  <I>Exactly the same as</I> current(this->parent()). Any new widgets
  added to the widget tree will be added to the parent of the group.
*/
void Fl_Group::end() {current_ = parent();}

/**
  Returns the currently active group.

  The Fl_Widget constructor automatically does current()->add(widget) if this
  is not null. To prevent new widgets from being added to a group, call
  Fl_Group::current(0).
*/
Fl_Group *Fl_Group::current() {return current_;}

/**
  Sets the current group.
  \see Fl_Group::current()
*/
void Fl_Group::current(Fl_Group *g) {current_ = g;}

extern Fl_Widget* fl_oldfocus; // set by Fl::focus

// For back-compatibility, we must adjust all events sent to child
// windows so they are relative to that window.

static int send(Fl_Widget* o, int event) {
  if (!o->as_window()) return o->handle(event);
  switch ( event )
  {
  case FL_DND_ENTER: /* FALLTHROUGH */
  case FL_DND_DRAG:
    // figure out correct type of event:
    event = (o->contains(Fl::belowmouse())) ? FL_DND_DRAG : FL_DND_ENTER;
  }
  int save_x = Fl::e_x; Fl::e_x -= o->x();
  int save_y = Fl::e_y; Fl::e_y -= o->y();
  int ret = o->handle(event);
  Fl::e_y = save_y;
  Fl::e_x = save_x;
  switch ( event )
  {
  case FL_ENTER: /* FALLTHROUGH */
  case FL_DND_ENTER:
    // Successful completion of FL_ENTER means the widget is now the
    // belowmouse widget, but only call Fl::belowmouse if the child
    // widget did not do so:
    if (!o->contains(Fl::belowmouse())) Fl::belowmouse(o);
    break;
  }
  return ret;
}

// translate the current keystroke into up/down/left/right for navigation:
static int navkey() {
  // The app may want these for hotkeys, check key state
  if (Fl::event_state(FL_CTRL | FL_ALT | FL_META)) return 0;

  switch (Fl::event_key()) {
  case 0: // not an FL_KEYBOARD/FL_SHORTCUT event
    break;
  case FL_Tab:
    if (!Fl::event_state(FL_SHIFT)) return FL_Right;
    return FL_Left;
  case FL_Right:
    return FL_Right;
  case FL_Left:
    return FL_Left;
  case FL_Up:
    return FL_Up;
  case FL_Down:
    return FL_Down;
  }
  return 0;
}

int Fl_Group::handle(int event) {

  Fl_Widget*const* a = array();
  int i;
  Fl_Widget* o;

  switch (event) {

  case FL_FOCUS:
    switch (navkey()) {
    default:
      if (savedfocus_ && savedfocus_->take_focus()) return 1;
    case FL_Right:
    case FL_Down:
      for (i = children(); i--;) if ((*a++)->take_focus()) return 1;
      break;
    case FL_Left:
    case FL_Up:
      for (i = children(); i--;) if (a[i]->take_focus()) return 1;
      break;
    }
    return 0;

  case FL_UNFOCUS:
    savedfocus_ = fl_oldfocus;
    return 0;

  case FL_KEYBOARD:
    return navigation(navkey());

  case FL_SHORTCUT:
    for (i = children(); i--;) {
      o = a[i];
      if (o->takesevents() && Fl::event_inside(o) && send(o,FL_SHORTCUT))
        return 1;
    }
    for (i = children(); i--;) {
      o = a[i];
      if (o->takesevents() && !Fl::event_inside(o) && send(o,FL_SHORTCUT))
        return 1;
    }
    if ((Fl::event_key() == FL_Enter || Fl::event_key() == FL_KP_Enter)) return navigation(FL_Down);
    return 0;

  case FL_ENTER:
  case FL_MOVE:
    for (i = children(); i--;) {
      o = a[i];
      if (o->visible() && Fl::event_inside(o)) {
        if (o->contains(Fl::belowmouse())) {
          return send(o,FL_MOVE);
        } else {
          Fl::belowmouse(o);
          if (send(o,FL_ENTER)) return 1;
        }
      }
    }
    Fl::belowmouse(this);
    return 1;

  case FL_DND_ENTER:
  case FL_DND_DRAG:
    for (i = children(); i--;) {
      o = a[i];
      if (o->takesevents() && Fl::event_inside(o)) {
        if (o->contains(Fl::belowmouse())) {
          return send(o,FL_DND_DRAG);
        } else if (send(o,FL_DND_ENTER)) {
          if (!o->contains(Fl::belowmouse())) Fl::belowmouse(o);
          return 1;
        }
      }
    }
    Fl::belowmouse(this);
    return 0;

  case FL_PUSH:
    for (i = children(); i--;) {
      o = a[i];
      if (o->takesevents() && Fl::event_inside(o)) {
        Fl_Widget_Tracker wp(o);
        if (send(o,FL_PUSH)) {
          if (Fl::pushed() && wp.exists() && !o->contains(Fl::pushed())) Fl::pushed(o);
          return 1;
        }
      }
    }
    return 0;

  case FL_RELEASE:
  case FL_DRAG:
    o = Fl::pushed();
    if (o == this) return 0;
    else if (o) send(o,event);
    else {
      for (i = children(); i--;) {
        o = a[i];
        if (o->takesevents() && Fl::event_inside(o)) {
          if (send(o,event)) return 1;
        }
      }
    }
    return 0;

  case FL_MOUSEWHEEL:
    for (i = children(); i--;) {
      o = a[i];
      if (o->takesevents() && Fl::event_inside(o) && send(o,FL_MOUSEWHEEL))
        return 1;
    }
    for (i = children(); i--;) {
      o = a[i];
      if (o->takesevents() && !Fl::event_inside(o) && send(o,FL_MOUSEWHEEL))
        return 1;
    }
    return 0;

  case FL_DEACTIVATE:
  case FL_ACTIVATE:
    for (i = children(); i--;) {
      o = *a++;
      if (o->active()) o->handle(event);
    }
    return 1;

  case FL_SHOW:
  case FL_HIDE:
    for (i = children(); i--;) {
      o = *a++;
      if (event == FL_HIDE && o == Fl::focus()) {
        // Give up input focus...
        int old_event = Fl::e_number;
        o->handle(Fl::e_number = FL_UNFOCUS);
        Fl::e_number = old_event;
        Fl::focus(0);
      }
      if (o->visible()) o->handle(event);
    }
    return 1;

  default:
    // For all other events, try to give to each child, starting at focus:
    for (i = 0; i < children(); i ++)
      if (Fl::focus_ == a[i]) break;

    if (i >= children()) i = 0;

    if (children()) {
      for (int j = i;;) {
        if (a[j]->takesevents()) if (send(a[j], event)) return 1;
        j++;
        if (j >= children()) j = 0;
        if (j == i) break;
      }
    }

    return 0;
  }
}

// try to move the focus in response to a keystroke:
int Fl_Group::navigation(int key) {
  if (children() <= 1) return 0;
  int i;
  for (i = 0; ; i++) {
    if (i >= children_) return 0;
    if (array_[i]->contains(Fl::focus())) break;
  }
  Fl_Widget *previous = array_[i];

  for (;;) {
    switch (key) {
    case FL_Right:
    case FL_Down:
      i++;
      if (i >= children_) {
        if (parent()) return 0;
        i = 0;
      }
      break;
    case FL_Left:
    case FL_Up:
      if (i) i--;
      else {
        if (parent()) return 0;
        i = children_-1;
      }
      break;
    default:
      return 0;
    }
    Fl_Widget* o = array_[i];
    if (o == previous) return 0;
    switch (key) {
    case FL_Down:
    case FL_Up:
      // for up/down, the widgets have to overlap horizontally:
      if (o->x() >= previous->x()+previous->w() ||
          o->x()+o->w() <= previous->x()) continue;
    }
    if (o->take_focus()) return 1;
  }
}

////////////////////////////////////////////////////////////////

Fl_Group::Fl_Group(int X,int Y,int W,int H,const char *l)
: Fl_Widget(X,Y,W,H,l) {
  align(FL_ALIGN_TOP);
  children_ = 0;
  array_ = 0;
  savedfocus_ = 0;
  resizable_ = this;
  bounds_ = 0; // this is allocated when first resize() is done
  sizes_ = 0; // see bounds_ (FLTK 1.3 compatibility)

  // Subclasses may want to construct child objects as part of their
  // constructor, so make sure they are add()'d to this object.
  // But you must end() the object!
  begin();
}

/**
  Deletes all child widgets from memory recursively.

  This method differs from the remove() method in that it
  affects all child widgets and deletes them from memory.

  The resizable() widget of the Fl_Group is set to the Fl_Group itself.

  \internal If the Fl_Group widget contains the Fl::focus() or the
  Fl::pushed() widget these are set to sensible values (other widgets
  or the Fl_Group widget itself).

  \see Fl_Group::remove(int), Fl_Group::delete_child(int), Fl_Group::~Fl_Group()
*/
void Fl_Group::clear() {
  savedfocus_ = 0;
  resizable_ = this;
  init_sizes();

  // we must change the Fl::pushed() widget, if it is one of
  // the group's children. Otherwise fl_fix_focus() would send lots
  // of events to children that are about to be deleted anyway.

  Fl_Widget *pushed = Fl::pushed();     // save pushed() widget
  if (contains(pushed)) pushed = this;  // set it to be the group, if it's a child
  Fl::pushed(this);                     // for fl_fix_focus etc.

  // Implementation note (AlbrechtS, Nov. 01, 2022):
  // For some obscure reason the order of all children had been
  // reversed in FLTK 1.3.x so the first child would be deleted
  // first but this is no longer done since FLTK 1.4.0.
  // Reasoning:
  //   (1) it is supposedly better to remove children in the
  //       order "last in, first out"
  //   (2) it would not be compatible with the new subclass
  //       notification feature Fl_Group::on_remove().
  // See git commit a918292547cfb154 or earlier for removed code.
  // End of implementation note.

  // Okay, now it is safe to destroy the children. Children are
  // removed and deleted in the order from last child to first
  // child which is much faster than the other way around and
  // should be the "natural order" (last in, first out).

  while (children_) {                   // delete all children
    int idx = children_-1;              // last child's index
    Fl_Widget* w = child(idx);          // last child widget
    if (w->parent()==this) {            // should always be true
      if (children_>2) {                // optimized removal
        w->parent_ = 0;                 // reset child's parent
        on_remove(idx);
        children_--;                    // update counter
      } else {                          // slow removal
        remove(idx);
      }
      delete w;                         // delete the child
    } else {                            // should never happen
      remove(idx);                      // remove it anyway
    }
  }

  if (pushed != this) Fl::pushed(pushed); // reset pushed() widget

}

/**
  The destructor <I>also deletes all the children</I>. This allows a
  whole tree to be deleted at once, without having to keep a pointer to
  all the children in the user code.

  It is allowed that the Fl_Group and all of its children are automatic
  (local) variables, but you must declare the Fl_Group \e first, so that
  it is destroyed last.

  If you add static or automatic (local) variables to an Fl_Group, then it
  is your responsibility to remove (or delete) all such static or automatic
  child widgets \e \b before destroying the group - otherwise the group will
  attempt to call delete operator on them leading to undefined behavior!
*/
Fl_Group::~Fl_Group() {
  if (current_ == this)
    end();
  clear();
}

/**
 Allow derived groups to act when a widget is added as a child.

 Widgets derived from Fl_Group may store additional data for their children.
 Overriding this method will allow derived classes to generate these data
 structures just before the child is added.

 This method usually returns the same index that was given in the parameters.
 By setting a new index, the position of other widgets in the child pointer
 array can be preserved (e.g. Fl_Scroll keeps its scroll bars as the last
 two children).

 By returning -1, Fl_Group::insert will not add the child to
 array_. This is not recommended, but Fl_Table does something similar to
 forward children to a hidden group.

 \param candidate the candidate will be added to the child array_ after this
            method returns.
 \param index add the child at this position in the array_
 \return index to position the child as planned
 \return a new index to force the child to a different position
 \return -1 to keep the group from adding the candidate
 */
int Fl_Group::on_insert(Fl_Widget *candidate, int index) {
  (void)candidate;
  return index;
}

/**
 Allow derived groups to act when a widget is moved within the group.

 Widgets derived from Fl_Group may store additional data for their children.
 Overriding this method will allow derived classes to move these data
 structures just before the child itself is moved.

 This method usually returns the new index that was given in the
 parameters. By setting a different destination index, the position of other
 widgets in the child pointer array can be preserved.

 By returning -1, Fl_Group::insert will not move the child.

 \param oldIndex the current index of the child that will be moved
 \param newIndex the new index of the child
 \return \p newIndex to position the child as planned
 \return a different index to force the child to a different position
 \return -1 to keep the group from moving the child
 */
int Fl_Group::on_move(int oldIndex, int newIndex) {
  (void)oldIndex;
  return newIndex;
}


/**
  The widget is removed from its current group (if any) and then
  inserted into this group. It is put at index n - or at the end,
  if n >= children(). This can also be used to rearrange
  the widgets inside a group.
*/
void Fl_Group::insert(Fl_Widget &o, int index) {
  if (o.parent()) {
    Fl_Group* g = o.parent();
    int n = g->find(o);
    if (g == this) {
      // avoid expensive remove() and add() if we just move a widget within the group
      index = on_move(n, index);
      if (index < 0) return;      // don't move: requested by subclass
      if (index > children_)
        index = children_;
      if (index > n) index--;     // compensate for removal and re-insertion
      if (index == n) return;     // same position; this includes (children_ == 1)
      if (index > n)
        memmove(array_+n, array_+(n+1), (index-n) * sizeof(Fl_Widget*));
      else
        memmove(array_+(index+1), array_+index, (n-index) * sizeof(Fl_Widget*));
      array_[index] = &o;
      init_sizes();
      return;
    }
    g->remove(n);
  }

  index = on_insert(&o, index);
  if (index == -1) return;

  o.parent_ = this;
  if (children_ == 0) { // use array pointer to point at single child
    child1_ = &o;
  } else if (children_ == 1) { // go from 1 to 2 children
    Fl_Widget* t = child1_;
    array_ = (Fl_Widget**)malloc(2*sizeof(Fl_Widget*));
    if (index) {array_[0] = t; array_[1] = &o;}
    else {array_[0] = &o; array_[1] = t;}
  } else {
    if (!(children_ & (children_-1))) // double number of children
      array_ = (Fl_Widget**)realloc((void*)array_,
                                    2*children_*sizeof(Fl_Widget*));
    int j; for (j = children_; j > index; j--) array_[j] = array_[j-1];
    array_[j] = &o;
  }
  children_++;
  init_sizes();
}

/**
  The widget is removed from its current group (if any) and then added
  to the end of this group.
*/
void Fl_Group::add(Fl_Widget &o) {insert(o, children_);}

/**
 Allow derived groups to act when a child widget is removed from the group.

 Widgets derived from Fl_Group may store additional data for their children.
 Overriding this method will allow derived classes to remove these data
 structures just before the child is removed.

 \param index remove the child at this position in the array_
 */
void Fl_Group::on_remove(int index) {
  (void)index;
}

/**
  Removes the widget at \p index from the group but does not delete it.

  This method does nothing if \p index is out of bounds.

  This method differs from the clear() method in that it only affects
  a single widget and does not delete it from memory.

  \since FLTK 1.3.0
*/
void Fl_Group::remove(int index) {
  if (index < 0 || index >= children_) return;
  on_remove(index);

  Fl_Widget &o = *child(index);
  if (&o == savedfocus_) savedfocus_ = 0;
  if (&o == resizable_) resizable_ = this;
  if (o.parent_ == this) {      // this should always be true
    o.parent_ = 0;
  }

  // remove the widget from the group

  children_--;
  if (children_ == 1) { // go from 2 to 1 child
    Fl_Widget *t = array_[!index];
    free((void*)array_);
    child1_ = t;
  } else if (children_ > 1) { // delete from array
    for (; index < children_; index++) array_[index] = array_[index+1];
  }
  init_sizes();
}

/**
  Removes a widget from the group but does not delete it.

  This method does nothing if the widget is not a child of the group.

  This method differs from the clear() method in that it only affects
  a single widget and does not delete it from memory.

  \note If you have the child's index anyway, use remove(int index)
  instead, because this doesn't need a child lookup in the group's
  table of children. This can be much faster, if there are lots of
  children.
*/
void Fl_Group::remove(Fl_Widget &o) {
  if (!children_) return;
  int i = find(o);
  if (i < children_) remove(i);
}

/**
  Removes the widget at \p index from the group and deletes it.

  This method does nothing if \p index is out of bounds.

  This method differs from the remove() method in that it deletes
  the widget from memory. Since this method is virtual it can be
  reimplemented in subclasses with additional requirements and
  consequences. See the documentation of subclasses.

  Many subclasses don't need to reimplement this method.

  \note This method \b may refuse to remove and delete the widget
    if it is an essential part of the Fl_Group, for instance
    a scrollbar in an Fl_Scroll group. In this case the widget is
    neither removed nor deleted.

  This method does not call init_sizes() or redraw(). This is left
  to user code if necessary.

  Returns 0 if the widget was removed and deleted.
  Return values \> 0 are reserved for use by FLTK core widgets.
  Return values \< 0 are free to be used by user defined widgets.

  \todo Reimplementation of Fl_Group::delete_child(int) in more FLTK
    subclasses. This is not yet complete.

  \param[in]  index   index of child to be removed

  \returns    success (0) or error code
  \retval     0   success
  \retval     1   index out of range
  \retval     2   widget not allowed to be removed (see note)
  \retval    >2   reserved for FLTK use

  \since FLTK 1.4.0
*/
int Fl_Group::delete_child(int index) {
  if (index < 0 || index >= children_)
    return 1;
  Fl_Widget *w = child(index);
  remove(index);
  delete w;
  return 0;
}

/**
  Resets the internal array of widget sizes and positions.

  The Fl_Group widget keeps track of the original widget sizes and
  positions when resizing occurs so that if you resize a window back to
  its original size the widgets will be in the correct places. If you
  rearrange the widgets in your group, call this method to register the
  new arrangement with the Fl_Group that contains them.

  If you add or remove widgets, this will be done automatically.

  \note The internal array of widget sizes and positions will be allocated
        and filled when the next resize() occurs. For more information on
        the contents and structure of the bounds() array see bounds().

  \see bounds()
  \see sizes() (deprecated)
*/
void Fl_Group::init_sizes() {
  delete[] bounds_;
  bounds_ = 0;
  delete[] sizes_;      // FLTK 1.3 compatibility
  sizes_ = 0;           // FLTK 1.3 compatibility
}

/**
  Returns the internal array of widget sizes and positions.

  If the bounds() array does not exist, it will be allocated and filled
  with the current widget sizes and positions.

  The bounds() array stores the initial positions of widgets as Fl_Rect's.
  The size of the array is children() + 2.

  - The first Fl_Rect is the group,
  - the second is the resizable (clipped to the group),
  - the rest are the children.

  This is a convenient order for the resize algorithm.

  If the group and/or the resizable() is a Fl_Window (or subclass) then
  the x() and y() coordinates of their respective Fl_Rect's are zero.

  \note You should never need to use this \e protected method directly,
        unless you have special needs to rearrange the children of a
        Fl_Group. Fl_Tile uses this to rearrange its widget positions.
        The returned array should be considered read-only. Do not change
        its contents. If you need to rearrange children in a group, do
        so by resizing the children and call init_sizes().

  \#include \<FL/Fl_Rect.H\> if you want to access the bounds() array in
  your derived class. Fl_Rect.H is intentionally not included by
  Fl_Group.H to avoid unnecessary dependencies.

  \returns Array of Fl_Rect's with widget positions and sizes. The
        returned array is only valid until init_sizes() is called
        or widgets are added to or removed from the group.

  \see init_sizes()

  \since FLTK 1.4.0

  \internal Notes to developers:
    - If you change this be sure to fix Fl_Tile which also uses this array!
    - Do not \#include Fl_Rect.H in Fl_Group.H because this would introduce
      lots of unnecessary dependencies on Fl_Rect.H.
*/
Fl_Rect* Fl_Group::bounds() {
  if (!bounds_) {
    Fl_Rect* p = bounds_ = new Fl_Rect[children_+2];
    // first thing in bounds array is the group's size:
    if (as_window())
      p[0] = Fl_Rect(w(),h()); // x = y = 0
    else
      p[0] = Fl_Rect(this);
    // next is the resizable's size:
    int left   = p->x(); // init to the group's position and size
    int top    = p->y();
    int right  = p->r();
    int bottom = p->b();
    Fl_Widget* r = resizable();
    if (r && r != this) { // then clip the resizable to it
      int t;
      t = r->x(); if (t > left) left = t;
      t +=r->w(); if (t < right) right = t;
      t = r->y(); if (t > top) top = t;
      t +=r->h(); if (t < bottom) bottom = t;
    }
    p[1] = Fl_Rect(left, top, right-left, bottom-top);
    // next is all the children's sizes:
    p += 2;
    Fl_Widget*const* a = array();
    for (int i=children_; i--;) {
      *p++ = Fl_Rect(*a++);
    }
  }
  return bounds_;
}

/** Returns the internal array of widget sizes and positions.

  For backward compatibility with FLTK versions before 1.4.

  The sizes() array stores the initial positions of widgets as
  (left, right, top, bottom) quads. The first quad is the group, the
  second is the resizable (clipped to the group), and the rest are the
  children. If the group and/or the resizable() is a Fl_Window, then
  the first (left) and third (top) entries of their respective quads
  (x,y) are zero.

  \deprecated Deprecated since 1.4.0. Please use bounds() instead.

  \note This method will be removed in a future FLTK version (1.5.0 or higher).

  \returns      Array of int's with widget positions and sizes. The returned
                array is only valid until init_sizes() is called or widgets
                are added to or removed from the group.

  \note Since FLTK 1.4.0 the returned array is a \b read-only and re-ordered
        copy of the internal bounds() array. Do not change its contents.
        If you need to rearrange children in a group, do so by resizing
        the children and call init_sizes().

  \see bounds()
*/
int* Fl_Group::sizes()
{
  if (sizes_) return sizes_;
  // allocate new sizes_ array and copy bounds_ over to sizes_
  int* pi = sizes_ = new int[4*(children_+2)];
  Fl_Rect *rb = bounds();
  for (int i = 0; i < children_+2; i++, rb++) {
    *pi++ = rb->x();
    *pi++ = rb->r();
    *pi++ = rb->y();
    *pi++ = rb->b();
  }
  return sizes_;
}

/**
  Resizes the Fl_Group widget and all of its children.

  The Fl_Group widget first resizes itself, and then it moves and resizes
  all its children according to the rules documented for
  Fl_Group::resizable(Fl_Widget*)

  \sa Fl_Group::resizable(Fl_Widget*)
  \sa Fl_Group::resizable()
  \sa Fl_Widget::resize(int,int,int,int)
*/
void Fl_Group::resize(int X, int Y, int W, int H) {

  int dx = X - x();
  int dy = Y - y();
  int dw = W - w();
  int dh = H - h();

  Fl_Rect* p = bounds(); // save initial sizes and positions

  Fl_Widget::resize(X, Y, W, H); // make new xywh values visible for children

  // Part 1: no resizable() or both width and height didn't change,
  // just move the children.
  // This case covers also window rescaling where dw == dh == 0.

  if (!resizable() || (dw==0 && dh==0)) {

    // top window and subwindows must not change the position of their children
    if (as_window())
      dx = dy = 0;

    // Check if there's anything to do, otherwise don't call resize().
    // Note that subwindows require resize() even if their relative position
    // didn't change, at least on macOS, if it's a rescale.

    if (Fl_Window::is_a_rescale() || dx || dy) {
      Fl_Widget*const* a = array();
      for (int i = children_; i--;) {
        Fl_Widget* o = *a++;
        o->resize(o->x() + dx, o->y() + dy, o->w(), o->h());
      }
    }
  } // End of part 1

  // Part 2: here we definitely have a resizable() widget, resize children

  else if (children_) {

    // get changes in size/position from the initial size:
    dx = X - p->x();
    dw = W - p->w();
    dy = Y - p->y();
    dh = H - p->h();
    if (as_window())
      dx = dy = 0;
    p++;

    // Developer note:
    // The following code uses T = top, L = left, R = right, and B = bottom
    // widget bounds. T and L are equivalent to x() and y(), whereas
    // R = x() + w() and B = y() + h(), respectively, i.e. the next pixel
    // beyond the widget border.
    // RL, RR, RT, and RB are those values of the resizable widget.

    // get initial size of resizable():
    int RL = p->x();
    int RR = RL + p->w();
    int RT = p->y();
    int RB = RT + p->h();
    p++;

    // resize children
    Fl_Widget*const* a = array();

    for (int i = children_; i--; p++) {

      Fl_Widget* o = *a++;
      int L = p->x();
      int R = L + p->w();
      int T = p->y();
      int B = T + p->h();

      // widget resizing code from Francois Ostiguy (since FLTK 1.4.0)

      if (L >= RR) L += dw;
      else if (L > RL) L += dw * (L-RL) / (RR-RL);
      if (R >= RR) R += dw;
      else if (R > RL) R += dw * (R-RL) / (RR-RL);
      if (T >= RB) T += dh;
      else if (T > RT) T += dh * (T-RT) / (RB-RT);
      if (B >= RB) B += dh;
      else if (B > RT) B += dh * (B-RT) / (RB-RT);

      o->resize(L+dx, T+dy, R-L, B-T);
    }
  } // End of part 2: we have a resizable() widget
}

/**
  Draws all children of the group.

  This is useful, if you derived a widget from Fl_Group and want to draw a special
  border or background. You can call draw_children() from the derived draw() method
  after drawing the box, border, or background.
*/
void Fl_Group::draw_children() {
  Fl_Widget*const* a = array();

  if (clip_children()) {
    fl_push_clip(x() + Fl::box_dx(box()),
                 y() + Fl::box_dy(box()),
                 w() - Fl::box_dw(box()),
                 h() - Fl::box_dh(box()));
  }

  if (damage() & ~FL_DAMAGE_CHILD) { // redraw the entire thing:
    for (int i=children_; i--;) {
      Fl_Widget& o = **a++;
      draw_child(o);
      draw_outside_label(o);
    }
  } else {      // only redraw the children that need it:
    for (int i=children_; i--;) update_child(**a++);
  }

  if (clip_children()) fl_pop_clip();
}

void Fl_Group::draw() {
  if (damage() & ~FL_DAMAGE_CHILD) { // redraw the entire thing:
    draw_box();
    draw_label();
  }
  draw_children();
}

/**
  Draws a child only if it needs it.

  This draws a child widget, if it is not clipped \em and if any damage() bits
  are set. The damage bits are cleared after drawing.

  \sa Fl_Group::draw_child(Fl_Widget& widget) const
*/
void Fl_Group::update_child(Fl_Widget& widget) const {
  if (widget.damage() && widget.visible() && widget.type() < FL_WINDOW &&
      fl_not_clipped(widget.x(), widget.y(), widget.w(), widget.h())) {
    widget.draw();
    widget.clear_damage();
  }
}

/**
  Forces a child to redraw.

  This draws a child widget, if it is not clipped.
  The damage bits are cleared after drawing.
*/
void Fl_Group::draw_child(Fl_Widget& widget) const {
  if (widget.visible() && widget.type() < FL_WINDOW &&
      fl_not_clipped(widget.x(), widget.y(), widget.w(), widget.h())) {
    // The following call clears all damage flags and then *sets* FL_DAMAGE_ALL
    widget.clear_damage(FL_DAMAGE_ALL);
    widget.draw();
    widget.clear_damage();
  }
}

/** Parents normally call this to draw outside labels of child widgets. */
void Fl_Group::draw_outside_label(const Fl_Widget& widget) const {
  if (!widget.visible()) return;
  // skip any labels that are inside the widget:
  if (!(widget.align()&15) || (widget.align() & FL_ALIGN_INSIDE)) return;
  // invent a box that is outside the widget:
  Fl_Align a = widget.align();
  int X = widget.x();
  int Y = widget.y();
  int W = widget.w();
  int H = widget.h();
  int wx, wy;
  if (const_cast<Fl_Group*>(this)->as_window()) {
    wx = wy = 0;
  } else {
    wx = x(); wy = y();
  }
  if ( (a & FL_ALIGN_POSITION_MASK) == FL_ALIGN_LEFT_TOP ) {
    a = (a &(~FL_ALIGN_POSITION_MASK) ) | FL_ALIGN_TOP_RIGHT;
    X = wx;
    W = widget.x()-X-3;
  } else if ( (a & FL_ALIGN_POSITION_MASK) == FL_ALIGN_LEFT_BOTTOM ) {
    a = (a &(~FL_ALIGN_POSITION_MASK) ) | FL_ALIGN_BOTTOM_RIGHT;
    X = wx;
    W = widget.x()-X-3;
  } else if ( (a & FL_ALIGN_POSITION_MASK) == FL_ALIGN_RIGHT_TOP ) {
    a = (a &(~FL_ALIGN_POSITION_MASK) ) | FL_ALIGN_TOP_LEFT;
    X = X+W+3;
    W = wx+this->w()-X;
  } else if ( (a & FL_ALIGN_POSITION_MASK) == FL_ALIGN_RIGHT_BOTTOM ) {
    a = (a &(~FL_ALIGN_POSITION_MASK) ) | FL_ALIGN_BOTTOM_LEFT;
    X = X+W+3;
    W = wx+this->w()-X;
  } else if (a & FL_ALIGN_TOP) {
    a ^= FL_ALIGN_TOP;
    a |= FL_ALIGN_BOTTOM;
    Y = wy;
    H = widget.y()-Y;
  } else if (a & FL_ALIGN_BOTTOM) {
    a ^= FL_ALIGN_BOTTOM;
    a |= FL_ALIGN_TOP;
    Y = Y+H;
    H = wy+h()-Y;
  } else if (a & FL_ALIGN_LEFT) {
    a ^= FL_ALIGN_LEFT;
    a |= FL_ALIGN_RIGHT;
    X = wx;
    W = widget.x()-X-3;
  } else if (a & FL_ALIGN_RIGHT) {
    a ^= FL_ALIGN_RIGHT;
    a |= FL_ALIGN_LEFT;
    X = X+W+3;
    W = wx+this->w()-X;
  }
  widget.draw_label(X,Y,W,H,(Fl_Align)a);
}
