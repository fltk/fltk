//
// "$Id$"
//

#include <FL/Fl.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Tree_Prefs.H>

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
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

// INTERNAL: BUILT IN OPEN/STOW XPMS
//    These can be replaced via prefs.openicon()/closeicon()
//
static const char * const L_open_xpm[] = {
#ifdef __APPLE__
  "11 11 2 1",
  ".  c None",
  "@  c #000000",
  "...@.......",
  "...@@......",
  "...@@@.....",
  "...@@@@....",
  "...@@@@@...",
  "...@@@@@@..",
  "...@@@@@...",
  "...@@@@....",
  "...@@@.....",
  "...@@......",
  "...@......."
#else /* __APPLE__ */
  "11 11 3 1",
  ".	c #fefefe",
  "#	c #444444",
  "@	c #000000",
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
#endif /* __APPLE__ */
};
static Fl_Pixmap L_openpixmap(L_open_xpm);

static const char * const L_close_xpm[] = {
#ifdef __APPLE__
  "11 11 2 1",
  ".  c None",
  "@  c #000000",
  "...........",
  "...........",
  "...........",
  "@@@@@@@@@@@",
  ".@@@@@@@@@.",
  "..@@@@@@@..",
  "...@@@@@...",
  "....@@@....",
  ".....@.....",
  "...........",
  "..........."
#else /* __APPLE__ */
  "11 11 3 1",
  ".	c #fefefe",
  "#	c #444444",
  "@	c #000000",
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
#endif /* __APPLE__ */
};
static Fl_Pixmap L_closepixmap(L_close_xpm);

/// Sets the default icon to be used as the 'open' icon
/// when items are add()ed to the tree.
/// This overrides the built in default '[+]' icon.
///
/// \param[in] val -- The new image, or zero to use the default [+] icon.
///
void Fl_Tree_Prefs::openicon(Fl_Image *val) {
  _openimage = val ? val : &L_openpixmap;
#if FLTK_ABI_VERSION >= 10304
  // Update deactivated version of icon..
  if ( _opendeimage ) delete _opendeimage;
  if ( _openimage ) {
    _opendeimage = _openimage->copy();
    _opendeimage->inactive();
  } else {
    _opendeimage = 0;
  }
#endif
}

/// Sets the icon to be used as the 'close' icon.
/// This overrides the built in default '[-]' icon.
///
/// \param[in] val -- The new image, or zero to use the default [-] icon.
///
void Fl_Tree_Prefs::closeicon(Fl_Image *val) {
  _closeimage = val ? val : &L_closepixmap;
#if FLTK_ABI_VERSION >= 10304
  // Update deactivated version of icon..
  if ( _closedeimage ) delete _closedeimage;
  if ( _closeimage ) {
    _closedeimage = _closeimage->copy();
    _closedeimage->inactive();
  } else {
    _closedeimage = 0;
  }
#endif
}

/// Fl_Tree_Prefs constructor
Fl_Tree_Prefs::Fl_Tree_Prefs() {
  _labelfont              = FL_HELVETICA;
  _labelsize              = FL_NORMAL_SIZE;
  _marginleft             = 6;
  _margintop              = 3;
#if FLTK_ABI_VERSION >= 10301
  _marginbottom           = 20;
#endif /*FLTK_ABI_VERSION*/
  _openchild_marginbottom = 0;
  _usericonmarginleft     = 3;
  _labelmarginleft        = 3;
#if FLTK_ABI_VERSION >= 10301
  _widgetmarginleft       = 3;
#endif /*FLTK_ABI_VERSION*/
  _linespacing            = 0;
  _labelfgcolor           = FL_BLACK;
  _labelbgcolor           = 0xffffffff;		// we use this as 'transparent'
  _connectorcolor         = Fl_Color(43);
#ifdef __APPLE__
  _connectorstyle         = FL_TREE_CONNECTOR_NONE;
#else /* __APPLE__ */
  _connectorstyle         = FL_TREE_CONNECTOR_DOTTED;
#endif /* __APPLE__ */
  _openimage              = &L_openpixmap;
  _closeimage             = &L_closepixmap;
  _userimage              = 0;
#if FLTK_ABI_VERSION >= 10304
  _opendeimage = _openimage->copy();
  _opendeimage->inactive();
  _closedeimage = _closeimage->copy();
  _closedeimage->inactive();
  _userdeimage            = 0;
#endif
  _showcollapse           = 1;
  _showroot               = 1;
  _connectorwidth         = 17;
  _sortorder              = FL_TREE_SORT_NONE;
  _selectbox              = FL_FLAT_BOX;
  _selectmode             = FL_TREE_SELECT_SINGLE;
#if FLTK_ABI_VERSION >= 10301
  _itemreselectmode       = FL_TREE_SELECTABLE_ONCE;
  _itemdrawmode           = FL_TREE_ITEM_DRAW_DEFAULT;
#endif
#if FLTK_ABI_VERSION >= 10303
  _itemdrawcallback       = 0;
  _itemdrawuserdata       = 0;
#endif
  // Let fltk's current 'scheme' affect defaults
  if (Fl::is_scheme("gtk+")) {
    _selectbox = _FL_GTK_THIN_UP_BOX;
  } else if (Fl::is_scheme("plastic")) {
    _selectbox = _FL_PLASTIC_THIN_UP_BOX;
  }
}

#if FLTK_ABI_VERSION >= 10304
/// Fl_Tree_Prefs destructor
Fl_Tree_Prefs::~Fl_Tree_Prefs() {
  if ( _opendeimage )  delete _opendeimage;
  if ( _closedeimage ) delete _closedeimage;
  if ( _userdeimage )  delete _userdeimage;
}
#endif

//
// End of "$Id$".
//
