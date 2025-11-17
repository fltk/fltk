//
// Dockable window header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2026 by Bill Spitzak and others.
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
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>

class Fl_Dockable_Group_Driver;

/** A group that can be detached from its parent, dragged with the mouse to an adequate widget, and docked there.
 Like any Fl_Group, an Fl_Dockable_Group can contain any set of Fl_Widget's.
 An  Fl_Dockable_Group object generally becomes detachable and draggable to some destination widget
 for being docked there after member function drag_box(int x, int y, int w, int h) is called once. This adds an Fl_Box
 to the Fl_Dockable_Group that can be called its  <b><u>drag widget</u></b>. Clicking this box and dragging the mouse
 undocks the Fl_Dockable_Group, that is, detaches it from its parent and allows to drag it away in the form of a functional Fl_Window.
 An Fl_Dockable_Group::Docking_Target_Box object replaces the Fl_Dockable_Group in its parent Fl_Group
 unless this group produces another layout.

 A <b><u>docking target widget</u></b> is a widget susceptible to dock an Fl_Dockable_Group object dragged over it.
 FLTK defines one docking target class, Fl_Dockable_Group::Docking_Target_Box, which is an Fl_Box-derived widget
 with this behavior: an object of this class appears as an Fl_Box with label "Dock here" and can dock any Fl_Dockable_Group
 that would be dragged and dropped above it. Any Fl_Dockable_Group docked at the
 Fl_Dockable_Group::Docking_Target_Box can be undocked and dragged away. The docking
 location appears again as an Fl_Box with label "Dock here".
 
 Any other desired behavior for a docking target widget is application-defined: define a custom widget class,
 usually derived from Fl_Group or one of its derived classes; implement its \p handle(int) virtual member function
 as explained in Fl_Dockable_Group::handle_docking_target and its destructor as explained in
 Fl_Dockable_Group::check_origin_target.

 A docking target widget also defines its <b><u>docking area</u></b>:
 when a dragged Fl_Dockable_Group object is positioned above the docking area of a docking target widget and the mouse
 is released, the Fl_Dockable_Group is docked there, that is, stops appearing as a draggable window and,
 if the target widget is an Fl_Group, becomes a child of this group, otherwise a child of the target's
 parent group.

 FLTK sends these docking-related events to an Fl_Dockable_Group object during its lifetime: (1) \ref FL_DOCK_ENTER
 when it's dragged and enters a docking area; (2) \ref FL_DOCK_LEAVE when it's dragged out of a docking area;
 (3) \ref FL_DOCK_RELEASE when it's above a docking area and the mouse is released so it docks there
 (usually it's also resized in this operation); (4) \ref FL_UNDOCK when it's being dragged away from its docked place.
 A user-defined Fl_Dockable_Group-derived class can override its \p handle(int) method and act upon these events.
 
 The Fl_Window containing an undocked, draggable Fl_Dockable_Group object has no titlebar. It can be resized
 moving the mouse to its edges till the cursor changes shape and dragging. Its default callback function has this effect: the
 Fl_Dockable_Group is docked at the docking target from which it was last undocked, if that widget exists
 (a dockable can be created in the undocked state) and has not been deleted. Otherwise, FLTK
 tries to dock it at all docking targets of the GUI. If none exists or accepts to dock it, a dialog window
 allows to choose between deleting the Fl_Dockable_Group and its window or keeping it alive and draggable
 (use the \ref close_dockable member variable to localize this dialog). This default callback can be replaced by
 an application-defined one, but the modified callback needs be assigned to the newly created window
 carrying the dockable each time this dockable receives the FL_UNDOCK event.
 
 Example program \p test/dock_undock.cxx shows the complete implementations of two docking target classes,
 \p tabs_docking_target derived from Fl_Tabs  and \p flex_docking_target derived from Fl_Flex each with a defined behavior.
 It also shows how to derive class Fl_Dockable_Group to catch the FL_DOCK_RELEASE and FL_UNDOCK events;
 a rudimentary compatibility mechanism between Fl_Dockable_Group objects and docking target widgets; and
 how to create an Fl_Dockable_Group object in an undocked, draggable state.
 
 Member functions color_for_states() and label_for_states() control the colors of a  Fl_Dockable_Group's
 drag widget and its label during each of the 3 enum Fl_Dockable_Group::states it can be in. These functions' default argument
 values indicate the colors and labels FLTK uses by default. Calling these member functions allows to
 customize such colors and labels.
 
 \since 1.5.0
  
 \note Platform details:
 <ul><li>Under Wayland, Fl_Dockable_Group undocking, dragging and docking is performed via a Drag-and-Drop operation
 which itself can be implemented in 2 ways.
 <ul><li>Recent versions of the Mutter (Gnome ≥ 48) and KWin (Qt ≥ 6.3) compositors support a Wayland
 protocol called <tt>XDG toplevel drag</tt> that allows to detach the Fl_Dockable_Group, transform it into a
 draggable window, and drop it to a 'docking target'  widget back into the form of an Fl_Dockable_Group.
 The KWin compositor's support of <tt>XDG toplevel drag</tt> forbids an Fl_Dockable_Group to be part of a subwindow.
 <li>When the active Wayland compositor doesn't support that protocol, FLTK computes a small image of the Fl_Dockable_Group and
 uses it as the cursor image of a DnD operation that ends being dropped to a 'docking target' widget or being cancelled.
 </ul>
 <li>Under other platforms, Fl_Dockable_Group undocking transforms the group into an Fl_Window which can be
 dragged around with the mouse and docked to any 'docking target' widget.
 </ul>
*/
class Fl_Dockable_Group : public Fl_Group {
#ifndef FL_DOXYGEN
  friend class Fl_Dockable_Group_Driver;
#endif
private:
  Fl_Dockable_Group_Driver *driver_;
  Fl_Widget *drag_widget_;
  Fl_Color undock_color_, drag_color_, dock_color_;
  const char *undock_label_, *drag_label_, *dock_label_;
  void color_targets_following_dock_();
  void after_release_();
public:
  Fl_Dockable_Group(int x, int y, int w, int h, const char *t=0);
  virtual ~Fl_Dockable_Group();
  /** a function that sets the docking_target's aspect according to whether it would dock the dragged Fl_Dockable_Group
   if the mouse would be released */
  typedef void target_state_f_type(Fl_Widget *docking_target, bool can_dock);
  /** Basic box widget able to dock an Fl_Dockable_Group dragged above it.
   An Fl_Dockable_Group dragged and dropped above this widget replaces it acquiring
   its position and size.
   An Fl_Dockable_Group docked above this widget can be undocked and dragged away.
   It gets replaced by the basic box widget.
   The default box label ("Dock here") can be changed.
   By default, the box switches color from FL_BACKGROUND_COLOR to FL_SELECTION_COLOR
   when it enters the state will-dock-if-mouse-released. This behavior can be changed classwise
   with class function docking_box_state_f().
   */
  class Docking_Target_Box : public Fl_Box {
  private:
    static target_state_f_type *state_f_;
    static void default_state_f_(Fl_Widget *target, bool can_dock);
  public:
    Docking_Target_Box(int x, int y, int w, int h);
    virtual ~Docking_Target_Box();
    static const char *dockable_label;
    int handle(int event) override;
    /** Sets the function called when the object enters or leaves the "will-dock-if-mouse-released" state. */
    inline static void docking_box_state_f(target_state_f_type *f) { state_f_ = f; }
    /** Gets the function called when the object enters or leaves the "will-dock-if-mouse-released" state. */
    inline static target_state_f_type *docking_box_state_f() { return state_f_; }
  };
  /** a function returning true if the mouse is currently inside the docking area of the \p docking_target object. */
  typedef bool inside_f_type(Fl_Widget *docking_target);
  /** a function returning true if \p docking_target recognizes \p dockable as being acceptable for docking. */
  typedef bool acceptable_f_type(Fl_Widget *docking_target, Fl_Dockable_Group *dockable);
  /** a function FLTK calls when a dockable needs be added to a docking_target */
  typedef void dock_payload_f_type(Fl_Widget *docking_target, Fl_Dockable_Group *dockable);
  /** a function called after FLTK has undocked an Fl_Dockable_Group from its docking_target */
  typedef void undock_f_type(Fl_Widget *docking_target, Fl_Dockable_Group *dockable);
  static int handle_docking_target(bool& processed, Fl_Widget *docking_target, int event,
                           inside_f_type inside_f, acceptable_f_type acceptable_f,
                      target_state_f_type target_state_f, dock_payload_f_type dock_payload_f,
                      undock_f_type undock_f);
  Fl_Box *drag_box(int x, int y, int w, int h);
  void drag_widget(Fl_Widget *widget);
  /** Returns the Fl_Dockable_Group's 'drag widget' */
  Fl_Widget *drag_widget() { return drag_widget_; }
  int handle_drag_widget(int event);
  void color_for_states(Fl_Color undock = FL_BACKGROUND_COLOR, Fl_Color drag = FL_BACKGROUND2_COLOR,
                        Fl_Color dock = FL_SELECTION_COLOR);
  void label_for_states(const char *undock = "Undock", const char *drag = "Drag",
                        const char *dock = "Dock");
  /** The 3 states that an Fl_Dockable_Group and its 'drag widget' can adopt during their lifetime. */
  enum states {
    UNDOCK, ///< The Fl_Dockable_Group is ready to be dragged away by a click-drag on its drag widget.
    DRAG, ///<  The Fl_Dockable_Group is being dragged away.
    DOCK, ///< The Fl_Dockable_Group is above a docking target widget where it can dock if the mouse is released.
  };
private:
  enum states state_; // The current state of the Fl_Dockable_Group
  Fl_Widget *active_target_; // Target widget below the active dockable
  static Fl_Dockable_Group *active_dockable_; // The Fl_Dockable_Group that is being dragged
  Fl_Widget *origin_target_; // Target widget where the Fl_Dockable_Group comes from
  void set_state_(enum states s); // Sets the state of the Fl_Dockable_Group
public:
  /** Gets the state of the Fl_Dockable_Group */
  enum states state() {return state_; }
  /** Non-NULL when an Fl_Dockable_Group is currently being dragged. */
  static Fl_Dockable_Group *active_dockable() { return active_dockable_; }
  static bool is_dockable_inside(Fl_Widget *widget);
  static void check_origin_target(Fl_Widget *target);
  static Fl_Dockable_Group *as_dockable(Fl_Widget *widget);
  static const char *close_dockable;
  Fl_Double_Window *make_draggable();
};

#endif // Fl_Dockable_Group_H
