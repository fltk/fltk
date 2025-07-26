//
// Idle routine support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

// Allows you to manage an arbitrary set of idle() callbacks.
// Replaces the older set_idle() call which has been renamed to set_idle_(),
// is now private in class Fl::, and is used to implement this.

#include <FL/Fl.H>

struct idle_cb {
  void (*cb)(void*);
  void* data;
  idle_cb *next;
};

// The callbacks are stored linked in a ring. `last` points at the one
// just called, `first` at the next to call.  last->next == first.

static idle_cb* first;
static idle_cb* last;
static idle_cb* freelist;

// The function call_idle()
// - removes the first idle callback from the front of the list (ring)
// - adds it as the last entry and
// - calls the idle callback.
// The idle callback may remove itself from the list of idle callbacks
// by calling Fl::remove_idle().

static void call_idle() {
  idle_cb* p = first;
  last = p; first = p->next;
  p->cb(p->data); // this may call add_idle() or remove_idle()!
}

/**
  Adds a callback function that is called every time by Fl::wait() and also
  makes it act as though the timeout is zero (this makes Fl::wait() return
  immediately, so if it is in a loop it is called repeatedly, and thus the
  idle function is called repeatedly). The idle function can be used to get
  background processing done.

  You can have multiple idle callbacks. If this is the case, then all idle
  callbacks are called in turn. Each idle callback should return after it
  has done \b some work to let the next idle callback or the FLTK event loop
  continue processing.

  To remove an idle callback use Fl::remove_idle().

  Fl::wait() and Fl::check() call idle callbacks, but Fl::ready() does not.

  The idle callback can call any FLTK functions, including Fl::wait(),
  Fl::check(), and Fl::ready().

  FLTK will not recursively call the idle callback.

  \param[in] cb   your idle callback
  \param[in] data an arbitrary data value provided to your callback
*/
void Fl::add_idle(Fl_Idle_Handler cb, void* data) {
  idle_cb* p = freelist;
  if (p) freelist = p->next;
  else p = new idle_cb;
  p->cb = cb;
  p->data = data;
  if (first) {
    last->next = p;
    last = p;
    p->next = first;
  } else {
    first = last = p;
    p->next = p;
    set_idle_(call_idle);
  }
}

void Fl::add_idle(Fl_Old_Idle_Handler cb) {
  Fl::add_idle((Fl_Idle_Handler)cb, nullptr);
}

/**
  Returns true if the specified idle callback is currently installed.

  An idle callback matches the request only if \p data matches the \p data
  argument when the callback was installed. There is no "wildcard" search.

  \param[in]  cb    idle callback in question
  \param[in]  data  optional data. Default: zero / nullptr.

  \returns    Whether the given callback \p cb is queued with \p data.
  \retval  1  The callback is currently in the callback queue.
  \retval  0  The callback is not queued, or \p data doesn't match.
*/
int Fl::has_idle(Fl_Idle_Handler cb, void* data) {
  idle_cb* p = first;
  if (!p) return 0;
  for (;; p = p->next) {
    if (p->cb == cb && p->data == data) return 1;
    if (p==last) return 0;
  }
}

/**
  Removes the specified idle callback, if it is installed.

  The given idle callback is only removed if \p data matches the
  value used when the idle callback was installed. If the idle
  callback wants to remove itself, the value provided by the \p data
  argument can (and should) be used.

  Example for a "one-shot" idle callback, i.e. one that removes itself
  when it is called for the first time.
  \code
    #include <FL/Fl.H>
    #include <FL/Fl_Double_Window.H>
    #include <FL/Fl_Button.H>
    void idle1(void *data) {
      printf("idle1 called with data %4d\n", fl_int(data));
      fflush(stdout);
      // ... do something ...
      Fl::remove_idle(idle1, data);
    }
    void quit_cb(Fl_Widget *w, void *v) {
      w->window()->hide();
    }
    int main(int argc, char **argv) {
      auto window = new Fl_Double_Window(200, 100);
      auto button = new Fl_Button(20, 20, 160, 60, "Quit");
      button->callback(quit_cb);
      window->end();
      window->show(argc, argv);
      Fl::add_idle(idle1, (void *)1234);
      return Fl::run();
    }
  \endcode

  \param[in]  cb    idle callback in question
  \param[in]  data  optional data. Default: zero / nullptr.
*/
void Fl::remove_idle(Fl_Idle_Handler cb, void* data) {
  idle_cb* p = first;
  if (!p) return;
  idle_cb* l = last;
  for (;; p = p->next) {
    if (p->cb == cb && p->data == data) break;
    if (p==last) return; // not found
    l = p;
  }
  if (l == p) { // only one
    first = last = 0;
    set_idle_(0);
  } else {
    last = l;
    first = l->next = p->next;
  }
  p->next = freelist;
  freelist = p;
}
