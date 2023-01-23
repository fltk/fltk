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
  Fl_Menu_Item* subtypes() FL_OVERRIDE {return menu_item_type_menu;}
  const char* type_name() FL_OVERRIDE {return "MenuItem";}
  const char* alt_type_name() FL_OVERRIDE {return "fltk::Item";}
  Fl_Type* make(Strategy strategy) FL_OVERRIDE;
  int is_menu_item() const FL_OVERRIDE {return 1;}
  int is_button() const FL_OVERRIDE {return 1;} // this gets shortcut to work
  Fl_Widget* widget(int,int,int,int) FL_OVERRIDE {return 0;}
  Fl_Widget_Type* _make() FL_OVERRIDE {return 0;}
  virtual const char* menu_name(Fd_Code_Writer& f, int& i);
  int flags();
  void write_static(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_item(Fd_Code_Writer& f);
  void write_code1(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE;
  int pixmapID() FL_OVERRIDE { return 16; }
};

class Fl_Radio_Menu_Item_Type : public Fl_Menu_Item_Type {
public:
  const char* type_name() FL_OVERRIDE {return "RadioMenuItem";}
  Fl_Type* make(Strategy strategy) FL_OVERRIDE;
  int pixmapID() FL_OVERRIDE { return 55; }
};

class Fl_Checkbox_Menu_Item_Type : public Fl_Menu_Item_Type {
public:
  const char* type_name() FL_OVERRIDE {return "CheckMenuItem";}
  Fl_Type* make(Strategy strategy) FL_OVERRIDE;
  int pixmapID() FL_OVERRIDE { return 54; }
};

class Fl_Submenu_Type : public Fl_Menu_Item_Type {
public:
  Fl_Menu_Item* subtypes() FL_OVERRIDE {return 0;}
  const char* type_name() FL_OVERRIDE {return "Submenu";}
  const char* alt_type_name() FL_OVERRIDE {return "fltk::ItemGroup";}
  int is_parent() const FL_OVERRIDE {return 1;}
  int is_button() const FL_OVERRIDE {return 0;} // disable shortcut
  Fl_Type* make(Strategy strategy) FL_OVERRIDE;
  // changes to submenu must propagate up so build_menu is called
  // on the parent Fl_Menu_Type:
  void add_child(Fl_Type*a, Fl_Type*b) FL_OVERRIDE {parent->add_child(a,b);}
  void move_child(Fl_Type*a, Fl_Type*b) FL_OVERRIDE {parent->move_child(a,b);}
  void remove_child(Fl_Type*a) FL_OVERRIDE {parent->remove_child(a);}
  int pixmapID() FL_OVERRIDE { return 18; }
};

class Fl_Menu_Type : public Fl_Widget_Type {
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) FL_OVERRIDE {
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
  int is_menu_button() const FL_OVERRIDE {return 1;}
  int is_parent() const FL_OVERRIDE {return 1;}
  int menusize;
  virtual void build_menu();
  Fl_Menu_Type() : Fl_Widget_Type() {menusize = 0;}
  ~Fl_Menu_Type() {
    if (menusize) delete[] (Fl_Menu_Item*)(((Fl_Menu_*)o)->menu());
  }
  void add_child(Fl_Type*, Fl_Type*) FL_OVERRIDE {build_menu();}
  void move_child(Fl_Type*, Fl_Type*) FL_OVERRIDE {build_menu();}
  void remove_child(Fl_Type*) FL_OVERRIDE {build_menu();}
  Fl_Type* click_test(int x, int y) FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;
};

extern Fl_Menu_Item button_type_menu[];

class Fl_Menu_Button_Type : public Fl_Menu_Type {
  Fl_Menu_Item *subtypes() FL_OVERRIDE {return button_type_menu;}
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    Fl_Widget_Type::ideal_size(w, h);
    w += 2 * ((o->labelsize() - 3) & ~1) + o->labelsize() - 4;
    h = (h / 5) * 5;
    if (h < 15) h = 15;
    if (w < (15 + h)) w = 15 + h;
  }
  const char *type_name() FL_OVERRIDE {return "Fl_Menu_Button";}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::MenuButton";}
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    return new Fl_Menu_Button(X,Y,W,H,"menu");}
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Menu_Button_Type();}
  int pixmapID() FL_OVERRIDE { return 26; }
};

extern Fl_Menu_Item dummymenu[];

#include <FL/Fl_Choice.H>
class Fl_Choice_Type : public Fl_Menu_Type {
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
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
  const char *type_name() FL_OVERRIDE {return "Fl_Choice";}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::Choice";}
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    Fl_Choice *myo = new Fl_Choice(X,Y,W,H,"choice:");
    myo->menu(dummymenu);
    return myo;
  }
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Choice_Type();}
  int pixmapID() FL_OVERRIDE { return 15; }
};

class Fl_Input_Choice_Type : public Fl_Menu_Type {
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) FL_OVERRIDE {
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
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    Fl_Input_Choice *myo = (Fl_Input_Choice *)o;
    fl_font(myo->textfont(), myo->textsize());
    h = fl_height() + myo->textsize() - 6;
    w = o->w() - 20 - Fl::box_dw(o->box());
    int ww = (int)fl_width('m');
    w = ((w + ww - 1) / ww) * ww + 20 + Fl::box_dw(o->box());
    if (h < 15) h = 15;
    if (w < (15 + h)) w = 15 + h;
  }
  const char *type_name() FL_OVERRIDE {return "Fl_Input_Choice";}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::ComboBox";}
  Fl_Type* click_test(int,int) FL_OVERRIDE;
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    Fl_Input_Choice *myo = new Fl_Input_Choice(X,Y,W,H,"input choice:");
    myo->menu(dummymenu);
    myo->value("input");
    return myo;
  }
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Input_Choice_Type();}
  void build_menu() FL_OVERRIDE;
  int pixmapID() FL_OVERRIDE { return 53; }
  void copy_properties() FL_OVERRIDE;
};

class Fl_Menu_Bar_Type : public Fl_Menu_Type {
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    w = o->window()->w();
    h = ((o->labelsize() + Fl::box_dh(o->box()) + 4) / 5) * 5;
    if (h < 15) h = 15;
  }
  const char *type_name() FL_OVERRIDE {return "Fl_Menu_Bar";}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::MenuBar";}
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {return new Fl_Menu_Bar(X,Y,W,H);}
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Menu_Bar_Type();}
  int pixmapID() FL_OVERRIDE { return 17; }
};


#endif // _FLUID_FL_MENU_TYPE_H
