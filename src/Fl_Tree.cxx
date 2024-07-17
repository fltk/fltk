//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <FL/Fl_Tree.H>
#include <FL/Fl_Preferences.H>
#include <FL/fl_string_functions.h>

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
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

// INTERNAL: scroller callback (hor+vert scroll)
static void scroll_cb(Fl_Widget*,void *data) {
  ((Fl_Tree*)data)->redraw();
}

// INTERNAL: Parse elements from 'path' into an array of null terminated strings
//    Handles escape characters, ignores multiple /'s.
//    Path="/aa/bb", returns arr[0]="aa", arr[1]="bb", arr[2]=0.
//    Caller must call free_path(arr).
//
static char **parse_path(const char *path) {
  size_t len = strlen(path);
  char *cp = new char[(len+1)], *word = cp, *s = cp; // freed below or in free_path()
  char **ap = new char*[(len+1)], **arr = ap;        // overallocates arr[]
  while (1) {
    if (*path =='/' || *path == 0) {            // handle path sep or eos
      if (word != s) { *s++ = 0; *arr++= word; word = s; }
      if ( !*path++) break; else continue;      // eos? done, else cont
    } else if ( *path == '\\' ) {               // handle escape
      if ( *(++path) ) { *s++ = *path++; } else continue;
    } else { *s++ = *path++; }                  // handle normal char
  }
  *arr = 0;
  if ( arr == ap ) delete[] cp; // empty arr[]? delete since free_path() can't
  return ap;
}

// INTERNAL: Free an array 'arr' returned by parse_path()
static void free_path(char **arr) {
  if ( arr ) {
    if ( arr[0] ) { delete[] arr[0]; }  // deletes cp in parse_path
    delete[] arr;                       // deletes ptr array
  }
}

#if 0           /* unused code -- STR #3169 */
// INTERNAL: Recursively descend 'item's tree hierarchy
//           accumulating total child 'count'
//
static int find_total_children(Fl_Tree_Item *item, int count=0) {
  count++;
  for ( int t=0; t<item->children(); t++ ) {
    count = find_total_children(item->child(t), count);
  }
  return(count);
}
#endif

/// Constructor.
Fl_Tree::Fl_Tree(int X, int Y, int W, int H, const char *L) : Fl_Group(X,Y,W,H,L) {
  _root = new Fl_Tree_Item(this);
  _root->parent(0);                             // we are root of tree
  _root->label("ROOT");
  _item_focus      = 0;
  _callback_item   = 0;
  _callback_reason = FL_TREE_REASON_NONE;
  _scrollbar_size  = 0;                         // 0: uses Fl::scrollbar_size()

  _lastselect       = 0;

  box(FL_DOWN_BOX);
  color(FL_BACKGROUND2_COLOR, FL_SELECTION_COLOR);
  when(FL_WHEN_CHANGED);
  int scrollsize = _scrollbar_size ? _scrollbar_size : Fl::scrollbar_size();
  _vscroll = new Fl_Scrollbar(X+W-scrollsize,Y,scrollsize,H);
  _vscroll->hide();
  _vscroll->type(FL_VERTICAL);
  _vscroll->step(1);
  _vscroll->callback(scroll_cb, (void*)this);
  _hscroll = new Fl_Scrollbar(X,Y+H-scrollsize,W,scrollsize);
  _hscroll->hide();
  _hscroll->type(FL_HORIZONTAL);
  _hscroll->step(1);
  _hscroll->callback(scroll_cb, (void*)this);
  _tox = _tix = X + Fl::box_dx(box());
  _toy = _tiy = Y + Fl::box_dy(box());
  _tow = _tiw = W - Fl::box_dw(box());
  _toh = _tih = H - Fl::box_dh(box());
  _tree_w = -1;
  _tree_h = -1;
  end();
}

/// Destructor.
Fl_Tree::~Fl_Tree() {
  if ( _root ) { delete _root; _root = 0; }
}

/// Extend the selection between and including \p 'from' and \p 'to'
/// depending on direction \p 'dir', \p 'val', and \p 'visible'.
///
/// Efficient: does not walk entire tree; starts with \p 'from' and stops
/// at \p 'to' while moving in direction \p 'dir'. Dir must be specified though.
///
/// If dir cannot be known in advance, such as during SHIFT-click operations,
/// the method extend_selection(Fl_Tree_Item*,Fl_Tree_Item*,int,bool)
/// should be used.
///
/// Handles calling redraw() if anything changed.
///
/// \param[in] from Starting item
/// \param[in] to   Ending item
/// \param[in] dir  Direction to extend selection (FL_Up or FL_Down)
/// \param[in] val  0=deselect, 1=select, 2=toggle
/// \param[in] visible true=affect only open(), visible items,<br>
///                    false=affect open or closed items (default)
/// \returns The number of items whose selection states were changed, if any.
/// \version 1.3.3
///
int Fl_Tree::extend_selection_dir(Fl_Tree_Item *from, Fl_Tree_Item *to,
                                  int dir, int val, bool visible ) {
  int changed = 0;
  for (Fl_Tree_Item *item=from; item; item = next_item(item, dir, visible) ) {
    switch (val) {
      case 0:
        if ( deselect(item, when()) ) ++changed;
        break;
      case 1:
        if ( select(item, when()) ) ++changed;
        break;
      case 2:
        select_toggle(item, when());
        ++changed;      // toggle always involves a change
        break;
    }
    if ( item==to ) break;
  }
  return(changed);
}

/// Extend a selection between \p 'from' and \p 'to' depending on \p 'visible'.
///
/// Similar to the more efficient
/// extend_selection_dir(Fl_Tree_Item*,Fl_Tree_Item*,int dir,int val,bool vis)
/// method, but direction (up or down) doesn't need to be known.<br>
/// We're less efficient because we search the tree for to/from, then operate
/// on items in between. The more efficient method avoids the "search",
/// but necessitates a direction to be specified to find \p 'to'.<br>
/// Used by SHIFT-click to extend a selection between two items inclusive.<br>
/// Handles calling redraw() if anything changed.
///
/// \param[in] from    Starting item
/// \param[in] to      Ending item
/// \param[in] val     Select or deselect items (0=deselect, 1=select, 2=toggle)
/// \param[in] visible true=affect only open(), visible items,<br>
///                    false=affect open or closed items (default)
/// \returns The number of items whose selection states were changed, if any.
/// \version 1.3.3 ABI feature
int Fl_Tree::extend_selection(Fl_Tree_Item *from, Fl_Tree_Item *to,
                              int val, bool visible) {
  int changed = 0;
  if ( from == to ) {
    if ( visible && !from->is_visible() ) return(0);    // do nothing
    switch (val) {
      case 0:
        if ( deselect(from, when()) ) ++changed;
        break;
      case 1:
        if ( select(from, when()) ) ++changed;
        break;
      case 2:
        select_toggle(from, when());
        ++changed;              // always changed
        break;
    }
    return(changed);
  }
  char on = 0;
  for ( Fl_Tree_Item *item = first(); item; item = item->next_visible(_prefs) ) {
    if ( visible && !item->is_visible() ) continue;
    if ( on || (item == from) || (item == to) ) {
      switch (val) {
        case 0:
          if ( deselect(item, when()) ) ++changed;
          break;
        case 1:
          if ( select(item, when()) ) ++changed;
          break;
        case 2:
          select_toggle(item, when());
          ++changed;    // toggle always involves a change
          break;
      }
      if ( (item == from) || (item == to) ) {
        on ^= 1;
        if ( !on ) break;       // done
      }
    }
  }
  return(changed);
}

enum { PUSHED_NONE=0, PUSHED_OPEN_CLOSE, PUSHED_USER_ICON, PUSHED_LABEL };
/// Standard FLTK event handler for this widget.
/// \todo add Fl_Widget_Tracker (see Fl_Browser_.cxx::handle())
int Fl_Tree::handle(int e) {
  if (e == FL_NO_EVENT) return(0);              // XXX: optimize to prevent slow resizes on large trees!
  int ret = 0;
  char is_shift   = Fl::event_state() & FL_SHIFT   ? 1 : 0;
  char is_ctrl    = Fl::event_state() & FL_CTRL    ? 1 : 0;
  char is_command = Fl::event_state() & FL_COMMAND ? 1 : 0;     // ctrl on win/lin, 'Command' on mac
  // Developer note: Fl_Browser_::handle() used for reference here..
  // #include <FL/names.h>      // for event debugging
  // fprintf(stderr, "DEBUG: %s (%d)\n", fl_eventnames[e], e);

  if (e == FL_ENTER || e == FL_LEAVE) return(1);
  switch (e) {
    case FL_FOCUS: {
      // FLTK tests if we want focus.
      //     If a nav key was used to give us focus, and we've got no saved
      //     focus widget, determine which item gets focus depending on nav key.
      //
      if ( ! _item_focus ) {                            // no focus established yet?
        switch (Fl::event_key()) {                      // determine if focus was navigated..
          case FL_Tab: {                                // received focus via TAB?
            int updown = is_shift ? FL_Up : FL_Down;    // SHIFT-TAB similar to Up, TAB similar to Down
            set_item_focus(next_visible_item(0, updown));
            break;
          }
          case FL_Left:         // received focus via LEFT or UP?
          case FL_Up: {         // XK_ISO_Left_Tab
            set_item_focus(next_visible_item(0, FL_Up));
            break;
          }
          case FL_Right:        // received focus via RIGHT or DOWN?
          case FL_Down:
          default: {
            set_item_focus(next_visible_item(0, FL_Down));
            break;
          }
        }
      }
      if ( visible_focus() ) redraw();  // draw focus change
      return(1);
    }
    case FL_UNFOCUS: {          // FLTK telling us some other widget took focus.
      if ( visible_focus() ) redraw();  // draw focus change
      return(1);
    }
    case FL_KEYBOARD: {         // keyboard shortcut
      // Do shortcuts first or scrollbar will get them...
      if ( (Fl::focus() == this) &&                             // tree has focus?
           _prefs.selectmode() > FL_TREE_SELECT_NONE ) {        // select mode that supports kb events?
        if ( !_item_focus ) {                                   // no current focus item?
          set_item_focus(first_visible_item());                 // use first vis item
          if ( Fl::event_key() == FL_Up ||                      // Up or down?
               Fl::event_key() == FL_Down )                     // ..if so, already did 'motion'
            return(1);                                          // ..so just return.
        }
        if ( _item_focus ) {
          int ekey = Fl::event_key();
          switch (ekey) {
            case FL_Enter:      // ENTER: toggle open/close
            case FL_KP_Enter: {
              open_toggle(_item_focus, when());                 // toggle item in focus
              return(1);                                        // done, we handled key
            }
            case ' ':           // SPACE: change selection state
              switch ( _prefs.selectmode() ) {
                case FL_TREE_SELECT_NONE:
                  break;                                        // ignore, let group have shot at event
                case FL_TREE_SELECT_SINGLE:
                case FL_TREE_SELECT_SINGLE_DRAGGABLE:
                  if ( is_ctrl ) {                              // CTRL-SPACE: (single mode) toggle
                    if ( ! _item_focus->is_selected() ) {
                      select_only(_item_focus, when());
                    } else {
                      deselect_all(0, when());
                    }
                  } else {
                    select_only(_item_focus, when());           // SPACE: (single mode) select only
                  }
                  _lastselect = _item_focus;
                  return(1);                                    // done, we handled key
                case FL_TREE_SELECT_MULTI:
                  if ( is_ctrl ) {
                    select_toggle(_item_focus, when());         // CTRL-SPACE: (multi mode) toggle selection
                  } else {
                    select(_item_focus, when());                // SPACE: (multi-mode) select
                  }
                  _lastselect = _item_focus;
                  return(1);                                    // done, we handled key
              }
              break;
            case FL_Right:      // RIGHT: open children (if any)
            case FL_Left: {     // LEFT: close children (if any)
              if ( _item_focus ) {
                if ( ekey == FL_Right && _item_focus->is_close() ) {
                  open(_item_focus);    // open closed item
                  ret = 1;
                } else if ( ekey == FL_Left && _item_focus->is_open() ) {
                  close(_item_focus);   // close open item
                  ret = 1;
                }
                return(1);
              }
              break;
            }
            case FL_Up:         // UP: next item up, or extend selection up
            case FL_Down: {     // DOWN: next item down, or extend selection down
              set_item_focus(next_visible_item(_item_focus, ekey));     // next item up|dn
              if ( _item_focus ) {                                      // item in focus?
                // Autoscroll
                int itemtop = _item_focus->y();
                int itembot = _item_focus->y()+_item_focus->h();
                if ( itemtop < y() ) { show_item_top(_item_focus); }
                if ( itembot > y()+h() ) { show_item_bottom(_item_focus); }
                // Extend selection
                if ( _prefs.selectmode() == FL_TREE_SELECT_MULTI &&     // multiselect on?
                     is_shift &&                                        // shift key?
                     ! _item_focus->is_selected() ) {                   // not already selected?
                  select(_item_focus, when());                          // extend selection..
                  _lastselect = _item_focus;
                }
                return(1);
              }
              break;
            }
            case 'a':
            case 'A': {
              if ( is_command ) {                                       // ^A (win/linux), Meta-A (mac)
                switch ( _prefs.selectmode() ) {
                  case FL_TREE_SELECT_NONE:
                  case FL_TREE_SELECT_SINGLE:
                  case FL_TREE_SELECT_SINGLE_DRAGGABLE:
                    break;
                  case FL_TREE_SELECT_MULTI:
                    // Do a 'select all'
                    select_all();
                    _lastselect = first_visible_item();
                    take_focus();
                    return(1);
                }
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
    return(1);                  // handled? don't continue below
  }

  // Handle events the child FLTK widgets didn't need

  // fprintf(stderr, "Fl_Tree::handle(): Event was %s (%d)\n", fl_eventnames[e], e); // DEBUGGING
  if ( ! _root ) return(ret);
  static int last_my = 0;
  switch ( e ) {
    case FL_PUSH: {             // clicked on tree
      last_my = Fl::event_y();  // save for dragging direction..
      if (Fl::visible_focus() && handle(FL_FOCUS)) Fl::focus(this);
      Fl_Tree_Item *item = find_clicked(0);
      // Tell FL_DRAG what was pushed
      _lastpushed = item ? item->event_on_collapse_icon(_prefs) ? PUSHED_OPEN_CLOSE  // open/close icon clicked
                         : item->event_on_user_icon(_prefs)     ? PUSHED_USER_ICON   // usericon clicked
                                                                : PUSHED_LABEL       // label clicked
                                                                : PUSHED_NONE;       // none of the above
      if ( !item ) {            // clicked, but not on an item?
        _lastselect = 0;
        switch ( _prefs.selectmode() ) {
          case FL_TREE_SELECT_NONE:
            break;
          case FL_TREE_SELECT_SINGLE:
          case FL_TREE_SELECT_SINGLE_DRAGGABLE:
          case FL_TREE_SELECT_MULTI:
            deselect_all();
            break;
        }
        break;
      }
      set_item_focus(item);                     // becomes new focus widget, calls redraw() if needed
      ret |= 1;                                 // handled
      if ( Fl::event_button() == FL_LEFT_MOUSE ) {
        if ( item->event_on_collapse_icon(_prefs) ) {   // collapse icon clicked?
          open_toggle(item);                            // toggle open (handles redraw)
        } else if ( !item->widget() || !Fl::event_inside(item->widget()) ) {  // not inside widget()
          switch ( _prefs.selectmode() ) {
            case FL_TREE_SELECT_NONE:
              break;
            case FL_TREE_SELECT_SINGLE:
            case FL_TREE_SELECT_SINGLE_DRAGGABLE:
              select_only(item, when());                // select only this item (handles redraw)
              _lastselect = item;
              break;
            case FL_TREE_SELECT_MULTI: {
              if ( is_shift ) {                 // SHIFT+PUSH?
                if ( _lastselect ) {
                  int val = is_ctrl ? 2 : 1;
                  bool visible = true;
                  extend_selection(_lastselect, item, val, visible);
                } else {
                  select(item);                 // add to selection
                }
              } else if ( is_ctrl ) {           // CTRL+PUSH?
                select_toggle(item, when());    // toggle selection state
              } else {
                select_only(item, when());
              }
              _lastselect = item;
              break;
            }
          }
        }
      }
      break;
    }
    case FL_DRAG: {
      // FL_PUSH outside item or on open/close?
      //     Ignore drag to prevent unexpected selections (STR #3527)
      //
      if ( _lastpushed == PUSHED_NONE ||
           _lastpushed == PUSHED_OPEN_CLOSE ) return 0;

      // Do scrolling first

      // Detect up/down dragging
      int my = Fl::event_y();
      int dir = (my>last_my) ? FL_Down : FL_Up;
      last_my = my;

      // Handle autoscrolling
      if ( my < y() ) {                         // Above top?
        dir = FL_Up;                            // ..going up
        int p = vposition()-(y()-my);           // ..position above us
        if ( p < 0 ) p = 0;                     // ..don't go above 0
        vposition(p);                           // ..scroll to new position
      } else if ( my > (y()+h()) ) {            // Below bottom?
        dir = FL_Down;                          // ..going down
        int p = vposition()+(my-y()-h());       // ..position below us
        if ( p > (int)_vscroll->maximum() )     // ..don't go below bottom
          p = (int)_vscroll->maximum();
        vposition(p);                           // ..scroll to new position
      }

      // Now handle the event..
      //    During drag, only interested in left-mouse operations.
      //
      if ( Fl::event_button() != FL_LEFT_MOUSE ) break;
      Fl_Tree_Item *item = find_clicked(1);     // item we're on, vertically
      if ( !item ) break;                       // not near item? ignore drag event
      ret |= 1;                                 // acknowledge event
      if (_prefs.selectmode() != FL_TREE_SELECT_SINGLE_DRAGGABLE)
        set_item_focus(item);                   // becomes new focus item
      if (item==_lastselect) break;             // same item as before? avoid reselect

      // Handle selection behavior
      switch ( _prefs.selectmode() ) {
        case FL_TREE_SELECT_NONE:
          break;                                // no selection changes
        case FL_TREE_SELECT_SINGLE: {
          select_only(item, when());            // select only this item (handles redraw)
          break;
        }
        case FL_TREE_SELECT_SINGLE_DRAGGABLE: {
          item = _lastselect; // Keep the source intact
          redraw();
          break;
        }
        case FL_TREE_SELECT_MULTI: {
          Fl_Tree_Item *from = next_visible_item(_lastselect, dir); // avoid reselecting item
          Fl_Tree_Item *to = item;
          int val = is_ctrl ? 2 : 1;    // toggle_select() or just select()?
          bool visible = true;
          extend_selection_dir(from, to, dir, val, visible);
          break;
        }
      }
      _lastselect = item;                       // save current item for later
      break;
    }
    case FL_RELEASE:
      if (_prefs.selectmode() == FL_TREE_SELECT_SINGLE_DRAGGABLE &&
          Fl::event_button() == FL_LEFT_MOUSE) {
        Fl_Tree_Item *item = find_clicked(1);                // item mouse is over (vertically)
        if (item &&                                          // mouse over valid item?
            _lastselect &&                                   // item being dragged is valid?
            item != _lastselect) {                           // item we're over not same as drag item?
          // Are we dropping above or below the target item?
          const int h = Fl::event_y() - item->y();           // mouse relative to item's top/left
          const int mid = item->h() / 2;                     // middle of item relative to item's top/left
          const bool is_above = h < mid;                     // is mouse above middle of item?
          //printf("Dropping %s target item\n", is_above ? "above" : "below");

          Fl_Tree_Item *target = is_above ? prev(item) : next(item); // target item
          if ( target != _lastselect ) {                     // Don't drop on self
            Fl_Tree_Item *parent = item->parent();           // find parent for item mouse is over
            if ( !parent ) {                                 // no parent (root)?
              // Special case for root; Drop as first child
              _lastselect->move_into(root(), 0);
            } else {
              // Not root..
              if (item->children() && item->is_open() && !is_above) {
                // Special case: Drop onto open folder below midline?
                //    Drop as first child (pos=0)
                //
                _lastselect->move_into(item, 0);             // STR #3432
              } else if (_lastselect->parent() == parent) {
                // If we're moving inside same parent, use the below/above methods
                if (is_above) _lastselect->move_above(item);
                else          _lastselect->move_below(item);
              } else {
                // Moving to different parent..
                int pos = parent->find_child(item);     // find position of item in parent
                if (!is_above) pos++;                   // below? next position down
                _lastselect->move_into(parent, pos);    // move item into parent at position
              }
            }
            redraw();
            do_callback_for_item(_lastselect, FL_TREE_REASON_DRAGGED);
          }
        }
        redraw();
      } // End single-drag check
      ret |= 1;
      break;
  }
  return(ret);
}


/// Recalculate widget dimensions and scrollbar visibility,
/// normally managed automatically.
///
/// Low overhead way to update the tree widget's outer/inner dimensions
/// and re-determine scrollbar visibility based on these changes without
/// recalculating the entire size of the tree data.
///
/// Assumes that either the tree's size in _tree_w/_tree_h are correct
/// so that scrollbar visibility can be calculated easily, or are both
/// zero indicating scrollbar visibility can't be calculated yet.
///
/// This method is called when the widget is resize()ed or if the
/// scrollbar's sizes are changed (affects tree widget's inner dimensions
/// tix/y/w/h), and also used by calc_tree().
/// \version 1.3.3 ABI feature
///
void Fl_Tree::calc_dimensions() {
  // Calc tree outer xywh
  //    Area of the tree widget /outside/ scrollbars
  //
  _tox = x() + Fl::box_dx(box());
  _toy = y() + Fl::box_dy(box());
  _tow = w() - Fl::box_dw(box());
  _toh = h() - Fl::box_dh(box());

  // Scrollbar visiblity + positions
  //    Calc this ONLY if tree_h and tree_w have been calculated.
  //    Zero values for these indicate calc in progress, but not done yet.
  //
  if ( _tree_h >= 0 && _tree_w >= 0 ) {
    int scrollsize = _scrollbar_size ? _scrollbar_size : Fl::scrollbar_size();
    int vshow = _tree_h > _toh ? 1 : 0;
    int hshow = _tree_w > _tow ? 1 : 0;
    // See if one scroller's appearance affects the other's visibility
    if ( hshow && !vshow && (_tree_h > (_toh-scrollsize)) ) vshow = 1;
    if ( vshow && !hshow && (_tree_w > (_tow-scrollsize)) ) hshow = 1;
    // vertical scrollbar visibility
    if ( vshow ) {
      _vscroll->show();
      _vscroll->resize(_tox+_tow-scrollsize, _toy,
                       scrollsize, h()-Fl::box_dh(box()) - (hshow ? scrollsize : 0));
    } else {
      _vscroll->hide();
      _vscroll->value(0);
    }
    // horizontal scrollbar visibility
    if ( hshow ) {
      _hscroll->show();
      _hscroll->resize(_tox, _toy+_toh-scrollsize,
                       _tow - (vshow ? scrollsize : 0), scrollsize);
    } else {
      _hscroll->hide();
      _hscroll->value(0);
    }

    // Calculate inner dimensions
    //    The area the tree occupies inside the scrollbars and margins
    //
    _tix = _tox;
    _tiy = _toy;
    _tiw = _tow - (_vscroll->visible() ? _vscroll->w() : 0);
    _tih = _toh - (_hscroll->visible() ? _hscroll->h() : 0);

    // Scrollbar tab sizes
    _vscroll->slider_size(float(_tih) / float(_tree_h));
    _vscroll->range(0.0, _tree_h - _tih);

    _hscroll->slider_size(float(_tiw) / float(_tree_w));
    _hscroll->range(0.0, _tree_w - _tiw);
  } else {
    // Best we can do without knowing tree_h/tree_w
    _tix = _tox;
    _tiy = _toy;
    _tiw = _tow;
    _tih = _toh;
  }
}

/// Recalculates the tree's sizes and scrollbar visibility,
/// normally managed automatically.
///
/// On return:
/// - _tree_w will be the overall pixel width of the entire viewable tree
/// - _tree_h will be the overall pixel height ""
/// - scrollbar visibility and pan sizes are updated
/// - internal _tix/_tiy/_tiw/_tih dimensions are updated
///
/// _tree_w/_tree_h include the tree's margins (e.g. marginleft()),
/// whether items are open or closed, label contents and font sizes, etc.
///
/// The tree hierarchy's size is managed separately from the widget's
/// size as an optimization; this way resize() on the widget doesn't
/// involve recalculating the tree's hierarchy needlessly, as widget
/// size has no bearing on the tree hierarchy.
///
/// The tree hierarchy's size only changes when items are added/removed,
/// open/closed, label contents or font sizes changed, margins changed, etc.
///
/// This calculation involves walking the *entire* tree from top to bottom,
/// potentially a slow calculation if the tree has many items (potentially
/// hundreds of thousands), and should therefore be called sparingly.
///
/// For this reason, recalc_tree() is used as a way to /schedule/
/// calculation when changes affect the tree hierarchy's size.
///
/// Apps may want to call this method directly if the app makes changes
/// to the tree's geometry, then immediately needs to work with the tree's
/// new dimensions before an actual redraw (and recalc) occurs. (This
/// use by an app should only rarely be needed)
///
void Fl_Tree::calc_tree() {
  // Set tree width and height to zero, and recalc just _tox/_toy/_tow/_toh for now.
  _tree_w = _tree_h = -1;
  calc_dimensions();
  if ( !_root ) return;
  // Walk the tree to determine its width and height.
  // We need this to compute scrollbars..
  // By the end, 'Y' will be the lowest point on the tree
  //
  int X = _tix + _prefs.marginleft() + _hscroll->value();
  int Y = _tiy + _prefs.margintop()  - _vscroll->value();
  int W = _tiw;
  // Adjust root's X/W if connectors off
  if (_prefs.connectorstyle() == FL_TREE_CONNECTOR_NONE) {
    X -= _prefs.openicon_w();
    W += _prefs.openicon_w();
  }
  int xmax = 0, render = 0, ytop = Y;
  fl_font(_prefs.labelfont(), _prefs.labelsize());
  _root->draw(X, Y, W, 0, xmax, 1, render);             // descend into tree without drawing (render=0)
  // Save computed tree width and height
  _tree_w = _prefs.marginleft() + xmax - X;             // include margin in tree's width
  _tree_h = _prefs.margintop()  + Y - ytop;             // include margin in tree's height
  // Calc tree dims again; now that tree_w/tree_h are known, scrollbars are calculated.
  calc_dimensions();
}

void Fl_Tree::resize(int X,int Y,int W, int H) {
  fix_scrollbar_order();
  Fl_Group::resize(X,Y,W,H);
  calc_dimensions();
  init_sizes();
}

/// Standard FLTK draw() method, handles drawing the tree widget.
void Fl_Tree::draw() {
  fix_scrollbar_order();
  // Has tree recalc been scheduled? If so, do it
  if ( _tree_w == -1 ) calc_tree();
  else calc_dimensions();
  // Let group draw box+label but *NOT* children.
  // We handle drawing children ourselves by calling each item's draw()
  {
    // Draw group's bg + label
    if ( damage() & ~FL_DAMAGE_CHILD) { // redraw entire widget?
      Fl_Group::draw_box();
      Fl_Group::draw_label();
    }
    if ( ! _root ) return;
    // These values are changed during drawing
    // By end, 'Y' will be the lowest point on the tree
    int X = _tix + _prefs.marginleft() - _hscroll->value();
    int Y = _tiy + _prefs.margintop()  - _vscroll->value();
    int W = _tiw - X + _tix;
    // Adjust root's X/W if connectors off
    if (_prefs.connectorstyle() == FL_TREE_CONNECTOR_NONE) {
      X -= _prefs.openicon_w();
      W += _prefs.openicon_w();
    }
    // Draw entire tree, starting with root
    fl_push_clip(_tix,_tiy,_tiw,_tih);
    {
      int xmax = 0;
      fl_font(_prefs.labelfont(), _prefs.labelsize());
      _root->draw(X, Y, W,                              // descend into tree here to draw it
                  (Fl::focus()==this)?_item_focus:0,    // show focus item ONLY if Fl_Tree has focus
                  xmax, 1, 1);
    }
    fl_pop_clip();
  }
  // Draw scrollbars last
  draw_child(*_vscroll);
  draw_child(*_hscroll);
  // That little tile between the scrollbars
  if ( _vscroll->visible() && _hscroll->visible() ) {
    fl_color(_vscroll->color());
    fl_rectf(_hscroll->x()+_hscroll->w(),
             _vscroll->y()+_vscroll->h(),
             _vscroll->w(),
             _hscroll->h());
  }

  // Draw dragging line
  if (_prefs.selectmode() == FL_TREE_SELECT_SINGLE_DRAGGABLE &&         // drag mode?
      Fl::pushed() == this) {                                           // item clicked is the one we're drawing?

    Fl_Tree_Item *item = find_clicked(1);                // item we're on, vertically
    if (item &&                                          // we're over a valid item?
        item != _item_focus) {                           // item doesn't have keyboard focus?
      // Are we dropping above or below the target item?
      const int h = Fl::event_y() - item->y();           // mouse relative to item's top/left
      const int mid = item->h() / 2;                     // middle of item relative to item's top/left
      const bool is_above = h < mid;                     // is mouse above middle of item?
      fl_color(FL_BLACK);
      int tgt = item->y() + (is_above ? 0 : item->h());
      fl_line(item->x(), tgt, item->x() + item->w(), tgt);
    }
  }
}

/// Print the tree as 'ascii art' to stdout.
/// Used mainly for debugging.
/// \todo should be const
/// \version 1.3.0
///
void Fl_Tree::show_self() {
  if ( ! _root ) return;
  _root->show_self();
}

/// Set the label for the root item to \p 'new_label'.
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

/// Sets the root item to \p 'newitem'.
///
/// If a root item already exists, clear() is called first to clear it
/// before replacing it with newitem.
///
/// Use this to install a custom item (derived from Fl_Tree_Item) as the root
/// of the tree. This allows the derived class to implement custom drawing
/// by overriding Fl_Tree_Item::draw_item_content().
///
/// \version 1.3.3
///
void Fl_Tree::root(Fl_Tree_Item *newitem) {
  if ( _root ) clear();
  _root = newitem;
}

/** Adds a new item, given a menu style \p 'path'.
 Any parent nodes that don't already exist are created automatically.
 Adds the item based on the value of sortorder().
 If \p 'item' is NULL, a new item is created.

 To specify items or submenus that contain slashes ('/' or '\')
 use an escape character to protect them, e.g.
 \par
 \code
 :
 tree->add("/Holidays/Photos/12\\/25\\/2010");         // Adds item "12/25/2010"
 tree->add("/Pathnames/c:\\\\Program Files\\\\MyApp"); // Adds item "c:\Program Files\MyApp"
 :
 \endcode
 \param[in] path The path to the item, e.g. "Flintstone/Fred".
 \param[in] item The new item to be added.
                 If NULL, a new item is created with
                 a name that is the last element in \p 'path'.
 \returns The new item added, or 0 on error.
 \version 1.3.3
*/
Fl_Tree_Item* Fl_Tree::add(const char *path, Fl_Tree_Item *item) {
  // Tree has no root? make one
  if ( ! _root ) {
    _root = new Fl_Tree_Item(this);
    _root->parent(0);
    _root->label("ROOT");
  }
  // Find parent item via path
  char **arr = parse_path(path);
  item = _root->add(_prefs, arr, item);
  free_path(arr);
  return(item);
}


/// Add a new child item labeled \p 'name' to the specified \p 'parent_item'.
///
/// \param[in] parent_item The parent item the new child item will be added to.
///                        Must not be NULL.
/// \param[in] name The label for the new item
/// \returns The new item added.
/// \version 1.3.0 release
///
Fl_Tree_Item* Fl_Tree::add(Fl_Tree_Item *parent_item, const char *name) {
  return(parent_item->add(_prefs, name));
}

/**
 Inserts a new item \p 'name' above the specified Fl_Tree_Item \p 'above'.
 Example:
 \par
 \code
 :
 tree->add("Aaa/000");       // "000" is index 0 in Aaa's children
 tree->add("Aaa/111");       // "111" is index 1 in Aaa's children
 tree->add("Aaa/222");       // "222" is index 2 in Aaa's children
 ..
 // How to use insert_above() to insert a new item above Aaa/222
 Fl_Tree_Item *item = tree->find_item("Aaa/222");  // get item Aaa/222
 if (item) tree->insert_above(item, "New item");   // insert new item above it
 :
 \endcode

 \param[in] above -- the item above which to insert the new item. Must not be NULL.
 \param[in] name -- the name of the new item
 \returns The new item added, or 0 if 'above' could not be found.
 \see insert()
*/
Fl_Tree_Item* Fl_Tree::insert_above(Fl_Tree_Item *above, const char *name) {
  return(above->insert_above(_prefs, name));
}

/**
 Insert a new item \p 'name' into \p 'item's children at position \p 'pos'.

 If \p pos is out of range the new item is
  - prepended if \p pos \< 0 or
  - appended  if \p pos \> item->children().

 Note: \p pos == children() is not considered out of range: the item is
 appended to the child list.

 Example:
 \par
 \code
 :
 tree->add("Aaa/000");       // "000" is index 0 in Aaa's children
 tree->add("Aaa/111");       // "111" is index 1 in Aaa's children
 tree->add("Aaa/222");       // "222" is index 2 in Aaa's children
 :
 // How to use insert() to insert a new item between Aaa/111 + Aaa/222
 Fl_Tree_Item *item = tree->find_item("Aaa");  // get parent item Aaa
 if (item) tree->insert(item, "New item", 2);  // insert as a child of Aaa at index #2
 :
 \endcode
 \param[in] item The existing item to insert new child into. Must not be NULL.
 \param[in] name The label for the new item
 \param[in] pos The position of the new item in the child list
 \returns The new item added.
 \see insert_above()
*/
Fl_Tree_Item* Fl_Tree::insert(Fl_Tree_Item *item, const char *name, int pos) {
  return(item->insert(_prefs, name, pos));
}

/// Remove the specified \p 'item' from the tree.
/// \p item may not be NULL.
/// If it has children, all those are removed too.
/// If item being removed has focus, no item will have focus.
/// \returns 0 if done, -1 if 'item' not found.
///
int Fl_Tree::remove(Fl_Tree_Item *item) {
  // Item being removed is focus item? zero focus
  if ( item == _item_focus ) _item_focus = 0;
  if ( item == _lastselect ) _lastselect = 0;
  if ( item == _root ) {
    clear();
  } else {
    Fl_Tree_Item *parent = item->parent();      // find item's parent
    if ( ! parent ) return(-1);
    parent->remove_child(item);                 // remove child + children
  }
  return(0);
}

/// Clear the entire tree's children, including the root.
/// The tree will be left completely empty.
///
void Fl_Tree::clear() {
  if ( ! _root ) return;
  _root->clear_children();
  delete _root; _root = 0;
  _item_focus = 0;
  _lastselect = 0;
}

/// Clear all the children for \p 'item'.
/// Item may not be NULL.
///
void Fl_Tree::clear_children(Fl_Tree_Item *item) {
  if ( item->has_children() ) {
    item->clear_children();
    redraw();                           // redraw only if there were children to clear
  }
}

/**
 Find the item, given a menu style path, e.g. "/Parent/Child/item".
 There is both a const and non-const version of this method.
 Const version allows pure const methods to use this method
 to do lookups without causing compiler errors.

 To specify items or submenus that contain slashes ('/' or '\')
 use an escape character to protect them, e.g.
 \par
 \code
 :
 tree->add("/Holidays/Photos/12\\/25\\/2010");         // Adds item "12/25/2010"
 tree->add("/Pathnames/c:\\\\Program Files\\\\MyApp"); // Adds item "c:\Program Files\MyApp"
 :
 \endcode

 \param[in] path -- the tree item's pathname to be found (e.g. "Flintstones/Fred")
 \returns The item, or NULL if not found.
 \see item_pathname()
*/
const Fl_Tree_Item *Fl_Tree::find_item(const char *path) const {
  if ( ! _root ) return(NULL);
  char **arr = parse_path(path);
  const Fl_Tree_Item *item = _root->find_item(arr);
  free_path(arr);
  return(item);
}

/// Non-const version of Fl_Tree::find_item(const char *path) const
Fl_Tree_Item *Fl_Tree::find_item(const char *path) {
  // "Effective C++, 3rd Ed", p.23. Sola fide, Amen.
  return(const_cast<Fl_Tree_Item*>(
         static_cast<const Fl_Tree&>(*this).find_item(path)));
}

// Handle safe 'reverse string concatenation'.
//   In the following we build the pathname from right-to-left,
//   since we start at the child and work our way up to the root.
//
#define SAFE_RCAT(c) { \
  slen += 1; if ( slen >= pathnamelen ) { pathname[0] = '\0'; return(-2); } \
  *s-- = c; \
  }

/// Return \p 'pathname' of size \p 'pathnamelen' for the specified \p 'item'.
///
/// If \p 'item' is NULL, root() is used.<br>
/// The tree's root will be included in the pathname if showroot() is on.<br>
/// Menu items or submenus that contain slashes ('/' or '\') in their names
/// will be escaped with a backslash. This is symmetrical with the add()
/// function which uses the same escape pattern to set names.
///
/// \param[out] pathname The string to use to return the pathname
/// \param[in] pathnamelen The maximum length of the string (including NULL). Must not be zero.
/// \param[in] item The item whose pathname is to be returned.
/// \returns
///     -   0 : OK (\p pathname returns the item's pathname)
///     -  -1 : item not found (pathname="")
///     -  -2 : pathname not large enough (pathname="")
/// \see find_item()
///
int Fl_Tree::item_pathname(char *pathname, int pathnamelen, const Fl_Tree_Item *item) const {
  pathname[0] = '\0';
  item = item ? item : _root;
  if ( !item ) return(-1);
  // Build pathname starting at end
  char *s = (pathname+pathnamelen-1);
  int slen = 0;                 // length of string compiled so far (including NULL)
  SAFE_RCAT('\0');
  while ( item ) {
    if ( item->is_root() && showroot() == 0 ) break;            // don't include root in path if showroot() off
    // Find name of current item
    const char *name = item->label() ? item->label() : "???";   // name for this item
    int len = (int) strlen(name);
    // Add name to end of pathname[]
    for ( --len; len>=0; len-- ) {
      SAFE_RCAT(name[len]);                                     // rcat name of item
      if ( name[len] == '/' || name[len] == '\\' ) {
        SAFE_RCAT('\\');                                        // escape front or back slashes within name
      }
    }
    SAFE_RCAT('/');                                             // rcat leading slash
    item = item->parent();                                      // move up tree (NULL==root)
  }
  if ( *(++s) == '/' ) { ++s; --slen; }                         // leave off leading slash from pathname
  if ( s != pathname ) memmove(pathname, s, slen);              // Shift down right-aligned string
  return(0);
}

/// Find the item that was last clicked on.
/// You should use callback_item() instead, which is fast,
/// and is meant to be used within a callback to determine the item clicked.
///
/// This method walks the entire tree looking for the first item that is
/// under the mouse. (The value of the \p 'yonly' flag affects whether
/// both x and y events are checked, or just y)
///
/// Use this method /only/ if you've subclassed Fl_Tree, and are receiving
/// events before Fl_Tree has been able to process and update callback_item().
///
/// \param[in] yonly -- 0: check both event's X and Y values.
///                  -- 1: only check event's Y value, don't care about X.
/// \returns The item clicked, or NULL if no item was under the current event.
/// \version 1.3.0
/// \version 1.3.3 ABI feature: added yonly parameter
///
const Fl_Tree_Item* Fl_Tree::find_clicked(int yonly) const {
  if ( ! _root ) return(NULL);
  return(_root->find_clicked(_prefs, yonly));
}

/// Non-const version of Fl_Tree::find_clicked(int yonly) const.
Fl_Tree_Item *Fl_Tree::find_clicked(int yonly) {
  // "Effective C++, 3rd Ed", p.23. Sola fide, Amen.
  return(const_cast<Fl_Tree_Item*>(
         static_cast<const Fl_Tree&>(*this).find_clicked(yonly)));
}

/// Set the item that was last clicked.
/// Should only be used by subclasses needing to change this value.
/// Normally Fl_Tree manages this value.
///
/// \deprecated in 1.3.3 ABI -- use callback_item() instead.
///
void Fl_Tree::item_clicked(Fl_Tree_Item* item) {
  _callback_item = item;
}

/// Return the item that was last clicked.
///
/// Valid only from within the callback().
///
/// \returns The item clicked, or 0 if none.
///          0 may also be used to indicate several items were clicked/changed.
/// \deprecated in 1.3.3 ABI -- use callback_item() instead.
///
Fl_Tree_Item* Fl_Tree::item_clicked() {
  return(_callback_item);
}

/**
 Returns next open(), visible item above (\p dir==FL_Up)
 or below (\p dir==FL_Down) the specified \p 'item', or 0 if no more items.

 If \p 'item' is 0, returns last() if \p 'dir' is FL_Up,
 or first() if \p dir is FL_Down.
 \par
 \code
 :
 // Walk down the tree (forwards)
 for ( Fl_Tree_Item *i=tree->first_visible_item(); i; i=tree->next_visible_item(i, FL_Down) )
     printf("Item: %s\n", i->label());

 // Walk up the tree (backwards)
 for ( Fl_Tree_Item *i=tree->last_visible_item(); i; i=tree->next_visible_item(i, FL_Up) )
     printf("Item: %s\n", i->label());
 :
 \endcode
 \param[in] item The item above/below which we'll find the next visible item
 \param[in] dir  The direction to search. Can be FL_Up or FL_Down.
 \returns The item found, or 0 if there's no visible items above/below the specified \p item.
 \version 1.3.3
*/
Fl_Tree_Item *Fl_Tree::next_visible_item(Fl_Tree_Item *item, int dir) {
  return next_item(item, dir, true);
}

/**
 Returns the first item in the tree, or 0 if none.

 Use this to walk the tree in the forward direction, e.g.
 \par
 \code
 :
 for ( Fl_Tree_Item *item = tree->first(); item; item = tree->next(item) )
     printf("Item: %s\n", item->label());
 :
 \endcode
 \returns First item in tree, or 0 if none (tree empty).
 \see first(), next(), last(), prev()
*/
Fl_Tree_Item* Fl_Tree::first() {
  return(_root);                                // first item always root
}

/// Returns the first open(), visible item in the tree, or 0 if none.
/// \deprecated in 1.3.3 ABI -- use first_visible_item() instead.
///
Fl_Tree_Item* Fl_Tree::first_visible() {
  return(first_visible_item());
}

/// Returns the first open(), visible item in the tree, or 0 if none.
/// \returns First visible item in tree, or 0 if none.
/// \see first_visible_item(), last_visible_item(), next_visible_item()
/// \version 1.3.3
///
Fl_Tree_Item* Fl_Tree::first_visible_item() {
  Fl_Tree_Item *i = showroot() ? first() : next(first());
  while ( i ) {
    if ( i->visible() ) return(i);
    i = next(i);
  }
  return(0);
}

/**
 Return the next item after \p 'item', or 0 if no more items.
 Use this code to walk the entire tree:
 \par
 \code
 :
 for ( Fl_Tree_Item *i = tree->first(); i; i = tree->next(i) )
     printf("Item: %s\n", i->label());
 :
 \endcode
 \param[in] item The item to use to find the next item. If NULL, returns 0.
 \returns Next item in tree, or 0 if at last item.
 \see first(), next(), last(), prev()
*/
Fl_Tree_Item *Fl_Tree::next(Fl_Tree_Item *item) {
  if ( ! item ) return(0);
  return(item->next());
}

/**
 Return the previous item before \p 'item', or 0 if no more items.
 This can be used to walk the tree in reverse, e.g.
 \par
 \code
 :
 for ( Fl_Tree_Item *item = tree->first(); item; item = tree->prev(item) )
     printf("Item: %s\n", item->label());
 :
 \endcode
 \param[in] item The item to use to find the previous item. If NULL, returns 0.
 \returns Previous item in tree, or 0 if at first item.
 \see first(), next(), last(), prev()
*/
Fl_Tree_Item *Fl_Tree::prev(Fl_Tree_Item *item) {
  if ( ! item ) return(0);
  return(item->prev());
}

/**
 Returns the last item in the tree.
 This can be used to walk the tree in reverse, e.g.
 \par
 \code
 for ( Fl_Tree_Item *item = tree->last(); item; item = tree->prev() )
     printf("Item: %s\n", item->label());
 \endcode
 \returns Last item in the tree, or 0 if none (tree empty).
 \see first(), next(), last(), prev()
*/
Fl_Tree_Item* Fl_Tree::last() {
  if ( ! _root ) return(0);
  Fl_Tree_Item *item = _root;
  while ( item->has_children() ) {
    item = item->child(item->children()-1);
  }
  return(item);
}

/// Returns the last open(), visible item in the tree.
/// \deprecated in 1.3.3 -- use last_visible_item() instead.
///
Fl_Tree_Item* Fl_Tree::last_visible() {
  return(last_visible_item());
}

/// Returns the last open(), visible item in the tree.
/// \returns Last visible item in the tree, or 0 if none.
/// \see first_visible_item(), last_visible_item(), next_visible_item()
/// \version 1.3.3
///
Fl_Tree_Item* Fl_Tree::last_visible_item() {
  Fl_Tree_Item *item = last();
  while ( item ) {
    if ( item->visible_r() ) {
      if ( item == _root && !showroot() ) {
        return(0);
      } else {
        return(item);
      }
    }
    item = prev(item);
  }
  return(item);
}

/**
 Returns the first selected item in the tree.

 Use this to walk the tree from top to bottom
 looking for all the selected items, e.g.
 \par
 \code
 :
 // Walk tree forward, from top to bottom
 for ( Fl_Tree_Item *i=tree->first_selected_item(); i; i=tree->next_selected_item(i) )
     printf("Selected item: %s\n", i->label());
 :
 \endcode
 \returns The first selected item, or 0 if none.
 \see first_selected_item(), last_selected_item(), next_selected_item()
*/
Fl_Tree_Item *Fl_Tree::first_selected_item() {
  return(next_selected_item(0));
}


/**
 Returns the last selected item in the tree.

 Use this to walk the tree in reverse from bottom to top
 looking for all the selected items, e.g.
 \par
 \code
 :
 // Walk tree in reverse, from bottom to top
 for ( Fl_Tree_Item *i=tree->last_selected_item(); i; i=tree->next_selected_item(i, FL_Up) )
     printf("Selected item: %s\n", i->label());
 :
 \endcode
 \returns The last selected item, or 0 if none.
 \see first_selected_item(), last_selected_item(), next_selected_item()
 \version 1.3.3
*/
Fl_Tree_Item *Fl_Tree::last_selected_item() {
  return(next_selected_item(0, FL_Up));
}

/**
 Returns next item after \p 'item' in direction \p 'dir'
 depending on \p 'visible'.

 Next item will be above (if dir==FL_Up) or below (if dir==FL_Down).
 If \p 'visible' is true, only items whose parents are open() will be returned.
 If \p 'visible' is false, even items whose parents are close()ed will be returned.

 If \p item is 0, the return value will be the result of this truth table:
 <PRE>
                        visible=true           visible=false
                        -------------------    -------------
          dir=FL_Up:    last_visible_item()    last()
        dir=FL_Down:    first_visible_item()   first()
 </PRE>

 Example use:
 \par
 \code
 :
 // Walk down the tree showing open(), visible items
 for ( Fl_Tree_Item *i=tree->first_visible_item(); i; i=tree->next_item(i, FL_Down, true) )
     printf("Item: %s\n", i->label());

 // Walk up the tree showing open(), visible items
 for ( Fl_Tree_Item *i=tree->last_visible_item(); i; i=tree->next_item(i, FL_Up, true) )
     printf("Item: %s\n", i->label());

 // Walk down the tree showing all items (open or closed)
 for ( Fl_Tree_Item *i=tree->first(); i; i=tree->next_item(i, FL_Down, false) )
     printf("Item: %s\n", i->label());

 // Walk up the tree showing all items (open or closed)
 for ( Fl_Tree_Item *i=tree->last(); i; i=tree->next_item(i, FL_Up, false) )
     printf("Item: %s\n", i->label());
 :
 \endcode

 \param[in] item    The item to use to find the next item. If NULL, returns 0.
 \param[in] dir     Can be FL_Up or FL_Down (default=FL_Down or 'next')
 \param[in] visible true=return only open(), visible items,<br>
                    false=return open or closed items (default)

 \returns Next item in tree in the direction and visibility specified,
          or 0 if no more items of specified visibility in that direction.
 \see first(), last(), next(),<BR>
      first_visible_item(), last_visible_item(), next_visible_item(),<BR>
      first_selected_item(), last_selected_item(), next_selected_item()
 \version 1.3.3
*/
Fl_Tree_Item *Fl_Tree::next_item(Fl_Tree_Item *item, int dir, bool visible) {
  if ( ! item ) {                                       // no start item?
    if ( visible ) {
        item = ( dir == FL_Up ) ? last_visible_item() : // wrap to bottom
                                  first_visible_item(); // wrap to top
    } else {
        item = ( dir == FL_Up ) ? last() :              // wrap to bottom
                                  first();              // wrap to top
    }
    if ( ! item ) return(0);
    if ( item->visible_r() ) return(item);              // return first/last visible item
  }
  switch (dir) {
    case FL_Up:
      if ( visible ) return(item->prev_visible(_prefs));
      else           return(item->prev());
    case FL_Down:
      if ( visible ) return(item->next_visible(_prefs));
      else           return(item->next());
  }
  return(0);            // unknown dir
}

/**
 Returns the next selected item above or below \p 'item', depending on \p 'dir'.
 If \p 'item' is 0, search starts at either first() or last(), depending on \p 'dir':
 first() if \p 'dir' is FL_Down (default), last() if \p 'dir' is FL_Up.

 Use this to walk the tree looking for all the selected items, e.g.
 \par
 \code
 :
 // Walk down the tree (forwards)
 for ( Fl_Tree_Item *i=tree->first_selected_item(); i; i=tree->next_selected_item(i, FL_Down) )
     printf("Item: %s\n", i->label());

 // Walk up the tree (backwards)
 for ( Fl_Tree_Item *i=tree->last_selected_item(); i; i=tree->next_selected_item(i, FL_Up) )
     printf("Item: %s\n", i->label());
 :
 \endcode

 \param[in] item The item above or below which we'll find the next selected item.
                 If NULL, first() is used if FL_Down, last() if FL_Up.
                 (default=NULL)
 \param[in] dir  The direction to go.
                 FL_Up for moving up the tree,
                 FL_Down for down the tree (default)
 \returns The next selected item, or 0 if there are no more selected items.
 \see first_selected_item(), last_selected_item(), next_selected_item()
 \version 1.3.3
*/
Fl_Tree_Item *Fl_Tree::next_selected_item(Fl_Tree_Item *item, int dir) {
  switch (dir) {
    case FL_Down:
      if ( ! item ) {
        if ( ! (item = first()) ) return(0);
        if ( item->is_selected() ) return(item);
      }
      while ( (item = item->next()) )
        if ( item->is_selected() )
          return(item);
      return(0);
    case FL_Up:
      if ( ! item ) {
        if ( ! (item = last()) ) return(0);
        if ( item->is_selected() ) return(item);
      }
      while ( (item = item->prev()) )
        if ( item->is_selected() )
          return(item);
      return(0);
  }
  return(0);
}

/**
 Returns the currently selected items as an array of \p 'ret_items'.

 Example:
 \par
 \code
   :
   // Get selected items as an array
   Fl_Tree_Item_Array items;
   tree->get_selected_items(items);
   // Manipulate the returned array
   for ( int t=0; t<items.total(); t++ ) {
       Fl_Tree_Item &item = items[t];
       ..do stuff with each selected item..
   }
   :
 \endcode
 \param[out] ret_items The returned array of selected items.
 \returns The number of items in the returned array.
 \see first_selected_item(), next_selected_item()
 \version 1.3.3 ABI feature
*/
int Fl_Tree::get_selected_items(Fl_Tree_Item_Array &ret_items) {
  ret_items.clear();
  for ( Fl_Tree_Item *i=first_selected_item(); i; i=next_selected_item(i) ) {
    ret_items.add(i);
  }
  return ret_items.total();
}

/// Open the specified \p 'item'.
///
/// This causes the item's children (if any) to be shown.<br>
/// Invokes the callback depending on the value of optional
/// parameter \p 'docallback'.<br>
/// Handles calling redraw() if anything changed.
///
/// The callback can use callback_item() and callback_reason() respectively to determine
/// the item changed and the reason the callback was called.
///
/// \param[in] item -- the item to be opened. Must not be NULL.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - callback() is not invoked
///     -   1 - callback() is invoked if item changed (default),
///             callback_reason() will be FL_TREE_REASON_OPENED
/// \returns
///     -   1 -- item was opened
///     -   0 -- item was already open, no change
///
/// \see open(), close(), is_open(), is_close(), callback_item(), callback_reason()
///
int Fl_Tree::open(Fl_Tree_Item *item, int docallback) {
  if ( item->is_open() ) return(0);
  item->open();         // handles recalc_tree()
  redraw();
  if ( docallback ) {
    do_callback_for_item(item, FL_TREE_REASON_OPENED);
  }
  return(1);
}

/// Opens the item specified by \p 'path'.
///
/// This causes the item's children (if any) to be shown.<br>
/// Invokes the callback depending on the value of optional
/// parameter \p 'docallback'.<br>
/// Handles calling redraw() if anything changed.
///
/// Items or submenus that themselves contain slashes ('/' or '\')
/// should be escaped, e.g. open("Holidays/12\\/25\\/2010").
///
/// The callback can use callback_item() and callback_reason() respectively to determine
/// the item changed and the reason the callback was called.
///
/// \param[in] path -- the tree item's pathname (e.g. "Flintstones/Fred")
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - callback() is not invoked
///     -   1 - callback() is invoked if item changed (default),
///             callback_reason() will be FL_TREE_REASON_OPENED
/// \returns
///     -   1 -- OK: item opened
///     -   0 -- OK: item was already open, no change
///     -  -1 -- ERROR: item was not found
/// \see open(), close(), is_open(), is_close(), callback_item(), callback_reason()
///
int Fl_Tree::open(const char *path, int docallback) {
  Fl_Tree_Item *item = find_item(path);
  if ( ! item ) return(-1);
  return(open(item, docallback));               // handles recalc_tree()
}

/// Toggle the open state of \p 'item'.
///
/// Invokes the callback depending on the value of optional
/// parameter \p 'docallback'.<br>
/// Handles calling redraw() if anything changed.
///
/// The callback can use callback_item() and callback_reason() respectively to determine
/// the item changed and the reason the callback was called.
///
/// \param[in] item -- the item whose open state is to be toggled. Must not be NULL.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - callback() is not invoked
///     -   1 - callback() is invoked (default), callback_reason() will be either
///             FL_TREE_REASON_OPENED or FL_TREE_REASON_CLOSED
///
/// \see open(), close(), is_open(), is_close(), callback_item(), callback_reason()
///
void Fl_Tree::open_toggle(Fl_Tree_Item *item, int docallback) {
  if ( item->is_open() ) {
    close(item, docallback);            // handles recalc_tree()
  } else {
    open(item, docallback);             // handles recalc_tree()
  }
}

/// Closes the specified \p 'item'.
///
/// Invokes the callback depending on the value of optional
/// parameter \p 'docallback'.<br>
/// Handles calling redraw() if anything changed.
///
/// The callback can use callback_item() and callback_reason() respectively to determine
/// the item changed and the reason the callback was called.
///
/// \param[in] item -- the item to be closed. Must not be NULL.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - callback() is not invoked
///     -   1 - callback() is invoked if item changed (default),
///             callback_reason() will be FL_TREE_REASON_CLOSED
/// \returns
///     -   1 -- item was closed
///     -   0 -- item was already closed, no change
/// \see open(), close(), is_open(), is_close(), callback_item(), callback_reason()
///
int Fl_Tree::close(Fl_Tree_Item *item, int docallback) {
  if ( item->is_close() ) return(0);
  item->close();                // handles recalc_tree()
  redraw();
  if ( docallback ) {
    do_callback_for_item(item, FL_TREE_REASON_CLOSED);
  }
  return(1);
}

/// Closes the item specified by \p 'path'.
///
/// Invokes the callback depending on the value of optional
/// parameter \p 'docallback'.<br>
/// Handles calling redraw() if anything changed.
///
/// Items or submenus that themselves contain slashes ('/' or '\')
/// should be escaped, e.g. close("Holidays/12\\/25\\/2010").
///
/// The callback can use callback_item() and callback_reason() respectively to determine
/// the item changed and the reason the callback was called.
///
/// \param[in] path -- the tree item's pathname (e.g. "Flintstones/Fred")
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - callback() is not invoked
///     -   1 - callback() is invoked if item changed (default),
///             callback_reason() will be FL_TREE_REASON_CLOSED
/// \returns
///     -   1 -- OK: item closed
///     -   0 -- OK: item was already closed, no change
///     -  -1 -- ERROR: item was not found
/// \see open(), close(), is_open(), is_close(), callback_item(), callback_reason()
///
int Fl_Tree::close(const char *path, int docallback) {
  Fl_Tree_Item *item = find_item(path);
  if ( ! item ) return(-1);
  return(close(item, docallback));              // handles recalc_tree()
}

/// See if \p 'item' is open.
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

/// See if item specified by \p 'path' is open.
///
/// Items or submenus that themselves contain slashes ('/' or '\')
/// should be escaped, e.g. is_open("Holidays/12\\/25\\/2010").
///
/// Items that are 'open' are themselves not necessarily visible;
/// one of the item's parents might be closed.
///
/// \param[in] path -- the tree item's pathname (e.g. "Flintstones/Fred")
/// \returns
///     -    1 - OK: item is open
///     -    0 - OK: item is closed
///     -   -1 - ERROR: item was not found
/// \see Fl_Tree_Item::visible_r()
///
int Fl_Tree::is_open(const char *path) const {
  const Fl_Tree_Item *item = find_item(path);
  if ( ! item ) return(-1);
  return(item->is_open()?1:0);
}

/// See if the specified \p 'item' is closed.
///
/// \param[in] item -- the item to be tested. Must not be NULL.
/// \returns
///     -   1 : item is closed
///     -   0 : item is open
///
int Fl_Tree::is_close(Fl_Tree_Item *item) const {
  return(item->is_close());
}

/// See if item specified by \p 'path' is closed.
///
/// Items or submenus that themselves contain slashes ('/' or '\')
/// should be escaped, e.g. is_close("Holidays/12\\/25\\/2010").
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

/// Select the specified \p 'item'. Use 'deselect()' to deselect it.
///
/// Invokes the callback depending on the value of optional parameter \p docallback.<br>
/// Handles calling redraw() if anything changed.
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
  int alreadySelected = item->is_selected();
  if ( !alreadySelected ) {
    item->select();
    set_changed();
    if ( docallback ) {
      do_callback_for_item(item, FL_TREE_REASON_SELECTED);
    }
    redraw();
    return(1);
  }
  // NEW
  if ( alreadySelected ) {
    if ( (item_reselect_mode() == FL_TREE_SELECTABLE_ALWAYS) && docallback ) {
      do_callback_for_item(item, FL_TREE_REASON_RESELECTED);
    }
  }
  return(0);
}

/// Select the item specified by \p 'path'.
///
/// Invokes the callback depending on the value of optional
/// parameter \p 'docallback'.<br>
/// Handles calling redraw() if anything changed.
///
/// Items or submenus that themselves contain slashes ('/' or '\')
/// should be escaped, e.g. select("Holidays/12\\/25\\/2010").
///
/// The callback can use callback_item() and callback_reason() respectively to determine
/// the item changed and the reason the callback was called.
///
/// \param[in] path -- the tree item's pathname (e.g. "Flintstones/Fred")
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - the callback() is not invoked
///     -   1 - the callback() is invoked if item changed state (default),
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

/// Toggle the select state of the specified \p 'item'.
///
/// Invokes the callback depending on the value of optional
/// parameter \p 'docallback'.<br>
/// Handles calling redraw() if anything changed.
///
/// The callback can use callback_item() and callback_reason() respectively to determine
/// the item changed and the reason the callback was called.
///
/// \param[in] item -- the item to be selected. Must not be NULL.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - the callback() is not invoked
///     -   1 - the callback() is invoked (default), callback_reason() will be
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

/// Deselect the specified \p item.
///
/// Invokes the callback depending on the value of optional
/// parameter \p 'docallback'.<br>
/// Handles calling redraw() if anything changed.
///
/// The callback can use callback_item() and callback_reason() respectively to determine
/// the item changed and the reason the callback was called.
///
/// \param[in] item -- the item to be deselected. Must not be NULL.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - the callback() is not invoked
///     -   1 - the callback() is invoked if item changed state (default),
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

/// Deselect an item specified by \p 'path'.
///
/// Invokes the callback depending on the value of optional
/// parameter \p 'docallback'.<br>
/// Handles calling redraw() if anything changed.
///
/// Items or submenus that themselves contain slashes ('/' or '\')
/// should be escaped, e.g. deselect("Holidays/12\\/25\\/2010").
///
/// The callback can use callback_item() and callback_reason() respectively to determine
/// the item changed and the reason the callback was called.
///
/// \param[in] path -- the tree item's pathname (e.g. "Flintstones/Fred")
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - the callback() is not invoked
///     -   1 - the callback() is invoked if item changed state (default),
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

/// Deselect \p 'item' and all its children.
///
/// If item is NULL, first() is used.<br>
/// Invokes the callback depending on the value of optional
/// parameter \p 'docallback'.<br>
/// Handles calling redraw() if anything changed.
///
/// The callback can use callback_item() and callback_reason() respectively to determine
/// the item changed and the reason the callback was called.
///
/// \param[in] item The item that will be deselected (along with all its children).
///                 If NULL, first() is used.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - the callback() is not invoked
///     -   1 - the callback() is invoked for each item that changed state (default),
///             callback_reason() will be FL_TREE_REASON_DESELECTED
/// \returns Count of how many items were actually changed to the deselected state.
///
int Fl_Tree::deselect_all(Fl_Tree_Item *item, int docallback) {
  item = item ? item : first();                 // NULL? use first()
  if ( ! item ) return(0);
  int count = 0;
  // Deselect item
  if ( item->is_selected() )
    if ( deselect(item, docallback) )
      ++count;
  // Deselect its children
  for ( int t=0; t<item->children(); t++ ) {
    count += deselect_all(item->child(t), docallback);  // recurse
  }
  return(count);
}

/// Select only the specified item, deselecting all others that might be selected.
///
/// If \p 'selitem' is 0, first() is used.<br>
/// Invokes the callback depending on the value of optional
/// parameter \p 'docallback'.<br>
/// Handles calling redraw() if anything changed.
///
/// The callback can use callback_item() and callback_reason() respectively to determine
/// the item changed and the reason the callback was called.
///
/// \param[in] selitem The item to be selected. If NULL, first() is used.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - the callback() is not invoked
///     -   1 - the callback() is invoked for each item that changed state (default),
///             callback_reason() will be either FL_TREE_REASON_SELECTED or
///             FL_TREE_REASON_DESELECTED
/// \returns The number of items whose selection states were changed, if any.
///
int Fl_Tree::select_only(Fl_Tree_Item *selitem, int docallback) {
  selitem = selitem ? selitem : first();        // NULL? use first()
  if ( ! selitem ) return(0);
  int changed = 0;
  // Deselect everything first.
  //    Prevents callbacks from seeing more than one item selected.
  //
  for ( Fl_Tree_Item *item = first(); item; item = item->next() ) {
    if ( item == selitem ) continue;            // don't do anything to selitem yet..
    if ( item->is_selected() ) {
      deselect(item, docallback);
      ++changed;
    }
  }
  // Should we 'reselect' item if already selected?
  if ( selitem->is_selected() && (item_reselect_mode()==FL_TREE_SELECTABLE_ALWAYS) ) {
    // Selection unchanged, so no ++changed
    select(selitem, docallback);                        // do callback with reason=reselect
  } else if ( !selitem->is_selected() ) {
    // Item was not already selected, select and indicate changed
    select(selitem, docallback);
    ++changed;
  }
  return(changed);
}

/// Select \p 'item' and all its children.
///
/// If item is NULL, first() is used.<br>
/// Invokes the callback depending on the value of optional
/// parameter \p 'docallback'.<br>
/// Handles calling redraw() if anything changed.
///
/// The callback can use callback_item() and callback_reason() respectively to determine
/// the item changed and the reason the callback was called.
///
/// \param[in] item The item that will be selected (along with all its children).
///            If NULL, first() is used.
/// \param[in] docallback -- A flag that determines if the callback() is invoked or not:
///     -   0 - the callback() is not invoked
///     -   1 - the callback() is invoked for each item that changed state (default),
///             callback_reason() will be FL_TREE_REASON_SELECTED
/// \returns Count of how many items were actually changed to the selected state.
///
int Fl_Tree::select_all(Fl_Tree_Item *item, int docallback) {
  item = item ? item : first();                 // NULL? use first()
  if ( ! item ) return(0);
  int count = 0;
  // Select item
  if ( !item->is_selected() )
    if ( select(item, docallback) )
      ++count;
  // Select its children
  for ( int t=0; t<item->children(); t++ ) {
    count += select_all(item->child(t), docallback);    // recurse
  }
  return(count);
}

/// Get the item that currently has keyboard focus.
Fl_Tree_Item* Fl_Tree::get_item_focus() const {
  return(_item_focus);
}

/// Set the item that currently should have keyboard focus.
///
/// Handles calling redraw() to update the focus box (if it is visible).
///
/// \param[in] item The item that should take focus. If NULL, none will have focus.
///
void Fl_Tree::set_item_focus(Fl_Tree_Item *item) {
  if ( _item_focus != item ) {          // changed?
    _item_focus = item;                 // update
    if ( visible_focus() ) redraw();    // redraw to update focus box
  }
}

/// See if the specified \p 'item' is selected.
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

/// See if item specified by \p 'path' is selected.
///
/// Items or submenus that themselves contain slashes ('/' or '\')
/// should be escaped, e.g. is_selected("Holidays/12\\/25\\/2010").
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
/// If the color is 0xffffffff, it is 'transparent'.
Fl_Color Fl_Tree::item_labelbgcolor(void) const {
  return(_prefs.labelbgcolor());
}

/// Set the default label background color used for creating new items.
/// A special case is made for color 0xffffffff (default) which is treated as 'transparent'.
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
  recalc_tree();
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
  recalc_tree();
}

/// Get the amount of white space (in pixels) that should appear
/// below the last visible item when the vertical scroller is scrolled to the bottom.
///
int Fl_Tree::marginbottom() const {
  return(_prefs.marginbottom());
}

/// Sets the amount of white space (in pixels) that should appear
/// below the last visible item when the vertical scroller is scrolled to the bottom.
///
void Fl_Tree::marginbottom(int val) {
  _prefs.marginbottom(val);
  redraw();
  recalc_tree();
}

/// Get the amount of white space (in pixels) that should appear
/// between items in the tree.
///
int Fl_Tree::linespacing() const {
  return(_prefs.linespacing());
}

/// Sets the amount of white space (in pixels) that should appear
/// between items in the tree.
///
void Fl_Tree::linespacing(int val) {
  _prefs.linespacing(val);
  redraw();
  recalc_tree();
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
  recalc_tree();
}

/// Get the amount of white space (in pixels) that should appear
/// to the left of the usericon.
int Fl_Tree::usericonmarginleft() const {
  return(_prefs.usericonmarginleft());
}

/// Set the amount of white space (in pixels) that should appear
/// to the left of the usericon.
void Fl_Tree::usericonmarginleft(int val) {
  _prefs.usericonmarginleft(val);
  redraw();
  recalc_tree();
}

/// Get the amount of white space (in pixels) that should appear
/// to the left of the label text.
int Fl_Tree::labelmarginleft() const {
  return(_prefs.labelmarginleft());
}

/// Set the amount of white space (in pixels) that should appear
/// to the left of the label text.
void Fl_Tree::labelmarginleft(int val) {
  _prefs.labelmarginleft(val);
  redraw();
  recalc_tree();
}

/// Get the amount of white space (in pixels) that should appear
/// to the left of the child fltk widget (if any).
int Fl_Tree::widgetmarginleft() const {
  return(_prefs.widgetmarginleft());
}

/// Set the amount of white space (in pixels) that should appear
/// to the left of the child fltk widget (if any).
void Fl_Tree::widgetmarginleft(int val) {
  _prefs.widgetmarginleft(val);
  redraw();
  recalc_tree();
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
  recalc_tree();
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
  recalc_tree();
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
  recalc_tree();
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
  recalc_tree();
}

/// Returns 1 if the collapse icon is enabled, 0 if not.
/// \see showcollapse(int)
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
  recalc_tree();
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
  recalc_tree();
}

/// Returns the line drawing style for inter-connecting items.
Fl_Tree_Connector Fl_Tree::connectorstyle() const {
  return(_prefs.connectorstyle());
}

/// Sets the line drawing style for inter-connecting items.
///     See ::Fl_Tree_Connector for possible values.
///
void Fl_Tree::connectorstyle(Fl_Tree_Connector val) {
  _prefs.connectorstyle(val);
  redraw();
}

/// Set the default sort order used when items are added to the tree.
///     See ::Fl_Tree_Sort for possible values.
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
/// This is an fltk ::Fl_Boxtype.
/// The default is influenced by FLTK's current Fl::scheme()
///
Fl_Boxtype Fl_Tree::selectbox() const {
  return(_prefs.selectbox());
}

/// Gets the style of box used to draw selected items.
/// This is an fltk ::Fl_Boxtype.
/// The default is influenced by FLTK's current Fl::scheme()
///
void Fl_Tree::selectbox(Fl_Boxtype val) {
  _prefs.selectbox(val);
  redraw();
}

/// Gets the tree's current selection mode.
/// See ::Fl_Tree_Select for possible values.
///
Fl_Tree_Select Fl_Tree::selectmode() const {
  return(_prefs.selectmode());
}

/// Sets the tree's selection mode.
/// See ::Fl_Tree_Select for possible values.
///
void Fl_Tree::selectmode(Fl_Tree_Select val) {
  _prefs.selectmode(val);
}

/// Returns the current item re/selection mode.
/// \version 1.3.1 ABI feature
///
Fl_Tree_Item_Reselect_Mode Fl_Tree::item_reselect_mode() const {
  return(_prefs.item_reselect_mode());
}

/// Sets the item re/selection mode.
/// See ::Fl_Tree_Item_Reselect_Mode for possible values.
/// \version 1.3.1 ABI feature
///
void Fl_Tree::item_reselect_mode(Fl_Tree_Item_Reselect_Mode mode) {
  _prefs.item_reselect_mode(mode);
}

/// Get the 'item draw mode' used for the tree.
/// \version 1.3.1 ABI feature
///
Fl_Tree_Item_Draw_Mode Fl_Tree::item_draw_mode() const {
  return(_prefs.item_draw_mode());
}

/// Set the 'item draw mode' used for the tree to \p 'mode'.
///
/// This affects how items in the tree are drawn,
/// such as when a widget() is defined.
/// See ::Fl_Tree_Item_Draw_Mode for possible values.
/// \version 1.3.1 ABI feature
///
void Fl_Tree::item_draw_mode(Fl_Tree_Item_Draw_Mode mode) {
  _prefs.item_draw_mode(mode);
}

/// Set the 'item draw mode' used for the tree to integer \p 'mode'.
///
/// This affects how items in the tree are drawn,
/// such as when a widget() is defined.
/// See ::Fl_Tree_Item_Draw_Mode for possible values.
/// \version 1.3.1 ABI feature
///
void Fl_Tree::item_draw_mode(int mode) {
  _prefs.item_draw_mode(Fl_Tree_Item_Draw_Mode(mode));
}

/// See if \p 'item' is currently displayed on-screen (visible within the widget).
///
/// This can be used to detect if the item is scrolled off-screen.
/// Checks to see if the item's vertical position is within the top and bottom
/// edges of the display window. This does NOT take into account the hide() / show()
/// or open() / close() status of the item.
///
/// \param[in] item The item to be checked. If NULL, first() is used.
/// \returns 1 if displayed, 0 if scrolled off screen or no items are in tree.
///
int Fl_Tree::displayed(Fl_Tree_Item *item) {
  item = item ? item : first();
  if (!item) return(0);
  return( (item->y() >= y()) && (item->y() <= (y()+h()-item->h())) ? 1 : 0);
}

/// Adjust the vertical scrollbar so that \p 'item' is visible
/// \p 'yoff' pixels from the top of the Fl_Tree widget's display.
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

/// Adjust the vertical scrollbar to show \p 'item' at the top
/// of the display IF it is currently off-screen (for instance show_item_top()).
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

/// Adjust the vertical scrollbar so that \p 'item' is at the top of the display.
///
/// \param[in] item The item to be shown. If NULL, first() is used.
///
void Fl_Tree::show_item_top(Fl_Tree_Item *item) {
  item = item ? item : first();
  if (item) show_item(item, 0);
}

/// Adjust the vertical scrollbar so that \p 'item' is in the middle of the display.
///
/// \param[in] item The item to be shown. If NULL, first() is used.
///
void Fl_Tree::show_item_middle(Fl_Tree_Item *item) {
  item = item ? item : first();
  if (item) show_item(item, (_tih/2)-(item->h()/2));
}

/// Adjust the vertical scrollbar so that \p 'item' is at the bottom of the display.
///
/// \param[in] item The item to be shown. If NULL, first() is used.
///
void Fl_Tree::show_item_bottom(Fl_Tree_Item *item) {
  item = item ? item : first();
  if (item) show_item(item, _tih-item->h());
}

/// Displays \p 'item', scrolling the tree as necessary.
/// \param[in] item The item to be displayed. If NULL, first() is used.
///
void Fl_Tree::display(Fl_Tree_Item *item) {
  item = item ? item : first();
  if (item) show_item_middle(item);
}

/// Returns the vertical scroll position as a pixel offset.
/// The position returned is how many pixels of the tree are scrolled off the top edge
/// of the screen.
/// \see vposition(int), hposition(), hposition(int)
///
int Fl_Tree::vposition() const {
  return((int)_vscroll->value());
}

/// Sets the vertical scroll offset to position \p 'pos'.
/// The position is how many pixels of the tree are scrolled off the top edge
/// of the screen.
/// \param[in] pos The vertical position (in pixels) to scroll the tree to.
/// \see vposition(), hposition(), hposition(int)
///
void Fl_Tree::vposition(int pos) {
  if (pos < 0) pos = 0;
  if (pos > _vscroll->maximum()) pos = (int)_vscroll->maximum();
  if (pos == _vscroll->value()) return;
  _vscroll->value(pos);
  redraw();
}

/// Returns the horizontal scroll position as a pixel offset.
/// The position returned is how many pixels of the tree are scrolled off the left edge
/// of the screen.
/// \see hposition(int), vposition(), vposition(int)
/// \note Must be using FLTK ABI 1.3.3 or higher for this to be effective.
///
int Fl_Tree::hposition() const {
  return((int)_hscroll->value());
}

/// Sets the horizontal scroll offset to position \p 'pos'.
/// The position is how many pixels of the tree are scrolled off the left edge
/// of the screen.
/// \param[in] pos The vertical position (in pixels) to scroll the tree to.
/// \see hposition(), vposition(), vposition(int)
/// \note Must be using FLTK ABI 1.3.3 or higher for this to be effective.
///
void Fl_Tree::hposition(int pos) {
  if (pos < 0) pos = 0;
  if (pos > _hscroll->maximum()) pos = (int)_hscroll->maximum();
  if (pos == _hscroll->value()) return;
  _hscroll->value(pos);
  redraw();
}

/**
 See if widget \p 'w' is one of the Fl_Tree widget's scrollbars.
 Use this to skip over the scrollbars when walking the child() array. Example:
 \par
 \code
 :
 for ( int i=0; i<tree->children(); i++ ) {    // walk children
     Fl_Widget *w = tree->child(i);
     if ( tree->is_scrollbar(w) ) continue;    // skip scrollbars
     ..do work here..
 }
 :
 \endcode
 \param[in] w Widget to test
 \returns 1 if \p w is a scrollbar, 0 if not.
 \todo should be const
*/
int Fl_Tree::is_scrollbar(Fl_Widget *w) {
  return( (w==_vscroll || w==_hscroll) ? 1 : 0 );
}

/// Gets the default size of scrollbars' troughs for this widget
/// in pixels.
///
/// If this value is zero (default), this widget will use the global
/// Fl::scrollbar_size() value as the scrollbar's width.
///
/// \returns Scrollbar size in pixels, or 0 if the global Fl::scrollbar_size() is being used.
/// \see Fl::scrollbar_size(int)
///
int Fl_Tree::scrollbar_size() const {
  return(_scrollbar_size);
}

/// Sets the pixel size of the scrollbars' troughs to \p 'size'
/// for this widget, in pixels.
///
/// Normally you should not need this method, and should use the global
/// Fl::scrollbar_size(int) instead to manage the size of ALL
/// your widgets' scrollbars. This ensures your application
/// has a consistent UI, and is the default behavior. Normally
/// this is what you want.
///
/// Only use this method if you really need to override just THIS
/// instance of the widget's scrollbar size. (This need should be rare.)
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
  if ( _hscroll->h() != scrollsize ) {
    _hscroll->resize(x(), y()+h()-scrollsize, _hscroll->w(), scrollsize);
  }
  // Changing scrollbar size affects _tiw/_tih + may affect scrollbar visibility
  calc_dimensions();
}

/// See if the vertical scrollbar is currently visible.
/// \returns 1 if scrollbar visible, 0 if not.
///
int Fl_Tree::is_vscroll_visible() const {
  return(_vscroll->visible() ? 1 : 0);
}

/// See if the horizontal scrollbar is currently visible.
/// \returns 1 if scrollbar visible, 0 if not.
/// \note Must be using FLTK ABI 1.3.3 or higher for this to be effective.
///
int Fl_Tree::is_hscroll_visible() const {
  return(_hscroll->visible() ? 1 : 0);
}

/// Do the callback for the specified \p 'item' using \p 'reason',
/// setting the callback_item() and callback_reason().
///
void Fl_Tree::do_callback_for_item(Fl_Tree_Item* item, Fl_Tree_Reason reason) {
  callback_reason(reason);
  callback_item(item);
  do_callback((Fl_Widget*)this, user_data(), (Fl_Callback_Reason)reason);
}

/// Sets the item that was changed for this callback.
/// Used internally to pass the item that invoked the callback.
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

/**
 Gets the reason for this callback.

 The callback() can use this value to see why it was called. Example:
 \par
 \code
 :
 void MyTreeCallback(Fl_Widget *w, void *userdata) {
     Fl_Tree *tree = (Fl_Tree*)w;
     Fl_Tree_Item *item = tree->callback_item();    // the item changed (can be NULL if more than one item was changed!)
     switch ( tree->callback_reason() ) {           // reason callback was invoked
         case     FL_TREE_REASON_OPENED: ..item was opened..
         case     FL_TREE_REASON_CLOSED: ..item was closed..
         case   FL_TREE_REASON_SELECTED: ..item was selected..
         case FL_TREE_REASON_RESELECTED: ..item was reselected (double-clicked, etc)..
         case FL_TREE_REASON_DESELECTED: ..item was deselected..
     }
 }
 :
 \endcode
 \see item_reselect_mode() -- enables FL_TREE_REASON_RESELECTED events
*/
Fl_Tree_Reason Fl_Tree::callback_reason() const {
  return(_callback_reason);
}

/**
 Read a preferences database into the tree widget.
 A preferences database is a hierarchical collection of data which can be
 directly loaded into the tree view for inspection.
 \param[in] prefs the Fl_Preferences database
 */
void Fl_Tree::load(Fl_Preferences &prefs) {
  int i, j, n, pn = (int) strlen(prefs.path());
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
    char *key = fl_strdup(prefs.entry(i));
    int kn = (int) strlen(key);
    for (j=0; j<kn; j++) {
      if (key[j]=='/') key[j]='\\';
    }
    char *val;  prefs.get(key, val, "");
    int vn = (int) strlen(val);
    for (j=0; j<vn; j++) {
      if (val[j]=='/') val[j]='\\';
    }
    if (vn<40) {
      size_t sze = pn + strlen(key) + vn;
      p = (char*)malloc(sze+5);
      snprintf(p, sze+5, "%s/%s = %s", path, key, val);
    } else {
      size_t sze = pn + strlen(key) + 40;
      p = (char*)malloc(sze+5);
      snprintf(p, sze+5, "%s/%s = %.40s...", path, key, val);
    }
    add(p[0]=='/'?p+1:p);
    free(p);
    free(val);
    free(key);
  }
}

/// Ensure the scrollbars are the last children
void Fl_Tree::fix_scrollbar_order() {
  Fl_Widget** a = (Fl_Widget**)array();
  if (a[children()-1] != _vscroll) {
    int i,j;
    for (i = j = 0; j < children(); j++) {
      if (a[j] != _vscroll && a[j] != _hscroll ) a[i++] = a[j];
    }
    a[i++] = _hscroll;
    a[i++] = _vscroll;
  }
}

/// Schedule tree to recalc the entire tree size.
/// \note Must be using FLTK ABI 1.3.3 or higher for this to be effective.
///
void Fl_Tree::recalc_tree() {
  _tree_w = _tree_h = -1;
}
