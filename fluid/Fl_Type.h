// Fl_Type.H

// Each object described by Fluid is one of these objects.  They
// are all stored in a double-linked list.

// There is also a single "factory" instance of each type of this.
// The method "make()" is called on this factory to create a new
// instance of this object.  It could also have a "copy()" function,
// but it was easier to implement this by using the file read/write
// that is needed to save the setup anyways.

#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu.H>

class Fl_Type {

  friend class Widget_Browser;
  friend Fl_Widget *make_type_browser(int,int,int,int,const char *l=0);
  friend class Fl_Window_Type;
  virtual void setlabel(const char *); // virtual part of label(char*)

protected:

  Fl_Type();

  const char *name_;
  const char *label_;
  const char *callback_;
  const char *user_data_;
  const char *user_data_type_;

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

public:

  virtual ~Fl_Type();
  virtual Fl_Type *make() = 0;

  void add(Fl_Type *parent); // add as new child
  void insert(Fl_Type *n); // insert into list before n
  Fl_Type* remove();	// remove from list
  void move_before(Fl_Type*); // move before a sibling

  virtual const char *title(); // string for browser
  virtual const char *type_name() = 0; // type for code output

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

  virtual Fl_Type* click_test(int,int);
  virtual void add_child(Fl_Type*, Fl_Type* beforethis);
  virtual void move_child(Fl_Type*, Fl_Type* beforethis);
  virtual void remove_child(Fl_Type*);

  static Fl_Type *current;  // most recently picked object
  virtual void open();	// what happens when you double-click

  // read and write data to a saved file:
  void write();
  virtual void write_properties();
  virtual void read_property(const char *);
  virtual int read_fdesign(const char*, const char*);

  // write code, these are called in order:
  virtual void write_declare(); // write #include and #define to .h file
  virtual void write_static(); // write static stuff to .c file
  virtual void write_code1(); // code and .h before children
  virtual void write_code2(); // code and .h after children

  // fake rtti:
  virtual int is_parent() const;
  virtual int is_widget() const;
  virtual int is_button() const;
  virtual int is_valuator() const;
  virtual int is_menu_item() const;
  virtual int is_menu_button() const;
  virtual int is_group() const;
  virtual int is_window() const;
  virtual int is_code_block() const;
  virtual int is_decl_block() const;
  virtual int is_class() const;

  const char* class_name() const;
};

// object list operations:
Fl_Widget *make_widget_browser(int x,int y,int w,int h);
extern int modflag;
void delete_all(int selected_only=0);
void selection_changed(Fl_Type* new_current);

// file operations:
void write_word(const char *);
void write_string(const char *,...);
int write_file(const char *, int selected_only = 0);
int write_code(const char *cfile, const char *hfile);

int write_declare(const char *, ...);
int is_id(char);
const char* unique_id(void* o, const char*, const char*, const char*);
void write_c(const char*, ...);
void write_h(const char*, ...);
void write_cstring(const char *);
void write_cstring(const char *,int length);
void write_indent(int n);
void write_open(int);
void write_close(int n);
extern int write_number;
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
