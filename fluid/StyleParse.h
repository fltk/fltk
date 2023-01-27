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

#ifndef StyleParse_h
#define StyleParse_h

// Class to manage style parsing, friend of CodeEditor
class StyleParse {
public:
  const char *tbuff;        // text buffer
  char       *sbuff;        // style buffer
  int         len;          // running length
  char        style;        // current style
  char        lwhite;       // leading white space (1=white, 0=past white)
  int         col;          // line's column counter
  char        keyword[40];  // keyword parsing buffer
  char        last;         // flag for keyword parsing

  StyleParse() {
    tbuff  = 0;
    sbuff  = 0;
    len    = 0;
    style  = 0;
    lwhite = 1;
    col    = 0;
    last   = 0;
  }

  // Methods to aid in parsing
  int  parse_over_char(int handle_crlf=1);
  int  parse_over_white();
  int  parse_over_alpha();
  int  parse_to_eol(char s);
  int  parse_block_comment();     // "/* text.. */"
  void buffer_keyword();
  int  parse_over_key(const char *key, char s);
  int  parse_over_angles(char s);
  int  parse_keyword();           // "switch"
  int  parse_quoted_string(char quote_char, char in_style);
                                  // "hello", 'x'
  int  parse_directive();         // "#define"
  int  parse_line_comment();      // "// text.."
  int  parse_escape();            // "\'"
  int  parse_all_else();          // all other code
};

#endif // StyleParse_h
