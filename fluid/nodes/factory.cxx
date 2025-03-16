//
// Node Factory code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

/**

 
 \todo Verify the text

 Type classes for most of the fltk widgets.  Most of the work
 is done by code in Widget_Node.cxx.  Also a factory instance
 of each of these type classes.

 This file also contains the "new" menu, which has a pointer
 to a factory instance for every class (both the ones defined
 here and ones in other files)


 Type classes for most of the fltk widgets.  Most of the work
 is done by code in Widget_Node.C.  Also a factory instance
 of each of these type classes.

 This file also contains the "new" menu, which has a pointer
 to a factory instance for every class (both the ones defined
 here and ones in other files)

 */
#include "nodes/factory.h"

#include "app/Snap_Action.h"
#include "Fluid.h"
#include "proj/undo.h"
#include "nodes/Button_Node.h"
#include "nodes/Function_Node.h"
#include "nodes/Grid_Node.h"
#include "nodes/Group_Node.h"
#include "nodes/Menu_Node.h"
#include "nodes/Widget_Node.h"
#include "nodes/Window_Node.h"
#include "rsrcs/pixmaps.h"

#include <FL/Fl.H>
#include <FL/Fl_Adjuster.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_Clock.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Dial.H>
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_File_Input.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Terminal.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Window.H>
#include "../src/flstring.h"

#include <stdio.h>
#include <stdlib.h>


// ---- Browser Types -------------------------------------------------- MARK: -


// ---- Browser_Base ----

static Fl_Menu_Item browser_base_type_menu[] = {
  {"No Select", 0, nullptr, (void*)nullptr},
  {"Select", 0, nullptr, (void*)FL_SELECT_BROWSER},
  {"Hold", 0, nullptr, (void*)FL_HOLD_BROWSER},
  {"Multi", 0, nullptr, (void*)FL_MULTI_BROWSER},
  {nullptr}
};

/**
 \brief This is the base class for some browsers types.
 This class will not be instantiated.
 */
class Browser_Base_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Browser_Base_Node prototype;
private:
  Fl_Menu_Item *subtypes() override { return browser_base_type_menu; }
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) override {
    Fl_Browser_ *myo = (Fl_Browser_*)(w==4 ? ((Widget_Node*)factory)->o : o);
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
  void ideal_size(int &w, int &h) override {
    w = 120;
    h = 160;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Browser_"; }
  const char *alt_type_name() override { return "fltk::Browser_"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Browser* b = new Fl_Browser(x, y, w, h);
    return b;
  }
  Widget_Node *_make() override { return new Browser_Base_Node(); }
  Type type() const override { return Type::Browser_; }
  bool is_a(Type inType) const override { return (inType==Type::Browser_) ? true : super::is_a(inType); }
};

Browser_Base_Node Browser_Base_Node::prototype;


// ---- Browser ----

/**
 \brief Handle a plain browser widget.
 Most of the work is already done in Browser_Base_Node.
 */
class Browser_Node : public Browser_Base_Node
{
public:
  typedef Browser_Base_Node super;
  static Browser_Node prototype;
public:
  const char *type_name() override { return "Fl_Browser"; }
  const char *alt_type_name() override { return "fltk::Browser"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Browser* b = new Fl_Browser(x, y, w, h);
    // Fl_Browser::add calls fl_height(), which requires the X display open.
    // Avoid this when compiling so it works w/o a display:
    if (!Fluid.batch_mode) {
      char buffer[20];
      for (int i = 1; i <= 20; i++) {
        sprintf(buffer,"Browser Line %d",i);
        b->add(buffer);
      }
    }
    return b;
  }
  Widget_Node *_make() override { return new Browser_Node(); }
  Type type() const override { return Type::Browser; }
  bool is_a(Type inType) const override { return (inType==Type::Browser) ? true : super::is_a(inType); }
};

Browser_Node Browser_Node::prototype;


// ---- Check Browser ----

/**
 \brief Manage the Check Browser.
 The Fl_Check_Browser is derived form Fl_Browser_ (underline!), not Fl_Browser.
 */
class Check_Browser_Node : public Browser_Base_Node
{
public:
  typedef Browser_Base_Node super;
  static Check_Browser_Node prototype;
public:
  const char *type_name() override { return "Fl_Check_Browser"; }
  const char *alt_type_name() override { return "fltk::CheckBrowser"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Check_Browser* b = new Fl_Check_Browser(x, y, w, h);
    // Fl_Check_Browser::add calls fl_height(), which requires the X display open.
    // Avoid this when compiling so it works w/o a display:
    if (!Fluid.batch_mode) {
      char buffer[20];
      for (int i = 1; i <= 20; i++) {
        sprintf(buffer,"Browser Line %d",i);
        b->add(buffer);
      }
    }
    return b;
  }
  Widget_Node *_make() override { return new Check_Browser_Node(); }
  Type type() const override { return Type::Check_Browser; }
  bool is_a(Type inType) const override { return (inType==Type::Check_Browser) ? true : super::is_a(inType); }
};

Check_Browser_Node Check_Browser_Node::prototype;


// ---- File Browser ----

/**
 \brief Manage the File Browser, not to be confused with the file dialog.
 As oppoesed to the Hold, Multi, and Select Browser, this is not a subclass, but
 its own implementation, based on Fl_Browser.
 */
class File_Browser_Node : public Browser_Node
{
public:
  typedef Browser_Node super;
  static File_Browser_Node prototype;
public:
  const char *type_name() override { return "Fl_File_Browser"; }
  const char *alt_type_name() override { return "fltk::FileBrowser"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_File_Browser* b = new Fl_File_Browser(x, y, w, h);
    if (!Fluid.batch_mode) b->load(".");
    return b;
  }
  Widget_Node *_make() override { return new File_Browser_Node(); }
  Type type() const override { return Type::File_Browser; }
  bool is_a(Type inType) const override { return (inType==Type::File_Browser) ? true : super::is_a(inType); }
};

File_Browser_Node File_Browser_Node::prototype;


// ---- Tree Type ------------------------------------------------------ MARK: -

/**
 \brief Handle the Tree widget.
 Fl_Tree is derived from Fl_Group, but FLUID does not support extended Fl_Tree
 functionality, so we derive the Type from Widget_Node.
 \note Updating item_labelfont etc. does not refresh any of the existing
    items in the tree, so I decided against implementig those via
    the labelfont UI.
 */
class Tree_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Tree_Node prototype;
public:
  void ideal_size(int &w, int &h) override {
    w = 120;
    h = 160;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Tree"; }
  const char *alt_type_name() override { return "fltk::TreeBrowser"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Tree* b = new Fl_Tree(x, y, w, h);
    if (!Fluid.batch_mode) {
      b->add("/A1/B1/C1");
      b->add("/A1/B1/C2");
      b->add("/A1/B2/C1");
      b->add("/A1/B2/C2");
      b->add("/A2/B1/C1");
      b->add("/A2/B1/C2");
      b->add("/A2/B2/C1");
      b->add("/A2/B2/C2");
    }
    return b;
  }
  Widget_Node *_make() override { return new Tree_Node(); }
  Type type() const override { return Type::Tree; }
  bool is_a(Type inType) const override { return (inType==Type::Tree) ? true : super::is_a(inType); }
};

Tree_Node Tree_Node::prototype;



// ---- Help Viewer ---------------------------------------------------- MARK: -

/**
 \brief Handle the Help View widget.
 Fl_Help_View is derived from Fl_Group, but supporting children is not useful,
 so we derive from Widget_Node.
 */
class Help_View_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Help_View_Node prototype;
private:
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) override {
    Fl_Help_View *myo = (Fl_Help_View*)(w==4 ? ((Widget_Node*)factory)->o : o);
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
  void ideal_size(int &w, int &h) override {
    w = 160;
    h = 120;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Help_View"; }
  const char *alt_type_name() override { return "fltk::HelpView"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Help_View *myo = new Fl_Help_View(x, y, w, h);
    if (!Fluid.batch_mode) {
      myo->value("<HTML><BODY><H1>Fl_Help_View Widget</H1>"
                 "<P>This is a Fl_Help_View widget.</P></BODY></HTML>");
    }
    return myo;
  }
  Widget_Node *_make() override { return new Help_View_Node(); }
  Type type() const override { return Type::Help_View; }
  bool is_a(Type inType) const override { return (inType==Type::Help_View) ? true : super::is_a(inType); }
};

Help_View_Node Help_View_Node::prototype;



// ---- Valuators ------------------------------------------------------ MARK: -


// ---- Valuator Base ----

/**
 \brief Just a base class for all valuators.
 */
class Valuator_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Valuator_Node prototype;
public:
  const char *type_name() override { return "Fl_Valuator"; }
  const char *alt_type_name() override { return "fltk::Valuator"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    return new Fl_Slider(x, y, w, h, "Valuator");
  }
  Widget_Node *_make() override { return new Valuator_Node(); }
  Type type() const override { return Type::Valuator_; }
  bool is_a(Type inType) const override { return (inType==Type::Valuator_) ? true : super::is_a(inType); }
};

Valuator_Node Valuator_Node::prototype;


// ---- Counter ----

static Fl_Menu_Item counter_type_menu[] = {
  { "Normal", 0, nullptr, (void*)nullptr },
  { "Simple", 0, nullptr, (void*)FL_SIMPLE_COUNTER },
  { nullptr }
};

/**
 \brief Manage the Counter widget.
 Strictly speaking, the ideal size should derive from the textsize not the labelsize.
 */
class Counter_Node : public Valuator_Node
{
public:
  typedef Valuator_Node super;
  static Counter_Node prototype;
private:
  Fl_Menu_Item *subtypes() override { return counter_type_menu; }
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) override {
    Fl_Counter *myo = (Fl_Counter*)(w==4 ? ((Widget_Node*)factory)->o : o);
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
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    h = layout->textsize_not_null() + 8;
    w = layout->textsize_not_null() * 4 + 4 * h; // make room for the arrows
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Counter"; }
  const char *alt_type_name() override { return "fltk::Counter"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    return new Fl_Counter(x, y, w, h, "counter:");
  }
  Widget_Node *_make() override { return new Counter_Node(); }
  Type type() const override { return Type::Counter; }
  bool is_a(Type inType) const override { return (inType==Type::Counter) ? true : super::is_a(inType); }
};

Counter_Node Counter_Node::prototype;


// ---- Adjuster ----

/**
 \brief Handle Adjuster widgets which are derived from valuators.
 */
class Adjuster_Node : public Valuator_Node
{
public:
  typedef Valuator_Node super;
  static Adjuster_Node prototype;
public:
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    h = layout->labelsize + 8;
    w = 3 * h;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Adjuster"; }
  const char *alt_type_name() override { return "fltk::Adjuster"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    return new Fl_Adjuster(x, y, w, h);
  }
  Widget_Node *_make() override { return new Adjuster_Node(); }
  Type type() const override { return Type::Adjuster; }
  bool is_a(Type inType) const override { return (inType==Type::Adjuster) ? true : super::is_a(inType); }
};

Adjuster_Node Adjuster_Node::prototype;


// ---- Dial ----

static Fl_Menu_Item dial_type_menu[] = {
  { "Dot", 0, nullptr, (void*)nullptr },
  { "Line", 0, nullptr, (void*)FL_LINE_DIAL },
  { "Fill", 0, nullptr, (void*)FL_FILL_DIAL },
  { nullptr }
};

/**
 \brief Manage dials.
 */
class Dial_Node : public Valuator_Node
{
public:
  typedef Valuator_Node super;
  static Dial_Node prototype;
private:
  Fl_Menu_Item *subtypes() override { return dial_type_menu; }
public:
  void ideal_size(int &w, int &h) override {
    w = 60; h = 60;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Dial"; }
  const char *alt_type_name() override { return "fltk::Dial"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    return new Fl_Dial(x, y, w, h);
  }
  Widget_Node *_make() override { return new Dial_Node(); }
  Type type() const override { return Type::Dial; }
  bool is_a(Type inType) const override { return (inType==Type::Dial) ? true : super::is_a(inType); }
};

Dial_Node Dial_Node::prototype;


// ---- Roller ----

static Fl_Menu_Item roller_type_menu[] = {
  { "Vertical", 0, nullptr, (void*)nullptr },
  { "Horizontal", 0, nullptr, (void*)FL_HORIZONTAL },
  { nullptr }
};

/**
 \brief Manage Roller widgets. They are vertical by default.
 */
class Roller_Node : public Valuator_Node
{
public:
  typedef Valuator_Node super;
  static Roller_Node prototype;
private:
  Fl_Menu_Item *subtypes() override { return roller_type_menu; }
public:
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    w = layout->labelsize + 8;
    h = 4 * w;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Roller"; }
  const char *alt_type_name() override { return "fltk::Roller"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    return new Fl_Roller(x, y, w, h);
  }
  Widget_Node *_make() override { return new Roller_Node(); }
  Type type() const override { return Type::Roller; }
  bool is_a(Type inType) const override { return (inType==Type::Roller) ? true : super::is_a(inType); }
};

Roller_Node Roller_Node::prototype;


// ---- Slider ----

static Fl_Menu_Item slider_type_menu[] = {
  { "Vertical", 0, nullptr, (void*)nullptr },
  { "Horizontal", 0, nullptr, (void*)FL_HOR_SLIDER },
  { "Vert Fill", 0, nullptr, (void*)FL_VERT_FILL_SLIDER },
  { "Horz Fill", 0, nullptr, (void*)FL_HOR_FILL_SLIDER },
  { "Vert Knob", 0, nullptr, (void*)FL_VERT_NICE_SLIDER },
  { "Horz Knob", 0, nullptr, (void*)FL_HOR_NICE_SLIDER },
  { nullptr }
};

/**
 \brief Manage Slider widgets.
 They are vertical by default.
 Fl_Value_Slider has its own type.
 */
class Slider_Node : public Valuator_Node
{
public:
  typedef Valuator_Node super;
  static Slider_Node prototype;
private:
  Fl_Menu_Item *subtypes() override { return slider_type_menu; }
public:
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    w = layout->labelsize + 8;
    h = 4 * w;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Slider"; }
  const char *alt_type_name() override { return "fltk::Slider"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    return new Fl_Slider(x, y, w, h, "slider:");
  }
  Widget_Node *_make() override { return new Slider_Node(); }
  Type type() const override { return Type::Slider; }
  bool is_a(Type inType) const override { return (inType==Type::Slider) ? true : super::is_a(inType); }
};

Slider_Node Slider_Node::prototype;


// ---- Scrollbar ----

static Fl_Menu_Item scrollbar_type_menu[] = {
  { "Vertical", 0, nullptr, (void*)nullptr },
  { "Horizontal", 0, nullptr, (void*)FL_HOR_SLIDER },
  { nullptr }
};

/**
 \brief Manage Scrollbars which are derived from Sliders.
 */
class Scrollbar_Node : public Slider_Node
{
public:
  typedef Slider_Node super;
  static Scrollbar_Node prototype;
private:
  Fl_Menu_Item *subtypes() override { return scrollbar_type_menu; }
public:
  const char *type_name() override { return "Fl_Scrollbar"; }
  const char *alt_type_name() override { return "fltk::Scrollbar"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    return new Fl_Scrollbar(x, y, w, h);
  }
  Widget_Node *_make() override { return new Scrollbar_Node(); }
  Type type() const override { return Type::Scrollbar; }
  bool is_a(Type inType) const override { return (inType==Type::Scrollbar) ? true : super::is_a(inType); }
};

Scrollbar_Node Scrollbar_Node::prototype;


// ---- Value Slider ----

/**
 \brief Manage Value Sliders and their text settings.
 */
class Value_Slider_Node : public Slider_Node
{
public:
  typedef Slider_Node super;
  static Value_Slider_Node prototype;
private:
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) override {
    Fl_Value_Slider *myo = (Fl_Value_Slider*)(w==4 ? ((Widget_Node*)factory)->o : o);
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
  const char *type_name() override { return "Fl_Value_Slider"; }
  const char *alt_type_name() override { return "fltk::ValueSlider"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    return new Fl_Value_Slider(x, y, w, h, "slider:");
  }
  Widget_Node *_make() override { return new Value_Slider_Node(); }
  Type type() const override { return Type::Value_Slider; }
  bool is_a(Type inType) const override { return (inType==Type::Value_Slider) ? true : super::is_a(inType); }
};

Value_Slider_Node Value_Slider_Node::prototype;


// ---- Value Input ----

/**
 \brief Manage Value Inputs and their text settings.
 */
class Value_Input_Node : public Valuator_Node
{
public:
  typedef Valuator_Node super;
  static Value_Input_Node prototype;
private:
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) override {
    Fl_Value_Input *myo = (Fl_Value_Input*)(w==4 ? ((Widget_Node*)factory)->o : o);
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
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    h = layout->textsize_not_null() + 8;
    w = layout->textsize_not_null() * 4 + 8;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Value_Input"; }
  const char *alt_type_name() override { return "fltk::ValueInput"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Value_Input *myo = new Fl_Value_Input(x, y, w, h, "value:");
    return myo;
  }
  Widget_Node *_make() override { return new Value_Input_Node(); }
  Type type() const override { return Type::Value_Input; }
  bool is_a(Type inType) const override { return (inType==Type::Value_Input) ? true : super::is_a(inType); }
};

Value_Input_Node Value_Input_Node::prototype;


// ---- Value Output ----

/**
 \brief Handle Value Output widgets, no shortcut with Value Input unfortunately.
 */
class Value_Output_Node : public Valuator_Node
{
public:
  typedef Valuator_Node super;
  static Value_Output_Node prototype;
private:
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) override {
    Fl_Value_Output *myo = (Fl_Value_Output*)(w==4 ? ((Widget_Node*)factory)->o : o);
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
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    h = layout->textsize_not_null() + 8;
    w = layout->textsize_not_null() * 4 + 8;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Value_Output"; }
  const char *alt_type_name() override { return "fltk::ValueOutput"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Value_Output *myo = new Fl_Value_Output(x, y, w, h, "value:");
    return myo;
  }
  Widget_Node *_make() override { return new Value_Output_Node(); }
  Type type() const override { return Type::Value_Output; }
  bool is_a(Type inType) const override { return (inType==Type::Value_Output) ? true : super::is_a(inType); }
};

Value_Output_Node Value_Output_Node::prototype;



// ---- Input ---------------------------------------------------------- MARK: -


// ---- Input ----

static Fl_Menu_Item input_type_menu[] = {
  { "Normal", 0, nullptr, (void*)nullptr },
  { "Multiline", 0, nullptr, (void*)FL_MULTILINE_INPUT },
  { "Secret", 0, nullptr, (void*)FL_SECRET_INPUT },
  { "Int", 0, nullptr, (void*)FL_INT_INPUT },
  { "Float", 0, nullptr, (void*)FL_FLOAT_INPUT },
  {nullptr}
};

/**
 \brief Manage simple text input widgets.
 The managed class is derived from Fl_Input_, but for simplicity, deriving from
 Widget_Node seems sufficient here.
 */
class Input_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Input_Node prototype;
private:
  Fl_Menu_Item *subtypes() override { return input_type_menu; }
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) override {
    Fl_Input_ *myo = (Fl_Input_*)(w==4 ? ((Widget_Node*)factory)->o : o);
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
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    h = layout->textsize_not_null() + 8;
    w = layout->textsize_not_null() * 6 + 8;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Input"; }
  const char *alt_type_name() override { return "fltk::Input"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Input *myo = new Fl_Input(x, y, w, h, "input:");
    myo->value("Text Input");
    return myo;
  }
  Widget_Node *_make() override { return new Input_Node(); }
  Type type() const override { return Type::Input; }
  bool is_a(Type inType) const override { return (inType==Type::Input) ? true : super::is_a(inType); }
  void copy_properties() override {
    Widget_Node::copy_properties();
    Fl_Input_ *d = (Fl_Input_*)live_widget, *s = (Fl_Input_*)o;
    d->textfont(s->textfont());
    d->textsize(s->textsize());
    d->textcolor(s->textcolor());
    d->shortcut(s->shortcut());
  }
};

Input_Node Input_Node::prototype;


// ---- File Input ----

/**
 \brief Manage file name input widgets.
 */
class File_Input_Node : public Input_Node
{
public:
  typedef Input_Node super;
  static File_Input_Node prototype;
private:
  Fl_Menu_Item *subtypes() override { return nullptr; } // Don't inherit.
public:
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    h = layout->textsize_not_null() + 8 + 10; // Directoy bar is additional 10 pixels high
    w = layout->textsize_not_null() * 10 + 8;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_File_Input"; }
  const char *alt_type_name() override { return "fltk::FileInput"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_File_Input *myo = new Fl_File_Input(x, y, w, h, "file:");
    myo->value("/usr/include/FL/Fl.H");
    return myo;
  }
  Widget_Node *_make() override { return new File_Input_Node(); }
  Type type() const override { return Type::File_Input; }
  bool is_a(Type inType) const override { return (inType==Type::File_Input) ? true : super::is_a(inType); }
};

File_Input_Node File_Input_Node::prototype;


// ---- Output ----

static Fl_Menu_Item output_type_menu[] = {
  { "Normal", 0, nullptr, (void*)FL_NORMAL_OUTPUT },
  { "Multiline", 0, nullptr, (void*)FL_MULTILINE_OUTPUT },
  { nullptr }
};

/**
 \brief Manage Output widgets, derived from Input.
 */
class Output_Node : public Input_Node
{
public:
  typedef Input_Node super;
  static Output_Node prototype;
private:
  Fl_Menu_Item *subtypes() override { return output_type_menu; }
public:
  const char *type_name() override { return "Fl_Output"; }
  const char *alt_type_name() override { return "fltk::Output"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Output *myo = new Fl_Output(x, y, w, h, "output:");
    myo->value("Text Output");
    return myo;
  }
  Widget_Node *_make() override { return new Output_Node(); }
  Type type() const override { return Type::Output; }
  bool is_a(Type inType) const override { return (inType==Type::Output) ? true : super::is_a(inType); }
};

Output_Node Output_Node::prototype;



// ---- Text Editor ---------------------------------------------------- MARK: -


// ---- Text Display ----

/**
 \brief Manage the Text Display as a base class.
 Fl_Text_Display is actually derived from Fl_Group, but for FLUID, deriving
 the type from Widget is better.
 */
class Text_Display_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Text_Display_Node prototype;
private:
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) override {
    Fl_Text_Display *myo = (Fl_Text_Display*)(w==4 ? ((Widget_Node*)factory)->o : o);
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
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    h = layout->textsize_not_null() * 4 + 8;
    w = layout->textsize_not_null() * 10 + 8;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Text_Display"; }
  const char *alt_type_name() override { return "fltk::TextDisplay"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Text_Display *myo = new Fl_Text_Display(x, y, w, h);
    if (!Fluid.batch_mode) {
      Fl_Text_Buffer *b = new Fl_Text_Buffer();
      b->text("Lorem ipsum dolor\nsit amet, consetetur\nsadipscing elitr");
      myo->buffer(b);
    }
    return myo;
  }
  Widget_Node *_make() override { return new Text_Display_Node(); }
  Type type() const override { return Type::Text_Display; }
  bool is_a(Type inType) const override { return (inType==Type::Text_Display) ? true : super::is_a(inType); }
};

Text_Display_Node Text_Display_Node::prototype;


// ---- Text Editor ----

/**
 \brief Manage Text Editors based on Text Display.
 */
class Text_Editor_Node : public Text_Display_Node
{
public:
  typedef Text_Display_Node super;
  static Text_Editor_Node prototype;
public:
  const char *type_name() override {return "Fl_Text_Editor";}
  const char *alt_type_name() override {return "fltk::TextEditor";}
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Text_Editor *myo = new Fl_Text_Editor(x, y, w, h);
    if (!Fluid.batch_mode) {
      Fl_Text_Buffer *b = new Fl_Text_Buffer();
      b->text("Lorem ipsum dolor\nsit amet, consetetur\nsadipscing elitr");
      myo->buffer(b);
    }
    return myo;
  }
  Widget_Node *_make() override { return new Text_Editor_Node(); }
  Type type() const override { return Type::Text_Editor; }
  bool is_a(Type inType) const override { return (inType==Type::Text_Editor) ? true : super::is_a(inType); }
};

Text_Editor_Node Text_Editor_Node::prototype;


// ---- Terminal ----

/** Use this terminal instead of Fl_Terminal to capture resize actions. */
class Fl_Terminal_Proxy : public Fl_Terminal {
public:
  Fl_Terminal_Proxy(int x, int y, int w, int h, const char *l=nullptr)
  : Fl_Terminal(x, y, w, h, l) { }
  void print_sample_text() {
    clear_screen_home(false);
    append("> ls -als");
  }
  void resize(int x, int y, int w, int h) override {
    Fl_Terminal::resize(x, y, w, h);
    // After a resize, the top text vanishes, so make sure we redraw it.
    print_sample_text();
  }
};

/** Use this terminal in batch mode to avoid opening a DISPLAY connection. */
class Fl_Batchmode_Terminal : public Fl_Group {
public:
  Fl_Font tfont_;
  int tsize_;
  Fl_Color tcolor_;
  Fl_Batchmode_Terminal(int x, int y, int w, int h, const char *l=nullptr)
  : Fl_Group(x, y, w, h, l)
  { // set the defaults that Fl_Terminal would set
    box(FL_DOWN_BOX);
    color(FL_FOREGROUND_COLOR);
    selection_color(FL_BACKGROUND_COLOR);
    labeltype(FL_NORMAL_LABEL);
    labelfont(0);
    labelsize(14);
    labelcolor(FL_FOREGROUND_COLOR);
    tfont_ = 4;
    tcolor_ = 0xd0d0d000;
    tsize_ = 14;
    align(Fl_Align(FL_ALIGN_TOP));
    when(FL_WHEN_RELEASE);
    end();
  }
};

/**
 \brief Manage a terminal widget.
 */
class Terminal_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Terminal_Node prototype;
public:
  const char *type_name() override { return "Fl_Terminal"; }
  // Older .fl files with Fl_Simple_Terminal will create a Fl_Terminal instead.
  const char *alt_type_name() override { return "Fl_Simple_Terminal"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Widget *ret = nullptr;
    if (Fluid.batch_mode) {
      ret = new Fl_Batchmode_Terminal(x, y, w, h);
    } else {
      Fl_Terminal_Proxy *term = new Fl_Terminal_Proxy(x, y, w+100, h);
      ret = term;
    }
    return ret;
  }
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) override {
    if (Fluid.batch_mode) {
      Fl_Batchmode_Terminal *myo = (Fl_Batchmode_Terminal*)(w==4 ? ((Widget_Node*)factory)->o : o);
      switch (w) {
        case 4:
        case 0: f = (Fl_Font)myo->tfont_; s = myo->tsize_; c = myo->tcolor_; break;
        case 1: myo->tfont_ = f; break;
        case 2: myo->tsize_ = s; break;
        case 3: myo->tcolor_ = c; break;
      }
    } else {
      Fl_Terminal_Proxy *myo = (Fl_Terminal_Proxy*)(w==4 ? ((Widget_Node*)factory)->o : o);
      switch (w) {
        case 4:
        case 0: f = (Fl_Font)myo->textfont(); s = myo->textsize(); c = myo->textcolor(); break;
        case 1: myo->textfont(f); myo->print_sample_text(); break;
        case 2: myo->textsize(s); myo->print_sample_text(); break;
        case 3: myo->textcolor(c); myo->print_sample_text(); break;
      }
    }
    return 1;
  }
  Widget_Node *_make() override {return new Terminal_Node();}
  Type type() const override { return Type::Terminal; }
  bool is_a(Type inType) const override { return (inType==Type::Terminal) ? true : super::is_a(inType); }
};

Terminal_Node Terminal_Node::prototype;


// ---- Other ---------------------------------------------------------- MARK: -


// ---- Box ----

/**
 \brief Manage box widgets.
 Ideal size is set to 100x100, snapped to layout.
 */
class Box_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Box_Node prototype;
public:
  void ideal_size(int &w, int &h) override {
    w = 100; h = 100;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Box"; }
  const char *alt_type_name() override { return "fltk::Widget"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    return new Fl_Box(x, y, w, h, "label");
  }
  Widget_Node *_make() override { return new Box_Node(); }
  Type type() const override { return Type::Box; }
  bool is_a(Type inType) const override { return (inType==Type::Box) ? true : super::is_a(inType); }
};

Box_Node Box_Node::prototype;


// ---- Clock ----

/**
 \brief Manage Clock widgets.
 Ideal size is set to 80x80 snapped to layout.
 */
class Clock_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Clock_Node prototype;
public:
  void ideal_size(int &w, int &h) override {
    w = 80; h = 80;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Clock"; }
  const char *alt_type_name() override { return "fltk::Clock"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    return new Fl_Clock(x, y, w, h);
  }
  Widget_Node *_make() override { return new Clock_Node(); }
  Type type() const override { return Type::Clock; }
  bool is_a(Type inType) const override { return (inType==Type::Clock) ? true : super::is_a(inType); }
};

Clock_Node Clock_Node::prototype;


// ---- Progress ----

/**
 \brief Manage a Progress widget.
 Ideal size is set to match the label font and label text width times 3.
 \note minimum, maximum, and value must be set via extra code fields.
 */
class Progress_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Progress_Node prototype;
public:
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    h = layout->labelsize + 8;
    w = layout->labelsize * 12;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Progress"; }
  const char *alt_type_name() override { return "fltk::ProgressBar"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Progress *myo = new Fl_Progress(x, y, w, h, "label");
    myo->value(50);
    return myo;
  }
  Widget_Node *_make() override { return new Progress_Node(); }
  Type type() const override { return Type::Progress; }
  bool is_a(Type inType) const override { return (inType==Type::Progress) ? true : super::is_a(inType); }
};

Progress_Node Progress_Node::prototype;

// ---- Spinner ----

static Fl_Menu_Item spinner_type_menu[] = {
  { "Integer", 0, nullptr, (void*)FL_INT_INPUT },
  { "Float",  0, nullptr, (void*)FL_FLOAT_INPUT },
  { nullptr }
};

/**
 \brief Manage Spinner widgets.
 \note Fl_Spinner is derived from Fl_Group, *not* Fl_Valuator as one may expect.
    For FLUID, this means some special handling and no Group support.
 */
class Spinner_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Spinner_Node prototype;
private:
  Fl_Menu_Item *subtypes() override { return spinner_type_menu; }
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) override {
    Fl_Spinner *myo = (Fl_Spinner*)(w==4 ? ((Widget_Node*)factory)->o : o);
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
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    h = layout->textsize_not_null() + 8;
    w = layout->textsize_not_null() * 4 + 8;
    fld::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Spinner"; }
  const char *alt_type_name() override { return "fltk::Spinner"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    return new Fl_Spinner(x, y, w, h, "spinner:");
  }
  Widget_Node *_make() override { return new Spinner_Node(); }
  Type type() const override { return Type::Spinner; }
  bool is_a(Type inType) const override { return (inType==Type::Spinner) ? true : super::is_a(inType); }
};

Spinner_Node Spinner_Node::prototype;



// ---- Type Factory --------------------------------------------------- MARK: -

extern void select(Node *,int);
extern void select_only(Node *);

/**
 List all known types.
 This is used to convert a type name into a pointer to the prototype.
 This list may contain types that are supported in .fl files, but not
 available in the *New* menu.

 \note Make sure that this array stays synchronized to `Fl_Menu_Item New_Menu[]`
    further down in this file.
 */
static Node *known_types[] = {
  // functions
  (Node*)&Function_Node::prototype,
  (Node*)&Code_Node::prototype,
  (Node*)&CodeBlock_Node::prototype,
  (Node*)&Decl_Node::prototype,
  (Node*)&DeclBlock_Node::prototype,
  (Node*)&Class_Node::prototype,
  (Node*)&Widget_Class_Node::prototype,
  (Node*)&Comment_Node::prototype,
  (Node*)&Data_Node::prototype,
  // groups
  (Node*)&Window_Node::prototype,
  (Node*)&Group_Node::prototype,
  (Node*)&Pack_Node::prototype,
  (Node*)&Flex_Node::prototype,
  (Node*)&Tabs_Node::prototype,
  (Node*)&Scroll_Node::prototype,
  (Node*)&Tile_Node::prototype,
  (Node*)&Wizard_Node::prototype,
  (Node*)&Grid_Node::prototype,
  // buttons
  (Node*)&Button_Node::prototype,
  (Node*)&Return_Button_Node::prototype,
  (Node*)&Light_Button_Node::prototype,
  (Node*)&Check_Button_Node::prototype,
  (Node*)&Repeat_Button_Node::prototype,
  (Node*)&Round_Button_Node::prototype,
  // valuators
  (Node*)&Slider_Node::prototype,
  (Node*)&Scrollbar_Node::prototype,
  (Node*)&Value_Slider_Node::prototype,
  (Node*)&Adjuster_Node::prototype,
  (Node*)&Counter_Node::prototype,
  (Node*)&Spinner_Node::prototype,
  (Node*)&Dial_Node::prototype,
  (Node*)&Roller_Node::prototype,
  (Node*)&Value_Input_Node::prototype,
  (Node*)&Value_Output_Node::prototype,
  // text
  (Node*)&Input_Node::prototype,
  (Node*)&Output_Node::prototype,
  (Node*)&Text_Editor_Node::prototype,
  (Node*)&Text_Display_Node::prototype,
  (Node*)&File_Input_Node::prototype,
  (Node*)&Terminal_Node::prototype,
  // menus
  (Node*)&Menu_Bar_Node::prototype,
  (Node*)&Menu_Button_Node::prototype,
  (Node*)&Choice_Node::prototype,
  (Node*)&Input_Choice_Node::prototype,
  (Node*)&Submenu_Node::prototype,
  (Node*)&Menu_Item_Node::prototype,
  (Node*)&Checkbox_Menu_Item_Node::prototype,
  (Node*)&Radio_Menu_Item_Node::prototype,
  // browsers
  (Node*)&Browser_Node::prototype,
  (Node*)&Check_Browser_Node::prototype,
  (Node*)&File_Browser_Node::prototype,
  (Node*)&Tree_Node::prototype,
  (Node*)&Help_View_Node::prototype,
  (Node*)&Table_Node::prototype,
  // misc
  (Node*)&Box_Node::prototype,
  (Node*)&Clock_Node::prototype,
  (Node*)&Progress_Node::prototype,
};

/**
 Create and add a new widget to the widget tree.

 Fluid will try to set a default position for widgets to the user's expectation.
 Using the context menu will put new widgets at the position of the mouse click.
 Pulldown menu and bin actions will generate widgets no too far from previously
 added widgets in the same group.

 Widgets can be added by dragging them from the widget bin to the
 desired location.

 By setting the strategy, widgets are added as the last child of a group (this
 is done when reading them from a file), or close to the current widget, which
 the user would expect in interactive mode.

 \param[in] inPrototype pointer to one of the FL_..._type prototype; note the
    lower case 't' in type.
 \param[in] strategy add after current or as last child
 \param[in] and_open if set to true, call open() on the widget after creating it
 \return the newly created type or nullptr

 \see add_new_widget_from_file(const char*, int)
 add_new_widget_from_user(Node*, int)
 add_new_widget_from_user(const char*, int)
 */
Node *add_new_widget_from_user(Node *inPrototype, Strategy strategy, bool and_open) {
  Fluid.proj.undo.checkpoint();
  Fluid.proj.undo.suspend();
  auto layout = Fluid.proj.layout;
  Node *t = ((Node*)inPrototype)->make(strategy);
  if (t) {
    if (t->is_widget() && !t->is_a(Type::Window)) {
      auto layout = Fluid.proj.layout;
      Widget_Node *wt = (Widget_Node *)t;
      bool changed = false;

      // Set font sizes...
      changed |= (wt->o->labelsize() != layout->labelsize);
      wt->o->labelsize(layout->labelsize);
      if (layout->labelfont >= 0) {
        changed |= (wt->o->labelfont() != layout->labelfont);
        wt->o->labelfont(layout->labelfont);
      }

      Fl_Font fc, f = layout->textfont;
      int sc, s = layout->textsize;
      Fl_Color cc, c;
      wt->textstuff(0, fc, sc, cc);

      if ((f >= 0) && (fc != f)) {
        changed = true;
        wt->textstuff(1, f, s, c);
      }
      if ((s > 0) && (sc != s)) {
        changed = true;
        wt->textstuff(2, f, s, c);
      }

      if (changed && t->is_a(Type::Menu_Item)) {
        Node * tt = t->parent;
        while (tt && !tt->is_a(Type::Menu_Manager_)) tt = tt->parent;
        if (tt)
          ((Menu_Manager_Node*)tt)->build_menu();
      }
    }
    if (t->is_true_widget() && !t->is_a(Type::Window)) {
      // Resize and/or reposition new widget...
      Widget_Node *wt = (Widget_Node *)t;

      // The parent field is already set at this point, so we can use that
      // inside ideal_size().
      int w = 0, h = 0;
      wt->ideal_size(w, h);

      if ((t->parent && t->parent->is_a(Type::Flex))) {
        if (Window_Node::popupx != 0x7FFFFFFF)
          ((Flex_Node*)t->parent)->insert_child_at(((Widget_Node*)t)->o, Window_Node::popupx, Window_Node::popupy);
        t->parent->layout_widget();
      } else if (   wt->is_a(Type::Group)
                 && wt->parent
                 && wt->parent->is_a(Type::Tabs)
                 //&& (Window_Node::popupx == 0x7FFFFFFF)
                 && (layout->top_tabs_margin > 0)) {
        // If the widget is a group and the parent is tabs and the top tabs
        // margin is set (and the user is not requesting a specific position)
        // then prefit the group correctly to the Tabs container.
        Fl_Widget *po = ((Tabs_Node*)wt->parent)->o;
        wt->o->resize(po->x(), po->y() + layout->top_tabs_margin,
                      po->w(), po->h() - layout->top_tabs_margin);
      } else if (   wt->is_a(Type::Menu_Bar)
                 && wt->parent
                 && wt->parent->is_a(Type::Window)
                 && (wt->prev == wt->parent)) {
        // If this is the first child of a window, make the menu bar as wide as
        // the window and drop it at 0, 0. Otherwise just use the suggested size.
        w = wt->o->window()->w();
        wt->o->resize(0, 0, w, h);
      } else {
        if (Window_Node::popupx != 0x7FFFFFFF) {
          // If this callback was called from the RMB popup menu in a window,
          // popupx and popupy will contain the mouse coordinates at RMB event.
          wt->o->resize(Window_Node::popupx, Window_Node::popupy, w, h);
        } else {
          // If popupx is invalid, use the default position and find a good
          // size for the widget.
          wt->o->size(w, h);
        }
      }
      if (t->parent && t->parent->is_a(Type::Grid)) {
        if (Window_Node::popupx != 0x7FFFFFFF) {
          ((Grid_Node*)t->parent)->insert_child_at(((Widget_Node*)t)->o, Window_Node::popupx, Window_Node::popupy);
        } else {
          ((Grid_Node*)t->parent)->insert_child_at_next_free_cell(((Widget_Node*)t)->o);
        }
      }
    }
    if (t->is_a(Type::Window)) {
      int x = 0, y = 0, w = 480, h = 320;
      Window_Node *wt = (Window_Node *)t;
      wt->ideal_size(w, h);
      if (Fluid.main_window) {
        int sx, sy, sw, sh;
        Fl_Window *win = Fluid.main_window;
        int screen = Fl::screen_num(win->x(), win->y());
        Fl::screen_work_area(sx, sy, sw, sh, screen);
        x = sx + sw/2 - w/2;
        y = sy + sh/2 - h/2;
      }
      wt->o->resize(x, y, w, h);
    }
    // make the new widget visible
    select_only(t);
    Fluid.proj.set_modflag(1);
    if (and_open)
      t->open();
  } else {
    Fluid.proj.undo.current_ --;
    Fluid.proj.undo.last_ --;
  }
  Fluid.proj.undo.resume();
  return t;
}

/**
 Create and add a new widget to the widget tree.

 \param[in] inName find the right prototype by this name
 \param[in] strategy where to add the node
 \param[in] and_open if set to true, call open() on the widget after creating it
 \return the newly created type or nullptr

 \see add_new_widget_from_file(const char*, int)
 add_new_widget_from_user(Node*, int)
 add_new_widget_from_user(const char*, int)
 */
Node *add_new_widget_from_user(const char *inName, Strategy strategy, bool and_open) {
  Node *prototype = typename_to_prototype(inName);
  if (prototype)
    return add_new_widget_from_user(prototype, strategy, and_open);
  else
    return nullptr;
}

/**
 Callback for all non-widget menu items.
 */
static void cbf(Fl_Widget *, void *v) {
  Node *t = nullptr;
  if (Fluid.proj.tree.current && Fluid.proj.tree.current->can_have_children())
    t = ((Node*)v)->make(Strategy::AS_LAST_CHILD);
  else
    t = ((Node*)v)->make(Strategy::AFTER_CURRENT);
  select_only(t);
}

/**
 Callback for all widget menu items.

 \param[in] v cast to Node to get the prototype of the type that the user
    wants to create.
 */
static void cb(Fl_Widget *, void *v) {
  Node *t = nullptr;
  if (Fluid.proj.tree.current && Fluid.proj.tree.current->can_have_children())
    t = add_new_widget_from_user((Node*)v, Strategy::AS_LAST_CHILD);
  else
    t = add_new_widget_from_user((Node*)v, Strategy::AFTER_CURRENT);
  select_only(t);
}

/**
 \note Make sure that this menu stays synchronized to `Node *known_types[]`
    defined further up in this file.
 */
Fl_Menu_Item New_Menu[] = {
  {"Code",0,nullptr,nullptr,FL_SUBMENU},
  {"Function/Method",0,cbf,(void*)&Function_Node::prototype},
  {"Code",0,cbf,(void*)&Code_Node::prototype},
  {"Code Block",0,cbf,(void*)&CodeBlock_Node::prototype},
  {"Declaration",0,cbf,(void*)&Decl_Node::prototype},
  {"Declaration Block",0,cbf,(void*)&DeclBlock_Node::prototype},
  {"Class",0,cbf,(void*)&Class_Node::prototype},
  {"Widget Class",0,cb,(void*)&Widget_Class_Node::prototype},
  {"Comment",0,cbf,(void*)&Comment_Node::prototype},
  {"Inlined Data",0,cbf,(void*)&Data_Node::prototype},
  {nullptr},
  {"Group",0,nullptr,nullptr,FL_SUBMENU},
  {nullptr,0,cb,(void*)&Window_Node::prototype},
  {nullptr,0,cb,(void*)&Group_Node::prototype},
  {nullptr,0,cb,(void*)&Pack_Node::prototype},
  {nullptr,0,cb,(void*)&Flex_Node::prototype},
  {nullptr,0,cb,(void*)&Tabs_Node::prototype},
  {nullptr,0,cb,(void*)&Scroll_Node::prototype},
  {nullptr,0,cb,(void*)&Tile_Node::prototype},
  {nullptr,0,cb,(void*)&Wizard_Node::prototype},
  {nullptr,0,cb,(void*)&Grid_Node::prototype},
  {nullptr},
  {"Buttons",0,nullptr,nullptr,FL_SUBMENU},
  {nullptr,0,cb,(void*)&Button_Node::prototype},
  {nullptr,0,cb,(void*)&Return_Button_Node::prototype},
  {nullptr,0,cb,(void*)&Light_Button_Node::prototype},
  {nullptr,0,cb,(void*)&Check_Button_Node::prototype},
  {nullptr,0,cb,(void*)&Repeat_Button_Node::prototype},
  {nullptr,0,cb,(void*)&Round_Button_Node::prototype},
  {nullptr},
  {"Valuators",0,nullptr,nullptr,FL_SUBMENU},
  {nullptr,0,cb,(void*)&Slider_Node::prototype},
  {nullptr,0,cb,(void*)&Scrollbar_Node::prototype},
  {nullptr,0,cb,(void*)&Value_Slider_Node::prototype},
  {nullptr,0,cb,(void*)&Adjuster_Node::prototype},
  {nullptr,0,cb,(void*)&Counter_Node::prototype},
  {nullptr,0,cb,(void*)&Spinner_Node::prototype},
  {nullptr,0,cb,(void*)&Dial_Node::prototype},
  {nullptr,0,cb,(void*)&Roller_Node::prototype},
  {nullptr,0,cb,(void*)&Value_Input_Node::prototype},
  {nullptr,0,cb,(void*)&Value_Output_Node::prototype},
  {nullptr},
  {"Text",0,nullptr,nullptr,FL_SUBMENU},
  {nullptr,0,cb,(void*)&Input_Node::prototype},
  {nullptr,0,cb,(void*)&Output_Node::prototype},
  {nullptr,0,cb,(void*)&Text_Editor_Node::prototype},
  {nullptr,0,cb,(void*)&Text_Display_Node::prototype},
  {nullptr,0,cb,(void*)&File_Input_Node::prototype},
  {nullptr,0,cb,(void*)&Terminal_Node::prototype},
  {nullptr},
  {"Menus",0,nullptr,nullptr,FL_SUBMENU},
  {nullptr,0,cb,(void*)&Menu_Bar_Node::prototype},
  {nullptr,0,cb,(void*)&Menu_Button_Node::prototype},
  {nullptr,0,cb,(void*)&Choice_Node::prototype},
  {nullptr,0,cb,(void*)&Input_Choice_Node::prototype},
  {nullptr,0,cb, (void*)&Submenu_Node::prototype},
  {nullptr,0,cb, (void*)&Menu_Item_Node::prototype},
  {"Checkbox Menu Item",0,cb, (void*)&Checkbox_Menu_Item_Node::prototype},
  {"Radio Menu Item",0,cb, (void*)&Radio_Menu_Item_Node::prototype},
  {nullptr},
  {"Browsers",0,nullptr,nullptr,FL_SUBMENU},
  {nullptr,0,cb,(void*)&Browser_Node::prototype},
  {nullptr,0,cb,(void*)&Check_Browser_Node::prototype},
  {nullptr,0,cb,(void*)&File_Browser_Node::prototype},
  {nullptr,0,cb,(void*)&Tree_Node::prototype},
  {nullptr,0,cb,(void*)&Help_View_Node::prototype},
  {nullptr,0,cb,(void*)&Table_Node::prototype},
  {nullptr},
  {"Other",0,nullptr,nullptr,FL_SUBMENU},
  {nullptr,0,cb,(void*)&Box_Node::prototype},
  {nullptr,0,cb,(void*)&Clock_Node::prototype},
  {nullptr,0,cb,(void*)&Progress_Node::prototype},
  {nullptr},
  {nullptr}};

#include <FL/Fl_Multi_Label.H>

/**
 Modify a menuitem to display an icon in front of the label.
 This is implemented using Fl_Multi_Label as the labeltype (FL_MULTI_LABEL).
 The icon may be null. If ic is null only the text is assigned
 to the label and Fl_Multi_Label is not used.
 \param[in] mi pointer to tme menu item that will be modified
 \param[in] ic icon for the menu, may be nullptr
 \param[in] txt new label text, may *not* be nullptr, will not be copied
 */
static void make_iconlabel(Fl_Menu_Item *mi, Fl_Image *ic, const char *txt)
{
  if (ic) {
    char *t1 = new char[strlen(txt)+6];
    strcpy(t1, " ");
    strcat(t1, txt);
    strcat(t1, "...");
    Fl_Multi_Label *ml = new Fl_Multi_Label;
    ml->labela = (char*)ic;
    ml->labelb = t1;
    ml->typea = FL_IMAGE_LABEL;
    ml->typeb = FL_NORMAL_LABEL;
    ml->label(mi);
  } else {
    if (txt != mi->text)
      mi->label(txt);
  }
}

/**
 Create the labels and icons for the `New_Menu` array.

 Names and icons are taken from the referenced prototypes.
 */
void fill_in_New_Menu() {
  for (unsigned i = 0; i < sizeof(New_Menu)/sizeof(*New_Menu); i++) {
    Fl_Menu_Item *m = New_Menu+i;
    if (m->user_data()) {
      Node *t = (Node*)m->user_data();
      if (m->text) {
        make_iconlabel( m, pixmap[(int)t->type()], m->label() );
      } else {
        const char *n = t->type_name();
        if (!strncmp(n,"Fl_",3)) n += 3;
        if (!strncmp(n,"fltk::",6)) n += 6;
        make_iconlabel( m, pixmap[(int)t->type()], n );
      }
    }
  }
}

/**
 Find the correct prototype for a given type name.
 \param[in] inName a C string that must match type_name() or alt_type_name() of
    one of the known Node classes.
 \return the matching prototype or nullptr
 */
Node *typename_to_prototype(const char *inName)
{
  if (inName==nullptr || *inName==0)
    return nullptr;
  for (unsigned i = 0; i < sizeof(known_types)/sizeof(*known_types); i++) {
    Node *prototype = known_types[i];
    if (fl_ascii_strcasecmp(inName, prototype->type_name())==0)
      return prototype;
    if (fl_ascii_strcasecmp(inName, prototype->alt_type_name())==0)
      return prototype;
  }
  return nullptr;
}

/**
 Create and add a new type node to the widget tree.

 This is used by the .fl file reader. New types are always created as
 the last child of the first compatible parent. New widgets have a default
 setup. Their position, size and label will be read next in the file.

 \param[in] inName a C string that described the type we want
 \param[in] strategy add after current or as last child
 \return the type node that was created or nullptr
 \see add_new_widget_from_file(const char*, int)
 add_new_widget_from_user(Node*, int)
 add_new_widget_from_user(const char*, int)
*/
Node *add_new_widget_from_file(const char *inName, Strategy strategy) {
  Node *prototype = typename_to_prototype(inName);
  if (!prototype)
    return nullptr;
  Node *new_node = prototype->make(strategy);
  return new_node;
}

////////////////////////////////////////////////////////////////

// Since I have included all the .H files, do this table here:
// This table is only used to read fdesign files:

struct symbol {const char *name; int value;};

/**
 Table with all symbols known by the "fdesign" format reader.
 This table does not need to be sorted alphabetically.
 */
static symbol table[] = {
  {"BLACK",                     FL_BLACK},
  {"RED",                       FL_RED},
  {"GREEN",                     FL_GREEN},
  {"YELLOW",                    FL_YELLOW},
  {"BLUE",                      FL_BLUE},
  {"MAGENTA",                   FL_MAGENTA},
  {"CYAN",                      FL_CYAN},
  {"WHITE",                     FL_WHITE},

  {"LCOL",                      FL_BLACK},
  {"COL1",                      FL_GRAY},
  {"MCOL",                      FL_LIGHT1},
  {"LEFT_BCOL",                 FL_LIGHT3},
  {"TOP_BCOL",                  FL_LIGHT2},
  {"BOTTOM_BCOL",               FL_DARK2},
  {"RIGHT_BCOL",                FL_DARK3},
  {"INACTIVE",                  FL_INACTIVE_COLOR},
  {"INACTIVE_COL",              FL_INACTIVE_COLOR},
  {"FREE_COL1",                 FL_FREE_COLOR},
  {"FREE_COL2",                 FL_FREE_COLOR+1},
  {"FREE_COL3",                 FL_FREE_COLOR+2},
  {"FREE_COL4",                 FL_FREE_COLOR+3},
  {"FREE_COL5",                 FL_FREE_COLOR+4},
  {"FREE_COL6",                 FL_FREE_COLOR+5},
  {"FREE_COL7",                 FL_FREE_COLOR+6},
  {"FREE_COL8",                 FL_FREE_COLOR+7},
  {"FREE_COL9",                 FL_FREE_COLOR+8},
  {"FREE_COL10",                FL_FREE_COLOR+9},
  {"FREE_COL11",                FL_FREE_COLOR+10},
  {"FREE_COL12",                FL_FREE_COLOR+11},
  {"FREE_COL13",                FL_FREE_COLOR+12},
  {"FREE_COL14",                FL_FREE_COLOR+13},
  {"FREE_COL15",                FL_FREE_COLOR+14},
  {"FREE_COL16",                FL_FREE_COLOR+15},
  {"TOMATO",                    131},
  {"INDIANRED",                 164},
  {"SLATEBLUE",                 195},
  {"DARKGOLD",                  84},
  {"PALEGREEN",                 157},
  {"ORCHID",                    203},
  {"DARKCYAN",                  189},
  {"DARKTOMATO",                113},
  {"WHEAT",                     174},
  {"ALIGN_CENTER",              FL_ALIGN_CENTER},
  {"ALIGN_TOP",                 FL_ALIGN_TOP},
  {"ALIGN_BOTTOM",              FL_ALIGN_BOTTOM},
  {"ALIGN_LEFT",                FL_ALIGN_LEFT},
  {"ALIGN_RIGHT",               FL_ALIGN_RIGHT},
  {"ALIGN_INSIDE",              FL_ALIGN_INSIDE},
  {"ALIGN_TOP_LEFT",            FL_ALIGN_TOP | FL_ALIGN_LEFT},
  {"ALIGN_TOP_RIGHT",           FL_ALIGN_TOP | FL_ALIGN_RIGHT},
  {"ALIGN_BOTTOM_LEFT",         FL_ALIGN_BOTTOM | FL_ALIGN_LEFT},
  {"ALIGN_BOTTOM_RIGHT",        FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT},
  {"ALIGN_CENTER|FL_ALIGN_INSIDE",      FL_ALIGN_CENTER|FL_ALIGN_INSIDE},
  {"ALIGN_TOP|FL_ALIGN_INSIDE",         FL_ALIGN_TOP|FL_ALIGN_INSIDE},
  {"ALIGN_BOTTOM|FL_ALIGN_INSIDE",      FL_ALIGN_BOTTOM|FL_ALIGN_INSIDE},
  {"ALIGN_LEFT|FL_ALIGN_INSIDE",        FL_ALIGN_LEFT|FL_ALIGN_INSIDE},
  {"ALIGN_RIGHT|FL_ALIGN_INSIDE",       FL_ALIGN_RIGHT|FL_ALIGN_INSIDE},
  {"ALIGN_INSIDE|FL_ALIGN_INSIDE",      FL_ALIGN_INSIDE|FL_ALIGN_INSIDE},
  {"ALIGN_TOP_LEFT|FL_ALIGN_INSIDE",    FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_INSIDE},
  {"ALIGN_TOP_RIGHT|FL_ALIGN_INSIDE",   FL_ALIGN_TOP|FL_ALIGN_RIGHT|FL_ALIGN_INSIDE},
  {"ALIGN_BOTTOM_LEFT|FL_ALIGN_INSIDE", FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_INSIDE},
  {"ALIGN_BOTTOM_RIGHT|FL_ALIGN_INSIDE",FL_ALIGN_BOTTOM|FL_ALIGN_RIGHT|FL_ALIGN_INSIDE},

  {"ALIGN_LEFT_TOP",            FL_ALIGN_TOP | FL_ALIGN_LEFT},
  {"ALIGN_RIGHT_TOP",           FL_ALIGN_TOP | FL_ALIGN_RIGHT},
  {"ALIGN_LEFT_BOTTOM",         FL_ALIGN_BOTTOM | FL_ALIGN_LEFT},
  {"ALIGN_RIGHT_BOTTOM",        FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT},
  {"INVALID_STYLE",             255},
  {"NORMAL_STYLE",              FL_HELVETICA},
  {"BOLD_STYLE",                FL_HELVETICA|FL_BOLD},
  {"ITALIC_STYLE",              FL_HELVETICA|FL_ITALIC},
  {"BOLDITALIC_STYLE",          FL_HELVETICA|FL_BOLD|FL_ITALIC},
  {"FIXED_STYLE",               FL_COURIER},
  {"FIXEDBOLD_STYLE",           FL_COURIER|FL_BOLD},
  {"FIXEDITALIC_STYLE",         FL_COURIER|FL_ITALIC},
  {"FIXEDBOLDITALIC_STYLE",     FL_COURIER|FL_BOLD|FL_ITALIC},
  {"TIMES_STYLE",               FL_TIMES},
  {"TIMESBOLD_STYLE",           FL_TIMES|FL_BOLD},
  {"TIMESITALIC_STYLE",         FL_TIMES|FL_ITALIC},
  {"TIMESBOLDITALIC_STYLE",     FL_TIMES|FL_BOLD|FL_ITALIC},
  {"SHADOW_STYLE",              (_FL_SHADOW_LABEL<<8)},
  {"ENGRAVED_STYLE",            (_FL_ENGRAVED_LABEL<<8)},
  {"EMBOSSED_STYLE",            (_FL_EMBOSSED_LABEL<<0)},
  {"TINY_SIZE",                 8},
  {"SMALL_SIZE",                11},
  {"NORMAL_SIZE",               FL_NORMAL_SIZE},
  {"MEDIUM_SIZE",               18},
  {"LARGE_SIZE",                24},
  {"HUGE_SIZE",                 32},
  {"DEFAULT_SIZE",              FL_NORMAL_SIZE},
  {"TINY_FONT",                 8},
  {"SMALL_FONT",                11},
  {"NORMAL_FONT",               FL_NORMAL_SIZE},
  {"MEDIUM_FONT",               18},
  {"LARGE_FONT",                24},
  {"HUGE_FONT",                 32},
  {"NORMAL_FONT1",              11},
  {"NORMAL_FONT2",              FL_NORMAL_SIZE},
  {"DEFAULT_FONT",              11},
  {"RETURN_END_CHANGED",        0},
  {"RETURN_CHANGED",            1},
  {"RETURN_END",                2},
  {"RETURN_ALWAYS",             3},
  {"PUSH_BUTTON",               FL_TOGGLE_BUTTON},
  {"RADIO_BUTTON",              FL_RADIO_BUTTON},
  {"HIDDEN_BUTTON",             FL_HIDDEN_BUTTON},
  {"SELECT_BROWSER",            FL_SELECT_BROWSER},
  {"HOLD_BROWSER",              FL_HOLD_BROWSER},
  {"MULTI_BROWSER",             FL_MULTI_BROWSER},
  {"SIMPLE_COUNTER",            FL_SIMPLE_COUNTER},
  {"LINE_DIAL",                 FL_LINE_DIAL},
  {"FILL_DIAL",                 FL_FILL_DIAL},
  {"VERT_SLIDER",               FL_VERT_SLIDER},
  {"HOR_SLIDER",                FL_HOR_SLIDER},
  {"VERT_FILL_SLIDER",          FL_VERT_FILL_SLIDER},
  {"HOR_FILL_SLIDER",           FL_HOR_FILL_SLIDER},
  {"VERT_NICE_SLIDER",          FL_VERT_NICE_SLIDER},
  {"HOR_NICE_SLIDER",           FL_HOR_NICE_SLIDER},
};

/**
 \brief Find a symbol in an array of name/value pairs and return the value.

 If numberok is 0, and the symbol was not found, v remains unchanged and the
 function returns 0.

 If numberok is set and no label matched, the symbol is interpreted as a
 string containing an integer. If the string is not an integer, v is set to 0
 and the function returns 0.

 If the symbol is found, or the integer could be read, v is set to the
 value, and the function returns 1.

 \param[in] name find a symbol by this name, a leading "FL_" is ignored
 \param[out] v value associated to the symbol, or the integer value
 \param[in] numberok if set, the symbol can also be a text representing an
    integer number
 \return 0 if the symbol was not found and the integer was not valid
 \return 1 otherwise and set v
 */
int lookup_symbol(const char *name, int &v, int numberok) {
  if ((name[0]=='F') && (name[1]=='L') && (name[2]=='_'))
    name += 3;
  for (int i=0; i < int(sizeof(table)/sizeof(*table)); i++) {
    if (!fl_ascii_strcasecmp(name,table[i].name)) {
      v = table[i].value;
      return 1;
    }
  }
  if (numberok && ((v = atoi(name)) || !strcmp(name,"0")))
    return 1;
  return 0;
}
