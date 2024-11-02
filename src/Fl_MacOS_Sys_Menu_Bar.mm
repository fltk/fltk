//
// MacOS system menu bar widget for the Fast Light Tool Kit (FLTK).
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

#if defined(__APPLE__)

#include <FL/platform.H>
#include <FL/fl_string_functions.h>
#include "drivers/Cocoa/Fl_MacOS_Sys_Menu_Bar_Driver.H"
#include "flstring.h"
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include "Fl_System_Driver.H"

#import <Cocoa/Cocoa.h> // keep this after include of Fl_MacOS_Sys_Menu_Bar_Driver.H because of check() conflict

typedef const Fl_Menu_Item *pFl_Menu_Item;

static Fl_Menu_Bar *custom_menu;
static NSString *localized_Window = nil;

static char *remove_ampersand(const char *s);
extern void (*fl_lock_function)();
extern void (*fl_unlock_function)();

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
static void previous_tab_cb(Fl_Widget *, void *data);
static void next_tab_cb(Fl_Widget *, void *data);
static void move_tab_cb(Fl_Widget *, void *data);
static void merge_all_windows_cb(Fl_Widget *, void *data);
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_13
const NSInteger NSControlStateValueOn = NSOnState;
const NSInteger NSControlStateValueOff = NSOffState;
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_12
const NSUInteger NSEventModifierFlagCommand = NSCommandKeyMask;
const NSUInteger NSEventModifierFlagOption = NSAlternateKeyMask;
const NSUInteger NSEventModifierFlagControl = NSControlKeyMask;
const NSUInteger NSEventModifierFlagShift = NSShiftKeyMask;
#endif

void Fl_MacOS_Sys_Menu_Bar_Driver::draw() {
  bar->deactivate(); // prevent Fl_Sys_Menu_Bar object from receiving events
}

Fl_MacOS_Sys_Menu_Bar_Driver* Fl_MacOS_Sys_Menu_Bar_Driver::driver() {
  static Fl_MacOS_Sys_Menu_Bar_Driver *once = new Fl_MacOS_Sys_Menu_Bar_Driver();
  if (driver_ != once) {
    if (driver_) {
      once->bar = driver_->bar;
      delete driver_;
    }
    driver_ = once;
    if (driver_->bar) driver_->update();
  }
  return once;
}

/*  Class FLMenuItem, derived from NSMenuItem, associates any item of the macOS system menu
    with a corresponding Fl_Menu_Item as follows:
    - if the system item's tag is >= 0, fl_sys_menu_bar->menu() + tag is the address
    of the relevant Fl_Menu_Item;
    - otherwise, the system item's representedObject is the Fl_Menu_Item's address.
    This allows the MacOS system menu to use the same Fl_Menu_Item's as those used by FLTK menus,
    the address of which can be relocated by the FLTK menu logic.
    The "representedObject" is used for non-relocatable Fl_Menu_Item's associated to FL_SUBMENU_POINTER.
    Sending the getFlItem message to a macOS system menu item (of class FLMenuItem) returns the address
    of the relevant Fl_Menu_Item.
*/

// Apple App Menu
const char *Fl_Mac_App_Menu::about = "About %@";
const char *Fl_Mac_App_Menu::print = "Print Front Window & Titlebar";
const char *Fl_Mac_App_Menu::print_no_titlebar = "Print Front Window";
const char *Fl_Mac_App_Menu::toggle_print_titlebar = "Toggle printing of titlebar";
const char *Fl_Mac_App_Menu::services = "Services";
const char *Fl_Mac_App_Menu::hide = "Hide %@";
const char *Fl_Mac_App_Menu::hide_others = "Hide Others";
const char *Fl_Mac_App_Menu::show = "Show All";
const char *Fl_Mac_App_Menu::quit = "Quit %@";


@interface FLMenuItem : NSMenuItem {
}
- (const Fl_Menu_Item*) getFlItem;
- (void) itemCallback:(Fl_Menu_*)menu;
- (void) doCallback;
- (void) customCallback;
- (void) directCallback;
- (void) setKeyEquivalentModifierMask:(int)value;
- (void) setFltkShortcut:(int)key;
+ (int) addNewItem:(const Fl_Menu_Item*)mitem menu:(NSMenu*)menu action:(SEL)selector;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
- (BOOL)validateMenuItem:(NSMenuItem *)item;
#endif
@end

@implementation FLMenuItem
- (const Fl_Menu_Item*) getFlItem
// returns the Fl_Menu_Item corresponding to this system menu item
{
  NSInteger tag = [self tag];
  if (tag >= 0) return fl_sys_menu_bar->menu() + tag;
  return *(const Fl_Menu_Item**)[(NSData*)[self representedObject] bytes];
}
- (void) itemCallback:(Fl_Menu_*)menu
{
  const Fl_Menu_Item *item = [self getFlItem];
  menu->picked(item);
  Fl::flush();
  if ( item->flags & FL_MENU_TOGGLE ) { // update the menu toggle symbol
    [self setState:(item->value() ? NSControlStateValueOn : NSControlStateValueOff)];
  }
  else if ( item->flags & FL_MENU_RADIO ) {     // update the menu radio symbols
    NSMenu* this_menu = [self menu];
    NSInteger flRank = [this_menu indexOfItem:self];
    NSInteger last = [this_menu numberOfItems] - 1;
    int from = (int)flRank;
    while(from > 0) {
      if ([[this_menu itemAtIndex:from-1] isSeparatorItem]) break;
      item = [(FLMenuItem*)[this_menu itemAtIndex:from-1] getFlItem];
      if ( !(item->flags & FL_MENU_RADIO) ) break;
      from--;
    }
    int to = (int)flRank;
    while (to < last) {
      if ([[this_menu itemAtIndex:to+1] isSeparatorItem]) break;
      item = [(FLMenuItem*)[this_menu itemAtIndex:to+1] getFlItem];
      if (!(item->flags & FL_MENU_RADIO)) break;
      to++;
    }
    for(int i =  from; i <= to; i++) {
      NSMenuItem *nsitem = [this_menu itemAtIndex:i];
      [nsitem setState:(nsitem != self ? NSControlStateValueOff : NSControlStateValueOn)];
    }
  }
}
- (void) doCallback
{
  fl_lock_function();
  [self itemCallback:fl_sys_menu_bar];
  fl_unlock_function();
}
- (void) customCallback
{
  fl_lock_function();
  [self itemCallback:custom_menu];
  fl_unlock_function();
}
- (void) directCallback
{
  fl_lock_function();
  Fl_Menu_Item *item = (Fl_Menu_Item *)[(NSData*)[self representedObject] bytes];
  if ( item && item->callback() ) item->do_callback(NULL);
  fl_unlock_function();
}
- (void) setKeyEquivalentModifierMask:(int)value
{
  NSUInteger macMod = 0;
  if ( value & FL_META ) macMod = NSEventModifierFlagCommand;
  if ( value & FL_SHIFT || (value > 0 && value < 127 && isupper(value)) ) macMod |= NSEventModifierFlagShift;
  if ( value & FL_ALT ) macMod |= NSEventModifierFlagOption;
  if ( value & FL_CTRL ) macMod |= NSEventModifierFlagControl;
  [super setKeyEquivalentModifierMask:macMod];
}
- (void) setFltkShortcut:(int)key
{
  // Separate key and modifier
  int mod = key;
  mod &= ~FL_KEY_MASK;  // modifier(s)
  key &=  FL_KEY_MASK;  // key
  unichar mac_key = (unichar)key;
  if ( (key >= (FL_F+1)) && (key <= FL_F_Last) ) { // Handle function keys
    int fkey_num = (key - FL_F);        // 1,2..
    mac_key = NSF1FunctionKey + fkey_num - 1;
  } else if (key == FL_Escape) {
    mac_key = 27;
  } else if (key == FL_Tab) {
    mac_key = NSTabCharacter;
  } else if (key == FL_Enter) {
    mac_key = 0x0d;
  } else if (key == FL_BackSpace) {
    mac_key = NSBackspaceCharacter;
  } else if (key == FL_Delete) {
    mac_key = NSDeleteCharacter;
  } else if (key == FL_Up) {
    mac_key = NSUpArrowFunctionKey;
  } else if (key == FL_Down) {
    mac_key = NSDownArrowFunctionKey;
  } else if (key == FL_Left) {
    mac_key = NSLeftArrowFunctionKey;
  } else if (key == FL_Right) {
    mac_key = NSRightArrowFunctionKey;
  } else if (key == FL_Page_Up) {
    mac_key = NSPageUpFunctionKey;
  } else if (key == FL_Page_Down) {
    mac_key = NSPageDownFunctionKey;
  } else if (key == FL_KP_Enter) {
    mac_key = 0x2324;  // "⌤" U+2324
  } else if (key == FL_Home) {
    mac_key = NSHomeFunctionKey;
  } else if (key == FL_End) {
    mac_key = NSEndFunctionKey;
  }
  [self setKeyEquivalent:[NSString stringWithCharacters:&mac_key length:1]];
  [self setKeyEquivalentModifierMask:mod];
}
+ (int) addNewItem:(const Fl_Menu_Item*)mitem menu:(NSMenu*)menu action:(SEL)selector
{
  char *name = remove_ampersand(mitem->label());
  NSString *title = NSLocalizedString([NSString stringWithUTF8String:name], nil);
  free(name);
  FLMenuItem *item = [[FLMenuItem alloc] initWithTitle:title
                                                action:selector
                                         keyEquivalent:@""];
  // >= 0 if mitem is in the menu items of fl_sys_menu_bar, -1 if not
  NSInteger index = (fl_sys_menu_bar ? fl_sys_menu_bar->find_index(mitem) : -1);
  [item setTag:index];
  if (index < 0) {
    NSData *pointer = [[NSData alloc] initWithBytes:&mitem length:sizeof(Fl_Menu_Item*)];
    [item setRepresentedObject:pointer];
    [pointer release];//pointer will dealloc each time item dealloc's
  }
  [menu addItem:item];
  [item setTarget:item];
  int retval = (int)[menu indexOfItem:item];
  [item release];
  return retval;
}

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
- (BOOL)validateMenuItem:(NSMenuItem *)item {
  // return YES for all but items of the Window menu
  if (fl_mac_os_version < 101200 ||
      Fl_Sys_Menu_Bar::window_menu_style() <= Fl_Sys_Menu_Bar::tabbing_mode_none ||
      [item hasSubmenu]) return YES;
  NSString *title = [[item parentItem] title]; // 10.6
  if (!title || !localized_Window || [title compare:localized_Window] != NSOrderedSame) return YES;
  const Fl_Menu_Item *flitem = [(FLMenuItem*)item getFlItem];
  Fl_Callback *item_cb = flitem->callback();
  if (item_cb == previous_tab_cb || item_cb == next_tab_cb || item_cb == move_tab_cb) {
    // is the current window tabbed?
    Fl_Window *win = Fl::first_window();
    NSWindow *main = win ? (NSWindow*)fl_xid(win) : nil;
    return (main && [main tabbedWindows] != nil);
  } else if (item_cb == merge_all_windows_cb) {
    // is there any untabbed, tabbable window?
    int total = 0, untabbed = 0;
    while ((++flitem)->label()) {
      total++;
      NSWindow *nsw = (NSWindow*)fl_xid( (Fl_Window*)flitem->user_data() );
      if (![nsw tabbedWindows] && [nsw tabbingMode] != NSWindowTabbingModeDisallowed) {
        untabbed++;
      }
    }
    return (untabbed > 0 && total >= 2);
  }
  return YES;
}
#endif
@end


void Fl_MacOS_Sys_Menu_Bar_Driver::about( Fl_Callback *cb, void *user_data)
{
  Fl_Menu_Item aboutItem;
  memset(&aboutItem, 0, sizeof(Fl_Menu_Item));
  aboutItem.callback(cb);
  aboutItem.user_data(user_data);
  NSMenu *appleMenu = [[[NSApp mainMenu] itemAtIndex:0] submenu];
  CFStringRef cfname = CFStringCreateCopy(NULL, (CFStringRef)[[appleMenu itemAtIndex:0] title]);
  [appleMenu removeItemAtIndex:0];
  FLMenuItem *item = [[[FLMenuItem alloc] initWithTitle:(NSString*)cfname
                                                 action:@selector(directCallback)
                                          keyEquivalent:@""] autorelease];
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
  FLMenuItem* menuItem = (FLMenuItem*)[mh itemAtIndex:miCnt];
  [menuItem setFltkShortcut:(m->shortcut_)];
}


/*
 * Set the Toggle and Radio flag based on FLTK flags
 */
static void setMenuFlags( NSMenu* mh, int miCnt, const Fl_Menu_Item *m )
{
  if ( m->flags & FL_MENU_TOGGLE )
  {
    NSMenuItem *menuItem = [mh itemAtIndex:miCnt];
    [menuItem setState:(m->flags & FL_MENU_VALUE ? NSControlStateValueOn : NSControlStateValueOff)];
  }
  else if ( m->flags & FL_MENU_RADIO ) {
    NSMenuItem *menuItem = [mh itemAtIndex:miCnt];
    [menuItem setState:(m->flags & FL_MENU_VALUE ? NSControlStateValueOn : NSControlStateValueOff)];
  }
}

static char *remove_ampersand(const char *s)
{
  char *ret = fl_strdup(s);
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
static void createSubMenu( NSMenu *mh, pFl_Menu_Item &mm,  const Fl_Menu_Item *mitem, SEL selector)
{
  NSMenu *submenu;
  int miCnt, flags;

  if (mitem) {
    NSMenuItem *menuItem;
    char *ts = remove_ampersand(mitem->text);
    NSString *title = NSLocalizedString([NSString stringWithUTF8String:ts], nil);
    free(ts);
    submenu = [[NSMenu alloc] initWithTitle:(NSString*)title];
    [submenu setAutoenablesItems:NO];

    int cnt;
    cnt = (int)[mh numberOfItems];
    cnt--;
    menuItem = [mh itemAtIndex:cnt];
    [menuItem setSubmenu:submenu];
    [submenu release];
  } else submenu = mh;

  while ( mm->text ) {
    if (!mm->visible() ) { // skip invisible items and submenus
      mm = mm->next(0);
      continue;
    }
    miCnt = [FLMenuItem addNewItem:mm menu:submenu
                            action:( (mm->flags & (FL_SUBMENU+FL_SUBMENU_POINTER) && !mm->callback()) ? nil : selector)
            ];
    setMenuFlags( submenu, miCnt, mm );
    setMenuShortcut( submenu, miCnt, mm );
    if (mitem && (mm->flags & FL_MENU_INACTIVE || mitem->flags & FL_MENU_INACTIVE)) {
      NSMenuItem *item = [submenu itemAtIndex:miCnt];
      [item setEnabled:NO];
    }
    flags = mm->flags;
    if ( mm->flags & FL_SUBMENU )
    {
      mm++;
      createSubMenu( submenu, mm, mm - 1, selector);
    }
    else if ( mm->flags & FL_SUBMENU_POINTER )
    {
      const Fl_Menu_Item *smm = (Fl_Menu_Item*)mm->user_data_;
      createSubMenu( submenu, smm, mm, selector);
    }
    if ( flags & FL_MENU_DIVIDER ) {
      [submenu addItem:[NSMenuItem separatorItem]];
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
  int count;//first, delete all existing system menus
  count = (int)[fl_system_menu numberOfItems];
  for(int i = count - 1; i > 0; i--) {
    [fl_system_menu removeItem:[fl_system_menu itemAtIndex:i]];
  }
  if (mm) createSubMenu(fl_system_menu, mm, NULL, @selector(doCallback));
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
  if (localized_Window) {
    NSMenuItem *item = [fl_system_menu itemWithTitle:localized_Window];
    if (item) [[item submenu] setAutoenablesItems:YES];
  }
#endif
}

void Fl_MacOS_Sys_Menu_Bar_Driver::update()
{
  convertToMenuBar(bar->Fl_Menu_::menu());
}


static int process_sys_menu_shortcuts(int event)
{
  if (event != FL_SHORTCUT || !fl_sys_menu_bar || Fl::modal()) return 0;
  // is the last event the shortcut of an item of the fl_sys_menu_bar menu ?
  const Fl_Menu_Item *item = fl_sys_menu_bar->menu()->test_shortcut();
  if (!item) return 0;
  if (item->visible()) // have the system menu process the shortcut, highlighting the corresponding menu
    [[NSApp mainMenu] performKeyEquivalent:[NSApp currentEvent]];
  else // have FLTK process the shortcut associated to an invisible Fl_Menu_Item
    fl_sys_menu_bar->picked(item);
  return 1;
}

Fl_MacOS_Sys_Menu_Bar_Driver::Fl_MacOS_Sys_Menu_Bar_Driver() : Fl_Sys_Menu_Bar_Driver()
{
  window_menu_items = NULL;
  first_window_menu_item = 0;
  Fl::add_handler(process_sys_menu_shortcuts);
}

Fl_MacOS_Sys_Menu_Bar_Driver::~Fl_MacOS_Sys_Menu_Bar_Driver()
{
  Fl::remove_handler(process_sys_menu_shortcuts);
}

void Fl_MacOS_Sys_Menu_Bar_Driver::menu(const Fl_Menu_Item *m)
{
  fl_open_display();
  bar->Fl_Menu_Bar::menu( m );
  convertToMenuBar(m);
}

void Fl_MacOS_Sys_Menu_Bar_Driver::clear()
{
  bar->Fl_Menu_::clear();
  convertToMenuBar(NULL);
}

int Fl_MacOS_Sys_Menu_Bar_Driver::clear_submenu(int index)
{
  int retval = bar->Fl_Menu_::clear_submenu(index);
  if (retval != -1) update();
  return retval;
}

void Fl_MacOS_Sys_Menu_Bar_Driver::remove(int index)
{
  bar->Fl_Menu_::remove(index);
  update();
}

void Fl_MacOS_Sys_Menu_Bar_Driver::replace(int index, const char *name)
{
  bar->Fl_Menu_::replace(index, name);
  update();
}

void Fl_MacOS_Sys_Menu_Bar_Driver::mode(int i, int fl) {
  bar->Fl_Menu_::mode(i, fl);
  update();
}

void Fl_MacOS_Sys_Menu_Bar_Driver::shortcut (int i, int s) {
  bar->Fl_Menu_Bar::shortcut(i, s);
  update();
}

void Fl_MacOS_Sys_Menu_Bar_Driver::setonly (Fl_Menu_Item *item) {
  bar->Fl_Menu_::setonly(item);
  update();
}

int Fl_MacOS_Sys_Menu_Bar_Driver::add(const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags)
{
  fl_open_display();
  int index = bar->Fl_Menu_::add(label, shortcut, cb, user_data, flags);
  update();
  return index;
}

int Fl_MacOS_Sys_Menu_Bar_Driver::add(const char* str)
{
  fl_open_display();
  int index = bar->Fl_Menu_::add(str);
  update();
  return index;
}

int Fl_MacOS_Sys_Menu_Bar_Driver::insert(int index, const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags)
{
  fl_open_display();
   int menu_index = bar->Fl_Menu_::insert(index, label, shortcut, cb, user_data, flags);
   update();
   return menu_index;
}

/** \class Fl_Mac_App_Menu
 Mac OS-specific class allowing to customize and localize the application menu.

 The public class attributes are used to build the application menu. They can be localized
 at run time to any UTF-8 text by placing instructions such as this before fl_open_display()
 gets called:
 \verbatim
 Fl_Mac_App_Menu::print = "Imprimer la fenêtre";
 \endverbatim
 \see \ref osissues_macos for another way to localization.
 */

void Fl_Mac_App_Menu::custom_application_menu_items(const Fl_Menu_Item *m)
{
  fl_open_display(); // create the system menu, if needed
  custom_menu = new Fl_Menu_Bar(0,0,0,0);
  custom_menu->menu(m);
  NSMenu *menu = [[[NSApp mainMenu] itemAtIndex:0] submenu]; // the application menu
  NSInteger to_index;
  if ([[menu itemAtIndex:2] action] != @selector(printPanel)) { // the 'Print' item was removed
    [menu insertItem:[NSMenuItem separatorItem] atIndex:1];
    to_index = 2;
  } else to_index = 5; // after the "Print Front Window/Toggle" items and the separator
  NSInteger count = [menu numberOfItems];
  createSubMenu(menu, m, NULL, @selector(customCallback)); // add new items at end of application menu
  NSInteger count2 = [menu numberOfItems];
  for (NSInteger i = count; i < count2; i++) { // move new items to their desired position in application menu
    NSMenuItem *item = [menu itemAtIndex:i];
    [item retain];
    [menu removeItemAtIndex:i];
    [menu insertItem:item atIndex:to_index++];
    [item release];
  }
}

static void minimize_win_cb(Fl_Widget *, void *data)
{
  [[NSApp mainWindow] miniaturize:nil];
}

static void window_menu_cb(Fl_Widget *, void *data)
{
  if (data) ((Fl_Window*)data)->show();
}

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12

static void previous_tab_cb(Fl_Widget *, void *data)
{
  [[NSApp mainWindow] selectPreviousTab:nil];
}

static void next_tab_cb(Fl_Widget *, void *data)
{
  [[NSApp mainWindow] selectNextTab:nil];
}

static void move_tab_cb(Fl_Widget *, void *data)
{
  [[NSApp mainWindow] moveTabToNewWindow:nil];
}

static void merge_all_windows_cb(Fl_Widget *, void *)
{
  Fl_Window *first = Fl::first_window();
  while (first && (first->parent() || !first->border()))
    first = Fl::next_window(first);
  if (first) {
    [(NSWindow*)fl_xid(first) mergeAllWindows:nil];
  }
}

#endif


static bool window_menu_installed = false;
static int window_menu_items_count = 0;

void Fl_MacOS_Sys_Menu_Bar_Driver::create_window_menu(void)
{
  if (window_menu_style() == Fl_Sys_Menu_Bar::no_window_menu) return;
  if (window_menu_installed) return;
  window_menu_installed = true;
  int rank = 0;
  if (fl_sys_menu_bar && fl_sys_menu_bar->menu()) {
    if (fl_sys_menu_bar->find_index("Window") >= 0) { // there's already a "Window" menu -> don't create another
      window_menu_style_ = Fl_Sys_Menu_Bar::no_window_menu;
      return;
    }
    // put the Window menu last in menu bar or before Help if it's present
    const Fl_Menu_Item *item = fl_sys_menu_bar->menu();
    while (item->label() && strcmp(item->label(), "Help") != 0) {
      item = item->next();
    }
    rank = fl_sys_menu_bar->find_index(item);
  } else if (!fl_sys_menu_bar) {
    fl_open_display();
    new Fl_Sys_Menu_Bar(0,0,0,0);
  }
  if (!window_menu_items_count) {
    window_menu_items_count = 6;
    window_menu_items = (Fl_Menu_Item*)calloc(window_menu_items_count, sizeof(Fl_Menu_Item));
  }
  rank = fl_sys_menu_bar->Fl_Menu_::insert(rank, "Window", 0, NULL, window_menu_items, FL_SUBMENU_POINTER);
  localized_Window = NSLocalizedString(@"Window", nil);
  window_menu_items[0].label("Minimize");
  window_menu_items[0].callback(minimize_win_cb);
  window_menu_items[0].shortcut(FL_COMMAND+'m');
  window_menu_items[0].flags = FL_MENU_DIVIDER;
  first_window_menu_item = 1;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
  if (fl_mac_os_version >= 101200 && window_menu_style() != Fl_Sys_Menu_Bar::tabbing_mode_none) {
    window_menu_items[1].label("Show Previous Tab");
    window_menu_items[1].callback(previous_tab_cb);
    window_menu_items[1].shortcut(FL_SHIFT+FL_CTRL+0x9);
    window_menu_items[2].label("Show Next Tab");
    window_menu_items[2].callback(next_tab_cb);
    window_menu_items[2].shortcut(FL_CTRL+0x9);
    window_menu_items[3].label("Move Tab To New Window");
    window_menu_items[3].callback(move_tab_cb);
    window_menu_items[4].label("Merge All Windows");
    window_menu_items[4].callback(merge_all_windows_cb);
    window_menu_items[4].flags = FL_MENU_DIVIDER;
    first_window_menu_item = 5;
  }
#endif
  fl_sys_menu_bar->menu_end();
  fl_sys_menu_bar->update();
}


void Fl_MacOS_Sys_Menu_Bar_Driver::new_window(Fl_Window *win)
{
  if (!window_menu_style() || !win->label()) return;
  int index = window_menu_items->size() - 1;
  if (index >= window_menu_items_count - 1) {
    window_menu_items_count += 5;
    window_menu_items = (Fl_Menu_Item*)realloc(window_menu_items,
                                    window_menu_items_count * sizeof(Fl_Menu_Item));
    Fl_Menu_Item *item = (Fl_Menu_Item*)fl_sys_menu_bar->find_item("Window");
    item->user_data(window_menu_items);
  }
  const char *p = win->iconlabel() ? win->iconlabel() : win->label();
  window_menu_items[index].label(p);
  window_menu_items[index].callback(window_menu_cb);
  window_menu_items[index].user_data(win);
  window_menu_items[index].flags = FL_MENU_RADIO;
  window_menu_items[index+1].label(NULL);
  window_menu_items[index].setonly();
  fl_sys_menu_bar->update();
}

void Fl_MacOS_Sys_Menu_Bar_Driver::remove_window(Fl_Window *win)
{
  if (!window_menu_style()) return;
  int index = first_window_menu_item;
  if (index < 1) return;
  while (true) {
    Fl_Menu_Item *item = window_menu_items + index;
    if (!item->label()) return;
    if (item->user_data() == win) {
      bool doit = item->value();
      int count = window_menu_items->size();
      if (count - index - 1 > 0) memmove(item, item + 1, (count - index - 1)*sizeof(Fl_Menu_Item));
      memset(window_menu_items + count - 2, 0, sizeof(Fl_Menu_Item));
      if (doit) { // select Fl::first_window() in Window menu
        item = window_menu_items + first_window_menu_item;
        while (item->label() && item->user_data() != Fl::first_window()) item++;
        if (item->label()) {
          ((Fl_Window*)item->user_data())->show();
          item->setonly();
        }
      }
      bar->update();
      break;
    }
    index++;
  }
}

void Fl_MacOS_Sys_Menu_Bar_Driver::rename_window(Fl_Window *win)
{
  if (!window_menu_style()) return;
  int index = first_window_menu_item;
  if (index < 1) return;
  while (true) {
    Fl_Menu_Item *item = window_menu_items + index;
    if (!item->label()) return;
    if (item->user_data() == win) {
      item->label(win->iconlabel() ? win->iconlabel() : win->label());
      bar->update();
      return;
    }
    index++;
  }
}

void fl_mac_set_about(Fl_Callback *cb, void *user_data, int shortcut) {
  Fl_Sys_Menu_Bar::about(cb, user_data);
}


void Fl_MacOS_Sys_Menu_Bar_Driver::play_menu(const Fl_Menu_Item *item) {
  // Use the accessibility interface to programmatically open a menu of the system menubar
  CFArrayRef children = NULL;
  CFIndex count = 0;
  AXUIElementRef element;
  char *label = remove_ampersand(item->label());
  NSString *mac_name = NSLocalizedString([NSString stringWithUTF8String:label], nil);
  free(label);
  AXUIElementRef appElement = AXUIElementCreateApplication(getpid());
  AXUIElementRef menu_bar = NULL;
  AXError error = AXUIElementCopyAttributeValue(appElement, kAXMenuBarAttribute,
                                                (CFTypeRef *)&menu_bar);
  if (!error) error = AXUIElementGetAttributeValueCount(menu_bar, kAXChildrenAttribute, &count);
  if (!error) error = AXUIElementCopyAttributeValues(menu_bar, kAXChildrenAttribute, 0, count,
                                                     &children);
  if (!error) {
    NSEnumerator *enumerator = [(NSArray*)children objectEnumerator];
    [enumerator nextObject]; // skip Apple menu
    [enumerator nextObject]; // skip application menu
    bool need_more = true;
    while (need_more && (element = (AXUIElementRef)[enumerator nextObject]) != nil) {
      CFTypeRef title = NULL;
      need_more = ( AXUIElementCopyAttributeValue(element, kAXTitleAttribute, &title) == 0 );
      if (need_more && [(NSString*)title isEqualToString:mac_name]) {
        AXUIElementPerformAction(element, kAXPressAction);
        need_more = false;
      }
      if (title) CFRelease(title);
    }
  }
  if (menu_bar) CFRelease(menu_bar);
  if (children) CFRelease(children);
  CFRelease(appElement);
}

#endif /* __APPLE__ */
