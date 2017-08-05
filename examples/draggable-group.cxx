//
// "$Id$"
//
// Demonstrate deriving a class with draggable children.
//
// Copyright 2017 by Bill Spitzak and others.
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

// K Dmitrij <kdiman@...> wrote on Wed, 26 Jul 2017 19:54:12 +0000 in
// fltk.general, thread "[fltk.general] Draggable Group for examples FLTK":
//
// "Here is class for drag widgets and/or their children inside its parent
//  widget. You can include it into examples of FLTK (and to adapt/modify
//  if needs)".
//
// Thanks to Dmitrij for this contribution.

// Use `fltk-config --compile draggable-group.cxx' to build this example.


#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>

#include <stdio.h>

/** \class DraggableGroup
    \brief Class with draggable children, derived from Fl_Group.

  Use this class if you want the user to be able to drag the children
  of a group inside the borders of the group. DraggableGroup widgets
  can be nested, but only direct children of the DraggableGroup widget
  can be dragged.

  Design decisions:
    - Widgets can be dragged with the right mouse button.
      This was chosen to avoid conflicts with other mouse button
      events such as clicking a button.
    - The dragged widget is raised to the top within its group for better
      visual feedback while dragging by making it the last child of the group.
    - The order of widgets is restored when the mouse button is released.
      This can result in hiding the widget behind another widget, but this
      was chosen to keep the order given by the programmer. If this
      is not desired it can be changed easily.
*/
class DraggableGroup : public Fl_Group {

protected:
  int xoff, yoff;         // start offsets while dragging
  int drag_index;         // index of dragged child
  Fl_Widget *drag_widget; // dragged child widget

public:
  DraggableGroup(int X, int Y, int W, int H, const char *L = 0)
    : Fl_Group(X, Y, W, H, L)
    , xoff(0)
    , yoff(0)
    , drag_index(0)
    , drag_widget(0) {
    box(FL_UP_BOX);
  }

  /** Raise the dragged widget to the top (last child) of its group.

    This ensures that the widget is always visible while it is dragged.
    The original index is saved in 'drag_index' and restored when the
    mouse button is released.

    \internal
    Since we allow only direct children of the DraggableGroup widget
    to be dragged the argument 'q' is always a child of 'this'.
    Note: add(q) first removes the widget from its parent group ('this')
    and then adds it again as its last child.
    This is the same as insert(*q, children()).
  */
  void top_level(Fl_Widget *q) {
    drag_index = find(q); // save the widget's current index
    add(q);               // raise to top (make it the last child)
  }

  /** Handle FL_PUSH, FL_DRAG, and FL_RELEASE events.

    All other events are handled in Fl_Group::handle().
    Dragged widgets are limited inside the borders of their parent group.
  */
  virtual int handle(int e) {

    switch (e) {

      case FL_PUSH: {
	if (Fl::event_button() == FL_RIGHT_MOUSE) {
	  if (Fl::belowmouse()->parent() == this) {
	    drag_widget = Fl::belowmouse();

	    // save pointer offsets relative to drag_widget's x/y position
	    xoff = Fl::event_x() - drag_widget->x();
	    yoff = Fl::event_y() - drag_widget->y();

	    top_level(drag_widget); // raise to top for visible feedback
	    redraw();
	    return 1;
	  }
	}
	break;
      }

      case FL_DRAG: {
	if (!drag_widget)
	  break;

	int nX = Fl::event_x() - xoff; // new x coordinate
	int nY = Fl::event_y() - yoff; // new y coordinate

	int bbx = Fl::box_dx(box()); // left and right border width
	int bby = Fl::box_dy(box()); // top and bottom border width

	// keep the widget inside its parent's borders

	if (nX < x() + bbx) {
	  nX = x() + bbx;
	} else if (nX + drag_widget->w() > x() + w() - bbx) {
	  nX = x() + w() - drag_widget->w() - bbx;
	}

	if (nY < y() + bby) {
	  nY = y() + bby;
	} else if (nY + drag_widget->h() > y() + h() - bby) {
	  nY = y() + h() - drag_widget->h() - bby;
	}

	drag_widget->position(nX, nY); // set the new position
	redraw();
	return 1;
      }

      case FL_RELEASE: {
	if (drag_widget && Fl::event_button() == FL_RIGHT_MOUSE) {

	  // Optional: restore the original widget order in the group.
	  // Remove the next statement (or comment it out) if not desired.
	  insert(*drag_widget, drag_index);

	  init_sizes(); // save widget positions for later resizing
	  drag_widget = 0;
	  redraw();
	  if (parent())
	    parent()->redraw();
	  return 1;
	}
	break;
      }

      default:
	break;
    } // switch(e)

    return Fl_Group::handle(e);
  }
};

// clear status message box after timeout
void clear_status(void *v) {
  Fl_Box *status = (Fl_Box *)v;
  status->label("");
  status->redraw();
}

// button callback: display status message for two seconds
void button_cb(Fl_Widget *w, void *v) {
  static char buf[128];
  sprintf(buf, "button_cb: '%s'.\n", w->label());
  Fl_Box *status = (Fl_Box *)v;
  status->label(buf);
  status->redraw();
  Fl::remove_timeout(clear_status); // remove any pending timeout
  Fl::add_timeout(2.0, clear_status, v);
}

int main() {

  Fl_Double_Window win(500, 500, "Drag children within their parent group");

  DraggableGroup area(0, 0, 500, 400, "Use the right mouse button (MB3)\nto drag objects");
  area.align(FL_ALIGN_INSIDE);

  // draggable group inside draggable area
  DraggableGroup dobj(5, 5, 140, 140, "DraggableGroup");
  dobj.align(FL_ALIGN_INSIDE);
  Fl_Box b1(25, 25, 20, 20);
  b1.color(FL_RED);
  b1.box(FL_FLAT_BOX);
  Fl_Box b2(105, 105, 20, 20);
  b2.color(FL_GREEN);
  b2.box(FL_FLAT_BOX);
  dobj.end();

  // regular group inside draggable area
  Fl_Group dobj2(5, 280, 110, 110, "Fl_Group");
  dobj2.box(FL_DOWN_BOX);
  dobj2.align(FL_ALIGN_INSIDE);
  Fl_Box b3(15, 290, 20, 20);
  b3.color(FL_BLUE);
  b3.box(FL_FLAT_BOX);
  Fl_Box b4(85, 360, 20, 20);
  b4.color(FL_YELLOW);
  b4.box(FL_FLAT_BOX);
  dobj2.end();

  // draggable group inside draggable area
  DraggableGroup dobj3(245, 5, 150, 150, "DraggableGroup");
  dobj3.align(FL_ALIGN_INSIDE);

  // nested draggable group
  DraggableGroup dobj4(250, 10, 50, 50);
  Fl_Box b5(255, 15, 15, 15);
  b5.color(FL_BLACK);
  b5.box(FL_FLAT_BOX);
  Fl_Box b6(275, 30, 15, 15);
  b6.color(FL_WHITE);
  b6.box(FL_FLAT_BOX);
  dobj4.end();

  dobj3.end();

  Fl_Button button1(200, 350, 180, 40, "Fl_Button inside DraggableGroup");
  button1.align(FL_ALIGN_INSIDE | FL_ALIGN_WRAP);

  area.end();

  Fl_Button button2(200, 410, 180, 40, "Fl_Button outside DraggableGroup");
  button2.align(FL_ALIGN_INSIDE | FL_ALIGN_WRAP);

  Fl_Box *status = new Fl_Box(0, 460, win.w(), 40, "Messages ...");
  status->box(FL_DOWN_BOX);

  button1.callback(button_cb, status);
  button2.callback(button_cb, status);

  win.end();
  win.resizable(win);
  win.show();

  return Fl::run();
}

//
// End of "$Id$".
//
