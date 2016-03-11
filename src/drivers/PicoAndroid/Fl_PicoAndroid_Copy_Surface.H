
class Fl_Copy_Surface::Helper : public Fl_Widget_Surface { // class model
  friend class Fl_Copy_Surface;
private:
  int width;
  int height;
  Helper(int w, int h) : Fl_Widget_Surface(NULL), width(w), height(h) {} // to implement
  ~Helper() {} // to implement
  void set_current(){} // to implement
  void translate(int x, int y) {} // to implement
  void untranslate() {} // to implement
  int w() {return width;}
  int h() {return height;}
  int printable_rect(int *w, int *h) {*w = width; *h = height; return 0;}
};
