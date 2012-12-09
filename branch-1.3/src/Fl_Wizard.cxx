//
// "$Id$"
//
// Fl_Wizard widget routines.
//
// Copyright 1997-2010 by Easy Software Products.
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
// Contents:

//
//   Fl_Wizard::Fl_Wizard() - Create an Fl_Wizard widget.
//   Fl_Wizard::draw()      - Draw the wizard border and visible child.
//   Fl_Wizard::next()      - Show the next child.
//   Fl_Wizard::prev()      - Show the previous child.
//   Fl_Wizard::value()     - Return the current visible child.
//   Fl_Wizard::value()     - Set the visible child.
//

//
// Include necessary header files...
//

#include <FL/Fl_Wizard.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>


//
// 'Fl_Wizard::Fl_Wizard()' - Create an Fl_Wizard widget.
//

/**
  The constructor creates the Fl_Wizard widget at the specified
  position and size.
  <P>The inherited destructor destroys the widget and its children.
*/
Fl_Wizard::Fl_Wizard(int        xx,	// I - Lefthand position
                     int        yy,	// I - Upper position
		     int        ww,	// I - Width
		     int        hh,	// I - Height
		     const char *l) :	// I - Label
    Fl_Group(xx, yy, ww, hh, l)
{
  box(FL_THIN_UP_BOX);

  value_ = (Fl_Widget *)0;
}


//
/** Draws the wizard border and visible child. */
void Fl_Wizard::draw() {
  Fl_Widget	*kid;	// Visible child


  kid = value();

  if (damage() & FL_DAMAGE_ALL)
  {
    // Redraw everything...
    if (kid)
    {
      draw_box(box(), x(), y(), w(), h(), kid->color());
      draw_child(*kid);
    }
    else
      draw_box(box(), x(), y(), w(), h(), color());

  }
  else if (kid)
    update_child(*kid);
}


/**
  This method shows the next child of the wizard. If the last child
  is already visible, this function does nothing.
*/
void Fl_Wizard::next() {
  int			num_kids;
  Fl_Widget	* const *kids;


  if ((num_kids = children()) == 0)
    return;

  for (kids = array(); num_kids > 0; kids ++, num_kids --)
    if ((*kids)->visible())
      break;

  if (num_kids > 1)
    value(kids[1]);
}

/** Shows the previous child.*/
void Fl_Wizard::prev()
{
  int			num_kids;
  Fl_Widget	* const *kids;


  if ((num_kids = children()) == 0)
    return;

  for (kids = array(); num_kids > 0; kids ++, num_kids --)
    if ((*kids)->visible())
      break;

  if (num_kids > 0 && num_kids < children())
    value(kids[-1]);
}

/**  Gets the current visible child widget. */
Fl_Widget* Fl_Wizard::value()
{
  int			num_kids;
  Fl_Widget	* const *kids;
  Fl_Widget		*kid;


  if ((num_kids = children()) == 0)
    return ((Fl_Widget *)0);

  for (kids = array(), kid = (Fl_Widget *)0; num_kids > 0; kids ++, num_kids --)
  {
    if ((*kids)->visible())
    {
      if (kid)
        (*kids)->hide();
      else
        kid = *kids;
    }
  }

  if (!kid)
  {
    kids --;
    kid = *kids;
    kid->show();
  }

  return (kid);
}

/**  Sets the child widget that is visible.*/
void Fl_Wizard::value(Fl_Widget *kid)
{
  int			num_kids;
  Fl_Widget	* const *kids;


  if ((num_kids = children()) == 0)
    return;

  for (kids = array(); num_kids > 0; kids ++, num_kids --)
  {
    if (*kids == kid)
    {
      if (!kid->visible())
        kid->show();
    }
    else
      (*kids)->hide();
  }

  // This will restore the mouse pointer to the window's default cursor
  // whenever the wizard pane is changed.  Otherwise text widgets that
  // show the next pane may leave the cursor set to the I beam, etc...
  if (window()) window()->cursor(FL_CURSOR_DEFAULT);
}



//
// End of "$Id$".
//
