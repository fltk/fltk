//
// Definition of Apple Cocoa Screen interface.
//
// Copyright 1998-2023 by Bill Spitzak and others.
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


#include <config.h>
#include "Fl_Cocoa_Screen_Driver.H"
#include "Fl_Cocoa_Window_Driver.H"
#include "../Quartz/Fl_Font.H"
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/Fl_Graphics_Driver.H>
#include <FL/Fl_Input.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Image_Surface.H>
#include <stdio.h>


extern "C" void NSBeep(void);
extern void (*fl_lock_function)();
extern void (*fl_unlock_function)();

int Fl_Cocoa_Screen_Driver::next_marked_length = 0;


// This key table is used for the Darwin system driver. It is defined here
// "static" and assigned in the constructor to avoid static initialization
// race conditions. It is used in fl_shortcut.cxx.
//
// This table must be in numeric order by fltk (X) keysym number:

Fl_Screen_Driver::Keyname darwin_key_table[] = {
  //              v - this column may contain UTF-8 characters
  {' ',           "Space"},
  {FL_BackSpace,  "⌫"/*"\xe2\x8c\xab"*/}, // U+232B : erase to the left
  {FL_Tab,        "⇥"/*"\xe2\x87\xa5"*/}, // U+21E5 rightwards arrow to bar
  {FL_Enter,      "↩"/*"\xe2\x86\xa9"*/}, // U+21A9 leftwards arrow with hook
  {FL_Pause,      "Pause"},
  {FL_Scroll_Lock, "Scroll_Lock"},
  {FL_Escape,     "⎋"/*"\xe2\x8e\x8b"*/}, // U+238B : broken circle with northwest arrow
  {FL_Home,       "↖"/*"\xe2\x86\x96"*/}, // U+2196 north west arrow
  {FL_Left,       "←"/*"\xe2\x86\x90"*/}, // U+2190 leftwards arrow
  {FL_Up,         "↑"/*"\xe2\x86\x91"*/}, // U+2191 upwards arrow
  {FL_Right,      "→"/*"\xe2\x86\x92"*/}, // U+2192 rightwards arrow
  {FL_Down,       "↓"/*"\xe2\x86\x93"*/}, // U+2193 downwards arrow
  {FL_Page_Up,    "⇞"/*"\xe2\x87\x9e"*/}, // U+21DE upwards arrow with double stroke
  {FL_Page_Down,  "⇟"/*"\xe2\x87\x9f"*/}, //  U+21DF downwards arrow with double stroke
  {FL_End,        "↘"/*"\xe2\x86\x98"*/}, // U+2198 south east arrow
  {FL_Print,      "Print"},
  {FL_Insert,     "Insert"},
  {FL_Menu,       "Menu"},
  {FL_Num_Lock,   "Num_Lock"},
  {FL_KP_Enter,   "⌤"/*"\xe2\x8c\xa4"*/}, // U+2324 up arrow head between two horizontal bars
  {FL_Shift_L,    "Shift_L"},
  {FL_Shift_R,    "Shift_R"},
  {FL_Control_L,  "Control_L"},
  {FL_Control_R,  "Control_R"},
  {FL_Caps_Lock,  "⇪"/*"\xe2\x87\xaa"*/}, // U+21EA upwards white arrow from bar
  {FL_Meta_L,     "Meta_L"},
  {FL_Meta_R,     "Meta_R"},
  {FL_Alt_L,      "Alt_L"},
  {FL_Alt_R,      "Alt_R"},
  {FL_Delete,     "⌦"/*"\xe2\x8c\xa6"*/}  // U+2326 : erase to the right
};


static Fl_Text_Editor::Key_Binding extra_bindings[] =  {
  // Define CMD+key accelerators...
  { 'z',          FL_COMMAND,               Fl_Text_Editor::kf_undo       ,0},
  { 'z',          FL_COMMAND|FL_SHIFT,      Fl_Text_Editor::kf_redo       ,0},
  { 'x',          FL_COMMAND,               Fl_Text_Editor::kf_cut        ,0},
  { 'c',          FL_COMMAND,               Fl_Text_Editor::kf_copy       ,0},
  { 'v',          FL_COMMAND,               Fl_Text_Editor::kf_paste      ,0},
  { 'a',          FL_COMMAND,               Fl_Text_Editor::kf_select_all ,0},
  { FL_Left,      FL_COMMAND,               Fl_Text_Editor::kf_meta_move  ,0},
  { FL_Right,     FL_COMMAND,               Fl_Text_Editor::kf_meta_move  ,0},
  { FL_Up,        FL_COMMAND,               Fl_Text_Editor::kf_meta_move  ,0},
  { FL_Down,      FL_COMMAND,               Fl_Text_Editor::kf_meta_move  ,0},
  { FL_Left,      FL_COMMAND|FL_SHIFT,      Fl_Text_Editor::kf_m_s_move   ,0},
  { FL_Right,     FL_COMMAND|FL_SHIFT,      Fl_Text_Editor::kf_m_s_move   ,0},
  { FL_Up,        FL_COMMAND|FL_SHIFT,      Fl_Text_Editor::kf_m_s_move   ,0},
  { FL_Down,      FL_COMMAND|FL_SHIFT,      Fl_Text_Editor::kf_m_s_move   ,0},
  { 0,            0,                        0                             ,0}
};


Fl_Cocoa_Screen_Driver::Fl_Cocoa_Screen_Driver() {
  text_editor_extra_key_bindings =  extra_bindings;
  scale_ = 1.;
  default_icon = nil;
  // initialize key table
  key_table = darwin_key_table;
  key_table_size = sizeof(darwin_key_table)/sizeof(*darwin_key_table);
}


void Fl_Cocoa_Screen_Driver::init()
{
  open_display();
  CGDirectDisplayID displays[MAX_SCREENS];
  CGDisplayCount count, i;
  CGRect r;
  CGGetActiveDisplayList(MAX_SCREENS, displays, &count);
  for( i = 0; i < count; i++) {
    r = CGDisplayBounds(displays[i]);
    screens[i].x      = int(r.origin.x);
    screens[i].y      = int(r.origin.y);
    screens[i].width  = int(r.size.width);
    screens[i].height = int(r.size.height);
    //fprintf(stderr,"screen %d %dx%dx%dx%d\n",i,screens[i].x,screens[i].y,screens[i].width,screens[i].height);
    if (&CGDisplayScreenSize != NULL) {
      CGSize s = CGDisplayScreenSize(displays[i]); // from 10.3
      dpi_h[i] = screens[i].width / (s.width/25.4);
      dpi_v[i] = screens[i].height / (s.height/25.4);
    } else {
      dpi_h[i] = dpi_v[i] = 75.;
    }
  }
  num_screens = count;
}


void Fl_Cocoa_Screen_Driver::screen_xywh(int &X, int &Y, int &W, int &H, int n)
{
  if (num_screens < 0) init();

  if ((n < 0) || (n >= num_screens))
    n = 0;

  float s = scale(0);
  X = screens[n].x/s;
  Y = screens[n].y/s;
  W = screens[n].width/s;
  H = screens[n].height/s;
}


void Fl_Cocoa_Screen_Driver::screen_dpi(float &h, float &v, int n)
{
  if (num_screens < 0) init();
  h = v = 0.0f;

  if (n >= 0 && n < num_screens) {
    h = dpi_h[n];
    v = dpi_v[n];
  }
}


// Implements fl_beep(). See documentation in src/fl_ask.cxx.
void Fl_Cocoa_Screen_Driver::beep(int type) {
  switch (type) {
    case FL_BEEP_DEFAULT :
    case FL_BEEP_ERROR :
      NSBeep();
      break;
    default :
      break;
  }
}


extern void fl_fix_focus(); // in Fl.cxx

extern void *fl_capture;


void Fl_Cocoa_Screen_Driver::grab(Fl_Window* win)
{
  if (win) {
    if (!Fl::grab_) {
      fl_capture = (FLWindow*)(Fl_X::flx(Fl::first_window())->xid);
      Fl_Cocoa_Window_Driver::driver(Fl::first_window())->set_key_window();
    }
    Fl::grab_ = win;
  } else {
    if (Fl::grab_) {
      fl_capture = 0;
      Fl::grab_ = 0;
      fl_fix_focus();
    }
  }
}


static void set_selection_color(uchar r, uchar g, uchar b)
{
  Fl::set_color(FL_SELECTION_COLOR,r,g,b);
}


// MacOS X currently supports two color schemes - Blue and Graphite.
// Since we aren't emulating the Aqua interface (even if Apple would
// let us), we use some defaults that are similar to both.  The
// Fl::scheme("plastic") color/box scheme provides a usable Aqua-like
// look-n-feel...
void Fl_Cocoa_Screen_Driver::get_system_colors()
{
  open_display();

  if (!bg2_set) Fl::background2(0xff, 0xff, 0xff);
  if (!fg_set) Fl::foreground(0, 0, 0);
  if (!bg_set) Fl::background(0xd8, 0xd8, 0xd8);

#if 0
  // this would be the correct code, but it does not run on all versions
  // of OS X. Also, setting a bright selection color would require
  // some updates in Fl_Adjuster and Fl_Help_Browser
  OSStatus err;
  RGBColor c;
  err = GetThemeBrushAsColor(kThemeBrushPrimaryHighlightColor, 24, true, &c);
  if (err)
    set_selection_color(0x00, 0x00, 0x80);
  else
    set_selection_color(c.red, c.green, c.blue);
#else
  set_selection_color(0x00, 0x00, 0x80);
#endif
}


int Fl_Cocoa_Screen_Driver::has_marked_text() const {
  return 1;
}


int Fl_Cocoa_Screen_Driver::insertion_point_x = 0;
int Fl_Cocoa_Screen_Driver::insertion_point_y = 0;
int Fl_Cocoa_Screen_Driver::insertion_point_height = 0;
bool Fl_Cocoa_Screen_Driver::insertion_point_location_is_valid = false;

void Fl_Cocoa_Screen_Driver::reset_marked_text() {
  Fl::compose_state = 0;
  next_marked_length = 0;
  insertion_point_location_is_valid = false;
}

// computes window coordinates & height of insertion point
int Fl_Cocoa_Screen_Driver::insertion_point_location(int *px, int *py, int *pheight)
// return true if the current coordinates of the insertion point are available
{
  if ( ! insertion_point_location_is_valid ) return false;
  *px = insertion_point_x;
  *py = insertion_point_y;
  *pheight = insertion_point_height;
  return true;
}

void Fl_Cocoa_Screen_Driver::insertion_point_location(int x, int y, int height) {
  insertion_point_location_is_valid = true;
  insertion_point_x = x;
  insertion_point_y = y;
  insertion_point_height = height;
}

int Fl_Cocoa_Screen_Driver::compose(int &del) {
  int condition;
  int has_text_key = Fl::compose_state || Fl::e_keysym <= '~' || Fl::e_keysym == FL_Iso_Key ||
  Fl::e_keysym == FL_JIS_Underscore || Fl::e_keysym == FL_Yen ||
  (Fl::e_keysym >= FL_KP && Fl::e_keysym <= FL_KP_Last && Fl::e_keysym != FL_KP_Enter);
  condition = Fl::e_state&(FL_META | FL_CTRL) ||
  (Fl::e_keysym >= FL_Shift_L && Fl::e_keysym <= FL_Alt_R) || // called from flagsChanged
  !has_text_key ;
  if (condition) { del = 0; return 0;} // this stuff is to be treated as a function key
  del = Fl::compose_state;
  Fl::compose_state = next_marked_length;
  return 1;
}


int Fl_Cocoa_Screen_Driver::input_widget_handle_key(int key, unsigned mods, unsigned shift, Fl_Input *input)
{
  switch (key) {
    case FL_Delete: {
      if (mods==0)          return input->kf_delete_char_right();       // Delete         (OSX-HIG,TE,SA,WOX)
      if (mods==FL_CTRL)    return input->kf_delete_char_right();       // Ctrl-Delete    (??? TE,!SA,!WOX)
      if (mods==FL_ALT)     return input->kf_delete_word_right();       // Alt-Delete     (OSX-HIG,TE,SA)
      return 0;                                                 // ignore other combos, pass to parent
    }

    case FL_Left:
      if (mods==0)          return input->kf_move_char_left();          // Left           (OSX-HIG)
      if (mods==FL_ALT)     return input->kf_move_word_left();          // Alt-Left       (OSX-HIG)
      if (mods==FL_META)    return input->kf_move_sol();                // Meta-Left      (OSX-HIG)
      if (mods==FL_CTRL)    return input->kf_move_sol();                // Ctrl-Left      (TE/SA)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_Right:
      if (mods==0)          return input->kf_move_char_right(); // Right          (OSX-HIG)
      if (mods==FL_ALT)     return input->kf_move_word_right(); // Alt-Right      (OSX-HIG)
      if (mods==FL_META)    return input->kf_move_eol();                // Meta-Right     (OSX-HIG)
      if (mods==FL_CTRL)    return input->kf_move_eol();                // Ctrl-Right     (TE/SA)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_Up:
      if (mods==0)          return input->kf_lines_up(1);               // Up             (OSX-HIG)
      if (mods==FL_CTRL)    return input->kf_page_up();         // Ctrl-Up        (TE !HIG)
      if (mods==FL_ALT)     return input->kf_move_up_and_sol(); // Alt-Up         (OSX-HIG)
      if (mods==FL_META)    return input->kf_top();                     // Meta-Up        (OSX-HIG)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_Down:
      if (mods==0)          return input->kf_lines_down(1);             // Dn             (OSX-HIG)
      if (mods==FL_CTRL)    return input->kf_page_down();               // Ctrl-Dn        (TE !HIG)
      if (mods==FL_ALT)     return input->kf_move_down_and_eol();       // Alt-Dn         (OSX-HIG)
      if (mods==FL_META)    return input->kf_bottom();                  // Meta-Dn        (OSX-HIG)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_Page_Up:
      // Fl_Input has no scroll control, so instead we move the cursor by one page
      // OSX-HIG recommends Alt increase one semantic unit, Meta next higher..
      if (mods==0)          return input->kf_page_up();         // PgUp           (OSX-HIG)
      if (mods==FL_ALT)     return input->kf_page_up();         // Alt-PageUp     (OSX-HIG)
      if (mods==FL_META)    return input->kf_top();                     // Meta-PageUp    (OSX-HIG,!TE)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_Page_Down:
      // Fl_Input has no scroll control, so instead we move the cursor by one page
      // OSX-HIG recommends Alt increase one semantic unit, Meta next higher..
      if (mods==0)          return input->kf_page_down();               // PgDn           (OSX-HIG)
      if (mods==FL_ALT)     return input->kf_page_down();               // Alt-PageDn     (OSX-HIG)
      if (mods==FL_META)    return input->kf_bottom();                  // Meta-PageDn    (OSX-HIG,!TE)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_Home:
      if (mods==0)          return input->kf_top();                     // Home           (OSX-HIG)
      if (mods==FL_ALT)     return input->kf_top();                     // Alt-Home       (???)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_End:
      if (mods==0)          return input->kf_bottom();                  // End            (OSX-HIG)
      if (mods==FL_ALT)     return input->kf_bottom();                  // Alt-End        (???)
      return 0;                                                 // ignore other combos, pass to parent

    case FL_BackSpace:
      if (mods==0)          return input->kf_delete_char_left();        // Backspace      (OSX-HIG)
      if (mods==FL_CTRL)    return input->kf_delete_char_left();        // Ctrl-Backspace (TE/SA)
      if (mods==FL_ALT)     return input->kf_delete_word_left();        // Alt-Backspace  (OSX-HIG)
      if (mods==FL_META)    return input->kf_delete_sol();              // Meta-Backspace (OSX-HIG,!TE)
      return 0;                                                 // ignore other combos, pass to parent
    }
  return -1;
}

void Fl_Cocoa_Screen_Driver::offscreen_size(Fl_Offscreen off, int &width, int &height)
{
  width = (int)CGBitmapContextGetWidth((CGContextRef)off);
  height = (int)CGBitmapContextGetHeight((CGContextRef)off);
}

Fl_RGB_Image *Fl_Cocoa_Screen_Driver::read_win_rectangle(int X, int Y, int w, int h, Fl_Window *window,
                                                         bool may_capture_subwins, bool *did_capture_subwins)
{
  int bpp, bpr;
  uchar *base, *p;
  if (!window) { // read from offscreen buffer
    float s = 1;
    CGContextRef src = (CGContextRef)Fl_Surface_Device::surface()->driver()->gc();  // get bitmap context
    base = (uchar *)CGBitmapContextGetData(src);  // get data
    if(!base) return NULL;
    int sw = (int)CGBitmapContextGetWidth(src);
    int sh = (int)CGBitmapContextGetHeight(src);
    if( (sw - X < w) || (sh - Y < h) )  return NULL;
    bpr = (int)CGBitmapContextGetBytesPerRow(src);
    bpp = (int)CGBitmapContextGetBitsPerPixel(src)/8;
    Fl_Image_Surface *imgs = (Fl_Image_Surface*)Fl_Surface_Device::surface();
    int fltk_w, fltk_h;
    imgs->printable_rect(&fltk_w, &fltk_h);
    s = sw / float(fltk_w);
    X *= s; Y *= s; w *= s; h *= s;
    if (X + w > sw) w = sw - X;
    if (Y + h > sh) h = sh - Y;
    // Copy the image from the off-screen buffer to the memory buffer.
    int idx, idy;  // Current X & Y in image
    uchar *pdst, *psrc;
    p = new uchar[w * h * 4];
    for (idy = Y, pdst = p; idy < h + Y; idy ++) {
      for (idx = 0, psrc = base + idy * bpr + X * bpp; idx < w; idx ++, psrc += bpp, pdst += 4) {
        pdst[0] = psrc[0];  // R
        pdst[1] = psrc[1];  // G
        pdst[2] = psrc[2];  // B
      }
    }
    bpr = 0;
  } else { // read from window
    Fl_Cocoa_Window_Driver *d = Fl_Cocoa_Window_Driver::driver(window);
    CGImageRef cgimg = d->CGImage_from_window_rect(X, Y, w, h, may_capture_subwins);
    if (did_capture_subwins) *did_capture_subwins = may_capture_subwins;
    if (!cgimg) {
      return NULL;
    }
    w = (int)CGImageGetWidth(cgimg);
    h = (int)CGImageGetHeight(cgimg);
    Fl_Image_Surface *surf = new Fl_Image_Surface(w, h);
    Fl_Surface_Device::push_current(surf);
    ((Fl_Quartz_Graphics_Driver*)fl_graphics_driver)->draw_CGImage(cgimg, 0, 0, w, h, 0, 0, w, h);
    CGContextRef gc = (CGContextRef)fl_graphics_driver->gc();
    w = (int)CGBitmapContextGetWidth(gc);
    h = (int)CGBitmapContextGetHeight(gc);
    bpr = (int)CGBitmapContextGetBytesPerRow(gc);
    bpp = (int)CGBitmapContextGetBitsPerPixel(gc)/8;
    base = (uchar*)CGBitmapContextGetData(gc);
    p = new uchar[bpr * h];
    memcpy(p, base, bpr * h);
    Fl_Surface_Device::pop_current();
    delete surf;
    CFRelease(cgimg);
  }
  Fl_RGB_Image *rgb = new Fl_RGB_Image(p, w, h, 4, bpr);
  rgb->alloc_array = 1;
  return rgb;
}

void Fl_Cocoa_Screen_Driver::set_spot(int /*font*/, int size, int X, int Y, int /*W*/, int /*H*/, Fl_Window* /*win*/) {
  Fl_Cocoa_Screen_Driver::insertion_point_location(X, Y, size);
}

void Fl_Cocoa_Screen_Driver::reset_spot() {
  Fl_Cocoa_Screen_Driver::reset_marked_text();
}
