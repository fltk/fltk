//
// "$Id$"
//
// Widget factory code for the Fast Light Tool Kit (FLTK).
//
// Type classes for most of the fltk widgets.  Most of the work
// is done by code in Fl_Widget_Type.C.  Also a factory instance
// of each of these type classes.
//
// This file also contains the "new" menu, which has a pointer
// to a factory instance for every class (both the ones defined
// here and ones in other files)
//
// Copyright 1998-2010 by Bill Spitzak and others.
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
#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Tree.H>
#include <stdio.h>
#include "../src/flstring.h"
#include "undo.h"

#include "Fl_Widget_Type.h"

extern Fl_Pixmap *pixmap[];

////////////////////////////////////////////////////////////////

#include <FL/Fl_Box.H>
class Fl_Box_Type : public Fl_Widget_Type {
public:
  virtual const char *type_name() {return "Fl_Box";}
  virtual const char *alt_type_name() {return "fltk::Widget";}
  Fl_Widget *widget(int x,int y,int w, int h) {
    return new Fl_Box(x,y,w,h,"label");}
  Fl_Widget_Type *_make() {return new Fl_Box_Type();}
  int pixmapID() { return 5; }
};
static Fl_Box_Type Fl_Box_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Button.H>
static Fl_Menu_Item buttontype_menu[] = {
  {"Normal",0,0,(void*)0},
  {"Toggle",0,0,(void*)FL_TOGGLE_BUTTON},
  {"Radio",0,0,(void*)FL_RADIO_BUTTON},
  {0}};
class Fl_Button_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return buttontype_menu;}
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Widget_Type::ideal_size(w, h);
    w += 2 * (o->labelsize() - 4);
    h = (h / 5) * 5;
  }
  virtual const char *type_name() {return "Fl_Button";}
  virtual const char *alt_type_name() {return "fltk::Button";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Button(x,y,w,h,"button");}
  Fl_Widget_Type *_make() {return new Fl_Button_Type();}
  int is_button() const {return 1;}
  int pixmapID() { return 2; }
};
static Fl_Button_Type Fl_Button_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Return_Button.H>
class Fl_Return_Button_Type : public Fl_Button_Type {
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Button_Type::ideal_size(w, h);
    int W = o->h();
    if (o->w()/3 < W) W = o->w()/3;
    w += W + 8 - o->labelsize();
  }
  virtual const char *type_name() {return "Fl_Return_Button";}
  virtual const char *alt_type_name() {return "fltk::ReturnButton";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Return_Button(x,y,w,h,"button");}
  Fl_Widget_Type *_make() {return new Fl_Return_Button_Type();}
  int pixmapID() { return 23; }
};
static Fl_Return_Button_Type Fl_Return_Button_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Repeat_Button.H>
class Fl_Repeat_Button_Type : public Fl_Widget_Type {
public:
  virtual const char *type_name() {return "Fl_Repeat_Button";}
  virtual const char *alt_type_name() {return "fltk::RepeatButton";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Repeat_Button(x,y,w,h,"button");}
  Fl_Widget_Type *_make() {return new Fl_Repeat_Button_Type();}
  int pixmapID() { return 25; }
};
static Fl_Repeat_Button_Type Fl_Repeat_Button_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Light_Button.H>
class Fl_Light_Button_Type : public Fl_Button_Type {
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Button_Type::ideal_size(w, h);
    w += 4;
  }
  virtual const char *type_name() {return "Fl_Light_Button";}
  virtual const char *alt_type_name() {return "fltk::LightButton";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Light_Button(x,y,w,h,"button");}
  Fl_Widget_Type *_make() {return new Fl_Light_Button_Type();}
  int pixmapID() { return 24; }
};
static Fl_Light_Button_Type Fl_Light_Button_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Check_Button.H>
class Fl_Check_Button_Type : public Fl_Button_Type {
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Button_Type::ideal_size(w, h);
    w += 4;
  }
  virtual const char *type_name() {return "Fl_Check_Button";}
  virtual const char *alt_type_name() {return "fltk::CheckButton";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Check_Button(x,y,w,h,"button");}
  Fl_Widget_Type *_make() {return new Fl_Check_Button_Type();}
  int pixmapID() { return 3; }
};
static Fl_Check_Button_Type Fl_Check_Button_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Round_Button.H>
class Fl_Round_Button_Type : public Fl_Button_Type {
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Button_Type::ideal_size(w, h);
    w += 4;
  }
  virtual const char *type_name() {return "Fl_Round_Button";}
  virtual const char *alt_type_name() {return "fltk::RadioButton";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Round_Button(x,y,w,h,"button");}
  Fl_Widget_Type *_make() {return new Fl_Round_Button_Type();}
  int pixmapID() { return 4; }
};
static Fl_Round_Button_Type Fl_Round_Button_type;

////////////////////////////////////////////////////////////////

extern int batch_mode;

#include <FL/Fl_Browser.H>
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_File_Browser.H>

static Fl_Menu_Item browser_type_menu[] = {
  {"No Select",0,0,(void*)FL_NORMAL_BROWSER},
  {"Select",0,0,(void*)FL_SELECT_BROWSER},
  {"Hold",0,0,(void*)FL_HOLD_BROWSER},
  {"Multi",0,0,(void*)FL_MULTI_BROWSER},
  {0}};
class Fl_Browser_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return browser_type_menu;}
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Browser *myo = (Fl_Browser *)o;
    fl_font(myo->textfont(), myo->textsize());
    h -= Fl::box_dh(o->box());
    w -= Fl::box_dw(o->box());
    int ww = (int)fl_width('m');
    w = ((w + ww - 1) / ww) * ww + Fl::box_dw(o->box());
    h = ((h + fl_height() - 1) / fl_height()) * fl_height() +
        Fl::box_dh(o->box());
    if (h < 30) h = 30;
    if (w < 50) w = 50;
  }
  virtual const char *type_name() {return "Fl_Browser";}
  virtual const char *alt_type_name() {return "fltk::Browser";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Browser* b = new Fl_Browser(x,y,w,h);
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
  Fl_Widget_Type *_make() {return new Fl_Browser_Type();}
  int pixmapID() { return 31; }
};
static Fl_Browser_Type Fl_Browser_type;

int Fl_Browser_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
  Fl_Browser *myo = (Fl_Browser*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
  switch (w) {
    case 4:
    case 0: f = myo->textfont(); s = myo->textsize(); c = myo->textcolor(); break;
    case 1: myo->textfont(f); break;
    case 2: myo->textsize(s); break;
    case 3: myo->textcolor(c); break;
  }
  return 1;
}

class Fl_Check_Browser_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return browser_type_menu;}
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Check_Browser *myo = (Fl_Check_Browser *)o;
    fl_font(myo->textfont(), myo->textsize());
    h -= Fl::box_dh(o->box());
    w -= Fl::box_dw(o->box()) - fl_height();
    int ww = (int)fl_width('m');
    w = ((w + ww - 1) / ww) * ww + Fl::box_dw(o->box());
    h = ((h + fl_height() - 1) / fl_height()) * fl_height() +
        Fl::box_dh(o->box());
    if (h < 30) h = 30;
    if (w < 50) w = 50;
  }
  virtual const char *type_name() {return "Fl_Check_Browser";}
  virtual const char *alt_type_name() {return "fltk::CheckBrowser";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Check_Browser* b = new Fl_Check_Browser(x,y,w,h);
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
  Fl_Widget_Type *_make() {return new Fl_Check_Browser_Type();}
  int pixmapID() { return 32; }
};
static Fl_Check_Browser_Type Fl_Check_Browser_type;

int Fl_Check_Browser_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
  Fl_Check_Browser *myo = (Fl_Check_Browser*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
  switch (w) {
    case 4:
    case 0: f = myo->textfont(); s = myo->textsize(); c = myo->textcolor(); break;
    case 1: myo->textfont(f); break;
    case 2: myo->textsize(s); break;
    case 3: myo->textcolor(c); break;
  }
  return 1;
}

class Fl_Tree_Type : public Fl_Widget_Type {
public:
  virtual void ideal_size(int &w, int &h) {
    if (h < 60) h = 60;
    if (w < 80) w = 80;
  }
  virtual const char *type_name() {return "Fl_Tree";}
  virtual const char *alt_type_name() {return "fltk::TreeBrowser";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Tree* b = new Fl_Tree(x,y,w,h);
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
  Fl_Widget_Type *_make() {return new Fl_Tree_Type();}
  int pixmapID() { return 50; }
};
static Fl_Tree_Type Fl_Tree_type;

class Fl_File_Browser_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return browser_type_menu;}
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_File_Browser *myo = (Fl_File_Browser *)o;
    fl_font(myo->textfont(), myo->textsize());
    h -= Fl::box_dh(o->box());
    w -= Fl::box_dw(o->box()) + fl_height();
    int ww = (int)fl_width('m');
    w = ((w + ww - 1) / ww) * ww + Fl::box_dw(o->box());
    h = ((h + fl_height() - 1) / fl_height()) * fl_height() +
        Fl::box_dh(o->box());
    if (h < 30) h = 30;
    if (w < 50) w = 50;
  }
  virtual const char *type_name() {return "Fl_File_Browser";}
  virtual const char *alt_type_name() {return "fltk::FileBrowser";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_File_Browser* b = new Fl_File_Browser(x,y,w,h);
    // Fl_File_Browser::add calls fl_height(), which requires the X display open.
    // Avoid this when compiling so it works w/o a display:
    if (!batch_mode) {
      b->load(".");
    }
    return b;
  }
  Fl_Widget_Type *_make() {return new Fl_File_Browser_Type();}
  int pixmapID() { return 33; }
};
static Fl_File_Browser_Type Fl_File_Browser_type;

int Fl_File_Browser_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
  Fl_File_Browser *myo = (Fl_File_Browser*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
  switch (w) {
    case 4:
    case 0: f = myo->textfont(); s = myo->textsize(); c = myo->textcolor(); break;
    case 1: myo->textfont(f); break;
    case 2: myo->textsize(s); break;
    case 3: myo->textcolor(c); break;
  }
  return 1;
}

////////////////////////////////////////////////////////////////

#include <FL/Fl_Counter.H>
static Fl_Menu_Item counter_type_menu[] = {
  {"Normal",0,0,(void*)FL_NORMAL_COUNTER},
  {"Simple",0,0,(void*)FL_SIMPLE_COUNTER},
  {0}};
class Fl_Counter_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return counter_type_menu;}
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
  int is_valuator() const {return 1;}
  int pixmapID() { return 41; }
public:
  virtual const char *type_name() {return "Fl_Counter";}
  virtual const char *alt_type_name() {return "fltk::Counter";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Counter(x,y,w,h,"counter:");}
  Fl_Widget_Type *_make() {return new Fl_Counter_Type();}
};
static Fl_Counter_Type Fl_Counter_type;

int Fl_Counter_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
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

////////////////////////////////////////////////////////////////

#include <FL/Fl_Spinner.H>
static Fl_Menu_Item spinner_type_menu[] = {
  {"Integer",0,0,(void*)FL_INT_INPUT},
  {"Float",  0,0,(void*)FL_FLOAT_INPUT},
  {0}};
class Fl_Spinner_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return spinner_type_menu;}
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
  int pixmapID() { return 47; }
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Spinner *myo = (Fl_Spinner *)o;
    fl_font(myo->textfont(), myo->textsize());
    h = fl_height() + myo->textsize() - 6;
    if (h < 15) h = 15;
    w -= Fl::box_dw(o->box());
    int ww = (int)fl_width('m');
    w = ((w + ww - 1) / ww) * ww + Fl::box_dw(o->box()) + h / 2;
    if (w < 40) w = 40	;
  }
  virtual const char *type_name() {return "Fl_Spinner";}
  virtual const char *alt_type_name() {return "fltk::Spinner";}
  int is_spinner() const { return 1; }
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Spinner(x,y,w,h,"spinner:");}
  Fl_Widget_Type *_make() {return new Fl_Spinner_Type();}
};
static Fl_Spinner_Type Fl_Spinner_type;

int Fl_Spinner_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
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

////////////////////////////////////////////////////////////////

#include <FL/Fl_Input.H>
static Fl_Menu_Item input_type_menu[] = {
  {"Normal",0,0,(void*)FL_NORMAL_INPUT},
  {"Multiline",0,0,(void*)FL_MULTILINE_INPUT},
  {"Secret",0,0,(void*)FL_SECRET_INPUT},
  {"Int",0,0,(void*)FL_INT_INPUT},
  {"Float",0,0,(void*)FL_FLOAT_INPUT},
  {0}};
class Fl_Input_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return input_type_menu;}
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Input *myo = (Fl_Input *)o;
    fl_font(myo->textfont(), myo->textsize());
    h = fl_height() + myo->textsize() - 6;
    w -= Fl::box_dw(o->box());
    int ww = (int)fl_width('m');
    w = ((w + ww - 1) / ww) * ww + Fl::box_dw(o->box());
    if (h < 15) h = 15;
    if (w < 15) w = 15;
  }
  virtual const char *type_name() {return "Fl_Input";}
  virtual const char *alt_type_name() {return "fltk::Input";}
  int is_input() const {return 1;}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Input *myo = new Fl_Input(x,y,w,h,"input:");
    myo->value("Text Input");
    return myo;
  }
  Fl_Widget_Type *_make() {return new Fl_Input_Type();}
  int pixmapID() { return 14; }
  virtual void copy_properties() {
    Fl_Widget_Type::copy_properties();
    Fl_Input_ *d = (Fl_Input_*)live_widget, *s = (Fl_Input_*)o;
    d->textfont(s->textfont());
    d->textsize(s->textsize());
    d->textcolor(s->textcolor());
    d->shortcut(s->shortcut());
  }
};
static Fl_Input_Type Fl_Input_type;

int Fl_Input_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
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

////////////////////////////////////////////////////////////////

#include <FL/Fl_File_Input.H>
class Fl_File_Input_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return 0;}
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_File_Input *myo = (Fl_File_Input *)o;
    fl_font(myo->textfont(), myo->textsize());
    h = fl_height() + myo->textsize() + 4;
    w -= Fl::box_dw(o->box());
    int ww = (int)fl_width('m');
    w = ((w + ww - 1) / ww) * ww + Fl::box_dw(o->box());
    if (h < 20) h = 20;
    if (w < 50) w = 50;
  }
  virtual const char *type_name() {return "Fl_File_Input";}
  virtual const char *alt_type_name() {return "fltk::FileInput";}
  int is_input() const {return 1;}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_File_Input *myo = new Fl_File_Input(x,y,w,h,"file:");
    myo->value("/now/is/the/time/for/a/filename.ext");
    return myo;
  }
  Fl_Widget_Type *_make() {return new Fl_File_Input_Type();}
  int pixmapID() { return 30; }
};
static Fl_File_Input_Type Fl_File_Input_type;

int Fl_File_Input_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
  Fl_File_Input *myo = (Fl_File_Input*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
  switch (w) {
    case 4:
    case 0: f = myo->textfont(); s = myo->textsize(); c = myo->textcolor(); break;
    case 1: myo->textfont(f); break;
    case 2: myo->textsize(s); break;
    case 3: myo->textcolor(c); break;
  }
  return 1;
}

////////////////////////////////////////////////////////////////

#include <FL/Fl_Text_Display.H>
class Fl_Text_Display_Type : public Fl_Widget_Type {
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Text_Display *myo = (Fl_Text_Display *)o;
    fl_font(myo->textfont(), myo->textsize());
    h -= Fl::box_dh(o->box());
    w -= Fl::box_dw(o->box());
    int ww = (int)fl_width('m');
    w = ((w + ww - 1) / ww) * ww + Fl::box_dw(o->box());
    h = ((h + fl_height() - 1) / fl_height()) * fl_height() +
        Fl::box_dh(o->box());
    if (h < 30) h = 30;
    if (w < 50) w = 50;
  }
  virtual const char *type_name() {return "Fl_Text_Display";}
  virtual const char *alt_type_name() {return "fltk::TextDisplay";}
  int is_text_display() const {return 1;}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Text_Display *myo = new Fl_Text_Display(x,y,w,h);
    return myo;
  }
  Fl_Widget_Type *_make() {return new Fl_Text_Display_Type();}
  int pixmapID() { return 28; }
};
static Fl_Text_Display_Type Fl_Text_Display_type;

int Fl_Text_Display_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
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

////////////////////////////////////////////////////////////////

#include <FL/Fl_Text_Editor.H>
class Fl_Text_Editor_Type : public Fl_Widget_Type {
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Text_Editor *myo = (Fl_Text_Editor *)o;
    fl_font(myo->textfont(), myo->textsize());
    h -= Fl::box_dh(o->box());
    w -= Fl::box_dw(o->box());
    int ww = (int)fl_width('m');
    w = ((w + ww - 1) / ww) * ww + Fl::box_dw(o->box());
    h = ((h + fl_height() - 1) / fl_height()) * fl_height() +
        Fl::box_dh(o->box());
    if (h < 30) h = 30;
    if (w < 50) w = 50;
  }
  virtual const char *type_name() {return "Fl_Text_Editor";}
  virtual const char *alt_type_name() {return "fltk::TextEditor";}
  int is_text_display() const {return 1;}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Text_Editor *myo = new Fl_Text_Editor(x,y,w,h);
    return myo;
  }
  Fl_Widget_Type *_make() {return new Fl_Text_Editor_Type();}
  int pixmapID() { return 29; }
};
static Fl_Text_Editor_Type Fl_Text_Editor_type;

int Fl_Text_Editor_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
  Fl_Text_Editor *myo = (Fl_Text_Editor*)(w==4 ? ((Fl_Widget_Type*)factory)->o : o);
  switch (w) {
    case 4:
    case 0: f = myo->textfont(); s = myo->textsize(); c = myo->textcolor(); break;
    case 1: myo->textfont(f); break;
    case 2: myo->textsize(s); break;
    case 3: myo->textcolor(c); break;
  }
  return 1;
}

////////////////////////////////////////////////////////////////

#include <FL/Fl_Clock.H>
class Fl_Clock_Type : public Fl_Widget_Type {
public:
  virtual const char *type_name() {return "Fl_Clock";}
  virtual const char *alt_type_name() {return "fltk::Clock";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Clock(x,y,w,h);}
  Fl_Widget_Type *_make() {return new Fl_Clock_Type();}
  int pixmapID() { return 34; }
};
static Fl_Clock_Type Fl_Clock_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Help_View.H>
class Fl_Help_View_Type : public Fl_Widget_Type {
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Help_View *myo = (Fl_Help_View *)o;
    fl_font(myo->textfont(), myo->textsize());
    h -= Fl::box_dh(o->box());
    w -= Fl::box_dw(o->box());
    int ww = (int)fl_width('m');
    w = ((w + ww - 1) / ww) * ww + Fl::box_dw(o->box());
    h = ((h + fl_height() - 1) / fl_height()) * fl_height() +
        Fl::box_dh(o->box());
    if (h < 30) h = 30;
    if (w < 50) w = 50;
  }
  virtual const char *type_name() {return "Fl_Help_View";}
  virtual const char *alt_type_name() {return "fltk::HelpView";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Help_View *myo = new Fl_Help_View(x,y,w,h);
    if (!batch_mode) {
      myo->value("<HTML><BODY><H1>Fl_Help_View Widget</H1>"
                 "<P>This is a Fl_Help_View widget.</P></BODY></HTML>");
    }
    return myo;}
  Fl_Widget_Type *_make() {return new Fl_Help_View_Type();}
  int pixmapID() { return 35; }
};
static Fl_Help_View_Type Fl_Help_View_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Progress.H>
class Fl_Progress_Type : public Fl_Widget_Type {
public:
  virtual const char *type_name() {return "Fl_Progress";}
  virtual const char *alt_type_name() {return "fltk::ProgressBar";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Progress *myo = new Fl_Progress(x,y,w,h,"label");
    myo->value(50);
    return myo;}
  Fl_Widget_Type *_make() {return new Fl_Progress_Type();}
  int pixmapID() { return 36; }
};
static Fl_Progress_Type Fl_Progress_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Adjuster.H>
class Fl_Adjuster_Type : public Fl_Widget_Type {
  int is_valuator() const {return 1;}
public:
  virtual const char *type_name() {return "Fl_Adjuster";}
  virtual const char *alt_type_name() {return "fltk::Adjuster";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Adjuster(x,y,w,h);}
  Fl_Widget_Type *_make() {return new Fl_Adjuster_Type();}
  int pixmapID() { return 40; }
};
static Fl_Adjuster_Type Fl_Adjuster_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Dial.H>
static Fl_Menu_Item dial_type_menu[] = {
  {"Dot",0,0,(void*)0},
  {"Line",0,0,(void*)FL_LINE_DIAL},
  {"Fill",0,0,(void*)FL_FILL_DIAL},
  {0}};
class Fl_Dial_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return dial_type_menu;}
  int is_valuator() const {return 1;}
public:
  virtual const char *type_name() {return "Fl_Dial";}
  virtual const char *alt_type_name() {return "fltk::Dial";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Dial(x,y,w,h);}
  Fl_Widget_Type *_make() {return new Fl_Dial_Type();}
  int pixmapID() { return 42; }
};
static Fl_Dial_Type Fl_Dial_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Roller.H>
static Fl_Menu_Item roller_type_menu[] = {
  {"Vertical",0,0,(void*)0},
  {"Horizontal",0,0,(void*)FL_HORIZONTAL},
  {0}};
class Fl_Roller_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return roller_type_menu;}
  int is_valuator() const {return 1;}
public:
  virtual const char *type_name() {return "Fl_Roller";}
  virtual const char *alt_type_name() {return "fltk::Roller";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Roller(x,y,w,h);}
  Fl_Widget_Type *_make() {return new Fl_Roller_Type();}
  int pixmapID() { return 43; }
};
static Fl_Roller_Type Fl_Roller_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Scrollbar.H>
static Fl_Menu_Item slider_type_menu[] = {
  {"Vertical",0,0,(void*)FL_VERT_SLIDER},
  {"Horizontal",0,0,(void*)FL_HOR_SLIDER},
  {"Vert Fill",0,0,(void*)FL_VERT_FILL_SLIDER},
  {"Horz Fill",0,0,(void*)FL_HOR_FILL_SLIDER},
  {"Vert Knob",0,0,(void*)FL_VERT_NICE_SLIDER},
  {"Horz Knob",0,0,(void*)FL_HOR_NICE_SLIDER},
  {0}};
class Fl_Slider_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return slider_type_menu;}
  int is_valuator() const {return 2;}
public:
  virtual const char *type_name() {return "Fl_Slider";}
  virtual const char *alt_type_name() {return "fltk::Slider";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Slider(x,y,w,h,"slider:");}
  Fl_Widget_Type *_make() {return new Fl_Slider_Type();}
  int pixmapID() { return 37; }
};
static Fl_Slider_Type Fl_Slider_type;

static Fl_Menu_Item scrollbar_type_menu[] = {
  {"Vertical",0,0,(void*)FL_VERT_SLIDER},
  {"Horizontal",0,0,(void*)FL_HOR_SLIDER},
  {0}};
class Fl_Scrollbar_Type : public Fl_Slider_Type {
  Fl_Menu_Item *subtypes() {return scrollbar_type_menu;}
  int is_valuator() const {return 3;}
public:
  virtual const char *type_name() {return "Fl_Scrollbar";}
  virtual const char *alt_type_name() {return "fltk::Scrollbar";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Scrollbar(x,y,w,h);}
  Fl_Widget_Type *_make() {return new Fl_Scrollbar_Type();}
  int pixmapID() { return 38; }
};
static Fl_Scrollbar_Type Fl_Scrollbar_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Output.H>
static Fl_Menu_Item output_type_menu[] = {
  {"Normal",0,0,(void*)FL_NORMAL_OUTPUT},
  {"Multiline",0,0,(void*)FL_MULTILINE_OUTPUT},
  {0}};
class Fl_Output_Type : public Fl_Input_Type {
  Fl_Menu_Item *subtypes() {return output_type_menu;}
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Output *myo = (Fl_Output *)o;
    fl_font(myo->textfont(), myo->textsize());
    h = fl_height() + myo->textsize() - 6;
    w -= Fl::box_dw(o->box());
    int ww = (int)fl_width('m');
    w = ((w + ww - 1) / ww) * ww + Fl::box_dw(o->box());
    if (h < 15) h = 15;
    if (w < 15) w = 15;
  }
  virtual const char *type_name() {return "Fl_Output";}
  virtual const char *alt_type_name() {return "fltk::Output";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Output *myo = new Fl_Output(x,y,w,h,"output:");
    myo->value("Text Output");
    return myo;
  }
  Fl_Widget_Type *_make() {return new Fl_Output_Type();}
  int pixmapID() { return 27; }
};
static Fl_Output_Type Fl_Output_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Value_Input.H>
class Fl_Value_Input_Type : public Fl_Widget_Type {
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Value_Input *myo = (Fl_Value_Input *)o;
    fl_font(myo->textfont(), myo->textsize());
    h = fl_height() + myo->textsize() - 6;
    w -= Fl::box_dw(o->box());
    int ww = (int)fl_width('m');
    w = ((w + ww - 1) / ww) * ww + Fl::box_dw(o->box());
    if (h < 15) h = 15;
    if (w < 15) w = 15;
  }
  virtual const char *type_name() {return "Fl_Value_Input";}
  virtual const char *alt_type_name() {return "fltk::ValueInput";}
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
  int is_valuator() const {return 1;}
  int is_value_input() const {return 1;}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Value_Input *myo = new Fl_Value_Input(x,y,w,h,"value:");
    return myo;
  }
  Fl_Widget_Type *_make() {return new Fl_Value_Input_Type();}
  int pixmapID() { return 44; }
};
static Fl_Value_Input_Type Fl_Value_Input_type;

int Fl_Value_Input_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
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

////////////////////////////////////////////////////////////////

#include <FL/Fl_Value_Output.H>
class Fl_Value_Output_Type : public Fl_Widget_Type {
public:
  virtual void ideal_size(int &w, int &h) {
    Fl_Value_Output *myo = (Fl_Value_Output *)o;
    fl_font(myo->textfont(), myo->textsize());
    h = fl_height() + myo->textsize() - 6;
    w = o->w() - Fl::box_dw(o->box());
    int ww = (int)fl_width('m');
    w = ((w + ww - 1) / ww) * ww + Fl::box_dw(o->box());
    if (h < 15) h = 15;
    if (w < 15) w = 15;
  }
  virtual const char *type_name() {return "Fl_Value_Output";}
  virtual const char *alt_type_name() {return "fltk::ValueOutput";}
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
  int is_valuator() const {return 1;}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Value_Output *myo = new Fl_Value_Output(x,y,w,h,"value:");
    return myo;
  }
  Fl_Widget_Type *_make() {return new Fl_Value_Output_Type();}
  int pixmapID() { return 45; }
};
static Fl_Value_Output_Type Fl_Value_Output_type;

int Fl_Value_Output_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
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

////////////////////////////////////////////////////////////////

#include <FL/Fl_Value_Slider.H>
class Fl_Value_Slider_Type : public Fl_Slider_Type {
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
public:
  virtual const char *type_name() {return "Fl_Value_Slider";}
  virtual const char *alt_type_name() {return "fltk::ValueSlider";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Value_Slider(x,y,w,h,"slider:");}
  Fl_Widget_Type *_make() {return new Fl_Value_Slider_Type();}
  int pixmapID() { return 39; }
};
static Fl_Value_Slider_Type Fl_Value_Slider_type;

int Fl_Value_Slider_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
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

////////////////////////////////////////////////////////////////

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
extern class Fl_Tabs_Type Fl_Tabs_type;
extern class Fl_Scroll_Type Fl_Scroll_type;
extern class Fl_Table_Type Fl_Table_type;
extern class Fl_Tile_Type Fl_Tile_type;
extern class Fl_Input_Choice_Type Fl_Input_Choice_type;
extern class Fl_Choice_Type Fl_Choice_type;
extern class Fl_Menu_Bar_Type Fl_Menu_Bar_type;
extern class Fl_Menu_Button_Type Fl_Menu_Button_type;
extern class Fl_Menu_Item_Type Fl_Menu_Item_type;
extern class Fl_Submenu_Type Fl_Submenu_type;
extern class Fl_Wizard_Type Fl_Wizard_type;

extern void select(Fl_Type *,int);
extern void select_only(Fl_Type *);

#include <FL/Fl_Window.H>

static void cb(Fl_Widget *, void *v) {
  undo_checkpoint();
  undo_suspend();
  Fl_Type *t = ((Fl_Type*)v)->make();
  if (t) {
    if (t->is_widget() && !t->is_window()) {
      Fl_Widget_Type *wt = (Fl_Widget_Type *)t;

      // Set font sizes...
      wt->o->labelsize(Fl_Widget_Type::default_size);

      Fl_Font f;
      int s = Fl_Widget_Type::default_size;
      Fl_Color c;

      wt->textstuff(2, f, s, c);

      // Resize and/or reposition new widget...
      int w = 0, h = 0;
      wt->ideal_size(w, h);

      if (!strcmp(wt->type_name(), "Fl_Menu_Bar")) {
        // Move and resize the menubar across the top of the window...
        wt->o->resize(0, 0, w, h);
      } else {
        // Just resize to the ideal size...
        wt->o->size(w, h);
      }
    }
    select_only(t);
    set_modflag(1);
    t->open();
  } else {
    undo_current --;
    undo_last --;
  }
  undo_resume();
}

Fl_Menu_Item New_Menu[] = {
{"Code",0,0,0,FL_SUBMENU},
  {"Function/Method",0,cb,(void*)&Fl_Function_type},
  {"Code",0,cb,(void*)&Fl_Code_type},
  {"Code Block",0,cb,(void*)&Fl_CodeBlock_type},
  {"Declaration",0,cb,(void*)&Fl_Decl_type},
  {"Declaration Block",0,cb,(void*)&Fl_DeclBlock_type},
  {"Class",0,cb,(void*)&Fl_Class_type},
  {"Widget Class",0,cb,(void*)&Fl_Widget_Class_type},
  {"Comment",0,cb,(void*)&Fl_Comment_type},
  {"Binary Data",0,cb,(void*)&Fl_Data_type},
{0},
{"Group",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Window_type},
  {0,0,cb,(void*)&Fl_Group_type},
  {0,0,cb,(void*)&Fl_Pack_type},
  {0,0,cb,(void*)&Fl_Tabs_type},
  {0,0,cb,(void*)&Fl_Scroll_type},
  {0,0,cb,(void*)&Fl_Table_type},
  {0,0,cb,(void*)&Fl_Tile_type},
  {0,0,cb,(void*)&Fl_Wizard_type},
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
  {0,0,cb,(void*)&Fl_File_Input_type},
  {0,0,cb,(void*)&Fl_Input_type},
  {0,0,cb,(void*)&Fl_Output_type},
  {0,0,cb,(void*)&Fl_Text_Display_type},
  {0,0,cb,(void*)&Fl_Text_Editor_type},
{0},
{"Menus",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Menu_Bar_type},
  {0,0,cb,(void*)&Fl_Menu_Button_type},
  {0,0,cb,(void*)&Fl_Choice_type},
  {0,0,cb,(void*)&Fl_Input_Choice_type},
  {0,0,cb, (void*)&Fl_Submenu_type},
  {0,0,cb, (void*)&Fl_Menu_Item_type},
{0},
{"Browsers",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Browser_type},
  {0,0,cb,(void*)&Fl_Check_Browser_type},
  {0,0,cb,(void*)&Fl_File_Browser_type},
  {0,0,cb,(void*)&Fl_Tree_type},
{0},
{"Other",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Box_type},
  {0,0,cb,(void*)&Fl_Clock_type},
  {0,0,cb,(void*)&Fl_Help_View_type},
  {0,0,cb,(void*)&Fl_Progress_type},
{0},
{0}};

#include <FL/Fl_Multi_Label.H>

// modify a menuitem to display an icon in front of the label
static void make_iconlabel( Fl_Menu_Item *mi, Fl_Image *ic, const char *txt )
{
  if (ic) {
    char *t1 = new char[strlen(txt)+6];
    strcpy( t1, " " );
    strcat(t1, txt);
    strcat(t1, "...");
    mi->image( ic );
    Fl_Multi_Label *ml = new Fl_Multi_Label;
    ml->labela = (char*)ic;
    ml->labelb = t1;
    ml->typea = _FL_IMAGE_LABEL;
    ml->typeb = FL_NORMAL_LABEL;
    ml->label( mi );
  }
  else if (txt!=mi->text)
    mi->label(txt);
}

void fill_in_New_Menu() {
  for (unsigned i = 0; i < sizeof(New_Menu)/sizeof(*New_Menu); i++) {
    Fl_Menu_Item *m = New_Menu+i;
    if (m->user_data()) {
      Fl_Type *t = (Fl_Type*)m->user_data();
      if (m->text) {
        make_iconlabel( m, pixmap[t->pixmapID()], m->label() );
      } else {
        const char *n = t->type_name();
        if (!strncmp(n,"Fl_",3)) n += 3;
        if (!strncmp(n,"fltk::",6)) n += 6;
        make_iconlabel( m, pixmap[t->pixmapID()], n );
      }
    }
  }
}

// use keyword to pick the type, this is used to parse files:
int reading_file;
Fl_Type *Fl_Type_make(const char *tn) {
  reading_file = 1; // makes labels be null
  Fl_Type *r = 0;
  for (unsigned i = 0; i < sizeof(New_Menu)/sizeof(*New_Menu); i++) {
    Fl_Menu_Item *m = New_Menu+i;
    if (!m->user_data()) continue;
    Fl_Type *t = (Fl_Type*)(m->user_data());
    if (!fl_ascii_strcasecmp(tn,t->type_name())) {r = t->make(); break;}
    if (!fl_ascii_strcasecmp(tn,t->alt_type_name())) {r = t->make(); break;}
  }
  reading_file = 0;
  return r;
}

////////////////////////////////////////////////////////////////

// Since I have included all the .H files, do this table here:
// This table is only used to read fdesign files:

struct symbol {const char *name; int value;};

static symbol table[] = {
  {"BLACK",	FL_BLACK},
  {"RED",	FL_RED},
  {"GREEN",	FL_GREEN},
  {"YELLOW",	FL_YELLOW},
  {"BLUE",	FL_BLUE},
  {"MAGENTA",	FL_MAGENTA},
  {"CYAN",	FL_CYAN},
  {"WHITE",	FL_WHITE},

  {"LCOL",		 FL_BLACK},
  {"COL1",		 FL_GRAY},
  {"MCOL",		 FL_LIGHT1},
  {"LEFT_BCOL",		 FL_LIGHT3},
  {"TOP_BCOL",		 FL_LIGHT2},
  {"BOTTOM_BCOL",	 FL_DARK2},
  {"RIGHT_BCOL",		 FL_DARK3},
  {"INACTIVE",		 FL_INACTIVE_COLOR},
  {"INACTIVE_COL",	 FL_INACTIVE_COLOR},
  {"FREE_COL1",		 FL_FREE_COLOR},
  {"FREE_COL2",		 FL_FREE_COLOR+1},
  {"FREE_COL3",		 FL_FREE_COLOR+2},
  {"FREE_COL4",		 FL_FREE_COLOR+3},
  {"FREE_COL5",		 FL_FREE_COLOR+4},
  {"FREE_COL6",		 FL_FREE_COLOR+5},
  {"FREE_COL7",		 FL_FREE_COLOR+6},
  {"FREE_COL8",		 FL_FREE_COLOR+7},
  {"FREE_COL9",		 FL_FREE_COLOR+8},
  {"FREE_COL10",		 FL_FREE_COLOR+9},
  {"FREE_COL11",		 FL_FREE_COLOR+10},
  {"FREE_COL12",		 FL_FREE_COLOR+11},
  {"FREE_COL13",		 FL_FREE_COLOR+12},
  {"FREE_COL14",		 FL_FREE_COLOR+13},
  {"FREE_COL15",		 FL_FREE_COLOR+14},
  {"FREE_COL16",		 FL_FREE_COLOR+15},
  {"TOMATO",		 131},
  {"INDIANRED",		 164},
  {"SLATEBLUE",		 195},
  {"DARKGOLD",		 84},
  {"PALEGREEN",		 157},
  {"ORCHID",		 203},
  {"DARKCYAN",		 189},
  {"DARKTOMATO",		 113},
  {"WHEAT",		 174},
  {"ALIGN_CENTER",	FL_ALIGN_CENTER},
  {"ALIGN_TOP",		FL_ALIGN_TOP},
  {"ALIGN_BOTTOM",	FL_ALIGN_BOTTOM},
  {"ALIGN_LEFT",	FL_ALIGN_LEFT},
  {"ALIGN_RIGHT",	FL_ALIGN_RIGHT},
  {"ALIGN_INSIDE",	FL_ALIGN_INSIDE},
  {"ALIGN_TOP_LEFT",	 FL_ALIGN_TOP | FL_ALIGN_LEFT},
  {"ALIGN_TOP_RIGHT",	 FL_ALIGN_TOP | FL_ALIGN_RIGHT},
  {"ALIGN_BOTTOM_LEFT",	 FL_ALIGN_BOTTOM | FL_ALIGN_LEFT},
  {"ALIGN_BOTTOM_RIGHT", FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT},
  {"ALIGN_CENTER|FL_ALIGN_INSIDE",	FL_ALIGN_CENTER|FL_ALIGN_INSIDE},
  {"ALIGN_TOP|FL_ALIGN_INSIDE",		FL_ALIGN_TOP|FL_ALIGN_INSIDE},
  {"ALIGN_BOTTOM|FL_ALIGN_INSIDE",	FL_ALIGN_BOTTOM|FL_ALIGN_INSIDE},
  {"ALIGN_LEFT|FL_ALIGN_INSIDE",	FL_ALIGN_LEFT|FL_ALIGN_INSIDE},
  {"ALIGN_RIGHT|FL_ALIGN_INSIDE",	FL_ALIGN_RIGHT|FL_ALIGN_INSIDE},
  {"ALIGN_INSIDE|FL_ALIGN_INSIDE",	FL_ALIGN_INSIDE|FL_ALIGN_INSIDE},
  {"ALIGN_TOP_LEFT|FL_ALIGN_INSIDE",	FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_INSIDE},
  {"ALIGN_TOP_RIGHT|FL_ALIGN_INSIDE",	FL_ALIGN_TOP|FL_ALIGN_RIGHT|FL_ALIGN_INSIDE},
  {"ALIGN_BOTTOM_LEFT|FL_ALIGN_INSIDE",	FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_INSIDE},
  {"ALIGN_BOTTOM_RIGHT|FL_ALIGN_INSIDE",FL_ALIGN_BOTTOM|FL_ALIGN_RIGHT|FL_ALIGN_INSIDE},

  {"ALIGN_LEFT_TOP",	 FL_ALIGN_TOP | FL_ALIGN_LEFT},
  {"ALIGN_RIGHT_TOP",	 FL_ALIGN_TOP | FL_ALIGN_RIGHT},
  {"ALIGN_LEFT_BOTTOM",	 FL_ALIGN_BOTTOM | FL_ALIGN_LEFT},
  {"ALIGN_RIGHT_BOTTOM", FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT},
  {"INVALID_STYLE",	 255},
  {"NORMAL_STYLE",	 FL_HELVETICA},
  {"BOLD_STYLE",		 FL_HELVETICA|FL_BOLD},
  {"ITALIC_STYLE",	 FL_HELVETICA|FL_ITALIC},
  {"BOLDITALIC_STYLE",	 FL_HELVETICA|FL_BOLD|FL_ITALIC},
  {"FIXED_STYLE",	 FL_COURIER},
  {"FIXEDBOLD_STYLE",	 FL_COURIER|FL_BOLD},
  {"FIXEDITALIC_STYLE",	 FL_COURIER|FL_ITALIC},
  {"FIXEDBOLDITALIC_STYLE",  FL_COURIER|FL_BOLD|FL_ITALIC},
  {"TIMES_STYLE",	 FL_TIMES},
  {"TIMESBOLD_STYLE",	 FL_TIMES|FL_BOLD},
  {"TIMESITALIC_STYLE",	 FL_TIMES|FL_ITALIC},
  {"TIMESBOLDITALIC_STYLE",  FL_TIMES|FL_BOLD|FL_ITALIC},
  {"SHADOW_STYLE",	(_FL_SHADOW_LABEL<<8)},
  {"ENGRAVED_STYLE",	(_FL_ENGRAVED_LABEL<<8)},
  {"EMBOSSED_STYLE",	(_FL_EMBOSSED_LABEL<<0)},
  {"TINY_SIZE",		 8},
  {"SMALL_SIZE",		 11},
  {"NORMAL_SIZE",	 FL_NORMAL_SIZE},
  {"MEDIUM_SIZE",	 18},
  {"LARGE_SIZE",		 24},
  {"HUGE_SIZE",		 32},
  {"DEFAULT_SIZE",	 FL_NORMAL_SIZE},
  {"TINY_FONT",		 8},
  {"SMALL_FONT",		 11},
  {"NORMAL_FONT",	 FL_NORMAL_SIZE},
  {"MEDIUM_FONT",	 18},
  {"LARGE_FONT",		 24},
  {"HUGE_FONT",		 32},
  {"NORMAL_FONT1",	 11},
  {"NORMAL_FONT2",	 FL_NORMAL_SIZE},
  {"DEFAULT_FONT",	 11},
  {"RETURN_END_CHANGED",  0},
  {"RETURN_CHANGED",	 1},
  {"RETURN_END",		 2},
  {"RETURN_ALWAYS",	 3},
  {"PUSH_BUTTON",	FL_TOGGLE_BUTTON},
  {"RADIO_BUTTON",	FL_RADIO_BUTTON},
  {"HIDDEN_BUTTON",	FL_HIDDEN_BUTTON},
  {"SELECT_BROWSER",	FL_SELECT_BROWSER},
  {"HOLD_BROWSER",	FL_HOLD_BROWSER},
  {"MULTI_BROWSER",	FL_MULTI_BROWSER},
  {"SIMPLE_COUNTER",	FL_SIMPLE_COUNTER},
  {"LINE_DIAL",		FL_LINE_DIAL},
  {"FILL_DIAL",		FL_FILL_DIAL},
  {"VERT_SLIDER",	FL_VERT_SLIDER},
  {"HOR_SLIDER",	FL_HOR_SLIDER},
  {"VERT_FILL_SLIDER",	FL_VERT_FILL_SLIDER},
  {"HOR_FILL_SLIDER",	FL_HOR_FILL_SLIDER},
  {"VERT_NICE_SLIDER",	FL_VERT_NICE_SLIDER},
  {"HOR_NICE_SLIDER",	FL_HOR_NICE_SLIDER},
};

#include <stdlib.h>

int lookup_symbol(const char *name, int &v, int numberok) {
  if (name[0]=='F' && name[1]=='L' && name[2]=='_') name += 3;
  for (int i=0; i < int(sizeof(table)/sizeof(*table)); i++)
    if (!fl_ascii_strcasecmp(name,table[i].name)) {v = table[i].value; return 1;}
  if (numberok && ((v = atoi(name)) || !strcmp(name,"0"))) return 1;
  return 0;
}

//
// End of "$Id$".
//
