//
// Colormap generation program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2026 by Bill Spitzak and others.
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

//
// This program produces the contents of "fl_cmap.h" on stdout. If you
// need to change the built-in FLTK colormap, change this source file
// accordingly and proceed as follows:
//
// (1) Build the program `bin/cmap`
//
//   $ `MAKE` cmap   # see below for details
//
// or
//
//   $ cmake --build . --target cmap
//
// in your CMake build directory (replace `MAKE` with your build tool,
// for instance `make` or `ninja` or whatever you chose to build FLTK).
//
// (2) Run the program from the build folder:
//
//   $ bin/cmap > output.h
//
// (3) Compare `output.h` with the existing file in the source folder
// `[fltk-source-dir]/src/fl_cmap.h`. Take care that the result (diff)
// is plausible.
//
// (4) Replace `[fltk-source-dir]/src/fl_cmap.h` in the FLTK source tree
// with the created file `output.h`.
//
// (5) Build and test FLTK, check if the new color map is as intended.
//
// (6) Finally check in *both* modified file with `git commit` and push
// your changes to the main repository with `git push`.
//
// Note: the created executable `bin/cmap` is temporary and may be deleted
// at any time. It is not intended to be `installed` with the FLTK library.
//

#include <stdio.h>
#include <math.h>
#include <time.h>

// This table is initialized with color values taken from the colormap
// on an IRIX 4.3 machine:

// "full intensity colors" have been turned down some to make white
// background less intense by default.  The hope is that this will make
// FLTK programs more friendly on color-adjusted screens. If you want
// pure colors you should get them out of the colormap.

// #define III 244 // maximum intensity of the basic colors

// that results in errors and unshared colormap entries, so full intensity:

#define III 255 // maximum intensity of the basic colors

static short cmap[256][3] = {

  // 3-bit colormap:

  {  0,  0,  0},        // black
  {III,  0,  0},        // red
  {  0,III,  0},        // green
  {III,III,  0},        // yellow
  {  0,  0,III},        // blue
  {III,  0,III},        // magenta
  {  0,III,III},        // cyan
  {III,III,III},        // white

  // pastel versions of those colors, from SGI's standard color map:

  { 85, 85, 85},        // 1/3 gray
  {198,113,113},        // salmon? pale red?
  {113,198,113},        // pale green
  {142,142, 56},        // khaki
  {113,113,198},        // pale blue
  {142, 56,142},        // purple, orchid, pale magenta
  { 56,142,142},        // cadet blue, aquamarine, pale cyan

  // The next location (index 15) is used for FL_SELECTION_COLOR. It formerly
  // was 2/3 gray but this is changed to be the Windows blue color. This
  // allows the default behavior on both X and Windows to match:
  // {170,170,170},     // old 2/3 gray color

  {  0,  0,128},        // 15 = FL_SELECTION_COLOR

  // These next 16 (index 16 - 31) are the FL_FREE_COLOR area. In some
  // versions of FLTK these were filled with random colors that a Irix 5.3
  // machine placed in these locations.

  // This version uses colors that NewTek has assigned for their GUI
  // (from George Yohng):

  {168,168,152},        // 16 = FL_FREE_COLOR
  {232,232,216},
  {104,104, 88},
  {152,168,168},
  {216,232,232},
  { 88,104,104},
  {156,156,168},
  {220,220,232},
  { 92, 92,104},
  {156,168,156},
  {220,232,220},
  { 92,104, 92},
  {144,144,144},
  {192,192,192},
  { 80, 80, 80},
  {160,160,160},        // 31

  // The rest of the colormap is a gray ramp and table, filled in below:
};

// This is Fl::background from Fl_get_system_colors.cxx, with modifications:

#define FL_GRAY_RAMP 32
#define FL_NUM_GRAY  24
#define FL_GRAY      49 // old value is 47

typedef unsigned char uchar;

void background(uchar r, uchar g, uchar b) {
  // replace the gray ramp so that color 47 (by default 2/3) is this color
  if (!r)
    r = 1;
  else if (r == 255)
    r = 254;
  double powr = log(r / 255.0) / log((FL_GRAY - FL_GRAY_RAMP) / (FL_NUM_GRAY - 1.0));
  if (!g)
    g = 1;
  else if (g == 255)
    g = 254;
  double powg = log(g / 255.0) / log((FL_GRAY - FL_GRAY_RAMP) / (FL_NUM_GRAY - 1.0));
  if (!b)
    b = 1;
  else if (b == 255)
    b = 254;
  double powb = log(b / 255.0) / log((FL_GRAY - FL_GRAY_RAMP) / (FL_NUM_GRAY - 1.0));
  for (int i = 0; i < FL_NUM_GRAY; i++) {
    double gray = i / (FL_NUM_GRAY - 1.0);
    cmap[i + FL_GRAY_RAMP][0] = uchar(pow(gray, powr) * 255 + .5);
    cmap[i + FL_GRAY_RAMP][1] = uchar(pow(gray, powg) * 255 + .5);
    cmap[i + FL_GRAY_RAMP][2] = uchar(pow(gray, powb) * 255 + .5);
  }
}

int main() {

  int i, r, g, b, year;
  time_t t = time(0);
  struct tm *lt = localtime(&t);
  year = lt->tm_year + 1900; // copyright year

  // fill in the gray ramp:
  // background(170, 170, 170); // old fltk colors
  background(0xc0, 0xc0, 0xc0); // microsoft colors

  // copy the 1/3 and 2/3 gray to the closest locations in gray ramp:
  cmap[39][0] = cmap[39][1] = cmap[39][2] = 85;
  cmap[47][0] = cmap[47][1] = cmap[47][2] = 170;

  // fill in the color cube
  i = 56;
  for (b = 0; b < 5; b++) {
    for (r = 0; r < 5; r++) {
      for (g = 0; g < 8; g++) {
        cmap[i][0] = r * 255 / 4;
        cmap[i][1] = g * 255 / 7;
        cmap[i][2] = b * 255 / 4;
        i++;
      }
    }
  }

  // write comment into 'cmap.h' so the reader knows what it is good for

  printf("//\n");
  printf("// DO NOT EDIT THIS FILE !\n");
  printf("//\n");
  printf("// This file must be generated by \"util/cmap.cxx\".\n");
  printf("// See instructions in this file.\n");
  printf("//\n");
  printf("// Copyright 1998-%d by Bill Spitzak and others.\n", year);
  printf("//\n");
  printf("// This library is free software. Distribution and use rights are outlined in\n");
  printf("// the file \"COPYING\" which should have been included with this file.  If this\n");
  printf("// file is missing or damaged, see the license at:\n");
  printf("//\n");
  printf("//     https://www.fltk.org/COPYING.php\n");
  printf("//\n");
  printf("// Please see the following page on how to report bugs and issues:\n");
  printf("//\n");
  printf("//     https://www.fltk.org/bugs.php\n");
  printf("//\n");

  // write color map values

  for (i = 0; i < 256; i++) {
    printf("    0x%02x%02x%02x00", cmap[i][0], cmap[i][1], cmap[i][2]);
    if (i < 255)
      printf(", // %3d\n", i);
    else
      printf("  // %3d\n", i);
  }

  // write final comment

  printf("//\n");
  printf("// End of src/fl_cmap.h - generated by util/cmap.cxx\n");
  printf("//\n");

  return 0;
}
