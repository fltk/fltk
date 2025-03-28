//
// Wayland-specific dockable window code for the Fast Light Tool Kit (FLTK).
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
/** \file
 Fl_Wayland_Dockable_Group_Driver implementation.
 */


#include <config.h> // for HAVE_XDG_TOPLEVEL_DRAG
#include <FL/platform.H>
#include <FL/Fl_Dockable_Group.H>
#include <FL/Fl_Image_Surface.H>
#include "../../Fl_Dockable_Group_Driver.H"
#include "Fl_Wayland_Screen_Driver.H"
#include "Fl_Wayland_Window_Driver.H"
#include "xdg-shell-client-protocol.h"
#if HAVE_XDG_TOPLEVEL_DRAG
#  include "xdg-toplevel-drag-client-protocol.h"
#endif


class Fl_Wayland_Dockable_Group_Driver : public Fl_Dockable_Group_Driver {
private:
#if HAVE_XDG_TOPLEVEL_DRAG
  struct xdg_toplevel_drag_v1 *drag_;
  int old_keyboard_screen_scaling_;
  static void delete_win_cb_(Fl_Window *);
  Fl_Window *copy_(cmd_box_class *box, const char *t);
#endif
public:
  class wld_target_box_class : public target_box_class {
    public:
      wld_target_box_class(int x, int y, int w, int h) : target_box_class(x,y,w,h) {}
      int handle(int event) FL_OVERRIDE;
  };
  Fl_Wayland_Dockable_Group_Driver(Fl_Dockable_Group *from);
  int handle(cmd_box_class *, int event) FL_OVERRIDE;
};


// This class is used when the compositor doesn't support protocol 'XDG toplevel drag'.
// An image of the Fl_Dockable_Group object is used as the DnD cursor icon.
class Fl_oldWayland_Dockable_Group_Driver : public Fl_Wayland_Dockable_Group_Driver {
private:
  struct Fl_Wayland_Graphics_Driver::wld_buffer *offscreen_from_group_(int scale);
public:
  Fl_oldWayland_Dockable_Group_Driver(Fl_Dockable_Group *from) : Fl_Wayland_Dockable_Group_Driver(from) {}
  int handle(cmd_box_class *, int event) FL_OVERRIDE;
};


Fl_Wayland_Dockable_Group_Driver::Fl_Wayland_Dockable_Group_Driver(Fl_Dockable_Group *from) :
  Fl_Dockable_Group_Driver(from) {
#if HAVE_XDG_TOPLEVEL_DRAG
  drag_ = NULL;
  old_keyboard_screen_scaling_ = 0;
    //((Fl_Wayland_Screen_Driver*)Fl::screen_driver())->xdg_toplevel_drag = NULL;//TMP
#endif
}


Fl_Dockable_Group_Driver *Fl_Dockable_Group_Driver::newDockableGroupDriver(Fl_Dockable_Group *dock) {
  fl_open_display();
  if (fl_wl_display()) {
    Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
    return ( scr_driver->xdg_toplevel_drag ? new Fl_Wayland_Dockable_Group_Driver(dock) :
               new Fl_oldWayland_Dockable_Group_Driver(dock) );
  } else return new Fl_Dockable_Group_Driver(dock);
}


Fl_Box *Fl_Dockable_Group_Driver::newTargetBoxClass(int x, int y, int w, int h) {
  fl_open_display();
  if (fl_wl_display()) {
    return new Fl_Wayland_Dockable_Group_Driver::wld_target_box_class(x, y, w, h);
  } else return new target_box_class(x, y, w, h);
}


#if HAVE_XDG_TOPLEVEL_DRAG

Fl_Window *Fl_Wayland_Dockable_Group_Driver::copy_(cmd_box_class *box, const char *t) {
  // transform the dockable group into a draggable, borderless toplevel window
  Fl_Group *top = dockable_->parent();
  store_docked_position(dockable_);
  top->remove(dockable_);
  *place_holder_while_dragged() =
    new Fl_Box(dockable_->box(), dockable_->x(), dockable_->y(), dockable_->w(), dockable_->h(), "Dragged");
  (*place_holder_while_dragged())->align(FL_ALIGN_CLIP);
  top->add(*place_holder_while_dragged());
  top->redraw();
  Fl_Window *win = new Fl_Window(dockable_->w(), dockable_->h(), t);
  dockable_->position(0,0);
  win->add(dockable_);
  win->end();
  win->callback((Fl_Callback0*)Fl_Wayland_Dockable_Group_Driver::delete_win_cb_);
  state(Fl_Dockable_Group::DRAG);
  win->border(0);
  return win;
}


void Fl_Wayland_Dockable_Group_Driver::delete_win_cb_(Fl_Window *win) {
  Fl_Dockable_Group *dock = (Fl_Dockable_Group*)win->child(0);
  Fl_Wayland_Dockable_Group_Driver *dr = (Fl_Wayland_Dockable_Group_Driver*)Fl_Dockable_Group_Driver::driver(dock);
  if (dr->old_keyboard_screen_scaling_) Fl::keyboard_screen_scaling(1);
  Fl_Dockable_Group_Driver::delete_win_cb(win);
}

#endif // HAVE_XDG_TOPLEVEL_DRAG


int Fl_Wayland_Dockable_Group_Driver::wld_target_box_class::handle(int event) {
  Fl_Dockable_Group *dock = Fl_Dockable_Group::active_dockable;
  if (!dock) return 0;
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  if (event == FL_DND_ENTER) {
    //puts("FL_DND_ENTER");
    if (scr_driver->xdg_toplevel_drag) Fl_Dockable_Group_Driver::driver(dock)->state(Fl_Dockable_Group::DOCK);
    state(DOCK_HERE);
    return 1;
  } else if (event == FL_DND_DRAG) {
    //puts("FL_DND_DRAG");
    return 1;
  } else if (event == FL_DND_LEAVE) {
    //puts("FL_DND_LEAVE");
    if (scr_driver->xdg_toplevel_drag) Fl_Dockable_Group_Driver::driver(dock)->state(Fl_Dockable_Group::DRAG);
    state(MAY_RECEIVE);
    return 1;
  } else if (event == FL_DND_RELEASE) {
    //puts("FL_DND_RELEASE");
    Fl_Dockable_Group::active_dockable = NULL;
    Fl_Group *target_group = this->parent();
    Fl_Group *new_target_box_place;
    int x_when_docked, y_when_docked;
#if HAVE_XDG_TOPLEVEL_DRAG
    if (scr_driver->xdg_toplevel_drag) {
      Fl_Wayland_Dockable_Group_Driver *dr = (Fl_Wayland_Dockable_Group_Driver*)driver(dock);
      xdg_toplevel_drag_v1_destroy(dr->drag_);
      dr->drag_ = NULL;
      Fl_Window *top = dock->window();
      top->hide();
      top->remove(dock);
      delete top;
      if (dr->old_keyboard_screen_scaling_) Fl::keyboard_screen_scaling(1);
      new_target_box_place = driver(dock)->parent_when_docked();
      x_when_docked = driver(dock)->x_when_docked();
      y_when_docked = driver(dock)->y_when_docked();
    } else
#endif // HAVE_XDG_TOPLEVEL_DRAG
    {
      dock->parent()->redraw();
      new_target_box_place = dock->parent();
      x_when_docked = dock->x();
      y_when_docked = dock->y();
    }
    // Replace 'this' by 'dock' in target_group keeping its position in the child array.
    // This is advantageous if target_group is an Fl_Tab by keeping the current active tab.
    target_group->insert(*dock, this);
    // move target-box this from its parent to dock's original parent
    new_target_box_place->add(this);
    if (*Fl_Dockable_Group_Driver::driver(dock)->place_holder_while_dragged()) {
      delete *Fl_Dockable_Group_Driver::driver(dock)->place_holder_while_dragged();
      *Fl_Dockable_Group_Driver::driver(dock)->place_holder_while_dragged() = NULL;
      Fl_Dockable_Group_Driver::driver(dock)->parent_when_docked()->redraw();
    }
    this->set_visible();
    this->state(MAY_RECEIVE);
    new_target_box_place->redraw();
    target_group->redraw();
    int dock_w = dock->w(), dock_h = dock->h();
    dock->resize(x(), y(), w(), h());
    this->resize(x_when_docked, y_when_docked, dock_w, dock_h);
    dock->clear_visible();
    Fl_Dockable_Group_Driver::driver(dock)->state(Fl_Dockable_Group::UNDOCK);
    wl_data_source_set_user_data(scr_driver->seat->data_source, NULL);
    dock->show();
    return 0; // not to generate FL_PASTE event
  }
  return Fl_Box::handle(event);
}


static void xdg_toplevel_drag_data_source_handle_cancelled(void *data, struct wl_data_source *source) {
  Fl_Dockable_Group *dock = (Fl_Dockable_Group*)data;
  if (dock) Fl_Dockable_Group_Driver::driver(dock)->state(Fl_Dockable_Group::UNDOCK);
  Fl_Dockable_Group::active_dockable = NULL;
  Fl_Wayland_Screen_Driver::p_data_source_listener->cancelled(NULL, source);
}


static void xdg_toplevel_drag_data_source_handle_dnd_finished(void *data, struct wl_data_source *source) {
  xdg_toplevel_drag_data_source_handle_cancelled(data, source);
}


static struct wl_data_source_listener *xdg_toplevel_drag_listener = NULL;

static const struct wl_data_source_listener *xdg_toplevel_drag_data_source_listener() {
  if (!xdg_toplevel_drag_listener) {
    xdg_toplevel_drag_listener = new struct wl_data_source_listener;
    memcpy(xdg_toplevel_drag_listener, Fl_Wayland_Screen_Driver::p_data_source_listener, sizeof(struct wl_data_source_listener));
    xdg_toplevel_drag_listener->cancelled = xdg_toplevel_drag_data_source_handle_cancelled;
    xdg_toplevel_drag_listener->dnd_finished = xdg_toplevel_drag_data_source_handle_dnd_finished;
  }
  return (const struct wl_data_source_listener *)xdg_toplevel_drag_listener;
}


int Fl_Wayland_Dockable_Group_Driver::handle(Fl_Dockable_Group_Driver::cmd_box_class *box, int event) {
#if HAVE_XDG_TOPLEVEL_DRAG
  static int drag_count;
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  Fl_Dockable_Group *dock = (Fl_Dockable_Group*)box->parent();
  if ((event != FL_PUSH && event != FL_DRAG) || !(Fl::event_state() & FL_BUTTON1))
    return box->Fl_Box::handle(event);
  if (event == FL_PUSH && dock->state == Fl_Dockable_Group::UNDOCK) {
    drag_count = 0;
  } else if (event == FL_DRAG && dock->state == Fl_Dockable_Group::UNDOCK && ++drag_count >= 3) {
    Fl_Window *top = dock->top_window();
    // It seems that while MUTTER accepts to apply the xdg_toplevel_drag protocol
    // to a subwindow, KWIN doesn't accept it and works OK only when dragging inside a toplevel.
    struct wld_window *xid = fl_wl_xid(dock->window());
    scr_driver->seat->data_source = wl_data_device_manager_create_data_source(scr_driver->seat->data_device_manager);
    wl_data_source_add_listener(scr_driver->seat->data_source, xdg_toplevel_drag_data_source_listener(), (void*)0);
    wl_data_source_offer(scr_driver->seat->data_source, Fl_Wayland_Screen_Driver::xdg_toplevel_drag_pseudo_mime);
    wl_data_source_set_actions(scr_driver->seat->data_source, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
    Fl_Wayland_Dockable_Group_Driver *dr = (Fl_Wayland_Dockable_Group_Driver*)Fl_Dockable_Group_Driver::driver(dock);
    dr->drag_ =
    xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(scr_driver->xdg_toplevel_drag, scr_driver->seat->data_source);
    //printf("start_drag surface=%p serial=%u\n",xid->wl_surface, scr_driver->seat->serial);
    wl_data_device_start_drag(scr_driver->seat->data_device, scr_driver->seat->data_source,
                              xid->wl_surface, NULL, scr_driver->seat->serial);
    int dock_x = dock->x(), dock_y = dock->y();
    Fl_Window *new_win = copy_(box, "dragged");
    box->parent()->show(); // necessary for tabs
    new_win->show();
    Fl::pushed(dock->command_box()); // necessary for tabs
    xid = fl_wl_xid(new_win);
    xdg_toplevel_set_parent(xid->xdg_toplevel, Fl_Wayland_Window_Driver::driver(top)->xdg_toplevel());
    float s = Fl::screen_scale(top->screen_num());
    if (Fl_Wayland_Screen_Driver::compositor == Fl_Wayland_Screen_Driver::MUTTER)
               s *= Fl_Wayland_Window_Driver::driver(top)->wld_scale();
    xdg_toplevel_drag_v1_attach(dr->drag_, xid->xdg_toplevel,
                                (Fl::event_x() - dock_x) * s, (Fl::event_y() - dock_y) * s);
    //printf("xdg_toplevel_drag_v1_attach to toplevel=%p\n",xid->xdg_toplevel);
    Fl_Dockable_Group::active_dockable = dock;
    old_keyboard_screen_scaling_ = Fl_Screen_Driver::keyboard_screen_scaling;
    Fl::keyboard_screen_scaling(0); // turn off dynamic GUI scaling
  } else if (event == FL_PUSH && dock->state == Fl_Dockable_Group::DRAG) {
    // catch again a draggable window
    Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
    Fl_Wayland_Dockable_Group_Driver *dr = (Fl_Wayland_Dockable_Group_Driver*)Fl_Dockable_Group_Driver::driver(dock);
    if (dr->drag_) xdg_toplevel_drag_v1_destroy(dr->drag_);
    scr_driver->seat->data_source = wl_data_device_manager_create_data_source(scr_driver->seat->data_device_manager);
    wl_data_source_add_listener(scr_driver->seat->data_source, xdg_toplevel_drag_data_source_listener(), (void*)0);
    wl_data_source_offer(scr_driver->seat->data_source, Fl_Wayland_Screen_Driver::xdg_toplevel_drag_pseudo_mime);
    wl_data_source_set_actions(scr_driver->seat->data_source, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
    dr->drag_ = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(scr_driver->xdg_toplevel_drag, scr_driver->seat->data_source);
    struct wld_window *xid = fl_wl_xid(dock->window());
    //printf("start_drag surface=%p serial=%u\n",scr_driver->seat->pointer_focus, scr_driver->seat->serial);
    wl_data_device_start_drag(scr_driver->seat->data_device, scr_driver->seat->data_source,
                              scr_driver->seat->pointer_focus, NULL, scr_driver->seat->serial);
    //printf("xdg_toplevel_drag_v1_attach to toplevel=%p\n",xid->xdg_toplevel);
    // need to attach AFTER start_drag even though xdg_toplevel_drag protocol doc says opposite!
    float s = Fl::screen_scale(dock->window()->screen_num());
    xdg_toplevel_drag_v1_attach(dr->drag_, xid->xdg_toplevel, Fl::event_x() * s, Fl::event_y() * s);
    Fl_Dockable_Group::active_dockable = dock;
  }
  return 1;
#endif // HAVE_XDG_TOPLEVEL_DRAG
  return box->Fl_Box::handle(event);
}


struct Fl_Wayland_Graphics_Driver::wld_buffer *
      Fl_oldWayland_Dockable_Group_Driver::offscreen_from_group_(int scale) {
  int icon_size = 200; // max width or height of image for cursor in FLTK units
  struct Fl_Wayland_Graphics_Driver::wld_buffer *off;
  double r1 = double(icon_size) / dockable_->w();
  double r2 = double(icon_size) / dockable_->h();
  double r = (r1 < r2 ? r1 : r2);
  r = (r < 1 ? r : 1);
  int width = dockable_->w() * r;
  int height = dockable_->h() * r;
  Fl_Image_Surface *surf = Fl_Wayland_Graphics_Driver::custom_offscreen(width * scale, height * scale, &off);
  Fl_Surface_Device::push_current(surf);
  cairo_scale(off->draw_buffer.cairo_, r * scale, r * scale);
  surf->draw(dockable_);
  Fl_Surface_Device::pop_current();
  delete surf;
  cairo_surface_flush( cairo_get_target(off->draw_buffer.cairo_) );
  memcpy(off->data, off->draw_buffer.buffer, off->draw_buffer.data_size);
  return off;
}


int Fl_oldWayland_Dockable_Group_Driver::handle(Fl_Dockable_Group_Driver::cmd_box_class *box, int event) {
  static int drag_count;
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  Fl_Dockable_Group *dock = (Fl_Dockable_Group*)box->parent();
  if ((event != FL_PUSH && event != FL_DRAG) || !(Fl::event_state() & FL_BUTTON1) || Fl::belowmouse() != box)
    return box->Fl_Box::handle(event);
  if (event == FL_PUSH && dock->state == Fl_Dockable_Group::UNDOCK) {
    drag_count = 0;
    return 1;
  } else if (event == FL_DRAG && dock->state == Fl_Dockable_Group::UNDOCK && ++drag_count < 3) {
    return 1;
  } else if (event == FL_DRAG && dock->state == Fl_Dockable_Group::UNDOCK && drag_count >= 3) {
    drag_count = 0;
    struct wld_window *xid = fl_wl_xid(dock->window());
    state(Fl_Dockable_Group::DRAG);
    scr_driver->seat->data_source = wl_data_device_manager_create_data_source(scr_driver->seat->data_device_manager);
    wl_data_source_add_listener(scr_driver->seat->data_source, xdg_toplevel_drag_data_source_listener(), dock);
    wl_data_source_offer(scr_driver->seat->data_source, Fl_Wayland_Screen_Driver::xdg_toplevel_drag_pseudo_mime);
    wl_data_source_set_actions(scr_driver->seat->data_source, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
    *Fl_Wayland_Screen_Driver::fl_dnd_icon = wl_compositor_create_surface(scr_driver->wl_compositor);
    //printf("start_drag surface=%p serial=%u\n",xid->wl_surface, scr_driver->seat->serial);
    wl_data_device_start_drag(scr_driver->seat->data_device, scr_driver->seat->data_source,
                              xid->wl_surface, *Fl_Wayland_Screen_Driver::fl_dnd_icon, scr_driver->seat->serial);
    int s = Fl_Wayland_Window_Driver::driver(box->top_window())->wld_scale();
    struct Fl_Wayland_Graphics_Driver::wld_buffer *off = offscreen_from_group_(s);
    double r = double(off->draw_buffer.width) / (dock->w() * s);
    wl_surface_attach(*Fl_Wayland_Screen_Driver::fl_dnd_icon, off->wl_buffer,
                      (dock->x() - Fl::event_x()) * r, (dock->y() - Fl::event_y()) * r );
    wl_surface_set_buffer_scale(*Fl_Wayland_Screen_Driver::fl_dnd_icon, s);
    wl_surface_damage(*Fl_Wayland_Screen_Driver::fl_dnd_icon, 0, 0, 10000, 10000);
    wl_surface_commit(*Fl_Wayland_Screen_Driver::fl_dnd_icon);
    wl_surface_set_user_data(*Fl_Wayland_Screen_Driver::fl_dnd_icon, off);
    Fl_Dockable_Group::active_dockable = dock;
    state(Fl_Dockable_Group::DRAGGED);
    return 1;
  }
  return box->Fl_Box::handle(event);
}
