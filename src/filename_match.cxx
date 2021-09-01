//
// Pattern matching routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2020 by Bill Spitzak and others.
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

/* Adapted from Rich Salz. */
#include <FL/filename.H>
#include <ctype.h>

/**
    Checks if a string \p s matches a pattern \p p.
    The following syntax is used for the pattern:
    - * matches any sequence of 0 or more characters.
    - ? matches any single character.
    - [set] matches any character in the set. Set can contain any single characters, or a-z to represent a range.
      To match ] or - they must be the first characters. To match ^ or ! they must not be the first characters.
    - [^set] or [!set] matches any character not in the set.
    - {X|Y|Z} or {X,Y,Z} matches any one of the subexpressions literally.
    - \\x quotes the character x so it has no special meaning.
    - x all other characters are matched "exactly" on a \b case-insensitive basis.

    Notes:
    - \p s and \p p are matched on a char/byte basis, not as UCS codepoints or UTF-8 sequences.
    - [set] ranges must run from low to high, i.e. [a-z] and not [z-a]
    - [set] comparison is \b case-sensitive, i.e. [a-z] won't match "A".
    - \\x only applies to the fl_filename_match special characters * ? [ {
    - \\x needs a double \\ or the compiler will complain about non-standard escape sequences.

    \b Include:
    \code
    #include <FL/filename.H>
    \endcode

    \param[in] s the string to check for a match
    \param[in] p the string pattern
    \return non zero if the string matches the pattern
*/
int fl_filename_match(const char *s, const char *p) {
  int matched;

  for (;;) {
    switch(*p++) {

    case '?' :  // match any single character
      if (!*s++) return 0;
      break;

    case '*' :  // match 0-n of any characters
      if (!*p) return 1; // do trailing * quickly
      while (!fl_filename_match(s, p)) if (!*s++) return 0;
      return 1;

    case '[': { // match one character in set of form [abc-d] or [^a-b]
      if (!*s) return 0;
      int reverse = (*p=='^' || *p=='!'); if (reverse) p++;
      matched = 0;
      char last = 0;
      while (*p) {
        if (*p=='-' && last) {
          if (*s <= *++p && *s >= last ) matched = 1;
          last = 0;
        } else {
          if (*s == *p) matched = 1;
        }
        last = *p++;
        if (*p==']') break;
      }
      if (matched == reverse) return 0;
      s++; p++;}
    break;

    case '{' : // {pattern1|pattern2|pattern3}
    NEXTCASE:
    if (fl_filename_match(s,p)) return 1;
    for (matched = 0;;) {
      switch (*p++) {
      case '\\': if (*p) p++; break;
      case '{': matched++; break;
      case '}': if (!matched--) return 0; break;
      case '|': case ',': if (matched==0) goto NEXTCASE;
      case 0: return 0;
      }
    }
    case '|':   // skip rest of |pattern|pattern} when called recursively
    case ',':
      for (matched = 0; *p && matched >= 0;) {
        switch (*p++) {
        case '\\': if (*p) p++; break;
        case '{': matched++; break;
        case '}': matched--; break;
        }
      }
      break;
    case '}':
      break;

    case 0:     // end of pattern
      return !*s;

    case '\\':  // quote next character
      if (*p) p++;
      /* FALLTHROUGH */
    default:
      if (tolower(*s) != tolower(*(p-1))) return 0;
      s++;
      break;
    }
  }
}
