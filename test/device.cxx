//
// "$Id$"
//
// Device test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Roman Kantor and others.
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

#include <math.h>
#include <FL/Fl.H>

#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Clock.H>
#include "pixmaps/porsche.xpm"
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Bitmap.H>
#include <FL/Fl_Round_Button.H>


#include <FL/Fl_Printer.H>
#include <FL/Fl_Copy_Surface.H>
#include <FL/Fl_Image_Surface.H>

#include <FL/Fl_File_Chooser.H>
#include <FL/fl_draw.H>


#define sorceress_width 75
#define sorceress_height 75


// shameles copy from bitmap...
static uchar sorceress_bits[] = {
  0xfc, 0x7e, 0x40, 0x20, 0x90, 0x00, 0x07, 0x80, 0x23, 0x00, 0x00, 0xc6,
  0xc1, 0x41, 0x98, 0xb8, 0x01, 0x07, 0x66, 0x00, 0x15, 0x9f, 0x03, 0x47,
  0x8c, 0xc6, 0xdc, 0x7b, 0xcc, 0x00, 0xb0, 0x71, 0x0e, 0x4d, 0x06, 0x66,
  0x73, 0x8e, 0x8f, 0x01, 0x18, 0xc4, 0x39, 0x4b, 0x02, 0x23, 0x0c, 0x04,
  0x1e, 0x03, 0x0c, 0x08, 0xc7, 0xef, 0x08, 0x30, 0x06, 0x07, 0x1c, 0x02,
  0x06, 0x30, 0x18, 0xae, 0xc8, 0x98, 0x3f, 0x78, 0x20, 0x06, 0x02, 0x20,
  0x60, 0xa0, 0xc4, 0x1d, 0xc0, 0xff, 0x41, 0x04, 0xfa, 0x63, 0x80, 0xa1,
  0xa4, 0x3d, 0x00, 0x84, 0xbf, 0x04, 0x0f, 0x06, 0xfc, 0xa1, 0x34, 0x6b,
  0x01, 0x1c, 0xc9, 0x05, 0x06, 0xc7, 0x06, 0xbe, 0x11, 0x1e, 0x43, 0x30,
  0x91, 0x05, 0xc3, 0x61, 0x02, 0x30, 0x1b, 0x30, 0xcc, 0x20, 0x11, 0x00,
  0xc1, 0x3c, 0x03, 0x20, 0x0a, 0x00, 0xe8, 0x60, 0x21, 0x00, 0x61, 0x1b,
  0xc1, 0x63, 0x08, 0xf0, 0xc6, 0xc7, 0x21, 0x03, 0xf8, 0x08, 0xe1, 0xcf,
  0x0a, 0xfc, 0x4d, 0x99, 0x43, 0x07, 0x3c, 0x0c, 0xf1, 0x9f, 0x0b, 0xfc,
  0x5b, 0x81, 0x47, 0x02, 0x16, 0x04, 0x31, 0x1c, 0x0b, 0x1f, 0x17, 0x89,
  0x4d, 0x06, 0x1a, 0x04, 0x31, 0x38, 0x02, 0x07, 0x56, 0x89, 0x49, 0x04,
  0x0b, 0x04, 0xb1, 0x72, 0x82, 0xa1, 0x54, 0x9a, 0x49, 0x04, 0x1d, 0x66,
  0x50, 0xe7, 0xc2, 0xf0, 0x54, 0x9a, 0x58, 0x04, 0x0d, 0x62, 0xc1, 0x1f,
  0x44, 0xfc, 0x51, 0x90, 0x90, 0x04, 0x86, 0x63, 0xe0, 0x74, 0x04, 0xef,
  0x31, 0x1a, 0x91, 0x00, 0x02, 0xe2, 0xc1, 0xfd, 0x84, 0xf9, 0x30, 0x0a,
  0x91, 0x00, 0x82, 0xa9, 0xc0, 0xb9, 0x84, 0xf9, 0x31, 0x16, 0x81, 0x00,
  0x42, 0xa9, 0xdb, 0x7f, 0x0c, 0xff, 0x1c, 0x16, 0x11, 0x00, 0x02, 0x28,
  0x0b, 0x07, 0x08, 0x60, 0x1c, 0x02, 0x91, 0x00, 0x46, 0x29, 0x0e, 0x00,
  0x00, 0x00, 0x10, 0x16, 0x11, 0x02, 0x06, 0x29, 0x04, 0x00, 0x00, 0x00,
  0x10, 0x16, 0x91, 0x06, 0xa6, 0x2a, 0x04, 0x00, 0x00, 0x00, 0x18, 0x24,
  0x91, 0x04, 0x86, 0x2a, 0x04, 0x00, 0x00, 0x00, 0x18, 0x27, 0x93, 0x04,
  0x96, 0x4a, 0x04, 0x00, 0x00, 0x00, 0x04, 0x02, 0x91, 0x04, 0x86, 0x4a,
  0x0c, 0x00, 0x00, 0x00, 0x1e, 0x23, 0x93, 0x04, 0x56, 0x88, 0x08, 0x00,
  0x00, 0x00, 0x90, 0x21, 0x93, 0x04, 0x52, 0x0a, 0x09, 0x80, 0x01, 0x00,
  0xd0, 0x21, 0x95, 0x04, 0x57, 0x0a, 0x0f, 0x80, 0x27, 0x00, 0xd8, 0x20,
  0x9d, 0x04, 0x5d, 0x08, 0x1c, 0x80, 0x67, 0x00, 0xe4, 0x01, 0x85, 0x04,
  0x79, 0x8a, 0x3f, 0x00, 0x00, 0x00, 0xf4, 0x11, 0x85, 0x06, 0x39, 0x08,
  0x7d, 0x00, 0x00, 0x18, 0xb7, 0x10, 0x81, 0x03, 0x29, 0x12, 0xcb, 0x00,
  0x7e, 0x30, 0x28, 0x00, 0x85, 0x03, 0x29, 0x10, 0xbe, 0x81, 0xff, 0x27,
  0x0c, 0x10, 0x85, 0x03, 0x29, 0x32, 0xfa, 0xc1, 0xff, 0x27, 0x94, 0x11,
  0x85, 0x03, 0x28, 0x20, 0x6c, 0xe1, 0xff, 0x07, 0x0c, 0x01, 0x85, 0x01,
  0x28, 0x62, 0x5c, 0xe3, 0x8f, 0x03, 0x4e, 0x91, 0x80, 0x05, 0x39, 0x40,
  0xf4, 0xc2, 0xff, 0x00, 0x9f, 0x91, 0x84, 0x05, 0x31, 0xc6, 0xe8, 0x07,
  0x7f, 0x80, 0xcd, 0x00, 0xc4, 0x04, 0x31, 0x06, 0xc9, 0x0e, 0x00, 0xc0,
  0x48, 0x88, 0xe0, 0x04, 0x79, 0x04, 0xdb, 0x12, 0x00, 0x30, 0x0c, 0xc8,
  0xe4, 0x04, 0x6d, 0x06, 0xb6, 0x23, 0x00, 0x18, 0x1c, 0xc0, 0x84, 0x04,
  0x25, 0x0c, 0xff, 0xc2, 0x00, 0x4e, 0x06, 0xb0, 0x80, 0x04, 0x3f, 0x8a,
  0xb3, 0x83, 0xff, 0xc3, 0x03, 0x91, 0x84, 0x04, 0x2e, 0xd8, 0x0f, 0x3f,
  0x00, 0x00, 0x5f, 0x83, 0x84, 0x04, 0x2a, 0x70, 0xfd, 0x7f, 0x00, 0x00,
  0xc8, 0xc0, 0x84, 0x04, 0x4b, 0xe2, 0x2f, 0x01, 0x00, 0x08, 0x58, 0x60,
  0x80, 0x04, 0x5b, 0x82, 0xff, 0x01, 0x00, 0x08, 0xd0, 0xa0, 0x84, 0x04,
  0x72, 0x80, 0xe5, 0x00, 0x00, 0x08, 0xd2, 0x20, 0x44, 0x04, 0xca, 0x02,
  0xff, 0x00, 0x00, 0x08, 0xde, 0xa0, 0x44, 0x04, 0x82, 0x02, 0x6d, 0x00,
  0x00, 0x08, 0xf6, 0xb0, 0x40, 0x02, 0x82, 0x07, 0x3f, 0x00, 0x00, 0x08,
  0x44, 0x58, 0x44, 0x02, 0x93, 0x3f, 0x1f, 0x00, 0x00, 0x30, 0x88, 0x4f,
  0x44, 0x03, 0x83, 0x23, 0x3e, 0x00, 0x00, 0x00, 0x18, 0x60, 0xe0, 0x07,
  0xe3, 0x0f, 0xfe, 0x00, 0x00, 0x00, 0x70, 0x70, 0xe4, 0x07, 0xc7, 0x1b,
  0xfe, 0x01, 0x00, 0x00, 0xe0, 0x3c, 0xe4, 0x07, 0xc7, 0xe3, 0xfe, 0x1f,
  0x00, 0x00, 0xff, 0x1f, 0xfc, 0x07, 0xc7, 0x03, 0xf8, 0x33, 0x00, 0xc0,
  0xf0, 0x07, 0xff, 0x07, 0x87, 0x02, 0xfc, 0x43, 0x00, 0x60, 0xf0, 0xff,
  0xff, 0x07, 0x8f, 0x06, 0xbe, 0x87, 0x00, 0x30, 0xf8, 0xff, 0xff, 0x07,
  0x8f, 0x14, 0x9c, 0x8f, 0x00, 0x00, 0xfc, 0xff, 0xff, 0x07, 0x9f, 0x8d,
  0x8a, 0x0f, 0x00, 0x00, 0xfe, 0xff, 0xff, 0x07, 0xbf, 0x0b, 0x80, 0x1f,
  0x00, 0x00, 0xff, 0xff, 0xff, 0x07, 0x7f, 0x3a, 0x80, 0x3f, 0x00, 0x80,
  0xff, 0xff, 0xff, 0x07, 0xff, 0x20, 0xc0, 0x3f, 0x00, 0x80, 0xff, 0xff,
  0xff, 0x07, 0xff, 0x01, 0xe0, 0x7f, 0x00, 0xc0, 0xff, 0xff, 0xff, 0x07,
  0xff, 0x0f, 0xf8, 0xff, 0x40, 0xe0, 0xff, 0xff, 0xff, 0x07, 0xff, 0xff,
  0xff, 0xff, 0x40, 0xf0, 0xff, 0xff, 0xff, 0x07, 0xff, 0xff, 0xff, 0xff,
  0x41, 0xf0, 0xff, 0xff, 0xff, 0x07};

class MyWidget: public Fl_Box{
protected:
  void draw(){
    Fl_Box::draw();
    fl_color(FL_RED);
    fl_rectf(x()+5,y()+5,w()-10,h()-10);
    fl_push_clip(x()+6,y()+6,w()-12,h()-12);
    fl_color(FL_DARK_GREEN);
    fl_rectf(x()+5,y()+5,w()-10,h()-10);
    fl_pop_clip();
    fl_color(FL_YELLOW);
    fl_rectf(x()+7,y()+7,w()-14,h()-14);
    fl_color(FL_BLUE);
    
    fl_rect(x()+8,y()+8,w()-16,h()-16);
    fl_push_clip(x()+25,y()+25,w()-50, h()-50);
    fl_color(FL_BLACK);
    fl_rect(x()+24,y()+24,w()-48,h()-48);
    fl_line(x()+27,y()+27,x()+w()-27,y()+h()-27);
    fl_line(x()+27,y()+h()-27,x()+w()-27,y()+27);
    //fl_rect(x()+30,y()+30,w()-60,h()-60);
    fl_pop_clip();
    
  }
public:
  MyWidget(int x, int y):Fl_Box(x,y,100,100, "Clipping and rect(f):\nYellow rect.framed\nby B-Y-G-R rect. 1 p.\nthick. Your printer may \nrender very thin lines\nsurrounding \"X\""){
    align(FL_ALIGN_TOP);
    labelsize(10);
  };
};


class MyWidget2: public Fl_Box{
protected:
  void draw(){
    Fl_Box::draw();
    int d;
    //    fl_line_style(0);
    for(d=y()+5;d<48+y();d+=2){
      fl_xyline(x()+5,d,x()+48);
    }
    
    fl_push_clip(x()+52,y()+5,45,43);
    for(d=y()+5;d<150+y();d+=3){
      fl_line(x()+52,d,x()+92,d-40);
    }
    fl_pop_clip();
    
    fl_line_style(FL_DASH);
    fl_xyline(x()+5,y()+55,x()+48);
    fl_line_style(FL_DOT);
    fl_xyline(x()+5,y()+58,x()+48);
    fl_line_style(FL_DASHDOT);
    fl_xyline(x()+5,y()+61,x()+48);
    fl_line_style(FL_DASHDOTDOT);
    fl_xyline(x()+5,y()+64,x()+48);
    fl_line_style(0,0,(char*)"\7\3\7\2");
    fl_xyline(x()+5,y()+67,x()+48);
    
    fl_line_style(0);
    
    fl_line(x()+5,y()+72,x()+25,y()+95);
    fl_line(x()+8,y()+72,x()+28,y()+95,x()+31,y()+72);
    
    fl_color(FL_YELLOW);
    fl_polygon(x()+11, y()+72,x()+27,y()+91,x()+29,y()+72);
    fl_color(FL_RED);
    fl_loop(x()+11, y()+72,x()+27,y()+91,x()+29,y()+72);
    
    fl_color(FL_BLUE); ////
    fl_line_style(FL_SOLID, 6);
    fl_loop(x()+31, y()+12,x()+47,y()+31,x()+49,y()+12);
    fl_line_style(0);
    
    fl_color(200,0,200);
    fl_polygon(x()+35,y()+72,x()+33,y()+95,x()+48,y()+95,x()+43,y()+72);
    fl_color(FL_GREEN);
    fl_loop(x()+35,y()+72,x()+33,y()+95,x()+48,y()+95,x()+43,y()+72);
    
    fl_color(FL_BLUE);    
    fl_yxline(x()+65,y()+63,y()+66);
    fl_color(FL_GREEN);    
    fl_yxline(x()+66,y()+66,y()+63);
    
    fl_color(FL_BLUE);
    fl_rect(x()+80,y()+55,5,5);
    fl_color(FL_YELLOW);
    fl_rectf(x()+81,y()+56,3,3);
    fl_color(FL_BLACK);
    fl_point(x()+82,y()+57);
    
    fl_color(FL_BLUE);
    fl_rect(x()+56, y()+79, 24, 17);
    fl_color(FL_CYAN);
    fl_rectf(x()+57, y()+80, 22 , 15 );
    fl_color(FL_RED);
    fl_arc(x()+57, y()+80, 22 ,15 ,40, 270);
    fl_color(FL_YELLOW);
    fl_pie(x()+58, y()+81, 20 ,13 ,40, 270);
    
    fl_line_style(0);
    
    fl_color(FL_BLACK);
    fl_point(x()+58,y()+58);
    fl_color(FL_RED);
    fl_yxline(x()+59,y()+58,y()+59);
    fl_color(FL_GREEN);
    fl_yxline(x()+60,y()+59,y()+58);
    fl_color(FL_BLACK);
    fl_xyline(x()+61,y()+58,x()+62);
    fl_color(FL_RED);
    fl_xyline(x()+62,y()+59,x()+61);
    
    fl_color(FL_GREEN);
    fl_yxline(x()+57,y()+58,y()+59,x()+58);
    fl_color(FL_BLUE);
    fl_xyline(x()+58,y()+60,x()+56,y()+58);
    fl_color(FL_RED);
    fl_xyline(x()+58,y()+61,x()+56,y()+63);
    fl_color(FL_GREEN);
    fl_yxline(x()+57,y()+63,y()+62,x()+58);
    
    fl_color(FL_BLUE);
    fl_line(x()+58,y()+63, x()+60, y()+65);
    fl_color(FL_BLACK);
    fl_line(x()+61,y()+65, x()+59, y()+63);
    
    fl_color(FL_BLACK);
  };
  
public:
  MyWidget2(int x, int y):Fl_Box(x,y,100,100, "Integer primitives"){
    labelsize(10);
    align(FL_ALIGN_TOP);
  };
};


class MyWidget3: public Fl_Box{
protected:
  void draw(){
    Fl_Box::draw();
    double d;
    //    fl_line_style(0);
    fl_push_clip(x()+5,y()+5,45,43);
    for(d=y()+5;d<95+y();d+=1.63){
      fl_begin_line();
      fl_vertex(x()+5,d);
      fl_vertex(x()+48,d);
      fl_end_line();
    }
    fl_pop_clip();
    
    fl_push_clip(x()+52,y()+5,45,43);
    for(d=y()+5;d<150+y();d+=2.3052){
      fl_begin_line();
      fl_vertex(x()+52,d);
      fl_vertex(x()+92,d-43);
      fl_end_line();
    }
    fl_pop_clip();
    
  };
public:
  MyWidget3(int x, int y):Fl_Box(x,y,100,100, "Sub-pixel drawing of\nlines 1.63 points apart\nOn the screen you\ncan see aliasing, the\nprinter should render\nthem properly"){
    labelsize(10);
    align(FL_ALIGN_TOP);
  };
};



class MyWidget4: public Fl_Box{
protected:
  void draw(){
    Fl_Box::draw();
    fl_push_matrix();
    fl_translate(x(),y());
    fl_scale(.75,.75);
    
    fl_line_style(FL_SOLID , 5);
    fl_begin_line();
    fl_vertex(10, 160);
    fl_vertex(40, 160);
    fl_vertex(40, 190);
    fl_end_line();
    fl_line_style(0);

    fl_color(FL_RED);
    fl_line_style(FL_SOLID | FL_CAP_FLAT |FL_JOIN_MITER , 5);
    fl_begin_line();
    fl_vertex(10, 150);
    fl_vertex(50, 150);
    fl_vertex(50, 190);
    fl_end_line();
    fl_line_style(0);
    
    fl_color(FL_GREEN);   
    fl_line_style(FL_SOLID | FL_CAP_ROUND |FL_JOIN_ROUND , 5);
    fl_begin_line();
    fl_vertex(10, 140);
    fl_vertex(60, 140);
    fl_vertex(60, 190);
    fl_end_line();
    fl_line_style(0);
    
    fl_color(FL_BLUE);
    fl_line_style(FL_SOLID | FL_CAP_SQUARE |FL_JOIN_BEVEL , 5);
    fl_begin_line();
    fl_vertex(10, 130);
    fl_vertex(70, 130);
    fl_vertex(70, 190);
    fl_end_line();
    fl_line_style(0);
    
    fl_color(FL_BLACK);
    fl_line_style(FL_DASH , 5);
    fl_begin_line();
    fl_vertex(10, 120);
    fl_vertex(80, 120);
    fl_vertex(80, 190);
    fl_end_line();
    fl_line_style(0);
    
    fl_color(FL_RED);
    fl_line_style(FL_DASH |FL_CAP_FLAT , 5);
    fl_begin_line();
    fl_vertex(10, 110);
    fl_vertex(90, 110);
    fl_vertex(90, 190);
    fl_end_line();
    fl_line_style(0);
    
    fl_color(FL_GREEN);
    fl_line_style(FL_DASH |FL_CAP_ROUND , 5);
    fl_begin_line();
    fl_vertex(10, 100);
    fl_vertex(100, 100);
    fl_vertex(100, 190);
    fl_end_line();
    fl_line_style(0);
    
    
    fl_color(FL_BLUE);
    fl_line_style(FL_DASH |FL_CAP_SQUARE , 5);
    fl_begin_line();
    fl_vertex(10, 90);
    fl_vertex(110, 90);
    fl_vertex(110, 190);
    fl_end_line();
    fl_line_style(0);
    
    fl_color(FL_BLACK);
    fl_line_style(FL_DOT, 5);
    fl_begin_line();
    fl_vertex(10, 80);
    fl_vertex(120, 80);
    fl_vertex(120, 190);
    fl_end_line();
    fl_line_style(0);
    
    fl_color(FL_RED);
    fl_line_style(FL_DOT | FL_CAP_FLAT, 5);
    fl_begin_line();
    fl_vertex(10, 70);
    fl_vertex(130, 70);
    fl_vertex(130, 190);
    fl_end_line();
    fl_line_style(0);
    
    fl_color(FL_GREEN);
    fl_line_style(FL_DOT | FL_CAP_ROUND, 5);
    fl_begin_line();
    fl_vertex(10, 60);
    fl_vertex(140, 60);
    fl_vertex(140, 190);
    fl_end_line();
    fl_line_style(0);
    
    fl_color(FL_BLUE);
    fl_line_style(FL_DOT | FL_CAP_SQUARE, 5);
    fl_begin_line();
    fl_vertex(10, 50);
    fl_vertex(150, 50);
    fl_vertex(150, 190);
    fl_end_line();
    fl_line_style(0);
    
    fl_color(FL_BLACK);
    fl_line_style(FL_DASHDOT |FL_CAP_ROUND |FL_JOIN_ROUND , 5);
    fl_begin_line();
    fl_vertex(10, 40);
    fl_vertex(160, 40);
    fl_vertex(160, 190);
    fl_end_line();
    fl_line_style(0);
    
    fl_color(FL_RED);
    fl_line_style(FL_DASHDOTDOT |FL_CAP_SQUARE |FL_JOIN_BEVEL , 5);
    fl_begin_line();
    fl_vertex(10, 30);
    fl_vertex(170, 30);
    fl_vertex(170, 190);
    fl_end_line();
    fl_line_style(0);
    
    
    fl_color(FL_GREEN);
    fl_line_style(FL_DASHDOTDOT |FL_CAP_ROUND |FL_JOIN_ROUND , 5);
    fl_begin_line();
    fl_vertex(10, 20);
    fl_vertex(180, 20);
    fl_vertex(180, 190);
    fl_end_line();
    fl_line_style(0);
    
    fl_color(FL_BLUE);
    fl_line_style(0, 5, (char*)"\12\3\4\2\2\1");
    fl_begin_line();
    fl_vertex(10, 10);
    fl_vertex(190, 10);
    fl_vertex(190, 190);
    
    fl_end_line();
    fl_line_style(0);
    fl_pop_matrix();    
    
    fl_color(FL_BLACK);
  };
public:
  MyWidget4(int x, int y):Fl_Box(x,y,150,150, "Line styles"){
    labelsize(10);
    align(FL_ALIGN_TOP);
  };
};


class MyWidget5: public Fl_Box{
protected:
  void draw(){
    Fl_Box::draw();
    fl_push_matrix();
    
    fl_translate(x(),y());
    fl_push_matrix();
    fl_mult_matrix(1,3,0,1,0,-20);
    fl_color(FL_GREEN);
    fl_begin_polygon();
    fl_vertex(10,10);
    fl_vertex(100,-80);
    fl_vertex(100,-190);
    fl_end_polygon();
    
    fl_color(FL_RED);
    fl_line_style(FL_DASHDOT, 7);
    fl_begin_loop();
    
    fl_vertex(10,10);
    fl_vertex(100,-80);
    fl_vertex(100,-190);
    fl_end_loop();
    fl_line_style(0);
    
    fl_color(FL_BLUE);
    fl_line_style(FL_SOLID, 3);
    fl_begin_loop();
    fl_circle(60,-50,30);
    fl_end_loop();
    fl_line_style(0);
    
    fl_pop_matrix();
    fl_scale(1.8,1);
    
    fl_color(FL_YELLOW);
    fl_begin_polygon();
    fl_arc(30,90,20,-45,200);
    fl_end_polygon();
    
    fl_color(FL_BLACK);
    fl_line_style(FL_DASH, 3);
    fl_begin_line();
    fl_arc(30,90,20,-45,200);
    fl_end_line();
    fl_line_style(0);
    
    fl_translate(15,0);
    fl_scale(1.5,3);
    fl_begin_complex_polygon();
    fl_vertex(30,70);
    fl_arc(45,55,10,200,90);
    fl_arc(55,45,8,-170,20);
    fl_vertex(60,40);
    fl_vertex(30,20);
    fl_vertex(40,5);
    fl_vertex(60,25);
    //fl_vertex(50,50);
    fl_curve(35,30,30,53,0,35,65,65);
    fl_gap();
    fl_vertex(50,25);
    fl_vertex(40,10);
    fl_vertex(35,20);
    fl_end_complex_polygon();
    
    fl_pop_matrix();
  };
public:
  MyWidget5(int x, int y):Fl_Box(x,y,230,250, "Complex (double) drawings:\nBlue ellipse may not be\ncorrectly transformed\ndue to non-orthogonal\ntransformation"){
    labelsize(10);
    align(FL_ALIGN_TOP);
  };
};


uchar *image;
int width = 80;
int height = 80;

void make_image() {
  image = new uchar[4*width*height];
  uchar *p = image;
  for (int y = 0; y < height; y++) {
    double Y = double(y)/(height-1);
    for (int x = 0; x < width; x++) {
      double X = double(x)/(width-1);
      *p++ = uchar(255*((1-X)*(1-Y))); // red in upper-left
      *p++ = uchar(255*((1-X)*Y));	// green in lower-left
      *p++ = uchar(255*(X*Y));	// blue in lower-right
      X -= 0.5;
      Y -= 0.5;
      int alpha = (int)(350 * sqrt(X * X + Y * Y));
      if (alpha < 255) *p++ = uchar(alpha);	// alpha transparency
      else *p++ = 255;
      Y += 0.5;
    }
  }
}


Fl_Widget *target;
const  char *operation;

void copy(Fl_Widget *, void *data) {
  if (strcmp(operation, "Fl_Image_Surface") == 0) {
    Fl_Image_Surface *rgb_surf;
    int W, H, decorated;
    if (target->as_window() && !target->parent()) {
      W = target->as_window()->decorated_w();
      H = target->as_window()->decorated_h();
      decorated = 1;
     }
    else {
      W = target->w();
      H = target->h();
      decorated = 0;
    }
    rgb_surf = new Fl_Image_Surface(W, H, 1);
    rgb_surf->set_current();
    if (decorated)
      rgb_surf->draw_decorated_window(target->as_window());
    else
      rgb_surf->draw(target);
    Fl_Image *img = rgb_surf->highres_image();
    delete rgb_surf;
    Fl_Display_Device::display_device()->set_current();
    Fl_Window* g2 = new Fl_Window(img->w()+10, img->h()+10);
    g2->color(FL_YELLOW);
    Fl_Box *b = new Fl_Box(FL_NO_BOX,5,5,img->w(), img->h(),0);
    b->image(img);
    g2->end();
    g2->show();
    return;
  }
  
  
  if (strcmp(operation, "Fl_Copy_Surface") == 0) {
    Fl_Copy_Surface *copy_surf;
    if (target->as_window() && !target->parent()) {
      copy_surf = new Fl_Copy_Surface(target->as_window()->decorated_w(), target->as_window()->decorated_h());
      copy_surf->set_current();
      copy_surf->draw_decorated_window(target->as_window(), 0, 0);
    }
    else {
      copy_surf = new Fl_Copy_Surface(target->w()+10, target->h()+20);
      copy_surf->set_current();
      fl_color(FL_YELLOW);fl_rectf(0,0,copy_surf->w(), copy_surf->h());
      copy_surf->draw(target, 5, 10);
    }
    delete copy_surf;
    Fl_Display_Device::display_device()->set_current();  
    }
  
  if (strcmp(operation, "Fl_Printer") == 0) {
    Fl_Printer * p = new Fl_Printer();
    if (!p->start_job(1)) {
      p->start_page();
      if (target->as_window()) p->print_window(target->as_window());
      else p->print_widget(target);
      p->end_page();
      p->end_job();
    }
    delete p;
  }
}


class My_Button:public Fl_Button{
protected:
  void draw(){
    if (type() == FL_HIDDEN_BUTTON) return;
    Fl_Color col = value() ? selection_color() : color();
    draw_box(value() ? (down_box()?down_box():fl_down(box())) : box(), col);
    fl_color(FL_WHITE);
    fl_line_style(FL_SOLID,5);
    fl_line(x()+15,y()+10,x()+w()-15,y()+h()-23);
    fl_line(x()+w()-15,y()+10,x()+15,y()+h()-23);
    fl_line_style(0);
    draw_label();
    
  };
public:
  My_Button(int x, int y, int w, int h, const char * label = 0):Fl_Button(x,y,w,h,label){}
};


void target_cb(Fl_Widget* wid, void *data)
{
  target = (Fl_Widget*)data;
}

void operation_cb(Fl_Widget* wid, void *data)
{
  operation = wid->label();
}

int main(int argc, char ** argv) {
  
  //Fl::scheme("plastic");  
  
  Fl_Window * w2 = new Fl_Window(500,560,"Graphics test");
  
  Fl_Group *c2 =new Fl_Group(3, 43, 494, 514 );
  
  new MyWidget(10,140);
  new MyWidget2(110,80);
  new MyWidget3(220,140);
  new MyWidget4(330,70);
  new MyWidget5(140,270);
  
  make_image();
  Fl_RGB_Image *rgb = new Fl_RGB_Image(image, width, height, 4);
  My_Button b_rgb(10,245,100,100,"RGB with alpha");
  b_rgb.image(rgb);
  
  My_Button b_pixmap(10,345,100,100,"Pixmap");
  Fl_Pixmap *pixmap = new Fl_Pixmap(porsche_xpm);
  b_pixmap.image(pixmap);
  
  My_Button b_bitmap(10,445,100,100,"Bitmap");
  b_bitmap.labelcolor(FL_GREEN);
  b_bitmap.image(new Fl_Bitmap(sorceress_bits,sorceress_width,sorceress_height));
  
  new Fl_Clock(360,230,120,120);
  Fl_Return_Button * ret = new Fl_Return_Button (360, 360, 120,30, "Return");
  ret->deactivate();
  Fl_Button but1(360, 390, 30, 30, "@->|");
  but1.labelcolor(FL_DARK3);
  Fl_Button but2(390, 390, 30, 30, "@UpArrow");
  but2.labelcolor(FL_DARK3);
  Fl_Button but3(420, 390, 30, 30, "@DnArrow");
  but3.labelcolor(FL_DARK3);
  Fl_Button but4(450, 390, 30, 30, "@+");
  but4.labelcolor(FL_DARK3);
  Fl_Button but5(360, 425, 120, 30, "Hello, World");
  but5.labelfont(FL_BOLD|FL_ITALIC);
  but5.labeltype(FL_SHADOW_LABEL);
  but5.box(FL_ROUND_UP_BOX);
  
  Fl_Button but6(360, 460, 120, 30, "Plastic");
  but6.box(FL_PLASTIC_UP_BOX);
  
  Fl_Group *group;
  { Fl_Group* o = new Fl_Group(360, 495, 120, 40); group=o;
    o->box(FL_UP_BOX);
    { Fl_Group* o = new Fl_Group(365, 500, 110, 30);
      o->box(FL_THIN_UP_FRAME);
      { Fl_Round_Button* o = new Fl_Round_Button(365, 500, 40, 30, "rad");
        o->value(1);
      }
      { Fl_Check_Button* o = new Fl_Check_Button(410, 500, 60, 30, "check");
        o->value(1);
        
      }
      o->end();
    }
    o->end();
    o->deactivate();
  }
  Fl_Box tx(120,492,230,50,"Background is not printed because\nencapsulating group, which we are\n printing, has not set the box type");
  tx.box(FL_SHADOW_BOX);
  tx.labelsize(12);
  
  tx.hide();
  
  c2->end();
  
  Fl_Radio_Round_Button *rb;
  Fl_Window *w3 = new Fl_Window(2,5,w2->w()-10,55);
  w3->box(FL_DOWN_BOX);
  Fl_Group *g1 = new Fl_Group(w3->x(),w3->y(),w3->w(),w3->h());
  rb = new Fl_Radio_Round_Button(5,5,150,12, "Fl_Image_Surface"); 
  rb->set(); rb->callback(operation_cb, NULL); operation = rb->label();
  rb = new Fl_Radio_Round_Button(5,22,150,12, "Fl_Copy_Surface"); rb->callback(operation_cb, NULL);
  rb = new Fl_Radio_Round_Button(5,39,150,12, "Fl_Printer"); rb->callback(operation_cb, NULL);
  g1->end();
  
  Fl_Group *g2 = new Fl_Group(w3->x(),w3->y(),w3->w(),w3->h());
  rb = new Fl_Radio_Round_Button(170,5,150,12, "Decorated Window"); 
  rb->set(); rb->callback(target_cb, w2); target = w2;
  rb = new Fl_Radio_Round_Button(170,22,150,12, "Sub-window"); rb->callback(target_cb, w3);
  rb = new Fl_Radio_Round_Button(170,39,150,12, "Group"); rb->callback(target_cb, group);
  g2->end();
  Fl_Button *b4 = new Fl_Button(330, (w3->h() - 25)/2, 150, 25, "GO");
  b4->callback((Fl_Callback*)copy,NULL);
  w3->end();
  
  w2->end();
  w2->show(argc, argv);
  
  Fl::run();
  return 0;
}

//
// End of "$Id$"
//
