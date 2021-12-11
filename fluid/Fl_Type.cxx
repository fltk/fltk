//
// Widget type code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

/** \class Fl_Type
Each object described by Fluid is one of these objects.  They
are all stored in a double-linked list.

The "type" of the object is covered by the virtual functions.
There will probably be a lot of these virtual functions.

The type browser is also a list of these objects, but they
are "factory" instances, not "real" ones.  These objects exist
only so the "make" method can be called on them.  They are
not in the linked list and are not written to files or
copied or otherwise examined.
*/

#include "Fl_Type.h"

#include "fluid.h"
#include "Fl_Function_Type.h"
#include "Fl_Widget_Type.h"
#include "Fl_Window_Type.h"
#include "widget_browser.h"
#include "file.h"
#include "code.h"
#include "undo.h"
#include "pixmaps.h"
#include "shell_command.h"

#include <FL/Fl.H>
#include <FL/Fl_Browser_.H>
#include <FL/fl_draw.H>
#include "../src/flstring.h"

#include <stdlib.h>
#include <stdio.h>

// ---- global variables

Fl_Type *Fl_Type::first = NULL;
Fl_Type *Fl_Type::last = NULL;

Fl_Type *in_this_only; // set if menu popped-up in window

// ---- various functions

void select_all_cb(Fl_Widget *,void *) {
  Fl_Type *p = Fl_Type::current ? Fl_Type::current->parent : 0;
  if (in_this_only) {
    Fl_Type *t = p;
    for (; t && t != in_this_only; t = t->parent) {/*empty*/}
    if (t != in_this_only) p = in_this_only;
  }
  for (;;) {
    if (p) {
      int foundany = 0;
      for (Fl_Type *t = p->next; t && t->level>p->level; t = t->next) {
        if (!t->new_selected) {widget_browser->select(t,1,0); foundany = 1;}
      }
      if (foundany) break;
      p = p->parent;
    } else {
      for (Fl_Type *t = Fl_Type::first; t; t = t->next)
        widget_browser->select(t,1,0);
      break;
    }
  }
  selection_changed(p);
}

void select_none_cb(Fl_Widget *,void *) {
  Fl_Type *p = Fl_Type::current ? Fl_Type::current->parent : 0;
  if (in_this_only) {
    Fl_Type *t = p;
    for (; t && t != in_this_only; t = t->parent) {/*empty*/}
    if (t != in_this_only) p = in_this_only;
  }
  for (;;) {
    if (p) {
      int foundany = 0;
      for (Fl_Type *t = p->next; t && t->level>p->level; t = t->next) {
        if (t->new_selected) {widget_browser->select(t,0,0); foundany = 1;}
      }
      if (foundany) break;
      p = p->parent;
    } else {
      for (Fl_Type *t = Fl_Type::first; t; t = t->next)
        widget_browser->select(t,0,0);
      break;
    }
  }
  selection_changed(p);
}

// move selected widgets in their parent's list:
void earlier_cb(Fl_Widget*,void*) {
  Fl_Type *f;
  int mod = 0;
  for (f = Fl_Type::first; f; ) {
    Fl_Type* nxt = f->next;
    if (f->selected) {
      Fl_Type* g;
      for (g = f->prev; g && g->level > f->level; g = g->prev) {/*empty*/}
      if (g && g->level == f->level && !g->selected) {
        f->move_before(g);
        mod = 1;
      }
    }
    f = nxt;
  }
  if (mod) set_modflag(1);
}

void later_cb(Fl_Widget*,void*) {
  Fl_Type *f;
  int mod = 0;
  for (f = Fl_Type::last; f; ) {
    Fl_Type* prv = f->prev;
    if (f->selected) {
      Fl_Type* g;
      for (g = f->next; g && g->level > f->level; g = g->next) {/*empty*/}
      if (g && g->level == f->level && !g->selected) {
        g->move_before(f);
        mod = 1;
      }
    }
    f = prv;
  }
  if (mod) set_modflag(1);
}

static void delete_children(Fl_Type *p) {
  Fl_Type *f;
  for (f = p; f && f->next && f->next->level > p->level; f = f->next) {/*empty*/}
  for (; f != p; ) {
    Fl_Type *g = f->prev;
    widget_browser->deleting(f);
    delete f;
    f = g;
  }
}

// object list operations:
void delete_all(int selected_only) {
  for (Fl_Type *f = Fl_Type::first; f;) {
    if (f->selected || !selected_only) {
      delete_children(f);
      Fl_Type *g = f->next;
      widget_browser->deleting(f);
      delete f;
      f = g;
    } else f = f->next;
  }
  if(!selected_only) {
    include_H_from_C=1;
    use_FL_COMMAND=0;
    // reset the setting for the external shell command
    shell_prefs_get();
    shell_settings_write();
    widget_browser->hposition(0);
    widget_browser->position(0);
  }
  selection_changed(0);
  widget_browser->redraw();
}

// update a string member:
// replace a string pointer with new value, strips leading/trailing blanks:
int storestring(const char *n, const char * & p, int nostrip) {
  if (n == p) return 0;
  undo_checkpoint();
  int length = 0;
  if (n) { // see if blank, strip leading & trailing blanks
    if (!nostrip) while (isspace((int)(unsigned char)*n)) n++;
    const char *e = n + strlen(n);
    if (!nostrip) while (e > n && isspace((int)(unsigned char)*(e-1))) e--;
    length = int(e-n);
    if (!length) n = 0;
  }
  if (n == p) return 0;
  if (n && p && !strncmp(n,p,length) && !p[length]) return 0;
  if (p) free((void *)p);
  if (!n || !*n) {
    p = 0;
  } else {
    char *q = (char *)malloc(length+1);
    strlcpy(q,n,length+1);
    p = q;
  }
  set_modflag(1);
  return 1;
}

void fixvisible(Fl_Type *p) {
  Fl_Type *t = p;
  for (;;) {
    if (t->parent) t->visible = t->parent->visible && t->parent->open_;
    else t->visible = 1;
    t = t->next;
    if (!t || t->level <= p->level) break;
  }
}

// ---- implemenation of Fl_Type

Fl_Type::Fl_Type() {
  factory = 0;
  parent = 0;
  next = prev = 0;
  selected = new_selected = 0;
  visible = 0;
  name_ = 0;
  label_ = 0;
  user_data_ = 0;
  user_data_type_ = 0;
  callback_ = 0;
  comment_ = 0;
  rtti = 0;
  level = 0;
  code_position = header_position = -1;
  code_position_end = header_position_end = -1;
}

Fl_Type::~Fl_Type() {
  // warning: destructor only works for widgets that have been add()ed.
  if (widget_browser) widget_browser->deleting(this);
  if (prev) prev->next = next; else first = next;
  if (next) next->prev = prev; else last = prev;
  if (current == this) current = 0;
  if (parent) parent->remove_child(this);
  if (name_) free((void*)name_);
  if (label_) free((void*)label_);
  if (callback_) free((void*)callback_);
  if (user_data_) free((void*)user_data_);
  if (user_data_type_) free((void*)user_data_type_);
  if (comment_) free((void*)comment_);
}

// Generate a descriptive text for this item, to put in browser & window titles
const char* Fl_Type::title() {
  const char* c = name();
  if (c)
    return c;
  return type_name();
}

/**
 Add this list/tree of widgets as a new child of p.

 \c this is not part of the widget browser. \c p should be in the
 widget_browser, so \c Fl_Type::first and \c Fl_Type::last are valid for \c p.

 This methods updates the widget_browser.

 \param[in] p insert \c this tree as a child of \c p
 */
void Fl_Type::add(Fl_Type *p) {
  if (p && parent == p) return;
  undo_checkpoint();
  parent = p;
  // p is not in the Widget_Browser, so we must run the linked list to find the last entry
  Fl_Type *end = this;
  while (end->next) end = end->next;
  // run the list again to set the future node levels
  Fl_Type *q;
  int newlevel;
  if (p) {
    for (q = p->next; q && q->level > p->level; q = q->next) {/*empty*/}
    newlevel = p->level+1;
  } else {
    q = 0;
    newlevel = 0;
  }
  for (Fl_Type *t = this->next; t; t = t->next) t->level += (newlevel-level);
  level = newlevel;
  // now link this tree after p
  if (q) {
    prev = q->prev;
    prev->next = this;
    q->prev = end;
    end->next = q;
  } else if (first) {
    prev = last;
    prev->next = this;
    end->next = 0;
    last = end;
  } else {
    first = this;
    last = end;
    prev = end->next = 0;
  }
  // tell this that it was added, so it can update itself
  if (p) p->add_child(this,0);
  open_ = 1;
  fixvisible(this);
  set_modflag(1);
  // run the p tree a last time to make sure the widget_browser updates correctly
  Fl_Type *a = p;
  for (Fl_Type *t = this; t && a != end; a = t, t = t->next)
    widget_browser->inserting(a, t);
  widget_browser->redraw();
}

/**
 Add this list/tree of widgets as a new sibling before p.

 \c this is not part of the widget browser. \c g should be in the
 widget_browser, so \c Fl_Type::first and \c Fl_Type::last are valid for \c p.

 This methods updates the widget_browser.

 \param[in] g insert \c this tree as a child of \c p
 */
void Fl_Type::insert(Fl_Type *g) {
  // p is not in the Widget_Browser, so we must run the linked list to find the last entry
  Fl_Type *end = this;
  while (end->next) end = end->next;
  // this wil get the sam parent as g
  parent = g->parent;
  // run the list again to set the future node levels
  int newlevel = g->level;
  visible = g->visible;
  for (Fl_Type *t = this->next; t; t = t->next) t->level += newlevel-level;
  level = newlevel;
  // insert this in the list before g
  prev = g->prev;
  if (prev) prev->next = this; else first = this;
  end->next = g;
  g->prev = end;
  fixvisible(this);
  // tell parent that it has a new child, so it can update itself
  if (parent) parent->add_child(this, g);
  // run this tree a last time to make sure the widget_browser updates correctly
  Fl_Type *a = prev;
  for (Fl_Type *t = this; t && a != end; a = t, t = t->next)
    if (a)
      widget_browser->inserting(a, t);
  widget_browser->redraw();
}

// Return message number for I18N...
int Fl_Type::msgnum() {
  int           count;
  Fl_Type       *p;

  for (count = 0, p = this; p;) {
    if (p->label()) count ++;
    if (p != this && p->is_widget() && ((Fl_Widget_Type *)p)->tooltip()) count ++;

    if (p->prev) p = p->prev;
    else p = p->parent;
  }

  return count;
}

/**
 Remove this node and all its children from the parent node.

 This does not delete anything. The resulting list//tree will no longer be in
 the widget_browser, so \c Fl_Type::first and \c Fl_Type::last do not apply
 to it.

 \return the node that follows this node after the operation; can be NULL
 */
Fl_Type *Fl_Type::remove() {
  // find the last child of this node
  Fl_Type *end = this;
  for (;;) {
    if (!end->next || end->next->level <= level)
      break;
    end = end->next;
  }
  // unlink this node from the previous one
  if (prev)
    prev->next = end->next;
  else
    first = end->next;
  // unlink the last child from their next node
  if (end->next)
    end->next->prev = prev;
  else
    last = prev;
  Fl_Type *r = end->next;
  prev = end->next = 0;
  // allow the parent to update changes in the UI
  if (parent) parent->remove_child(this);
  parent = 0;
  // tell the widget_browser that we removed some nodes
  for (Fl_Type *t = this; t; t = t->next)
    widget_browser->deleting(t);
  widget_browser->redraw();
  selection_changed(0);
  return r;
}

void Fl_Type::name(const char *n) {
  int nostrip = is_comment();
  if (storestring(n,name_,nostrip)) {
    if (visible) widget_browser->redraw();
  }
}

void Fl_Type::label(const char *n) {
  if (storestring(n,label_,1)) {
    setlabel(label_);
    if (visible && !name_) widget_browser->redraw();
  }
}

void Fl_Type::callback(const char *n) {
  storestring(n,callback_);
}

void Fl_Type::user_data(const char *n) {
  storestring(n,user_data_);
}

void Fl_Type::user_data_type(const char *n) {
  storestring(n,user_data_type_);
}

void Fl_Type::comment(const char *n) {
  if (storestring(n,comment_,1)) {
    if (visible) widget_browser->redraw();
  }
}

void Fl_Type::open() {
  printf("Open of '%s' is not yet implemented\n",type_name());
}

// returns pointer to whatever is after f & children

/**
 Move this node (and its children) into list before g.
 \param[in] g move \c this tree before \c g
 */
void Fl_Type::move_before(Fl_Type* g) {
  if (level != g->level) printf("move_before levels don't match! %d %d\n",
                                level, g->level);
  // Find the last child in the list
  Fl_Type* n;
  for (n = next; n && n->level > level; n = n->next) ;
  if (n == g) return;
  // Tell the widget browser that we delete them
  for (n = next; n && n->level > level; n = n->next)
    widget_browser->deleting(n);
  // now link this tree before g
  Fl_Type *l = n ? n->prev : Fl_Type::last;
  prev->next = n;
  if (n) n->prev = prev; else Fl_Type::last = prev;
  prev = g->prev;
  l->next = g;
  if (prev) prev->next = this; else Fl_Type::first = this;
  g->prev = l;
  // tell parent that it has a new child, so it can update itself
  if (parent && is_widget()) parent->move_child(this,g);
  // run this tree a last time to make sure the widget_browser updates correctly
  Fl_Type *a = prev;
  for (Fl_Type *t = this; t && a != n; a = t, t = t->next)
    if (a)
      widget_browser->inserting(a, t);
  widget_browser->display(this);
  widget_browser->redraw();
}


// write a widget and all its children:
void Fl_Type::write() {
    write_indent(level);
    write_word(type_name());

    if (is_class()) {
      const char * p =  ((Fl_Class_Type*)this)->prefix();
      if (p &&  strlen(p))
        write_word(p);
    }

    write_word(name());
    write_open(level);
    write_properties();
    write_close(level);
    if (!is_parent()) return;
    // now do children:
    write_open(level);
    Fl_Type *child;
    for (child = next; child && child->level > level; child = child->next)
        if (child->level == level+1) child->write();
    write_close(level);
}

void Fl_Type::write_properties() {
  // repeat this for each attribute:
  if (label()) {
    write_indent(level+1);
    write_word("label");
    write_word(label());
  }
  if (user_data()) {
    write_indent(level+1);
    write_word("user_data");
    write_word(user_data());
  }
  if (user_data_type()) {
    write_word("user_data_type");
    write_word(user_data_type());
  }
  if (callback()) {
    write_indent(level+1);
    write_word("callback");
    write_word(callback());
  }
  if (comment()) {
    write_indent(level+1);
    write_word("comment");
    write_word(comment());
  }
  if (is_parent() && open_) write_word("open");
  if (selected) write_word("selected");
}

void Fl_Type::read_property(const char *c) {
  if (!strcmp(c,"label"))
    label(read_word());
  else if (!strcmp(c,"user_data"))
    user_data(read_word());
  else if (!strcmp(c,"user_data_type"))
    user_data_type(read_word());
  else if (!strcmp(c,"callback"))
    callback(read_word());
  else if (!strcmp(c,"comment"))
    comment(read_word());
  else if (!strcmp(c,"open"))
    open_ = 1;
  else if (!strcmp(c,"selected"))
    select(this,1);
  else
    read_error("Unknown property \"%s\"", c);
}

int Fl_Type::read_fdesign(const char*, const char*) {return 0;}

/**
 Write a comment into the header file.
 \param[in] pre indent the comment by this string
*/
void Fl_Type::write_comment_h(const char *pre)
{
  if (comment() && *comment()) {
    write_h("%s/**\n", pre);
    const char *s = comment();
    write_h("%s ", pre);
    while(*s) {
      if (*s=='\n') {
        if (s[1]) {
          write_h("\n%s ", pre);
        }
      } else {
        write_h("%c", *s); // FIXME this is much too slow!
      }
      s++;
    }
    write_h("\n%s*/\n", pre);
  }
}

/**
  Write a comment into the source file.
*/
void Fl_Type::write_comment_c(const char *pre)
{
  if (comment() && *comment()) {
    write_c("%s/**\n", pre);
    const char *s = comment();
    write_c("%s ", pre);
    while(*s) {
      if (*s=='\n') {
        if (s[1]) {
          write_c("\n%s ", pre);
        }
      } else {
        write_c("%c", *s); // FIXME this is much too slow!
      }
      s++;
    }
    write_c("\n%s*/\n", pre);
  }
}

/**
  Write a comment into the source file.
*/
void Fl_Type::write_comment_inline_c(const char *pre)
{
  if (comment() && *comment()) {
    const char *s = comment();
    if (strchr(s, '\n')==0L) {
      // single line comment
      if (pre) write_c("%s", pre);
      write_c("// %s\n", s);
      if (!pre) write_c("%s", indent_plus(1));
    } else {
      write_c("%s/*\n", pre?pre:"");
      if (pre)
        write_c("%s ", pre);
      else
        write_c("%s ", indent_plus(1));
      while(*s) {
        if (*s=='\n') {
          if (s[1]) {
            if (pre)
              write_c("\n%s ", pre);
            else
              write_c("\n%s ", indent_plus(1));
          }
        } else {
          write_c("%c", *s); // FIXME this is much too slow!
        }
        s++;
      }
      if (pre)
        write_c("\n%s */\n", pre);
      else
        write_c("\n%s */\n", indent_plus(1));
      if (!pre)
        write_c("%s", indent_plus(1));
    }
  }
}

/**
  Build widgets and dataset needed in live mode.
  \return a widget pointer that the live mode initiator can 'show()'
  \see leave_live_mode()
*/
Fl_Widget *Fl_Type::enter_live_mode(int) {
  return 0L;
}

/**
  Release all resources created when entering live mode.
  \see enter_live_mode()
*/
void Fl_Type::leave_live_mode() {
}

/**
  Copy all needed properties for this type into the live object.
*/
void Fl_Type::copy_properties() {
}

/**
  Check whether callback \p cbname is declared anywhere else by the user.

  \b Warning: this just checks that the name is declared somewhere,
  but it should probably also check that the name corresponds to a
  plain function or a member function within the same class and that
  the parameter types match.
 */
int Fl_Type::user_defined(const char* cbname) const {
  for (Fl_Type* p = Fl_Type::first; p ; p = p->next)
    if (strcmp(p->type_name(), "Function") == 0 && p->name() != 0)
      if (strncmp(p->name(), cbname, strlen(cbname)) == 0)
        if (p->name()[strlen(cbname)] == '(')
          return 1;
  return 0;
}

const char *Fl_Type::callback_name() {
  if (is_name(callback())) return callback();
  return unique_id(this, "cb", name(), label());
}

const char* Fl_Type::class_name(const int need_nest) const {
  Fl_Type* p = parent;
  while (p) {
    if (p->is_class()) {
      // see if we are nested in another class, we must fully-qualify name:
      // this is lame but works...
      const char* q = 0;
      if(need_nest) q=p->class_name(need_nest);
      if (q) {
        static char s[256];
        if (q != s) strlcpy(s, q, sizeof(s));
        strlcat(s, "::", sizeof(s));
        strlcat(s, p->name(), sizeof(s));
        return s;
      }
      return p->name();
    }
    p = p->parent;
  }
  return 0;
}

/**
 If this Type resides inside a class, this function returns the class type, or null.
 */
const Fl_Class_Type *Fl_Type::is_in_class() const {
  Fl_Type* p = parent;
  while (p) {
    if (p->is_class()) {
      return (Fl_Class_Type*)p;
    }
    p = p->parent;
  }
  return 0;
}

void Fl_Type::write_static() {
}

void Fl_Type::write_code1() {
  write_h("// Header for %s\n", title());
  write_c("// Code for %s\n", title());
}

void Fl_Type::write_code2() {
}

