//
// "$Id$"
//
// Multi-label header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
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

#ifndef Fl_Multi_Label_H
#define Fl_Multi_Label_H

class Fl_Widget;
struct Fl_Menu_Item;

/** This struct allows multiple labels to be added to objects that might normally have only one label.

    This struct allows a mixed text and/or graphics label to be applied to an object that
    would normally only have a single (usually text only) label.

    Most regular FLTK widgets now support the ability to associate both images and text
    with a label but some special cases, notably the non-widget Fl_Menu_Item objects, do not.
    Fl_Multi_Label may be used to create menu items that have an icon and text, which would
    not normally be possible for an Fl_Menu_Item.
    For example, Fl_Multi_Label is used in the New->Code submenu in fluid, and others.

    Each Fl_Multi_Label holds two elements, labela and labelb; each may hold either a
    text label (const char*) or an image (Fl_Image*). When displayed, labela is drawn first
    and labelb is drawn immediately to its right.

    More complex labels might be constructed by setting labelb as another Fl_Multi_Label and
    thus chaining up a series of label elements.

    When assigning a label element to one of labela or labelb, they should be explicitly cast
    to (const char*) if they are not of that type already.

    \see Fl_Label and Fl_Labeltype
 */
struct FL_EXPORT Fl_Multi_Label {
  /** Holds the "leftmost" of the two elements in the composite label.
      Typically this would be assigned either a text string (const char*),
      a (Fl_Image*) or a (Fl_Multi_Label*). */
  const char* labela;
  /** Holds the "rightmost" of the two elements in the composite label.
      Typically this would be assigned either a text string (const char*),
      a (Fl_Image*) or a (Fl_Multi_Label*). */
  const char* labelb;
  /** Holds the "type" of labela.
    Typically this is set to FL_NORMAL_LABEL for a text label,
    _FL_IMAGE_LABEL for an image (based on Fl_image) or _FL_MULTI_LABEL
    if "chaining" multiple Fl_Multi_Label elements together. */
  uchar typea;
  /** Holds the "type" of labelb.
    Typically this is set to FL_NORMAL_LABEL for a text label,
    _FL_IMAGE_LABEL for an image (based on Fl_image) or _FL_MULTI_LABEL
    if "chaining" multiple Fl_Multi_Label elements together. */
  uchar typeb;

  /** This method is used to associate a Fl_Multi_Label with a Fl_Widget. */
  void label(Fl_Widget*);
  /** This method is used to associate a Fl_Multi_Label with a Fl_Menu_Item. */
  void label(Fl_Menu_Item*);
};

#endif

//
// End of "$Id$".
//
