//
// Convert Windows-1252 (Latin-1) encoded text to the local encoding.
//
// Copyright 1998-2021 by Bill Spitzak and others.
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
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include "Fl_System_Driver.H"
#include <FL/Enumerations.H>
#include <stdlib.h>
#include "flstring.h"

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

/**
 Default implementation of latin-to-local text conversion.

 The default implementation returns the original text. This method should
 be reimplemented by drivers for platforms that commonly use latin
 text encoding.

 \see fl_latin1_to_local(const char *, int)
 */
const char *Fl_System_Driver::latin1_to_local(const char *t, int)
{
  return t;
}

/**
 Default implementation of local-to-latin text conversion.

 The default implementation returns the original text. This method should
 be reimplemented by drivers for platforms that commonly use latin
 text encoding.

 \see fl_local_to_latin1(const char *, int)
 */
const char *Fl_System_Driver::local_to_latin1(const char *t, int)
{
  return t;
}

/**
 \}
 \endcond
 */

const char *fl_latin1_to_local(const char *t, int n)
{
  return Fl::system_driver()->latin1_to_local(t, n);
}

const char *fl_local_to_latin1(const char *t, int n)
{
  return Fl::system_driver()->local_to_latin1(t, n);
}
