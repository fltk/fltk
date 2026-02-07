//
// UTF-8 test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2026 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Output.H>
#include <FL/fl_draw.H>
#include <FL/fl_utf8.h>
#include <FL/fl_string_functions.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//
// Font chooser widget for the Fast Light Tool Kit(FLTK).
//

static const int DEF_SIZE = 16; // default value for the font size picker

static Fl_Double_Window *fnt_chooser_win;
static Fl_Hold_Browser *fontobj;
static Fl_Hold_Browser *sizeobj;

static Fl_Value_Output *fnt_cnt;
static Fl_Button *refresh_btn;
static Fl_Button *choose_btn;
static Fl_Output *fix_prop;
static Fl_Check_Button *own_face;

static int  **sizes = NULL;
static int  *numsizes = NULL;
static int  pickedsize = DEF_SIZE;
static char label[1000];

static Fl_Double_Window *main_win;
static Fl_Scroll *thescroll;
static Fl_Font extra_font;

static int font_count = 0;
static int first_free = 0;

// window callback: hide all windows if any window is closed

static void cb_hide_all(Fl_Widget*, void*) {
  Fl::hide_all_windows();
}

/*
 Class for displaying sample fonts.
 */
class FontDisplay : public Fl_Widget {
  void draw(void) override;

public:
  int font, size;

  int test_fixed_pitch(void);

  FontDisplay(Fl_Boxtype B, int X, int Y, int W, int H, const char *L = 0)
    : Fl_Widget(X, Y, W, H, L) {
    box(B);
    font = 0;
    size = DEF_SIZE;
  }
};


/*
 Draw the sample text.
 */
void FontDisplay::draw(void) {
  draw_box();
  fl_font((Fl_Font)font, size);
  fl_color(FL_BLACK);
  fl_draw(label(), x() + 3, y() + 3, w() - 6, h() - 6, align());
}

// test_fixed_pitch() measures two strings and returns:
//  1: fixed font, measurements match exactly
//  2: nearly fixed font, measurements are within 5%
//  0: proportional font

int FontDisplay::test_fixed_pitch(void) {

  int w1 = 0, w2 = 0;
  int h1 = 0, h2 = 0;

  fl_font((Fl_Font)font, size);

  fl_measure("MHMHWWMHMHMHM###WWX__--HUW", w1, h1, 0);
  fl_measure("iiiiiiiiiiiiiiiiiiiiiiiiii", w2, h2, 0);

  if (w1 == w2) return 1; // exact match - fixed pitch

  // Is the font "nearly" fixed pitch? If it is within 5%, say it is...
  double f1 = (double)w1;
  double f2 = (double)w2;
  double delta = fabs(f1 - f2) * 20.0;
  if (delta <= f1) return 2; // nearly fixed pitch...
  return 0; // NOT fixed pitch
}


static FontDisplay *textobj;


static void size_cb(Fl_Widget *, long) {
  int size_idx = sizeobj->value();

  if (!size_idx) return;

  const char *c = sizeobj->text(size_idx);

  while (*c < '0' || *c > '9') c++; // find the first numeric char
  pickedsize = atoi(c);             // convert the number string to a value

  // Now set the font view to the selected size and redraw it.
  textobj->size = pickedsize;
  textobj->redraw();
}


static void font_cb(Fl_Widget *, long) {
  int font_idx = fontobj->value() + first_free;

  if (!font_idx) return;
  font_idx--;

  textobj->font = font_idx;
  sizeobj->clear();

  int  size_count = numsizes[font_idx - first_free];
  int *size_array = sizes[font_idx - first_free];
  if (!size_count) {
    // no preferred sizes - probably TT fonts etc...
  } else if (size_array[0] == 0) {
    // many sizes, probably a scaleable font with preferred sizes
    int j = 1;
    for (int i = 1; i <= 64 || i < size_array[size_count - 1]; i++) {
      char buf[16];
      if (j < size_count && i == size_array[j]) {
        snprintf(buf, 16, "@b%d", i);
        j++;
      } else {
        snprintf(buf, 16, "%d", i);
      }
      sizeobj->add(buf);
    }
    sizeobj->value(pickedsize);
  } else {
    // some sizes, probably a font with a few fixed sizes available
    int w = 0;
    for (int i = 0; i < size_count; i++) {
      // find the nearest available size to the current picked size
      if (size_array[i] <= pickedsize)
        w = i;

      char buf[16];
      snprintf(buf, 16, "@b%d", size_array[i]);
      sizeobj->add(buf);
    }
    sizeobj->value(w + 1);
  }
  size_cb(sizeobj, 0); // force selection of nearest valid size, then redraw

  // Now check to see if the font looks like a fixed pitch font or not...
  int looks_fixed = textobj->test_fixed_pitch();
  switch(looks_fixed) {
    case 1:
      fix_prop->value("fixed");
      break;
    case 2:
      fix_prop->value("near");
      break;
    default:
      fix_prop->value("prop");
      break;
  }
}


static void choose_cb(Fl_Widget *, long)
{
  int font_idx = fontobj->value() + first_free;
  if (!font_idx)
  {
    puts("No font chosen");
  }
  else
  {
    int font_type;
    font_idx -= 1;
    const char *name = Fl::get_font_name((Fl_Font)font_idx, &font_type);
    printf("idx %d\nUser name :%s:\n", font_idx, name);
    printf("FLTK name :%s:\n", Fl::get_font((Fl_Font)font_idx));

    Fl::set_font(extra_font, (Fl_Font)font_idx);
    //          Fl::set_font(extra_font, Fl::get_font((Fl_Font)font_idx));
  }

  int size_idx = sizeobj->value();
  if (!size_idx)
  {
    puts("No size selected");
  }
  else
  {
    const char *c = sizeobj->text(size_idx);
    while (*c < '0' || *c > '9') c++; // find the first numeric char
    int pickedsize = atoi(c);         // convert the number string to a value

    printf("size %d\n\n", pickedsize);
  }

  fflush(stdout);
  main_win->redraw();
}


static void refresh_cb(Fl_Widget *, long) {
  main_win->redraw();
}


static void own_face_cb(Fl_Widget *, void *) {
  int font_idx;
  int cursor_restore = 0;
  static int i_was = -1; // used to keep track of where we were in the list...

  if (i_was < 0) { // not been here before
    i_was = 1;
  } else {
    i_was = fontobj->topline(); // record which was the topmost visible line
    fontobj->clear();
    // Populating the font widget can be slower than an old dog with three legs
    // on a bad day, show a wait cursor
    fnt_chooser_win->cursor(FL_CURSOR_WAIT);
    cursor_restore = 1;
  }

  // Populate the font list with the names of the fonts found
  for (font_idx = first_free; font_idx < font_count; font_idx++) {
    int font_type;
    const char *name = Fl::get_font_name((Fl_Font)font_idx, &font_type);
    char buffer[128];

    if(own_face->value() == 0) {
      char *p = buffer;
      // if the font is BOLD, set the bold attribute in the list
      if (font_type & FL_BOLD) {
        *p++ = '@';
        *p++ = 'b';
      }
      if (font_type & FL_ITALIC) { //  ditto for italic fonts
        *p++ = '@';
        *p++ = 'i';
      }
      // Suppress subsequent formatting - some MS fonts have '@' in their name
      *p++ = '@';
      *p++ = '.';
      strcpy(p, name);
    } else {
      // Show font in its own face
      // this is neat, but really slow on some systems:
      // uses each font to display its own name
      snprintf (buffer, sizeof(buffer), "@F%d@.%s", font_idx, name);
    }
    fontobj->add(buffer);
  }
  // now put the browser position back the way it was... more or less
  fontobj->topline(i_was);
  // restore the cursor
  if (cursor_restore)
    fnt_chooser_win->cursor(FL_CURSOR_DEFAULT);
}


static void create_font_widget() {
  // Create the font sample label
  strcpy(label, "Font Sample\n");
  int i = 12; // strlen(label);
  int n = 0;
  ulong c;
  for (c = ' '+1; c < 127; c++) {
    if (!(c&0x1f)) label[i++]='\n';
    if (c == '@') label[i++] = '@';
    label[i++] = (char)c;
  }
  label[i++] = '\n';
  for (c = 0xA1; c < 0x600; c += 9) {
    if (!(++n&(0x1f))) label[i++]='\n';
    i += fl_utf8encode((unsigned int)c, label + i);
  }
  label[i] = 0;

  // Create the window layout
  fnt_chooser_win = new Fl_Double_Window(380, 420, "Font Selector");
  {
    Fl_Tile *tile = new Fl_Tile(0, 0, 380, 420);
    {
      Fl_Group *textgroup = new Fl_Group(0, 0, 380, 105);
      {
        textobj = new FontDisplay(FL_FRAME_BOX, 10, 10, 360, 90, label);
        textobj->align(FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
        textobj->color(53, 3);

        textgroup->box(FL_FLAT_BOX);
        textgroup->resizable(textobj);
        textgroup->end();
      }
      Fl_Group *fontgroup = new Fl_Group(0, 105, 380, 315);
      {
        fontobj = new Fl_Hold_Browser(10, 110, 290, 270);
        fontobj->box(FL_FRAME_BOX);
        fontobj->color(53, 3);
        fontobj->callback(font_cb);
        fnt_chooser_win->resizable(fontobj);

        sizeobj = new Fl_Hold_Browser(310, 110, 60, 270);
        sizeobj->box(FL_FRAME_BOX);
        sizeobj->color(53, 3);
        sizeobj->callback(size_cb);

        // Create the status bar
        Fl_Group *stat_bar = new Fl_Group (10, 385, 380, 30);
        {
          fnt_cnt = new Fl_Value_Output(10, 390, 40, 20);
          fnt_cnt->label("fonts");
          fnt_cnt->align(FL_ALIGN_RIGHT);

          fix_prop = new Fl_Output(100, 390, 40, 20);
          fix_prop->color(FL_BACKGROUND_COLOR);
          fix_prop->value("prop");
          fix_prop->clear_visible_focus();

          own_face = new Fl_Check_Button(150, 390, 40, 20, "Self");
          own_face->value(0);
          own_face->type(FL_TOGGLE_BUTTON);
          own_face->clear_visible_focus();
          own_face->callback(own_face_cb);
          own_face->tooltip("Display font names in their own face");

          Fl_Box * dummy = new Fl_Box(220, 390, 1, 1);

          choose_btn = new Fl_Button(240, 385, 60, 30);
          choose_btn->label("Select");
          choose_btn->callback(choose_cb);

          refresh_btn = new Fl_Button(310, 385, 60, 30);
          refresh_btn->label("Refresh");
          refresh_btn->callback(refresh_cb);

          stat_bar->resizable (dummy);
          stat_bar->end();
        }

        fontgroup->box(FL_FLAT_BOX);
        fontgroup->resizable(fontobj);
        fontgroup->end();
      }
      tile->end();
    }
    fnt_chooser_win->resizable(tile);
    fnt_chooser_win->end();
    fnt_chooser_win->callback(cb_hide_all);
  }
}

int make_font_chooser(void) {
  int font_idx;

  // create the widget frame
  create_font_widget();

  // Load the system's available fonts
#if defined(FLTK_USE_X11) && !defined(FLTK_USE_CAIRO)
  // ask for everything that claims to be iso10646 compatible
  font_count = Fl::set_fonts("-*-*-*-*-*-*-*-*-*-*-*-*-iso10646-1");
#else
  // ask for everything
  font_count = Fl::set_fonts("*");
#endif

  // allocate space for the sizes and numsizes array, now we know how many
  // entries it needs
  sizes = new int*[font_count];
  numsizes = new int[font_count];

  // Populate the font list with the names of the fonts found
  first_free = FL_FREE_FONT;
  for (font_idx = first_free; font_idx < font_count; font_idx++) {
    // Find out how many sizes are supported for each font face
    int *size_array;
    int size_count = Fl::get_font_sizes((Fl_Font)font_idx, size_array);
    numsizes[font_idx-first_free] = size_count;
    // if the font has multiple sizes, populate the 2-D sizes array
    if (size_count) {
      sizes[font_idx - first_free] = new int[size_count];
      for (int j = 0; j < size_count; j++) {
        sizes[font_idx - first_free][j] = size_array[j];
      }
    }
  } // end of font list filling loop

  // Call this once to get the font browser loaded up
  own_face_cb(NULL, 0);

  fontobj->value(1);
  // optional hard-coded font for testing - do not use!
  //    fontobj->textfont(261);

  font_cb(fontobj, 0);

  fnt_cnt->value(font_count);

  return font_count;

} // make_font_chooser

/* End of Font Chooser Widget code */

/* Unicode Font display widget */

void box_cb(Fl_Widget* o, void*) {
  thescroll->box(((Fl_Button*)o)->value() ? FL_DOWN_FRAME : FL_NO_BOX);
  thescroll->redraw();
}

class right_left_input : public Fl_Input {
public:
  right_left_input(int x, int y, int w, int h)
    : Fl_Input(x, y, w, h) {}
  void draw() override {
    if (type() == FL_HIDDEN_INPUT)
      return;
    Fl_Boxtype b = box();
    if (damage() & FL_DAMAGE_ALL)
      draw_box(b, color());
    drawtext(x() + Fl::box_dx(b) + 3, y() + Fl::box_dy(b),
             w() - Fl::box_dw(b) - 6, h() - Fl::box_dh(b));
  }
  void drawtext(int X, int Y, int W, int H) {
    fl_color(textcolor());
    fl_font(textfont(), textsize());
    fl_rtl_draw(value(), (int)strlen(value()),
                X + W, Y + fl_height() - fl_descent());
  }
};


void i7_cb(Fl_Widget *w, void *d)
{
  int i = 0;
  char nb[] = "01234567";
  Fl_Input *i7 = (Fl_Input*)w;
  Fl_Input *i8 = (Fl_Input*)d;
  static char buf[1024];
  const char *ptr = i7->value();
  while (ptr && *ptr) {
    if (*ptr < ' ' || *ptr > 126) {
      buf[i++] = '\\';
      buf[i++] = nb[((*ptr >> 6) & 0x3)];
      buf[i++] = nb[((*ptr >> 3) & 0x7)];
      buf[i++] = nb[(*ptr & 0x7)];
    } else {
      if (*ptr == '\\') buf[i++] = '\\';
      buf[i++] = *ptr;
    }
    ptr++;
  }
  buf[i] = 0;
  i8->value(buf);
}

class UCharDropBox : public Fl_Output {
public:
  UCharDropBox(int x, int y, int w, int h, const char *label=0) :
  Fl_Output(x, y, w, h, label) {
    tooltip("Drop one Unicode character here to decode it.\n"
            "Only the first Unicode code point will be displayed.\n"
            "Example: U+1F308 '🌈' 0x{f0,9f,8c,88}");
  }
  int handle(int event) override {
    switch (event) {
      case FL_DND_ENTER: return 1;
      case FL_DND_DRAG: return 1;
      case FL_DND_RELEASE: return 1;
      case FL_PASTE: {
        int i, n;
        const char *t = Fl::event_text();
        char temp[10];
        unsigned int ucode = fl_utf8decode(t, t + Fl::event_length(), &n);
        if (n == 0) {
          value("");
          return 1;
        }
        // Example output length and format:
        // - length = 15: "U+0040 '@' 0x40"              -- UTF-8 encoding = 1 byte (ASCII)
        // - length = 30: "U+1F308 '🌈' 0x{f0,9f,8c,88}" -- UTF-8 encoding = 4 bytes
        // - length = 31: "U+10FFFF '􏿿' 0x{f4,8f,bf,bf}" -- UTF-8 encoding = 4 bytes
        //
        static const size_t max_size = 32;
        // begin with first Unicode code point of Fl::event_text() in single quotes
        int tl = fl_utf8len(t[0]);
        if (tl < 1)
          tl = 1;
        std::string buffer;
        buffer.reserve(max_size);
        // add Unicode code point: "U+0000" - "U+10FFFF" (4-6 hex digits)
        buffer += "U+";
        snprintf(temp, 9, "%04X", ucode);
        buffer += temp;
        // add Unicode character in quotes
        buffer += " '";
        buffer += std::string(t, tl);
        buffer += "'";
        // add hex UTF-8 codes, format: "0xab" or "0x{de,ad,be,ef}"
        buffer += " 0x";
        if (n > 1)
          buffer += "{";
        for (i = 0; i < n; i++) {
          if (i > 0)
            buffer += ',';
          snprintf(temp, 9, "%02x", (unsigned char)t[i]);
          buffer += temp;
        }
        if (n > 1)
          buffer += "}";
        value(buffer.c_str());
        // printf("size: %d\n", (int)buffer.size()); fflush(stdout);
      }
        return 1;
    }
    return Fl_Output::handle(event);
  }
};

// Create the layout with widgets. Widget contents will be assigned later.

// constants for window layout

const int IW = 280;           // width of input widgets (left col.)
const int SW = 470;           // width of 'scroll' incl. scrollbars
const int WW = IW + SW + 15;  // total window width
const int WH = 400;           // minimal window height

Fl_Input  *iw[20];            // global widget pointers


// create the Fl_Scroll widget: right column of the main window:
// input: `off` = offset for unicode character display
// returns: the Fl_Scroll widget

Fl_Scroll *make_scroll(int off) {

  auto scroll = new Fl_Scroll(IW + 10, 0, SW, WH);

  int end_list = 0x10000 / 16;
  if (off > 2) {
    if (off > 0x10F000)       // would extend higher than Unicode range
      off = 0x10F000;
    end_list = off + 0x10000;
    if (end_list > 0x10FFFF)  // would be greater than Unicode range
      end_list = 0x10FFFF;
    off /= 16;
    end_list /= 16;
  }

  for (int y = off; y < end_list; y++) {
    // skip Unicode space reserved for surrogate pairs (U+D800 ... U+DFFF)
    if (y == 0xD80) { // U+D800
      Fl_Box *bx = new Fl_Box(IW + 10, (y - off) * 25, 450, 25);
      bx->label("U+D800 … U+DFFF: reserved for UTF-16 surrogate pairs");
      bx->color(fl_lighter(FL_YELLOW));
      bx->box(FL_DOWN_BOX);
      bx->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
      y = 0xE00;
      off += 127;
      if (y >= end_list) break;
    }
    int o = 0;
    char bu[25]; // index label
    char buf[16 * 6]; // utf8 text
    int i = 16 * y;
    for (int x = 0; x < 16; x++) {
      int len;
      len = fl_utf8encode(i, buf + o);
      if (len < 1) len = 1;
      o += len;
      i++;
    }
    buf[o] = '\0';
    snprintf(bu, sizeof(bu), "0x%06X", y * 16);

    Fl_Input *b = new Fl_Input(IW + 10, (y - off) * 25, 80, 25);
    b->textfont(FL_COURIER);
    b->value(bu);

    b = new Fl_Input(IW + 90, (y - off) * 25, 370, 25);
    b->textfont(extra_font);
    b->value(buf);
  }
  scroll->end();

  return scroll;
}

// create the "grid" layout of the main window:
// - left column: several widgets; subclasses of Fl_Input
// - right column: one Fl_Scroll; see above: make_scroll()
// input: `off` = offset for unicode character display; used in make_scroll()
// returns: the Fl_Grid widget

Fl_Grid *make_grid(int off) {

  auto grid = new Fl_Grid(0, 0, WW, WH);  // full window size
  grid->layout(10, 2, 5, 5);              // rows, cols, margin, gap
  int col_weights[] = { 100, 0 };         // resize only first column
  grid->col_weight(col_weights, 2);

  // left column:

  iw[0] = new Fl_Input(0, 0, IW, 25);
  grid->widget(iw[0], 0, 0);

  iw[1] = new Fl_Input(0, 0, IW, 25);
  grid->widget(iw[1], 1, 0);

  iw[2] = new Fl_Input(0, 0, IW, 25);
  grid->widget(iw[2], 2, 0);

  iw[3] = new Fl_Input(0, 0, IW, 25);
  grid->widget(iw[3], 3, 0);
  iw[3]->textfont(extra_font);

  iw[4] = new right_left_input(0, 0, IW, 40);
  grid->widget(iw[4], 4, 0);
  iw[4]->textfont(extra_font);
  iw[4]->textsize(24);

  iw[5] = new right_left_input(0, 0, IW, 40);
  grid->widget(iw[5], 5, 0);
  iw[5]->textfont(extra_font);
  iw[5]->textsize(24);

  iw[6] = new Fl_Input(0, 0, IW, 25);
  grid->widget(iw[6], 6, 0);

  iw[7] = new Fl_Output(0, 0, IW, 25);
  grid->widget(iw[7], 7, 0);

  iw[6]->textsize(20);
  iw[6]->when(FL_WHEN_CHANGED);
  iw[6]->tooltip("Edit this field to decode non-ASCII characters\n(in octal bytes) and view the result below");
  iw[6]->callback(i7_cb, iw[7]);

  iw[7]->tooltip("Edit the field above to decode it\nand display the result in this field");

  iw[8] = new Fl_Output(0, 0, IW, 40);
  grid->widget(iw[8], 8, 0);
  iw[8]->textfont(extra_font);
  iw[8]->textsize(30);

  iw[9] = new UCharDropBox(0, 0, IW, 40);
  grid->widget(iw[9], 9, 0);
  iw[9]->textsize(20);
  iw[9]->value("drop box (see tooltip)");
  iw[9]->color(fl_lighter(FL_GREEN));

  // right column:

  thescroll = make_scroll(off);
  grid->widget(thescroll, 0, 1, 10, 1);

  return grid;
}

int main(int argc, char** argv) {

  int off = 2;
  if (argc > 1) {
    off = (int)strtoul(argv[1], NULL, 0);
    argc = 1;
  }

  make_font_chooser();
  extra_font = FL_TIMES_BOLD_ITALIC;

  /* setup the extra font */
  Fl::set_font(extra_font,
#ifdef _WIN32
               " Microsoft Sans Serif"
#elif defined(__APPLE__)
               "Monaco"
#else
               "-*-*-*-*-*-*-*-*-*-*-*-*-iso10646-1"
#endif
               );

  main_win = new Fl_Double_Window (WW, WH, "Unicode Display");
  main_win->begin();

  auto grid = make_grid(off);      // make a grid layout of widgets

  // populate the grid's contents

  const char *utf8 =
    "@ABCabcàèéïßîöüã123 "        // latin1 (ISO-8859-1)
    "\360\237\230\204 "           // emoji: grinning face with smiling eyes
    "\360\237\221\215 "           // emoji: thumbs up
    "\360\237\214\210 "           // emoji: Rainbow
    ".";                          // final '.'
  int utf8_l = (int)strlen(utf8); // total length of UTF-8 example string

  // convert UTF-8 string to lowercase
  char *utf8_lc = (char *)malloc(utf8_l * 3 + 1);
  int llc = fl_utf_tolower((const unsigned char *)utf8, utf8_l, utf8_lc);
  utf8_lc[llc] = '\0';

  // convert UTF-8 string to uppercase
  char *utf8_uc = (char *)malloc(utf8_l * 3 + 1);
  int luc = fl_utf_toupper((const unsigned char *)utf8, utf8_l, utf8_uc);
  utf8_uc[luc] = '\0';

  iw[0]->value(utf8);
  iw[1]->value(utf8_lc);
  iw[2]->value(utf8_uc);

  // free strings that are no longer used
  free(utf8_lc);
  free(utf8_uc);

  // accented text in two forms:
  //  - e\xCC\x82 = "e" + U+0303 = "e" + "Combining Circumflex Accent"
  // -   \xC3\xAA = U+00ea = "ê" = "Latin Small Letter E with Circumflex"

  const char *ltr_txt = "\\->e\xCC\x82=\xC3\xAA";
  iw[3]->value(ltr_txt);

  // right-to-left text

  char abuf[60];

  wchar_t r_to_l_txt[] = { /*8238,*/   // U+202E = "RIGHT-TO-LEFT OVERRIDE"
    1610, 1608, 1606, 1604, 1603, 1608, 1583, 0};
  int len = fl_utf8fromwc(abuf, 60, r_to_l_txt, 8);
  abuf[len] = 0;

  iw[4]->value(abuf);

  wchar_t r_to_l_txt1[] = { /*8238,*/   // U+202E = "RIGHT-TO-LEFT OVERRIDE"
    1610, 0x20, 1608, 0x20, 1606, 0x20,
    1604, 0x20, 1603, 0x20, 1608, 0x20, 1583, 0};
  len = fl_utf8fromwc(abuf, 60, r_to_l_txt1, 14);
  abuf[len] = 0;

  iw[5]->value(abuf);

  iw[6]->value(abuf);

  // Greg Ercolano's Japanese test sequence
  // Note: in English: "Do nothing."

  iw[8]->value("\xe4\xbd\x95\xe3\x82\x82\xe8\xa1\x8c\xe3\x82\x8b\xe3\x80\x82");

  main_win->end();
  main_win->callback(cb_hide_all);
  main_win->resizable(grid);
  main_win->size_range(WW, WH);

  // fl_set_status(0, 370, 100, 30);

  main_win->show(argc, argv);

  fnt_chooser_win->show();

  int ret = Fl::run();

  // Free up the sizes arrays we allocated
  if(numsizes) { delete [] numsizes; }
  if(sizes) { delete [] sizes; }

  return ret;
}
