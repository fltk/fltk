//
// "$Id$"
//
// Code editor widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2005 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

//
// Include necessary headers...
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "CodeEditor.h"


Fl_Text_Display::Style_Table_Entry CodeEditor::
		styletable[] = {	// Style table
		  { FL_FOREGROUND_COLOR, FL_COURIER,        11 }, // A - Plain
		  { FL_DARK_GREEN,       FL_COURIER_ITALIC, 11 }, // B - Line comments
		  { FL_DARK_GREEN,       FL_COURIER_ITALIC, 11 }, // C - Block comments
		  { FL_BLUE,             FL_COURIER,        11 }, // D - Strings
		  { FL_DARK_RED,         FL_COURIER,        11 }, // E - Directives
		  { FL_DARK_RED,         FL_COURIER_BOLD,   11 }, // F - Types
		  { FL_BLUE,             FL_COURIER_BOLD,   11 }  // G - Keywords
		};
const char * const CodeEditor::
		code_keywords[] = {	// Sorted list of C/C++ keywords...
		  "and",
		  "and_eq",
		  "asm",
		  "bitand",
		  "bitor",
		  "break",
		  "case",
		  "catch",
		  "compl",
		  "continue",
		  "default",
		  "delete",
		  "do",
		  "else",
		  "false",
		  "for",
		  "goto",
		  "if",
		  "new",
		  "not",
		  "not_eq",
		  "operator",
		  "or",
		  "or_eq",
		  "return",
		  "switch",
		  "template",
		  "this",
		  "throw",
		  "true",
		  "try",
		  "while",
		  "xor",
		  "xor_eq"
		};
const char * const CodeEditor::
		code_types[] = {	// Sorted list of C/C++ types...
		  "auto",
		  "bool",
		  "char",
		  "class",
		  "const",
		  "const_cast",
		  "double",
		  "dynamic_cast",
		  "enum",
		  "explicit",
		  "extern",
		  "float",
		  "friend",
		  "inline",
		  "int",
		  "long",
		  "mutable",
		  "namespace",
		  "private",
		  "protected",
		  "public",
		  "register",
		  "short",
		  "signed",
		  "sizeof",
		  "static",
		  "static_cast",
		  "struct",
		  "template",
		  "typedef",
		  "typename",
		  "union",
		  "unsigned",
		  "virtual",
		  "void",
		  "volatile"
		};


// 'compare_keywords()' - Compare two keywords...
int CodeEditor::compare_keywords(const void *a, const void *b) {
  return (strcmp(*((const char **)a), *((const char **)b)));
}

// 'style_parse()' - Parse text and produce style data.
void CodeEditor::style_parse(const char *text, char *style, int length) {
  char		current;
  int		col;
  int		last;
  char		buf[255],
		*bufptr;
  const char	*temp;

  // Style letters:
  //
  // A - Plain
  // B - Line comments
  // C - Block comments
  // D - Strings
  // E - Directives
  // F - Types
  // G - Keywords

  for (current = *style, col = 0, last = 0; length > 0; length --, text ++) {
    if (current == 'B' || current == 'F' || current == 'G') current = 'A';
    if (current == 'A') {
      // Check for directives, comments, strings, and keywords...
      if (col == 0 && *text == '#') {
        // Set style to directive
        current = 'E';
      } else if (strncmp(text, "//", 2) == 0) {
        current = 'B';
	for (; length > 0 && *text != '\n'; length --, text ++) *style++ = 'B';

        if (length == 0) break;
      } else if (strncmp(text, "/*", 2) == 0) {
        current = 'C';
      } else if (strncmp(text, "\\\"", 2) == 0) {
        // Quoted quote...
	*style++ = current;
	*style++ = current;
	text ++;
	length --;
	col += 2;
	continue;
      } else if (*text == '\"') {
        current = 'D';
      } else if (!last && (islower(*text) || *text == '_')) {
        // Might be a keyword...
	for (temp = text, bufptr = buf;
	     (islower(*temp) || *temp == '_') && bufptr < (buf + sizeof(buf) - 1);
	     *bufptr++ = *temp++);

        if (!islower(*temp) && *temp != '_') {
	  *bufptr = '\0';

          bufptr = buf;

	  if (bsearch(&bufptr, code_types,
	              sizeof(code_types) / sizeof(code_types[0]),
		      sizeof(code_types[0]), compare_keywords)) {
	    while (text < temp) {
	      *style++ = 'F';
	      text ++;
	      length --;
	      col ++;
	    }

	    text --;
	    length ++;
	    last = 1;
	    continue;
	  } else if (bsearch(&bufptr, code_keywords,
	                     sizeof(code_keywords) / sizeof(code_keywords[0]),
		             sizeof(code_keywords[0]), compare_keywords)) {
	    while (text < temp) {
	      *style++ = 'G';
	      text ++;
	      length --;
	      col ++;
	    }

	    text --;
	    length ++;
	    last = 1;
	    continue;
	  }
	}
      }
    } else if (current == 'C' && strncmp(text, "*/", 2) == 0) {
      // Close a C comment...
      *style++ = current;
      *style++ = current;
      text ++;
      length --;
      current = 'A';
      col += 2;
      continue;
    } else if (current == 'D') {
      // Continuing in string...
      if (strncmp(text, "\\\"", 2) == 0) {
        // Quoted end quote...
	*style++ = current;
	*style++ = current;
	text ++;
	length --;
	col += 2;
	continue;
      } else if (*text == '\"') {
        // End quote...
	*style++ = current;
	col ++;
	current = 'A';
	continue;
      }
    }

    // Copy style info...
    if (current == 'A' && (*text == '{' || *text == '}')) *style++ = 'G';
    else *style++ = current;
    col ++;

    last = isalnum(*text) || *text == '_' || *text == '.';

    if (*text == '\n') {
      // Reset column and possibly reset the style
      col = 0;
      if (current == 'B' || current == 'E') current = 'A';
    }
  }
}

// 'style_unfinished_cb()' - Update unfinished styles.
void CodeEditor::style_unfinished_cb(int, void*) { }

// 'style_update()' - Update the style buffer...
void CodeEditor::style_update(int pos, int nInserted, int nDeleted,
                              int /*nRestyled*/, const char * /*deletedText*/,
                              void *cbArg) {
  CodeEditor	*editor = (CodeEditor *)cbArg;
  int		start,				// Start of text
		end;				// End of text
  char		last,				// Last style on line
		*style,				// Style data
		*text;				// Text data


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

  // Re-parse the changed region; we do this by parsing from the
  // beginning of the line of the changed region to the end of
  // the line of the changed region...  Then we check the last
  // style character and keep updating if we have a multi-line
  // comment character...
  start = editor->mBuffer->line_start(pos);
  end   = editor->mBuffer->line_end(pos + nInserted);
  text  = editor->mBuffer->text_range(start, end);
  style = editor->mStyleBuffer->text_range(start, end);
  if (start==end)
    last = 0;
  else
    last  = style[end - start - 1];

  style_parse(text, style, end - start);

  editor->mStyleBuffer->replace(start, end, style);
  editor->redisplay_range(start, end);

  if (start==end || last != style[end - start - 1]) {
    // The last character on the line changed styles, so reparse the
    // remainder of the buffer...
    free(text);
    free(style);

    end   = editor->mBuffer->length();
    text  = editor->mBuffer->text_range(start, end);
    style = editor->mStyleBuffer->text_range(start, end);

    style_parse(text, style, end - start);

    editor->mStyleBuffer->replace(start, end, style);
    editor->redisplay_range(start, end);
  }

  free(text);
  free(style);
}

int CodeEditor::auto_indent(int, CodeEditor* e) {
  if (e->buffer()->selected()) {
    e->insert_position(e->buffer()->primary_selection()->start());
    e->buffer()->remove_selection();
  }

  int pos = e->insert_position();
  int start = e->line_start(pos);
  char *text = e->buffer()->text_range(start, pos);
  char *ptr;

  for (ptr = text; isspace(*ptr); ptr ++);
  *ptr = '\0';  
  if (*text) {
    // use only a single 'insert' call to avoid redraw issues
    int n = strlen(text);
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
  if (e->when()&FL_WHEN_CHANGED) e->do_callback();

  free(text);

  return 1;
}

// Create a CodeEditor widget...
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

  style_parse(text, style, mBuffer->length());

  mStyleBuffer->text(style);
  delete[] style;
  free(text);

  mBuffer->add_modify_callback(style_update, this);
  add_key_binding(FL_Enter, FL_TEXT_EDITOR_ANY_STATE,
                  (Fl_Text_Editor::Key_Func)auto_indent);
}

// Destroy a CodeEditor widget...
CodeEditor::~CodeEditor() {
  Fl_Text_Buffer *buf = mStyleBuffer;
  mStyleBuffer = 0;
  delete buf;

  buf = mBuffer;
  buffer(0);
  delete buf;
}


//
// End of "$Id$".
//
