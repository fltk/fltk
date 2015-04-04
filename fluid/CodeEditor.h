//
// "$Id$"
//
// Code editor widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

#ifndef CodeEditor_h
#  define CodeEditor_h

//
// Include necessary headers...
//

#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#  include <ctype.h>
#  include <FL/Fl.H>
#  include <FL/Fl_Text_Buffer.H>
#  include <FL/Fl_Text_Editor.H>


class CodeEditor : public Fl_Text_Editor {
  static Fl_Text_Display::Style_Table_Entry styletable[];
  static const char * const code_keywords[];
  static const char * const code_types[];


  // 'style_parse()' - Parse text and produce style data.
  static void style_parse(const char *text, char *style, int length);

  // 'style_unfinished_cb()' - Update unfinished styles.
  static void style_unfinished_cb(int, void*);

  // 'style_update()' - Update the style buffer...
  static void style_update(int pos, int nInserted, int nDeleted,
                           int /*nRestyled*/, const char * /*deletedText*/,
                           void *cbArg);

  static int auto_indent(int, CodeEditor* e);

  public:

  CodeEditor(int X, int Y, int W, int H, const char *L=0);
  ~CodeEditor();
  int top_line() { return get_absolute_top_line_number(); }

  // attempt to make the fluid code editor widget honour textsize setting
  void textsize(Fl_Fontsize s);
  
};

class CodeViewer : public CodeEditor {

  public:

  CodeViewer(int X, int Y, int W, int H, const char *L=0);

  protected:

  int handle(int ev) { return Fl_Text_Display::handle(ev); }
  void draw();
};

#endif // !CodeEditor_h

//
// End of "$Id$".
//
