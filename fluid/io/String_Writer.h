//
// String File Writer header for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_IO_STRING_WRITER_H
#define FLUID_IO_STRING_WRITER_H

#include <string>

namespace fld {

class Project;

namespace io {

int write_strings(Project &proj, const std::string &filename);

} // namespace io
} // namespace fld

#endif // FLUID_IO_STRING_WRITER_H
