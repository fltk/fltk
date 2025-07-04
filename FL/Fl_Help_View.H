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
#include "fl_draw.H"
#include "filename.H"

//
// C++ header files
//

#include <map>
#include <vector>
#include <string>
#include <memory>

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
private: // classes, structs, and types

  /** Private struct to describe blocks of text. */
  struct Text_Block {
    const char    *start,         // Start of text
                  *end;           // End of text
    uchar         border;         // Draw border?
    Fl_Color      bgcolor;        // Background color
    int           x,              // Indentation/starting X coordinate
                  y,              // Starting Y coordinate
                  w,              // Width
                  h;              // Height
    int           line[32];       // Left starting position for each line
    int           ol;             // is ordered list <OL> element
    int           ol_num;         // item number in ordered list
  };

  /** Private class to hold a link with target and its position on screen. */
  class Link {
  public:
    std::string filename_;   // Filename part of a link
    std::string target;   // Target part of a link
    Fl_Rect box;          // Clickable rectangle that defines the link area
  };

  /** Private font stack element definition. */
  struct Font_Style {
    Fl_Font      f;  ///< Font
    Fl_Fontsize  s;  ///< Font Size
    Fl_Color     c;  ///< Font Color
    void get(Fl_Font &afont, Fl_Fontsize &asize, Fl_Color &acolor);
    void set(Fl_Font afont, Fl_Fontsize asize, Fl_Color acolor);
    Font_Style(Fl_Font afont, Fl_Fontsize asize, Fl_Color acolor);
    Font_Style() = default; ///< Default constructor
  };

  /** Private class to hold font information on a stack. */
  struct Font_Stack {
    void init(Fl_Font f, Fl_Fontsize s, Fl_Color c);
    void top(Fl_Font &f, Fl_Fontsize &s, Fl_Color &c);
    void push(Fl_Font f, Fl_Fontsize s, Fl_Color c);
    void pop(Fl_Font &f, Fl_Fontsize &s, Fl_Color &c);
    size_t count() const;
  protected:
    std::vector<Font_Style> elts_; ///< font elements
  };

  enum class Align { RIGHT = -1, CENTER, LEFT };    ///< Alignments

private: // data members

  std::string title_;           ///< Title string
  Fl_Color      defcolor_,              ///< Default text color
                bgcolor_,               ///< Background color
                textcolor_,             ///< Text color
                linkcolor_;             ///< Link color
  Fl_Font       textfont_;              ///< Default font for text
  Fl_Fontsize  textsize_;               ///< Default font size
  const char    *value_;                ///< HTML text value
  Font_Stack fstack_;           ///< font stack management
  std::vector<Text_Block> blocks_; ///< Blocks

  Fl_Help_Func  *link_;                 ///< Link transform function

  std::vector<std::shared_ptr<Link>> link_list_; ///< List of links for each line

  std::map<std::string, int> target_line_map_; ///< Map of targets for fast access

  std::string directory_; ///< Directory for current file
  std::string filename_; ///< Current filename

  int           topline_,               ///< Top line in document, measure in pixels
                leftline_,              ///< Lefthand position in pixels
                size_,                  ///< Total document height in pixels
                hsize_,                 ///< Maximum document width
                scrollbar_size_;        ///< Size for both scrollbars
  Fl_Scrollbar  scrollbar_,             ///< Vertical scrollbar for document
                hscrollbar_;            ///< Horizontal scrollbar

  static int    selection_first_;
  static int    selection_last_;
  static int    selection_push_first_;
  static int    selection_push_last_;
  static int    selection_drag_first_;
  static int    selection_drag_last_;
  static int    selected_;
  static int    draw_mode_; // 0 if drawing, 1 if mouse down, 2 if mouse dragged
  static int    mouse_x_;
  static int    mouse_y_;
  static int    current_pos_;
  static Fl_Help_View *current_view_;
  static Fl_Color hv_selection_color_;
  static Fl_Color hv_selection_text_color_;

private: // methods

  void initfont(Fl_Font &f, Fl_Fontsize &s, Fl_Color &c) { f = textfont_; s = textsize_; c = textcolor_; fstack_.init(f, s, c); }
  void pushfont(Fl_Font f, Fl_Fontsize s) {fstack_.push(f, s, textcolor_);}
  void pushfont(Fl_Font f, Fl_Fontsize s, Fl_Color c) {fstack_.push(f, s, c);}
  void popfont(Fl_Font &f, Fl_Fontsize &s, Fl_Color &c) {fstack_.pop(f, s, c);}

  Text_Block *add_block(const char *s, int xx, int yy, int ww, int hh, uchar border = 0);
  void          add_link(const std::string &link, int xx, int yy, int ww, int hh);
  void          add_target(const std::string &n, int yy);
  int           do_align(Text_Block *block, int line, int xx, Align a, int &l);
  void          format();
  void          format_table(int *table_width, int *columns, const char *table);
  void          free_data();
  Align         get_align(const char *p, Align a);
  const char    *get_attr(const char *p, const char *n, char *buf, int bufsize);
  Fl_Color      get_color(const char *n, Fl_Color c);
  Fl_Shared_Image *get_image(const char *name, int W, int H);
  int           get_length(const char *l);

  void          hv_draw(const char *t, int x, int y, int entity_extra_length = 0);
  char          begin_selection();
  char          extend_selection();
  void          end_selection(int c=0);
  void          clear_global_selection();
  std::shared_ptr<Link> find_link(int, int);
  void          follow_link(std::shared_ptr<Link>);

protected:

  void          draw() override;

public:

  static const char *copy_menu_text;

  Fl_Help_View(int xx, int yy, int ww, int hh, const char *l = 0);
  ~Fl_Help_View() override;

  int handle(int) override;
  void resize(int,int,int,int) override;

  const char *filename() const;
  const char *directory() const;
  const char *title() const;

  int           find(const char *s, int p = 0);
  void          link(Fl_Help_Func *fn);
  int           load(const char *f);

  /** Return the document height in pixels. */
  int           size() const { return (size_); }

  /** Changes the size of the widget. \see Fl_Widget::size(int, int) */
  void          size(int W, int H) { Fl_Widget::size(W, H); }

  /** Set the default text color. */
  void          textcolor(Fl_Color c) { if (textcolor_ == defcolor_) textcolor_ = c; defcolor_ = c; }

  /** Return the current default text color. */
  Fl_Color      textcolor() const { return (defcolor_); }

  /** Set the default text font. */
  void          textfont(Fl_Font f) { textfont_ = f; format(); }

  /** Return the default text font. */
  Fl_Font       textfont() const { return (textfont_); }

  /** Set the default text size. */
  void          textsize(Fl_Fontsize s) { textsize_ = s; format(); }

  /** Get the default text size. */

  Fl_Fontsize  textsize() const { return (textsize_); }
  void          topline(const char *n);
  void          topline(int);

  /** Get the current top line in pixels. */
  int           topline() const { return (topline_); }

  void          leftline(int);

  /** Get the left position in pixels. */
  int           leftline() const { return (leftline_); }

  void          value(const char *val);

  //** Return a pointer to the internal text buffer. */
  const char    *value() const { return (value_); }
  void          clear_selection();
  void          select_all();
  int scrollbar_size() const;
  void scrollbar_size(int newSize);
  int text_selected();
  int copy(int clipboard=1);
};

#endif // !Fl_Help_View_H
