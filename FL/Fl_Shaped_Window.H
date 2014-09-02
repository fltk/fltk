//
// "$Id$"
//
// Fl_Shaped_Window header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2014 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#ifndef Fl_Shaped_Window_H
#define Fl_Shaped_Window_H

#include <FL/Fl_Window.H>
#if defined(__APPLE__)
#include <FL/x.H>
#endif


class Fl_Bitmap;
class Fl_Pixmap;

/**
  The Fl_Shaped_Window is an Fl_Window that can take an arbitrary shape (not just a rectangular region).
  The window shape is determined by the argument of the Fl_Shaped_Window::shape(const Fl_Image*)
  member function.
  An Fl_Image of any dimension can be used as mask; it is rescaled to the window's dimension as needed.

  The layout and widgets inside are unaware of the mask shape, and most will act as though the window's
  rectangular bounding box is available 
  to them. It is up to you to make sure they adhere to the bounds of their masking shape.
 
  On the Mac platform, OS version 10.4 or above is required.
  On the unix/linux platform, the SHAPE extension of the X server is required.

  The window borders and caption created by the window system are turned off by default.  They 
  can be re-enabled by calling void Fl_Window::border(1).
 
  A usage example is found at example/shapedwindow.cxx.
 */
class FL_EXPORT Fl_Shaped_Window : public Fl_Window {
private:
  int lw_, lh_;
  Fl_Image* shape_;
#if defined(__APPLE__)
  CGImageRef mask;
#endif
  Fl_Bitmap *todelete_;
  void shape_bitmap_(Fl_Bitmap*);
  void shape_pixmap_(Fl_Pixmap*);
  void shape_alpha_(Fl_RGB_Image* img, int offset);
protected:
  virtual void draw();
public:
    Fl_Shaped_Window(int w, int h, const char* l = 0);
    Fl_Shaped_Window(int x, int y, int w, int h, const char* l = 0);
    ~Fl_Shaped_Window();
    void shape(const Fl_Image*);
    /** Set the window's shape with an Fl_Image */
    inline void shape(const Fl_Image& b) { shape(&b); }
#if ! (defined(WIN32) || defined(__APPLE__) || defined(FL_DOXYGEN))
  void combine_mask(void);
#endif
};

#endif // Fl_Shaped_Window_H

//
// End of "$Id$".
//
