//
// "$Id: Fl_Group.H,v 1.6.2.4 2001/01/22 15:13:37 easysw Exp $"
//
// Group header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

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

  FL_EXPORT int navigation(int);
  static FL_EXPORT Fl_Group *current_;

protected:

  FL_EXPORT void draw();
  FL_EXPORT void draw_child(Fl_Widget&) const;
  FL_EXPORT void update_child(Fl_Widget&) const;
  FL_EXPORT void draw_outside_label(const Fl_Widget&) const ;
  FL_EXPORT short* sizes();

public:

  FL_EXPORT int handle(int);
  void begin() {current_ = this;}
  void end() {current_ = (Fl_Group*)parent();}
  static Fl_Group *current() {return current_;}
  static void current(Fl_Group *g) {current_ = g;}

  int children() const {return children_;}
  Fl_Widget* child(int n) const {return array()[n];}
  FL_EXPORT int find(const Fl_Widget*) const;
  int find(const Fl_Widget& o) const {return find(&o);}
  FL_EXPORT Fl_Widget* const* array() const;

  FL_EXPORT void resize(int,int,int,int);
  FL_EXPORT Fl_Group(int,int,int,int, const char * = 0);
  FL_EXPORT virtual ~Fl_Group();
  FL_EXPORT void add(Fl_Widget&);
  void add(Fl_Widget* o) {add(*o);}
  FL_EXPORT void insert(Fl_Widget&, int i);
  void insert(Fl_Widget& o, Fl_Widget* before) {insert(o,find(before));}
  FL_EXPORT void remove(Fl_Widget&);
  void remove(Fl_Widget* o) {remove(*o);}
  FL_EXPORT void clear();

  void resizable(Fl_Widget& o) {resizable_ = &o;}
  void resizable(Fl_Widget* o) {resizable_ = o;}
  Fl_Widget* resizable() const {return resizable_;}
  void add_resizable(Fl_Widget& o) {resizable_ = &o; add(o);}
  FL_EXPORT void init_sizes();

  // back compatability function:
  void focus(Fl_Widget* o) {o->take_focus();}
  Fl_Widget* & _ddfdesign_kludge() {return resizable_;}
  FL_EXPORT void forms_end();
};

// dummy class used to end child groups in constructors for complex
// subclasses of Fl_Group:
class Fl_End {
public:
  Fl_End() {Fl_Group::current()->end();}
};

#endif

//
// End of "$Id: Fl_Group.H,v 1.6.2.4 2001/01/22 15:13:37 easysw Exp $".
//
