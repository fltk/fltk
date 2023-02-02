//
//  Test program for displaying animated GIF files using the
//  Fl_Anim_GIF_Image class.
//
#include <FL/Fl_Anim_GIF_Image.H>

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Tiled_Image.H>
#include <stdlib.h>

static int g_good_count = 0, g_bad_count = 0, g_frame_count = 0;

static const Fl_Color BackGroundColor = FL_GRAY; // use e.g. FL_RED to see
                                                 // transparent parts better
static const double RedrawDelay = 1./20;         // interval [sec] for forced redraw

static void quit_cb(Fl_Widget* w_, void*) {
  exit(0);
}

static void set_title(Fl_Window *win, Fl_Anim_GIF_Image *animgif) {
  char buf[200];
  snprintf(buf, sizeof(buf), "%s (%d frames)  %2.2fx", fl_filename_name(animgif->name()),
          animgif->frames(), animgif->speed());
  if (animgif->frame_uncache())
    strcat(buf, " U");
  win->copy_label(buf);
  win->copy_tooltip(buf);
}

static void cb_forced_redraw(void *d) {
  Fl_Window *win = Fl::first_window();
  while (win) {
    if (!win->menu_window())
      win->redraw();
    win = Fl::next_window(win);
  }
  if (Fl::first_window())
    Fl::repeat_timeout(RedrawDelay, cb_forced_redraw);
}

Fl_Window *openFile(const char *name, char *flags, bool close = false) {
  // determine test options from 'flags'
  bool uncache = strchr(flags, 'u');
  char *d = flags - 1;
  int debug = 0;
  while ((d = strchr(++d, 'd'))) debug++;
  bool optimize_mem = strchr(flags, 'm');
  bool desaturate = strchr(flags, 'D');
  bool average = strchr(flags, 'A');
  bool test_tiles = strchr(flags, 'T');
  bool test_forced_redraw = strchr(flags, 'f');
  char *r = strchr(flags, 'r');
  bool resizable = r && !test_tiles;
  double scale = 1.0;
  if (r && resizable) scale = atof(r+1);
  if (scale <= 0.1 || scale > 5)
    scale = resizable ? 0.7 : 1.0;

  // setup window
  Fl::remove_timeout(cb_forced_redraw);
  Fl_Double_Window *win = new Fl_Double_Window(300, 300);
  win->color(BackGroundColor);
  if (close)
    win->callback(quit_cb);
  printf("Loading '%s'%s%s ... ", name,
    uncache ? " (uncached)" : "",
    optimize_mem ? " (optimized)" : "");

  // create a canvas for the animation
  Fl_Box *canvas = test_tiles ? 0 : new Fl_Box(0, 0, 0, 0); // canvas will be resized by animation
  Fl_Box *canvas2 = 0;
  unsigned short gif_flags = debug ? Fl_Anim_GIF_Image::LOG_FLAG : 0;
  if (debug > 1)
    gif_flags |= Fl_Anim_GIF_Image::DEBUG_FLAG;
  if (optimize_mem)
    gif_flags |= Fl_Anim_GIF_Image::OPTIMIZE_MEMORY;

  // create animation, specifying this canvas as display widget
  Fl_Anim_GIF_Image *animgif = new Fl_Anim_GIF_Image(name, canvas, gif_flags);
  bool good( animgif->ld() == 0  && animgif->valid() );
  printf("%s: %d x %d (%d frames) %s\n",
    animgif->name(), animgif->w(), animgif->h(), animgif->frames(), good ? "OK" : "ERROR");
  // for the statistics (when run on testsuite):
  g_good_count += good;
  g_bad_count += !good;
  g_frame_count += animgif->frames();

  win->user_data(animgif); // store address of image (see note in main())

  // exercise the optional tests on the animation
  animgif->frame_uncache(uncache);
  if (scale != 1.0) {
    animgif->resize(scale);
    printf("TEST: resized %s by %.2f to %d x %d\n", animgif->name(), scale, animgif->w(), animgif->h());
  }
  if (average) {
    printf("TEST: color_average %s\n", animgif->name());
    animgif->color_average(FL_GREEN, 0.5); // currently hardcoded
  }
  if (desaturate) {
    printf("TEST: desaturate %s\n", animgif->name());
    animgif->desaturate();
  }
  int W = animgif->w();
  int H = animgif->h();
  if (animgif->frames()) {
    if (test_tiles) {
      // demonstrate a way how to use the animation with Fl_Tiled_Image
      printf("TEST: use %s as tiles\n", animgif->name());
      W *= 2;
      H *= 2;
      Fl_Tiled_Image *tiled_image = new Fl_Tiled_Image(animgif);
      Fl_Group *group = new Fl_Group(0, 0, win->w(), win->h());
      group->image(tiled_image);
      group->align(FL_ALIGN_INSIDE);
      animgif->canvas(group, Fl_Anim_GIF_Image::DONT_RESIZE_CANVAS | Fl_Anim_GIF_Image::DONT_SET_AS_IMAGE );
      win->resizable(group);
    } else {
      // demonstrate a way how to use same animation in another canvas simultaneously:
      // as the current implementation allows only automatic redraw of one canvas..
      if (test_forced_redraw) {
        if (W < 400) {
          printf("TEST: open %s in another animation with application redraw\n", animgif->name());
          canvas2 = new Fl_Box(W, 0, animgif->w(), animgif->h()); // another canvas for animation
          canvas2->image(animgif); // is set to same animation!
          W *= 2;
          Fl::add_timeout(RedrawDelay, cb_forced_redraw); // force periodic redraw
        }
      }
    }
    // make window resizable (must be done before show())
    if (resizable && canvas && !test_tiles) {
      win->resizable(win);
    }
    win->size(W, H); // change to actual size of canvas
    // start the animation
    win->end();
    win->show();
    win->wait_for_expose();
    set_title(win, animgif);
    if (resizable && !test_tiles) {
      // need to reposition the widgets (have been moved by setting resizable())
      if (canvas && canvas2) {
        canvas->resize(0, 0, W/2, canvas->h());
        canvas2->resize(W/2, 0, W/2, canvas2->h());
      }
      else if (canvas) {
        canvas->resize(0, 0, animgif->canvas_w(), animgif->canvas_h());
      }
    }
    win->init_sizes(); // IMPORTANT: otherwise weird things happen at Ctrl+/- scaling
  } else {
    delete win;
    return 0;
  }
  if (debug >=3) {
    // open each frame in a separate window
    for (int i = 0; i < animgif->frames(); i++) {
      char buf[200];
      snprintf(buf, sizeof(buf), "Frame #%d", i + 1);
      Fl_Double_Window *win = new Fl_Double_Window(animgif->w(), animgif->h());
      win->copy_tooltip(buf);
      win->copy_label(buf);
      win->color(BackGroundColor);
      int w = animgif->image(i)->w();
      int h = animgif->image(i)->h();
      // in 'optimize_mem' mode frames must be offsetted to canvas
      int x = (w == animgif->w() && h == animgif->h()) ? 0 : animgif->frame_x(i);
      int y = (w == animgif->w() && h == animgif->h()) ? 0 : animgif->frame_y(i);
      Fl_Box *b = new Fl_Box(x, y, w, h);
      // get the frame image
      b->image(animgif->image(i));
      win->end();
      win->show();
    }
  }
  return win;
}

#include <FL/filename.H>
bool openDirectory(const char *dir, char *flags) {
  dirent **list;
  int nbr_of_files = fl_filename_list(dir, &list, fl_alphasort);
  if (nbr_of_files <= 0)
    return false;
  int cnt = 0;
  for (int i = 0; i < nbr_of_files; i++) {
    char buf[512];
    const char *name = list[i]->d_name;
    if (!strcmp(name, ".") || !strcmp(name, "..")) continue;
    const char *p = strstr(name, ".gif");
    if (!p) p = strstr(name, ".GIF");
    if (!p) continue;
    if (*(p+4)) continue; // is no extension!
    snprintf(buf, sizeof(buf), "%s/%s", dir, name);
    if (strstr(name, "debug"))  // hack: when name contains 'debug' open single frames
      strcat(flags, "d");
    if (openFile(buf, flags, cnt == 0))
      cnt++;
  }
  return cnt != 0;
}

static void change_speed(double delta) {
  Fl_Widget *below = Fl::belowmouse();
  if (below && below->image()) {
    Fl_Anim_GIF_Image *animgif = 0;
    // Q: is there a way to determine Fl_Tiled_Image without using dynamic cast?
    Fl_Tiled_Image *tiled = dynamic_cast<Fl_Tiled_Image *>(below->image());
    animgif = tiled ?
              dynamic_cast<Fl_Anim_GIF_Image *>(tiled->image()) :
              dynamic_cast<Fl_Anim_GIF_Image *>(below->image());
    if (animgif && animgif->playing()) {
      double speed = animgif->speed();
      if (!delta) speed = 1.;
      else speed += delta;
      if (speed < 0.1) speed = 0.1;
      if (speed > 10) speed = 10;
      animgif->speed(speed);
      set_title(below->window(), animgif);
    }
  }
}

static int events(int event) {
  if (event == FL_SHORTCUT) {
    if (Fl::event_key() == '+')
      change_speed(0.1);
    else if (Fl::event_key() == '-')
      change_speed(-0.1);
    else if (Fl::event_key() == '0')
      change_speed(0);
    else
      return 0;
    return 1;
  }
  return 0;
}

static const char testsuite[] = "testsuite";

int main(int argc, char *argv[]) {
  fl_register_images();
  Fl::add_handler(events);
  char *openFlags = (char *)calloc(1024, 1);
  if (argc > 1) {
    // started with argumemts
    if (strstr(argv[1], "-h")) {
      printf("Usage:\n"
             "   -t [directory] [-{flags}] open all files in directory (default name: %s) [with options]\n"
             "   filename [-{flags}] open single file [with options] \n"
             "   No arguments open a fileselector\n"
             "   {flags} can be: d=debug mode, u=uncached, D=desaturated, A=color averaged, T=tiled\n"
             "                   m=minimal update, r[scale factor]=resize by 'scale factor'\n"
             "   Use keys '+'/'-/0' to change speed of the active image (belowmouse).\n", testsuite);
      exit(1);
    }
    for (int i = 1; i < argc; i++) {
      if (argv[i][0] == '-')
        strcat(openFlags, &argv[i][1]);
    }
    if (strchr(openFlags, 't')) { // open all GIF-files in a given directory
      const char *dir = testsuite;
      for (int i = 2; i < argc; i++)
        if (argv[i][0] != '-')
          dir = argv[i];
      openDirectory(dir, openFlags);
      printf("Summary: good=%d, bad=%d, frames=%d\n", g_good_count, g_bad_count, g_frame_count);
    } else { // open given file(s)
      for (int i = 1; i < argc; i++)
        if (argv[i][0] != '-')
          openFile(argv[i], openFlags, strchr(openFlags, 'd'));
    }
  } else {
    // started without arguments: choose file
    Fl_GIF_Image::animate = true; // create animated shared .GIF images (e.g. file chooser)
    while (1) {
      Fl::add_timeout(0.1, cb_forced_redraw); // animate images in chooser
      const char *filename = fl_file_chooser("Select a GIF image file","*.{gif,GIF}", NULL);
      Fl::remove_timeout(cb_forced_redraw);
      if (!filename)
        break;
      Fl_Window *win = openFile(filename, openFlags);
      Fl::run();
      // delete last window (which is now just hidden) to test destructors
      // NOTE: it is essential that *before* doing this also the
      //       animated image is destroyed, otherwise it will crash
      //       because it's canvas will be gone.
      //       In order to keep this demo simple, the adress of the
      //       Fl_Anim_GIF_Image has been stored in the window's user_data.
      //       In a real-life application you will probably store
      //       it somewhere in the window's or canvas' object and destroy
      //       the image in the window's or canvas' destructor.
      if (win && win->user_data())
        delete ((Fl_Anim_GIF_Image *)win->user_data());
      delete win;
    }
  }
  return Fl::run();
}
