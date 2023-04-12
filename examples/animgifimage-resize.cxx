//
//  Test program for Fl_Anim_GIF_Image::copy().
//
#include <FL/Fl_Anim_GIF_Image.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <stdio.h>
#include <stdlib.h>

static Fl_Anim_GIF_Image *orig = 0;
static bool draw_grid = true;

static int events(int event_) {
  if (event_ == FL_SHORTCUT && Fl::first_window()) {
    if (Fl::event_key()=='g') {
      draw_grid = !draw_grid;
      printf("grid: %s\n", (draw_grid ? "ON" : "OFF"));
    }
    else if (Fl::event_key()=='b') {
      if (Fl_Image::scaling_algorithm() != FL_RGB_SCALING_BILINEAR)
        Fl_Image::scaling_algorithm(FL_RGB_SCALING_BILINEAR);
      else
        Fl_Image::scaling_algorithm(FL_RGB_SCALING_NEAREST);
      printf("bilenear: %s\n", (Fl_Image::scaling_algorithm() != FL_RGB_SCALING_BILINEAR ? "OFF" : "ON"));
    }
    else
      return 0;
    Fl::first_window()->redraw();
  }
  return 1;
}

class Canvas : public Fl_Box {
  typedef Fl_Box Inherited;
public:
  Canvas(int x, int y, int w, int h) :
    Inherited(x, y, w, h) {}
  void draw() FL_OVERRIDE {
    if (draw_grid) {
      // draw a transparency grid as background
      static const Fl_Color C1 = fl_rgb_color(0xcc, 0xcc, 0xcc);
      static const Fl_Color C2 = fl_rgb_color(0x88, 0x88, 0x88);
      static const int SZ = 8;
      for (int y = 0; y < h(); y += SZ) {
        for (int x = 0; x < w(); x += SZ) {
          fl_color(x%(SZ * 2) ? y%(SZ * 2) ? C1 : C2 : y%(SZ * 2) ? C2 : C1);
          fl_rectf(x, y, 32, 32);
        }
      }
    }
    // draw the current image frame over the grid
    Inherited::draw();
  }
  void do_resize(int W, int H) {
    if (image() && (image()->w() != W || image()->h() != H)) {
      Fl_Anim_GIF_Image *animgif = (Fl_Anim_GIF_Image *)image();
      animgif->stop();
      image(0);
      // delete already copied images
      if (animgif != orig ) {
        delete animgif;
      }
      Fl_Anim_GIF_Image *copied = (Fl_Anim_GIF_Image *)orig->copy(W, H);
      if (!copied->valid()) { // check success of copy
        Fl::warning("Fl_Anim_GIF_Image::copy() %d x %d failed", W, H);
      }
      else {
        printf("resized to %d x %d\n", copied->w(), copied->h());
      }
      copied->canvas(this, Fl_Anim_GIF_Image::DONT_RESIZE_CANVAS);
    }
    window()->cursor(FL_CURSOR_DEFAULT);
  }
  static void do_resize_cb(void *d) {
    Canvas *c = (Canvas *)d;
    c->do_resize(c->w(), c->h());
  }
  void resize(int x, int y, int w, int h) FL_OVERRIDE {
    Inherited::resize(x, y, w, h);
    // decouple resize event from actual resize operation
    // to avoid lockups..
    Fl::remove_timeout(do_resize_cb, this);
    Fl::add_timeout(0.1, do_resize_cb, this);
    window()->cursor(FL_CURSOR_WAIT);
  }
};

int main(int argc, char *argv[]) {
  // setup play parameters from args
  const char *fileName = 0;
  bool bilinear = false;
  bool optimize = false;
  bool uncache = false;
  bool debug = false;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-b")) // turn bilinear scaling on
      bilinear = true;
    else if (!strcmp(argv[i], "-o")) // turn optimize on
      optimize = true;
    else if (!strcmp(argv[i], "-g")) // disable grid
      draw_grid = false;
    else if (!strcmp(argv[i], "-u")) // uncache
      uncache = true;
    else if (!strcmp(argv[i], "-d")) // debug
      debug = true;
    else if (argv[i][0] != '-' && !fileName) {
      fileName = argv[i];
    }
    else if (argv[i][0] == '-') {
      printf("Invalid argument: '%s'\n", argv[i]);
      exit(1);
    }
  }
  if (!fileName) {
    fprintf(stderr, "Test program for animated copy.\n");
    fprintf(stderr, "Usage: %s fileName [-b]ilinear [-o]ptimize [-g]rid [-u]ncache\n", argv[0]);
    exit(0);
  }
  Fl_Anim_GIF_Image::min_delay = 0.1; // set a minumum delay for playback

  Fl_Double_Window win(640, 480);

  // prepare a canvas for the animation
  // (we want to show it in the center of the window)
  Canvas canvas(0, 0, win.w(), win.h());
  win.resizable(win);
  win.size_range(1, 1);

  win.end();
  win.show();

  // create/load the animated gif and start it immediately.
  // We use the 'DONT_RESIZE_CANVAS' flag here to tell the
  // animation not to change the canvas size (which is the default).
  int flags = Fl_Anim_GIF_Image::Fl_Anim_GIF_Image::DONT_RESIZE_CANVAS;
  if (optimize) {
    flags |= Fl_Anim_GIF_Image::OPTIMIZE_MEMORY;
    printf("Using memory optimization (if image supports)\n");
  }
  if (debug) {
    flags |= Fl_Anim_GIF_Image::DEBUG_FLAG;
  }
  orig = new Fl_Anim_GIF_Image(/*name_=*/ fileName,
                             /*canvas_=*/ &canvas,
                              /*flags_=*/ flags );

  // check if loading succeeded
  printf("%s: valid: %d frames: %d uncache: %d\n",
    orig->name(), orig->valid(), orig->frames(), orig->frame_uncache());
  if (orig->valid()) {
    win.copy_label(fileName);

    // print information about image optimization
    int n = 0;
    for (int i = 0; i < orig->frames(); i++) {
      if (orig->frame_x(i) != 0 || orig->frame_y(i) != 0) n++;
    }
    printf("image has %d optimized frames\n", n);

    Fl_Image::scaling_algorithm(FL_RGB_SCALING_NEAREST);
    if (bilinear) {
      Fl_Image::scaling_algorithm(FL_RGB_SCALING_BILINEAR);
      printf("Using bilinear scaling - can be slow!\n");
      // NOTE: this can be *really* slow with large sizes, if FLTK
      //       has to resize on its own without hardware scaling enabled.
    }
    orig->frame_uncache(uncache);
    if (uncache) {
      printf("Caching disabled - watch cpu load!\n");
    }

    // set initial size to fit into window
    double ratio = orig->valid() ? (double)orig->w() / orig->h() : 1;
    int W = win.w() - 40;
    int H = int(W / ratio);
    printf("original size: %d x %d\n", orig->w(), orig->h());
    win.size(W, H);
    Fl::add_handler(events);

    return Fl::run();
  }
}
