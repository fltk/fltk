//
// An input/chooser widget.
//            ______________  ____
//           |              || __ |
//           | input area   || \/ |
//           |______________||____|
//
// Copyright 1998-2024 by Bill Spitzak and others.
// Copyright 2004 by Greg Ercolano.
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

/* \file
   Fl_Input_Choice widget . */

#include <FL/Fl_Input_Choice.H>

#include <FL/fl_draw.H>
#include <string.h>

/**
  \class Fl_Input_Choice

  \brief A combination of the input widget and a menu button.

  \image html input_choice.png
  \image latex input_choice.png "Fl_Input_Choice widget" width=12cm

  The user can either type into the input area, or use the
  menu button chooser on the right to choose an item which loads
  the input area with the selected text.

  The application can directly access both the internal Fl_Input
  and Fl_Menu_Button widgets respectively using the input() and menubutton()
  accessor methods.

  The default behavior is to invoke the Fl_Input_Choice::callback()
  if the user changes the input field's contents, either by typing,
  pasting, or clicking a different item in the choice menu.

  The callback can determine if an item was picked vs. typing
  into the input field by checking the value of menubutton()->changed(),
  which will be:

      - 1: the user picked a different item in the choice menu
      - 0: the user typed or pasted directly into the input field

  \par Example Use of Fl_Input_Choice
  \code
  #include <stdio.h>
  #include <FL/Fl.H>
  #include <FL/Fl_Double_Window.H>
  #include <FL/Fl_Input_Choice.H>
  // Fl_Input_Choice callback()
  void choice_cb(Fl_Widget *w, void *userdata) {
    // Show info about the picked item
    Fl_Input_Choice *choice = (Fl_Input_Choice*)w;
    printf("*** Choice Callback:\n");
    printf("    widget's text value='%s'\n", choice->value());   // normally all you need
    // Access the menu via menubutton()..
    const Fl_Menu_Item *item = choice->menubutton()->mvalue();
    printf("    item label()='%s'\n", item ? item->label() : "(No item)");
    printf("    item value()=%d\n", choice->menubutton()->value());
    printf("    input value()='%s'\n", choice->input()->value());
    printf("    The user %s\n", choice->menubutton()->changed()
                                    ? "picked a menu item"
                                    : "typed text");
  }
  int main() {
    Fl_Double_Window win(200,100,"Input Choice");
    win.begin();
      Fl_Input_Choice choice(10,10,100,30);
      choice.callback(choice_cb, 0);
      choice.add("Red");
      choice.add("Orange");
      choice.add("Yellow");
      //choice.value("Red");    // uncomment to make "Red" default
    win.end();
    win.show();
    return Fl::run();
  }
  \endcode

 \par Subclassing Example
 One can subclass Fl_Input_Choice to override the virtual methods inp_x/y/w/h()
 and menu_x/y/w/h() to take control of the internal Fl_Input and Fl_Menu_Button widget
 positioning. In this example, input and menubutton's positions are swapped:
 \code
  #include <FL/Fl.H>
  #include <FL/Fl_Window.H>
  #include <FL/Fl_Input_Choice.H>

  class MyInputChoice : public Fl_Input_Choice {
  protected:
    virtual int inp_x()  const { return x() + Fl::box_dx(box()) + menu_w(); }  // override to reposition
    virtual int menu_x() const { return x() + Fl::box_dx(box()); }             // override to reposition
  public:
    MyInputChoice(int X,int Y,int W,int H,const char*L=0) : Fl_Input_Choice(X,Y,W,H,L) {
      resize(X,Y,W,H);  // necessary for ctor to trigger our overrides
    }
  };

  int main(int argc, char **argv) {
    Fl_Window *win = new Fl_Window(400,300);
    MyInputChoice *mychoice = new MyInputChoice(150,40,150,25,"Right Align Input");
    mychoice->add("Aaa");
    mychoice->add("Bbb");
    mychoice->add("Ccc");
    win->end();
    win->resizable(win);
    win->show();
    return Fl::run();
  }
 \endcode

*/

/** Constructor for private menu button. */
Fl_Input_Choice::InputMenuButton::InputMenuButton(int x,int y,int w,int h,const char*l)
                                 :Fl_Menu_Button(x,y,w,h,l)
{
  box(FL_UP_BOX);
}

/** Draws the private menu button. */
void Fl_Input_Choice::InputMenuButton::draw() {
  if (!box()) return;

  // Draw box for default scheme only
  //    For all other schemes, let parent group's box show through
  //
  if (!Fl::scheme())
    draw_box(pressed_menu_button_ == this ? fl_down(box()) : box(), color());
  if (Fl::focus() == this) {
    int woff = Fl::scheme() ? 2 : 1;   // helps center focus box
    draw_focus(FL_UP_BOX, x(), y(), w()+woff, h(), color());
  }

  // draw the arrow (choice button)
  Fl_Color arrow_color = active_r() ? labelcolor() : fl_inactive(labelcolor());
  fl_draw_arrow(Fl_Rect(x(), y(), w(), h()), FL_ARROW_CHOICE, FL_ORIENT_NONE, arrow_color);
}

// Make pulldown menu appear under entire width of widget
const Fl_Menu_Item* Fl_Input_Choice::InputMenuButton::popup() {
  menu_end();
  redraw();
  Fl_Widget_Tracker mb(this);
  // Make menu appear under entire width of Fl_Input_Choice parent group
  const Fl_Menu_Item *m = menu()->pulldown(parent()->x(), y(), parent()->w(), h(), 0, this);
  picked(m);
  if (mb.exists()) redraw();
  return m;
}

// Invokes our popup() method to ensure pulldown menu appears full width under widget
//    (This is the same handle() code in Fl_Menu_Button and Fl_Choice)
//
int Fl_Input_Choice::InputMenuButton::handle(int e) {
  if (!menu() || !menu()->text) return 0;
  switch (e) {
  case FL_ENTER: /* FALLTHROUGH */
  case FL_LEAVE:
    return (box() && !type()) ? 1 : 0;
  case FL_PUSH:
    if (!box()) {
      if (Fl::event_button() != 3) return 0;
    } else if (type()) {
      if (!(type() & (1 << (Fl::event_button()-1)))) return 0;
    }
    if (Fl::visible_focus()) Fl::focus(this);
    popup();
    return 1;
  case FL_KEYBOARD:
    if (!box()) return 0;
    if (Fl::event_key() == ' ' &&
        !(Fl::event_state() & (FL_SHIFT | FL_CTRL | FL_ALT | FL_META))) {
      popup();
      return 1;
    } else return 0;
  case FL_SHORTCUT:
    if (Fl_Widget::test_shortcut()) {popup(); return 1;}
    return test_shortcut() != 0;
  case FL_FOCUS: /* FALLTHROUGH */
  case FL_UNFOCUS:
    if (box() && Fl::visible_focus()) {
      redraw();
      return 1;
    }
  default:
    return 0;
  }
}

/** Callback for the Fl_Input_Choice menu. */

void Fl_Input_Choice::menu_cb(Fl_Widget*, void *data) {
  Fl_Input_Choice *o=(Fl_Input_Choice *)data;
  Fl_Widget_Tracker wp(o);
  const Fl_Menu_Item *item = o->menubutton()->mvalue();
  if (item && item->flags & (FL_SUBMENU|FL_SUBMENU_POINTER)) return;    // ignore submenus
  if (!strcmp(o->inp_->value(), o->menu_->text()))
  {
    o->Fl_Widget::clear_changed();
    if (o->when() & FL_WHEN_NOT_CHANGED)
      o->do_callback(FL_REASON_RESELECTED);
  }
  else
  {
    o->inp_->value(o->menu_->text());
    o->inp_->set_changed();
    o->Fl_Widget::set_changed();
    if (o->when() & (FL_WHEN_CHANGED|FL_WHEN_RELEASE))
      o->do_callback(FL_REASON_CHANGED);
  }

  if (wp.deleted()) return;

  if (o->callback() != default_callback)
  {
    o->Fl_Widget::clear_changed();
    o->inp_->clear_changed();
  }
}

/** Callback for the Fl_Input_Choice input field. */

void Fl_Input_Choice::inp_cb(Fl_Widget*, void *data) {
  Fl_Input_Choice *o=(Fl_Input_Choice *)data;
  Fl_Widget_Tracker wp(o);
  if (o->inp_->changed()) {
    o->Fl_Widget::set_changed();
    if (o->when() & (FL_WHEN_CHANGED|FL_WHEN_RELEASE))
      o->do_callback(FL_REASON_CHANGED);
  } else {
    o->Fl_Widget::clear_changed();
    if (o->when() & FL_WHEN_NOT_CHANGED)
      o->do_callback(FL_REASON_RESELECTED);
  }
  if (wp.deleted()) return;

  if (o->callback() != default_callback)
    o->Fl_Widget::clear_changed();
}

/**
  Creates a new Fl_Input_Choice widget using the given position, size,
  and label string.

  Inherited destructor destroys the widget and any values associated with it.
*/
Fl_Input_Choice::Fl_Input_Choice(int X, int Y, int W, int H, const char *L)
: Fl_Group(X,Y,W,H,L) {
  Fl_Group::box(FL_DOWN_BOX);
  align(FL_ALIGN_LEFT);                                 // default like Fl_Input
  inp_ = new Fl_Input(inp_x(), inp_y(), inp_w(), inp_h());
  inp_->callback(inp_cb, (void*)this);
  inp_->box(FL_FLAT_BOX);                               // cosmetic
  inp_->when(FL_WHEN_CHANGED|FL_WHEN_NOT_CHANGED);
  menu_ = new InputMenuButton(menu_x(), menu_y(), menu_w(), menu_h());
  menu_->callback(menu_cb, (void*)this);
  end();
}

/** Resizes the Fl_Input_Choice widget.
*/
void Fl_Input_Choice::resize(int X, int Y, int W, int H) {
  Fl_Group::resize(X,Y,W,H);
  inp_->resize(inp_x(), inp_y(), inp_w(), inp_h());
  menu_->resize(menu_x(), menu_y(), menu_w(), menu_h());
}

/** Chooses item# \p val in the menu, and sets the Fl_Input text field
    to that value. Any previous text is cleared.
    \see void value(const char *val)
*/
void Fl_Input_Choice::value(int val) {
  menu_->value(val);
  inp_->value(menu_->text(val));
}

/** Sets the changed() state of both input and menu button widgets
    to the specified value.
*/
void Fl_Input_Choice::set_changed() {
  inp_->set_changed();
  // no need to call Fl_Widget::set_changed()
}

/** Clears the changed() state of both input and menu button widgets. */
void Fl_Input_Choice::clear_changed() {
  inp_->clear_changed();
  Fl_Widget::clear_changed();
}

/** Updates the menubutton with the string value in Fl_Input.

    If the string value currently in Fl_Input matches one of the
    menu items in menubutton(), that menu item will become the
    current item selected.

    Call this method after setting value(const char*) if you need
    the menubutton() to be synchronized with the Fl_Input field.

    \code
    // Add items
    choice->add(".25");
    choice->add(".50");
    choice->add("1.0");
    choice->add("2.0");
    choice->add("4.0");

    choice->value("1.0");            // sets Fl_Input to "1.0"
    choice->update_menubutton();     // cause menubutton to reflect this value too
                                     // (returns 1 if match was found, 0 if not)
    // Verify menubutton()'s value.
    printf("menu button choice index=%d, value=%s\n",
                                choice->menubutton()->value(),    // would be -1 if update not done
                                choice->menubutton()->text());    // would be NULL if update not done
    \endcode

    \returns 1 if a matching menuitem was found and value set, 0 if not.
    \version 1.4.0
*/
int Fl_Input_Choice::update_menubutton() {
  // Find item in menu
  for ( int i=0; i<menu_->size(); i++ ) {
    const Fl_Menu_Item &item = menu_->menu()[i];
    if (item.flags & (FL_SUBMENU|FL_SUBMENU_POINTER)) continue;   // ignore submenus
    const char *name = menu_->text(i);
    if ( name && strcmp(name, inp_->value()) == 0) {
      menu_->value(i);
      return 1;
    }
  }
  return 0;             // not found
}

void Fl_Input_Choice::draw() {
  // This is copied from Fl_Choice::draw() and customized
  Fl_Boxtype btype = Fl::scheme() ? FL_UP_BOX           // non-default uses up box
                                  : FL_DOWN_BOX;        // default scheme uses down box
  int dx = Fl::box_dx(btype);
  int dy = Fl::box_dy(btype);

  // From "original" code: modify the box color *only* for the default scheme.
  // This is weird (why?). I believe we should either make sure that the text
  // color contrasts well when the text is rendered *or* we should do this for
  // *all* schemes. Anyway, adapting the old code... (Albrecht)
  //
  Fl_Color box_color = color();
  if (!Fl::scheme()) {            // default scheme only, see comment above
    if (fl_contrast(textcolor(), FL_BACKGROUND2_COLOR) == textcolor())
      box_color = FL_BACKGROUND2_COLOR;
    else
      box_color = fl_lighter(color());
  }

  // Draw the widget box
  draw_box(btype, box_color);

  // Draw menu button
  draw_child(*menu_);

  // Draw vertical divider lines for: gtk+, gleam, oxy
  //
  // Scheme:            Box or divider line
  // ----------------------------------------
  // Default (None):    Arrow box (FL_UP_BOX)
  // gtk+, gleam, oxy:  Divider line
  // else:              Nothing - Fl_Group::box() shows through
  //
  int woff = 0;
  if (Fl::is_scheme("gtk+") ||
      Fl::is_scheme("gleam") ||
      Fl::is_scheme("oxy")) {
    // draw the vertical divider line
    int x1 = menu_x() - dx;
    int y1 = y() + dy;
    int y2 = y() + h() - dy;

    fl_color(fl_darker(color()));
    fl_yxline(x1+0, y1, y2);

    fl_color(fl_lighter(color()));
    fl_yxline(x1+1, y1, y2);
    woff = 2;           // prevent Fl_Input from overdrawing divider
  }

  // Draw the input field
  fl_push_clip(inp_x(), inp_y(), inp_w() - woff, inp_h());
    draw_child(*inp_);
  fl_pop_clip();

  // Widget's label
  draw_label();
}
