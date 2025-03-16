//
// Node base class code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

/** \class Node
 Each object described by Fluid is one of these objects.  They
 are all stored in a double-linked list.

 The "type" of the object is covered by the virtual functions.
 There will probably be a lot of these virtual functions.

 The type browser is also a list of these objects, but they
 are "factory" instances, not "real" ones.  These objects exist
 only so the "make" method can be called on them.  They are
 not in the linked list and are not written to files or
 copied or otherwise examined.

 The Node inheritance is currently:
      --+-- Node
        +-- Function_Node
        +-- Code_Node
        +-- CodeBlock_Node
        +-+ Decl_Node
        | +-- Fl_Data
        +-- DeclBlock_Node
        +-- Comment_Node
        +-- Class_Node
        +-+ Widget_Node, 'o' points to a class derived from Fl_Widget
          +-+ Browser_Base_Node, 'o' is Fl_Browser
          | +-+ Fl_Browser
          | | +-- Fl_File_Browser
          | +-- Fl_Check_Browser
          +-- Tree_Node
          +-- Help_View_Node
          +-+ Valuator_Node, 'o' is Fl_Valuator_
          | +-- Counter_Node
          | +-- Adjuster_Node
          | +-- Dial_Node
          | +-- Roller_Node
          | +-- Slider_Node
          | +-- Value_Input_Node
          | +-- Value_Output_Node
          +-+ Input_Node
          | +-- Output_Node
          +-+ Text_Display_Node
          | +-- Text_Editor_Node
          +-- Terminal_Node
          +-- Box_Node
          +-- Clock_Node
          +-- Progress_Node
          +-- Spinner_Node
          +-+ Group_Node
          | +-- Pack_Node
          | +-- Flex_Node
          | +-- Grid_Node
          | +-- Table_Node
          | +-- Tabs_Node
          | +-- Scroll_Node
          | +-- Tile_Node
          | +-- Wizard_Node
          | +-+ Window_Node
          |   +-- Widget_Class_Node
          +-+ Menu_Manager_Node, 'o' is based on Fl_Widget
          | +-+ Menu_Base_Node, 'o' is based on Fl_Menu_
          | | +-- Menu_Button_Node
          | | +-- Choice_Node
          | | +-- Menu_Bar_Node
          | +-- Input_Choice_Node, 'o' is based on Fl_Input_Choice which is Fl_Group
          +-+ Button_Node
            +-- Return_Button_Node
            +-- Repeat_Button_Node
            +-- Light_Button_Node
            +-- Check_Button_Node
            +-- Round_Button_Node
            +-+ Menu_Item_Node, 'o' is derived from Fl_Button in FLUID
              +-- Radio_Menu_Item_Node
              +-- Checkbox_Menu_Item_Node
              +-- Fl_Submenu_Item_Type

*/

#include "nodes/Node.h"

#include "Fluid.h"
#include "Project.h"
#include "app/Snap_Action.h"
#include "app/shell_command.h"
#include "proj/undo.h"
#include "io/Project_Reader.h"
#include "io/Project_Writer.h"
#include "io/Code_Writer.h"
#include "nodes/Function_Node.h"
#include "nodes/Widget_Node.h"
#include "nodes/Window_Node.h"
#include "nodes/Group_Node.h"
#include "rsrcs/pixmaps.h"
#include "widgets/Node_Browser.h"

#include <FL/Fl.H>
#include <FL/Fl_Browser_.H>
#include <FL/fl_draw.H>
#include "../src/flstring.h"

#include <stdlib.h>
#include <stdio.h>

// ---- global variables

Node *in_this_only; // set if menu popped-up in window


// ---- various functions

#if 0
#ifndef NDEBUG
/**
 Print the current project tree to stderr.
 */
void print_project_tree() {
  fprintf(stderr, "---- %s --->\n", Fluid.proj.projectfile_name().c_str());
  for (Node *t = Fluid.proj.tree.first; t; t = t->next) {
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
  if (Fluid.proj.tree.first == nullptr) {
    if (Fluid.proj.tree.last == nullptr) {
      return true;
    } else {
      fprintf(stderr, "ERROR: `first` is nullptr, but `last` is not!\n");
      return false;
    }
  }
  if (Fluid.proj.tree.last == nullptr) {
    fprintf(stderr, "ERROR: `last` is nullptr, but `first` is not!\n");
    return false;
  }
  // Validate the branch linkage, parent links, etc.
  return validate_branch(Fluid.proj.tree.first);
}
#endif

#ifndef NDEBUG
/**
 Check the validity of a Type branch that is not connected to the project.

 Write problems with the branch to stderr.

 \param[in] root the first node in a branch
 \return true if the branch is correctly separated and valid
 */
bool validate_independent_branch(class Node *root) {
  // Make sure that `first` and `last` do not point at any node in this branch
  if (Fluid.proj.tree.first) {
    for (Node *t = root; t; t = t->next) {
      if (Fluid.proj.tree.first == t) {
        fprintf(stderr, "ERROR: Branch is not independent, `first` is pointing to branch member!\n");
        return false;
      }
    }
  }
  if (Fluid.proj.tree.last) {
    for (Node *t = root; t; t = t->next) {
      if (Fluid.proj.tree.last == t) {
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
bool validate_branch(class Node *root) {
  // Only check real branches
  if (!root) {
    fprintf(stderr, "WARNING: Branch is empty!\n");
    return false;
  }
  // Check relation between this and next node
  for (Node *t = root; t; t = t->next) {
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
    for (Node *p = t->prev; ; p = p->prev) {
      if (p == nullptr) {
        if (t->parent != nullptr) {
          fprintf(stderr, "ERROR: `parent` pointer should be nullptr!\n");
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
  Node *p = Fluid.proj.tree.current ? Fluid.proj.tree.current->parent : nullptr;
  if (in_this_only) {
    Node *t = p;
    for (; t && t != in_this_only; t = t->parent) {/*empty*/}
    if (t != in_this_only) p = in_this_only;
  }
  for (;;) {
    if (p) {
      int foundany = 0;
      for (Node *t = p->next; t && t->level>p->level; t = t->next) {
        if (!t->new_selected) {widget_browser->select(t,1,0); foundany = 1;}
      }
      if (foundany) break;
      p = p->parent;
    } else {
      for (Node *t = Fluid.proj.tree.first; t; t = t->next)
        widget_browser->select(t,1,0);
      break;
    }
  }
  selection_changed(p);
}

void select_none_cb(Fl_Widget *,void *) {
  Node *p = Fluid.proj.tree.current ? Fluid.proj.tree.current->parent : nullptr;
  if (in_this_only) {
    Node *t = p;
    for (; t && t != in_this_only; t = t->parent) {/*empty*/}
    if (t != in_this_only) p = in_this_only;
  }
  for (;;) {
    if (p) {
      int foundany = 0;
      for (Node *t = p->next; t && t->level>p->level; t = t->next) {
        if (t->new_selected) {widget_browser->select(t,0,0); foundany = 1;}
      }
      if (foundany) break;
      p = p->parent;
    } else {
      for (Node *t = Fluid.proj.tree.first; t; t = t->next)
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
  Node *f;
  int mod = 0;
  for (f = Fluid.proj.tree.first; f; ) {
    Node* nxt = f->next;
    if (f->selected) {
      Node* g;
      for (g = f->prev; g && g->level > f->level; g = g->prev) {/*empty*/}
      if (g && g->level == f->level && !g->selected) {
        if (!mod) Fluid.proj.undo.checkpoint();
        f->move_before(g);
        if (f->parent) f->parent->layout_widget();
        mod = 1;
      }
    }
    f = nxt;
  }
  if (mod) Fluid.proj.set_modflag(1);
  widget_browser->display(Fluid.proj.tree.current);
  widget_browser->rebuild();
}

/**
 Callback to move all selected items after their next unselected sibling.
 */
void later_cb(Fl_Widget*,void*) {
  Node *f;
  int mod = 0;
  for (f = Fluid.proj.tree.last; f; ) {
    Node* prv = f->prev;
    if (f->selected) {
      Node* g;
      for (g = f->next; g && g->level > f->level; g = g->next) {/*empty*/}
      if (g && g->level == f->level && !g->selected) {
        if (!mod) Fluid.proj.undo.checkpoint();
        g->move_before(f);
        if (f->parent) f->parent->layout_widget();
        mod = 1;
      }
    }
    f = prv;
  }
  if (mod) Fluid.proj.set_modflag(1);
  widget_browser->display(Fluid.proj.tree.current);
  widget_browser->rebuild();
}

/** \brief Delete all children of a Type.
 */
static void delete_children(Node *p) {
  Node *f;
  // find all types following p that are higher in level, effectively finding
  // the last child of the last child
  for (f = p; f && f->next && f->next->level > p->level; f = f->next) {/*empty*/}
  // now loop back up to p, deleting all children on the way
  for (; f != p; ) {
    Node *g = f->prev;
    delete f;
    f = g;
  }
}

/** Delete all nodes in the Types tree and reset project settings, or delete selected nodes.
 Also calls the browser to refresh.
 \note Please refactor this into two separate methods of Project.
 \param[in] selected_only if set, delete only the selected widgets and
 don't reset the project.
 */
void delete_all(int selected_only) {
  if (widget_browser) {
    if (selected_only)
      widget_browser->save_scroll_position();
    widget_browser->new_list();
  }
  for (Node *f = Fluid.proj.tree.first; f;) {
    if (f->selected || !selected_only) {
      delete_children(f);
      Node *g = f->next;
      delete f;
      f = g;
    } else {
      f = f->next;
    }
  }
  if(!selected_only) {
    // reset the setting for the external shell command
    if (g_shell_config) {
      g_shell_config->clear(fld::Tool_Store::PROJECT);
      g_shell_config->rebuild_shell_menu();
      g_shell_config->update_settings_dialog();
    }
    if (widget_browser) {
      widget_browser->hposition(0);
      widget_browser->vposition(0);
    }
    Fluid.layout_list.remove_all(fld::Tool_Store::PROJECT);
    Fluid.layout_list.current_suite(0);
    Fluid.layout_list.current_preset(0);
    Fluid.layout_list.update_dialogs();
  }
  selection_changed(nullptr);
  if (widget_browser) {
    if (selected_only)
      widget_browser->restore_scroll_position();
    widget_browser->rebuild();
  }
}

/** Update a string.
 Replace a string pointer with new value, strips leading/trailing blanks.
 As a side effect, this call also sets the mod flags.
 \param[in] n new string, can be nullptr
 \param[out] p update this pointer, possibly reallocate memory
 \param[in] nostrip if set, do not strip leading and trailing spaces and tabs
 \return 1 if the string in p changed
 */
int storestring(const char *n, const char * & p, int nostrip) {
  if (n == p) return 0;
  Fluid.proj.undo.checkpoint();
  int length = 0;
  if (n) { // see if blank, strip leading & trailing blanks
    if (!nostrip) while (isspace((int)(unsigned char)*n)) n++;
    const char *e = n + strlen(n);
    if (!nostrip) while (e > n && isspace((int)(unsigned char)*(e-1))) e--;
    length = int(e-n);
    if (!length) n = nullptr;
  }
  if (n == p) return 0;
  if (n && p && !strncmp(n,p,length) && !p[length]) return 0;
  if (p) free((void *)p);
  if (!n || !*n) {
    p = nullptr;
  } else {
    char *q = (char *)malloc(length+1);
    strlcpy(q,n,length+1);
    p = q;
  }
  Fluid.proj.set_modflag(1);
  return 1;
}

/** Update the `visible` flag for `p` and all its descendants.
 \param[in] p start here and update all descendants
 */
void update_visibility_flag(Node *p) {
  Node *t = p;
  for (;;) {
    if (t->parent) t->visible = t->parent->visible && !t->parent->folded_;
    else t->visible = 1;
    t = t->next;
    if (!t || t->level <= p->level) break;
  }
}

// ---- implementation of Node

/** \var Node *Node::parent
 Link to the parent node in the tree structure.
 Used for simulating a tree structure via a doubly linked list.
 */
/** \var Node *Node::level
 Zero based depth of the node within the tree structure.
 Level is used to emulate a tree structure: the first node with a lower
 level in the prev list would be the parent of this node. If the next member
 has a higher level value, it is this nodes first child. At the same level,
 it would be the first sibling.
 */
/** \var Node *Node::next
 Points to the next node in the doubly linked list.
 If this is nullptr, we are at the end of the list.
 Used for simulating a tree structure via a doubly linked list.
 */
/** \var Node *Node::prev
 Link to the next node in the tree structure.
 If this is nullptr, we are at the beginning of the list.
 Used for simulating a tree structure via a doubly linked list.
 */

/**
 Constructor and base for any node in the widget tree.
 */
Node::Node() :
  name_(nullptr),
  label_(nullptr),
  callback_(nullptr),
  user_data_(nullptr),
  user_data_type_(nullptr),
  comment_(nullptr),
  uid_(0),
  parent(nullptr),
  new_selected(0),
  selected(0),
  folded_(0),
  visible(0),
  level(0),
  next(nullptr), prev(nullptr),
  factory(nullptr),
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
Node::~Node() {
  // warning: destructor only works for widgets that have been add()ed.
  if (prev) prev->next = next; // else first = next; // don't do that! The Type may not be part of the main list
  if (next) next->prev = prev; // else last = prev;
  if (Fluid.proj.tree.last == this) Fluid.proj.tree.last = prev;
  if (Fluid.proj.tree.first == this) Fluid.proj.tree.first = next;
  if (Fluid.proj.tree.current == this) Fluid.proj.tree.current = nullptr;
  if (parent) parent->remove_child(this);
  if (name_) free((void*)name_);
  if (label_) free((void*)label_);
  if (callback_) free((void*)callback_);
  if (user_data_) free((void*)user_data_);
  if (user_data_type_) free((void*)user_data_type_);
  if (comment_) free((void*)comment_);
}

// Return the previous sibling in the tree structure or nullptr.
Node *Node::prev_sibling() {
  Node *n;
  for (n = prev; n && n->level > level; n = n->prev) ;
  if (n && (n->level == level))
    return n;
  return nullptr;
}

// Return the next sibling in the tree structure or nullptr.
Node *Node::next_sibling() {
  Node *n;
  for (n = next; n && n->level > level; n = n->next) ;
  if (n && (n->level == level))
    return n;
  return nullptr;
}

// Return the first child or nullptr
Node *Node::first_child() {
  Node *n = next;
  if (n->level > level)
    return n;
  return nullptr;
}

// Generate a descriptive text for this item, to put in browser & window titles
const char* Node::title() {
  const char* c = name();
  if (c)
    return c;
  return type_name();
}

/**
 Return the window that contains this widget.
 \return nullptr if this is not a widget.
 */
Window_Node *Node::window() {
  if (!is_widget())
    return nullptr;
  for (Node *t = this; t; t=t->parent)
    if (t->is_a(Type::Window))
      return (Window_Node*)t;
  return nullptr;
}

/**
 Return the group that contains this widget.
 \return nullptr if this is not a widget.
 */
Group_Node *Node::group() {
  if (!is_widget())
    return nullptr;
  for (Node *t = this; t; t=t->parent)
    if (t->is_a(Type::Group))
      return (Group_Node*)t;
  return nullptr;
}

/**
 Add this list/tree of widgets as a new last child of p.

 \c this must not be part of the widget browser. \c p however must be in the
 widget_browser, so \c Fluid.proj.tree.first and \c Fluid.proj.tree.last are valid for \c p.

 This methods updates the widget_browser.

 \param[in] p insert \c this tree as a child of \c p
 \param[in] strategy is Strategy::AS_LAST_CHILD or Strategy::AFTER_CURRENT
 */
void Node::add(Node *anchor, Strategy strategy) {
#if 0
#ifndef NDEBUG
  // print_project_tree();
  // fprintf(stderr, "Validating project\n");
  validate_project_tree();
  // fprintf(stderr, "Validating branch\n");
  validate_independent_branch(this);
#endif
#endif

  Node *target = nullptr; // insert self before target node, if nullptr, insert last
  Node *target_parent = nullptr; // this will be the new parent for branch
  int target_level = 0;   // adjust self to this new level

  // Find the node after our insertion position
  switch (strategy.placement()) {
    case Strategy::AS_FIRST_CHILD:
    default:
      if (anchor == nullptr) {
        target = Fluid.proj.tree.first;
      } else {
        target = anchor->next;
        target_level = anchor->level + 1;
        target_parent = anchor;
      }
      break;
    case Strategy::AS_LAST_CHILD:
      if (anchor == nullptr) {
        /* empty */
      } else {
        for (target = anchor->next; target && target->level > anchor->level; target = target->next) {/*empty*/}
        target_level = anchor->level + 1;
        target_parent = anchor;
      }
      break;
    case Strategy::AFTER_CURRENT:
      if (anchor == nullptr) {
        target = Fluid.proj.tree.first;
      } else {
        for (target = anchor->next; target && target->level > anchor->level; target = target->next) {/*empty*/}
        target_level = anchor->level;
        target_parent = anchor->parent;
      }
      break;
  }


  // Find the last node of our tree
  Node *end = this;
  while (end->next) end = end->next;

  // Everything is prepared, now insert ourself in front of the target node
  Fluid.proj.undo.checkpoint();

  // Walk the tree to update parent pointers and levels
  int source_level = level;
  for (Node *t = this; t; t = t->next) {
    t->level += (target_level-source_level);
    if (t->level == target_level)
      t->parent = target_parent;
  }

  // Now link ourselves and our children before 'target', or last, if 'target' is nullptr
  if (target) {
    prev = target->prev;
    target->prev = end;
    end->next = target;
  } else {
    prev = Fluid.proj.tree.last;
    end->next = nullptr;
    Fluid.proj.tree.last = end;
  }
  if (prev) {
    prev->next = this;
  } else {
    Fluid.proj.tree.first = this;
  }

#if 0
  { // make sure that we have no duplicate uid's
    Node *tp = this;
    do {
      tp->set_uid(tp->uid_);
      tp = tp->next;
    } while (tp!=end && tp!=nullptr);
  }
#endif

  // Give the widgets in our tree a chance to update themselves
  for (Node *t = this; t && t!=end->next; t = t->next) {
    if (target_parent && (t->level == target_level))
      target_parent->add_child(t, nullptr);
    update_visibility_flag(t);
  }

  Fluid.proj.set_modflag(1);
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
 widget_browser, so `Fluid.proj.tree.first` and `Fluid.proj.tree.last` are valid for `g .

 This methods updates the widget_browser.

 \param[in] g pointer to a node within the tree
 */
void Node::insert(Node *g) {
  // 'this' is not in the Node_Browser, so we must run the linked list to find the last entry
  Node *end = this;
  while (end->next) end = end->next;
  // 'this' will get the same parent as 'g'
  parent = g->parent;
  // run the list again to set the future node levels
  int newlevel = g->level;
  visible = g->visible;
  for (Node *t = this->next; t; t = t->next) t->level += newlevel-level;
  level = newlevel;
  // insert this in the list before g
  prev = g->prev;
  if (prev) prev->next = this; else Fluid.proj.tree.first = this;
  end->next = g;
  g->prev = end;
  update_visibility_flag(this);
  { // make sure that we have no duplicate uid's
    Node *tp = this;
    do {
      tp->set_uid(tp->uid_);
      tp = tp->next;
    } while (tp!=end && tp!=nullptr);
  }
  // tell parent that it has a new child, so it can update itself
  if (parent) parent->add_child(this, g);
  widget_browser->redraw();
}

// Return message number for I18N...
int Node::msgnum() {
  int           count;
  Node       *p;

  for (count = 0, p = this; p;) {
    if (p->label()) count ++;
    if (p != this && p->is_widget() && ((Widget_Node *)p)->tooltip()) count ++;

    if (p->prev) p = p->prev;
    else p = p->parent;
  }

  return count;
}

/**
 Remove this node and all its children from the parent node.

 This does not delete anything. The resulting list//tree will no longer be in
 the widget_browser, so \c Fluid.proj.tree.first and \c Fluid.proj.tree.last do not apply
 to it.

 \return the node that follows this node after the operation; can be nullptr
 */
Node *Node::remove() {
  // find the last child of this node
  Node *end = this;
  for (;;) {
    if (!end->next || end->next->level <= level)
      break;
    end = end->next;
  }
  // unlink this node from the previous one
  if (prev)
    prev->next = end->next;
  else
    Fluid.proj.tree.first = end->next;
  // unlink the last child from their next node
  if (end->next)
    end->next->prev = prev;
  else
    Fluid.proj.tree.last = prev;
  Node *r = end->next;
  prev = end->next = nullptr;
  // allow the parent to update changes in the UI
  if (parent) parent->remove_child(this);
  parent = nullptr;
  // tell the widget_browser that we removed some nodes
  widget_browser->redraw();
  selection_changed(nullptr);
  return r;
}

void Node::name(const char *n) {
  int nostrip = is_a(Type::Comment);
  if (storestring(n,name_,nostrip)) {
    if (visible) widget_browser->redraw();
  }
}

void Node::label(const char *n) {
  if (storestring(n,label_,1)) {
    setlabel(label_);
    if (visible && !name_) widget_browser->redraw();
  }
}

void Node::callback(const char *n) {
  storestring(n,callback_);
}

void Node::user_data(const char *n) {
  storestring(n,user_data_);
}

void Node::user_data_type(const char *n) {
  storestring(n,user_data_type_);
}

void Node::comment(const char *n) {
  if (storestring(n,comment_,1)) {
    if (visible) widget_browser->redraw();
  }
}

void Node::open() {
  printf("Open of '%s' is not yet implemented\n",type_name());
}

// returns pointer to whatever is after f & children

/**
 Move this node (and its children) into list before g.
 Both `this` and `g` must be in the widget browser.
 The caller must make sure that the widget browser is rebuilt correctly.
 \param[in] g move \c this tree before \c g
 */
void Node::move_before(Node* g) {
  if (level != g->level) printf("move_before levels don't match! %d %d\n",
                                level, g->level);
  // Find the last child in the list
  Node *n;
  for (n = next; n && n->level > level; n = n->next) ;
  if (n == g) return;
  // now link this tree before g
  Node *l = n ? n->prev : Fluid.proj.tree.last;
  prev->next = n;
  if (n) n->prev = prev; else Fluid.proj.tree.last = prev;
  prev = g->prev;
  l->next = g;
  if (prev) prev->next = this; else Fluid.proj.tree.first = this;
  g->prev = l;
  // tell parent that it has a new child, so it can update itself
  if (parent && is_widget()) parent->move_child(this,g);
}


// write a widget and all its children:
void Node::write(fld::io::Project_Writer &f) {
  if (f.write_codeview()) proj1_start = (int)ftell(f.file()) + 1;
  if (f.write_codeview()) proj2_start = (int)ftell(f.file()) + 1;
  f.write_indent(level);
  f.write_word(type_name());

  if (is_class()) {
    const char * p =  ((Class_Node*)this)->prefix();
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
  Node *child;
  for (child = next; child && child->level > level; child = child->next)
    if (child->level == level+1) child->write(f);
  if (f.write_codeview()) proj2_start = (int)ftell(f.file()) + 1;
  f.write_close(level);
  if (f.write_codeview()) proj2_end = (int)ftell(f.file());
}

void Node::write_properties(fld::io::Project_Writer &f) {
  // repeat this for each attribute:
  if (Fluid.proj.write_mergeback_data && uid_) {
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

void Node::read_property(fld::io::Project_Reader &f, const char *c) {
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

 \see Grid_Node::write_parent_properties(fld::io::Project_Writer &f, Node *child, bool encapsulate)

 \param[in] f the project file writer
 \param[in] child write properties for this child, make sure it has the correct type
 \param[in] encapsulate write the `parent_properties {}` block if true before writing any properties
 */
void Node::write_parent_properties(fld::io::Project_Writer &f, Node *child, bool encapsulate) {
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

 \see Node::write_parent_properties(fld::io::Project_Writer &f, Node *child, bool encapsulate)
 \see Grid_Node::read_parent_property(fld::io::Project_Reader &f, Node *child, const char *property)

 \param[in] f the project file writer
 \param[in] child read properties for this child
 \param[in] property the name of a property, or "}" when we reach the end of the list
 */
void Node::read_parent_property(fld::io::Project_Reader &f, Node *child, const char *property) {
  (void)child;
  f.read_error("Unknown parent property \"%s\"", property);
}


int Node::read_fdesign(const char*, const char*) {return 0;}

/**
 Write a comment into the header file.
 \param[in] pre indent the comment by this string
*/
void Node::write_comment_h(fld::io::Code_Writer& f, const char *pre)
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
void Node::write_comment_c(fld::io::Code_Writer& f, const char *pre)
{
  if (comment() && *comment()) {
    f.write_c("%s/**\n", pre);
    const char *s = comment();
    if (*s && *s!='\n')
      f.write_c("%s ", pre);
    while(*s) {
      if (*s=='\n') {
        f.write_c("\n");
        if (s[1] && s[1]!='\n') {
          f.write_c("%s ", pre);
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
void Node::write_comment_inline_c(fld::io::Code_Writer& f, const char *pre)
{
  if (comment() && *comment()) {
    const char *s = comment();
    if (strchr(s, '\n')==nullptr) {
      // single line comment
      if (pre) f.write_c("%s", pre);
      f.write_c("// %s\n", s);
      if (!pre) f.write_c("%s", f.indent_plus(1));
    } else {
      f.write_c("%s/*\n", pre?pre:"");
      if (*s && *s!='\n') {
        if (pre)
          f.write_c("%s ", pre);
        else
          f.write_c("%s ", f.indent_plus(1));
      }
      while(*s) {
        if (*s=='\n') {
          f.write_c("\n");
          if (s[1] && s[1]!='\n') {
            if (pre)
              f.write_c("%s ", pre);
            else
              f.write_c("%s ", f.indent_plus(1));
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
Fl_Widget *Node::enter_live_mode(int) {
  return nullptr;
}

/**
  Release all resources created when entering live mode.
  \see enter_live_mode()
*/
void Node::leave_live_mode() {
}

/**
  Copy all needed properties for this type into the live object.
*/
void Node::copy_properties() {
}

/**
  Check whether callback \p cbname is declared anywhere else by the user.

  \b Warning: this just checks that the name is declared somewhere,
  but it should probably also check that the name corresponds to a
  plain function or a member function within the same class and that
  the parameter types match.
 */
int Node::user_defined(const char* cbname) const {
  for (Node* p = Fluid.proj.tree.first; p ; p = p->next)
    if (p->is_a(Type::Function) && p->name() != nullptr)
      if (strncmp(p->name(), cbname, strlen(cbname)) == 0)
        if (p->name()[strlen(cbname)] == '(')
          return 1;
  return 0;
}

const char *Node::callback_name(fld::io::Code_Writer& f) {
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
 in a static buffe (don't call free), or nullptr if this Type is not inside a class
 */
const char* Node::class_name(const int need_nest) const {
  Node* p = parent;
  while (p) {
    if (p->is_class()) {
      // see if we are nested in another class, we must fully-qualify name:
      // this is lame but works...
      const char* q = nullptr;
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
  return nullptr;
}

/**
 Check if this is inside a Class_Node or Widget_Class_Node.
 \return true if any of the parents is Class_Node or Widget_Class_Node
 */
bool Node::is_in_class() const {
  Node* p = parent;
  while (p) {
    if (p->is_class()) return true;
    p = p->parent;
  }
  return false;
}

void Node::write_static(fld::io::Code_Writer&) {
}

void Node::write_static_after(fld::io::Code_Writer&) {
}

void Node::write_code1(fld::io::Code_Writer& f) {
  f.write_h("// Header for %s\n", title());
  f.write_c("// Code for %s\n", title());
}

void Node::write_code2(fld::io::Code_Writer&) {
}

/** Set a uid that is unique within the project.

 Try to set the given id as the unique id for this node. If the suggested id
 is 0, or it is already taken inside this project, we try another random id
 until we find one that is unique.

 \param[in] suggested_uid the preferred uid for this node
 \return the actualt uid that was given to the node
 */
unsigned short Node::set_uid(unsigned short suggested_uid) {
  if (suggested_uid==0)
    suggested_uid = (unsigned short)rand();
  for (;;) {
    Node *tp = Fluid.proj.tree.first;
    for ( ; tp; tp = tp->next)
      if (tp!=this && tp->uid_==suggested_uid)
        break;
    if (tp==nullptr)
      break;
    suggested_uid = (unsigned short)rand();
  }
  uid_ = suggested_uid;
  return suggested_uid;
}


/// \}

