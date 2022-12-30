//
// Widget type header file for the Fast Light Tool Kit (FLTK).
//
// Type for creating all subclasses of Fl_Widget
// This should have the widget pointer in it, but it is still in the
// Fl_Type base class.
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#ifndef _FLUID_FL_WIDGET_TYPE_H
#define _FLUID_FL_WIDGET_TYPE_H

#include "Fl_Type.h"

#define NUM_EXTRA_CODE 4

class Fl_Widget_Type;
class Fluid_Image;

extern void* const LOAD;
extern Fl_Widget_Type *current_widget; // one of the selected ones

extern const char* subclassname(Fl_Type* l);
extern int is_name(const char *c);
void selection_changed(Fl_Type* new_current);
Fl_Type *sort(Fl_Type *parent);
void comment_cb(class Fl_Text_Editor* i, void *v);

class Fl_Widget_Type : public Fl_Type {
  virtual Fl_Widget *widget(int,int,int,int) = 0;
  virtual Fl_Widget_Type *_make() = 0; // virtual constructor
  void setlabel(const char *) FL_OVERRIDE;

  const char *extra_code_[NUM_EXTRA_CODE];
  const char *subclass_;
  const char *tooltip_;
  const char *image_name_;
  const char *inactive_name_;
  uchar hotspot_;

protected:

  void write_static() FL_OVERRIDE;
  void write_code1() FL_OVERRIDE;
  void write_widget_code();
  void write_extra_code();
  void write_block_close();
  void write_code2() FL_OVERRIDE;
  void write_color(const char*, Fl_Color);
  Fl_Widget *live_widget;

public:
  static int default_size;

  const char *xclass; // junk string, used for shortcut
  Fl_Widget *o;
  int public_;
  int bind_image_;
  int compress_image_;
  int bind_deimage_;
  int compress_deimage_;

  Fluid_Image *image;
  void setimage(Fluid_Image *);
  Fluid_Image *inactive;
  void setinactive(Fluid_Image *);

  Fl_Widget_Type();
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void open() FL_OVERRIDE;

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

  int is_widget() const FL_OVERRIDE;
  int is_public() const FL_OVERRIDE;

  void write_properties() FL_OVERRIDE;
  void read_property(const char *) FL_OVERRIDE;
  int read_fdesign(const char*, const char*) FL_OVERRIDE;

  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  void leave_live_mode() FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;

  virtual void ideal_size(int &w, int &h);
  virtual void ideal_spacing(int &x, int &y);

  ~Fl_Widget_Type();
  void redraw();
};


#endif // _FLUID_FL_WIDGET_TYPE_H
