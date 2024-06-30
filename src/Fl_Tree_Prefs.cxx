//

//////////////////////
// Fl_Tree_Prefs.cxx
//////////////////////
//
// Fl_Tree -- This file is part of the Fl_Tree widget for FLTK
// Copyright (C) 2009-2010 by Greg Ercolano.
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

#include <config.h>

#include "Fl_System_Driver.H"
#include <FL/Fl.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Tree_Prefs.H>
#include <FL/fl_draw.H>

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

// Draw non-OS specific Fl_Tree open/close icons
//    ┌───┐   ┌───┐
//    │ + │   │ - │
//    └───┘   └───┘
void Fl_System_Driver::tree_draw_expando_button(int x, int y, bool state, bool active) {
  fl_rectf(x, y, 11, 11, active ? FL_BACKGROUND2_COLOR : fl_inactive(FL_BACKGROUND2_COLOR)); // fill
  fl_rect(x, y, 11, 11, FL_INACTIVE_COLOR);             // outline
  fl_color(active ? FL_FOREGROUND_COLOR : FL_INACTIVE_COLOR);
  fl_line(x + 3, y + 5, x + 7, y + 5);                  // horiz line
  if (state) fl_line(x + 5, y + 3, x + 5, y + 7);       // vert line
}

int Fl_System_Driver::tree_connector_style() {
  return FL_TREE_CONNECTOR_DOTTED;
}

/**
 \}
 \endcond
 */


/// Sets the default icon to be used as the 'open' icon
/// when items are add()ed to the tree.
/// This overrides the built in default '[+]' icon.
///
/// \param[in] val -- The new image, or zero to use the default [+] icon.
///
void Fl_Tree_Prefs::openicon(Fl_Image *val) {
  _openimage = val ? val : 0;
  // Update deactivated version of icon..
  if ( _opendeimage ) delete _opendeimage;
  if ( _openimage ) {
    _opendeimage = _openimage->copy();
    _opendeimage->inactive();
  } else {
    _opendeimage = 0;
  }
}

/// Sets the icon to be used as the 'close' icon.
/// This overrides the built in default '[-]' icon.
///
/// \param[in] val -- The new image, or zero to use the default [-] icon.
///
void Fl_Tree_Prefs::closeicon(Fl_Image *val) {
  _closeimage = val ? val : 0;
  // Update deactivated version of icon..
  if ( _closedeimage ) delete _closedeimage;
  if ( _closeimage ) {
    _closedeimage = _closeimage->copy();
    _closedeimage->inactive();
  } else {
    _closedeimage = 0;
  }
}

/// Fl_Tree_Prefs constructor
Fl_Tree_Prefs::Fl_Tree_Prefs() {
  _labelfont              = FL_HELVETICA;
  _labelsize              = FL_NORMAL_SIZE;
  _marginleft             = 6;
  _margintop              = 3;
  _marginbottom           = 20;
  _openchild_marginbottom = 0;
  _usericonmarginleft     = 3;
  _labelmarginleft        = 3;
  _widgetmarginleft       = 3;
  _linespacing            = 0;
  _labelfgcolor           = FL_FOREGROUND_COLOR;
  _labelbgcolor           = 0xffffffff;         // we use this as 'transparent'
  _connectorcolor         = FL_INACTIVE_COLOR;
  _connectorstyle         = (Fl_Tree_Connector)Fl::system_driver()->tree_connector_style();
  _openimage              = 0;
  _closeimage             = 0;
  _userimage              = 0;
  _opendeimage            = 0;
  _closedeimage           = 0;
  _userdeimage            = 0;
  _showcollapse           = 1;
  _showroot               = 1;
  _connectorwidth         = 17;
  _sortorder              = FL_TREE_SORT_NONE;
  _selectbox              = FL_THIN_UP_BOX;
  _selectmode             = FL_TREE_SELECT_SINGLE;
  _itemreselectmode       = FL_TREE_SELECTABLE_ONCE;
  _itemdrawmode           = FL_TREE_ITEM_DRAW_DEFAULT;
  _itemdrawcallback       = 0;
  _itemdrawuserdata       = 0;
}

/// Fl_Tree_Prefs destructor
Fl_Tree_Prefs::~Fl_Tree_Prefs() {
  if ( _opendeimage )  delete _opendeimage;
  if ( _closedeimage ) delete _closedeimage;
  if ( _userdeimage )  delete _userdeimage;
}
