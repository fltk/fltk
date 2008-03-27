//
// "$Id$"
//
// fl_draw_image test program for the Fast Light Tool Kit (FLTK).
//
// Be sure to try every visual with the -v switch and try -m (monochrome)
// on each of them.
//
// This program requires either the libjpeg.a library or an internal DD
// library to read images (this is chosen by the presence of the "DD"
// #define).
//
// To get the jpeg library:
//
// The "official" archive site for this software is ftp.uu.net (Internet
// address 192.48.96.9).  The most recent released version can always be
// found there in directory graphics/jpeg.  This particular version will
// be archived as graphics/jpeg/jpegsrc.v6a.tar.gz.
//
// The makefile assummes you decompressed and build these in a directory
// called "jpeg-6a" in the same location as the "FL" directory.
//
// Copyright 1998-2005 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <stdio.h>
#include <stdlib.h>

void readtheimage(const char *name); // below
int width;
int height;
int depth;
int linedelta;
uchar *ibuffer;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Window.H>
int mono;

class image_window : public Fl_Window {
  void draw();
public:
  image_window(int w,int h) : Fl_Window(w,h) {box(FL_NO_BOX);}
};

void image_window::draw() {
  if (mono)
    fl_draw_image_mono(ibuffer+1,0,0,width,height,depth,linedelta);
  else
    fl_draw_image(ibuffer,0,0,width,height,depth,linedelta);
}

////////////////////////////////////////////////////////////////

#include <FL/x.H>
#include "list_visuals.cxx"

////////////////////////////////////////////////////////////////

int visid = -1;
int arg(int argc, char **argv, int &i) {
  if (argv[i][1] == 'm') {mono = 1; i++; return 1;}

  if (argv[i][1] == 'v') {
    if (i+1 >= argc) return 0;
    visid = atoi(argv[i+1]);
    i += 2;
    return 2;
  }

  return 0;
}

int main(int argc, char ** argv) {

  int i = 1;
  if (!Fl::args(argc,argv,i,arg) || i != argc-1) {
    fprintf(stderr,"usage: %s <switches> image_file\n"
" -v # : use visual\n"
" -m : monochrome\n"
"%s\n",
	    argv[0],Fl::help);
    exit(1);
  }

  readtheimage(argv[i]);
  image_window *window = new image_window(width,height);

  if (visid>=0) {
    fl_open_display();
    XVisualInfo templt; int num;
    templt.visualid = visid;
    fl_visual = XGetVisualInfo(fl_display, VisualIDMask, &templt, &num);
    if (!fl_visual) {
      fprintf(stderr, "No visual with id %d, use one of:\n",visid);
      list_visuals();
      exit(1);
    }
    fl_colormap = XCreateColormap(fl_display, RootWindow(fl_display,fl_screen),
				fl_visual->visual, AllocNone);
    fl_xpixel(0); // make sure black is allocated
  }

  window->show(argc,argv);
  return Fl::run();
}

////////////////////////////////////////////////////////////////
#ifndef DD_LIBRARY
// Read using jpeg library:

extern "C" {
#include "jpeglib.h"
}

void readtheimage(const char *name) {
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE * infile = fopen(name, "rb");
  if (!infile) {
    fprintf(stderr, "can't open %s\n", name);
    exit(1);
  }
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);
  width = cinfo.output_width;
  height = cinfo.output_height;
  depth = cinfo.output_components;
  ibuffer = new uchar[width*height*depth];
  uchar *rp = ibuffer;
  for (int i=0; i<height; i++) {
    jpeg_read_scanlines(&cinfo, &rp, 1);
    rp += width*depth;
  }
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);
}

////////////////////////////////////////////////////////////////
#else // Digital Domain in-house library

#include "DDNewImage/DDImageOp.H"
#include "DDNewImage/DDImgRead.H"
#include "DDNewImage/DDImgToBuffer.H"

void readtheimage(const char *name) {
  DDImgRead reader(name);
  width = reader.xsize();
  height = reader.ysize();
  depth = 4; // reader.zsize();
  ibuffer = new uchar[width*height*depth];
  DDImgToBuffer b(&reader,depth,ibuffer,0,0,width,height);
  b.execute();
  if (DDImage::haderror) {
    fprintf(stderr,"%s\n",DDImage::errormsg());
    exit(1);
  }
  // swap it around into RGBA order:
  for (uchar *p = ibuffer+width*height*4-4; p >= ibuffer; p-=4) {
    uchar r = p[3];
    uchar g = p[2];
    uchar b = p[1];
    uchar a = p[0];
    p[0] = r;
    p[1] = g;
    p[2] = b;
    p[3] = a;
  }
  // make it bottom-to-top:
  ibuffer = ibuffer + width*(height-1)*depth;
  linedelta = -(width*depth);
}
#endif

//
// End of "$Id$".
//
