//
// Wayland-specific code for clipboard and drag-n-drop support.
//
// Copyright 1998-2026 by Bill Spitzak and others.
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

#if !defined(FL_DOXYGEN)

#  include <FL/Fl.H>
#  include <FL/platform.H>
#  include <FL/Fl_Window.H>
#  include <FL/Fl_PNG_Image.H>
#  include <FL/Fl_Image_Surface.H>
#  include <FL/Fl_Drag_Data.H>
#  include "Fl_Wayland_Screen_Driver.H"
#  include "Fl_Wayland_Window_Driver.H"
#  include "../Unix/Fl_Unix_System_Driver.H"
#  include "Fl_Wayland_Graphics_Driver.H"
#if HAVE_PRIMARY_SELECTION
#  include "primary-selection-client-protocol.h"
#endif

#  include <errno.h>
#  include <stdio.h>
#  include <stdlib.h>
#  include <map>
#  include <string>
#  include <vector>
#  include <algorithm>


////////////////////////////////////////////////////////////////
// Code used for copy and paste and DnD into the program:

static std::string selection_string[2];
static const char * fl_selection_type[2];
static char fl_i_own_selection[2] = {0,0};
static struct wl_data_offer *fl_selection_offer = NULL;
// The MIME type Wayland uses for text-containing clipboard:
static const char wld_plain_text_clipboard[] = "text/plain;charset=utf-8";


int Fl_Wayland_Screen_Driver::clipboard_contains(const char *type)
{
  return fl_selection_type[1] == type;
}


struct data_source_write_struct {
  size_t rest;
  const char *from;
};

// Value of the listener data of the source of a drag started by Fl::dnd(Fl_Drag_Data*).
// Values 0 and 1 designate a text drag and the clipboard, respectively.
static const fl_intptr_t typed_dnd_rank = 2;
// MIME types and data of the latest drag started by Fl::dnd(Fl_Drag_Data*)
static Fl_Unix_Screen_Driver::Dnd_Offers typed_dnd_offers;
// what a widget of this program receives when this drag is dropped on it
static std::string typed_dnd_local_text;
static Fl_Drag_Data *typed_dnd_data = NULL; // non-NULL while the drag is in progress
static bool typed_dnd_dropped = false;

static void write_data_source_cb(FL_SOCKET fd, data_source_write_struct *data) {
  while (data->rest) {
    ssize_t n = write(fd, data->from, data->rest);
    if (n == -1) {
      if (errno == EAGAIN) return;
      Fl::error("write_data_source_cb: error while writing clipboard data\n");
      break;
    }
    data->from += n;
    data->rest -= n;
  }
  Fl::remove_fd(fd, FL_WRITE);
  delete data;
  close(fd);
}


static void data_source_handle_send(void *data, struct wl_data_source *source,
                                    const char *mime_type, int fd) {
  fl_intptr_t rank = (fl_intptr_t)data;
  if (rank == typed_dnd_rank) {
    for (size_t i = 0; i < typed_dnd_offers.size(); i++) {
      if (typed_dnd_offers[i].first == mime_type) {
        data_source_write_struct *write_data = new data_source_write_struct;
        write_data->rest = typed_dnd_offers[i].second.length();
        write_data->from = typed_dnd_offers[i].second.data();
        Fl::add_fd(fd, FL_WRITE, (Fl_FD_Handler)write_data_source_cb, write_data);
        return;
      }
    }
    close(fd);
    return;
  }
//printf("data_source_handle_send: %s fd=%d l=%lu\n", mime_type, fd, selection_string[rank].length());
  if (((!strcmp(mime_type, wld_plain_text_clipboard) || !strcmp(mime_type, "text/plain")) &&
       fl_selection_type[rank] == Fl::clipboard_plain_text)
      ||
    (!strcmp(mime_type, "image/bmp") && fl_selection_type[rank] == Fl::clipboard_image) ) {
    data_source_write_struct *write_data = new data_source_write_struct;
    write_data->rest = selection_string[rank].length();
    write_data->from = selection_string[rank].c_str();
    Fl::add_fd(fd, FL_WRITE, (Fl_FD_Handler)write_data_source_cb, write_data);
  } else {
    //Fl::error("Destination client requested unsupported MIME type: %s\n", mime_type);
    close(fd);
  }
}


static Fl_Window *fl_dnd_target_window = 0;
static wl_surface *fl_dnd_target_surface = 0;
static bool doing_dnd = false; // true when DnD is in action
static wl_surface *dnd_icon = NULL; // non null when DnD uses text as cursor


static uint32_t last_dnd_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;


static void data_source_handle_cancelled(void *data, struct wl_data_source *source) {
  // An application has replaced the clipboard contents or DnD finished
  if ((fl_intptr_t)data == typed_dnd_rank && typed_dnd_data) {
    int action = FL_DND_NONE;
    if (typed_dnd_dropped) {
      action = (last_dnd_action == WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE ? FL_DND_MOVE : FL_DND_COPY);
    }
    Fl_Drag_Data *d = typed_dnd_data;
    typed_dnd_data = NULL;
    d->do_callback(action);
    delete d;
  }
  wl_data_source_destroy(source);
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  if (scr_driver->seat->data_source == source) scr_driver->seat->data_source = NULL;
  doing_dnd = false;
  if (dnd_icon) {
    struct Fl_Wayland_Graphics_Driver::wld_buffer *off =
      (struct Fl_Wayland_Graphics_Driver::wld_buffer *)
      wl_surface_get_user_data(dnd_icon);
    struct wld_window fake_window;
    memset(&fake_window, 0, sizeof(fake_window));
    fake_window.buffer = off;
    Fl_Wayland_Graphics_Driver::buffer_release(&fake_window);
    wl_surface_destroy(dnd_icon);
    dnd_icon = NULL;
  }
  fl_i_own_selection[1] = 0;
  if ((fl_intptr_t)data != 1) { // at end of DnD
    if (fl_dnd_target_window) {
      Fl::handle(FL_RELEASE, fl_dnd_target_window);
      fl_dnd_target_window = 0;
    }
    Fl::pushed(0);
  }
}


static void data_source_handle_target(void *data, struct wl_data_source *source, const char *mime_type) {
  if (!Fl::pushed()) {
    data_source_handle_cancelled(data, source);
    return;
  }
  if (mime_type != NULL) {
    //printf("Destination would accept MIME type if dropped: %s\n", mime_type);
  } else {
    //printf("Destination would reject if dropped\n");
  }
}


static void data_source_handle_action(void *data, struct wl_data_source *source,
                                      uint32_t dnd_action) {
  last_dnd_action = dnd_action;
  switch (dnd_action) {
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY:
    //printf("Destination would perform a copy action if dropped\n");
    break;
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE:
    //printf("Destination would reject the drag if dropped\n");
    break;
  }
}


static void data_source_handle_dnd_drop_performed(void *data, struct wl_data_source *source) {
  //printf("Drop performed\n");
  if ((fl_intptr_t)data == typed_dnd_rank) typed_dnd_dropped = true;
}


static void data_source_handle_dnd_finished(void *data, struct wl_data_source *source) {
  if ((fl_intptr_t)data == typed_dnd_rank) {
    // the source is not cancelled after a successful drop: report the action and clean up
    typed_dnd_dropped = true;
    data_source_handle_cancelled(data, source);
    return;
  }
  switch (last_dnd_action) {
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE:
    //printf("Destination has accepted the drop with a move action\n");
    break;
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY:
    //printf("Destination has accepted the drop with a copy action\n");
    break;
  }
}


static const struct wl_data_source_listener data_source_listener = {
  .target = data_source_handle_target,
  .send = data_source_handle_send,
  .cancelled = data_source_handle_cancelled,
  .dnd_drop_performed = data_source_handle_dnd_drop_performed,
  .dnd_finished = data_source_handle_dnd_finished,
  .action = data_source_handle_action,
};


static struct Fl_Wayland_Graphics_Driver::wld_buffer *offscreen_from_text(const char *text,
                                                                          int scale) {
  const char *p, *q;
  int width = 0, height, w2, ltext = strlen(text);
  fl_font(FL_HELVETICA, 10 * scale);
  p = text;
  int nl = 0;
  while(nl < 20 && (q=strchr(p, '\n')) != NULL) {
    nl++;
    w2 = int(fl_width(p, q - p));
    if (w2 > width) width = w2;
    p = q + 1;
  }
  if (nl < 20 && text[ ltext - 1] != '\n') {
    nl++;
    w2 = int(fl_width(p));
    if (w2 > width) width = w2;
  }
  if (width > 300*scale) width = 300*scale;
  height = nl * fl_height() + 3;
  width += 6;
  width = ceil(width/float(scale)) * scale; // these must be multiples of scale
  height = ceil(height/float(scale)) * scale;
  struct Fl_Wayland_Graphics_Driver::wld_buffer *off;
  Fl_Image_Surface *surf = Fl_Wayland_Graphics_Driver::custom_offscreen(
      width, height, &off);
  Fl_Surface_Device::push_current(surf);
  p = text;
  fl_font(FL_HELVETICA, 10 * scale);
  int y = fl_height();
  while (nl > 0) {
    q = strchr(p, '\n');
    if (q) {
      fl_draw(p, q - p, 3, y);
    } else {
      fl_draw(p, 3, y);
      break;
    }
    y += fl_height();
    p = q + 1;
    nl--;
  }
  Fl_Surface_Device::pop_current();
  delete surf;
  cairo_surface_flush( cairo_get_target(off->draw_buffer.cairo_) );
  memcpy(off->data, off->draw_buffer.buffer, off->draw_buffer.data_size);
  return off;
}


int Fl_Wayland_Screen_Driver::dnd(int use_selection) {
  struct wl_data_source *source = wl_data_device_manager_create_data_source(seat->data_device_manager);
  // we transmit the adequate value of index in selection_string[index]
  wl_data_source_add_listener(source, &data_source_listener, (void*)0);
  // Firefox seems to require mime-type "text/plain" when doing text DnD both ways;
  // therefore offer both mime-types.
  wl_data_source_offer(source, wld_plain_text_clipboard);
  wl_data_source_offer(source, "text/plain");
  wl_data_source_set_actions(source, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
  struct Fl_Wayland_Graphics_Driver::wld_buffer *off = NULL;
  int s = 1;
  if (use_selection) { // use the text as dragging icon
    Fl_Widget *current = Fl::pushed() ? Fl::pushed() : Fl::first_window();
    s = Fl_Wayland_Window_Driver::driver(current->top_window())->wld_scale();
    off = (struct Fl_Wayland_Graphics_Driver::wld_buffer *)
                  offscreen_from_text(selection_string[0].c_str(), s);
    dnd_icon = wl_compositor_create_surface(wl_compositor);
  } else dnd_icon = NULL;
  doing_dnd = true;
  wl_data_device_start_drag(seat->data_device, source, seat->pointer_focus, dnd_icon, seat->serial);
  if (use_selection) {
    wl_surface_attach(dnd_icon, off->wl_buffer, 0, 0);
    wl_surface_set_buffer_scale(dnd_icon, s);
    wl_surface_damage(dnd_icon, 0, 0, 10000, 10000);
    wl_surface_commit(dnd_icon);
    wl_surface_set_user_data(dnd_icon, off);
  }
  return 1;
}


// Returns a buffer showing img with a size of W x H FLTK units at the given scale
static struct Fl_Wayland_Graphics_Driver::wld_buffer *offscreen_from_image(const Fl_RGB_Image *img,
                                                                       int W, int H, int scale) {
  struct Fl_Wayland_Graphics_Driver::wld_buffer *off;
  Fl_Image_Surface *surf = Fl_Wayland_Graphics_Driver::custom_offscreen(W * scale, H * scale, &off);
  Fl_Surface_Device::push_current(surf);
  Fl_RGB_Image *copy = (Fl_RGB_Image *)img->copy(W * scale, H * scale);
  copy->draw(0, 0);
  delete copy;
  Fl_Surface_Device::pop_current();
  delete surf;
  cairo_surface_flush( cairo_get_target(off->draw_buffer.cairo_) );
  memcpy(off->data, off->draw_buffer.buffer, off->draw_buffer.data_size);
  return off;
}


int Fl_Wayland_Screen_Driver::dnd_data(Fl_Drag_Data *data) {
  if (doing_dnd || !seat->pointer_focus) {
    data->do_callback(FL_DND_NONE);
    delete data;
    return 0;
  }
  dnd_offers(data, typed_dnd_offers, typed_dnd_local_text);
  if (typed_dnd_offers.empty()) {
    data->do_callback(FL_DND_NONE);
    delete data;
    return 0;
  }
  typed_dnd_data = data;
  typed_dnd_dropped = false;
  last_dnd_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
  struct wl_data_source *source = wl_data_device_manager_create_data_source(seat->data_device_manager);
  wl_data_source_add_listener(source, &data_source_listener, (void*)typed_dnd_rank);
  for (size_t i = 0; i < typed_dnd_offers.size(); i++) {
    wl_data_source_offer(source, typed_dnd_offers[i].first.c_str());
  }
  uint32_t actions = 0;
  if (data->actions() & FL_DND_COPY) actions |= WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
  if (data->actions() & FL_DND_MOVE) actions |= WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE;
  if (!actions) actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
  wl_data_source_set_actions(source, actions);
  // show the drag image, or a thumbnail of a dragged image
  const Fl_RGB_Image *icon = data->drag_image() ? data->drag_image() : data->image();
  struct Fl_Wayland_Graphics_Driver::wld_buffer *off = NULL;
  int s = 1, hot_x = 0, hot_y = 0;
  dnd_icon = NULL;
  if (icon && icon->array && icon->w() > 0 && icon->h() > 0) {
    Fl_Widget *current = Fl::pushed() ? Fl::pushed() : Fl::first_window();
    s = Fl_Wayland_Window_Driver::driver(current->top_window())->wld_scale();
    int W = icon->w(), H = icon->h();
    if (data->drag_image()) {
      hot_x = data->drag_image_hot_x();
      hot_y = data->drag_image_hot_y();
    } else {
      int m = (W > H ? W : H);
      if (m > 128) { W = W * 128 / m; H = H * 128 / m; }
      if (W < 1) W = 1;
      if (H < 1) H = 1;
      hot_x = W / 2;
      hot_y = H / 2;
    }
    off = offscreen_from_image(icon, W, H, s);
    dnd_icon = wl_compositor_create_surface(wl_compositor);
  }
  doing_dnd = true;
  wl_data_device_start_drag(seat->data_device, source, seat->pointer_focus, dnd_icon, seat->serial);
  if (dnd_icon) {
    // the hot spot: initially, the top left corner of the icon is at the pointer
    wl_surface_attach(dnd_icon, off->wl_buffer, -hot_x, -hot_y);
    wl_surface_set_buffer_scale(dnd_icon, s);
    wl_surface_damage(dnd_icon, 0, 0, 10000, 10000);
    wl_surface_commit(dnd_icon);
    wl_surface_set_user_data(dnd_icon, off);
  }
  return 1;
}


struct compare_utf8 { // used as key_comp member of following map object
    bool operator()(const char *a, const char *b) const { return strcmp(a, b) < 0; }
};

// map: for each clipboard mime-type FLTK has interest in, give FLTK clipboard type and priority.
// A mime-type with higher priority for same FLTK clipboard type is preferred.
// Two mime-types are given the same priority. The effect is that if a data offer proposes
// both of them, the first one proposed will be selected by the data offer receiver.
// That's key for DnD between FLTK and Firefox to be successful.
typedef struct { const char * const fltk_type; int priority; } type_prio_struct;
static std::map<const char * const, type_prio_struct, compare_utf8> clipboard_mimetypes_map  {
//  mime-type                  FLTK-clipboard-type        priority
  {"image/png",               {Fl::clipboard_image,       1} },
  {"image/bmp",               {Fl::clipboard_image,       2} },
  {"text/uri-list",           {Fl::clipboard_plain_text,  1} },
  {"UTF8_STRING",             {Fl::clipboard_plain_text,  2} },
  {"text/plain",              {Fl::clipboard_plain_text,  3} },
  {wld_plain_text_clipboard,  {Fl::clipboard_plain_text,  3} },
};

// map: for each FLTK-clipboard-type, give current preferred mime-type and priority
typedef struct { const char *mime_type; int priority; } mime_prio_struct;
static std::map<const char * const, mime_prio_struct> clipboard_kinds_map  {
//  FLTK-clipboard-type        current mime-type   current highest priority
  {Fl::clipboard_image,       {NULL,               0} },
  {Fl::clipboard_plain_text,  {NULL,               0} },
};


// all MIME types announced by each live data offer (clipboard and DnD)
static std::map<struct wl_data_offer *, std::vector<std::string> > offer_mime_types;


static bool offer_has_mime_type(struct wl_data_offer *offer, const char *mime_type) {
  std::map<struct wl_data_offer *, std::vector<std::string> >::iterator it =
    offer_mime_types.find(offer);
  if (it == offer_mime_types.end()) return false;
  return std::find(it->second.begin(), it->second.end(), mime_type) != it->second.end();
}


static void destroy_offer(struct wl_data_offer *offer) {
  offer_mime_types.erase(offer);
  wl_data_offer_destroy(offer);
}


static void data_offer_handle_offer(void *data, struct wl_data_offer *offer,
                                    const char *mime_type) {
  // runs when app becomes active once for each offered clipboard type
//fprintf(stderr, "Clipboard offer=%p supports MIME type: %s\n", offer, mime_type);
  offer_mime_types[offer].push_back(mime_type);
  std::map<const char*const, type_prio_struct, compare_utf8>::iterator iter_mime =
    clipboard_mimetypes_map.find(mime_type);
  if (iter_mime == clipboard_mimetypes_map.end()) return; // FLTK doesn't handle this mime_type
  std::map<const char*const, mime_prio_struct>::iterator iter_kind =
    clipboard_kinds_map.find(iter_mime->second.fltk_type);
  if (iter_mime->second.priority > iter_kind->second.priority) { // found mime-type with higher priority
    iter_kind->second.priority = iter_mime->second.priority;
    iter_kind->second.mime_type = iter_mime->first;
    fl_selection_type[1] = iter_kind->first;
//fprintf(stderr,"mime_type=%s priority=%d [%s]\n",iter_kind->second.mime_type, iter_kind->second.priority, fl_selection_type[1]);
  }
}


// actions allowed by the source of the latest drag offer, and action selected by the compositor
static uint32_t dnd_source_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
static uint32_t dnd_offer_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;

static int fltk_dnd_actions(uint32_t wl_actions) {
  int actions = 0;
  if (wl_actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY) actions |= FL_DND_COPY;
  if (wl_actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE) actions |= FL_DND_MOVE;
  return actions;
}

static uint32_t wayland_dnd_action(int action) {
  if (action == FL_DND_MOVE) return WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE;
  if (action == FL_DND_COPY) return WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
  return WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
}

// the action the compositor selected, taking into account the modifier keys
static int proposed_dnd_action() {
  int a = fltk_dnd_actions(dnd_offer_action);
  return a ? a : FL_DND_COPY;
}

// tells the compositor which actions a widget accepts
static void set_dnd_offer_actions(struct wl_data_offer *offer, int accepted) {
  uint32_t supported = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE, preferred = supported;
  if (accepted) {
    supported = dnd_source_actions & (WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY |
                                      WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE);
    if (!supported) supported = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
    if (Fl_Screen_Driver::dnd_chosen_action) { // the widget chose one action
      preferred = wayland_dnd_action(Fl_Screen_Driver::dnd_target_action(fltk_dnd_actions(supported)));
      supported = preferred;
    } else { // let the compositor select the action from the modifier keys
      preferred = (supported & WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY) ?
        WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY : WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE;
    }
  }
  wl_data_offer_set_actions(offer, supported, preferred);
}


static void data_offer_handle_source_actions(void *data, struct wl_data_offer *offer,
                                             uint32_t actions) {
  // sent for drag offers only, before they enter a surface
  dnd_source_actions = actions;
}


static void data_offer_handle_action(void *data, struct wl_data_offer *offer,
                                     uint32_t dnd_action) {
  dnd_offer_action = dnd_action;
  switch (dnd_action) {
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE:
    //printf("A move action would be performed if dropped\n");
    break;
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY:
    //printf("A copy action would be performed if dropped\n");
    break;
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE:
    //printf("The drag would be rejected if dropped\n");
    break;
  }
}


static const struct wl_data_offer_listener data_offer_listener = {
  .offer = data_offer_handle_offer,
  .source_actions = data_offer_handle_source_actions,
  .action = data_offer_handle_action,
};


static void data_device_handle_data_offer(void *data, struct wl_data_device *data_device,
                                          struct wl_data_offer *offer) {
  dnd_source_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY; // until the offer tells otherwise
  dnd_offer_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
  // An application has created a new data source
//fprintf(stderr, "data_device_handle_data_offer offer=%p\n", offer);
  fl_selection_type[1] = NULL;
  wl_data_offer_add_listener(offer, &data_offer_listener, NULL);
  // reset current best mime-type and priority
  std::map<const char*const, mime_prio_struct>::iterator iter = clipboard_kinds_map.begin();
  while (iter != clipboard_kinds_map.end()) {
    iter->second.mime_type = NULL;
    iter->second.priority = 0;
    iter++;
  }
}


static void data_device_handle_selection(void *data, struct wl_data_device *data_device,
                                         struct wl_data_offer *offer) {
  // An application has set the clipboard contents. W
//fprintf(stderr, "data_device_handle_selection\n");
  if (fl_selection_offer) destroy_offer(fl_selection_offer);
  fl_selection_offer = offer;
//if (offer == NULL) fprintf(stderr, "Clipboard is empty\n");
}


// Gets from the system the clipboard or dnd text and puts it in selection_string[1]
// which is enlarged if necessary.
static void get_clipboard_or_dragged_text(struct wl_data_offer *offer, const char *type = NULL) {
  int fds[2];
  if (pipe(fds)) return;
  // preferred mime-type for the text clipboard type
  if (!type) type = clipboard_kinds_map[Fl::clipboard_plain_text].mime_type;
  wl_data_offer_receive(offer, type, fds[1]);
  close(fds[1]);
  wl_display_flush(Fl_Wayland_Screen_Driver::wl_display);
  // read in selection_string[1]
  selection_string[1].clear();
  while (true) {
    char buffer[1025];
    ssize_t n = read(fds[0], buffer, sizeof(buffer) - 1);
    if (n <= 0) {
      close(fds[0]);
      break;
    }
    buffer[n] = 0;
    selection_string[1] += buffer;
  }
  if (strcmp(type, "text/uri-list") == 0) {
    selection_string[1] = Fl_Screen_Driver::decode_uri_list(selection_string[1].c_str(),
                                                            selection_string[1].length());
  }
  Fl::e_clipboard_type = Fl::clipboard_plain_text;
}


static struct wl_data_offer *current_drag_offer = NULL;
static uint32_t fl_dnd_serial;
// true while a drop is processed: receiving the data may run a nested
// wl_display_roundtrip() which dispatches the leave event that follows the drop
static bool dnd_dropping = false;


// MIME type FLTK accepts during a drag: dropped files are announced as text/uri-list,
// which is preferred so that all platforms deliver the same file list.
static const char *dnd_accepted_mime_type(struct wl_data_offer *offer) {
  static const char *types[] = { "text/uri-list", wld_plain_text_clipboard, "text/plain",
                                 "UTF8_STRING", "image/png", "image/bmp" };
  for (int i = 0; i < 6; i++) if (offer_has_mime_type(offer, types[i])) return types[i];
  return NULL;
}


static void data_device_handle_enter(void *data, struct wl_data_device *data_device,
                                     uint32_t serial, struct wl_surface *surface,
                                     wl_fixed_t x, wl_fixed_t y,
                                     struct wl_data_offer *offer) {
  Fl_Window *win = Fl_Wayland_Window_Driver::surface_to_window(surface);
//printf("Drag entered our surface %p(win=%p) at %dx%d\n", surface, win, wl_fixed_to_int(x), wl_fixed_to_int(y));
  if (win) {
    fl_dnd_target_surface = surface;
    float f = Fl::screen_scale(win->screen_num());
    Fl::e_x = wl_fixed_to_int(x) / f;
    Fl::e_y = wl_fixed_to_int(y) / f;
    while (win->parent()) {
      Fl::e_x += win->x();
      Fl::e_y += win->y();
      win = win->window();
    }
    fl_dnd_target_window = win;
    Fl::e_x_root = Fl::e_x + fl_dnd_target_window->x();
    Fl::e_y_root = Fl::e_y + fl_dnd_target_window->y();
    current_drag_offer = offer; // needed by Fl::dnd_contains()
    fl_dnd_serial = serial;
    Fl_Screen_Driver::dnd_target_reset();
    Fl::e_dnd_action = proposed_dnd_action();
    set_dnd_offer_actions(offer, Fl::handle(FL_DND_ENTER, fl_dnd_target_window));
  } else { // we enter a non-FLTK window (titlebar, shade)
    fl_dnd_target_window = NULL;
    set_dnd_offer_actions(offer, 1);
  }
}


static void data_device_handle_motion(void *data, struct wl_data_device *data_device,
                                      uint32_t time, wl_fixed_t x, wl_fixed_t y) {
  if (!current_drag_offer) return;
//printf("data_device_handle_motion fl_dnd_target_window=%p\n", fl_dnd_target_window);
  int ret = 0;
  if (fl_dnd_target_window) {
    float f = Fl::screen_scale(fl_dnd_target_window->screen_num());
    Fl_Window *win = Fl_Wayland_Window_Driver::surface_to_window(fl_dnd_target_surface);
    Fl::e_x = wl_fixed_to_int(x) / f;
    Fl::e_y = wl_fixed_to_int(y) / f;
    while (win->parent()) {
      Fl::e_x += win->x();
      Fl::e_y += win->y();
      win = win->window();
    }
    Fl::e_x_root = Fl::e_x + fl_dnd_target_window->x();
    Fl::e_y_root = Fl::e_y + fl_dnd_target_window->y();
    Fl::e_dnd_action = proposed_dnd_action();
    ret = Fl::handle(FL_DND_DRAG, fl_dnd_target_window);
    if (Fl::belowmouse()) Fl::belowmouse()->take_focus();
  }
  set_dnd_offer_actions(current_drag_offer, ret && (Fl::pushed() || !doing_dnd));
  wl_display_roundtrip(Fl_Wayland_Screen_Driver::wl_display);
  if (ret && current_drag_offer)
    wl_data_offer_accept(current_drag_offer, fl_dnd_serial, dnd_accepted_mime_type(current_drag_offer));
}


static void data_device_handle_leave(void *data, struct wl_data_device *data_device) {
  //printf("Drag left our surface\n");
  if (dnd_dropping) return; // the drop handler destroys the offer when done
  if (current_drag_offer) {
    Fl::handle(FL_DND_LEAVE, fl_dnd_target_window);
    // the offer is not valid anymore after a leave event
    destroy_offer(current_drag_offer);
    current_drag_offer = NULL;
  }
}


static int get_clipboard_image(struct wl_data_offer *offer, const char *type = NULL);


// MIME type of text that FLTK requests when text is dropped, or NULL
static const char *dnd_plain_text_mime_type(struct wl_data_offer *offer) {
  static const char *types[] = { wld_plain_text_clipboard, "text/plain", "UTF8_STRING" };
  for (int i = 0; i < 3; i++) if (offer_has_mime_type(offer, types[i])) return types[i];
  return NULL;
}


// MIME type of an image that FLTK requests when an image is dropped, or NULL
static const char *dnd_image_mime_type(struct wl_data_offer *offer) {
  if (offer_has_mime_type(offer, "image/png")) return "image/png";
  if (offer_has_mime_type(offer, "image/bmp")) return "image/bmp";
  return NULL;
}


int Fl_Wayland_Screen_Driver::dnd_contains(const char *type) {
  if (!current_drag_offer) return 0;
  if (!strcmp(type, Fl::clipboard_files)) return offer_has_mime_type(current_drag_offer, "text/uri-list");
  if (!strcmp(type, Fl::clipboard_plain_text)) return dnd_plain_text_mime_type(current_drag_offer) != NULL;
  if (!strcmp(type, Fl::clipboard_image)) return dnd_image_mime_type(current_drag_offer) != NULL;
  return offer_has_mime_type(current_drag_offer, type);
}


// Reads the data of a MIME type from an offer, keeping nul bytes
static void read_offer_data(struct wl_data_offer *offer, const char *mime_type, std::string &bytes) {
  bytes.clear();
  int fds[2];
  if (pipe(fds)) return;
  wl_data_offer_receive(offer, mime_type, fds[1]);
  close(fds[1]);
  wl_display_flush(Fl_Wayland_Screen_Driver::wl_display);
  char buffer[16384];
  ssize_t n;
  while ((n = read(fds[0], buffer, sizeof(buffer))) > 0) bytes.append(buffer, n);
  close(fds[0]);
}


// Returns the data of the drag of this program for a MIME type, or NULL
static const std::string *typed_dnd_offer(const char *mime_type) {
  for (size_t i = 0; i < typed_dnd_offers.size(); i++)
    if (typed_dnd_offers[i].first == mime_type) return &typed_dnd_offers[i].second;
  return NULL;
}


static void data_device_handle_drop(void *data, struct wl_data_device *data_device) {
  if (!current_drag_offer || dnd_dropping) return;
  struct wl_data_offer *offer = current_drag_offer;
  dnd_dropping = true;
  Fl::handle(FL_ENTER, fl_dnd_target_window); // useful to set the belowmouse widget
  Fl::e_dnd_action = proposed_dnd_action();
  int ret = Fl::handle(FL_DND_RELEASE, fl_dnd_target_window);
//printf("data_device_handle_drop ret=%d doing_dnd=%d\n", ret, doing_dnd);

  if (!ret || !Fl::belowmouse()) {
    destroy_offer(offer);
    if (current_drag_offer == offer) current_drag_offer = NULL;
    dnd_dropping = false;
    return;
  }

  // the action selected by the compositor is the one performed
  Fl::e_dnd_action = proposed_dnd_action();
  std::string text; // what FL_PASTE delivers, unless an image
  Fl_RGB_Image *image = NULL;
  const char *type;
  // the MIME type requested with Fl::dnd_request(), if not one of FLTK's types
  const char *raw = Fl_Screen_Driver::dnd_requests_raw() ?
                    Fl_Screen_Driver::dnd_requested_type.c_str() : NULL;
  if (doing_dnd && typed_dnd_data) { // a typed drag of this program
    const std::string *files = typed_dnd_offer("text/uri-list");
    const std::string *bmp = typed_dnd_offer("image/bmp");
    const std::string *utf8 = typed_dnd_offer("UTF8_STRING");
    const std::string *raw_data = raw ? typed_dnd_offer(raw) : NULL;
    type = Fl_Screen_Driver::dnd_delivery_type(files != NULL, bmp != NULL, utf8 != NULL,
                                               raw_data != NULL);
    if (raw_data && type == raw) text = *raw_data;
    else if (type == Fl::clipboard_image) image = Fl_Unix_System_Driver::own_bmp_to_RGB(bmp->c_str());
    else if (type == Fl::clipboard_plain_text) text = *utf8;
    else text = typed_dnd_local_text;
  } else if (doing_dnd) { // a text drag of this program
    type = Fl::clipboard_plain_text;
    text = selection_string[0];
  } else {
    const char *text_type = dnd_plain_text_mime_type(offer);
    const char *image_type = dnd_image_mime_type(offer);
    bool has_files = offer_has_mime_type(offer, "text/uri-list");
    bool has_raw = raw && offer_has_mime_type(offer, raw);
    type = Fl_Screen_Driver::dnd_delivery_type(has_files, image_type != NULL, text_type != NULL,
                                               has_raw);
    if (has_raw && type == raw) {
      read_offer_data(offer, raw, text);
    } else if (type == Fl::clipboard_image) {
      if (get_clipboard_image(offer, image_type) == 0)
        image = (Fl_RGB_Image *)Fl::e_clipboard_data;
    } else if (type) {
      get_clipboard_or_dragged_text(offer,
                                    type == Fl::clipboard_files ? "text/uri-list" : text_type);
      text = selection_string[1];
      if (type == Fl::clipboard_files) {
        // web links are also dropped as text/uri-list: deliver them as text
        size_t i = 0;
        while (i < text.length()) {
          if (text[i] != '/') { type = Fl::clipboard_plain_text; break; }
          size_t nl = text.find('\n', i);
          if (nl == std::string::npos) break;
          i = nl + 1;
        }
      }
    }
  }
  int old_event = Fl::e_number;
  if (image) {
    Fl::e_clipboard_data = image;
    Fl::e_clipboard_type = Fl::clipboard_image;
    Fl::e_text = (char *)"";
    Fl::e_length = 0;
    if (!Fl::belowmouse()->handle(Fl::e_number = FL_PASTE)) delete image;
    Fl::e_clipboard_data = NULL;
  } else if (type && type != Fl::clipboard_image) {
    Fl::e_text = (char *)text.c_str();
    Fl::e_length = (int)text.length();
    Fl::e_clipboard_type = type;
    Fl::belowmouse()->handle(Fl::e_number = FL_PASTE);
    Fl::e_text = (char *)"";
    Fl::e_length = 0;
  }
  Fl::e_number = old_event;

  // finishing an offer without a selected action is a protocol error
  if (dnd_offer_action != WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE)
    wl_data_offer_finish(offer);
  destroy_offer(offer);
  if (current_drag_offer == offer) current_drag_offer = NULL;
  dnd_dropping = false;
}


static const struct wl_data_device_listener data_device_listener = {
  .data_offer = data_device_handle_data_offer,
  .enter = data_device_handle_enter,
  .leave = data_device_handle_leave,
  .motion = data_device_handle_motion,
  .drop = data_device_handle_drop,
  .selection = data_device_handle_selection,
};


const struct wl_data_device_listener *Fl_Wayland_Screen_Driver::p_data_device_listener =
  &data_device_listener;


// Reads from the clipboard an image which can be in image/bmp or image/png MIME type.
// Returns 0 if OK, != 0 if error.
static int get_clipboard_image(struct wl_data_offer *offer, const char *type) {
  int fds[2];
  if (pipe(fds)) return 1;
  // preferred mime-type for the image clipboard type
  if (!type) type = clipboard_kinds_map[Fl::clipboard_image].mime_type;
  wl_data_offer_receive(offer, type, fds[1]);
  close(fds[1]);
  wl_display_roundtrip(Fl_Wayland_Screen_Driver::wl_display);
  if (strcmp(type, "image/png") == 0) {
    std::string png_data;
    while (true) {
      char buf[10000];
      ssize_t n = read(fds[0], buf, sizeof(buf));
      if (n <= 0) break;
      png_data.insert(png_data.end(), buf, buf + n);
    }
    close(fds[0]);
    static Fl_Load_PNG_Plugin *png_plugin = NULL;
    if (!png_plugin) {
      Fl_Plugin_Manager pm("loadpng.fltk.org");
      png_plugin = (Fl_Load_PNG_Plugin*)pm.plugin("implementation.loadpng.fltk.org");
      if (!png_plugin) return 1;
    }
    Fl::e_clipboard_data = png_plugin->png_load_from_memory(png_data.c_str(), (int)png_data.length());
    if (!Fl::e_clipboard_data) return 1;
  } else { // process image/bmp
    uchar buf[54];
    size_t rest = 1;
    char *bmp = NULL;
    ssize_t n = read(fds[0], buf, sizeof(buf)); // read size info of the BMP image
    if (n == sizeof(buf)) {
      int w, h; // size of the BMP image
      Fl_Unix_System_Driver::read_int(buf + 18, w);
      Fl_Unix_System_Driver::read_int(buf + 22, h);
      // the number of bytes per row of BMP image, rounded up to multiple of 4
      int R = ((3*w+3)/4) * 4;
      bmp = new char[R * h + 54];
      memcpy(bmp, buf, 54);
      char *from = bmp + 54;
      rest = R * h;
      while (rest) {
        ssize_t n = read(fds[0], from, rest);
        if (n <= 0) break;
        from += n;
        rest -= n;
      }
//fprintf(stderr, "get_clipboard_image: image/bmp %dx%d rest=%lu\n", w,h,rest);
    }
    close(fds[0]);
    if (!rest) Fl::e_clipboard_data = Fl_Unix_System_Driver::own_bmp_to_RGB(bmp);
    delete[] bmp;
    if (rest) return 1;
  }
  Fl::e_clipboard_type = Fl::clipboard_image;
  return 0;
}


#if HAVE_PRIMARY_SELECTION

static struct zwp_primary_selection_offer_v1 *primary_selection_offer = NULL;

static void primary_selection_source_handle_send(void *data,
                                                 struct zwp_primary_selection_source_v1 *source,
                                                 const char *mime_type, int fd) {
  data_source_write_struct *write_data = new data_source_write_struct;
  write_data->rest = selection_string[0].length();
  write_data->from = selection_string[0].c_str();
  Fl::add_fd(fd, FL_WRITE, (Fl_FD_Handler)write_data_source_cb, write_data);
}

static void primary_selection_source_handle_cancelled(void *, struct zwp_primary_selection_source_v1 *);

static const struct zwp_primary_selection_source_v1_listener primary_selection_source_listener = {
  .send = primary_selection_source_handle_send,
  .cancelled = primary_selection_source_handle_cancelled
};

static void primary_selection_source_handle_cancelled(void *data,
                                                      struct zwp_primary_selection_source_v1 *source) {
  zwp_primary_selection_source_v1_destroy(source);
  ((struct Fl_Wayland_Screen_Driver *)data)->primary_selection_init();
}


static void primary_selection_offer_handle_offer(void *data,
                                                 struct zwp_primary_selection_offer_v1 *offer,
                                                 const char *mime) {
}

static const struct zwp_primary_selection_offer_v1_listener primary_selection_offer_listener = {
  .offer = primary_selection_offer_handle_offer
};


static void primary_selection_device_handle_data_offer(void *data,
                                                 struct zwp_primary_selection_device_v1 *device,
                                                 struct zwp_primary_selection_offer_v1 *offer) {
  zwp_primary_selection_offer_v1_add_listener(offer, &primary_selection_offer_listener, NULL);
}

static void primary_selection_device_handle_selection(void *data,
                                                      struct zwp_primary_selection_device_v1 *device,
                                                      struct zwp_primary_selection_offer_v1 *offer) {
  if (primary_selection_offer) zwp_primary_selection_offer_v1_destroy(primary_selection_offer);
  primary_selection_offer = offer;
}

static const struct zwp_primary_selection_device_v1_listener primary_selection_device_listener = {
  .data_offer  = primary_selection_device_handle_data_offer,
  .selection = primary_selection_device_handle_selection
};

static std::string tmp_selection_string;

static void read_fd_cb(FL_SOCKET fd, bool *p_finished) {
  char buffer[1000];
  int l = read(fd, buffer, sizeof(buffer));
  if (l > 0) tmp_selection_string.insert(tmp_selection_string.end(), buffer, buffer + l);
  else {
    close(fd);
    Fl::remove_fd(fd, FL_READ);
    *p_finished = true;
  }
}


static void paste_primary_selection_offer() {
  int fds[2];
  pipe(fds);
  zwp_primary_selection_offer_v1_receive(primary_selection_offer, wld_plain_text_clipboard, fds[1]);
  close(fds[1]);
  tmp_selection_string.clear();
  bool finished = false;
  Fl::add_fd(fds[0], FL_READ, (Fl_FD_Handler)read_fd_cb, &finished);
  while (!finished) Fl::wait();
  selection_string[0] = tmp_selection_string;
  tmp_selection_string.clear();
}


/* called after both the primary-selection protocol and the seat have been received */
void Fl_Wayland_Screen_Driver::primary_selection_init() {
  if (!seat->primary_selection_device) {
    seat->primary_selection_device = zwp_primary_selection_device_manager_v1_get_device(seat->primary_selection_device_manager, seat->wl_seat);
    zwp_primary_selection_device_v1_add_listener(seat->primary_selection_device, &primary_selection_device_listener, &primary_selection_offer);
  }
  seat->primary_selection_source = zwp_primary_selection_device_manager_v1_create_source(seat->primary_selection_device_manager);
  zwp_primary_selection_source_v1_add_listener(seat->primary_selection_source, &primary_selection_source_listener, this);
}

#endif // HAVE_PRIMARY_SELECTION


void Fl_Wayland_Screen_Driver::paste(Fl_Widget &receiver, int clipboard, const char *type) {
  if (clipboard > 1) return;
  if (clipboard == 1 && fl_i_own_selection[1]) {
    // We already have it, do it quickly without compositor.
    if (type == Fl::clipboard_plain_text && fl_selection_type[1] == type) {
      Fl::e_text = (char*)selection_string[1].c_str();
      Fl::e_length = selection_string[1].length();
    } else if (type == Fl::clipboard_image && fl_selection_type[1] == type) {
      Fl::e_clipboard_data = Fl_Unix_System_Driver::own_bmp_to_RGB(selection_string[1].c_str());
      Fl::e_clipboard_type = Fl::clipboard_image;
    } else return;
    receiver.handle(FL_PASTE);
    return;
  }
  // otherwise get the compositor to return it:
  if (clipboard == 0) {
#if HAVE_PRIMARY_SELECTION
    if (primary_selection_offer) {
      paste_primary_selection_offer();
      Fl::e_text = (char*)selection_string[0].c_str();
      Fl::e_length = selection_string[0].length();
      fl_selection_type[0] = Fl::clipboard_plain_text;
      receiver.handle(FL_PASTE);
    }
#endif
    return;
  }
  if (!fl_selection_offer) return;
  if (type == Fl::clipboard_plain_text && clipboard_contains(Fl::clipboard_plain_text)) {
    get_clipboard_or_dragged_text(fl_selection_offer, wld_plain_text_clipboard);
    Fl::e_text = (char*)selection_string[1].c_str();
    Fl::e_length = selection_string[1].length();
    receiver.handle(FL_PASTE);
  } else if (type == Fl::clipboard_image && clipboard_contains(Fl::clipboard_image)) {
    if (get_clipboard_image(fl_selection_offer)) return;
    struct wld_window * xid = fl_wl_xid(receiver.top_window());
    if (xid) {
      int s = Fl_Wayland_Window_Driver::driver(receiver.top_window())->wld_scale();
      if ( s > 1) {
        Fl_RGB_Image *rgb = (Fl_RGB_Image*)Fl::e_clipboard_data;
        rgb->scale(rgb->data_w() / s, rgb->data_h() / s);
      }
    }
    int done = receiver.handle(FL_PASTE);
    Fl::e_clipboard_type = "";
    if (done == 0) {
      delete (Fl_RGB_Image*)Fl::e_clipboard_data;
      Fl::e_clipboard_data = NULL;
    }
  }
}


void Fl_Wayland_Screen_Driver::copy(const char *stuff, int len, int clipboard,
                                    const char *type) {
  if (!stuff || len < 0) return;
  if (clipboard == 0 && Fl::selection_to_clipboard())
    clipboard = 2;
  if (clipboard >= 2) {
    copy(stuff, len, 1, type);  // copy to clipboard first (this is a recursion!)
    clipboard = 0;              // ... and then to selection buffer: fall through
  }

  selection_string[clipboard].clear();
  selection_string[clipboard].insert(selection_string[clipboard].begin(), stuff, stuff + len);
  fl_i_own_selection[clipboard] = 1;
  fl_selection_type[clipboard] = Fl::clipboard_plain_text;
  if (clipboard == 1) {
    if (seat->data_source) wl_data_source_destroy(seat->data_source);
    seat->data_source = wl_data_device_manager_create_data_source(seat->data_device_manager);
    // we transmit the adequate value of index in selection_string[index]
    wl_data_source_add_listener(seat->data_source, &data_source_listener, (void*)1);
    wl_data_source_offer(seat->data_source, wld_plain_text_clipboard);
    wl_data_source_offer(seat->data_source, "text/plain");
    wl_data_device_set_selection(seat->data_device,
                                 seat->data_source,
                                 seat->keyboard_enter_serial);
//fprintf(stderr, "wl_data_device_set_selection len=%d to %d\n", len, clipboard);
#if HAVE_PRIMARY_SELECTION
  } else if (seat->primary_selection_device_manager) {
    zwp_primary_selection_device_v1_set_selection(seat->primary_selection_device, NULL, seat->serial);
    wl_display_roundtrip(wl_display);
    zwp_primary_selection_source_v1_offer(seat->primary_selection_source, wld_plain_text_clipboard);
    zwp_primary_selection_device_v1_set_selection(seat->primary_selection_device, seat->primary_selection_source, seat->serial);
#endif // HAVE_PRIMARY_SELECTION
  }
}


// takes a raw RGB image and puts it in the copy/paste buffer
void Fl_Wayland_Screen_Driver::copy_image(const unsigned char *data, int W, int H) {
  if (!data || W <= 0 || H <= 0) return;
  int l;
  char *stuff = (char*)Fl_Unix_System_Driver::create_bmp(data, W, H, &l);
  selection_string[1].clear();
  selection_string[1].insert(selection_string[1].begin(), stuff, stuff + l);
  delete[] stuff;
  fl_i_own_selection[1] = 1;
  fl_selection_type[1] = Fl::clipboard_image;
  if (seat->data_source) wl_data_source_destroy(seat->data_source);
  seat->data_source = wl_data_device_manager_create_data_source(seat->data_device_manager);
  // we transmit the adequate value of index in selection_string[index]
  wl_data_source_add_listener(seat->data_source, &data_source_listener, (void*)1);
  wl_data_source_offer(seat->data_source, "image/bmp");
  wl_data_device_set_selection(seat->data_device, seat->data_source,
                               seat->keyboard_enter_serial);
}

#endif // !defined(FL_DOXYGEN)
