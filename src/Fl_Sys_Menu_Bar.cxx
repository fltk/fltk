//
// "$Id$"
//
// system menu bar widget for the Fast Light Tool Kit (FLTK).
//
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


#include <FL/Fl_Sys_Menu_Bar_Driver.H>
#include <FL/x.H>


Fl_Sys_Menu_Bar *fl_sys_menu_bar = 0;

// initialize this static variable if it was not initialized previously
Fl_Sys_Menu_Bar_Driver* Fl_Sys_Menu_Bar_Driver::new_driver() {
  if (!driver_) { // initialize this static variable if it was not initialized previously
    static Fl_Sys_Menu_Bar_Driver *once = new Fl_Sys_Menu_Bar_Driver();
    driver_ = once;
  }
  return driver_;
}

inline Fl_Sys_Menu_Bar_Driver *Fl_Sys_Menu_Bar::driver() {
  return Fl_Sys_Menu_Bar_Driver::new_driver();
}

Fl_Sys_Menu_Bar_Driver *Fl_Sys_Menu_Bar_Driver::driver_ = Fl_Sys_Menu_Bar_Driver::new_driver();

/**
 The constructor.
 On Mac OS X, all arguments are unused. On other platforms they are used as by Fl_Menu_Bar::Fl_Menu_Bar().
 */
Fl_Sys_Menu_Bar::Fl_Sys_Menu_Bar(int x,int y,int w,int h,const char *l)
: Fl_Menu_Bar(x,y,w,h,l)
{
  if (fl_sys_menu_bar) delete fl_sys_menu_bar;
  fl_sys_menu_bar = this;
  driver()->bar = this;
}

/** The destructor */
Fl_Sys_Menu_Bar::~Fl_Sys_Menu_Bar()
{
  fl_sys_menu_bar = 0;
  clear();
}

Fl_Sys_Menu_Bar_Driver::Fl_Sys_Menu_Bar_Driver() {bar = NULL;}

Fl_Sys_Menu_Bar_Driver::~Fl_Sys_Menu_Bar_Driver() {}

void Fl_Sys_Menu_Bar::update() {
  driver()->update();
}

/**
 * @brief create a system menu bar using the given list of menu structs
 *
 * \author Matthias Melcher
 *
 * @param m Zero-ending list of Fl_Menu_Item's
 */
void Fl_Sys_Menu_Bar::menu(const Fl_Menu_Item *m)
{
  driver()->menu(m);
}

/** Changes the shortcut of item i to n.
 */
void Fl_Sys_Menu_Bar::shortcut (int i, int s) {
  driver()->shortcut(i, s);
}

/** Turns the radio item "on" for the menu item and turns "off" adjacent radio items of the same group.*/
void Fl_Sys_Menu_Bar::setonly (Fl_Menu_Item *item) {
  driver()->setonly(item);
}

/** Sets the flags of item i
 \see Fl_Menu_::mode(int i, int fl) */
void   Fl_Sys_Menu_Bar::mode (int i, int fl) {
  driver()->mode(i, fl);
}

/**
 * @brief Add a new menu item to the system menu bar.
 *
 * Add to the system menu bar a new menu item, with a title string, shortcut int,
 * callback, argument to the callback, and flags.
 *
 * @param label     - new menu item's label
 * @param shortcut  - new menu item's integer shortcut (can be 0 for none, or e.g. FL_ALT+'x')
 * @param cb        - callback to be invoked when item selected (can be 0 for none, in which case the menubar's callback() can be used instead)
 * @param user_data - argument to the callback
 * @param flags     - item's flags, e.g. ::FL_MENU_TOGGLE, etc.
 *
 * \returns the index into the menu() array, where the entry was added
 *
 * @see Fl_Menu_::add(const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags)
 */
int Fl_Sys_Menu_Bar::add(const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags)
{
  return driver()->add(label, shortcut, cb, user_data, flags);
}

/**
 * Forms-compatible procedure to add items to the system menu bar
 *
 * \returns the index into the menu() array, where the entry was added
 * @see Fl_Menu_::add(const char* str)
 */
int Fl_Sys_Menu_Bar::add(const char* str)
{
  return driver()->add(str);
}

/**
 * @brief insert in the system menu bar a new menu item
 *
 * Insert in the system menu bar a new menu item, with a title string, shortcut int,
 * callback, argument to the callback, and flags.
 *
 * \returns the index into the menu() array, where the entry was inserted
 * @see Fl_Menu_::insert(int index, const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags)
 */
int Fl_Sys_Menu_Bar::insert(int index, const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags)
{
  return driver()->insert(index, label, shortcut, cb, user_data, flags);
}

/** Set the Fl_Menu_Item array pointer to null, indicating a zero-length menu.
 \see Fl_Menu_::clear()
 */
void Fl_Sys_Menu_Bar::clear()
{
  driver()->clear();
}

/** Clears the specified submenu pointed to by index of all menu items.
 \see Fl_Menu_::clear_submenu(int index)
 */
int Fl_Sys_Menu_Bar::clear_submenu(int index)
{
  return driver()->clear_submenu(index);
}

/**
 * @brief remove an item from the system menu bar
 *
 * @param index    the index of the item to remove
 */
void Fl_Sys_Menu_Bar::remove(int index)
{
  driver()->remove(index);
}

/**
 * @brief rename an item from the system menu bar
 *
 * @param index    the index of the item to rename
 * @param name    the new item name as a UTF8 string
 */
void Fl_Sys_Menu_Bar::replace(int index, const char *name)
{
  driver()->replace(index, name);
}

/**
 *  Attaches a callback to the "About myprog" item of the system application menu.
 * This cross-platform function is effective only under the MacOS platform.
 * \param cb   a callback that will be called by "About myprog" menu item
 *       with NULL 1st argument.
 * \param user_data   a pointer transmitted as 2nd argument to the callback.
 */
void Fl_Sys_Menu_Bar::about(Fl_Callback *cb, void *data) {
  if (fl_sys_menu_bar) fl_sys_menu_bar->driver()->about(cb, data);
}

void Fl_Sys_Menu_Bar::draw() {
  driver()->draw();
}

//
// End of "$Id$".
//
