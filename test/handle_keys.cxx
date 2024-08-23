//
// Keyboard event test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 2024 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Terminal.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/fl_ask.H>
#include <FL/names.h>
#include <stdio.h>
#include <stdlib.h> // free()

// Global variables to simplify the code

Fl_Check_Button *keydown  = NULL;
Fl_Check_Button *keyup    = NULL;
Fl_Check_Button *shortcut = NULL;
Fl_Check_Button *scaling  = NULL;

// Text in the headline and after clearing the terminal buffer. For alignment ...
//          1         2         3         4         5         6         7         8
// 1234567890123456789012345678901234567890123456789012345678901234567890123456789012345
static const char *headline_text =
  "[nnn] Event           Key     Name,  Flags: C A S M N L  Text  Unicode   UTF-8/hex";

static const int lkn = 14; // length of key name field

// Tooltip for headline and terminal widgets
static const char *tt =
        "Flags:\n"
        "C=Ctrl, A=Alt, S=Shift, M=Meta\n"
        "N=NumLock, L=CapsLock";

// This table is a duplicate of the table in test/keyboard.cxx.
// In the future this should be moved to the FLTK core so FLTK key
// numbers can be translated to strings (key names) in user programs.

struct keycode_table {
  int n;                // key code
  const char* text;     // key name
} key_table[] = {
  { FL_Escape,          "FL_Escape"},
  { FL_BackSpace,       "FL_BackSpace"},
  { FL_Tab,             "FL_Tab"},
  { FL_Iso_Key,         "FL_Iso_Key"},
  { FL_Enter,           "FL_Enter"},
  { FL_Print,           "FL_Print"},
  { FL_Scroll_Lock,     "FL_Scroll_Lock"},
  { FL_Pause,           "FL_Pause"},
  { FL_Insert,          "FL_Insert"},
  { FL_Home,            "FL_Home"},
  { FL_Page_Up,         "FL_Page_Up"},
  { FL_Delete,          "FL_Delete"},
  { FL_End,             "FL_End"},
  { FL_Page_Down,       "FL_Page_Down"},
  { FL_Left,            "FL_Left"},
  { FL_Up,              "FL_Up"},
  { FL_Right,           "FL_Right"},
  { FL_Down,            "FL_Down"},
  { FL_Shift_L,         "FL_Shift_L"},
  { FL_Shift_R,         "FL_Shift_R"},
  { FL_Control_L,       "FL_Control_L"},
  { FL_Control_R,       "FL_Control_R"},
  { FL_Caps_Lock,       "FL_Caps_Lock"},
  { FL_Alt_L,           "FL_Alt_L"},
  { FL_Alt_R,           "FL_Alt_R"},
  { FL_Meta_L,          "FL_Meta_L"},
  { FL_Meta_R,          "FL_Meta_R"},
  { FL_Menu,            "FL_Menu"},
  { FL_Help,            "FL_Help"},
  { FL_Num_Lock,        "FL_Num_Lock"},
  { FL_KP_Enter,        "FL_KP_Enter"},
  { FL_Alt_Gr,          "FL_Alt_Gr"}
};

// This function is very similar to the code in test/keyboard.cxx.
// In the future this should be moved to the FLTK core so FLTK key
// numbers can be translated to strings (key names) in user programs.

// Returns:
//  - function return is a pointer to the key name string
//  - parameter lg returns the length in characters (not bytes)
// The latter can be used to align strings.

// Todo: this function may not be complete yet and is
//       maybe not correct for all key values.

const char *get_keyname(int k, int &lg) {
  static char buffer[32];
  lg = 0;
  if (!k) {
    lg = 1;
    return "0";
  }
  else if (k < 32) {  // control character
    lg = sprintf(buffer, "^%c", (char)(k + 64));
  }
  else if (k < 128) { // ASCII
    lg = snprintf(buffer, sizeof(buffer), "'%c'", k);
  } else if (k >= 0xa0 && k <= 0xff) { // ISO-8859-1 (international keyboards)
    char key[8];
    int kl = fl_utf8encode((unsigned)k, key);
    key[kl] = '\0';
    lg = snprintf(buffer, sizeof(buffer), "'%s'", key);
  } else if (k > FL_F && k <= FL_F_Last) {
    lg = snprintf(buffer, sizeof(buffer), "FL_F+%d", k - FL_F);
  } else if (k >= FL_KP && k <= FL_KP_Last) {
    if (k == FL_KP_Enter)
      lg = snprintf(buffer, sizeof(buffer), "FL_KP_Enter");
    else
      lg = snprintf(buffer, sizeof(buffer), "FL_KP+'%c'", k-FL_KP);
  } else if (k >= FL_Button && k <= FL_Button+7) {
    lg = snprintf(buffer, sizeof(buffer), "FL_Button+%d", k-FL_Button);
  } else {
    lg = snprintf(buffer, sizeof(buffer), "0x%04x", k);
    for (int i = 0; i < int(sizeof(key_table)/sizeof(*key_table)); i++) {
      if (key_table[i].n == k) {
        lg = (int)strlen(key_table[i].text);
        return key_table[i].text;
      }
    }
  }
  return buffer;
}

class Terminal : public Fl_Terminal {
public:
  int handle(int ev) FL_OVERRIDE {
    switch(ev) {
    case FL_KEYBOARD:
    case FL_KEYUP:
    case FL_SHORTCUT:
      return 0;
    }
  return Fl_Terminal::handle(ev);
  }
  Terminal(int X, int Y, int W, int H)
  : Fl_Terminal(X, Y, W, H) {}
};

// Class to handle events
class app : public Fl_Double_Window {
protected:
  int handle(int) FL_OVERRIDE;
public:
  // storage for the last event
  int eventnum;
  const char *eventname;
  Terminal *tty;
  Fl_Box *headline;
  app(int X, int Y, int W, int H, const char *L = 0)
    : Fl_Double_Window(X, Y, W, H, L) {
    eventname = NULL;
    eventnum = 0;
    headline = new Fl_Box(2, 0, W - 4, 25);
    headline->color(FL_LIGHT2);
    headline->box(FL_FLAT_BOX);
    headline->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    headline->labelfont(FL_COURIER);
    headline->labelsize(12);
    headline->label(headline_text);
    headline->tooltip(tt);
    tty = new Terminal(0, 25, W, H - 100);
    tty->color(FL_WHITE);
    tty->textfgcolor(fl_darker(FL_BLUE));
    tty->selectionbgcolor(FL_BLUE);
    tty->selectionfgcolor(FL_WHITE);
    tty->textfont(FL_COURIER);
    tty->textsize(12);
    // tty->selection_color(FL_RED);
    tty->tooltip(tt);
  }
  int print_event(int ev);
};

// print_event() counts and prints the current event.
// Returns 1 if printed (used), 0 if suppressed.
// The event counter is incremented only if the event is printed
// and wraps at 1000.

int app::print_event(int ev) {
  switch(ev) {
    case FL_KEYBOARD:
      if (!keydown->value()) return 0;
      tty->textfgcolor(FL_BLACK);
      break;
    case FL_KEYUP:
      if (!keyup->value()) return 0;
      tty->textfgcolor(FL_BLUE);
      break;
    case FL_SHORTCUT:
      if (!shortcut->value()) return 0;
      tty->textfgcolor(0x00aa0000);   // dark green
      break;
    default:
      return 0;
  }
  eventnum++;
  eventnum %= 1000;
  eventname = fl_eventnames[ev];
  tty->printf("[%3d] %-12s", eventnum, eventname);
  return 1;
} // app::print_event()

// Event handling
int app::handle(int ev) {
  int res = Fl_Double_Window::handle(ev);
  // filter and output keyboard events only
  if (!print_event(ev))
    return res;

  const char *etxt = Fl::event_text();
  int ekey    = Fl::event_key();
  int elen    = Fl::event_length();
  char ctrl   = (Fl::event_state() & FL_CTRL)      ? 'C' : '.';
  char alt    = (Fl::event_state() & FL_ALT)       ? 'A' : '.';
  char shift  = (Fl::event_state() & FL_SHIFT)     ? 'S' : '.';
  char meta   = (Fl::event_state() & FL_META)      ? 'M' : '.';
  char numlk  = (Fl::event_state() & FL_NUM_LOCK)  ? 'N' : '.';
  char capslk = (Fl::event_state() & FL_CAPS_LOCK) ? 'L' : '.';

  int lg = 0;
  char ekns[12];                  // event key (hex) as string
  sprintf(ekns, "0x%04x", ekey);  // may be up to 10 chars
  tty->printf("%10s  ", ekns);    // event key number (hex)

  tty->printf("%s", get_keyname(ekey, lg));
  if (lg < lkn) {
    for (int i = 0; i < lkn - lg; i++) {
      tty->append_ascii(" ");
    }
  }

  tty->printf("%c %c %c %c %c %c  ", ctrl, alt, shift, meta, numlk, capslk);

  if (elen) {
    if (elen == 1 && etxt[0] < 32) {              // control character (0-31)
      tty->printf("'^%c' ", (char)(etxt[0] + 64));
    } else {
      tty->printf("'%s'  ", etxt);
    }
    unsigned int ucs = fl_utf8decode(etxt, etxt + elen, NULL);
    tty->printf(" U+%04x   ", ucs);
    for (int i = 0; i < Fl::event_length(); i++) {
      tty->printf(" %02x", Fl::event_text()[i]&0xff);
    }
  } else {
    tty->printf("'' ");
  }
  tty->textfgcolor(FL_BLACK);
  tty->printf("\n");
  return res;
} // app::handle()

// Quit button callback: closes the window
void quit_cb(Fl_Widget *w, void *) {
  w->window()->hide();
}

// Clear button callback: clears the terminal widget
void clear_cb(Fl_Button *b, void *) {
  Terminal *tty = ((app *)b->window())->tty;
  tty->clear_screen_home();
  tty->clear_history();
  tty->printf("%s\n", headline_text); // helpful if copied to the clipboard
  tty->redraw();
  tty->take_focus();
}

// Copy button callback: copies the selected text to the clipboard
void copy_cb(Fl_Widget *b, void *) {
  Terminal *tty = ((app *)b->window())->tty;
  const char *what = "Full";
  const char *text;
  int tlen = tty->selection_text_len();
  if (tlen > 0) {
    text = tty->selection_text();
    what = "Selected";
  } else {
    text = tty->text();
  }
  tlen = (int)strlen(text);
  Fl::copy(text, tlen, 1, Fl::clipboard_plain_text);
  tty->printf("[%s text copied to clipboard, length = %d]\n", what, tlen);
  free((void *)text);
  tty->take_focus();
}

// Callback for all (light) buttons
void toggle_cb(Fl_Widget *w, void *) {
  Terminal *tty = ((app *)w->window())->tty;
  tty->take_focus();
}

// Toggle recognition of GUI scaling shortcuts
void toggle_scaling(Fl_Widget *w, void *v) {
  int toggle = ((Fl_Button*)w)->value() ? 1 : 0;
  Fl::keyboard_screen_scaling(toggle);
  if (toggle) {
    Terminal *tty = ((app *)w->window())->tty;
    int simple_zoom = Fl::option(Fl::OPTION_SIMPLE_ZOOM_SHORTCUT);
    tty->printf("GUI-Scaling = %s, OPTION_SIMPLE_ZOOM_SHORTCUT = %s\n",
                toggle ? "ON" : "OFF", simple_zoom ? "ON" : "OFF");
  }
  toggle_cb(w, v); // give focus to 'app'
}

// Window close callback (Esc does not close the window)
void close_cb(Fl_Widget *win, void *) {
  if (Fl::event() == FL_SHORTCUT)
    return;
  win->hide();
}

// Main program

int main(int argc, char **argv) {

#ifdef _WIN32
  // Set an appropriate font for Wine on Linux (test only).
  // This is very likely not necessary on a real Windows system.
  // Set environment variable FLTK_USING_WINE to a non-empty string
  // to enable this feature, e.g. (in bash) `export FLTK_USING_WINE=1`.
  const char *using_wine = fl_getenv("FLTK_USING_WINE");
  printf("FLTK_USING_WINE = %s\n", using_wine ? using_wine : "");
  if (using_wine && *using_wine) {
    // Fl::set_font(FL_COURIER, " DejaVu Sans Mono");     // DejaVu Mono
    Fl::set_font(FL_COURIER, " Liberation Mono");         // Liberation Mono
  }
#endif

  const int WW = 700, WH = 400;
  app *win = new app(0, 0, WW, WH);
  win->tty->box(FL_DOWN_BOX);
  win->tty->show_unknown(true);
  win->tty->textfgcolor(FL_BLACK);
  win->tty->printf("Please press any key ...\n");

  Fl_Grid *grid = new Fl_Grid(0, WH - 75, WW, 75);
  grid->layout(2, 5, 5, 5);

  keydown = new Fl_Check_Button(0, 0, 80, 30, "Keydown");
  grid->widget(keydown, 0, 0);
  keydown->value(1);
  keydown->callback(toggle_cb);
  keydown->tooltip("Show FL_KEYDOWN aka FL_KEYBOARD events");

  keyup = new Fl_Check_Button(0, 0, 80, 30, "Keyup");
  grid->widget(keyup, 0, 1);
  keyup->value(0);
  keyup->callback(toggle_cb);
  keyup->tooltip("Show FL_KEYUP events");

  shortcut = new Fl_Check_Button(0, 0, 80, 30, "Shortcut");
  grid->widget(shortcut, 0, 2);
  shortcut->value(0);
  shortcut->callback(toggle_cb);
  shortcut->tooltip("Show FL_SHORTCUT events");

  scaling = new Fl_Check_Button(0, 0, 80, 30, "GUI scaling");
  grid->widget(scaling, 0, 3);
  scaling->value(0);
  scaling->callback(toggle_scaling);
  scaling->tooltip("Use GUI scaling shortcuts");
  toggle_scaling(scaling, 0);

  Fl_Button *clear = new Fl_Button(0, 0, 80, 30, "Clear");
  grid->widget(clear, 1, 0);
  clear->callback((Fl_Callback *)clear_cb);
  clear->tooltip("Clear the display");

  Fl_Button *copy = new Fl_Button(0, 0, 80, 30, "Copy");
  grid->widget(copy, 1, 1);
  copy->callback(copy_cb);
  copy->tooltip("Copy terminal contents to clipboard");

  Fl_Button *quit = new Fl_Button(WW - 70, WH - 50, 80, 30, "Quit");
  grid->widget(quit, 1, 4);
  quit->box(FL_THIN_UP_BOX);
  quit->callback((Fl_Callback *)quit_cb);
  quit->tooltip("Exit the program");

  grid->end();
  win->end();
  win->callback(close_cb);
  win->resizable(win->tty);
  win->size_range(660, 300);
  win->show(argc, argv);
  return Fl::run();
}
