// style.C

// Demo of a control panel for Fltk "style" changes.

// You can use this as-is, or modify it to your needs.

// To save & restore a style you should write the data to a file of
// your own design.  Most likely your program has it's own configuration
// that you want to save as well, and it is user friendly to put all
// this into the same file.

////////////////////////////////////////////////////////////////
// This is the part you want to copy to another program.  The
// program should call "show_style_panel()" in response to a button
// that the user presses.

#include "style_ui.C"

#include <string.h>

Fl_Menu_Item* font_menu() {
  static Fl_Menu_Item* menu;
  if (menu) return menu;
  int k = Fl::set_fonts(/*"*"*/);
  menu = new Fl_Menu_Item[k+1];
  memset(menu, 0, (k+1)*sizeof(Fl_Menu_Item));
  for (int i = 0; i < k; i++) {
    int t; const char *name = Fl::get_font_name((Fl_Font)i,&t);
    char buf[256];
    strcpy(buf, name);
    if (t & FL_BOLD) strcat(buf," bold");
    if (t & FL_ITALIC) strcat(buf," italic");
    menu[i].text = strdup(buf);
  }
  return menu;
}

void font_cb(Fl_Choice* c, long i) {
  Fl_Font n = Fl_Font(c->value());
  switch (i) {
  case 2: Fl_Menu_::default_font(n); break;
  case 1: Fl_Input_::default_font(n); break;
  default: Fl_Widget::default_font(n); break;
  }
  Fl::redraw();
}

void font_size_cb(Fl_Value_Input* c, long i) {
  int n = int(c->value()); if (n <= 0) n = 1; n -= FL_NORMAL_SIZE;
  switch (i) {
  case 2: Fl_Menu_::default_size(n); break;
  case 1: Fl_Input_::default_size(n); break;
  default: Fl_Widget::default_size(n); break;
  }
  Fl::redraw();
}

#include <FL/Fl_Color_Chooser.H>

void color_button_cb(Fl_Button* w, void*) {
  Fl_Color c = w->color();
  uchar r,g,b; Fl::get_color(c, r,g,b);
  if (fl_color_chooser(0,r,g,b)) {
    if (c == FL_GRAY) Fl::background(r,g,b);
    else Fl::set_color(c,r,g,b);
    Fl::redraw();
  }
}

// functions hidden inside fl_boxtype.C:
void fl_thin_down_frame(int, int, int, int, Fl_Color);
void fl_thin_up_frame(int, int, int, int, Fl_Color);
void fl_thin_down_box(int, int, int, int, Fl_Color);
void fl_thin_up_box(int, int, int, int, Fl_Color);
void fl_down_frame(int, int, int, int, Fl_Color);
void fl_up_frame(int, int, int, int, Fl_Color);
void fl_down_box(int, int, int, int, Fl_Color);
void fl_up_box(int, int, int, int, Fl_Color);

// define the 2-pixel boxes:
#include <FL/fl_draw.H>
void fl_2_up_frame(int x, int y, int w, int h, Fl_Color) {
  fl_frame2("AAPPMMWU",x,y,w,h);
}
void fl_2_up_box(int x, int y, int w, int h, Fl_Color c) {
  fl_2_up_frame(x,y,w,h,c);
  fl_color(c); fl_rectf(x+2, y+2, w-4, h-4);
}
void fl_2_down_frame(int x, int y, int w, int h, Fl_Color) {
  fl_frame2("UWMMPPAA",x,y,w,h);
}
void fl_2_down_box(int x, int y, int w, int h, Fl_Color c) {
  fl_2_down_frame(x,y,w,h,c);
  fl_color(c); fl_rectf(x+2, y+2, w-4, h-4);
}

void box_thickness_cb(Fl_Value_Slider*v, void*) {
  switch (int(v->value())) {
  case 1:
    Fl::set_boxtype(FL_UP_BOX, fl_thin_up_box, 1,1,2,2);
    Fl::set_boxtype(FL_DOWN_BOX, fl_thin_down_box, 1,1,2,2);
    Fl::set_boxtype(FL_UP_FRAME, fl_thin_up_frame, 1,1,2,2);
    Fl::set_boxtype(FL_DOWN_FRAME, fl_thin_down_frame, 1,1,2,2);
    break;
  case 2:
    Fl::set_boxtype(FL_UP_BOX, fl_2_up_box, 2,2,4,4);
    Fl::set_boxtype(FL_DOWN_BOX, fl_2_down_box, 2,2,4,4);
    Fl::set_boxtype(FL_UP_FRAME, fl_2_up_frame, 2,2,4,4);
    Fl::set_boxtype(FL_DOWN_FRAME, fl_2_down_frame, 2,2,4,4);
    break;
  default:
    Fl::set_boxtype(FL_UP_BOX, fl_up_box, 3,3,6,6);
    Fl::set_boxtype(FL_DOWN_BOX, fl_down_box, 3,3,6,6);
    Fl::set_boxtype(FL_UP_FRAME, fl_up_frame, 3,3,6,6);
    Fl::set_boxtype(FL_DOWN_FRAME, fl_down_frame, 3,3,6,6);
    break;
  }
  Fl::redraw();
}

void text_box_thickness_cb(Fl_Value_Slider* v, void*) {
  int n = int(v->value());
  switch (n) {
  case 0: Fl_Input_::default_box(FL_FLAT_BOX); break;
  case 1: Fl_Input_::default_box(FL_THIN_DOWN_BOX); break;
  case 2: Fl_Input_::default_box(FL_DOWN_BOX); break;
  }
  Fl::redraw();
}

void scrollbar_thickness_cb(Fl_Value_Slider* v, void*) {
  Fl_Browser::scrollbar_width(int(v->value()));
  Fl::redraw();
}

#include <FL/fl_ask.H>

void defaults_cb(Fl_Button*, void*) {
  fl_alert("Sorry, I didn't implement that");
}

