//
// Help Viewer widget definitions.
//
// Copyright 1997-2010 by Easy Software Products.
// Image support by Matthias Melcher, Copyright 2000-2009.
// Copyright 2011-2025 by Bill Spitzak and others.
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

#ifndef Fl_Help_View_H
#define Fl_Help_View_H

//
// FLTK header files
//

#include "Fl.H"
#include "Fl_Group.H"
#include "Fl_Scrollbar.H"

//
// System and C++ header files
//

#include <memory>   // std::unique_ptr<>

//
// Forward declarations and typedefs
//

class Fl_Shared_Image;
typedef const char *(Fl_Help_Func)(Fl_Widget *, const char *);

//
// Class declaration
//

/**
  \brief A widget to display formatted text, formatted in a subset of HTML.

  The Fl_Help_View widget displays HTML text. Most HTML 2.0 elements are
  supported, as well as a primitive implementation of tables.
  GIF, JPEG, and PNG images are displayed inline.

  Supported HTML tags:
     - A: HREF/NAME
     - B
     - BODY: BGCOLOR/TEXT/LINK
     - BR
     - CENTER
     - CODE
     - DD
     - DL
     - DT
     - EM
     - FONT: COLOR/SIZE/FACE=(helvetica/arial/sans/times/serif/symbol/courier)
     - H1/H2/H3/H4/H5/H6
     - HEAD
     - HR
     - I
     - IMG: SRC/WIDTH/HEIGHT/ALT
     - KBD
     - LI
     - OL
     - P
     - PRE
     - STRONG
     - TABLE: TH/TD/TR/BORDER/BGCOLOR/COLSPAN/ALIGN=CENTER|RIGHT|LEFT
     - TITLE
     - TT
     - U
     - UL
     - VAR

  Supported color names:
     - black,red,green,yellow,blue,magenta,fuchsia,cyan,aqua,white,gray,grey,lime,maroon,navy,olive,purple,silver,teal.

  Supported urls:
     - Internal: file:
     - External: http: ftp: https: ipp: mailto: news:

  Quoted char names:
     - Aacute aacute Acirc acirc acute AElig aelig Agrave agrave amp Aring aring Atilde atilde Auml auml
     - brvbar bull
     - Ccedil ccedil cedil cent copy curren
     - dagger deg divide
     - Eacute eacute Ecirc ecirc Egrave egrave ETH eth Euml euml euro
     - frac12 frac14 frac34
     - gt
     - Iacute iacute Icirc icirc iexcl Igrave igrave iquest Iuml iuml
     - laquo lt
     - macr micro middot
     - nbsp not Ntilde ntilde
     - Oacute oacute Ocirc ocirc Ograve ograve ordf ordm Oslash oslash Otilde otilde Ouml ouml
     - para permil plusmn pound
     - quot
     - raquo reg
     - sect shy sup1 sup2 sup3 szlig
     - THORN thorn times trade
     - Uacute uacute Ucirc ucirc Ugrave ugrave uml Uuml uuml
     - Yacute yacute
     - yen Yuml yuml

  \note You can't effectively set the box() to FL_NO_BOX, this would result
    in FL_DOWN_BOX being used as the boxtype of the widget. This is unexpected
    but can't be changed for backwards compatibility. If you don't want a frame
    around the widget you can use FL_FLAT_BOX instead.
*/
class FL_EXPORT Fl_Help_View : public Fl_Group
{
private:
  class Impl;
  std::unique_ptr<Impl> impl_;          ///< Pointer to the implementation details

  Fl_Scrollbar  scrollbar_;             ///< Vertical scrollbar for document
  Fl_Scrollbar  hscrollbar_;            ///< Horizontal scrollbar

protected:

  // Widget management

  void          draw() override;

public:

  static const char *copy_menu_text;

  // Widget management

  Fl_Help_View(int xx, int yy, int ww, int hh, const char *l = 0);
  ~Fl_Help_View() override;

  int           handle(int) override;
  void          resize(int,int,int,int) override;
  void          size(int W, int H) { Fl_Widget::size(W, H); }

  // HTML source and raw data

  void          value(const char *val);
  const char    *value() const;
  int           load(const char *f);
  int           find(const char *s, int p = 0);
  void          link(Fl_Help_Func *fn);

  const char    *filename() const;
  const char    *directory() const;
  const char    *title() const;

  // Rendering attributes

  int           size() const;
  void          textcolor(Fl_Color c);
  Fl_Color      textcolor() const;
  void          textfont(Fl_Font f);
  Fl_Font       textfont() const;
  void          textsize(Fl_Fontsize s);
  Fl_Fontsize   textsize() const;
  void          topline(const char *n);
  void          topline(int);
  int           topline() const;
  void          leftline(int);
  int           leftline() const;

  // Text selection

  void          clear_selection();
  void          select_all();
  int           text_selected() const;
  int           copy(int clipboard=1);

  // Scroll bars

  int scrollbar_size() const;
  void scrollbar_size(int newSize);
  /// Return pointer to vertical scrollbar
  Fl_Scrollbar  *scrollbar() { return &scrollbar_; }
  /// Return pointer to horizontal scrollbar
  Fl_Scrollbar  *hscrollbar() { return &hscrollbar_; }

};

#endif // !Fl_Help_View_H
