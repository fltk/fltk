//
// "$Id: Fl_lock.cxx,v 1.13.2.3 2002/07/01 20:14:08 easysw Exp $"
//
// Multi-threading support code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2002 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//


#include <FL/Fl.H>
#include <config.h>

/*
   From Bill:

   I would prefer that FLTK contain the minimal amount of extra
   stuff for doing threads.  There are other portable thread
   wrapper libraries out there and FLTK should not be providing
   another.  This file is an attempt to make minimal additions
   and make them self-contained in this source file.

   Fl::lock() - recursive lock.  You must call this before the
   first call to Fl::wait()/run() to initialize the thread
   system. The lock is locked all the time except when
   Fl::wait() is waiting for events.

   Fl::unlock() - release the recursive lock.

   Fl::awake(void*) - Causes Fl::wait() to return (with the lock
   locked) even if there are no events ready.

   Fl::thread_message() - returns an argument sent to an
   Fl::awake() call, or returns NULL if none.  WARNING: the
   current implementation only has a one-entry queue and only
   returns the most recent value!
*/


////////////////////////////////////////////////////////////////
// Windows threading...
#ifdef WIN32
#  include <windows.h>
#  include <process.h>
#  include <FL/x.H>

// These pointers are in Fl_win32.cxx:
extern void (*fl_lock_function)();
extern void (*fl_unlock_function)();

// The main thread's ID
static DWORD main_thread;

// Microsoft's version of a MUTEX...
CRITICAL_SECTION cs;

//
// 'unlock_function()' - Release the lock.
//

static void unlock_function() {
  LeaveCriticalSection(&cs);
}

//
// 'lock_function()' - Get the lock.
//

static void lock_function() {
  EnterCriticalSection(&cs);
}

//
// 'Fl::lock()' - Lock access to FLTK data structures...
//

void Fl::lock() {
  if (!main_thread) InitializeCriticalSection(&cs);

  lock_function();

  if (!main_thread) {
    fl_lock_function   = lock_function;
    fl_unlock_function = unlock_function;
    main_thread        = GetCurrentThreadId();
  }
}

//
// 'Fl::unlock()' - Unlock access to FLTK data structures...
//

void Fl::unlock() {
  unlock_function();
}


//
// 'Fl::awake()' - Let the main thread know an update is pending.
//
// When called from a thread, it causes FLTK to awake from Fl::wait()...
//

void Fl::awake(void* msg) {
  PostThreadMessage( main_thread, fl_wake_msg, (WPARAM)msg, 0);
}

////////////////////////////////////////////////////////////////
// POSIX threading...
#elif HAVE_PTHREAD
#  include <unistd.h>
#  include <pthread.h>

#  ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
// Linux supports recursive locks, use them directly:

static pthread_mutex_t fltk_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

static void lock_function() {
  pthread_mutex_lock(&fltk_mutex);
}

void Fl::unlock() {
  pthread_mutex_unlock(&fltk_mutex);
}

// this is needed for the Fl_Mutex constructor:
pthread_mutexattr_t Fl_Mutex_attrib = {PTHREAD_MUTEX_RECURSIVE_NP};

#  else
// Make a recursive lock out of the pthread mutex:

static pthread_mutex_t fltk_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t owner;
static int counter;

static void lock_function() {
  if (!counter || owner != pthread_self()) {
    pthread_mutex_lock(&fltk_mutex);
    owner = pthread_self();
  }
  counter++;
}

void Fl::unlock() {
  if (!--counter) pthread_mutex_unlock(&fltk_mutex);
}

#  endif

// Pipe for thread messaging...
static int thread_filedes[2];

// These pointers are in Fl_x.cxx:
extern void (*fl_lock_function)();
extern void (*fl_unlock_function)();

static void* thread_message_;
void* Fl::thread_message() {
  void* r = thread_message_;
  thread_message_ = 0;
  return r;
}

static void thread_awake_cb(int fd, void*) {
  read(fd, &thread_message_, sizeof(void*));
}

void Fl::lock() {
  lock_function();
  if (!thread_filedes[1]) { // initialize the mt support
    // Init threads communication pipe to let threads awake FLTK from wait
    pipe(thread_filedes);
    Fl::add_fd(thread_filedes[0], FL_READ, thread_awake_cb);
    fl_lock_function   = lock_function;
    fl_unlock_function = Fl::unlock;
  }
}

void Fl::awake(void* msg) {
  write(thread_filedes[1], &msg, sizeof(void*));
}

#endif

//
// End of "$Id: Fl_lock.cxx,v 1.13.2.3 2002/07/01 20:14:08 easysw Exp $".
//
