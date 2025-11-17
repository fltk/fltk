//
// Program using class Fl_Dockable_Group for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl_Dockable_Group.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Round_Clock.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Tile.H>
#include <FL/platform.H> // for fl_open_display()


void delete_win(Fl_Window *win) {
  delete win;
}


/* A dockable target widget derived from Fl_Tabs with this behavior:
 - Any Fl_Dockable_Group object that's a tab of this widget can be undocked and
 dragged away from the Fl_Tabs. This leaves the Fl_Tabs with one child less.
 - There are 2 ways to undock an Fl_Dockable_Group docked inside such an Fl_Tabs object
  1. click and drag on the dockable's drag widget;
  2. click and drag on the dockable's tab when the dockable is the Fl_Tabs' value() child.
 - The target widget's docking area is its tabs area.
 Any Fl_Dockable_Group object dragged above this area can be docked
 leaving the Fl_Tabs object with one child more.
  */
class tabs_docking_target : public Fl_Tabs {
private:

  static bool inside_f(Fl_Widget *target) {
    Fl_Tabs *tabs = (Fl_Tabs*)target;
    bool inside = Fl_Dockable_Group::is_dockable_inside(tabs); // true if mouse inside Fl_Tabs
    if (inside) { // is mouse inside the Fl_Tabs' tab area?
      int X, Y, W, H; // rectangle of tab area relatively to Fl_Tabs top-left
      if (tabs->children() > 0) {
        X = tabs->child(0)->x(); Y= 0; W = tabs->child(0)->w(); H = tabs->h() - tabs->child(0)->h();
      } else {
        tabs->client_area(X, Y, W, H);
        Y = 0; H = tabs->h() - H;
      }
      int pos_x, pos_y; // screen coordinates of Fl_Tabs top-left
      Fl_Window *top = tabs->top_window_offset(pos_x, pos_y);
      pos_x += top->x();
      pos_y += top->y();
      // mouse position relatively to Fl_Tabs top-left
      int e_x = Fl::event_x_root() - pos_x;
      int e_y = Fl::event_y_root() - pos_y;
      inside = (e_x >= X && e_x < X + W && e_y >= Y && e_y < Y + H);
    }
    return inside;
  }

  static void dock_payload_f(Fl_Widget *target, Fl_Dockable_Group *payload) {
    Fl_Tabs *tabs = (Fl_Tabs*)target;
    int x, y, w, h;
    tabs->client_area(x, y, w, h);
    payload->resize(x, y, w, h);
    tabs->add(payload);
    tabs->value(payload);
  }

  static void undock_f(Fl_Widget *target, Fl_Dockable_Group *dock) {
    Fl_Tabs *tabs = (Fl_Tabs*)target;
    Fl_Widget *c = tabs->child(tabs->children() - 1);
    tabs->remove(c);
    delete c;
  }

public:
  tabs_docking_target(int x, int y, int w, int h, const char *l=NULL) : Fl_Tabs(x,y,w,h,l) {}
  virtual ~tabs_docking_target() {
    Fl_Dockable_Group::check_origin_target(this);
  }
  int handle(int event) override {
    bool processed;
    if (event == FL_PUSH || event == FL_DRAG) {
      Fl_Widget *tab = which(Fl::event_x(), Fl::event_y());
      if (tab && value() == tab) {
        // replaces dock = dynamic_cast<Fl_Dockable_Group*>(tab);
        Fl_Dockable_Group *dock = Fl_Dockable_Group::as_dockable(tab);
        if (dock) {
          int r = dock->handle_drag_widget(event);
          if (r && event == FL_DRAG) return r;
        }
      }
    }
    int retval =  Fl_Dockable_Group::handle_docking_target(processed, this, event,
                                                    inside_f, NULL, NULL, dock_payload_f, undock_f);
    return (processed ? retval : Fl_Tabs::handle(event));
  }
};


/* A dockable target widget derived from Fl_Flex with this behavior:
 - The Fl_Flex initially contains a single child widget.
 - The Fl_Flex's dragging area is its right third.
 - An Fl_Dockable_Group object is acceptable if its width is less than
 the dragging area width.
 - Any acceptable Fl_Dockable_Group can be dragged above this dragging area.
 This adds a second child to the Fl_Flex which is a Docking_Target_Box object.
 - If the mouse is released, the dragged Fl_Dockable_Group docks in and replaces
 the Docking_Target_Box.
 - Any docked Fl_Dockable_Group can be undocked leaving the Fl_Flex
 back with one child.
*/
class flex_docking_target : public Fl_Flex {
private:
  
  static bool acceptable_f(Fl_Widget *target, Fl_Dockable_Group *dock) {
    return (dock->w() <= target->w() / 3 + 8);
  }

  static bool inside_f(Fl_Widget *target) {
    Fl_Flex *target_flex = (Fl_Flex*)target;
    bool inside = Fl_Dockable_Group::is_dockable_inside(target_flex); // is mouse inside Fl_Flex?
    if (inside) { // is mouse inside the Fl_Flex' docking zone?
      int X, Y, W, H; // rectangle of docking zone relatively to Fl_Flex top-left
      X = target_flex->x() + target_flex->w() * 0.66;
      Y = target_flex->y();
      W = target_flex->w() * 0.33;
      H = target_flex->h();
      // mouse position relatively to Fl_Flex top-left
      int pos_x, pos_y;
      Fl_Window *top = target_flex->top_window_offset(pos_x, pos_y);
      pos_x += top->x();
      pos_y += top->y();
      int e_x = Fl::event_x_root() - pos_x;
      int e_y = Fl::event_y_root() - pos_y;
      inside = (e_x >= X && e_x < X + W && e_y >= Y && e_y < Y + H);
    }
    return inside;
  }
  
  static void target_state_f(Fl_Widget *target, bool can_dock) {
    Fl_Flex *target_flex = (Fl_Flex*)target;
    if (can_dock && target_flex->children() == 1) {
      int X, Y, W, H; // rectangle of docking zone relatively to Fl_Flex top-left
      X = target_flex->x() + target_flex->w() * 0.66;
      Y = target_flex->y();
      W = target_flex->w() * 0.33;
      H = target_flex->h();
      Fl_Dockable_Group::Docking_Target_Box *box = new Fl_Dockable_Group::Docking_Target_Box(X, Y, W, H);
      target_flex->add(box);
      target_flex->fixed(target_flex->child(0), target_flex->w() * 0.66);
      target_flex->fixed(target_flex->child(1), target_flex->w() - target_flex->child(0)->w());
      Fl_Dockable_Group::target_state_f_type *set_state_f = Fl_Dockable_Group::Docking_Target_Box::docking_box_state_f();
      set_state_f(box, true);
      target_flex->redraw();
    } else if (!can_dock && target_flex->children() == 2) {
      undock_f(target, NULL);
    }
  }

  static void dock_payload_f(Fl_Widget *target, Fl_Dockable_Group *payload) {
    Fl_Flex *target_flex = (Fl_Flex*)target;
    if (target_flex->children() == 1) target_state_f(target, true);
    Fl_Widget *box = target_flex->child(1);
    payload->resize(box->x(), box->y(), box->w(), box->h());
    target_flex->add(payload);
    target_flex->remove(box);
    payload->redraw();
    delete box;
  }
  
  static void undock_f(Fl_Widget *target, Fl_Dockable_Group *) {
    Fl_Flex *target_flex = (Fl_Flex*)target;
    if (target_flex->children() == 2) {
      Fl_Widget *c = target_flex->child(1);
      target_flex->remove(c);
      delete c;
      target_flex->fixed(target_flex->child(0), target_flex->w());
      target_flex->redraw();
    }
  }

public:
  flex_docking_target(int x, int y, int w, int h) : Fl_Flex(x, y, w, h, Fl_Flex::HORIZONTAL) {}
  virtual ~flex_docking_target() {
    Fl_Dockable_Group::check_origin_target(this);
  }
  int handle(int event) override {
    bool processed;
    int retval =  Fl_Dockable_Group::handle_docking_target(processed, this, event,
                                inside_f, acceptable_f, target_state_f, dock_payload_f, undock_f);
    return (processed ? retval : Fl_Flex::handle(event));
  }
};


/* This class creates an Fl_Dockable_Group object that performs a programmed
 operation (here, change its background color) when it docks or undocks.
 */
class Dockable_With_Events : public Fl_Dockable_Group {
public:
  Dockable_With_Events(int x, int y, int w, int h, const char *l) : Fl_Dockable_Group(x,y,w,h,l) {}
  int handle(int event) override {
    if (event == FL_DOCK_RELEASE) {
      printf("%s receives FL_DOCK_RELEASE\n",label());
      color(FL_GREEN);
    } else if (event == FL_UNDOCK) {
      printf("%s receives FL_UNDOCK\n",label());
      color(FL_DARK_GREEN);
    } else if (event == FL_DOCK_LEAVE) {
      printf("%s receives FL_DOCK_LEAVE\n",label());
    } else if (event == FL_DOCK_ENTER) {
      printf("%s receives FL_DOCK_ENTER\n",label());
    }
    return Fl_Dockable_Group::handle(event);
  }
};


int main(int argc, char **argv) {
  fl_open_display();
  Fl_Window *source = new Fl_Window(150, 150, 340, 180, "Fl_Tabs-derived docking target");
  source->callback((Fl_Callback0*)delete_win);
  Fl_Tabs *tabs = new tabs_docking_target(20, 40, 310, 115);
  int X, Y, W, H;
  tabs->client_area(X, Y, W, H); // requires display to be opened
  Fl_Dockable_Group *dock = new Fl_Dockable_Group(X, Y, W, H, "Clock Dockable");
  dock->align(FL_ALIGN_CLIP+FL_ALIGN_LEFT+FL_ALIGN_INSIDE);
  dock->color(FL_YELLOW);
  new Fl_Clock(245, 90, 60, 60);
  Fl_Box *r = new Fl_Box(FL_NO_BOX, 0, 151, 155, 10, NULL);
  dock->end();
  new Fl_Box(FL_FLAT_BOX, X, Y, W, H, "Tab 2");
  tabs->end();
  source->end();
  source->resizable(dock);
  dock->drag_box(27, 67, 300, 13);
  dock->drag_widget()->labelsize(10);
  source->show(argc, argv);
  
  dock = new Dockable_With_Events(25, 60, 305, 95, "Round-clock Dockable");
  dock->align(FL_ALIGN_CLIP+FL_ALIGN_BOTTOM+FL_ALIGN_INSIDE);
  dock->color(FL_DARK_GREEN);
  new Fl_Round_Clock(245, 65, 60, 60);
  new Fl_Input(30, 110, 60, 20);
  r = new Fl_Box(FL_NO_BOX, 0, 126, 155, 10, NULL);
  r->clear_visible(); // important for DnD to Fl_Input
  dock->end();
  dock->drag_box(270, 140, 60, 13);
  dock->drag_widget()->labelsize(10);
  Fl_Window *last_win = dock->make_draggable();
  last_win->position(source->x() + source->w() + 50, source->y());
  source = last_win;

  Fl_Window *destination = new Fl_Window(150, source->y() + source->h() + 150,
                                         340, 190, "Fl_Tabs-derived docking target");
  destination->callback((Fl_Callback0*)delete_win);
  new Fl_Input(5,5,100,30, NULL);
  tabs = new tabs_docking_target(5, 40, 310, 140);
  tabs->client_area(X, Y, W, H);
  new Fl_Box(FL_FLAT_BOX, X, Y, W, H, "Tab 1");
  tabs->end();
  tabs->value(dock);
  destination->end();
  destination->resizable(destination);
  destination->show();

  destination = new Fl_Window(destination->x() + destination->w() + 50, source->y() + source->h() + 50,
                              340, 190, "Docking_Target_Box docking target");
  destination->callback((Fl_Callback0*)delete_win);
  new Fl_Dockable_Group::Docking_Target_Box(10, 30, 320, 150);
  
  destination->end();
  destination->show();
  
  source = new Fl_Window(source->x()+source->w()+50, source->y(), 340, 180, "Docking_Target_Box in Fl_Tile");
  Fl_Tile *tile = new Fl_Tile(5, 5, 330, 160, "");
  Fl_Box *yellow = new Fl_Box(FL_DOWN_BOX, 5, 5, 250, 160, "");
  yellow->color(FL_YELLOW);
  dock = new Fl_Dockable_Group(255, 5, 80, 160, "Small-clock Dockable");
  dock->align(FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
  dock->color(FL_BLUE);
  dock->labelcolor(FL_WHITE);
  dock->drag_box(260, 10, 70, 20);
  dock->begin();
  new Fl_Round_Clock(300, 130, 30, 30);
  dock->resizable(new Fl_Box(260, 40, 30, 50, ""));
  dock->end();
  tile->end();
  source->end();
  source->callback((Fl_Callback0*)delete_win);
  source->resizable(tile);
  source->show();

  source = new Fl_Window(source->x(), source->y() +source->h()+50, 340, 180, "Fl_Flex-derived docking target");
  flex_docking_target *flex = new flex_docking_target(5, 5, 330, 160);
  Fl_Box *box = new Fl_Box(FL_DOWN_BOX, 5, 5, 330, 160, "child");
  box->color(FL_GREEN);
  flex->end();
  source->end();
  source->show();
  source->wait_for_expose();
  last_win->show();
  
  return Fl::run();
}
