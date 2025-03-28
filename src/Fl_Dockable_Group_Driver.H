//
// Dockable window code for the Fast Light Tool Kit (FLTK).
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

#ifndef FL_DOXYGEN

#include <FL/Fl_Dockable_Group.H>
#include <vector>

class Fl_Dockable_Group_Driver {
  friend class Fl_Dockable_Group;
private:
  Fl_Color undock_color_, drag_color_, dock_color_, docked_color_, dragged_color_;
  const char *undock_label_, *drag_label_, *dock_label_, *docked_label_, *dragged_label_;
  static const char *target_box_label_;
  static std::vector<Fl_Box *>target_; // vector of target boxes
  static int target_index_; // index in vector to target where docking will occur:
protected:
  static void delete_win_cb(Fl_Window *);
  Fl_Dockable_Group *dockable_;
public:
  class cmd_box_class : public Fl_Box {
  public:
    cmd_box_class(Fl_Boxtype b, int x, int y, int w, int h, const char *l) : Fl_Box(b, x, y, w, h, l) {
      labelsize(12);
    }
    int handle(int event) FL_OVERRIDE;
  };
  enum target_states {
    MAY_RECEIVE,
    DOCK_HERE
  };
  class target_box_class : public Fl_Box {
  public:
    enum target_states state_;
    target_box_class(int x, int y, int w, int h) : Fl_Box(FL_DOWN_BOX, x,y,w,h,NULL) {
      labelfont(FL_ITALIC);
      state(MAY_RECEIVE);
    }
    virtual ~target_box_class();
    void state(enum target_states);
  };
  static Fl_Dockable_Group_Driver *newDockableGroupDriver(Fl_Dockable_Group*);
  static Fl_Box *newTargetBoxClass(int x, int y, int w, int h);
  Fl_Dockable_Group_Driver(Fl_Dockable_Group *from) { dockable_ = from; }
  void store_docked_position(Fl_Dockable_Group *dock);
  static Fl_Box *new_target_box(int x, int y, int w, int h);
  virtual int handle(cmd_box_class *, int event);
  static Fl_Dockable_Group_Driver *driver(Fl_Dockable_Group *dock) { return dock->driver_; }
  void state(enum Fl_Dockable_Group::states);
  Fl_Group *parent_when_docked() { return dockable_->parent_when_docked_; }
  Fl_Widget **place_holder_while_dragged() { return &dockable_->place_holder_while_dragged_; }
  int x_when_docked() { return dockable_->x_when_docked_; }
  int y_when_docked() { return dockable_->y_when_docked_; }
};

#endif // ! FL_DOXYGEN
