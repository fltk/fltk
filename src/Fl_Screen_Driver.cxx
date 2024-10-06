//
// All screen related calls in a driver style class.
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

#include "Fl_Screen_Driver.H"
#include <FL/Fl_Image.H>
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include "Fl_Window_Driver.H"
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tooltip.H>
#include <string.h> // for memchr

// these are set by Fl::args() and override any system colors: from Fl_get_system_colors.cxx
extern const char *fl_fg;
extern const char *fl_bg;
extern const char *fl_bg2;
// end of extern additions workaround

char Fl_Screen_Driver::bg_set = 0;
char Fl_Screen_Driver::bg2_set = 0;
char Fl_Screen_Driver::fg_set = 0;
Fl_System_Driver *Fl_Screen_Driver::system_driver = NULL;
const int Fl_Screen_Driver::fl_NoValue =     0x0000;
const int Fl_Screen_Driver::fl_WidthValue =  0x0004;
const int Fl_Screen_Driver::fl_HeightValue = 0x0008;
const int Fl_Screen_Driver::fl_XValue =      0x0001;
const int Fl_Screen_Driver::fl_YValue =      0x0002;
const int Fl_Screen_Driver::fl_XNegative =   0x0010;
const int Fl_Screen_Driver::fl_YNegative =   0x0020;

// This default key table is used for all system drivers that don't define
// and/or use their own table. It is defined here "static" and assigned
// in the constructor to avoid static initialization race conditions.
//
// As of January 2022, the Windows and Wayland platforms use this table.
// X11 does not use a key table at all; macOS has its own table.
// Platforms that use their own key tables must assign them in their
// constructors (which overwrites the pointer and size).

static Fl_Screen_Driver::Keyname default_key_table[] = {
  {' ',           "Space"},
  {FL_BackSpace,  "Backspace"},
  {FL_Tab,        "Tab"},
  {0xff0b/*XK_Clear*/, "Clear"},
  {FL_Enter,      "Enter"}, // X says "Enter"
  {FL_Pause,      "Pause"},
  {FL_Scroll_Lock, "Scroll_Lock"},
  {FL_Escape,     "Escape"},
  {FL_Home,       "Home"},
  {FL_Left,       "Left"},
  {FL_Up,         "Up"},
  {FL_Right,      "Right"},
  {FL_Down,       "Down"},
  {FL_Page_Up,    "Page_Up"}, // X says "Prior"
  {FL_Page_Down,  "Page_Down"}, // X says "Next"
  {FL_End,        "End"},
  {FL_Print,      "Print"},
  {FL_Insert,     "Insert"},
  {FL_Menu,       "Menu"},
  {FL_Num_Lock,   "Num_Lock"},
  {FL_KP_Enter,   "KP_Enter"},
  {FL_Shift_L,    "Shift_L"},
  {FL_Shift_R,    "Shift_R"},
  {FL_Control_L,  "Control_L"},
  {FL_Control_R,  "Control_R"},
  {FL_Caps_Lock,  "Caps_Lock"},
  {FL_Meta_L,     "Meta_L"},
  {FL_Meta_R,     "Meta_R"},
  {FL_Alt_L,      "Alt_L"},
  {FL_Alt_R,      "Alt_R"},
  {FL_Delete,     "Delete"}
};

int Fl_Screen_Driver::keyboard_screen_scaling = 1;

Fl_Screen_Driver::Fl_Screen_Driver() :
num_screens(-1), text_editor_extra_key_bindings(NULL)
{
  // initialize default key table (used in fl_shortcut.cxx)
  key_table = default_key_table;
  key_table_size = sizeof(default_key_table)/sizeof(*default_key_table);
}

Fl_Screen_Driver::~Fl_Screen_Driver() {
}


int Fl_Screen_Driver::visual(int) {
  // blank
  return 1;
}


void Fl_Screen_Driver::screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my)
{
  screen_xywh(X, Y, W, H, screen_num(mx, my));
}


void Fl_Screen_Driver::screen_work_area(int &X, int &Y, int &W, int &H, int mx, int my)
{
  screen_work_area(X, Y, W, H, screen_num(mx, my));
}


int Fl_Screen_Driver::screen_count()
{
  if (num_screens < 0)
    init();
  return num_screens ? num_screens : 1;
}


void Fl_Screen_Driver::screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my, int mw, int mh)
{
  screen_xywh(X, Y, W, H, screen_num(mx, my, mw, mh));
}


int Fl_Screen_Driver::screen_num(int x, int y)
{
  int screen = 0;
  if (num_screens < 0) init();

  for (int i = 0; i < num_screens; i ++) {
    int sx, sy, sw, sh;
    screen_xywh(sx, sy, sw, sh, i);
    if ((x >= sx) && (x < (sx+sw)) && (y >= sy) && (y < (sy+sh))) {
      screen = i;
      break;
    }
  }
  return screen;
}


// Return the number of pixels common to the two rectangular areas
float Fl_Screen_Driver::fl_intersection(int x1, int y1, int w1, int h1,
                                    int x2, int y2, int w2, int h2)
{
  if(x1+w1 < x2 || x2+w2 < x1 || y1+h1 < y2 || y2+h2 < y1)
    return 0.;
  int int_left = x1 > x2 ? x1 : x2;
  int int_right = x1+w1 > x2+w2 ? x2+w2 : x1+w1;
  int int_top = y1 > y2 ? y1 : y2;
  int int_bottom = y1+h1 > y2+h2 ? y2+h2 : y1+h1;
  return (float)(int_right - int_left) * (int_bottom - int_top);
}


int Fl_Screen_Driver::screen_num(int x, int y, int w, int h)
{
  int best_screen = 0;
  float best_intersection = 0.;
  if (num_screens < 0) init();
  for (int i = 0; i < num_screens; i++) {
    int sx = 0, sy = 0, sw = 0, sh = 0;
    screen_xywh(sx, sy, sw, sh, i);
    float sintersection = fl_intersection(x, y, w, h, sx, sy, sw, sh);
    if (sintersection > best_intersection) {
      best_screen = i;
      best_intersection = sintersection;
    }
  }
  return best_screen;
}

static void getsyscolor(const char* arg, void (*func)(uchar,uchar,uchar))
{
  if (arg && *arg) {
    uchar r,g,b;
    if (!fl_parse_color(arg, r,g,b))
      Fl::error("Unknown color: %s", arg);
    else
      func(r,g,b);
  }
}

static void set_selection_color(uchar r, uchar g, uchar b)
{
  Fl::set_color(FL_SELECTION_COLOR,r,g,b);
}

void Fl_Screen_Driver::get_system_colors()
{
  if (!bg2_set) getsyscolor(fl_bg2, Fl::background2);
  if (!fg_set)  getsyscolor(fl_fg, Fl::foreground);
  if (!bg_set)  getsyscolor(fl_bg, Fl::background);
  getsyscolor(0, set_selection_color);
}

const char *Fl_Screen_Driver::get_system_scheme()
{
  return fl_getenv("FLTK_SCHEME");
}

/** The bullet character used by default by Fl_Secret_Input */
int Fl_Screen_Driver::secret_input_character = 0x2022;

void Fl_Screen_Driver::compose_reset() {
  Fl::compose_state = 0;
}

void Fl_Screen_Driver::write_image_inside(Fl_RGB_Image *to, Fl_RGB_Image *from, int to_x, int to_y)
/* Copy the image "from" inside image "to" with its top-left angle at coordinates to_x, to_y.
Image depths can differ between "to" and "from".
*/
{
  int to_ld = (to->ld() == 0? to->w() * to->d() : to->ld());
  int from_ld = (from->ld() == 0? from->w() * from->d() : from->ld());
  uchar *tobytes = (uchar*)to->array + to_y * to_ld + to_x * to->d();
  const uchar *frombytes = from->array;
  int need_alpha = (from->d() == 3 && to->d() == 4);
  for (int i = 0; i < from->h(); i++) {
    if (from->d() == to->d()) memcpy(tobytes, frombytes, from->w() * from->d());
    else {
      for (int j = 0; j < from->w(); j++) {
        memcpy(tobytes + j * to->d(), frombytes + j * from->d(), from->d());
        if (need_alpha) *(tobytes + j * to->d() + 3) = 0xff;
      }
    }
    tobytes += to_ld;
    frombytes += from_ld;
  }
}


/* Captures rectangle x,y,w,h from a mapped window or GL window.
 All sub-GL-windows that intersect x,y,w,h, and their subwindows, are also captured.

 Arguments when this function is initially called:
 g: a window or GL window
 x,y,w,h: a rectangle in window g's coordinates
 full_img: NULL

 Arguments when this function recursively calls itself:
 g: an Fl_Group
 x,y,w,h: a rectangle in g's coordinates if g is a window, or in g's parent window coords if g is a group
 full_img: NULL, or a previously captured image that encompasses the x,y,w,h rectangle and that
 will be partially overwritten with the new capture

 Return value:
 An Fl_RGB_Image*, the depth of which is platform-dependent, containing the captured pixels,
 or NULL if capture failed.
 */
Fl_RGB_Image *Fl_Screen_Driver::traverse_to_gl_subwindows(Fl_Group *g, int x, int y, int w, int h,
                                                          Fl_RGB_Image *full_img)
{
  bool captured_subwin = false;
  if ( g->as_gl_window() ) {
    Fl_Device_Plugin *plugin = Fl_Device_Plugin::opengl_plugin();
    if (!plugin) return full_img;
    full_img = plugin->rectangle_capture(g, x, y, w, h);
  }
  else if ( g->as_window() ) {
    full_img = Fl::screen_driver()->read_win_rectangle(x, y, w, h, g->as_window(), true, &captured_subwin);
  }
  if (!full_img) return NULL;
  float full_img_scale =  (full_img && w > 0 ? float(full_img->data_w())/w : 1);
  int n = (captured_subwin ? 0 : g->children());
  for (int i = 0; i < n; i++) {
    Fl_Widget *c = g->child(i);
    if ( !c->visible() || !c->as_group()) continue;
    if ( c->as_window() ) {
      int origin_x = x; // compute intersection of x,y,w,h and the c window
      if (x < c->x()) origin_x = c->x();
      int origin_y = y;
      if (y < c->y()) origin_y = c->y();
      int maxi = x + w; if (maxi > c->x() + c->w()) maxi = c->x() + c->w();
      int width = maxi - origin_x;
      maxi = y + h; if (maxi > c->y() + c->h()) maxi = c->y() + c->h();
      int height = maxi - origin_y;
      if (width > 0 && height > 0) {
        Fl_RGB_Image *img = traverse_to_gl_subwindows(c->as_window(),  origin_x - c->x(),
                                                      origin_y - c->y(), width, height, full_img);
        if (img == full_img) continue;
        write_image_inside(full_img, img, int((origin_x - x) * full_img_scale), int((origin_y - y) * full_img_scale));
        delete img;
      }
    }
    else traverse_to_gl_subwindows(c->as_group(), x, y, w, h, full_img);
  }
  return full_img;
}


int Fl_Screen_Driver::input_widget_handle_key(int key, unsigned mods, unsigned shift, Fl_Input *input)
{
  switch (key) {
    case FL_Delete: {
      int selected = (input->insert_position() != input->mark()) ? 1 : 0;
      if (mods==0 && shift && selected)
        return input->kf_copy_cut();            // Shift-Delete with selection (WP,NP,WOW,GE,KE,OF)
      if (mods==0 && shift && !selected)
        return input->kf_delete_char_right();   // Shift-Delete no selection (WP,NP,WOW,GE,KE,!OF)
      if (mods==0)          return input->kf_delete_char_right();       // Delete         (Standard)
      if (mods==FL_CTRL)    return input->kf_delete_word_right();       // Ctrl-Delete    (WP,!NP,WOW,GE,KE,!OF)
      return 0;                                                 // ignore other combos, pass to parent
    }

    case FL_Left:
      if (mods==0)          return input->kf_move_char_left();          // Left           (WP,NP,WOW,GE,KE,OF)
      if (mods==FL_CTRL)    return input->kf_move_word_left();          // Ctrl-Left      (WP,NP,WOW,GE,KE,!OF)
      if (mods==FL_META)    return input->kf_move_char_left();          // Meta-Left      (WP,NP,?WOW,GE,KE)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_Right:
      if (mods==0)          return input->kf_move_char_right(); // Right          (WP,NP,WOW,GE,KE,OF)
      if (mods==FL_CTRL)    return input->kf_move_word_right(); // Ctrl-Right     (WP,NP,WOW,GE,KE,!OF)
      if (mods==FL_META)    return input->kf_move_char_right(); // Meta-Right     (WP,NP,?WOW,GE,KE,!OF)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_Up:
      if (mods==0)          return input->kf_lines_up(1);               // Up             (WP,NP,WOW,GE,KE,OF)
      if (mods==FL_CTRL)    return input->kf_move_up_and_sol(); // Ctrl-Up        (WP,!NP,WOW,GE,!KE,OF)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_Down:
      if (mods==0)          return input->kf_lines_down(1);             // Dn             (WP,NP,WOW,GE,KE,OF)
      if (mods==FL_CTRL)    return input->kf_move_down_and_eol();       // Ctrl-Down      (WP,!NP,WOW,GE,!KE,OF)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_Page_Up:
      // Fl_Input has no scroll control, so instead we move the cursor by one page
      if (mods==0)          return input->kf_page_up();         // PageUp         (WP,NP,WOW,GE,KE)
      if (mods==FL_CTRL)    return input->kf_page_up();         // Ctrl-PageUp    (!WP,!NP,!WOW,!GE,KE,OF)
      if (mods==FL_ALT)     return input->kf_page_up();         // Alt-PageUp     (!WP,!NP,!WOW,!GE,KE,OF)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_Page_Down:
      if (mods==0)          return input->kf_page_down();               // PageDn         (WP,NP,WOW,GE,KE)
      if (mods==FL_CTRL)    return input->kf_page_down();               // Ctrl-PageDn    (!WP,!NP,!WOW,!GE,KE,OF)
      if (mods==FL_ALT)     return input->kf_page_down();               // Alt-PageDn     (!WP,!NP,!WOW,!GE,KE,OF)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_Home:
      if (mods==0)          return input->kf_move_sol();                // Home           (WP,NP,WOW,GE,KE,OF)
      if (mods==FL_CTRL)    return input->kf_top();                     // Ctrl-Home      (WP,NP,WOW,GE,KE,OF)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_End:
      if (mods==0)          return input->kf_move_eol();                // End            (WP,NP,WOW,GE,KE,OF)
      if (mods==FL_CTRL)    return input->kf_bottom();                  // Ctrl-End       (WP,NP,WOW,GE,KE,OF)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_BackSpace:
      if (mods==0)          return input->kf_delete_char_left();        // Backspace      (WP,NP,WOW,GE,KE,OF)
      if (mods==FL_CTRL)    return input->kf_delete_word_left();        // Ctrl-Backspace (WP,!NP,WOW,GE,KE,!OF)
      return 0;
      // ignore other combos, pass to parent
  }
  return -1;
}


void Fl_Screen_Driver::rescale_all_windows_from_screen(int screen, float f, float old_f)
{
  this->scale(screen, f);
  Fl_Graphics_Driver *d = Fl_Display_Device::display_device()->driver();
  d->scale(f);
  int i = 0, count = 0; // count top-level windows, except transient scale-displaying window
  Fl_Window *win = Fl::first_window();
  while (win) {
    if (!win->parent() && (Fl_Window_Driver::driver(win)->screen_num() == screen) &&
        win->user_data() != &Fl_Screen_Driver::transient_scale_display) {
      count++;
    }
    win = Fl::next_window(win);
  }
  if (count == 0)
    return;
  Fl_Window **win_array = new Fl_Window*[count];
  win = Fl::first_window(); // memorize all top-level windows
  while (win) {
    if (!win->parent() && (Fl_Window_Driver::driver(win)->screen_num() == screen) &&
        win->user_data() != &Fl_Screen_Driver::transient_scale_display) {
      win_array[i++] = win;
    }
    win = Fl::next_window(win);
  }
  for (i = count - 1; i >= 0; i--) { // rescale all top-level windows, finishing with front one
    win = win_array[i];
    Fl_Window_Driver::driver(win)->resize_after_scale_change(screen, old_f, f);
    win->wait_for_expose();
  }
  delete[] win_array;
}


static Fl_Window *transient_scale_window = NULL;
Fl_Window *Fl_Screen_Driver::transient_scale_parent = NULL;


void Fl_Screen_Driver::del_transient_window(void *) {
  transient_scale_parent = NULL;
  delete (Fl_Image*)transient_scale_window->shape();
  delete transient_scale_window;
  transient_scale_window = NULL;
}


void Fl_Screen_Driver::transient_scale_display(float f, int nscreen)
{
  if (!Fl::option(Fl::OPTION_SHOW_SCALING)) return;
  // transiently show the new scaling value using a shaped window
  int w = 150;
  // draw a white rounded box on black background
  Fl_Screen_Driver *d = Fl::screen_driver();
  float s = d->scale(nscreen);
  if (s > 3) s = 3; // limit the growth of the transient window
  Fl_Image_Surface *surf = new Fl_Image_Surface(int(w*s), int(w*s/2));
  Fl_Surface_Device::push_current(surf);
  fl_color(FL_BLACK);
  fl_rectf(-1, -1, int(w*s)+2, int(w*s)+2);
  Fl_Box *b = new Fl_Box(FL_RFLAT_BOX, 0, 0, int(w*s), int(w*s/2), "");
  b->color(FL_WHITE);
  surf->draw(b);
  delete b;
  Fl_RGB_Image* img = surf->image(); // img will be the window's shape
  Fl_Surface_Device::pop_current();
  delete surf;
  //create a window shaped with the rounded box
  int X, Y, W, H;
  Fl::screen_xywh(X, Y, W, H, nscreen);
  w = int(w / (d->scale(nscreen)/s));
  Fl_Window *win = new Fl_Window((X + W/2) -w/2, (Y + H/2) -w/4, w, w/2, 0);
  b = new Fl_Box(FL_FLAT_BOX, 0, 0, w, w/2, NULL);
  char str[10];
  snprintf(str, 10, "%d %%", int(f * 100 + 0.5));
  b->copy_label(str);
  b->labelfont(FL_TIMES_BOLD);
  b->labelsize(Fl_Fontsize(30 * s / d->scale(nscreen)));
  b->labelcolor(Fl_Tooltip::textcolor());
  b->color(Fl_Tooltip::color());
  win->end();
  win->shape(img);
  win->user_data((void*)&transient_scale_display); // prevent this window from being rescaled later
  win->set_output();
  win->set_non_modal();
  Fl_Window_Driver::driver(win)->screen_num(nscreen);
  Fl_Window_Driver::driver(win)->force_position(1);
  if (transient_scale_window) {
    Fl::remove_timeout(del_transient_window);
    del_transient_window(NULL);
  }
  transient_scale_window = win;
  win->show();
  // delete transient win after 1 sec
  Fl::add_timeout(1, del_transient_window, NULL);
}

// respond to Ctrl-'+' and Ctrl-'-' and Ctrl-'0' (Ctrl-'=' is same as Ctrl-'+') by rescaling all windows
int Fl_Screen_Driver::scale_handler(int event)
{
  if (!keyboard_screen_scaling) return 0;
  if ( event != FL_SHORTCUT || !Fl::event_command() ) return 0;
  enum {none, zoom_in, zoom_out, zoom_reset} zoom = none;
  if (Fl::test_shortcut(FL_COMMAND+'+')) zoom = zoom_in;
  else if (Fl::test_shortcut(FL_COMMAND+'-')) zoom = zoom_out;
  else if (Fl::test_shortcut(FL_COMMAND+'0')) zoom = zoom_reset;

  // Kludge to recognize shortcut FL_COMMAND+'+' without pressing SHIFT.
  if (Fl::option(Fl::OPTION_SIMPLE_ZOOM_SHORTCUT)) {
    if ((Fl::event_state() & (FL_META|FL_ALT|FL_CTRL|FL_SHIFT)) == FL_COMMAND) {
      // We use Ctrl + key '=|+' for instance on US, UK, and FR keyboards.
      // This works as expected on all keyboard layouts that have the '=' key in the
      // lower and the '+' key in the upper position on the same key.
      // This test would be "false positive" if a keyboard layout had the '=' key in
      // the lower and any other key than '+' in the upper position!

      if (Fl::event_key() == '=') zoom = zoom_in;

      // Note: Fl::event_key() is often incorrect under X11 *if* the selected keyboard
      // layout is *not* the primary one in the keyboard selection list, e.g. under Gnome.
      // The observation is that Fl::event_key() is erroneously derived from the primary
      // keyboard layout instead. This can be very confusing and I don't know why this
      // happens. Albrecht-S, Oct 2024, on Debian 12 (Bookworm aka Stable as of now).

      // Example: 0xfe51 ("dead_acute") is sent by the '=' key of the US layout if the
      // primary layout is German. This *would* be the correct key value for the German
      // keyboard layout but not for the US layout.
      // The following statement would work around this for this very special case but
      // this should IMHO not be done. A valid workaround by the user is to make the
      // *used* layout the first in the keyboard layout selection list!

      // else if (Fl::event_key() == 0xfe51) zoom = zoom_in; // dead_acute, see above
    }
  }
  if (zoom != none) {
    int i, count;
    if (Fl::grab()) return 0; // don't rescale when menu windows are on
    Fl_Widget *wid = Fl::focus();
    if (!wid) return 0;
    Fl_Window *top = wid->top_window();
    int screen = Fl_Window_Driver::driver(top)->screen_num();
    Fl_Screen_Driver *screen_dr = Fl::screen_driver();
    // don't rescale when any top window on same screen as
    // focus window is fullscreen or maximized
    top = Fl::first_window();
    while (top) {
      if (!top->parent() &&
          (Fl_Window_Driver::driver(top)->screen_num() == screen ||
           screen_dr->rescalable() == SYSTEMWIDE_APP_SCALING)) {
        if (top->fullscreen_active() || top->maximize_active()) return 0;
      }
      top = Fl::next_window(top);
    }
    float initial_scale = screen_dr->base_scale(screen);
#if defined(TEST_SCALING)
    // test scaling factors: lots of values from 0.3 to 8.0
    static float scaling_values[] = {
      0.30000, 0.33661, 0.37768, 0.42376, 0.47547,
      0.50000, 0.53348, 0.59858, 0.67162, 0.75357,
      0.84551, 0.94868, 1.00000, 1.06444, 1.19432,
      1.34005, 1.50000, 1.68702, 1.89287, 2.00000,
      2.12384, 7.0f/3,  2.38298, 8.0f/3,  3.00000,
      10.0f/3, 3.75000, 4.00000, 13.0f/3, 14.0f/3,
      5.00000, 16.0f/3, 17.0f/3, 6.00000, 7.00000,
      8.00000};
#else
    // scaling factors for the standard GUI
    static float scaling_values[] = {
      0.5f, 2.f/3, 0.8f, 0.9f, 1.0f,
      1.1f, 1.2f, 4.f/3, 1.5f, 1.7f,
      2.0f, 2.4f, 3.0f};
#endif
    float f, old_f = screen_dr->scale(screen)/initial_scale;
    if (zoom == zoom_reset) f = 1;
    else {
      count = sizeof(scaling_values)/sizeof(float);
      for (i = 0; i < count; i++) {
        if (old_f >= scaling_values[i] - 1e-4 && (i+1 >= count || old_f < scaling_values[i+1] - 1e-4)) {
          break;
        }
      }
      if (zoom == zoom_out) i--; else i++;
      if (i < 0) i = 0;
      else if (i >= count) i = count - 1;
      f = scaling_values[i];
    }
    if (f == old_f) return 1;
    if (screen_dr->rescalable() == SYSTEMWIDE_APP_SCALING) {
      float old_f = screen_dr->scale(0);
      for (int i = 0; i < Fl::screen_count(); i++) {
        screen_dr->rescale_all_windows_from_screen(i, f * initial_scale, old_f);
      }
    } else {
      screen_dr->rescale_all_windows_from_screen(screen, f * initial_scale, screen_dr->scale(screen));
    }
    Fl_Screen_Driver::transient_scale_display(f, screen);
    Fl::handle(FL_ZOOM_EVENT, NULL);
    return 1;
  }
  return 0;
}


// use the startup time scaling value
void Fl_Screen_Driver::use_startup_scale_factor()
{
  char *p;
  int s_count = screen_count();
  desktop_scale_factor();
  if ((p = fl_getenv("FLTK_SCALING_FACTOR"))) {
    float factor = 1;
    sscanf(p, "%f", &factor);
    if (rescalable() == SYSTEMWIDE_APP_SCALING) {
      float new_val = factor * scale(0);
      for (int i = 0; i < s_count; i++)  scale(i, new_val);
    } else {
      for (int i = 0; i < s_count; i++)  scale(i, factor * scale(i));
    }
  }
}


void Fl_Screen_Driver::open_display()
{
  static bool been_here = false;
  if (!been_here) {
    been_here = true;
    open_display_platform();
    // Memorize the most recently added handler. It may have been
    // added by open_display_platform()
    Fl_Event_Handler last_added = Fl::last_handler();
    if (rescalable()) {
      use_startup_scale_factor();
      if (keyboard_screen_scaling && rescalable()) {
        // Add scale_handler after memorized one in linked list
        // so it has less priority
        Fl::add_handler(Fl_Screen_Driver::scale_handler, last_added);
      }
      int mx, my;
      int ns = Fl::screen_driver()->get_mouse(mx, my);
      Fl_Graphics_Driver::default_driver().scale(scale(ns));
    }
  }
}


// simulation of XParseColor:
int Fl_Screen_Driver::parse_color(const char* p, uchar& r, uchar& g, uchar& b)
{
  if (*p == '#') p++;
  size_t n = strlen(p);
  size_t m = n/3;
  const char *pattern = 0;
  switch(m) {
    case 1: pattern = "%1x%1x%1x"; break;
    case 2: pattern = "%2x%2x%2x"; break;
    case 3: pattern = "%3x%3x%3x"; break;
    case 4: pattern = "%4x%4x%4x"; break;
    default: return 0;
  }
  int R,G,B; if (sscanf(p,pattern,&R,&G,&B) != 3) return 0;
  switch(m) {
    case 1: R *= 0x11; G *= 0x11; B *= 0x11; break;
    case 3: R >>= 4; G >>= 4; B >>= 4; break;
    case 4: R >>= 8; G >>= 8; B >>= 8; break;
  }
  r = (uchar)R; g = (uchar)G; b = (uchar)B;
  return 1;
}

void Fl_Screen_Driver::default_icons(const Fl_RGB_Image *icons[], int count) {}


/** see fl_set_spot() */
void Fl_Screen_Driver::set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win)
{}

void Fl_Screen_Driver::set_status(int X, int Y, int W, int H) {}


/** see fl_reset_spot() */
void Fl_Screen_Driver::reset_spot() {}


/* the following function was stolen from the X sources as indicated. */

/* Copyright    Massachusetts Institute of Technology  1985, 1986, 1987 */
/* $XConsortium: XParseGeom.c,v 11.18 91/02/21 17:23:05 rws Exp $ */

/*
 Permission to use, copy, modify, distribute, and sell this software and its
 documentation for any purpose is hereby granted without fee, provided that
 the above copyright notice appear in all copies and that both that
 copyright notice and this permission notice appear in supporting
 documentation, and that the name of M.I.T. not be used in advertising or
 publicity pertaining to distribution of the software without specific,
 written prior permission.  M.I.T. makes no representations about the
 suitability of this software for any purpose.  It is provided "as is"
 without express or implied warranty.
 */

/*
    XParseGeometry parses strings of the form
   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
   width, height, xoffset, and yoffset are unsigned integers.
   Example:  "=80x24+300-49"
   The equal sign is optional.
   It returns a bitmask that indicates which of the four values
   were actually found in the string.  For each value found,
   the corresponding argument is updated;  for each value
   not found, the corresponding argument is left unchanged.
 */

static int ReadInteger(char* string, char** NextString)
{
  int Result = 0;
  int Sign = 1;

  if (*string == '+')
    string++;
  else if (*string == '-') {
    string++;
    Sign = -1;
  }
  for (; (*string >= '0') && (*string <= '9'); string++) {
    Result = (Result * 10) + (*string - '0');
  }
  *NextString = string;
  if (Sign >= 0)
    return (Result);
  else
    return (-Result);
}


int Fl_Screen_Driver::XParseGeometry(const char* string, int* x, int* y,
                   unsigned int* width, unsigned int* height)
{
  int mask = Fl_Screen_Driver::fl_NoValue;
  char *strind;
  unsigned int tempWidth = 0, tempHeight = 0;
  int tempX = 0, tempY = 0;
  char *nextCharacter;

  if ( (string == NULL) || (*string == '\0')) return(mask);
  if (*string == '=')
    string++;  /* ignore possible '=' at beg of geometry spec */

  strind = (char *)string;
  if (*strind != '+' && *strind != '-' && *strind != 'x') {
    tempWidth = ReadInteger(strind, &nextCharacter);
    if (strind == nextCharacter)
      return (0);
    strind = nextCharacter;
    mask |= fl_WidthValue;
  }

  if (*strind == 'x' || *strind == 'X') {
    strind++;
    tempHeight = ReadInteger(strind, &nextCharacter);
    if (strind == nextCharacter)
      return (0);
    strind = nextCharacter;
    mask |= fl_HeightValue;
  }

  if ((*strind == '+') || (*strind == '-')) {
    if (*strind == '-') {
      strind++;
      tempX = -ReadInteger(strind, &nextCharacter);
      if (strind == nextCharacter)
        return (0);
      strind = nextCharacter;
      mask |= fl_XNegative;

    } else {
      strind++;
      tempX = ReadInteger(strind, &nextCharacter);
      if (strind == nextCharacter)
        return(0);
      strind = nextCharacter;
    }
    mask |= fl_XValue;
    if ((*strind == '+') || (*strind == '-')) {
      if (*strind == '-') {
        strind++;
        tempY = -ReadInteger(strind, &nextCharacter);
        if (strind == nextCharacter)
          return(0);
        strind = nextCharacter;
        mask |= fl_YNegative;

      } else {
        strind++;
        tempY = ReadInteger(strind, &nextCharacter);
        if (strind == nextCharacter)
          return(0);
        strind = nextCharacter;
      }
      mask |= fl_YValue;
    }
  }

  /* If strind isn't at the end of the string the it's an invalid
   geometry specification. */

  if (*strind != '\0') return (0);

  if (mask & fl_XValue)
    *x = tempX;
  if (mask & fl_YValue)
    *y = tempY;
  if (mask & fl_WidthValue)
    *width = tempWidth;
  if (mask & fl_HeightValue)
    *height = tempHeight;
  return (mask);
}


// turn '\r' characters into '\n' and "\r\n" sequences into '\n'
// returns new length
size_t Fl_Screen_Driver::convert_crlf(char *s, size_t len) {
  char *src = (char *)memchr(s, '\r', len); // find first `\r` in buffer
  if (src) {
    char *dst = src;
    char *end = s + len;
    while (src < end) {
      if (*src == '\r') {
        if (src + 1 < end && *(src + 1) == '\n') {
          src++; // skip '\r'
          continue;
        } else {
          *dst++ = '\n'; // replace single '\r' with '\n'
        }
      } else {
        *dst++ = *src;
      }
      src++;
    }
    return (dst - s);
  }
  return len;
}


float Fl_Screen_Driver::base_scale(int numscreen) {
  static float base = scale(numscreen);
  return base;
}


/**
 \}
 \endcond
 */
