//
// FLUID main entry for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

#ifndef _FLUID_MAIN_H
#define _FLUID_MAIN_H

#ifdef FLUID_CMD
constexpr bool FLUID_CONFIG_CONSOLE = true;
#else
constexpr bool FLUID_CONFIG_CONSOLE = false;
#endif

extern int main(int argc,char **argv);

#endif // _FLUID_MAIN_H
