//
// "$Id: Fl_Widget_Type.h,v 1.3 1998/12/06 15:18:39 mike Exp $"
//
// Widget type header file for the Fast Light Tool Kit (FLTK).
//
// Type for creating all subclasses of Fl_Widget
// This should have the widget pointer in it, but it is still in the
// Fl_Type base class.
//
// Copyright 1998 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#include "Fl_Type.h"

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

//
// End of "$Id: Fl_Widget_Type.h,v 1.3 1998/12/06 15:18:39 mike Exp $".
//
