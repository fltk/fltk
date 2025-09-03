//
// Fl_Grid based dialog window for the Fast Light Tool Kit (FLTK).
//
// Copyright 2023 by Bill Spitzak and others.
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

// This demo program builds a flexible layout of a dialog similar
// to fl_ask(), fl_choice(), and others.
//
// For <N> buttons we use an Fl_Grid with 2 rows and <N+2> columns:
// - Cell (0, 0) (III) holds an icon (top left cell)
// - Cell (0, 1) holds the message text; spans <N+1> columns
// - Cell (1, n) holds buttons (2 <= n <= <N+1>)
// - Column 1    (XX) is the resizable column; not used for buttons
// - Column 2+   is used for buttons
//      _________________________________________________
//     |                                                 |
//     |  III  Some message text ... ... ... ... ... ... |
//     |  III  more message text ... ... ... ... ... ... |
//     |  III  more message text ... ... ... ... ... ... |
//     |       more message text ... ... ... ... ... ... |
//     |       more message text ... ... ... ... ... ... |
//     |       XX                      +––––––––+ +––––+ |
//     |       XX  [more buttons ...]  | Cancel | | OK | |
//     |       XX                      +––––––––+ +––––+ |
//     |_________________________________________________|

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>

static const int ROWS     = 2;
static const int BUTTONS  = 4; // default = 4, use 1 to 5 to test
static const int COLS     = BUTTONS + 2;
static const int BUTTON_H = 25;
static const int ICON_W   = 60;
static const int ICON_H   = 70;
static const int MARGIN   = 10;
static const int GAP      =  8;

// Button labels (left to right)
static const char *labels[5] = {
  "Quit", "Copy", "Cancel", "OK", "More ..." };

static const char *tooltips[5] = {
  "Quit this program",
  "Copy the message text to the clipboard",
  "Cancel - does nothing",
  "OK - does nothing",
  "More buttons could be added here"
};

// button widths (left to right) to avoid font calculations
static const int button_w[5] = { 50, 50, 70, 40, 100};

static int col_weights[] = { 0, 100, 0, 0, 0, 0, 0 };
static int row_weights[] = { 100, 0, 0, 0, 0, 0, 0 };

static const char *message_text =
  "This is a long message in an Fl_Grid based dialog "
  "that may wrap over more than one line. "
  "Resize the window to see how it (un)wraps.";

Fl_Box *message_box = 0; // global only to simplify the code

// Common button callback

void button_cb(Fl_Widget *w, void *v) {
  int val = fl_int(v);
  printf("Button %d: '%s'\n", val, w->label());
  switch(val) {
    case 0:                 // Quit
      w->window()->hide();
      break;
    case 1: {               // Copy
        const char *text = message_box->label();
        const int len = (int)strlen(text);
        Fl::copy(text, len, 1);
      }
      printf("Message copied to clipboard.\n");
      break;
    default:
      break;
  }
  fflush(stdout);
}

int main(int argc, char **argv) {

  int min_w = ICON_W + 2 * MARGIN + (BUTTONS + 1) * GAP;
  int min_h = ICON_H + 10 + 2 * MARGIN + GAP + BUTTON_H;

  for (int i = 0; i < BUTTONS; i++) {
    min_w += button_w[i];
  }

  Fl_Double_Window *win = new Fl_Double_Window(min_w, min_h, "Fl_Grid Based Dialog");

  Fl_Grid *grid = new Fl_Grid(0, 0, win->w(), win->h());
  grid->layout(ROWS, COLS, MARGIN, GAP);
  grid->color(FL_WHITE);
  grid->tooltip("Resize the window to see this dialog \"in action\"");

  // Child 0: Fl_Box for the "icon" or image (fixed size)

  Fl_Box *icon = new Fl_Box(0, 0, ICON_W, ICON_H, "ICON");
  grid->widget(icon, 0, 0, 1, 1, FL_GRID_TOP);
  icon->box(FL_THIN_UP_BOX);
  icon->color(0xddffff00);
  icon->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
  icon->tooltip("This could also be a full Fl_Image or subclass thereof");

  // Child 1: the message box

  message_box = new Fl_Box(0, 0, 0, 0);
  grid->widget(message_box, 0, 1, 1, BUTTONS + 1, FL_GRID_FILL);
  message_box->label(message_text);
  message_box->align(FL_ALIGN_TOP|FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
  message_box->tooltip("The text in this box can be copied to the clipboard");

  // Children 2++: the buttons (left to right for tab navigation order)

  for (int i = 0; i < BUTTONS; i++) {
    Fl_Button *b = new Fl_Button(0, 0, button_w[i], BUTTON_H, labels[i]);
    grid->widget(b, 1, i + 2);
    b->callback(button_cb, fl_voidptr(i));
    b->tooltip(tooltips[i]);
  }

  grid->end();

  // set row and column weights for resizing

  grid->row_weight(row_weights, ROWS);
  grid->col_weight(col_weights, COLS);

  // Set environment variable "FLTK_GRID_DEBUG=1" or uncomment this line:
  // grid->show_grid(1);     // enable to display grid helper lines

  win->end();
  win->resizable(grid);
  win->size_range(min_w, min_h, 3 * min_w, min_h + 50);
  win->show(argc, argv);

  int ret = Fl::run();
  delete win; // not necessary but useful to test for memory leaks
  return ret;
}
