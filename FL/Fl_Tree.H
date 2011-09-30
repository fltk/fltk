//
// "$Id$"
//

#ifndef FL_TREE_H
#define FL_TREE_H

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/fl_draw.H>

#include <FL/Fl_Tree_Item.H>
#include <FL/Fl_Tree_Prefs.H>

//////////////////////
// FL/Fl_Tree.H
//////////////////////
//
// Fl_Tree -- This file is part of the Fl_Tree widget for FLTK
// Copyright (C) 2009-2010 by Greg Ercolano.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

///
/// \file
/// \brief This file contains the definitions of the Fl_Tree class
///

/// \class Fl_Tree
///
/// \brief Tree widget.
///
///     \image html tree-simple.png "Fl_Tree example program"
///     \image latex tree-simple.png "Fl_Tree example program" width=4cm
///
/// \code
///     Fl_Tree                                         // Top level widget
///        |--- Fl_Tree_Item                            // Items in the tree
///        |--- Fl_Tree_Prefs                           // Preferences for the tree
///                  |--- Fl_Tree_Connector (enum)      // Connection modes
///                  |--- Fl_Tree_Select (enum)         // Selection modes
///                  |--- Fl_Tree_Sort (enum)           // Sort behavior
/// \endcode
///
///     Similar to Fl_Browser, Fl_Tree is a browser of Fl_Tree_Item's, which is arranged
///     in a parented hierarchy, or 'tree'. Subtrees can be expanded or closed. Items can be
///     added, deleted, inserted, sorted and re-ordered.
///
///     The tree items may also contain other FLTK widgets, like buttons, input fields,
///     or even "custom" widgets.
///
///     The callback() is invoked depending on the value of when():
///
///         - FL_WHEN_RELEASE -- callback invoked when left mouse button is released on an item
///         - FL_WHEN_CHANGED -- callback invoked when left mouse changes selection state
///
///     The simple way to define a tree:
/// \code
///    #include <FL/Fl_Tree.H>
///    [..]
///    Fl_Tree tree(X,Y,W,H);
///    tree.begin();
///      tree.add("Flintstones/Fred");
///      tree.add("Flintstones/Wilma");
///      tree.add("Flintstones/Pebbles");
///      tree.add("Simpsons/Homer");
///      tree.add("Simpsons/Marge");
///      tree.add("Simpsons/Bart");
///      tree.add("Simpsons/Lisa");
///    tree.end();
/// \endcode
///     
///     Items can be added with add(),
///     removed with remove(),
///     completely cleared with clear(),
///     inserted with insert() and insert_above(),
///     selected/deselected with select() and deselect(),
///     open/closed with open() and closed().
///     Children of an item can be swapped around with Fl_Tree_Item::swap_children(),
///     sorting can be controlled when items are add()ed via sortorder().
///     You can walk the entire tree with first() and next().
///     You can walk selected items with first_selected_item() and
///     next_selected_item().
///     Items can be found by their pathname using find_item(const char*),
///     and an item's pathname can be found with item_pathname().
///     The selected items' colors are controlled by selection_color() (inherited from Fl_Widget).
///
///     The tree can have different selection behaviors controlled by selectmode().
///
///     FLTK widgets (including custom widgets) can be assigned to tree items via
///     Fl_Tree_Item::widget().
///
///     Icons for individual items can be changed with
///     Fl_Tree_Item::openicon(), 
///     Fl_Tree_Item::closeicon(),
///     Fl_Tree_Item::usericon().
///
///     Various default preferences can be globally manipulated via Fl_Tree_Prefs, 
///     including colors, margins, icons, connection lines. 
///
///     The tree's callback() will be invoked when items change state or are open/closed.
///     when() controls when mouse/keyboard events invoke the callback.
///     callback_item() and callback_reason() can be used to determine the cause of the callback. eg:
///
/// \code
/// void MyTreeCallback(Fl_Widget *w, void *data) {
///   Fl_Tree      *tree = (Fl_Tree*)w;
///   Fl_Tree_Item *item = (Fl_Tree_Item*)tree->callback_item();	// get selected item
///   switch ( tree->callback_reason() ) {
///     case FL_TREE_REASON_SELECTED: [..]
///     case FL_TREE_REASON_DESELECTED: [..]
///     case FL_TREE_REASON_OPENED: [..]
///     case FL_TREE_REASON_CLOSED: [..]
///   }
/// \endcode
///
///     To get the item's full menu pathname, you can use Fl_Tree_Item::item_pathname(), eg:
///
/// \code
///   char pathname[256] = "???";
///   tree->item_pathname(pathname, sizeof(pathname), item);		// eg. "Parent/Child/Item"
/// \endcode
///
///     To walk all the items of the tree from top to bottom:
/// \code
/// // Walk all the items in the tree, and print their labels
/// for ( Fl_Tree_Item *item = tree->first(); item; item = tree->next(item) ) {
///     printf("Item: %s\n", item->label());
/// }
/// \endcode
///
///     To recursively walk all the children of a particular item, 
///     define a function that uses recursion:
///     \code
/// // Find all of the item's children and print an indented report of their labels
/// void my_print_all_children(Fl_Tree_Item *item, int indent=0) {
///     for ( int t=0; t<item->children(); t++ ) {
///         printf("%*s Item: %s\n", indent, "", item->child(t)->label());
///         my_print_all_children(item->child(t), indent+4);   // recurse
///     }
/// }
///     \endcode
///
///     To change the default label font and color for creating new items:
/// \code
///  tree = new Fl_Tree(..);
///  tree->item_labelfont(FL_COURIER);	// Use Courier font for all new items
///  tree->item_labelfgcolor(FL_RED);	// Use red color for labels of all new items
///  [..]
///  // Now create the items in the tree using the above defaults.
///  tree->add("Aaa");
///  tree->add("Bbb");
///  [..]
/// \endcode
///
///     To change the font and color of all items in the tree:
/// \code
/// // Change the font and color of all items currently in the tree
/// for ( Fl_Tree_Item *item = tree->first(); item; item = tree->next(item) ) {
///     item->labelfont(FL_COURIER);
///     item->labelcolor(FL_RED);
/// }
/// \endcode
///
///     The following image shows the tree's various visual elements
///     and the methods that control them:
///     
///     \image html tree-elements.png
///     \image latex tree-elements.png "Fl_Tree dimensions" width=6cm
///

/// \enum Fl_Tree_Reason
/// The reason the callback was invoked.
///
enum Fl_Tree_Reason {
  FL_TREE_REASON_NONE=0,	///< unknown reason
  FL_TREE_REASON_SELECTED,	///< an item was selected
  FL_TREE_REASON_DESELECTED,	///< an item was de-selected
  FL_TREE_REASON_OPENED,	///< an item was opened
  FL_TREE_REASON_CLOSED		///< an item was closed
};


class FL_EXPORT Fl_Tree : public Fl_Group {
  Fl_Tree_Item  *_root;				// can be null!
  Fl_Tree_Item  *_item_focus;			// item that has focus box
  Fl_Tree_Item  *_callback_item;		// item invoked during callback (can be NULL)
  Fl_Tree_Reason _callback_reason;		// reason for the callback
  Fl_Tree_Prefs  _prefs;			// all the tree's settings
  int            _scrollbar_size;		// size of scrollbar trough

protected:
  Fl_Scrollbar *_vscroll;			///< Vertical scrollbar
  void item_clicked(Fl_Tree_Item* val);
  void do_callback_for_item(Fl_Tree_Item* item, Fl_Tree_Reason reason);
  Fl_Tree_Item *next_visible_item(Fl_Tree_Item *start, int dir);

public:
  Fl_Tree(int X, int Y, int W, int H, const char *L=0);
  ~Fl_Tree();
  int handle(int e);
  void draw();
  void show_self();
  
  ///////////////////////
  // root methods
  ///////////////////////
  void root_label(const char *new_label);
  Fl_Tree_Item* root();

  ////////////////////////////////
  // Item creation/removal methods
  ////////////////////////////////
  Fl_Tree_Item *add(const char *path);
  Fl_Tree_Item* add(Fl_Tree_Item *item, const char *name);
  Fl_Tree_Item *insert_above(Fl_Tree_Item *above, const char *name);
  Fl_Tree_Item* insert(Fl_Tree_Item *item, const char *name, int pos);
  int remove(Fl_Tree_Item *item);
  void clear();
  void clear_children(Fl_Tree_Item *item);

  ////////////////////////
  // Item lookup methods
  ////////////////////////
  Fl_Tree_Item *find_item(const char *path);
  const Fl_Tree_Item *find_item(const char *path) const;
  int item_pathname(char *pathname, int pathnamelen, const Fl_Tree_Item *item) const;
  const Fl_Tree_Item *find_clicked() const;
  Fl_Tree_Item *item_clicked();
  Fl_Tree_Item *first();
  Fl_Tree_Item *next(Fl_Tree_Item *item=0);
  Fl_Tree_Item *prev(Fl_Tree_Item *item=0);
  Fl_Tree_Item *last();
  Fl_Tree_Item *first_selected_item();
  Fl_Tree_Item *next_selected_item(Fl_Tree_Item *item=0);

  //////////////////////////
  // Item open/close methods
  //////////////////////////
  int open(Fl_Tree_Item *item, int docallback=1);
  int open(const char *path, int docallback=1);
  void open_toggle(Fl_Tree_Item *item, int docallback=1);
  int close(Fl_Tree_Item *item, int docallback=1);
  int close(const char *path, int docallback=1);
  int is_open(Fl_Tree_Item *item) const;
  int is_open(const char *path) const;
  int is_close(Fl_Tree_Item *item) const;
  int is_close(const char *path) const;

  /////////////////////////
  // Item selection methods
  /////////////////////////
  int select(Fl_Tree_Item *item, int docallback=1);
  int select(const char *path, int docallback=1);
  void select_toggle(Fl_Tree_Item *item, int docallback=1);
  int deselect(Fl_Tree_Item *item, int docallback=1);
  int deselect(const char *path, int docallback=1);
  int deselect_all(Fl_Tree_Item *item=0, int docallback=1);
  int select_only(Fl_Tree_Item *selitem, int docallback=1);
  int select_all(Fl_Tree_Item *item=0, int docallback=1);
  void set_item_focus(Fl_Tree_Item *item);
  int is_selected(Fl_Tree_Item *item) const;
  int is_selected(const char *path);

  /////////////////////////////////
  // Item attribute related methods
  /////////////////////////////////
  Fl_Fontsize item_labelsize() const;
  void item_labelsize(Fl_Fontsize val);
  Fl_Font item_labelfont() const;
  void item_labelfont(Fl_Font val);
  Fl_Color item_labelfgcolor(void) const;
  void item_labelfgcolor(Fl_Color val);
  Fl_Color item_labelbgcolor(void) const;
  void item_labelbgcolor(Fl_Color val);
  Fl_Color connectorcolor() const;
  void connectorcolor(Fl_Color val);
  int marginleft() const;
  void marginleft(int val);
  int margintop() const;
  void margintop(int val);
  int openchild_marginbottom() const;
  void openchild_marginbottom(int val);
  int connectorwidth() const;
  void connectorwidth(int val);
  Fl_Image* usericon() const;
  void usericon(Fl_Image *val);
  Fl_Image* openicon() const;
  void openicon(Fl_Image *val);
  Fl_Image* closeicon() const;
  void closeicon(Fl_Image *val);
  int showcollapse() const;
  void showcollapse(int val);
  int showroot() const;
  void showroot(int val);
  Fl_Tree_Connector connectorstyle() const;
  void connectorstyle(Fl_Tree_Connector val);
  Fl_Tree_Sort sortorder() const;
  void sortorder(Fl_Tree_Sort val);
  Fl_Boxtype selectbox() const;
  void selectbox(Fl_Boxtype val);
  Fl_Tree_Select selectmode() const;
  void selectmode(Fl_Tree_Select val);

  int displayed(Fl_Tree_Item *item);
  void show_item(Fl_Tree_Item *item, int yoff);
  void show_item(Fl_Tree_Item *item);
  void show_item_top(Fl_Tree_Item *item);
  void show_item_middle(Fl_Tree_Item *item);
  void show_item_bottom(Fl_Tree_Item *item);
  void display(Fl_Tree_Item *item);
  int  vposition() const;
  void vposition(int pos);

  int is_scrollbar(Fl_Widget *w);
  int scrollbar_size() const;
  void scrollbar_size(int size);

  ///////////////////////
  // callback related
  ///////////////////////
  void callback_item(Fl_Tree_Item* item);
  Fl_Tree_Item* callback_item();
  void callback_reason(Fl_Tree_Reason reason);
  Fl_Tree_Reason callback_reason() const;

  /// Load FLTK preferences
  void load(class Fl_Preferences&);
};

#endif /*FL_TREE_H*/

//
// End of "$Id$".
//
