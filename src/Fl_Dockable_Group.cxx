//
// Dockable window code for the Fast Light Tool Kit (FLTK).
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
/** \file
 Fl_Dockable_Group implementation.
 */

#include <FL/platform.H> // for FLTK_USE_WAYLAND
#include <FL/Fl_Dockable_Group.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#include "Fl_Dockable_Group_Driver.H"
#include <set>

Fl_Dockable_Group *Fl_Dockable_Group::active_dockable_ = NULL;

static std::set<Fl_Widget*> dockables_;// Set of all Fl_Dockable_Group objects in app


/** Used in dialog while attempting to close dragged Fl_Dockable_Group object. */
const char *Fl_Dockable_Group::close_dockable = "Close this dockable window%s";


#if !defined(FLTK_USE_WAYLAND) && !defined(FL_DOXYGEN)

Fl_Dockable_Group_Driver *Fl_Dockable_Group_Driver::newDockableGroupDriver(Fl_Dockable_Group *dock) {
  return new Fl_Dockable_Group_Driver(dock);
}

#endif


/**
 Constructor.
 \param x,y,w,h,t used as with Fl_Group
 */
Fl_Dockable_Group::Fl_Dockable_Group(int x, int y, int w, int h, const char *t) : Fl_Group(x,y,w,h,t) {
  drag_widget_ = NULL;
  state_ = UNDOCK;
  driver_ = Fl_Dockable_Group_Driver::newDockableGroupDriver(this);
  color_for_states();
  label_for_states();
  active_target_ = NULL;
  origin_target_ = NULL;
  box(FL_FLAT_BOX);
  dockables_.insert(this);
}


Fl_Dockable_Group::~Fl_Dockable_Group() {
  if (active_dockable_ == this) active_dockable_ = NULL;
  dockables_.erase(this);
}


/** Returns an Fl_Dockable_Group pointer if \p widget is an Fl_Dockable_Group and NULL otherwise.  */
Fl_Dockable_Group *Fl_Dockable_Group::as_dockable(Fl_Widget *widget) {
  return (dockables_.count(widget) > 0 ? (Fl_Dockable_Group*)widget : NULL);
}


/**
 Adds an Fl_Box to the dockable group and makes it the group's 'drag widget'.
 The 'drag widget' allows to drag away an Fl_Dockable_Group by clicking that box-widget
 and dragging the mouse. The position and size of the 'drag widget' inside the Fl_Dockable_Group
 can be freely set by the caller. Its changing colors and labels are best set via member functions color_for_states()
 and label_for_states(). Its boxtype, labelsize and labelfont can be be freely changed.
 \param x,y,w,h Position and size of the 'drag widget'.
 \return The created 'drag widget'.
 */
Fl_Box *Fl_Dockable_Group::drag_box(int x, int y, int w, int h) {
  Fl_Dockable_Group_Driver::drag_box_class *drag =
    new Fl_Dockable_Group_Driver::drag_box_class(FL_DOWN_BOX, x, y, w, h, NULL);
  this->add(drag);
  drag_widget_ = drag;
  set_state_(state());
  return drag;
}

/**
 Makes any Fl_Widget the drag widget of this dockable group.
 In the case that an Fl_Box widget is not enough to control the dragging
 of a dockable group, this member function allows to use any child widget of the
 dockable group as its drag widget. That widget's \p handle(int) method should be
 implemented as follows, where \p drag_widget_class is the widget's class name and
 \p parent_class is the widget's parent class name:
 \code
 int drag_widget_class::handle(int event) override {
   Fl_Dockable_Group *dock = (Fl_Dockable_Group*)parent();
   int r = dock->handle_drag_widget(event);
   return (r ? r : parent_class::handle(event));
 }
 \endcode
 \param widget NULL, or a child of the dockable group. Use NULL to temporarily
 deactivate the possibility to undock, drag, or dock the dockable group.
 */
void Fl_Dockable_Group::drag_widget(Fl_Widget *widget) {
  if (drag_widget_ != widget) {
    drag_widget_ = widget;
    if (widget) set_state_(UNDOCK);
  }
}


/**
 Sets the colors the object's 'drag widget' takes in each of its possible states.
 \param undock, drag, dock Color used for states Fl_Dockable_Group::UNDOCK, Fl_Dockable_Group::DRAG,
 Fl_Dockable_Group::DOCK, respectively.
 */
void Fl_Dockable_Group::color_for_states(Fl_Color undock, Fl_Color drag, Fl_Color dock) {
  undock_color_ = undock;
  drag_color_ = drag;
  dock_color_ = dock;
  if (drag_widget()) set_state_(state_);
}


/**
 Sets the text labels the object's 'drag widget' uses in each of its possible states.
 \param undock, drag, dock Text labels used for states Fl_Dockable_Group::UNDOCK, Fl_Dockable_Group::DRAG,
 Fl_Dockable_Group::DOCK, respectively.
 */
void Fl_Dockable_Group::label_for_states(const char *undock, const char * drag, const char * dock) {
  undock_label_ = undock;
  drag_label_ = drag;
  dock_label_ = dock;
  if (drag_widget()) set_state_(state_);
}


/** Default label of Docking_Target_Box objects */
const char *Fl_Dockable_Group::Docking_Target_Box::dockable_label = "Dock here";

Fl_Dockable_Group::target_state_f_type *Fl_Dockable_Group::Docking_Target_Box::state_f_ =
  Fl_Dockable_Group::Docking_Target_Box::default_state_f_;


/** Constructor */
Fl_Dockable_Group::Docking_Target_Box::Docking_Target_Box(int x, int y, int w, int h) :
    Fl_Box(FL_DOWN_BOX, x, y, w, h, dockable_label) {
      align(FL_ALIGN_CLIP | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    }


/** Destructor */
Fl_Dockable_Group::Docking_Target_Box::~Docking_Target_Box() {
  check_origin_target(this);
}


/** The destructor of any docking target class must call this function.
 The destructor should contain this statement:
 \code
   Fl_Dockable_Group::check_origin_target(this);
 \endcode
*/
void Fl_Dockable_Group::check_origin_target(Fl_Widget *target) {
  for (const auto& element : dockables_) {
    Fl_Dockable_Group *dock = (Fl_Dockable_Group*)element;
    if (dock->origin_target_ == target) {
      dock->origin_target_ = NULL;
      return;
    }
  }
}


/** Returns whether the mouse currently dragging an Fl_Dockable_Group is above  \p widget  */
bool Fl_Dockable_Group::is_dockable_inside(Fl_Widget *widget) {
  int X = Fl::event_x_root();
  int Y = Fl::event_y_root();
  int box_x, box_y, retval = 1;
  Fl_Window *top = widget->top_window_offset(box_x, box_y);
  box_x += top->x();
  box_y += top->y();
  return (X >= box_x && X < box_x + widget->w() && Y >= box_y && Y < box_y + widget->h());
}


void Fl_Dockable_Group::color_targets_following_dock_() {
  if (Fl::event_button() != FL_LEFT_MOUSE) return;
  int X = Fl::event_x_root(); // screen coordinates of the dockable-dragging mouse
  int Y = Fl::event_y_root();
  Fl_Window *dock = top_window(); // the dockable being dragged as a window
  Fl_Window *win = Fl::first_window();
  bool found = false;
  while (win) { // search other window on same screen below the mouse
    if (win != dock && !win->parent() && win->screen_num() == dock->screen_num()) {
      if (X >= win->x() && X < win->x() + win->w() && Y >= win->y() && Y < win->y() + win->h()) {
        int event = 0;
        if (this->state_ == DRAG) event = FL_DOCK_ENTER;
        else if (this->state_ == DOCK) event = FL_DOCK_DRAG;
        if (event) win->handle(event);
        found = true;
        break;
      }
    }
    win = Fl::next_window(win);
  }
  if (!found && active_dockable_) {
    set_state_(Fl_Dockable_Group::DRAG);
    if (active_target_) active_target_->handle(FL_DOCK_LEAVE);
  }
}


void Fl_Dockable_Group::after_release_() {
  driver_->after_release_();
}


/** The \p handle(int) virtual member function of any docking target class must call this function.
 That member function should be implemented as follows
 \code
 int docking_target_class::handle(int event) override {
   bool processed;
   int retval =  Fl_Dockable_Group::handle_docking_target(processed, this, event,
                   inside_f, acceptable_f, target_state_f, dock_payload_f, undock_f);
   return (processed ? retval : parent_class::handle(event));
 }
\endcode
 where \p docking_target_class is the docking target class name, \p inside_f, \p acceptable_f, \p target_state_f,
 \p dock_payload_f, \p undock_f are NULL or static functions defined by the
 docking target class and where \p parent_class is the docking target class'  parent class.
 \param[out] processed a bool variable that will be assigned by the \p handle_docking_target function. \p processed
 is assigned value \p true when \p event has been completely processed, and value \p false
 when processing of \p event has not been performed yet. In that case the \p handle(int)
 member function of the target class must transmit \p event to the \p handle(int) member function of its parent class.
 \param docking_target transmit \p this.
 \param event the calling \p handle(int) function's single argument.
 \param inside_f a function defining the target widget's docking area by returning \p true if and only if the mouse is in this area.
 Member function Fl_Dockable_Group::is_dockable_inside(Fl_Widget*) can be useful to implement that function.
 Work with the mouse location in screen coordinates given by  Fl::event_x_root() and Fl::event_y_root().
 \param acceptable_f NULL, or a function that returns whether the target widget recognizes the dockable as acceptable for docking.
 This function can be used to implement a compatibility mechanism between a given Fl_Dockable_Group object and a given docking target.
 \param target_state_f NULL, or a function that modifies the aspect of the target widget when the mouse is in the docking area.
 \param dock_payload_f a function that adds the dockable to the docking target widget if that widget is an Fl_Group,
 or to its parent group otherwise.
 \param undock_f NULL, or a function that performs operations that are necessary after FLTK has undocked the dockable
 and replaced it by a newly created Fl_Dockable_Group::Docking_Target_Box.
 \return use as return value of the \p handle(int) member function of the target class if \p handle_docking_target
 set \p processed to true.
 */
int Fl_Dockable_Group::handle_docking_target(bool& processed, Fl_Widget *docking_target, int event,
                                     inside_f_type inside_f, acceptable_f_type acceptable_f,
                                     target_state_f_type target_state_f,
                                     dock_payload_f_type dock_payload_f, undock_f_type undock_f) {
//printf("handle_docking_target: event=%d docking_target=%p active_dockable_=%p\n", event,docking_target,active_dockable_);
  if (Fl_Dockable_Group::active_dockable_) {
    active_dockable_->driver_->check_event_(event);
  }
  processed = true;
  switch (event) {
    case FL_DOCK_ENTER:
    case FL_DOCK_DRAG:
      // useful under Wayland w/o xdg-toplevel protocol where the active_dockable_ remains in place
      // in the docking target when it's dragged and should not accept DOCK events
      if (docking_target->as_group() && docking_target->contains(active_dockable_)) return 0;
      if (acceptable_f && !acceptable_f(docking_target, Fl_Dockable_Group::active_dockable_)) return 0;
      if (inside_f(docking_target)) { // true if mouse is inside the target's docking area
        if (active_dockable_->active_target_ != docking_target) {
          active_dockable_->active_target_ = docking_target;
        }
        if (active_dockable_->state_ != DOCK) {
          // firstly, set the state of the dockable, next that of the target
          active_dockable_->set_state_(DOCK);
          if (target_state_f) target_state_f(docking_target, true);
          active_dockable_->handle(FL_DOCK_ENTER);
        }
        if (Fl::belowmouse() != docking_target) Fl::belowmouse(docking_target); // important for Wayland
      } else {
        if (active_dockable_->state_ == DOCK) {
          active_dockable_->set_state_(Fl_Dockable_Group::DRAG);
          active_dockable_->handle(FL_DOCK_LEAVE);
          if (target_state_f) target_state_f(docking_target, false);
          docking_target->handle(FL_DOCK_LEAVE);
        }
        return 0;
      }
      break;
    case FL_DOCK_LEAVE:
      if (Fl_Dockable_Group::active_dockable_) {
        if (active_dockable_->state_ != DRAG) {
          active_dockable_->handle(FL_DOCK_LEAVE);
          if (target_state_f) target_state_f(docking_target, false);
        }
        Fl_Dockable_Group::active_dockable_->set_state_(Fl_Dockable_Group::DRAG);
      }
      if (active_dockable_->active_target_) {
        if( target_state_f) target_state_f(active_dockable_->active_target_, false);
        active_dockable_->active_target_ = NULL;
      }
      break;
    case FL_DOCK_RELEASE:
    {
      Fl_Dockable_Group *dock = Fl_Dockable_Group::active_dockable_;
      if (acceptable_f && !acceptable_f(docking_target, dock)) return 0;
      dock->driver_->before_dock();
      dock_payload_f(docking_target, dock);
      dock->handle(FL_DOCK_RELEASE);
      dock->active_target_ = NULL;
      dock->after_release_();
      dock->set_state_(Fl_Dockable_Group::UNDOCK);
      dock->origin_target_ = NULL;
      Fl_Dockable_Group::active_dockable_ = NULL;
    }
      break;
    case FL_UNDOCK:
      Fl_Dockable_Group::active_dockable_->origin_target_ = docking_target;
      if (undock_f) undock_f(docking_target, Fl_Dockable_Group::active_dockable_);
      Fl_Dockable_Group::active_dockable_->handle(FL_UNDOCK);
      break;
    default:
      processed = false;
      return 0;
  }
  return 1;
}


void Fl_Dockable_Group::Docking_Target_Box::default_state_f_(Fl_Widget *docking_target, bool can_dock) {
  Fl_Box *box = (Fl_Box*)docking_target;
  if (can_dock) {
    box->color(FL_SELECTION_COLOR);
    box->labelcolor(fl_contrast(FL_WHITE, FL_SELECTION_COLOR));
  } else {
    box->color(FL_BACKGROUND_COLOR);
    box->labelcolor(FL_BLACK);
  }
  box->redraw();
}
  

static void dock_payload_f(Fl_Widget *target, Fl_Dockable_Group *payload) {
    Fl_Group *group = target->parent();
    payload->resize(target->x(), target->y(), target->w(), target->h());
    group->add(payload);
    delete target;
    group->redraw();
}
   

int Fl_Dockable_Group::Docking_Target_Box::handle(int event) {
    bool processed;
    int retval = Fl_Dockable_Group::handle_docking_target(processed, this, event,
                    Fl_Dockable_Group::is_dockable_inside, NULL, state_f_, dock_payload_f, NULL);
    return (processed ? retval : Fl_Box::handle(event));
}


/** Process events that lead to undocking or dragging of an Fl_Dockable_Group.
 This method is useful for advanced uses of Fl_Dockable_Group and docking target widgets:
 - When an Fl_Dockable_Group's 'drag widget' was not created by its drag_box() method
 but by drag_widget(Fl_Widget*).
 That widget's handle(int) method must call this method for all events received.
 - When an Fl_Dockable_Group object docked in a 'docking target widget' can be undocked
 by another way than by clicking and dragging that dockable's 'drag widget'.
 
 \param event an FLTK event that can have 2 origins. 1) Any event received by an Fl_Dockable_Group's
 custom 'drag widget'. 2) An FL_PUSH or FL_DRAG event received by
 a 'docking target widget' (but not by a 'drag widget') and that is nevertheless susceptible to
 trigger undocking of a docked Fl_Dockable_Group child. The target widget's handle(int) method
 transmits this event to the child dockable's handle_drag_widget() method that will process it adequately
 if the child needs be undocked. See class \p tabs_docking_target of \p test/dock_undock.cxx for an example.
 \return non-zero when the event has been handled.
 */
int Fl_Dockable_Group::handle_drag_widget(int event) {
  if (!this->drag_widget()) { return 0; }
  return driver_->handle_drag_widget_(event);
}


/** Puts the Fl_Dockable_Group object inside a draggable top-level window.
 Call this member function after construction of a Fl_Dockable_Group object
 and after its drag widget was assigned to put it inside a borderless top-level window,
 in the undocked, draggable state.
 \return The created window which needs to be show()'n. */
Fl_Double_Window *Fl_Dockable_Group::make_draggable() {
  if (state() == DRAG)
    return window()->as_double_window();
  origin_target_ = NULL;
  Fl_Double_Window *win = driver_->undock();
  Fl::pushed(NULL);
  return win;
}

#ifndef FL_DOXYGEN

bool Fl_Dockable_Group_Driver::using_wayland = false;


int Fl_Dockable_Group_Driver::drag_box_class::handle(int event) {
  Fl_Dockable_Group *dock = (Fl_Dockable_Group*)parent();
  int r = dock->driver_->handle_drag_widget_(event);
  return (r ? r : Fl_Box::handle(event));
}


void Fl_Dockable_Group_Driver::dragged_dockable_cb(Fl_Window *win) {
  Fl_Dockable_Group *dock = (Fl_Dockable_Group*)win->child(0);
  Fl_Dockable_Group::active_dockable_ = dock;
  if (dock->origin_target_) {
    int r = dock->origin_target_->handle(FL_DOCK_RELEASE);
    if (r) return;
  }
  Fl_Window *top = Fl::first_window();
  int count = 0;
  while (top) { // search for a toplevel window able to dock the Fl_Dockable_Group inside win
    if (top != win && !top->parent()) {
      count++;
      int retval = top->handle(FL_DOCK_RELEASE);
      if (retval) return; // the Fl_Dockable_Group is docked
    }
    top = Fl::next_window(top);
  }
  int reply = (count > 0 ? fl_choice_n(Fl_Dockable_Group::close_dockable, fl_close, fl_cancel, NULL, "?") : 0);
  if (reply == 0) {
    Fl_Dockable_Group::active_dockable_ = NULL;
    Fl::delete_widget(win);
  }
}


int Fl_Dockable_Group_Driver::handle_drag_widget_(int event) {
  static int fromx, fromy, winx, winy, drag_count;
  Fl_Dockable_Group *dock = dockable_;
  if (event == FL_PUSH && (Fl::event_state() & FL_BUTTON1) &&
      (dock->state_ == Fl_Dockable_Group::UNDOCK || dock->state_ == Fl_Dockable_Group::DRAG)) {
    if (dock->state_ == Fl_Dockable_Group::UNDOCK) drag_count = 0;
    Fl_Group *top = dock->parent();
    Fl_Window *top_win = top->top_window();
    fromx = Fl::event_x_root();
    fromy = Fl::event_y_root();
    int offset_x = 0, offset_y = 0;
    if (top->window()) top->window()->top_window_offset(offset_x, offset_y);
    winx = top_win->x_root() + offset_x + dock->x();
    winy = top_win->y_root() + offset_y + dock->y();
    return 1;
  } else if (event == FL_DRAG && (Fl::event_state() & FL_BUTTON1) &&
             (dock->state_ == Fl_Dockable_Group::UNDOCK ||
              (dock->state_ == Fl_Dockable_Group::DRAG && Fl_Dockable_Group::active_dockable_ != dock))) {
    if (dock->state_ == Fl_Dockable_Group::DRAG) {
      Fl_Dockable_Group::active_dockable_ = dock; // re-drag previously abandoned dockable
      return dockable_->handle_drag_widget(FL_DRAG);
    }
    if (++drag_count >= 3) {
      Fl_Dockable_Group::active_dockable_ = dock;
      Fl_Group *top = dock->parent();
      // transform the dockable group into a draggable, borderless toplevel window
      // and replace it by a "Docking_Target_Box"
      Fl_Double_Window *new_win = undock();
      new_win->position(winx, winy);
      new_win->show();
      top->handle(FL_UNDOCK);
    }
    return 1;
  } else if (event == FL_DRAG &&
             (dock->state_ == Fl_Dockable_Group::DRAG || dock->state_ == Fl_Dockable_Group::DOCK)) {
    int deltax = Fl::event_x_root() - fromx; // Drag dockable around
    int deltay = Fl::event_y_root() - fromy;
    dock->window()->position(winx + deltax, winy + deltay);
    dock->color_targets_following_dock_();
    return 1;
  } else if (event == FL_RELEASE && dock->state_ == Fl_Dockable_Group::DOCK) { // Dock dockable in place
    return dockable_->active_target_->handle(FL_DOCK_RELEASE);
  }
  return 0;
}


void Fl_Dockable_Group_Driver::before_dock() {
  Fl_Window *top = dockable_->window();
  top->remove(dockable_);
  delete top;
}


class resizable_draggable_win : public Fl_Double_Window {
private:
  Fl_Cursor cursor_;
  enum resize_border { none, top, bottom, left, top_left, bottom_left, right, top_right, bottom_right };
public:
  resizable_draggable_win(int w, int h, const char *l, Fl_Dockable_Group *dock) :
      Fl_Double_Window(w, h, l), cursor_(FL_CURSOR_DEFAULT) {
    box(FL_UP_BOX);
    int margin = Fl::box_dx(box());
    dock->position(margin, margin);
    resizable(dock);
    size(w + 2*margin, h + 2*margin);
    cursor(cursor_);
  }
  int handle(int event) override;
};


int resizable_draggable_win::handle(int event) {
  static enum resize_border b = none;
  static int push_x, push_y;
  if (event == FL_LEAVE || event == FL_RELEASE) {
    cursor(cursor_ = FL_CURSOR_DEFAULT);
    return 1;
  }
  int margin = 2 * Fl::box_dx(FL_DOWN_BOX);
  bool inside_dock = (Fl::event_x() >= margin && Fl::event_x() < w() - margin && Fl::event_y() >= margin && Fl::event_y() < h() - margin);
  if (event == FL_MOVE && !inside_dock && cursor_ == FL_CURSOR_DEFAULT) {
    push_x = Fl::event_x_root(); push_y = Fl::event_y_root();
    if (Fl::event_x() < margin && Fl::event_y() < margin) {
      cursor_ = FL_CURSOR_NWSE;
      b = top_left;
    } else if (Fl::event_x() < margin && Fl::event_y() >= h() - margin) {
      cursor_ = FL_CURSOR_NESW;
      b = bottom_left;
    } else if (Fl::event_x() >= w() - margin && Fl::event_y() < margin) {
      cursor_ = FL_CURSOR_NESW;
      b = top_right;
    } else if (Fl::event_x() >= w() - margin && Fl::event_y() >= h() - margin) {
      cursor_ = FL_CURSOR_NWSE;
      b = bottom_right;
    } else if (Fl::event_x() < margin) { cursor_ = FL_CURSOR_W; b = left; }
    else if (Fl::event_y() < margin) { cursor_ = FL_CURSOR_N; b = top; }
    else if (Fl::event_x() >= w() - margin) { cursor_ = FL_CURSOR_E; b = right; }
    else if (Fl::event_y() >= h() - margin) { cursor_ = FL_CURSOR_S; b = bottom; }
    else b = none;
    cursor(cursor_);
    return 1;
  } else if (event == FL_MOVE && inside_dock && cursor_ != FL_CURSOR_DEFAULT) {
    cursor(cursor_ = FL_CURSOR_DEFAULT);
    return 1;
  } else if (event == FL_PUSH && cursor_ != FL_CURSOR_DEFAULT) {
    Fl_Dockable_Group *dock = (Fl_Dockable_Group*)child(0);
    return Fl_Dockable_Group_Driver::driver(dock)->start_interactive_resize(this, cursor_);
  } else if (event == FL_DRAG && cursor_ != FL_CURSOR_DEFAULT && !Fl_Dockable_Group_Driver::using_wayland) {
    int diffX = Fl::event_x_root() - push_x;
    int diffY = Fl::event_y_root() - push_y;
    if (b == right) size(w() + diffX, h());
    else if (b == left) resize(x() + diffX, y(), w() - diffX, h());
    else if (b == bottom) size(w(), h() + diffY);
    else if (b == top) resize(x(), y() + diffY, w(), h() - diffY);
    else if (b == bottom_right) size(w() + diffX, h() + diffY);
    else if (b == top_right) resize(x(), y() + diffY, w() + diffX, h() - diffY);
    else if (b == top_left) resize(x() + diffX, y() + diffY, w() - diffX, h() - diffY);
    else if (b == bottom_left) resize(x() + diffX, y(), w() - diffX, h() + diffY);
    push_x = Fl::event_x_root(); push_y = Fl::event_y_root();
    return 1;
  }
  return Fl_Window::handle(event);
}


Fl_Double_Window *Fl_Dockable_Group_Driver::undock() {
  // transform the dockable group into a draggable, borderless toplevel window
  Fl_Group *top = dockable_->parent();
  if (top) {
    top->remove(dockable_);
    Fl_Box *tmp = new Fl_Dockable_Group::Docking_Target_Box(
                                                      dockable_->x(), dockable_->y(), dockable_->w(), dockable_->h());
    top->add(tmp);
    top->redraw();
  }
  const char *label = "Fl_Dockable_Group";
  Fl_Double_Window *win = dockable_->resizable() ? new resizable_draggable_win(dockable_->w(), dockable_->h(),
                   label, dockable_) : new Fl_Double_Window(dockable_->w(), dockable_->h(), label);
  win->end();
  win->add(dockable_);
  dockable_->show(); // necessary for tabs
  win->border(0);
  Fl::pushed(dockable_->drag_widget()); // necessary for tabs

  win->callback((Fl_Callback0*)dragged_dockable_cb);
  dockable_->set_state_(Fl_Dockable_Group::DRAG);
  return win;
}


void Fl_Dockable_Group::set_state_(enum Fl_Dockable_Group::states s) {
  state_ = s;
  Fl_Color c;
  const char *t;
  if (state_ == Fl_Dockable_Group::UNDOCK) {
    c = undock_color_; t = undock_label_;
  } else if (state_ == Fl_Dockable_Group::DRAG) {
    c = drag_color_; t = drag_label_; }
  else if (state_ == Fl_Dockable_Group::DOCK) {
    c = dock_color_; t = dock_label_;
  }
  drag_widget()->color(c);
  drag_widget()->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, c));
  drag_widget()->label(t);
  drag_widget()->redraw();
}

#endif // FL_DOXYGEN
