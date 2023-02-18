//
// Shortcut support routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

// Code to test and parse fltk shortcut numbers.
//
// A shortcut is a keysym or'd with shift flags.  In the simplest
// sense a shortcut is matched if the shift state is exactly as
// given and the key returning that keysym is pressed.
//
// To make it easier to match some things it is more complex:
//
// Only FL_META, FL_ALT, FL_SHIFT, and FL_CTRL must be "off".  A
// zero in the other shift flags indicates "don't care".
//
// It also checks against the first character of Fl::event_text(),
// and zero for FL_SHIFT means "don't care".
// This allows punctuation shortcuts like "#" to work (rather than
// calling it "shift+3" on a US keyboard)

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Button.H>
#include <FL/Fl.H>
#include "Fl_System_Driver.H"
#include "Fl_Screen_Driver.H"
#include <FL/fl_draw.H>
#include <stdlib.h>
#include <ctype.h>
#include "flstring.h"

/**
  Tests the current event, which must be an FL_KEYBOARD or
  FL_SHORTCUT, against a shortcut value (described in Fl_Button).

  Not to be confused with Fl_Widget::test_shortcut().

  \return non-zero if there is a match.
*/
int Fl::test_shortcut(unsigned int shortcut) {
  if (!shortcut) return 0;

  unsigned int v = shortcut & FL_KEY_MASK;
  if (((unsigned)fl_tolower(v))!=v) {
    shortcut |= FL_SHIFT;
  }

  int shift = Fl::event_state();
  // see if any required shift flags are off:
  if ((shortcut&shift) != (shortcut&0x7fff0000)) return 0;
  // record shift flags that are wrong:
  int mismatch = (shortcut^shift)&0x7fff0000;
  // these three must always be correct:
  if (mismatch&(FL_META|FL_ALT|FL_CTRL)) return 0;

  unsigned int key = shortcut & FL_KEY_MASK;

  // if shift is also correct, check for exactly equal keysyms:
  if (!(mismatch&(FL_SHIFT)) && key == (unsigned)Fl::event_key()) return 1;

  // try matching utf8, ignore shift:
  unsigned int firstChar = fl_utf8decode(Fl::event_text(), Fl::event_text()+Fl::event_length(), 0);
  if ( ! (FL_CAPS_LOCK&shift) && key==firstChar) return 1;

  // kludge so that Ctrl+'_' works (as opposed to Ctrl+'^_'):
  if ((shift&FL_CTRL) && key >= 0x3f && key <= 0x5F
      && firstChar==(key^0x40)) return 1; // firstChar should be within a-z
  return 0;
}

/**
  Get a human-readable string from a shortcut value.

  Unparse a shortcut value as used by Fl_Button or Fl_Menu_Item into
  a human-readable string like "Alt+N". This only works if the shortcut
  is a character key or a numbered function key. If the shortcut is
  zero then an empty string is returned. The return value points at
  a static buffer that is overwritten with each call.

  \since FLTK 1.3.4 modifier key names can be localized, but key names
    can not yet be localized. This may be added to a future FLTK version.

  Modifier key names (human-readable shortcut names) can be defined
  with the following global const char * pointer variables:

   - fl_local_ctrl  => name of FL_CTRL
   - fl_local_alt   => name of FL_ALT
   - fl_local_shift => name of FL_SHIFT
   - fl_local_meta  => name of FL_META

  \code
    fl_local_ctrl = "Strg";      // German for "Ctrl"
    fl_local_shift = "Umschalt"; // German for "Shift"
  \endcode

  \note Due to \b random static initialization order this should always
    be done from code in main() or called by main() as opposed to static
    initialization since the default strings in the FLTK library are set by
    static initializers. Otherwise this \b might result in the wrong order
    so FLTK's internal initialization overwrites your strings.

  The shortcut name will be constructed by adding all modifier names in the
  order defined above plus the name of the key. A '+' character is added to
  each modifier name unless it has a trailing '\' or a trailing '+'.

  Example:

    Ctrl+Alt+Shift+Meta+F12

  The default values for modifier key names are as given above for all
  platforms except macOS. macOS uses graphical characters that represent
  the typical macOS modifier names in menus, e.g. cloverleaf, saucepan, etc.
  You may, however, redefine macOS modifier names as well.

  \param [in] shortcut the integer value containing the ASCII character or extended keystroke plus modifiers
  \return a pointer to a static buffer containing human readable text for the shortcut
  */
const char* fl_shortcut_label(unsigned int shortcut) {
  return fl_shortcut_label(shortcut, 0L);
}

/*
  This static function adds a modifier key name to a character
  buffer and returns the pointer behind the modifier name and a
  trailing '+' character.

  Exceptions:
   (1) Last character = '\' : remove it, done (don't add '+')
   (2) Last character = '+' : user added '+', don't add another one

  In case of buffer overflow the modifier key name is replaced with "..."
  if that fits or not added at all. This should rarely (never) happen.
*/

static char *add_modifier_key(char *p, const char *end, const char *name) {
  size_t ln = strlen(name);
  if (p+ln > end) {             // string too long
    if (p+4 <= end) {           // can replace with "..." ?
      strcpy(p,"...");
      p += 3;
    } else
      return p;
  } else {
    strcpy(p,name);
    p += ln;
  }
  if (p[-1] == '\\')            // remove (last) '\' character
    p--;
  else if (p[-1] == '+')        // don't add another '+' character
    {/*empty*/}
  else                          // not a '\' or '+'
    *p++ = '+';                 // add a '+' character
  return p;
}

/**
  Get a human-readable string from a shortcut value.

  \param [in] shortcut the integer value containing the ASCII character or extended keystroke plus modifiers
  \param [in] eom if this pointer is set, it will receive a pointer to the end of the modifier text
  \return a pointer to a static buffer containing human readable text for the shortcut

  \see fl_shortcut_label(unsigned int shortcut)
*/

const char* fl_shortcut_label(unsigned int shortcut, const char **eom) {
  static char buf[80];
  char *p = buf;
  char *end = &buf[sizeof(buf)-20]; // account for key name (max. ~10 + x)
  if (eom) *eom = p;
  if (!shortcut) {*p = 0; return buf;}
  // fix upper case shortcuts
  unsigned int key = shortcut & FL_KEY_MASK;
  if (((unsigned)fl_tolower(key)) != key) {
    shortcut |= FL_SHIFT;
  }

  // Add modifier key names.
  // Note: if necessary we could change the order here depending on the platform.
  // However, as discussed in fltk.coredev, the order appears to be the
  // same on all platforms, with exceptions in _some_ Linux applications.

  if (shortcut & FL_CTRL)  {p = add_modifier_key(p, end, fl_local_ctrl);}
  if (shortcut & FL_ALT)   {p = add_modifier_key(p, end, fl_local_alt);}
  if (shortcut & FL_SHIFT) {p = add_modifier_key(p, end, fl_local_shift);}
  if (shortcut & FL_META)  {p = add_modifier_key(p, end, fl_local_meta);}
  if (eom) *eom = p;

  // add key name
  return Fl::screen_driver()->shortcut_add_key_name(key, p, buf, eom);
}

/**
  Emulation of XForms named shortcuts.

  Converts ASCII shortcut specifications (eg. "^c")
  into the FLTK integer equivalent (eg. FL_CTRL+'c')

  These ASCII characters are used to specify the various keyboard modifier keys:
  \verbatim
   # - Alt
   + - Shift
   ^ - Control
   ! - Meta
   @ - Command (Ctrl on linux/win, Meta on OSX)
  \endverbatim

  These special characters can be combined to form chords of modifier
  keys. (See 'Remarks' below)

  After the optional modifier key prefixes listed above, one can either
  specify a single keyboard character to use as the shortcut, or a
  numeric sequence in hex, decimal or octal.

  Examples:
  \verbatim
   "c"      -- Uses 'c' as the shortcut
   "#^c"    -- Same as FL_ALT|FL_CTRL|'c'
   "#^!c"   -- Same as FL_ALT|FL_CTRL|FL_META|'c'
   "@c"     -- Same as FL_COMMAND|'c' (see FL_COMMAND for platform specific behavior)
   "0x63"   -- Same as "c" (hex 63=='c')
   "99"     -- Same as "c" (dec 99=='c')
   "0143"   -- Same as "c" (octal 0143=='c')
   "^0x63"  -- Same as (FL_CTRL|'c'), or (FL_CTRL|0x63)
   "^99"    -- Same as (FL_CTRL|'c'), or (FL_CTRL|99)
   "^0143"  -- Same as (FL_CTRL|'c'), or (FL_CTRL|0143)
  \endverbatim

  \remarks
      Due to XForms legacy, there are some odd things to consider
      when using the modifier characters.
  \remarks
      (1) You can use the special modifier keys for chords *only*
      if the modifiers are provided in this order: #, +, ^, !, \@.
      Other ordering can yield undefined results.
  \remarks
      So for instance, Ctrl-Alt-c must be specified as "#^c" (and not
      "^#c"), due to the above ordering rule.
  \remarks
      (2) If you want to make a shortcut that uses one of the special
      modifier characters (as the character being modified), then to
      avoid confusion, specify the numeric equivalent, e.g.
  \remarks
  \verbatim
   If you want..                    Then use..
   -----------------------------    ------------------------------
   '#' as the shortcut..            "0x23"  (instead of just "#").
   '+' as the shortcut..            "0x2b"  (instead of just "+").
   '^' as the shortcut..            "0x5e"  (instead of just "^").
   Alt-+ as the shortcut..          "#0x2b" (instead of "#+").
   Alt-^ as the shortcut..          "#0x5e" (instead of "#^").
   ..etc..
  \endverbatim
  \remarks
      As a general rule that's easy to remember, unless the shortcut
      key to be modified is a single alpha-numeric character [A-Z,a-z,0-9),
      it's probably best to use the numeric equivalents.

  \remarks Don't fix these silly legacy issues in a future release.
      Nobody is using this anymore.
*/
unsigned int fl_old_shortcut(const char* s) {
  if (!s || !*s) return 0;
  if (s[1]==0 && strchr("@!",s[0])) return (unsigned int)s[0]; // maintain legacy behavior for "!" and "@"
  unsigned int n = 0;
  if (*s == '#') {n |= FL_ALT; s++;}
  if (*s == '+') {n |= FL_SHIFT; s++;}
  if (*s == '^') {n |= FL_CTRL; s++;}
  if (*s == '!') {n |= FL_META; s++;}     // added in 1.3.3
  if (*s == '@') {n |= FL_COMMAND; s++;}  // added in 1.3.3
  if (*s && s[1]) return n | (int)strtol(s,0,0); // allow 0xf00 to get any key
  return n | *s;
}

// Tests for &x shortcuts in button labels:

/** Returns the Unicode value of the '&x' shortcut in a given text.

  The given text \p t (usually a widget's label or a menu text) is
  searched for a '&x' shortcut label, and if found, the Unicode
  value (code point) of the '&x' shortcut is returned.

  \param t text or label to search for '&x' shortcut.

  \return Unicode (UCS-4) value of shortcut in \p t or 0.

  \note Internal use only.
*/
unsigned int Fl_Widget::label_shortcut(const char *t) {
  if (!t) return 0;
  for (;;) {
    if (*t==0) return 0;
    if (*t=='&') {
      unsigned int s = fl_utf8decode(t+1, 0, 0);
      if (s==0) return 0;
      else if (s==(unsigned int)'&') t++;
      else return s;
    }
    t++;
  }
}

/** Returns true if the given text \p t contains the entered '&x' shortcut.

  This method must only be called in handle() methods or callbacks after
  a keypress event (usually FL_KEYDOWN or FL_SHORTCUT). The given text
  \p t (usually a widget's label or menu text) is searched for a '&x'
  shortcut, and if found, this is compared with the entered key value.

  Fl::event_text() is used to get the entered key value.
  Fl::event_state() is used to get the Alt modifier, if \p require_alt
  is true.

  \param t text or label to search for '&x' shortcut.
  \param require_alt if true: match only if Alt key is pressed.

  \return true, if the entered text matches the '&x' shortcut in \p t
          false (0) otherwise.

  \note Useful when a widget's handle(int) method needs dedicated processing of FL_SHORTCUT.
*/
int Fl_Widget::test_shortcut(const char *t, const bool require_alt) {
  static int extra_test = Fl::system_driver()->need_test_shortcut_extra();
  if (!t) return 0;
  // for menubars etc. shortcuts must work only if the Alt modifier is pressed
  if (require_alt && Fl::event_state(FL_ALT)==0) return 0;
  unsigned int c = fl_utf8decode(Fl::event_text(), Fl::event_text()+Fl::event_length(), 0);
  // this line makes underline shortcuts work the same way they do on Windows
  // and Linux.
  if (extra_test && c && Fl::event_state(FL_ALT))
    c = Fl::event_key();
  if (!c) return 0;
  unsigned int ls = label_shortcut(t);
  if (c == ls)
    return 1;
  // On macOS, we need to simulate the upper case keystroke as well
  if (extra_test && Fl::event_state(FL_ALT) && c<128 && isalpha(c) && (unsigned)toupper(c)==ls)
    return 1;
  return 0;
}

/** Returns true if the widget's label contains the entered '&x' shortcut.

  This method must only be called in handle() methods or callbacks after
  a keypress event (usually FL_KEYDOWN or FL_SHORTCUT).
  The widget's label is searched for a '&x'
  shortcut, and if found, this is compared with the entered key value.

  Fl::event_text() is used to get the entered key value.

  \return true, if the entered text matches the widget's'&x' shortcut,
          false (0) otherwise.

 \note Useful when a widget's handle(int) method needs dedicated processing of FL_SHORTCUT.
*/

int Fl_Widget::test_shortcut() {
  if (!(flags()&SHORTCUT_LABEL)) return 0;
  return test_shortcut(label());
}

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

const char *Fl_Screen_Driver::shortcut_add_key_name(unsigned key, char *p, char *buf, const char **eom)
{
  if (key >= FL_F && key <= FL_F_Last) {
    *p++ = 'F';
    if (key > FL_F+9) *p++ = (char)((key-FL_F)/10+'0');
    *p++ = (char)((key-FL_F)%10 + '0');
  } else {
    // binary search the table for a match:
    int a = 0;
    int b = key_table_size;
    while (a < b) {
      int c = (a+b)/2;
      if (key_table[c].key == key) {
        if (p > buf) {
          strcpy(p, key_table[c].name);
          return buf;
        } else {
          const char *sp = key_table[c].name;
          if (eom) *eom = sp;
          return sp;
        }
      }
      if (key_table[c].key < key) a = c+1;
      else b = c;
    }
    if (key >= FL_KP && key <= FL_KP_Last) {
      // mark keypad keys with KP_ prefix
      strcpy(p,"KP_"); p += 3;
      *p++ = uchar(key & 127);
    } else {
      // if none found, use the keystroke as a match:
      p += fl_utf8encode(fl_toupper(key), p);
    }
  }
  *p = 0;
  return buf;
}

/**
 \}
 \endcond
 */
