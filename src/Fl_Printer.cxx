//
// "$Id$"
//
// Encompasses platform-specific printing-support code and 
// PostScript output code for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010 by Bill Spitzak and others.
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
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl_Printer.H>

#ifdef __APPLE__
#include <src/Fl_Quartz_Printer.mm>
#elif defined(WIN32)
#include <src/Fl_GDI_Printer.cxx>
#endif

#include <src/Fl_PS_Printer.cxx>

// print dialog customization strings

const char *Fl_Printer::dialog_title = "Print";
const char *Fl_Printer::dialog_printer = "Printer:";
const char *Fl_Printer::dialog_range = "Print Range";
const char *Fl_Printer::dialog_copies = "Copies";
const char *Fl_Printer::dialog_all = "All";
const char *Fl_Printer::dialog_pages = "Pages";
const char *Fl_Printer::dialog_from = "From:";
const char *Fl_Printer::dialog_to = "To:";
const char *Fl_Printer::dialog_properties = "Properties...";
const char *Fl_Printer::dialog_copyNo = "# Copies:";
const char *Fl_Printer::dialog_print_button = "Print";
const char *Fl_Printer::dialog_cancel_button = "Cancel";
const char *Fl_Printer::dialog_print_to_file = "Print To File";
const char *Fl_Printer::property_title = "Printer Properties";
const char *Fl_Printer::property_pagesize = "Page Size:";
const char *Fl_Printer::property_mode = "Output Mode:";
const char *Fl_Printer::property_use = "Use";
const char *Fl_Printer::property_save = "Save";
const char *Fl_Printer::property_cancel = "Cancel";

const char *Fl_Printer::device_type = "Fl_Printer";

Fl_Device *Fl_Printer::set_current(void)
{
#ifdef __APPLE__
  fl_gc = (CGContextRef)gc;
#elif defined(WIN32)
  fl_gc = (HDC)gc;
#else
  fl_gc = (_XGC*)gc;
#endif
  return this->Fl_Device::set_current();
}

//
// End of "$Id$".
//
