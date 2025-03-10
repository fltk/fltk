//
// Button type header file for the Fast Light Tool Kit (FLTK).
//
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

#ifndef _FL_BUTTON_TYPE_H
#define _FL_BUTTON_TYPE_H

#include "nodes/Widget_Node.h"

/**
 \brief A handler for the simple push button and a base class for all other buttons.
 */
class Button_Node : public Widget_Node
{
  typedef Widget_Node super;
  Fl_Menu_Item *subtypes() FL_OVERRIDE;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE { return "Fl_Button"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Button"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE;
  Widget_Node *_make() FL_OVERRIDE { return new Button_Node(); }
  int is_button() const FL_OVERRIDE { return 1; }
  ID id() const FL_OVERRIDE { return ID_Button; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Button) ? true : super::is_a(inID); }
  void write_properties(fld::io::Project_Writer &f) FL_OVERRIDE;
  void read_property(fld::io::Project_Reader &f, const char *) FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;
};

extern Button_Node Fl_Button_type;


#endif // _FL_BUTTON_TYPE_H
