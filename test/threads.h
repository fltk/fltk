//
// "$Id$"
//
// Simple threading API for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
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

#  ifdef HAVE_PTHREAD_H
// Use POSIX threading...

#    include <pthread.h>

typedef pthread_t Fl_Thread;
extern "C" {
  typedef void *(Fl_Thread_Func)(void *);
}

static int fl_create_thread(Fl_Thread& t, Fl_Thread_Func* f, void* p) {
  return pthread_create((pthread_t*)&t, 0, f, p);
}

#  elif defined(WIN32) && !defined(__WATCOMC__) // Use Windows threading...

#    include <windows.h>
#    include <process.h>

typedef unsigned long Fl_Thread;
extern "C" {
  typedef void *(__cdecl Fl_Thread_Func)(void *);
}

static int fl_create_thread(Fl_Thread& t, Fl_Thread_Func* f, void* p) {
  return t = (Fl_Thread)_beginthread((void( __cdecl * )( void * ))f, 0, p);
}

#  elif defined(__WATCOMC__)
#    include <process.h>

typedef unsigned long Fl_Thread;
extern "C" {
  typedef void *(__cdecl Fl_Thread_Func)(void *);
}

static int fl_create_thread(Fl_Thread& t, Fl_Thread_Func* f, void* p) {
  return t = (Fl_Thread)_beginthread((void(* )( void * ))f, 32000, p);
}
#  endif // !HAVE_PTHREAD_H
#endif // !Threads_h

//
// End of "$Id$".
//
