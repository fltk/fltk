//
// "$Id$"
//
// Progress bar widget routines.
//
// Copyright 2000-2010 by Michael Sweet.
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
// Contents:

//
//   Fl_Progress::draw()        - Draw the check button.
//   Fl_Progress::Fl_Progress() - Construct a Fl_Progress widget.
//

//
// Include necessary header files...
//

#include <FL/Fl.H>
#include <FL/Fl_Progress.H>
#include <FL/fl_draw.H>


//
// Fl_Progress is a progress bar widget based off Fl_Widget that shows a
// standard progress bar...
//


//
// 'Fl_Progress::draw()' - Draw the progress bar.
//

/** Draws the progress bar. */
void Fl_Progress::draw()
{
  int	progress;	// Size of progress bar...
  int	bx, by, bw, bh;	// Box areas...
  int	tx, tw;		// Temporary X + width


  // Get the box borders...
  bx = Fl::box_dx(box());
  by = Fl::box_dy(box());
  bw = Fl::box_dw(box());
  bh = Fl::box_dh(box());

  tx = x() + bx;
  tw = w() - bw;

  // Draw the progress bar...
  if (maximum_ > minimum_)
    progress = (int)(w() * (value_ - minimum_) / (maximum_ - minimum_) + 0.5f);
  else
    progress = 0;

  // Draw the box and label...
  if (progress > 0) {
    Fl_Color c = labelcolor();
    labelcolor(fl_contrast(labelcolor(), selection_color()));

    fl_push_clip(x(), y(), progress + bx, h());
      draw_box(box(), x(), y(), w(), h(), active_r() ? selection_color() : fl_inactive(selection_color()));
      draw_label(tx, y() + by, tw, h() - bh);
    fl_pop_clip();

    labelcolor(c);

    if (progress<w()) {
      fl_push_clip(tx + progress, y(), w() - progress, h());
        draw_box(box(), x(), y(), w(), h(), active_r() ? color() : fl_inactive(color()));
        draw_label(tx, y() + by, tw, h() - bh);
      fl_pop_clip();
    }
  } else {
    draw_box(box(), x(), y(), w(), h(), active_r() ? color() : fl_inactive(color()));
    draw_label(tx, y() + by, tw, h() - bh);
  }
}


/**  
    The constructor creates the progress bar using the position, size, and label.
    
    You can set the background color with color() and the
    progress bar color with selection_color(), or you can set both colors
    together with color(unsigned bg, unsigned sel).
    
    The default colors are FL_BACKGROUND2_COLOR and FL_YELLOW, resp.
*/
Fl_Progress::Fl_Progress(int X, int Y, int W, int H, const char* L)
: Fl_Widget(X, Y, W, H, L) {
  align(FL_ALIGN_INSIDE);
  box(FL_DOWN_BOX);
  color(FL_BACKGROUND2_COLOR, FL_YELLOW);
  minimum(0.0f);
  maximum(100.0f);
  value(0.0f);
}


//
// End of "$Id$".
//
