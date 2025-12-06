//
// Menu code for the Fast Light Tool Kit (FLTK).
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

// This file contains code for implementing Fl_Menu_Item, and for
// methods for bringing up popup menu hierarchies without using the
// Fl_Menu_ widget.

// The menu code is in the process of refactoring.

#include <FL/Fl.H>
#include "Fl_Screen_Driver.H"
#include "Fl_Window_Driver.H"
#include <FL/Fl_Menu_Window.H>
#include <FL/Fl_Menu_.H>
#include <FL/fl_draw.H>
#include <stdio.h>
#include "flstring.h"

// This file will declare:
class Menu_Window_Basetype;
class Menu_Title_Window;
class Menu_Window;
struct Menu_State;

typedef int menu_index_t;
typedef int item_index_t;

static bool is_special_labeltype(uchar t);

// Global variables:

// Global variable that tells the label draw call if `&x` Alt-key shortcuts must be rendered.
extern char fl_draw_shortcut;

// Local variables:

// appearance of current menus are pulled from this parent widget:
static const Fl_Menu_* button = nullptr;

//
// ==== Declarations ===========================================================
//
//
// ---- Menu_State -------------------------------------------------------------
//

// Fl_Menu_Item::popup(...)

// Because Fl::grab() is done, all events go to one of the menu windows.
// But the handle method needs to look at all of them to find out
// what item the user is pointing at.  And it needs a whole lot
// of other state variables to determine what is going on with
// the currently displayed menus.
// So the main loop (handlemenu()) puts all the state in a structure
// and puts a pointer to it in a static location, so the handle()
// on menus can refer to it and alter it.  The handle() method
// changes variables in this state to indicate what item is
// picked, but does not actually alter the display, instead the
// main loop does that.  This is because the X mapping and unmapping
// of windows is slow, and we don't want to fall behind the events.

// values for Menu_State.state:
enum class State {
  INIT = 0,     // no mouse up or down since popup() called
  PUSHED,       // mouse has been pushed on a normal item
  MENU_PUSHED,  // mouse has been pushed on a menu title
  DONE,         // exit the popup, the current item was picked
};

/*
 This class handles the current cascade of menu windows for a pulldown call.
 */
struct Menu_State
{
  // menu item under the mouse pinter or selected by keyboard, or nullptr
  const Fl_Menu_Item* current_item { nullptr };

  // index of the menu window that contains the current_item
  menu_index_t current_menu_ix { 0 };

  // index of current_item within the menu window indicated by menu_number, -1 if none
  item_index_t current_item_ix { -1 };

  // pointers to open menu windows
  Menu_Window* menu_window[20] { nullptr };

  // number of open menuwindows
  menu_index_t num_menus { 0 };

  // if true, pulldown is initiated by a menubar, and menu_window[0] holds the
  // horizontally arranged level 0 menu item list
  bool in_menubar { false };

  // State::INIT, etc. See above
  State state { State::INIT };

  // simulate a button in the top level of a menubar
  Menu_Window* menubar_button_helper { nullptr };

  // check if mouse coordinates are inside any of the menu windows
  bool is_inside(int mx, int my);

  // set the current menu item
  void set_current_item(const Fl_Menu_Item* i, menu_index_t m, item_index_t n);

  // set the current menu item
  void set_current_item(menu_index_t m, item_index_t n);

  // previous item in menu menu if possible
  bool prev_item(menu_index_t menu);

  // go to next item in menu menu if possible
  bool next_item(menu_index_t menu);

  // handle FL_SHORTCUT in any of the menu windows
  int handle_shortcut();

  // move menu item selection left
  int handle_left();

  // move menu item selection right
  int handle_right();

  // handle activation of the selected menu item
  int handle_select();

  // handle menu cancellation
  int handle_cancel();

  // handle any keyboard event from the menu windows
  int handle_keyboard_event();

  // handle all mouse events from the menu windows
  int handle_mouse_events(int e);

  // create a submenu based on the selected menu item m
  bool create_submenu(const Fl_Rect &r, Menu_Window& cw, const Fl_Menu_Item *m,
                      const Fl_Menu_Item *initial_item, bool menubar);

  // delete all menu windows beyond the selected one
  void delete_unused_menus(Menu_Window& cw, const Fl_Menu_Item* m);
};

// Global state of menu windows and popup windows.
static Menu_State* menu_state = nullptr;


//
// ---- Menu_Window_Basetype ---------------------------------------------------
//

/*
  Base type for Menu_Title_Window and Menu_Window, derived from Fl_Menu_Window.
*/
class Menu_Window_Basetype : public Fl_Menu_Window
{
protected:

  /* Create a window that can hold menu items.
   The implementation is in the derived class Menu_Title_Window and Menu_Window.
   \param X, Y, W, H position and size of the window
   \param m saved in member variable `menu` for derived classes
   */
  Menu_Window_Basetype(int X, int Y, int W, int H, const Fl_Menu_Item *m)
  : Fl_Menu_Window(X, Y, W, H, 0),
    menu(m)
  {
    set_menu_window();
    Fl_Window_Driver::driver(this)->set_popup_window();
    end();
    set_modal();
    clear_border();
  }

public:

  // Store a pointer to the first item in the menu array.
  const Fl_Menu_Item* menu { nullptr };

  // Use this to check this is a Fl_Menu_Window or a derived window class.
  virtual Menu_Window* as_menuwindow() { return nullptr; }
};

//
// ---- Menu_Title_Window ------------------------------------------------------
//

/*
  Menu window showing the title above a popup menu.
 */
class Menu_Title_Window : public Menu_Window_Basetype
{
  /* Draw the contents of the menu title window. */
  void draw() override {
    menu->draw(0, 0, w(), h(), button, 2);
  }

public:

  /* Create a window that hold the title of another menu.
   \param X, Y, W, H position and size of the window
   \param[in] L pointer to menu item that holds the label text
   \param[in] inbar true if this is part of an Fl_Menu_Bar
   */
  Menu_Title_Window(int X, int Y, int W, int H, const Fl_Menu_Item* L, bool inbar = false)
  : Menu_Window_Basetype(X, Y, W, H, L),
  in_menubar(inbar) { }

  // If set, title is part of a menubar.
  bool in_menubar { false };
};

//
// ---- Menu_Window ------------------------------------------------------------
//

/* A window that renders the menu items from an array and handles all events.
 The event handler runs in its own loop inside `Fl_Menu_Item::pulldown`.
 All Menu Windows are managed in the struct `menu_state`.
 */
class Menu_Window : public Menu_Window_Basetype
{
  // For tricky direct access
  friend class Fl_Window_Driver;

  // Fl_Menu_Item::pulldown does some direct manipulations
  friend struct Fl_Menu_Item;

  // Draw this window, either entirely, or just the selected and deselect items.
  void draw() override;

  // Draw a single menu item in this window
  void draw_entry(const Fl_Menu_Item*, int i, int erase);

  // Draw the submenu arrow
  void draw_submenu_arrow(const Fl_Rect& bbox);

  // Draw the benu item shortcut text.
  void draw_shortcut(const Fl_Rect& bbox, const Fl_Menu_Item* mi);

  // Draw the menu item divider.
  void draw_divider(const Fl_Rect& bbox);

  // Main event handler
  int handle_part1(int);

  // Kludge to avoid abandoned window on macOS
  int handle_part2(int e, int ret);

  // All open menu windows are positioned relative to this window
  static Fl_Window *parent_;

  // Helper to store the height of the screen that contains the menu windows
  static int display_height_;

public:

  // Create our menu window
  Menu_Window(const Fl_Menu_Item* m, int X, int Y, int W, int H,
              const Fl_Menu_Item* picked, const Fl_Menu_Item* title,
              bool in_menubar = false, bool mb_title = false, int right_edge = 0);

  // Destructor
  ~Menu_Window();

  // Override to fixup the current selection
  void hide() override;

  // Override to handle all incoming events
  int handle(int) override;

  // Change the index of the selected item, -1 for none. Trigger chatty callbacks
  // and marks the window area of the newly selected item for redraw.
  void set_selected(item_index_t);

  // Find the index to the item under the given mouse coordinates.
  item_index_t find_selected(int mx, int my);

  // Calculate the horizontal position of an item by index for horizontal
  // menus inside a menubar.
  int titlex(int);

  // Scroll so item i is visible on screen. This may move the entire window..
  void autoscroll(item_index_t i);

  // Also reposition the title (relative to the parent_ window?)
  void position(int x, int y);

  // return true, if the given root coordinates are inside the window
  bool is_inside(int x, int y);

  // Fake runtime type information
  Menu_Window* as_menuwindow() override { return this; }

  // Optional title for menubar windows and floating menus
  Menu_Title_Window* title { nullptr };

  // Height of the tallest menu item in the array, zero == menubar.
  int item_height { 0 };

  // Number of menu items in the window.
  item_index_t num_items { 0 };

  // Index of selected item, or -1 if none is selected.
  item_index_t selected { -1 };

  // Remember the last item we drew selected, so we can redraw it unselected
  // when the selection changes. -1 if none.
  int drawn_selected { -1 };

  // Width of the longest shortcut key text minus modifier keys
  int shortcut_width { 0 };

  // If set, the title window is also the button in Fl_Menu_Bar
  bool menubar_title { false };

  // In a cascading window, this points to the menu window that opened this menu.
  Menu_Window *origin { nullptr };

  // Used by the window driver
  int offset_y { 0 };
};

//
// ==== Implementations ========================================================
//
//
// ---- Menu_State -------------------------------------------------------------
//

/* Find out if any menu window is under the mouse.
  \return 1 if the coordinates are inside any of the menuwindows
*/
bool Menu_State::is_inside(int mx, int my) {
  for (menu_index_t i=num_menus-1; i>=0; i--) {
    if (menu_window[i]->is_inside(mx, my)) {
      return true;
    }
  }
  return false;
}

/* Remember this item in the state machine.
  \param[in] i current menu item
  \param[in] m index into menu window array
  \param[in] n index into visible item in that menu window
*/
void Menu_State::set_current_item(const Fl_Menu_Item* i, menu_index_t m, item_index_t n) {
  current_item = i;
  current_menu_ix = m;
  current_item_ix = n;
}

/* Find and store a menu item in the state machine.
  \param[in] m index into menu window array
  \param[in] n index into visible item in that menu window
*/
void Menu_State::set_current_item(menu_index_t m, item_index_t n) {
  current_item = (n >= 0) ? menu_window[m]->menu->next(n) : 0;
  current_menu_ix = m;
  current_item_ix = n;
}

/* Go down to the next selectable menu item.
  If the event button is FL_Down, increment once, else go to the bottom of the menu.
  \param[in] menu index into menu window list
  \return `true` if an item was found, `false` if the menu wrapped
*/
bool Menu_State::next_item(menu_index_t menu) { // go to next item in menu menu if possible
  // `menu` is -1 if no item is currently selected, so use the first menu
  if (menu < 0)
    menu = 0;
  Menu_Window &m = *(menu_window[menu]);
  item_index_t item = (menu == current_menu_ix) ? current_item_ix : m.selected;
  bool wrapped = false;
  do {
    while (++item < m.num_items) {
      const Fl_Menu_Item* m1 = m.menu->next(item);
      if (m1->selectable()) {
        set_current_item(m1, menu, item);
        return true;
      }
    }
    if (wrapped) break;
    item = -1;
    wrapped = true;
  }
  while (Fl::event_key() != FL_Down);
  return false;
}

/* Go up to the previous selectable menu item.
  If the event button is FL_Up, decrement once, else go to the top of the menu.
  \param[in] menu index into menu window list
  \return `true` if an item was found, `false` if the menu wrapped
*/
bool Menu_State::prev_item(menu_index_t menu) { // previous item in menu menu if possible
  // `menu` is -1 if no item is currently selected, so use the first menu
  if (menu < 0)
    menu = 0;
  Menu_Window &m = *(menu_window[menu]);
  item_index_t item = (menu == current_menu_ix) ? current_item_ix : m.selected;
  bool wrapped = false;
  do {
    while (--item >= 0) {
      const Fl_Menu_Item* m1 = m.menu->next(item);
      if (m1->selectable()) {
        set_current_item(m1, menu, item);
        return true;
      }
    }
    if (wrapped) break;
    item = m.num_items;
    wrapped = true;
  }
  while (Fl::event_key() != FL_Up);
  return false;
}

/* Handle the FL_SHORTCUT event.
  \return 1 if the shortcut was found in the menu and handled.
*/
int Menu_State::handle_shortcut() {
  for (menu_index_t mymenu = num_menus; mymenu--;) {
    Menu_Window &mw = *(menu_window[mymenu]);
    int item;
    const Fl_Menu_Item* m = mw.menu->find_shortcut(&item);
    if (m) {
      set_current_item(m, mymenu, item);
      if (!m->submenu())
        state = State::DONE;
      return 1;
    }
  }
  return 0;
}

/* Move menu item selection left.
  \return 1
*/
int Menu_State::handle_left() {
  if (in_menubar && current_menu_ix<=1) {
    prev_item(0);
  } else if (current_menu_ix>0) {
    set_current_item(current_menu_ix-1, menu_window[current_menu_ix-1]->selected);
  }
  return 1;
}

/* Move menu item selection right.
  \return 1
*/
int Menu_State::handle_right() {
  if (in_menubar && (current_menu_ix<=0 || (current_menu_ix == num_menus-1))) {
    next_item(0);
  } else if (current_menu_ix < num_menus-1) {
    next_item(current_menu_ix+1);
  }
  return 1;
}

/* Handle activation of the selected menu item.
  \return 1
*/
int Menu_State::handle_select() {
  // if the current item is a submenu with no callback,
  // simulate FL_Right to enter the submenu
  if (   current_item
      && (!in_menubar || current_menu_ix > 0)
      && current_item->selectable()
      && current_item->submenu()
      && !current_item->callback_)
  {
    return handle_right();
  }
  // Ignore keypresses over inactive items, mark KEYBOARD event as used.
  if (current_item && !current_item->selectable())
    return 1;
  // Mark the menu 'done' which will trigger the callback
  state = State::DONE;
  return 1;
}

/* Handle menu cancellation.
  \return 1
*/
int Menu_State::handle_cancel() {
  set_current_item(0, -1, 0);
  state = State::DONE;
  return 1;
}

/* Handle any keyboard event from the menu windows.
  \return 1 if the keyboard event was handled, else 0
*/
int Menu_State::handle_keyboard_event() {
  switch (Fl::event_key()) {
    case FL_BackSpace:
      prev_item(current_menu_ix);
      return 1;
    case FL_Up:
      if (in_menubar && current_menu_ix == 0) {
        // Do nothing...
      } else if (prev_item(current_menu_ix)) {
        // Do nothing...
      } else if (in_menubar && current_menu_ix==1) {
        set_current_item(0, menu_window[0]->selected);
      }
      return 1;
    case FL_Tab:
      if (Fl::event_shift()) {
        prev_item(current_menu_ix);
        return 1;
      }
      if (in_menubar && current_menu_ix == 0)
        return handle_right();
      /* FALLTHROUGH */
    case FL_Down:
      if (current_menu_ix || !in_menubar) {
        next_item(current_menu_ix);
      } else if (current_menu_ix < num_menus-1) {
        next_item(current_menu_ix+1);
      }
      return 1;
    case FL_Right:
      return handle_right();
    case FL_Left:
      return handle_left();
    case FL_Enter:
    case FL_KP_Enter:
    case ' ':
      return handle_select();
    case FL_Escape:
      return handle_cancel();
  }
  return 0;
}

/* Handle all mouse events from the menu windows.
  \return 1 if the event was handled, else 0
*/
int Menu_State::handle_mouse_events(int e) {
  switch (e) {
    case FL_MOVE: {
      static int use_part1_extra = Fl::screen_driver()->need_menu_handle_part1_extra();
      if (use_part1_extra && state == State::DONE) {
        return 1; // Fix for STR #2619
      }
    }
      /* FALLTHROUGH */
    case FL_ENTER:
    case FL_PUSH:
    case FL_DRAG:
    {
      int mx = Fl::event_x_root();
      int my = Fl::event_y_root();
      item_index_t item = 0;
      menu_index_t mymenu = num_menus-1;
      // Clicking or dragging outside menu cancels it...
      if ((!in_menubar || mymenu) && !is_inside(mx, my)) {
        set_current_item(0, -1, 0);
        if (e==FL_PUSH)
          state = State::DONE;
        return 1;
      }
      for (mymenu = num_menus-1; ; mymenu--) {
        item = menu_window[mymenu]->find_selected(mx, my);
        if (item >= 0)
          break;
        if (mymenu <= 0) {
          // buttons in menubars must be deselected if we move outside of them!
          if (current_menu_ix==-1 && e==FL_PUSH) {
            state = State::DONE;
            return 1;
          }
          if (current_item && current_menu_ix==0 && !current_item->submenu()) {
            if (e==FL_PUSH) {
              state = State::DONE;
              set_current_item(0, -1, 0);
            }
            return 1;
          }
          // all others can stay selected
          return 0;
        }
      }
      set_current_item(mymenu, item);
      if (e == FL_PUSH) {
        if (current_item && current_item->submenu() // this is a menu title
            && item != menu_window[mymenu]->selected // and it is not already on
            && !current_item->callback_) // and it does not have a callback
          state = State::MENU_PUSHED;
        else
          state = State::PUSHED;
      }
    }
      return 1;
    case FL_RELEASE:
      // Mouse must either be held down/dragged some, or this must be
      // the second click (not the one that popped up the menu):
      if (   !Fl::event_is_click()
          || state == State::PUSHED
          || (in_menubar && current_item && !current_item->submenu()) // button
          ) {
#if 0 // makes the check/radio items leave the menu up
        const Fl_Menu_Item* m = current_item;
        if (m && button && (m->flags & (FL_MENU_TOGGLE|FL_MENU_RADIO))) {
          ((Fl_Menu_*)button)->picked(m);
          p[menu_number]->redraw();
        } else
#endif
          // do nothing if they try to pick an inactive item, or a submenu with no callback
          if (!current_item || (current_item->selectable() &&
                                   (!current_item->submenu() || current_item->callback_ || (in_menubar && current_menu_ix <= 0))))
            state = State::DONE;
      }
      return 1;
  }
  return 0;
}

/* Create a submenu based on the selected menu item m.
  \param[in] r suggested rectangle for new menu window
  \param[in] cw window of menu window with currently selected item
  \param[in] m currently selected menu item
  \param[in] initial_item if set, the new menu is aligned so that this item
      is close to m, or under the mouse
  \param[in] menubar if set, the menu list is part of a menubar, so the window
      at 0 is a horizontal menu item list
  \return true if the menu list was update to show the initial_item
*/
bool Menu_State::create_submenu(const Fl_Rect &r, Menu_Window& cw, const Fl_Menu_Item *m,
                                const Fl_Menu_Item *initial_item, bool menubar) {
  const Fl_Menu_Item* title = m;
  const Fl_Menu_Item* menutable;
  if (m->flags&FL_SUBMENU)
    menutable = m+1;
  else
    menutable = (Fl_Menu_Item*)(m)->user_data_;
  // figure out where new menu goes:
  int nX, nY;
  if (!current_menu_ix && in_menubar) {      // menu off a menubar:
    nX = cw.x() + cw.titlex(current_item_ix);
    nY = cw.y() + cw.h();
    initial_item = 0;
  } else {
    nX = cw.x() + cw.w();
    nY = cw.y() + current_item_ix * cw.item_height;
    title = 0;
  }
  if (initial_item) { // bring up submenu containing initial item:
    Menu_Window* n = new Menu_Window(menutable, r.x(), r.y(), r.w(), r.h(), initial_item, title, false, false, cw.x());
    menu_window[num_menus++] = n;
    if (num_menus >= 2)
      menu_window[num_menus-1]->origin = menu_window[num_menus-2];
    // move all earlier menus to line up with this new one:
    if (n->selected>=0) {
      int dy = n->y()-nY;
      int dx = n->x()-nX;
      int waX, waY, waW, waH;
      Fl_Window_Driver::driver(n)->menu_window_area(waX, waY, waW, waH, Fl::screen_num(r.x(), r.y()));
      for (menu_index_t menu = 0; menu <= current_menu_ix; menu++) {
        Menu_Window* tt = menu_window[menu];
        int nx = tt->x()+dx; if (nx < waX) {nx = waX; dx = -tt->x() + waX;}
        int ny = tt->y()+dy; if (ny < waY) {ny = waY; dy = -tt->y() + waY;}
        tt->position(nx, ny);
      }
      menu_state->set_current_item(num_menus-1, n->selected);
      return true;
    }
  } else if (num_menus > current_menu_ix+1 &&
             menu_window[current_menu_ix+1]->menu == menutable) {
    // the menu is already up:
    while (num_menus > current_menu_ix+2) delete menu_window[--num_menus];
    menu_window[num_menus-1]->set_selected(-1);
  } else {
    // delete all the old menus and create new one:
    while (num_menus > current_menu_ix+1) delete menu_window[--num_menus];
    menu_window[num_menus++] = new Menu_Window(menutable, nX, nY,
        title?1:0, 0, nullptr, title, false, (bool)menubar,
        (title ? 0 : cw.x()) );
    if (num_menus >= 2 && menu_window[num_menus-2]->item_height) {
      menu_window[num_menus-1]->origin = menu_window[num_menus-2];
    }
  }
  return false;
}

/* Delete all menu windows beyond the selected one.
  This deletes menus in the list that are beyond the selected menu window w.
  It also fakes a menubar button entry by only showing the title of an emty menu.
  \param[in] cw the selected menu window
  \param[in] m the selected menu item within the menu window
*/
void Menu_State::delete_unused_menus(Menu_Window& cw, const Fl_Menu_Item* m) {
  while (num_menus > current_menu_ix+1)
    delete menu_window[--num_menus];
  if (in_menubar && (current_menu_ix == 0)) {
    // kludge so "menubar buttons" turn "on" by using menu title:
    menubar_button_helper = new Menu_Window(nullptr,
        cw.x()+cw.titlex(current_item_ix),
        cw.y()+cw.h(), 0, 0,
        nullptr, m, false, true);
    menubar_button_helper->title->show();
  }
}

//
// ---- Menu_Window ------------------------------------------------------------
//

// Static members:
Fl_Window *Menu_Window::parent_ = nullptr;
int Menu_Window::display_height_ = 0;


/*
 Construct a menu window that can render a list of menu items.
 \param[in] m pointer to the first menu item in the array
 \param[in] X, Y position relative to parent_
 \param[in] Wp, Hp initial minimum size; if Wp is 0, the window will open on the
    screen with X and Y, else it will open in the screen with the mouse pointer.
 \param[in] picked pointer to the currently picked menu item, can be nullptr
 \param[in] t pointer to the menutitle window
 \param[in] in_menubar set if part of an Fl_Menu_Bar menu
 \param[in] mb_title set if the title window is also the button in Fl_Menu_Bar
 \param[in] right_edge maximum right edge of menu on current screen(?), not used
 */
Menu_Window::Menu_Window(const Fl_Menu_Item* m, int X, int Y, int Wp, int Hp,
                         const Fl_Menu_Item* picked, const Fl_Menu_Item* t,
                         bool in_menubar, bool mb_title, int right_edge)
: Menu_Window_Basetype(X, Y, Wp, Hp, m)
{
  int scr_x, scr_y, scr_w, scr_h; // available screen rect for the menu
  int tx = X, ty = Y;             /// initial title origin
  menubar_title = mb_title;
  int n = (Wp > 0 ? Fl::screen_num(X, Y) : -1);
  Fl_Window_Driver::driver(this)->menu_window_area(scr_x, scr_y, scr_w, scr_h, n);
  if (!right_edge || right_edge > scr_x+scr_w)
    right_edge = scr_x+scr_w;

  if (m) m = m->first(); // find the first item that needs to be rendered
  if (button) {
    Fl_Boxtype b = button->menu_box();
    if (b==FL_NO_BOX)
      b = button->box();
    if (b==FL_NO_BOX)
      b = FL_FLAT_BOX;
    box(b);
  } else {
    box(FL_UP_BOX);
  }
  color(button && !Fl::scheme() ? button->color() : FL_GRAY);
  {
    item_index_t j = 0;
    if (m) for (const Fl_Menu_Item* m1=m; ; m1 = m1->next(), j++) {
      if (picked) {
        if (m1 == picked) {
          selected = j;
          picked = 0;
        } else if (m1 > picked) {
          selected = j-1;
          picked = 0;
          Wp = Hp = 0;
        }
      }
      if (!m1->text) break;
    }
    num_items = j;
  }

  if (in_menubar) {
    item_height = 0;
    title = 0;
    return;
  }

  item_height = 1;

  int shortcuts_w = 0;  // maximum width in pixels of all shortcut texts w/o modifiers
  int modifiers_w = 0;  // maximum width of all shortcut modifiers texts
  int titile_w = 0;     // width of the title window
  int title_h = 0;      // height of the title window
  if (t) titile_w = t->measure(&title_h, button) + 12;
  int W = 0;
  if (m) for (; m->text; m = m->next()) {
    int hh;
    int w1 = m->measure(&hh, button);
    if (hh+Fl::menu_linespacing()>item_height) item_height = hh+Fl::menu_linespacing();
    if (m->flags&(FL_SUBMENU|FL_SUBMENU_POINTER))
      w1 += FL_NORMAL_SIZE;
    if (w1 > W) W = w1;
    // calculate the maximum width of all shortcuts
    if (m->shortcut_) {
      // s is a pointer to the UTF-8 string for the entire shortcut
      // k points only to the key part (minus the modifier keys)
      const char *k, *s = fl_shortcut_label(m->shortcut_, &k);
      if (fl_utf_nb_char((const unsigned char*)k, (int) strlen(k))<=4) {
        // a regular shortcut has a right-justified modifier followed by a left-justified key
        w1 = int(fl_width(s, (int) (k-s)));
        if (w1 > modifiers_w) modifiers_w = w1;
        w1 = int(fl_width(k))+4;
        if (w1 > shortcuts_w) shortcuts_w = w1;
      } else {
        // a shortcut with a long modifier is right-justified to the menu
        w1 = int(fl_width(s))+4;
        if (w1 > (modifiers_w+shortcuts_w)) {
          modifiers_w = w1-shortcuts_w;
        }
      }
    }
  }
  shortcut_width = shortcuts_w;
  if (selected >= 0 && !Wp) X -= W/2;
  int BW = Fl::box_dx(box());
  W += shortcuts_w+modifiers_w+2*BW+7;
  if (Wp > W) W = Wp;
  if (titile_w > W) W = titile_w;

  if (X < scr_x) X = scr_x;
  // this change improves popup submenu positioning at right screen edge,
  // but it makes right_edge argument useless
  //if (X > scr_x+scr_w-W) X = right_edge-W;
  if (X > scr_x+scr_w-W) X = scr_x+scr_w-W;
  x(X); w(W);
  h((num_items ? item_height*num_items-4 : 0)+2*BW+3);
  if (selected >= 0) {
    Y = Y+(Hp-item_height)/2-selected*item_height-BW;
  } else {
    Y = Y+Hp;
    // if the menu hits the bottom of the screen, we try to draw
    // it above the menubar instead. We will not adjust any menu
    // that has a selected item.
    if (Y+h()>scr_y+scr_h && Y-h()>=scr_y) {
      if (Hp>1) {
        // if we know the height of the Fl_Menu_, use it
        Y = Y-Hp-h();
      } else if (t) {
        // assume that the menubar item height relates to the first
        // menuitem as well
        Y = Y-item_height-h()-Fl::box_dh(box());
      } else {
        // draw the menu to the right
        Y = Y-h()+item_height+Fl::box_dy(box());
      }
      if (t) {
        if (menubar_title) {
          Y = Y + Fl::menu_linespacing() - Fl::box_dw(button->box());
        } else {
          Y += 2*title_h+2*BW+3;
        }
      }
    }
  }
  if (m) y(Y); else {y(Y-2); w(1); h(1);}

  if (t) {
    if (menubar_title) {
      int dy = Fl::box_dy(button->box())+1;
      int ht = button->h()-dy*2;
      title = new Menu_Title_Window(tx, ty-ht-dy, titile_w, ht, t, true);
    } else {
      int dy = 2;
      int ht = title_h+2*BW+3;
      title = new Menu_Title_Window(X, Y-ht-dy, titile_w, ht, t);
    }
  } else {
    title = 0;
  }
}

/* Destroy this window. */
Menu_Window::~Menu_Window() {
  hide();
  delete title;
}

/* Fixup the selection and hide this window */
void Menu_Window::hide() {
  set_selected(-1);
  Menu_Window_Basetype::hide();
}

/* Handle events sent to the window.
 \param[in] e event number
 \return 1 if the event was used
 */
int Menu_Window::handle(int e) {
  /* In FLTK 1.3.4, the equivalent of handle_part2() is called for the Mac OS and X11 platforms
   and "svn blame" shows it is here to fix STR #449.
   But this STR is Mac OS-specific.
   So, it is unclear why handle_part2() is called also for X11.

   Furthermore, calling handle_part2() for X11 renders the
   fix for STR #2619 below necessary. If handle_part2() is not called under X11,
   then STR #2619 does not occur. need_menu_handle_part1_extra() activates this fix.

   FLTK 1.3.4 behavior:
   Fl::screen_driver()->need_menu_handle_part2() returns true on Mac + X11
   Fl::screen_driver()->need_menu_handle_part1_extra() returns true on X11

   Alternative behavior that seems equally correct:
   Fl::screen_driver()->need_menu_handle_part2() returns true on Mac
   need_menu_handle_part1_extra() does not exist

   Other alternative:
   Neither need_menu_handle_part2() nor need_menu_handle_part1_extra() exist
   --> the menuwindow code is entirely cross-platform and simpler.
   It makes a small difference with Mac OS when resizing a window with a menu on:
   the menu disappears after the end of the resize rather than at its beginning.
   Apple applications do close popups at the beginning of resizes.
   */
  static int use_part2 = Fl::screen_driver()->need_menu_handle_part2();
  int ret = handle_part1(e);
  if (use_part2) ret = handle_part2(e, ret);
  return ret;
}

/* Window event handling implementation.
 \param[in] e event number
 \return 1 if the event was used
 */
int Menu_Window::handle_part1(int e) {
  Menu_State &pp = *menu_state;
  switch (e) {
    case FL_KEYBOARD:
      if (pp.handle_keyboard_event()) return 1;
      break;
    case FL_SHORTCUT:
      if (pp.handle_shortcut()) return 1;
      break;
    case FL_MOVE:
    case FL_ENTER:
    case FL_PUSH:
    case FL_DRAG:
    case FL_RELEASE:
      if (pp.handle_mouse_events(e)) return 1;
      break;
  }
  return Fl_Window::handle(e);
}

int Menu_Window::handle_part2(int e, int ret) {
  // This off-route takes care of the "detached menu" bug on OS X (STR #449).
  // Apple event handler requires that we hide all menu windows right
  // now, so that Carbon can continue undisturbed with handling window
  // manager events, like dragging the application window.
  Menu_State &pp = *menu_state;
  if (pp.state == State::DONE) {
    hide();
    if (pp.menubar_button_helper) {
      pp.menubar_button_helper->hide();
      if (pp.menubar_button_helper->title)
        pp.menubar_button_helper->title->hide();
    }
    menu_index_t i = pp.num_menus;
    while (i>0) {
      Menu_Window *mw = pp.menu_window[--i];
      if (mw) {
        mw->hide();
        if (mw->title)
          mw->title->hide();
      }
    }
  }
  return ret;
}

/* Set a new selected item.
 \param[in] n index into visible item list
 */
void Menu_Window::set_selected(item_index_t n) {
  if (n != selected) {
    if ((selected!=-1) && (menu)) {
      const Fl_Menu_Item *mi = menu->next(selected);
      if ((mi) && (mi->callback_) && (mi->flags & FL_MENU_CHATTY))
        mi->do_callback(this, FL_REASON_LOST_FOCUS);
    }
    selected = n;
    if ((selected!=-1) && (menu)) {
      const Fl_Menu_Item *mi = menu->next(selected);
      if ((mi) && (mi->callback_) && (mi->flags & FL_MENU_CHATTY))
        mi->do_callback(this, FL_REASON_GOT_FOCUS);
    }
    damage(FL_DAMAGE_CHILD);
  }
}

/* Find the item at the give pixel position.
 \param[in] mx, my position in pixels
 \return index of item that is under the pixel, or -1 for none
 */
item_index_t Menu_Window::find_selected(int mx, int my) {
  if (!menu || !menu->text) return -1;
  mx -= x();
  my -= y();
  if (my < 0 || my >= h()) return -1;
  if (!item_height) { // menubar
    int xx = 3; int n = 0;
    const Fl_Menu_Item* m = menu->first();
    for (; ; m = m->next(), n++) {
      if (!m->text) return -1;
      xx += m->measure(0, button) + 16;
      if (xx > mx) break;
    }
    return n;
  }
  if (mx < Fl::box_dx(box()) || mx >= w()) return -1;
  item_index_t n = (my-Fl::box_dx(box())-1)/item_height;
  if (n < 0 || n>=num_items) return -1;
  return n;
}

/* Return horizontal position for item n in a menubar.
 \return position in window in pixels.
 */
int Menu_Window::titlex(int n) {
  const Fl_Menu_Item* m;
  int xx = 3;
  for (m=menu->first(); n--; m = m->next())
    xx += m->measure(0, button) + 16;
  return xx;
}

/* Scroll so item i is visible on screen.
 May scroll or move the window.
 \param[in] n index into visible menu items
 */
void Menu_Window::autoscroll(item_index_t n) {
  int scr_y, scr_h;
  int Y = y()+Fl::box_dx(box())+2+n*item_height;

  int xx, ww;
  Fl_Window_Driver::driver(this)->menu_window_area(xx, scr_y, ww, scr_h, this->screen_num());
  if (n==0 && Y <= scr_y + item_height) {
    Y = scr_y - Y + 10;
  } else if (Y <= scr_y + item_height) {
    Y = scr_y - Y + 10 + item_height;
  } else {
    Y = Y+item_height-scr_h-scr_y;
    if (Y < 0) return;
    Y = -Y-10;
  }
  Fl_Window_Driver::driver(this)->reposition_menu_window(x(), y()+Y);
  // y(y()+Y); // don't wait for response from X
}

/* Set the position of this menu and its title window. */
void Menu_Window::position(int X, int Y) {
  if (title) {
    title->position(X, title->y()+Y-y());
  }
  Fl_Menu_Window::position(X, Y);
  // x(X); y(Y); // don't wait for response from X
}

/* Check if mouse is positions over the window.
 \return 1, if the given root coordinates are inside the window
 */
bool Menu_Window::is_inside(int mx, int my) {
  if ( mx < x_root() || mx >= x_root() + w() ||
      my < y_root() || my >= y_root() + h()) {
    return false;
  }
  if (item_height == 0 && find_selected(mx, my) == -1) {
    // in the menubar but out from any menu header
    return false;
  }
  return true;
}

/* Draw one menu item.
 \param[in] m pointer to the item
 \param[in] n index into the visible item list
 \param[in] eraseit if set, redraw the unselected background
 */
void Menu_Window::draw_entry(const Fl_Menu_Item* m, int n, int eraseit) {
  if (!m) return; // this happens if -1 is selected item and redrawn

  Fl_Rect bbox {
    Fl::box_dx(box()),
    Fl::box_dy(box()) + 1 + n*item_height + Fl::menu_linespacing()/2 - 2,
    w() - Fl::box_dw(box()) - 1/*sic*/,
    item_height - Fl::menu_linespacing()
  };

  // Clear the entire item rect including the spacing
  if (eraseit && n != selected) {
    fl_push_clip(bbox.x()+1, bbox.y()-(Fl::menu_linespacing()-2)/2,
                 bbox.w()-2, bbox.h()+(Fl::menu_linespacing()-2));
    draw_box(box(), 0, 0, w(), h(), button ? button->color() : color());
    fl_pop_clip();
  }

  // Draw the checkbox, radio box, the menu icon, and the label
  m->draw(bbox.x(), bbox.y(), bbox.w(), bbox.h(), button, n==selected);

  // Draw additional decorations on the right side of the label
  if (m->submenu()) {
    draw_submenu_arrow(bbox);
  } else if (m->shortcut_) {
    draw_shortcut(bbox, m);
  }
  if (m->flags & FL_MENU_DIVIDER) {
    draw_divider(bbox);
  }
}

/* Draw the submenu arrow.
  \param[in] bbox menu item bounding box
 */
void Menu_Window::draw_submenu_arrow(const Fl_Rect& bbox) {
  // calculate the bounding box of the submenu pointer (arrow)
  int sz = ((bbox.h()-2) & (-2)) + 1 ;  // must be odd for better centering
  if (sz > 13) sz = 13;           // limit arrow size
  int x1 = bbox.x() + bbox.w() - sz - 2;      // left border
  int y1 = bbox.y() + (bbox.h()-sz)/2 + 1;    // top border

  // draw an arrow whose style depends on the active scheme
  fl_draw_arrow(Fl_Rect(x1, y1, sz, sz), FL_ARROW_SINGLE, FL_ORIENT_RIGHT, fl_color());
}

/* Draw the benu item shortcut text.
  \param[in] bbox menu item bounding box
  \param[in] m take the shortcut from this menu item
 */
// the shortcuts and arrows assume fl_color() was left set by draw():
void Menu_Window::draw_shortcut(const Fl_Rect& bbox, const Fl_Menu_Item* m) {
  // Draw the shortcut modifiers and key texts
  Fl_Font f = m->labelsize_ || m->labelfont_ ? (Fl_Font)m->labelfont_ :
  button ? button->textfont() : FL_HELVETICA;
  fl_font(f, m->labelsize_ ? m->labelsize_ :
          button ? button->textsize() : FL_NORMAL_SIZE);
  const char *k, *s = fl_shortcut_label(m->shortcut_, &k);
  if (fl_utf_nb_char((const unsigned char*)k, (int) strlen(k))<=4) {
    // right-align the modifiers and left-align the key
    char *buf = (char*)malloc(k-s+1);
    memcpy(buf, s, k-s); buf[k-s] = 0;
    fl_draw(buf, bbox.x(), bbox.y(),
            bbox.w()-shortcut_width, bbox.h(), FL_ALIGN_RIGHT);
    fl_draw(  k, bbox.x()+bbox.w()-shortcut_width, bbox.y(),
            shortcut_width, bbox.h(), FL_ALIGN_LEFT);
    free(buf);
  } else {
    // right-align to the menu
    fl_draw(s, bbox.x(), bbox.y(), bbox.w()-4, bbox.h(), FL_ALIGN_RIGHT);
  }
}

/* Draw the divider. It's part of the menu, but drawn in the spacing area.
  \param[in] bbox menu item bounding box
*/
void Menu_Window::draw_divider(const Fl_Rect& bbox) {
  int y_offset = (Fl::menu_linespacing()-2)/2;
  fl_color(FL_DARK3);
  fl_xyline(bbox.x()-1, bbox.b() + y_offset, bbox.r());
  fl_color(FL_LIGHT3);
  fl_xyline(bbox.x()-1, bbox.b() + y_offset+1, bbox.r());
}


/* Draw the menuwindow. If the damage flags are FL_DAMAGE_CHILD, only redraw
 the old selected and the newly selected items.
 */
void Menu_Window::draw() {
  if (damage() != FL_DAMAGE_CHILD) {    // complete redraw
    if (    (box() != FL_FLAT_BOX)
         && (Fl::is_scheme( "gtk+" ) || Fl::is_scheme( "plastic") || Fl::is_scheme( "gleam" ) )) {
      // Draw a FL_FLAT_BOX to avoid on macOS the white corners of the menus
      fl_draw_box( FL_FLAT_BOX, 0, 0, w(), h(),
                  button ? button->color() : color());
    }
    fl_draw_box(box(), 0, 0, w(), h(), button ? button->color() : color());
    if (menu) {
      const Fl_Menu_Item* m; int j;
      for (m=menu->first(), j=0; m->text; j++, m = m->next())
        draw_entry(m, j, 0);
    }
  } else {
    if (damage() & FL_DAMAGE_CHILD && selected!=drawn_selected) {
      // change selection
      draw_entry(menu->next(drawn_selected), drawn_selected, 1);
      draw_entry(menu->next(selected), selected, 1);
    }
  }
  drawn_selected = selected;
}

//
// ---- Fl_Menu_Item -----------------------------------------------------------
//

/** Size of the menu starting from this menu item.

 This method counts all menu items starting with \p this menu item,
 including all menu items in the same (sub)menu level, all nested
 submenus, \b and the terminating empty (0) menu item.

 It does \b not count menu items referred to by FL_SUBMENU_POINTER
 menu items (except the single menu item with FL_SUBMENU_POINTER).

 All menu items counted are consecutive in memory (one array).

 Example:

 \code
 schemechoice = new Fl_Choice(X+125,Y,140,25,"FLTK Scheme");
 schemechoice->add("none");
 schemechoice->add("plastic");
 schemechoice->add("gtk+");
 schemechoice->add("gleam");
 printf("schemechoice->menu()->size() = %d\n", schemechoice->menu()->size());
 \endcode

 Output:

 schemechoice->menu()->%size() = 5
 */
int Fl_Menu_Item::size() const {
  const Fl_Menu_Item* m = this;
  int nest = 0;
  for (;;) {
    if (!m->text) {
      if (!nest) return (int) (m-this+1);
      nest--;
    } else if (m->flags & FL_SUBMENU) {
      nest++;
    }
    m++;
  }
}

/* Advance a pointer to next visible or invisible item of a menu array.

 This function skips over the contents of submenus.

 \param[in] m start from this menu item inside an array
 \return a pointer to the next menu item. If the label() of the returned
 item is nullptr, the function reached the end of the array and
 no next item was found.
 */
static const Fl_Menu_Item* next_visible_or_not(const Fl_Menu_Item* m) {
  int nest = 0;
  do {
    if (!m->text) {
      if (!nest) return m;
      nest--;
    } else if (m->flags&FL_SUBMENU) {
      nest++;
    }
    m++;
  }
  while (nest);
  return m;
}

/** Advance a pointer by n items through a menu array.

 This function skips the contents of submenus, and also invisible items.
 There are two calls so that you can advance through const and non-const data.

 \param[in] n advance by n items
 \return a pointer to the next menu item. If the label() of the returned
 item is nullptr, the function reached the end of the array and
 no next item was found.
 */
const Fl_Menu_Item* Fl_Menu_Item::next(int n) const {
  if (n < 0) return 0; // this is so selected==-1 returns NULL
  const Fl_Menu_Item* m = this;
  if (!m->visible()) n++;
  while (n) {
    m = next_visible_or_not(m);
    if (m->visible() || !m->text) n--;
  }
  return m;
}

/**
 Measures width of label, including effect of & characters.
 Optionally, can get height if hp is not NULL.
 \param[out] hp return the height of the label
 \param[in] m for this menu item
 \return width of the label without shortcut text, but including checkbox
 for radio and check items.
 */
int Fl_Menu_Item::measure(int* hp, const Fl_Menu_* m) const {
  Fl_Label l;
  l.value   = text;
  l.image   = 0;
  l.deimage = 0;
  l.type    = labeltype_;
  l.font    = labelsize_ || labelfont_ ? labelfont_ : (m ? m->textfont() : FL_HELVETICA);
  l.size    = labelsize_ ? labelsize_ : m ? m->textsize() : FL_NORMAL_SIZE;
  l.color   = FL_FOREGROUND_COLOR; // this makes no difference?
  l.h_margin_ = l.v_margin_ = l.spacing = 0;
  fl_draw_shortcut = 1;
  int w = 0; int h = 0;
  l.measure(w, hp ? *hp : h);
  fl_draw_shortcut = 0;
  if (flags & (FL_MENU_TOGGLE|FL_MENU_RADIO))
    w += FL_NORMAL_SIZE + 4;
  return w;
}

/**
 Draws the menu item in the bounding box selected or unselected.
 This does not draw the shortcut: see menuwindow::drawentry().
 \param[in] x, y, w, h bounding box for the menu item
 \param[in] m draw the background, label, and checkbox of this item
 \param[in] draw_mode 0 = draw unselected, 1 = draw selected, 2 = draw menu title
 */
void Fl_Menu_Item::draw(int x, int y, int w, int h, const Fl_Menu_* m,
                        int draw_mode) const {
  Fl_Label l;
  l.value   = text;
  l.image   = 0;
  l.deimage = 0;
  l.type    = labeltype_;
  l.font    = labelsize_ || labelfont_ ? labelfont_ : (m ? m->textfont() : FL_HELVETICA);
  l.size    = labelsize_ ? labelsize_ : m ? m->textsize() : FL_NORMAL_SIZE;
  l.color   = labelcolor_ ? labelcolor_ : m ? m->textcolor() : int(FL_FOREGROUND_COLOR);
  l.h_margin_ = l.v_margin_ = l.spacing = 0;
  if (!active()) l.color = fl_inactive((Fl_Color)l.color);
  if (draw_mode) {
    Fl_Color r = m ? m->selection_color() : FL_SELECTION_COLOR;
    Fl_Boxtype b = m && m->down_box() ? m->down_box() : FL_FLAT_BOX;
    l.color = fl_contrast((Fl_Color)labelcolor_, r);
    if (draw_mode == 2) { // menu title
      fl_draw_box(b, x, y, w, h, r);
      x += 3;
      w -= 8;
    } else {
      fl_draw_box(b, x+1, y-(Fl::menu_linespacing()-2)/2, w-2, h+(Fl::menu_linespacing()-2), r);
    }
  }

  if (flags & (FL_MENU_TOGGLE|FL_MENU_RADIO)) {
    int d = (h - FL_NORMAL_SIZE + 1) / 2;
    int W = h - 2 * d;

    Fl_Color check_color = labelcolor_;
    if (Fl::is_scheme("gtk+"))
      check_color = FL_SELECTION_COLOR;
    check_color = fl_contrast(check_color, FL_BACKGROUND2_COLOR);

    if (flags & FL_MENU_RADIO) {

      fl_draw_box(FL_ROUND_DOWN_BOX, x+2, y+d, W, W, FL_BACKGROUND2_COLOR);
      if (value()) {
        int tW = (W - Fl::box_dw(FL_ROUND_DOWN_BOX)) / 2 + 1;
        if ((W - tW) & 1) tW++; // Make sure difference is even to center
        int td = (W - tW) / 2;
        fl_draw_radio(x + td + 1, y + d + td - 1, tW + 2, check_color);
      } // FL_MENU_RADIO && value()

    } else { // FL_MENU_TOGGLE && ! FL_MENU_RADIO

      fl_draw_box(FL_DOWN_BOX, x+2, y+d, W, W, FL_BACKGROUND2_COLOR);
      if (value()) {
        fl_draw_check(Fl_Rect(x+3, y+d+1, W-2, W-2), check_color);
      }
    }
    x += W + 3;
    w -= W + 3;
  }

  if (!fl_draw_shortcut) fl_draw_shortcut = 1;
  l.draw(x+3, y, w>6 ? w-6 : 0, h, FL_ALIGN_LEFT);
  fl_draw_shortcut = 0;
}

/**
  Pulldown() is similar to popup(), but a rectangle is provided
  to position the menu.

  The menu is made at least \p W wide, and the picked item \p initial_item
  is centered over the rectangle (like Fl_Choice uses).

  If \p initial_item is \p NULL or not found, the menu is aligned just
  below the rectangle (like a pulldown menu).

  The \p title and \p menubar arguments are used internally by the
  Fl_Menu_Bar widget.
*/
const Fl_Menu_Item* Fl_Menu_Item::pulldown(
    int X, int Y, int W, int H,
    const Fl_Menu_Item* initial_item,
    const Fl_Menu_* pbutton,
    const Fl_Menu_Item* title,
    int menubar) const
{
  Fl_Group::current(0); // fix possible user error...

  // track the Fl_Menu_ widget to make sure we notice if it gets
  // deleted while the menu is open (STR #3503)
  Fl_Widget_Tracker wp((Fl_Widget *)pbutton);

  button = pbutton;
  if (pbutton && pbutton->window()) {
    Menu_Window::parent_ = pbutton->top_window();
    for (Fl_Window* w = pbutton->window(); w; w = w->window()) {
      X += w->x();
      Y += w->y();
    }
  } else {
    X += Fl::event_x_root()-Fl::event_x();
    Y += Fl::event_y_root()-Fl::event_y();
    Menu_Window::parent_ = Fl::first_window();
  }

  int XX, YY, WW;
  Fl::screen_xywh(XX, YY, WW, Menu_Window::display_height_, Menu_Window::parent_->screen_num());
  Menu_Window mw(this, X, Y, W, H, initial_item, title, (bool)menubar);
  Fl::grab(mw);
  // If we grab the mouse pointer, we should also make sure that it is visible.
  if (Menu_Window::parent_)
    Menu_Window::parent_->cursor(FL_CURSOR_DEFAULT);
  Menu_State pp; menu_state = &pp;
  pp.menu_window[0] = &mw;
  pp.num_menus = 1;
  pp.in_menubar = (bool)menubar;

  // preselected item, pop up submenus if necessary:
  if (initial_item && mw.selected >= 0) {
    menu_state->set_current_item(0, mw.selected);
    goto STARTUP;
  }

  pp.current_item = nullptr;
  pp.current_menu_ix = 0;
  pp.current_item_ix = -1;
  if (menubar) {
    // find the initial menu
    if (!mw.handle(FL_DRAG)) {
      Fl::grab(0);
      return 0;
    }
  }
  initial_item = pp.current_item;
  if (initial_item) {
    if (menubar && !initial_item->selectable()) { // pointing at inactive item
      Fl::grab(0);
      return NULL;
    }
    goto STARTUP;
  }

  // the main loop: runs until p.state goes to State::DONE or the menu
  // widget is deleted (e.g. from a timer callback, see STR #3503):
  for (;;) {

    // make sure all the menus are shown:
    {
      for (menu_index_t k = menubar ? 1 : 0; k < pp.num_menus; k++) {
        if (!pp.menu_window[k]->shown()) {
          if (pp.menu_window[k]->title) pp.menu_window[k]->title->show();
          pp.menu_window[k]->show();
        }
      }
    }

    // get events:
    {
      const Fl_Menu_Item* oldi = pp.current_item;
      Fl::wait();
      if (pbutton && wp.deleted()) // menu widget has been deleted (STR #3503)
        break;
      if (pp.state == State::DONE) break; // done.
      if (pp.current_item == oldi) continue;
    }

    // only do rest if item changes:
    if (pp.menubar_button_helper) {
      delete pp.menubar_button_helper;
      pp.menubar_button_helper = nullptr;
    } // turn off "menubar button"

    if (!pp.current_item) { // pointing at nothing
      // turn off selection in deepest menu, but don't erase other menus:
      pp.menu_window[pp.num_menus-1]->set_selected(-1);
      continue;
    }

    if (pp.menubar_button_helper) {
      delete pp.menubar_button_helper;
      pp.menubar_button_helper = nullptr;
    }
    initial_item = 0; // stop the startup code
    if (pp.current_menu_ix < 0 || pp.current_menu_ix >= pp.num_menus) {
      initial_item = 0; // turn off startup code
      continue;
    }
    pp.menu_window[pp.current_menu_ix]->autoscroll(pp.current_item_ix);

  STARTUP:
    Menu_Window& cw = *pp.menu_window[pp.current_menu_ix];
    const Fl_Menu_Item* m = pp.current_item;
    if (!m || !m->selectable()) { // pointing at inactive item
      cw.set_selected(-1);
      initial_item = 0; // turn off startup code
      continue;
    }
    cw.set_selected(pp.current_item_ix);

    if (m==initial_item) initial_item=0; // stop the startup code if item found
    if (m->submenu()) {
      if (pp.create_submenu(Fl_Rect { X, Y, W, H }, cw, m, initial_item, menubar))
        goto STARTUP;
    } else { // !m->submenu():
      pp.delete_unused_menus(cw, m);
    }
  }
  const Fl_Menu_Item* m = (pbutton && wp.deleted()) ? NULL : pp.current_item;
  delete pp.menubar_button_helper;
  while (pp.num_menus>1)
    delete pp.menu_window[--pp.num_menus];
  mw.hide();
  Fl::grab(0);
  Menu_Window::parent_ = NULL;
  return m;
}

/**
  This method is called by widgets that want to display menus.

  The menu stays up until the user picks an item or dismisses it.
  The selected item (or NULL if none) is returned. <I>This does not
  do the callbacks or change the state of check or radio items.</I>

  The menu is positioned so the cursor is centered over the item
  picked.  This will work even if \p picked is in a submenu.
  If \p picked is zero or not in the menu item table the menu is
  positioned with the cursor in the top-left corner.

  \param[in] X,Y the position of the mouse cursor, relative to the
  window that got the most recent event (usually you can pass
  Fl::event_x() and Fl::event_y() unchanged here).

  \param[in] title a character string title for the menu.  If
  non-zero a small box appears above the menu with the title in it.

  \param[in] picked if this pointer is not NULL, the popup menu will appear
  so that the picked menu is under the mouse pointer.

  \param[in] menu_button is a pointer to an Fl_Menu_ from which the color and
  boxtypes for the menu are pulled.  If NULL then defaults are used.

  \return a pointer to the menu item selected by the user, or NULL
*/
const Fl_Menu_Item* Fl_Menu_Item::popup(
  int X, int Y,
  const char* title,
  const Fl_Menu_Item* picked,
  const Fl_Menu_* menu_button
) const {
  static Fl_Menu_Item dummy; // static so it is all zeros
  dummy.text = title;
  return pulldown(X, Y, 0, 0, picked, menu_button, title ? &dummy : 0);
}

/**
 Search only the top level menu for a shortcut.
 Either &x in the label or the shortcut fields are used.

 This tests the current event, which must be an FL_KEYBOARD or
 FL_SHORTCUT, against a shortcut value.

 \param ip returns the index of the item, if \p ip is not NULL.
 \param require_alt if true: match only if Alt key is pressed.

 \return found Fl_Menu_Item or NULL
 */
const Fl_Menu_Item* Fl_Menu_Item::find_shortcut(int* ip, const bool require_alt) const {
  const Fl_Menu_Item* m = this;
  if (m) for (int ii = 0; m->text; m = next_visible_or_not(m), ii++) {
    if (m->active()) {
      if (Fl::test_shortcut(m->shortcut_)
          || (!is_special_labeltype(m->labeltype_) && Fl_Widget::test_shortcut(m->text, require_alt))
          || (m->labeltype_ == _FL_MULTI_LABEL
              && !is_special_labeltype(((Fl_Multi_Label*)m->text)->typea)
              && Fl_Widget::test_shortcut(((Fl_Multi_Label*)m->text)->labela, require_alt))
          || (m->labeltype_ == _FL_MULTI_LABEL
              && !is_special_labeltype(((Fl_Multi_Label*)m->text)->typeb)
              && Fl_Widget::test_shortcut(((Fl_Multi_Label*)m->text)->labelb, require_alt))) {
        if (ip) *ip=ii;
        return m;
      }
    }
  }
  return 0;
}

// Recursive search of all submenus for anything with this key as a
// shortcut.  Only uses the shortcut field, ignores &x in the labels:
/**
 This is designed to be called by a widgets handle() method in
 response to a FL_SHORTCUT event.  If the current event matches
 one of the items shortcut, that item is returned.  If the keystroke
 does not match any shortcuts then NULL is returned.  This only
 matches the shortcut() fields, not the letters in the title
 preceeded by '
 */
const Fl_Menu_Item* Fl_Menu_Item::test_shortcut() const {
  const Fl_Menu_Item* m = this;
  const Fl_Menu_Item* ret = 0;
  if (m) for (; m->text; m = next_visible_or_not(m)) {
    if (m->active()) {
      // return immediately any match of an item in top level menu:
      if (Fl::test_shortcut(m->shortcut_)) return m;
      // if (Fl_Widget::test_shortcut(m->text)) return m;
      // only return matches from lower menu if nothing found in top menu:
      if (!ret && m->submenu()) {
        const Fl_Menu_Item* s =
        (m->flags&FL_SUBMENU) ? m+1:(const Fl_Menu_Item*)m->user_data_;
        ret = s->test_shortcut();
      }
    }
  }
  return ret;
}

//
// ---- Fl_Window_Driver -------------------------------------------------------
//

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

/** The Fl_Window from which currently displayed popups originate.
  Optionally, gives also the height of the display containing this window.
  \param[out] display_height return the height of the display here.
  \return pointer to the owning window
*/
Fl_Window *Fl_Window_Driver::menu_parent(int *display_height) {
  if (display_height) *display_height = Menu_Window::display_height_;
  return Menu_Window::parent_;
}

/* Cast to menuwindow if win is of claa menuwindow and the driver is initialized. */
static Menu_Window *to_menuwindow(Fl_Window *win) {
  if (!Fl_Window_Driver::driver(win)->popup_window() || !win->menu_window()) return nullptr;
  return ((Menu_Window_Basetype*)win)->as_menuwindow();
}

/** Accessor to the "origin" member variable of class menuwindow.
  Variable origin is not NULL when 2 menuwindow's occur, one being a submenu of the other;
  it links the menuwindow at right to the one at left.
*/
Fl_Window *Fl_Window_Driver::menu_leftorigin(Fl_Window *win) {
  Menu_Window *mwin = to_menuwindow(win);
  return (mwin ? mwin->origin : NULL);
}

/** Accessor to the "title" member variable of class menuwindow */
Fl_Window *Fl_Window_Driver::menu_title(Fl_Window *win) {
  Menu_Window *mwin = to_menuwindow(win);
  return (mwin ? mwin->title : NULL);
}

/** Accessor to the "itemheight" member variable of class menuwindow */
int Fl_Window_Driver::menu_itemheight(Fl_Window *win) {
  Menu_Window *mwin = to_menuwindow(win);
  return (mwin ? mwin->item_height : 0);
}

/** Accessor to the "menubartitle" member variable of class menuwindow */
int Fl_Window_Driver::menu_bartitle(Fl_Window *win) {
  Menu_Window *mwin = to_menuwindow(win);
  return (mwin ? mwin->menubar_title : 0);
}

/** Accessor to the "selected" member variable of class menuwindow */
int Fl_Window_Driver::menu_selected(Fl_Window *win) {
  Menu_Window *mwin = to_menuwindow(win);
  return (mwin ? mwin->selected : -1);
}

/** Accessor to the address of the offset_y member variable of class menuwindow */
int *Fl_Window_Driver::menu_offset_y(Fl_Window *win) {
  Menu_Window *mwin = to_menuwindow(win);
  return (mwin ? &(mwin->offset_y) : NULL);
}

/** Returns whether win is a non-menubar menutitle */
bool Fl_Window_Driver::is_floating_title(Fl_Window *win) {
  if (!Fl_Window_Driver::driver(win)->popup_window() || !win->menu_window()) return false;
  Fl_Window *mwin = ((Menu_Window_Basetype*)win)->as_menuwindow();
  return !mwin && !((Menu_Title_Window*)win)->in_menubar;
}

/** Makes sure that the tall menu's selected item is visible in display */
void Fl_Window_Driver::scroll_to_selected_item(Fl_Window *win) {
  Menu_Window *mwin = to_menuwindow(win);
  if (mwin && mwin->selected > 0) {
    mwin->autoscroll(mwin->selected);
  }
}

/**
 \}
 \endcond
 */

//
// ---- helper functions -------------------------------------------------------
//

static bool is_special_labeltype(uchar t) {
  return t == _FL_MULTI_LABEL || t == _FL_ICON_LABEL || t == _FL_IMAGE_LABEL;
}

