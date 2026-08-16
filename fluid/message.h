//
// Fluid message dialog header for the Fast Light Tool Kit (FLTK).
//
// Copyright 2026 by Bill Spitzak and others.
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


#ifndef FLUID_MESSAGE_H
#define FLUID_MESSAGE_H

#include <FL/fl_ask.H>

#include <string>
#include <vector>
#include <cstdarg>

// map FLTK dialog functions into Fluid console/gui output
extern void fluid_alert(const char *fmt, ...);
extern void fluid_alert(const char *fmt, va_list ap);
extern void fluid_message(const char *fmt, ...);
extern void fluid_message(const char *fmt, va_list ap);
extern int fluid_choice(const char *fmt, const char *b0, const char *b1, const char *b2, ...);
extern int fluid_choice(const char *fmt, const char *b0, const char *b1, const char *b2, va_list ap);
extern int fluid_choice(const char *title, const char *fmt, const char *b0, const char *b1, const char *b2, va_list ap);
// fl_input

namespace fluid {

namespace msg {

using Option = struct { const std::string &label; char key; };
constexpr int CLOSE = -2;
constexpr int ESC = -1;
constexpr int ABORT = 0;
constexpr int CONTINUE = 1;

} // namespace msg

void message(const std::string &title, const std::string &message);
void alert(const std::string &title, const std::string &message);
int error_choice(const std::string &title, const std::string &message);
int choice(const std::string &title, const std::string &message,
    const std::vector<msg::Option> &option);
int big_choice(const std::string &title, const std::string &message,
    const std::vector<msg::Option> &option);
} // namespace fluid

#endif // FLUID_MESSAGE_H


