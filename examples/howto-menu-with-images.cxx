// "$Id$"
// vim: autoindent tabstop=2 shiftwidth=2 expandtab softtabstop=2 filetype=cpp
//
//     How to use Fl_Multi_Label to make menu items with images and labels.
//
// Copyright 2017 Greg Ercolano.
// Copyright 1998-2017 by Bill Spitzak and others.
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
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Multi_Label.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Pixmap.H>
#include <FL/fl_message.H>

// Document icon
static const char *L_document_xpm[] = {
  "13 11 3 1",
  "   c None",
  "x  c #d8d8f8",
  "@  c #202060",
  " @@@@@@@@@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @@@@@@@@@   "};
static Fl_Pixmap L_document_pixmap(L_document_xpm);

// Folder icon
static const char *L_folder_xpm[] = {
  "13 11 3 1",
  "   c None",
  "x  c #d8d833",
  "@  c #808011",
  "             ",
  "     @@@@    ",
  "    @xxxx@   ",
  "@@@@@xxxx@@  ",
  "@xxxxxxxxx@  ",
  "@xxxxxxxxx@  ",
  "@xxxxxxxxx@  ",
  "@xxxxxxxxx@  ",
  "@xxxxxxxxx@  ",
  "@xxxxxxxxx@  ",
  "@@@@@@@@@@@  "};
static Fl_Pixmap L_folder_pixmap(L_folder_xpm);

// Red "X"
static const char *L_redx_xpm[] = {
  "13 11 5 1",
  "   c None",
  "+  c #222222",
  "x  c #555555",
  "-  c #882222",
  "@  c #ffffff",
  "   x+++x     ",
  "  ++---++    ",
  " ++-----++   ",
  "++-@@-@@-++  ",
  "++--@@@--++  ",
  "++---@---++  ",
  "++--@@@--++  ",
  "++-@@-@@-++  ",
  " ++-----++   ",
  "  ++---++    ",
  "   x+++x     "};
static Fl_Pixmap L_redx_pixmap(L_redx_xpm);

// Handle the different menu items..
void Menu_CB(Fl_Widget *w, void* data) {
  const char *itemname = (const char*)data;             // "New", "Open", etc
  //DEBUG printf("Clicked on '%s'\n", itemname);
  if (        strcmp(itemname, "New")  == 0 ) {
    fl_message("File/New would happen here..");
  } else if ( strcmp(itemname, "Open") == 0 ) {
    fl_message("File/Open would happen here..");
  } else if ( strcmp(itemname, "Quit") == 0 ) {
    exit(0);
  } else if ( strcmp(itemname, "Copy") == 0 ) {
    fl_message("Edit/Copy would happen here..");
  } else if ( strcmp(itemname, "Paste") == 0 ) {
    fl_message("Edit/Paste would happen here..");
  } else {
    fl_message("'Image' operation '%s'..", itemname);
  }
}

// ADD AN IMAGE IN FRONT OF ITEM'S TEXT
int AddItemToMenu(Fl_Menu_   *menu,                   // menu to add item to
                 const char  *labeltext,              // label text
                 int         shortcut,                // shortcut (e.g. FL_COMMAND+'a')
                 Fl_Callback *cb,                     // callback to invoke
                 void        *userdata,               // userdata for callback
                 Fl_Pixmap*  pixmap) {                // image (if any) to add to item
  // Add a new menu item
  int i = menu->add(labeltext, shortcut, cb, userdata);

  if ( !pixmap ) return i;
  Fl_Menu_Item *item = (Fl_Menu_Item*)&(menu->menu()[i]);
  const char *itemtext = item->label();	 // keep item's label() -- item->image() clobbers it!

  // Assign image to menu item
  item->image(*pixmap);                  // note: clobbers item->label()

  // Create a multi label, assign it an image + text
  Fl_Multi_Label *ml = new Fl_Multi_Label;

  // Left side of label is image
  ml->typea  = _FL_IMAGE_LABEL;
  ml->labela = (const char*)pixmap;

  // Right side of label is text
  ml->typeb  = FL_NORMAL_LABEL;
  ml->labelb = itemtext;

  // Assign multilabel to item
  ml->label(item);

  return i;
}

// Create Menu Items
//    This same technique works for Fl_Menu_ derived widgets,
//    e.g. Fl_Menu_Bar, Fl_Menu_Button, Fl_Choice..
//
void CreateMenuItems(Fl_Menu_* menu) {

  // Add items with LABLES AND IMAGES using Fl_Multi_Label..
  AddItemToMenu(menu, "File/New",  FL_COMMAND+'n', Menu_CB, (void*)"New",  &L_document_pixmap);
  AddItemToMenu(menu, "File/Open", FL_COMMAND+'o', Menu_CB, (void*)"Open", &L_folder_pixmap);
  AddItemToMenu(menu, "File/Quit", FL_COMMAND+'q', Menu_CB, (void*)"Quit", &L_redx_pixmap);

  // Create menu bar items with JUST LABELS
  menu->add("Edit/Copy",  FL_COMMAND+'c', Menu_CB, (void*)"Copy");
  menu->add("Edit/Paste", FL_COMMAND+'v', Menu_CB, (void*)"Paste");

  // Create menu bar items with JUST IMAGES (no labels)
  //    This shows why you need Fl_Multi_Label; the item->label()
  //    gets clobbered by the item->image() setting.
  //
  int i;
  Fl_Menu_Item *item;

  i = menu->add("Images/One", 0, Menu_CB, (void*)"One");
  item = (Fl_Menu_Item*)&(menu->menu()[i]);
  item->image(L_document_pixmap);   // note: this clobbers the item's label()

  i = menu->add("Images/Two", 0, Menu_CB, (void*)"Two");
  item = (Fl_Menu_Item*)&(menu->menu()[i]);
  item->image(L_folder_pixmap);

  i = menu->add("Images/Three", 0, Menu_CB, (void*)"Three");
  item = (Fl_Menu_Item*)&(menu->menu()[i]);
  item->image(L_redx_pixmap);
}

int main() {
  Fl_Double_Window *win = new Fl_Double_Window(400, 400, "Menu items with images");
  win->tooltip("Right click on window background\nfor popup menu");

  // Help message
  Fl_Box *box = new Fl_Box(100,100,200,200);
  box->copy_label(win->tooltip());
  box->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);

  // Menu bar
  Fl_Menu_Bar *menubar = new Fl_Menu_Bar(0,0,win->w(), 25);
  CreateMenuItems(menubar);

  // Right click context menu
  Fl_Menu_Button *menubutt = new Fl_Menu_Button(0,25,win->w(), win->h()-25);
  CreateMenuItems(menubutt);
  menubutt->type(Fl_Menu_Button::POPUP3);

  // Chooser menu
  Fl_Choice *choice = new Fl_Choice(140,50,200,25,"Choice");
  CreateMenuItems(choice);
  choice->value(1);

  // TODO: Show complex labels with Fl_Multi_Label. From docs:
  //
  //     "More complex labels might be constructed by setting labelb as another
  //     Fl_Multi_Label and thus chaining up a series of label elements."
  //

  win->end();
  win->resizable(win);
  win->show();
  return Fl::run();
}

//
// End of "$Id$".
//
