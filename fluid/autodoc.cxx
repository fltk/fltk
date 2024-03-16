//
// Self-generate snapshots of user interface for FLUID documentation.
//
// Copyright 2024 by Bill Spitzak and others.
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

#ifndef NDEBUG

#include "autodoc.h"
#include "fluid.h"
#include "widget_browser.h"
#include "widget_panel.h"
#include "Fl_Widget_Type.h"
#include "function_panel.h"

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_PNG_Image.H>

/** \file autodoc.cxx

 \todo Implement a function to snapshot a window including decoration
    - see: void Fl_Widget_Surface::draw_decorated_window(Fl_Window *win, int win_offset_x, int win_offset_y)
    - see: void Fl_Widget_Surface::origin(int x, int y)
    - see: void Fl_Widget_Surface::draw(Fl_Widget* widget, int delta_x, int delta_y)
    - see: void Fl_Widget_Surface::print_window_part(Fl_Window *win, int x, int y, int w, int h, int delta_x, int delta_y)

 \todo Implement a version that snaps multipel windows in a desktop style situation.

 \todo a version that takes snapshots of a range of menu items

 \todo implement FL_SNAP_TO_GROUP, possibly with a number on how many groups up in the hierarchy
 */

/** \addtogroup fl_drawings
 @{
 */

static inline int fl_min(int a, int b) { return a < b ? a : b; }
static inline uchar fl_min(uchar a, uchar b) { return a < b ? a : b; }
static inline int fl_max(int a, int b) { return a > b ? a : b; }

/**
 Create a rect by providing a margin around a zero size rectangle.
 \param[in] dx, dy positive integers, move margin up and left
 \param[in] dr, db move margin to the right and down
 */
Fl_Margin::Fl_Margin(int dx, int dy, int dr, int db)
  : Fl_Rect(-dx, -dy, dx+dr, dy+db)
{
}

/**
 Convert an RGB image into an RGBA image.
 \param[inout] image pointer to an RGB image, deletes the RGB image, returns the RGBA image
 \return 0 if the image is now in RGBA format, or -1 if it can't be converted
 */
static int convert_RGB_to_RGBA(Fl_RGB_Image *&img) {
  if (img->d() == 4)
    return 0;
  if (img->d() != 3)
    return -1;

  // Copy pixel data from RGB to RGBA raw data
  int img_w = img->w();
  int img_h = img->h();
  uchar *data = new uchar[img_w * img_h * 4], *dst = data;
  int ld = img->ld(); if (ld == 0) ld = img_w * 3;
  int i, j;
  for (i=0; i<img_h; i++) {
    const uchar *src = (const uchar*)img->data()[0] + i * ld;
    for (j=0; j<img_w; j++) {
      *dst++ = *src++;
      *dst++ = *src++;
      *dst++ = *src++;
      *dst++ = 255;
    }
  }

  // Delete the old image
  delete img;
  // Create the new image
  img = new Fl_RGB_Image(data, img_w, img_h, 4);
  return 0;
}

/**
 Blend the left side lines of the alpha channel of an RBGA image to full transparency.
 \param[in] img must be an RGBA image
 \param[in] dx number of lines to blend
 */
void blend_alpha_left(const Fl_RGB_Image *img, int dx) {
  if (img->d() != 4)
    return;
  if (dx > img->w())
    return;
  if (dx > 0) {
    int max_x = dx, max_y = img->h();
    int ld = img->ld(); if (ld == 0) ld = img->w() * img->d();
    float a = 255/max_x;
    for (int i = 0; i < max_x; i++) {
      uchar *rgba = (uchar*)img->data()[0] + i * img->d();
      uchar alpha = static_cast<uchar>(i * a);
      for (int j = 0; j < max_y; j++) {
        rgba[3] = fl_min(alpha, rgba[3]);
        rgba += ld;
      }
    }
  }
}

/**
 Blend the top lines of the alpha channel of an RBGA image to full transparency.
 \param[in] img must be an RGBA image
 \param[in] dy number of lines to blend
 */
void blend_alpha_top(const Fl_RGB_Image *img, int dy) {
  if (img->d() != 4)
    return;
  if (dy  > img->h())
    return;
  if (dy > 0) {
    int max_x = img->w(), max_y = dy;
    int ld = img->ld(); if (ld == 0) ld = img->w() * img->d();
    float a = 255/max_y;
    for (int i = 0; i < max_y; i++) {
      uchar *rgba = (uchar*)img->data()[0] + i * ld;
      uchar alpha = static_cast<uchar>(i * a);
      for (int j = 0; j < max_x; j++) {
        rgba[3] = fl_min(alpha, rgba[3]);
        rgba += 4;
      }
    }
  }
}

/**
 Blend the right side lines of the alpha channel of an RBGA image to full transparency.
 \param[in] img must be an RGBA image
 \param[in] dx number of lines to blend
 */
void blend_alpha_right(const Fl_RGB_Image *img, int dx) {
  if (img->d() != 4)
    return;
  if (dx  > img->w())
    return;
  if (dx > 0) {
    int max_x = dx, max_y = img->h();
    int ld = img->ld(); if (ld == 0) ld = img->w() * img->d();
    float a = 255/max_x;
    for (int i = 0; i < max_x; i++) {
      uchar *rgba = (uchar*)img->data()[0] + (img->w()-i-1) * img->d();
      uchar alpha = static_cast<uchar>(i * a);
      for (int j = 0; j < max_y; j++) {
        rgba[3] = fl_min(alpha, rgba[3]);
        rgba += ld;
      }
    }
  }
}

/**
 Blend the bottom lines of the alpha channel of an RBGA image to full transparency.
 \param[in] img must be an RGBA image
 \param[in] dy number of lines to blend
 */
void blend_alpha_bottom(const Fl_RGB_Image *img, int dy) {
  if (img->d() != 4)
    return;
  if (dy  > img->h())
    return;
  if (dy > 0) {
    int max_x = img->w(), max_y = dy;
    int ld = img->ld(); if (ld == 0) ld = img->w() * img->d();
    float a = 255/max_y;
    for (int i = 0; i < max_y; i++) {
      uchar *rgba = (uchar*)img->data()[0] + (img->h()-i-1) * ld;
      uchar alpha = static_cast<uchar>(i * a);
      for (int j = 0; j < max_x; j++) {
        rgba[3] = fl_min(alpha, rgba[3]);
        rgba += 4;
      }
    }
  }
}

/**
 Take a snapshot of a number of widgets and save it as a png image.

 Draw a rectangular snapshot that fits around all widgets inside a window.
 All widgets must be inside the same window. It's up to the caller to ensure
 that widgets are visible. This includes children of `Fl_Tabs`.

 Outside labels of widgets are not taken into account, but a `frame` can be
 provided to grow the snapshot rectangle. Setting individual parameters of the
 frame to `FL_SNAP_TO_WINDOW` will extend the snapshot to the borders of the
 top level window.

 Another `blend` frame can be added around the image that fades to full
 transparency on selected sides.

 Use `Fl_Margin` to create `frame` and `blend` using positive integers to grow
 the rectangle to the left, top, right, and bottom.

 The image can be scaled after all processing. Note that snapshot is always
 created in FLTK resolution, even if the screen uses a higher resolution.

 \param[in] filename the snapshot will be written to this file in png format
 \param[in] w draw a bounding box around all widgets in the NULL terminated list
 \param[in] frame add a margin around the bounding box
 \param[in] blend add another margin around the bounding box that fades to full transparency
 \param[in] scale scale everything by this factor before saving it
 \return the result of fl_write_png or -3 if another error occurred
 */
int fl_snapshot(const char *filename, Fl_Widget **w,
                const Fl_Rect &frame,
                const Fl_Rect &blend,
                double scale)
{
  int i, min_x, min_y, max_x, max_y, bb_w, bb_h, img_w, img_h;

  // Get the bounding box for all widgets and make sure that all widgets are shown
  for (i=0; w[i]; i++) {
    int x, y;
    Fl_Widget *ww = w[i];
    ww->top_window_offset(x, y);
    if (i==0) {
      min_x = x; max_x = x + ww->w();
      min_y = y; max_y = y + ww->h();
    } else {
      min_x = fl_min(min_x, x); max_x = fl_max(max_x, x + ww->w());
      min_y = fl_min(min_y, y); max_y = fl_max(max_y, y + ww->h());
    }

    // this does not help us with Fl_Tab groups
    while (ww) { ww->show(); ww = ww->parent(); }
  }

  // Check for special values in frame and adjust bounding box
  Fl_Rect c_frame = frame;
  if (frame.x() == -FL_SNAP_TO_WINDOW) c_frame.x(-min_x);
  if (frame.y() == -FL_SNAP_TO_WINDOW) c_frame.y(-min_y);
  if (frame.r() == FL_SNAP_TO_WINDOW) c_frame.r(w[0]->top_window()->w()-max_x);
  if (frame.b() == FL_SNAP_TO_WINDOW) c_frame.b(w[0]->top_window()->h()-max_y);

  min_x += c_frame.x(); max_x += c_frame.r();
  min_y += c_frame.y(); max_y += c_frame.b();
  bb_w = max_x - min_x; bb_h = max_y - min_y;
  img_w = bb_w + blend.w();
  img_h = bb_h + blend.h();

  // Generate the Image Surface
  Fl_Image_Surface *srfc = new Fl_Image_Surface(img_w, img_h);
  Fl_Image_Surface::push_current(srfc);

  // Draw the background
  fl_rectf(0, 0, img_w, img_h, 0x1395bf00);

  // Draw the top level window
  srfc->draw(w[0]->top_window(), -blend.x()-min_x, -blend.y()-min_y);
  Fl_Image_Surface::pop_current();
  Fl_RGB_Image *img = srfc->image();

  // Do we want an alpha blended extension of the frame?
  if ((blend.x()<0 || blend.y()<0 || blend.r()>0 || blend.b()>0)) {
    if (convert_RGB_to_RGBA(img) == -1) {
      delete img;
      delete srfc;
      return -3;
    }
    if (blend.x() < 0) blend_alpha_left(img, -blend.x());
    if (blend.y() < 0) blend_alpha_top(img, -blend.y());
    if (blend.r() > 0) blend_alpha_right(img, blend.r());
    if (blend.b() > 0) blend_alpha_bottom(img, blend.b());
  }

  // If scale is set, scale the image
  if (scale != 1.0) {
    Fl_Image::scaling_algorithm(FL_RGB_SCALING_BILINEAR);
    Fl_RGB_Image *scaled_img = (Fl_RGB_Image*)img->copy(img->w()*scale, img->h()*scale);
    delete img;
    img = scaled_img;
  }

  // Write the image to disk
  int ret = fl_write_png(filename, img);

  // Clean up
  delete img;
  delete srfc;
  return ret;
}

/**
 Take a snapshot of the size of the bounding box around two widgets and save it as a png image.

 \param[in] filename the snapshot will be written to this file in png format
 \param[in] w1, w2 top left and bottom right widget
 \param[in] frame add a margin around the bounding box
 \param[in] blend add another margin around the bounding box that fades to full transparency
 \param[in] scale scale everything by this factor before saving it
 \return the result of fl_write_png or -3 if another error occurred

 \see fl_snapshot(const char*, Fl_Widget**, const Fl_Rect&, const Fl_Rect&, double)
 */
int fl_snapshot(const char *filename, Fl_Widget *w1, Fl_Widget *w2,
                const Fl_Rect &frame,
                const Fl_Rect &blend,
                double scale)
{
  Fl_Widget *ww[3] = { w1, w2, NULL };
  return fl_snapshot(filename, ww, frame, blend, scale);
}

/**
 Take a snapshot of a widget inside its window and save it as a png image.

 \param[in] filename the snapshot will be written to this file in png format
 \param[in] w snap this window, can also be a groups
 \param[in] frame add a margin around the bounding box
 \param[in] blend add another margin around the bounding box that fades to full transparency
 \param[in] scale scale everything by this factor before saving it
 \return the result of fl_write_png or -3 if another error occurred

 \see fl_snapshot(const char*, Fl_Widget**, const Fl_Rect&, const Fl_Rect&, double)
 */
int fl_snapshot(const char *filename, Fl_Widget *w,
                const Fl_Rect &frame,
                const Fl_Rect &blend,
                double scale)
{
  Fl_Widget *ww[2] = { w, NULL };
  return fl_snapshot(filename, ww, frame, blend, scale);
}

/** @} */


void run_autodoc(const Fl_String &target_dir) {
  Fl_Margin win_margin(0, 0, 0, 0);
  Fl_Margin win_blend(10, 10, 10, 10);

  printf("Writing documentation snapshots to %s\n", target_dir.c_str());

  // TODO: take a snapshot of FLUID in a desktop situation
  // (main, toolbar, document, widget editor, source view)

  // TODO: main window
  // explain titlebar
  // explain menubar?
  // explain widget browser
  // explain widget browser entry

  // TODO: toolbar
  // show grouping
  // explain non-widget types and where they will be located
  // explain widgets types an their dnd option
  // explain menu arrays
  // list exceptions (subwindow, scroll)

  // TODO: document view
  // explain dnd
  // explain selection, multiple selection, keyboard shortcuts
  // explain mouse functionality and alignment
  // explain live resize

  // TODO: source view
  // explain functionality
  // explain live update and choices
  // show various tabs
  // explain find and locate

  // ---- dialog types
  // list and show all non-widget types and their respective dialog boxes
  // - ID_Function
  Fl_Window *adoc_function_panel = make_function_panel();
  f_name_input->value("count_trees(const char *forest_name)");
  f_return_type_input->value("unsigned int");
  fl_snapshot((target_dir + "function_panel.png").c_str(), adoc_function_panel, win_margin, win_blend);
  // - ID_Code
  Fl_Window *adoc_code_panel = make_code_panel();
  code_input->buffer()->text("// increment user count\nif (new_user) {\n  user_count++;\n}\n");
  fl_snapshot((target_dir + "code_panel.png").c_str(), adoc_code_panel, win_margin, win_blend);
  // - ID_CodeBlock
  Fl_Window *adoc_codeblock_panel = make_codeblock_panel();
  code_before_input->value("if (test())");
  code_after_input->value("// test widgets added...");
  fl_snapshot((target_dir + "codeblock_panel.png").c_str(), adoc_codeblock_panel, win_margin, win_blend);
  // - ID_Decl
  Fl_Window *adoc_decl_panel = make_decl_panel();
  decl_class_choice->hide();
  decl_input->buffer()->text("const char *damage = \"'tis but a scratch\";");
  fl_snapshot((target_dir + "decl_panel.png").c_str(), adoc_decl_panel, win_margin, win_blend);
  // - ID_DeclBlock
  Fl_Window *adoc_declblock_panel = make_declblock_panel();
  decl_before_input->value("#ifdef NDEBUG");
  decl_after_input->value("#endif // NDEBUG");
  fl_snapshot((target_dir + "declblock_panel.png").c_str(), adoc_declblock_panel, win_margin, win_blend);
  // - ID_Class
  Fl_Window *adoc_class_panel = make_class_panel();
  decl_class_choice->hide();
  c_name_input->value("Zoo_Giraffe");
  c_subclass_input->value("Zoo_Animal");
  fl_snapshot((target_dir + "class_panel.png").c_str(), adoc_class_panel, win_margin, win_blend);
  // - ID_Widget_Class is handled like Fl_Window_Type
  // - ID_Comment
  Fl_Window *adoc_comment_panel = make_comment_panel();
  comment_input->buffer()->text("Make sure that the giraffe gets enough hay,\nbut the monkey can't reach it.");
  fl_snapshot((target_dir + "comment_panel.png").c_str(), adoc_comment_panel, win_margin, win_blend);
  // - ID_Data
  Fl_Window *adoc_data_panel = make_data_panel();
  data_class_choice->hide();
  data_input->value("emulated_ROM");
  data_filename->value("./ROM.bin");
  fl_snapshot((target_dir + "data_panel.png").c_str(), adoc_data_panel, win_margin, win_blend);

  // TODO: widget dialog
  // overview, multiple selection, instand feedback
  // individual standard tabs
  // list of special tabs for Grid

  // TODO: settings dialog
  // show and explain all tabs


  the_panel = make_widget_panel();
//  fl_snapshot("/Users/matt/test1.png", widget_browser, Fl_Rect(10, 20, 30, 40), Fl_Rect(40, 30, 20, 10));
//  fl_snapshot("/Users/matt/test2.png", the_panel, Fl_Rect(10, 10, 10, 10), Fl_Rect(20, 0, 0, 0));
//  fl_snapshot("/Users/matt/test.png", widget_x_input, widget_h_input, 70, 12, 10, 10, 20, 2.0);
//  fl_snapshot("/Users/matt/test.png", Main_Menu+1, Main_Menu+1, Main_Menu+4, 10, 10, 20, 2.0);
//  Fl_Menu_Item *mi1 = (Fl_Menu_Item*)main_menubar->find_item("&Layout/&Align");
//  Fl_Menu_Item *mi2 = (Fl_Menu_Item*)main_menubar->find_item("&Layout/Presets");
//  fl_snapshot("/Users/matt/test.png", mi1, mi1, mi2, 10, 10, 20, 2.0);
//  fl_snapshot("/Users/matt/test.png", New_Menu+1, New_Menu+1, New_Menu+4, 10, 10, 20, 2.0);


//  widget_tabs->value(widget_tab_grid_child);
//  fl_snapshot("/Users/matt/test1.png", the_panel, Fl_Rect(-1, -1, -1, -1), Fl_Rect(40, 40, 40, 40), 1.0f);
//  fl_snapshot("/Users/matt/test1.png", 
//              widget_x_input, widget_h_input,
//              Fl_Margin(0, 0, 0, 0),
//              Fl_Margin(10, 10, 10, 10));

//  fl_open_uri("file:///Users/matt/test1.png");
//  fl_open_uri("file:///Users/matt/test2.png");
  fl_open_uri(("file://" + target_dir + "data_panel.png").c_str());

}


#endif // NDEBUG
