//
// Fl_Scheme header for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022-2025 by Bill Spitzak and others.
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

#ifndef _FL_Fl_Scheme_H_
#define _FL_Fl_Scheme_H_

#include <FL/Fl.H>

/**
  \brief Base class with static methods for future Fl_Scheme classes.

  This class is intentionally not fully documented and is subject to change
  in future FLTK versions.

  \note Do not rely on details of this class.

  \since 1.4.0
*/
class FL_EXPORT Fl_Scheme {

private:

  static const char **names_;           // registered scheme names
  static int num_schemes_;              // number of registered schemes
  static int alloc_size_;               // number of allocated scheme name entries

protected:

  // const char *name_;                 // the scheme's name

  // protected constructor - not yet implemented
  // Fl_Scheme(const char *name);

public:

  // Static methods.

  // Some of these methods will replace the scheme related methods of class Fl,
  // for instance Fl::scheme() and Fl::is_scheme().
  // Backwards compatibility must be kept though.

  static const char **names();

  /**
    Return the number of currently registered schemes.

    \return  Number of registered schemes.
  */
  static int num_schemes() {
    if (!names_) names(); // force initialization
    return num_schemes_;
  }

  // Adding a scheme name must be a public static method in (since) FLTK 1.4.0.
  // This will later be protected or replaced by another method name.

  static int add_scheme_name(const char *name);

  // Set color average parameter of the 'plastic' scheme.
  // See GitHub Issue 464: "RFE: plastic scheme with faithful colors".
  // See also documentation and implementation in src/fl_plastic.cxx.

  static void plastic_color_average(int av);

}; // class Fl_Scheme

#endif // _FL_Fl_Scheme_H_
