//
// Demonstrate how to use Fl_Chart in fltk
//
//     Demonstrates rudimentary features of Fl_Chart.
//     Origin: http://seriss.com/people/erco/fltk/#Fl_Chart
//
// Copyright 2008 by Greg Ercolano.
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
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Chart.H>
#include <FL/Fl_Choice.H>
#include <math.h>

// Globals
Fl_Window *G_win = 0;
Fl_Chart  *G_chart = 0;
Fl_Choice *G_choice = 0;

// Fl_Choice callback for changing chart type()
static void chart_type_cb(Fl_Widget *w, void*) {
    const Fl_Menu_Item *item = G_choice->mvalue();  // item picked
    G_chart->type( (uchar)item->argument() );       // apply change
    G_chart->redraw();
    // printf("Choice: '%s', argument=%ld\n", G_choice->text(), item->argument());
}

// main
int main() {
    G_win = new Fl_Window(1000, 510, "Chart Simple");
    // Fl_Chart with a sin() wave of data
    G_chart = new Fl_Chart(20, 20, G_win->w()-40, G_win->h()-80, "Chart");
    G_chart->bounds(-125.0, 125.0);
    const double start = 1.5;
    const double end = start + 15.1;
    for (double t = start; t < end; t += 0.5) {
        double val = sin(t) * 125.0;
        static char val_str[20];
        sprintf(val_str, "%.0lf", val);
        G_chart->add(val, val_str, (val<0)?FL_RED:FL_GREEN);
    }
    // Let user change chart type
    G_choice = new Fl_Choice(140,470,200,25,"Chart Type: ");
    G_choice->add("FL_BAR_CHART",        0, chart_type_cb, (void*)FL_BAR_CHART );
    G_choice->add("FL_HORBAR_CHART",     0, chart_type_cb, (void*)FL_HORBAR_CHART);
    G_choice->add("FL_LINE_CHART",       0, chart_type_cb, (void*)FL_LINE_CHART);
    G_choice->add("FL_FILL_CHART",       0, chart_type_cb, (void*)FL_FILL_CHART);
    G_choice->add("FL_SPIKE_CHART",      0, chart_type_cb, (void*)FL_SPIKE_CHART);
    G_choice->add("FL_PIE_CHART",        0, chart_type_cb, (void*)FL_PIE_CHART);
    G_choice->add("FL_SPECIALPIE_CHART", 0, chart_type_cb, (void*)FL_SPECIALPIE_CHART);
    G_choice->value(0);
    G_win->resizable(G_win);
    G_win->show();
    return(Fl::run());
}
