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

#ifndef FL_DOXYGEN

#include <FL/Fl_Dockable_Group.H>

class Fl_Dockable_Group_Driver {
  friend class Fl_Dockable_Group;
private:
  virtual void after_release_() {}
  virtual void check_event_(int& event) {}
  virtual int handle_drag_widget_(int event);
protected:
  Fl_Dockable_Group *dockable_;
  Fl_Double_Window *undock();
  static void dragged_dockable_cb(Fl_Window *win);
public:
  static void active_dockable(Fl_Dockable_Group *dock) { Fl_Dockable_Group::active_dockable_ = dock; }
  class drag_box_class : public Fl_Box {
  public:
    drag_box_class(Fl_Boxtype b, int x, int y, int w, int h, const char *l) : Fl_Box(b, x, y, w, h, l) {
      labelsize(12);
    }
    int handle(int event) override;
  };
  static Fl_Dockable_Group_Driver *newDockableGroupDriver(Fl_Dockable_Group*);
  Fl_Dockable_Group_Driver(Fl_Dockable_Group *from) { dockable_ = from; }
  static Fl_Dockable_Group_Driver *driver(Fl_Dockable_Group *dock) { return dock->driver_; }
  inline void state(enum Fl_Dockable_Group::states s) { dockable_->set_state_(s); }
  virtual void before_dock();
  virtual int start_interactive_resize(Fl_Window *, Fl_Cursor) { return 1; }
  static bool using_wayland;
};

#endif // ! FL_DOXYGEN
