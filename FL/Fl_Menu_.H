// Fl_Menu_.H

// This is a base class for all items that have a menu:
// Fl_Menu_Bar, Fl_Menu_Button, Fl_Choice
// This provides storage for a menu item, functions to add/modify/delete
// items, and a call for when the user picks a menu item.
// Implementation in Fl_Menu.C

#ifndef Fl_Menu__H
#define Fl_Menu__H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif
#include "Fl_Menu_Item.H"

class Fl_Menu_ : public Fl_Widget {

  Fl_Menu_Item *menu_;
  const Fl_Menu_Item *value_;
  static Fl_Font default_font_;
  static int default_size_;

protected:

  const Fl_Menu_Item* picked(const Fl_Menu_Item*);
  uchar down_box_;
  uchar textfont_;
  uchar textsize_;
  uchar textcolor_;
  uchar alloc;

public:

  Fl_Menu_(int,int,int,int,const char * =0);
  ~Fl_Menu_();

  const Fl_Menu_Item* test_shortcut() {return picked(menu()->test_shortcut());}
  void global();

  const Fl_Menu_Item *menu() const {return menu_;}
  void menu(const Fl_Menu_Item *m);
  int  add(const char*, int shortcut, Fl_Callback*, void* = 0, int = 0);
  int  add(const char* a, const char* b, Fl_Callback* c,
	  void* d = 0, int e = 0) {return add(a,fl_old_shortcut(b),c,d,e);}
  int  size() const ;
  void clear();
  int  add(const char *);
  void replace(int,const char *);
  void remove(int);
  void shortcut(int i, int s) {menu_[i].shortcut(s);}
  void mode(int i,int x) {menu_[i].flags = x;}
  int  mode(int i) const {return menu_[i].flags;}

  const Fl_Menu_Item *mvalue() const {return value_;}
  int value() const {return value_-menu_;}
  int value(const Fl_Menu_Item*);
  int value(int i) {return value(menu_+i);}
  const char *text() const {return value_ ? value_->text : 0;}
  const char *text(int i) const {return menu_[i].text;}

  Fl_Font textfont() const {return (Fl_Font)textfont_;}
  void textfont(uchar c) {textfont_=c;}
  uchar textsize() const {return textsize_;}
  void textsize(uchar c) {textsize_=c;}
  Fl_Color textcolor() const {return (Fl_Color)textcolor_;}
  void textcolor(uchar c) {textcolor_=c;}

  static void default_font(Fl_Font b) {default_font_ = b;}
  static Fl_Font default_font() {return default_font_;}
  static void default_size(int b) {default_size_ = b;}
  static int default_size() {return default_size_;}

  Fl_Boxtype down_box() const {return (Fl_Boxtype)down_box_;}
  void down_box(Fl_Boxtype b) {down_box_ = b;}

  // back compatability:
  Fl_Color down_color() const {return selection_color();}
  void down_color(uchar c) {selection_color(c);}
};

#endif
