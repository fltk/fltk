//
// "$Id$"
//
// Widget type header file for the Fast Light Tool Kit (FLTK).
//
// Each object described by Fluid is one of these objects.  They
// are all stored in a double-linked list.
//
// There is also a single "factory" instance of each type of this.
// The method "make()" is called on this factory to create a new
// instance of this object.  It could also have a "copy()" function,
// but it was easier to implement this by using the file read/write
// that is needed to save the setup anyways.
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

#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu.H>
#include <FL/Fl_Plugin.H>
#include "Fluid_Image.h"
#include <FL/fl_draw.H>
#include <stdarg.h>

#ifdef WIN32
  #include "ExternalCodeEditor_WIN32.h"
#else
  #include "ExternalCodeEditor_UNIX.h"
#endif

void set_modflag(int mf);

class Fl_Type {

  friend class Widget_Browser;
  friend Fl_Widget *make_type_browser(int,int,int,int,const char *);
  friend class Fl_Window_Type;
  virtual void setlabel(const char *); // virtual part of label(char*)

protected:

  Fl_Type();

  const char *name_;
  const char *label_;
  const char *callback_;
  const char *user_data_;
  const char *user_data_type_;
  const char *comment_;

public:	// things that should not be public:

  Fl_Type *parent; // parent, which is previous in list
  char new_selected; // browser highlight
  char selected; // copied here by selection_changed()
  char open_;	// state of triangle in browser
  char visible; // true if all parents are open
  char rtti;	// hack because I have no rtti, this is 0 for base class
  int level;	// number of parents over this
  static Fl_Type *first, *last; // linked list of all objects
  Fl_Type *next, *prev;	// linked list of all objects

  Fl_Type *factory;
  const char *callback_name();

  int code_position, header_position;
  int code_position_end, header_position_end;

protected:
  int user_defined(const char* cbname) const;

public:

  virtual ~Fl_Type();
  virtual Fl_Type *make() = 0;

  void add(Fl_Type *parent); // add as new child
  void insert(Fl_Type *n); // insert into list before n
  Fl_Type* remove();	// remove from list
  void move_before(Fl_Type*); // move before a sibling

  virtual const char *title(); // string for browser
  virtual const char *type_name() = 0; // type for code output
  virtual const char *alt_type_name() { return type_name(); } // alternate type for FLTK2 code output

  const char *name() const {return name_;}
  void name(const char *);
  const char *label() const {return label_;}
  void label(const char *);
  const char *callback() const {return callback_;}
  void callback(const char *);
  const char *user_data() const {return user_data_;}
  void user_data(const char *);
  const char *user_data_type() const {return user_data_type_;}
  void user_data_type(const char *);
  const char *comment() { return comment_; }
  void comment(const char *);

  virtual Fl_Type* click_test(int,int);
  virtual void add_child(Fl_Type*, Fl_Type* beforethis);
  virtual void move_child(Fl_Type*, Fl_Type* beforethis);
  virtual void remove_child(Fl_Type*);

  static Fl_Type *current;  // most recently picked object
  virtual void open();	// what happens when you double-click

  // read and write data to a saved file:
  virtual void write();
  virtual void write_properties();
  virtual void read_property(const char *);
  virtual int read_fdesign(const char*, const char*);

  // write code, these are called in order:
  virtual void write_static(); // write static stuff to .c file
  virtual void write_code1(); // code and .h before children
  virtual void write_code2(); // code and .h after children
  void write_comment_h(const char *ind=""); // write the commentary text into the header file
  void write_comment_c(const char *ind=""); // write the commentary text into the source file
  void write_comment_inline_c(const char *ind=0L); // write the commentary text

  // live mode
  virtual Fl_Widget *enter_live_mode(int top=0); // build wdgets needed for live mode
  virtual void leave_live_mode(); // free allocated resources
  virtual void copy_properties(); // copy properties from this type into a potetial live object

  // get message number for I18N
  int msgnum();

  // fake rtti:
  virtual int is_parent() const;
  virtual int is_widget() const;
  virtual int is_button() const;
  virtual int is_input() const;
  virtual int is_value_input() const;
  virtual int is_text_display() const;
  virtual int is_valuator() const;
  virtual int is_spinner() const;
  virtual int is_menu_item() const;
  virtual int is_menu_button() const;
  virtual int is_group() const;
  virtual int is_window() const;
  virtual int is_code() const;
  virtual int is_code_block() const;
  virtual int is_decl_block() const;
  virtual int is_comment() const;
  virtual int is_class() const;
  virtual int is_public() const;

  virtual int pixmapID() { return 0; }

  const char* class_name(const int need_nest) const;
  const class Fl_Class_Type* is_in_class() const;
};

class Fl_Function_Type : public Fl_Type {
  const char* return_type;
  char public_, cdecl_, constructor, havewidgets;
public:
  Fl_Function_Type() : 
    Fl_Type(), 
    return_type(0L), public_(0), cdecl_(0), constructor(0), havewidgets(0)
  { }
  ~Fl_Function_Type() {
    if (return_type) free((void*)return_type);
  }
  Fl_Type *make();
  void write_code1();
  void write_code2();
  void open();
  int ismain() {return name_ == 0;}
  virtual const char *type_name() {return "Function";}
  virtual const char *title() {
    return name() ? name() : "main()";
  }
  int is_parent() const {return 1;}
  int is_code_block() const {return 1;}
  virtual int is_public() const;
  int pixmapID() { return 7; }
  void write_properties();
  void read_property(const char *);
  int has_signature(const char *, const char*) const;
};

class Fl_Code_Type : public Fl_Type {
  ExternalCodeEditor editor_;
public:
  Fl_Type *make();
  void write();
  void write_code1();
  void write_code2();
  void open();
  virtual const char *type_name() {return "code";}
  int is_code_block() const {return 0;}
  int is_code() const {return 1;}
  int pixmapID() { return 8; }
  virtual int is_public() const;
  // See if external editor is open
  int is_editing() {
    return editor_.is_editing();
  }
  // Reap the editor's pid
  // Returns:
  //   -2 -- editor not open
  //   -1 -- wait failed
  //    0 -- process still running
  //   >0 -- process finished + reaped (returns pid)
  //
  int reap_editor() {
    return editor_.reap_editor();
  }
  // Handle external editor file modifications
  // If changed, record keeping is updated and file's contents is loaded into ram
  //
  // Returns:
  //     0 -- file unchanged or not editing
  //     1 -- file changed, internal records updated, 'code' has new content
  //    -1 -- error getting file info (get_ms_errmsg() has reason)
  //
  // TODO: Figure out how saving a fluid file can be intercepted to grab 
  //       current contents of editor file..
  //
  int handle_editor_changes() {
    const char *newcode = 0;
    switch ( editor_.handle_changes(&newcode) ) {
      case 1: {            // (1)=changed
        name(newcode);     // update value in ram
        free((void*)newcode);
        return 1;
      }
      case -1: return -1;  // (-1)=error -- couldn't read file (dialog showed reason)
      default: break;      // (0)=no change
    }
    return 0;
  }
};

class Fl_CodeBlock_Type : public Fl_Type {
  const char* after;
public:
  Fl_CodeBlock_Type() : Fl_Type(), after(0L) { }
  ~Fl_CodeBlock_Type() {
    if (after) free((void*)after);
  }
  Fl_Type *make();
  void write_code1();
  void write_code2();
  void open();
  virtual const char *type_name() {return "codeblock";}
  int is_code_block() const {return 1;}
  int is_parent() const {return 1;}
  virtual int is_public() const;
  int pixmapID() { return 9; }
  void write_properties();
  void read_property(const char *);
};

class Fl_Decl_Type : public Fl_Type {
protected:
  char public_;
  char static_;
public:
  Fl_Type *make();
  void write_code1();
  void write_code2();
  void open();
  virtual const char *type_name() {return "decl";}
  void write_properties();
  void read_property(const char *);
  virtual int is_public() const;
  int pixmapID() { return 10; }
};

class Fl_Data_Type : public Fl_Decl_Type {
  const char *filename_;
public:
  Fl_Data_Type() : Fl_Decl_Type(), filename_(0L) { }
  ~Fl_Data_Type() {
    if (filename_) free((void*)filename_);
  }
  Fl_Type *make();
  void write_code1();
  void write_code2();
  void open();
  virtual const char *type_name() {return "data";}
  void write_properties();
  void read_property(const char *);
  int pixmapID() { return 49; }
};

class Fl_DeclBlock_Type : public Fl_Type {
  const char* after;
  char public_;
public:
  Fl_DeclBlock_Type() : Fl_Type(), after(0L) { }
  ~Fl_DeclBlock_Type() {
    if (after) free((void*)after);
  }
  Fl_Type *make();
  void write_code1();
  void write_code2();
  void open();
  virtual const char *type_name() {return "declblock";}
  void write_properties();
  void read_property(const char *);
  int is_parent() const {return 1;}
  int is_decl_block() const {return 1;}
  virtual int is_public() const;
  int pixmapID() { return 11; }
};

class Fl_Comment_Type : public Fl_Type {
  char in_c_, in_h_, style_;
  char title_buf[64];
public:
  Fl_Type *make();
  void write_code1();
  void write_code2();
  void open();
  virtual const char *type_name() {return "comment";}
  virtual const char *title(); // string for browser
  void write_properties();
  void read_property(const char *);
  virtual int is_public() const { return 1; }
  virtual int is_comment() const { return 1; }
  int pixmapID() { return 46; }
};

class Fl_Class_Type : public Fl_Type {
  const char* subclass_of;
  char public_;
public:
  Fl_Class_Type() : Fl_Type(), subclass_of(0L) { }
  ~Fl_Class_Type() {
    if (subclass_of) free((void*)subclass_of);
  }
  
  // state variables for output:
  char write_public_state; // true when public: has been printed
  Fl_Class_Type* parent_class; // save class if nested
//
  Fl_Type *make();
  void write_code1();
  void write_code2();
  void open();
  virtual const char *type_name() {return "class";}
  int is_parent() const {return 1;}
  int is_decl_block() const {return 1;}
  int is_class() const {return 1;}
  virtual int is_public() const;
  int pixmapID() { return 12; }
  void write_properties();
  void read_property(const char *);

  // class prefix attribute access
  void prefix(const char* p);
  const char*  prefix() const {return class_prefix;}
  int has_function(const char*, const char*) const;
private:
  const char* class_prefix;
};

#define NUM_EXTRA_CODE 4

class Fl_Widget_Type : public Fl_Type {
  virtual Fl_Widget *widget(int,int,int,int) = 0;
  virtual Fl_Widget_Type *_make() = 0; // virtual constructor
  virtual void setlabel(const char *);

  const char *extra_code_[NUM_EXTRA_CODE];
  const char *subclass_;
  const char *tooltip_;
  const char *image_name_;
  const char *inactive_name_;
  uchar hotspot_;

protected:

  void write_static();
  void write_code1();
  void write_widget_code();
  void write_extra_code();
  void write_block_close();
  void write_code2();
  void write_color(const char*, Fl_Color);
  Fl_Widget *live_widget;

public:
  static int default_size;

  const char *xclass; // junk string, used for shortcut
  Fl_Widget *o;
  int public_;

  Fluid_Image *image;
  void setimage(Fluid_Image *);
  Fluid_Image *inactive;
  void setinactive(Fluid_Image *);

  Fl_Widget_Type();
  Fl_Type *make();
  void open();

  const char *extra_code(int n) const {return extra_code_[n];}
  void extra_code(int n,const char *);
  const char *subclass() const {return subclass_;}
  void subclass(const char *);
  const char *tooltip() const {return tooltip_;}
  void tooltip(const char *);
  const char *image_name() const {return image_name_;}
  void image_name(const char *);
  const char *inactive_name() const {return inactive_name_;}
  void inactive_name(const char *);
  uchar hotspot() const {return hotspot_;}
  void hotspot(uchar v) {hotspot_ = v;}
  uchar resizable() const;
  void resizable(uchar v);

  virtual int textstuff(int what, Fl_Font &, int &, Fl_Color &);
  virtual Fl_Menu_Item *subtypes();

  virtual int is_widget() const;
  virtual int is_public() const;

  virtual void write_properties();
  virtual void read_property(const char *);
  virtual int read_fdesign(const char*, const char*);

  virtual Fl_Widget *enter_live_mode(int top=0);
  virtual void leave_live_mode();
  virtual void copy_properties();

  virtual void ideal_size(int &w, int &h);
  virtual void ideal_spacing(int &x, int &y);

  ~Fl_Widget_Type();
  void redraw();
};

#include <FL/Fl_Tabs.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Wizard.H>

class igroup : public Fl_Group {
public:
  void resize(int,int,int,int);
  void full_resize(int X, int Y, int W, int H) { Fl_Group::resize(X, Y, W, H); }
  igroup(int X,int Y,int W,int H) : Fl_Group(X,Y,W,H) {Fl_Group::current(0);}
};

class itabs : public Fl_Tabs {
public:
  void resize(int,int,int,int);
  void full_resize(int X, int Y, int W, int H) { Fl_Group::resize(X, Y, W, H); }
  itabs(int X,int Y,int W,int H) : Fl_Tabs(X,Y,W,H) {}
};

class iwizard : public Fl_Wizard {
public:
  void resize(int,int,int,int);
  void full_resize(int X, int Y, int W, int H) { Fl_Group::resize(X, Y, W, H); }
  iwizard(int X,int Y,int W,int H) : Fl_Wizard(X,Y,W,H) {}
};

class Fl_Group_Type : public Fl_Widget_Type {
public:
  virtual const char *type_name() {return "Fl_Group";}
  virtual const char *alt_type_name() {return "fltk::Group";}
  Fl_Widget *widget(int X,int Y,int W,int H) {
    igroup *g = new igroup(X,Y,W,H); Fl_Group::current(0); return g;}
  Fl_Widget_Type *_make() {return new Fl_Group_Type();}
  Fl_Type *make();
  void write_code1();
  void write_code2();
  void add_child(Fl_Type*, Fl_Type*);
  void move_child(Fl_Type*, Fl_Type*);
  void remove_child(Fl_Type*);
  int is_parent() const {return 1;}
  int is_group() const {return 1;}
  int pixmapID() { return 6; }

  virtual Fl_Widget *enter_live_mode(int top=0);
  virtual void leave_live_mode();
  virtual void copy_properties();
};

extern const char pack_type_name[];
extern Fl_Menu_Item pack_type_menu[];

class Fl_Pack_Type : public Fl_Group_Type {
  Fl_Menu_Item *subtypes() {return pack_type_menu;}
public:
  virtual const char *type_name() {return pack_type_name;}
  virtual const char *alt_type_name() {return "fltk::PackedGroup";}
  Fl_Widget_Type *_make() {return new Fl_Pack_Type();}
  int pixmapID() { return 22; }
  void copy_properties();
};

extern const char table_type_name[];

class Fl_Table_Type : public Fl_Group_Type {
public:
  virtual const char *type_name() {return table_type_name;}
  virtual const char *alt_type_name() {return "fltk::TableGroup";}
  Fl_Widget_Type *_make() {return new Fl_Table_Type();}
  Fl_Widget *widget(int X,int Y,int W,int H);
  int pixmapID() { return 51; }
  virtual Fl_Widget *enter_live_mode(int top=0);
  void add_child(Fl_Type*, Fl_Type*);
  void move_child(Fl_Type*, Fl_Type*);
  void remove_child(Fl_Type*);
};

extern const char tabs_type_name[];

class Fl_Tabs_Type : public Fl_Group_Type {
public:
  virtual void ideal_spacing(int &x, int &y) {
     x = 10;
     fl_font(o->labelfont(), o->labelsize());
     y = fl_height() + o->labelsize() - 6;
  }
  virtual const char *type_name() {return tabs_type_name;}
  virtual const char *alt_type_name() {return "fltk::TabGroup";}
  Fl_Widget *widget(int X,int Y,int W,int H) {
    itabs *g = new itabs(X,Y,W,H); Fl_Group::current(0); return g;}
  Fl_Widget_Type *_make() {return new Fl_Tabs_Type();}
  Fl_Type* click_test(int,int);
  void add_child(Fl_Type*, Fl_Type*);
  void remove_child(Fl_Type*);
  int pixmapID() { return 13; }
  Fl_Widget *enter_live_mode(int top=0);
};

extern const char scroll_type_name[];
extern Fl_Menu_Item scroll_type_menu[];

class Fl_Scroll_Type : public Fl_Group_Type {
  Fl_Menu_Item *subtypes() {return scroll_type_menu;}
public:
  virtual const char *type_name() {return scroll_type_name;}
  virtual const char *alt_type_name() {return "fltk::ScrollGroup";}
  Fl_Widget_Type *_make() {return new Fl_Scroll_Type();}
  int pixmapID() { return 19; }
  Fl_Widget *enter_live_mode(int top=0);
  void copy_properties();
};

extern const char tile_type_name[];

class Fl_Tile_Type : public Fl_Group_Type {
public:
  virtual const char *type_name() {return tile_type_name;}
  virtual const char *alt_type_name() {return "fltk::TileGroup";}
  Fl_Widget_Type *_make() {return new Fl_Tile_Type();}
  int pixmapID() { return 20; }
  void copy_properties();
};

extern const char wizard_type_name[];

class Fl_Wizard_Type : public Fl_Group_Type {
public:
  virtual const char *type_name() {return wizard_type_name;}
  virtual const char *alt_type_name() {return "fltk::WizardGroup";}
  Fl_Widget *widget(int X,int Y,int W,int H) {
    iwizard *g = new iwizard(X,Y,W,H); Fl_Group::current(0); return g;}
  Fl_Widget_Type *_make() {return new Fl_Wizard_Type();}
  int pixmapID() { return 21; }
};

extern Fl_Menu_Item window_type_menu[];

class Fl_Window_Type : public Fl_Widget_Type {
protected:

  Fl_Menu_Item* subtypes() {return window_type_menu;}

  friend class Overlay_Window;
  int mx,my;		// mouse position during dragging
  int x1,y1;		// initial position of selection box
  int bx,by,br,bt;	// bounding box of selection before snapping
  int sx,sy,sr,st;	// bounding box of selection after snapping to guides
  int dx,dy;
  int drag;		// which parts of bbox are being moved
  int numselected;	// number of children selected
  enum {LEFT=1,RIGHT=2,BOTTOM=4,TOP=8,DRAG=16,BOX=32};
  void draw_overlay();
  void newdx();
  void newposition(Fl_Widget_Type *,int &x,int &y,int &w,int &h);
  int handle(int);
  virtual void setlabel(const char *);
  void write_code1();
  void write_code2();
  Fl_Widget_Type *_make() {return 0;} // we don't call this
  Fl_Widget *widget(int,int,int,int) {return 0;}
  int recalc;		// set by fix_overlay()
  void moveallchildren();
  int pixmapID() { return 1; }

public:

  Fl_Window_Type() { drag = dx = dy = 0; sr_min_w = sr_min_h = sr_max_w = sr_max_h = 0; }
  uchar modal, non_modal;

  Fl_Type *make();
  virtual const char *type_name() {return "Fl_Window";}
  virtual const char *alt_type_name() {return "fltk::Window";}

  void open();

  void fix_overlay();			// Update the bounding box, etc
  uchar *read_image(int &ww, int &hh);	// Read an image of the window

  virtual void write_properties();
  virtual void read_property(const char *);
  virtual int read_fdesign(const char*, const char*);

  void add_child(Fl_Type*, Fl_Type*);
  void move_child(Fl_Type*, Fl_Type*);
  void remove_child(Fl_Type*);

  int is_parent() const {return 1;}
  int is_group() const {return 1;}
  int is_window() const {return 1;}

  Fl_Widget *enter_live_mode(int top=0);
  void leave_live_mode();
  void copy_properties();

  int sr_min_w, sr_min_h, sr_max_w, sr_max_h;
};

class Fl_Widget_Class_Type : private Fl_Window_Type {
public:
  Fl_Widget_Class_Type() {
    write_public_state = 0;
    wc_relative = 0;
  }
  // state variables for output:
  char write_public_state; // true when public: has been printed
  char wc_relative; // if true, reposition all child widgets in an Fl_Group

  virtual void write_properties();
  virtual void read_property(const char *);

  void write_code1();
  void write_code2();
  Fl_Type *make();
  virtual const char *type_name() {return "widget_class";}
  int pixmapID() { return 48; }
  int is_parent() const {return 1;}
  int is_code_block() const {return 1;}
  int is_decl_block() const {return 1;}
  int is_class() const {return 1;}
};


extern Fl_Menu_Item menu_item_type_menu[];

class Fl_Menu_Item_Type : public Fl_Widget_Type {
public:
  Fl_Menu_Item* subtypes() {return menu_item_type_menu;}
  const char* type_name() {return "MenuItem";}
  const char* alt_type_name() {return "fltk::Item";}
  Fl_Type* make();
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

class Fl_Submenu_Type : public Fl_Menu_Item_Type {
public:
  Fl_Menu_Item* subtypes() {return 0;}
  const char* type_name() {return "Submenu";}
  const char* alt_type_name() {return "fltk::ItemGroup";}
  int is_parent() const {return 1;}
  int is_button() const {return 0;} // disable shortcut
  Fl_Type* make();
  // changes to submenu must propagate up so build_menu is called
  // on the parent Fl_Menu_Type:
  void add_child(Fl_Type*a, Fl_Type*b) {parent->add_child(a,b);}
  void move_child(Fl_Type*a, Fl_Type*b) {parent->move_child(a,b);}
  void remove_child(Fl_Type*a) {parent->remove_child(a);}
  int pixmapID() { return 18; }
};


#include <FL/Fl_Menu_.H>
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

#include <FL/Fl_Menu_Button.H>
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

#include <FL/Fl_Input_Choice.H>
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
  int pixmapID() { return 15; }
  void copy_properties();
};

#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
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
// object list operations:
Fl_Widget *make_widget_browser(int X,int Y,int W,int H);
void redraw_widget_browser(Fl_Type*);
extern int modflag;
void delete_all(int selected_only=0);
void selection_changed(Fl_Type* new_current);
void reveal_in_browser(Fl_Type*);
int has_toplevel_function(const char *rtype, const char *sig);

// file operations:
#  ifdef __GNUC__
#    define __fl_attr(x) __attribute__ (x)
#  else
#    define __fl_attr(x)
#  endif // __GNUC__

void write_word(const char *);
void write_string(const char *,...) __fl_attr((__format__ (__printf__, 1, 2)));
int write_file(const char *, int selected_only = 0);
int write_code(const char *cfile, const char *hfile);
int write_strings(const char *sfile);

int write_declare(const char *, ...) __fl_attr((__format__ (__printf__, 1, 2)));
int is_id(char);
const char* unique_id(void* o, const char*, const char*, const char*);
void write_c(const char*, ...) __fl_attr((__format__ (__printf__, 1, 2)));
void vwrite_c(const char* format, va_list args);
void write_h(const char*, ...) __fl_attr((__format__ (__printf__, 1, 2)));
void write_cstring(const char *);
void write_cstring(const char *,int length);
void write_cdata(const char *,int length);
void write_indent(int n);
void write_open(int);
void write_close(int n);
extern int write_number;
extern int write_sourceview;
void write_public(int state); // writes pubic:/private: as needed
extern int indentation;
extern const char* indent();

int read_file(const char *, int merge);
const char *read_word(int wantbrace = 0);
void read_error(const char *format, ...);

// check legality of c code (sort of) and return error:
const char *c_check(const char *c, int type = 0);

// replace a string pointer with new value, strips leading/trailing blanks:
int storestring(const char *n, const char * & p, int nostrip=0);

extern int include_H_from_C;
extern int use_FL_COMMAND;

/*
 * This class is needed for additional command line plugins.
 */
class Fl_Commandline_Plugin : public Fl_Plugin {
public:
  Fl_Commandline_Plugin(const char *name)
  : Fl_Plugin(klass(), name) { }
  virtual const char *klass() { return "commandline"; }
  // return a unique name for this plugin
  virtual const char *name() = 0;
  // return a help text for all supported commands
  virtual const char *help() = 0;
  // handle a command and return the number of args used, or 0
  virtual int arg(int argc, char **argv, int &i) = 0;
  // optional test the plugin
  virtual int test(const char *a1=0L, const char *a2=0L, const char *a3=0L) { 
    return 0;
  }
  // show a GUI panel to edit some data
  virtual void show_panel() { }
};


//
// End of "$Id$".
//
