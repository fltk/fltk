//
// "$Id$"
//
// MacOS system menu bar widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2013 by Bill Spitzak and others.
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

/*
 * This code is a quick hack! It was written as a proof of concept.
 * It has been tested on the "menubar" sample program and provides
 * basic functionality. 
 * 
 * To use the System Menu Bar, simply replace the main Fl_Menu_Bar
 * in an application with Fl_Sys_Menu_Bar.
 *
 * FLTK features not supported by the Mac System menu
 *
 * - no symbolic labels
 * - no embossed labels
 * - no font sizes
 * - Shortcut Characters should be Latin letters only
 * - no disable main menus
 *
 * Many other calls of the parent class don't work.
 */

#if defined(__APPLE__) || defined(FL_DOXYGEN)
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/x.H>
#include <FL/Fl.H>

#import <Cocoa/Cocoa.h>

#include "flstring.h"
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

typedef const Fl_Menu_Item *pFl_Menu_Item;

Fl_Sys_Menu_Bar *fl_sys_menu_bar = 0;

static char *remove_ampersand(const char *s);
extern void (*fl_lock_function)();
extern void (*fl_unlock_function)();

@interface FLMenuItem : NSMenuItem {
}
- (void) doCallback:(id)unused;
- (void) directCallback:(id)unused;
- (const Fl_Menu_Item*) getFlItem;
- (void) setKeyEquivalent:(char)value;
- (void) setKeyEquivalentModifierMask:(int)value;
+ (int) addNewItem:(const Fl_Menu_Item*)mitem menu:(NSMenu*)menu;
@end

@implementation FLMenuItem
- (const Fl_Menu_Item*) getFlItem
{
  return (const Fl_Menu_Item *)[(NSData*)[self representedObject] bytes];
}
- (void) doCallback:(id)unused
{
  fl_lock_function();
  const Fl_Menu_Item *item = [self getFlItem];
  fl_sys_menu_bar->picked(item);
  if ( item->flags & FL_MENU_TOGGLE ) {	// update the menu toggle symbol
    [self setState:(item->value() ? NSOnState : NSOffState)];
  }
  else if ( item->flags & FL_MENU_RADIO ) {	// update the menu radio symbols
    NSMenu* menu = [self menu];
    NSInteger flRank = [menu indexOfItem:self];
    NSInteger last = [menu numberOfItems] - 1;
    int from = flRank;
    while(from > 0) {
      if ([[menu itemAtIndex:from-1] isSeparatorItem]) break;
      item = [(FLMenuItem*)[menu itemAtIndex:from-1] getFlItem];
      if ( !(item->flags & FL_MENU_RADIO) ) break;
      from--;
    }
    int to = flRank;
    while (to < last) {
      if ([[menu itemAtIndex:to+1] isSeparatorItem]) break;
      item = [(FLMenuItem*)[menu itemAtIndex:to+1] getFlItem];
      if (!(item->flags & FL_MENU_RADIO)) break;
      to++;
    }
    for(int i =  from; i <= to; i++) {
      NSMenuItem *nsitem = [menu itemAtIndex:i];
      [nsitem setState:(nsitem != self ? NSOffState : NSOnState)];
    }
  }
  fl_unlock_function();
}
- (void) directCallback:(id)unused
{
  fl_lock_function();
  Fl_Menu_Item *item = (Fl_Menu_Item *)[(NSData*)[self representedObject] bytes];
  if ( item && item->callback() ) item->do_callback(NULL);
  fl_unlock_function();
}
- (void) setKeyEquivalent:(char)key
{
  NSString *equiv = [[NSString alloc] initWithBytes:&key length:1 encoding:NSASCIIStringEncoding];
  [super setKeyEquivalent:equiv];
  [equiv release];
}
- (void) setKeyEquivalentModifierMask:(int)value
{
  NSUInteger macMod = 0;
  if ( value & FL_META ) macMod = NSCommandKeyMask;
  if ( value & FL_SHIFT || isupper(value) ) macMod |= NSShiftKeyMask;
  if ( value & FL_ALT ) macMod |= NSAlternateKeyMask;
  if ( value & FL_CTRL ) macMod |= NSControlKeyMask;
  [super setKeyEquivalentModifierMask:macMod];
}
+ (int) addNewItem:(const Fl_Menu_Item*)mitem menu:(NSMenu*)menu
{
  char *name = remove_ampersand(mitem->label());
  CFStringRef cfname = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
  free(name);
  FLMenuItem *item = [[FLMenuItem alloc] initWithTitle:(NSString*)cfname 
						action:@selector(doCallback:) 
					 keyEquivalent:@""];
  NSData *pointer = [NSData dataWithBytesNoCopy:(void*)mitem length:sizeof(Fl_Menu_Item) freeWhenDone:NO];
  [item setRepresentedObject:pointer];
  [menu addItem:item];
  CFRelease(cfname);
  [item setTarget:item];
  int retval = [menu indexOfItem:item];
  [item release];
  return retval;
}
@end

 
void fl_mac_set_about( Fl_Callback *cb, void *user_data, int shortcut) 
{
  fl_open_display();
  Fl_Menu_Item aboutItem;
  memset(&aboutItem, 0, sizeof(Fl_Menu_Item));
  aboutItem.callback(cb);
  aboutItem.user_data(user_data);
  aboutItem.shortcut(shortcut);
  NSMenu *appleMenu = [[[NSApp mainMenu] itemAtIndex:0] submenu];
  CFStringRef cfname = CFStringCreateCopy(NULL, (CFStringRef)[[appleMenu itemAtIndex:0] title]);
  [appleMenu removeItemAtIndex:0];
  FLMenuItem *item = [[[FLMenuItem alloc] initWithTitle:(NSString*)cfname 
						 action:@selector(directCallback:) 
					  keyEquivalent:@""] autorelease];
  if (aboutItem.shortcut()) {
    [item setKeyEquivalent:(aboutItem.shortcut() & 0xff)];
    [item setKeyEquivalentModifierMask:aboutItem.shortcut()];
  }
  NSData *pointer = [NSData dataWithBytes:&aboutItem length:sizeof(Fl_Menu_Item)];
  [item setRepresentedObject:pointer];
  [appleMenu insertItem:item atIndex:0];
  CFRelease(cfname);
  [item setTarget:item];
}

/*
 * Set a shortcut for an Apple menu item using the FLTK shortcut descriptor.
 */
static void setMenuShortcut( NSMenu* mh, int miCnt, const Fl_Menu_Item *m )
{
  if ( !m->shortcut_ ) 
    return;
  if ( m->flags & FL_SUBMENU )
    return;
  if ( m->flags & FL_SUBMENU_POINTER )
    return;
  char key = m->shortcut_ & 0xff;
  if ( !isalnum( key ) )
    return;
  
  FLMenuItem* menuItem = (FLMenuItem*)[mh itemAtIndex:miCnt];
  [menuItem setKeyEquivalent:(m->shortcut_ & 0xff)];
  [menuItem setKeyEquivalentModifierMask:m->shortcut_];
}


/*
 * Set the Toggle and Radio flag based on FLTK flags
 */
static void setMenuFlags( NSMenu* mh, int miCnt, const Fl_Menu_Item *m )
{
  if ( m->flags & FL_MENU_TOGGLE )
  {
    NSMenuItem *menuItem = [mh itemAtIndex:miCnt];
    [menuItem setState:(m->flags & FL_MENU_VALUE ? NSOnState : NSOffState)];
  }
  else if ( m->flags & FL_MENU_RADIO ) {
    NSMenuItem *menuItem = [mh itemAtIndex:miCnt];
    [menuItem setState:(m->flags & FL_MENU_VALUE ? NSOnState : NSOffState)];
  }
}

static char *remove_ampersand(const char *s)
{
  char *ret = strdup(s);
  const char *p = s;
  char *q = ret;
  while(*p != 0) {
    if (p[0]=='&') {
      if (p[1]=='&') {
        *q++ = '&'; p+=2;
      } else {
        p++;
      }
    } else {
      *q++ = *p++;
    }
  }
  *q = 0;
  return ret;
}


/*
 * create a sub menu for a specific menu handle
 */
static void createSubMenu( NSMenu *mh, pFl_Menu_Item &mm,  const Fl_Menu_Item *mitem)
{
  NSMenu *submenu;
  int miCnt, flags;
  
  NSMenuItem *menuItem;
  char *ts = remove_ampersand(mitem->text);
  CFStringRef title = CFStringCreateWithCString(NULL, ts, kCFStringEncodingUTF8);
  free(ts);
  submenu = [[NSMenu alloc] initWithTitle:(NSString*)title];
  CFRelease(title);
  [submenu setAutoenablesItems:NO];
  
  int cnt;
  cnt = [mh numberOfItems];
  cnt--;
  menuItem = [mh itemAtIndex:cnt];
  [menuItem setSubmenu:submenu];
  [submenu release];
  
  while ( mm->text )
  {
    char visible = mm->visible() ? 1 : 0;
    miCnt = [FLMenuItem addNewItem:mm menu:submenu];
    setMenuFlags( submenu, miCnt, mm );
    setMenuShortcut( submenu, miCnt, mm );
    if ( mm->flags & FL_MENU_INACTIVE || mitem->flags & FL_MENU_INACTIVE) {
      NSMenuItem *item = [submenu itemAtIndex:miCnt];
      [item setEnabled:NO];
    }
    flags = mm->flags;
    if ( mm->flags & FL_SUBMENU )
    {
      mm++;
      createSubMenu( submenu, mm, mm - 1 );
    }
    else if ( mm->flags & FL_SUBMENU_POINTER )
    {
      const Fl_Menu_Item *smm = (Fl_Menu_Item*)mm->user_data_;
      createSubMenu( submenu, smm, mm );
    }
    if ( flags & FL_MENU_DIVIDER ) {
      [submenu addItem:[NSMenuItem separatorItem]];
      }
    if ( !visible ) {
      [submenu removeItem:[submenu itemAtIndex:miCnt]];
    }
    mm++;
  }
}
 

/*
 * convert a complete Fl_Menu_Item array into a series of menus in the top menu bar
 * ALL PREVIOUS SYSTEM MENUS, EXCEPT THE APPLICATION MENU, ARE REPLACED BY THE NEW DATA
 */
static void convertToMenuBar(const Fl_Menu_Item *mm)
{
  NSMenu *fl_system_menu = [NSApp mainMenu];
  int rank;
  int count;//first, delete all existing system menus
  count = [fl_system_menu numberOfItems];
  for(int i = count - 1; i > 0; i--) {
    [fl_system_menu removeItem:[fl_system_menu itemAtIndex:i]];
  }
  //now convert FLTK stuff into MacOS menus
  for (;;)
  {
    if ( !mm || !mm->text )
      break;
    char visible = mm->visible() ? 1 : 0;
    rank = [FLMenuItem  addNewItem:mm menu:fl_system_menu];
    
    if ( mm->flags & FL_SUBMENU ) {
      mm++;
      createSubMenu(fl_system_menu, mm, mm - 1);
      }
    else if ( mm->flags & FL_SUBMENU_POINTER ) {
      const Fl_Menu_Item *smm = (Fl_Menu_Item*)mm->user_data_;
      createSubMenu(fl_system_menu, smm, mm);
    }
    if ( !visible ) {
      [fl_system_menu removeItem:[fl_system_menu itemAtIndex:rank]];
    }
    mm++;
  }
}


/**
 * @brief create a system menu bar using the given list of menu structs
 *
 * \author Matthias Melcher
 *
 * @param m list of Fl_Menu_Item
 */
void Fl_Sys_Menu_Bar::menu(const Fl_Menu_Item *m) 
{
  fl_open_display();
  Fl_Menu_Bar::menu( m );
  convertToMenuBar(m);
}


/**
 * @brief add to the system menu bar a new menu item
 *
 * add to the system menu bar a new menu item, with a title string, shortcut int,
 * callback, argument to the callback, and flags.
 *
 * @see Fl_Menu_::add(const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags) 
 */
int Fl_Sys_Menu_Bar::add(const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags)
{
  fl_open_display();
  int rank = Fl_Menu_::add(label, shortcut, cb, user_data, flags);
  convertToMenuBar(Fl_Menu_::menu());
  return rank;
}

/**
 * @brief insert in the system menu bar a new menu item
 *
 * insert in the system menu bar a new menu item, with a title string, shortcut int,
 * callback, argument to the callback, and flags.
 *
 * @see Fl_Menu_::insert(int index, const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags) 
 */
int Fl_Sys_Menu_Bar::insert(int index, const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags)
{
  fl_open_display();
  int rank = Fl_Menu_::insert(index, label, shortcut, cb, user_data, flags);
  convertToMenuBar(Fl_Menu_::menu());
  return rank;
}

void Fl_Sys_Menu_Bar::clear()
{
  Fl_Menu_::clear();
  convertToMenuBar(NULL);
}

int Fl_Sys_Menu_Bar::clear_submenu(int index)
{
  int retval = Fl_Menu_::clear_submenu(index);
  if (retval != -1) convertToMenuBar(Fl_Menu_::menu());
  return retval;
}

/**
 * @brief remove an item from the system menu bar
 *
 * @param rank		the rank of the item to remove
 */
void Fl_Sys_Menu_Bar::remove(int rank)
{
  Fl_Menu_::remove(rank);
  convertToMenuBar(Fl_Menu_::menu());
}


/**
 * @brief rename an item from the system menu bar
 *
 * @param rank		the rank of the item to rename
 * @param name		the new item name as a UTF8 string
 */
void Fl_Sys_Menu_Bar::replace(int rank, const char *name)
{
  Fl_Menu_::replace(rank, name);
  convertToMenuBar(Fl_Menu_::menu());
}


/*
 * Draw the menu bar. 
 * Nothing here because the OS does this for us.
 */
void Fl_Sys_Menu_Bar::draw() {
}


Fl_Sys_Menu_Bar::Fl_Sys_Menu_Bar(int x,int y,int w,int h,const char *l)
: Fl_Menu_Bar(x,y,w,h,l) 
{
  deactivate();			// don't let the old area take events
  fl_sys_menu_bar = this;
}


#endif /* __APPLE__ */

//
// End of "$Id$".
//
