//
// "$Id$"
//
// Unit tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/fl_draw.H>

// Note: currently (March 2010) fl_draw_image() supports transparency with
//	 alpha channel only on Apple (Mac OS X), but Fl_RGB_Image->draw()
//	 supports transparency on all platforms !

//
//------- test the image drawing capabilities of this implementation ----------
//

// Parameters for fine tuning for developers.
// Default values: CB=1, DX=0, IMG=1, LX=0, FLIPH=0

static int CB = 1;  // 1 to show the checker board background for alpha images,
              		  // 0 otherwise
static int DX = 0;  // additional (undefined (0)) pixels per line, must be >= 0
                    // ignored (irrelevant), if LX == 0 (see below)
static int IMG = 1; // 1 to use Fl_RGB_Image for drawing images with transparency,
                    // 0 to use fl_draw_image() instead.
                    // Note: as of Feb 2016, only 1 (Fl_RGB_Image) works with
                    // alpha channel, 0 (fl_draw_image()) ignores the alpha
                    // channel (FLTK 1.3.x).
                    // There are plans to support transparency (alpha channel)
                    // in fl_draw_image() in FLTK 1.4.0 and/or later.
static int LX = 0;  // 0 for default: ld() = 0, i.e. ld() defaults (internally) to w()*d()
                    // +1: ld() = (w() + DX) * d()
                    // -1 to flip image vertically: ld() = - ((w() + DX) * d())
static int FLIPH = 0; // 1 = Flip image horizontally (only if IMG == 0)
                      // 0 = Draw image normal, w/o horizontal flipping

// ----------------------------------------------------------------------
//  Test scenario for fl_draw_image() with pos. and neg. d and ld args:
// ----------------------------------------------------------------------
//  (1) set IMG   =  0:	normal, but w/o transparency: no checker board
//  (2) set LX    = -1:	images flipped vertically
//  (3) set FLIPH =  1:	images flipped vertically and horizontally
//  (4) set LX    =  0:	images flipped horizontally
//  (5) set FLIPH =  0, IMG = 1: back to default (with transparency)
// ----------------------------------------------------------------------

class ImageTest : public Fl_Group {
  static void build_imgs() {
    int x, y;
    uchar *dg, *dga, *drgb, *drgba;
    dg    = img_gray   = img_gray_base   = (uchar*)malloc((128+DX)*128*1);
    dga   = img_gray_a = img_gray_a_base = (uchar*)malloc((128+DX)*128*2);
    drgb  = img_rgb    = img_rgb_base    = (uchar*)malloc((128+DX)*128*3);
    drgba = img_rgba   = img_rgba_base   = (uchar*)malloc((128+DX)*128*4);
    for (y=0; y<128; y++) {
      for (x=0; x<128; x++) {
        *drgba++ = *drgb++ = *dga++ = *dg++ = y<<1;
        *drgba++ = *drgb++                  = x<<1;
        *drgba++ = *drgb++                  = (127-x)<<1;
        *drgba++           = *dga++         = x+y;
      }
      if (DX > 0 && LX != 0) {
        memset(dg,   0,1*DX); dg    += 1*DX;
        memset(dga,  0,2*DX); dga   += 2*DX;
        memset(drgb, 0,3*DX); drgb  += 3*DX;
        memset(drgba,0,4*DX); drgba += 4*DX;
      }
    }
    if (LX<0) {
      img_gray   += 127*(128+DX);
      img_gray_a += 127*(128+DX)*2;
      img_rgb    += 127*(128+DX)*3;
      img_rgba   += 127*(128+DX)*4;
    }
    if (FLIPH && !IMG ) {
      img_gray   += 127;
      img_gray_a += 127*2;
      img_rgb    += 127*3;
      img_rgba   += 127*4;
    }
    i_g    = new Fl_RGB_Image (img_gray  ,128,128,1,LX*(128+DX));
    i_ga   = new Fl_RGB_Image (img_gray_a,128,128,2,LX*(128+DX)*2);
    i_rgb  = new Fl_RGB_Image (img_rgb,   128,128,3,LX*(128+DX)*3);
    i_rgba = new Fl_RGB_Image (img_rgba,  128,128,4,LX*(128+DX)*4);
  } // build_imgs method ends

  void free_images() {
    if (i_rgba) { delete i_rgba; i_rgba = 0; }
    if (i_rgb) { delete i_rgb; i_rgb = 0; }
    if (i_ga) { delete i_ga; i_ga = 0; }
    if (i_g) { delete i_g; i_g = 0; }
    if (img_rgba_base) { free (img_rgba_base); img_rgba_base = 0; }
    if (img_rgb_base) { free (img_rgb_base); img_rgb_base = 0; }
    if (img_gray_a_base) { free (img_gray_a_base); img_gray_a_base = 0; }
    if (img_gray_base) { free (img_gray_base); img_gray_base = 0; }
  } // end of free_images method

  static void refresh_imgs_CB(Fl_Widget*,void *data) {
    ImageTest *it = (ImageTest*)data;
    it->free_images(); // release the previous images
    // determine the state for the next images
    CB = it->ck_CB->value();
    IMG = it->ck_IMG->value();
    FLIPH = it->ck_FLIPH->value();
    // read the LX state radio buttons
    if (it->rb_LXp1->value()) { LX = 1; }
    else if (it->rb_LXm1->value()) { LX = (-1); }
    else { LX = 0; }
    // construct the next images
    build_imgs();
    it->redraw();
  }

public:
  static Fl_Widget *create() {
    build_imgs();
    return new ImageTest(TESTAREA_X, TESTAREA_Y, TESTAREA_W, TESTAREA_H);
  } // create method ends

  static uchar *img_gray_base;
  static uchar *img_gray_a_base;
  static uchar *img_rgb_base;
  static uchar *img_rgba_base;
  static uchar *img_gray;
  static uchar *img_gray_a;
  static uchar *img_rgb;
  static uchar *img_rgba;
  static Fl_RGB_Image *i_g;
  static Fl_RGB_Image *i_ga;
  static Fl_RGB_Image *i_rgb;
  static Fl_RGB_Image *i_rgba;

  // control widgets
  Fl_Group *ctr_grp;
  Fl_Check_Button *ck_CB;
  Fl_Check_Button *ck_IMG;
  Fl_Check_Button *ck_FLIPH;
  Fl_Radio_Button *rb_LXm1;
  Fl_Radio_Button *rb_LX0;
  Fl_Radio_Button *rb_LXp1;
  Fl_Button *refresh;

  ImageTest(int x, int y, int w, int h) : Fl_Group(x, y, w, h) {
    label("Testing Image Drawing\n\n"
	"This test renders four images, two of them with a checker board\n"
	"visible through the graphics. Color and gray gradients should be\n"
	"visible. This does not test any image formats such as JPEG.");
    align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);
    int cw = 90;
    int ch = 270;
    int cx = x + w - cw - 5;
    int cy = y + 10;
    ctr_grp = new Fl_Group(cx, cy, cw, ch);

    ck_CB = new Fl_Check_Button(cx+10, cy+10, cw-20, 30, "CB");
    ck_CB->value(CB);

    ck_IMG = new Fl_Check_Button(cx+10, cy+40, cw-20, 30, "IMG");
    ck_IMG->value(IMG);

    ck_FLIPH = new Fl_Check_Button(cx+10, cy+70, cw-20, 30, "FLIPH");
    ck_FLIPH->value(FLIPH);

    Fl_Group *rd_grp = new Fl_Group(cx+10, cy+100, cw-20, 90, "LX");

    rb_LXp1 = new Fl_Radio_Button(cx+15, cy+105, cw-30, 20, "+1");
    rb_LX0  = new Fl_Radio_Button(cx+15, cy+125, cw-30, 20, "0");
    rb_LXm1 = new Fl_Radio_Button(cx+15, cy+145, cw-30, 20, "-1");
    rb_LX0->value(1);

    rd_grp->box(FL_BORDER_BOX);
    rd_grp->align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_CENTER);
    rd_grp->end();

    refresh = new Fl_Button(cx+10, cy+ch-40, cw-20, 30, "Refresh");
    refresh->callback(refresh_imgs_CB, (void*)this);

    ctr_grp->box(FL_BORDER_BOX);
    ctr_grp->end();
    end(); // make sure this ImageTest group is closed
  } // constructor ends

  void draw() {
    Fl_Group::draw();

    // top left: RGB

    int xx = x()+10, yy = y()+10;
    fl_color(FL_BLACK); fl_rect(xx, yy, 130, 130);
    if (IMG) {
      i_rgb->draw(xx+1,yy+1);
    }
    else {
      if (!FLIPH) {
        fl_draw_image(img_rgb, xx+1, yy+1, 128, 128, 3, LX*((128+DX)*3));
      }
      else {
        fl_draw_image(img_rgb, xx+1, yy+1, 128, 128,-3, LX*((128+DX)*3));
      }
    }
    fl_draw("RGB", xx+134, yy+64);

    // bottom left: RGBA

    xx = x()+10; yy = y()+10+134;
    fl_color(FL_BLACK); fl_rect(xx, yy, 130, 130);	// black frame
    fl_color(FL_WHITE); fl_rectf(xx+1, yy+1, 128, 128);	// white background
    if (CB) { // checker board
      fl_color(FL_BLACK); fl_rectf(xx+65, yy+1, 64, 64);
      fl_color(FL_BLACK); fl_rectf(xx+1, yy+65, 64, 64);
    }
    if (IMG) {
      i_rgba->draw(xx+1,yy+1);
    }
    else {
      if (!FLIPH) {
        fl_draw_image(img_rgba, xx+1, yy+1, 128, 128, 4, LX*((128+DX)*4));
      }
      else {
        fl_draw_image(img_rgba, xx+1, yy+1, 128, 128,-4, LX*((128+DX)*4));
      }
    }
    fl_color(FL_BLACK); fl_draw("RGBA", xx+134, yy+64);

    // top right: Gray

    xx = x()+10+200; yy = y()+10;
    fl_color(FL_BLACK); fl_rect(xx, yy, 130, 130);
    if (IMG) {
      i_g->draw(xx+1,yy+1);
    }
    else {
      if (!FLIPH) {
        fl_draw_image(img_gray, xx+1, yy+1, 128, 128, 1, LX*((128+DX)*1));
      }
      else {
        fl_draw_image(img_gray, xx+1, yy+1, 128, 128,-1, LX*((128+DX)*1));
      }
    }
    fl_draw("Gray", xx+134, yy+64);

    // bottom right: Gray+Alpha

    xx = x()+10+200; yy = y()+10+134;
    fl_color(FL_BLACK); fl_rect(xx, yy, 130, 130);	// black frame
    fl_color(FL_WHITE); fl_rectf(xx+1, yy+1, 128, 128);	// white background
    if (CB) { // checker board
      fl_color(FL_BLACK); fl_rectf(xx+65, yy+1, 64, 64);
      fl_color(FL_BLACK); fl_rectf(xx+1, yy+65, 64, 64);
    }
    if (IMG) {
      i_ga->draw(xx+1,yy+1);
    }
    else {
      if (!FLIPH) {
        fl_draw_image(img_gray_a, xx+1, yy+1, 128, 128, 2, LX*((128+DX)*2));
      }
      else {
        fl_draw_image(img_gray_a, xx+1, yy+1, 128, 128,-2, LX*((128+DX)*2));
      }
    }
    fl_color(FL_BLACK); fl_draw("Gray+Alpha", xx+134, yy+64);
  } // draw method end
};

uchar *ImageTest::img_gray_base = 0;
uchar *ImageTest::img_gray_a_base = 0;
uchar *ImageTest::img_rgb_base = 0;
uchar *ImageTest::img_rgba_base = 0;
uchar *ImageTest::img_gray = 0;
uchar *ImageTest::img_gray_a = 0;
uchar *ImageTest::img_rgb = 0;
uchar *ImageTest::img_rgba = 0;
Fl_RGB_Image *ImageTest::i_g = 0;
Fl_RGB_Image *ImageTest::i_ga = 0;
Fl_RGB_Image *ImageTest::i_rgb = 0;
Fl_RGB_Image *ImageTest::i_rgba = 0;

UnitTest images("drawing images", ImageTest::create);

//
// End of "$Id$"
//
