//
// Code editor widget for the Fast Light Tool Kit (FLTK).
// Syntax highlighting rewritten by erco@seriss.com 09/15/20.
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

#ifndef CodeEditor_h
#define CodeEditor_h

//
// Include necessary headers...
//

#include "StyleParse.h"

#include <FL/Fl.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ---- CodeEditor declaration

/**
 A widget derived from Fl_Text_Editor that implements C++ code highlighting.

 CodeEditor is used in Fluid whenever the user can edit C++ source
 code or header text.
 */
class CodeEditor : public Fl_Text_Editor {
  friend class StyleParse;

  static Fl_Text_Display::Style_Table_Entry styletable[];
  static void style_parse(const char *tbuff, char *sbuff, int len, char style);
  static void style_unfinished_cb(int, void*);
  static void style_update(int pos, int nInserted, int nDeleted,
                           int /*nRestyled*/, const char * /*deletedText*/,
                           void *cbArg);
  static int auto_indent(int, CodeEditor* e);

public:
  CodeEditor(int X, int Y, int W, int H, const char *L=0);
  ~CodeEditor();
  void textsize(Fl_Fontsize s);

  /// access to protected member get_absolute_top_line_number()
  int top_line() { return get_absolute_top_line_number(); }

  /// access to protected member mTopLineNum
  int scroll_row() { return mTopLineNum; }

  /// access to protected member mHorizOffset
  int scroll_col() { return mHorizOffset; }
};

// ---- CodeViewer declaration

/**
 A widget derived from CodeEditor with highlighting for code blocks.

 This widget is used by the codeview system to show the design's
 source and header code. The secondary highlighting show the text
 part that corresponds to the selected widget(s).
 */
class CodeViewer : public CodeEditor {
public:
  CodeViewer(int X, int Y, int W, int H, const char *L=0);

protected:
  void draw() FL_OVERRIDE;

  /// Limit event handling to viewing, not editing
  int handle(int ev) FL_OVERRIDE { return Fl_Text_Display::handle(ev); }
};

// ---- Project File Text Viewer declaration

/**
 A text viewer with an additional highlighting color scheme.
 */
class TextViewer : public Fl_Text_Display {
public:
  TextViewer(int X, int Y, int W, int H, const char *L=0);
  ~TextViewer();
  void draw() FL_OVERRIDE;

  /// access to protected member get_absolute_top_line_number()
  int top_line() { return get_absolute_top_line_number(); }
};

#endif // !CodeEditor_h
