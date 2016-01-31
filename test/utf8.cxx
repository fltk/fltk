//
// "$Id$"
//
// UTF-8 test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//
// Font chooser widget for the Fast Light Tool Kit(FLTK).
//


#define DEF_SIZE 16 // default value for the font size picker


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

static void cb_exit(Fl_Button*, void*) {
  if(fnt_chooser_win) fnt_chooser_win->hide();
  if(main_win) main_win->hide();
} /* cb_exit */

/*
 Class for displaying sample fonts.
 */
class FontDisplay : public Fl_Widget
{
  void draw(void);
  
public:
  int  font, size;
  
  int test_fixed_pitch(void);
  
  FontDisplay(Fl_Boxtype B, int X, int Y, int W, int H, const char *L = 0) 
  : Fl_Widget(X, Y, W, H, L)
  {
    box(B);
    font = 0;
    size = DEF_SIZE;
  }
};


/*
 Draw the sample text.
 */
void FontDisplay::draw(void)
{
  draw_box();
  fl_font((Fl_Font)font, size);
  fl_color(FL_BLACK);
  fl_draw(label(), x() + 3, y() + 3, w() - 6, h() - 6, align());
}


int FontDisplay::test_fixed_pitch(void)
{
  int w1, w2;
  int h1, h2;
  
  w1 = w2 = 0;
  h1 = h2 = 0;
  
  fl_font((Fl_Font)font, size);
  
  fl_measure("MHMHWWMHMHMHM###WWX__--HUW", w1, h1, 0);
  fl_measure("iiiiiiiiiiiiiiiiiiiiiiiiii", w2, h2, 0);
  
  if (w1 == w2) return 1; // exact match - fixed pitch
  
  // Is the font "nearly" fixed pitch? If it is within 5%, say it is...
  double f1 = (double)w1;
  double f2 = (double)w2;
  double delta = fabs(f1 - f2) * 5.0;
  if (delta <= f1) return 2; // nearly fixed pitch...
  
  return 0; // NOT fixed pitch
}


static FontDisplay *textobj;


static void size_cb(Fl_Widget *, long)
{
  int size_idx = sizeobj->value();
  
  if (!size_idx) return;
  
  const char *c = sizeobj->text(size_idx);
  
  while (*c < '0' || *c > '9') c++; // find the first numeric char
  pickedsize = atoi(c);             // convert the number string to a value
  
  // Now set the font view to the selected size and redraw it.
  textobj->size = pickedsize;
  textobj->redraw();
}


static void font_cb(Fl_Widget *, long)
{
  int font_idx = fontobj->value() + first_free;
  
  if (!font_idx) return;
  font_idx--;
  
  textobj->font = font_idx;
  sizeobj->clear();
  
  int  size_count = numsizes[font_idx-first_free];
  int *size_array = sizes[font_idx-first_free];
  if (!size_count)
  {
    // no preferred sizes - probably TT fonts etc...
  }
  else if (size_array[0] == 0)
  {
    // many sizes, probably a scaleable font with preferred sizes
    int j = 1;
    for (int i = 1; i <= 64 || i < size_array[size_count - 1]; i++)
    {
      char buf[16];
      if (j < size_count && i == size_array[j])
      {
        sprintf(buf, "@b%d", i);
        j++;
      }
      else
        sprintf(buf, "%d", i);
      sizeobj->add(buf);
    }
    sizeobj->value(pickedsize);
  }
  else
  {
    // some sizes, probably a font with a few fixed sizes available
    int w = 0;
    for (int i = 0; i < size_count; i++)
    {
      // find the nearest available size to the current picked size
      if (size_array[i] <= pickedsize) w = i;
      
      char buf[16];
      sprintf(buf, "@b%d", size_array[i]);
      sizeobj->add(buf);
    }
    sizeobj->value(w + 1);
  }
  size_cb(sizeobj, 0); // force selection of nearest valid size, then redraw
  
  // Now check to see if the font looks like a fixed pitch font or not...
  int looks_fixed = textobj->test_fixed_pitch();
  if(looks_fixed)
  {
    if (looks_fixed > 1)
      fix_prop->value("near");
    else
      fix_prop->value("fixed");
  }
  else
  {
    fix_prop->value("prop");
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
    //		Fl::set_font(extra_font, Fl::get_font((Fl_Font)font_idx));
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


static void refresh_cb(Fl_Widget *, long)
{
  main_win->redraw();
}


static void own_face_cb(Fl_Widget *, void *)
{
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
  for (font_idx = first_free; font_idx < font_count; font_idx++)
  {
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
      sprintf (buffer, "@F%d@.%s", font_idx, name);
    }
    fontobj->add(buffer);
  }
  // now put the browser position back the way it was... more or less
  fontobj->topline(i_was);
  // restore the cursor
  if(cursor_restore) fnt_chooser_win->cursor(FL_CURSOR_DEFAULT);
}


static void create_font_widget()
{
  // Create the font sample label
  strcpy(label, "Font Sample\n");
  int i = 12; // strlen(label);
  int n = 0;
  ulong c;
  for (c = ' '+1; c < 127; c++) {
    if (!(c&0x1f)) label[i++]='\n';
    if (c=='@') label[i++]=c;
    label[i++]=c;
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
	fnt_chooser_win->callback((Fl_Callback*)cb_exit);
  }
}


int make_font_chooser(void)
{
  int font_idx;
  
  // create the widget frame
  create_font_widget();
  
  // Load the systems available fonts - ask for everything
  //	font_count = Fl::set_fonts("*");
#ifdef WIN32
  font_count = Fl::set_fonts("*");
#elif defined(__APPLE__)
  font_count = Fl::set_fonts("*");
#else
  // Load the systems available fonts - ask for everything that claims to be 
  // iso10646 compatible
  font_count = Fl::set_fonts("-*-*-*-*-*-*-*-*-*-*-*-*-iso10646-1");
#endif
  
  // allocate space for the sizes and numsizes array, now we know how many 
  // entries it needs
  sizes = new int*[font_count];
  numsizes = new int[font_count];
  
  // Populate the font list with the names of the fonts found
  first_free = FL_FREE_FONT;
  for (font_idx = first_free; font_idx < font_count; font_idx++)
  {
    // Find out how many sizes are supported for each font face
    int *size_array;
    int size_count = Fl::get_font_sizes((Fl_Font)font_idx, size_array);
    numsizes[font_idx-first_free] = size_count;
    // if the font has multiple sizes, populate the 2-D sizes array
    if (size_count)
    {
      sizes[font_idx-first_free] = new int[size_count];
      for (int j = 0; j < size_count; j++)
        sizes[font_idx-first_free][j] = size_array[j];
    }
  } // end of font list filling loop
  
  // Call this once to get the font browser loaded up
  own_face_cb(NULL, 0);
  
  fontobj->value(1);
  // optional hard-coded font for testing - do not use!
  //	fontobj->textfont(261);
  
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


class right_left_input : public Fl_Input
{
public:
  right_left_input (int x, int y, int w, int h) : Fl_Input(x, y, w, h) {};
  void draw() {
    if (type() == FL_HIDDEN_INPUT) return;
    Fl_Boxtype b = box();
    if (damage() & FL_DAMAGE_ALL) draw_box(b, color());
    drawtext(x()+Fl::box_dx(b)+3, y()+Fl::box_dy(b),
             w()-Fl::box_dw(b)-6, h()-Fl::box_dh(b));
  }
  void drawtext(int X, int Y, int W, int H) {
    fl_color(textcolor());
    fl_font(textfont(), textsize());
    fl_rtl_draw(value(), strlen(value()),
                X + W, Y + fl_height() -fl_descent());
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
  Fl_Output(x, y, w, h, label) { }
  int handle(int event) {
    switch (event) {
      case FL_DND_ENTER: return 1;
      case FL_DND_DRAG: return 1;
      case FL_DND_RELEASE: return 1;
      case FL_PASTE:
      {
        static const char lut[] = "0123456789abcdef";
        const char *t = Fl::event_text();
        int i, n;
        unsigned int ucode = fl_utf8decode(t, t+Fl::event_length(), &n);
        if (n==0) {
          value("");
          return 1;
        }
        char buffer[200], *d = buffer;
        for (i=0; i<n; i++) *d++ = t[i];
        *d++ = ' ';
        for (i=0; i<n; i++) {
          *d++ = '\\'; *d++ = 'x';
          *d++ = lut[(t[i]>>4)&0x0f]; *d++ = lut[t[i]&0x0f];
        }
        *d++ = ' ';
        *d++ = '0';
        *d++ = 'x';
        *d++ = lut[(ucode>>20)&0x0f]; *d++ = lut[(ucode>>16)&0x0f];
        *d++ = lut[(ucode>>12)&0x0f]; *d++ = lut[(ucode>>8)&0x0f];
        *d++ = lut[(ucode>>4)&0x0f]; *d++ = lut[ucode&0x0f];
        *d++ = 0;
        value(buffer);
      }
        return 1;
    }
    return Fl_Output::handle(event);
  }
};


int main(int argc, char** argv)
{
  int l;
  const char *latin1 = 
  "\x41\x42\x43\x61\x62\x63\xe0\xe8\xe9\xef\xe2\xee\xf6\xfc\xe3\x31\x32\x33";
  char *utf8 = (char*) malloc(strlen(latin1) * 5 + 1);
  l = 0;
  //	l = fl_latin12utf((const unsigned char*)latin1, strlen(latin1), utf8);
  l = fl_utf8froma(utf8, (strlen(latin1) * 5 + 1), latin1, strlen(latin1));
  
  make_font_chooser();
  extra_font = FL_TIMES_BOLD_ITALIC;
  
  /* setup the extra font */
  Fl::set_font(extra_font,
#ifdef WIN32
               " Arial Unicode MS"
#elif defined(__APPLE__)
               "Monaco"
#else
               "-*-*-*-*-*-*-*-*-*-*-*-*-iso10646-1"
#endif
               );
  
  main_win = new Fl_Double_Window (200 + 5*75, 400, "Unicode Display Test");
  main_win->begin();
  
  Fl_Input i1(5, 5, 190, 25);
  utf8[l] = '\0';
  i1.value(utf8);
  Fl_Scroll scroll(200,0,5 * 75,400);
  
  int off = 2;
  int end_list = 0x10000 / 16;
  if (argc > 1) {
    off = (int)strtoul(argv[1], NULL, 0);
    end_list = off + 0x10000;
    off /= 16;
    end_list /= 16;
  }
  argc = 1;
  for (long y = off; y < end_list; y++) {
    int o = 0;
    char bu[25]; // index label
    char buf[16 * 6]; // utf8 text
    int i = 16 * y;
    for (int x = 0; x < 16; x++) {
      int l;
      l = fl_utf8encode(i, buf + o);
      if (l < 1) l = 1;
      o += l;
      i++;
    }
    buf[o] = '\0';
    sprintf(bu, "0x%06lX", y * 16);
    Fl_Input *b = new Fl_Input(200,(y-off)*25,80,25);
    b->textfont(FL_COURIER);
    b->value(strdup(bu));
    b = new Fl_Input(280,(y-off)*25,380,25);
    b->textfont(extra_font);
    b->value(strdup(buf));
  }
  scroll.end();
  main_win->resizable(scroll);
  
  thescroll = &scroll;
  
  char *utf8l = (char*) malloc(strlen(utf8) * 3 + 1);
  Fl_Input i2(5, 35, 190, 25);
  l = fl_utf_tolower((const unsigned char*)utf8, l, utf8l);
  utf8l[l] = '\0';
  i2.value(utf8l);
  
  char *utf8u = (char*) malloc(strlen(utf8l) * 3 + 1);
  Fl_Input i3(5, 65, 190, 25);
  l = fl_utf_toupper((const unsigned char*)utf8l, l, utf8u);
  utf8u[l] = '\0';
  i3.value(utf8u);
  
  const char *ltr_txt = "\\->e\xCC\x82=\xC3\xAA";
  Fl_Input i4(5, 90, 190, 25);
  i4.value(ltr_txt);
  i4.textfont(extra_font);
  
  wchar_t r_to_l_txt[] = {/*8238,*/ 
    1610, 1608, 1606, 1604, 1603, 1608, 1583, 0};
  
  char abuf[40];
  //  l = fl_unicode2utf(r_to_l_txt, 8, abuf);
  l = fl_utf8fromwc(abuf, 40, r_to_l_txt, 8);
  abuf[l] = 0;
  
  right_left_input i5(5, 115, 190, 50);
  i5.textfont(extra_font);
  i5.textsize(30);
  i5.value(abuf);
  
  Fl_Input i7(5, 230, 190, 25);
  Fl_Input i8(5, 260, 190, 25);
  i7.callback(i7_cb, &i8);
  i7.textsize(20);
  i7.value(abuf);
  i7.when(FL_WHEN_CHANGED);
  
  wchar_t r_to_l_txt1[] = { /*8238,*/ 
    1610, 0x20, 1608, 0x20, 1606, 0x20,  
    1604, 0x20, 1603, 0x20, 1608, 0x20, 1583, 0};
  
  //  l = fl_unicode2utf(r_to_l_txt1, 14, abuf);
  l = fl_utf8fromwc(abuf, 40, r_to_l_txt1, 14);
  abuf[l] = 0;
  right_left_input i6(5, 175, 190, 50);
  i6.textfont(extra_font);
  i6.textsize(30);
  i6.value(abuf);
  
  // Now try Greg Ercolano's Japanese test sequence
  // SOME JAPANESE UTF-8 TEXT
  const char *utfstr = 
  "\xe4\xbd\x95\xe3\x82\x82\xe8\xa1"
  "\x8c\xe3\x82\x8b\xe3\x80\x82"; 
  
  UCharDropBox db(5, 300, 190, 30);
  db.textsize(16);
  db.value("unichar drop box");
  
  Fl_Output o9(5, 330, 190, 45);
  o9.textfont(extra_font);
  o9.textsize(30);
  o9.value(utfstr);
  
  main_win->end();
  main_win->callback((Fl_Callback*)cb_exit);

  fl_set_status(0, 370, 100, 30);
  
  main_win->show(argc,argv);

  fnt_chooser_win->show();

  int ret = Fl::run();

  // Free up the sizes arrays we allocated
  if(numsizes) {delete [] numsizes;}
  if(sizes) {delete [] sizes;}

  return ret;
}

//
// End of "$Id$".
//
