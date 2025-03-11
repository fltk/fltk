//
// Menu type header file for the Fast Light Tool Kit (FLTK).
//
// Type for creating all subclasses of Fl_Widget
// This should have the widget pointer in it, but it is still in the
// Node base class.
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

#ifndef FLUID_NODES_MENU_NODE_H
#define FLUID_NODES_MENU_NODE_H

#include "nodes/Button_Node.h"

#include "app/Fd_Snap_Action.h"

#include <FL/Fl_Choice.H>
#include <FL/Fl_Menu_.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>

extern Fl_Menu_Item dummymenu[];
extern Fl_Menu_Item button_type_menu[];
extern Fl_Menu_Item menu_item_type_menu[];
extern Fl_Menu_Item menu_bar_type_menu[];

/**
 \brief Manage all types on menu items.
 Deriving Menu_Item_Node from Button_Node is intentional. For the purpose
 of editing, a Menu Item is implemented with `o` pointing to an Fl_Button for
 holding all properties.
 */
class Menu_Item_Node : public Button_Node
{
public:
  typedef Button_Node super;
  static Menu_Item_Node prototype;
public:
  Fl_Menu_Item* subtypes() FL_OVERRIDE {return menu_item_type_menu;}
  const char* type_name() FL_OVERRIDE {return "MenuItem";}
  const char* alt_type_name() FL_OVERRIDE {return "fltk::Item";}
  Node* make(Strategy strategy) FL_OVERRIDE;
  Node* make(int flags, Strategy strategy);
  int is_button() const FL_OVERRIDE {return 1;} // this gets shortcut to work
  Fl_Widget* widget(int,int,int,int) FL_OVERRIDE {return nullptr;}
  Widget_Node* _make() FL_OVERRIDE {return nullptr;}
  virtual const char* menu_name(fld::io::Code_Writer& f, int& i);
  int flags();
  void write_static(fld::io::Code_Writer& f) FL_OVERRIDE;
  void write_item(fld::io::Code_Writer& f);
  void write_code1(fld::io::Code_Writer& f) FL_OVERRIDE;
  void write_code2(fld::io::Code_Writer& f) FL_OVERRIDE;
  int is_true_widget() const FL_OVERRIDE { return 0; }
  ID id() const FL_OVERRIDE { return ID_Menu_Item; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Menu_Item) ? true : super::is_a(inID); }
};

/**
 \brief Manage Radio style Menu Items.
 */
class Radio_Menu_Item_Node : public Menu_Item_Node
{
public:
  typedef Menu_Item_Node super;
  static Radio_Menu_Item_Node prototype;
public:
  const char* type_name() FL_OVERRIDE {return "RadioMenuItem";}
  Node* make(Strategy strategy) FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Radio_Menu_Item; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Radio_Menu_Item) ? true : super::is_a(inID); }
};

/**
 \brief Manage Checkbox style Menu Items.
 */
class Checkbox_Menu_Item_Node : public Menu_Item_Node
{
public:
  typedef Menu_Item_Node super;
  static Checkbox_Menu_Item_Node prototype;
public:
  const char* type_name() FL_OVERRIDE {return "CheckMenuItem";}
  Node* make(Strategy strategy) FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Checkbox_Menu_Item; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Checkbox_Menu_Item) ? true : super::is_a(inID); }
};

/**
 \brief Manage Submenu style Menu Items.
 Submenu Items are simply buttons just like all other menu items, but they
 can also hold a pointer to a list of submenus, or have a flag set that
 allows submenus to follow in the current array. As buttons, they can
 be clicked by the user, and they will call their callback, if one is set.
 */
class Submenu_Node : public Menu_Item_Node
{
public:
  typedef Menu_Item_Node super;
  static Submenu_Node prototype;
public:
  Fl_Menu_Item* subtypes() FL_OVERRIDE {return nullptr;}
  const char* type_name() FL_OVERRIDE {return "Submenu";}
  const char* alt_type_name() FL_OVERRIDE {return "fltk::ItemGroup";}
  int can_have_children() const FL_OVERRIDE {return 1;}
  int is_button() const FL_OVERRIDE {return 0;} // disable shortcut
  Node* make(Strategy strategy) FL_OVERRIDE;
  // changes to submenu must propagate up so build_menu is called
  // on the parent Menu_Node:
  void add_child(Node*a, Node*b) FL_OVERRIDE {parent->add_child(a,b);}
  void move_child(Node*a, Node*b) FL_OVERRIDE {parent->move_child(a,b);}
  void remove_child(Node*a) FL_OVERRIDE {parent->remove_child(a);}
  ID id() const FL_OVERRIDE { return ID_Submenu; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Submenu) ? true : super::is_a(inID); }
};

// -----------------------------------------------------------------------------

/**
 \brief Base class for all widgets that can have a pulldown menu attached.
 Widgets with this type can be derived from Fl_Menu_ or from
 Fl_Group (Fl_Input_Choice).
 */
class Menu_Manager_Node : public Widget_Node
{
  typedef Widget_Node super;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    h = layout->textsize_not_null() + 8;
    w = layout->textsize_not_null() * 6 + 8;
    Fd_Snap_Action::better_size(w, h);
  }
  int can_have_children() const FL_OVERRIDE {return 1;}
  int menusize;
  virtual void build_menu() = 0;
  Menu_Manager_Node() : Widget_Node() {menusize = 0;}
  void add_child(Node*, Node*) FL_OVERRIDE { build_menu(); }
  void move_child(Node*, Node*) FL_OVERRIDE { build_menu(); }
  void remove_child(Node*) FL_OVERRIDE { build_menu();}
  Node* click_test(int x, int y) FL_OVERRIDE = 0;
  void write_code2(fld::io::Code_Writer& f) FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE = 0;
  ID id() const FL_OVERRIDE { return ID_Menu_Manager_; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Menu_Manager_) ? true : super::is_a(inID); }
};

/**
 \brief Manage the composite widget Input Choice.
 \note Input Choice is a composite window, so `o` will be pointing to a widget
 derived from Fl_Group. All menu related methods from Fl_Menu_Trait_Type must
 be virtual and must be reimplemented here (click_test, build_menu, textstuff).
 */
class Input_Choice_Node : public Menu_Manager_Node
{
public:
  typedef Menu_Manager_Node super;
  static Input_Choice_Node prototype;
private:
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) FL_OVERRIDE {
    Fl_Input_Choice *myo = (Fl_Input_Choice*)(w==4 ? ((Widget_Node*)this->factory)->o : this->o);
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
  ~Input_Choice_Node() {
    if (menusize) delete[] (Fl_Menu_Item*)(((Fl_Input_Choice*)o)->menu());
  }
  const char *type_name() FL_OVERRIDE {return "Fl_Input_Choice";}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::ComboBox";}
  Node* click_test(int,int) FL_OVERRIDE;
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    Fl_Input_Choice *myo = new Fl_Input_Choice(X,Y,W,H,"input choice:");
    myo->menu(dummymenu);
    myo->value("input");
    return myo;
  }
  Widget_Node *_make() FL_OVERRIDE {return new Input_Choice_Node();}
  void build_menu() FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Input_Choice; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Input_Choice) ? true : super::is_a(inID); }
  void copy_properties() FL_OVERRIDE;
};

/**
 \brief Base class to handle widgets that are derived from Fl_Menu_.
 */
class Menu_Base_Node : public Menu_Manager_Node
{
  typedef Menu_Manager_Node super;
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) FL_OVERRIDE {
    Fl_Menu_ *myo = (Fl_Menu_*)(w==4 ? ((Widget_Node*)this->factory)->o : this->o);
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
  int can_have_children() const FL_OVERRIDE {return 1;}
  void build_menu() FL_OVERRIDE;
  ~Menu_Base_Node() {
    if (menusize) delete[] (Fl_Menu_Item*)(((Fl_Menu_*)o)->menu());
  }
  Node* click_test(int x, int y) FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Menu_; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Menu_) ? true : super::is_a(inID); }
};

extern Fl_Menu_Item button_type_menu[];

/**
 \brief Make Menu Button widgets.
 */
class Menu_Button_Node : public Menu_Base_Node
{
public:
  typedef Menu_Base_Node super;
  static Menu_Button_Node prototype;
private:
  Fl_Menu_Item *subtypes() FL_OVERRIDE {return button_type_menu;}
public:
  const char *type_name() FL_OVERRIDE {return "Fl_Menu_Button";}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::MenuButton";}
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    return new Fl_Menu_Button(X,Y,W,H,"menu");}
  Widget_Node *_make() FL_OVERRIDE {return new Menu_Button_Node();}
  ID id() const FL_OVERRIDE { return ID_Menu_Button; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Menu_Button) ? true : super::is_a(inID); }
};


/**
 \brief Manage Choice type menu widgets.
 */
class Choice_Node : public Menu_Base_Node
{
public:
  typedef Menu_Base_Node super;
  static Choice_Node prototype;
public:
  const char *type_name() FL_OVERRIDE {return "Fl_Choice";}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::Choice";}
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    Fl_Choice *myo = new Fl_Choice(X,Y,W,H,"choice:");
    myo->menu(dummymenu);
    return myo;
  }
  Widget_Node *_make() FL_OVERRIDE {return new Choice_Node();}
  ID id() const FL_OVERRIDE { return ID_Choice; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Choice) ? true : super::is_a(inID); }
};


/**
 \brief Manage Menubar widgets.
 */
class Menu_Bar_Node : public Menu_Base_Node
{
public:
  typedef Menu_Base_Node super;
  static Menu_Bar_Node prototype;
private:
  Fl_Menu_Item *subtypes() FL_OVERRIDE {return menu_bar_type_menu;}
public:
  Menu_Bar_Node();
  ~Menu_Bar_Node() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "Fl_Menu_Bar";}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::MenuBar";}
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {return new Fl_Menu_Bar(X,Y,W,H);}
  Widget_Node *_make() FL_OVERRIDE {return new Menu_Bar_Node();}
  void write_static(fld::io::Code_Writer& f) FL_OVERRIDE;
  void write_code1(fld::io::Code_Writer& f) FL_OVERRIDE;
//  void write_code2(fld::io::Code_Writer& f) FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Menu_Bar; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Menu_Bar) ? true : super::is_a(inID); }
  bool is_sys_menu_bar();
  const char *sys_menubar_name();
  const char *sys_menubar_proxy_name();
protected:
  char *_proxy_name;
};


#endif // FLUID_NODES_MENU_NODE_H
