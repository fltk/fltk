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
 * \param data   a pointer transmitted as 2nd argument to the callback.
 */
void Fl_Sys_Menu_Bar::about(Fl_Callback *cb, void *data) {
  driver()->about(cb, data);
}

void Fl_Sys_Menu_Bar::draw() {
  driver()->draw();
}



/** Get the style of the Window menu in the system menu bar */
Fl_Sys_Menu_Bar::window_menu_style_enum Fl_Sys_Menu_Bar::window_menu_style() {
  return Fl_Sys_Menu_Bar_Driver::window_menu_style();
}

/** Set the desired style of the Window menu in the system menu bar.
 This function, to be called before the first call to Fl_Window::show(), allows to
 control whether the system menu bar should contain a Window menu,
 and if yes, whether new windows should be displayed in tabbed form. These are
 the effects of various values for \p style :
 \li \c  no_window_menu : don't add a Window menu to the system menu bar
 \li \c tabbing_mode_none :   add a simple Window menu to the system menu bar
 \li \c  tabbing_mode_automatic : the window menu also contains "Merge All Windows" to group
 all windows in a single tabbed display mode. This is the \b default Window menu style
 for FLTK apps.
 \li \c tabbing_mode_preferred : new windows are displayed in tabbed mode when first created
 
 The Window menu, if present, is entirely created and controlled by the FLTK library.
 Mac OS version 10.12 or later must be running for windows to be displayed in tabbed form.
 Under non MacOS platforms, this function does nothing.
 \version 1.4
 */
void Fl_Sys_Menu_Bar::window_menu_style(Fl_Sys_Menu_Bar::window_menu_style_enum style) {
  Fl_Sys_Menu_Bar_Driver::window_menu_style(style);
}

/** Adds a Window menu, to the end of the system menu bar.
 FLTK apps typically don't need to call this function which is automatically
 called by the library the first time a window is shown. The default system menu bar
 contains a Window menu with a "Merge All Windows" item.
 Other Window menu styles can be obtained calling
 Fl_Sys_Menu_Bar::window_menu_style(window_menu_style_enum) before the first Fl_Window::show().
 Alternatively, an app can call create_window_menu() after having populated the system menu bar,
 for example with menu(const Fl_Menu_Item *), and before the first Fl_Window::show().
 
 This function does nothing on non MacOS platforms.
 \version 1.4
 */
void Fl_Sys_Menu_Bar::create_window_menu() {
  fl_open_display();
  driver()->create_window_menu();
}

#if !defined(FL_DOXYGEN)
Fl_Sys_Menu_Bar_Driver *Fl_Sys_Menu_Bar::driver() {
  if (!Fl_Sys_Menu_Bar_Driver::driver_) { // initialize this static variable if it was not initialized previously
    Fl_Sys_Menu_Bar_Driver::driver_ = new Fl_Sys_Menu_Bar_Driver();
  }
  return Fl_Sys_Menu_Bar_Driver::driver_;
}

Fl_Sys_Menu_Bar_Driver *Fl_Sys_Menu_Bar_Driver::driver_ = 0;

Fl_Sys_Menu_Bar_Driver::Fl_Sys_Menu_Bar_Driver() {bar = NULL;}

Fl_Sys_Menu_Bar_Driver::~Fl_Sys_Menu_Bar_Driver() {}

Fl_Sys_Menu_Bar::window_menu_style_enum Fl_Sys_Menu_Bar_Driver::window_menu_style_ = Fl_Sys_Menu_Bar::tabbing_mode_automatic;
#endif // !defined(FL_DOXYGEN)

//
// End of "$Id$".
//
