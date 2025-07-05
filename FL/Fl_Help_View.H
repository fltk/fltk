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
    const char    *start;               // Start of text
    const char    *end;                 // End of text
    uchar         border;               // Draw border?
    Fl_Color      bgcolor;              // Background color
    int           x;                    // Indentation/starting X coordinate
    int           y;                    // Starting Y coordinate
    int           w;                    // Width
    int           h;                    // Height
    int           line[32];             // Left starting position for each line
    int           ol;                   // is ordered list <OL> element
    int           ol_num;               // item number in ordered list
  };

  /** Private class to hold a link with target and its position on screen. */
  struct Link {
    std::string   filename_;            // Filename part of a link
    std::string   target;               // Target part of a link
    Fl_Rect       box;                  // Clickable rectangle that defines the link area
  };

  /** Private font stack element definition. */
  struct Font_Style {
    Fl_Font       f;                    ///< Font
    Fl_Fontsize   s;                    ///< Font Size
    Fl_Color      c;                    ///< Font Color
    Font_Style(Fl_Font afont, Fl_Fontsize asize, Fl_Color acolor);
    Font_Style() = default;             ///< Default constructor
    void          get(Fl_Font &afont, Fl_Fontsize &asize, Fl_Color &acolor);
    void          set(Fl_Font afont, Fl_Fontsize asize, Fl_Color acolor);
  };

  /** Private class to hold font information on a stack. */
  struct Font_Stack {
    void          init(Fl_Font f, Fl_Fontsize s, Fl_Color c);
    void          top(Fl_Font &f, Fl_Fontsize &s, Fl_Color &c);
    void          push(Fl_Font f, Fl_Fontsize s, Fl_Color c);
    void          pop(Fl_Font &f, Fl_Fontsize &s, Fl_Color &c);
    size_t        count() const;
  private:
    std::vector<Font_Style> elts_;    ///< font elements
  };

  enum class Align { RIGHT = -1, CENTER, LEFT };  ///< Alignments
  enum class Mode { DRAW, PUSH, DRAG };           ///< Draw modes

private: // data members

  // HTML source and raw data

  const char    *value_;                ///< Copy of raw HTML text, as set by `value()` or `load()`
  std::string   directory_;             ///< Directory for current document
  std::string   filename_;              ///< Original file name from `load()`

  // HTML document data

  std::string   title_;                 ///< Title string from <title> tag
  Font_Stack    fstack_;                ///< Font and style stack
  std::vector<Text_Block> blocks_;      ///< List of all text blocks on screen
  std::vector<std::shared_ptr<Link>> link_list_;  ///< List of all clickable links and their position on screen
  std::map<std::string, int> target_line_map_;    ///< List of vertical position of all HTML Targets in a document

  int           topline_;               ///< Vertical offset of document, measure in pixels
  int           leftline_;              ///< Horizontal offset of document, measure in pixels
  int           size_;                  ///< Total document height in pixels
  int           hsize_;                 ///< Maximum document width in pixels

  // Default visual attributes

  Fl_Color      defcolor_;              ///< Default text color, defaults to FL_FOREGROUND_COLOR
  Fl_Color      bgcolor_;               ///< Background color, defaults to FL_BACKGROUND_COLOR
  Fl_Color      textcolor_;             ///< Text color, defaults to FL_FOREGROUND_COLOR
  Fl_Color      linkcolor_;             ///< Link color, FL_SELECTION_COLOR
  Fl_Font       textfont_;              ///< Default font for text, FL_TIMES
  Fl_Fontsize   textsize_;              ///< Default font size, defaults to 12, should be FL_NORMAL_SIZE

  // Text selection and mouse handling

  Mode          selection_mode_;        ///< Remember election mode between FL_PUSH, FL_DRAG, and FL_RELEASE
  bool          selected_;              ///< True if there is text selected
  int           selection_first_;       ///< First character of selection, offset in value_
  int           selection_last_;        ///< Last character of selection, offset in value_
  Fl_Color      tmp_selection_color_;   ///< Selection color during draw operation
  Fl_Color      selection_text_color_;  ///< Selection text color during draw operation
  // The following members are static because we need them only once during mouse events
  static int    selection_push_first_;  ///< First character of selection during mouse down
  static int    selection_push_last_;   ///< Last character of selection during mouse down
  static int    selection_drag_first_;  ///< First character of selection during mouse drag
  static int    selection_drag_last_;   ///< Last character of selection during mouse drag
  static Mode   draw_mode_;             ///< Temporarily modify `draw()` method to measure selection start or end during `handle()`
  static int    current_pos_;           ///< Temporarily store text offset while measuring during `handle()`

  // Callback

  Fl_Help_Func  *link_;                 ///< Link transform function

  // Scrollbars

  Fl_Scrollbar  scrollbar_;             ///< Vertical scrollbar for document
  Fl_Scrollbar  hscrollbar_;            ///< Horizontal scrollbar
  int           scrollbar_size_;        ///< Size for both scrollbars

private: // methods

  // HTML source and raw data, getter

  void          free_data();
  std::shared_ptr<Link> find_link(int, int);
  void          follow_link(std::shared_ptr<Link>);

  // HTML interpretation and formatting

  Text_Block    *add_block(const char *s, int xx, int yy, int ww, int hh, uchar border = 0);
  void          add_link(const std::string &link, int xx, int yy, int ww, int hh);
  void          add_target(const std::string &n, int yy);
  int           do_align(Text_Block *block, int line, int xx, Align a, int &l);
  void          format();
  void          format_table(int *table_width, int *columns, const char *table);
  Align         get_align(const char *p, Align a);
  const char    *get_attr(const char *p, const char *n, char *buf, int bufsize);
  Fl_Color      get_color(const char *n, Fl_Color c);
  Fl_Shared_Image *get_image(const char *name, int W, int H);
  int           get_length(const char *l);

  // Font and font stack

  /// Initialize the font stack with default values.
  void          initfont(Fl_Font &f, Fl_Fontsize &s, Fl_Color &c) { f = textfont_; s = textsize_; c = textcolor_; fstack_.init(f, s, c); }
  /// Push the current font and size onto the stack.
  void          pushfont(Fl_Font f, Fl_Fontsize s) {fstack_.push(f, s, textcolor_);}
  /// Push the current font, size, and color onto the stack.
  void          pushfont(Fl_Font f, Fl_Fontsize s, Fl_Color c) {fstack_.push(f, s, c);}
  /// Get the current font, size, and color from the stack.
  void          popfont(Fl_Font &f, Fl_Fontsize &s, Fl_Color &c) {fstack_.pop(f, s, c);}

  // Text selection

  void          hv_draw(const char *t, int x, int y, int entity_extra_length = 0);
  char          begin_selection();
  char          extend_selection();
  void          end_selection();

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
  /** Changes the size of the widget. \see Fl_Widget::size(int, int) */
  void          size(int W, int H) { Fl_Widget::size(W, H); }

  // HTML source and raw data

  void          value(const char *val);
  /** Return a pointer to the internal text buffer. */
  const char    *value() const { return (value_); }
  int           load(const char *f);
  int           find(const char *s, int p = 0);
  void          link(Fl_Help_Func *fn);

  const char    *filename() const;
  const char    *directory() const;
  const char    *title() const;

  // Rendering attributes

  /** Return the document height in pixels. */
  int           size() const { return (size_); }
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
  Fl_Fontsize   textsize() const { return (textsize_); }
  void          topline(const char *n);
  void          topline(int);
  /** Get the current top line in pixels. */
  int           topline() const { return (topline_); }
  void          leftline(int);
  /** Get the left position in pixels. */
  int           leftline() const { return (leftline_); }

  // Text selection

  void          clear_selection();
  void          select_all();
  int           text_selected();
  int           copy(int clipboard=1);

  // Scroll bars

  int scrollbar_size() const;
  void scrollbar_size(int newSize);
};

#endif // !Fl_Help_View_H
