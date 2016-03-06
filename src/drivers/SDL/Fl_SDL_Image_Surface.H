class Fl_Image_Surface::Helper : public Fl_Widget_Surface { // class model
  friend class Fl_Image_Surface;
public:
  Fl_Offscreen offscreen;
  int width;
  int height;
  Helper(int w, int h, int high_res) : Fl_Widget_Surface(NULL), width(w), height(h) {} // to implement
  ~Helper() {} // to implement
  void set_current(){} // to implement
  void translate(int x, int y) {} // to implement
  void untranslate() {} // to implement
  Fl_RGB_Image *image() {} // to implement
  void end_current() {} // to implement
  int printable_rect(int *w, int *h) {*w = width; *h = height; return 0;}
};
