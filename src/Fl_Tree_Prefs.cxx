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
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Tree_Prefs.H>

// INTERNAL: BUILT IN OPEN/STOW XPMS
//    These can be replaced via prefs.openicon()/closeicon()
//

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

const char * const Fl_System_Driver::tree_open_xpm[] = {
  "11 11 3 1",
  ".    c #fefefe",
  "#    c #444444",
  "@    c #000000",
  "###########",
  "#.........#",
  "#.........#",
  "#....@....#",
  "#....@....#",
  "#..@@@@@..#",
  "#....@....#",
  "#....@....#",
  "#.........#",
  "#.........#",
  "###########"
};

const char * const Fl_System_Driver::tree_close_xpm[] = {
"11 11 3 1",
".      c #fefefe",
"#      c #444444",
"@      c #000000",
"###########",
"#.........#",
"#.........#",
"#.........#",
"#.........#",
"#..@@@@@..#",
"#.........#",
"#.........#",
"#.........#",
"#.........#",
"###########"
};


/**
 Return the address of a pixmap that show a plus in a box.

 This pixmap is used to indicate a brach of a tree that is closed and
 can be opened by clicking it.

 Other platforms may use other symbols which can be reimplemented in the
 driver. Notably, Apple Mac systems mark a closed branch with a triangle
 pointing to the right, and an open branch with a triangle pointing down.
 */
Fl_Pixmap *Fl_System_Driver::tree_openpixmap() {
  static Fl_Pixmap *pixmap = new Fl_Pixmap(tree_open_xpm);
  return pixmap;
}

Fl_Pixmap *Fl_System_Driver::tree_closepixmap() {
  static Fl_Pixmap *pixmap = new Fl_Pixmap(tree_close_xpm);
  return pixmap;
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
  _openimage = val ? val : Fl::system_driver()->tree_openpixmap();
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
  _closeimage = val ? val : Fl::system_driver()->tree_closepixmap();
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
  _connectorcolor         = Fl_Color(43);
  _connectorstyle         = (Fl_Tree_Connector)Fl::system_driver()->tree_connector_style();
  _openimage              = Fl::system_driver()->tree_openpixmap();
  _closeimage             = Fl::system_driver()->tree_closepixmap();
  _userimage              = 0;
  _opendeimage = _openimage->copy();
  _opendeimage->inactive();
  _closedeimage = _closeimage->copy();
  _closedeimage->inactive();
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
