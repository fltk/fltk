//
// Alpha rendering benchmark program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
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
#include <FL/Fl_Button.H>
#include <FL/Fl_Image.H>
#include <FL/platform.H>
#include <FL/fl_draw.H>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// These constants define the image dimensions and
// the number of frames of the animation
const unsigned int dim = 256;
const unsigned int frames = 48;

static Fl_RGB_Image *img[frames];
static uchar curframe;

static void make_images() {

  unsigned i;
  for (i = 0; i < frames; i++) {
    const unsigned size = dim * dim * 4;
    uchar *data = new uchar[size];

    memset(data, 0, size);

    // First a black box, 10x10 pixels in the top-left corner
    int x, y;
    for (x = 0; x < 10; x++) {
      for (y = 0; y < 10; y++) {
        data[y * dim * 4 + x * 4 + 3] = 255;
      }
    }

    // A fading sphere
    uchar alpha = 255;
    if (i < frames / 2)
      alpha = uchar(255 * (i / ((float) frames / 2)));
    else
      alpha = uchar(255 * (((frames / 2) - (i - frames / 2)) / ((float) frames / 2)));

    const int spherew = 60;
    const int spherex = (dim - spherew) / 2;
    const int maxdist = (spherew / 2) * (spherew / 2);
    for (x = spherex; x < spherex + spherew; x++) {
      for (y = 20; y < 20 + spherew; y++) {

        float distx = x - (spherex + (float) spherew / 2);
        float disty = y - (20 + (float) spherew / 2);
        float dist = distx * distx + disty * disty;

        if (dist > maxdist)
          continue;

        const float fill = dist / maxdist;
        const uchar grey = uchar(fill * 255);

        uchar myalpha = alpha;
        if (fill > 0.9)
          myalpha *= uchar((1.0f - fill) * 10);

        data[y * dim * 4 + x * 4 + 0] = grey;
        data[y * dim * 4 + x * 4 + 1] = grey;
        data[y * dim * 4 + x * 4 + 2] = grey;
        data[y * dim * 4 + x * 4 + 3] = myalpha;
      }
    }

    // A moving blob
    const float pos = (i / (float) frames) * 2 - 0.5f;

    const int xoffset = int(pos * dim);
    const int yoffset = 2 * dim / 3;
    const int w = dim / 4;

    for (x = -w; x < w; x++) {
      if (x + xoffset < 0 || x + xoffset >= (int)dim)
        continue;
      for (y = yoffset - w; y < yoffset + w; y++) {
        const uchar grey = abs(y - yoffset);
        data[y * dim * 4 + (x + xoffset) * 4 + 2] = grey;
        data[y * dim * 4 + (x + xoffset) * 4 + 3] = 127;
      }
    }

    img[i] = new Fl_RGB_Image(data, dim, dim, 4);
  }
}

class window: public Fl_Double_Window {
public:
  window(int X, int Y, const char *lbl): Fl_Double_Window(X, Y, lbl) {}

  void draw() FL_OVERRIDE {
    Fl_Double_Window::draw();

    // Test both cx/cy offset and clipping. Both borders should have a 5-pixel edge,
    // and the upper-left black box should not be visible.
    fl_push_clip(5, 5, w() - 5, h() - 5);
    img[curframe]->draw(0, 0, dim, dim, 5, 5);
    fl_pop_clip();
  }
};

static window *win;

static void cb(void *) {

  win->redraw();

  Fl::repeat_timeout(1.0f / 24, cb);

  curframe++;
  curframe %= frames;
}

int main(int argc, char **argv) {
  win = new window(256, 256, "Alpha rendering benchmark, watch CPU use");
  win->color(fl_rgb_color(142, 0, 0));

  make_images();

  win->end();
  win->show(argc, argv);

  Fl::add_timeout(1.0f / 24, cb);

  return Fl::run();
}
