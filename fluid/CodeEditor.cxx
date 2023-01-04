//
// Code editor widget for the Fast Light Tool Kit (FLTK).
// Syntax highlighting rewritten by erco@seriss.com 09/15/20.
//
// Copyright 1998-2022 by Bill Spitzak and others.
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

//
// Include necessary headers...
//

#include "CodeEditor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ---- CodeEditor implementation

/** \class CodeEditor
 A widget derived from Fl_Text_Editor that implements C++ code highlighting.

 CodeEditor is used in Fluid whenever the user can edit C++ source
 code or header text.
 */

/**
 Lookup table for all supported styles.
 Every table entry describes a rendering style for the corresponding text.
 */
Fl_Text_Display::Style_Table_Entry CodeEditor::styletable[] = {   // Style table
                  { FL_FOREGROUND_COLOR, FL_COURIER,        11 }, // A - Plain
                  { FL_DARK_GREEN,       FL_COURIER_ITALIC, 11 }, // B - Line comments
                  { FL_DARK_GREEN,       FL_COURIER_ITALIC, 11 }, // C - Block comments
                  { FL_BLUE,             FL_COURIER,        11 }, // D - Strings
                  { FL_DARK_RED,         FL_COURIER,        11 }, // E - Directives
                  { FL_DARK_RED,         FL_COURIER_BOLD,   11 }, // F - Types
                  { FL_BLUE,             FL_COURIER_BOLD,   11 }, // G - Keywords
                  { 220, /* med cyan */  FL_COURIER,        11 }  // H - Single quote chars
                };

/**
 Parse text and produce style data.
 \param[in] in_tbuff text buffer to parse
 \param[inout] in_sbuff style buffer we modify
 \param[in] in_len byte length to parse
 \param[in] in_style starting style letter
 */
void CodeEditor::style_parse(const char *in_tbuff,         // text buffer to parse
                             char       *in_sbuff,         // style buffer we modify
                             int         in_len,           // byte length to parse
                             char        in_style) {       // starting style letter
  // Style letters:
  //
  // 'A' - Plain
  // 'B' - Line comments  // ..
  // 'C' - Block comments /*..*/
  // 'D' - Strings        "xxx"
  // 'E' - Directives     #define, #include..
  // 'F' - Types          void, char..
  // 'G' - Keywords       if, while..
  // 'H' - Chars          'x'

  StyleParse sp;
  sp.tbuff  = in_tbuff;
  sp.sbuff  = in_sbuff;
  sp.len    = in_len;
  sp.style  = in_style;
  sp.lwhite = 1;        // 1:while parsing over leading white and first char past, 0:past white
  sp.col    = 0;
  sp.last   = 0;

  // Loop through the code, updating style buffer
  char c;
  while ( sp.len > 0 ) {
    c = sp.tbuff[0];  // current char
    if ( sp.style == 'C' ) {                              // Started in middle of comment block?
      if ( !sp.parse_block_comment() ) break;
    } else if ( strncmp(sp.tbuff, "/*", 2)==0 ) {         // C style comment block?
      if ( !sp.parse_block_comment() ) break;
    } else if ( c == '\\' ) {                             // Backslash escape char?
      if ( !sp.parse_escape() ) break;
    } else if ( strncmp(sp.tbuff, "//", 2)==0 ) {         // Line comment?
      if ( !sp.parse_line_comment() ) break;
    } else if ( c == '"' ) {                              // Start of double quoted string?
      if ( !sp.parse_quoted_string('"', 'D') ) break;
    } else if ( c == '\'' ) {                             // Start of single quoted string?
      if ( !sp.parse_quoted_string('\'', 'H') ) break;
    } else if ( c == '#' && sp.lwhite ) {                 // Start of '#' directive?
      if ( !sp.parse_directive() ) break;
    } else if ( !sp.last && (islower(c) || c == '_') ) {  // Possible C/C++ keyword?
      if ( !sp.parse_keyword() ) break;
    } else {                                              // All other chars?
      if ( !sp.parse_all_else() ) break;
    }
  }
}

/**
 Update unfinished styles.
 */
void CodeEditor::style_unfinished_cb(int, void*) {
}

/**
 Update the style buffer.
 \param[in] pos insert position in text
 \param[in] nInserted number of bytes inserted
 \param[in] nDeleted number of bytes deleted
 \param[in] cbArg pointer back to the code editr
 */
void CodeEditor::style_update(int pos, int nInserted, int nDeleted,
                              int /*nRestyled*/, const char * /*deletedText*/,
                              void *cbArg) {
  CodeEditor *editor = (CodeEditor*)cbArg;
  char       *style,                         // Style data
             *text;                          // Text data


  // If this is just a selection change, just unselect the style buffer...
  if (nInserted == 0 && nDeleted == 0) {
    editor->mStyleBuffer->unselect();
    return;
  }

  // Track changes in the text buffer...
  if (nInserted > 0) {
    // Insert characters into the style buffer...
    style = new char[nInserted + 1];
    memset(style, 'A', nInserted);
    style[nInserted] = '\0';

    editor->mStyleBuffer->replace(pos, pos + nDeleted, style);
    delete[] style;
  } else {
    // Just delete characters in the style buffer...
    editor->mStyleBuffer->remove(pos, pos + nDeleted);
  }

  // Select the area that was just updated to avoid unnecessary
  // callbacks...
  editor->mStyleBuffer->select(pos, pos + nInserted - nDeleted);

  // Reparse whole buffer, don't get cute. Maybe optimize range later
  int len = editor->buffer()->length();
  text  = editor->mBuffer->text_range(0, len);
  style = editor->mStyleBuffer->text_range(0, len);

  style_parse(text, style, editor->mBuffer->length(), 'A');

  editor->mStyleBuffer->replace(0, len, style);
  editor->redisplay_range(0, len);
  editor->redraw();

  free(text);
  free(style);
}

/**
 Find the right indentation depth after pressing the Enter key.
 \param[in] e pointer back to the code editor
 */
int CodeEditor::auto_indent(int, CodeEditor* e) {
  if (e->buffer()->selected()) {
    e->insert_position(e->buffer()->primary_selection()->start());
    e->buffer()->remove_selection();
  }

  int pos = e->insert_position();
  int start = e->line_start(pos);
  char *text = e->buffer()->text_range(start, pos);
  char *ptr;

  for (ptr = text; isspace(*ptr); ptr ++) {/*empty*/}
  *ptr = '\0';
  if (*text) {
    // use only a single 'insert' call to avoid redraw issues
    size_t n = strlen(text);
    char *b = (char*)malloc(n+2);
    *b = '\n';
    strcpy(b+1, text);
    e->insert(b);
    free(b);
  } else {
    e->insert("\n");
  }
  e->show_insert_position();
  e->set_changed();
  if (e->when()&FL_WHEN_CHANGED) e->do_callback(FL_REASON_CHANGED);

  free(text);

  return 1;
}

/**
 Create a CodeEditor widget.
 \param[in] X, Y, W, H position and size of the widget
 \param[in] L optional label
 */
CodeEditor::CodeEditor(int X, int Y, int W, int H, const char *L) :
  Fl_Text_Editor(X, Y, W, H, L) {
  buffer(new Fl_Text_Buffer);

  char *style = new char[mBuffer->length() + 1];
  char *text = mBuffer->text();

  memset(style, 'A', mBuffer->length());
  style[mBuffer->length()] = '\0';

  highlight_data(new Fl_Text_Buffer(mBuffer->length()), styletable,
                 sizeof(styletable) / sizeof(styletable[0]),
                 'A', style_unfinished_cb, this);

  style_parse(text, style, mBuffer->length(), 'A');

  mStyleBuffer->text(style);
  delete[] style;
  free(text);

  mBuffer->add_modify_callback(style_update, this);
  add_key_binding(FL_Enter, FL_TEXT_EDITOR_ANY_STATE,
                  (Fl_Text_Editor::Key_Func)auto_indent);
}

/**
 Destroy a CodeEditor widget.
 */
CodeEditor::~CodeEditor() {
  Fl_Text_Buffer *buf = mStyleBuffer;
  mStyleBuffer = 0;
  delete buf;

  buf = mBuffer;
  buffer(0);
  delete buf;
}

/**
 Attempt to make the fluid code editor widget honour textsize setting.
 This works by updating the fontsizes in the style table.
 \param[in] s the new general height of the text font
 */
void CodeEditor::textsize(Fl_Fontsize s) {
  Fl_Text_Editor::textsize(s); // call base class method
  // now attempt to update our styletable to honour the new size...
  int entries = sizeof(styletable) / sizeof(styletable[0]);
  for(int iter = 0; iter < entries; iter++) {
    styletable[iter].size = s;
  }
} // textsize

// ---- CodeViewer implementation

/** \class CodeViewer
 A widget derived from CodeEditor with highlighting for code blocks.

 This widget is used by the SourceView system to show the design's
 source and header code. The secondary highlighting show the text
 part that corresponds to the selected widget(s).
 */

/**
 Create a CodeViewer widget.
 \param[in] X, Y, W, H position and size of the widget
 \param[in] L optional label
 */
CodeViewer::CodeViewer(int X, int Y, int W, int H, const char *L)
: CodeEditor(X, Y, W, H, L)
{
  default_key_function(kf_ignore);
  remove_all_key_bindings(&key_bindings);
  cursor_style(CARET_CURSOR);
}

/**
 Tricking Fl_Text_Display into using bearable colors for this specific task.
 */
void CodeViewer::draw()
{
  Fl_Color c = Fl::get_color(FL_SELECTION_COLOR);
  Fl::set_color(FL_SELECTION_COLOR, fl_color_average(FL_BACKGROUND_COLOR, FL_FOREGROUND_COLOR, 0.9f));
  CodeEditor::draw();
  Fl::set_color(FL_SELECTION_COLOR, c);
}

// ---- TextViewer implementation

/**
 Create a TextViewer widget.
 \param[in] X, Y, W, H position and size of the widget
 \param[in] L optional label
 */
TextViewer::TextViewer(int X, int Y, int W, int H, const char *L)
: Fl_Text_Display(X, Y, W, H, L)
{
  buffer(new Fl_Text_Buffer);
}

/**
 Avoid memory leaks.
 */
TextViewer::~TextViewer() {
  Fl_Text_Buffer *buf = mBuffer;
  buffer(0);
  delete buf;
}
