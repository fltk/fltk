//
// Tile widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2017 by Bill Spitzak and others.
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

  For the tiling to work correctly, the children of an Fl_Tile must
  cover the entire area of the widget, but not overlap.
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

/**
  Drags the intersection at (\p oldx,\p oldy) to (\p newx,\p newy).
  This redraws all the necessary children.

  Pass zero as \p oldx or \p oldy to disable drag in that direction.
*/
void Fl_Tile::move_intersection(int oldx, int oldy, int newx, int newy) {
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

/**
  Resizes the Fl_Tile widget and its children.

  Fl_Tile implements its own resize() method. It does not use
  Fl_Group::resize() to resize itself and its children.

  Enlarging works by just moving the lower-right corner and resizing
  the bottom and right border widgets accordingly.

  Shrinking the Fl_Tile works in the opposite way by shrinking
  the bottom and right border widgets, unless they are reduced to zero
  width or height, resp. or to their minimal sizes defined by the
  resizable() widget. In this case other widgets will be shrunk as well.

  See the Fl_Tile class documentation about how the resizable() works.
*/

void Fl_Tile::resize(int X,int Y,int W,int H) {

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

static void set_cursor(Fl_Tile*t, Fl_Cursor c) {
  static Fl_Cursor cursor;
  if (cursor == c || !t->window()) return;
  cursor = c;
#ifdef __sgi
  t->window()->cursor(c,FL_RED,FL_WHITE);
#else
  t->window()->cursor(c);
#endif
}

static Fl_Cursor cursors[4] = {
  FL_CURSOR_DEFAULT,
  FL_CURSOR_WE,
  FL_CURSOR_NS,
  FL_CURSOR_MOVE};

int Fl_Tile::handle(int event) {
  static int sdrag;
  static int sdx, sdy;
  static int sx, sy;
#define DRAGH 1
#define DRAGV 2
#define GRABAREA 4

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
      if (o == resizable()) continue;
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
    set_cursor(this, cursors[sdrag]);
    if (sdrag) return 1;
    return Fl_Group::handle(event);
  }

  case FL_LEAVE:
    set_cursor(this, FL_CURSOR_DEFAULT);
    break;

  case FL_DRAG:
    // This is necessary if CONSOLIDATE_MOTION in Fl_x.cxx is turned off:
    // if (damage()) return 1; // don't fall behind
  case FL_RELEASE: {
    if (!sdrag) break;
    Fl_Widget* r = resizable(); if (!r) r = this;
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
      if (newy < r->y()) newy = r->y();
      else if (newy > r->y()+r->h()) newy = r->y()+r->h();
    } else
      newy = sy;
    move_intersection(sx, sy, newx, newy);
    if (event == FL_DRAG) {
      set_changed();
      do_callback(FL_REASON_DRAGGED);
    } else {
      do_callback(FL_REASON_CHANGED);
    }
    return 1;}

  }

  return Fl_Group::handle(event);
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
: Fl_Group(X,Y,W,H,L)
{
}
