// Fl_Group.H

#ifndef Fl_Group_H
#define Fl_Group_H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

class Fl_Group : public Fl_Widget {

  Fl_Widget** array_;
  Fl_Widget* savedfocus_;
  Fl_Widget* resizable_;
  int children_;
  short *sizes_; // remembered initial sizes of children

  int navigation(int = 0);
  static Fl_Group *current_;

protected:

  void draw();
  int handle(int);
  void draw_child(Fl_Widget&) const;
  void update_child(Fl_Widget&) const;
  void draw_outside_label(const Fl_Widget&) const ;
  short* sizes();

public:

  void begin() {current_ = this;}
  void end() {current_ = (Fl_Group*)parent();}
  static Fl_Group *current() {return current_;}
  static void current(Fl_Group *g) {current_ = g;}

  int children() const {return children_;}
  Fl_Widget* child(int n) const {return array()[n];}
  int find(const Fl_Widget*) const;
  int find(const Fl_Widget& o) const {return find(&o);}
  Fl_Widget* const* array() const;

  void resize(int,int,int,int);
  Fl_Group(int,int,int,int, const char * = 0);
  virtual ~Fl_Group();
  void add(Fl_Widget&);
  void add(Fl_Widget* o) {add(*o);}
  void insert(Fl_Widget&, int i);
  void insert(Fl_Widget& o, Fl_Widget* before) {insert(o,find(before));}
  void remove(Fl_Widget&);
  void remove(Fl_Widget* o) {remove(*o);}
  void clear();

  void resizable(Fl_Widget& o) {resizable_ = &o;}
  void resizable(Fl_Widget* o) {resizable_ = o;}
  Fl_Widget* resizable() const {return resizable_;}
  void add_resizable(Fl_Widget& o) {resizable_ = &o; add(o);}
  void init_sizes();

  // back compatability function:
  void focus(Fl_Widget* o) {o->take_focus();}
  Fl_Widget* & _ddfdesign_kludge() {return resizable_;}
  void forms_end();
};

// dummy class used to end child groups in constructors for complex
// subclasses of Fl_Group:
class Fl_End {
public:
  Fl_End() {Fl_Group::current()->end();}
};

#endif
