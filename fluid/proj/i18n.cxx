//
// Fluid Project Internationalization code for the Fast Light Tool Kit (FLTK).
//
// Copyright 2025 by Bill Spitzak and others.
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

#include "proj/i18n.h"

#include "io/Project_Reader.h"
#include "io/Project_Writer.h"

using namespace fld;

using namespace fld::proj;


/**
 Reset all project setting to create a new empty project.
 */
void I18n::reset() {
  type = fld::I18n_Type::NONE;

  gnu_include = "<libintl.h>";
  gnu_conditional = "";
  gnu_function = "gettext";
  gnu_static_function = "gettext_noop";

  posix_include = "<nl_types.h>";
  posix_conditional = "";
  posix_file = "";
  posix_set = "1";
}

void I18n::read(io::Project_Reader &f, const char *key) {
  if (!strcmp(key, "i18n_type")) {
    type = static_cast<fld::I18n_Type>(atoi(f.read_word()));
  } else if (!strcmp(key, "i18n_gnu_function")) {
    gnu_function = f.read_word();
  } else if (!strcmp(key, "i18n_gnu_static_function")) {
    gnu_static_function = f.read_word();
  } else if (!strcmp(key, "i18n_pos_file")) {
    posix_file = f.read_word();
  } else if (!strcmp(key, "i18n_pos_set")) {
    posix_set = f.read_word();
  } else if (!strcmp(key, "i18n_include")) {
    if (type == fld::I18n_Type::GNU) {
      gnu_include = f.read_word();
    } else if (type == fld::I18n_Type::POSIX) {
      posix_include = f.read_word();
    }
  } else if (!strcmp(key, "i18n_conditional")) {
    if (type == fld::I18n_Type::GNU) {
      gnu_conditional = f.read_word();
    } else if (type == fld::I18n_Type::POSIX) {
      posix_conditional = f.read_word();
    }
  }
}

void I18n::write(io::Project_Writer &f) const {
  if ((type != fld::I18n_Type::NONE)) {
    f.write_string("\ni18n_type %d", static_cast<int>(type));
    switch (type) {
      case fld::I18n_Type::NONE:
        break;
      case fld::I18n_Type::GNU : /* GNU gettext */
        f.write_string("\ni18n_include"); f.write_word(gnu_include.c_str());
        f.write_string("\ni18n_conditional"); f.write_word(gnu_conditional.c_str());
        f.write_string("\ni18n_gnu_function"); f.write_word(gnu_function.c_str());
        f.write_string("\ni18n_gnu_static_function"); f.write_word(gnu_static_function.c_str());
        break;
      case fld::I18n_Type::POSIX : /* POSIX catgets */
        f.write_string("\ni18n_include"); f.write_word(posix_include.c_str());
        f.write_string("\ni18n_conditional"); f.write_word(posix_conditional.c_str());
        if (!posix_file.empty()) {
          f.write_string("\ni18n_pos_file");
          f.write_word(posix_file.c_str());
        }
        f.write_string("\ni18n_pos_set"); f.write_word(posix_set.c_str());
        break;
    }
  }
}