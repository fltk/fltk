//
// "$Id$"
//
// Unit tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

// Note: currently (March 2010) fl_draw_image() supports transparency with
//	 alpha channel only on Apple (Mac OS X), but Fl_RGB_Image->draw()
//	 supports transparency on all platforms !

//
//------- test the image drawing capabilities of this implementation ----------
//
class ImageTest : public Fl_Box {
public: 
  static Fl_Widget *create() {
    int x, y;
    uchar *dg, *dga, *drgb, *drgba;
    dg = img_gray = (uchar*)malloc(128*128*1);
    dga = img_gray_a = (uchar*)malloc(128*128*2);
    drgb = img_rgb = (uchar*)malloc(128*128*3);
    drgba = img_rgba = (uchar*)malloc(128*128*4);
    for (y=0; y<128; y++) {
      for (x=0; x<128; x++) {
        *drgba++ = *drgb++ = *dga++ = *dg++ = y<<1;
        *drgba++ = *drgb++                  = x<<1;
        *drgba++ = *drgb++                  = (127-x)<<1;
        *drgba++           = *dga++         = x+y;
      }
    }
    i_rgba = new Fl_RGB_Image (img_rgba,128,128,4);
    i_ga = new Fl_RGB_Image (img_gray_a,128,128,2);
    return new ImageTest(TESTAREA_X, TESTAREA_Y, TESTAREA_W, TESTAREA_H);
  }
  static uchar *img_gray;
  static uchar *img_gray_a;
  static uchar *img_rgb;
  static uchar *img_rgba;
  static Fl_RGB_Image *i_rgba;
  static Fl_RGB_Image *i_ga;
  ImageTest(int x, int y, int w, int h) : Fl_Box(x, y, w, h) {
    label("Testing Image Drawing\n\n"
	"This test renders four images, two of them with a checker board\n"
	"visible through the graphics. Color and gray gradients should be\n"
	"visible. This does not test any image formats such as JPEG.");
    align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);
  }
  void draw() {
    Fl_Box::draw();

    // top left: RGB

    int xx = x()+10, yy = y()+10;
    fl_color(FL_BLACK); fl_rect(xx, yy, 130, 130);
    fl_draw_image(img_rgb, xx+1, yy+1, 128, 128, 3);
    fl_draw("RGB", xx+134, yy+64);

    // bottom left: RGBA

    xx = x()+10; yy = y()+10+134;
    fl_color(FL_BLACK); fl_rectf(xx, yy, 130, 130);
    fl_color(FL_WHITE); fl_rectf(xx+1, yy+1, 64, 64);
    fl_color(FL_WHITE); fl_rectf(xx+65, yy+65, 64, 64);
#ifdef __APPLE__
    fl_draw_image(img_rgba, xx+1, yy+1, 128, 128, 4);	// Apple: okay w/alpha
#else
    i_rgba->draw(xx+1,yy+1);	// only Fl_RGB_Image->draw() works with alpha
#endif
    fl_color(FL_BLACK); fl_draw("RGBA", xx+134, yy+64);
    
    // top right: Gray

    xx = x()+10+200; yy = y()+10;
    fl_color(FL_BLACK); fl_rect(xx, yy, 130, 130);
    fl_draw_image(img_gray, xx+1, yy+1, 128, 128, 1);
    fl_draw("Gray", xx+134, yy+64);

    // bottom right: Gray+Alpha

    xx = x()+10+200; yy = y()+10+134;
    fl_color(FL_BLACK); fl_rectf(xx, yy, 130, 130);
    fl_color(FL_WHITE); fl_rectf(xx+1, yy+1, 64, 64);
    fl_color(FL_WHITE); fl_rectf(xx+65, yy+65, 64, 64);
#ifdef __APPLE__
    fl_draw_image(img_gray_a, xx+1, yy+1, 128, 128, 2);	// Apple: okay w/alpha
#else
    i_ga->draw(xx+1,yy+1);	// only Fl_RGB_Image->draw() works with alpha
#endif
    fl_color(FL_BLACK); fl_draw("Gray+Alpha", xx+134, yy+64);
  }
};

uchar *ImageTest::img_gray = 0;
uchar *ImageTest::img_gray_a = 0;
uchar *ImageTest::img_rgb = 0;
uchar *ImageTest::img_rgba = 0;
Fl_RGB_Image *ImageTest::i_rgba = 0;
Fl_RGB_Image *ImageTest::i_ga = 0;

UnitTest images("drawing images", ImageTest::create);

//
// End of "$Id$"
//
