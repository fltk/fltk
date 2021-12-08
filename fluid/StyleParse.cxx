//
// Syntax highlighting for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2020 by Bill Spitzak and others.
// Copyright 2020 Greg Ercolano.
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

#include "StyleParse.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>     // bsearch()

// Sorted list of C/C++ keywords...
static const char * const code_keywords[] = {
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

// Sorted list of C/C++ types...
static const char * const code_types[] = {
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
extern "C" {
  static int compare_keywords(const void *a, const void *b) {
    return strcmp(*((const char **)a), *((const char **)b));
  }
}

// See if 'find' is a C/C++ keyword.
//     Refer to bsearch(3) for return value.
//
static void* search_keywords(char *find) {
  return bsearch(&find, code_keywords,
                 sizeof(code_keywords) / sizeof(code_keywords[0]),
                 sizeof(code_keywords[0]), compare_keywords);
}

// See if 'find' is a C/C++ type.
//     Refer to bsearch(3) for return value.
//
static void* search_types(char *find) {
  return bsearch(&find, code_types,
                 sizeof(code_types) / sizeof(code_types[0]),
                 sizeof(code_types[0]), compare_keywords);
}

// Handle style parsing over a character
//    Handles updating col counter when \n encountered.
//    Applies the current style, advances to next text + style char.
//    Returns 0 if hit end of buffer, 1 otherwise.
//
int StyleParse::parse_over_char(int handle_crlf) {
  char c = *tbuff;

  // End of line?
  if ( handle_crlf ) {
    if ( c == '\n' ) {
      lwhite = 1;           // restart leading white flag
    } else {
      // End of leading white? (used by #directive)
      if ( !strchr(" \t", c) ) lwhite = 0;
    }
  }

  // Adjust and advance
  //    If handling crlfs, zero col on crlf. If not handling, let col continue to count past crlf
  //    e.g. for multiline #define's that have lines ending in backslashes.
  //
  col = (c=='\n') ? (handle_crlf ? 0 : col) : col+1;   // column counter
  tbuff++;                              // advance text ptr
  *sbuff++ = style;                     // apply style & advance its ptr
  if ( --len <= 0 ) return 0;           // keep track of length
  return 1;
}

// Parse over white space using current style
//    Returns 0 if hit end of buffer, 1 otherwise.
//
int StyleParse::parse_over_white() {
  while ( len > 0 && strchr(" \t", *tbuff))
    { if ( !parse_over_char() ) return 0; }
  return 1;
}

// Parse over non-white alphabetic text
//    Returns 0 if hit end of buffer, 1 otherwise.
//
int StyleParse::parse_over_alpha() {
  while ( len > 0 && isalpha(*tbuff) )
    { if ( !parse_over_char() ) return 0; }
  return 1;
}

// Parse to end of line in specified style.
//    Returns 0 if hit end of buffer, 1 otherwise.
//
int StyleParse::parse_to_eol(char s) {
  char save = style;
  style = s;
  while ( *tbuff != '\n' )
    { if ( !parse_over_char() ) return 0; }
  style = save;
  return 1;
}

// Parse a block comment until end of comment or buffer.
//    Returns 0 if hit end of buffer, 1 otherwise.
//
int StyleParse::parse_block_comment() {
  char save = style;
  style = 'C';                            // block comment style
  while ( len > 0 ) {
    if ( strncmp(tbuff, "*/", 2) == 0 ) {
      if ( !parse_over_char() ) return 0; // handle '*'
      if ( !parse_over_char() ) return 0; // handle '/'
      break;
    }
    if ( !parse_over_char() ) return 0;   // handle comment text
  }
  style = save;                           // revert style
  return 1;
}

// Copy keyword from tbuff -> keyword[] buffer
void StyleParse::buffer_keyword() {
  char *key  = keyword;
  char *kend = key + sizeof(keyword) - 1; // end of buffer
  for ( const char *s=tbuff;
        (islower(*s) || *s=='_') && (key < kend);
        *key++ = *s++ ) { }
  *key = 0;     // terminate
}

// Parse over specified 'key'word in specified style 's'.
//    Returns 0 if hit end of buffer, 1 otherwise.
//
int StyleParse::parse_over_key(const char *key, char s) {
  char save = style;
  style = s;
  // Parse over the keyword while applying style to sbuff
  while ( *key++ )
    { if ( !parse_over_char() ) return 0; }
  last  = 1;
  style = save;
  return 1;
}

// Parse over angle brackets <..> in specified style.
//    Returns 0 if hit end of buffer, 1 otherwise.
//
int StyleParse::parse_over_angles(char s) {
  if ( *tbuff != '<' ) return 1; // not <..>, early exit
  char save = style;
  style = s;
  // Parse over angle brackets in specified style
  while ( len > 0 && *tbuff != '>' )
    { if ( !parse_over_char() ) return 0; }  // parse over '<' and angle content
  if ( !parse_over_char() ) return 0;        // parse over trailing '>'
  style = save;
  return 1;
}

// Parse line for possible keyword
//    spi.keyword[] will contain parsed word.
//    Returns 0 if hit end of buffer, 1 otherwise.
//
int StyleParse::parse_keyword() {
  // Parse into 'keyword' buffer
  buffer_keyword();
  char *key = keyword;
  // C/C++ type? (void, char..)
  if ( search_types(key) )
    return parse_over_key(key, 'F');           // 'type' style
  // C/C++ Keyword? (switch, return..)
  else if ( search_keywords(key) )
    return parse_over_key(key, 'G');           // 'keyword' style
  // Not a type or keyword? Parse over it
  return parse_over_key(key, style);
}

// Style parse a quoted string, either "" or ''.
//    Returns 0 if hit end of buffer, 1 otherwise.
//
int StyleParse::parse_quoted_string(char quote_char, // e.g. '"' or '\''
                                    char in_style) { // style for quoted text
  style = in_style;                      // start string style
  if ( !parse_over_char() ) return 0;    // parse over opening quote

  // Parse until closing quote reached
  char c;
  while ( len > 0 ) {
    c = tbuff[0];
    if ( c == quote_char ) {              // Closing quote? Parse and done
      if ( !parse_over_char() ) return 0; // close quote
      break;
    } else if ( c == '\\' ) {             // Escape sequence? Parse over, continue
      if ( !parse_over_char() ) return 0; // escape
      if ( !parse_over_char() ) return 0; // char being escaped
      continue;
    }
    // Keep parsing until end of buffer or closing quote..
    if ( !parse_over_char() ) return 0;
  }
  style = 'A';                            // revert normal style
  return 1;
}

// Style parse a directive (#include, #define..)
//    Returns 0 if hit end of buffer, 1 otherwise.
//
int StyleParse::parse_directive() {
  style = 'E';                             // start directive style
  if ( !parse_over_char()  )    return 0;  // Parse over '#'
  if ( !parse_over_white() )    return 0;  // Parse over any whitespace after '#'
  if ( !parse_over_alpha() )    return 0;  // Parse over the directive
  style = 'A';                             // revert normal style
  if ( !parse_over_white() )    return 0;  // Parse over white after directive
  if ( !parse_over_angles('D')) return 0;  // #include <..> (if any)
  return 1;
}

// Style parse a line comment to end of line.
//    Returns 0 if hit end of buffer, 1 otherwise.
//
int StyleParse::parse_line_comment() {
  return parse_to_eol('B');
}

// Parse a backslash escape character sequence.
//    Purposefully don't 'handle' \n, since an escaped \n should be
//    a continuation of a line, such as in a multiline #directive.
//    Returns 0 if hit end of buffer, 1 otherwise.
//
int StyleParse::parse_escape() {
  const char no_crlf = 0;
  if ( !parse_over_char(no_crlf) ) return 0;     // backslash
  if ( !parse_over_char(no_crlf) ) return 0;     // char escaped
  return 1;
}

// Parse all other non-specific characters
//    Returns 0 if hit end of buffer, 1 otherwise.
//
int StyleParse::parse_all_else() {
  last = isalnum(*tbuff) || *tbuff == '_' || *tbuff == '.';
  return parse_over_char();
}
