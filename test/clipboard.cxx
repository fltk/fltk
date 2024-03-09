//
// Clipboard display test application for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>

#include <stdio.h>

// optional: display extra technical info about clipboard content if defined
// #define DEBUG_CLIPBOARD_DATA

#if defined(_WIN32) && defined(DEBUG_CLIPBOARD_DATA)
#include <windows.h>
#endif // _WIN32 && DEBUG_CLIPBOARD_DATA

/* Displays and follows the content of the clipboard with either image or text data
 */

Fl_Box *image_box;
Fl_Box *image_size;
Fl_Text_Display *display;
Fl_RGB_Image *cl_img = 0; // image from clipboard

inline int fl_min(int a, int b) {
  return (a < b ? a : b);
}

// a box with a chess-like pattern below its image
class chess : public Fl_Box {
public:
  chess(int x, int y, int w, int h)
    : Fl_Box(FL_FLAT_BOX, x, y, w, h, 0) {
    align(FL_ALIGN_CENTER | FL_ALIGN_CLIP);
  }
  void draw() FL_OVERRIDE {
    draw_box();
    Fl_Image *img = image();
    if (img) { // draw the chess pattern below the box centered image
      int X = x() + (w() - img->w()) / 2;
      int Y = y() + (h() - img->h()) / 2;
      int W = img->w();
      int H = img->h();
      fl_push_clip(X, Y, W, H);
      fl_push_clip(x(), y(), w(), h());
      fl_color(FL_WHITE);
      fl_rectf(X, Y, W, H);
      fl_color(FL_LIGHT2);
      const int side = 4, side2 = 2 * side;
      for (int j = Y; j < Y + H; j += side) {
        for (int i = X + (j - Y) % side2; i < X + W; i += side2) {
          fl_rectf(i, j, side, side);
        }
      }
      fl_pop_clip();
      fl_pop_clip();
    }
    draw_label(); // draw the box image
  }
};

#define TAB_COLOR FL_DARK3

// use tabs to display either the image or textual content of the clipboard
class clipboard_viewer : public Fl_Tabs {
public:
  clipboard_viewer(int x, int y, int w, int h)
    : Fl_Tabs(x, y, w, h) {}
  int handle(int event) FL_OVERRIDE {
    if (event != FL_PASTE)
      return Fl_Tabs::handle(event);
    if (strcmp(Fl::event_clipboard_type(), Fl::clipboard_image) == 0) { // an image is being pasted
      cl_img = (Fl_RGB_Image *)Fl::event_clipboard();                   // get it as an Fl_RGB_Image object
      if (!cl_img)
        return 1;
      char title[300];
      snprintf(title, 300, "%dx%d", cl_img->w(), cl_img->h()); // display the image original size

      // optional: display extra technical info about clipboard content

#if defined(_WIN32) && defined(DEBUG_CLIPBOARD_DATA)

      OpenClipboard(NULL); //
      char *p = title + strlen(title);
      int format = EnumClipboardFormats(0);
      if (format && format < CF_MAX) {
        snprintf(p, sizeof(title) - strlen(title), " %d", format);
        p += strlen(p);
      }
      while (format) {
        format = EnumClipboardFormats(format);
        if (format && format < CF_MAX) {
          snprintf(p, sizeof(title) - strlen(title), " %d", format);
          p += strlen(p);
        }
      }
      HANDLE h;
      if ((h = GetClipboardData(CF_DIB))) {
        LPBITMAPINFO lpBI = (LPBITMAPINFO)GlobalLock(h);
        snprintf(p, sizeof(title) - strlen(title), " biBitCount=%d biCompression=%d biClrUsed=%d",
                lpBI->bmiHeader.biBitCount,
                (int)lpBI->bmiHeader.biCompression,
                (int)lpBI->bmiHeader.biClrUsed);
      }
      CloseClipboard();

#endif // _WIN32 && DEBUG_CLIPBOARD_DATA

      Fl_Image *oldimg = image_box->image();
      delete oldimg;
      if (cl_img->w() > image_box->w() || cl_img->h() > image_box->h())
        cl_img->scale(image_box->w(), image_box->h());
      image_box->image(cl_img); // show the scaled image
      image_size->copy_label(title);
      value(image_box->parent());
      window()->redraw();
    } else { // text is being pasted
      display->buffer()->text(Fl::event_text());
      value(display);
      display->redraw();
    }
    return 1;
  }
};

clipboard_viewer *tabs;

// clipboard viewer callback
void cb(Fl_Widget *wid, clipboard_viewer *tabs) {
  if (Fl::clipboard_contains(Fl::clipboard_image)) {
    Fl::paste(*tabs, 1, Fl::clipboard_image); // try to find image in the clipboard
    return;
  }
  if (Fl::clipboard_contains(Fl::clipboard_plain_text))
    Fl::paste(*tabs, 1, Fl::clipboard_plain_text); // also try to find text
}

// "Save PNG" callback
void save_cb(Fl_Widget *wid, clipboard_viewer *tabs) {
  if (cl_img && !cl_img->fail()) {
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Please select a .png file");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fnfc.filter("PNG\t*.png\n");
    fnfc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM | Fl_Native_File_Chooser::USE_FILTER_EXT);
    if (fnfc.show())
      return;
    const char *filename = fnfc.filename();
    if (filename)
      fl_write_png(filename, cl_img);
  } else {
    fl_message("%s", "No image available");
  }
}

// called after clipboard was changed or at application activation
void clip_callback(int source, void *data) {
  if (source == 1)
    cb(NULL, (clipboard_viewer *)data);
}

int main(int argc, char **argv) {
  fl_register_images(); // required for the X11 platform to allow pasting of images
  Fl_Window *win = new Fl_Window(500, 550, "FLTK Clipboard Viewer");
  tabs = new clipboard_viewer(0, 0, 500, 500);
  Fl_Group *g = new Fl_Group(5, 30, 490, 460, Fl::clipboard_image); // will display the image form
  g->box(FL_FLAT_BOX);
  image_box = new chess(5, 30, 490, 450);
  image_size = new Fl_Box(FL_NO_BOX, 5, 485, 490, 10, 0);
  g->end();
  g->selection_color(TAB_COLOR);

  Fl_Text_Buffer *buffer = new Fl_Text_Buffer();
  display = new Fl_Text_Display(5, 30, 490, 460, Fl::clipboard_plain_text); // will display the text form
  display->buffer(buffer);
  display->selection_color(TAB_COLOR);
  display->textfont(FL_COURIER);          // use fixed font for text display
  tabs->end();
  tabs->resizable(display);

  Fl_Group *g2 = new Fl_Group(10, 510, 330, 25);
  Fl_Button *refresh = new Fl_Button(10, 510, 200, 25, "Refresh from clipboard");
  refresh->callback((Fl_Callback *)cb, tabs);
  Fl_Button *save = new Fl_Button(220, 510, 100, 25, "Save PNG");
  save->callback((Fl_Callback *)save_cb, tabs);
  g2->end();
  g2->resizable(NULL);
  win->end();
  win->resizable(tabs);
  win->show(argc, argv);
  // TEST: set another default background color
#if (0)
  if (argc < 2) {
    Fl::set_color(FL_BACKGROUND_COLOR, 0xff, 0xee, 0xdd);
    Fl::set_color(FL_BACKGROUND2_COLOR, 0xdd, 0xee, 0xff);
  }
#endif
  clip_callback(1, tabs);                         // use clipboard content at start
  Fl::add_clipboard_notify(clip_callback, tabs);  // will update with new clipboard content
  Fl_Image::RGB_scaling(FL_RGB_SCALING_BILINEAR); // set bilinear image scaling method
  return Fl::run();
}
