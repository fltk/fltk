//
// "$Id$"
//
// Menubar test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Choice.H>
#include <stdio.h>
#include <stdlib.h>
#include "../src/flstring.h"
#include <FL/fl_draw.H>

void window_cb(Fl_Widget* w, void*) {
  puts("window callback called");
  ((Fl_Double_Window *)w)->hide();
}

void test_cb(Fl_Widget* w, void*) {
  Fl_Menu_* mw = (Fl_Menu_*)w;
  const Fl_Menu_Item* m = mw->mvalue();
  if (!m)
    printf("NULL\n");
  else if (m->shortcut())
    printf("%s - %s\n", m->label(), fl_shortcut_label(m->shortcut()));
  else
    printf("%s\n", m->label());
}

void quit_cb(Fl_Widget*, void*) {exit(0);}

Fl_Menu_Item hugemenu[100];

Fl_Menu_Item menutable[] = {
  {"foo",0,0,0,FL_MENU_INACTIVE},
  {"&File",0,0,0,FL_SUBMENU},
    {"&Open",	FL_ALT+'o', 0, 0, FL_MENU_INACTIVE},
    {"&Close",	0,	0},
    {"&Quit",	FL_ALT+'q', quit_cb, 0, FL_MENU_DIVIDER},
    {"shortcut",'a'},
    {"shortcut",FL_SHIFT+'a'},
    {"shortcut",FL_CTRL+'a'},
    {"shortcut",FL_CTRL+FL_SHIFT+'a'},
    {"shortcut",FL_ALT+'a'},
    {"shortcut",FL_ALT+FL_SHIFT+'a'},
    {"shortcut",FL_ALT+FL_CTRL+'a'},
    {"shortcut",FL_ALT+FL_SHIFT+FL_CTRL+'a', 0,0, FL_MENU_DIVIDER},
    {"shortcut",'\r'/*FL_Enter*/},
    {"shortcut",FL_CTRL+FL_Enter, 0,0, FL_MENU_DIVIDER},
    {"shortcut",FL_F+1},
    {"shortcut",FL_SHIFT+FL_F+1},
    {"shortcut",FL_CTRL+FL_F+1},
    {"shortcut",FL_SHIFT+FL_CTRL+FL_F+1},
    {"shortcut",FL_ALT+FL_F+1},
    {"shortcut",FL_ALT+FL_SHIFT+FL_F+1},
    {"shortcut",FL_ALT+FL_CTRL+FL_F+1},
    {"shortcut",FL_ALT+FL_SHIFT+FL_CTRL+FL_F+1, 0,0, FL_MENU_DIVIDER},
    {"&Submenus", FL_ALT+'S',	0, (void*)"Submenu1", FL_SUBMENU},
      {"A very long menu item"},
      {"&submenu",FL_CTRL+'S',	0, (void*)"submenu2", FL_SUBMENU},
	{"item 1"},
	{"item 2"},
	{"item 3"},
	{"item 4"},
	{0},
      {"after submenu"},
      {0},
    {0},
  {"&Edit",FL_F+2,0,0,FL_SUBMENU},
    {"Undo",	FL_ALT+'z',	0},
    {"Redo",	FL_ALT+'r',	0, 0, FL_MENU_DIVIDER},
    {"Cut",	FL_ALT+'x',	0},
    {"Copy",	FL_ALT+'c',	0},
    {"Paste",	FL_ALT+'v',	0},
    {"Inactive",FL_ALT+'d',	0, 0, FL_MENU_INACTIVE},
    {"Clear",	0,	0, 0, FL_MENU_DIVIDER},
    {"Invisible",FL_ALT+'e',	0, 0, FL_MENU_INVISIBLE},
    {"Preferences",0,	0},
    {"Size",	0,	0},
    {0},
  {"&Checkbox",FL_F+3,0,0,FL_SUBMENU},
    {"&Alpha",	FL_F+2,	0, (void *)1, FL_MENU_TOGGLE},
    {"&Beta",	0,	0, (void *)2, FL_MENU_TOGGLE},
    {"&Gamma",	0,	0, (void *)3, FL_MENU_TOGGLE},
    {"&Delta",	0,	0, (void *)4, FL_MENU_TOGGLE|FL_MENU_VALUE},
    {"&Epsilon",0,	0, (void *)5, FL_MENU_TOGGLE},
    {"&Pi",	0,	0, (void *)6, FL_MENU_TOGGLE},
    {"&Mu",	0,	0, (void *)7, FL_MENU_TOGGLE|FL_MENU_DIVIDER},
    {"Red",	0,	0, (void *)1, FL_MENU_TOGGLE, 0, 0, 0, 1},
    {"Black",	0,	0, (void *)1, FL_MENU_TOGGLE|FL_MENU_DIVIDER},
    {"00",	0,	0, (void *)1, FL_MENU_TOGGLE},
    {"000",	0,	0, (void *)1, FL_MENU_TOGGLE},
    {0},
  {"&Radio",0,0,0,FL_SUBMENU},
    {"&Alpha",	0,	0, (void *)1, FL_MENU_RADIO},
    {"&Beta",	0,	0, (void *)2, FL_MENU_RADIO},
    {"&Gamma",	0,	0, (void *)3, FL_MENU_RADIO},
    {"&Delta",	0,	0, (void *)4, FL_MENU_RADIO|FL_MENU_VALUE},
    {"&Epsilon",0,	0, (void *)5, FL_MENU_RADIO},
    {"&Pi",	0,	0, (void *)6, FL_MENU_RADIO},
    {"&Mu",	0,	0, (void *)7, FL_MENU_RADIO|FL_MENU_DIVIDER},
    {"Red",	0,	0, (void *)1, FL_MENU_RADIO},
    {"Black",	0,	0, (void *)1, FL_MENU_RADIO|FL_MENU_DIVIDER},
    {"00",	0,	0, (void *)1, FL_MENU_RADIO},
    {"000",	0,	0, (void *)1, FL_MENU_RADIO},
    {0},
  {"&Font",0,0,0,FL_SUBMENU /*, 0, FL_BOLD, 20*/},
    {"Normal",	0, 0, 0, 0, 0, 0, 14},
    {"Bold",	0, 0, 0, 0, 0, FL_BOLD, 14},
    {"Italic",	0, 0, 0, 0, 0, FL_ITALIC, 14},
    {"BoldItalic",0,0,0, 0, 0, FL_BOLD+FL_ITALIC, 14},
    {"Small",	0, 0, 0, 0, 0, FL_BOLD+FL_ITALIC, 10},
    {"Emboss",	0, 0, 0, 0, (uchar)FL_EMBOSSED_LABEL},
    {"Engrave",	0, 0, 0, 0, (uchar)FL_ENGRAVED_LABEL},
    {"Shadow",	0, 0, 0, 0, (uchar)FL_SHADOW_LABEL},
    {"@->",	0, 0, 0, 0, (uchar)FL_SYMBOL_LABEL},
    {0},
  {"&International",0,0,0,FL_SUBMENU},
    {"Sharp Ess",0x0000df},
    {"A Umlaut",0x0000c4},
    {"a Umlaut",0x0000e4},
    {"Euro currency",FL_COMMAND+0x0020ac},
    {"the &\xc3\xbc Umlaut"},  // &uuml;
    {"the capital &\xc3\x9c"}, // &Uuml;
    {"convert \xc2\xa5 to &\xc2\xa3"}, // Yen to GBP
    {"convert \xc2\xa5 to &\xe2\x82\xac"}, // Yen to Euro
    {"Hangul character Sios &\xe3\x85\x85"},
    {"Hangul character Cieuc", 0x003148},
    {0},
  {"E&mpty",0,0,0,FL_SUBMENU},
    {0},
  {"&Inactive", 0,	0, 0, FL_MENU_INACTIVE|FL_SUBMENU},
    {"A very long menu item"},
    {"A very long menu item"},
    {0},
  {"Invisible",0,	0, 0, FL_MENU_INVISIBLE|FL_SUBMENU},
    {"A very long menu item"},
    {"A very long menu item"},
    {0},
  {"&Huge", 0, 0, (void*)hugemenu, FL_SUBMENU_POINTER},
  {"button",FL_F+4, 0, 0, FL_MENU_TOGGLE},
  {0}
};

Fl_Menu_Item pulldown[] = {
  {"Red",	FL_ALT+'r'},
  {"Green",	FL_ALT+'g'},
  {"Blue",	FL_ALT+'b'},
  {"Strange",	FL_ALT+'s', 0, 0, FL_MENU_INACTIVE},
  {"&Charm",	FL_ALT+'c'},
  {"Truth",	FL_ALT+'t'},
  {"Beauty",	FL_ALT+'b'},
  {0}
};

#ifdef __APPLE__
Fl_Menu_Item menu_location[] = {
  {"Fl_Menu_Bar",	0, 0, 0, FL_MENU_VALUE},
  {"Fl_Sys_Menu_Bar",	},
  {0}
};

Fl_Sys_Menu_Bar* smenubar;

void menu_location_cb(Fl_Widget* w, void* data) 
{
  Fl_Menu_Bar *menubar = (Fl_Menu_Bar*)data;
  if (((Fl_Choice*)w)->value() == 1) { // switch to system menu bar
    menubar->hide();
    const Fl_Menu_Item *menu = menubar->menu();
    smenubar = new  Fl_Sys_Menu_Bar(0,0,0,30); 
    smenubar->menu(menu);
    smenubar->callback(test_cb);
    }
  else { // switch to window menu bar
    smenubar->clear();
    delete smenubar;
    menubar->show();
    }
}
#endif // __APPLE__

#define WIDTH 700

Fl_Menu_* menus[4];

int main(int argc, char **argv) {
  //Fl::set_color(Fl_Color(15),0,0,128);
  for (int i=0; i<99; i++) {
    char buf[100];
    sprintf(buf,"item %d",i);
    hugemenu[i].text = strdup(buf);
  }
  Fl_Double_Window window(WIDTH,400);
  window.callback(window_cb);
  Fl_Menu_Bar menubar(0,0,WIDTH,30); menubar.menu(menutable);
  menubar.callback(test_cb);
  menus[0] = &menubar;
  Fl_Menu_Button mb1(100,100,120,25,"&menubutton"); mb1.menu(pulldown);
  mb1.tooltip("this is a menu button");
  mb1.callback(test_cb);
  menus[1] = &mb1;
  Fl_Choice ch(300,100,80,25,"&choice:"); ch.menu(pulldown);
  ch.tooltip("this is a choice menu");
  ch.callback(test_cb);
  menus[2] = &ch;
  Fl_Menu_Button mb(0,0,WIDTH,400,"&popup");
  mb.type(Fl_Menu_Button::POPUP3);
  mb.box(FL_NO_BOX);
  mb.menu(menutable);
  mb.remove(1); // delete the "File" submenu
  mb.callback(test_cb);
  menus[3] = &mb;
  Fl_Box b(200,200,200,100,"Press right button\nfor a pop-up menu");
  window.resizable(&mb);
  window.size_range(300,400,0,400);
#ifdef __APPLE__
  Fl_Choice ch2(500,100,150,25,"Use:"); 
  ch2.menu(menu_location);
  ch2.callback(menu_location_cb, &menubar);
#endif
  window.end();
  
#ifdef __APPLE__
  Fl_Menu_Item custom[] = {
    {"Preferences…",	0,	test_cb, NULL, FL_MENU_DIVIDER},
    {"Radio1",	0,	test_cb, NULL, FL_MENU_RADIO|FL_MENU_VALUE},
    {"Radio2",	0,	test_cb, NULL, FL_MENU_RADIO},
    {0}
  };
  Fl_Mac_App_Menu::custom_application_menu_items(custom);
#endif
  window.show(argc, argv);
  return Fl::run();
}

//
// End of "$Id$".
//
