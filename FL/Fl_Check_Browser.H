//
// "$Id: Fl_Check_Browser.H,v 1.1.2.1 2001/08/11 14:49:51 easysw Exp $"
//
// Fl_Check_Browser header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Check_Browser_H
#define Fl_Check_Browser_H

#include "Fl.H"
#include "Fl_Browser_.H"

class Fl_Check_Browser : public Fl_Browser_ {
  /* required routines for Fl_Browser_ subclass: */

  FL_EXPORT void *item_first() const;
  FL_EXPORT void *item_next(void *) const;
  FL_EXPORT void *item_prev(void *) const;
  FL_EXPORT int item_height(void *) const;
  FL_EXPORT int item_width(void *) const;
  FL_EXPORT void item_draw(void *, int, int, int, int) const;
  FL_EXPORT void item_select(void *, int);
  FL_EXPORT int item_selected(void *) const;

  /* private data */

  struct cb_item {
	  cb_item *next;
	  cb_item *prev;
	  char checked;
	  char selected;
	  char *text;
  };

  cb_item *first;
  cb_item *last;
  cb_item *cache;
  int cached_item;
  int nitems_;
  int nchecked_;
  FL_EXPORT cb_item *find_item(int) const;
  FL_EXPORT int lineno(cb_item *) const;

  public:

  FL_EXPORT Fl_Check_Browser(int x, int y, int w, int h, const char *l = 0);

  FL_EXPORT int add(char *s);               // add an (unchecked) item
  FL_EXPORT int add(char *s, int b);        // add an item and set checked
				  // both return the new nitems()
  FL_EXPORT void clear();                   // delete all items
  int nitems() const { return nitems_; }
  int nchecked() const { return nchecked_; }
  FL_EXPORT int checked(int item) const;
  FL_EXPORT void checked(int item, int b);
  void set_checked(int item) { checked(item, 1); }
  FL_EXPORT void check_all();
  FL_EXPORT void check_none();
  FL_EXPORT int value() const;              // currently selected item
  FL_EXPORT char *text(int item) const;     // returns pointer to internal buffer
};

#endif // Fl_Check_Browser_H

//
// End of "$Id: Fl_Check_Browser.H,v 1.1.2.1 2001/08/11 14:49:51 easysw Exp $".
//

