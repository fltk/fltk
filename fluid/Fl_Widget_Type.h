//
// Widget type header file for the Fast Light Tool Kit (FLTK).
//
// Type for creating all subclasses of Fl_Widget
// This should have the widget pointer in it, but it is still in the
// Fl_Type base class.
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

class Fl_Widget_Type : public Fl_Type
{
  typedef Fl_Type super;

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

  /// This variable is set for visible windows in batch mode.
  /// We can't open a window in batch mode, even if we want the "visible" flags
  /// set, so we need a second place to store this information while also
  /// disabling the output of the "hide" property by the Widget Type.
  uchar override_visible_;

  void write_static(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_code1(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_widget_code(Fd_Code_Writer& f);
  void write_extra_code(Fd_Code_Writer& f);
  void write_block_close(Fd_Code_Writer& f);
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_color(Fd_Code_Writer& f, const char*, Fl_Color);
  Fl_Widget *live_widget;

public:
  Fl_Widget *o;
  int public_;
  int bind_image_;
  int compress_image_;
  int bind_deimage_;
  int compress_deimage_;
  int scale_image_w_, scale_image_h_;
  int scale_deimage_w_, scale_deimage_h_;

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

  ID id() const FL_OVERRIDE { return ID_Widget_; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Widget_) ? true : super::is_a(inID); }
  int is_widget() const FL_OVERRIDE;
  int is_true_widget() const FL_OVERRIDE { return 1; }
  int is_public() const FL_OVERRIDE;

  void write_properties(Fd_Project_Writer &f) FL_OVERRIDE;
  void read_property(Fd_Project_Reader &f, const char *) FL_OVERRIDE;
  int read_fdesign(const char*, const char*) FL_OVERRIDE;

  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  Fl_Widget *propagate_live_mode(Fl_Group* grp);
  void leave_live_mode() FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;

  virtual void ideal_size(int &w, int &h);

  ~Fl_Widget_Type();
  void redraw();
};

extern Fl_Window *the_panel;

#endif // _FLUID_FL_WIDGET_TYPE_H
