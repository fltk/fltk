//
// "$Id"
//
// Input widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

// This is the "user interface", it decodes user actions into what to
// do to the text.  See also Fl_Input_.C, where the text is actually
// manipulated (and some ui, in particular the mouse, is done...).
// In theory you can replace this code with another subclass to change
// the keybindings.

#include <FL/Fl.H>
#include <FL/Fl_Input.H>
#include <FL/fl_draw.H>
#include <math.h>
#include <string.h>
#include <ctype.h>

void Fl_Input::draw() {
  if (type() == FL_HIDDEN_INPUT) return;
  Fl_Boxtype b = box() ? box() : default_box();
  if (damage() & FL_DAMAGE_ALL) draw_box(b, color());
  Fl_Input_::drawtext(x()+Fl::box_dx(b)+3, y()+Fl::box_dy(b),
		      w()-Fl::box_dw(b)-6, h()-Fl::box_dh(b));
}

// kludge so shift causes selection to extend:
int Fl_Input::shift_position(int p) {
  return position(p, Fl::event_state(FL_SHIFT) ? mark() : p);
}
int Fl_Input::shift_up_down_position(int p) {
  return up_down_position(p, Fl::event_state(FL_SHIFT));
}

////////////////////////////////////////////////////////////////
// Fltk "compose"
// I tried to do compose characters "correctly" with much more user
// feedback.  They can see the character they will get, rather than
// the "dead key" effect.  Notice that I completely ignore that horrid
// XIM extension!
// Although the current scheme only works for Latin-NR1 character sets
// the intention is to expand this to UTF-8 someday, to allow you to
// compose all characters in all languages with no stupid "locale"
// setting.
// To use, you call "fl_compose()" for each keystroke.  You pass it
// the characters it displayed last time plus the new character.  It
// returns a new set of characters to replace the old one with.  If
// it returns zero length you should leave the old set unchanged and
// treat the new key normally.
// Pressing any function keys or moving the cursor should set the
// compose state back to zero.

// This string lists a pair for each possible foreign letter in Latin-NR1
// starting at code 0xa0 (nbsp).  If the second character is a space then
// only the first character needs to by typed:
static const char* const compose_pairs =
"  ! % # $ y=| & : c a <<~ - r _ * +-2 3 ' u p . , 1 o >>141234? "
"A`A'A^A~A:A*AEC,E`E'E^E:I`I'I^I:D-N~O`O'O^O~O:x O/U`U'U^U:Y'DDss"
"a`a'a^a~a:a*aec,e`e'e^e:i`i'i^i:d-n~o`o'o^o~o:-:o/u`u'u^u:y'ddy:";

int fl_compose(int state, char c, int& del, char* buffer, int& ins) {
  del = 0; ins = 1; buffer[0] = c;

  if (c == '"') c = ':';

  if (!state) {	// first character
    if (c == ' ') {buffer[0]=char(0xA0);return 0x100;} // space turns into nbsp
    // see if it is either character of any pair:
    state = 0;
    for (const char *p = compose_pairs; *p; p += 2) 
      if (p[0] == c || p[1] == c) {
	if (p[1] == ' ') buffer[0] = (p-compose_pairs)/2+0xA0;
	state = c;
      }
    return state;

  } else if (state == 0x100) { // third character
    return 0;

  } else { // second character
    char c1 = char(state); // first character
    // now search for the pair in either order:
    for (const char *p = compose_pairs; *p; p += 2) {
      if (p[0] == c && p[1] == c1 || p[1] == c && p[0] == c1) {
	buffer[0] = (p-compose_pairs)/2+0xA0;
	ins = del = 1;
	return 0x100;
      }
    }
    return 0;
  }
}

////////////////////////////////////////////////////////////////

static int compose; // compose state (# of characters so far + 1)

// If you define this symbol as zero you will get the peculiar fltk
// behavior where moving off the end of an input field will move the
// cursor into the next field:
// define it as 1 to prevent cursor movement from going to next field:
#define NORMAL_INPUT_MOVE 0

#define ctrl(x) (x^0x40)

int Fl_Input::handle_key() {
  int i;

  int pcompose = compose; compose = 0;
  char key = Fl::event_text()[0];

  if (pcompose && Fl::event_length()) {
    char buf[20]; int ins; int del;
    compose = fl_compose(pcompose-1, key, del, buf, ins);
    if (compose) {
      replace(position(), del ? position()-del : mark(), buf, ins);
      compose++; // store value+1 so 1 can initialize compose state
      return 1;
    } else {
      if (pcompose==1)	// compose also acts as quote-next:
	return replace(position(),mark(),Fl::event_text(),Fl::event_length());
    }
  }

  if (Fl::event_state(FL_ALT|FL_META)) { // reserved for shortcuts
    compose = pcompose;
    return 0;
  }

  switch (Fl::event_key()) {
  case FL_Left:
    key = ctrl('B'); break;
  case FL_Right:
    key = ctrl('F'); break;
  case FL_Up:
    key = ctrl('P'); break;
  case FL_Down:
    key = ctrl('N'); break;
  case FL_Delete:
    key = ctrl('D'); break;
  case FL_Home:
    key = ctrl('A'); break;
  case FL_End:
    key = ctrl('E'); break;
  case FL_BackSpace:
    if (mark() != position()) cut();
    else cut(-1);
    return 1;
  case FL_Enter:
  case FL_KP_Enter:
    if (when() & FL_WHEN_ENTER_KEY) {
      position(size(), 0);
      maybe_do_callback();
      return 1;
    } else if (type() == FL_MULTILINE_INPUT)
      return replace(position(), mark(), "\n", 1);
    else 
      return 0;	// reserved for shortcuts
  case FL_Tab:
    if (Fl::event_state(FL_CTRL) || type()!=FL_MULTILINE_INPUT) return 0;
    break;
  case FL_Escape:
    return 0;	// reserved for shortcuts (Forms cleared field)
  case FL_Control_R:
  case 0xff20: // Multi-Key
    compose = 1;
    return 1;
  }

  switch(key) {
  case 0:	// key did not translate to any text
    compose = pcompose; // allow user to hit shift keys after ^Q
    return 0;
  case ctrl('A'):
    if (type() == FL_MULTILINE_INPUT)
      for (i=position(); i && index(i-1)!='\n'; i--) ;
    else
      i = 0;
    return shift_position(i) + NORMAL_INPUT_MOVE;
  case ctrl('B'):
    return shift_position(position()-1) + NORMAL_INPUT_MOVE;
  case ctrl('C'): // copy
    return copy();
  case ctrl('D'):
    if (mark() != position()) return cut();
    else return cut(1);
  case ctrl('E'):
    if (type() == FL_MULTILINE_INPUT)
      for (i=position(); index(i) && index(i)!='\n'; i++) ;
    else
      i = size();
    return shift_position(i) + NORMAL_INPUT_MOVE;
  case ctrl('F'):
    return shift_position(position()+1) + NORMAL_INPUT_MOVE;
  case ctrl('K'):
    if (position()>=size()) return 0;
    if (type() == FL_MULTILINE_INPUT) {
      if (index(position()) == '\n')
	i = position() + 1;
      else 
	for (i=position()+1; index(i) && index(i) != '\n'; i++);
    } else
      i = size();
    cut(position(), i);
    return copy_cuts();
  case ctrl('N'):
    if (type()!=FL_MULTILINE_INPUT) return 0;
    for (i=position(); index(i)!='\n'; i++)
      if (!index(i)) return NORMAL_INPUT_MOVE;
    shift_up_down_position(i+1);
    return 1;
  case ctrl('P'):
    if (type()!=FL_MULTILINE_INPUT) return 0;
    for (i = position(); i > 0 && index(i-1) != '\n'; i--) ;
    if (!i) return NORMAL_INPUT_MOVE;
    shift_up_down_position(i-1);
    return 1;
  case ctrl('Q'):
    compose = 1;
    return 1;
  case ctrl('U'):
    return cut(0, size());
  case ctrl('V'):
  case ctrl('Y'):
    Fl::paste(*this);
    return 1;
  case ctrl('X'):
  case ctrl('W'):
    copy();
    return cut();
  case ctrl('Z'):
  case ctrl('_'):
    return undo();
  }

  // skip all illegal characters
  // this could be improved to make sure characters are inserted at
  // legal positions...
  if (type() == FL_FLOAT_INPUT) {
    if (!strchr("0123456789.eE+-", key)) return 0;
  } else if (type() == FL_INT_INPUT) {
    if (!strchr("0123456789+-", key)) return 0;
  }

  return replace(position(), mark(), Fl::event_text(), Fl::event_length());
}

int Fl_Input::handle(int event) {
  switch (event) {

  case FL_FOCUS:
    switch (Fl::event_key()) {
    case FL_Right:
      position(0);
      break;
    case FL_Left:
      position(size());
      break;
    case FL_Down:
      up_down_position(0);
      break;
    case FL_Up:
      up_down_position(size());
      break;
    case FL_Tab:
      position(size(),0);
      break;
    }
    break;

  case FL_UNFOCUS:
    compose = 0;
    break;

  case FL_KEYBOARD:
    return handle_key();

  case FL_PUSH:
    compose = 0;
    if (Fl::event_button() == 2) {
      Fl::paste(*this);
      if (Fl::focus()==this) return 1; // remove line for Motif behavior
    }
    if (Fl::focus() != this) {
      Fl::focus(this);
      handle(FL_FOCUS); // cause minimal update
    }
    break;

  case FL_DRAG:
  case FL_RELEASE:
    if (Fl::event_button() == 2) return 0;
    break;
  }
  Fl_Boxtype b = box() ? box() : default_box();
  return Fl_Input_::handletext(event,
	x()+Fl::box_dx(b)+3, y()+Fl::box_dy(b),
	w()-Fl::box_dw(b)-6, h()-Fl::box_dh(b));
}

Fl_Input::Fl_Input(int x, int y, int w, int h, const char *l)
: Fl_Input_(x, y, w, h, l) {}

//
// End of "$Id: Fl_Input.cxx,v 1.3 1998/10/19 20:45:49 mike Exp $".
//
