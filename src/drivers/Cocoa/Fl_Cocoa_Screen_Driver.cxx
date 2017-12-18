//
// "$Id$"
//
// Definition of Apple Cocoa Screen interface.
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


#include "../../config_lib.h"
#include "Fl_Cocoa_Screen_Driver.H"
#include "Fl_Cocoa_Window_Driver.H"
#include "../Quartz/Fl_Font.H"
#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Graphics_Driver.H>
#include <FL/Fl_Input.H>
#include <FL/fl_ask.H>
#include <stdio.h>


extern "C" void NSBeep(void);

int Fl_Cocoa_Screen_Driver::next_marked_length = 0;

/*
 Creates a driver that manages all screen and display related calls.
 
 This function must be implemented once for every platform.
 */
Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver()
{
  return new Fl_Cocoa_Screen_Driver();
}

static Fl_Text_Editor::Key_Binding extra_bindings[] =  {
  // Define CMD+key accelerators...
  { 'z',          FL_COMMAND,               Fl_Text_Editor::kf_undo       ,0},
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
}


void Fl_Cocoa_Screen_Driver::init()
{
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


/**
 Emits a system beep message.
 \param[in] type   The beep type from the \ref Fl_Beep enumeration.
 \note \#include <FL/fl_ask.H>
 */
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


void Fl_Cocoa_Screen_Driver::flush() {
  CGContextRef gc = (CGContextRef)Fl_Graphics_Driver::default_driver().gc();
  if (gc)
    CGContextFlush(gc);
}


extern void fl_fix_focus(); // in Fl.cxx

extern void *fl_capture;


void Fl_Cocoa_Screen_Driver::grab(Fl_Window* win)
{
  if (win) {
    if (!Fl::grab_) {
      fl_capture = Fl_X::i(Fl::first_window())->xid;
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


// simulation of XParseColor:
int Fl_Cocoa_Screen_Driver::parse_color(const char* p, uchar& r, uchar& g, uchar& b)
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


const char *Fl_Cocoa_Screen_Driver::get_system_scheme()
{
  return fl_getenv("FLTK_SCHEME");
}


int Fl_Cocoa_Screen_Driver::has_marked_text() {
  return true;
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
  (Fl::e_keysym >= FL_KP && Fl::e_keysym <= FL_KP_Last && Fl::e_keysym != FL_KP_Enter);
  condition = Fl::e_state&(FL_META | FL_CTRL) ||
  (Fl::e_keysym >= FL_Shift_L && Fl::e_keysym <= FL_Alt_R) || // called from flagsChanged
  !has_text_key ;
  if (condition) { del = 0; return 0;} // this stuff is to be treated as a function key
  del = Fl::compose_state;
  Fl::compose_state = next_marked_length;
  return 1;
}

uchar *                                                 // O - Pixel buffer or NULL if failed
Fl_Cocoa_Screen_Driver::read_image(uchar *p,		// I - Pixel buffer or NULL to allocate
                                   int   x,		// I - Left position
                                   int   y,		// I - Top position
                                   int   w,		// I - Width of area to read
                                   int   h,		// I - Height of area to read
                                   int   alpha) 	// I - Alpha value for image (0 for none)
{
  uchar *base;
  int rowBytes, delta;
  if (fl_window == NULL) { // reading from an offscreen buffer
    CGContextRef src = (CGContextRef)Fl_Surface_Device::surface()->driver()->gc();  // get bitmap context
    base = (uchar *)CGBitmapContextGetData(src);  // get data
    if(!base) return NULL;
    int sw = CGBitmapContextGetWidth(src);
    int sh = CGBitmapContextGetHeight(src);
    rowBytes = CGBitmapContextGetBytesPerRow(src);
    delta = CGBitmapContextGetBitsPerPixel(src)/8;
    if( (sw - x < w) || (sh - y < h) )  return NULL;
  }
  else { // reading from current window
    Fl_Cocoa_Window_Driver *d = Fl_Cocoa_Window_Driver::driver(Fl_Window::current());
    base = d->bitmap_from_window_rect(x,y,w,h,&delta);
    if (!base) return NULL;
    rowBytes = delta*w;
    x = y = 0;
  }
  // Allocate the image data array as needed...
  int d = alpha ? 4 : 3;
  if (!p) p = new uchar[w * h * d];
  // Initialize the default colors/alpha in the whole image...
  memset(p, alpha, w * h * d);
  // Copy the image from the off-screen buffer to the memory buffer.
  int           idx, idy;	// Current X & Y in image
  uchar *pdst, *psrc;
  for (idy = y, pdst = p; idy < h + y; idy ++) {
    for (idx = 0, psrc = base + idy * rowBytes + x * delta; idx < w; idx ++, psrc += delta, pdst += d) {
      pdst[0] = psrc[0];  // R
      pdst[1] = psrc[1];  // G
      pdst[2] = psrc[2];  // B
    }
  }
  if(fl_window != NULL) delete[] base;
  return p;
}


int Fl_Cocoa_Screen_Driver::input_widget_handle_key(int key, unsigned mods, unsigned shift, Fl_Input *input)
{
  switch (key) {
    case FL_Delete: {
      if (mods==0)          return input->kf_delete_char_right();	// Delete         (OSX-HIG,TE,SA,WOX)
      if (mods==FL_CTRL)    return input->kf_delete_char_right();	// Ctrl-Delete    (??? TE,!SA,!WOX)
      if (mods==FL_ALT)     return input->kf_delete_word_right();	// Alt-Delete     (OSX-HIG,TE,SA)
      return 0;							// ignore other combos, pass to parent
    }
      
    case FL_Left:
      if (mods==0)          return input->kf_move_char_left();		// Left           (OSX-HIG)
      if (mods==FL_ALT)     return input->kf_move_word_left();		// Alt-Left       (OSX-HIG)
      if (mods==FL_META)    return input->kf_move_sol();		// Meta-Left      (OSX-HIG)
      if (mods==FL_CTRL)    return input->kf_move_sol();		// Ctrl-Left      (TE/SA)
      return 0;							// ignore other combos, pass to parent
      
    case FL_Right:
      if (mods==0)          return input->kf_move_char_right();	// Right          (OSX-HIG)
      if (mods==FL_ALT)     return input->kf_move_word_right();	// Alt-Right      (OSX-HIG)
      if (mods==FL_META)    return input->kf_move_eol();		// Meta-Right     (OSX-HIG)
      if (mods==FL_CTRL)    return input->kf_move_eol();		// Ctrl-Right     (TE/SA)
      return 0;							// ignore other combos, pass to parent
      
    case FL_Up:
      if (mods==0)          return input->kf_lines_up(1);		// Up             (OSX-HIG)
      if (mods==FL_CTRL)    return input->kf_page_up();		// Ctrl-Up        (TE !HIG)
      if (mods==FL_ALT)     return input->kf_move_up_and_sol();	// Alt-Up         (OSX-HIG)
      if (mods==FL_META)    return input->kf_top();			// Meta-Up        (OSX-HIG)
      return 0;							// ignore other combos, pass to parent
      
    case FL_Down:
      if (mods==0)          return input->kf_lines_down(1);		// Dn             (OSX-HIG)
      if (mods==FL_CTRL)    return input->kf_page_down();		// Ctrl-Dn        (TE !HIG)
      if (mods==FL_ALT)     return input->kf_move_down_and_eol();	// Alt-Dn         (OSX-HIG)
      if (mods==FL_META)    return input->kf_bottom();			// Meta-Dn        (OSX-HIG)
      return 0;							// ignore other combos, pass to parent
      
    case FL_Page_Up:
      // Fl_Input has no scroll control, so instead we move the cursor by one page
      // OSX-HIG recommends Alt increase one semantic unit, Meta next higher..
      if (mods==0)          return input->kf_page_up();		// PgUp           (OSX-HIG)
      if (mods==FL_ALT)     return input->kf_page_up();		// Alt-PageUp     (OSX-HIG)
      if (mods==FL_META)    return input->kf_top();			// Meta-PageUp    (OSX-HIG,!TE)
      return 0;							// ignore other combos, pass to parent
      
    case FL_Page_Down:
      // Fl_Input has no scroll control, so instead we move the cursor by one page
      // OSX-HIG recommends Alt increase one semantic unit, Meta next higher..
      if (mods==0)          return input->kf_page_down();		// PgDn           (OSX-HIG)
      if (mods==FL_ALT)     return input->kf_page_down();		// Alt-PageDn     (OSX-HIG)
      if (mods==FL_META)    return input->kf_bottom();			// Meta-PageDn    (OSX-HIG,!TE)
      return 0;							// ignore other combos, pass to parent
      
    case FL_Home:
      if (mods==0)          return input->kf_top();			// Home           (OSX-HIG)
      if (mods==FL_ALT)     return input->kf_top();			// Alt-Home       (???)
      return 0;							// ignore other combos, pass to parent
      
    case FL_End:
      if (mods==0)          return input->kf_bottom();			// End            (OSX-HIG)
      if (mods==FL_ALT)     return input->kf_bottom();			// Alt-End        (???)
      return 0;							// ignore other combos, pass to parent
      
    case FL_BackSpace:
      if (mods==0)          return input->kf_delete_char_left();	// Backspace      (OSX-HIG)
      if (mods==FL_CTRL)    return input->kf_delete_char_left();	// Ctrl-Backspace (TE/SA)
      if (mods==FL_ALT)     return input->kf_delete_word_left();	// Alt-Backspace  (OSX-HIG)
      if (mods==FL_META)    return input->kf_delete_sol();		// Meta-Backspace (OSX-HIG,!TE)
      return 0;							// ignore other combos, pass to parent
    }
  return -1;
}

void Fl_Cocoa_Screen_Driver::offscreen_size(Fl_Offscreen off, int &width, int &height)
{
  width = CGBitmapContextGetWidth(off);
  height = CGBitmapContextGetHeight(off);
}

//
// End of "$Id$".
//
