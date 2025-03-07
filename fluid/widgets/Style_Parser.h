//
// Syntax highlighting for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

#ifndef FLUID_WIDGETS_STYLE_PARSER_H
#define FLUID_WIDGETS_STYLE_PARSER_H

namespace fld {
namespace widget {

// Class to manage style parsing, friend of Code_Editor
class Style_Parser {
public:
  const char *tbuff { nullptr };  // text buffer
  char       *sbuff { nullptr };  // style buffer
  int         len { 0 };          // running length
  char        style { 0 };        // current style
  char        lwhite { 1 };       // leading white space (1=white, 0=past white)
  int         col { 0 };          // line's column counter
  char        keyword[40] { };    // keyword parsing buffer
  char        last { 0 };         // flag for keyword parsing

  Style_Parser() = default;

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

} // namespace widget
} // namespace fld

#endif // FLUID_WIDGETS_STYLE_PARSER_H
