//
// "$Id$"
//
// ICO image header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2020 by Bill Spitzak and others.
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

#ifndef Fl_ICO_Image_H
#define Fl_ICO_Image_H
#  include "Fl_BMP_Image.H"

class FL_EXPORT Fl_ICO_Image : public Fl_BMP_Image {

public:

  Fl_ICO_Image(const char *filename);
  Fl_ICO_Image(const char *imagename, const unsigned char *data);

  protected:

    void load_ico_(class Fl_Image_Reader &rdr);

};

#endif
