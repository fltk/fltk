//
// "$Id: Fl_Tooltip.cxx,v 1.38.2.1 2001/08/01 21:24:49 easysw Exp $"
//
// Tooltip source file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <FL/Fl.H>
#include <FL/Fl_Menu_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

#include <stdio.h>

//
// Fl_Tooltip global variables...
//

void		(*Fl_Tooltip::tooltip_callback_)(void *) = 0;
void		(*Fl_Tooltip::tooltip_exit_)(void *) = 0;
float		Fl_Tooltip::delay_ = 0.5;
Fl_TooltipBox	*Fl_Tooltip::box = 0;
Fl_Menu_Window	*Fl_Tooltip::window = 0;
Fl_Widget	*Fl_Tooltip::widget = 0;
int		Fl_Tooltip::shown = 0;
uchar		Fl_Tooltip::color_ = FL_YELLOW;
int		Fl_Tooltip::font_ = FL_HELVETICA;
int		Fl_Tooltip::size_ = FL_NORMAL_SIZE;


//
// Tooltip label class...
//

class Fl_TooltipBox : public Fl_Box {
public:
  
  Fl_TooltipBox() : Fl_Box(0,0,10,10) {
    color(Fl_Tooltip::color_);
    align(FL_ALIGN_CENTER);
    box(FL_BORDER_BOX);
    Fl_Tooltip::widget = 0;
  }

  ~Fl_TooltipBox() { }

  void draw() {
    tooltip(0); // Just in case

    if (!Fl_Tooltip::widget || !Fl_Tooltip::widget->tooltip())
      return;
    
    Fl_Window *widgetWindow = Fl_Tooltip::widget->window();
    
    if (!widgetWindow) {
      printf("!widgetWindow\n");
      return;
    }
    
    int ww, hh;
    ww = 0;
  
    labelfont(Fl_Tooltip::font_);
    labelsize(Fl_Tooltip::size_);
    color(Fl_Tooltip::color_);

    fl_font(Fl_Tooltip::font_, Fl_Tooltip::size_);

    fl_measure(Fl_Tooltip::widget->tooltip(), ww, hh);
    label(Fl_Tooltip::widget->tooltip());

    int ox = 
      widgetWindow->x_root() + Fl_Tooltip::widget->x() + Fl_Tooltip::widget->w()/2;
    int oy = 
      widgetWindow->y_root() + Fl_Tooltip::widget->y() + Fl_Tooltip::widget->h() + 10;
    
    if (ox >= Fl::w())
      ox = Fl::w() - ww - 6;
    if (oy >= Fl::h())
      oy = widgetWindow->y_root() + Fl_Tooltip::widget->y() - hh - 6;
    
    parent()->resize(ox, oy, ww+6, hh+6);
    
    draw_box();
    draw_label();
  }
};


// This function is called by widgets
// when the pointer enters them
void
Fl_Tooltip::enter(Fl_Widget *w) {
  if (!tooltip_callback_ || !w || !w->tooltip()) return;
  Fl::add_timeout(delay_, tooltip_callback_, w);
}


// This function must be called when
// an event != FL_MOVE has occured in
// the widget
void
Fl_Tooltip::exit(Fl_Widget *w) {
  if (tooltip_exit_ && w && w->tooltip()) tooltip_exit_(w);
}

void
Fl_Tooltip::tooltip_exit(Fl_Widget *w) {
  Fl::remove_timeout(tooltip_callback_, w);
  if ((w == widget || (widget && w == widget->window())) && shown && window) {
    widget = 0;
    window->hide();
    shown = 0;
  }
}

void 
Fl_Tooltip::tooltip_timeout(void *v) {
  if (!window) {
    Fl_Group* saveCurrent = Fl_Group::current();
    Fl_Group::current(0);
    window = new Fl_Menu_Window(0, 0, 10, 10, 0);
    window->clear_border();
    window->box(FL_NO_BOX);

    window->begin();
    box = new Fl_TooltipBox;
    box->color(FL_YELLOW);
    box->align(FL_ALIGN_CENTER);
    window->resizable(box);
    window->end();
    Fl_Group::current(saveCurrent);
  }

  if (!v)
    return;

  if (shown && widget != (Fl_Widget *)v) {
    tooltip_exit(widget);
  }

  widget = (Fl_Widget*) v;
  Fl_Window *widgetWindow = widget->window();

  if (widgetWindow) {
    Fl::grab(*widgetWindow);
    window->show();
    Fl::release();
    shown = 1;
  } else {
    widget = 0;
  }
}


//
// End of "$Id: Fl_Tooltip.cxx,v 1.38.2.1 2001/08/01 21:24:49 easysw Exp $".
//
