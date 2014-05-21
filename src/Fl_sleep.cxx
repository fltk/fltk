//
// "$Id: Fl_sleep.cxx $"
//
// Multi-threading support code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

// Cross platform sleep API for FLTK, F. Costantini, May 20th, 2014

#include <FL/Fl.H>
#include <config.h>

#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/** Make the current thread sleep for n seconds, support decimals ... */
void Fl::sleep(double seconds)
{
  Fl::usleep((unsigned long long) (seconds*1000000));
}

/** Make the current thread to sleep for n milliseconds */
void Fl::msleep(unsigned long milliseconds)
{
#ifdef WIN32
  ::Sleep( (DWORD) milliseconds);
#else
  ::usleep((useconds_t) (milliseconds*1000));
#endif
}

/** Make the current thread to sleep for n microseconds */
void Fl::usleep(unsigned long long microseconds) 
// unsigned long long more should be more portable than int64_t before c++ 2011 ...
{
#ifdef WIN32
  HANDLE timer; 
  LARGE_INTEGER reltime; 
  
  reltime.QuadPart = (LONGLONG) -(10*microseconds); // Convert to 100 nanosecond relative time interval
  timer = CreateWaitableTimer(NULL, TRUE, NULL); 
  SetWaitableTimer(timer, &reltime, 0, NULL, NULL, 0); 
  WaitForSingleObject(timer, INFINITE); 
  CloseHandle(timer); 
#else
  ::usleep((useconds_t) microseconds);
#endif
}

//
// End of "$Id: $".
//
