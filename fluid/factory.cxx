//
// Widget factory code for the Fast Light Tool Kit (FLTK).
//
// Type classes for most of the fltk widgets.  Most of the work
// is done by code in Fl_Widget_Type.cxx.  Also a factory instance
// of each of these type classes.
//
// This file also contains the "new" menu, which has a pointer
// to a factory instance for every class (both the ones defined
// here and ones in other files)
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

#include "factory.h"

#include "fluid.h"
#include "Fl_Group_Type.h"
#include "Fl_Grid_Type.h"
#include "Fl_Menu_Type.h"
#include "Fd_Snap_Action.h"
#include "pixmaps.h"
#include "undo.h"

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
  {"No Select", 0, 0, (void*)FL_NORMAL_BROWSER},
  {"Select", 0, 0, (void*)FL_SELECT_BROWSER},
  {"Hold", 0, 0, (void*)FL_HOLD_BROWSER},
  {"Multi", 0, 0, (void*)FL_MULTI_BROWSER},
  {0}
};

/**
 \brief This is the base class for some browsers types.
 This class will not be instantiated.
 */
class Fl_Browser_Base_Type : public Fl_Widget_Type
{
  typedef Fl_Widget_Type super;
  Fl_Menu_Item *subtypes() FL_OVERRIDE { return browser_base_type_menu; }
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) FL_OVERRIDE {
    Fl_Browser_ *myo = (Fl_Browser_*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
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
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    w = 120;
    h = 160;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Browser_"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Browser_"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    Fl_Browser* b = new Fl_Browser(x, y, w, h);
    return b;
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Browser_Base_Type(); }
  ID id() const FL_OVERRIDE { return ID_Browser_; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Browser_) ? true : super::is_a(inID); }
};

static Fl_Browser_Base_Type Fl_Browser_Base_type;


// ---- Browser ----

/**
 \brief Handle a plain browser widget.
 Most of the work is already done in Fl_Browser_Base_Type.
 */
class Fl_Browser_Type : public Fl_Browser_Base_Type
{
  typedef Fl_Browser_Base_Type super;
public:
  const char *type_name() FL_OVERRIDE { return "Fl_Browser"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Browser"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    Fl_Browser* b = new Fl_Browser(x, y, w, h);
    // Fl_Browser::add calls fl_height(), which requires the X display open.
    // Avoid this when compiling so it works w/o a display:
    if (!batch_mode) {
      char buffer[20];
      for (int i = 1; i <= 20; i++) {
        sprintf(buffer,"Browser Line %d",i);
        b->add(buffer);
      }
    }
    return b;
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Browser_Type(); }
  ID id() const FL_OVERRIDE { return ID_Browser; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Browser) ? true : super::is_a(inID); }
};

static Fl_Browser_Type Fl_Browser_type;


// ---- Check Browser ----

/**
 \brief Manage the Check Browser.
 The Fl_Check_Browser is derived form Fl_Browser_ (underline!), not Fl_Browser.
 */
class Fl_Check_Browser_Type : public Fl_Browser_Base_Type
{
  typedef Fl_Browser_Base_Type super;
public:
  const char *type_name() FL_OVERRIDE { return "Fl_Check_Browser"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::CheckBrowser"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    Fl_Check_Browser* b = new Fl_Check_Browser(x, y, w, h);
    // Fl_Check_Browser::add calls fl_height(), which requires the X display open.
    // Avoid this when compiling so it works w/o a display:
    if (!batch_mode) {
      char buffer[20];
      for (int i = 1; i <= 20; i++) {
        sprintf(buffer,"Browser Line %d",i);
        b->add(buffer);
      }
    }
    return b;
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Check_Browser_Type(); }
  ID id() const FL_OVERRIDE { return ID_Check_Browser; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Check_Browser) ? true : super::is_a(inID); }
};

static Fl_Check_Browser_Type Fl_Check_Browser_type;


// ---- File Browser ----

/**
 \brief Manage the File Browser, not to be confused with the file dialog.
 As oppoesed to the Hold, Multi, and Select Browser, this is not a subclass, but
 its own implementation, based on Fl_Browser.
 */
class Fl_File_Browser_Type : public Fl_Browser_Type
{
  typedef Fl_Browser_Type super;
public:
  const char *type_name() FL_OVERRIDE { return "Fl_File_Browser"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::FileBrowser"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    Fl_File_Browser* b = new Fl_File_Browser(x, y, w, h);
    if (!batch_mode) b->load(".");
    return b;
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_File_Browser_Type(); }
  ID id() const FL_OVERRIDE { return ID_File_Browser; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_File_Browser) ? true : super::is_a(inID); }
};

static Fl_File_Browser_Type Fl_File_Browser_type;


// ---- Tree Type ------------------------------------------------------ MARK: -

/**
 \brief Handle the Tree widget.
 Fl_Tree is derived from Fl_Group, but FLUID does not support extended Fl_Tree
 functionality, so we derive the Type from Fl_Widget_Type.
 \note Updating item_labelfont etc. does not refresh any of the existing
    items in the tree, so I decided against implementig those via
    the labelfont UI.
 */
class Fl_Tree_Type : public Fl_Widget_Type
{
  typedef Fl_Widget_Type super;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    w = 120;
    h = 160;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Tree"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::TreeBrowser"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    Fl_Tree* b = new Fl_Tree(x, y, w, h);
    if (!batch_mode) {
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
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Tree_Type(); }
  ID id() const FL_OVERRIDE { return ID_Tree; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Tree) ? true : super::is_a(inID); }
};

static Fl_Tree_Type Fl_Tree_type;



// ---- Help Viewer ---------------------------------------------------- MARK: -

/**
 \brief Handle the Help View widget.
 Fl_Help_View is derived from Fl_Group, but supporting children is not useful,
 so we derive from Fl_Widget_Type.
 */
class Fl_Help_View_Type : public Fl_Widget_Type
{
  typedef Fl_Widget_Type super;
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) FL_OVERRIDE {
    Fl_Help_View *myo = (Fl_Help_View*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
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
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    w = 160;
    h = 120;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Help_View"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::HelpView"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    Fl_Help_View *myo = new Fl_Help_View(x, y, w, h);
    if (!batch_mode) {
      myo->value("<HTML><BODY><H1>Fl_Help_View Widget</H1>"
                 "<P>This is a Fl_Help_View widget.</P></BODY></HTML>");
    }
    return myo;
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Help_View_Type(); }
  ID id() const FL_OVERRIDE { return ID_Help_View; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Help_View) ? true : super::is_a(inID); }
};

static Fl_Help_View_Type Fl_Help_View_type;



// ---- Valuators ------------------------------------------------------ MARK: -


// ---- Valuator Base ----

/**
 \brief Just a base class for all valuators.
 */
class Fl_Valuator_Type : public Fl_Widget_Type
{
  typedef Fl_Widget_Type super;
public:
  const char *type_name() FL_OVERRIDE { return "Fl_Valuator"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Valuator"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Slider(x, y, w, h, "Valuator");
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Valuator_Type(); }
  ID id() const FL_OVERRIDE { return ID_Valuator_; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Valuator_) ? true : super::is_a(inID); }
};

static Fl_Valuator_Type Fl_Valuator_type;


// ---- Counter ----

static Fl_Menu_Item counter_type_menu[] = {
  { "Normal", 0, 0, (void*)FL_NORMAL_COUNTER },
  { "Simple", 0, 0, (void*)FL_SIMPLE_COUNTER },
  { 0 }
};

/**
 \brief Manage the Counter widget.
 Strictly speaking, the ideal size should derive from the textsize not the labelsize.
 */
class Fl_Counter_Type : public Fl_Valuator_Type
{
  typedef Fl_Valuator_Type super;
  Fl_Menu_Item *subtypes() FL_OVERRIDE { return counter_type_menu; }
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) FL_OVERRIDE {
    Fl_Counter *myo = (Fl_Counter*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
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
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    h = layout->textsize_not_null() + 8;
    w = layout->textsize_not_null() * 4 + 4 * h; // make room for the arrows
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Counter"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Counter"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Counter(x, y, w, h, "counter:");
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Counter_Type(); }
  ID id() const FL_OVERRIDE { return ID_Counter; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Counter) ? true : super::is_a(inID); }
};

static Fl_Counter_Type Fl_Counter_type;


// ---- Adjuster ----

/**
 \brief Handle Adjuster widgets which are derived from valuators.
 */
class Fl_Adjuster_Type : public Fl_Valuator_Type
{
  typedef Fl_Valuator_Type super;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    h = layout->labelsize + 8;
    w = 3 * h;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Adjuster"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Adjuster"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Adjuster(x, y, w, h);
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Adjuster_Type(); }
  ID id() const FL_OVERRIDE { return ID_Adjuster; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Adjuster) ? true : super::is_a(inID); }
};

static Fl_Adjuster_Type Fl_Adjuster_type;


// ---- Dial ----

static Fl_Menu_Item dial_type_menu[] = {
  { "Dot", 0, 0, (void*)0 },
  { "Line", 0, 0, (void*)FL_LINE_DIAL },
  { "Fill", 0, 0, (void*)FL_FILL_DIAL },
  { 0 }
};

/**
 \brief Manage dials.
 */
class Fl_Dial_Type : public Fl_Valuator_Type
{
  typedef Fl_Valuator_Type super;
  Fl_Menu_Item *subtypes() FL_OVERRIDE { return dial_type_menu; }
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    w = 60; h = 60;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Dial"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Dial"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Dial(x, y, w, h);
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Dial_Type(); }
  ID id() const FL_OVERRIDE { return ID_Dial; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Dial) ? true : super::is_a(inID); }
};
static Fl_Dial_Type Fl_Dial_type;


// ---- Roller ----

static Fl_Menu_Item roller_type_menu[] = {
  { "Vertical", 0, 0, (void*)0 },
  { "Horizontal", 0, 0, (void*)FL_HORIZONTAL },
  { 0 }
};

/**
 \brief Manage Roller widgets. They are vertical by default.
 */
class Fl_Roller_Type : public Fl_Valuator_Type
{
  typedef Fl_Valuator_Type super;
  Fl_Menu_Item *subtypes() FL_OVERRIDE { return roller_type_menu; }
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    w = layout->labelsize + 8;
    h = 4 * w;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Roller"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Roller"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Roller(x, y, w, h);
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Roller_Type(); }
  ID id() const FL_OVERRIDE { return ID_Roller; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Roller) ? true : super::is_a(inID); }
};

static Fl_Roller_Type Fl_Roller_type;


// ---- Slider ----

static Fl_Menu_Item slider_type_menu[] = {
  { "Vertical", 0, 0, (void*)FL_VERT_SLIDER },
  { "Horizontal", 0, 0, (void*)FL_HOR_SLIDER },
  { "Vert Fill", 0, 0, (void*)FL_VERT_FILL_SLIDER },
  { "Horz Fill", 0, 0, (void*)FL_HOR_FILL_SLIDER },
  { "Vert Knob", 0, 0, (void*)FL_VERT_NICE_SLIDER },
  { "Horz Knob", 0, 0, (void*)FL_HOR_NICE_SLIDER },
  { 0 }
};

/**
 \brief Manage Slider widgets.
 They are vertical by default.
 Fl_Value_Slider has its own type.
 */
class Fl_Slider_Type : public Fl_Valuator_Type
{
  typedef Fl_Valuator_Type super;
  Fl_Menu_Item *subtypes() FL_OVERRIDE { return slider_type_menu; }
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    w = layout->labelsize + 8;
    h = 4 * w;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Slider"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Slider"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Slider(x, y, w, h, "slider:");
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Slider_Type(); }
  ID id() const FL_OVERRIDE { return ID_Slider; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Slider) ? true : super::is_a(inID); }
};

static Fl_Slider_Type Fl_Slider_type;


// ---- Scrollbar ----

static Fl_Menu_Item scrollbar_type_menu[] = {
  { "Vertical", 0, 0, (void*)FL_VERT_SLIDER },
  { "Horizontal", 0, 0, (void*)FL_HOR_SLIDER },
  { 0 }
};

/**
 \brief Manage Scrollbars which are derived from Sliders.
 */
class Fl_Scrollbar_Type : public Fl_Slider_Type
{
  typedef Fl_Slider_Type super;
  Fl_Menu_Item *subtypes() FL_OVERRIDE { return scrollbar_type_menu; }
public:
  const char *type_name() FL_OVERRIDE { return "Fl_Scrollbar"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Scrollbar"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Scrollbar(x, y, w, h);
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Scrollbar_Type(); }
  ID id() const FL_OVERRIDE { return ID_Scrollbar; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Scrollbar) ? true : super::is_a(inID); }
};
static Fl_Scrollbar_Type Fl_Scrollbar_type;


// ---- Value Slider ----

/**
 \brief Manage Value Sliders and their text settings.
 */
class Fl_Value_Slider_Type : public Fl_Slider_Type
{
  typedef Fl_Slider_Type super;
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) FL_OVERRIDE {
    Fl_Value_Slider *myo = (Fl_Value_Slider*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
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
  const char *type_name() FL_OVERRIDE { return "Fl_Value_Slider"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::ValueSlider"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Value_Slider(x, y, w, h, "slider:");
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Value_Slider_Type(); }
  ID id() const FL_OVERRIDE { return ID_Value_Slider; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Value_Slider) ? true : super::is_a(inID); }
};

static Fl_Value_Slider_Type Fl_Value_Slider_type;


// ---- Value Input ----

/**
 \brief Manage Value Inputs and their text settings.
 */
class Fl_Value_Input_Type : public Fl_Valuator_Type
{
  typedef Fl_Valuator_Type super;
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) FL_OVERRIDE {
    Fl_Value_Input *myo = (Fl_Value_Input*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
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
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    h = layout->textsize_not_null() + 8;
    w = layout->textsize_not_null() * 4 + 8;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Value_Input"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::ValueInput"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    Fl_Value_Input *myo = new Fl_Value_Input(x, y, w, h, "value:");
    return myo;
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Value_Input_Type(); }
  ID id() const FL_OVERRIDE { return ID_Value_Input; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Value_Input) ? true : super::is_a(inID); }
};

static Fl_Value_Input_Type Fl_Value_Input_type;


// ---- Value Output ----

/**
 \brief Handle Value Output widgets, no shortcut with Value Input unfortunately.
 */
class Fl_Value_Output_Type : public Fl_Valuator_Type
{
  typedef Fl_Valuator_Type super;
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) FL_OVERRIDE {
    Fl_Value_Output *myo = (Fl_Value_Output*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
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
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    h = layout->textsize_not_null() + 8;
    w = layout->textsize_not_null() * 4 + 8;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Value_Output"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::ValueOutput"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    Fl_Value_Output *myo = new Fl_Value_Output(x, y, w, h, "value:");
    return myo;
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Value_Output_Type(); }
  ID id() const FL_OVERRIDE { return ID_Value_Output; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Value_Output) ? true : super::is_a(inID); }
};

static Fl_Value_Output_Type Fl_Value_Output_type;



// ---- Input ---------------------------------------------------------- MARK: -


// ---- Input ----

static Fl_Menu_Item input_type_menu[] = {
  { "Normal", 0, 0, (void*)FL_NORMAL_INPUT },
  { "Multiline", 0, 0, (void*)FL_MULTILINE_INPUT },
  { "Secret", 0, 0, (void*)FL_SECRET_INPUT },
  { "Int", 0, 0, (void*)FL_INT_INPUT },
  { "Float", 0, 0, (void*)FL_FLOAT_INPUT },
  {0}
};

/**
 \brief Manage simple text input widgets.
 The managed class is derived from Fl_Input_, but for simplicity, deriving from
 Fl_Widget_Type seems sufficient here.
 */
class Fl_Input_Type : public Fl_Widget_Type
{
  typedef Fl_Widget_Type super;
  Fl_Menu_Item *subtypes() FL_OVERRIDE { return input_type_menu; }
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) FL_OVERRIDE {
    Fl_Input_ *myo = (Fl_Input_*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
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
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    h = layout->textsize_not_null() + 8;
    w = layout->textsize_not_null() * 6 + 8;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Input"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Input"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    Fl_Input *myo = new Fl_Input(x, y, w, h, "input:");
    myo->value("Text Input");
    return myo;
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Input_Type(); }
  ID id() const FL_OVERRIDE { return ID_Input; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Input) ? true : super::is_a(inID); }
  void copy_properties() FL_OVERRIDE {
    Fl_Widget_Type::copy_properties();
    Fl_Input_ *d = (Fl_Input_*)live_widget, *s = (Fl_Input_*)o;
    d->textfont(s->textfont());
    d->textsize(s->textsize());
    d->textcolor(s->textcolor());
    d->shortcut(s->shortcut());
  }
};
static Fl_Input_Type Fl_Input_type;


// ---- File Input ----

/**
 \brief Manage file name input widgets.
 */
class Fl_File_Input_Type : public Fl_Input_Type
{
  typedef Fl_Input_Type super;
  Fl_Menu_Item *subtypes() FL_OVERRIDE { return NULL; } // Don't inherit.
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    h = layout->textsize_not_null() + 8 + 10; // Directoy bar is additional 10 pixels high
    w = layout->textsize_not_null() * 10 + 8;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_File_Input"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::FileInput"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    Fl_File_Input *myo = new Fl_File_Input(x, y, w, h, "file:");
    myo->value("/usr/include/FL/Fl.H");
    return myo;
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_File_Input_Type(); }
  ID id() const FL_OVERRIDE { return ID_File_Input; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_File_Input) ? true : super::is_a(inID); }
};

static Fl_File_Input_Type Fl_File_Input_type;


// ---- Output ----

static Fl_Menu_Item output_type_menu[] = {
  { "Normal", 0, 0, (void*)FL_NORMAL_OUTPUT },
  { "Multiline", 0, 0, (void*)FL_MULTILINE_OUTPUT },
  { 0 }
};

/**
 \brief Manage Output widgets, derived from Input.
 */
class Fl_Output_Type : public Fl_Input_Type
{
  typedef Fl_Input_Type super;
  Fl_Menu_Item *subtypes() FL_OVERRIDE { return output_type_menu; }
public:
  const char *type_name() FL_OVERRIDE { return "Fl_Output"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Output"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    Fl_Output *myo = new Fl_Output(x, y, w, h, "output:");
    myo->value("Text Output");
    return myo;
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Output_Type(); }
  ID id() const FL_OVERRIDE { return ID_Output; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Output) ? true : super::is_a(inID); }
};

static Fl_Output_Type Fl_Output_type;



// ---- Text Editor ---------------------------------------------------- MARK: -


// ---- Text Display ----

/**
 \brief Manage the Text Display as a base class.
 Fl_Text_Display is actually derived from Fl_Group, but for FLUID, deriving
 the type from Widget is better.
 */
class Fl_Text_Display_Type : public Fl_Widget_Type
{
  typedef Fl_Widget_Type super;
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) FL_OVERRIDE {
    Fl_Text_Display *myo = (Fl_Text_Display*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
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
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    h = layout->textsize_not_null() * 4 + 8;
    w = layout->textsize_not_null() * 10 + 8;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Text_Display"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::TextDisplay"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    Fl_Text_Display *myo = new Fl_Text_Display(x, y, w, h);
    if (!batch_mode) {
      Fl_Text_Buffer *b = new Fl_Text_Buffer();
      b->text("Lorem ipsum dolor\nsit amet, consetetur\nsadipscing elitr");
      myo->buffer(b);
    }
    return myo;
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Text_Display_Type(); }
  ID id() const FL_OVERRIDE { return ID_Text_Display; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Text_Display) ? true : super::is_a(inID); }
};
static Fl_Text_Display_Type Fl_Text_Display_type;


// ---- Text Editor ----

/**
 \brief Manage Text Editors based on Text Display.
 */
class Fl_Text_Editor_Type : public Fl_Text_Display_Type
{
  typedef Fl_Text_Display_Type super;
public:
  const char *type_name() FL_OVERRIDE {return "Fl_Text_Editor";}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::TextEditor";}
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    Fl_Text_Editor *myo = new Fl_Text_Editor(x, y, w, h);
    if (!batch_mode) {
      Fl_Text_Buffer *b = new Fl_Text_Buffer();
      b->text("Lorem ipsum dolor\nsit amet, consetetur\nsadipscing elitr");
      myo->buffer(b);
    }
    return myo;
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Text_Editor_Type(); }
  ID id() const FL_OVERRIDE { return ID_Text_Editor; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Text_Editor) ? true : super::is_a(inID); }
};

static Fl_Text_Editor_Type Fl_Text_Editor_type;


// ---- Terminal ----

/** Use this terminal instead of Fl_Terminal to capture resize actions. */
class Fl_Terminal_Proxy : public Fl_Terminal {
public:
  Fl_Terminal_Proxy(int x, int y, int w, int h, const char *l=NULL)
  : Fl_Terminal(x, y, w, h, l) { }
  void print_sample_text() {
    clear_screen_home(false);
    append("> ls -als");
  }
  void resize(int x, int y, int w, int h) FL_OVERRIDE {
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
  Fl_Batchmode_Terminal(int x, int y, int w, int h, const char *l=NULL)
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
class Fl_Terminal_Type : public Fl_Widget_Type
{
  typedef Fl_Widget_Type super;
public:
  const char *type_name() FL_OVERRIDE { return "Fl_Terminal"; }
  // Older .fl files with Fl_Simple_Terminal will create a Fl_Terminal instead.
  const char *alt_type_name() FL_OVERRIDE { return "Fl_Simple_Terminal"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    Fl_Widget *ret = NULL;
    if (batch_mode) {
      ret = new Fl_Batchmode_Terminal(x, y, w, h);
    } else {
      Fl_Terminal_Proxy *term = new Fl_Terminal_Proxy(x, y, w+100, h);
      ret = term;
    }
    return ret;
  }
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) FL_OVERRIDE {
    if (batch_mode) {
      Fl_Batchmode_Terminal *myo = (Fl_Batchmode_Terminal*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
      switch (w) {
        case 4:
        case 0: f = (Fl_Font)myo->tfont_; s = myo->tsize_; c = myo->tcolor_; break;
        case 1: myo->tfont_ = f; break;
        case 2: myo->tsize_ = s; break;
        case 3: myo->tcolor_ = c; break;
      }
    } else {
      Fl_Terminal_Proxy *myo = (Fl_Terminal_Proxy*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
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
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Terminal_Type();}
  ID id() const FL_OVERRIDE { return ID_Terminal; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Terminal) ? true : super::is_a(inID); }
};

static Fl_Terminal_Type Fl_Terminal_type;


// ---- Other ---------------------------------------------------------- MARK: -


// ---- Box ----

/**
 \brief Manage box widgets.
 Ideal size is set to 100x100, snapped to layout.
 */
class Fl_Box_Type : public Fl_Widget_Type
{
  typedef Fl_Widget_Type super;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    w = 100; h = 100;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Box"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Widget"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Box(x, y, w, h, "label");
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Box_Type(); }
  ID id() const FL_OVERRIDE { return ID_Box; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Box) ? true : super::is_a(inID); }
};

static Fl_Box_Type Fl_Box_type;


// ---- Clock ----

/**
 \brief Manage Clock widgets.
 Ideal size is set to 80x80 snapped to layout.
 */
class Fl_Clock_Type : public Fl_Widget_Type
{
  typedef Fl_Widget_Type super;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    w = 80; h = 80;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Clock"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Clock"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Clock(x, y, w, h);
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Clock_Type(); }
  ID id() const FL_OVERRIDE { return ID_Clock; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Clock) ? true : super::is_a(inID); }
};

static Fl_Clock_Type Fl_Clock_type;


// ---- Progress ----

/**
 \brief Manage a Progress widget.
 Ideal size is set to match the label font and label text width times 3.
 \note minimum, maximum, and value must be set via extra code fields.
 */
class Fl_Progress_Type : public Fl_Widget_Type
{
  typedef Fl_Widget_Type super;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    h = layout->labelsize + 8;
    w = layout->labelsize * 12;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Progress"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::ProgressBar"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    Fl_Progress *myo = new Fl_Progress(x, y, w, h, "label");
    myo->value(50);
    return myo;
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Progress_Type(); }
  ID id() const FL_OVERRIDE { return ID_Progress; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Progress) ? true : super::is_a(inID); }
};

static Fl_Progress_Type Fl_Progress_type;

// ---- Spinner ----

static Fl_Menu_Item spinner_type_menu[] = {
  { "Integer", 0, 0, (void*)FL_INT_INPUT },
  { "Float",  0, 0, (void*)FL_FLOAT_INPUT },
  { 0 }
};

/**
 \brief Manage Spinner widgets.
 \note Fl_Spinner is derived from Fl_Group, *not* Fl_Valuator as one may expect.
    For FLUID, this means some special handling and no Group support.
 */
class Fl_Spinner_Type : public Fl_Widget_Type
{
  typedef Fl_Widget_Type super;
  Fl_Menu_Item *subtypes() FL_OVERRIDE { return spinner_type_menu; }
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) FL_OVERRIDE {
    Fl_Spinner *myo = (Fl_Spinner*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
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
    h = layout->textsize_not_null() + 8;
    w = layout->textsize_not_null() * 4 + 8;
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Spinner"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Spinner"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Spinner(x, y, w, h, "spinner:");
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Spinner_Type(); }
  ID id() const FL_OVERRIDE { return ID_Spinner; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Spinner) ? true : super::is_a(inID); }
};

static Fl_Spinner_Type Fl_Spinner_type;



// ---- Type Factory --------------------------------------------------- MARK: -

extern class Fl_Function_Type Fl_Function_type;
extern class Fl_Code_Type Fl_Code_type;
extern class Fl_CodeBlock_Type Fl_CodeBlock_type;
extern class Fl_Data_Type Fl_Data_type;
extern class Fl_Decl_Type Fl_Decl_type;
extern class Fl_DeclBlock_Type Fl_DeclBlock_type;
extern class Fl_Comment_Type Fl_Comment_type;
extern class Fl_Class_Type Fl_Class_type;
extern class Fl_Window_Type Fl_Window_type;
extern class Fl_Widget_Class_Type Fl_Widget_Class_type;
extern class Fl_Group_Type Fl_Group_type;
extern class Fl_Pack_Type Fl_Pack_type;
extern class Fl_Flex_Type Fl_Flex_type;
extern class Fl_Grid_Type Fl_Grid_type;
extern class Fl_Tabs_Type Fl_Tabs_type;
extern class Fl_Scroll_Type Fl_Scroll_type;
extern class Fl_Table_Type Fl_Table_type;
extern class Fl_Tile_Type Fl_Tile_type;
extern class Fl_Input_Choice_Type Fl_Input_Choice_type;
extern class Fl_Choice_Type Fl_Choice_type;
extern class Fl_Menu_Bar_Type Fl_Menu_Bar_type;
extern class Fl_Menu_Button_Type Fl_Menu_Button_type;
extern class Fl_Menu_Item_Type Fl_Menu_Item_type;
extern class Fl_Checkbox_Menu_Item_Type Fl_Checkbox_Menu_Item_type;
extern class Fl_Radio_Menu_Item_Type Fl_Radio_Menu_Item_type;
extern class Fl_Submenu_Type Fl_Submenu_type;
extern class Fl_Wizard_Type Fl_Wizard_type;

extern class Fl_Button_Type Fl_Button_type;
extern class Fl_Return_Button_Type Fl_Return_Button_type;
extern class Fl_Light_Button_Type Fl_Light_Button_type;
extern class Fl_Check_Button_Type Fl_Check_Button_type;
extern class Fl_Repeat_Button_Type Fl_Repeat_Button_type;
extern class Fl_Round_Button_Type Fl_Round_Button_type;

extern void select(Fl_Type *,int);
extern void select_only(Fl_Type *);

/**
 List all known types.
 This is used to convert a type name into a pointer to the prototype.
 This list may contain types that are supported in .fl files, but not
 available in the *New* menu.
 */
static Fl_Type *known_types[] = {
  // functions
  (Fl_Type*)&Fl_Function_type,
  (Fl_Type*)&Fl_Code_type,
  (Fl_Type*)&Fl_CodeBlock_type,
  (Fl_Type*)&Fl_Decl_type,
  (Fl_Type*)&Fl_DeclBlock_type,
  (Fl_Type*)&Fl_Class_type,
  (Fl_Type*)&Fl_Widget_Class_type,
  (Fl_Type*)&Fl_Comment_type,
  (Fl_Type*)&Fl_Data_type,
  // groups
  (Fl_Type*)&Fl_Window_type,
  (Fl_Type*)&Fl_Group_type,
  (Fl_Type*)&Fl_Pack_type,
  (Fl_Type*)&Fl_Flex_type,
  (Fl_Type*)&Fl_Tabs_type,
  (Fl_Type*)&Fl_Scroll_type,
  (Fl_Type*)&Fl_Tile_type,
  (Fl_Type*)&Fl_Wizard_type,
  (Fl_Type*)&Fl_Grid_type,
  // buttons
  (Fl_Type*)&Fl_Button_type,
  (Fl_Type*)&Fl_Return_Button_type,
  (Fl_Type*)&Fl_Light_Button_type,
  (Fl_Type*)&Fl_Check_Button_type,
  (Fl_Type*)&Fl_Repeat_Button_type,
  (Fl_Type*)&Fl_Round_Button_type,
  // valuators
  (Fl_Type*)&Fl_Slider_type,
  (Fl_Type*)&Fl_Scrollbar_type,
  (Fl_Type*)&Fl_Value_Slider_type,
  (Fl_Type*)&Fl_Adjuster_type,
  (Fl_Type*)&Fl_Counter_type,
  (Fl_Type*)&Fl_Spinner_type,
  (Fl_Type*)&Fl_Dial_type,
  (Fl_Type*)&Fl_Roller_type,
  (Fl_Type*)&Fl_Value_Input_type,
  (Fl_Type*)&Fl_Value_Output_type,
  // text
  (Fl_Type*)&Fl_Input_type,
  (Fl_Type*)&Fl_Output_type,
  (Fl_Type*)&Fl_Text_Editor_type,
  (Fl_Type*)&Fl_Text_Display_type,
  (Fl_Type*)&Fl_File_Input_type,
  (Fl_Type*)&Fl_Terminal_type,
  // menus
  (Fl_Type*)&Fl_Menu_Bar_type,
  (Fl_Type*)&Fl_Menu_Button_type,
  (Fl_Type*)&Fl_Choice_type,
  (Fl_Type*)&Fl_Input_Choice_type,
  (Fl_Type*)&Fl_Submenu_type,
  (Fl_Type*)&Fl_Menu_Item_type,
  (Fl_Type*)&Fl_Checkbox_Menu_Item_type,
  (Fl_Type*)&Fl_Radio_Menu_Item_type,
  // browsers
  (Fl_Type*)&Fl_Browser_type,
  (Fl_Type*)&Fl_Check_Browser_type,
  (Fl_Type*)&Fl_File_Browser_type,
  (Fl_Type*)&Fl_Tree_type,
  (Fl_Type*)&Fl_Help_View_type,
  (Fl_Type*)&Fl_Table_type,
  // misc
  (Fl_Type*)&Fl_Box_type,
  (Fl_Type*)&Fl_Clock_type,
  (Fl_Type*)&Fl_Progress_type,
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

 \see add_new_widget_from_file(const char*, int)
 add_new_widget_from_user(Fl_Type*, int)
 add_new_widget_from_user(const char*, int)
 */
Fl_Type *add_new_widget_from_user(Fl_Type *inPrototype, Strategy strategy, bool and_open) {
  undo_checkpoint();
  undo_suspend();
  Fl_Type *t = ((Fl_Type*)inPrototype)->make(strategy);
  if (t) {
    if (t->is_widget() && !t->is_a(ID_Window)) {
      Fl_Widget_Type *wt = (Fl_Widget_Type *)t;
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

      if (changed && t->is_a(ID_Menu_Item)) {
        Fl_Type * tt = t->parent;
        while (tt && !tt->is_a(ID_Menu_Manager_)) tt = tt->parent;
        if (tt)
          ((Fl_Menu_Manager_Type*)tt)->build_menu();
      }
    }
    if (t->is_true_widget() && !t->is_a(ID_Window)) {
      // Resize and/or reposition new widget...
      Fl_Widget_Type *wt = (Fl_Widget_Type *)t;

      // The parent field is already set at this point, so we can use that
      // inside ideal_size().
      int w = 0, h = 0;
      wt->ideal_size(w, h);

      if ((t->parent && t->parent->is_a(ID_Flex))) {
        if (Fl_Window_Type::popupx != 0x7FFFFFFF)
          ((Fl_Flex_Type*)t->parent)->insert_child_at(((Fl_Widget_Type*)t)->o, Fl_Window_Type::popupx, Fl_Window_Type::popupy);
        t->parent->layout_widget();
      } else if (   wt->is_a(ID_Group)
                 && wt->parent
                 && wt->parent->is_a(ID_Tabs)
                 //&& (Fl_Window_Type::popupx == 0x7FFFFFFF)
                 && (layout->top_tabs_margin > 0)) {
        // If the widget is a group and the parent is tabs and the top tabs
        // margin is set (and the user is not requesting a specific position)
        // then prefit the group correctly to the Tabs container.
        Fl_Widget *po = ((Fl_Tabs_Type*)wt->parent)->o;
        wt->o->resize(po->x(), po->y() + layout->top_tabs_margin,
                      po->w(), po->h() - layout->top_tabs_margin);
      } else if (   wt->is_a(ID_Menu_Bar)
                 && wt->parent
                 && wt->parent->is_a(ID_Window)
                 && (wt->prev == wt->parent)) {
        // If this is the first child of a window, make the menu bar as wide as
        // the window and drop it at 0, 0. Otherwise just use the suggested size.
        w = wt->o->window()->w();
        wt->o->resize(0, 0, w, h);
      } else {
        if (Fl_Window_Type::popupx != 0x7FFFFFFF) {
          // If this callback was called from the RMB popup menu in a window,
          // popupx and popupy will contain the mouse coordinates at RMB event.
          wt->o->resize(Fl_Window_Type::popupx, Fl_Window_Type::popupy, w, h);
        } else {
          // If popupx is invalid, use the default position and find a good
          // size for the widget.
          wt->o->size(w, h);
        }
      }
      if (t->parent && t->parent->is_a(ID_Grid)) {
        if (Fl_Window_Type::popupx != 0x7FFFFFFF) {
          ((Fl_Grid_Type*)t->parent)->insert_child_at(((Fl_Widget_Type*)t)->o, Fl_Window_Type::popupx, Fl_Window_Type::popupy);
        } else {
          ((Fl_Grid_Type*)t->parent)->insert_child_at_next_free_cell(((Fl_Widget_Type*)t)->o);
        }
      }
    }
    if (t->is_a(ID_Window)) {
      int x = 0, y = 0, w = 480, h = 320;
      Fl_Window_Type *wt = (Fl_Window_Type *)t;
      wt->ideal_size(w, h);
      if (main_window) {
        int sx, sy, sw, sh;
        Fl_Window *win = main_window;
        int screen = Fl::screen_num(win->x(), win->y());
        Fl::screen_work_area(sx, sy, sw, sh, screen);
        x = sx + sw/2 - w/2;
        y = sy + sh/2 - h/2;
      }
      wt->o->resize(x, y, w, h);
    }
    // make the new widget visible
    select_only(t);
    set_modflag(1);
    if (and_open)
      t->open();
  } else {
    undo_current --;
    undo_last --;
  }
  undo_resume();
  return t;
}

/**
 Create and add a new widget to the widget tree.
 \param[in] inName find the right prototype by this name
 \param[in] strategy where to add the node
 \return the newly created node
 \see add_new_widget_from_file(const char*, int)
 add_new_widget_from_user(Fl_Type*, int)
 add_new_widget_from_user(const char*, int)
 */
Fl_Type *add_new_widget_from_user(const char *inName, Strategy strategy, bool and_open) {
  Fl_Type *prototype = typename_to_prototype(inName);
  if (prototype)
    return add_new_widget_from_user(prototype, strategy, and_open);
  else
    return NULL;
}

/**
 Callback for all non-widget menu items.
 */
static void cbf(Fl_Widget *, void *v) {
  Fl_Type *t = NULL;
  if (Fl_Type::current && Fl_Type::current->can_have_children())
    t = ((Fl_Type*)v)->make(kAddAsLastChild);
  else
    t = ((Fl_Type*)v)->make(kAddAfterCurrent);
  select_only(t);
}

/**
 Callback for all widget menu items.
 */
static void cb(Fl_Widget *, void *v) {
  Fl_Type *t = NULL;
  if (Fl_Type::current && Fl_Type::current->can_have_children())
    t = add_new_widget_from_user((Fl_Type*)v, kAddAsLastChild);
  else
    t = add_new_widget_from_user((Fl_Type*)v, kAddAfterCurrent);
  select_only(t);
}

Fl_Menu_Item New_Menu[] = {
{"Code",0,0,0,FL_SUBMENU},
  {"Function/Method",0,cbf,(void*)&Fl_Function_type},
  {"Code",0,cbf,(void*)&Fl_Code_type},
  {"Code Block",0,cbf,(void*)&Fl_CodeBlock_type},
  {"Declaration",0,cbf,(void*)&Fl_Decl_type},
  {"Declaration Block",0,cbf,(void*)&Fl_DeclBlock_type},
  {"Class",0,cbf,(void*)&Fl_Class_type},
  {"Widget Class",0,cb,(void*)&Fl_Widget_Class_type},
  {"Comment",0,cbf,(void*)&Fl_Comment_type},
  {"Inlined Data",0,cbf,(void*)&Fl_Data_type},
{0},
{"Group",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Window_type},
  {0,0,cb,(void*)&Fl_Group_type},
  {0,0,cb,(void*)&Fl_Pack_type},
  {0,0,cb,(void*)&Fl_Flex_type},
  {0,0,cb,(void*)&Fl_Tabs_type},
  {0,0,cb,(void*)&Fl_Scroll_type},
  {0,0,cb,(void*)&Fl_Tile_type},
  {0,0,cb,(void*)&Fl_Wizard_type},
  {0,0,cb,(void*)&Fl_Grid_type},
{0},
{"Buttons",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Button_type},
  {0,0,cb,(void*)&Fl_Return_Button_type},
  {0,0,cb,(void*)&Fl_Light_Button_type},
  {0,0,cb,(void*)&Fl_Check_Button_type},
  {0,0,cb,(void*)&Fl_Repeat_Button_type},
  {0,0,cb,(void*)&Fl_Round_Button_type},
{0},
{"Valuators",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Slider_type},
  {0,0,cb,(void*)&Fl_Scrollbar_type},
  {0,0,cb,(void*)&Fl_Value_Slider_type},
  {0,0,cb,(void*)&Fl_Adjuster_type},
  {0,0,cb,(void*)&Fl_Counter_type},
  {0,0,cb,(void*)&Fl_Spinner_type},
  {0,0,cb,(void*)&Fl_Dial_type},
  {0,0,cb,(void*)&Fl_Roller_type},
  {0,0,cb,(void*)&Fl_Value_Input_type},
  {0,0,cb,(void*)&Fl_Value_Output_type},
{0},
{"Text",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Input_type},
  {0,0,cb,(void*)&Fl_Output_type},
  {0,0,cb,(void*)&Fl_Text_Editor_type},
  {0,0,cb,(void*)&Fl_Text_Display_type},
  {0,0,cb,(void*)&Fl_File_Input_type},
  {0,0,cb,(void*)&Fl_Terminal_type},
{0},
{"Menus",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Menu_Bar_type},
  {0,0,cb,(void*)&Fl_Menu_Button_type},
  {0,0,cb,(void*)&Fl_Choice_type},
  {0,0,cb,(void*)&Fl_Input_Choice_type},
  {0,0,cb, (void*)&Fl_Submenu_type},
  {0,0,cb, (void*)&Fl_Menu_Item_type},
  {"Checkbox Menu Item",0,cb, (void*)&Fl_Checkbox_Menu_Item_type},
  {"Radio Menu Item",0,cb, (void*)&Fl_Radio_Menu_Item_type},
{0},
{"Browsers",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Browser_type},
  {0,0,cb,(void*)&Fl_Check_Browser_type},
  {0,0,cb,(void*)&Fl_File_Browser_type},
  {0,0,cb,(void*)&Fl_Tree_type},
  {0,0,cb,(void*)&Fl_Help_View_type},
  {0,0,cb,(void*)&Fl_Table_type},
{0},
{"Other",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Box_type},
  {0,0,cb,(void*)&Fl_Clock_type},
  {0,0,cb,(void*)&Fl_Progress_type},
{0},
{0}};

#include <FL/Fl_Multi_Label.H>

/**
 Modify a menuitem to display an icon in front of the label.
 This is implemented using Fl_Multi_Label as the labeltype (FL_MULTI_LABEL).
 The icon may be null. If ic is null only the text is assigned
 to the label and Fl_Multi_Label is not used.
 \param[in] mi pointer to tme menu item that will be modified
 \param[in] ic icon for the menu, may be NULL
 \param[in] txt new label text, may *not* be NULL, will not be copied
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

void fill_in_New_Menu() {
  for (unsigned i = 0; i < sizeof(New_Menu)/sizeof(*New_Menu); i++) {
    Fl_Menu_Item *m = New_Menu+i;
    if (m->user_data()) {
      Fl_Type *t = (Fl_Type*)m->user_data();
      if (m->text) {
        make_iconlabel( m, pixmap[t->id()], m->label() );
      } else {
        const char *n = t->type_name();
        if (!strncmp(n,"Fl_",3)) n += 3;
        if (!strncmp(n,"fltk::",6)) n += 6;
        make_iconlabel( m, pixmap[t->id()], n );
      }
    }
  }
}

/**
 Find the correct prototype for a given type name.
 \param[in] inName a C string that must match type_name() or alt_type_name() of
    one of the known Fl_Type classes.
 \return the matching prototype or NULL
 */
Fl_Type *typename_to_prototype(const char *inName)
{
  if (inName==NULL || *inName==0)
    return NULL;
  for (unsigned i = 0; i < sizeof(known_types)/sizeof(*known_types); i++) {
    Fl_Type *prototype = known_types[i];
    if (fl_ascii_strcasecmp(inName, prototype->type_name())==0)
      return prototype;
    if (fl_ascii_strcasecmp(inName, prototype->alt_type_name())==0)
      return prototype;
  }
  return NULL;
}

/**
 Create and add a new type node to the widget tree.

 This is used by the .fl file reader. New types are always created as
 the last child of the first compatible parent. New widgets have a default
 setup. Their position, size and label will be read next in the file.

 \param[in] inName a C string that described the type we want
 \param[in] strategy add after current or as last child
 \return the type node that was created or NULL
 \see add_new_widget_from_file(const char*, int)
 add_new_widget_from_user(Fl_Type*, int)
 add_new_widget_from_user(const char*, int)
*/
Fl_Type *add_new_widget_from_file(const char *inName, Strategy strategy) {
  reading_file = 1; // makes labels be null
  Fl_Type *prototype = typename_to_prototype(inName);
  if (!prototype)
    return NULL;
  Fl_Type *new_node = prototype->make(strategy);
  reading_file = 0;
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
