//
// Wayland-specific code for clipboard and drag-n-drop support.
//
// Copyright 1998-2024 by Bill Spitzak and others.
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
#  include <FL/Fl_Shared_Image.H>
#  include <FL/Fl_Image_Surface.H>
#  include "Fl_Wayland_Screen_Driver.H"
#  include "Fl_Wayland_Window_Driver.H"
#  include "../Unix/Fl_Unix_System_Driver.H"
#  include "Fl_Wayland_Graphics_Driver.H"
#  include "../../flstring.h" // includes <string.h>

#  include <errno.h>
#  include <stdio.h>
#  include <stdlib.h>


////////////////////////////////////////////////////////////////
// Code used for copy and paste and DnD into the program:

static char *fl_selection_buffer[2];
static int fl_selection_length[2];
static const char * fl_selection_type[2];
static int fl_selection_buffer_length[2];
static char fl_i_own_selection[2] = {0,0};
static struct wl_data_offer *fl_selection_offer = NULL;
static const char *fl_selection_offer_type = NULL;
// The MIME type Wayland uses for text-containing clipboard:
static const char wld_plain_text_clipboard[] = "text/plain;charset=utf-8";


int Fl_Wayland_Screen_Driver::clipboard_contains(const char *type)
{
  return fl_selection_type[1] == type;
}


struct data_source_write_struct {
  size_t rest;
  char *from;
};

void write_data_source_cb(FL_SOCKET fd, data_source_write_struct *data) {
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
//fprintf(stderr, "data_source_handle_send: %s fd=%d l=%d\n", mime_type, fd, fl_selection_length[1]);
  if (((!strcmp(mime_type, wld_plain_text_clipboard) || !strcmp(mime_type, "text/plain")) &&
       fl_selection_type[rank] == Fl::clipboard_plain_text)
      ||
    (!strcmp(mime_type, "image/bmp") && fl_selection_type[rank] == Fl::clipboard_image) ) {
    data_source_write_struct *write_data = new data_source_write_struct;
    write_data->rest = fl_selection_length[rank];
    write_data->from = fl_selection_buffer[rank];
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
static wl_cursor* save_cursor = NULL; // non null when DnD uses "dnd-copy" cursor


static void data_source_handle_cancelled(void *data, struct wl_data_source *source) {
  // An application has replaced the clipboard contents or DnD finished
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
  if (data == 0) { // at end of DnD
    if (save_cursor) {
      scr_driver->default_cursor(save_cursor);
      scr_driver->set_cursor();
      save_cursor = NULL;
    }
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


static uint32_t last_dnd_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;


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
}


static void data_source_handle_dnd_finished(void *data, struct wl_data_source *source) {
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
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();

  struct wl_data_source *source =
    wl_data_device_manager_create_data_source(scr_driver->seat->data_device_manager);
  // we transmit the adequate value of index in fl_selection_buffer[index]
  wl_data_source_add_listener(source, &data_source_listener, (void*)0);
  wl_data_source_offer(source, wld_plain_text_clipboard);
  wl_data_source_set_actions(source, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
  struct Fl_Wayland_Graphics_Driver::wld_buffer *off = NULL;
  int s = 1;
  if (use_selection) {
    // use the text as dragging icon
    Fl_Widget *current = Fl::pushed() ? Fl::pushed() : Fl::first_window();
    s = Fl_Wayland_Window_Driver::driver(current->top_window())->wld_scale();
    off = (struct Fl_Wayland_Graphics_Driver::wld_buffer *)offscreen_from_text(fl_selection_buffer[0], s);
    dnd_icon = wl_compositor_create_surface(scr_driver->wl_compositor);
  } else dnd_icon = NULL;
  doing_dnd = true;
  wl_data_device_start_drag(scr_driver->seat->data_device, source,
                            scr_driver->seat->pointer_focus, dnd_icon,
                            scr_driver->seat->serial);
  if (use_selection) {
    wl_surface_attach(dnd_icon, off->wl_buffer, 0, 0);
    wl_surface_set_buffer_scale(dnd_icon, s);
    wl_surface_damage(dnd_icon, 0, 0, 10000, 10000);
    wl_surface_commit(dnd_icon);
    wl_surface_set_user_data(dnd_icon, off);
  } else {
    static struct wl_cursor *dnd_cursor = scr_driver->cache_cursor("dnd-copy");
    if (dnd_cursor) {
      save_cursor = scr_driver->default_cursor();
      scr_driver->default_cursor(dnd_cursor);
      scr_driver->set_cursor();
    } else save_cursor = NULL;
  }
  return 1;
}


static void data_offer_handle_offer(void *data, struct wl_data_offer *offer,
                                    const char *mime_type) {
  // runs when app becomes active and lists possible clipboard types
//fprintf(stderr, "Clipboard offer=%p supports MIME type: %s\n", offer, mime_type);
  if (strcmp(mime_type, "image/png") == 0) {
    fl_selection_type[1] = Fl::clipboard_image;
    fl_selection_offer_type = "image/png";
  } else if (strcmp(mime_type, "image/bmp") == 0 && (!fl_selection_offer_type ||
                                      strcmp(fl_selection_offer_type, "image/png"))) {
    fl_selection_type[1] = Fl::clipboard_image;
    fl_selection_offer_type = "image/bmp";
  } else if (strcmp(mime_type, "text/uri-list") == 0 && !fl_selection_type[1]) {
    fl_selection_type[1] = Fl::clipboard_plain_text;
    fl_selection_offer_type = "text/uri-list";
  } else if (strcmp(mime_type, wld_plain_text_clipboard) == 0 && !fl_selection_type[1]) {
    fl_selection_type[1] = Fl::clipboard_plain_text;
    fl_selection_offer_type = wld_plain_text_clipboard;
  } else if (strcmp(mime_type, "text/plain") == 0 && !fl_selection_type[1]) {
    fl_selection_type[1] = Fl::clipboard_plain_text;
    fl_selection_offer_type = "text/plain";
  } else if (strcmp(mime_type, "UTF8_STRING") == 0 && !fl_selection_type[1]) {
    fl_selection_type[1] = Fl::clipboard_plain_text;
    fl_selection_offer_type = "text/plain";
  }
}


static void data_offer_handle_source_actions(void *data, struct wl_data_offer *offer,
                                             uint32_t actions) {
  if (actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY) {
    //printf("Drag supports the copy action\n");
  }
}


static void data_offer_handle_action(void *data, struct wl_data_offer *offer,
                                     uint32_t dnd_action) {
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
  // An application has created a new data source
//fprintf(stderr, "data_device_handle_data_offer offer=%p\n", offer);
  fl_selection_type[1] = NULL;
  fl_selection_offer_type = NULL;
  wl_data_offer_add_listener(offer, &data_offer_listener, NULL);
}


static void data_device_handle_selection(void *data, struct wl_data_device *data_device,
                                         struct wl_data_offer *offer) {
  // An application has set the clipboard contents. W
//fprintf(stderr, "data_device_handle_selection\n");
  if (fl_selection_offer) wl_data_offer_destroy(fl_selection_offer);
  fl_selection_offer = offer;
//if (offer == NULL) fprintf(stderr, "Clipboard is empty\n");
}


// Gets from the system the clipboard or dnd text and puts it in fl_selection_buffer[1]
// which is enlarged if necessary.
static void get_clipboard_or_dragged_text(struct wl_data_offer *offer) {
  int fds[2];
  char *from;
  if (pipe(fds)) return;
  wl_data_offer_receive(offer, fl_selection_offer_type, fds[1]);
  close(fds[1]);
  wl_display_flush(Fl_Wayland_Screen_Driver::wl_display);
  // read in fl_selection_buffer
  char *to = fl_selection_buffer[1];
  ssize_t rest = fl_selection_buffer_length[1];
  while (rest) {
    ssize_t n = read(fds[0], to, rest);
    if (n <= 0) {
      close(fds[0]);
      fl_selection_length[1] = to - fl_selection_buffer[1];
      fl_selection_buffer[1][ fl_selection_length[1] ] = 0;
      goto way_out;
    }
    n = Fl_Screen_Driver::convert_crlf(to, n);
    to += n;
    rest -= n;
  }
  // compute size of unread clipboard data
  rest = fl_selection_buffer_length[1];
  while (true) {
    char buf[1000];
    ssize_t n = read(fds[0], buf, sizeof(buf));
    if (n <= 0) {
      close(fds[0]);
      break;
    }
    rest += n;
  }
//fprintf(stderr, "get_clipboard_or_dragged_text: size=%ld\n", rest);
  // read full clipboard data
  if (pipe(fds)) goto way_out;
  wl_data_offer_receive(offer, fl_selection_offer_type, fds[1]);
  close(fds[1]);
  wl_display_flush(Fl_Wayland_Screen_Driver::wl_display);
  if (rest+1 > fl_selection_buffer_length[1]) {
    delete[] fl_selection_buffer[1];
    fl_selection_buffer[1] = new char[rest+1000+1];
    fl_selection_buffer_length[1] = rest+1000;
  }
  from = fl_selection_buffer[1];
  while (true) {
    ssize_t n = read(fds[0], from, rest);
    if (n <= 0) {
      close(fds[0]);
      break;
    }
    n = Fl_Screen_Driver::convert_crlf(from, n);
    from += n;
  }
  fl_selection_length[1] = from - fl_selection_buffer[1];
  fl_selection_buffer[1][fl_selection_length[1]] = 0;
way_out:
  if (strcmp(fl_selection_offer_type, "text/uri-list") == 0) {
    fl_decode_uri(fl_selection_buffer[1]); // decode encoded bytes
    char *p = fl_selection_buffer[1];
    while (*p) { // remove prefixes
      if (strncmp(p, "file://", 7) == 0) {
        memmove(p, p+7, strlen(p+7)+1);
      }
      p = strchr(p, '\n');
      if (!p) break;
      if (*++p == 0) *(p-1) = 0; // remove last '\n'
    }
    fl_selection_length[1] = strlen(fl_selection_buffer[1]);
  }
  Fl::e_clipboard_type = Fl::clipboard_plain_text;
}


static struct wl_data_offer *current_drag_offer = NULL;
static uint32_t fl_dnd_serial;


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
    Fl::handle(FL_DND_ENTER, fl_dnd_target_window);
    current_drag_offer = offer;
    fl_dnd_serial = serial;
  }
  uint32_t supported_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
  uint32_t preferred_action = supported_actions;
  wl_data_offer_set_actions(offer, supported_actions, preferred_action);
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
    ret = Fl::handle(FL_DND_DRAG, fl_dnd_target_window);
    if (Fl::belowmouse()) Fl::belowmouse()->take_focus();
  }
  uint32_t supported_actions =  ret && (Fl::pushed() || !doing_dnd) ?
    WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY : WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
  uint32_t preferred_action = supported_actions;
  wl_data_offer_set_actions(current_drag_offer, supported_actions, preferred_action);
  wl_display_roundtrip(Fl_Wayland_Screen_Driver::wl_display);
  if (ret && current_drag_offer) wl_data_offer_accept(current_drag_offer, fl_dnd_serial, "text/plain");
}


static void data_device_handle_leave(void *data, struct wl_data_device *data_device) {
  //printf("Drag left our surface\n");
  if (current_drag_offer)  Fl::handle(FL_DND_LEAVE, fl_dnd_target_window);
}


static void data_device_handle_drop(void *data, struct wl_data_device *data_device) {
  if (!current_drag_offer) return;
  Fl::handle(FL_ENTER, fl_dnd_target_window); // useful to set the belowmouse widget
  int ret = Fl::handle(FL_DND_RELEASE, fl_dnd_target_window);
//printf("data_device_handle_drop ret=%d doing_dnd=%d\n", ret, doing_dnd);

  if (!ret) {
    wl_data_offer_destroy(current_drag_offer);
    current_drag_offer = NULL;
    return;
  }

  if (doing_dnd) {
    Fl::e_text = fl_selection_buffer[0];
    Fl::e_length = fl_selection_length[0];
  } else {
    get_clipboard_or_dragged_text(current_drag_offer);
    Fl::e_text = fl_selection_buffer[1];
    Fl::e_length = fl_selection_length[1];
  }
  int old_event = Fl::e_number;
  Fl::belowmouse()->handle(Fl::e_number = FL_PASTE);
  Fl::e_number = old_event;

  wl_data_offer_finish(current_drag_offer);
  wl_data_offer_destroy(current_drag_offer);
  current_drag_offer = NULL;
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
static int get_clipboard_image() {
  int fds[2];
  if (pipe(fds)) return 1;
  wl_data_offer_receive(fl_selection_offer, fl_selection_offer_type, fds[1]);
  close(fds[1]);
  wl_display_roundtrip(Fl_Wayland_Screen_Driver::wl_display);
  if (strcmp(fl_selection_offer_type, "image/png") == 0) {
    char tmp_fname[21];
    Fl_Shared_Image *shared = 0;
    strcpy(tmp_fname, "/tmp/clipboardXXXXXX");
    int fd = mkstemp(tmp_fname);
    if (fd == -1) return 1;
    while (true) {
      char buf[10000];
      ssize_t n = read(fds[0], buf, sizeof(buf));
      if (n <= 0) {
        close(fds[0]);
        close(fd);
        break;
      }
      n = write(fd, buf, n);
    }
    shared = Fl_Shared_Image::get(tmp_fname);
    fl_unlink(tmp_fname);
    if (!shared) return 1;
    int ld = shared->ld() ? shared->ld() : shared->w() * shared->d();
    uchar *rgb = new uchar[shared->w() * shared->h() * shared->d()];
    memcpy(rgb, shared->data()[0], ld * shared->h() );
    Fl_RGB_Image *image = new Fl_RGB_Image(rgb, shared->w(), shared->h(), shared->d(),
                                           shared->ld());
    shared->release();
    image->alloc_array = 1;
    Fl::e_clipboard_data = (void*)image;
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


void Fl_Wayland_Screen_Driver::paste(Fl_Widget &receiver, int clipboard, const char *type) {
  if (clipboard != 1) return;
  if (fl_i_own_selection[1]) {
    // We already have it, do it quickly without compositor.
    if (type == Fl::clipboard_plain_text && fl_selection_type[1] == type) {
      Fl::e_text = fl_selection_buffer[1];
      Fl::e_length = fl_selection_length[1];
      if (!Fl::e_text) Fl::e_text = (char *)"";
    } else if (type == Fl::clipboard_image && fl_selection_type[1] == type) {
      Fl::e_clipboard_data = Fl_Unix_System_Driver::own_bmp_to_RGB(fl_selection_buffer[1]);
      Fl::e_clipboard_type = Fl::clipboard_image;
    } else return;
    receiver.handle(FL_PASTE);
    return;
  }
  // otherwise get the compositor to return it:
  if (!fl_selection_offer) return;
  if (type == Fl::clipboard_plain_text && clipboard_contains(Fl::clipboard_plain_text)) {
    get_clipboard_or_dragged_text(fl_selection_offer);
    Fl::e_text = fl_selection_buffer[1];
    Fl::e_length = fl_selection_length[1];
    receiver.handle(FL_PASTE);
  } else if (type == Fl::clipboard_image && clipboard_contains(Fl::clipboard_image)) {
    if (get_clipboard_image()) return;
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

  if (clipboard >= 2)
    clipboard = 1; // Only on X11 do multiple clipboards make sense.

  if (len+1 > fl_selection_buffer_length[clipboard]) {
    delete[] fl_selection_buffer[clipboard];
    fl_selection_buffer[clipboard] = new char[len+100];
    fl_selection_buffer_length[clipboard] = len+100;
  }
  memcpy(fl_selection_buffer[clipboard], stuff, len);
  fl_selection_buffer[clipboard][len] = 0; // needed for direct paste
  fl_selection_length[clipboard] = len;
  fl_i_own_selection[clipboard] = 1;
  fl_selection_type[clipboard] = Fl::clipboard_plain_text;
  if (clipboard == 1) {
    if (seat->data_source) wl_data_source_destroy(seat->data_source);
    seat->data_source = wl_data_device_manager_create_data_source(seat->data_device_manager);
    // we transmit the adequate value of index in fl_selection_buffer[index]
    wl_data_source_add_listener(seat->data_source, &data_source_listener, (void*)1);
    wl_data_source_offer(seat->data_source, wld_plain_text_clipboard);
    wl_data_device_set_selection(seat->data_device,
                                 seat->data_source,
                                 seat->keyboard_enter_serial);
//fprintf(stderr, "wl_data_device_set_selection len=%d to %d\n", len, clipboard);
  }
}


// takes a raw RGB image and puts it in the copy/paste buffer
void Fl_Wayland_Screen_Driver::copy_image(const unsigned char *data, int W, int H){
  if (!data || W <= 0 || H <= 0) return;
  delete[] fl_selection_buffer[1];
  fl_selection_buffer[1] =
    (char *)Fl_Unix_System_Driver::create_bmp(data,W,H,&fl_selection_length[1]);
  fl_selection_buffer_length[1] = fl_selection_length[1];
  fl_i_own_selection[1] = 1;
  fl_selection_type[1] = Fl::clipboard_image;
  if (seat->data_source) wl_data_source_destroy(seat->data_source);
  seat->data_source = wl_data_device_manager_create_data_source(seat->data_device_manager);
  // we transmit the adequate value of index in fl_selection_buffer[index]
  wl_data_source_add_listener(seat->data_source, &data_source_listener, (void*)1);
  wl_data_source_offer(seat->data_source, "image/bmp");
  wl_data_device_set_selection(seat->data_device, seat->data_source,
                               seat->keyboard_enter_serial);
//fprintf(stderr, "copy_image: len=%d\n", fl_selection_length[1]);
}

////////////////////////////////////////////////////////////////
// Code for tracking clipboard changes:

// is that possible with Wayland ?

////////////////////////////////////////////////////////////////

#endif // !defined(FL_DOXYGEN)
