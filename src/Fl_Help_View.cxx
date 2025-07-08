//
// Fl_Help_View widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1997-2010 by Easy Software Products.
// Image support by Matthias Melcher, Copyright 2000-2009.
//
// Buffer management (HV_Edit_Buffer) and more by AlbrechtS and others.
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

//
// FLTK header files
//

#include <FL/Fl_Help_View.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/fl_utf8.h>
#include <FL/filename.H>                // fl_open_uri()
#include <FL/fl_string_functions.h>     // fl_strdup()
#include <FL/fl_draw.H>
#include <FL/filename.H>
#include "flstring.h"

//
// System and C++ header files
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <map>
#include <vector>
#include <string>

//
// Debugging
//

// Debug: set to 1 for basic debugging, 2 for more, 0 for none
#define DEBUG_EDIT_BUFFER 0
#if (DEBUG_EDIT_BUFFER > 1)
#define DEBUG_FUNCTION(L,F) \
  printf("\n========\n  [%d] --- %s\n========\n", L, F); \
  fflush(stdout);
#else
#define DEBUG_FUNCTION(L,F)
#endif

//
// Constants
//

static constexpr int MAX_COLUMNS = 200;

//
// Implementation class
//

class Fl_Help_View::Impl
{
  friend Fl_Help_View;
  Fl_Help_View &view;
public:
  Impl(Fl_Help_View *view) : view(*view)
  {
    title_[0]     = '\0';
    defcolor_     = FL_FOREGROUND_COLOR;
    bgcolor_      = FL_BACKGROUND_COLOR;
    textcolor_    = FL_FOREGROUND_COLOR;
    linkcolor_    = FL_SELECTION_COLOR;
    textfont_     = FL_TIMES;
    textsize_     = 12;
    value_        = nullptr;

    blocks_.clear();

    link_         = (Fl_Help_Func *)0;

    link_list_.clear();

    directory_.clear();
    filename_.clear();

    topline_      = 0;
    leftline_     = 0;
    size_         = 0;
    hsize_        = 0;

    selection_mode_ = Mode::DRAW;
    selected_ = false;
    selection_first_ = 0;
    selection_last_ = 0;

    scrollbar_size_ = 0;
  }
  ~Impl()
  {
    clear_selection();
    free_data();
  }

  private: // classes, structs, and types

  /** Helper class to manage margins in Fl_Help_View. */
  class Margin_Stack
  {
    std::vector<int> margins_;
  public:
    Margin_Stack() = default;
    void clear();
    int current() const;
    int pop();
    int push(int indent);
  };

  /** Internal class to manage the Fl_Help_View edit buffer.
     This class is for internal use in this file. Its sole purpose is to
     allow buffer management to avoid buffer overflows in stack variables
     used to edit strings for formatting and drawing (STR #3275).
   */
  class Edit_Buffer : public std::string {
  public:
    void add(int ucs);
    int cmp(const char *str);
    int width() const;
  };

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
  std::vector<std::shared_ptr<Link> > link_list_; ///< List of all clickable links and their position on screen
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

  void          draw();

public:

  static const char *copy_menu_text;

  // Widget management

  int           handle(int);
  void          resize(int,int,int,int);
  /** Changes the size of the widget. \see Fl_Widget::size(int, int) */

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
  int           text_selected() const;
  int           copy(int clipboard=1);

  // Scroll bars

  int scrollbar_size() const;
  void scrollbar_size(int newSize);
};


//
// global and class static values
//

static char initial_load = 0;
// We don't put the offscreen buffer in the help view class because
// we'd need to include platform.H in the header...
static Fl_Offscreen fl_help_view_buffer;
int Fl_Help_View::Impl::selection_push_first_ = 0;
int Fl_Help_View::Impl::selection_push_last_ = 0;
int Fl_Help_View::Impl::selection_drag_first_ = 0;
int Fl_Help_View::Impl::selection_drag_last_ = 0;
Fl_Help_View::Impl::Mode Fl_Help_View::Impl::draw_mode_ = Mode::DRAW;
int Fl_Help_View::Impl::current_pos_ = 0;

//
// Local functions declarations, implementations are at the end of the file
//

static int quote_char(const char *);
static std::string to_lower(const std::string &str);
static size_t url_scheme(const std::string &url, bool skip_slashes=false);
static const char *vanilla(const char *p, const char *end);
static uint32_t command(const char *cmd);

static constexpr uint32_t CMD(char a, char b, char c, char d)
{
  return ((a<<24)|(b<<16)|(c<<8)|d);
}

//
// Static data.
//

// Broken Image
static const char * const broken_xpm[] =
                {
                  "16 24 4 1",
                  "@ c #000000",
                  "  c #ffffff",
                  "+ c none",
                  "x c #ff0000",
                  // pixels
                  "@@@@@@@+++++++++",
                  "@    @++++++++++",
                  "@   @+++++++++++",
                  "@   @++@++++++++",
                  "@    @@+++++++++",
                  "@     @+++@+++++",
                  "@     @++@@++++@",
                  "@ xxx  @@  @++@@",
                  "@  xxx    xx@@ @",
                  "@   xxx  xxx   @",
                  "@    xxxxxx    @",
                  "@     xxxx     @",
                  "@    xxxxxx    @",
                  "@   xxx  xxx   @",
                  "@  xxx    xxx  @",
                  "@ xxx      xxx @",
                  "@              @",
                  "@              @",
                  "@              @",
                  "@              @",
                  "@              @",
                  "@              @",
                  "@              @",
                  "@@@@@@@@@@@@@@@@",
                  nullptr
                };

static Fl_Pixmap broken_image(broken_xpm);

/** This text may be customized at run-time. */
const char *Fl_Help_View::copy_menu_text = "Copy";

static Fl_Menu_Item rmb_menu[] = {
  { nullptr, 0, nullptr, (void*)1 },  // Copy
  { nullptr }
};

//
// Implementation
//


// ---- Helper class to manage margins in Fl_Help_View

void Fl_Help_View::Impl::Margin_Stack::clear() {
  margins_.clear();
  margins_.push_back(4); // default margin
}

int Fl_Help_View::Impl::Margin_Stack::current() const {
  return margins_.back();
}

int Fl_Help_View::Impl::Margin_Stack::pop() {
  if (margins_.size() > 1) {
    margins_.pop_back();
  }
  return margins_.back();
}

int Fl_Help_View::Impl::Margin_Stack::push(int indent) {
  int xx = current() + indent;
  margins_.push_back(xx);
  return xx;
}


// ---- Helper class HV_Edit_Buffer to ease buffer management

// Append one Unicode character (code point) to the buffer.
// The Unicode character \p ucs is converted to UTF-8 and appended to
// the buffer.
// \param[in]  ucs  Unicode character (code point) to be added
void Fl_Help_View::Impl::Edit_Buffer::add(int ucs){
  int len;
  char cbuf[6];
  len = fl_utf8encode((unsigned int)ucs, cbuf); // always returns value >= 1
  append(cbuf, len);
}

// case insensitive comparison of buffer contents with a string
int Fl_Help_View::Impl::Edit_Buffer::cmp(const char *str) {
  return !strcasecmp(c_str(), str);
}

// string width of the entire buffer contents
int Fl_Help_View::Impl::Edit_Buffer::width() const {
  return (int)fl_width(c_str());
}


// ---- Implementation of Font_Style class methods

/**
  \brief Constructs a Font_Style object with the specified font, size, and color.
  \param afont The font to use.
  \param asize The font size.
  \param acolor The font color.
*/
Fl_Help_View::Impl::Font_Style::Font_Style(Fl_Font afont, Fl_Fontsize asize, Fl_Color acolor) {
  set(afont, asize, acolor);
}


/**
  \brief Retrieves the font, font size, and color settings from this Font_Style instance.
  \param[out] afont  Reference to a variable where the font will be stored.
  \param[out] asize  Reference to a variable where the font size will be stored.
  \param[out] acolor Reference to a variable where the font color will be stored.
 */
void Fl_Help_View::Impl::Font_Style::get(Fl_Font &afont, Fl_Fontsize &asize, Fl_Color &acolor) {
  afont=f; asize=s; acolor=c;
}


/**
  \brief Sets the font, font size, and color for the Font_Style.
  This only set the members of the class, but does not change the current
  rendering settings.
  \param afont   The font to be used.
  \param asize   The font size to be set.
  \param acolor  The color to be applied to the font.
*/
void Fl_Help_View::Impl::Font_Style::set(Fl_Font afont, Fl_Fontsize asize, Fl_Color acolor) {
  f=afont; s=asize; c=acolor;
}


// ---- Implementation of Font_Stack class methods

/**
  \brief Initializes the font stack with a default font, size, and color.
  Clears the stack and pushes one element with a default font, size, and color.
  \param[in] f font to apply
  \param[in] s font size to apply
  \param[in] c color to apply
 */
void Fl_Help_View::Impl::Font_Stack::init(Fl_Font f, Fl_Fontsize s, Fl_Color c) {
  elts_.clear();
  push(f, s, c);
}


/**
  \brief Gets the top (current) element on the stack.
  \param[out] f font to apply
  \param[out] s font size to apply
  \param[out] c color to apply
  \note This function does not pop the stack, it just returns the top element.
  */
void Fl_Help_View::Impl::Font_Stack::top(Fl_Font &f, Fl_Fontsize &s, Fl_Color &c) {
  elts_.back().get(f, s, c);
}


/**
  \brief Push the font style triplet on the stack.
  Also calls fl_font() and fl_color().
  \param[in] f font to apply
  \param[in] s font size to apply
  \param[in] c color to apply
 */
void Fl_Help_View::Impl::Font_Stack::push(Fl_Font f, Fl_Fontsize s, Fl_Color c) {
  elts_.push_back(Font_Style(f, s, c));
  fl_font(f, s);
  fl_color(c);
}


/**
  \brief Pop style form the stack and apply new top style.
  Pops the font style triplet from the stack and calls fl_font()
  and fl_color().
  \param[out] f font to apply
  \param[out] s font size to apply
  \param[out] c color to apply
  \note If the stack has only one element left, that element will not be popped,
      but the top element will be applied again.
 */
void Fl_Help_View::Impl::Font_Stack::pop(Fl_Font &f, Fl_Fontsize &s, Fl_Color &c) {
  if (elts_.size() > 1)
    elts_.pop_back();
  top(f, s, c);
  fl_font(f, s);
  fl_color(c);
}


/**
  \brief Gets the current count of font style elements in the stack.
  \return stack size in number of elements
  */
size_t Fl_Help_View::Impl::Font_Stack::count() const {
  return elts_.size();
}


// ------ Fl_Help_View Private methods

// ---- HTML source and raw data, getter

/**
  \brief Frees memory used for the document.
  */
void Fl_Help_View::Impl::free_data() {
  // Release all images...
  if (value_) {
    const char  *ptr,           // Pointer into block
                *attrs;         // Pointer to start of element attributes
    Edit_Buffer buf;            // Text buffer
    char        attr[1024],     // Attribute buffer
                wattr[1024],    // Width attribute buffer
                hattr[1024];    // Height attribute buffer

    DEBUG_FUNCTION(__LINE__,__FUNCTION__);

    for (ptr = value_; *ptr;)
    {
      if (*ptr == '<')
      {
        ptr ++;

        if (strncmp(ptr, "!--", 3) == 0)
        {
          // Comment...
          ptr += 3;
          if ((ptr = strstr(ptr, "-->")) != nullptr)
          {
            ptr += 3;
            continue;
          }
          else
            break;
        }

        buf.clear();

        while (*ptr && *ptr != '>' && !isspace((*ptr)&255))
          buf += *ptr++;

        attrs = ptr;
        while (*ptr && *ptr != '>')
          ptr ++;

        if (*ptr == '>')
          ptr ++;

        if (buf.cmp("IMG"))
        {
          Fl_Shared_Image       *img;
          int           width;
          int           height;

          get_attr(attrs, "WIDTH", wattr, sizeof(wattr));
          get_attr(attrs, "HEIGHT", hattr, sizeof(hattr));
          width  = get_length(wattr);
          height = get_length(hattr);

          if (get_attr(attrs, "SRC", attr, sizeof(attr))) {
            // Get and release the image to free it from memory...
            img = get_image(attr, width, height);
            if ((void*)img != &broken_image) {
              img->release();
            }
          }
        }
      }
      else
        ptr++;
    }

    free((void *)value_);
    value_ = 0;
  }

  blocks_ .clear();
  link_list_.clear();
  target_line_map_.clear();
}


/**
  \brief Find out if the mouse is over a hyperlink and return the link data.
  \parm[in] xx, yy Pixel coordinates inside the widget.
  \return Shared pointer to the link if found, nullptr otherwise.
 */
std::shared_ptr<Fl_Help_View::Impl::Link> Fl_Help_View::Impl::find_link(int xx, int yy)
{
  for (auto &link : link_list_) {
    if (link->box.contains(xx, yy)) {
      return link;
    }
  }
  return nullptr;
}


/**
  \brief Follow a link and load the target document or scroll to the target.
  This function clears the current selection and loads a new document or
  scrolls to a target line in the current document.
  \param linkp Shared pointer to the link to follow.
 */
void Fl_Help_View::Impl::follow_link(std::shared_ptr<Link> linkp)
{
  clear_selection();
  view.set_changed();
  std::string target = linkp->target;;     // Current target

  if ( (linkp->filename_ != filename_) && !linkp->filename_.empty() ) {
    // Load the new document, if the filename is different
    std::string url;
    size_t directory_scheme_length = url_scheme(directory_);
    size_t filename_scheme_length = url_scheme(linkp->filename_);
    if ( (directory_scheme_length > 0) && (filename_scheme_length == 0) ) {
      // If directory_ starts with a scheme (e.g.ftp:), but linkp->filename_ does not:
      if (linkp->filename_[0] == '/') {
        // If linkp->filename_ is absolute...
        url = directory_.substr(0, directory_scheme_length) + linkp->filename_;;
      } else {
        // If linkp->filename_ is relative, the URL is the directory_ plus the filename
        url = directory_ + "/" + linkp->filename_;
      }
    } else if (linkp->filename_[0] != '/' && (filename_scheme_length == 0)) {
      // If the filename is relative and does not start with a scheme (ftp: , etc.)...
      if (!directory_.empty()) {
        // If we have a current directory, use that as the base for the URL
        url = directory_ + "/" + linkp->filename_;
      } else {
        // If we do not have a current directory, use the application's current working directory
        char dir[FL_PATH_MAX];       // Current directory (static size ok until we have fl_getcwd_std()
        fl_getcwd(dir, sizeof(dir));
        url = "file:" + std::string(dir) + "/" + linkp->filename_;
      }
    } else {
      // If the filename is absolute or starts with a protocol (e.g.ftp:), use it as is
      url = linkp->filename_;
    }

    // If a target is specified, append it to the URL
    if (!linkp->target.empty()) {
      url += "#" + linkp->target;
    }

    load(url.c_str());

  } else if (!target.empty()) {
    // Keep the same document, scroll to the target line
    topline(target.c_str());
  } else {
    // No target, no filename, just scroll to the top of the document
    topline(0);
  }

  // Scroll the content horizontally to the left
  leftline(0);
}

// ---- HTML interpretation and formatting

/**
  \brief Adds a text block to the list.
  \param[in] s Pointer to start of block text
  \param[in] xx X position of block
  \param[in] yy Y position of block
  \param[in] ww Right margin of block
  \param[in] hh Height of block
  \param[in] border Draw border?
  \return Pointer to the new block in the list.
  */
Fl_Help_View::Impl::Text_Block *Fl_Help_View::Impl::add_block(
  const char *s,
  int xx, int yy, int ww, int hh,
  unsigned char border)
{
  Text_Block *temp;                          // New block

  // printf("add_block(s = %p, xx = %d, yy = %d, ww = %d, hh = %d, border = %d)\n",
  //        s, xx, yy, ww, hh, border);

  blocks_.push_back(Text_Block()); // Add a new block to the vector
  temp = &blocks_.back();

  memset(temp, 0, sizeof(Text_Block));
  temp->start   = s;
  temp->end     = s;
  temp->x       = xx;
  temp->y       = yy;
  temp->w       = ww;
  temp->h       = hh;
  temp->border  = border;
  temp->bgcolor = bgcolor_;

  return (temp);
}


/**
  \brief Add a new link and its postion on screen to the link list.
  \param[in] link a filename, followed by a hash and a target. All parts are optional.
  \param[in] xx, yy, ww, hh bounding box of the link text on screen
 */
void Fl_Help_View::Impl::add_link(const std::string &link, int xx, int yy, int ww, int hh)
{
  auto new_link = std::make_shared<Link>(); // Create a new link storage object.

  new_link->box = { xx, yy, ww, hh };

  size_t hash_pos = link.find('#'); // Find the hash character
  if (hash_pos != std::string::npos) {
    // If a '#' is found, split the link into filename and target
    new_link->filename_ = link.substr(0, hash_pos);
    new_link->target = link.substr(hash_pos + 1);
  } else {
    // No '#' found, use the whole link as filename
    new_link->filename_ = link;
    new_link->target.clear();
  }

  link_list_.push_back(new_link);  // Add the link to the list.
}


/**
  \brief Adds a new target to the list.
  \param[in] n  Name of target (string)
  \param[in] yy line number of target position
 */
void Fl_Help_View::Impl::add_target(const std::string &n, int yy)
{
  std::string target = to_lower(n); // Convert target name to lower case
  target_line_map_[target] = yy; // Store the target line in the map
}


/**
  \brief Computes the alignment for a line in a block.
  \param[in] block Pointer to the block to add to
  \param[in] line Current line number in the block
  \param[in] xx Current X position in the block
  \param[in] a Current alignment
  \param[in,out] l Starting link index for alignment adjustment
  \return The new line number after alignment adjustment
 */
int Fl_Help_View::Impl::do_align(
  Text_Block *block,
  int line,
  int xx,
  Align a,
  int &l)
{
  int   offset;                                 // Alignment offset

  switch (a)
  {
    case Align::RIGHT:        // Right align
        offset = block->w - xx;
        break;
    case Align::CENTER:       // Center
        offset = (block->w - xx) / 2;
        break;
    default:           // Left align
        offset = 0;
        break;
  }

  block->line[line] = block->x + offset;

  if (line < 31)
    line ++;

  while (l < (int)link_list_.size()) {
    link_list_[l]->box.x( link_list_[l]->box.x() + offset);
    l++;
  }

  return (line);
}


/**
  \brief Formats the help text and lays out the HTML content for display.

  This function parses the HTML-like text buffer, breaks it into blocks and lines,
  computes positions and sizes for each text and image element, manages links and targets,
  and sets up the scrolling and rendering parameters for the widget.

  The main algorithm consists of an outer loop that may repeat if the computed content
  exceeds the available width (to adjust hsize_), and an inner loop that parses the text,
  handles tags, manages formatting state, and builds the layout structures.
*/
void Fl_Help_View::Impl::format() {
  int           i;              // Looping var
  int           done;           // Are we done yet?
  Text_Block *block,         // Current block
                *cell;          // Current table cell
  int           cells[MAX_COLUMNS],
                                // Cells in the current row...
                row;            // Current table row (block number)
  const char    *ptr,           // Pointer into block
                *start,         // Pointer to start of element
                *attrs;         // Pointer to start of element attributes
  Edit_Buffer   buf;            // Text buffer
  char          attr[1024],     // Attribute buffer
                wattr[1024],    // Width attribute buffer
                hattr[1024],    // Height attribute buffer
                linkdest[1024]; // Link destination
  int           xx, yy, ww, hh; // Size of current text fragment
  int           line;           // Current line in block
  int           links;          // Links for current line
  Fl_Font       font;
  Fl_Fontsize   fsize;          // Current font and size
  Fl_Color      fcolor;         // Current font color
  unsigned char border;         // Draw border?
  Align talign;         // Current alignment
  Align newalign;       // New alignment
  int           head,           // In the <HEAD> section?
                pre,            // <PRE> text?
                needspace;      // Do we need whitespace?
  int           table_width,    // Width of table
                table_offset;   // Offset of table
  int           column,         // Current table column number
                columns[MAX_COLUMNS];
                                // Column widths
  Fl_Color      tc, rc;         // Table/row background color
  Fl_Boxtype    b = view.box() ? view.box() : FL_DOWN_BOX;
                                // Box to draw...
  Margin_Stack  margins;        // Left margin stack...
  std::vector<int> OL_num;         // if nonnegative, in OL mode and this is the item number

  OL_num.push_back(-1);

  DEBUG_FUNCTION(__LINE__,__FUNCTION__);

  // Reset document width...
  int scrollsize = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size();
  hsize_ = view.w() - scrollsize - Fl::box_dw(b);

  done = 0;
  while (!done)
  {
    // Reset state variables...
    done       = 1;
    blocks_.clear();
    link_list_.clear();
    target_line_map_.clear();
    size_      = 0;
    bgcolor_   = view.color();
    textcolor_ = textcolor();
    linkcolor_ = fl_contrast(FL_BLUE, view.color());

    tc = rc = bgcolor_;

    title_ = "Untitled";

    if (!value_)
      return;

    // Setup for formatting...
    initfont(font, fsize, fcolor);

    line         = 0;
    links        = 0;
    margins.clear();
    xx           = 4;
    yy           = fsize + 2;
    ww           = 0;
    column       = 0;
    border       = 0;
    hh           = 0;
    block        = add_block(value_, xx, yy, hsize_, 0);
    row          = 0;
    head         = 0;
    pre          = 0;
    talign       = Align::LEFT;
    newalign     = Align::LEFT;
    needspace    = 0;
    linkdest[0]  = '\0';
    table_offset = 0;

    // Html text character loop
    for (ptr = value_, buf.clear(); *ptr;)
    {
      // End of word?
      if ((*ptr == '<' || isspace((*ptr)&255)) && buf.size() > 0)
      {
        // Get width of word parsed so far...
        ww = buf.width();

        if (!head && !pre)
        {
          // Check width...
          if (ww > hsize_) {
            hsize_ = ww;
            done   = 0;
            break;
          }

          if (needspace && xx > block->x)
            ww += (int)fl_width(' ');

  //        printf("line = %d, xx = %d, ww = %d, block->x = %d, block->w = %d\n",
  //           line, xx, ww, block->x, block->w);

          if ((xx + ww) > block->w)
          {
            line     = do_align(block, line, xx, newalign, links);
            xx       = block->x;
            yy       += hh;
            block->h += hh;
            hh       = 0;
          }

          if (linkdest[0])
            add_link(linkdest, xx, yy - fsize, ww, fsize);

          xx += ww;
          if ((fsize + 2) > hh)
            hh = fsize + 2;

          needspace = 0;
        }
        else if (pre)
        {
          // Add a link as needed...
          if (linkdest[0])
            add_link(linkdest, xx, yy - hh, ww, hh);

          xx += ww;
          if ((fsize + 2) > hh)
            hh = fsize + 2;

          // Handle preformatted text...
          while (isspace((*ptr)&255))
          {
            if (*ptr == '\n')
            {
              if (xx > hsize_) break;

              line     = do_align(block, line, xx, newalign, links);
              xx       = block->x;
              yy       += hh;
              block->h += hh;
              hh       = fsize + 2;
            }
            else
              xx += (int)fl_width(' ');

            if ((fsize + 2) > hh)
              hh = fsize + 2;

            ptr ++;
          }

          if (xx > hsize_) {
            hsize_ = xx;
            done   = 0;
            break;
          }

          needspace = 0;
        }
        else
        {
          // Handle normal text or stuff in the <HEAD> section...
          while (isspace((*ptr)&255))
            ptr ++;
        }

        buf.clear();
      }

      if (*ptr == '<')
      {
        // Handle html tags..
        start = ptr;
        ptr ++;

        if (strncmp(ptr, "!--", 3) == 0)
        {
          // Comment...
          ptr += 3;
          if ((ptr = strstr(ptr, "-->")) != nullptr)
          {
            ptr += 3;
            continue;
          }
          else
            break;
        }

        while (*ptr && *ptr != '>' && !isspace((*ptr)&255))
          buf += *ptr++;

        attrs = ptr;
        while (*ptr && *ptr != '>')
          ptr ++;

        if (*ptr == '>')
          ptr ++;

        if (buf.cmp("HEAD"))
          head = 1;
        else if (buf.cmp("/HEAD"))
          head = 0;
        else if (buf.cmp("TITLE"))
        {
          // Copy the title in the document...
          title_.clear();
          for ( ; *ptr != '<' && *ptr; ptr++) {
            title_.push_back(*ptr);
          }
          buf.clear();
        }
        else if (buf.cmp("A"))
        {
          if (get_attr(attrs, "NAME", attr, sizeof(attr)) != nullptr)
            add_target(attr, yy - fsize - 2);

          if (get_attr(attrs, "HREF", attr, sizeof(attr)) != nullptr)
            strlcpy(linkdest, attr, sizeof(linkdest));
        }
        else if (buf.cmp("/A"))
          linkdest[0] = '\0';
        else if (buf.cmp("BODY"))
        {
          bgcolor_   = get_color(get_attr(attrs, "BGCOLOR", attr, sizeof(attr)),
                                 view.color());
          textcolor_ = get_color(get_attr(attrs, "TEXT", attr, sizeof(attr)),
                                 textcolor());
          linkcolor_ = get_color(get_attr(attrs, "LINK", attr, sizeof(attr)),
                                 fl_contrast(FL_BLUE, view.color()));
        }
        else if (buf.cmp("BR"))
        {
          line     = do_align(block, line, xx, newalign, links);
          xx       = block->x;
          block->h += hh;
          yy       += hh;
          hh       = 0;
        }
        else if (buf.cmp("CENTER") ||
                 buf.cmp("P") ||
                 buf.cmp("H1") ||
                 buf.cmp("H2") ||
                 buf.cmp("H3") ||
                 buf.cmp("H4") ||
                 buf.cmp("H5") ||
                 buf.cmp("H6") ||
                 buf.cmp("UL") ||
                 buf.cmp("OL") ||
                 buf.cmp("DL") ||
                 buf.cmp("LI") ||
                 buf.cmp("DD") ||
                 buf.cmp("DT") ||
                 buf.cmp("HR") ||
                 buf.cmp("PRE") ||
                 buf.cmp("TABLE"))
        {
          block->end = start;
          line       = do_align(block, line, xx, newalign, links);
          newalign   = buf.cmp("CENTER") ? Align::CENTER : Align::LEFT;
          xx         = block->x;
          block->h   += hh;

          if (buf.cmp("OL")) {
            int ol_num = 1;
            if (get_attr(attrs, "START", attr, sizeof(attr)) != nullptr) {
              errno = 0;
              char *endptr = 0;
              ol_num = (int)strtol(attr, &endptr, 10);
              if (errno || endptr == attr || ol_num < 0)
                ol_num = 1;
            }
            OL_num.push_back(ol_num);
          }
          else if (buf.cmp("UL"))
            OL_num.push_back(-1);

          if (buf.cmp("UL") ||
              buf.cmp("OL") ||
              buf.cmp("DL"))
          {
            block->h += fsize + 2;
            xx       = margins.push(4 * fsize);
          }
          else if (buf.cmp("TABLE"))
          {
            if (get_attr(attrs, "BORDER", attr, sizeof(attr)))
              border = (uchar)atoi(attr);
            else
              border = 0;

            tc = rc = get_color(get_attr(attrs, "BGCOLOR", attr, sizeof(attr)), bgcolor_);

            block->h += fsize + 2;

            format_table(&table_width, columns, start);

            if ((xx + table_width) > hsize_) {
#ifdef DEBUG
              printf("xx=%d, table_width=%d, hsize_=%d\n", xx, table_width,
                     hsize_);
#endif // DEBUG
              hsize_ = xx + table_width;
              done   = 0;
              break;
            }

            switch (get_align(attrs, talign))
            {
              default :
                  table_offset = 0;
                  break;

              case Align::CENTER :
                  table_offset = (hsize_ - table_width) / 2 - textsize_;
                  break;

              case Align::RIGHT :
                  table_offset = hsize_ - table_width - textsize_;
                  break;
            }

            column = 0;
          }

          if (tolower(buf[0]) == 'h' && isdigit(buf[1]))
          {
            font  = FL_HELVETICA_BOLD;
            fsize = textsize_ + '7' - buf[1];
          }
          else if (buf.cmp("DT"))
          {
            font  = textfont_ | FL_ITALIC;
            fsize = textsize_;
          }
          else if (buf.cmp("PRE"))
          {
            font  = FL_COURIER;
            fsize = textsize_;
            pre   = 1;
          }
          else
          {
            font  = textfont_;
            fsize = textsize_;
          }

          pushfont(font, fsize);

          yy = block->y + block->h;
          hh = 0;

          if ((tolower(buf[0]) == 'h' && isdigit(buf[1])) ||
              buf.cmp("DD") ||
              buf.cmp("DT") ||
              buf.cmp("P"))
            yy += fsize + 2;
          else if (buf.cmp("HR"))
          {
            hh += 2 * fsize;
            yy += fsize;
          }

          if (row)
            block = add_block(start, xx, yy, block->w, 0);
          else
            block = add_block(start, xx, yy, hsize_, 0);

          if (buf.cmp("LI")) {
            block->ol = 0;
            if (OL_num.size() && (OL_num.back() >= 0)) {
              block->ol = 1;
              block->ol_num = OL_num.back();
              OL_num.back()++;
            }
          }

          needspace = 0;
          line      = 0;

          if (buf.cmp("CENTER"))
            newalign = talign = Align::CENTER;
          else
            newalign = get_align(attrs, talign);
        }
        else if (buf.cmp("/CENTER") ||
                 buf.cmp("/P") ||
                 buf.cmp("/H1") ||
                 buf.cmp("/H2") ||
                 buf.cmp("/H3") ||
                 buf.cmp("/H4") ||
                 buf.cmp("/H5") ||
                 buf.cmp("/H6") ||
                 buf.cmp("/PRE") ||
                 buf.cmp("/UL") ||
                 buf.cmp("/OL") ||
                 buf.cmp("/DL") ||
                 buf.cmp("/TABLE"))
        {
          line       = do_align(block, line, xx, newalign, links);
          xx         = block->x;
          block->end = ptr;

          if (buf.cmp("/OL") ||
              buf.cmp("/UL")) {
            if (OL_num.size()) OL_num.pop_back();
          }

          if (buf.cmp("/UL") ||
              buf.cmp("/OL") ||
              buf.cmp("/DL"))
          {
            xx       = margins.pop();
            block->h += fsize + 2;
          }
          else if (buf.cmp("/TABLE"))
          {
            block->h += fsize + 2;
            xx       = margins.current();
          }
          else if (buf.cmp("/PRE"))
          {
            pre = 0;
            hh  = 0;
          }
          else if (buf.cmp("/CENTER"))
            talign = Align::LEFT;

          popfont(font, fsize, fcolor);

          //#if defined(__GNUC__)
          //#warning FIXME this isspace & 255 test will probably not work on a utf8 stream... And we use it everywhere!
          //#endif /*__GNUC__*/
          while (isspace((*ptr)&255))
            ptr ++;

          block->h += hh;
          yy       += hh;

          if (tolower(buf[2]) == 'l')
            yy += fsize + 2;

          if (row)
            block = add_block(ptr, xx, yy, block->w, 0);
          else
            block = add_block(ptr, xx, yy, hsize_, 0);

          needspace = 0;
          hh        = 0;
          line      = 0;
          newalign  = talign;
        }
        else if (buf.cmp("TR"))
        {
          block->end = start;
          line       = do_align(block, line, xx, newalign, links);
          xx         = block->x;
          block->h   += hh;

          if (row)
          {
            yy = blocks_[row].y + blocks_[row].h;

            for (cell = &blocks_[row + 1]; cell <= block; cell ++)
              if ((cell->y + cell->h) > yy)
                yy = cell->y + cell->h;

            block = &blocks_[row];

            block->h = yy - block->y + 2;

            for (i = 0; i < column; i ++)
              if (cells[i])
              {
                cell = &blocks_[cells[i]];
                cell->h = block->h;
              }
          }

          memset(cells, 0, sizeof(cells));

          yy        = block->y + block->h - 4;
          hh        = 0;
          block     = add_block(start, xx, yy, hsize_, 0);
          row       = (int) (block - &blocks_[0]);
          needspace = 0;
          column    = 0;
          line      = 0;

          rc = get_color(get_attr(attrs, "BGCOLOR", attr, sizeof(attr)), tc);
        }
        else if (buf.cmp("/TR") && row)
        {
          line       = do_align(block, line, xx, newalign, links);
          block->end = start;
          block->h   += hh;
          talign     = Align::LEFT;

          xx = blocks_[row].x;
          yy = blocks_[row].y + blocks_[row].h;

          for (cell = &blocks_[row + 1]; cell <= block; cell ++)
            if ((cell->y + cell->h) > yy)
              yy = cell->y + cell->h;

          block = &blocks_[row];

          block->h = yy - block->y + 2;

          for (i = 0; i < column; i ++)
            if (cells[i])
            {
              cell = &blocks_[cells[i]];
              cell->h = block->h;
            }

          yy        = block->y + block->h /*- 4*/;
          block     = add_block(start, xx, yy, hsize_, 0);
          needspace = 0;
          row       = 0;
          line      = 0;
        }
        else if ((buf.cmp("TD") ||
                  buf.cmp("TH")) && row)
        {
          int   colspan;                // COLSPAN attribute


          line       = do_align(block, line, xx, newalign, links);
          block->end = start;
          block->h   += hh;

          if (buf.cmp("TH"))
            font = textfont_ | FL_BOLD;
          else
            font = textfont_;

          fsize = textsize_;

          xx = blocks_[row].x + fsize + 3 + table_offset;
          for (i = 0; i < column; i ++)
            xx += columns[i] + 6;

          margins.push(xx - margins.current());

          if (get_attr(attrs, "COLSPAN", attr, sizeof(attr)) != nullptr)
            colspan = atoi(attr);
          else
            colspan = 1;

          for (i = 0, ww = -6; i < colspan; i ++)
            ww += columns[column + i] + 6;

          if (block->end == block->start && blocks_.size() > 1)
          {
            blocks_.pop_back();
            block --;
          }

          pushfont(font, fsize);

          yy        = blocks_[row].y;
          hh        = 0;
          block     = add_block(start, xx, yy, xx + ww, 0, border);
          needspace = 0;
          line      = 0;
          newalign  = get_align(attrs, tolower(buf[1]) == 'h' ? Align::CENTER : Align::LEFT);
          talign    = newalign;

          cells[column] = (int) (block - &blocks_[0]);

          column += colspan;

          block->bgcolor = get_color(get_attr(attrs, "BGCOLOR", attr,
                                              sizeof(attr)), rc);
        }
        else if ((buf.cmp("/TD") ||
                  buf.cmp("/TH")) && row)
        {
          line = do_align(block, line, xx, newalign, links);
          popfont(font, fsize, fcolor);
          xx = margins.pop();
          talign = Align::LEFT;
        }
        else if (buf.cmp("FONT"))
        {
          if (get_attr(attrs, "FACE", attr, sizeof(attr)) != nullptr) {
            if (!strncasecmp(attr, "helvetica", 9) ||
                !strncasecmp(attr, "arial", 5) ||
                !strncasecmp(attr, "sans", 4)) font = FL_HELVETICA;
            else if (!strncasecmp(attr, "times", 5) ||
                     !strncasecmp(attr, "serif", 5)) font = FL_TIMES;
            else if (!strncasecmp(attr, "symbol", 6)) font = FL_SYMBOL;
            else font = FL_COURIER;
          }

          if (get_attr(attrs, "SIZE", attr, sizeof(attr)) != nullptr) {
            if (isdigit(attr[0] & 255)) {
              // Absolute size
              fsize = (int)(textsize_ * pow(1.2, atoi(attr) - 3.0));
            } else {
              // Relative size
              fsize = (int)(fsize * pow(1.2, atoi(attr)));
            }
          }

          pushfont(font, fsize);
        }
        else if (buf.cmp("/FONT"))
          popfont(font, fsize, fcolor);
        else if (buf.cmp("B") ||
                 buf.cmp("STRONG"))
          pushfont(font |= FL_BOLD, fsize);
        else if (buf.cmp("I") ||
                 buf.cmp("EM"))
          pushfont(font |= FL_ITALIC, fsize);
        else if (buf.cmp("CODE") ||
                 buf.cmp("TT"))
          pushfont(font = FL_COURIER, fsize);
        else if (buf.cmp("KBD"))
          pushfont(font = FL_COURIER_BOLD, fsize);
        else if (buf.cmp("VAR"))
          pushfont(font = FL_COURIER_ITALIC, fsize);
        else if (buf.cmp("/B") ||
                 buf.cmp("/STRONG") ||
                 buf.cmp("/I") ||
                 buf.cmp("/EM") ||
                 buf.cmp("/CODE") ||
                 buf.cmp("/TT") ||
                 buf.cmp("/KBD") ||
                 buf.cmp("/VAR"))
          popfont(font, fsize, fcolor);
        else if (buf.cmp("IMG"))
        {
          Fl_Shared_Image       *img = 0;
          int           width;
          int           height;


          get_attr(attrs, "WIDTH", wattr, sizeof(wattr));
          get_attr(attrs, "HEIGHT", hattr, sizeof(hattr));
          width  = get_length(wattr);
          height = get_length(hattr);

          if (get_attr(attrs, "SRC", attr, sizeof(attr))) {
            img    = get_image(attr, width, height);
            width  = img->w();
            height = img->h();
          }

          ww = width;

          if (ww > hsize_) {
            hsize_ = ww;
            done   = 0;
            break;
          }

          if (needspace && xx > block->x)
            ww += (int)fl_width(' ');

          if ((xx + ww) > block->w)
          {
            line     = do_align(block, line, xx, newalign, links);
            xx       = block->x;
            yy       += hh;
            block->h += hh;
            hh       = 0;
          }

          if (linkdest[0])
            add_link(linkdest, xx, yy-fsize, ww, height);

          xx += ww;
          if ((height + 2) > hh)
            hh = height + 2;

          needspace = 0;
        }
        buf.clear();
      }
      else if (*ptr == '\n' && pre)
      {
        if (linkdest[0])
          add_link(linkdest, xx, yy - hh, ww, hh);

        if (xx > hsize_) {
          hsize_ = xx;
          done   = 0;
          break;
        }

        line      = do_align(block, line, xx, newalign, links);
        xx        = block->x;
        yy        += hh;
        block->h  += hh;
        needspace = 0;
        ptr ++;
      }
      else if (isspace((*ptr)&255))
      {
        needspace = 1;
        if ( pre ) {
          xx += (int)fl_width(' ');
        }
        ptr ++;
      }
      else if (*ptr == '&')
      {
        // Handle html '&' codes, eg. "&amp;"
        ptr ++;

        int qch = quote_char(ptr);

        if (qch < 0)
          buf += '&';
        else {
          buf.add(qch);
          ptr = strchr(ptr, ';') + 1;
        }

        if ((fsize + 2) > hh)
          hh = fsize + 2;
      }
      else
      {
        buf += *ptr++;

        if ((fsize + 2) > hh)
          hh = fsize + 2;
      }
    }

    if (buf.size() > 0 && !head)
    {
      ww = buf.width();

  //    printf("line = %d, xx = %d, ww = %d, block->x = %d, block->w = %d\n",
  //       line, xx, ww, block->x, block->w);

      if (ww > hsize_) {
        hsize_ = ww;
        done   = 0;
        break;
      }

      if (needspace && xx > block->x)
        ww += (int)fl_width(' ');

      if ((xx + ww) > block->w)
      {
        line     = do_align(block, line, xx, newalign, links);
        xx       = block->x;
        yy       += hh;
        block->h += hh;
        hh       = 0;
      }

      if (linkdest[0])
        add_link(linkdest, xx, yy - fsize, ww, fsize);

      xx += ww;
    }

    do_align(block, line, xx, newalign, links);

    block->end = ptr;
    size_      = yy + hh;
  }
  // Make sure that the last block will have the correct height.
  if (hh > block->h) block->h = hh;

//  printf("margins.depth_=%d\n", margins.depth_);

  int dx = Fl::box_dw(b) - Fl::box_dx(b);
  int dy = Fl::box_dh(b) - Fl::box_dy(b);
  int ss = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size();
  int dw = Fl::box_dw(b) + ss;
  int dh = Fl::box_dh(b);

  if (hsize_ > (view.w() - dw)) {
    view.hscrollbar_.show();

    dh += ss;

    if (size_ < (view.h() - dh)) {
      view.scrollbar_.hide();
      view.hscrollbar_.resize(view.x() + Fl::box_dx(b), view.y() + view.h() - ss - dy,
                         view.w() - Fl::box_dw(b), ss);
    } else {
      view.scrollbar_.show();
      view.scrollbar_.resize(view.x() +view. w() - ss - dx, view.y() + Fl::box_dy(b),
                        ss, view.h() - ss - Fl::box_dh(b));
      view.hscrollbar_.resize(view.x() + Fl::box_dx(b), view.y() + view.h() - ss - dy,
                         view.w() - ss - Fl::box_dw(b), ss);
    }
  } else {
    view.hscrollbar_.hide();

    if (size_ < (view.h() - dh)) view.scrollbar_.hide();
    else {
      view.scrollbar_.resize(view.x() + view.w() - ss - dx, view.y() + Fl::box_dy(b),
                        ss, view.h() - Fl::box_dh(b));
      view.scrollbar_.show();
    }
  }

  // Reset scrolling if it needs to be...
  if (view.scrollbar_.visible()) {
    int temph = view.h() - Fl::box_dh(b);
    if (view.hscrollbar_.visible()) temph -= ss;
    if ((topline_ + temph) > size_) topline(size_ - temph);
    else topline(topline_);
  } else topline(0);

  if (view.hscrollbar_.visible()) {
    int tempw = view.w() - ss - Fl::box_dw(b);
    if ((leftline_ + tempw) > hsize_) leftline(hsize_ - tempw);
    else leftline(leftline_);
  } else leftline(0);
}


/**
  \brief Format a table
  \param[out] table_width Total width of the table
  \param[out] columns Array of column widths
  \param[in] table Pointer to the start of the table in the HTML text
  */
void Fl_Help_View::Impl::format_table(
  int *table_width,
  int *columns,
  const char *table)
{
  int           column,                                 // Current column
                num_columns,                            // Number of columns
                colspan,                                // COLSPAN attribute
                width,                                  // Current width
                temp_width,                             // Temporary width
                max_width,                              // Maximum width
                incell,                                 // In a table cell?
                pre,                                    // <PRE> text?
                needspace;                              // Need whitespace?
  Edit_Buffer   buf;                                    // Text buffer
  char          attr[1024],                             // Other attribute
                wattr[1024],                            // WIDTH attribute
                hattr[1024];                            // HEIGHT attribute
  const char    *ptr,                                   // Pointer into table
                *attrs,                                 // Pointer to attributes
                *start;                                 // Start of element
  int           minwidths[MAX_COLUMNS];                 // Minimum widths for each column
  Fl_Font       font;
  Fl_Fontsize   fsize;                                  // Current font and size
  Fl_Color      fcolor;                                 // Currrent font color

  DEBUG_FUNCTION(__LINE__,__FUNCTION__);

  // Clear widths...
  *table_width = 0;
  for (column = 0; column < MAX_COLUMNS; column ++)
  {
    columns[column]   = 0;
    minwidths[column] = 0;
  }

  num_columns = 0;
  colspan     = 0;
  max_width   = 0;
  pre         = 0;
  needspace   = 0;
  fstack_.top(font, fsize, fcolor);

  // Scan the table...
  for (ptr = table, column = -1, width = 0, incell = 0; *ptr;)
  {
    if ((*ptr == '<' || isspace((*ptr)&255)) && buf.size() > 0 && incell)
    {
      // Check width...
      if (needspace)
      {
        buf += ' ';
        needspace = 0;
      }

      temp_width = buf.width();
      buf.clear();

      if (temp_width > minwidths[column])
        minwidths[column] = temp_width;

      width += temp_width;

      if (width > max_width)
        max_width = width;
    }

    if (*ptr == '<')
    {
      start = ptr;

      for (buf.clear(), ptr ++; *ptr && *ptr != '>' && !isspace((*ptr)&255);)
        buf += *ptr++;

      attrs = ptr;
      while (*ptr && *ptr != '>')
        ptr ++;

      if (*ptr == '>')
        ptr ++;

      if (buf.cmp("BR") ||
          buf.cmp("HR"))
      {
        width     = 0;
        needspace = 0;
      }
      else if (buf.cmp("TABLE") && start > table)
        break;
      else if (buf.cmp("CENTER") ||
               buf.cmp("P") ||
               buf.cmp("H1") ||
               buf.cmp("H2") ||
               buf.cmp("H3") ||
               buf.cmp("H4") ||
               buf.cmp("H5") ||
               buf.cmp("H6") ||
               buf.cmp("UL") ||
               buf.cmp("OL") ||
               buf.cmp("DL") ||
               buf.cmp("LI") ||
               buf.cmp("DD") ||
               buf.cmp("DT") ||
               buf.cmp("PRE"))
      {
        width     = 0;
        needspace = 0;

        if (tolower(buf[0]) == 'h' && isdigit(buf[1]))
        {
          font  = FL_HELVETICA_BOLD;
          fsize = textsize_ + '7' - buf[1];
        }
        else if (buf.cmp("DT"))
        {
          font  = textfont_ | FL_ITALIC;
          fsize = textsize_;
        }
        else if (buf.cmp("PRE"))
        {
          font  = FL_COURIER;
          fsize = textsize_;
          pre   = 1;
        }
        else if (buf.cmp("LI"))
        {
          width  += 4 * fsize;
          font   = textfont_;
          fsize  = textsize_;
        }
        else
        {
          font  = textfont_;
          fsize = textsize_;
        }

        pushfont(font, fsize);
      }
      else if (buf.cmp("/CENTER") ||
               buf.cmp("/P") ||
               buf.cmp("/H1") ||
               buf.cmp("/H2") ||
               buf.cmp("/H3") ||
               buf.cmp("/H4") ||
               buf.cmp("/H5") ||
               buf.cmp("/H6") ||
               buf.cmp("/PRE") ||
               buf.cmp("/UL") ||
               buf.cmp("/OL") ||
               buf.cmp("/DL"))
      {
        width     = 0;
        needspace = 0;

        popfont(font, fsize, fcolor);
      }
      else if (buf.cmp("TR") || buf.cmp("/TR") ||
               buf.cmp("/TABLE"))
      {
//        printf("%s column = %d, colspan = %d, num_columns = %d\n",
//             buf.c_str(), column, colspan, num_columns);

        if (column >= 0)
        {
          // This is a hack to support COLSPAN...
          max_width /= colspan;

          while (colspan > 0)
          {
            if (max_width > columns[column])
              columns[column] = max_width;

            column ++;
            colspan --;
          }
        }

        if (buf.cmp("/TABLE"))
          break;

        needspace = 0;
        column    = -1;
        width     = 0;
        max_width = 0;
        incell    = 0;
      }
      else if (buf.cmp("TD") ||
               buf.cmp("TH"))
      {
//        printf("BEFORE column = %d, colspan = %d, num_columns = %d\n",
//             column, colspan, num_columns);

        if (column >= 0)
        {
          // This is a hack to support COLSPAN...
          max_width /= colspan;

          while (colspan > 0)
          {
            if (max_width > columns[column])
              columns[column] = max_width;

            column ++;
            colspan --;
          }
        }
        else
          column ++;

        if (get_attr(attrs, "COLSPAN", attr, sizeof(attr)) != nullptr)
          colspan = atoi(attr);
        else
          colspan = 1;

//        printf("AFTER column = %d, colspan = %d, num_columns = %d\n",
//             column, colspan, num_columns);

        if ((column + colspan) >= num_columns)
          num_columns = column + colspan;

        needspace = 0;
        width     = 0;
        incell    = 1;

        if (buf.cmp("TH"))
          font = textfont_ | FL_BOLD;
        else
          font = textfont_;

        fsize = textsize_;

        pushfont(font, fsize);

        if (get_attr(attrs, "WIDTH", attr, sizeof(attr)) != nullptr)
          max_width = get_length(attr);
        else
          max_width = 0;

//        printf("max_width = %d\n", max_width);
      }
      else if (buf.cmp("/TD") ||
               buf.cmp("/TH"))
      {
        incell = 0;
        popfont(font, fsize, fcolor);
      }
      else if (buf.cmp("B") ||
               buf.cmp("STRONG"))
        pushfont(font |= FL_BOLD, fsize);
      else if (buf.cmp("I") ||
               buf.cmp("EM"))
        pushfont(font |= FL_ITALIC, fsize);
      else if (buf.cmp("CODE") ||
               buf.cmp("TT"))
        pushfont(font = FL_COURIER, fsize);
      else if (buf.cmp("KBD"))
        pushfont(font = FL_COURIER_BOLD, fsize);
      else if (buf.cmp("VAR"))
        pushfont(font = FL_COURIER_ITALIC, fsize);
      else if (buf.cmp("/B") ||
               buf.cmp("/STRONG") ||
               buf.cmp("/I") ||
               buf.cmp("/EM") ||
               buf.cmp("/CODE") ||
               buf.cmp("/TT") ||
               buf.cmp("/KBD") ||
               buf.cmp("/VAR"))
        popfont(font, fsize, fcolor);
      else if (buf.cmp("IMG") && incell)
      {
        Fl_Shared_Image *img = 0;
        int             iwidth, iheight;


        get_attr(attrs, "WIDTH", wattr, sizeof(wattr));
        get_attr(attrs, "HEIGHT", hattr, sizeof(hattr));
        iwidth  = get_length(wattr);
        iheight = get_length(hattr);

        if (get_attr(attrs, "SRC", attr, sizeof(attr))) {
          img     = get_image(attr, iwidth, iheight);
          iwidth  = img->w();
          iheight = img->h();
        }

        if (iwidth > minwidths[column])
          minwidths[column] = iwidth;

        width += iwidth;
        if (needspace)
          width += (int)fl_width(' ');

        if (width > max_width)
          max_width = width;

        needspace = 0;
      }
      buf.clear();
    }
    else if (*ptr == '\n' && pre)
    {
      width     = 0;
      needspace = 0;
      ptr ++;
    }
    else if (isspace((*ptr)&255))
    {
      needspace = 1;

      ptr ++;
    }
    else if (*ptr == '&' )
    {
      ptr ++;

      int qch = quote_char(ptr);

      if (qch < 0)
        buf += '&';
      else {
        buf.add(qch);
        ptr = strchr(ptr, ';') + 1;
      }
    }
    else
    {
      buf += *ptr++;
    }
  }

  // Now that we have scanned the entire table, adjust the table and
  // cell widths to fit on the screen...
  if (get_attr(table + 6, "WIDTH", attr, sizeof(attr)))
    *table_width = get_length(attr);
  else
    *table_width = 0;

#ifdef DEBUG
  printf("num_columns = %d, table_width = %d\n", num_columns, *table_width);
#endif // DEBUG

  if (num_columns == 0)
    return;

  // Add up the widths...
  for (column = 0, width = 0; column < num_columns; column ++)
    width += columns[column];

#ifdef DEBUG
  printf("width = %d, w() = %d\n", width, w());
  for (column = 0; column < num_columns; column ++)
    printf("    columns[%d] = %d, minwidths[%d] = %d\n", column, columns[column],
           column, minwidths[column]);
#endif // DEBUG

  // Adjust the width if needed...
  int scale_width = *table_width;

  int scrollsize = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size();
  if (scale_width == 0) {
    if (width > (hsize_ - scrollsize)) scale_width = hsize_ - scrollsize;
    else scale_width = width;
  }

  if (width < scale_width) {
#ifdef DEBUG
    printf("Scaling table up to %d from %d...\n", scale_width, width);
#endif // DEBUG

    *table_width = 0;

    scale_width = (scale_width - width) / num_columns;

#ifdef DEBUG
    printf("adjusted scale_width = %d\n", scale_width);
#endif // DEBUG

    for (column = 0; column < num_columns; column ++) {
      columns[column] += scale_width;

      (*table_width) += columns[column];
    }
  }
  else if (width > scale_width) {
#ifdef DEBUG
    printf("Scaling table down to %d from %d...\n", scale_width, width);
#endif // DEBUG

    for (column = 0; column < num_columns; column ++) {
      width       -= minwidths[column];
      scale_width -= minwidths[column];
    }

#ifdef DEBUG
    printf("adjusted width = %d, scale_width = %d\n", width, scale_width);
#endif // DEBUG

    if (width > 0) {
      for (column = 0; column < num_columns; column ++) {
        columns[column] -= minwidths[column];
        columns[column] = scale_width * columns[column] / width;
        columns[column] += minwidths[column];
      }
    }

    *table_width = 0;
    for (column = 0; column < num_columns; column ++) {
      (*table_width) += columns[column];
    }
  }
  else if (*table_width == 0)
    *table_width = width;

#ifdef DEBUG
  printf("FINAL table_width = %d\n", *table_width);
  for (column = 0; column < num_columns; column ++)
    printf("    columns[%d] = %d\n", column, columns[column]);
#endif // DEBUG
}


/**
  \brief Gets an alignment attribute.
  \param[in] p Pointer to start of attributes.
  \param[in] a Default alignment.
  \return Alignment value, either CENTER, RIGHT, or LEFT.
*/
Fl_Help_View::Impl::Align Fl_Help_View::Impl::get_align(const char *p, Align a)
{
  char  buf[255];                       // Alignment value

  if (get_attr(p, "ALIGN", buf, sizeof(buf)) == nullptr)
    return (a);

  if (strcasecmp(buf, "CENTER") == 0)
    return Align::CENTER;
  else if (strcasecmp(buf, "RIGHT") == 0)
    return Align::RIGHT;
  else
    return Align::LEFT;
}


/**
  \brief Gets an attribute value from the string.
  \param[in] p Pointer to start of attributes.
  \param[in] n Name of attribute.
  \param[out] buf Buffer for attribute value.
  \param[in] bufsize Size of buffer.
  \return Pointer to buf or nullptr if not found.
  */
const char *Fl_Help_View::Impl::get_attr(
  const char *p,
  const char *n,
  char *buf,
  int bufsize)
{
  char  name[255],                              // Name from string
        *ptr,                                   // Pointer into name or value
        quote;                                  // Quote


  buf[0] = '\0';

  while (*p && *p != '>')
  {
    while (isspace((*p)&255))
      p ++;

    if (*p == '>' || !*p)
      return (nullptr);

    for (ptr = name; *p && !isspace((*p)&255) && *p != '=' && *p != '>';)
      if (ptr < (name + sizeof(name) - 1))
        *ptr++ = *p++;
      else
        p ++;

    *ptr = '\0';

    if (isspace((*p)&255) || !*p || *p == '>')
      buf[0] = '\0';
    else
    {
      if (*p == '=')
        p ++;

      for (ptr = buf; *p && !isspace((*p)&255) && *p != '>';)
        if (*p == '\'' || *p == '\"')
        {
          quote = *p++;

          while (*p && *p != quote)
            if ((ptr - buf + 1) < bufsize)
              *ptr++ = *p++;
            else
              p ++;

          if (*p == quote)
            p ++;
        }
        else if ((ptr - buf + 1) < bufsize)
          *ptr++ = *p++;
        else
          p ++;

      *ptr = '\0';
    }

    if (strcasecmp(n, name) == 0)
      return (buf);
    else
      buf[0] = '\0';

    if (*p == '>')
      return (nullptr);
  }

  return (nullptr);
}


/**
  \brief Gets a color attribute.
  \param[in] n the color name, either a name or a hex value.
  \param[in] c the default color value.
  \return the color value, either the color from the name or the default value.
  */
Fl_Color Fl_Help_View::Impl::get_color(const char *n, Fl_Color c)
{
  int   i;                              // Looping var
  int   rgb, r, g, b;                   // RGB values
  static const struct {                 // Color name table
    const char *name;
    int r, g, b;
  }     colors[] = {
    { "black",          0x00, 0x00, 0x00 },
    { "red",            0xff, 0x00, 0x00 },
    { "green",          0x00, 0x80, 0x00 },
    { "yellow",         0xff, 0xff, 0x00 },
    { "blue",           0x00, 0x00, 0xff },
    { "magenta",        0xff, 0x00, 0xff },
    { "fuchsia",        0xff, 0x00, 0xff },
    { "cyan",           0x00, 0xff, 0xff },
    { "aqua",           0x00, 0xff, 0xff },
    { "white",          0xff, 0xff, 0xff },
    { "gray",           0x80, 0x80, 0x80 },
    { "grey",           0x80, 0x80, 0x80 },
    { "lime",           0x00, 0xff, 0x00 },
    { "maroon",         0x80, 0x00, 0x00 },
    { "navy",           0x00, 0x00, 0x80 },
    { "olive",          0x80, 0x80, 0x00 },
    { "purple",         0x80, 0x00, 0x80 },
    { "silver",         0xc0, 0xc0, 0xc0 },
    { "teal",           0x00, 0x80, 0x80 }
  };


  if (!n || !n[0]) return c;

  if (n[0] == '#') {
    // Do hex color lookup
    rgb = (int)strtol(n + 1, nullptr, 16);

    if (strlen(n) > 4) {
      r = rgb >> 16;
      g = (rgb >> 8) & 255;
      b = rgb & 255;
    } else {
      r = (rgb >> 8) * 17;
      g = ((rgb >> 4) & 15) * 17;
      b = (rgb & 15) * 17;
    }
    return (fl_rgb_color((uchar)r, (uchar)g, (uchar)b));
  } else {
    for (i = 0; i < (int)(sizeof(colors) / sizeof(colors[0])); i ++)
      if (!strcasecmp(n, colors[i].name)) {
        return fl_rgb_color(colors[i].r, colors[i].g, colors[i].b);
      }
    return c;
  }
}


/* Implementation note: (A.S. Apr 05, 2009)

  Fl_Help_View::Impl::get_image() uses a static global flag (initial_load)
  to determine, if it is called from the initial loading of a document
  (load() or value()), or from resize() or draw().

  A better solution would be to manage all loaded images in an own
  structure like Fl_Help_Target (Fl_Help_Image ?) to avoid using this
  global flag, but this would break the ABI !

  This should be fixed in FLTK 1.3 !


  If initial_load is true, then Fl_Shared_Image::get() is called to
  load the image, and the reference count of the shared image is
  increased by one.

  If initial_load is false, then Fl_Shared_Image::find() is called to
  load the image, and the image is released immediately. This avoids
  increasing the reference count when calling get_image() from draw()
  or resize().

  Calling Fl_Shared_Image::find() instead of Fl_Shared_Image::get() avoids
  doing unnecessary i/o for "broken images" within each resize/redraw.

  Each image must be released exactly once in the destructor or before
  a new document is loaded: see free_data().
*/

/**
  \brief Gets an inline image.

  The image reference count is maintained accordingly, such that
  the image can be released exactly once when the document is closed.

  \param[in] name the image name, either a local filename or a URL.
  \param[in] W, H the size of the image, or 0 if not specified.
  \return a pointer to a cached Fl_Shared_Image, if the image can be loaded,
          otherwise a pointer to an internal Fl_Pixmap (broken_image).

  \todo Fl_Help_View::Impl::get_image() returns a pointer to the internal
  Fl_Pixmap broken_image, but this is _not_ compatible with the
  return type Fl_Shared_Image (release() must not be called).
*/
Fl_Shared_Image *Fl_Help_View::Impl::get_image(const char *name, int W, int H)
{
  std::string url;
  Fl_Shared_Image *ip;                  // Image pointer...

  if (!name || !name[0]) {
    // No image name given, return broken image
    return (Fl_Shared_Image *)&broken_image;
  }
  std::string imagename = name;

  size_t directory_scheme_length = url_scheme(directory_);
  size_t imagename_scheme_length = url_scheme(imagename);

  // See if the image can be found...
  if ( (directory_scheme_length > 0) && (imagename_scheme_length == 0) ) {
    // If directory_ starts with a scheme (e.g.ftp:), but linkp->filename_ does not:
    if (imagename[0] == '/') {
      // If linkp->filename_ is absolute...
      url = directory_.substr(0, directory_scheme_length) + imagename;;
    } else {
      // If linkp->filename_ is relative, the URL is the directory_ plus the filename
      url = directory_ + "/" + imagename;
    }
  } else if (imagename[0] != '/' && (imagename_scheme_length == 0)) {
    // If the filename is relative and does not start with a scheme (ftp: , etc.)...
    if (!directory_.empty()) {
      // If we have a current directory, use that as the base for the URL
      url = directory_ + "/" + imagename;
    } else {
      // If we do not have a current directory, use the application's current working directory
      char dir[FL_PATH_MAX];       // Current directory (static size ok until we have fl_getcwd_std()
      fl_getcwd(dir, sizeof(dir));
      url = "file:" + std::string(dir) + "/" + imagename;
    }
  } else {
    // If the filename is absolute or starts with a protocol (e.g.ftp:), use it as is
    url = imagename;
  }
  if (link_) {
    const char *n = (*link_)(&view, url.c_str());
    if (n == nullptr)
      return 0;
    url = n;
  }
  if (url.empty()) return 0;

  // If the URL starts with "file:", remove it
  if (url.find("file:") == 0) {
    url = url.substr(5);
  }

  if (initial_load) {
    if ((ip = Fl_Shared_Image::get(url.c_str(), W, H)) == nullptr) {
      ip = (Fl_Shared_Image *)&broken_image;
    }
  } else { // draw or resize
    if ((ip = Fl_Shared_Image::find(url.c_str(), W, H)) == nullptr) {
      ip = (Fl_Shared_Image *)&broken_image;
    } else {
      ip->release();
    }
  }

  return ip;
}


/**
  \brief Gets a length value, either absolute or %.
  \param[in] l string containing the length value
  \return the length in pixels, or 0 if the string is empty.
*/
int Fl_Help_View::Impl::get_length(const char *l) {
  int val;

  if (!l[0]) return 0;

  val = atoi(l);
  if (l[strlen(l) - 1] == '%') {
    if (val > 100) val = 100;
    else if (val < 0) val = 0;

    int scrollsize = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size();
    val = val * (hsize_ - scrollsize) / 100;
  }

  return val;
}

// ---- Text selection


/*
  About selecting text:

  Still to do:
  - &word; style characters mess up our count inside a word boundary
  - we can only select words, no individual characters
  - no dragging of the selection into another widget
  - we are using the draw() function to measure screen postion of text
    by rerouting the code via draw_mode_. Some drawing functions are
    still called which is slow and requires a fake graphics context.
    It may help to get rid of those calls if not in DRAW mode.

  matt.
 */

/**
  \brief Draws a text string in the help view.

  This function draws the text string \p t at position (\p x, \p y) in the help view.
  If the text is selected, it draws a selection rectangle around it and changes the text color.

  \param[in] t Text to draw
  \param[in] x X position to draw at
  \param[in] y Y position to draw at
  \param[in] entity_extra_length (unclear)
 */
void Fl_Help_View::Impl::hv_draw(const char *t, int x, int y, int entity_extra_length)
{
  if (draw_mode_ == Mode::DRAW) {
    if (selected_ && current_pos_<selection_last_ && current_pos_>=selection_first_) {
      Fl_Color c = fl_color();
      fl_color(tmp_selection_color_);
      int w = (int)fl_width(t);
      if (current_pos_+(int)strlen(t)<selection_last_)
        w += (int)fl_width(' ');
      fl_rectf(x, y+fl_descent()-fl_height(), w, fl_height());
      fl_color(selection_text_color_);
      fl_draw(t, x, y);
      fl_color(c);
    } else {
      fl_draw(t, x, y);
    }
  } else {
    // If draw_mode_ is not DRAW, we don't actually draw anything, but instead
    // measure where text blocks are on screen during a mouse selection process.
    int w = (int)fl_width(t);
    if ( (Fl::event_x() >= x) && (Fl::event_x() < x+w) ) {
      if ( (Fl::event_y() >= y-fl_height()+fl_descent()) && (Fl::event_y() <= y+fl_descent()) ) {
        int f = (int) current_pos_;
        int l = (int) (f+strlen(t)); // use 'quote_char' to calculate the true length of the HTML string
        if (draw_mode_ == Mode::PUSH) {
          selection_push_first_ = selection_drag_first_ = f;
          selection_push_last_ = selection_drag_last_ = l;
        } else { // Mode::DRAG
          selection_drag_first_ = f;
          selection_drag_last_ = l + entity_extra_length;
        }
      }
    }
  }
}


/**
  \brief Called from `handle()>FL_PUSH`, starts new text selection process.

  This method return 1 if the user clicks on selectable text. It sets
  selection_push_first_ and selection_push_last_ to the current
  selection start and end positions, respectively.

  \return 1 if the selection was started, 0 if not.
*/
char Fl_Help_View::Impl::begin_selection()
{
  clear_selection();
  selection_push_first_ = selection_push_last_ = 0;
  selection_drag_first_ = selection_drag_last_ = 0;

  if (!fl_help_view_buffer) fl_help_view_buffer = fl_create_offscreen(1, 1);

  draw_mode_ = Mode::PUSH;

    fl_begin_offscreen(fl_help_view_buffer);
    draw();
    fl_end_offscreen();

  draw_mode_ = Mode::DRAW;

  if (selection_push_last_) return 1;
  else return 0;
}


/**
  \brief Called from `handle()>FL_DRAG`, extending text selection.
  \return 1 if more than just the initial text is selected.
*/
char Fl_Help_View::Impl::extend_selection()
{
  if (Fl::event_is_click())
    return 0;

  // Give this widget the focus during the selection process. This will
  // deselect other text selection and make sure, we receive the Copy
  // keyboard shortcut.
  if (Fl::focus()!=&view)
    Fl::focus(&view);

//  printf("old selection_first_=%d, selection_last_=%d\n",
//         selection_first_, selection_last_);

  int sf = selection_first_, sl = selection_last_;

  selected_ = true;

  draw_mode_ = Mode::DRAG;

    fl_begin_offscreen(fl_help_view_buffer);
    draw();
    fl_end_offscreen();

  draw_mode_ = Mode::DRAW;

  if (selection_push_first_ < selection_drag_first_) {
    selection_first_ = selection_push_first_;
  } else {
    selection_first_ = selection_drag_first_;
  }

  if (selection_push_last_ > selection_drag_last_) {
    selection_last_ = selection_push_last_;
  } else {
    selection_last_ = selection_drag_last_;
  }

//  printf("new selection_first_=%d, selection_last_=%d\n",
//         selection_first_, selection_last_);

  if (sf!=selection_first_ || sl!=selection_last_) {
//    puts("REDRAW!!!\n");
    return 1;
  } else {
//    puts("");
    return 0;
  }
}


/**
  \brief Called from `handle()>FL_RELEASE`, ends text selection process.
  This method clears the static selection helper member variables.
*/
void Fl_Help_View::Impl::end_selection()
{
  selection_push_first_ = 0;
  selection_push_last_ = 0;
  selection_drag_first_ = 0;
  selection_drag_last_ = 0;
}


// ------ Fl_Help_View Protected and Public methods

// ---- Widget management

/**
  \brief Draws the Fl_Help_View widget.
*/
void Fl_Help_View::draw() {
  impl_->draw();
}

/**
  \brief Draws the Fl_Help_View widget.
  \see Fl_Help_View::draw()
 */
void Fl_Help_View::Impl::draw()
{
  int                   i;              // Looping var
  const Text_Block      *block;         // Pointer to current block
  const char            *ptr,           // Pointer to text in block
                        *attrs;         // Pointer to start of element attributes
  Edit_Buffer           buf;            // Text buffer
  char                  attr[1024];     // Attribute buffer
  int                   xx, yy, ww, hh; // Current positions and sizes
  int                   line;           // Current line
  Fl_Font               font;
  Fl_Fontsize           fsize;          // Current font and size
  Fl_Color              fcolor;         // current font color
  int                   head, pre,      // Flags for text
                        needspace;      // Do we need whitespace?
  Fl_Boxtype            b = view.box() ? view.box() : FL_DOWN_BOX;
                                        // Box to draw...
  int                   underline,      // Underline text?
                        xtra_ww;        // Extra width for underlined space between words

  DEBUG_FUNCTION(__LINE__,__FUNCTION__);

  // Draw the scrollbar(s) and box first...
  ww = view.w();
  hh = view.h();
  i  = 0;

  view.draw_box(b, view.x(), view.y(), ww, hh, bgcolor_);

  if ( view.hscrollbar_.visible() || view.scrollbar_.visible() ) {
    int scrollsize = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size();
    int hor_vis = view.hscrollbar_.visible();
    int ver_vis = view.scrollbar_.visible();
    // Scrollbar corner
    int scorn_x = view.x() + ww - (ver_vis?scrollsize:0) - Fl::box_dw(b) + Fl::box_dx(b);
    int scorn_y = view.y() + hh - (hor_vis?scrollsize:0) - Fl::box_dh(b) + Fl::box_dy(b);
    if ( hor_vis ) {
      if ( view.hscrollbar_.h() != scrollsize ) {            // scrollsize changed?
        view.hscrollbar_.resize(view.x(), scorn_y, scorn_x - view.x(), scrollsize);
        view.init_sizes();
      }
      view.draw_child(view.hscrollbar_);
      hh -= scrollsize;
    }
    if ( ver_vis ) {
      if ( view.scrollbar_.w() != scrollsize ) {             // scrollsize changed?
        view.scrollbar_.resize(scorn_x, view.y(), scrollsize, scorn_y - view.y());
        view.init_sizes();
      }
      view.draw_child(view.scrollbar_);
      ww -= scrollsize;
    }
    if ( hor_vis && ver_vis ) {
      // Both scrollbars visible? Draw little gray box in corner
      fl_color(FL_GRAY);
      fl_rectf(scorn_x, scorn_y, scrollsize, scrollsize);
    }
  }

  if (!value_)
    return;

  if (selected_) {
    if (Fl::focus() == &view) {
      // If this widget has the focus, we use the selection color directly
      tmp_selection_color_ = view.selection_color();
    } else {
      // Otherwise we blend the selection color with the background color
      tmp_selection_color_ = fl_color_average(bgcolor_, view.selection_color(), 0.8f);
    }
    selection_text_color_ = fl_contrast(textcolor_, tmp_selection_color_);
  }
  current_pos_ = 0;

  // Clip the drawing to the inside of the box...
  fl_push_clip(view.x() + Fl::box_dx(b), view.y() + Fl::box_dy(b),
               ww - Fl::box_dw(b), hh - Fl::box_dh(b));
  fl_color(textcolor_);

  // Draw all visible blocks...
  for (i = 0, block = &blocks_[0]; i < (int)blocks_.size(); i ++, block ++)
    if ((block->y + block->h) >= topline_ && block->y < (topline_ + view.h()))
    {
      line      = 0;
      xx        = block->line[line];
      yy        = block->y - topline_;
      hh        = 0;
      pre       = 0;
      head      = 0;
      needspace = 0;
      underline = 0;

      initfont(font, fsize, fcolor);
      // byte length difference between html entity (encoded by &...;) and
      // UTF-8 encoding of same character
      int entity_extra_length = 0;
      for (ptr = block->start, buf.clear(); ptr < block->end;)
      {
        if ((*ptr == '<' || isspace((*ptr)&255)) && buf.size() > 0)
        {
          if (!head && !pre)
          {
            // Check width...
            ww = buf.width();

            if (needspace && xx > block->x)
              xx += (int)fl_width(' ');

            if ((xx + ww) > block->w)
            {
              if (line < 31)
                line ++;
              xx = block->line[line];
              yy += hh;
              hh = 0;
            }

            hv_draw(buf.c_str(), xx + view.x() - leftline_, yy + view.y(), entity_extra_length);
            buf.clear();
            entity_extra_length = 0;
            if (underline) {
              xtra_ww = isspace((*ptr)&255)?(int)fl_width(' '):0;
              fl_xyline(xx + view.x() - leftline_, yy + view.y() + 1,
                        xx + view.x() - leftline_ + ww + xtra_ww);
            }
            current_pos_ = (int) (ptr-value_);

            xx += ww;
            if ((fsize + 2) > hh)
              hh = fsize + 2;

            needspace = 0;
          }
          else if (pre)
          {
            while (isspace((*ptr)&255))
            {
              if (*ptr == '\n')
              {
                hv_draw(buf.c_str(), xx + view.x() - leftline_, yy + view.y());
                if (underline) fl_xyline(xx + view.x() - leftline_, yy + view.y() + 1,
                                         xx + view.x() - leftline_ + buf.width());
                buf.clear();
                current_pos_ = (int) (ptr-value_);
                if (line < 31)
                  line ++;
                xx = block->line[line];
                yy += hh;
                hh = fsize + 2;
              }
              else if (*ptr == '\t')
              {
                // Do tabs every 8 columns...
                buf += ' '; // add at least one space
                while (buf.size() & 7)
                  buf += ' ';
              }
              else {
                buf += ' ';
              }
              if ((fsize + 2) > hh)
                hh = fsize + 2;

              ptr ++;
            }

            if (buf.size() > 0)
            {
              hv_draw(buf.c_str(), xx + view.x() - leftline_, yy + view.y());
              ww = buf.width();
              buf.clear();
              if (underline) fl_xyline(xx + view.x() - leftline_, yy + view.y() + 1,
                                       xx + view.x() - leftline_ + ww);
              xx += ww;
              current_pos_ = (int) (ptr-value_);
            }

            needspace = 0;
          }
          else
          {
            buf.clear();

            while (isspace((*ptr)&255))
              ptr ++;
            current_pos_ = (int) (ptr-value_);
          }
        }

        if (*ptr == '<')
        {
          ptr ++;

          if (strncmp(ptr, "!--", 3) == 0)
          {
            // Comment...
            ptr += 3;
            if ((ptr = strstr(ptr, "-->")) != nullptr)
            {
              ptr += 3;
              continue;
            }
            else
              break;
          }

          while (*ptr && *ptr != '>' && !isspace((*ptr)&255))
            buf += *ptr++;

          attrs = ptr;
          while (*ptr && *ptr != '>')
            ptr ++;

          if (*ptr == '>')
            ptr ++;

          // end of command reached, set the supposed start of printed eord here
          current_pos_ = (int) (ptr-value_);
          if (buf.cmp("HEAD"))
            head = 1;
          else if (buf.cmp("BR"))
          {
            if (line < 31)
              line ++;
            xx = block->line[line];
            yy += hh;
            hh = 0;
          }
          else if (buf.cmp("HR"))
          {
            fl_line(block->x + view.x(), yy + view.y(), block->w + view.x(),
                    yy + view.y());

            if (line < 31)
              line ++;
            xx = block->line[line];
            yy += 2 * fsize;//hh;
            hh = 0;
          }
          else if (buf.cmp("CENTER") ||
                   buf.cmp("P") ||
                   buf.cmp("H1") ||
                   buf.cmp("H2") ||
                   buf.cmp("H3") ||
                   buf.cmp("H4") ||
                   buf.cmp("H5") ||
                   buf.cmp("H6") ||
                   buf.cmp("UL") ||
                   buf.cmp("OL") ||
                   buf.cmp("DL") ||
                   buf.cmp("LI") ||
                   buf.cmp("DD") ||
                   buf.cmp("DT") ||
                   buf.cmp("PRE"))
          {
            if (tolower(buf[0]) == 'h')
            {
              font  = FL_HELVETICA_BOLD;
              fsize = textsize_ + '7' - buf[1];
            }
            else if (buf.cmp("DT"))
            {
              font  = textfont_ | FL_ITALIC;
              fsize = textsize_;
            }
            else if (buf.cmp("PRE"))
            {
              font  = FL_COURIER;
              fsize = textsize_;
              pre   = 1;
            }

            if (buf.cmp("LI"))
            {
              if (block->ol) {
                char buf[10];
                snprintf(buf, sizeof(buf), "%d. ", block->ol_num);
                hv_draw(buf, xx - (int)fl_width(buf) + view.x() - leftline_, yy + view.y());
              }
              else {
                // draw bullet (&bull;) Unicode: U+2022, UTF-8 (hex): e2 80 a2
                unsigned char bullet[4] = { 0xe2, 0x80, 0xa2, 0x00 };
                hv_draw((char *)bullet, xx - fsize + view.x() - leftline_, yy + view.y());
              }
            }

            pushfont(font, fsize);
            buf.clear();
          }
          else if (buf.cmp("A") &&
                   get_attr(attrs, "HREF", attr, sizeof(attr)) != nullptr)
          {
            fl_color(linkcolor_);
            underline = 1;
          }
          else if (buf.cmp("/A"))
          {
            fl_color(textcolor_);
            underline = 0;
          }
          else if (buf.cmp("FONT"))
          {
            if (get_attr(attrs, "COLOR", attr, sizeof(attr)) != nullptr) {
              textcolor_ = get_color(attr, textcolor_);
            }

            if (get_attr(attrs, "FACE", attr, sizeof(attr)) != nullptr) {
              if (!strncasecmp(attr, "helvetica", 9) ||
                  !strncasecmp(attr, "arial", 5) ||
                  !strncasecmp(attr, "sans", 4)) font = FL_HELVETICA;
              else if (!strncasecmp(attr, "times", 5) ||
                       !strncasecmp(attr, "serif", 5)) font = FL_TIMES;
              else if (!strncasecmp(attr, "symbol", 6)) font = FL_SYMBOL;
              else font = FL_COURIER;
            }

            if (get_attr(attrs, "SIZE", attr, sizeof(attr)) != nullptr) {
              if (isdigit(attr[0] & 255)) {
                // Absolute size
                fsize = (int)(textsize_ * pow(1.2, atof(attr) - 3.0));
              } else {
                // Relative size
                fsize = (int)(fsize * pow(1.2, atof(attr) - 3.0));
              }
            }

            pushfont(font, fsize);
          }
          else if (buf.cmp("/FONT"))
          {
            popfont(font, fsize, textcolor_);
          }
          else if (buf.cmp("U"))
            underline = 1;
          else if (buf.cmp("/U"))
            underline = 0;
          else if (buf.cmp("B") ||
                   buf.cmp("STRONG"))
            pushfont(font |= FL_BOLD, fsize);
          else if (buf.cmp("TD") ||
                   buf.cmp("TH"))
          {
            int tx, ty, tw, th;

            if (tolower(buf[1]) == 'h')
              pushfont(font |= FL_BOLD, fsize);
            else
              pushfont(font = textfont_, fsize);

            tx = block->x - 4 - leftline_;
            ty = block->y - topline_ - fsize - 3;
            tw = block->w - block->x + 7;
            th = block->h + fsize - 5;

            if (tx < 0)
            {
              tw += tx;
              tx  = 0;
            }

            if (ty < 0)
            {
              th += ty;
              ty  = 0;
            }

            tx += view.x();
            ty += view.y();

            if (block->bgcolor != bgcolor_)
            {
              fl_color(block->bgcolor);
              fl_rectf(tx, ty, tw, th);
              fl_color(textcolor_);
            }

            if (block->border)
              fl_rect(tx, ty, tw, th);
          }
          else if (buf.cmp("I") ||
                   buf.cmp("EM"))
            pushfont(font |= FL_ITALIC, fsize);
          else if (buf.cmp("CODE") ||
                   buf.cmp("TT"))
            pushfont(font = FL_COURIER, fsize);
          else if (buf.cmp("KBD"))
            pushfont(font = FL_COURIER_BOLD, fsize);
          else if (buf.cmp("VAR"))
            pushfont(font = FL_COURIER_ITALIC, fsize);
          else if (buf.cmp("/HEAD"))
            head = 0;
          else if (buf.cmp("/H1") ||
                   buf.cmp("/H2") ||
                   buf.cmp("/H3") ||
                   buf.cmp("/H4") ||
                   buf.cmp("/H5") ||
                   buf.cmp("/H6") ||
                   buf.cmp("/B") ||
                   buf.cmp("/STRONG") ||
                   buf.cmp("/I") ||
                   buf.cmp("/EM") ||
                   buf.cmp("/CODE") ||
                   buf.cmp("/TT") ||
                   buf.cmp("/KBD") ||
                   buf.cmp("/VAR"))
            popfont(font, fsize, fcolor);
          else if (buf.cmp("/PRE"))
          {
            popfont(font, fsize, fcolor);
            pre = 0;
          }
          else if (buf.cmp("IMG"))
          {
            Fl_Shared_Image *img = 0;
            int         width, height;
            char        wattr[8], hattr[8];


            get_attr(attrs, "WIDTH", wattr, sizeof(wattr));
            get_attr(attrs, "HEIGHT", hattr, sizeof(hattr));
            width  = get_length(wattr);
            height = get_length(hattr);

            if (get_attr(attrs, "SRC", attr, sizeof(attr))) {
              img = get_image(attr, width, height);
              if (!width) width = img->w();
              if (!height) height = img->h();
            }

            if (!width || !height) {
              if (get_attr(attrs, "ALT", attr, sizeof(attr)) == nullptr) {
                strcpy(attr, "IMG");
              }
            }

            ww = width;

            if (needspace && xx > block->x)
              xx += (int)fl_width(' ');

            if ((xx + ww) > block->w)
            {
              if (line < 31)
                line ++;

              xx = block->line[line];
              yy += hh;
              hh = 0;
            }

            if (img) {
              img->draw(xx + view.x() - leftline_,
                        yy + view.y() - fl_height() + fl_descent() + 2);
            }

            xx += ww;
            if ((height + 2) > hh)
              hh = height + 2;

            needspace = 0;
          }
          buf.clear();
        }
        else if (*ptr == '\n' && pre)
        {
          hv_draw(buf.c_str(), xx + view.x() - leftline_, yy + view.y());
          buf.clear();

          if (line < 31)
            line ++;
          xx = block->line[line];
          yy += hh;
          hh = fsize + 2;
          needspace = 0;

          ptr ++;
          current_pos_ = (int) (ptr-value_);
        }
        else if (isspace((*ptr)&255))
        {
          if (pre)
          {
            if (*ptr == ' ')
              buf += ' ';
            else
            {
              // Do tabs every 8 columns...
              buf += ' '; // at least one space
              while (buf.size() & 7)
                buf += ' ';
            }
          }

          ptr ++;
          if (!pre) current_pos_ = (int) (ptr-value_);
          needspace = 1;
        }
        else if (*ptr == '&') // process html entity
        {
          ptr ++;

          int qch = quote_char(ptr);

          if (qch < 0)
            buf += '&';
          else {
            size_t utf8l = buf.size();
            buf.add(qch);
            utf8l = buf.size() - utf8l; // length of added UTF-8 text
            const char *oldptr = ptr;
            ptr = strchr(ptr, ';') + 1;
            entity_extra_length += int(ptr - (oldptr-1)) - utf8l; // extra length between html entity and UTF-8
          }

          if ((fsize + 2) > hh)
            hh = fsize + 2;
        }
        else
        {
          buf += *ptr++;

          if ((fsize + 2) > hh)
            hh = fsize + 2;
        }
      }

      if (buf.size() > 0 && !pre && !head)
      {
        ww = buf.width();

        if (needspace && xx > block->x)
          xx += (int)fl_width(' ');

        if ((xx + ww) > block->w)
        {
          if (line < 31)
            line ++;
          xx = block->line[line];
          yy += hh;
          hh = 0;
        }
      }

      if (buf.size() > 0 && !head)
      {
        hv_draw(buf.c_str(), xx + view.x() - leftline_, yy + view.y());
        if (underline) fl_xyline(xx + view.x() - leftline_, yy + view.y() + 1,
                                 xx + view.x() - leftline_ + ww);
        current_pos_ = (int) (ptr-value_);
      }
    }

  fl_pop_clip();
} // draw()


/**
  \brief Creates the Fl_Help_View widget at the specified position and size.
  \param[in] xx, yy, ww, hh Position and size of the widget
  \param[in] l Label for the widget, can be nullptr
*/
Fl_Help_View::Fl_Help_View(int xx, int yy, int ww, int hh, const char *l)
: Fl_Group(xx, yy, ww, hh, l),
  impl_(new Fl_Help_View::Impl(this)),
  scrollbar_(xx + ww - Fl::scrollbar_size(), yy, Fl::scrollbar_size(), hh - Fl::scrollbar_size()),
  hscrollbar_(xx, yy + hh - Fl::scrollbar_size(), ww - Fl::scrollbar_size(), Fl::scrollbar_size())
{
  color(FL_BACKGROUND2_COLOR, FL_SELECTION_COLOR);

  scrollbar_.value(0, hh, 0, 1);
  scrollbar_.step(8.0);
  scrollbar_.show();
  scrollbar_.callback( [](Fl_Widget *s, void *u) {
      ((Fl_Help_View*)u)->topline((int)(((Fl_Scrollbar*)s)->value()));
    }, this );

  hscrollbar_.value(0, ww, 0, 1);
  hscrollbar_.step(8.0);
  hscrollbar_.show();
  hscrollbar_.type(FL_HORIZONTAL);
  hscrollbar_.callback( [](Fl_Widget *s, void *u) {
      ((Fl_Help_View*)u)->leftline(int(((Fl_Scrollbar*)s)->value()));
    }, this );

  end();

  resize(xx, yy, ww, hh);
}


/**
  \brief Destroys the Fl_Help_View widget.

  The destructor destroys the widget and frees all memory that has been
  allocated for the current document.
*/
Fl_Help_View::~Fl_Help_View()
{
}


/**
  \brief Handles events in the widget.
  \param[in] event Event to handle.
  \return 1 if the event was handled, 0 otherwise.
 */
int Fl_Help_View::handle(int event)
{
  return impl_->handle(event);
}

/**
  \brief Handles events in the widget.
  \see Fl_Help_View::handle(int event)
 */
int Fl_Help_View::Impl::handle(int event)
{
  static std::shared_ptr<Link> linkp = nullptr;   // currently clicked link

  int xx = Fl::event_x() - view.x() + leftline_;
  int yy = Fl::event_y() - view.y() + topline_;

  switch (event)
  {
    case FL_FOCUS:
      // Selection style changes, so ask for a redraw
      if (selected_)
        view.redraw();
      return 1;
    case FL_UNFOCUS:
      // Selection style changes, so ask for a redraw
      if (selected_)
        view.redraw();
      return 1;
    case FL_ENTER :
      view.Fl_Group::handle(event);
      return 1;
    case FL_LEAVE :
      fl_cursor(FL_CURSOR_DEFAULT);
      break;
    case FL_MOVE:
      if (find_link(xx, yy)) fl_cursor(FL_CURSOR_HAND);
      else fl_cursor(FL_CURSOR_DEFAULT);
      return 1;
    case FL_PUSH:
      // RMB will pop up a menu
      if (Fl::event_button() == FL_RIGHT_MOUSE) {
        rmb_menu[0].label(view.copy_menu_text);
        if (text_selected())
          rmb_menu[0].activate();
        else
          rmb_menu[0].deactivate();
        fl_cursor(FL_CURSOR_DEFAULT);
        const Fl_Menu_Item *mi = rmb_menu->popup(Fl::event_x(), Fl::event_y());
        if (mi) switch (mi->argument()) {
          case 1:
            copy();
            break;
        }
        return 1;
      }
      // Check if the scrollbars used up the event
      if (view.Fl_Group::handle(event))
        return 1;
      // Check if the user clicked on a link
      linkp = find_link(xx, yy);
      if (linkp) {
        fl_cursor(FL_CURSOR_HAND);
        return 1;
      }
      // If nothing else, the user cancels the current selection and might start a new one
      if (begin_selection()) {
        selection_mode_ = Mode::PUSH;
        fl_cursor(FL_CURSOR_INSERT);
        return 1;
      }
      // Nothing to do.
      fl_cursor(FL_CURSOR_DEFAULT);
      return 1;
    case FL_DRAG:
      // If we clicked on a link, check if this remains a click, or if the user drags the mouse
      if (linkp) {
        if (Fl::event_is_click()) {
          fl_cursor(FL_CURSOR_HAND);
        } else {
          // No longer just a click, so we cancel the link and start a drag selection
          linkp = 0;
          if (begin_selection()) {
            selection_mode_ = Mode::PUSH;
            fl_cursor(FL_CURSOR_INSERT);
          }
        }
      }
      // If the FL_PUSH started a selection, we extend the selection
      if (selection_mode_ == Mode::PUSH) {
        if (extend_selection())
          view.redraw();
        fl_cursor(FL_CURSOR_INSERT);
        return 1;
      }
      // Nothing to do.
      fl_cursor(FL_CURSOR_DEFAULT);
      return 1;
    case FL_RELEASE:
      // If we clicked on a link, follow it
      if (linkp) {
        if (Fl::event_is_click()) {
          follow_link(linkp);
        }
        fl_cursor(FL_CURSOR_DEFAULT);
        linkp = 0;
        return 1;
      }
      // If in a selection process, end the selection.
      if (selection_mode_ == Mode::PUSH) {
        end_selection();
        selection_mode_ = Mode::DRAW;
        return 1;
      }
      // Nothing to do.
      return 1;
    case FL_SHORTCUT: {
      int mods = Fl::event_state() & (FL_META|FL_CTRL|FL_ALT|FL_SHIFT);
      if ( mods == FL_COMMAND) {
        switch ( Fl::event_key() ) {
          case 'a': select_all(); view.redraw(); return 1;
          case 'c':
          case 'x': copy(1); return 1;
        }
      }
      break; }
  }
  return (view.Fl_Group::handle(event));
}


/**
  \brief Override the superclass's resize method.
  \param[in] xx, yy, ww, hh New position and size of the widget
 */
void Fl_Help_View::resize(int xx, int yy, int ww, int hh)
{
  impl_->resize(xx, yy, ww, hh);
}

/**
  \brief Override the superclass's resize method.
  \see Fl_Help_View::resize(int xx, int yy, int ww, int hh)
 */
void Fl_Help_View::Impl::resize(int xx, int yy, int ww, int hh)
{
  Fl_Boxtype b = view.box() ? view.box() : FL_DOWN_BOX; // Box to draw...

  view.Fl_Widget::resize(xx, yy, ww, hh);

  int scrollsize = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size();
  view.scrollbar_.resize(view.x() + view.w() - scrollsize - Fl::box_dw(b) + Fl::box_dx(b),
                    view.y() + Fl::box_dy(b), scrollsize, view.h() - scrollsize - Fl::box_dh(b));
  view.hscrollbar_.resize(view.x() + Fl::box_dx(b),
                     view.y() + view.h() - scrollsize - Fl::box_dh(b) + Fl::box_dy(b),
                     view.w() - scrollsize - Fl::box_dw(b), scrollsize);
  format();
}


// ---- HTML source and raw data

/**
  \brief Sets the current help text buffer to the string provided and reformats the text.

  The provided character string \p val is copied internally and will be
  freed when value() is called again, or when the widget is destroyed.

  If \p val is nullptr, then the widget is cleared.

  \param[in] val Text to view, or nullptr to clear the widget,
      Fl_Help_View will creat a local copy of the string.
*/
void Fl_Help_View::value(const char *val) {
  impl_->value(val);
}

/**
  \brief Sets the current help text buffer to the string provided and reformats the text.
  \see Fl_Help_View::value(const char *val)
 */
void Fl_Help_View::Impl::value(const char *val)
{
  clear_selection();
  free_data();
  view.set_changed();

  if (!val)
    return;

  value_ = fl_strdup(val);

  initial_load = 1;
  format();
  initial_load = 0;

  topline(0);
  leftline(0);
}


/**
  \brief Loads the specified file.

  This method loads the specified file or URL. The filename may end in a
  \c \#name style target.

  If the URL starts with \a ftp, \a http, \a https, \a ipp, \a mailto, or
  \a news, followed by a colon, FLTK will use fl_open_uri() to show the
  requested page in an external browser.

  In all other cases, the URL is interpreted as a filename. The file is read and
  displayed in this browser. Note that Windows style backslashes are not
  supported in the file name.

  \param[in] f filename or URL
  \return 0 on success, -1 on error

  \see fl_open_uri()
*/
int Fl_Help_View::load(const char *f) {
  return impl_->load(f);
}

/**
  \brief Loads the specified file.
  \see Fl_Help_View::load(const char *f)
 */
int Fl_Help_View::Impl::load(const char *f)
{
  FILE          *fp;            // File to read from
  long          len;            // Length of file
  std::string target;        // Target in file
  std::string localname;     // Local filename
  std::string error; // Error buffer
  std::string newname;   // New filename buffer

  // printf("load(%s)\n",f); fflush(stdout);

  if (strncmp(f, "ftp:", 4) == 0 ||
      strncmp(f, "http:", 5) == 0 ||
      strncmp(f, "https:", 6) == 0 ||
      strncmp(f, "ipp:", 4) == 0 ||
      strncmp(f, "mailto:", 7) == 0 ||
      strncmp(f, "news:", 5) == 0)
  {
    char urimsg[FL_PATH_MAX]; // Use of static size ok.
    if ( fl_open_uri(f, urimsg, sizeof(urimsg)) == 0 ) {
      clear_selection();

      newname = f;
      size_t hash_pos = newname.rfind('#');
      if (hash_pos != std::string::npos) {
        target = newname.substr(hash_pos + 1);
        newname.resize(hash_pos);
      }

      if (link_) {
        const char *n = (*link_)(&view, newname.c_str());
        if (n == nullptr)
          return 0;
        localname = n;
      } else {
        localname = newname;
      }

      free_data();

      filename_ = newname;

      // Note: We do not support Windows backslashes, since they are illegal
      //       in URLs...
      directory_ = newname;
      size_t slash_pos = directory_.rfind('/');
      if (slash_pos == std::string::npos) {
        directory_.clear();
      } else if ((slash_pos > 0) && (directory_[slash_pos-1] != '/')) {
        directory_.resize(slash_pos);
      }

      error = "<HTML><HEAD><TITLE>Error</TITLE></HEAD>"
              "<BODY><H1>Error</H1>"
              "<P>Unable to follow the link \""
              + std::string(f) + "\" - " + std::string(urimsg) + ".</P></BODY>";
      value(error.c_str());
      return -1;
    } else {
      return 0;
    }
  }

  clear_selection();

  newname = f;
  size_t hash_pos = newname.rfind('#');
  if (hash_pos != std::string::npos) {
    target = newname.substr(hash_pos + 1);
    newname.resize(hash_pos);
  }

  if (link_) {
    const char *n = (*link_)(&view, newname.c_str());
    if (n == nullptr)
      return -1;
    localname = n;
  } else {
    localname = newname;
  }

  free_data();

  filename_ = newname;
  directory_ = newname;

  // Note: We do not support Windows backslashes, since they are illegal
  //       in URLs...
  size_t slash_pos = directory_.rfind('/');
  if (slash_pos == std::string::npos) {
    directory_.clear();
  } else if ((slash_pos > 0) && (directory_[slash_pos-1] != '/')) {
    directory_.resize(slash_pos);
  }

  if (localname.find("file:") == 0) {
    localname.erase(0, 5);     // Adjust for local filename...
  }

  int ret = 0;
  if ((fp = fl_fopen(localname.c_str(), "rb")) != nullptr)
  {
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    rewind(fp);

    value_ = (const char *)calloc(len + 1, 1);
    if (fread((void *)value_, 1, len, fp)==0) { /* use default 0 */ }
    fclose(fp);
  }
  else
  {
    error = "<HTML><HEAD><TITLE>Error</TITLE></HEAD>"
            "<BODY><H1>Error</H1>"
            "<P>Unable to follow the link \"" +localname + "\" - "
            + strerror(errno) + ".</P></BODY>";
    value_ = fl_strdup(error.c_str());
    ret = -1;
  }

  initial_load = 1;
  format();
  initial_load = 0;

  if (!target.empty())
    topline(target.c_str());
  else
    topline(0);

  return ret;
}


/**
  \brief Finds the specified string \p s at starting position \p p.

  The argument \p p and the return value are offsets in Fl_Help_View::value(),
  counting from 0. If \p p is out of range, 0 is used.

  The string comparison is simple but honors some special cases:
  - the specified string \p s must be in UTF-8 encoding
  - HTML tags in value() are filtered (not compared as such, they never match)
  - HTML entities like '\&lt;' or '\&x#20ac;' are converted to Unicode (UTF-8)
  - ASCII characters (7-bit, \< 0x80) are compared case insensitive
  - every newline (LF, '\\n') in value() is treated like a single space
  - all other strings are compared as-is (byte by byte)

  \param[in] s search string in UTF-8 encoding
  \param[in] p starting position for search (0,...), Default = 0
  \return the matching position or -1 if not found
*/
int Fl_Help_View::find(const char *s, int p) {
  return impl_->find(s, p);
}

/**
  \brief Finds the specified string \p s at starting position \p p.
  \see Fl_Help_View::find(const char *s, int p)
 */
int Fl_Help_View::Impl::find(const char *s, int p)
{
  int           i,                              // Looping var
                c;                              // Current character
  Text_Block *b;                             // Current block
  const char    *bp,                            // Block matching pointer
                *bs,                            // Start of current comparison
                *sp;                            // Search string pointer

  DEBUG_FUNCTION(__LINE__,__FUNCTION__);

  // Range check input and value...
  if (!s || !value_) return -1;

  if (p < 0 || p >= (int)strlen(value_)) p = 0;

  // Look for the string...
  for (i = (int)blocks_.size(), b = &blocks_[0]; i > 0; i--, b++) {
    if (b->end < (value_ + p))
      continue;

    if (b->start < (value_ + p))
      bp = value_ + p;
    else
      bp = b->start;

    bp = vanilla(bp, b->end);
    if (bp == b->end)
      continue;

    for (sp = s, bs = bp; *sp && *bp && bp < b->end; ) {
      bool is_html_entity = false;
      if (*bp == '&') {
        // decode HTML entity...
        if ((c = quote_char(bp + 1)) < 0) {
          c = '&';
        } else {
          const char *entity_end = strchr(bp + 1, ';');
          if (entity_end) {
            is_html_entity = true; // c contains the unicode character
            bp = entity_end;
          } else {
            c = '&';
          }
        }
      } else {
        c = *bp;
      }

      if (c == '\n') c = ' '; // treat newline as a single space

      // *FIXME* *UTF-8* (A.S. 02/14/2016)
      // At this point c may be an arbitrary Unicode Code Point corresponding
      // to a quoted character (see above), i.e. it _can_ be a multi byte
      // UTF-8 sequence and must be compared with the corresponding
      // multi byte string in (*sp)...
      // For instance: "&euro;" == 0x20ac -> 0xe2 0x82 0xac (UTF-8: 3 bytes).
      // Hint: use fl_utf8encode() [see below]

      int utf_len = 1;
      if (c > 0x20 && c < 0x80 && tolower(*sp) == tolower(c)) {
        // Check for ASCII case insensitive match.
        //printf("%ld text match %c/%c\n", bp-value_, *sp, c);
        sp++;
        bp = vanilla(bp+1, b->end);
      } else if (is_html_entity && fl_utf8decode(sp, nullptr, &utf_len) == (unsigned int)c ) {
        // Check if a &lt; entity ini html matches a UTF-8 character in the
        // search string.
        //printf("%ld unicode match 0x%02X 0x%02X\n", bp-value_, *sp, c);
        sp += utf_len;
        bp = vanilla(bp+1, b->end);
      } else if (*sp == c) {
        // Check if UTF-8 bytes in html and the search string match.
        //printf("%ld binary match %c/%c\n", bp-value_, *sp, c);
        sp++;
        bp = vanilla(bp+1, b->end);
      } else {
        // No match, so reset to start of search... .
        //printf("reset search (%c/%c)\n", *sp, c);
        sp = s;
        bp = bs = vanilla(bs+1, b->end);
      }
    }

    if (!*sp) { // Found a match!
      topline(b->y - b->h);
      return int(bs - value_);
    }
  }

  // No match!
  return (-1);
}


/**
  \brief Set a callback function for following links.

  This method assigns a callback function to use when a link is
  followed or a file is loaded (via Fl_Help_View::load()) that
  requires a different file or path.

  The callback function receives a pointer to the Fl_Help_View
  widget and the URI or full pathname for the file in question.
  It must return a pathname that can be opened as a local file or nullptr:

  \code
  const char *fn(Fl_Widget *w, const char *uri);
  \endcode

  The link function can be used to retrieve remote or virtual
  documents, returning a temporary file that contains the actual
  data. If the link function returns nullptr, the value of
  the Fl_Help_View widget will remain unchanged.

  If the link callback cannot handle the URI scheme, it should
  return the uri value unchanged or set the value() of the widget
  before returning nullptr.

  \param[in] fn Pointer to the callback function
*/
void Fl_Help_View::link(Fl_Help_Func *fn) {
  impl_->link(fn);
}

/**
  \brief Set a callback function for following links.
  \see Fl_Help_View::link(Fl_Help_Func *fn)
 */
void Fl_Help_View::Impl::link(Fl_Help_Func *fn)
{
  link_ = fn;
}


/**
  \brief Return the current filename for the text in the buffer.

  Fl_Help_View remains the owner of the allocated memory. If the filename
  changes, the returned pointer will become stale.

  \return nullptr if the filename is empty
*/
const char *Fl_Help_View::filename() const {
  return impl_->filename(); // Ensure the filename is up to date
}

/**
  \brief Return the current filename for the text in the buffer.
  \see Fl_Help_View::filename() const
 */
const char *Fl_Help_View::Impl::filename() const {
  if (filename_.empty())
    return nullptr;
  else
    return filename_.c_str();
}


/**
  \brief Return the current directory for the text in the buffer.

  Fl_Help_View remains the owner of the allocated memory. If the directory
  changes, the returned pointer will become stale.

  \return nullptr if the directory name is empty
*/
const char *Fl_Help_View::directory() const {
  return impl_->directory(); // Ensure the directory is up to date
}

/**
  \brief Return the current directory for the text in the buffer.
  \see Fl_Help_View::directory() const
 */
const char *Fl_Help_View::Impl::directory() const {
  if (directory_.empty())
    return nullptr;
  else
    return directory_.c_str();
}


/**
  \brief Return the title of the current document.

  Fl_Help_View remains the owner of the allocated memory. If the document
  changes, the returned pointer will become stale.

  \return empty string if the directory name is empty
 */
const char *Fl_Help_View::title() const {
  return impl_->title(); // Ensure the title is up to date
}

/**
  \brief Return the title of the current document.
  \see Fl_Help_View::title() const
 */
const char *Fl_Help_View::Impl::title() const
{
  return title_.c_str();
}


// ---- Rendering attributes

/**
  \brief Scroll the text to the given anchor.
  \param[in] anchor scroll to this named anchor
*/
void Fl_Help_View::topline(const char *anchor) {
  impl_->topline(anchor);
}

/**
  \brief Scroll the text to the given anchor.
  \see Fl_Help_View::topline(const char *anchor)
 */
void Fl_Help_View::Impl::topline(const char *anchor)
{
  std::string target_name = to_lower(anchor); // Convert to lower case
  auto tl = target_line_map_.find(target_name);
  if (tl != target_line_map_.end()) {
    // Found the target name, scroll to the line
    topline(tl->second);
  } else {
    // Scroll to the top.
    topline(0);
  }
}


/**
  \brief Scrolls the text to the indicated position, given a pixel line.

  If the given pixel value \p top is out of range, then the text is
  scrolled to the top or bottom of the document, resp.

  \param[in] top top line number in pixels (0 = start of document)
*/
void Fl_Help_View::topline(int top) {
  impl_->topline(top);
}

/**
  \brief Scrolls the text to the indicated position, given a pixel line.
  \see Fl_Help_View::topline(int top)
 */
void Fl_Help_View::Impl::topline(int top)
{
  if (!value_)
    return;

  int scrollsize = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size();
  if (size_ < (view.h() - scrollsize) || top < 0)
    top = 0;
  else if (top > size_)
    top = size_;

  topline_ = top;

  view.scrollbar_.value(topline_, view.h() - scrollsize, 0, size_);

  view.do_callback(FL_REASON_DRAGGED);

  view.redraw();
}


/**
  \brief Scrolls the text to the indicated position, given a pixel column.

  If the given pixel value \p left is out of range, then the text is
  scrolled to the left or right side of the document, resp.

  \param[in] left left column number in pixels (0 = left side)
*/
void Fl_Help_View::leftline(int left) {
  impl_->leftline(left);
}

/**
 \brief Scrolls the text to the indicated position, given a pixel column.
 \see Fl_Help_View::leftline(int left)
 */
void Fl_Help_View::Impl::leftline(int left)
{
  if (!value_)
    return;

  int scrollsize = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size();
  if (hsize_ < (view.w() - scrollsize) || left < 0)
    left = 0;
  else if (left > hsize_)
    left = hsize_;

  leftline_ = left;

  view.hscrollbar_.value(leftline_, view.w() - scrollsize, 0, hsize_);

  view.redraw();
}


// ---- Text selection

/**
  \brief Removes the current text selection.
*/
void Fl_Help_View::clear_selection() {
  impl_->clear_selection();
}

/**
  \brief Removes the current text selection.
  \see Fl_Help_View::Impl::clear_selection()
 */
void Fl_Help_View::Impl::clear_selection()
{
  selected_ = false;
  selection_first_ = 0;
  selection_last_ = 0;
  view.redraw();
}


/**
  \brief Selects all the text in the view.
*/
void Fl_Help_View::select_all() {
  impl_->select_all();
}

/**
  \brief Selects all the text in the view.
  Fl_Help_View::Impl::select_all()
 */
void Fl_Help_View::Impl::select_all()
{
  clear_selection();
  if (!value_) return;
  selection_first_ = 0;
  selection_last_ = (int) strlen(value_);
  selected_ = true;
}


/**
  \brief Check if the user selected text in this view.
  \return 1 if text is selected, 0 if no text is selected
 */
int Fl_Help_View::text_selected() const {
  return impl_->text_selected();
}

/**
 \brief Check if the user selected text in this view.
 \see Fl_Help_View::text_selected()
 */
int Fl_Help_View::Impl::text_selected() const
{
  return selected_;
}


/**
  \brief If text is selected in this view, copy it to a clipboard.
  \param[in] clipboard for x11 only, 0=selection buffer, 1=clipboard, 2=both
  \return 1 if text is selected, 0 if no text is selected
 */
int Fl_Help_View::copy(int clipboard) {
  return impl_->copy(clipboard);
}

/**
  \brief If text is selected in this view, copy it to a clipboard.
  \see Fl_Help_View::copy(int clipboard)
 */
int Fl_Help_View::Impl::copy(int clipboard)
{
  if (!selected_)
    return 0;

  // convert the select part of our html text into some kind of somewhat readable UTF-8
  // and store it in the selection buffer
  int p = 0;
  char pre = 0;
  int len = (int) strlen(value_);
  char *txt = (char*)malloc(len+1), *d = txt;
  const char *s = value_, *cmd, *src;
  for (;;) {
    int c = (*s++) & 0xff;
    if (c==0) break;
    if (c=='<') { // begin of some html command. Skip until we find a '>'
      cmd = s;
      for (;;) {
        c = (*s++) & 0xff;
        if (c==0 || c=='>') break;
      }
      if (c==0) break;
      // do something with this command... .
      // The replacement string must not be longer than the command
      // itself plus '<' and '>'
      src = 0;
      switch (command(cmd)) {
        case CMD('p','r','e', 0 ): pre = 1; break;
        case CMD('/','p','r','e'): pre = 0; break;
        case CMD('t','d', 0 , 0 ):
        case CMD('p', 0 , 0 , 0 ):
        case CMD('/','p', 0 , 0 ):
        case CMD('b','r', 0 , 0 ): src = "\n"; break;
        case CMD('l','i', 0 , 0 ): src = "\n * "; break;
        case CMD('/','h','1', 0 ):
        case CMD('/','h','2', 0 ):
        case CMD('/','h','3', 0 ):
        case CMD('/','h','4', 0 ):
        case CMD('/','h','5', 0 ):
        case CMD('/','h','6', 0 ): src = "\n\n"; break;
        case CMD('t','r', 0 , 0 ):
        case CMD('h','1', 0 , 0 ):
        case CMD('h','2', 0 , 0 ):
        case CMD('h','3', 0 , 0 ):
        case CMD('h','4', 0 , 0 ):
        case CMD('h','5', 0 , 0 ):
        case CMD('h','6', 0 , 0 ): src = "\n\n"; break;
        case CMD('d','t', 0 , 0 ): src = "\n "; break;
        case CMD('d','d', 0 , 0 ): src = "\n - "; break;
      }
      int n = (int) (s-value_);
      if (src && n>selection_first_ && n<=selection_last_) {
        while (*src) {
          *d++ = *src++;
        }
        c = src[-1] & 0xff;
        p = isspace(c) ? ' ' : c;
      }
      continue;
    }
    const char *s2 = s;
    if (c=='&') { // special characters (HTML entities)
      int xx = quote_char(s);
      if (xx >= 0) {
        c = xx;
        for (;;) {
          char cc = *s++;
          if (!cc || cc==';') break;
        }
      }
    }
    int n = (int) (s2-value_);
    if (n>selection_first_ && n<=selection_last_) {
      if (!pre && c < 256 && isspace(c)) c = ' ';
      if (p != ' ' || c != ' ') {
        if (s2 != s) { // c was an HTML entity
          d += fl_utf8encode(c, d);
        }
        else *d++ = c;
      }
      p = c;
    }
    if (n>selection_last_) break; // stop parsing html after end of selection
  }
  *d = 0;
  Fl::copy(txt, (int) strlen(txt), clipboard);
  // printf("copy [%s]\n", txt);
  free(txt);
  return 1;
}


// ---- Scroll bars

/**
  \brief Get the current size of the scrollbars' troughs, in pixels.

  If this value is zero (default), this widget will use the
  Fl::scrollbar_size() value as the scrollbar's width.

  \returns Scrollbar size in pixels, or 0 if the global Fl::scrollbar_size() is being used.
  \see Fl::scrollbar_size(int)
*/
int Fl_Help_View::scrollbar_size() const {
  return impl_->scrollbar_size();
}

/**
  \brief Get the current size of the scrollbars' troughs, in pixels.
  \see Fl_Help_View::scrollbar_size() const
 */
int Fl_Help_View::Impl::scrollbar_size() const {
  return(scrollbar_size_);
}


/**
  \brief Set the pixel size of the scrollbars' troughs to \p newSize, in pixels.

  Normally you should not need this method, and should use
  Fl::scrollbar_size(int) instead to manage the size of ALL
  your widgets' scrollbars. This ensures your application
  has a consistent UI, is the default behavior, and is normally
  what you want.

  Only use THIS method if you really need to override the global
  scrollbar size. The need for this should be rare.

  Setting \p newSize to the special value of 0 causes the widget to
  track the global Fl::scrollbar_size(), which is the default.

  \param[in] newSize Sets the scrollbar size in pixels.\n
      If 0 (default), scrollbar size tracks the global Fl::scrollbar_size()
  \see Fl::scrollbar_size()
*/
void Fl_Help_View::scrollbar_size(int newSize) {
  impl_->scrollbar_size(newSize);
}

/**
  \brief Set the pixel size of the scrollbars' troughs to \p newSize, in pixels.
  \see Fl_Help_View::scrollbar_size(int)
 */
void Fl_Help_View::Impl::scrollbar_size(int newSize)
{
    scrollbar_size_ = newSize;
}


/**
  \brief Skips over HTML tags in a text.

  In an html style text, set the character pointer p, skipping anything from a
  leading '<' up to and including the closing '>'. If the end of the buffer is
  reached, the function returns `end`.

  No need to handle UTF-8 here.

  \param[in] p pointer to html text, UTF-8 characters possible
  \param[in] end pointer to the end of the text (need nut be NUL)
  \return new pointer to text after skipping over '<...>' blocks, or `end`
    if NUL was found or a '<...>' block was not closed.
*/
static const char *vanilla(const char *p, const char *end) {
  if (*p == '\0' || p >= end) return end;
  for (;;) {
    if (*p != '<') {
      return p;
    } else {
      while (*p && p < end && *p != '>') p++;
    }
    p++;
    if (*p == '\0' || p >= end) return end;
  }
}



// convert a command with up to four letters into an unsigned int
static uint32_t command(const char *cmd)
{
  uint32_t ret = (tolower(cmd[0])<<24);
  char c = cmd[1];
  if (c=='>' || c==' ' || c==0) return ret;
  ret |= (tolower(c)<<16);
  c = cmd[2];
  if (c=='>' || c==' ' || c==0) return ret;
  ret |= (tolower(c)<<8);
  c = cmd[3];
  if (c=='>' || c==' ' || c==0) return ret;
  ret |= tolower(c);
  c = cmd[4];
  if (c=='>' || c==' ' || c==0) return ret;
  return 0;
}


// ------ Some more helper functions


/*
  \brief Returns the Unicode Code Point associated with a quoted character (aka "HTML Entity").

  Possible encoding formats:
    - `&name;`           named entity
    - `&#nn..;`          numeric (decimal) Unicode Code Point
    - `&#xnn..;`         numeric (hexadecimal) Unicode Code Point
    - `&#Xnn..;`         numeric (hexadecimal) Unicode Code Point
  `nn..` = decimal or hexadecimal number, resp.

  Contents of the table `names[]` below:

  All printable ASCII (32-126) and ISO-8859-1 (160-255) characters
  are encoded with the same value in Unicode. Special characters
  outside the range [0-255] are encoded with their Unicode Code Point
  as hexadecimal constants. Example:
    - Euro sign: (Unicode) U+20ac = (hex) 0x20ac

  \note Converted to correct Unicode values and tested (compared with
  the display of Firefox). AlbrechtS, 14 Feb. 2016.

  \note if you add or remove items to/from this list, please
  update the documentation for Fl_Help_View::Fl_Help_View().

  \param[in] p Pointer to the quoted character string, e.g. `&copy;` or `&#169;`
  \return the Unicode Code Point for the quoted character, or -1 if not found.
*/
static int quote_char(const char *p) {
  int i;
  static const struct {
    const char  *name;
    int         namelen;
    int         code;
  } *nameptr,   // Pointer into name array
    names[] =   // Quoting names
  {
    { "Aacute;", 7, 193 },
    { "aacute;", 7, 225 },
    { "Acirc;",  6, 194 },
    { "acirc;",  6, 226 },
    { "acute;",  6, 180 },
    { "AElig;",  6, 198 },
    { "aelig;",  6, 230 },
    { "Agrave;", 7, 192 },
    { "agrave;", 7, 224 },
    { "amp;",    4, '&' },
    { "Aring;",  6, 197 },
    { "aring;",  6, 229 },
    { "Atilde;", 7, 195 },
    { "atilde;", 7, 227 },
    { "Auml;",   5, 196 },
    { "auml;",   5, 228 },
    { "brvbar;", 7, 166 },
    { "bull;",   5, 0x2022 },
    { "Ccedil;", 7, 199 },
    { "ccedil;", 7, 231 },
    { "cedil;",  6, 184 },
    { "cent;",   5, 162 },
    { "copy;",   5, 169 },
    { "curren;", 7, 164 },
    { "dagger;", 7, 0x2020 },
    { "deg;",    4, 176 },
    { "divide;", 7, 247 },
    { "Eacute;", 7, 201 },
    { "eacute;", 7, 233 },
    { "Ecirc;",  6, 202 },
    { "ecirc;",  6, 234 },
    { "Egrave;", 7, 200 },
    { "egrave;", 7, 232 },
    { "ETH;",    4, 208 },
    { "eth;",    4, 240 },
    { "Euml;",   5, 203 },
    { "euml;",   5, 235 },
    { "euro;",   5, 0x20ac },
    { "frac12;", 7, 189 },
    { "frac14;", 7, 188 },
    { "frac34;", 7, 190 },
    { "gt;",     3, '>' },
    { "Iacute;", 7, 205 },
    { "iacute;", 7, 237 },
    { "Icirc;",  6, 206 },
    { "icirc;",  6, 238 },
    { "iexcl;",  6, 161 },
    { "Igrave;", 7, 204 },
    { "igrave;", 7, 236 },
    { "iquest;", 7, 191 },
    { "Iuml;",   5, 207 },
    { "iuml;",   5, 239 },
    { "laquo;",  6, 171 },
    { "lt;",     3, '<' },
    { "macr;",   5, 175 },
    { "micro;",  6, 181 },
    { "middot;", 7, 183 },
    { "nbsp;",   5, ' ' },
    { "ndash;",  6, 0x2013 },
    { "not;",    4, 172 },
    { "Ntilde;", 7, 209 },
    { "ntilde;", 7, 241 },
    { "Oacute;", 7, 211 },
    { "oacute;", 7, 243 },
    { "Ocirc;",  6, 212 },
    { "ocirc;",  6, 244 },
    { "Ograve;", 7, 210 },
    { "ograve;", 7, 242 },
    { "ordf;",   5, 170 },
    { "ordm;",   5, 186 },
    { "Oslash;", 7, 216 },
    { "oslash;", 7, 248 },
    { "Otilde;", 7, 213 },
    { "otilde;", 7, 245 },
    { "Ouml;",   5, 214 },
    { "ouml;",   5, 246 },
    { "para;",   5, 182 },
    { "permil;", 7, 0x2030 },
    { "plusmn;", 7, 177 },
    { "pound;",  6, 163 },
    { "quot;",   5, '\"' },
    { "raquo;",  6, 187 },
    { "reg;",    4, 174 },
    { "sect;",   5, 167 },
    { "shy;",    4, 173 },
    { "sup1;",   5, 185 },
    { "sup2;",   5, 178 },
    { "sup3;",   5, 179 },
    { "szlig;",  6, 223 },
    { "THORN;",  6, 222 },
    { "thorn;",  6, 254 },
    { "times;",  6, 215 },
    { "trade;",  6, 0x2122 },
    { "Uacute;", 7, 218 },
    { "uacute;", 7, 250 },
    { "Ucirc;",  6, 219 },
    { "ucirc;",  6, 251 },
    { "Ugrave;", 7, 217 },
    { "ugrave;", 7, 249 },
    { "uml;",    4, 168 },
    { "Uuml;",   5, 220 },
    { "uuml;",   5, 252 },
    { "Yacute;", 7, 221 },
    { "yacute;", 7, 253 },
    { "yen;",    4, 165 },
    { "Yuml;",   5, 0x0178 },
    { "yuml;",   5, 255 }
  };

  if (!strchr(p, ';')) return -1;
  if (*p == '#') {
    if (*(p+1) == 'x' || *(p+1) == 'X') {
      return (int)strtol(p+2, nullptr, 16);
    } else {
      return atoi(p+1);
    }
  }
  for (i = (int)(sizeof(names) / sizeof(names[0])), nameptr = names; i > 0; i --, nameptr ++) {
    if (strncmp(p, nameptr->name, nameptr->namelen) == 0)
      return nameptr->code;
  }

  return -1;
}


// The following function is used to make anchors in a link (the part after
// the '#') case insensitive. As it is, the function handles only ASCII
// characters. Should it be extended to handle UTF-8 characters as well?
//
// UTF8 is allowed in anchors, but only when it is encoded between '%', so
// `https://example.com/page#%C3%BCber` is valid, but
// `https://example.com/page#über is *not*.
//
// But as with everything in HTML, nobody cares and everybody does what they
// want anyway ;-) .

static std::string to_lower(const std::string &str) {
  std::string lower_str;
  lower_str.reserve(str.size());
  for (char c : str) {
    lower_str += fl_tolower(c);
  }
  return lower_str;
}


/**
  \brief Check if a URL is starting with a scheme (e.g. ftp:).
  \param[in] url the URL to check.
  \return 0 if not found, otherwise the length of the scheme string including
      the following '/' characters.
 */
static size_t url_scheme(const std::string &url, bool skip_slashes)
{
  // First skip all ascii letters and digits
  size_t pos = 0;
  while ( (pos < url.size()) && ( isalnum(url[pos]) || (url[pos] == '+') || (url[pos] == '-') || (url[pos] == '.') )) {
    pos++;
  }
  // Next, check for the ':' character
  if ( (pos < url.size()) && (url[pos] == ':') ) {
    pos++; // Skip the ':' character
    if (skip_slashes) {
      // If found, skip up to two '/' characters as well
      if ( (pos < url.size()) && (url[pos] == '/') ) {
        pos++; // Skip the first '/' character
        if ( (pos < url.size()) && (url[pos] == '/') ) {
          pos++; // Skip the second '/' character
        }
      }
    }
    return pos; // Return the length of the scheme including the following '/' characters
  }
  return 0; // No scheme found
}

/** Return a pointer to the internal text buffer. */
const char *Fl_Help_View::value() const { return impl_->value(); }

/** Return the document height in pixels. */
int Fl_Help_View::size() const { return impl_->size(); }

/** Set the default text color. */
void Fl_Help_View::textcolor(Fl_Color c) { impl_->textcolor(c); }

/** Return the current default text color. */
Fl_Color Fl_Help_View::textcolor() const { return impl_->textcolor(); }

/** Set the default text font. */
void Fl_Help_View::textfont(Fl_Font f) { impl_->textfont(f); }

/** Return the default text font. */
Fl_Font Fl_Help_View::textfont() const { return impl_->textfont(); }

/** Set the default text size. */
void Fl_Help_View::textsize(Fl_Fontsize s) { impl_->textsize(s); }

/** Get the default text size. */
Fl_Fontsize Fl_Help_View::textsize() const { return impl_->textsize(); }

/** Get the current top line in pixels. */
int Fl_Help_View::topline() const { return impl_->topline(); }

/** Get the left position in pixels. */
int Fl_Help_View::leftline() const { return impl_->leftline(); }
