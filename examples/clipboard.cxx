//
// "$Id$"
//
// Clipboard display test application for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2014 by Bill Spitzak and others.
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

#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Button.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#ifdef WIN32
#include <windows.h>
#endif // WIN32

/* Displays and follows the content of the clipboard with either image or text data
 */

Fl_Box *image_box;
Fl_Box *image_size;
Fl_Text_Display *display;

class chess : public Fl_Box { // a box with a chess-like pattern below its image
public:
  chess(int x, int y, int w, int h) : Fl_Box(FL_FLAT_BOX,x,y,w,h,0) {
    align(FL_ALIGN_CENTER | FL_ALIGN_CLIP);
  }
  void draw() {
    draw_box();
    Fl_Image *im = image();
    if (im) { // draw the chess pattern below the box centered image
      int X = x() + (w()-im->w())/2, Y = y() + (h()-im->h())/2, W = im->w(), H = im->h();
      fl_push_clip(X,Y,W,H);
      fl_push_clip(x(),y(),w(),h());
      fl_color(FL_WHITE); fl_rectf(X,Y,W,H);
      fl_color(FL_LIGHT2);
      const int side = 4, side2 = 2*side;
      for (int j=Y; j<Y+H; j+=side) {
	for (int i=X + (j-Y)%side2; i<X+W; i+=side2) {
	  fl_rectf(i,j,side,side);
	}
      }
      fl_pop_clip();
      fl_pop_clip();
    }
    draw_label(); // draw the box image
  }
} *chess_obj;

#define TAB_COLOR FL_DARK3

class clipboard_viewer : public Fl_Tabs { // use tabs to display as appropriate the image or textual content of the clipboard
public:
  clipboard_viewer(int x, int y, int w, int h) : Fl_Tabs(x,y,w,h) {};
  virtual int handle(int event) {
    if (event != FL_PASTE) return Fl_Tabs::handle(event);
    if (strcmp(Fl::event_clipboard_type(), Fl::clipboard_image) == 0) { // an image is being pasted
      Fl_Image *im = (Fl_Image*)Fl::event_clipboard(); // get it as an Fl_Image object
      if (!im) return 1;
      char title[300];
      sprintf(title, "%dx%d",im->w(), im->h()); // display the image original size
#ifdef WIN32
      OpenClipboard(NULL); // display extra technical info about clipboard content
      char *p=title + strlen(title);
      int format = EnumClipboardFormats(0);
      if (format && format < CF_MAX) { sprintf(p, " %d",format); p += strlen(p); }
      while (format) {
	format = EnumClipboardFormats(format);
	if (format && format < CF_MAX) { sprintf(p, " %d",format); p += strlen(p); }
      }
      HANDLE h;
      if ((h = GetClipboardData(CF_DIB))) {
	LPBITMAPINFO lpBI = (LPBITMAPINFO)GlobalLock(h);
	sprintf(p, " biBitCount=%d biCompression=%d biClrUsed=%d",
		lpBI->bmiHeader.biBitCount, (int)lpBI->bmiHeader.biCompression, (int)lpBI->bmiHeader.biClrUsed);
      }
      CloseClipboard();
#endif
      float scale_x =  (float)im->w() / image_box->w(); // rescale the image if larger than the display box
      float scale_y =  (float)im->h() / image_box->h();
      float scale = scale_x;
      if (scale_y > scale) scale = scale_y;
      if (scale > 1) {
	Fl_Image *im2 = im->copy(im->w()/scale, im->h()/scale);
	delete im;
	im = im2;
      }
      Fl_Image *oldim = image_box->image();
      if (oldim) delete oldim;
      image_box->image(im); // show the scaled image
      image_size->copy_label(title);
      value(image_box->parent());
      window()->redraw();
    }
    else { // text is being pasted
      display->buffer()->text(Fl::event_text());
      value(display);
      display->redraw();
    }
    return 1;
  }
} *tabs;


void cb(Fl_Widget *wid, clipboard_viewer *tabs)
{
  if (Fl::clipboard_contains(Fl::clipboard_image)) {
    Fl::paste(*tabs, 1, Fl::clipboard_image); // try to find image in the clipboard
    return;
  }
  if (Fl::clipboard_contains(Fl::clipboard_plain_text)) Fl::paste(*tabs, 1, Fl::clipboard_plain_text); // also try to find text
}

void clip_callback(int source, void *data) { // called after clipboard was changed or at application activation
  if ( source == 1 ) cb(NULL, (clipboard_viewer *)data);
}

int main(int argc, char **argv)
{
#if !(defined(__APPLE__) || defined(WIN32))
  extern void fl_register_images();
  fl_register_images(); // required to allow pasting of images
#endif
  Fl_Window* win = new Fl_Window(500, 550, "clipboard viewer");
  tabs = new clipboard_viewer(0, 0, 500, 500);
  Fl_Group *g = new Fl_Group( 5, 30, 490, 460, Fl::clipboard_image); // g will display the image form
  g->box(FL_FLAT_BOX);
  image_box = new chess(5, 30, 490, 450);
  image_size = new Fl_Box(FL_NO_BOX, 5, 485, 490, 10, 0);
  g->end();
  g->selection_color(TAB_COLOR);

  Fl_Text_Buffer *buffer = new Fl_Text_Buffer();
  display = new Fl_Text_Display(5,30,490, 460, Fl::clipboard_plain_text); // display will display the text form
  display->buffer(buffer);
  display->selection_color(TAB_COLOR);
  tabs->end();
  tabs->resizable(display);

  Fl_Group *g2 = new Fl_Group( 10,510,200,25);
  Fl_Button *refresh = new Fl_Button(10,510,200,25, "Refresh from clipboard");
  refresh->callback((Fl_Callback*)cb, tabs);
  g2->end();
  g2->resizable(NULL);
  win->end();
  win->resizable(tabs);
  win->show(argc,argv);
  clip_callback(1, tabs); // use clipboard content at start
  Fl::add_clipboard_notify(clip_callback, tabs); // will update with new clipboard content immediately or at application activation

  Fl_Image::RGB_scaling(FL_RGB_SCALING_BILINEAR); // set bilinear image scaling method
  return Fl::run();
}

//
// End of "$Id$".
//
