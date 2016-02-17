//
// "$Id$"
//
// Definition of Apple Cocoa Screen interface.
//
// Copyright 1998-2016 by Bill Spitzak and others.
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
#include "Fl_Cocoa_Screen_Driver.h"
#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/fl_ask.H>
#include <stdio.h>


extern "C" void NSBeep(void);

extern double fl_mac_flush_and_wait(double time_to_wait);
extern int fl_ready();


/**
 Creates a driver that manages all screen and display related calls.
 
 This function must be implemented once for every platform. It is called
 when the static members of the class "Fl" are created.
 */
Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver()
{
  return new Fl_Cocoa_Screen_Driver();
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


void Fl_Cocoa_Screen_Driver::screen_work_area(int &X, int &Y, int &W, int &H, int n)
{
  if (num_screens < 0) init();
  if (n < 0 || n >= num_screens) n = 0;
  Fl_X::screen_work_area(X, Y, W, H, n);
}


void Fl_Cocoa_Screen_Driver::screen_xywh(int &X, int &Y, int &W, int &H, int n)
{
  if (num_screens < 0) init();

  if ((n < 0) || (n >= num_screens))
    n = 0;

  X = screens[n].x;
  Y = screens[n].y;
  W = screens[n].width;
  H = screens[n].height;
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
  if (fl_gc)
    CGContextFlush(fl_gc);
}


double Fl_Cocoa_Screen_Driver::wait(double time_to_wait)
{
  Fl::run_checks();
  return fl_mac_flush_and_wait(time_to_wait);
}


int Fl_Cocoa_Screen_Driver::ready()
{
  return fl_ready();
}


extern void fl_fix_focus(); // in Fl.cxx

extern void *fl_capture;


void Fl_Cocoa_Screen_Driver::grab(Fl_Window* win)
{
  if (win) {
    if (!Fl::grab_) {
      fl_capture = Fl_X::i(Fl::first_window())->xid;
      Fl_X::i(Fl::first_window())->set_key_window();
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
  fl_open_display();

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
  return getenv("FLTK_SCHEME");
}



//
// End of "$Id$".
//
