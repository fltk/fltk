//
// Definition of the part of the screen driver shared by X11 and Wayland platforms
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021-2023 by Bill Spitzak and others.
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

#ifndef FL_UNIX_SCREEN_DRIVER_H
#define FL_UNIX_SCREEN_DRIVER_H

#include <config.h>
#include "../../Fl_Screen_Driver.H"

#  if USE_POLL

#    include <poll.h>
static pollfd *pollfds = 0;

#  else
#    if HAVE_SYS_SELECT_H
#      include <sys/select.h>
#    endif /* HAVE_SYS_SELECT_H */

// The following #define is only needed for HP-UX 9.x and earlier:
//#define select(a,b,c,d,e) select((a),(int *)(b),(int *)(c),(int *)(d),(e))

#    define POLLIN 1
#    define POLLOUT 4
#    define POLLERR 8

#  endif /* USE_POLL */


class Fl_Unix_Screen_Driver : public Fl_Screen_Driver {
public:
  static fd_set fdsets[3];
  static int maxfd;
  static int nfds;
  static struct FD {
  #  if !USE_POLL
    int fd;
    short events;
  #  endif
    void (*cb)(int, void*);
    void* arg;
  } *fd;
  virtual int poll_or_select_with_delay(double time_to_wait);
  virtual int poll_or_select();
  virtual void *control_maximize_button(void *) { return NULL; }
};

#endif /* FL_UNIX_SCREEN_DRIVER_H */
