//
// "$Id$"
//
// All screen related calls in a driver style class.
//
// Copyright 1998-2017 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl_Screen_Driver.H>
#include <FL/Fl_Image.H>
#include <FL/Fl.H>
#include <FL/x.H> // for fl_window
#include <FL/Fl_Plugin.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Window_Driver.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tooltip.H>

char Fl_Screen_Driver::bg_set = 0;
char Fl_Screen_Driver::bg2_set = 0;
char Fl_Screen_Driver::fg_set = 0;


Fl_Screen_Driver::Fl_Screen_Driver() :
num_screens(-1), text_editor_extra_key_bindings(NULL)
{
}

Fl_Screen_Driver::~Fl_Screen_Driver() {
}


void Fl_Screen_Driver::display(const char *) {
  // blank
}


int Fl_Screen_Driver::visual(int) {
  // blank
  return 1;
}


void Fl_Screen_Driver::screen_xywh(int &X, int &Y, int &W, int &H)
{
  int x, y;
  get_mouse(x, y);
  screen_xywh(X, Y, W, H, x, y);
}


void Fl_Screen_Driver::screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my)
{
  screen_xywh(X, Y, W, H, screen_num(mx, my));
}


void Fl_Screen_Driver::screen_work_area(int &X, int &Y, int &W, int &H)
{
  int x, y;
  get_mouse(x, y);
  screen_work_area(X, Y, W, H, x, y);
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
    Fl::screen_xywh(sx, sy, sw, sh, i);
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


const char *Fl_Screen_Driver::get_system_scheme()
{
  return 0L;
}

/** The bullet character used by default by Fl_Secret_Input */
int Fl_Screen_Driver::secret_input_character = 0x2022;

void Fl_Screen_Driver::compose_reset() {
  Fl::compose_state = 0;
}

uchar *Fl_Screen_Driver::read_image(uchar *p, int X, int Y, int w, int h, int alpha) {
  uchar *image_data = NULL;
  Fl_RGB_Image *img;
  if (fl_find(fl_window) == 0) { // read from off_screen buffer
    img = read_win_rectangle(p, X, Y, w, h, alpha);
    if (!img) {
      return NULL;
    }
    img->alloc_array = 1;
  } else {
    img = traverse_to_gl_subwindows(Fl_Window::current(), p, X, Y, w, h, alpha, NULL);
  }
  if (img) {
    if (img->w() > w) {
      Fl_RGB_Image *img2 = (Fl_RGB_Image*)img->copy(w, h);
      delete img;
      img = img2;
    }
    img->alloc_array = 0;
    image_data = (uchar*)img->array;
    delete img;
  }
  return image_data;
}

void Fl_Screen_Driver::write_image_inside(Fl_RGB_Image *to, Fl_RGB_Image *from, int to_x, int to_y)
/* Copy the image "from" inside image "to" with its top-left angle at coordinates to_x, to_y.
Image depth can differ between "to" and "from".
*/
{
  int to_ld = (to->ld() == 0? to->w() * to->d() : to->ld());
  int from_ld = (from->ld() == 0? from->w() * from->d() : from->ld());
  uchar *tobytes = (uchar*)to->array + to_y * to_ld + to_x * to->d();
  const uchar *frombytes = from->array;
  for (int i = 0; i < from->h(); i++) {
    if (from->d() == to->d()) memcpy(tobytes, frombytes, from->w() * from->d());
    else {
      for (int j = 0; j < from->w(); j++) {
        memcpy(tobytes + j * to->d(), frombytes + j * from->d(), from->d());
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
 p: as in fl_read_image()
 x,y,w,h: a rectangle in window g's coordinates
 alpha: as in fl_read_image()
 full_img: NULL
 
 Arguments when this function recursively calls itself:
 g: an Fl_Group
 p: as above
 x,y,w,h: a rectangle in g's coordinates if g is a window, or in g's parent window coords if g is a group
 alpha: as above
 full_img: NULL, or a previously captured image that encompasses the x,y,w,h rectangle and that
 will be partially overwritten with the new capture
 
 Return value:
 An Fl_RGB_Image* of depth 4 if alpha>0 or 3 if alpha = 0 containing the captured pixels.
 */
Fl_RGB_Image *Fl_Screen_Driver::traverse_to_gl_subwindows(Fl_Group *g, uchar *p, int x, int y, int w, int h, int alpha,
                                                          Fl_RGB_Image *full_img)
{
  if ( g->as_gl_window() ) {
    Fl_Plugin_Manager pm("fltk:device");
    Fl_Device_Plugin *pi = (Fl_Device_Plugin*)pm.plugin("opengl.device.fltk.org");
    if (!pi) return full_img;
    Fl_RGB_Image *img = pi->rectangle_capture(g, x, y, w, h);
    if (full_img) full_img = img;
    else {
      uchar *data = ( p ? p : new uchar[img->w() * img->h() * (alpha?4:3)] );
      full_img = new Fl_RGB_Image(data, img->w(), img->h(), alpha?4:3);
      if (!p) full_img->alloc_array = 1;
      if (alpha) memset(data, alpha, img->w() * img->h() * 4);
      write_image_inside(full_img, img, 0, 0);
      delete img;
    }
  }
  else if ( g->as_window() && (!full_img || (g->window() && g->window()->as_gl_window())) ) {
    // the starting window or one inside a GL window
    if (full_img) g->as_window()->make_current();
    int alloc_img = (full_img != NULL || p == NULL); // false means use p, don't alloc new memory for image
    full_img = Fl::screen_driver()->read_win_rectangle( (alloc_img ? NULL : p), x, y, w, h, alpha);
  }
  int n = g->children();
  for (int i = 0; i < n; i++) {
    Fl_Widget *c = g->child(i);
    if ( !c->visible() || !c->as_group()) continue;
    if ( c->as_window() ) {
      int origin_x = x; // compute intersection of x,y,w,h and the c window
      if (x < c->x()) origin_x = c->x();
      int origin_y = y;
      if (y < c->y()) origin_y = c->y();
      int width = c->w();
      if (origin_x + width > c->x() + c->w()) width = c->x() + c->w() - origin_x;
      if (origin_x + width > x + w) width = x + w - origin_x;
      int height = c->w();
      if (origin_y + height > c->y() + c->h()) height = c->y() + c->h() - origin_y;
      if (origin_y + height > y + h) height = y + h - origin_y;
      if (width > 0 && height > 0) {
        Fl_RGB_Image *img = traverse_to_gl_subwindows(c->as_window(), p, origin_x - c->x(),
                                                      origin_y - c->y(), width, height, alpha, full_img);
        if (img == full_img) continue;
        int top;
        if (c->as_gl_window()) {
          top = origin_y - y;
        } else {
          top = full_img->h() - (origin_y - y + img->h());
        }
        int nscreen = c->as_window()->driver()->screen_num();
        float s = Fl::screen_driver()->scale(nscreen);
        write_image_inside(full_img, img, (origin_x - x) * s, top * s);
        delete img;
      }
    }
    else traverse_to_gl_subwindows(c->as_group(), p, x, y, w, h, alpha, full_img);
  }
  return full_img;
}


int Fl_Screen_Driver::input_widget_handle_key(int key, unsigned mods, unsigned shift, Fl_Input *input)
{
  switch (key) {
    case FL_Delete: {
      int selected = (input->position() != input->mark()) ? 1 : 0;
      if (mods==0 && shift && selected)
        return input->kf_copy_cut();		// Shift-Delete with selection (WP,NP,WOW,GE,KE,OF)
      if (mods==0 && shift && !selected)
        return input->kf_delete_char_right();	// Shift-Delete no selection (WP,NP,WOW,GE,KE,!OF)
      if (mods==0)          return input->kf_delete_char_right();	// Delete         (Standard)
      if (mods==FL_CTRL)    return input->kf_delete_word_right();	// Ctrl-Delete    (WP,!NP,WOW,GE,KE,!OF)
      return 0;							// ignore other combos, pass to parent
    }
      
    case FL_Left:
      if (mods==0)          return input->kf_move_char_left();		// Left           (WP,NP,WOW,GE,KE,OF)
      if (mods==FL_CTRL)    return input->kf_move_word_left();		// Ctrl-Left      (WP,NP,WOW,GE,KE,!OF)
      if (mods==FL_META)    return input->kf_move_char_left();		// Meta-Left      (WP,NP,?WOW,GE,KE)
      return 0;							// ignore other combos, pass to parent
      
    case FL_Right:
      if (mods==0)          return input->kf_move_char_right();	// Right          (WP,NP,WOW,GE,KE,OF)
      if (mods==FL_CTRL)    return input->kf_move_word_right();	// Ctrl-Right     (WP,NP,WOW,GE,KE,!OF)
      if (mods==FL_META)    return input->kf_move_char_right();	// Meta-Right     (WP,NP,?WOW,GE,KE,!OF)
      return 0;							// ignore other combos, pass to parent
      
    case FL_Up:
      if (mods==0)          return input->kf_lines_up(1);		// Up             (WP,NP,WOW,GE,KE,OF)
      if (mods==FL_CTRL)    return input->kf_move_up_and_sol();	// Ctrl-Up        (WP,!NP,WOW,GE,!KE,OF)
      return 0;							// ignore other combos, pass to parent
      
    case FL_Down:
      if (mods==0)          return input->kf_lines_down(1);		// Dn             (WP,NP,WOW,GE,KE,OF)
      if (mods==FL_CTRL)    return input->kf_move_down_and_eol();	// Ctrl-Down      (WP,!NP,WOW,GE,!KE,OF)
      return 0;							// ignore other combos, pass to parent
      
    case FL_Page_Up:
      // Fl_Input has no scroll control, so instead we move the cursor by one page
      if (mods==0)          return input->kf_page_up();		// PageUp         (WP,NP,WOW,GE,KE)
      if (mods==FL_CTRL)    return input->kf_page_up();		// Ctrl-PageUp    (!WP,!NP,!WOW,!GE,KE,OF)
      if (mods==FL_ALT)     return input->kf_page_up();		// Alt-PageUp     (!WP,!NP,!WOW,!GE,KE,OF)
      return 0;							// ignore other combos, pass to parent
      
    case FL_Page_Down:
      if (mods==0)          return input->kf_page_down();		// PageDn         (WP,NP,WOW,GE,KE)
      if (mods==FL_CTRL)    return input->kf_page_down();		// Ctrl-PageDn    (!WP,!NP,!WOW,!GE,KE,OF)
      if (mods==FL_ALT)     return input->kf_page_down();		// Alt-PageDn     (!WP,!NP,!WOW,!GE,KE,OF)
      return 0;							// ignore other combos, pass to parent
      
    case FL_Home:
      if (mods==0)          return input->kf_move_sol();		// Home           (WP,NP,WOW,GE,KE,OF)
      if (mods==FL_CTRL)    return input->kf_top();			// Ctrl-Home      (WP,NP,WOW,GE,KE,OF)
      return 0;							// ignore other combos, pass to parent
      
    case FL_End:
      if (mods==0)          return input->kf_move_eol();		// End            (WP,NP,WOW,GE,KE,OF)
      if (mods==FL_CTRL)    return input->kf_bottom();			// Ctrl-End       (WP,NP,WOW,GE,KE,OF)
      return 0;							// ignore other combos, pass to parent
      
    case FL_BackSpace:
      if (mods==0)          return input->kf_delete_char_left();	// Backspace      (WP,NP,WOW,GE,KE,OF)
      if (mods==FL_CTRL)    return input->kf_delete_word_left();	// Ctrl-Backspace (WP,!NP,WOW,GE,KE,!OF)
      return 0;
      // ignore other combos, pass to parent
  }
  return -1;
}


#if defined(FLTK_HIDPI_SUPPORT) || !(defined(WIN32) || defined(__APPLE__))

void Fl_Screen_Driver::rescale_all_windows_from_screen(int screen, float f)
{
  float old_f = this->scale(screen);
  if (f == old_f) return;
  this->scale(screen, f);
  Fl_Graphics_Driver *d = Fl_Display_Device::display_device()->driver();
  d->scale(f);
  int i = 0, count = 0; // count top-level windows, except transient scale-displaying window
  Fl_Window *win = Fl::first_window();
  while (win) {
    if (!win->parent() && (win->driver()->screen_num() == screen || rescalable() == SYSTEMWIDE_APP_SCALING) &&
        win->user_data() != &Fl_Screen_Driver::transient_scale_display) count++;
    win = Fl::next_window(win);
  }
  Fl_Window **win_array = new Fl_Window*[count];
  win = Fl::first_window(); // memorize all top-level windows
  while (win) {
    if (!win->parent() && win->user_data() != &Fl_Screen_Driver::transient_scale_display &&
        (win->driver()->screen_num() == screen  || rescalable() == SYSTEMWIDE_APP_SCALING) ) {
      win_array[i++] = win;
    }
    win = Fl::next_window(win);
  }
  for (i = count - 1; i >= 0; i--) { // rescale all top-level windows, finishing with front one
    win = win_array[i];
    int oldx = win->x(), oldy = win->y();
    win->hide();
    win->driver()->screen_num(screen);
    win->position(oldx*old_f/f, oldy*old_f/f);
    win->driver()->force_position(1);
    if (win->fullscreen_active()) {
      win->size(win->w() * old_f/f, win->h() * old_f/f);
    }
    win->show();
    win->wait_for_expose();
  }
  delete[] win_array;
}

static void del_transient_window(void *data) {
  Fl_Window *win = (Fl_Window*)data;
  delete (Fl_RGB_Image*)win->child(0)->user_data();
  Fl::delete_widget(win);
}

void Fl_Screen_Driver::transient_scale_display(float f, int nscreen)
{ // transiently show the new scaling value using a shaped window
  int w = 150;
  // draw a white rounded box on black background
  Fl_Screen_Driver *d = Fl::screen_driver();
  float s = d->scale(nscreen);
  if (s > 3) s = 3; // limit the growth of the transient window
  Fl_Image_Surface *surf = new Fl_Image_Surface(w*s, w*s/2);
  Fl_Surface_Device::push_current(surf);
  fl_color(FL_BLACK);
  fl_rectf(-1, -1, w*s+2, w*s+2);
  Fl_Box *b = new Fl_Box(FL_RFLAT_BOX, 0, 0, w*s, w*s/2, "");
  b->color(FL_WHITE);
  surf->draw(b);
  delete b;
  Fl_RGB_Image* img = surf->image(); // img will be the window's shape
  Fl_Surface_Device::pop_current();
  delete surf;
  //create a window shaped with the rounded box
  int X, Y, W, H;
  Fl::screen_xywh(X, Y, W, H, nscreen);
  w /= d->scale(nscreen)/s;
  Fl_Window *win = new Fl_Window((X + W/2) -w/2, (Y + H/2) -w/4, w, w/2, 0);
  b = new Fl_Box(FL_FLAT_BOX, 0, 0, w, w/2, NULL);
  char str[10];
  sprintf(str, "%d %%", int(f * 100 + 0.5));
  b->copy_label(str);
  b->labelfont(FL_TIMES_BOLD);
  b->labelsize(30 * s / d->scale(nscreen));
  b->labelcolor(FL_BLACK);
  b->color(Fl_Tooltip::color());
  win->end();
  win->shape(img);
  b->user_data(img);
  win->user_data((void*)&transient_scale_display); // prevent this window from being rescaled later
  win->set_output();
  win->set_non_modal();
  win->driver()->screen_num(nscreen);
  win->driver()->force_position(1);
  win->show();
  Fl::add_timeout(1, del_transient_window, win); // delete after 1 sec
}

// respond to Ctrl-'+' and Ctrl-'-' and Ctrl-'0' (Ctrl-'=' is same as Ctrl-'+') by rescaling all windows
int Fl_Screen_Driver::scale_handler(int event)
{
  if ( event != FL_SHORTCUT || (!Fl::event_command()) ) return 0;
  int key = Fl::event_key() & ~(FL_SHIFT+FL_COMMAND);
  if (key == '=' || key == '-' || key == '+' || key == '0' || key == 0xE0/* for '0' on Fr keyboard */) {
    int i, count;
    if (Fl::grab()) return 0; // don't rescale when menu windows are on
    Fl_Widget *wid = Fl::focus();
    if (!wid) return 0;
    int screen = wid->top_window()->driver()->screen_num();
    Fl_Screen_Driver *screen_dr = Fl::screen_driver();
    static float initial_scale = screen_dr->scale(screen);
    static float scaling_values[] = {0.5, 2.f/3, 0.8, 0.9, 1, 1.1, 1.2, 4.f/3, 1.5, 1.7, 2, 2.4, 3,
      4, 5.5, 6.5, 7.5, 9.75 };
    float f, old_f = screen_dr->scale(screen)/initial_scale;
    if (key == '0' || key == 0xE0) f = 1;
    else {
      count = sizeof(scaling_values)/sizeof(float);
      for (i = 0; i < count; i++) {
        if (old_f >= scaling_values[i] - 1e-4 && (i+1 >= count || old_f < scaling_values[i+1] - 1e-4)) {
          break;
        }
      }
      if (key == '-') i--; else i++;
      if (i < 0) i = 0; if (i >= count) i = count - 1;
      f = scaling_values[i];
    }
    if (f == old_f) return 1;
    screen_dr->rescale_all_windows_from_screen(screen, f*initial_scale);
    Fl_Screen_Driver::transient_scale_display(f, screen);
    screen_dr->init_workarea();
    return 1;
  }
  return 0;
}

#endif // FLTK_HIDPI_SUPPORT


// determine the scaling value used at startup time (helps supporting HiDPI displays)
float Fl_Screen_Driver::default_scale_factor()
{
  float factor = 1;
#if defined(FLTK_HIDPI_SUPPORT) || !(defined(WIN32) || defined(__APPLE__))
  char *p = 0;
  if ((p = getenv("FLTK_SCALING_FACTOR"))) {
    sscanf(p, "%f", &factor);
  }
  else {
    factor = desktop_scale_factor();
  }
#endif // FLTK_HIDPI_SUPPORT
  return factor;
}


void Fl_Screen_Driver::open_display()
{
  open_display_platform();
  static bool been_here = false;
  if (!been_here) {
    been_here = true;
    if (rescalable()) {
      float factor = default_scale_factor();
      for (int i = 0; i < screen_count(); i++) scale(i, factor);
#if defined(FLTK_HIDPI_SUPPORT) || !(defined(WIN32) || defined(__APPLE__))
      Fl::add_handler(Fl_Screen_Driver::scale_handler);
#endif
      Fl_Graphics_Driver::default_driver().scale(factor);
    }
  }
}

//
// End of "$Id$".
//
