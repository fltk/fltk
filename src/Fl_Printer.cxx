//
// "$Id$"
//
// Encompasses platform-specific printing-support code and 
// PostScript output code for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl_Printer.H>

#ifdef NO_PRINT_SUPPORT

Fl_Printer::Fl_Printer(void) {
  printer = NULL;
}
int Fl_Printer::start_job(int pagecount, int *frompage, int *topage) {return 1;}
int Fl_Printer::start_page(void) {return 1;}
int Fl_Printer::printable_rect(int *w, int *h) {return 1;}
void Fl_Printer::margins(int *left, int *top, int *right, int *bottom) {}
void Fl_Printer::origin(int *x, int *y) {}
void Fl_Printer::origin(int x, int y) {}
void Fl_Printer::scale(float scale_x, float scale_y) {}
void Fl_Printer::rotate(float angle) {}
void Fl_Printer::translate(int x, int y) {}
void Fl_Printer::untranslate(void) {}
int Fl_Printer::end_page (void) {return 1;}
void Fl_Printer::end_job (void) {}
void Fl_Printer::print_widget(Fl_Widget* widget, int delta_x, int delta_y) {}
void Fl_Printer::print_window_part(Fl_Window *win, int x, int y, int w, int h, int delta_x, int delta_y) {}
void Fl_Printer::set_current(void) {}
Fl_Printer::~Fl_Printer(void) {}

const char *Fl_Printer::dialog_title = NULL;
const char *Fl_Printer::dialog_printer = NULL;
const char *Fl_Printer::dialog_range = NULL;
const char *Fl_Printer::dialog_copies = NULL;
const char *Fl_Printer::dialog_all = NULL;
const char *Fl_Printer::dialog_pages = NULL;
const char *Fl_Printer::dialog_from = NULL;
const char *Fl_Printer::dialog_to = NULL;
const char *Fl_Printer::dialog_properties = NULL;
const char *Fl_Printer::dialog_copyNo = NULL;
const char *Fl_Printer::dialog_print_button = NULL;
const char *Fl_Printer::dialog_cancel_button = NULL;
const char *Fl_Printer::dialog_print_to_file = NULL;
const char *Fl_Printer::property_title = NULL;
const char *Fl_Printer::property_pagesize = NULL;
const char *Fl_Printer::property_mode = NULL;
const char *Fl_Printer::property_use = NULL;
const char *Fl_Printer::property_save = NULL;
const char *Fl_Printer::property_cancel = NULL;

#else

// print dialog customization strings
/** [this text may be customized at run-time] */
const char *Fl_Printer::dialog_title = "Print";
/** [this text may be customized at run-time] */
const char *Fl_Printer::dialog_printer = "Printer:";
/** [this text may be customized at run-time] */
const char *Fl_Printer::dialog_range = "Print Range";
/** [this text may be customized at run-time] */
const char *Fl_Printer::dialog_copies = "Copies";
/** [this text may be customized at run-time] */
const char *Fl_Printer::dialog_all = "All";
/** [this text may be customized at run-time] */
const char *Fl_Printer::dialog_pages = "Pages";
/** [this text may be customized at run-time] */
const char *Fl_Printer::dialog_from = "From:";
/** [this text may be customized at run-time] */
const char *Fl_Printer::dialog_to = "To:";
/** [this text may be customized at run-time] */
const char *Fl_Printer::dialog_properties = "Properties...";
/** [this text may be customized at run-time] */
const char *Fl_Printer::dialog_copyNo = "# Copies:";
/** [this text may be customized at run-time] */
const char *Fl_Printer::dialog_print_button = "Print";
/** [this text may be customized at run-time] */
const char *Fl_Printer::dialog_cancel_button = "Cancel";
/** [this text may be customized at run-time] */
const char *Fl_Printer::dialog_print_to_file = "Print To File";
/** [this text may be customized at run-time] */
const char *Fl_Printer::property_title = "Printer Properties";
/** [this text may be customized at run-time] */
const char *Fl_Printer::property_pagesize = "Page Size:";
/** [this text may be customized at run-time] */
const char *Fl_Printer::property_mode = "Output Mode:";
/** [this text may be customized at run-time] */
const char *Fl_Printer::property_use = "Use";
/** [this text may be customized at run-time] */
const char *Fl_Printer::property_save = "Save";
/** [this text may be customized at run-time] */
const char *Fl_Printer::property_cancel = "Cancel";


Fl_Printer::Fl_Printer(void) {
  printer = new Helper();
  Fl_Surface_Device::driver(printer->driver());
}

/**
 Starts a print job.
 Opens a platform-specific dialog window allowing the user to set several options including
 the desired printer and the page orientation. Optionally, the user can also select a range of pages to  be
 printed. This range is returned to the caller that is in charge of sending only these pages 
 for printing.
 
 @param[in] pagecount the total number of pages of the job (or 0 if you don't know the number of pages)
 @param[out] frompage if non-null, *frompage is set to the first page the user wants printed
 @param[out] topage if non-null, *topage is set to the last page the user wants printed
 @return 0 if OK, non-zero if any error occurred or if the user cancelled the print request.
 */
int Fl_Printer::start_job(int pagecount, int *frompage, int *topage)
{
  return printer->start_job(pagecount, frompage, topage);
}

int Fl_Printer::start_page(void)
{
  return printer->start_page();
}

int Fl_Printer::printable_rect(int *w, int *h)
{
  return printer->printable_rect(w, h);
}

void Fl_Printer::margins(int *left, int *top, int *right, int *bottom)
{
  printer->margins(left, top, right, bottom);
}

void Fl_Printer::origin(int *x, int *y)
{
  printer->origin(x, y);
}

void Fl_Printer::origin(int x, int y)
{
  printer->origin(x, y);
}

void Fl_Printer::scale(float scale_x, float scale_y)
{
  printer->scale(scale_x, scale_y);
}

void Fl_Printer::rotate(float angle)
{
  printer->rotate(angle);
}

void Fl_Printer::translate(int x, int y)
{
  printer->translate(x, y);
}

void Fl_Printer::untranslate(void)
{
  printer->untranslate();
}

int Fl_Printer::end_page (void)
{
  return printer->end_page();
}

void Fl_Printer::end_job (void)
{
  printer->end_job();
}

void Fl_Printer::print_widget(Fl_Widget* widget, int delta_x, int delta_y)
{
  printer->draw(widget, delta_x, delta_y);
}

void Fl_Printer::draw_decorated_window(Fl_Window* win, int delta_x, int delta_y)
{
  printer->draw_decorated_window(win, delta_x, delta_y);
}


void Fl_Printer::print_window_part(Fl_Window *win, int x, int y, int w, int h, int delta_x, int delta_y)
{
  printer->print_window_part(win, x, y, w, h, delta_x, delta_y);
}

void Fl_Printer::set_current(void)
{
  printer->set_current();
}

Fl_Printer::~Fl_Printer(void)
{
  delete printer;
}

#endif // NO_PRINT_SUPPORT

//
// End of "$Id$".
//
