// Fl_Input_.H

#ifndef Fl_Input__H
#define Fl_Input__H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

#define FL_NORMAL_INPUT		0
#define FL_FLOAT_INPUT		1
#define FL_INT_INPUT		2
#define FL_HIDDEN_INPUT		3
#define FL_MULTILINE_INPUT	4
#define FL_SECRET_INPUT		5

class Fl_Input_ : public Fl_Widget {

  const char* value_;
  char* buffer;

  int size_;
  int bufsize;
  int position_;
  int mark_;
  int xscroll_, yscroll_;
  int mu_p;
  int maximum_size_;

  uchar textfont_;
  uchar textsize_;
  uchar textcolor_;
  uchar cursor_color_;
  uchar erase_cursor_only;

  const char* expand(const char*, char*) const;
  double expandpos(const char*, const char*, const char*, int*) const;
  void minimal_update(int, int);
  void minimal_update(int p);
  void put_in_buffer(int newsize);

  static Fl_Boxtype default_box_;
  static Fl_Font default_font_;
  static int default_size_;
  void setfont() const;

protected:

  int wordboundary(int i) const;
  int lineboundary(int i) const;
  void drawtext(int, int, int, int);
  int up_down_position(int, int keepmark=0);
  void handle_mouse(int, int, int, int, int keepmark=0);
  int handletext(int e, int, int, int, int);
  void maybe_do_callback();
  int xscroll() const {return xscroll_;}
  int yscroll() const {return yscroll_;}

public:

  void resize(int, int, int, int);

  Fl_Input_(int, int, int, int, const char* = 0);
  ~Fl_Input_();

  int value(const char*);
  int value(const char*, int);
  int static_value(const char*);
  int static_value(const char*, int);
  const char* value() const {return value_;}
  char index(int i) const {return value_[i];}
  int size() const {return size_;}
  int maximum_size() const {return maximum_size_;}
  void maximum_size(int m) {maximum_size_ = m;}

  int position() const {return position_;}
  int mark() const {return mark_;}
  int position(int p, int m);
  int position(int p) {return position(p, p);}
  int mark(int m) {return position(position(), m);}
  int replace(int, int, const char*, int=0);
  int cut() {return replace(position(), mark(), 0);}
  int cut(int n) {return replace(position(), position()+n, 0);}
  int cut(int a, int b) {return replace(a, b, 0);}
  int insert(const char* t, int l=0){return replace(position_, mark_, t, l);}
  int copy();
  int undo();
  int copy_cuts();

  Fl_Font textfont() const {return (Fl_Font)textfont_;}
  void textfont(uchar s) {textfont_ = s;}
  uchar textsize() const {return textsize_;}
  void textsize(uchar s) {textsize_ = s;}
  Fl_Color textcolor() const {return (Fl_Color)textcolor_;}
  void textcolor(uchar n) {textcolor_ = n;}
  Fl_Color cursor_color() const {return (Fl_Color)cursor_color_;}
  void cursor_color(uchar n) {cursor_color_ = n;}

  static void default_box(Fl_Boxtype b) {default_box_ = b;}
  static Fl_Boxtype default_box() {return default_box_;}
  static void default_font(Fl_Font b) {default_font_ = b;}
  static Fl_Font default_font() {return default_font_;}
  static void default_size(int b) {default_size_ = b;}
  static int default_size() {return default_size_;}
};

#endif 
