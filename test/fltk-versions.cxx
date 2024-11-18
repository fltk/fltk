//
// Library version test program for the Fast Light Tool Kit (FLTK).
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

// This program is work in progress and may not be "perfect".

#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>
#include <stdio.h>

static const int ww = 640, mw = 750;  // initial, max. window width
static const int wh = 200, mh = 300;  // initial, max. window height

// Function to determine the platform (system and backend).
// Note: the display must have been opened before this is called.
// Returns a string describing the system/platform and backend.

static const char *get_platform() {
#if defined(_WIN32)
  return "Windows";
#elif defined(FLTK_USE_X11) || defined(FLTK_USE_WAYLAND)
# if defined(FLTK_USE_X11)
  if (fl_x11_display())
    return "Unix/Linux (X11)";
# endif
# if defined(FLTK_USE_WAYLAND)
  if (fl_wl_display())
    return "Unix/Linux (Wayland)";
# endif
  return "X11 or Wayland (backend unknown or display not opened)";
#elif defined(__APPLE__)
  return "macOS (native)";
#endif
  return "platform unknown, unsupported, or display not opened";
}

// set box attributes and optionally set a background color (debug mode)

static void set_attributes(Fl_Widget *w, Fl_Color col) {
  w->labelfont(FL_COURIER);
  w->labelsize(16);
  w->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
#if (0) // 1 = debug: set a background color for a box (widget)
  w->box(FL_FLAT_BOX);
  w->color(col);
#endif
}

static char version[9][80];
static Fl_Box *box[9];

// Optional: uncomment next line to disable wayland backend
// FL_EXPORT bool fl_disable_wayland = true;

int main(int argc, char **argv) {
  int versions = 0;
  fl_open_display();
  const char *platform = get_platform();
  printf("System/platform   = %s\n", platform);

  // Version comparison results (Unicode check marks in comments are experimental)

  const char *YES = "OK";   // "🗹"; // "✓";
  const char *NO  = "FAIL"; // "🗷"; // "❌";

  sprintf(version[versions++], "FL_VERSION        = %6.4f", FL_VERSION);
  sprintf(version[versions++], "Fl::version()     = %6.4f", Fl::version());
  sprintf(version[versions++], "%s", (FL_VERSION == Fl::version()) ? YES : NO);

  sprintf(version[versions++], "FL_API_VERSION    = %6d", FL_API_VERSION);
  sprintf(version[versions++], "Fl::api_version() = %6d", Fl::api_version());
  sprintf(version[versions++], "%s", (FL_API_VERSION == Fl::api_version()) ? YES : NO);

  sprintf(version[versions++], "FL_ABI_VERSION    = %6d", FL_ABI_VERSION);
  sprintf(version[versions++], "Fl::abi_version() = %6d", Fl::abi_version());
  sprintf(version[versions++], "%s", (FL_ABI_VERSION == Fl::abi_version()) ? YES : NO);

  for (int i = 0; i < versions; i++) {
    if (i % 3 == 1)                 // 2nd line followed by check mark or text
      printf("%s  ", version[i]);
    else                            // 1st and 3rd line
      printf("%s\n", version[i]);
  }
  fflush(stdout);

#ifdef FL_ABI_VERSION
  if (FL_ABI_VERSION != Fl::abi_version()) {
    printf("*** FLTK ABI version mismatch: headers = %d, lib = %d ***\n",
           FL_ABI_VERSION, Fl::abi_version());
    fflush(stdout);
    fl_message("*** FLTK ABI version mismatch: headers = %d, lib = %d ***",
               FL_ABI_VERSION, Fl::abi_version());
  }
#endif

  Fl_Window *window = new Fl_Window(ww, wh);

  Fl_Grid *grid = new Fl_Grid(0, 0, ww, wh);
  grid->layout(4, 3, 20, 5);

  Fl_Box *title = new Fl_Box(0, 0, 0, 0, platform);
  set_attributes(title, FL_YELLOW);
  title->labelfont(FL_HELVETICA_BOLD);
  grid->widget(title, 0, 0, 1, 3);
  grid->row_height(0, 40);
  title->labelsize(20);

  for (int i = 0; i < 3; i += 1) {
    box[3 * i    ] = new Fl_Box(0, 0, 270, 0, version[3 * i]);
    box[3 * i + 1] = new Fl_Box(0, 0, 270, 0, version[3 * i + 1]);
    box[3 * i + 2] = new Fl_Box(0, 0,  40, 0, version[3 * i + 2]);
    grid->widget(box[3 * i],     i + 1, 0);
    grid->widget(box[3 * i + 1], i + 1, 1);
    grid->widget(box[3 * i + 2], i + 1, 2);
    grid->row_height(i + 1, 30);
  }

  for (int i = 0; i < 9; i++)
    set_attributes(box[i], FL_GREEN);

  window->end();
  window->resizable(grid);
  window->size_range(ww, wh, mw, mh);
  window->show(argc, argv);
  return Fl::run();
}
