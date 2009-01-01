//
// "$Id$"
//
// Threading example program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
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
#  include <FL/fl_ask.H>
#  include "threads.h"
#  include <stdio.h>
#  include <math.h>

Fl_Thread prime_thread;

Fl_Browser *browser1, *browser2;
Fl_Value_Output *value1, *value2;
int start2 = 3;

void magic_number_cb(void *p)
{
  Fl_Value_Output *w = (Fl_Value_Output*)p;
  w->labelcolor(FL_RED);
  w->redraw_label();
}

void* prime_func(void* p)
{
  Fl_Browser* browser = (Fl_Browser*) p;
  Fl_Value_Output *value;
  int n;
  int step;
  char proud = 0;

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
  //
  // The return at the end of this function can never be reached and thus 
  // will generate a warning with some compilers, however we need to have 
  // a return statement or other compilers will complain there is no return 
  // statement. To avoid warnings on all compilers, we fool the smart ones 
  // into beleiving that there is a chance that we reach the end by testing 
  // n>=0, knowing that logically, n will never be negative in this context.
  if (n>=0) for (;;) {
    int pp;
    int hn = (int)sqrt((double)n);

    for (pp=3; pp<=hn; pp+=2) if ( n%pp == 0 ) break;
    if (pp >= hn) {
      char s[128];
      sprintf(s, "%d", n);

      // Obtain a lock before we access the browser widget...
      Fl::lock();

      browser->add(s);
      browser->bottomline(browser->size());
      if (n > value->value()) value->value(n);
      n += step;

      // Release the lock...
      Fl::unlock();

      // Send a message to the main thread, at which point it will
      // process any pending redraws for our browser widget.  The
      // message we pass here isn't used for anything, so we could also
      // just pass NULL.
      Fl::awake(p);
      if (n>10000 && !proud) {
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

  // Enable multi-thread support by locking from the main
  // thread.  Fl::wait() and Fl::run() call Fl::unlock() and
  // Fl::lock() as needed to release control to the child threads
  // when it is safe to do so...
  Fl::lock();

  // Start threads...

  // One thread displaying in one browser
  fl_create_thread(prime_thread, prime_func, browser1);

  // Several threads displaying in another browser
  fl_create_thread(prime_thread, prime_func, browser2);
  fl_create_thread(prime_thread, prime_func, browser2);
  fl_create_thread(prime_thread, prime_func, browser2);
  fl_create_thread(prime_thread, prime_func, browser2);
  fl_create_thread(prime_thread, prime_func, browser2);
  fl_create_thread(prime_thread, prime_func, browser2);

  Fl::run();

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
