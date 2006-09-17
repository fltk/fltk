//
// "$Id$"
//
// "GTK" drawing routines for the Fast Light Tool Kit (FLTK).
//
// These box types provide a GTK+ look, based on Red Hat's Bluecurve
// theme...
//
// Copyright 2006 by Michael Sweet.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

// Box drawing code for an obscure box type.
// These box types are in seperate files so they are not linked
// in if not used.

#include <FL/Fl.H>
#include <FL/fl_draw.H>

extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*);


static void gtk_up_frame(int x, int y, int w, int h, Fl_Color c) {
  fl_color(fl_color_average(FL_WHITE, c, 0.5));
  fl_xyline(x + 2, y + 1, x + w - 3);
  fl_yxline(x + 1, y + 2, y + h - 4);

  fl_color(fl_color_average(FL_BLACK, c, 0.5));
  fl_begin_loop();
    fl_vertex(x, y + 2);
    fl_vertex(x + 2, y);
    fl_vertex(x + w - 3, y);
    fl_vertex(x + w - 1, y + 2);
    fl_vertex(x + w - 1, y + h - 3);
    fl_vertex(x + w - 3, y + h - 1);
    fl_vertex(x + 2, y + h - 1);
    fl_vertex(x, y + h - 3);
  fl_end_loop();
}


static void gtk_up_box(int x, int y, int w, int h, Fl_Color c) {
  gtk_up_frame(x, y, w, h, c);

  fl_color(fl_color_average(FL_WHITE, c, 0.4));
  fl_xyline(x + 2, y + 2, x + w - 3);
  fl_color(fl_color_average(FL_WHITE, c, 0.2));
  fl_xyline(x + 2, y + 3, x + w - 3);
  fl_color(fl_color_average(FL_WHITE, c, 0.1));
  fl_xyline(x + 2, y + 4, x + w - 3);
  fl_color(c);
  fl_rectf(x + 2, y + 5, w - 4, h - 7);
  fl_color(fl_color_average(FL_BLACK, c, 0.025));
  fl_xyline(x + 2, y + h - 4, x + w - 3);
  fl_color(fl_color_average(FL_BLACK, c, 0.05));
  fl_xyline(x + 2, y + h - 3, x + w - 3);
  fl_color(fl_color_average(FL_BLACK, c, 0.1));
  fl_xyline(x + 2, y + h - 2, x + w - 3);
  fl_yxline(x + w - 2, y + 2, y + h - 3);
}


void gtk_down_frame(int x, int y, int w, int h, Fl_Color c) {
  fl_color(fl_color_average(FL_BLACK, c, 0.5));
  fl_begin_loop();
    fl_vertex(x, y + 2);
    fl_vertex(x + 2, y);
    fl_vertex(x + w - 3, y);
    fl_vertex(x + w - 1, y + 2);
    fl_vertex(x + w - 1, y + h - 3);
    fl_vertex(x + w - 3, y + h - 1);
    fl_vertex(x + 2, y + h - 1);
    fl_vertex(x, y + h - 3);
  fl_end_loop();

  fl_color(fl_color_average(FL_BLACK, c, 0.1));
  fl_xyline(x + 2, y + 1, x + w - 3);
  fl_yxline(x + 1, y + 2, y + h - 3);

  fl_color(fl_color_average(FL_BLACK, c, 0.05));
  fl_yxline(x + 2, y + h - 2, y + 2, x + w - 2);
}


void gtk_down_box(int x, int y, int w, int h, Fl_Color c) {
  gtk_down_frame(x, y, w, h, c);

  fl_color(c);
  fl_rectf(x + 3, y + 3, w - 5, h - 4);
  fl_yxline(x + w - 2, y + 3, y + h - 3);
}


void gtk_round_box(int x, int y, int w, int h, Fl_Color c) {
  fl_color(c);
  fl_pie(x, y, w, h, 0.0, 360.0);
  fl_color(fl_color_average(FL_BLACK, c, 0.5));
  fl_arc(x, y, w, h, 0.0, 360.0);
}


Fl_Boxtype fl_define_FL_GTK_UP_BOX() {
  fl_internal_boxtype(_FL_GTK_UP_BOX, gtk_up_box);
  fl_internal_boxtype(_FL_GTK_DOWN_BOX, gtk_down_box);
  fl_internal_boxtype(_FL_GTK_UP_FRAME, gtk_up_frame);
  fl_internal_boxtype(_FL_GTK_DOWN_FRAME, gtk_down_frame);
  fl_internal_boxtype(_FL_GTK_ROUND_UP_BOX, gtk_round_box);
  fl_internal_boxtype(_FL_GTK_ROUND_DOWN_BOX, gtk_round_box);

  return _FL_GTK_UP_BOX;
}


//
// End of "$Id$".
//
