//
// "$Id$"
//
// Character compose processing for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/x.H>


#if !defined(__APPLE__) && !defined(WIN32)

static const char* const compose_pairs =
"=E  _'f _\"..+ ++^ %%^S< OE  ^Z    ^''^^\"\"^-*- --~ TM^s> oe  ^z:Y" 
"  ! % # $ y=| & : c a <<~ - r _ * +-2 3 ' u p . , 1 o >>141234? "
"`A'A^A~A:A*AAE,C`E'E^E:E`I'I^I:I-D~N`O'O^O~O:Ox O/`U'U^U:U'YTHss"
"`a'a^a~a:a*aae,c`e'e^e:e`i'i^i:i-d~n`o'o^o~o:o-:o/`u'u^u:u'yth:y";

#endif

#ifndef FL_DOXYGEN
int Fl::compose_state = 0;
#endif

#if defined(__APPLE__) || defined(WIN32)
// under Mac OS X character composition is handled by the OS
int Fl::compose(int& del) {
  if(Fl::e_length == 0 || Fl::e_keysym == FL_Enter || Fl::e_keysym == FL_KP_Enter || 
     Fl::e_keysym == FL_Tab || Fl::e_keysym == FL_Escape || Fl::e_state&FL_META || Fl::e_state&FL_CTRL) {
    del = 0;
    return 0;
    }
  if(Fl::compose_state) {
    del = 1;
    Fl::compose_state = 0;
    }
  else {
    del = 0;
    }
  return 1;
}

#else

/** Any text editing widget should call this for each FL_KEYBOARD event.
    Use of this function is very simple.

    <p>If <i>true</i> is returned, then it has modified the
    Fl::event_text() and Fl::event_length() to a set of <i>bytes</i> to
    insert (it may be of zero length!).  In will also set the "del"
    parameter to the number of <i>bytes</i> to the left of the cursor to
    delete, this is used to delete the results of the previous call to
    Fl::compose().
    
    <p>If <i>false</i> is returned, the keys should be treated as function
    keys, and del is set to zero. You could insert the text anyways, if
    you don't know what else to do.
    
    <p>Though the current implementation returns immediately, future
    versions may take quite awhile, as they may pop up a window or do
    other user-interface things to allow characters to be selected.
*/
int Fl::compose(int& del) {

  del = 0;
  unsigned char ascii = (unsigned)e_text[0];

  // Alt+letters are reserved for shortcuts.  But alt+foreign letters
  // has to be allowed, because some key layouts require alt to be held
  // down in order to type them...
  //
  // OSX users sometimes need to hold down ALT for keys, so we only check
  // for META on OSX...
  if ((e_state & (FL_ALT|FL_META)) && !(ascii & 128)) return 0;

  if (compose_state == 1) { // after the compose key
    if ( // do not get distracted by any modifier keys
      e_keysym==FL_Shift_L||
      e_keysym==FL_Shift_R ||
      e_keysym==FL_Alt_L ||
      e_keysym==FL_Alt_R ||
      e_keysym==FL_Meta_L ||
      e_keysym==FL_Meta_R ||
      e_keysym==FL_Control_R ||
      e_keysym==FL_Control_L ||
      e_keysym==FL_Menu
      ) return 0;

    if (ascii == ' ') { // space turns into nbsp
      int len = fl_utf8encode(0xA0, e_text);
      e_text[len] = '\0';
      e_length = len;
      compose_state = 0;
      return 1;
    } else if (ascii < ' ' || ascii == 127) {
      compose_state = 0;
      return 0;
    }

    // see if it is either character of any pair:
    for (const char *p = compose_pairs; *p; p += 2)
      if (p[0] == ascii || p[1] == ascii) {
       if (p[1] == ' ') {
               int len = fl_utf8encode((p-compose_pairs)/2+0xA0, e_text);
               e_text[len] = '\0';
               e_length = len;
       }

	compose_state = ascii;
	return 1;
      }

    if (e_length) { // compose key also "quotes" control characters
      compose_state = 0;
      return 1;
    }

  } else if (compose_state) { // second character of compose

    char c1 = char(compose_state); // retrieve first character
    // now search for the pair in either order:
    for (const char *p = compose_pairs; *p; p += 2) {
      if ( (p[0] == ascii && p[1] == c1) || (p[1] == ascii && p[0] == c1)) {
        int len = fl_utf8encode((p-compose_pairs)/2+0xA0, e_text);
        e_text[len] = '\0';
        e_length = len;
	del = 1; // delete the old character and insert new one
	compose_state = 0;
	return 1;
      }
    }

  }

  int i = e_keysym;

  // See if they type the compose prefix key:
  if (i == FL_Control_R || i == 0xff20/* Multi-Key */) {
    compose_state = 1;
    return 1;
  }

  // See if they typed a dead key.  This gets it into the same state as
  // typing prefix+accent:
  if (i >= 0xfe50 && i <= 0xfe5b) {
    ascii = e_text[0];
    for (const char *p = compose_pairs; *p; p += 2)
      if (p[0] == ascii ||
          (p[1] == ' ' && (p - compose_pairs) / 2 + 0xA0 == ascii)) {
        compose_state = p[0];
        return 1;
      }
    compose_state = 0;
    return 1;
  }

  // Only insert non-control characters:
  if (e_length && (ascii & ~31 && ascii!=127)) {compose_state = 0; return 1;}

  return 0;
}

#endif // __APPLE__ || WIN32

//
// End of "$Id$"
//

