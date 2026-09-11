//
// Wayland-specific dockable window code for the Fast Light Tool Kit (FLTK).
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
#include <map>


class Fl_Wayland_Dockable_Group_Driver : public Fl_Dockable_Group_Driver {
private:
#if HAVE_XDG_TOPLEVEL_DRAG
  struct xdg_toplevel_drag_v1 *drag_;
  int old_keyboard_screen_scaling_;
#endif
  void after_release_() override;
  void check_event_(int& event) override;
  int handle_drag_widget_(int event) override;
public:
  Fl_Wayland_Dockable_Group_Driver(Fl_Dockable_Group *from);
  // Used only without xdg-toplevel protocol.
  // Parent group of the dragged Fl_Dockable_Group, or NULL if started in undocked state.
  static Fl_Group *origin_group;
  void before_dock() override;
  int start_interactive_resize(Fl_Window *win, Fl_Cursor cursor) override;
};


Fl_Group *Fl_Wayland_Dockable_Group_Driver::origin_group = NULL;


// This class is used when the compositor doesn't support protocol 'XDG toplevel drag'.
// An image of the Fl_Dockable_Group object is used as the DnD cursor icon.
class Fl_oldWayland_Dockable_Group_Driver : public Fl_Wayland_Dockable_Group_Driver {
private:
  struct Fl_Wayland_Graphics_Driver::wld_buffer *offscreen_from_group_(int scale);
  int handle_drag_widget_(int event) override;
public:
  Fl_oldWayland_Dockable_Group_Driver(Fl_Dockable_Group *from) : Fl_Wayland_Dockable_Group_Driver(from) {}
};


Fl_Wayland_Dockable_Group_Driver::Fl_Wayland_Dockable_Group_Driver(Fl_Dockable_Group *from) :
  Fl_Dockable_Group_Driver(from) {
#if HAVE_XDG_TOPLEVEL_DRAG
  drag_ = NULL;
  old_keyboard_screen_scaling_ = 0;
#endif
}


Fl_Dockable_Group_Driver *Fl_Dockable_Group_Driver::newDockableGroupDriver(Fl_Dockable_Group *dock) {
  fl_open_display();
  if (fl_wl_display()) {
    using_wayland = true;
    Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
    return ( scr_driver->xdg_toplevel_drag ? new Fl_Wayland_Dockable_Group_Driver(dock) :
               new Fl_oldWayland_Dockable_Group_Driver(dock) );
  } else return new Fl_Dockable_Group_Driver(dock);
}


void Fl_Wayland_Dockable_Group_Driver::after_release_() {
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
#if HAVE_XDG_TOPLEVEL_DRAG
  if (drag_ && scr_driver->xdg_toplevel_drag) {
    xdg_toplevel_drag_v1_destroy(drag_);
    drag_ = NULL;
  }
#endif
  if (scr_driver->seat->data_source) wl_data_source_set_user_data(scr_driver->seat->data_source, NULL);
}


static void xdg_toplevel_drag_data_source_handle_cancelled(void *data, struct wl_data_source *source) {
  Fl_Dockable_Group *dock = (Fl_Dockable_Group*)data;
  if (dock) {
    enum Fl_Dockable_Group::states s = Fl_Dockable_Group::UNDOCK;
    if ( ((Fl_Wayland_Screen_Driver*)Fl::screen_driver())->xdg_toplevel_drag == NULL &&
        !Fl_Wayland_Dockable_Group_Driver::origin_group) s = Fl_Dockable_Group::DRAG;
    Fl_Dockable_Group_Driver::driver(dock)->state(s);
    Fl_Dockable_Group_Driver::active_dockable(NULL);
  }
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


static int get_gnome_version() {
  static int version = 0;
  if (!version) {
    FILE *in = popen("gnome-shell --version", "r");
    if (in) {
      char line[100], *p;
      p = fgets(line, sizeof(line), in);
      if (p) { // we got "GNOME Shell xx.y\n"
        p = strchr(p, ' ') + 1;
        p = strchr(p, ' ') + 1;
        sscanf(p, "%d", &version);
      }
      pclose(in);
    }
  }
  return version;
}


int Fl_Wayland_Dockable_Group_Driver::handle_drag_widget_(int event) {
#if HAVE_XDG_TOPLEVEL_DRAG
  static int drag_count;
  if ((event != FL_PUSH && event != FL_DRAG) || !(Fl::event_state() & FL_BUTTON1))
    { return 0; }
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  if (event == FL_PUSH && dockable_->state() == Fl_Dockable_Group::UNDOCK) {
    drag_count = 0;
  } else if (event == FL_DRAG && dockable_->state() == Fl_Dockable_Group::UNDOCK && ++drag_count >= 3) {
    Fl_Window *top = dockable_->top_window();
    // It seems that while MUTTER accepts to apply the xdg_toplevel_drag protocol
    // to a subwindow, KWIN doesn't accept it and works OK only when dragging inside a toplevel.
    struct wld_window *xid = fl_wl_xid(dockable_->window());
    scr_driver->seat->data_source = wl_data_device_manager_create_data_source(scr_driver->seat->data_device_manager);
    wl_data_source_add_listener(scr_driver->seat->data_source, xdg_toplevel_drag_data_source_listener(), (void*)0);
    wl_data_source_offer(scr_driver->seat->data_source, Fl_Wayland_Screen_Driver::xdg_toplevel_drag_pseudo_mime);
    wl_data_source_set_actions(scr_driver->seat->data_source, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
    drag_ =
    xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(scr_driver->xdg_toplevel_drag, scr_driver->seat->data_source);
    //printf("start_drag surface=%p serial=%u\n",xid->wl_surface, scr_driver->seat->serial);
    wl_data_device_start_drag(scr_driver->seat->data_device, scr_driver->seat->data_source,
                              xid->wl_surface, NULL, scr_driver->seat->serial);
    int dock_x = dockable_->x(), dock_y = dockable_->y();
    Fl_Group *dock_parent = dockable_->parent();
    Fl_Window *new_win = undock();
    new_win->show();
    xid = fl_wl_xid(new_win);
    xdg_toplevel_set_parent(xid->xdg_toplevel, Fl_Wayland_Window_Driver::driver(top)->xdg_toplevel());
    float s = Fl::screen_scale(top->screen_num());
    // Here, we need to know whether the running gnome version is >= 49
    if (Fl_Wayland_Screen_Driver::compositor == Fl_Wayland_Screen_Driver::MUTTER &&
        get_gnome_version() >= 49) s *= Fl_Wayland_Window_Driver::driver(top)->wld_scale();
    xdg_toplevel_drag_v1_attach(drag_, xid->xdg_toplevel,
                                (Fl::event_x() - dock_x) * s, (Fl::event_y() - dock_y) * s);
    //printf("xdg_toplevel_drag_v1_attach to toplevel=%p\n",xid->xdg_toplevel);
    active_dockable(dockable_);
    old_keyboard_screen_scaling_ = Fl_Screen_Driver::keyboard_screen_scaling;
    Fl::keyboard_screen_scaling(0); // turn off dynamic GUI scaling
    dock_parent->handle(FL_UNDOCK);
  } else if (event == FL_PUSH && dockable_->state() == Fl_Dockable_Group::DRAG) {
    // catch again a draggable window
    if (drag_) xdg_toplevel_drag_v1_destroy(drag_);
    scr_driver->seat->data_source = wl_data_device_manager_create_data_source(scr_driver->seat->data_device_manager);
    wl_data_source_add_listener(scr_driver->seat->data_source, xdg_toplevel_drag_data_source_listener(), (void*)0);
    wl_data_source_offer(scr_driver->seat->data_source, Fl_Wayland_Screen_Driver::xdg_toplevel_drag_pseudo_mime);
    wl_data_source_set_actions(scr_driver->seat->data_source, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
    drag_ = xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(scr_driver->xdg_toplevel_drag, scr_driver->seat->data_source);
    struct wld_window *xid = fl_wl_xid(dockable_->window());
    //printf("start_drag surface=%p serial=%u\n",scr_driver->seat->pointer_focus, scr_driver->seat->serial);
    wl_data_device_start_drag(scr_driver->seat->data_device, scr_driver->seat->data_source,
                              scr_driver->seat->pointer_focus, NULL, scr_driver->seat->serial);
    //printf("xdg_toplevel_drag_v1_attach to toplevel=%p\n",xid->xdg_toplevel);
    // need to attach AFTER start_drag even though xdg_toplevel_drag protocol doc says opposite!
    float s = Fl::screen_scale(dockable_->window()->screen_num());
    xdg_toplevel_drag_v1_attach(drag_, xid->xdg_toplevel, Fl::event_x() * s, Fl::event_y() * s);
    active_dockable(dockable_);
  }
  return 1;
#endif // HAVE_XDG_TOPLEVEL_DRAG
  return 0;
}


void Fl_Wayland_Dockable_Group_Driver::check_event_(int& event) {
  if (Fl::clipboard_contains(Fl_Wayland_Screen_Driver::xdg_toplevel_drag_pseudo_mime)) {
    if (event == FL_DND_ENTER) event = FL_DOCK_ENTER;
    else if (event == FL_DND_DRAG) event = FL_DOCK_DRAG;
    else if (event == FL_DND_LEAVE) event = FL_DOCK_LEAVE;
    else if (event == FL_DND_RELEASE) event = FL_DOCK_RELEASE;
  }
}


void Fl_Wayland_Dockable_Group_Driver::before_dock() {
  if (((Fl_Wayland_Screen_Driver*)Fl::screen_driver())->xdg_toplevel_drag) {
    Fl_Dockable_Group_Driver::before_dock();
  } else {
    Fl_Window *old_win = dockable_->window();
    undock();
    if (origin_group) origin_group->handle(FL_UNDOCK);
    else if (dockable_->state() == Fl_Dockable_Group::DRAG) delete old_win;
  }
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


int Fl_oldWayland_Dockable_Group_Driver::handle_drag_widget_(int event) {
  static int drag_count;
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  Fl_Dockable_Group *dock = dockable_;
  if ((event != FL_PUSH && event != FL_DRAG) || !(Fl::event_state() & FL_BUTTON1) || Fl::belowmouse() != dockable_->drag_widget())
    { return 0; }
  if (event == FL_PUSH && (dockable_->state() == Fl_Dockable_Group::UNDOCK || dockable_->state() == Fl_Dockable_Group::DRAG)) {
    drag_count = 0;
    return 1;
  } else if (event == FL_DRAG &&
             (dockable_->state() == Fl_Dockable_Group::UNDOCK || dockable_->state() == Fl_Dockable_Group::DRAG)
             && ++drag_count < 3) {
    return 1;
  } else if (event == FL_DRAG &&
             (dockable_->state() == Fl_Dockable_Group::UNDOCK || dockable_->state() == Fl_Dockable_Group::DRAG)
             && drag_count >= 3) {
    drag_count = 0;
    struct wld_window *xid = fl_wl_xid(dockable_->window());
    scr_driver->seat->data_source = wl_data_device_manager_create_data_source(scr_driver->seat->data_device_manager);
    wl_data_source_add_listener(scr_driver->seat->data_source, xdg_toplevel_drag_data_source_listener(), dockable_);
    wl_data_source_offer(scr_driver->seat->data_source, Fl_Wayland_Screen_Driver::xdg_toplevel_drag_pseudo_mime);
    wl_data_source_set_actions(scr_driver->seat->data_source, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
    *Fl_Wayland_Screen_Driver::fl_dnd_icon = wl_compositor_create_surface(scr_driver->wl_compositor);
    //printf("start_drag surface=%p serial=%u\n",xid->wl_surface, scr_driver->seat->serial);
    wl_data_device_start_drag(scr_driver->seat->data_device, scr_driver->seat->data_source,
                              xid->wl_surface, *Fl_Wayland_Screen_Driver::fl_dnd_icon, scr_driver->seat->serial);
    int s = Fl_Wayland_Window_Driver::driver(dockable_->top_window())->wld_scale();
    origin_group = (dockable_->state() == Fl_Dockable_Group::UNDOCK ? dockable_->parent() : NULL);
    state(Fl_Dockable_Group::DRAG);
    struct Fl_Wayland_Graphics_Driver::wld_buffer *off = offscreen_from_group_(s);
    double r = double(off->draw_buffer.width) / (dockable_->w() * s);
    wl_surface_attach(*Fl_Wayland_Screen_Driver::fl_dnd_icon, off->wl_buffer,
                      (dockable_->x() - Fl::event_x()) * r, (dockable_->y() - Fl::event_y()) * r );
    wl_surface_set_buffer_scale(*Fl_Wayland_Screen_Driver::fl_dnd_icon, s);
    wl_surface_damage(*Fl_Wayland_Screen_Driver::fl_dnd_icon, 0, 0, 10000, 10000);
    wl_surface_commit(*Fl_Wayland_Screen_Driver::fl_dnd_icon);
    wl_surface_set_user_data(*Fl_Wayland_Screen_Driver::fl_dnd_icon, off);
    active_dockable(dockable_);
    return 1;
  }
  return 0;
}


int Fl_Wayland_Dockable_Group_Driver::start_interactive_resize(Fl_Window *win, Fl_Cursor cursor) {
  static std::map<int, int> cursor_to_edge_map = {
    {FL_CURSOR_N, XDG_TOPLEVEL_RESIZE_EDGE_TOP },
    {FL_CURSOR_E, XDG_TOPLEVEL_RESIZE_EDGE_RIGHT },
    {FL_CURSOR_S, XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM },
    {FL_CURSOR_W, XDG_TOPLEVEL_RESIZE_EDGE_LEFT },
    {FL_CURSOR_NESW, XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT },
    {FL_CURSOR_NWSE, XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT },
  };

  int edge = cursor_to_edge_map[cursor];
  if (cursor == FL_CURSOR_NESW && Fl::event_x() < 4) edge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
  if (cursor == FL_CURSOR_NWSE && Fl::event_x() < 4) edge = XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  xdg_toplevel_resize(fl_wl_xid(win)->xdg_toplevel, scr_driver->seat->wl_seat,
                      scr_driver->seat->serial, edge);
  Fl::pushed(0); // important
  return 1;
}
