//
// Application Menu Bar widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 2025 by Bill Spitzak and others.
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

//
// Include necessary headers...
//

#include "widgets/App_Menu_Bar.h"

#include "Fluid.h"
#include "Project.h"

using namespace fld;
using namespace fld::widget;

extern void mergeback_cb(Fl_Widget *, void *);

/**
 Create a fld::widget::App_Menu_Bar widget.
 \param[in] X, Y, W, H position and size of the widget
 \param[in] L optional label
 */
App_Menu_Bar::App_Menu_Bar(int X, int Y, int W, int H, const char *L)
: Fl_Menu_Bar(X, Y, W, H, L)
{
}

/**
 Set menu item visibility and active state before menu pops up.
 */
int App_Menu_Bar::handle(int event)
{
  Fl_Menu_Item *mi = nullptr;
  if (event == FL_BEFORE_MENU) {
    mi = (Fl_Menu_Item*)find_item(mergeback_cb);
    if (mi && Fluid.proj.write_mergeback_data)
      mi->show();
    else
      mi->hide();
    return 1;
  } else {
    return Fl_Menu_Bar::handle(event);
  }
}


