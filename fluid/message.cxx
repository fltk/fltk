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
  char buffer[8096];
  vsnprintf(buffer, sizeof(buffer), fmt, ap);
  if (b0 && b1 && b2) {
    return fluid::choice("Fluid", buffer, { {b0, b0[0]}, {b1, b1[0]}, {b2, b2[0]} });
  } else if (b0 && b1) {
    return fluid::choice("Fluid", buffer, { {b0, b0[0]}, {b1, b1[0]} });
  } else if (b0) {
    return fluid::choice("Fluid", buffer, { {b0, b0[0]} });
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
    bool comma = false;
    for (const auto &opt : option) {
      if (comma) printf(", ");
      printf("%s[%c]", opt.label.c_str(), opt.key);
      comma = true;
    }
    printf(": ");
    fflush(stdout);
    // Loop until we receive a supported key, or the user hits <esc> or <return>
    int default_index = (option.size() > 1) ? 1 : 0;
    for (;;) {
      int key = read_console_key();
      if (key == 27) { printf("<esc>\n"); return msg::ESC; }
      if (key == '\r' || key == '\n') { printf("<return>\n"); return default_index; }
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

