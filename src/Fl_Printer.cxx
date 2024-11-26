//
// Encompasses platform-specific printing-support code and
// PostScript output code for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2020 by Bill Spitzak and others.
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

#include <FL/Fl_Printer.H>
#include <config.h>

#if defined(FL_NO_PRINT_SUPPORT)
#include <FL/Fl_PostScript.H>
#include <FL/Fl_PDF_File_Surface.H>

Fl_Printer::Fl_Printer(void) {
  printer = NULL;
}
Fl_Paged_Device* Fl_Printer::newPrinterDriver(void) {
  return NULL;
}
int Fl_Printer::begin_job(int pagecount, int *frompage, int *topage, char **perr_message) {return 2;}
int Fl_Printer::begin_page(void) {return 1;}
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
void Fl_Printer::set_current(void) {}
void Fl_PostScript_File_Device::end_current(void) {}
void Fl_PostScript_File_Device::set_current(void) {}
bool Fl_Printer::is_current(void) {return false;}
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

Fl_PostScript_File_Device::Fl_PostScript_File_Device(void) {}
int Fl_PostScript_File_Device::begin_job(int pagecount, int* from, int* to, char **perr_message) {return 2;}
int Fl_PostScript_File_Device::begin_job(int pagecount, enum Fl_Paged_Device::Page_Format format,
                                          enum Fl_Paged_Device::Page_Layout layout) {return 1;}
int Fl_PostScript_File_Device::begin_job(FILE *ps_output, int pagecount, enum Fl_Paged_Device::Page_Format format,
              enum Fl_Paged_Device::Page_Layout layout) {return 1;}
int Fl_PostScript_File_Device::begin_page (void) {return 1;}
int Fl_PostScript_File_Device::printable_rect(int *w, int *h) {return 1;}
void Fl_PostScript_File_Device::margins(int *left, int *top, int *right, int *bottom) {}
void Fl_PostScript_File_Device::origin(int *x, int *y) {}
void Fl_PostScript_File_Device::origin(int x, int y) {}
void Fl_PostScript_File_Device::scale (float scale_x, float scale_y) {}
void Fl_PostScript_File_Device::rotate(float angle) {}
void Fl_PostScript_File_Device::translate(int x, int y) {}
void Fl_PostScript_File_Device::untranslate(void) {}
int Fl_PostScript_File_Device::end_page (void) {return 1;}
void Fl_PostScript_File_Device::end_job(void) {}
FILE* Fl_PostScript_File_Device::file() {return NULL;}
void Fl_PostScript_File_Device::close_command(Fl_PostScript_Close_Command cmd) {}
Fl_PostScript_File_Device::~Fl_PostScript_File_Device(void) {}

Fl_EPS_File_Surface::Fl_EPS_File_Surface(int width, int height, FILE *eps_output,
                                         Fl_Color background, Fl_PostScript_Close_Command closef) : Fl_Widget_Surface(NULL) {}
Fl_EPS_File_Surface::~Fl_EPS_File_Surface() {}
void Fl_EPS_File_Surface::origin(int, int) {}
void Fl_EPS_File_Surface::origin(int*, int*) {}
int Fl_EPS_File_Surface::printable_rect(int*, int*) {return 1;}
void Fl_EPS_File_Surface::translate(int, int) {}
void Fl_EPS_File_Surface::untranslate() {}
FILE* Fl_EPS_File_Surface::file() {return NULL;}
int Fl_EPS_File_Surface::close() {return 1;}

Fl_PDF_File_Surface::Fl_PDF_File_Surface(void) {}
int Fl_PDF_File_Surface::begin_job(const char* defaultfilename, char **perr)  {return 2;}
int Fl_PDF_File_Surface::begin_document(const char* pathname,
              enum Fl_Paged_Device::Page_Format format,
              enum Fl_Paged_Device::Page_Layout layout,
                                        char **perr) {return 2;}
Fl_PDF_File_Surface::~Fl_PDF_File_Surface(void) {}

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
  printer = Fl_Printer::newPrinterDriver();
  driver(printer->driver());
}

int Fl_Printer::begin_job(int pagecount, int *frompage, int *topage, char **perr_message)
{
  return printer->begin_job(pagecount, frompage, topage, perr_message);
}

int Fl_Printer::begin_page(void)
{
  return printer->begin_page();
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

void Fl_Printer::set_current(void)
{
  printer->set_current();
}

bool Fl_Printer::is_current() {
  return surface() == printer;
}

Fl_Printer::~Fl_Printer(void)
{
  delete printer;
}

#endif // defined(FL_NO_PRINT_SUPPORT)
