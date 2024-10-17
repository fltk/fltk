//
// Widget type code for the Fast Light Tool Kit (FLTK).
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

/// \defgroup fl_type Basic Node for all Widgets and Functions
/// \{

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

 The Fl_Type inheritance is currently:
      --+-- Fl_Type
        +-- Fl_Function_Type
        +-- Fl_Code_Type
        +-- Fl_CodeBlock_Type
        +-+ Fl_Decl_Type
        | +-- Fl_Data
        +-- Fl_DeclBlock_Type
        +-- Fl_Comment_Type
        +-- Fl_Class_Type
        +-+ Fl_Widget_Type, 'o' points to a class derived from Fl_Widget
          +-+ Fl_Browser_Base_Type, 'o' is Fl_Browser
          | +-+ Fl_Browser
          | | +-- Fl_File_Browser
          | +-- Fl_Check_Browser
          +-- Fl_Tree_Type
          +-- Fl_Help_View_Type
          +-+ Fl_Valuator_Type, 'o' is Fl_Valuator_
          | +-- Fl_Counter_Type
          | +-- Fl_Adjuster_Type
          | +-- Fl_Dial_Type
          | +-- Fl_Roller_Type
          | +-- Fl_Slider_Type
          | +-- Fl_Value_Input_Type
          | +-- Fl_Value_Output_Type
          +-+ Fl_Input_Type
          | +-- Fl_Output_Type
          +-+ Fl_Text_Display_Type
          | +-- Fl_Text_Editor_Type
          +-- Fl_Terminal_Type
          +-- Fl_Box_Type
          +-- Fl_Clock_Type
          +-- Fl_Progress_Type
          +-- Fl_Spinner_Type
          +-+ Fl_Group_Type
          | +-- Fl_Pack_Type
          | +-- Fl_Flex_Type
          | +-- Fl_Grid_Type
          | +-- Fl_Table_Type
          | +-- Fl_Tabs_Type
          | +-- Fl_Scroll_Type
          | +-- Fl_Tile_Type
          | +-- Fl_Wizard_Type
          | +-+ Fl_Window_Type
          |   +-- Fl_Widget_Class_Type
          +-+ Fl_Menu_Manager_Type, 'o' is based on Fl_Widget
          | +-+ Fl_Menu_Base_Type, 'o' is based on Fl_Menu_
          | | +-- Fl_Menu_Button_Type
          | | +-- Fl_Choice_Type
          | | +-- Fl_Menu_Bar_Type
          | +-- Fl_Input_Choice_Type, 'o' is based on Fl_Input_Choice which is Fl_Group
          +-+ Fl_Button_Type
            +-- Fl_Return_Button_Type
            +-- Fl_Repeat_Button_Type
            +-- Fl_Light_Button_Type
            +-- Fl_Check_Button_Type
            +-- Fl_Round_Button_Type
            +-+ Fl_Menu_Item_Type, 'o' is derived from Fl_Button in FLUID
              +-- Fl_Radio_Menu_Item_Type
              +-- Fl_Checkbox_Menu_Item_Type
              +-- Fl_Submenu_Item_Type

*/

#include "Fl_Type.h"

#include "fluid.h"
#include "Fd_Snap_Action.h"
#include "Fl_Function_Type.h"
#include "Fl_Widget_Type.h"
#include "Fl_Window_Type.h"
#include "Fl_Group_Type.h"
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
Fl_Type *Fl_Type::current = NULL;
Fl_Type *Fl_Type::current_dnd = NULL;
int Fl_Type::allow_layout = 0;

Fl_Type *in_this_only; // set if menu popped-up in window


// ---- various functions

#if 0
#ifndef NDEBUG
/**
 Print the current project tree to stderr.
 */
void print_project_tree() {
  fprintf(stderr, "---- %s --->\n", g_project.projectfile_name().c_str());
  for (Fl_Type *t = Fl_Type::first; t; t = t->next) {
    for (int i = t->level; i > 0; i--)
      fprintf(stderr, ". ");
    fprintf(stderr, "%s\n", subclassname(t));
  }
}
#endif

#ifndef NDEBUG
/**
 Check the validity of the project tree.

 Write problems with the project tree to stderr.

 \return true if the project tree is valid
 */
bool validate_project_tree() {
  // Validate `first` and `last`
  if (Fl_Type::first == NULL) {
    if (Fl_Type::last == NULL) {
      return true;
    } else {
      fprintf(stderr, "ERROR: `first` is NULL, but `last` is not!\n");
      return false;
    }
  }
  if (Fl_Type::last == NULL) {
    fprintf(stderr, "ERROR: `last` is NULL, but `first` is not!\n");
    return false;
  }
  // Validate the branch linkage, parent links, etc.
  return validate_branch(Fl_Type::first);
}
#endif

#ifndef NDEBUG
/**
 Check the validity of a Type branch that is not connected to the project.

 Write problems with the branch to stderr.

 \param[in] root the first node in a branch
 \return true if the branch is correctly separated and valid
 */
bool validate_independent_branch(class Fl_Type *root) {
  // Make sure that `first` and `last` do not point at any node in this branch
  if (Fl_Type::first) {
    for (Fl_Type *t = root; t; t = t->next) {
      if (Fl_Type::first == t) {
        fprintf(stderr, "ERROR: Branch is not independent, `first` is pointing to branch member!\n");
        return false;
      }
    }
  }
  if (Fl_Type::last) {
    for (Fl_Type *t = root; t; t = t->next) {
      if (Fl_Type::last == t) {
        fprintf(stderr, "ERROR: Branch is not independent, `last` is pointing to branch member!\n");
        return false;
      }
    }
  }
  // Validate the branch linkage, parent links, etc.
  return validate_branch(root);
}
#endif

#ifndef NDEBUG
/**
 Check the validity of a Type branch.

 Write problems with the branch to stderr.

 \param[in] root the first node in a branch
 \return true if the branch is valid
 */
bool validate_branch(class Fl_Type *root) {
  // Only check real branches
  if (!root) {
    fprintf(stderr, "WARNING: Branch is empty!\n");
    return false;
  }
  // Check relation between this and next node
  for (Fl_Type *t = root; t; t = t->next) {
    if (t->level < root->level) {
      fprintf(stderr, "ERROR: Node in tree is above root level!\n");
      return false;
    }
    if (t->next) {
      // Make sure that all `next` types have the `prev` member link back
      if (t->next->prev != t) {
        fprintf(stderr, "ERROR: Doubly linked list broken!\n");
        return false;
      }
      if (t->next->level > t->level) {
        // Validate `level` changes
        if (t->next->level - t->level > 1) {
          fprintf(stderr, "ERROR: Child level increment greater than one!\n");
          return false;
        }
        // Ensure that this node can actually have children
        if (!t->can_have_children()) {
          fprintf(stderr, "ERROR: This parent must not have children!\n");
          return false;
        }
      }
    }
    // Validate the `parent` entry
    for (Fl_Type *p = t->prev; ; p = p->prev) {
      if (p == NULL) {
        if (t->parent != NULL) {
          fprintf(stderr, "ERROR: `parent` pointer should be NULL!\n");
          return false;
        }
        break;
      }
      if (p->level < t->level) {
        if (t->parent != p) {
          fprintf(stderr, "ERROR: `parent` points to wrong parent!\n");
          return false;
        }
        break;
      }
    }
  }
  return true;
}
#endif
#endif

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

/**
 Callback to move all selected items before their previous unselected sibling.
 */
void earlier_cb(Fl_Widget*,void*) {
  Fl_Type *f;
  int mod = 0;
  for (f = Fl_Type::first; f; ) {
    Fl_Type* nxt = f->next;
    if (f->selected) {
      Fl_Type* g;
      for (g = f->prev; g && g->level > f->level; g = g->prev) {/*empty*/}
      if (g && g->level == f->level && !g->selected) {
        if (!mod) undo_checkpoint();
        f->move_before(g);
        if (f->parent) f->parent->layout_widget();
        mod = 1;
      }
    }
    f = nxt;
  }
  if (mod) set_modflag(1);
  widget_browser->display(Fl_Type::current);
  widget_browser->rebuild();
}

/**
 Callback to move all selected items after their next unselected sibling.
 */
void later_cb(Fl_Widget*,void*) {
  Fl_Type *f;
  int mod = 0;
  for (f = Fl_Type::last; f; ) {
    Fl_Type* prv = f->prev;
    if (f->selected) {
      Fl_Type* g;
      for (g = f->next; g && g->level > f->level; g = g->next) {/*empty*/}
      if (g && g->level == f->level && !g->selected) {
        if (!mod) undo_checkpoint();
        g->move_before(f);
        if (f->parent) f->parent->layout_widget();
        mod = 1;
      }
    }
    f = prv;
  }
  if (mod) set_modflag(1);
  widget_browser->display(Fl_Type::current);
  widget_browser->rebuild();
}

/** \brief Delete all children of a Type.
 */
static void delete_children(Fl_Type *p) {
  Fl_Type *f;
  // find all types following p that are higher in level, effectively finding
  // the last child of the last child
  for (f = p; f && f->next && f->next->level > p->level; f = f->next) {/*empty*/}
  // now loop back up to p, deleting all children on the way
  for (; f != p; ) {
    Fl_Type *g = f->prev;
    delete f;
    f = g;
  }
}

/** Delete all nodes in the Types tree and reset project settings, or delete selected nodes.
 Also calls the browser to refresh.
 \note Please refactor this into two separate methods of Fluid_Project.
 \param[in] selected_only if set, delete only the selected widgets and
 don't reset the project.
 */
void delete_all(int selected_only) {
  for (Fl_Type *f = Fl_Type::first; f;) {
    if (f->selected || !selected_only) {
      delete_children(f);
      Fl_Type *g = f->next;
      delete f;
      f = g;
    } else {
      f = f->next;
    }
  }
  if(!selected_only) {
    // reset the setting for the external shell command
    if (g_shell_config) {
      g_shell_config->clear(FD_STORE_PROJECT);
      g_shell_config->rebuild_shell_menu();
      g_shell_config->update_settings_dialog();
    }
    widget_browser->hposition(0);
    widget_browser->vposition(0);
    g_layout_list.remove_all(FD_STORE_PROJECT);
    g_layout_list.current_suite(0);
    g_layout_list.current_preset(0);
    g_layout_list.update_dialogs();
  }
  selection_changed(0);
  widget_browser->redraw();
}

/** Update a string.
 Replace a string pointer with new value, strips leading/trailing blanks.
 As a side effect, this call also sets the mod flags.
 \param[in] n new string, can be NULL
 \param[out] p update this pointer, possibly reallocate memory
 \param[in] nostrip if set, do not strip leading and trailing spaces and tabs
 \return 1 if the string in p changed
 */
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

/** Update the `visible` flag for `p` and all its descendants.
 \param[in] p start here and update all descendants
 */
void update_visibility_flag(Fl_Type *p) {
  Fl_Type *t = p;
  for (;;) {
    if (t->parent) t->visible = t->parent->visible && !t->parent->folded_;
    else t->visible = 1;
    t = t->next;
    if (!t || t->level <= p->level) break;
  }
}

// ---- implementation of Fl_Type

/** \var Fl_Type *Fl_Type::parent
 Link to the parent node in the tree structure.
 Used for simulating a tree structure via a doubly linked list.
 */
/** \var Fl_Type *Fl_Type::level
 Zero based depth of the node within the tree structure.
 Level is used to emulate a tree structure: the first node with a lower
 level in the prev list would be the parent of this node. If the next member
 has a higher level value, it is this nodes first child. At the same level,
 it would be the first sibling.
 */
/** \var Fl_Type *Fl_Type::next
 Points to the next node in the doubly linked list.
 If this is NULL, we are at the end of the list.
 Used for simulating a tree structure via a doubly linked list.
 */
/** \var Fl_Type *Fl_Type::prev
 Link to the next node in the tree structure.
 If this is NULL, we are at the beginning of the list.
 Used for simulating a tree structure via a doubly linked list.
 */

/**
 Constructor and base for any node in the widget tree.
 */
Fl_Type::Fl_Type() :
  name_(NULL),
  label_(NULL),
  callback_(NULL),
  user_data_(NULL),
  user_data_type_(NULL),
  comment_(NULL),
  uid_(0),
  parent(NULL),
  new_selected(0),
  selected(0),
  folded_(0),
  visible(0),
  level(0),
  next(NULL), prev(NULL),
  factory(NULL),
  code_static_start(-1), code_static_end(-1),
  code1_start(-1), code1_end(-1),
  code2_start(-1), code2_end(-1),
  header1_start(-1), header1_end(-1),
  header2_start(-1), header2_end(-1),
  header_static_start(-1), header_static_end(-1),
  proj1_start(-1), proj1_end(-1),
  proj2_start(-1), proj2_end(-1)
{
}


/**
 Destructor for any node in the tree.

 The destructor removes itself from the doubly linked list. This is dangerous,
 because the node does not know if it is part of the widget tree, or if it is
 in a separate tree. We try to take care of that as well as possible.
 */
Fl_Type::~Fl_Type() {
  // warning: destructor only works for widgets that have been add()ed.
  if (prev) prev->next = next; // else first = next; // don't do that! The Type may not be part of the main list
  if (next) next->prev = prev; // else last = prev;
  if (Fl_Type::last == this) Fl_Type::last = prev;
  if (Fl_Type::first == this) Fl_Type::first = next;
  if (current == this) current = NULL;
  if (parent) parent->remove_child(this);
  if (name_) free((void*)name_);
  if (label_) free((void*)label_);
  if (callback_) free((void*)callback_);
  if (user_data_) free((void*)user_data_);
  if (user_data_type_) free((void*)user_data_type_);
  if (comment_) free((void*)comment_);
}

// Return the previous sibling in the tree structure or NULL.
Fl_Type *Fl_Type::prev_sibling() {
  Fl_Type *n;
  for (n = prev; n && n->level > level; n = n->prev) ;
  if (n && (n->level == level))
    return n;
  return 0;
}

// Return the next sibling in the tree structure or NULL.
Fl_Type *Fl_Type::next_sibling() {
  Fl_Type *n;
  for (n = next; n && n->level > level; n = n->next) ;
  if (n && (n->level == level))
    return n;
  return 0;
}

// Return the first child or NULL
Fl_Type *Fl_Type::first_child() {
  Fl_Type *n = next;
  if (n->level > level)
    return n;
  return NULL;
}

// Generate a descriptive text for this item, to put in browser & window titles
const char* Fl_Type::title() {
  const char* c = name();
  if (c)
    return c;
  return type_name();
}

/**
 Return the window that contains this widget.
 \return NULL if this is not a widget.
 */
Fl_Window_Type *Fl_Type::window() {
  if (!is_widget())
    return NULL;
  for (Fl_Type *t = this; t; t=t->parent)
    if (t->is_a(ID_Window))
      return (Fl_Window_Type*)t;
  return NULL;
}

/**
 Return the group that contains this widget.
 \return NULL if this is not a widget.
 */
Fl_Group_Type *Fl_Type::group() {
  if (!is_widget())
    return NULL;
  for (Fl_Type *t = this; t; t=t->parent)
    if (t->is_a(ID_Group))
      return (Fl_Group_Type*)t;
  return NULL;
}

/**
 Add this list/tree of widgets as a new last child of p.

 \c this must not be part of the widget browser. \c p however must be in the
 widget_browser, so \c Fl_Type::first and \c Fl_Type::last are valid for \c p.

 This methods updates the widget_browser.

 \param[in] p insert \c this tree as a child of \c p
 \param[in] strategy is kAddAsLastChild or kAddAfterCurrent
 */
void Fl_Type::add(Fl_Type *anchor, Strategy strategy) {
#if 0
#ifndef NDEBUG
  // print_project_tree();
  // fprintf(stderr, "Validating project\n");
  validate_project_tree();
  // fprintf(stderr, "Validating branch\n");
  validate_independent_branch(this);
#endif
#endif

  Fl_Type *target = NULL; // insert self before target node, if NULL, insert last
  Fl_Type *target_parent = NULL; // this will be the new parent for branch
  int target_level = 0;   // adjust self to this new level

  // Find the node after our insertion position
  switch (strategy) {
    case kAddAsFirstChild:
      if (anchor == NULL) {
        target = Fl_Type::first;
      } else {
        target = anchor->next;
        target_level = anchor->level + 1;
        target_parent = anchor;
      }
      break;
    case kAddAsLastChild:
      if (anchor == NULL) {
        /* empty */
      } else {
        for (target = anchor->next; target && target->level > anchor->level; target = target->next) {/*empty*/}
        target_level = anchor->level + 1;
        target_parent = anchor;
      }
      break;
    case kAddAfterCurrent:
      if (anchor == NULL) {
        target = Fl_Type::first;
      } else {
        for (target = anchor->next; target && target->level > anchor->level; target = target->next) {/*empty*/}
        target_level = anchor->level;
        target_parent = anchor->parent;
      }
      break;
  }


  // Find the last node of our tree
  Fl_Type *end = this;
  while (end->next) end = end->next;

  // Everything is prepared, now insert ourself in front of the target node
  undo_checkpoint();

  // Walk the tree to update parent pointers and levels
  int source_level = level;
  for (Fl_Type *t = this; t; t = t->next) {
    t->level += (target_level-source_level);
    if (t->level == target_level)
      t->parent = target_parent;
  }

  // Now link ourselves and our children before 'target', or last, if 'target' is NULL
  if (target) {
    prev = target->prev;
    target->prev = end;
    end->next = target;
  } else {
    prev = Fl_Type::last;
    end->next = NULL;
    Fl_Type::last = end;
  }
  if (prev) {
    prev->next = this;
  } else {
    Fl_Type::first = this;
  }

#if 0
  { // make sure that we have no duplicate uid's
    Fl_Type *tp = this;
    do {
      tp->set_uid(tp->uid_);
      tp = tp->next;
    } while (tp!=end && tp!=NULL);
  }
#endif

  // Give the widgets in our tree a chance to update themselves
  for (Fl_Type *t = this; t && t!=end->next; t = t->next) {
    if (target_parent && (t->level == target_level))
      target_parent->add_child(t, 0);
    update_visibility_flag(t);
  }

  set_modflag(1);
  widget_browser->redraw();

#if 0
#ifndef NDEBUG
  // fprintf(stderr, "Validating project after adding branch\n");
  validate_project_tree();
#endif
#endif
}

/**
 Add `this` list/tree of widgets as a new sibling before `g`.

 `This` is not part of the widget browser. `g` must be in the
 widget_browser, so `Fl_Type::first` and `Fl_Type::last` are valid for `g .

 This methods updates the widget_browser.

 \param[in] g pointer to a node within the tree
 */
void Fl_Type::insert(Fl_Type *g) {
  // 'this' is not in the Widget_Browser, so we must run the linked list to find the last entry
  Fl_Type *end = this;
  while (end->next) end = end->next;
  // 'this' will get the same parent as 'g'
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
  update_visibility_flag(this);
  { // make sure that we have no duplicate uid's
    Fl_Type *tp = this;
    do {
      tp->set_uid(tp->uid_);
      tp = tp->next;
    } while (tp!=end && tp!=NULL);
  }
  // tell parent that it has a new child, so it can update itself
  if (parent) parent->add_child(this, g);
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
  widget_browser->redraw();
  selection_changed(0);
  return r;
}

void Fl_Type::name(const char *n) {
  int nostrip = is_a(ID_Comment);
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
 Both `this` and `g` must be in the widget browser.
 The caller must make sure that the widget browser is rebuilt correctly.
 \param[in] g move \c this tree before \c g
 */
void Fl_Type::move_before(Fl_Type* g) {
  if (level != g->level) printf("move_before levels don't match! %d %d\n",
                                level, g->level);
  // Find the last child in the list
  Fl_Type *n;
  for (n = next; n && n->level > level; n = n->next) ;
  if (n == g) return;
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
}


// write a widget and all its children:
void Fl_Type::write(Fd_Project_Writer &f) {
  if (f.write_codeview()) proj1_start = (int)ftell(f.file()) + 1;
  if (f.write_codeview()) proj2_start = (int)ftell(f.file()) + 1;
  f.write_indent(level);
  f.write_word(type_name());

  if (is_class()) {
    const char * p =  ((Fl_Class_Type*)this)->prefix();
    if (p &&  strlen(p))
      f.write_word(p);
  }

  f.write_word(name());
  f.write_open();
  write_properties(f);
  if (parent) parent->write_parent_properties(f, this, true);
  f.write_close(level);
  if (f.write_codeview()) proj1_end = (int)ftell(f.file());
  if (!can_have_children()) {
    if (f.write_codeview()) proj2_end = (int)ftell(f.file());
    return;
  }
  // now do children:
  f.write_open();
  Fl_Type *child;
  for (child = next; child && child->level > level; child = child->next)
    if (child->level == level+1) child->write(f);
  if (f.write_codeview()) proj2_start = (int)ftell(f.file()) + 1;
  f.write_close(level);
  if (f.write_codeview()) proj2_end = (int)ftell(f.file());
}

void Fl_Type::write_properties(Fd_Project_Writer &f) {
  // repeat this for each attribute:
  if (g_project.write_mergeback_data && uid_) {
    f.write_word("uid");
    f.write_string("%04x", uid_);
  }
  if (label()) {
    f.write_indent(level+1);
    f.write_word("label");
    f.write_word(label());
  }
  if (user_data()) {
    f.write_indent(level+1);
    f.write_word("user_data");
    f.write_word(user_data());
  }
  if (user_data_type()) {
    f.write_word("user_data_type");
    f.write_word(user_data_type());
  }
  if (callback()) {
    f.write_indent(level+1);
    f.write_word("callback");
    f.write_word(callback());
  }
  if (comment()) {
    f.write_indent(level+1);
    f.write_word("comment");
    f.write_word(comment());
  }
  if (can_have_children() && !folded_) f.write_word("open");
  if (selected) f.write_word("selected");
}

void Fl_Type::read_property(Fd_Project_Reader &f, const char *c) {
  if (!strcmp(c,"uid")) {
    const char *hex = f.read_word();
    int x = 0;
    if (hex)
      x = sscanf(hex, "%04x", &x);
    set_uid(x);
  } else if (!strcmp(c,"label"))
    label(f.read_word());
  else if (!strcmp(c,"user_data"))
    user_data(f.read_word());
  else if (!strcmp(c,"user_data_type"))
    user_data_type(f.read_word());
  else if (!strcmp(c,"callback"))
    callback(f.read_word());
  else if (!strcmp(c,"comment"))
    comment(f.read_word());
  else if (!strcmp(c,"open"))
    folded_ = 0;
  else if (!strcmp(c,"selected"))
    select(this,1);
  else if (!strcmp(c,"parent_properties"))
    if (parent) {
      const char *cc = f.read_word(1);
      if (strcmp(cc, "{")==0) {
        for (;;) {
          cc = f.read_word();
          if (!cc || cc[0]==0 || strcmp(cc, "}")==0) break;
          parent->read_parent_property(f, this, cc);
        }
      } else {
        f.read_error("'parent_properties' must be followed by '{'");
      }
    } else {
      f.read_error("Types using 'parent_properties' must have a parent");
      f.read_word();  // skip the entire block (this should generate a warning)
    }
  else
    f.read_error("Unknown property \"%s\"", c);
}

/** Write parent properties into the child property list.

 Some widgets store information for every child they manage. For example,
 Fl_Grid stores the row and column position of every child. This method stores
 this information with the child, but it is read and written by the parent.

 Parent properties solve several issues. A child will keep parent properties
 if copied from on grid into another. The parent does not have to keep lists
 of properties that may diverge from the actual order or number of children.
 And lastly, properties are read when they are actually needed and don't have
 to be stored in some temporary array.

 Parent properties are written as their own block at the end of the child's
 property list. The block starts with the `parent_properties` keyword, followed
 by a list of property/value pairs. The order of properties is significant,
 however individual properties can be left out.

 To avoid writing the `parent_properties` block unnecessarily, this method
 should only generate it if `encapsulate` is set *and* the contained
 properties are not at their default.

 Lastly, this method should call the super class to give it a chance to append
 its own properties.

 \see Fl_Grid_Type::write_parent_properties(Fd_Project_Writer &f, Fl_Type *child, bool encapsulate)

 \param[in] f the project file writer
 \param[in] child write properties for this child, make sure it has the correct type
 \param[in] encapsulate write the `parent_properties {}` block if true before writing any properties
 */
void Fl_Type::write_parent_properties(Fd_Project_Writer &f, Fl_Type *child, bool encapsulate) {
  (void)f; (void)child; (void)encapsulate;
  // nothing to do here
  // put the following code into your implementation of write_parent_properties
  // if there are actual non-default properties to write
  //  if (encapsulate) {
  //    f.write_indent(level+2);
  //    f.write_string("parent_properties {");
  //  }
  // now write your properties as name/value pairs
  //  f.write_indent(level+3);
  //  f.write_string("location {%d %d}", cell->row(), cell->col());
  // give the super class a chance to write its properties as well
  //  super::write_parent_properties(f, child, false);
  // close the encapsulation
  //  if (encapsulate) {
  //    f.write_indent(level+2);
  //    f.write_string("}");
  //  }
}

/** Read one parent per-child property.

 A parent widget can store properties for every child that it manages. This
 method reads back those properties. This function is virtual, so if a Type
 does not support a property, it will propagate to its super class.

 \see Fl_Type::write_parent_properties(Fd_Project_Writer &f, Fl_Type *child, bool encapsulate)
 \see Fl_Grid_Type::read_parent_property(Fd_Project_Reader &f, Fl_Type *child, const char *property)

 \param[in] f the project file writer
 \param[in] child read properties for this child
 \param[in] property the name of a property, or "}" when we reach the end of the list
 */
void Fl_Type::read_parent_property(Fd_Project_Reader &f, Fl_Type *child, const char *property) {
  (void)child;
  f.read_error("Unknown parent property \"%s\"", property);
}


int Fl_Type::read_fdesign(const char*, const char*) {return 0;}

/**
 Write a comment into the header file.
 \param[in] pre indent the comment by this string
*/
void Fl_Type::write_comment_h(Fd_Code_Writer& f, const char *pre)
{
  if (comment() && *comment()) {
    f.write_h("%s/**\n", pre);
    const char *s = comment();
    f.write_h("%s ", pre);
    while(*s) {
      if (*s=='\n') {
        if (s[1]) {
          f.write_h("\n%s ", pre);
        }
      } else {
        f.write_h("%c", *s); // FIXME this is much too slow!
      }
      s++;
    }
    f.write_h("\n%s*/\n", pre);
  }
}

/**
  Write a comment into the source file.
*/
void Fl_Type::write_comment_c(Fd_Code_Writer& f, const char *pre)
{
  if (comment() && *comment()) {
    f.write_c("%s/**\n", pre);
    const char *s = comment();
    f.write_c("%s ", pre);
    while(*s) {
      if (*s=='\n') {
        if (s[1]) {
          f.write_c("\n%s ", pre);
        }
      } else {
        f.write_c("%c", *s); // FIXME this is much too slow!
      }
      s++;
    }
    f.write_c("\n%s*/\n", pre);
  }
}

/**
  Write a comment into the source file.
*/
void Fl_Type::write_comment_inline_c(Fd_Code_Writer& f, const char *pre)
{
  if (comment() && *comment()) {
    const char *s = comment();
    if (strchr(s, '\n')==0L) {
      // single line comment
      if (pre) f.write_c("%s", pre);
      f.write_c("// %s\n", s);
      if (!pre) f.write_c("%s", f.indent_plus(1));
    } else {
      f.write_c("%s/*\n", pre?pre:"");
      if (pre)
        f.write_c("%s ", pre);
      else
        f.write_c("%s ", f.indent_plus(1));
      while(*s) {
        if (*s=='\n') {
          if (s[1]) {
            if (pre)
              f.write_c("\n%s ", pre);
            else
              f.write_c("\n%s ", f.indent_plus(1));
          }
        } else {
          f.write_c("%c", *s); // FIXME this is much too slow!
        }
        s++;
      }
      if (pre)
        f.write_c("\n%s */\n", pre);
      else
        f.write_c("\n%s */\n", f.indent_plus(1));
      if (!pre)
        f.write_c("%s", f.indent_plus(1));
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
    if (p->is_a(ID_Function) && p->name() != 0)
      if (strncmp(p->name(), cbname, strlen(cbname)) == 0)
        if (p->name()[strlen(cbname)] == '(')
          return 1;
  return 0;
}

const char *Fl_Type::callback_name(Fd_Code_Writer& f) {
  if (is_name(callback())) return callback();
  return f.unique_id(this, "cb", name(), label());
}

/**
 \brief Return the class name if this type is inside a Class or Widget Class.

 This methods traverses up the hirarchy to find out if this Type is located
 inside a Class or Widget Class. It then return the name of that class. If
 need_nest is set, class_name searches all the way up the tree and concatenates
 the names of classes within classes, separated by a "::".

 \param need_nest if clear, search up one level to the first enclosing class.
 If set, recurse all the way up to the top node.
 \return the name of the enclosing class, or names of the enclosing classes
 in a static buffe (don't call free), or NULL if this Type is not inside a class
 */
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
 Check if this is inside a Fl_Class_Type or Fl_Widget_Class_Type.
 \return true if any of the parents is Fl_Class_Type or Fl_Widget_Class_Type
 */
bool Fl_Type::is_in_class() const {
  Fl_Type* p = parent;
  while (p) {
    if (p->is_class()) return true;
    p = p->parent;
  }
  return false;
}

void Fl_Type::write_static(Fd_Code_Writer&) {
}

void Fl_Type::write_static_after(Fd_Code_Writer&) {
}

void Fl_Type::write_code1(Fd_Code_Writer& f) {
  f.write_h("// Header for %s\n", title());
  f.write_c("// Code for %s\n", title());
}

void Fl_Type::write_code2(Fd_Code_Writer&) {
}

/** Set a uid that is unique within the project.

 Try to set the given id as the unique id for this node. If the suggested id
 is 0, or it is already taken inside this project, we try another random id
 until we find one that is unique.

 \param[in] suggested_uid the preferred uid for this node
 \return the actualt uid that was given to the node
 */
unsigned short Fl_Type::set_uid(unsigned short suggested_uid) {
  if (suggested_uid==0)
    suggested_uid = (unsigned short)rand();
  for (;;) {
    Fl_Type *tp = Fl_Type::first;
    for ( ; tp; tp = tp->next)
      if (tp!=this && tp->uid_==suggested_uid)
        break;
    if (tp==NULL)
      break;
    suggested_uid = (unsigned short)rand();
  }
  uid_ = suggested_uid;
  return suggested_uid;
}

/** Find a node by its unique id.

 Every node in a type tree has an id that is unique for the current project.
 Walk the tree and return the node with this uid.

 \param[in] uid any number between 0 and 65535
 \return the node with this uid, or NULL if not found
 */
Fl_Type *Fl_Type::find_by_uid(unsigned short uid) {
  for (Fl_Type *tp = Fl_Type::first; tp; tp = tp->next) {
    if (tp->uid_ == uid) return tp;
  }
  return NULL;
}

/** Find a type node by using the codeview text positions.

 \param[in] text_type 0=source file, 1=header, 2=.fl project file
 \param[in] crsr cursor position in text
 \return the node we found or NULL
 */
Fl_Type *Fl_Type::find_in_text(int text_type, int crsr) {
  for (Fl_Type *node = first; node; node = node->next) {
    switch (text_type) {
      case 0:
        if (crsr >= node->code1_start && crsr < node->code1_end) return node;
        if (crsr >= node->code2_start && crsr < node->code2_end) return node;
        if (crsr >= node->code_static_start && crsr < node->code_static_end) return node;
        break;
      case 1:
        if (crsr >= node->header1_start && crsr < node->header1_end) return node;
        if (crsr >= node->header2_start && crsr < node->header2_end) return node;
        if (crsr >= node->header_static_start && crsr < node->header_static_end) return node;
        break;
      case 2:
        if (crsr >= node->proj1_start && crsr < node->proj1_end) return node;
        if (crsr >= node->proj2_start && crsr < node->proj2_end) return node;
        break;
    }
  }
  return 0;
}

/// \}

