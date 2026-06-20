//
// Interface with the libdecor library for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022-2024 by Bill Spitzak and others.
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

#ifndef fl_libdecor_plugins_h
#define fl_libdecor_plugins_h

enum plugin_kind { UNKNOWN, SSD, CAIRO, GTK3 };

struct libdecor_frame;
enum plugin_kind get_plugin_kind(struct libdecor_frame *frame);



#endif
