//
// "$Id$"
//
// Template header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2016 by Bill Spitzak and others.
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

// Do not directly include this file, instead use <FL/x.H>.

// These types and variables give access to internal, platform-specific data through the public API.
// They require to include x.H (in contrast to types defined in platform_types.h)

#if !defined(Fl_X_H)
#  error "Never use <FL/porting.H> directly; include <FL/x.H> instead."
#endif // !Fl_X_H

typedef void *Window; // used by fl_find(), fl_xid() and class Fl_X

/* Reference to the current device context
 For back-compatibility only. The preferred procedure to get this reference is
 Fl_Surface_Device::surface()->driver()->gc().
 */
extern void *fl_gc;

//
// End of "$Id$".
//
