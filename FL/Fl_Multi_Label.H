// Fl_Multi_Label.H

#ifndef Fl_Multi_Label_H
#define Fl_Multi_Label_H

class Fl_Widget;
struct Fl_Menu_Item;

struct Fl_Multi_Label {
  const char* labela;
  const char* labelb;
  uchar typea;
  uchar typeb;
  void label(Fl_Widget*);
  void label(Fl_Menu_Item*);
};

#endif
