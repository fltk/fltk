//
// Dockable window header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2025 by Bill Spitzak and others.
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

/* \file
   Fl_Dockable_Group widget . */

#ifndef Fl_Dockable_Group_H
#define Fl_Dockable_Group_H

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>

class Fl_Dockable_Group_Driver;

/** A group that can be detached from its parent, dragged with the mouse to some destination, and docked there.
 Like any Fl_Group, an Fl_Dockable_Group can contain any set of Fl_Widget's.
 An  Fl_Dockable_Group object becomes detachable and draggable to some destination
 after member function command_box(int x, int y, int w, int h) is called once. This adds an Fl_Box
 to the Fl_Dockable_Group that can be called its  'command box'. Clicking that 'command box' and dragging the mouse
 allows to detach and drag the Fl_Dockable_Group away.
 Class function Fl_Dockable_Group::target_box(int x, int y, int w, int h, Fl_Group *g) allows to define 'target boxes', that is,
 destination boxes to which Fl_Dockable_Group objects can be dragged and docked. That function
 is to be called once for each location in the GUI where Fl_Dockable_Group objects are susceptible
 to be docked. When an Fl_Dockable_Group is dragged away from one location and docked at a new location,
 it gets resized to the location and size of its receiving 'target box'. Another effect of this move operation is that
 the 'target box' moves from the Fl_Dockable_Group's destination location to its location of origin.
 Therefore, after an Fl_Dockable_Group gets dragged from location A to location B in the GUI, that group
 can be dragged from B back to A because the 'target box'  that was initially at B has been moved to A.
 
 While an Fl_Dockable_Group is being dragged away, hitting Escape before it's docked somewhere sends it back to its
 starting location.
 
 An Fl_Dockable_Group can be made the child of an Fl_Tabs object. This results in a tab that can be detached
 from its parent Fl_Tabs and dragged with the mouse via its 'command box' to any destination 'target box'.
 
 A 'target box' can be made the child of an Fl_Tabs object. This results in a tab that, when the Fl_Tabs' value, can
 be replaced by a dragged Fl_Dockable_Group.
  
 Member functions color_for_states() and label_for_states() control the colors of a  Fl_Dockable_Group's
 command box and its label during each of the 4 enum Fl_Dockable_Group::states it can be in. These functions' default argument
 values indicate the colors and labels FLTK uses by default. Calling these member functions allows to
 customize such colors and labels.
 
 A usage example is found at \p test/dock_undock.cxx.
 
 \note Platform details:
 <ul><li>Under Wayland, Fl_Dockable_Group dragging and docking is performed via a Drag-and-Drop operation which itself
 can be implemented in 2 ways.
 <ul><li>Recent versions of the Mutter (Gnome ≥ 48) and KWin (Qt ≥ 6.3) compositors support a Wayland
 protocol called <tt>XDG toplevel drag</tt> that allows to detach the Fl_Dockable_Group, transform it into a
 draggable window, and drop it to a 'target box' back into the form of an Fl_Dockable_Group.
 The KWin compositor's support of <tt>XDG toplevel drag</tt> forbids an Fl_Dockable_Group to be part of a subwindow.
 <li>When the active Wayland compositor doesn't support that protocol, FLTK computes a small image of the Fl_Dockable_Group and
 uses it as the cursor image of a DnD operation that ends being dropped to a 'target box' or being cancelled.
 </ul>
 <li>Under other platforms, Fl_Dockable_Group dragging and docking begins by transforming the group into an Fl_Window which is
 dragged around with the mouse and can be docked to any 'target box', or sent back to its initial location with Escape.
 </ul>
*/
class Fl_Dockable_Group : public Fl_Group {
#ifndef FL_DOXYGEN
  friend class Fl_Dockable_Group_Driver;
#endif
private:
  Fl_Dockable_Group_Driver *driver_;
  Fl_Box *command_box_;
  Fl_Group *parent_when_docked_;
  int x_when_docked_, y_when_docked_;
  Fl_Widget *next_in_group_when_docked_;
  void color_targets_following_dock_();
  Fl_Widget *place_holder_while_dragged_;
public:
  Fl_Dockable_Group(int x, int y, int w, int h, const char *t=0);
  Fl_Box *command_box(int x, int y, int w, int h);
  /** Returns the Fl_Dockable_Group's 'command box' */
  Fl_Box *command_box() { return command_box_; }
  static Fl_Box *target_box(int x, int y, int w, int h, Fl_Group *g);
  static Fl_Box *target_box(int n);
  static int target_count();
  static int target_index();
  void color_for_states(Fl_Color undock = FL_BACKGROUND_COLOR, Fl_Color drag = FL_BACKGROUND2_COLOR,
                        Fl_Color dock = FL_SELECTION_COLOR, Fl_Color dragged = FL_SELECTION_COLOR);
  void label_for_states(const char *undock = "Undock", const char *drag = "Drag",
                        const char *dock = "Dock", const char *dragged = "Dragged");
  /**
   The 4 states that an Fl_Dockable_Group and its 'command box' can adopt during their lifetime.
   */
  enum states {
    UNDOCK, ///< The Fl_Dockable_Group is ready to be dragged away by a click-drag on its command box.
    DRAG, ///<  The Fl_Dockable_Group is being dragged away.
    DOCK, ///< The Fl_Dockable_Group is above a target box where it will dock if the mouse is released.
    DRAGGED ///<  Used only when the Fl_Dockable_Group is represented by an icon and is being dragged.
  } state; ///< The current state of the Fl_Dockable_Group.
  static void label_for_target_boxes(const char *l);
  /** Non-NULL when an Fl_Dockable_Group is currently active.
   Under Wayland, this means a Drag-and-Drop of a window (or an image of a window) is active.
   Under other platforms, this means an Fl_Dockable_Group is currently above a target box
   and can be docked there.
   */
  static Fl_Dockable_Group *active_dockable;
};

#endif // Fl_Dockable_Group_H
