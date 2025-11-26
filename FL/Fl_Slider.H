//
// Slider header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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
   Fl_Slider widget . */

#ifndef Fl_Slider_H
#define Fl_Slider_H

#ifndef Fl_Valuator_H
#include "Fl_Valuator.H"
#endif

#include <FL/Fl_Rect.H>

#define FL_VERT_SLIDER          0
#define FL_HOR_SLIDER           1
#define FL_VERT_FILL_SLIDER     2
#define FL_HOR_FILL_SLIDER      3
#define FL_VERT_NICE_SLIDER     4
#define FL_HOR_NICE_SLIDER      5

/**
  The Fl_Slider widget contains a sliding knob inside a box. It is
  often used as a scrollbar.  Moving the box all the way to the
  top/left sets it to the minimum(), and to the bottom/right to the
  maximum().  The minimum() may be greater than the maximum() to
  reverse the slider direction.

  Use void Fl_Widget::type(int) to set how the slider is drawn,
  which can be one of the following:

  \li FL_VERTICAL - Draws a vertical slider (this is the default).
  \li FL_HORIZONTAL - Draws a horizontal slider.
  \li FL_VERT_FILL_SLIDER - Draws a filled vertical slider,
      useful as a progress or value meter.
  \li FL_HOR_FILL_SLIDER - Draws a filled horizontal  slider,
      useful as a progress or value meter.
  \li FL_VERT_NICE_SLIDER - Draws a vertical slider with  a nice
      looking control knob.
  \li FL_HOR_NICE_SLIDER - Draws a horizontal slider with  a
      nice looking control knob.

  \image html  slider.png
  \image latex slider.png "Fl_Slider" width=4cm
*/
class FL_EXPORT Fl_Slider : public Fl_Valuator {

public:

  /// Bitset for tick mark positions for Fl_Nice_Slider.
  /// \see Fl_Slider::ticks(), Fl_Slider::ticks(Tick_Position)
  typedef enum : uchar {
    TICKS_NONE = 0x00,    ///< draw no ticks
    TICKS_BELOW = 0x01,   ///< draw ticks below a horizontal slider
    TICKS_ABOVE = 0x02,   ///< draw ticks above a horizontal slider
    TICKS_LEFT = 0x01,    ///< draw ticks to the left of a vertical slider
    TICKS_RIGHT = 0x02    ///< draw ticks to the right of a vertical slider
    // Later: LABEL : label the first, middle, and last tick with its value
    // Later: FRACTIONAL : make a half ticks a bit shorter, and quarter ticks even shorter
    // Later: DECIMAL: make .5 ticks shorter and 0.1 ticks even shorter
    // Later: POINTY_KNOB : make the slider knob pointy on the side where the ticks are
    // Later: SMALLE_KNOB : make the slider knob small, so the ticks are better visible
  } Tick_Position;

  /// Bitset for tick mark positions for Fl_Nice_Slider.
  /// \see Fl_Slider::ticks(), Fl_Slider::ticks(Tick_Position)
  typedef enum : uchar {
    LINEAR_SCALE = 0,     ///< Linear scale (default)
    LOG_SCALE             ///< Logarithmic scale
  } Scale_Type;

private:

  float slider_size_;
  uchar slider_;
  Scale_Type scale_type_ { LINEAR_SCALE };
  Tick_Position ticks_ { TICKS_NONE };
  uchar num_ticks_ { 11 };
  void _Fl_Slider();
  void draw_bg(int, int, int, int, int S=16);

protected:

  // these allow subclasses to put the slider in a smaller area:
  void draw(int, int, int, int);
  int handle(int, int, int, int, int);
  void draw() override;
  void draw_ticks(const Fl_Rect& r, int S);
  double value_to_position(double val) const;
  double position_to_value(double pos) const;

public:

  int handle(int) override;
  Fl_Slider(int X,int Y,int W,int H, const char *L = 0);
  Fl_Slider(uchar t,int X,int Y,int W,int H, const char *L);

  int scrollvalue(int pos,int size,int first,int total);
  void bounds(double a, double b);

  /**
    Get the dimensions of the moving piece of slider.
  */
  float slider_size() const {return slider_size_;}

  /**
    Set the dimensions of the moving piece of slider. This is
    the fraction of the size of the entire widget. If you set this
    to 1 then the slider cannot move.  The default value is .08.

    For the "fill" sliders this is the size of the area around the
    end that causes a drag effect rather than causing the slider to
    jump to the mouse.
  */
  void slider_size(double v);

  /** Gets the slider box type. */
  Fl_Boxtype slider() const {return (Fl_Boxtype)slider_;}

  /** Sets the slider box type. */
  void slider(Fl_Boxtype c) {slider_ = (uchar)c;}

  /** Gets the scale type.
    \return scale type
   */
  Scale_Type scale() const { return scale_type_; }

  /** Sets the scale type.
    \param[in] s scale type
   */
  void scale(Scale_Type s) { scale_type_ = s; }

  /** Gets the tick mark position.
    \return tick mark position bitset
  */
  uchar ticks() const { return ticks_; }

  /** Gets the number of tick marks.
    \return number of tick marks
  */
  uchar num_ticks() const { return num_ticks_; }

  /** Sets the tick mark position for Fl_Nice_Slider.
   FL_TICKS_BELOW and FL_TICKS_ABOVE can be or'd together for horizontal sliders,
   and FL_TICKS_LEFT and FL_TICKS_RIGHT for vertical sliders.
   \param[in] t tick mark position bitset
   \param[in] num_ticks number of tick marks to draw
  */
  void ticks(uchar t, uchar num_ticks=11) { ticks_ = (Tick_Position)t; num_ticks_ = num_ticks; }
};

#endif
