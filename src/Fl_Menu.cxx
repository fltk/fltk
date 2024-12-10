//
// Menu code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

// Warning: this menu code is quite a mess!

// This file contains code for implementing Fl_Menu_Item, and for
// methods for bringing up popup menu hierarchies without using the
// Fl_Menu_ widget.

#include <FL/Fl.H>
#include "Fl_Screen_Driver.H"
#include "Fl_Window_Driver.H"
#include <FL/Fl_Menu_Window.H>
#include <FL/Fl_Menu_.H>
#include <FL/fl_draw.H>
#include <stdio.h>
#include "flstring.h"

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

// Advance a pointer to next visible or invisible item of a menu array,
// skipping the contents of submenus.
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

/**
  Advance a pointer by n items through a menu array, skipping
  the contents of submenus and invisible items.  There are two calls so
  that you can advance through const and non-const data.
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

// appearance of current menus are pulled from this parent widget:
static const Fl_Menu_* button=0;

////////////////////////////////////////////////////////////////
class menuwindow;

// utility class covering both menuwindow and menutitle
class window_with_items : public Fl_Menu_Window {
protected:
  window_with_items(int X, int Y, int W, int H, const Fl_Menu_Item *m) :
    Fl_Menu_Window(X, Y, W, H, 0) {
      menu = m;
      set_menu_window();
      Fl_Window_Driver::driver(this)->set_popup_window();
      end();
      set_modal();
      clear_border();
    }
public:
  const Fl_Menu_Item* menu;
  virtual menuwindow* as_menuwindow() { return NULL; }
};

// tiny window for title of menu:
class menutitle : public window_with_items {
  void draw() FL_OVERRIDE;
public:
  menutitle(int X, int Y, int W, int H, const Fl_Menu_Item*, bool menubar = false);
  bool in_menubar;
};

// each vertical menu has one of these:
class menuwindow : public window_with_items {
  friend class Fl_Window_Driver;
  friend struct Fl_Menu_Item;
  void draw() FL_OVERRIDE;
  void drawentry(const Fl_Menu_Item*, int i, int erase);
  int handle_part1(int);
  int handle_part2(int e, int ret);
  static Fl_Window *parent_;
  static int display_height_;
public:
  menutitle* title;
  int handle(int) FL_OVERRIDE;
  int itemheight;       // zero == menubar
  int numitems;
  int selected;
  int drawn_selected;   // last redraw has this selected
  int shortcutWidth;
  menuwindow(const Fl_Menu_Item* m, int X, int Y, int W, int H,
             const Fl_Menu_Item* picked, const Fl_Menu_Item* title,
             int menubar = 0, int menubar_title = 0, int right_edge = 0);
  ~menuwindow();
  void set_selected(int);
  int find_selected(int mx, int my);
  int titlex(int);
  void autoscroll(int);
  void position(int x, int y);
  int is_inside(int x, int y);
  menuwindow* as_menuwindow() FL_OVERRIDE { return this; }
  int menubartitle;
  menuwindow *origin;
  int offset_y;
};

Fl_Window *menuwindow::parent_ = NULL;
int menuwindow::display_height_ = 0;

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

/** The Fl_Window from which currently displayed popups originate.
 Optionally, gives also the height of the display containing this window */
Fl_Window *Fl_Window_Driver::menu_parent(int *display_height) {
  if (display_height) *display_height = menuwindow::display_height_;
  return menuwindow::parent_;
}

static menuwindow *to_menuwindow(Fl_Window *win) {
  if (!Fl_Window_Driver::driver(win)->popup_window() || !win->menu_window()) return NULL;
  return ((window_with_items*)win)->as_menuwindow();
}

/** Accessor to the "origin" member variable of class menuwindow.
 Variable origin is not NULL when 2 menuwindow's occur, one being a submenu of the other;
 it links the menuwindow at right to the one at left. */
Fl_Window *Fl_Window_Driver::menu_leftorigin(Fl_Window *win) {
  menuwindow *mwin = to_menuwindow(win);
  return (mwin ? mwin->origin : NULL);
}

/** Accessor to the "title" member variable of class menuwindow */
Fl_Window *Fl_Window_Driver::menu_title(Fl_Window *win) {
  menuwindow *mwin = to_menuwindow(win);
  return (mwin ? mwin->title : NULL);
}

/** Accessor to the "itemheight" member variable of class menuwindow */
int Fl_Window_Driver::menu_itemheight(Fl_Window *win) {
  menuwindow *mwin = to_menuwindow(win);
  return (mwin ? mwin->itemheight : 0);
}

/** Accessor to the "menubartitle" member variable of class menuwindow */
int Fl_Window_Driver::menu_bartitle(Fl_Window *win) {
  menuwindow *mwin = to_menuwindow(win);
  return (mwin ? mwin->menubartitle : 0);
}

/** Accessor to the "selected" member variable of class menuwindow */
int Fl_Window_Driver::menu_selected(Fl_Window *win) {
  menuwindow *mwin = to_menuwindow(win);
  return (mwin ? mwin->selected : -1);
}

/** Accessor to the address of the offset_y member variable of class menuwindow */
int *Fl_Window_Driver::menu_offset_y(Fl_Window *win) {
  menuwindow *mwin = to_menuwindow(win);
  return (mwin ? &(mwin->offset_y) : NULL);
}

/** Returns whether win is a non-menubar menutitle */
bool Fl_Window_Driver::is_floating_title(Fl_Window *win) {
  if (!Fl_Window_Driver::driver(win)->popup_window() || !win->menu_window()) return false;
  Fl_Window *mwin = ((window_with_items*)win)->as_menuwindow();
  return !mwin && !((menutitle*)win)->in_menubar;
}

/** Makes sure that the tall menu's selected item is visible in display */
void Fl_Window_Driver::scroll_to_selected_item(Fl_Window *win) {
  menuwindow *mwin = to_menuwindow(win);
  if (mwin && mwin->selected > 0) {
    mwin->autoscroll(mwin->selected);
  }
}

/**
 \}
 \endcond
 */

extern char fl_draw_shortcut;

/**
  Measures width of label, including effect of & characters.
  Optionally, can get height if hp is not NULL.
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
  if (flags & (FL_MENU_TOGGLE|FL_MENU_RADIO)) w += FL_NORMAL_SIZE + 4;
  return w;
}

/** Draws the menu item in bounding box x,y,w,h, optionally selects the item. */
void Fl_Menu_Item::draw(int x, int y, int w, int h, const Fl_Menu_* m,
                        int selected) const {
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
  if (selected) {
    Fl_Color r = m ? m->selection_color() : FL_SELECTION_COLOR;
    Fl_Boxtype b = m && m->down_box() ? m->down_box() : FL_FLAT_BOX;
    l.color = fl_contrast((Fl_Color)labelcolor_, r);
    if (selected == 2) { // menu title
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

menutitle::menutitle(int X, int Y, int W, int H, const Fl_Menu_Item* L, bool inbar) :
  window_with_items(X, Y, W, H, L) {
  in_menubar = inbar;
}

menuwindow::menuwindow(const Fl_Menu_Item* m, int X, int Y, int Wp, int Hp,
                       const Fl_Menu_Item* picked, const Fl_Menu_Item* t,
                       int menubar, int menubar_title, int right_edge)
  : window_with_items(X, Y, Wp, Hp, m)
{
  int scr_x, scr_y, scr_w, scr_h;
  int tx = X, ty = Y;
  menubartitle = menubar_title;
  origin = NULL;
  offset_y = 0;
  int n = (Wp > 0 ? Fl::screen_num(X, Y) : -1);
  Fl_Window_Driver::driver(this)->menu_window_area(scr_x, scr_y, scr_w, scr_h, n);
  if (!right_edge || right_edge > scr_x+scr_w) right_edge = scr_x+scr_w;

  if (m) m = m->first(); // find the first item that needs to be rendered
  drawn_selected = -1;
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
  selected = -1;
  {
    int j = 0;
    if (m) for (const Fl_Menu_Item* m1=m; ; m1 = m1->next(), j++) {
      if (picked) {
        if (m1 == picked) {selected = j; picked = 0;}
        else if (m1 > picked) {selected = j-1; picked = 0; Wp = Hp = 0;}
    }
    if (!m1->text) break;
  }
  numitems = j;}

  if (menubar) {
    itemheight = 0;
    title = 0;
    return;
  }

  itemheight = 1;

  int hotKeysw = 0;
  int hotModsw = 0;
  int Wtitle = 0;
  int Htitle = 0;
  if (t) Wtitle = t->measure(&Htitle, button) + 12;
  int W = 0;
  if (m) for (; m->text; m = m->next()) {
    int hh;
    int w1 = m->measure(&hh, button);
    if (hh+Fl::menu_linespacing()>itemheight) itemheight = hh+Fl::menu_linespacing();
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
        if (w1 > hotModsw) hotModsw = w1;
        w1 = int(fl_width(k))+4;
        if (w1 > hotKeysw) hotKeysw = w1;
      } else {
        // a shortcut with a long modifier is right-justified to the menu
        w1 = int(fl_width(s))+4;
        if (w1 > (hotModsw+hotKeysw)) {
          hotModsw = w1-hotKeysw;
        }
      }
    }
  }
  shortcutWidth = hotKeysw;
  if (selected >= 0 && !Wp) X -= W/2;
  int BW = Fl::box_dx(box());
  W += hotKeysw+hotModsw+2*BW+7;
  if (Wp > W) W = Wp;
  if (Wtitle > W) W = Wtitle;

  if (X < scr_x) X = scr_x;
  // this change improves popup submenu positioning at right screen edge,
  // but it makes right_edge argument useless
  //if (X > scr_x+scr_w-W) X = right_edge-W;
  if (X > scr_x+scr_w-W) X = scr_x+scr_w-W;
  x(X); w(W);
  h((numitems ? itemheight*numitems-4 : 0)+2*BW+3);
  if (selected >= 0) {
    Y = Y+(Hp-itemheight)/2-selected*itemheight-BW;
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
        Y = Y-itemheight-h()-Fl::box_dh(box());
      } else {
        // draw the menu to the right
        Y = Y-h()+itemheight+Fl::box_dy(box());
      }
      if (t) {
        if (menubar_title) {
          Y = Y + Fl::menu_linespacing() - Fl::box_dw(button->box());
        } else {
          Y += 2*Htitle+2*BW+3;
        }
      }
    }
  }
  if (m) y(Y); else {y(Y-2); w(1); h(1);}

  if (t) {
    if (menubar_title) {
      int dy = Fl::box_dy(button->box())+1;
      int ht = button->h()-dy*2;
      title = new menutitle(tx, ty-ht-dy, Wtitle, ht, t, true);
    } else {
      int dy = 2;
      int ht = Htitle+2*BW+3;
      title = new menutitle(X, Y-ht-dy, Wtitle, ht, t);
    }
  } else {
    title = 0;
  }
}

menuwindow::~menuwindow() {
  hide();
  delete title;
}

void menuwindow::position(int X, int Y) {
  if (title) {title->position(X, title->y()+Y-y());}
  Fl_Menu_Window::position(X, Y);
  // x(X); y(Y); // don't wait for response from X
}

// scroll so item i is visible on screen
void menuwindow::autoscroll(int n) {
  int scr_y, scr_h;
  int Y = y()+Fl::box_dx(box())+2+n*itemheight;

  int xx, ww;
  Fl_Window_Driver::driver(this)->menu_window_area(xx, scr_y, ww, scr_h);
  if (n==0 && Y <= scr_y + itemheight) {
    Y = scr_y - Y + 10;
  } else if (Y <= scr_y + itemheight) {
    Y = scr_y - Y + 10 + itemheight;
  } else {
    Y = Y+itemheight-scr_h-scr_y;
    if (Y < 0) return;
    Y = -Y-10;
  }
  Fl_Window_Driver::driver(this)->reposition_menu_window(x(), y()+Y);
  // y(y()+Y); // don't wait for response from X
}

////////////////////////////////////////////////////////////////

void menuwindow::drawentry(const Fl_Menu_Item* m, int n, int eraseit) {
  if (!m) return; // this happens if -1 is selected item and redrawn

  int BW = Fl::box_dx(box());
  int xx = BW;
  int W = w();
  int ww = W-2*BW-1;
  int yy = BW+1+n*itemheight+Fl::menu_linespacing()/2-2;
  int hh = itemheight - Fl::menu_linespacing();

  if (eraseit && n != selected) {
    fl_push_clip(xx+1, yy-(Fl::menu_linespacing()-2)/2, ww-2, hh+(Fl::menu_linespacing()-2));
    draw_box(box(), 0, 0, w(), h(), button ? button->color() : color());
    fl_pop_clip();
  }

  m->draw(xx, yy, ww, hh, button, n==selected);

  // the shortcuts and arrows assume fl_color() was left set by draw():
  if (m->submenu()) {

    // calculate the bounding box of the submenu pointer (arrow)
    int sz = ((hh-2) & (-2)) + 1 ;  // must be odd for better centering
    if (sz > 13) sz = 13;           // limit arrow size
    int x1 = xx + ww - sz - 2;      // left border
    int y1 = yy + (hh-sz)/2 + 1;    // top border

    // draw an arrow whose style depends on the active scheme
    fl_draw_arrow(Fl_Rect(x1, y1, sz, sz), FL_ARROW_SINGLE, FL_ORIENT_RIGHT, fl_color());

  } else if (m->shortcut_) {
    Fl_Font f = m->labelsize_ || m->labelfont_ ? (Fl_Font)m->labelfont_ :
                    button ? button->textfont() : FL_HELVETICA;
    fl_font(f, m->labelsize_ ? m->labelsize_ :
                   button ? button->textsize() : FL_NORMAL_SIZE);
    const char *k, *s = fl_shortcut_label(m->shortcut_, &k);
    if (fl_utf_nb_char((const unsigned char*)k, (int) strlen(k))<=4) {
      // right-align the modifiers and left-align the key
      char *buf = (char*)malloc(k-s+1);
      memcpy(buf, s, k-s); buf[k-s] = 0;
      fl_draw(buf, xx, yy, ww-shortcutWidth, hh, FL_ALIGN_RIGHT);
      fl_draw(  k, xx+ww-shortcutWidth, yy, shortcutWidth, hh, FL_ALIGN_LEFT);
      free(buf);
    } else {
      // right-align to the menu
      fl_draw(s, xx, yy, ww-4, hh, FL_ALIGN_RIGHT);
    }
  }

  if (m->flags & FL_MENU_DIVIDER) {
    fl_color(FL_DARK3);
    fl_xyline(BW-1, yy+hh+(Fl::menu_linespacing()-2)/2, W-2*BW+2);
    fl_color(FL_LIGHT3);
    fl_xyline(BW-1, yy+hh+((Fl::menu_linespacing()-2)/2+1), W-2*BW+2);
  }
}

void menutitle::draw() {
  menu->draw(0, 0, w(), h(), button, 2);
}

void menuwindow::draw() {
  if (damage() != FL_DAMAGE_CHILD) {    // complete redraw
    if ( box() != FL_FLAT_BOX && ( Fl::is_scheme( "gtk+" ) ||
        Fl::is_scheme( "plastic") || Fl::is_scheme( "gleam" ) )) {
      // Draw a FL_FLAT_BOX to avoid on macOS the white corners of the menus
      fl_draw_box( FL_FLAT_BOX, 0, 0, w(), h(),
                  button ? button->color() : color());
    }
    fl_draw_box(box(), 0, 0, w(), h(), button ? button->color() : color());
    if (menu) {
      const Fl_Menu_Item* m; int j;
      for (m=menu->first(), j=0; m->text; j++, m = m->next()) drawentry(m, j, 0);
    }
  } else {
    if (damage() & FL_DAMAGE_CHILD && selected!=drawn_selected) { // change selection
      drawentry(menu->next(drawn_selected), drawn_selected, 1);
      drawentry(menu->next(selected), selected, 1);
    }
  }
  drawn_selected = selected;
}

void menuwindow::set_selected(int n) {
  if (n != selected) {selected = n; damage(FL_DAMAGE_CHILD);}
}

////////////////////////////////////////////////////////////////

int menuwindow::find_selected(int mx, int my) {
  if (!menu || !menu->text) return -1;
  mx -= x();
  my -= y();
  if (my < 0 || my >= h()) return -1;
  if (!itemheight) { // menubar
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
  int n = (my-Fl::box_dx(box())-1)/itemheight;
  if (n < 0 || n>=numitems) return -1;
  return n;
}

// return horizontal position for item n in a menubar:
int menuwindow::titlex(int n) {
  const Fl_Menu_Item* m;
  int xx = 3;
  for (m=menu->first(); n--; m = m->next()) xx += m->measure(0, button) + 16;
  return xx;
}

// return 1, if the given root coordinates are inside the window
int menuwindow::is_inside(int mx, int my) {
  if ( mx < x_root() || mx >= x_root() + w() ||
       my < y_root() || my >= y_root() + h()) {
    return 0;
  }
  if (itemheight == 0 && find_selected(mx, my) == -1) {
    // in the menubar but out from any menu header
    return 0;
    }
  return 1;
}

////////////////////////////////////////////////////////////////
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

// values for menustate.state:
#define INITIAL_STATE 0 // no mouse up or down since popup() called
#define PUSH_STATE 1    // mouse has been pushed on a normal item
#define DONE_STATE 2    // exit the popup, the current item was picked
#define MENU_PUSH_STATE 3 // mouse has been pushed on a menu title

struct menustate {
  const Fl_Menu_Item* current_item; // what mouse is pointing at
  int menu_number; // which menu it is in
  int item_number; // which item in that menu, -1 if none
  menuwindow* p[20]; // pointers to menus
  int nummenus;
  int menubar; // if true p[0] is a menubar
  int state;
  menuwindow* fakemenu; // kludge for buttons in menubar
  int is_inside(int mx, int my);
};
static menustate* p=0;

// return 1 if the coordinates are inside any of the menuwindows
int menustate::is_inside(int mx, int my) {
  int i;
  for (i=nummenus-1; i>=0; i--) {
    if (p[i]->is_inside(mx, my))
      return 1;
  }
  return 0;
}

static inline void setitem(const Fl_Menu_Item* i, int m, int n) {
  p->current_item = i;
  p->menu_number = m;
  p->item_number = n;
}

static void setitem(int m, int n) {
  menustate &pp = *p;
  pp.current_item = (n >= 0) ? pp.p[m]->menu->next(n) : 0;
  pp.menu_number = m;
  pp.item_number = n;
}

static int forward(int menu) { // go to next item in menu menu if possible
  // `menu` is -1 if no item is currently selected, so use the first menu
  if (menu < 0)
    menu = 0;
  menustate &pp = *p;
  menuwindow &m = *(pp.p[menu]);
  int item = (menu == pp.menu_number) ? pp.item_number : m.selected;
  bool wrapped = false;
  do {
    while (++item < m.numitems) {
      const Fl_Menu_Item* m1 = m.menu->next(item);
      if (m1->activevisible()) {setitem(m1, menu, item); return 1;}
    }
    if (wrapped) break;
    item = -1;
    wrapped = true;
  }
  while (Fl::event_key() != FL_Down);
  return 0;
}

static int backward(int menu) { // previous item in menu menu if possible
  // `menu` is -1 if no item is currently selected, so use the first menu
  if (menu < 0)
    menu = 0;
  menustate &pp = *p;
  menuwindow &m = *(pp.p[menu]);
  int item = (menu == pp.menu_number) ? pp.item_number : m.selected;
  bool wrapped = false;
  do {
    while (--item >= 0) {
      const Fl_Menu_Item* m1 = m.menu->next(item);
      if (m1->activevisible()) {setitem(m1, menu, item); return 1;}
    }
    if (wrapped) break;
    item = m.numitems;
    wrapped = true;
  }
  while (Fl::event_key() != FL_Up);
  return 0;
}

int menuwindow::handle(int e) {
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

int menuwindow::handle_part2(int e, int ret) {
  // This off-route takes care of the "detached menu" bug on OS X (STR #449).
  // Apple event handler requires that we hide all menu windows right
  // now, so that Carbon can continue undisturbed with handling window
  // manager events, like dragging the application window.
  menustate &pp = *p;
  if (pp.state == DONE_STATE) {
    hide();
    if (pp.fakemenu) {
      pp.fakemenu->hide();
      if (pp.fakemenu->title)
        pp.fakemenu->title->hide();
    }
    int i = pp.nummenus;
    while (i>0) {
      menuwindow *mw = pp.p[--i];
      if (mw) {
        mw->hide();
        if (mw->title)
          mw->title->hide();
      }
    }
  }
  return ret;
}

int menuwindow::handle_part1(int e) {
  menustate &pp = *p;
  switch (e) {
  case FL_KEYBOARD:
    switch (Fl::event_key()) {
    case FL_BackSpace:
    BACKTAB:
      backward(pp.menu_number);
      return 1;
    case FL_Up:
      if (pp.menubar && pp.menu_number == 0) {
        // Do nothing...
      } else if (backward(pp.menu_number)) {
        // Do nothing...
      } else if (pp.menubar && pp.menu_number==1) {
        setitem(0, pp.p[0]->selected);
      }
      return 1;
    case FL_Tab:
      if (Fl::event_shift()) goto BACKTAB;
      if (pp.menubar && pp.menu_number == 0) goto RIGHT;
    case FL_Down:
      if (pp.menu_number || !pp.menubar) {
        forward(pp.menu_number);
      } else if (pp.menu_number < pp.nummenus-1) {
        forward(pp.menu_number+1);
      }
      return 1;
    case FL_Right:
    RIGHT:
      if (pp.menubar && (pp.menu_number<=0 || (pp.menu_number == pp.nummenus-1)))
        forward(0);
      else if (pp.menu_number < pp.nummenus-1) forward(pp.menu_number+1);
      return 1;
    case FL_Left:
      if (pp.menubar && pp.menu_number<=1) backward(0);
      else if (pp.menu_number>0)
        setitem(pp.menu_number-1, pp.p[pp.menu_number-1]->selected);
      return 1;
    case FL_Enter:
    case FL_KP_Enter:
    case ' ':
      // if the current item is a submenu with no callback,
      // simulate FL_Right to enter the submenu
      if (   pp.current_item
          && (!pp.menubar || pp.menu_number > 0)
          && pp.current_item->activevisible()
          && pp.current_item->submenu()
          && !pp.current_item->callback_)
      {
        goto RIGHT;
      }
      // Ignore keypresses over inactive items, mark KEYBOARD event as used.
      if (pp.current_item && !pp.current_item->activevisible())
        return 1;
      // Mark the menu 'done' which will trigger the callback
      pp.state = DONE_STATE;
      return 1;
    case FL_Escape:
      setitem(0, -1, 0);
      pp.state = DONE_STATE;
      return 1;
    }
    break;
  case FL_SHORTCUT:
    {
      for (int mymenu = pp.nummenus; mymenu--;) {
        menuwindow &mw = *(pp.p[mymenu]);
        int item; const Fl_Menu_Item* m = mw.menu->find_shortcut(&item);
        if (m) {
          setitem(m, mymenu, item);
          if (!m->submenu()) pp.state = DONE_STATE;
          return 1;
        }
      }
    }
    break;
  case FL_MOVE: {
    static int use_part1_extra = Fl::screen_driver()->need_menu_handle_part1_extra();
    if (use_part1_extra && pp.state == DONE_STATE) {
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
      int item=0; int mymenu = pp.nummenus-1;
      // Clicking or dragging outside menu cancels it...
      if ((!pp.menubar || mymenu) && !pp.is_inside(mx, my)) {
        setitem(0, -1, 0);
        if (e==FL_PUSH)
          pp.state = DONE_STATE;
        return 1;
      }
      for (mymenu = pp.nummenus-1; ; mymenu--) {
        item = pp.p[mymenu]->find_selected(mx, my);
        if (item >= 0)
          break;
        if (mymenu <= 0) {
          // buttons in menubars must be deselected if we move outside of them!
          if (pp.menu_number==-1 && e==FL_PUSH) {
            pp.state = DONE_STATE;
            return 1;
          }
          if (pp.current_item && pp.menu_number==0 && !pp.current_item->submenu()) {
            if (e==FL_PUSH) {
              pp.state = DONE_STATE;
              setitem(0, -1, 0);
            }
            return 1;
          }
          // all others can stay selected
          return 0;
        }
      }
      setitem(mymenu, item);
      if (e == FL_PUSH) {
        if (pp.current_item && pp.current_item->submenu() // this is a menu title
            && item != pp.p[mymenu]->selected // and it is not already on
            && !pp.current_item->callback_) // and it does not have a callback
          pp.state = MENU_PUSH_STATE;
        else
          pp.state = PUSH_STATE;
      }
    }
    return 1;
  case FL_RELEASE:
    // Mouse must either be held down/dragged some, or this must be
    // the second click (not the one that popped up the menu):
    if (   !Fl::event_is_click()
        || pp.state == PUSH_STATE
        || (pp.menubar && pp.current_item && !pp.current_item->submenu()) // button
        ) {
#if 0 // makes the check/radio items leave the menu up
      const Fl_Menu_Item* m = pp.current_item;
      if (m && button && (m->flags & (FL_MENU_TOGGLE|FL_MENU_RADIO))) {
        ((Fl_Menu_*)button)->picked(m);
        pp.p[pp.menu_number]->redraw();
      } else
#endif
      // do nothing if they try to pick an inactive item, or a submenu with no callback
      if (!pp.current_item || (pp.current_item->activevisible() &&
         (!pp.current_item->submenu() || pp.current_item->callback_ || (pp.menubar && pp.menu_number <= 0))))
        pp.state = DONE_STATE;
    }
    return 1;
  }
  return Fl_Window::handle(e);
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
    int menubar) const {
  Fl_Group::current(0); // fix possible user error...

  // track the Fl_Menu_ widget to make sure we notice if it gets
  // deleted while the menu is open (STR #3503)
  Fl_Widget_Tracker wp((Fl_Widget *)pbutton);

  button = pbutton;
  if (pbutton && pbutton->window()) {
    menuwindow::parent_ = pbutton->top_window();
    for (Fl_Window* w = pbutton->window(); w; w = w->window()) {
      X += w->x();
      Y += w->y();
    }
  } else {
    X += Fl::event_x_root()-Fl::event_x();
    Y += Fl::event_y_root()-Fl::event_y();
    menuwindow::parent_ = Fl::first_window();
  }

  int XX, YY, WW;
  Fl::screen_xywh(XX, YY, WW, menuwindow::display_height_, menuwindow::parent_->screen_num());
  menuwindow mw(this, X, Y, W, H, initial_item, title, menubar);
  Fl::grab(mw);
  // If we grab the mouse pointer, we should also make sure that it is visible.
  if (menuwindow::parent_)
    menuwindow::parent_->cursor(FL_CURSOR_DEFAULT);
  menustate pp; p = &pp;
  pp.p[0] = &mw;
  pp.nummenus = 1;
  pp.menubar = menubar;
  pp.state = INITIAL_STATE;
  pp.fakemenu = 0; // kludge for buttons in menubar

  // preselected item, pop up submenus if necessary:
  if (initial_item && mw.selected >= 0) {
    setitem(0, mw.selected);
    goto STARTUP;
  }

  pp.current_item = 0; pp.menu_number = 0; pp.item_number = -1;
  if (menubar) {
    // find the initial menu
    if (!mw.handle(FL_DRAG)) {
      Fl::grab(0);
      return 0;
    }
  }
  initial_item = pp.current_item;
  if (initial_item) {
    if (menubar && !initial_item->activevisible()) { // pointing at inactive item
      Fl::grab(0);
      return NULL;
    }
    goto STARTUP;
  }

  // the main loop: runs until p.state goes to DONE_STATE or the menu
  // widget is deleted (e.g. from a timer callback, see STR #3503):
  for (;;) {

    // make sure all the menus are shown:
    {
      for (int k = menubar; k < pp.nummenus; k++) {
        if (!pp.p[k]->shown()) {
          if (pp.p[k]->title) pp.p[k]->title->show();
          pp.p[k]->show();
        }
      }
    }

    // get events:
    {
      const Fl_Menu_Item* oldi = pp.current_item;
      Fl::wait();
      if (pbutton && wp.deleted()) // menu widget has been deleted (STR #3503)
        break;
      if (pp.state == DONE_STATE) break; // done.
      if (pp.current_item == oldi) continue;
    }

    // only do rest if item changes:
    if(pp.fakemenu) {delete pp.fakemenu; pp.fakemenu = 0;} // turn off "menubar button"

    if (!pp.current_item) { // pointing at nothing
      // turn off selection in deepest menu, but don't erase other menus:
      pp.p[pp.nummenus-1]->set_selected(-1);
      continue;
    }

    if(pp.fakemenu) {delete pp.fakemenu; pp.fakemenu = 0;}
    initial_item = 0; // stop the startup code
    pp.p[pp.menu_number]->autoscroll(pp.item_number);

  STARTUP:
    menuwindow& cw = *pp.p[pp.menu_number];
    const Fl_Menu_Item* m = pp.current_item;
    if (!m->activevisible()) { // pointing at inactive item
      cw.set_selected(-1);
      initial_item = 0; // turn off startup code
      continue;
    }
    cw.set_selected(pp.item_number);

    if (m==initial_item) initial_item=0; // stop the startup code if item found
    if (m->submenu()) {
      const Fl_Menu_Item* title = m;
      const Fl_Menu_Item* menutable;
      if (m->flags&FL_SUBMENU) menutable = m+1;
      else menutable = (Fl_Menu_Item*)(m)->user_data_;
      // figure out where new menu goes:
      int nX, nY;
      if (!pp.menu_number && pp.menubar) {      // menu off a menubar:
        nX = cw.x() + cw.titlex(pp.item_number);
        nY = cw.y() + cw.h();
        initial_item = 0;
      } else {
        nX = cw.x() + cw.w();
        nY = cw.y() + pp.item_number * cw.itemheight;
        title = 0;
      }
      if (initial_item) { // bring up submenu containing initial item:
        menuwindow* n = new menuwindow(menutable,X,Y,W,H,initial_item,title,0,0,cw.x());
        pp.p[pp.nummenus++] = n;
        if (pp.nummenus >= 2) pp.p[pp.nummenus-1]->origin = pp.p[pp.nummenus-2];
        // move all earlier menus to line up with this new one:
        if (n->selected>=0) {
          int dy = n->y()-nY;
          int dx = n->x()-nX;
          int waX, waY, waW, waH;
          Fl_Window_Driver::driver(n)->menu_window_area(waX, waY, waW, waH, Fl::screen_num(X, Y));
          for (int menu = 0; menu <= pp.menu_number; menu++) {
            menuwindow* tt = pp.p[menu];
            int nx = tt->x()+dx; if (nx < waX) {nx = waX; dx = -tt->x() + waX;}
            int ny = tt->y()+dy; if (ny < waY) {ny = waY; dy = -tt->y() + waY;}
            tt->position(nx, ny);
          }
          setitem(pp.nummenus-1, n->selected);
          goto STARTUP;
        }
      } else if (pp.nummenus > pp.menu_number+1 &&
                 pp.p[pp.menu_number+1]->menu == menutable) {
        // the menu is already up:
        while (pp.nummenus > pp.menu_number+2) delete pp.p[--pp.nummenus];
        pp.p[pp.nummenus-1]->set_selected(-1);
      } else {
        // delete all the old menus and create new one:
        while (pp.nummenus > pp.menu_number+1) delete pp.p[--pp.nummenus];
        pp.p[pp.nummenus++]= new menuwindow(menutable, nX, nY,
                                          title?1:0, 0, 0, title, 0, menubar,
                                            (title ? 0 : cw.x()) );
        if (pp.nummenus >= 2 && pp.p[pp.nummenus-2]->itemheight) {
          pp.p[pp.nummenus-1]->origin = pp.p[pp.nummenus-2];
        }
      }
    } else { // !m->submenu():
      while (pp.nummenus > pp.menu_number+1) delete pp.p[--pp.nummenus];
      if (!pp.menu_number && pp.menubar) {
        // kludge so "menubar buttons" turn "on" by using menu title:
        pp.fakemenu = new menuwindow(0,
                                  cw.x()+cw.titlex(pp.item_number),
                                  cw.y()+cw.h(), 0, 0,
                                  0, m, 0, 1);
        pp.fakemenu->title->show();
      }
    }
  }
  const Fl_Menu_Item* m = (pbutton && wp.deleted()) ? NULL : pp.current_item;
  delete pp.fakemenu;
  while (pp.nummenus>1) delete pp.p[--pp.nummenus];
  mw.hide();
  Fl::grab(0);
  menuwindow::parent_ = NULL;
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

static bool is_special_labeltype(uchar t) {
  return t == _FL_MULTI_LABEL || t == _FL_ICON_LABEL || t == _FL_IMAGE_LABEL;
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
