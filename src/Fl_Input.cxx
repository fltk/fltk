//
// Input widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

// This is the "user interface", it decodes user actions into what to
// do to the text.  See also Fl_Input_.cxx, where the text is actually
// manipulated (and some ui, in particular the mouse, is done...).
// In theory you can replace this code with another subclass to change
// the keybindings.

#include <stdio.h>
#include <stdlib.h>
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/Fl_Window.H>
#include "Fl_System_Driver.H"
#include "Fl_Screen_Driver.H"
#include <FL/Fl_Input.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include "flstring.h"           // this #includes "<config.h>" !

#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Menu_Item.H>

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

/** [this text may be customized at run-time] */
const char *Fl_Input::cut_menu_text = "Cut";
/** [this text may be customized at run-time] */
const char *Fl_Input::copy_menu_text = "Copy";
/** [this text may be customized at run-time] */
const char *Fl_Input::paste_menu_text = "Paste";

static Fl_Menu_Item rmb_menu[] = {
  { NULL, 0, NULL, (void*)1 },
  { NULL, 0, NULL, (void*)2 },
  { NULL, 0, NULL, (void*)3 },
  { NULL }
};

void Fl_Input::draw() {
  if (input_type() == FL_HIDDEN_INPUT) return;
  Fl_Boxtype b = box();
  if (damage() & FL_DAMAGE_ALL) draw_box(b, color());
  Fl_Input_::drawtext(x()+Fl::box_dx(b), y()+Fl::box_dy(b),
                      w()-Fl::box_dw(b), h()-Fl::box_dh(b));
}

// kludge so shift causes selection to extend:
int Fl_Input::shift_position(int p) {
  return insert_position(p, Fl::event_state(FL_SHIFT) ? mark() : p);
}

int Fl_Input::shift_up_down_position(int p) {
  return up_down_position(p, Fl::event_state(FL_SHIFT));
}

// Old text from FLTK 1.1 for reference:
// If you define NORMAL_INPUT_MOVE as zero you will get the peculiar fltk
// behavior where moving off the end of an input field will move the
// cursor into the next field:
// define it as 1 to prevent cursor movement from going to next field:
//
// Note: this has been replaced by Fl::option(Fl::OPTION_ARROW_FOCUS)
// in FLTK 1.3.  This option has "inverted" values:
//   1 = Arrow keys move focus (previously 0)
//   0 = Arrow keys don't move focus (previously 1)
// Hence we define ...
//
#define NORMAL_INPUT_MOVE (Fl::option(Fl::OPTION_ARROW_FOCUS) ? 0 : 1)

#define ctrl(x) ((x)^0x40)

// List of characters that are legal in a floating point input field.
// This text string is created at run-time to take the current locale
// into account (for example, continental Europe uses a comma instead
// of a decimal point). For back compatibility reasons, we always
// allow the decimal point.
#ifdef HAVE_LOCALECONV
static const char *standard_fp_chars = ".eE+-";
static const char *legal_fp_chars = 0L;
#else
static const char *legal_fp_chars = ".eE+-";
#endif

// Move cursor up specified #lines
//    If OPTION_ARROW_FOCUS is disabled, return 1 to prevent focus navigation.
//
int Fl_Input::kf_lines_up(int repeat_num) {
  int i = insert_position();
  if (!line_start(i)) {
    //UNNEEDED if (input_type()==FL_MULTILINE_INPUT && !Fl::option(Fl::OPTION_ARROW_FOCUS)) return 1;
    return NORMAL_INPUT_MOVE;
  }
  while(repeat_num--) {
    i = line_start(i);
    if (!i) break;
    i--;
  }
  shift_up_down_position(line_start(i));
  return 1;
}

// Move cursor down specified #lines
//    If OPTION_ARROW_FOCUS is disabled, return 1 to prevent focus navigation.
//
int Fl_Input::kf_lines_down(int repeat_num) {
  int i = insert_position();
  if (line_end(i) >= size()) {
    //UNNEEDED if (input_type()==FL_MULTILINE_INPUT && !Fl::option(Fl::OPTION_ARROW_FOCUS)) return 1;
    return NORMAL_INPUT_MOVE;
  }
  while (repeat_num--) {
    i = line_end(i);
    if (i >= size()) break;
    i++;
  }
  shift_up_down_position(i);
  return 1;
}

// Move up a page
int Fl_Input::kf_page_up() {
  return kf_lines_up(linesPerPage());
}

// Move down a page
int Fl_Input::kf_page_down() {
  return kf_lines_down(linesPerPage());
}

// Toggle insert mode
int Fl_Input::kf_insert_toggle() {
  if (readonly()) { fl_beep(); return 1; }
  return 1;                             // \todo: needs insert mode
}

// Delete word right
int Fl_Input::kf_delete_word_right() {
  if (readonly()) { fl_beep(); return 1; }
  if (mark() != insert_position()) return cut();
  cut(insert_position(), word_end(insert_position()));
  return 1;
}

// Delete word left
int Fl_Input::kf_delete_word_left() {
  if (readonly()) { fl_beep(); return 1; }
  if (mark() != insert_position()) return cut();
  cut(word_start(insert_position()), insert_position());
  return 1;
}

// Delete to start of line
int Fl_Input::kf_delete_sol() {
  if (readonly()) { fl_beep(); return 1; }
  if (mark() != insert_position()) return cut();
  cut(line_start(insert_position()), insert_position());
  return 1;
}

// Delete to end of line
int Fl_Input::kf_delete_eol() {
  if (readonly()) { fl_beep(); return 1; }
  if (mark() != insert_position()) return cut();
  cut(insert_position(), line_end(insert_position()));
  return 1;
}

int Fl_Input::kf_delete_char_right() {
  if (readonly()) { fl_beep(); return 1; }
  if (mark() != insert_position()) cut();
  else cut(1);
  return 1;
}

int Fl_Input::kf_delete_char_left() {
  if (readonly()) { fl_beep(); return 1; }
  if (mark() != insert_position()) cut();
  else cut(-1);
  return 1;
}

// Move cursor to start of line
int Fl_Input::kf_move_sol() {
  return shift_position(line_start(insert_position())) + NORMAL_INPUT_MOVE;
}

// Move cursor to end of line
int Fl_Input::kf_move_eol() {
  return shift_position(line_end(insert_position())) + NORMAL_INPUT_MOVE;
}

// Clear to end of line
int Fl_Input::kf_clear_eol() {
  if (readonly()) { fl_beep(); return 1; }
  if (insert_position()>=size()) return 0;
  int i = line_end(insert_position());
  if (i == insert_position() && i < size()) i++;
  cut(insert_position(), i);
  return copy_cuts();
}

// Move cursor one character to the left
//    If OPTION_ARROW_FOCUS is disabled, return 1 to prevent focus navigation.
//
int Fl_Input::kf_move_char_left() {
  int i = shift_position(insert_position()-1) + NORMAL_INPUT_MOVE;
  return Fl::option(Fl::OPTION_ARROW_FOCUS) ? i : 1;
}

// Move cursor one character to the right
//    If OPTION_ARROW_FOCUS is disabled, return 1 to prevent focus navigation.
//
int Fl_Input::kf_move_char_right() {
  int i = shift_position(insert_position()+1) + NORMAL_INPUT_MOVE;
  return Fl::option(Fl::OPTION_ARROW_FOCUS) ? i : 1;
}

// Move cursor word-left
int Fl_Input::kf_move_word_left() {
  shift_position(word_start(insert_position()));
  return 1;
}

// Move cursor word-right
int Fl_Input::kf_move_word_right() {
  shift_position(word_end(insert_position()));
  return 1;
}

// Move cursor up one line and to the start of line (paragraph up)
int Fl_Input::kf_move_up_and_sol() {
  if (line_start(insert_position())==insert_position() && insert_position()>0)
    return shift_position(line_start(insert_position()-1)) + NORMAL_INPUT_MOVE;
  else
    return shift_position(line_start(insert_position())) + NORMAL_INPUT_MOVE;
}

// Move cursor down one line and to the end of line (paragraph down)
int Fl_Input::kf_move_down_and_eol() {
  if (line_end(insert_position())==insert_position() && insert_position()<size())
    return shift_position(line_end(insert_position()+1)) + NORMAL_INPUT_MOVE;
  else
    return shift_position(line_end(insert_position())) + NORMAL_INPUT_MOVE;
}

// Move to top of document
int Fl_Input::kf_top() {
  shift_position(0);
  return 1;
}

// Move to bottom of document
int Fl_Input::kf_bottom() {
  shift_position(size());
  return 1;
}

// Select all text in the widget
int Fl_Input::kf_select_all() {
  insert_position(0,size());
  return 1;
}

// Undo.
int Fl_Input::kf_undo() {
  if (readonly()) { fl_beep(); return 1; }
  return undo();
}

// Redo.
int Fl_Input::kf_redo() {
  if (readonly()) { fl_beep(); return 1; }
  return redo();
}

// Do a copy operation
int Fl_Input::kf_copy() {
  return copy(1);
}

// Do a paste operation
int Fl_Input::kf_paste() {
  if (readonly()) { fl_beep(); return 1; }
  Fl::paste(*this, 1);
  return 1;
}

// Do a cut with copy
int Fl_Input::kf_copy_cut() {
  if (readonly()) { fl_beep(); return 1; }
  copy(1);
  return cut();
}

/** Handles a keystroke.

  This \p protected method handles a keystroke in an Fl_Input or derived
  class. It handles compose key sequences and can also be used e.g. in
  Fl_Multiline_Input, Fl_Float_Input and several more derived classes.

  The method first checks in Fl::compose if the keystroke is a text entry or
  a control key. If it is text, the method inserts the composed characters into
  the input field, taking into account the input type (e.g., numeric fields).

  If the keystroke is a control key as determined by Fl::compose, the method
  handles key combinations for Insert, Enter, and Tab depending on the
  widget's input_type().

  The method then checks for Ctrl key combinations, such as Ctrl-A, Ctrl-C,
  Ctrl-V, Ctrl-X, and Ctrl-Z, which are commonly used for select all, copy,
  paste, cut, and undo operations.

  Finally, the method checks for ASCII control characters, such as Ctrl-H,
  Ctrl-I, Ctrl-J, Ctrl-L, and Ctrl-M, which can be used to insert literal
  control characters into the input field.

  If none of the above cases match, the method returns 0, indicating that the
  keystroke was not handled.

  \returns  1 if the keystroke is handled by us, 0 if not.
*/
int Fl_Input::handle_key() {

  // This is unicode safe: only character codes < 128 are queried
  char ascii = Fl::event_text()[0];

  int del;
  if (Fl::compose(del)) {

    // Insert characters into numeric fields after checking for legality:
    if (input_type() == FL_FLOAT_INPUT || input_type() == FL_INT_INPUT) {
      Fl::compose_reset(); // ignore any composed characters...

      // initialize the list of legal characters inside a floating point number
#if defined(HAVE_LOCALECONV)
      if (!legal_fp_chars) {
        size_t len = strlen(standard_fp_chars);
        struct lconv *lc = localeconv();
        if (lc) {
          if (lc->decimal_point) len += strlen(lc->decimal_point);
          if (lc->mon_decimal_point) len += strlen(lc->mon_decimal_point);
          if (lc->positive_sign) len += strlen(lc->positive_sign);
          if (lc->negative_sign) len += strlen(lc->negative_sign);
        }
        // the following line is not a true memory leak because the array is only
        // allocated once if required, and automatically freed when the program quits
        char *chars = (char*)malloc(len+1);
        legal_fp_chars = chars;
        strcpy(chars, standard_fp_chars);
        if (lc) {
          if (lc->decimal_point) strcat(chars, lc->decimal_point);
          if (lc->mon_decimal_point) strcat(chars, lc->mon_decimal_point);
          if (lc->positive_sign) strcat(chars, lc->positive_sign);
          if (lc->negative_sign) strcat(chars, lc->negative_sign);
        }
      }
#endif // HAVE_LOCALECONV

      // find the insert position
      int ip = insert_position()<mark() ? insert_position() : mark();
      // This is complex to allow "0xff12" hex to be typed:
      if (   (!ip && (ascii == '+' || ascii == '-'))
          || (ascii >= '0' && ascii <= '9')
          || (ip==1 && index(0)=='0' && (ascii=='x' || ascii == 'X'))
          || (ip>1 && index(0)=='0' && (index(1)=='x'||index(1)=='X')
              && ((ascii>='A'&& ascii<='F') || (ascii>='a'&& ascii<='f')))
          || (input_type()==FL_FLOAT_INPUT && ascii && strchr(legal_fp_chars, ascii)))
      {
        if (readonly()) fl_beep();
        else replace(insert_position(), mark(), &ascii, 1);
      }
      return 1;
    }

    if (del || Fl::event_length()) {
      if (readonly()) fl_beep();
      else replace(insert_position(), del ? insert_position()-del : mark(),
                   Fl::event_text(), Fl::event_length());
    }
    if (Fl::screen_driver()->has_marked_text() && Fl::compose_state) {
      this->mark( this->insert_position() - Fl::compose_state );
    }
    return 1;
  }

  int mods = Fl::event_state() & (FL_META|FL_CTRL|FL_ALT);
  unsigned int shift = Fl::event_state() & FL_SHIFT;
  unsigned int multiline = (input_type() == FL_MULTILINE_INPUT) ? 1 : 0;
  //
  // The following lists apps that support these keypresses.
  // Prefixes: '!' indicates NOT supported, '?' indicates un-verified.
  //
  //    HIG=Human Interface Guide,
  //    TE=TextEdit.app, SA=Safari.app, WOX=MS Word/OSX -- OSX 10.4.x
  //    NP=Notepad, WP=WordPad, WOW=MS Word/Windows     -- WinXP
  //    GE=gedit, KE=kedit                              -- Ubuntu8.04
  //    OF=old FLTK behavior (<=1.1.10)
  //
  // Example: (NP,WP,!WO) means supported in notepad + wordpad, but NOT word.
  //

  // handle keypresses that can have a platform-dependent processing
  int retval = Fl::screen_driver()->input_widget_handle_key(Fl::event_key(), mods, shift, this);
  if (retval >= 0) return retval;

  switch (Fl::event_key()) {

    case FL_Insert:
      // Note: Mac has no "Insert" key; it's the "Help" key.
      //       This keypress is apparently not possible on macs.
      //
      if (mods==0 && shift) return kf_paste();                  // Shift-Insert   (WP,NP,WOW,GE,KE,OF)
      if (mods==0)          return kf_insert_toggle();          // Insert         (Standard)
      if (mods==FL_CTRL)    return kf_copy();                   // Ctrl-Insert    (WP,NP,WOW,GE,KE,OF)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_Enter:
    case FL_KP_Enter:
      if (when() & FL_WHEN_ENTER_KEY) {
        insert_position(size(), 0);
        maybe_do_callback(FL_REASON_ENTER_KEY);
        return 1;
      } else if (multiline && !readonly()) {
        return replace(insert_position(), mark(), "\n", 1);
      } return 0;                       // reserved for shortcuts

    case FL_Tab:
      // Handle special case for multiline input with 'old tab behavior';
      // tab handled as a normal insertable character.
      //
      if (mods==0 && !shift             // Tab?
           && !tab_nav()                // old tab behavior enabled?
           && multiline) {              // multiline input?
        break;                          // insert tab character
      }
      if (mods==0) return 0;                                    // Tab, Shift-Tab? nav focus      (Standard/OSX-HIG)
      return 0;                                                 // ignore other combos, pass to parent

    case 'a':
      if (mods==FL_COMMAND) return kf_select_all();             // Ctrl-A, Mac:Meta-A             (Standard/OSX-HIG)
      break;                                                    // handle other combos elsewhere
    case 'c':
      if (mods==FL_COMMAND) return kf_copy();                   // Ctrl-C, Mac:Meta-C             (Standard/OSX-HIG)
      break;                                                    // handle other combos elsewhere
    case 'v':
      if (mods==FL_COMMAND) return kf_paste();                  // Ctrl-V, Mac:Meta-V             (Standard/OSX-HIG)
      break;                                                    // handle other combos elsewhere
    case 'x':
      if (mods==FL_COMMAND) return kf_copy_cut();               // Ctrl-X, Mac:Meta-X             (Standard/OSX-HIG)
      break;
    case 'z':
      if (mods==FL_COMMAND && !shift) {                         // Ctrl-Z, Mac:Meta-Z             (Standard/OSX-HIG)
        if (!kf_undo()) fl_beep();
        return 1;
      }
      if (mods==FL_COMMAND && shift) {                          // Shift-Ctrl-Z, Mac:Shift-Meta-Z (Standard/OSX-HIG)
        if (!kf_redo()) fl_beep();
        return 1;
      }
      break;                                                    // handle other combos elsewhere
  }

  switch (ascii) {
    case ctrl('H'):
      return kf_delete_char_left();                             // Ctrl-H                           (!WP,!NP,!WOW,!WOX,TE,SA,GE,KE,OF)
    case ctrl('I'):                                             // Ctrl-I (literal Tab)             (!WP,NP,!WOW,!GE,KE,OF)
    case ctrl('J'):                                             // Ctrl-J (literal Line Feed/Enter) (Standard)
    case ctrl('L'):                                             // Ctrl-L (literal Form Feed)       (Standard)
    case ctrl('M'):                                             // Ctrl-M (literal Cr)              (Standard)
      if (readonly()) { fl_beep(); return 1; }
      // insert a few selected control characters literally:
      if (input_type() != FL_FLOAT_INPUT && input_type() != FL_INT_INPUT)
        return replace(insert_position(), mark(), &ascii, 1);
      break;
  }

  return 0;             // ignored
}

/** \internal
 Simple function that determines if a character is a whitespace.
 \todo This function is not UTF-8-aware.
 */
static int fltk__isspace(char c) {
  return (c&128 || isspace(c));
}

/** Handle right mouse button down events.
 \return 1
 */
int Fl_Input::handle_rmb() {
  if (Fl::event_button() == FL_RIGHT_MOUSE) {
    // on right mouse button, pop up a Cut/Copy/Paste menu
    int newpos, oldpos = insert_position(), oldmark = mark();
    Fl_Boxtype b = box();
    Fl_Input_::handle_mouse(x()+Fl::box_dx(b), y()+Fl::box_dy(b),
                            w()-Fl::box_dw(b), h()-Fl::box_dh(b), 0);
    newpos = insert_position();
    if (   ((oldpos < newpos) && (oldmark > newpos))
        || ((oldmark < newpos) && (oldpos > newpos))
        || (type() == FL_SECRET_INPUT)) {
      // if the user clicked inside an existing selection, keep
      // the selection
      insert_position(oldpos, oldmark);
    } else {
      if ((index(newpos) == 0) || (index(newpos) == '\n')) {
        // if clicked to the right of the line or text end, clear the
        // selection and set the cursor at the end of the line
        insert_position(newpos, newpos);
      } else if (fltk__isspace(index(newpos))) {
        // if clicked into a whitespace, select the entire whitespace
        oldpos = newpos;
        while (oldpos > 0 && fltk__isspace(index(oldpos-1))) oldpos--;
        oldmark = newpos+1;
        while (oldmark < size() && fltk__isspace(index(oldmark))) oldmark++;
        insert_position(oldpos, oldmark);
      } else {
        // if clicked on a word, select the entire word
        insert_position(word_start(newpos), word_end(newpos));
      }
    }
    // keep the menu labels current
    rmb_menu[0].label(Fl_Input::cut_menu_text);
    rmb_menu[1].label(Fl_Input::copy_menu_text);
    rmb_menu[2].label(Fl_Input::paste_menu_text);
    // give only the menu options that make sense
    if (readonly()) {
      rmb_menu[0].deactivate(); // cut
      rmb_menu[2].deactivate(); // paste
    } else {
      rmb_menu[0].activate(); // cut
      rmb_menu[2].activate(); // paste
    }
    // pop up the menu
    fl_cursor(FL_CURSOR_DEFAULT);
    const Fl_Menu_Item *mi = rmb_menu->popup(Fl::event_x(), Fl::event_y());
    if (mi) switch (mi->argument()) {
      case 1:
        if (type() != FL_SECRET_INPUT) kf_copy_cut();
        break;
      case 2:
        if (type() != FL_SECRET_INPUT) kf_copy();
        break;
      case 3:
        kf_paste();
        break;
    }
  }

  return 1;
}

int Fl_Input::handle(int event) {
  static int dnd_save_position, dnd_save_mark, drag_start = -1, newpos;
  static Fl_Widget *dnd_save_focus = NULL;
  switch (event) {
    case FL_UNFOCUS:
      if (Fl::screen_driver()->has_marked_text() && Fl::compose_state) {
        this->mark( this->insert_position() );
        fl_reset_spot();
      }
      break;
    case FL_FOCUS:
      switch (Fl::event_key()) {
        case FL_Right:
          insert_position(0);
          break;
        case FL_Left:
          insert_position(size());
          break;
        case FL_Down:
          up_down_position(0);
          break;
        case FL_Up:
          up_down_position(line_start(size()));
          break;
        case FL_Tab:
          insert_position(size(),0);
          break;
        default:
          insert_position(insert_position(),mark());// turns off the saved up/down arrow position
          break;
      }
      break;

    case FL_KEYBOARD:
      // Handle special case for multiline input with 'old tab behavior'
      // where tab is entered as a character: make sure user attempt to 'tab over'
      // widget doesn't destroy the field, replacing it with a tab character.
      //
      if (Fl::event_key() == FL_Tab                     // Tab key?
          && !Fl::event_state(FL_SHIFT)                 // no shift?
          && !tab_nav()                                 // with tab navigation disabled?
          && input_type() == FL_MULTILINE_INPUT         // with a multiline input?
          && size() > 0                                 // non-empty field?
          && ((mark()==0 && insert_position()==size()) || (insert_position()==0 && mark()==size()))) {// while entire field selected?
        // Set cursor to the end of the selection...
        if (mark() > insert_position())
          insert_position(mark());
        else
          insert_position(insert_position());
        return (1);
      } else {
        if (active_r() && window() && this == Fl::belowmouse())
          window()->cursor(FL_CURSOR_NONE);
        return handle_key();
      }
      //NOTREACHED

    case FL_PUSH:
      if (Fl::dnd_text_ops() && (Fl::event_button() != FL_RIGHT_MOUSE)) {
        int oldpos = insert_position(), oldmark = mark();
        Fl_Boxtype b = box();
        Fl_Input_::handle_mouse(x()+Fl::box_dx(b), y()+Fl::box_dy(b),
                                w()-Fl::box_dw(b), h()-Fl::box_dh(b), 0);
        newpos = insert_position();
        insert_position( oldpos, oldmark );
        if (Fl::focus()==this && !Fl::event_state(FL_SHIFT) && input_type()!=FL_SECRET_INPUT &&
            ( (newpos >= mark() && newpos < insert_position()) ||
             (newpos >= insert_position() && newpos < mark()) ) ) {
          // user clicked in the selection, may be trying to drag
          drag_start = newpos;
          return 1;
        }
        drag_start = -1;
      }

      if (Fl::focus() != this) {
        Fl::focus(this);
        handle(FL_FOCUS);
      }

      if (Fl::event_button() == FL_RIGHT_MOUSE) {
        return handle_rmb();
      }

      break;

    case FL_DRAG:
      if (Fl::dnd_text_ops()) {
        if (drag_start >= 0) {
          if (Fl::event_is_click()) return 1; // debounce the mouse
                                              // save the position because sometimes we don't get DND_ENTER:
          dnd_save_position = insert_position();
          dnd_save_mark = mark();
          dnd_save_focus = this;
          // drag the data:
          copy(0);
          Fl::screen_driver()->dnd(1);
          return 1;
        }
      }
      break;

    case FL_RELEASE:
      if (Fl::event_button() == FL_MIDDLE_MOUSE) {
        Fl::event_is_click(0); // stop double click from picking a word
        Fl::paste(*this, 0);
      } else if (Fl::event_button() == FL_RIGHT_MOUSE) {
        return 1;
      } else if (!Fl::event_is_click()) {
        // copy drag-selected text to the clipboard.
        copy(0);
      } else if (Fl::event_is_click() && drag_start >= 0) {
        // user clicked in the field and wants to reset the cursor position...
        insert_position(drag_start, drag_start);
        drag_start = -1;
      } else if (Fl::event_clicks()) {
        // user double or triple clicked to select word or whole text
        copy(0);
      }

      // For output widgets, do the callback so the app knows the user
      // did something with the mouse...
      if (readonly()) do_callback(FL_REASON_RELEASED);

      return 1;

    case FL_DND_ENTER:
      Fl::belowmouse(this); // send the leave events first
      if (dnd_save_focus != this) {
        dnd_save_position = insert_position();
        dnd_save_mark = mark();
        dnd_save_focus = Fl::focus();
        Fl::focus(this);
        handle(FL_FOCUS);
      }
      // fall through:
    case FL_DND_DRAG:
      //int p = mouse_position(X, Y, W, H);
#ifdef DND_OUT_XXXX
      if (Fl::focus()==this && (p>=dnd_save_position && p<=dnd_save_mark ||
                                p>=dnd_save_mark && p<=dnd_save_position)) {
        position(dnd_save_position, dnd_save_mark);
        return 0;
      }
#endif
      {
        Fl_Boxtype b = box();
        Fl_Input_::handle_mouse(x()+Fl::box_dx(b), y()+Fl::box_dy(b),
                                w()-Fl::box_dw(b), h()-Fl::box_dh(b), 0);
      }
      return 1;

    case FL_DND_LEAVE:
      insert_position(dnd_save_position, dnd_save_mark);
#ifdef DND_OUT_XXXX
      if (!focused())
#endif
        if (dnd_save_focus && dnd_save_focus != this) {
          Fl::focus(dnd_save_focus);
          handle(FL_UNFOCUS);
        }
      Fl::first_window()->cursor(FL_CURSOR_MOVE);
      dnd_save_focus = NULL;
      return 1;

    case FL_DND_RELEASE:
      if (dnd_save_focus == this) { // if the dragged text comes from the same widget
        if (!readonly()) {
          // remove the selected text
          int old_position = insert_position();
          if (dnd_save_mark > dnd_save_position) {
            int tmp = dnd_save_mark;
            dnd_save_mark = dnd_save_position;
            dnd_save_position = tmp;
          }
          replace(dnd_save_mark, dnd_save_position, NULL, 0);
          if (old_position > dnd_save_position)
            insert_position(old_position - (dnd_save_position - dnd_save_mark));
          else
            insert_position(old_position);
        } // !readonly()
      } // from the same widget
      else if (dnd_save_focus) {
        dnd_save_focus->handle(FL_UNFOCUS);
      }
      dnd_save_focus = NULL;
      take_focus();
      return 1;

      /* TODO: this will scroll the area, but stop if the cursor would become invisible.
       That clipping happens in drawtext(). Do we change the clipping or should
       we move the cursor (ouch)?
       case FL_MOUSEWHEEL:
       if (Fl::e_dy > 0) {
       yscroll( yscroll() - Fl::e_dy*15 );
       } else if (Fl::e_dy < 0) {
       yscroll( yscroll() - Fl::e_dy*15 );
       }
       return 1;
       */
  }
  Fl_Boxtype b = box();
  return Fl_Input_::handletext(event,
                               x()+Fl::box_dx(b), y()+Fl::box_dy(b),
                               w()-Fl::box_dw(b), h()-Fl::box_dh(b));
}

/**
 Creates a new Fl_Input widget using the given position, size,
 and label string. The default boxtype is FL_DOWN_BOX.
 */
Fl_Input::Fl_Input(int X, int Y, int W, int H, const char *l)
: Fl_Input_(X, Y, W, H, l) {
}


Fl_Float_Input::Fl_Float_Input(int X,int Y,int W,int H,const char *l)
: Fl_Input(X,Y,W,H,l)
{
  type(FL_FLOAT_INPUT);
  clear_flag(MAC_USE_ACCENTS_MENU);
}


Fl_Int_Input::Fl_Int_Input(int X,int Y,int W,int H,const char *l)
: Fl_Input(X,Y,W,H,l) {
  type(FL_INT_INPUT);
  clear_flag(MAC_USE_ACCENTS_MENU);
}


Fl_Multiline_Input::Fl_Multiline_Input(int X,int Y,int W,int H,const char *l)
: Fl_Input(X,Y,W,H,l) {
  type(FL_MULTILINE_INPUT);
}


Fl_Output::Fl_Output(int X,int Y,int W,int H, const char *l)
: Fl_Input(X, Y, W, H, l) {
  type(FL_NORMAL_OUTPUT);
  clear_flag(NEEDS_KEYBOARD);
}


Fl_Multiline_Output::Fl_Multiline_Output(int X,int Y,int W,int H,const char *l)
: Fl_Output(X,Y,W,H,l) {
  type(FL_MULTILINE_OUTPUT);
  clear_flag(NEEDS_KEYBOARD);
}


Fl_Secret_Input::Fl_Secret_Input(int X,int Y,int W,int H,const char *l)
: Fl_Input(X,Y,W,H,l) {
  type(FL_SECRET_INPUT);
  clear_flag(MAC_USE_ACCENTS_MENU);
}

int Fl_Secret_Input::handle(int event) {
  int retval = Fl_Input::handle(event);
  if (event == FL_KEYBOARD && Fl::screen_driver()->has_marked_text() && Fl::compose_state) {
    this->mark( this->insert_position() ); // don't underline marked text
  }
  return retval;
}
