//
// Fluid message dialog implementation for the Fast Light Tool Kit (FLTK).
//
// Copyright 2026 by Bill Spitzak and others.
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

#include "message.h"
#include "Fluid.h"

#include <FL/fl_ask.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>

#include "../src/flstring.h"

#undef min
#undef max
#include <limits>
#include <algorithm>
#include <cctype>
#include <cstdio>

#if defined(_WIN32)
#  include <conio.h>
#else
#  include <termios.h>
#  include <unistd.h>
#endif

namespace {

/**
 Read a single character from the console.

 Blocks until a key is pressed. The key is not echoed, and unlike
 `std::cin >> ch` or `fgetc(stdin)`, no Enter key is required.
 */
int read_console_key() {
#if defined(_WIN32)
  return _getch();
#else
  termios old_attr, raw_attr;
  tcgetattr(STDIN_FILENO, &old_attr);
  raw_attr = old_attr;
  raw_attr.c_lflag &= ~(ICANON | ECHO);
  raw_attr.c_cc[VMIN] = 1;
  raw_attr.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &raw_attr);
  int key = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &old_attr);
  return key;
#endif
}

} // namespace


void fluid_message(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  fluid_message(fmt, ap);
  va_end(ap);
}

void fluid_message(const char *fmt, va_list ap) {
  char buffer[8096];
  vsnprintf(buffer, sizeof(buffer), fmt, ap);
  fluid::message("Fluid", buffer);
}

void fluid_alert(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  fluid_alert(fmt, ap);
  va_end(ap);
}

void fluid_alert(const char *fmt, va_list ap) {
  char buffer[8096];
  vsnprintf(buffer, sizeof(buffer), fmt, ap);
  fluid::alert("Fluid ALERT", buffer);
}

int fluid_choice(const char *fmt, const char *b0, const char *b1, const char *b2, ...) {
  va_list ap;
  va_start(ap, b2);
  int r = fluid_choice(fmt, b0, b1, b2, ap);
  va_end(ap);
  return r;
}

int fluid_choice(const char *fmt, const char *b0, const char *b1, const char *b2, va_list ap) {
  return fluid_choice("Fluid", fmt, b0, b1, b2, ap);
}

int fluid_choice(const char *title, const char *fmt, const char *b0, const char *b1, const char *b2, va_list ap) {
  char buffer[8096];
  vsnprintf(buffer, sizeof(buffer), fmt, ap);
  if (b0 && b1 && b2) {
    return fluid::choice(title, buffer, { {b0, b0[0]}, {b1, b1[0]}, {b2, b2[0]} });
  } else if (b0 && b1) {
    return fluid::choice(title, buffer, { {b0, b0[0]}, {b1, b1[0]} });
  } else if (b0) {
    return fluid::choice(title, buffer, { {b0, b0[0]} });
  } else {
    return -1;
  }
}

/**
 Send a message to the user.

 In interactive mode, open a dialog box with an [i] icon and an [ok] button.
 In batch mode, print the message to stdout.
 */
void fluid::message(const std::string &title, const std::string &message) {
  if (Fluid.console_mode()) {
    printf("%s: %s\n", title.c_str(), message.c_str());
  } else {
    fl_message_title(title.c_str());
    fl_message("%s", message.c_str());
  }
}

/**
 Send a warning message to the user.

 In interactive mode, open a dialog box with an [!] icon and an [ok] button.
 In batch mode, print the message to stdout.
 */
void fluid::alert(const std::string &title, const std::string &message) {
  if (Fluid.console_mode()) {
    printf("%s: %s\n", title.c_str(), message.c_str());
  } else {
    fl_message_title(title.c_str());
    fl_alert("%s", message.c_str());
  }
}

/**
  Send an error message to the user.

  In interactive mode, open a dialog box with an [!] icon, and an [abort] and
  a [continue] button. In batch mode, print the message to stdout and wait for
  the user to hit 'a', or <esc>, to abort, or 'c' or <enter> to continue.

  \retval -2 if the dialog was closed by pushing the window close button
  \retval -1 if the dialog was closed by hitting Escape
  \retval  0 if the user chose to abort
  \retval  1 if the user chose to continue
 */
int fluid::error_choice(const std::string &title, const std::string &message) {
  if (!Fluid.console_mode()) {
    fl_message_icon_label("!");
  }
  return fluid::choice(title, message, {{"Abort", 'a'}, {"Continue", 'c'}});
}

/**
  \param[in] title the title of the dialog window
  \param[in] message the message to display in the dialog window
  \param[in] option[0] the button all the way on the right
  \param[in] option[1] the button in the middle, if defined, defaults to <return> key
  \param[in] option[2] the button all the way on the left, if defined, or nullptr
  \retval -2 if the dialog was closed by pushing the window close button
  \retval -1 if the dialog was closed by hitting Escape
  \retval  0 if the button with first option is pushed
  \retval  1 if the button with second option is pushed
  \retval  2 if the button with third option is pushed
*/
int fluid::choice(const std::string &title, const std::string &message,
    const std::vector<msg::Option> &option)
{
  if (Fluid.console_mode()) {
    // Write the text to the console
    printf("%s: %s\n", title.c_str(), message.c_str());
    // Write all available options
    printf("Options: ");
    int default_index = (option.size() > 1) ? 1 : 0;
    bool comma = false;
    int ix = 0;
    for (const auto &opt : option) {
      if (comma) printf(", ");
      int key = (ix==default_index) ? fl_ascii_toupper(opt.key) : fl_ascii_tolower(opt.key);
      printf("%s[%c]", opt.label.c_str(), key);
      comma = true;
      ix++;
    }
    printf(": ");
    fflush(stdout);
    // Loop until we receive a supported key, or the user hits <esc> or <return>
    for (;;) {
      int key = read_console_key();
      if (key == 27) { printf("<esc>\n"); return msg::ESC; }
      if (key == '\r' || key == '\n') { printf("%s\n", option[default_index].label.c_str()); return default_index; }
      for (size_t i = 0; i < option.size(); ++i) {
        if (fl_ascii_tolower(key) == fl_ascii_tolower((unsigned char)option[i].key)) {
          printf("%s\n", option[i].label.c_str());
          return (int)i;
        }
      }
      fl_beep();
    }
    // no return
  } else {
    fl_message_title(title.c_str());
    const char *b[3] = { nullptr, nullptr, nullptr };
    // If there is only a single option, make sure the <return> key triggers it
    if (option.size() == 1) {
        return fl_choice_n("%s", nullptr, option[0].label.c_str(), nullptr, message.c_str());
    }
    // option 1 defaults to the <return> key.
    for (int i = 0; i < std::min((int)option.size(), (int)3); ++i) {
        b[i] = option[i].label.c_str();
    }
    return fl_choice_n("%s", b[0], b[1], b[2], message.c_str());
  }
}

int fluid::big_choice(const std::string &title, const std::string &message,
    const std::vector<msg::Option> &option)
{
  if (Fluid.console_mode()) return fluid::choice(title, message, option);
  struct Dialog {
    Fl_Window* win;
    Fl_Box* symbol, *msg;
    Fl_Button* ok;
    Fl_Button* cancel;
    Fl_Check_Button* choice[3];
    int result = msg::ESC;
  } dlg;
  dlg.win = new Fl_Window(400, 400, title.c_str());
  dlg.win->set_modal();
  dlg.win->callback(
    [](Fl_Widget* w, void* data) {
      Fl_Window* win = (Fl_Window*)w;
      Dialog* dlg = (Dialog*)data;
      dlg->result = msg::ESC;
      win->hide();
    }, &dlg);
  dlg.symbol = new Fl_Box(10, 10, 50, 50, "?");
  dlg.symbol->box(FL_THIN_UP_BOX);
  dlg.symbol->labelfont(FL_COURIER_BOLD);
  dlg.symbol->labelsize(42);
  dlg.symbol->labeltype(FL_SHADOW_LABEL);
  dlg.symbol->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
  dlg.symbol->labelcolor(FL_BLUE);
  dlg.symbol->color(FL_WHITE);
  fl_font(FL_HELVETICA, 12);
  int w = 320, h = 0;
  fl_measure(message.c_str(), w, h, 0);
  if (h < 50) h = 50;
  dlg.msg = new Fl_Box(70, 10, 320, h, message.c_str());
  dlg.msg->box(FL_FLAT_BOX);
  dlg.msg->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
  dlg.win->size(400, 10 + h + 18 + option.size() * 28 + 10 + 25 + 10);
  for (size_t i = 0; i < option.size(); ++i) {
    dlg.choice[i] = new Fl_Check_Button(70, 10 + h + 18 + i * 28, 320, 25, option[i].label.c_str());
    dlg.choice[i]->type(FL_RADIO_BUTTON);
    dlg.choice[i]->callback(
      [](Fl_Widget* w, void* data) {
        Fl_Window* win = w->window();
        Dialog* dlg = (Dialog*)win->user_data();
        dlg->result = fl_int(data);
        dlg->ok->activate();
    }, fl_voidptr(i));
    dlg.choice[i]->shortcut(option[i].key);
  }
  dlg.ok = new Fl_Return_Button(dlg.win->w()-220, dlg.win->h() - 10 - 25, 100, 25, "OK");
  dlg.ok->deactivate();
  dlg.ok->callback(
    [](Fl_Widget* w, void* data) {
      w->window()->hide();
    }, &dlg);
  dlg.cancel = new Fl_Button(dlg.win->w()-110, dlg.win->h() - 10 - 25, 100, 25, "Cancel");
  dlg.cancel->callback(
    [](Fl_Widget* w, void* data) {
      Dialog* dlg = (Dialog*)data;
      dlg->result = msg::ESC;
      w->window()->hide();
    }, &dlg);
  dlg.win->show();
  Fl_Window* gg = Fl::grab();
  if (gg) Fl::grab(nullptr);
  while (dlg.win->shown()) {
    Fl::wait();
  }
  delete dlg.win;
  if (gg) Fl::grab(gg);
  return dlg.result;
}
