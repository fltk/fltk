//
// "$Id: threads.h,v 1.1.2.1 2001/12/08 16:23:51 easysw Exp $"
//
// Simple threading API for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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

// Inline classes to provide portable support for threads and mutexes.
//
// FLTK does not use this (it has an internal mutex implementation
// that is used if Fl::lock() is called). This header file's only
// purpose is so we can write portable demo programs. It may be useful
// or an inspiration to people who want to try writing multithreaded
// programs themselves.
//
// FLTK has no multithreaded support unless the main thread calls Fl::lock().
// This main thread is the only thread allowed to call Fl::run() or Fl::wait().
// From then on FLTK will be locked except when the main thread is actually
// waiting for events from the user. Other threads must call Fl::lock() and
// Fl::unlock() to surround calls to FLTK (such as to change widgets or
// redraw them).

#ifndef Threads_H
#  define Threads_H

#  ifndef WIN32
// Use POSIX threading...

#    include <pthread.h>

typedef pthread_t Fl_Thread;

static int fl_create_thread(Fl_Thread& t, void *(*f) (void *), void* p)
{
  return pthread_create((pthread_t*)&t, 0, f, p);
}

// Linux supports recursive locks, use them directly, with some cheating:
#    ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP

extern pthread_mutexattr_t Fl_Mutex_attrib;

class Fl_Mutex {
  friend class Fl_SignalMutex;
  pthread_mutex_t mutex;
  Fl_Mutex(const Fl_Mutex&);
  Fl_Mutex& operator=(const Fl_Mutex&);
public:
  Fl_Mutex() {pthread_mutex_init(&mutex, &Fl_Mutex_attrib);}
  void lock() {pthread_mutex_lock(&mutex);}
  void unlock() {pthread_mutex_unlock(&mutex);}
  ~Fl_Mutex() {pthread_mutex_destroy(&mutex);}
};

class Fl_SignalMutex : public Fl_Mutex {
  pthread_cond_t cond;
public:
  Fl_SignalMutex() : Fl_Mutex() {pthread_cond_init(&cond, 0);}
  void signal() {pthread_cond_broadcast(&cond);}
  void wait() {
    int save_counter = mutex.m_count; mutex.m_count = 1;
    pthread_cond_wait(&cond, &mutex);
    mutex.m_count = save_counter;
  }
};

#    else // standard pthread mutexes need a bit of work to be recursive:

class Fl_Mutex {
  friend class Fl_SignalMutex;
  pthread_mutex_t mutex;
  pthread_t owner;
  int counter;
  Fl_Mutex(const Fl_Mutex&);
  Fl_Mutex& operator=(const Fl_Mutex&);
public:
  Fl_Mutex() : counter(0) {pthread_mutex_init(&mutex, 0);}
  void lock() {
    if (!counter || owner != pthread_self()) {
      pthread_mutex_lock(&mutex); owner = pthread_self();
    }
    counter++;
  }
  void unlock() {if (!--counter) pthread_mutex_unlock(&mutex);}
  ~Fl_Mutex() {pthread_mutex_destroy(&mutex);}
};

class Fl_SignalMutex : public Fl_Mutex {
  pthread_cond_t cond;
public:
  Fl_SignalMutex() : Fl_Mutex() {pthread_cond_init(&cond, 0);}
  void signal() {pthread_cond_broadcast(&cond);}
  void wait() {
    int save_counter = counter; counter = 0;
    pthread_cond_wait(&cond, &mutex);
    counter = save_counter;
    owner = pthread_self();
  }
};

#    endif

#  else // Use Windows threading...

#    include <windows.h>
#    include <process.h>

typedef unsigned long Fl_Thread;

static int fl_create_thread(Fl_Thread& t, void *(*f) (void *), void* p) {
  return t = (Fl_Thread)_beginthread((void( __cdecl * )( void * ))f, 0, p);
}

class FL_API Fl_Mutex {
  friend class Fl_SignalMutex;
  CRITICAL_SECTION cs;
  Fl_Mutex(const Fl_Mutex&);
  Fl_Mutex& operator=(const Fl_Mutex&);
public:
  Fl_Mutex() {InitializeCriticalSection(&cs);}
  void lock() {EnterCriticalSection(&cs);}
  void unlock() {LeaveCriticalSection(&cs);}
  ~Fl_Mutex() {DeleteCriticalSection(&cs);}
};

class FL_API Fl_SignalMutex : public Fl_Mutex {
  HANDLE event;
public:
  Fl_SignalMutex() : Fl_Mutex() {event = CreateEvent(0, FALSE, FALSE, 0);}
  void signal() {SetEvent(event);}
  void wait() {
    // int save_counter = cs.count; cs.count = 1;
    // the following three calls should be atomic, sigh...
    LeaveCriticalSection(&cs);
    WaitForSingleObject(event, INFINITE);
    EnterCriticalSection(&cs);
    // cs.count = save_counter;
  }
};

#  endif // !WIN32
#endif // !Threads_h

//
// End of "$Id: threads.h,v 1.1.2.1 2001/12/08 16:23:51 easysw Exp $".
//
