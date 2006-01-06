//
// "$Id$"
//
// Threading example program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2005 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include "config.h"

#if HAVE_PTHREAD || defined(WIN32)
#  include <FL/Fl.H>
#  include <FL/Fl_Window.H>
#  include <FL/Fl_Browser.H>
#  include <FL/Fl_Value_Output.H>
#  include "threads.h"
#  include <stdio.h>
#  include <math.h>

Fl_Thread prime_thread;

Fl_Browser *browser1, *browser2;
Fl_Value_Output *value1, *value2;
int start2 = 3;

void* prime_func(void* p)
{
  Fl_Browser* browser = (Fl_Browser*) p;
  Fl_Value_Output *value;
  int n;
  int step;

  if (browser == browser2) {
    n      = start2;
    start2 += 2;
    step   = 12;
    value  = value2;
  } else {
    n     = 3;
    step  = 2;
    value = value1;
  }

  // very simple prime number calculator !
  for (;;) {
    int p;
    int hn = (int)sqrt((double)n);

    for (p=3; p<=hn; p+=2) if ( n%p == 0 ) break;
    if (p >= hn) {
      char s[128];
      sprintf(s, "%d", n);
      Fl::lock();
      browser->add(s);
      browser->bottomline(browser->size());
      if (n > value->value()) value->value(n);
      n += step;
      Fl::unlock();
      Fl::awake((void*) (browser == browser1? p:0));	// Cause the browser to redraw ...
    } else {
      // This should not be necessary since "n" and "step" a local variables,
      // however it appears that at least MacOS X has some threading issues
      // that cause semi-random corruption of the (stack) variables.
      Fl::lock();
      n += step;
      Fl::unlock();
    }
  }
  return 0;
}

int main(int argc, char **argv)
{
  Fl_Window* w = new Fl_Window(200, 200, "Single Thread");
  browser1 = new Fl_Browser(0, 0, 200, 175);
  w->resizable(browser1);
  value1 = new Fl_Value_Output(100, 175, 200, 25, "Max Prime:");
  w->end();
  w->show(argc, argv);
  w = new Fl_Window(200, 200, "Six Threads");
  browser2 = new Fl_Browser(0, 0, 200, 175);
  w->resizable(browser2);
  value2 = new Fl_Value_Output(100, 175, 200, 25, "Max Prime:");
  w->end();
  w->show();
  
  browser1->add("Prime numbers:");
  browser2->add("Prime numbers:");

  Fl::lock(); // you must do this before creating any threads!

  // One thread displaying in one browser
  fl_create_thread(prime_thread, prime_func, browser1);
  // Several threads displaying in another browser
  fl_create_thread(prime_thread, prime_func, browser2);
  fl_create_thread(prime_thread, prime_func, browser2);
  fl_create_thread(prime_thread, prime_func, browser2);
  fl_create_thread(prime_thread, prime_func, browser2);
  fl_create_thread(prime_thread, prime_func, browser2);
  fl_create_thread(prime_thread, prime_func, browser2);

  //  Fl::run();
  while (w->visible()) {
    Fl::wait();
//    void* m = Fl::thread_message();
//    printf("Received message: %p\n", m);
  }

  return 0;
}
#else
#  include <FL/fl_ask.H>

int main() {
  fl_alert("Sorry, threading not supported on this platform!");
}
#endif // HAVE_PTHREAD || WIN32


//
// End of "$Id$".
//
