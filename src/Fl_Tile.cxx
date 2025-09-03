//
// Tile widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file. If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

/**
  \class Fl_Tile

  The Fl_Tile class lets you resize its children by dragging
  the border between them.

  \image html Fl_Tile.png
  \image latex Fl_Tile.png "Fl_Tile" width=5cm

  For the tiling to work correctly, the children of an Fl_Tile **must**
  cover the entire area of the widget, but **must not** overlap.
  This means that all children must touch each other at their edges,
  and no gaps can be left inside the Fl_Tile.

  Fl_Tile does not normally draw any graphics of its own.
  The "borders" which can be seen in the snapshot above are actually
  part of the children. Their boxtypes have been set to FL_DOWN_BOX
  creating the impression of "ridges" where the boxes touch. What
  you see are actually two adjacent FL_DOWN_BOX's drawn next to each
  other. All neighboring widgets share the same edge - the widget's
  thick borders make it appear as though the widgets aren't actually
  touching, but they are. If the edges of adjacent widgets do not
  touch, then it will be impossible to drag the corresponding edges.

  \note Fl_Tile works in two distinctive modes. In classic mode, the range of
  motion for edges and intersections is controlled using an invisible child
  that is marked as the `resizable()` widget of the tile group. Classic mode
  is described in detail a few paragraphs down.

  Fl_Tile size_range mode
  -----------------------

  By assigning a default minimum size to all children with
  `Fl_Tile::init_size_range(int default_minimum_width, int default_minimum_height)`
  or by assigning minimal sizes to individual
  children with
  `size_range(Fl_Widget *child, int minimum_width, int minimum_height, int, int)`,
  the tile group is put into size_range operation mode.

  In this mode, the child that is marked resizable() will behave as it would
  in a regular Fl_Group widget.
  When dragging edges or intersections with the mouse, Fl_Tile will ensure that
  none of the children shrinks to a size that is smaller than requested.
  When resizing the Fl_Tile group, size ranges are not enforced by the tile.
  Instead, the size range of the enclosing window should be limited to a
  valid range.

  Tile does not differentiate between visible and invisible children.
  If children are created smaller than their assigned minimum size, dragging
  intersections may cause unexpected jumps in size.
  Zero width or height widget are not harmful, but should be avoided.

  Example for a center document tile and two tool boxes on the left and right
  \code
  Fl_Window win(400, 300, "My App");

  Fl_Tile tile(0, 0, 400, 300);

  Fl_Box left_tool_box(0, 0, 100, 300, "Tools");
  left_tool_box.box(FL_DOWN_BOX);
  tile.size_range(&left_tool_box, 50, 50);

  Fl_Box document(100, 0, 200, 300, "Document");
  document.box(FL_DOWN_BOX);
  tile.size_range(&document, 100, 50);

  Fl_Box right_tool_box(300, 0, 100, 300, "More\nTools");
  right_tool_box.box(FL_DOWN_BOX);
  tile.size_range(&right_tool_box, 50, 50);

  tile.end();
  tile.resizable(document);

  win.end();
  win.resizable(tile);
  win.show(argc,argv);
  win.size_range(200, 50);
  \endcode

  Fl_Tile classic mode
  --------------------

  Fl_Tile allows objects to be resized to zero dimensions.
  To prevent this you can use the resizable() to limit where
  corners can be dragged to. For more information see note below.

  Even though objects can be resized to zero sizes, they must initially
  have non-zero sizes so the Fl_Tile can figure out their layout.
  If desired, call position() after creating the children but before
  displaying the window to set the borders where you want.

  <b>Note on resizable(Fl_Widget &w):</b>
  The "resizable" child widget (which should be invisible) limits where
  the borders can be dragged to. All dragging will be limited inside the
  resizable widget's borders. If you don't set it, it will be possible
  to drag the borders right to the edges of the Fl_Tile widget, and thus
  resize objects on the edges to zero width or height. When the entire
  Fl_Tile widget is resized, the resizable() widget will keep its border
  distance to all borders the same (this is normal resize behavior), so
  that you can effectively set a border width that will never change.
  To ensure correct event delivery to all child widgets the resizable()
  widget must be the first child of the Fl_Tile widget group. Otherwise
  some events (e.g. FL_MOVE and FL_ENTER) might be consumed by the resizable()
  widget so that they are lost for widgets covered (overlapped) by the
  resizable() widget.

  \note
  You can still resize widgets \b inside the resizable() to zero width and/or
  height, i.e. box \b 2b above to zero width and box \b 3a to zero height.

  \see void Fl_Group::resizable(Fl_Widget &w)

  Example for resizable with 20 pixel border distance:
  \code
    int dx = 20, dy = dx;
    Fl_Tile tile(50,50,300,300);
    // create resizable() box first
    Fl_Box r(tile.x()+dx,tile.y()+dy,tile.w()-2*dx,tile.h()-2*dy);
    tile.resizable(r);
    // ... create widgets inside tile (see test/tile.cxx) ...
    tile.end();
  \endcode

  See also the complete example program in test/tile.cxx.
*/

#include <FL/Fl_Tile.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Rect.H>
#include <stdlib.h>

static Fl_Cursor Fl_Tile_cursors[4] = {
  FL_CURSOR_DEFAULT,  // 0 normal
  FL_CURSOR_WE,       // 1 drag horizontally
  FL_CURSOR_NS,       // 2 drag vertically
  FL_CURSOR_MOVE      // 3 move intersection
};

static int fl_min(int a, int b) { return a<b ? a : b; }
static int fl_max(int a, int b) { return a>b ? a : b; }

/**
 Request for children to change their layout.

 drag_intersection requests that all children with the left edge at old_l to
 shrink to new_l towards the right side of the tile. If the child can not shrink
 by that amount, it will ask all other children that touch its right side to
 shrink by the remainder (recursion). new_l will return the the maximum possible
 value while maintaining minimum width for all children involved.

 request_shrink_r asks children to shrink toward the left, so that their right
 edge is as close as possible to new_r. request_shrink_t and request_shrink_b
 provide the same functionality for vertical resizing.

 \param[in] old_l shrink all children with this current left edge
 \param[inout] new_l try to shrink to this coordinate, return the maximum
    possible shrinkage
 \param[inout] final_size if not NULL, write the new position and size of
    all affected children into this list of Fl_Rect
 */
void Fl_Tile::request_shrink_l(int old_l, int &new_l, Fl_Rect *final_size) {
  Fl_Rect *p = bounds();
  int min_l = new_l;
  for (int i=0; i<children(); i++) {
    Fl_Rect *ri = p+i+2;
    if (ri->x() == old_l) {
      if (ri->w() == 0) {
        if (final_size) final_size[i].x(new_l);
      } else {
        // first, try to shrink
        int min_w = size_range_[i].minw;
        int may_l = fl_min(new_l, ri->r()-min_w); // enforce minimum width
        int new_l_right = ri->r();
        // if that is not sufficient, try to move
        if (may_l < new_l) {
          int missing_w = new_l - may_l;
          new_l_right = ri->r() + missing_w;
          request_shrink_l(ri->r(), new_l_right, NULL);
          new_l_right = fl_min(new_l_right, p->r());
          if (final_size) {
            request_shrink_l(ri->r(), new_l_right, final_size);
            request_grow_r(ri->r(), new_l_right, final_size);
          }
          min_l = fl_min(min_l, new_l_right - min_w);
        }
        if (final_size) {
          final_size[i].x(new_l);
          final_size[i].w(new_l_right-new_l);
        }
      }
    }
  }
  new_l = min_l;
}

/**
 Request for children to change their layout.

 \see Fl_Tile::request_shrink_l(int old_l, int &new_l, Fl_Rect *final_size)

 \param[in] old_r shrink all children with this current right edge toward
    the left edge of this tile
 \param[inout] new_r try to shrink to this coordinate, return the maximum
    possible shrinkage
 \param[inout] final_size if not NULL, write the new position and size of
    all affected children into this list of Fl_Rect
 */
void Fl_Tile::request_shrink_r(int old_r, int &new_r, Fl_Rect *final_size) {
  Fl_Rect *p = bounds();
  int min_r = new_r;
  for (int i=0; i<children(); i++) {
    Fl_Rect *ri = p+i+2;
    if (ri->r() == old_r) {
      if (ri->w() == 0) {
        if (final_size) final_size[i].x(new_r);
      } else {
        // first, try to shrink
        int min_w = size_range_[i].minw;
        int may_r = fl_max(new_r, ri->x()+min_w); // enforce minimum width
        int new_r_left = ri->x();
        // if that is not sufficient, try to move
        if (may_r > new_r) {
          int missing_w = may_r - new_r;
          new_r_left = ri->x() - missing_w;
          request_shrink_r(ri->x(), new_r_left, NULL);
          new_r_left = fl_max(new_r_left, p->x());
          if (final_size) {
            request_shrink_r(ri->x(), new_r_left, final_size);
            request_grow_l(ri->x(), new_r_left, final_size);
          }
          min_r = fl_max(min_r, new_r_left + min_w);
        }
        if (final_size) {
          final_size[i].x(new_r_left);
          final_size[i].w(new_r-new_r_left);
        }
      }
    }
  }
  new_r = min_r;
}

/**
 Request for children to change their layout.

 \see Fl_Tile::request_shrink_l(int old_l, int &new_l, Fl_Rect *final_size)

 \param[in] old_t shrink all children with this current top edge toward
    the bottom edge of this tile
 \param[inout] new_t try to shrink to this coordinate, return the maximum
    possible shrinkage
 \param[inout] final_size if not NULL, write the new position and size of
    all affected children into this list of Fl_Rect
 */
void Fl_Tile::request_shrink_t(int old_t, int &new_t, Fl_Rect *final_size) {
  Fl_Rect *p = bounds();
  int min_y = new_t;
  for (int i=0; i<children(); i++) {
    Fl_Rect *ri = p+i+2;
    if (ri->y() == old_t) {
      if (ri->h() == 0) {
        if (final_size) final_size[i].y(new_t);
      } else {
        // first, try to shrink
        int min_h = size_range_[i].minh;
        int may_y = fl_min(new_t, ri->b()-min_h); // enforce minimum height
        int new_y_below = ri->b();
        // if that is not sufficient, try to move
        if (may_y < new_t) {
          int missing_h = new_t - may_y;
          new_y_below = ri->b() + missing_h;
          request_shrink_t(ri->b(), new_y_below, NULL);
          new_y_below = fl_min(new_y_below, p->b());
          if (final_size) {
            request_shrink_t(ri->b(), new_y_below, final_size);
            request_grow_b(ri->b(), new_y_below, final_size);
          }
          min_y = fl_min(min_y, new_y_below - min_h);
        }
        if (final_size) {
          final_size[i].y(new_t);
          final_size[i].h(new_y_below-new_t);
        }
      }
    }
  }
  new_t = min_y;
}

/**
 Request for children to change their layout.

 \see Fl_Tile::request_shrink_l(int old_l, int &new_l, Fl_Rect *final_size)

 \param[in] old_b shrink all children with this current bottoom edge toward
    the top edge of this tile
 \param[inout] new_b try to shrink to this coordinate, return the maximum
    possible shrinkage
 \param[inout] final_size if not NULL, write the new position and size of
    all affected children into this list of Fl_Rect
 */
void Fl_Tile::request_shrink_b(int old_b, int &new_b, Fl_Rect *final_size) {
  Fl_Rect *p = bounds();
  int min_b = new_b;
  for (int i=0; i<children(); i++) {
    Fl_Rect *ri = p+i+2;
    if (ri->b() == old_b) {
      if (ri->h() == 0) {
        if (final_size) final_size[i].y(new_b);
      } else {
        // first, try to shrink
        int min_h = size_range_[i].minh;
        int may_b = fl_max(new_b, ri->y()+min_h); // enforce minimum height
        int new_b_above = ri->y();
        // if that is not sufficient, try to move
        if (may_b > new_b) {
          int missing_h = may_b - new_b;
          new_b_above = ri->y() - missing_h;
          request_shrink_b(ri->y(), new_b_above, NULL);
          new_b_above = fl_max(new_b_above, p->y());
          if (final_size) {
            request_shrink_b(ri->y(), new_b_above, final_size);
            request_grow_t(ri->y(), new_b_above, final_size);
          }
          min_b = fl_max(min_b, new_b_above + min_h);
        }
        if (final_size) {
          final_size[i].y(new_b_above);
          final_size[i].h(new_b-new_b_above);
        }
      }
    }
  }
  new_b = min_b;
}

/**
 Request for children to change their layout.

 \param[in] old_l grow all children with this current left edge toward
    the left edge of this tile
 \param[inout] new_l try to grow to this coordinate, return the maximum
    possible growth (currently maxw is ignored, so we always grow to new_l)
 \param[inout] final_size write the new position and size of all affected
    children into this list of Fl_Rect
 */
void Fl_Tile::request_grow_l(int old_l, int &new_l, Fl_Rect *final_size) {
  Fl_Rect *p = bounds();
  for (int i=0; i<children(); i++) {
    Fl_Rect *ri = p+i+2;
    if (ri->x() == old_l) {
      final_size[i].w(final_size[i].r() - new_l);
      final_size[i].x(new_l);
    }
  }
}

/**
 Request for children to change their layout.

 \param[in] old_r grow all children with this current right edge toward
    the right edge of this tile
 \param[inout] new_r try to grow to this coordinate, return the maximum
    possible growth (currently maxw is ignored, so we always grow to new_r)
 \param[inout] final_size write the new position and size of all affected
    children into this list of Fl_Rect
 */
void Fl_Tile::request_grow_r(int old_r, int &new_r, Fl_Rect *final_size) {
  Fl_Rect *p = bounds();
  for (int i=0; i<children(); i++) {
    Fl_Rect *ri = p+i+2;
    if (ri->r() == old_r) {
      final_size[i].r(new_r);
    }
  }
}

/**
 Request for children to change their layout.

 \param[in] old_t grow all children with this current top edge toward
    the top edge of this tile
 \param[inout] new_t try to grow to this coordinate, return the maximum
    possible growth (currently maxh is ignored, so we always grow to new_t)
 \param[inout] final_size write the new position and size of all affected
    children into this list of Fl_Rect
 */
void Fl_Tile::request_grow_t(int old_t, int &new_t, Fl_Rect *final_size) {
  Fl_Rect *p = bounds();
  for (int i=0; i<children(); i++) {
    Fl_Rect *ri = p+i+2;
    if (ri->y() == old_t) {
      final_size[i].h(final_size[i].b() - new_t);
      final_size[i].y(new_t);
    }
  }
}

/**
 Request for children to change their layout.

 \param[in] old_b grow all children with this current bottom edge toward
    the bottom edge of this tile
 \param[inout] new_b try to grow to this coordinate, return the maximum
    possible growth (currently maxh is ignored, so we always grow to new_b)
 \param[inout] final_size write the new position and size of all affected
    children into this list of Fl_Rect
 */
void Fl_Tile::request_grow_b(int old_b, int &new_b, Fl_Rect *final_size) {
  Fl_Rect *p = bounds();
  for (int i=0; i<children(); i++) {
    Fl_Rect *ri = p+i+2;
    if (ri->b() == old_b) {
      final_size[i].b(new_b);
    }
  }
}

/**
 Drags the intersection at (\p oldx,\p oldy) to (\p newx,\p newy).

 This redraws all the necessary children.

 If no size ranges are set, the new intersection position is limited to the
 size of the tile group. The resizable() option is not taken into account here.

 If size ranges are set, the actual new position of the intersection will
 depend on the size range of every individual child. No child will be smaller
 than their minw and minh. After the new position is found, move_intersection()
 will call init_sizes(). The resizable() range is ignored.

 \param[in] oldx, oldy move the intersection at this coordinate, pass zero to
    disable drag in that direction.
 \param[in] newx, newy move the intersection as close to this new coordinate
    as possible
*/
void Fl_Tile::move_intersection(int oldx, int oldy, int newx, int newy) {
  if (size_range_) {
    drag_intersection(oldx, oldy, newx, newy);
    init_sizes();
  } else {
    Fl_Widget*const* a = array();
    Fl_Rect *p = bounds();
    p += 2; // skip group & resizable's saved size
    for (int i=children(); i--; p++) {
      Fl_Widget* o = *a++;
      if (o == resizable()) continue;
      int X = o->x();
      int R = X+o->w();
      if (oldx) {
        int t = p->x();
        if (t == oldx || (t>oldx && X<newx) || (t<oldx && X>newx) ) X = newx;
        t = p->r();
        if (t == oldx || (t>oldx && R<newx) || (t<oldx && R>newx) ) R = newx;
      }
      int Y = o->y();
      int B = Y+o->h();
      if (oldy) {
        int t = p->y();
        if (t == oldy || (t>oldy && Y<newy) || (t<oldy && Y>newy) ) Y = newy;
        t = p->b();
        if (t == oldy || (t>oldy && B<newy) || (t<oldy && B>newy) ) B = newy;
      }
      o->damage_resize(X,Y,R-X,B-Y);
    }
  }
}


/**
 Drags the intersection at (\p oldx,\p oldy) to (\p newx,\p newy).

 \see Fl_Tile::move_intersection(int oldx, int oldy, int newx, int newy) , but
 this method does not call init_sizes() and is used for interactive children
 layout using the mouse.

 \param[in] oldx, oldy move the intersection at this coordinate, pass zero to
 disable drag in that direction.
 \param[in] newx, newy move the intersection as close to this new coordinate
 as possible
 */
void Fl_Tile::drag_intersection(int oldx, int oldy, int newx, int newy) {
  if (size_range_) {
    int i;
    Fl_Rect *p = bounds();
    Fl_Rect *final_size = new Fl_Rect[children()];
    for (i = 0; i < children(); i++) {
      final_size[i] = p[i+2];
    }
    // apply changes in x and y intersection recursively to all children
    if ((oldy != 0) && (oldy != newy)) {
      if (newy <= oldy) { // user moves intersection up
        int new_y = newy;
        request_shrink_b(oldy, new_y, NULL); // updates new_y to the topmost possible
        request_shrink_b(oldy, new_y, final_size); // now update all children touching above
        request_grow_t(oldy, new_y, final_size); // now update all children touching below
      }
      if (newy > oldy) { // user moves intersection down
        int new_y = newy;
        request_shrink_t(oldy, new_y, NULL); // updates new_b to the bottommost possible
        request_shrink_t(oldy, new_y, final_size); // now update all children touching below
        request_grow_b(oldy, new_y, final_size); // now update all children touching above
      }
    }
    if ((oldx != 0) && (oldx != newx)) {
      if (newx <= oldx) { // user moves intersection left
        int new_x = newx;
        request_shrink_r(oldx, new_x, NULL); // updates new_x to the leftmost possible
        request_shrink_r(oldx, new_x, final_size); // now shring all children touching to the left
        request_grow_l(oldx, new_x, final_size); // now grow all children touching to the right
      }
      if (newx > oldx) { // user moves intersection right
        int new_x = newx;
        request_shrink_l(oldx, new_x, NULL); // updates new_x to the rightmost possible
        request_shrink_l(oldx, new_x, final_size); // now shrink all children touching on the right
        request_grow_r(oldx, new_x, final_size); // now grow all children touching on th eleft
      }
    }
    // resize all children that have changed in size
    for (i = 0; i < children(); i++) {
      Fl_Rect &r = final_size[i];
      child(i)->damage_resize(r.x(), r.y(), r.w(), r.h());
    }
    delete[] final_size;
  } else {
    move_intersection(oldx, oldy, newx, newy);
  }
}

/**
  Resizes the Fl_Tile widget and its children.

  Fl_Tile implements its own resize() method. It does not use
  Fl_Group::resize() to resize itself and its children.

  In size_range mode, the child marked resizable() is resized first. Only if
  its minimum size is reached, other widgets in the tile will resize too.

  In classic mode or when no resizable child is set, enlarging works by moving
  the lower-right corner and resizing the bottom and right border
  widgets accordingly.

  Shrinking the Fl_Tile works in the opposite way by shrinking
  the bottom and right border widgets, unless they are reduced to zero
  width or height, resp. or to their minimal sizes defined by the
  resizable() widget. In this case other widgets will be shrunk as well.

  See the Fl_Tile class documentation about how the resizable() works.
*/
void Fl_Tile::resize(int X,int Y,int W,int H) {

  if (size_range_) {
    // -- handle size_range style resizing
    int dx = X - x();
    int dy = Y - y();
    int dw = w() - W;
    int dh = h() - H;
    // -- if the size does not change, Group resize will suffice
    if ((dw==0) && (dh==0)) {
      Fl_Group::resize(X, Y, W, H);
      init_sizes();
      redraw();
      return;
    }
    // -- if the position changes, move all widgets first
    if ((dx!=0) || (dy!=0)) {
      for (int i = 0; i < children(); i++) {
        Fl_Widget *c = child(i);
        c->position(c->x()+dx, c->y()+dy);
      }
    }
    // check the current bounding box of all children
    init_sizes();
    Fl_Rect *p = bounds();
    int bbr = X, bbb = Y;
    for (int i = 0; i < children(); i++) {
      // find the current bounding box
      bbr = fl_max(bbr, p[i+2].r());
      bbb = fl_max(bbb, p[i+2].b());
    }
    // fix the dw to the maximum difference possible
    int r2 = X+W;
    request_shrink_r(bbr, r2, NULL);
    dw = bbr - r2;
    // fix the dh to the maximum difference possible
    int b2 = Y+H;
    request_shrink_b(bbb, b2, NULL);
    dh = bbb - b2;
    // perform the actual resize within a safe range
    if ((dw!=0) || (dh!=0)) {
      Fl_Widget *r = resizable();
      // Find the target right and bottom position of the resizable child.
      int trr = 0, trb = 0;
      if (r) {
        trr = r->x() + r->w() - dw;
        trb = r->y() + r->h() - dh;
      }
      // Grow width and/or height of tile and adjust all children as needed.
      if ((dw < 0) && (dh < 0)) {
        move_intersection(bbr, bbb, bbr-dw, bbb-dh);
      } else if (dw < 0) {
        move_intersection(bbr, bbb, bbr-dw, bbb);
      } else if (dh < 0) {
        move_intersection(bbr, bbb, bbr, bbb-dh);
      }
      // Fix the resizable child, trying to keep its size plus all other
      // widgets within their limits.
      if (r) {
        int rr = r->x() + r->w(), rb = r->y() + r->h();
        move_intersection(rr, rb, trr, trb);
      }
      // Shrink width and/or height of tile and adjust all children as needed.
      if ((dw > 0) && (dh > 0)) {
        move_intersection(bbr, bbb, bbr-dw, bbb-dh);
      } else if (dw > 0) {
        move_intersection(bbr, bbb, bbr-dw, bbb);
      } else if (dh > 0) {
        move_intersection(bbr, bbb, bbr, bbb-dh);
      }
      init_sizes();
    }
    // resize the tile itself
    if (Fl_Window::is_a_rescale())
      Fl_Group::resize(X, Y, W, H);
    else
      Fl_Widget::resize(X, Y, W, H);
    return;
  }

  // remember how much to move the child widgets:
  int dx = X-x();
  int dy = Y-y();
  int dw = W-w();
  int dh = H-h();
  Fl_Rect *p = bounds();
  // resize this (skip the Fl_Group resize):
  Fl_Widget::resize(X,Y,W,H);

  // find bottom-right corner of resizable:
  int OR = p[1].r();            // old right border
  int NR = X+W-(p[0].r()-OR);   // new right border
  int OB = p[1].b();            // old bottom border
  int NB = Y+H-(p[0].b()-OB);   // new bottom border

  // move everything to be on correct side of new resizable:
  Fl_Widget*const* a = array();
  p += 2;
  for (int i=children(); i--; p++) {
    Fl_Widget* o = *a++;
    int xx = o->x()+dx;
    int R = xx+o->w();
    if (p->x() >= OR) xx += dw; else if (xx > NR) xx = NR;
    if (p->r() >= OR) R += dw; else if (R > NR) R = NR;
    int yy = o->y()+dy;
    int B = yy+o->h();
    if (p->y() >= OB) yy += dh; else if (yy > NB) yy = NB;
    if (p->b() >= OB) B += dh; else if (B > NB) B = NB;
    o->resize(xx,yy,R-xx,B-yy);
    // do *not* call o->redraw() here! If you do, and the tile is inside a
    // scroll, it'll set the damage areas wrong for all children!
  }
}

/**
  Set one of four cursors used for dragging etc…

  Fl_Tile uses an array of four cursors that are set depending on user actions:
    - 0: normal cursor
    - 1: horizontal dragging
    - 2: vertical dragging
    - 3: dragging an intersection

  This method sets the window cursor for the given index \p n.
*/
void Fl_Tile::set_cursor(int n) {
  if (n < 0 || n > 3) n = 0; // check array index
  if (cursor_ == n) return;  // nothing to do
  cursor_ = n;               // store the cursor index
  if (window())
    window()->cursor(cursor(n));
}

#define DRAGH 1
#define DRAGV 2
#define GRABAREA 4

int Fl_Tile::handle(int event) {
  static int sdrag;
  static int sdx, sdy;
  static int sx, sy;

  int mx = Fl::event_x();
  int my = Fl::event_y();

  switch (event) {

  case FL_MOVE:
  case FL_ENTER:
  case FL_PUSH:
    // don't potentially change the mouse cursor if inactive:
    if (!active()) break; // will cascade inherited handle()
    {
    int mindx = 100;
    int mindy = 100;
    int oldx = 0;
    int oldy = 0;
    Fl_Widget*const* a = array();
    Fl_Rect *q = bounds();
    Fl_Rect *p = q+2;
    for (int i=children(); i--; p++) {
      Fl_Widget* o = *a++;
      if (!size_range_ && o == resizable()) continue;
      if (p->r() < q->r() && o->y()<=my+GRABAREA && o->y()+o->h()>=my-GRABAREA) {
        int t = mx - (o->x()+o->w());
        if (abs(t) < mindx) {
          sdx = t;
          mindx = abs(t);
          oldx = p->r();
        }
      }
      if (p->b() < q->b() && o->x()<=mx+GRABAREA && o->x()+o->w()>=mx-GRABAREA) {
        int t = my - (o->y()+o->h());
        if (abs(t) < mindy) {
          sdy = t;
          mindy = abs(t);
          oldy = p->b();
        }
      }
    }
    sdrag = 0; sx = sy = 0;
    if (mindx <= GRABAREA) {sdrag = DRAGH; sx = oldx;}
    if (mindy <= GRABAREA) {sdrag |= DRAGV; sy = oldy;}
    set_cursor(sdrag);
    if (sdrag) return 1;
    return Fl_Group::handle(event);
  }

  case FL_LEAVE:
    set_cursor(0); // set default cursor
    break;

  case FL_DRAG:
    // This is necessary if CONSOLIDATE_MOTION in Fl_x.cxx is turned off:
    // if (damage()) return 1; // don't fall behind
  case FL_RELEASE: {
    if (!sdrag) break;
    Fl_Widget* r = resizable();
    if (size_range_ || !r) r = this;
    int newx;
    if (sdrag&DRAGH) {
      newx = Fl::event_x()-sdx;
      if (newx < r->x()) newx = r->x();
      else if (newx > r->x()+r->w()) newx = r->x()+r->w();
    } else
      newx = sx;
    int newy;
    if (sdrag&DRAGV) {
      newy = Fl::event_y()-sdy;
      if (newy < r->y())
        newy = r->y();
      else if (newy > r->y()+r->h())
        newy = r->y()+r->h();
    } else {
      newy = sy;
    }
    if (event == FL_DRAG) {
      drag_intersection(sx, sy, newx, newy);
      set_changed();
      do_callback(FL_REASON_DRAGGED);
    } else {
      move_intersection(sx, sy, newx, newy);
      do_callback(FL_REASON_CHANGED);
    }
    return 1;
    } // case FL_RELEASE

  } // switch()

  return Fl_Group::handle(event);
}

/**
 Insert a new entry in the size range list.
 */
int Fl_Tile::on_insert(Fl_Widget *candidate, int index) {
  if (size_range_) {
    if (index >= size_range_capacity_) {
      size_range_capacity_ = (index+8) & ~7; // allocate in steps of 8
      size_range_ = (Size_Range*)::realloc(size_range_, sizeof(Size_Range)*size_range_capacity_);
    }
    if (index < size_range_size_)
      memmove(size_range_+index+1, size_range_+index, sizeof(Size_Range)*(size_range_size_-index));
    size_range_size_++;
    size_range_[index].minw = default_min_w_;
    size_range_[index].minh = default_min_h_;
    size_range_[index].maxw = 0x7FFFFFFF;
    size_range_[index].maxh = 0x7FFFFFFF; /* INTMAX_MAX */
  }
  return index;
}

/**
 Move the entry in the size range list.
 */
int Fl_Tile::on_move(int oldIndex, int newIndex) {
  if (size_range_) {
    int delta = newIndex - oldIndex;
    if (delta) {
      Size_Range size_bak = size_range_[oldIndex];
      if (delta > 0)
        memmove(size_range_+oldIndex, size_range_+oldIndex+1, sizeof(Size_Range)*delta);
      else
        memmove(size_range_+newIndex+1, size_range_+newIndex, sizeof(Size_Range)*-delta);
      size_range_[newIndex] = size_bak;
    }
  }
  return newIndex;
}

/**
 Remove the entry from the size range list.
 */
void Fl_Tile::on_remove(int index) {
  if (size_range_) {
    int num_trailing = size_range_size_-index-1;
    if ((index >= 0) && (index < size_range_size_) && (num_trailing > 0))
      memmove(size_range_+index, size_range_+index+1, sizeof(Size_Range)*num_trailing);
    size_range_size_--;
  }
}

/**
 Set the allowed size range for the child at the given index.

 Fl_Tile currently supports only the minimal width and height setting.

 \param[in] index set the range for the child at this index
 \param[in] minw, minh minimum width and height for that child
 \param[in] maxw, maxh maximum size, defaults to infinite, currently ignored
 */
void Fl_Tile::size_range(int index, int minw, int minh, int maxw, int maxh) {
  if (!size_range_)
    init_size_range();
  if ((index >= 0) && (index < children())) {
    size_range_[index].minw = minw;
    size_range_[index].minh = minh;
    size_range_[index].maxw = maxw;
    size_range_[index].maxh = maxh;
  }
}

/**
 Set the allowed size range for the give child widget.

 Fl_Tile currently supports only the minimal width and height setting.

 \param[in] w set the range for this child widget
 \param[in] minw, minh minimum width and height for that child
 \param[in] maxw, maxh maximum size, defaults to infinite, currently ignored
 */
void Fl_Tile::size_range(Fl_Widget *w , int minw, int minh, int maxw, int maxh) {
  int index = find(w);
  if ((index >= 0) && (index < children()))
    size_range(index, minw, minh, maxw, maxh);
}

/**
 Initialize the size range mode of Fl_Tile and set the default minimum width and height.

 The default minimum width and height is the size of the mouse pointer grab
 area at about 4 pixel units.

 \param[in] default_min_w, default_min_h default size range for widgets that don't
    have an individual range assigned
 */
void Fl_Tile::init_size_range(int default_min_w, int default_min_h) {
  if (default_min_w > 0) default_min_w_ = default_min_w;
  if (default_min_h > 0) default_min_h_ = default_min_h;
  if (!size_range_) {
    size_range_size_ = children();
    size_range_capacity_ = (size_range_size_+8) & ~7; // allocate in steps of 8
    size_range_ = (Size_Range*)::realloc(size_range_, sizeof(Size_Range)*size_range_capacity_);
    for (int i=0; i<size_range_size_; i++) {
      size_range_[i].minw = default_min_w_;
      size_range_[i].minh = default_min_h_;
      size_range_[i].maxw = 0x7FFFFFFF;
      size_range_[i].maxh = 0x7FFFFFFF; /* INTMAX_MAX */
    }
  }
}

/**
  Creates a new Fl_Tile widget using the given position, size,
  and label string. The default boxtype is FL_NO_BOX.

  The destructor <I>also deletes all the children</I>. This allows a
  whole tree to be deleted at once, without having to keep a pointer to
  all the children in the user code. A kludge has been done so the
  Fl_Tile and all of its children can be automatic (local)
  variables, but you must declare the Fl_Tile <I>first</I>, so
  that it is destroyed last.

  \see class Fl_Group
*/
Fl_Tile::Fl_Tile(int X,int Y,int W,int H,const char*L)
: Fl_Group(X,Y,W,H,L),
  cursor_(0),
  cursors_(Fl_Tile_cursors),
  size_range_(NULL),
  size_range_size_(0),
  size_range_capacity_(0),
  default_min_w_(GRABAREA),
  default_min_h_(GRABAREA)
{
}

/**
 Destructor.
 */
Fl_Tile::~Fl_Tile() {
  if (size_range_)
    ::free(size_range_);
}
