#include <fltk/Fl.h>
#include <fltk/Fl_Window.h>
#include <fltk/Fl_Browser.h>
#include <fltk/Fl_Threads.h>
#include <stdio.h>

Fl_Thread prime_thread;

Fl_Browser *browser1, *browser2;

void* prime_func(void* p)
{
  Fl_Browser* browser = (Fl_Browser*) p;

  // very loosy prime number calculator !
  for (int n=1000000; ; n++) {
    int p;
    for (p=2; p<n; p++) if ( n%p == 0 ) break;
    if (p == n) {
      char s[128];
      sprintf(s, "%d", n);
      Fl::lock();
      browser->add(s);
      Fl::unlock();
      Fl::awake((void*) (browser == browser1? p:0));	// Cause the browser to redraw ...
    }
  }
  return 0;
}

int main()
{
  Fl_Window* w = new Fl_Window(200, 300, "Multithread test");
  browser1 = new Fl_Browser(0, 0, 200, 300);
  w->end();
  w->show();
  w = new Fl_Window(200, 300, "Multithread test");
  browser2 = new Fl_Browser(0, 0, 200, 300);
  w->end();
  w->show();
  
  browser1->add("Prime numbers :");
  browser2->add("Prime numbers :");

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
    void* m = Fl::thread_message();
    if (m) printf("Recieved message: %d\n", int(m));
  }

  return 0;
}
