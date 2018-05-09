//
// "$Id$"
//
// PostScript priting support for the Fast Light Tool Kit (FLTK).
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

#include "../../config_lib.h"

#if defined(FL_CFG_PRN_PS) && !defined(FL_NO_PRINT_SUPPORT)

#include <FL/Fl_PostScript.H>
#include <FL/Fl_Printer.H>
#include <FL/fl_ask.H>

#include <src/print_panel.cxx>

/** Support for printing on the Unix/Linux platform */
class Fl_Posix_Printer_Driver : public Fl_PostScript_File_Device {
  virtual int begin_job(int pagecount, int *frompage = NULL, int *topage = NULL);
};

Fl_Paged_Device* Fl_Paged_Device::newPrinterDriver(void)
{
  return new Fl_Posix_Printer_Driver();
}

/*    Begins a print job. */
int Fl_Posix_Printer_Driver::begin_job(int pages, int *firstpage, int *lastpage) {
  enum Fl_Paged_Device::Page_Format format;
  enum Fl_Paged_Device::Page_Layout layout;
  
  // first test version for print dialog
  if (!print_panel) make_print_panel();
  printing_style style = print_load();
  print_selection->deactivate();
  print_all->setonly();
  print_all->do_callback();
  print_from->value("1");
  { char tmp[10]; snprintf(tmp, sizeof(tmp), "%d", pages); print_to->value(tmp); }
  print_panel->show(); // this is modal
  while (print_panel->shown()) Fl::wait();
  
  if (!print_start) // user clicked cancel
    return 1;
  
  // get options
  
  switch (print_page_size->value()) {
    case 0:
      format = Fl_Paged_Device::LETTER;
      break;
    case 2:
      format = Fl_Paged_Device::LEGAL;
      break;
    case 3:
      format = Fl_Paged_Device::EXECUTIVE;
      break;
    case 4:
      format = Fl_Paged_Device::A3;
      break;
    case 5:
      format = Fl_Paged_Device::A5;
      break;
    case 6:
      format = Fl_Paged_Device::B5;
      break;
    case 7:
      format = Fl_Paged_Device::ENVELOPE;
      break;
    case 8:
      format = Fl_Paged_Device::DLE;
      break;
    default:
      format = Fl_Paged_Device::A4;
  }
  
  { // page range choice
    int from = 1, to = pages;
    if (print_pages->value()) {
      sscanf(print_from->value(), "%d", &from);
      sscanf(print_to->value(), "%d", &to);
    }
    if (from < 1) from = 1;
    if (to > pages) to = pages;
    if (to < from) to = from;
    if (firstpage) *firstpage = from;
    if (lastpage) *lastpage = to;
    if (pages > 0) pages = to - from + 1;
  }
  
  if (print_output_mode[0]->value()) layout = Fl_Paged_Device::PORTRAIT;
  else if (print_output_mode[1]->value()) layout = Fl_Paged_Device::LANDSCAPE;
  else if (print_output_mode[2]->value()) layout = Fl_Paged_Device::PORTRAIT;
  else layout = Fl_Paged_Device::LANDSCAPE;
  
  int print_pipe = print_choice->value();	// 0 = print to file, >0 = printer (pipe)
  
  const char *media = print_page_size->text(print_page_size->value());
  const char *printer = (const char *)print_choice->menu()[print_choice->value()].user_data();
  if (!print_pipe) printer = "<File>";
  
  if (!print_pipe) // fall back to file printing
    return Fl_PostScript_File_Device::begin_job (pages, format, layout);
  
  // Print: pipe the output into the lp command...
  
  char command[1024];
  if (style == SystemV) snprintf(command, sizeof(command), "lp -s -d %s -n %d -t '%s' -o media=%s",
                                 printer, print_collate_button->value() ? 1 : (int)(print_copies->value() + 0.5), "FLTK", media);
  else snprintf(command, sizeof(command), "lpr -h -P%s -#%d -T FLTK ",
                printer, print_collate_button->value() ? 1 : (int)(print_copies->value() + 0.5));
  
  Fl_PostScript_Graphics_Driver *ps = driver();
  ps->output = popen(command, "w");
  if (!ps->output) {
    fl_alert("could not run command: %s\n",command);
    return 1;
  }
  ps->close_command(pclose);
  this->set_current();
  return ps->start_postscript(pages, format, layout); // start printing
}

#endif // defined(FL_CFG_PRN_PS) && !defined(FL_NO_PRINT_SUPPORT)


//
// End of "$Id$".
//
