//
// "$Id: Fl_Tooltip.cxx,v 1.38.2.12 2002/04/14 02:43:48 easysw Exp $"
//
// Tooltip source file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2002 by Bill Spitzak and others.
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
#include <FL/Fl_Tooltip.H>

#include <stdio.h>

//
// Fl_Tooltip global variables...
//

void		(*Fl_Tooltip::tooltip_callback_)(void *) = Fl_Tooltip::tooltip_timeout;
void		(*Fl_Tooltip::tooltip_exit_)(void *) = (void (*)(void *))Fl_Tooltip::tooltip_exit;
float		Fl_Tooltip::delay_ = 0.5;
Fl_Tooltip_Box	*Fl_Tooltip::box = 0;
Fl_Tooltip_Window *Fl_Tooltip::window = 0;
Fl_Widget	*Fl_Tooltip::widget = 0;
int		Fl_Tooltip::shown = 0;
unsigned	Fl_Tooltip::color_ = fl_color_cube(FL_NUM_RED - 1,
		                                   FL_NUM_GREEN - 1,
						   FL_NUM_BLUE - 2);
int		Fl_Tooltip::font_ = FL_HELVETICA;
int		Fl_Tooltip::size_ = FL_NORMAL_SIZE;


//
// Tooltip window class...
//

class Fl_Tooltip_Window : public Fl_Menu_Window {
  public:

  FL_EXPORT ~Fl_Tooltip_Window() {}
  Fl_Tooltip_Window(int W, int H, const char *l = 0)
    : Fl_Menu_Window(W,H,l) {}
  Fl_Tooltip_Window(int X, int Y, int W, int H, const char *l = 0)
    : Fl_Menu_Window(X,Y,W,H,l) {}

  virtual FL_EXPORT int handle(int event);
};


int
Fl_Tooltip_Window::handle(int event) {
  switch (event) {
    case FL_KEYUP :
    case FL_KEYDOWN :
    case FL_SHORTCUT :
    case FL_PUSH :
    case FL_DRAG :
    case FL_RELEASE :
    case FL_MOUSEWHEEL :
      if (Fl_Tooltip::widget) {
        // Pass events to widget...
        Fl::belowmouse(Fl_Tooltip::widget);
	// Update event_x() and event_y() to be relative to the
	// widget's window, not the tooltip window...
	Fl::e_x = Fl::e_x_root - Fl_Tooltip::widget->window()->x();
	Fl::e_y = Fl::e_y_root - Fl_Tooltip::widget->window()->y();
	return Fl_Tooltip::widget->handle(event);
      } else return 0;
    default :
      return Fl_Menu_Window::handle(event);
  }
}


//
// Tooltip label class...
//

class Fl_Tooltip_Box : public Fl_Box {
public:
  
  Fl_Tooltip_Box() : Fl_Box(0,0,10,10) {
    color(Fl_Tooltip::color_);
    align(FL_ALIGN_CENTER);
    box(FL_BORDER_BOX);
    Fl_Tooltip::widget = 0;
  }

  ~Fl_Tooltip_Box() { }

  void draw() {
    tooltip(0); // Just in case

    if (!Fl_Tooltip::widget || !Fl_Tooltip::widget->tooltip())
      return;
    
    Fl_Window *widgetWindow = Fl_Tooltip::widget->window();
    
    if (!widgetWindow) {
//      printf("!widgetWindow\n");
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

    int ox = Fl::event_x_root() + 10;
    int oy = Fl::event_y_root() + 10;

    if (ox >= (Fl::w() - ww - 6))
      ox = Fl::w() - ww - 6;
    if (oy >= (Fl::h() - hh - 6))
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
//  printf("Fl_Tooltip::enter(%p)\n", w);
//  if (w) {
//    printf("    label() = \"%s\"\n", w->label() ? w->label() : "(null)");
//    printf("    visible() = %d\n", w->visible());
//    printf("    active() = %d\n", w->active());
//  }
  Fl_Widget* temp = w;
  while (temp && !temp->tooltip()) {
    if (temp == window) return;	// Don't do anything if pointed at tooltip
    temp = temp->parent();
  }

  if ((!w || !w->tooltip()) && tooltip_callback_ && window) {
//    puts("Hiding tooltip...");
    Fl::remove_timeout(tooltip_callback_);
    window->hide();
    shown = 0;
    return;
  }
  if (!tooltip_callback_ || !w || !w->tooltip()) return;
  Fl::remove_timeout(tooltip_callback_);
  if (window && window->shown()) (*tooltip_callback_)(w);
  else Fl::add_timeout(delay_, tooltip_callback_, w);
}


// This function must be called when
// an event != FL_MOVE has occured in
// the widget
void
Fl_Tooltip::exit(Fl_Widget *w) {
//  printf("Fl_Tooltip::exit(%p)\n", w);
  if (tooltip_exit_) tooltip_exit_(w);
}

void
Fl_Tooltip::tooltip_exit(Fl_Widget *w) {
//  printf("Fl_Tooltip::tooltip_exit(%p), widget = %p, window = %p, shown = %d\n",
//         w, widget, window, shown);
  if (!w || w != widget) return;

  Fl::remove_timeout(tooltip_callback_);

  widget = 0;

  if (window) {
//    puts("Hiding tooltip...");
    window->hide();
    shown = 0;
  }
}

void 
Fl_Tooltip::tooltip_timeout(void *v) {
//  printf("Fl_Tooltip::tooltip_timeout(%p)\n", v);
  if (!window) {
    Fl_Group* saveCurrent = Fl_Group::current();
    Fl_Group::current(0);
    window = new Fl_Tooltip_Window(0, 0, 10, 10, 0);
    window->clear_border();
    window->box(FL_NO_BOX);
    window->set_override();

    window->begin();
    box = new Fl_Tooltip_Box;
    box->color(FL_YELLOW);
    box->align(FL_ALIGN_CENTER);
    window->resizable(box);
    window->end();
    Fl_Group::current(saveCurrent);

//    printf("Fl_Tooltip::window = %p\n", window);
//    printf("Fl_Tooltip::box    = %p\n", box);
  }

  if (!v)
    return;

  if (shown && widget != (Fl_Widget *)v) {
    tooltip_exit(widget);
  }

  widget = (Fl_Widget*) v;
  Fl_Window *widgetWindow = widget->window();

  if (widgetWindow) {
//    puts("Showing tooltip");
    Fl::grab(*widgetWindow);
    window->show();
    Fl::release();
    shown = 1;
  } else {
    widget = 0;
  }
}


//
// End of "$Id: Fl_Tooltip.cxx,v 1.38.2.12 2002/04/14 02:43:48 easysw Exp $".
//
