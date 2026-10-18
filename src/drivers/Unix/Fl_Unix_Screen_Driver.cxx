//
// Definition of the part of the Screen interface shared by X11/Wayland
//
// Copyright 2022-2024 by Bill Spitzak and others.
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

#include <config.h>
#include <sys/time.h>
#include "Fl_Unix_Screen_Driver.H"
#include "Fl_Unix_System_Driver.H"
#include <FL/Fl_Drag_Data.H>
#include <FL/Fl_Image.H>

#if USE_POLL
pollfd *Fl_Unix_Screen_Driver::pollfds = NULL;
#else
fd_set Fl_Unix_Screen_Driver::fdsets[3];
#endif
int Fl_Unix_Screen_Driver::maxfd = 0;
int Fl_Unix_Screen_Driver::nfds = 0;
Fl_Unix_Screen_Driver::FD *Fl_Unix_Screen_Driver::fd = NULL;

// these pointers are set by the Fl::lock() function:
static void nothing() {}
void (*fl_lock_function)() = nothing;
void (*fl_unlock_function)() = nothing;


// This is never called with time_to_wait < 0.0:
// It should return negative on error, 0 if nothing happens before
// timeout, and >0 if any callbacks were done.
int Fl_Unix_Screen_Driver::poll_or_select_with_delay(double time_to_wait) {
#  if !USE_POLL
  fd_set fdt[3];
  fdt[0] = fdsets[0];
  fdt[1] = fdsets[1];
  fdt[2] = fdsets[2];
#  endif
  int n;

  fl_unlock_function();

  if (time_to_wait < 2147483.648) {
#  if USE_POLL
    n = ::poll(pollfds, nfds, int(time_to_wait*1000 + .5));
#  else
    timeval t;
    t.tv_sec = int(time_to_wait);
    t.tv_usec = int(1000000 * (time_to_wait-t.tv_sec));
    n = ::select(maxfd+1,&fdt[0],&fdt[1],&fdt[2],&t);
#  endif
  } else {
#  if USE_POLL
    n = ::poll(pollfds, nfds, -1);
#  else
    n = ::select(maxfd+1,&fdt[0],&fdt[1],&fdt[2],0);
#  endif
  }

  fl_lock_function();

  if (n > 0) {
    for (int i=0; i<nfds; i++) {
#  if USE_POLL
      if (pollfds[i].revents) fd[i].cb(pollfds[i].fd, fd[i].arg);
#  else
      int f = fd[i].fd;
      short revents = 0;
      if (FD_ISSET(f,&fdt[0])) revents |= POLLIN;
      if (FD_ISSET(f,&fdt[1])) revents |= POLLOUT;
      if (FD_ISSET(f,&fdt[2])) revents |= POLLERR;
      if (fd[i].events & revents) fd[i].cb(f, fd[i].arg);
#  endif
    }
  }
  return n;
}


int Fl_Unix_Screen_Driver::poll_or_select() {
  if (!nfds) return 0; // nothing to select or poll
#  if USE_POLL
  return ::poll(pollfds, nfds, 0);
#  else
  timeval t;
  t.tv_sec = 0;
  t.tv_usec = 0;
  fd_set fdt[3];
  fdt[0] = fdsets[0];
  fdt[1] = fdsets[1];
  fdt[2] = fdsets[2];
  return ::select(maxfd+1,&fdt[0],&fdt[1],&fdt[2],&t);
#  endif
}


// Returns the pixels of img as packed RGB, composited over white
static unsigned char *packed_rgb(const Fl_RGB_Image *img) {
  int w = img->data_w(), h = img->data_h(), d = img->d();
  int ld = img->ld() ? img->ld() : w * d;
  unsigned char *rgb = new unsigned char[w * h * 3];
  unsigned char *q = rgb;
  for (int y = 0; y < h; y++) {
    const unsigned char *p = img->array + y * ld;
    for (int x = 0; x < w; x++, p += d) {
      unsigned r, g, b, a;
      if (d < 3) { r = g = b = p[0]; a = (d == 2 ? p[1] : 255); }
      else { r = p[0]; g = p[1]; b = p[2]; a = (d == 4 ? p[3] : 255); }
      *q++ = (unsigned char)((r * a + 255 * (255 - a)) / 255);
      *q++ = (unsigned char)((g * a + 255 * (255 - a)) / 255);
      *q++ = (unsigned char)((b * a + 255 * (255 - a)) / 255);
    }
  }
  return rgb;
}


/* Describes the content of an Fl_Drag_Data object as MIME types for XDND and Wayland.
 Promised files are written to a temporary directory first because neither protocol
 supports file promises in a portable way.
 */
void Fl_Unix_Screen_Driver::dnd_offers(Fl_Drag_Data *data, Dnd_Offers &offers,
                                       std::string &local_text) {
  offers.clear();
  local_text.clear();
  data->write_promises_to_temp_dir();
  std::string uri_list, paths;
  for (int i = 0; i < data->file_count(); i++) {
    const char *path = data->file_path(i);
    if (!path) continue; // a promise that could not be written
    uri_list += encode_file_uri(path);
    uri_list += "\r\n";
    if (!paths.empty()) paths += '\n';
    paths += path;
  }
  if (!uri_list.empty()) offers.push_back(std::make_pair(std::string("text/uri-list"), uri_list));
  if (data->has_text()) {
    offers.push_back(std::make_pair(std::string("text/plain;charset=utf-8"), data->text()));
    offers.push_back(std::make_pair(std::string("UTF8_STRING"), data->text()));
    offers.push_back(std::make_pair(std::string("text/plain"), data->text()));
  }
  if (data->image() && data->image()->array) {
    const Fl_RGB_Image *img = data->image();
    unsigned char *rgb = packed_rgb(img);
    int size;
    unsigned char *bmp = Fl_Unix_System_Driver::create_bmp(rgb, img->data_w(), img->data_h(), &size);
    delete[] rgb;
    offers.push_back(std::make_pair(std::string("image/bmp"), std::string((char *)bmp, size)));
    delete[] bmp;
  }
  for (int i = 0; i < data->data_count(); i++) {
    bool known = false;
    for (size_t j = 0; j < offers.size(); j++) {
      if (offers[j].first == data->data_mime_type(i)) known = true;
    }
    if (!known) offers.push_back(std::make_pair(std::string(data->data_mime_type(i)), data->data_bytes(i)));
  }
  local_text = paths.empty() ? data->text() : paths;
}
