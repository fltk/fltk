//
// Window widget class for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Widget_Tracker.H>
#include <FL/fl_ask.H>

#include <algorithm> // for std::swap

struct recent_data {
  const char *text;
};

static const int max_recent_items = 5;
static recent_data recent_items[max_recent_items] = {{"File 1"}, {"File 2"}, {"File 3"}, {"File 4"}, {"File 5"}};
static recent_data *focussed_recent_data = nullptr;
static bool showing_popup_on_menu = false;

static void quit_cb(Fl_Widget*, void*);
static void restart_cb(Fl_Widget*, void*);
static void recent_cb(Fl_Widget*, void*);

static Fl_Menu_Item *find_menu_with_callback(Fl_Menu_Item *menu, Fl_Callback cb, void *ud = nullptr);
static int override_event_dispatch(int e, Fl_Window *w);
static bool handle_menu(int e, Fl_Window *w);
static void show_popup_on_menu(Fl_Window *w, Fl_Window *grab, bool show_title);
static void hide_empty_items(Fl_Menu_Item *menu);

static Fl_Menu_Item menutable[] = {
  {"&File", 0,0,0, FL_SUBMENU}, 
    {"&New",     FL_CTRL+'n'},
    {"&Open",    FL_CTRL+'o', 0,0, FL_MENU_DIVIDER},
    {"Recent 1", 0, recent_cb, &recent_items[0], FL_MENU_CHATTY},
    {"Recent 2", 0, recent_cb, &recent_items[1], FL_MENU_CHATTY},
    {"Recent 3", 0, recent_cb, &recent_items[2], FL_MENU_CHATTY},
    {"Recent 4", 0, recent_cb, &recent_items[3], FL_MENU_CHATTY},
    {"Recent 5", 0, recent_cb, &recent_items[4], FL_MENU_CHATTY|FL_MENU_DIVIDER},
    {"&Quit", 0, quit_cb},
    {0},
  {"&Edit", 0,0,0, FL_SUBMENU},
    {"&Copy",  FL_CTRL+'c'},
    {"&Paste", FL_CTRL+'v', 0,0, FL_MENU_DIVIDER},
    {"&Reload 'Recent files'", 0, restart_cb},
    {0},
  {0}
};

class MenuBar : public Fl_Menu_Bar {
  using base = Fl_Menu_Bar;
public:
  MenuBar(int X, int Y, int W, int H, const char *l=0) : base(X,Y,W,H,l) {}
  int handle(int e) override {
    if(e == FL_BEFORE_MENU)
      hide_empty_items(const_cast<Fl_Menu_Item*>(menu()));
    return base::handle(e);
  }
};

int main(int argc, char **argv) {
  Fl::event_dispatch(override_event_dispatch);

  Fl_Double_Window win(400, 200, "Menu-Edit");
  
  MenuBar menubar(0, 0, 400, 30);
  menubar.menu(menutable);

  Fl_Box b(0, 30, 400, 170, "Right-click on the 'Recent ...' menu items in the File menu.");
  win.resizable(&b);
  
  win.show(argc, argv);
  return Fl::run();
}

static void quit_cb(Fl_Widget*, void*) {
  exit(0);
}

static void restart_cb(Fl_Widget*, void*) {
  Fl_Menu_Item *mi = find_menu_with_callback(menutable, recent_cb);
  int i = 0; 
  for(;mi && mi->text && mi->callback() == recent_cb && i < max_recent_items; ++mi, ++i) {
    //mi->user_data(&recent_items[i]); // Note that the items maight have been reordered.

    mi->labeltype(FL_NORMAL_LABEL);
    mi->flags &= ~FL_MENU_INVISIBLE;

    if(i + 1 == max_recent_items)
      mi->flags |= FL_MENU_DIVIDER;
    else
      mi->flags &= ~FL_MENU_DIVIDER;
  }
}

static void recent_cb(Fl_Widget*, void *_data) {
  recent_data *data = (recent_data*)_data;
  switch(Fl::callback_reason()) {
    case FL_REASON_GOT_FOCUS:
      focussed_recent_data = data;
      break;
    case FL_REASON_LOST_FOCUS:
      if(focussed_recent_data == data) 
        focussed_recent_data = nullptr;
      break;
    case FL_REASON_SELECTED: {
      Fl_Menu_Item *item = find_menu_with_callback(menutable, recent_cb, data);
      fl_message("Selected %s: Open %s", item ? item->text : "?", data->text);
    } break;
  }
}

static Fl_Menu_Item *find_menu_with_callback(Fl_Menu_Item *menu, Fl_Callback cb, void *ud) {
  if(!menu)
    return nullptr;
  for(; menu->text; menu = menu->next()) {
    if(menu->callback() == cb && (!ud || menu->user_data() == ud))
      return menu;
    if(menu->flags & FL_SUBMENU) {
      if(Fl_Menu_Item *found = find_menu_with_callback(menu + 1, cb, ud))
        return found;
    }
  }
  return nullptr;
}

void hide_empty_items(Fl_Menu_Item *menu) {
  if(!menu)
    return;
  
  Fl_Menu_Item *last_visible = nullptr;
  while(menu->text) {
    if(menu->labeltype() == FL_NO_LABEL) {
      if(last_visible) {
        if(menu->flags & FL_MENU_DIVIDER)
          last_visible->flags |= FL_MENU_DIVIDER;
      }
      menu->flags|= FL_MENU_INVISIBLE;
      menu = menu->next(0);
      continue;
    }
    
    last_visible = menu;
    if(menu->flags & FL_SUBMENU) {
      hide_empty_items(menu + 1);
    }
    menu = menu->next();
  }
}

static int override_event_dispatch(int e, Fl_Window *w) {
  if(handle_menu(e, w))
    return 1;

  return Fl::handle_(e, w);
}

static bool handle_menu(int e, Fl_Window *w) {
  Fl_Window *grab = Fl::grab();
  if(!w || !w->menu_window() && (!grab || !grab->menu_window()))
    return false;
  
  // A menu is being displayed (w or grab).
  switch(e) {
    case FL_PUSH: 
      if(Fl::event_button() == FL_RIGHT_MOUSE) {
        return true;
      }
      break;
    
    case FL_RELEASE:
      if(Fl::event_button() == FL_RIGHT_MOUSE) {
        Fl::handle_(FL_MOVE, w); // Ensure that the menu system highlights the item under the mouse
        show_popup_on_menu(w, grab, /* show_title */ false);
        return true;
      }
      break;

    case FL_MOVE:
      Fl::handle_(FL_MOVE, w);
      w->cursor(focussed_recent_data && !showing_popup_on_menu ? FL_CURSOR_HELP : FL_CURSOR_DEFAULT);
      return true;
      
    case FL_KEYBOARD:
      if(Fl::test_shortcut(FL_Menu) || Fl::test_shortcut(FL_SHIFT + FL_F+10)) {
        show_popup_on_menu(w, grab, /* show_title */ true);
        return true;
      }
      break;
  }

  return false;
}

static void show_popup_on_menu(Fl_Window *w, Fl_Window *grab, bool show_title) {
  if(showing_popup_on_menu)
    return; // recursion
  
  if(!focussed_recent_data)
    return; // not on "Recent #" menu item

  Fl_Menu_Item *orig_item = find_menu_with_callback(menutable, recent_cb, focussed_recent_data);
  if(!orig_item)
    return; // not on "Recent #" menu item

  if(orig_item->labeltype() == FL_NO_LABEL)
    return; // On a previously "removed" menu item
  
  Fl_Widget_Tracker track_grab(grab);
  Fl_Widget_Tracker track_w(w);
    
  Fl_Menu_Item popup[] = {
    {"Select", 0,0,0,0,0, FL_HELVETICA_BOLD },
    {"Remove", 0,0,0, FL_MENU_DIVIDER},
    {"Cancel"},
    {0}
  };

  showing_popup_on_menu = true;
  const Fl_Menu_Item *picked = popup->popup(Fl::event_x(), Fl::event_y(), show_title ? orig_item->label() : nullptr);
  showing_popup_on_menu = false;

  bool close_outer_menu = false;
  if(!track_w.exists() || (grab && !track_grab.exists())) {
    // Menu lost
    return;
  }

  if(!Fl::grab() && grab) {
    // Nested menu did not restore grab properly. Grabbing again would cause wrong fl_capture := current menu window
    // Which goes bonkers when the menu window is dismissed by hovering another menu.
    printf("Lost grab. Close menu\n");
    fl_beep();
      
    Fl::grab(grab); // So that the ESC key will go to the menu window
    close_outer_menu = true;
  }

  if(close_outer_menu) {
    // Send ESC key to original menu to close it
    Fl::e_keysym = FL_Escape;
    Fl::handle_(FL_KEYBOARD, w);
  } else if(picked == &popup[0]) { // "Select"
    // Send ENTER key to original menu to close invoke the item
    Fl::e_keysym = FL_Enter;
    Fl::handle_(FL_KEYBOARD, w);
  } else if(picked == &popup[1]) { // "Remove"
    // "disable" the item by hiding its label. Keep menu open. 
    // Caution: Do not really disable or hide the item, because the working menu machinery does not properly recognize such changes.
    orig_item->labeltype(FL_NO_LABEL);

    // Move "empty" label to bottom
    for(Fl_Menu_Item *mi = orig_item + 1; mi->text && mi->callback() == recent_cb; ++mi) {
      using namespace std;
      swap(mi[-1], mi[0]);
      if(mi[-1].flags & FL_MENU_DIVIDER) {
        mi[-1].flags &= ~FL_MENU_DIVIDER;
        mi[0].flags |= FL_MENU_DIVIDER;
      }
    }

    // redraw the popup window. For simplicity redraw them all
    for(Fl_Window *win = Fl::first_window(); win; win = Fl::next_window(win)) {
      if(win->menu_window())
        win->redraw();
    }
       
    // Let the original (still open) menu recognize the change
    Fl::handle_(FL_MOVE, w);
  } else {
    Fl::handle_(FL_MOVE, w); // Let the original (still open) menu recognize that the mouse was probaly moved around a lot
  }
}
