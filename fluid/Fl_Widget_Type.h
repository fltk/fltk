// Fl_Widget_Type.H

// Type for creating all subclasses of Fl_Widget
// This should have the widget pointer in it, but it is still in the
// Fl_Type base class.

#include "Fl_Type.H"

struct Fl_Menu_Item;
class Fluid_Image;

#define NUM_EXTRA_CODE 4

class Fl_Widget_Type : public Fl_Type {
  virtual Fl_Widget *widget(int,int,int,int) = 0;
  virtual Fl_Widget_Type *_make() = 0; // virtual constructor
  virtual void setlabel(const char *);

  const char *extra_code_[NUM_EXTRA_CODE];
  const char *subclass_;
  uchar hotspot_;

protected:

  void write_declare();
  void write_static();
  void write_code1();
  void write_widget_code();
  void write_code2();

public:

  const char *xclass; // junk string, used for shortcut
  Fl_Widget *o;
  int public_;

  Fluid_Image *image;
  void setimage(Fluid_Image *);

  Fl_Widget_Type();
  Fl_Type *make();
  void open();

  const char *extra_code(int n) const {return extra_code_[n];}
  void extra_code(int n,const char *);
  const char *subclass() const {return subclass_;}
  void subclass(const char *);
  uchar hotspot() const {return hotspot_;}
  void hotspot(uchar v) {hotspot_ = v;}
  uchar resizable() const;
  void resizable(uchar v);

  virtual int textstuff(int what, Fl_Font &, int &, Fl_Color &);
  virtual Fl_Menu_Item *subtypes();

  virtual int is_widget() const;

  virtual void write_properties();
  virtual void read_property(const char *);
  virtual int read_fdesign(const char*, const char*);

  ~Fl_Widget_Type();
  void redraw();
};

void* const LOAD = (void *)9831;
extern Fl_Widget_Type *current_widget; // one of the selected ones
