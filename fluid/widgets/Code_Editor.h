//
// Code editor widget for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_WIDGETS_CODE_EDITOR_H
#define FLUID_WIDGETS_CODE_EDITOR_H

// Syntax highlighting rewritten by erco@seriss.com 09/15/20.

//
// Include necessary headers...
//

#include "Style_Parser.h"

#include <FL/Fl_Text_Editor.H>

namespace fld {
namespace widget {

// ---- Code_Editor declaration

/**
 A widget derived from Fl_Text_Editor that implements C++ code highlighting.

 Code_Editor is used in Fluid whenever the user can edit C++ source
 code or header text.
 */
class Code_Editor : public Fl_Text_Editor {
  friend class Style_Parser;

  static Fl_Text_Display::Style_Table_Entry styletable[];
  static void style_parse(const char *tbuff, char *sbuff, int len, char style);
  static void style_unfinished_cb(int, void*);
  static void style_update(int pos, int nInserted, int nDeleted,
                           int /*nRestyled*/, const char * /*deletedText*/,
                           void *cbArg);
  static int auto_indent(int, Code_Editor* e);

public:
  Code_Editor(int X, int Y, int W, int H, const char *L=nullptr);
  ~Code_Editor();
  void textsize(Fl_Fontsize s);

  /// access to protected member get_absolute_top_line_number()
  int top_line() { return get_absolute_top_line_number(); }

  /// access to protected member mTopLineNum
  int scroll_row() { return mTopLineNum; }

  /// access to protected member mHorizOffset
  int scroll_col() { return mHorizOffset; }
};

} // namespace widget
} // namespace fld


#endif // FLUID_WIDGETS_CODE_EDITOR_H
