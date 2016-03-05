//
// "$Id$"
//
// Offscreen drawing test program for the Fast Light Tool Kit (FLTK).
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

/* Standard headers */
#include <stdlib.h>
#include <time.h> // time() - used to seed rand()

/* Fltk headers */
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/x.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

static Fl_Double_Window *main_window = 0;

// constants to define the view etc.
static const int offscreen_size = 1000;
static const int win_size = 512;
static const int first_useful_color = 56;
static const int last_useful_color = 255;
static const int num_iterations = 300;
static const double max_line_width = 9.0;
static const double delta_time = 0.1;

/*****************************************************************************/
class oscr_box : public Fl_Box
{
public:
  oscr_box(int x, int y, int w, int h);
  void oscr_drawing(void);
  bool has_oscr() const
  {
    if (oscr) return true;
    return false;
  }
private:
  void draw();
  int handle(int event);
  // Generate "random" values for the line display
  double random_val(int v) const
  {
    double dr = (double)(rand()) / (double)(RAND_MAX); // 0.0 to 1.0
    dr = dr * (double)(v); // 0 to v
    return dr;
  }
  // The offscreen surface
  Fl_Offscreen oscr;
  // variables used to handle "dragging" of the view within the box
  int x1, y1;   // drag start positions
  int xoff, yoff; // drag offsets
  int drag_state; // non-zero if drag is in progress
  int page_x, page_y; // top left of view area
  // Width and height of the offscreen surface
  int offsc_w, offsc_h;
};

/*****************************************************************************/
oscr_box::oscr_box(int x, int y, int w, int h) :
  Fl_Box(x, y, w, h), // base box
  oscr(0), // offscreen is not set at start
  x1(0), y1(0), drag_state(0), // not dragging view
  page_x((offscreen_size - win_size) / 2), // roughly centred in view
  page_y((offscreen_size - win_size) / 2),
  offsc_w(0), offsc_h(0) // offscreen size - initially none
{ } // Constructor

/*****************************************************************************/
void oscr_box::draw()
{
  int wd = w();
  int ht = h();
  int xo = x();
  int yo = y();

  fl_color(fl_gray_ramp(19)); // a light grey background shade
  fl_rectf(xo, yo, wd, ht); // fill the box with this colour

  // then add the offscreen on top of the grey background
  if (has_oscr()) // offscreen exists
  {
    fl_copy_offscreen(xo, yo, wd, ht, oscr, page_x, page_y);
  }
  else  // create offscreen
  {
    // some hosts may need a valid window context to base the offscreen on...
    main_window->make_current();
    offsc_w = offscreen_size;
    offsc_h = offscreen_size;
    oscr = fl_create_offscreen(offsc_w, offsc_h);
  }
} // draw method

/*****************************************************************************/
int oscr_box::handle(int ev)
{
  int ret = Fl_Box::handle(ev);
  // handle dragging of visible page area - if a valid context exists
  if (has_oscr())
  {
    switch (ev)
    {
    case FL_ENTER:
      main_window->cursor(FL_CURSOR_MOVE);
      ret = 1;
      break;

    case FL_LEAVE:
      main_window->cursor(FL_CURSOR_DEFAULT);
      ret = 1;
      break;

    case FL_PUSH:
      x1 = Fl::event_x_root();
      y1 = Fl::event_y_root();
      drag_state = 1; // drag
      ret = 1;
      break;

    case FL_DRAG:
      if (drag_state == 1) // dragging page
      {
	int x2 = Fl::event_x_root();
	int y2 = Fl::event_y_root();
	xoff = x1 - x2;
	yoff = y1 - y2;
	x1 = x2;
	y1 = y2;
	page_x += xoff;
	page_y += yoff;
	// check the page bounds
	if (page_x < -w())
	{
	  page_x = -w();
	}
	else if (page_x > offsc_w)
	{
	  page_x = offsc_w;
	}
	if (page_y < -h())
	{
	  page_y = -h();
	}
	else if (page_y > offsc_h)
	{
	  page_y = offsc_h;
	}
	redraw();
      }
      ret = 1;
      break;

    case FL_RELEASE:
      drag_state = 0;
      ret = 1;
      break;

    default:
      break;
    }
  }
  return ret;
} // handle

/*****************************************************************************/
void oscr_box::oscr_drawing(void)
{
  Fl_Color col;
  static int icol = first_useful_color;
  static int ox = (offscreen_size / 2);
  static int oy = (offscreen_size / 2);
  static int iters = num_iterations + 1; // Must be set on first pass!

  if (!has_oscr())
  {
    return; // no valid offscreen, nothing to do here
  }

  fl_begin_offscreen(oscr); /* Open the offscreen context for drawing */
  {
    if (iters > num_iterations) // clear the offscreen and start afresh
    {
      fl_color(FL_WHITE);
      fl_rectf(0, 0, offsc_w, offsc_h);
      iters = 0;
    }
    iters++;

    icol++;
    if (icol > last_useful_color)
    {
      icol = first_useful_color;
    }
    col = static_cast<Fl_Color>(icol);
    fl_color(col); // set the colour

    double drx = random_val(offsc_w);
    double dry = random_val(offsc_h);
    double drt = random_val(max_line_width);

    int ex = static_cast<int>(drx);
    int ey = static_cast<int>(dry);
    fl_line_style(FL_SOLID, static_cast<int>(drt));
    fl_line(ox, oy, ex, ey);
    ox = ex;
    oy = ey;
  }
  fl_end_offscreen(); // close the offscreen context
  redraw();
} // oscr_drawing

/*****************************************************************************/
static oscr_box *os_box = 0; // a widget to view the offscreen with

/*****************************************************************************/
static void oscr_anim(void *)
{
  os_box->oscr_drawing(); // if the offscreen exists, draw something
  Fl::repeat_timeout(delta_time, oscr_anim);
} // oscr_anim

/*****************************************************************************/
int main(int argc, char **argv)
{
  int dim1 = win_size;
  main_window = new Fl_Double_Window(dim1, dim1, "Offscreen demo");
  main_window->begin();

  dim1 -= 10;
  os_box = new oscr_box(5, 5, dim1, dim1);
  main_window->end();
  main_window->resizable(os_box);

  main_window->show(argc, argv);

  srand(time(NULL)); // seed the random sequence generator

  Fl::add_timeout(delta_time, oscr_anim);

  return Fl::run();
} // main

//
// End of "$Id$".
//
