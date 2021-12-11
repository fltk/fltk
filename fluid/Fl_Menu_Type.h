//
// Widget type header file for the Fast Light Tool Kit (FLTK).
//
// Type for creating all subclasses of Fl_Widget
// This should have the widget pointer in it, but it is still in the
// Fl_Type base class.
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

#ifndef _FLUID_FL_MENU_TYPE_H
#define _FLUID_FL_MENU_TYPE_H

#include "Fl_Widget_Type.h"

#include <FL/Fl_Menu_.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>

extern Fl_Menu_Item menu_item_type_menu[];

class Fl_Menu_Item_Type : public Fl_Widget_Type {
public:
  Fl_Menu_Item* subtypes() {return menu_item_type_menu;}
  const char* type_name() {return "MenuItem";}
  const char* alt_type_name() {return "fltk::Item";}
  Fl_Type* make(Strategy strategy);
  int is_menu_item() const {return 1;}
  int is_button() const {return 1;} // this gets shortcut to work
  Fl_Widget* widget(int,int,int,int) {return 0;}
  Fl_Widget_Type* _make() {return 0;}
  const char* menu_name(int& i);
  int flags();
  void write_static();
  void write_item();
  void write_code1();
  void write_code2();
  int pixmapID() { return 16; }
};

class Fl_Radio_Menu_Item_Type : public Fl_Menu_Item_Type {
public:
  const char* type_name() {return "RadioMenuItem";}
  Fl_Type* make(Strategy strategy);
  int pixmapID() { return 55; }
};

class Fl_Checkbox_Menu_Item_Type : public Fl_Menu_Item_Type {
public:
  const char* type_name() {return "CheckMenuItem";}
  Fl_Type* make(Strategy strategy);
  int pixmapID() { return 54; }
};

class Fl_Submenu_Type : public Fl_Menu_Item_Type {
public:
  Fl_Menu_Item* subtypes() {return 0;}
  const char* type_name() {return "Submenu";}
  const char* alt_type_name() {return "fltk::ItemGroup";}
  int is_parent() const {return 1;}
  int is_button() const {return 0;} // disable shortcut
  Fl_Type* make(Strategy strategy);
  // changes to submenu must propagate up so build_menu is called
  // on the parent Fl_Menu_Type:
  void add_child(Fl_Type*a, Fl_Type*b) {parent->add_child(a,b);}
  void move_child(Fl_Type*a, Fl_Type*b) {parent->move_child(a,b);}
  void remove_child(Fl_Type*a) {parent->remove_child(a);}
  int pixmapID() { return 18; }
};

class Fl_Menu_Type : public Fl_Widget_Type {
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
    Fl_Menu_ *myo = (Fl_Menu_*)(w==4 ? ((Fl_Widget_Type*)this->factory)->o : this->o);
    switch (w) {
      case 4:
      case 0: f = myo->textfont(); s = myo->textsize(); c = myo->textcolor(); break;
      case 1: myo->textfont(f); break;
      case 2: myo->textsize(s); break;
      case 3: myo->textcolor(c); break;
    }
    return 1;
  }
public:
  int is_menu_button() const {return 1;}
  int is_parent() const {return 1;}
  int menusize;
  virtual void build_menu();
  Fl_Menu_Type() : Fl_Widget_Type() {menusize = 0;}
  ~Fl_Menu_Type() {
    if (menusize) delete[] (Fl_Menu_Item*)(((Fl_Menu_*)o)->menu());
  }
  void add_child(Fl_Type*, Fl_Type*) {build_menu();}
  void move_child(Fl_Type*, Fl_Type*) {build_menu();}
  void remove_child(Fl_Type*) {build_menu();}
  Fl_Type* click_test(int x, int y);
  void write_code2();
  void copy_properties();
};

extern Fl_Menu_Item button_type_menu[];

class Fl_Menu_Button_Type : public Fl_Menu_Type {
  Fl_Menu_Item *subtypes() {return button_type_menu;}
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Widget_Type::ideal_size(w, h);
    w += 2 * ((o->labelsize() - 3) & ~1) + o->labelsize() - 4;
    h = (h / 5) * 5;
    if (h < 15) h = 15;
    if (w < (15 + h)) w = 15 + h;
  }
  virtual const char *type_name() {return "Fl_Menu_Button";}
  virtual const char *alt_type_name() {return "fltk::MenuButton";}
  Fl_Widget *widget(int X,int Y,int W,int H) {
    return new Fl_Menu_Button(X,Y,W,H,"menu");}
  Fl_Widget_Type *_make() {return new Fl_Menu_Button_Type();}
  int pixmapID() { return 26; }
};

extern Fl_Menu_Item dummymenu[];

#include <FL/Fl_Choice.H>
class Fl_Choice_Type : public Fl_Menu_Type {
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Widget_Type::ideal_size(w, h);
    int w1 = o->h() - Fl::box_dh(o->box());
    if (w1 > 20) w1 = 20;
    w1 = (w1 - 4) / 3;
    if (w1 < 1) w1 = 1;
    w += 2 * w1 + o->labelsize() - 4;
    h = (h / 5) * 5;
    if (h < 15) h = 15;
    if (w < (15 + h)) w = 15 + h;
  }
  virtual const char *type_name() {return "Fl_Choice";}
  virtual const char *alt_type_name() {return "fltk::Choice";}
  Fl_Widget *widget(int X,int Y,int W,int H) {
    Fl_Choice *myo = new Fl_Choice(X,Y,W,H,"choice:");
    myo->menu(dummymenu);
    return myo;
  }
  Fl_Widget_Type *_make() {return new Fl_Choice_Type();}
  int pixmapID() { return 15; }
};

class Fl_Input_Choice_Type : public Fl_Menu_Type {
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
    Fl_Input_Choice *myo = (Fl_Input_Choice*)(w==4 ? ((Fl_Widget_Type*)this->factory)->o : this->o);
    switch (w) {
      case 4:
      case 0: f = (Fl_Font)myo->textfont(); s = myo->textsize(); c = myo->textcolor(); break;
      case 1: myo->textfont(f); break;
      case 2: myo->textsize(s); break;
      case 3: myo->textcolor(c); break;
    }
    return 1;
  }
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Input_Choice *myo = (Fl_Input_Choice *)o;
    fl_font(myo->textfont(), myo->textsize());
    h = fl_height() + myo->textsize() - 6;
    w = o->w() - 20 - Fl::box_dw(o->box());
    int ww = (int)fl_width('m');
    w = ((w + ww - 1) / ww) * ww + 20 + Fl::box_dw(o->box());
    if (h < 15) h = 15;
    if (w < (15 + h)) w = 15 + h;
  }
  virtual const char *type_name() {return "Fl_Input_Choice";}
  virtual const char *alt_type_name() {return "fltk::ComboBox";}
  virtual Fl_Type* click_test(int,int);
  Fl_Widget *widget(int X,int Y,int W,int H) {
    Fl_Input_Choice *myo = new Fl_Input_Choice(X,Y,W,H,"input choice:");
    myo->menu(dummymenu);
    myo->value("input");
    return myo;
  }
  Fl_Widget_Type *_make() {return new Fl_Input_Choice_Type();}
  virtual void build_menu();
  int pixmapID() { return 53; }
  void copy_properties();
};

class Fl_Menu_Bar_Type : public Fl_Menu_Type {
public:
  virtual void ideal_size(int &w, int &h) {
    w = o->window()->w();
    h = ((o->labelsize() + Fl::box_dh(o->box()) + 4) / 5) * 5;
    if (h < 15) h = 15;
  }
  virtual const char *type_name() {return "Fl_Menu_Bar";}
  virtual const char *alt_type_name() {return "fltk::MenuBar";}
  Fl_Widget *widget(int X,int Y,int W,int H) {return new Fl_Menu_Bar(X,Y,W,H);}
  Fl_Widget_Type *_make() {return new Fl_Menu_Bar_Type();}
  int pixmapID() { return 17; }
};


#endif // _FLUID_FL_MENU_TYPE_H
