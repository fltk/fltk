//
// Threading example program for the Fast Light Tool Kit (FLTK).
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

#include <config.h>

#if defined(HAVE_PTHREAD) || defined(_WIN32)
#  include <FL/Fl.H>
#  include <FL/Fl_Double_Window.H>
#  include <FL/Fl_Terminal.H>
#  include <FL/Fl_Value_Output.H>
#  include <FL/fl_ask.H>
#  include "threads.h"
#  include <stdio.h>
#  include <math.h>
#  include <vector>

// min. time in seconds before calling Fl::awake(...)
#define DELTA 0.25

// struct to collect primes until at least <DELTA> seconds passed.
// Two such structs per thread are used as alternate buffers.

struct prime {
  int idx;                              // thread index: 0 or {1..6}
  int done;                             // set to 1 after it was worked on
  Fl_Terminal *terminal;                // widget to write output to
  Fl_Value_Output *value;               // highest prime
  std::vector<unsigned long> primes;    // collected primes within time frame
};

Fl_Thread prime_thread;

Fl_Terminal *tty1, *tty2;
Fl_Value_Output *value1, *value2;
int start2 = 3;

void magic_number_cb(void *p) {
  Fl_Value_Output *w = (Fl_Value_Output*)p;
  w->labelcolor(FL_RED);
  w->redraw_label();
}

// This is called indirectly by Fl::awake(update_handler, (void *)prime)
// in the context of the main (FLTK GUI) thread.

void update_handler(void *v) {
  struct prime *pr = (struct prime *)v;
  for (auto n : pr->primes) {
    pr->terminal->printf("prime: %10u\n", n);
    if (n > pr->value->value())
      pr->value->value(n);
  }
  pr->done = 1;
}

extern "C" void* prime_func(void* p) {
  Fl_Terminal* terminal = (Fl_Terminal*)p;
  Fl_Value_Output *value;
  unsigned long n;
  int step;
  char proud = 0;

  // initialize thread variables

  if (terminal == tty2) {       // multiple threads
    Fl::lock();                 // lock to prevent race condition on `start2`
    n       = start2;
    start2 += 2;
    Fl::unlock();
    step    = 12;
    value   = value2;
  } else {                      // single thread
    n       = 3;
    step    = 2;
    value   = value1;
  }

  // initialize alternate buffers (struct prime) to store primes

  struct prime pr[2];
  pr[0].done     = 0;
  pr[1].done     = 1;
  pr[0].terminal = pr[1].terminal = terminal;
  pr[0].idx      = pr[1].idx      = n/2 - 1;
  pr[0].value    = pr[1].value    = value;
  pr[0].primes.clear();
  pr[1].primes.clear();
  int pi = 0;   // prime buffer index

  Fl_Timestamp last = Fl::now();

  // very simple prime number calculator !
  //
  // The return at the end of this function can never be reached and thus
  // will generate a warning with some compilers, however we need to have
  // a return statement or other compilers will complain there is no return
  // statement. To avoid warnings on all compilers, we fool the smart ones into
  // believing that there is a chance that we reach the end by testing n > 0,
  // knowing that logically, n will never be less than 3 in this context.

  if (n > 0) for (;;) {
    int pp;
    int hn = (int)sqrt((double)n);

    for (pp = 3; pp <= hn; pp += 2) if ( n%pp == 0 ) break;

    if (pp > hn) { // n is a prime

      pr[pi].primes.push_back(n);

      // Send a message to the main thread, at which point it will
      // process any pending updates.

      double ssl = Fl::seconds_since(last);

      if (ssl > DELTA && pr[1-pi].done) {       // ready to switch buffers
        last = Fl::now();
        Fl::awake(update_handler, (void *)(&pr[pi]));
        pi = 1 - pi;                            // switch to alternate buffer
        pr[pi].primes.clear();                  // clear primes
      }

      n += step;

      if (n > 5*1000*1000 && !proud) {
        proud = 1;
        Fl::awake(magic_number_cb, value);
      }
    } else {
      // This should not be necessary since "n" and "step" are local variables,
      // however it appears that at least MacOS X has some threading issues
      // that cause semi-random corruption of the (stack) variables.
      Fl::lock();
      n += step;
      Fl::unlock();
    }
  }
  return 0L;
}

int main(int argc, char **argv)
{
  // First window: single thread
  Fl_Double_Window* w = new Fl_Double_Window(300, 200, "Single Thread");
  tty1 = new Fl_Terminal(0, 0, 300, 175);
  w->resizable(tty1);
  value1 = new Fl_Value_Output(100, 175, 200, 25, "Max Prime:");
  w->end();
  w->show(argc, argv);

  // Second window: multiple threads
  w = new Fl_Double_Window(300, 200, "Six Threads");
  tty2 = new Fl_Terminal(0, 0, 300, 175);
  w->resizable(tty2);
  value2 = new Fl_Value_Output(100, 175, 200, 25, "Max Prime:");
  w->end();
  w->show();

  tty1->printf("Prime numbers:\n");
  tty2->printf("Prime numbers:\n");

  // Enable multi-thread support by locking from the main thread.
  // Fl::wait() and Fl::run() call Fl::unlock() and Fl::lock() as needed
  // to release control to the child threads when it is safe to do so...

  Fl::lock();

  // Start threads...

  // One thread displaying in one terminal
  fl_create_thread(prime_thread, prime_func, tty1);

  // Six threads displaying in another terminal

  fl_create_thread(prime_thread, prime_func, tty2);
  fl_create_thread(prime_thread, prime_func, tty2);
  fl_create_thread(prime_thread, prime_func, tty2);
  fl_create_thread(prime_thread, prime_func, tty2);
  fl_create_thread(prime_thread, prime_func, tty2);
  fl_create_thread(prime_thread, prime_func, tty2);

  Fl::run();

  return 0;
}
#else
#  include <FL/fl_ask.H>

int main() {
  fl_alert("Sorry, threading not supported on this platform!");
}
#endif // HAVE_PTHREAD || _WIN32
