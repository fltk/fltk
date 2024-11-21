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
#include "factory.h"
#include "widget_browser.h"
#include "widget_panel.h"
#include "Fl_Widget_Type.h"
#include "Fl_Window_Type.h"
#include "function_panel.h"
#include "settings_panel.h"
#include "codeview_panel.h"

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Menu_Bar.H>

extern Fl_Double_Window *settings_window;

/** \file autodoc.cxx

 \todo Implement a function to snapshot a window including decoration
    - see: void Fl_Widget_Surface::draw_decorated_window(Fl_Window *win, int win_offset_x, int win_offset_y)
    - see: void Fl_Widget_Surface::origin(int x, int y)
    - see: void Fl_Widget_Surface::draw(Fl_Widget* widget, int delta_x, int delta_y)
    - see: void Fl_Widget_Surface::print_window_part(Fl_Window *win, int x, int y, int w, int h, int delta_x, int delta_y)

 \todo Implement a version that snaps multiple windows in a desktop style situation.

 \todo a version that takes snapshots of a range of menu items

 \todo implement FL_SNAP_TO_GROUP, possibly with a number on how many groups up in the hierarchy
 */

/** \addtogroup fl_drawings
 @{
 */

const int FL_SNAP_TO_WINDOW = 0x01000000;

static Fl_Box snap_clear_(0, 0, 0, 0);
Fl_Widget *FL_SNAP_AREA_CLEAR = &snap_clear_;

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
  img->alloc_array = 1;
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
    float a = 255.0f/static_cast<float>(max_x);
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
    float a = 255.0f/static_cast<float>(max_y);
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
    float a = 255.0f/static_cast<float>(max_x);
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
    float a = 255.0f/static_cast<float>(max_y);
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
  int i, min_x = 0, min_y = 0, max_x = 0, max_y = 0, bb_w, bb_h, img_w, img_h;

  // Get the bounding box for all widgets and make sure that all widgets are shown
  for (i=0; w[i]; i++) {
    int x, y;
    Fl_Widget *ww = w[i];
    if (ww == FL_SNAP_AREA_CLEAR) {
      min_x = max_x = 0;
      min_y = max_y = 0;
    } else {
      ww->top_window_offset(x, y);
      if (i==0) {
        min_x = x; max_x = x + ww->w();
        min_y = y; max_y = y + ww->h();
      } else {
        min_x = fl_min(min_x, x); max_x = fl_max(max_x, x + ww->w());
        min_y = fl_min(min_y, y); max_y = fl_max(max_y, y + ww->h());
      }
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
    int scaled_img_w = static_cast<int>(img->w()*scale);
    int scaled_img_h = static_cast<int>(img->h()*scale);
    Fl_RGB_Image *scaled_img =
      static_cast<Fl_RGB_Image*>(img->copy(scaled_img_w, scaled_img_h));
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
  // A list of all the margins we will use later
  Fl_Margin win_margin(0, 0, 0, 0);
  Fl_Margin win_blend(10, 10, 10, 10);
  Fl_Margin tab_margin(FL_SNAP_TO_WINDOW, 32, FL_SNAP_TO_WINDOW, 4);
  Fl_Margin xtab_margin(FL_SNAP_TO_WINDOW, 50, FL_SNAP_TO_WINDOW, 4);
  Fl_Margin row_margin(FL_SNAP_TO_WINDOW, 4, FL_SNAP_TO_WINDOW, 4);
  Fl_Margin xrow_margin(FL_SNAP_TO_WINDOW, 14, FL_SNAP_TO_WINDOW, 4);
  Fl_Margin row_blend(0, 10, 0, 10);

//  Fl::scheme("gtk+");

  // Create a silly project that contains all widgets that we want to document
  new_project(false);

  /*Fl_Type *t_func = */ add_new_widget_from_user("Function", kAddAsLastChild, false);
  Fl_Window_Type *t_win = (Fl_Window_Type*)add_new_widget_from_user("Fl_Window", kAddAsLastChild, false);
  t_win->label("My Main Window");
  Fl_Widget_Type *t_grp = (Fl_Widget_Type*)add_new_widget_from_user("Fl_Group", kAddAsLastChild, false);
  t_grp->public_ = 0;
  Fl_Widget_Type *t_btn = (Fl_Widget_Type*)add_new_widget_from_user("Fl_Button", kAddAsLastChild, false);
  t_btn->comment("Don't press this button!");
  t_btn->name("emergency_btn");
  ((Fl_Button*)t_btn->o)->shortcut(FL_COMMAND|'g');
  Fl_Type *t_sldr = add_new_widget_from_user("Fl_Slider", kAddAsLastChild, false);
  Fl_Type *t_inp = add_new_widget_from_user("Fl_Input", kAddAsLastChild, false);
  Fl_Type *t_flx = add_new_widget_from_user("Fl_Flex", kAddAsLastChild, false);
  Fl_Type *t_flxc = add_new_widget_from_user("Fl_Button", kAddAsLastChild, false);
  select_only(t_grp);
  Fl_Type *t_grd = add_new_widget_from_user("Fl_Grid", kAddAsLastChild, false);
  Fl_Type *t_grdc = add_new_widget_from_user("Fl_Button", kAddAsLastChild, false);

  widget_browser->rebuild();
  g_project.update_settings_dialog();

  // TODO: FLUID overview

  // TODO: explain FLUID command line usage

  // TODO: take a snapshot of FLUID in a desktop situation
  // (main, toolbar, document, widget editor, code view)

  // ---- main window
  // explain titlebar
  // explain menubar?
  // explain widget browser
  // explain widget browser entry
  main_window->size(350, 320);
  fl_snapshot((target_dir + "main_window.png").c_str(), main_window, win_margin, win_blend);
  fl_snapshot((target_dir + "main_menubar.png").c_str(), main_menubar, row_margin, row_blend);
  fl_snapshot((target_dir + "main_browser.png").c_str(), widget_browser, FL_SNAP_AREA_CLEAR,
              Fl_Rect(0, 30, FL_SNAP_TO_WINDOW, 100), row_blend, 2.0);


  // TODO: document view
  // explain dnd
  // explain selection, multiple selection, keyboard shortcuts
  // explain mouse functionality and alignment
  // explain live resize
  // arrow: move by 1
  // shift: resize by one
  // Meta: move by Widget Gap
  // Shift Meta: resize by Widget Increment

  // ---- widget bin
  // show grouping
  // explain non-widget types and where they will be located
  // explain widgets types an their dnd option
  // explain menu arrays
  // list exceptions (subwindow, scroll)
  Fl::wait(0.2);
  Fl::flush();
  fl_snapshot((target_dir + "widgetbin_panel.png").c_str(), widgetbin_panel, win_margin, win_blend);

  // ---- code view
  // explain functionality
  // explain live update and choices
  // show various tabs
  // explain find and locate
  if (!codeview_panel) make_codeview();
  codeview_panel->show();
  Fl::wait(0.2);
  Fl::flush();
  update_codeview_cb(NULL, NULL); // must be visible on screen for this to work
  cv_tab->value(cv_source_tab);
  codeview_panel->redraw();
  Fl::flush();
  fl_snapshot((target_dir + "codeview_panel.png").c_str(), codeview_panel, win_margin, win_blend);
  fl_snapshot((target_dir + "cv_find_row.png").c_str(), cv_find_row, row_margin, row_blend);
  fl_snapshot((target_dir + "cv_settings_row.png").c_str(), cv_settings_row, row_margin, row_blend);

  // ---- settings dialog
  // show and explain all tabs
  fl_snapshot((target_dir + "w_settings.png").c_str(), settings_window, win_margin, win_blend);
  fl_snapshot((target_dir + "w_settings_general_tab.png").c_str(), w_settings_general_tab, xtab_margin, row_blend);
  w_settings_tabs->value(w_settings_project_tab);
  fl_snapshot((target_dir + "w_settings_project_tab.png").c_str(), w_settings_project_tab, xtab_margin, row_blend);
  w_settings_tabs->value(w_settings_layout_tab);
  fl_snapshot((target_dir + "w_settings_layout_tab.png").c_str(), w_settings_layout_tab, xtab_margin, row_blend);
  w_settings_tabs->value(w_settings_shell_tab);
  w_settings_shell_list->value(1);
  w_settings_shell_list->do_callback();
  fl_snapshot((target_dir + "w_settings_shell_tab.png").c_str(), w_settings_shell_tab, xtab_margin, row_blend);
  w_settings_tabs->value(w_settings_i18n_tab);
  i18n_type_chooser->value(1);
  i18n_type_chooser->do_callback();
  fl_snapshot((target_dir + "w_settings_i18n_gnu.png").c_str(), i18n_type_chooser, i18n_gnu_static_function_input, row_margin, row_blend);
  i18n_type_chooser->value(2);
  i18n_type_chooser->do_callback();
  fl_snapshot((target_dir + "w_settings_i18n_psx.png").c_str(), i18n_type_chooser, i18n_pos_set_input, row_margin, row_blend);
  w_settings_tabs->value(w_settings_user_tab);
  fl_snapshot((target_dir + "w_settings_user_tab.png").c_str(), w_settings_user_tab, xtab_margin, row_blend);


  // ---- dialog types
  // list and show all non-widget types and their respective dialog boxes

  // -- ID_Function
  Fl_Window *adoc_function_panel = make_function_panel();
  f_name_input->value("count_trees(const char *forest_name)");
  f_return_type_input->value("unsigned int");
  fl_snapshot((target_dir + "function_panel.png").c_str(), adoc_function_panel, win_margin, win_blend);
  adoc_function_panel->hide();

  // -- ID_Code
  Fl_Window *adoc_code_panel = make_code_panel();
  code_input->buffer()->text("// increment user count\nif (new_user) {\n  user_count++;\n}\n");
  fl_snapshot((target_dir + "code_panel.png").c_str(), adoc_code_panel, win_margin, win_blend);
  adoc_code_panel->hide();

  // -- ID_CodeBlock
  Fl_Window *adoc_codeblock_panel = make_codeblock_panel();
  code_before_input->value("if (test())");
  code_after_input->value("// test widgets added...");
  fl_snapshot((target_dir + "codeblock_panel.png").c_str(), adoc_codeblock_panel, win_margin, win_blend);
  adoc_codeblock_panel->hide();

  // -- ID_Decl
  Fl_Window *adoc_decl_panel = make_decl_panel();
  decl_class_choice->hide();
  decl_input->buffer()->text("const char *damage = \"'tis but a scratch\";");
  fl_snapshot((target_dir + "decl_panel.png").c_str(), adoc_decl_panel, win_margin, win_blend);
  adoc_decl_panel->hide();

  // -- ID_DeclBlock
  Fl_Window *adoc_declblock_panel = make_declblock_panel();
  declblock_before_input->value("#ifdef NDEBUG");
  declblock_after_input->value("#endif // NDEBUG");
  fl_snapshot((target_dir + "declblock_panel.png").c_str(), adoc_declblock_panel, win_margin, win_blend);
  adoc_declblock_panel->hide();

  // -- ID_Class
  Fl_Window *adoc_class_panel = make_class_panel();
  decl_class_choice->hide();
  c_name_input->value("Zoo_Giraffe");
  c_subclass_input->value("Zoo_Animal");
  fl_snapshot((target_dir + "class_panel.png").c_str(), adoc_class_panel, win_margin, win_blend);
  adoc_class_panel->hide();

  // -- ID_Widget_Class is handled like Fl_Window_Type

  // -- ID_Comment
  Fl_Window *adoc_comment_panel = make_comment_panel();
  comment_input->buffer()->text("Make sure that the giraffe gets enough hay,\nbut the monkey can't reach it.");
  fl_snapshot((target_dir + "comment_panel.png").c_str(), adoc_comment_panel, win_margin, win_blend);
  adoc_comment_panel->hide();

  // -- ID_Data
  Fl_Window *adoc_data_panel = make_data_panel();
  data_class_choice->hide();
  data_input->value("emulated_ROM");
  data_filename->value("./ROM.bin");
  fl_snapshot((target_dir + "data_panel.png").c_str(), adoc_data_panel, win_margin, win_blend);
  adoc_data_panel->hide();


  // ---- widget dialog
  t_win->open(); // open the window
  t_win->open(); // open the panel
  select_only(t_win);

  // -- snapshot of the widget properties panel
  fl_snapshot((target_dir + "widget_panel.png").c_str(), the_panel, win_margin, win_blend);
  fl_snapshot((target_dir + "wLiveMode.png").c_str(), wLiveMode, row_margin, row_blend);

  // -- snapshot of the GUI tab
  widget_tabs->value(wp_gui_tab);
  fl_snapshot((target_dir + "wp_gui_tab.png").c_str(), wp_gui_tab, tab_margin, row_blend);
  fl_snapshot((target_dir + "wp_gui_label.png").c_str(), wp_gui_label, row_margin, row_blend);
  select_only(t_btn);
  fl_snapshot((target_dir + "wp_gui_image.png").c_str(), widget_image_input, widget_deimage_input, row_margin, row_blend);
  fl_snapshot((target_dir + "wp_gui_alignment.png").c_str(), wp_gui_alignment, row_margin, row_blend);
  fl_snapshot((target_dir + "wp_gui_size.png").c_str(), widget_x_input, xrow_margin, row_blend);
  select_only(t_sldr);
  fl_snapshot((target_dir + "wp_gui_values.png").c_str(), wp_gui_values, xrow_margin, row_blend);
  select_only(t_flxc);
  fl_snapshot((target_dir + "wp_gui_flexp.png").c_str(), wp_gui_flexp, xrow_margin, row_blend);
  select_only(t_flx);
  fl_snapshot((target_dir + "wp_gui_margins.png").c_str(), wp_gui_margins, xrow_margin, row_blend);
  select_only(t_win);
  fl_snapshot((target_dir + "wp_gui_sizerange.png").c_str(), wp_gui_sizerange, xrow_margin, row_blend);
  select_only(t_btn);
  fl_snapshot((target_dir + "wp_gui_shortcut.png").c_str(), wp_gui_shortcut, row_margin, row_blend);
  select_only(t_win);
  fl_snapshot((target_dir + "wp_gui_xclass.png").c_str(), wp_gui_xclass, row_margin, row_blend);
  select_only(t_btn);
  fl_snapshot((target_dir + "wp_gui_attributes.png").c_str(), wp_gui_attributes, row_margin, row_blend);
  fl_snapshot((target_dir + "wp_gui_tooltip.png").c_str(), wp_gui_tooltip, row_margin, row_blend);

  // -- snapshot of the style tab
  widget_tabs->value(wp_style_tab);
  select_only(t_inp);
  fl_snapshot((target_dir + "wp_style_tab.png").c_str(), wp_style_tab, tab_margin, row_blend);
  fl_snapshot((target_dir + "wp_style_label.png").c_str(), wp_style_label, row_margin, row_blend);
  select_only(t_btn);
  fl_snapshot((target_dir + "wp_style_box.png").c_str(), wp_style_box, wp_style_downbox, row_margin, row_blend);
  select_only(t_inp);
  fl_snapshot((target_dir + "wp_style_text.png").c_str(), wp_style_text, row_margin, row_blend);

  // -- snapshot of the C++ tab
  widget_tabs->value(wp_cpp_tab);
  select_only(t_btn);
  fl_snapshot((target_dir + "wp_cpp_tab.png").c_str(), wp_cpp_tab, tab_margin, row_blend);
  fl_snapshot((target_dir + "wp_cpp_class.png").c_str(), wp_cpp_class, row_margin, row_blend);
  fl_snapshot((target_dir + "wp_cpp_name.png").c_str(), wp_cpp_name, row_margin, row_blend);
  fl_snapshot((target_dir + "v_input.png").c_str(), v_input[0], v_input[3], row_margin, row_blend);
  fl_snapshot((target_dir + "wComment.png").c_str(), wComment, row_margin, row_blend);
  fl_snapshot((target_dir + "wp_cpp_callback.png").c_str(), wCallback, w_when_box, row_margin, row_blend);

  // -- snapshot of the Grid tab
  select_only(t_grd);
  widget_tabs->value(widget_tab_grid);
  fl_snapshot((target_dir + "wp_grid_tab.png").c_str(), widget_tab_grid, tab_margin, row_blend);

  // -- snapshot of the Grid Child tab
  select_only(t_grdc);
  widget_tabs->value(widget_tab_grid_child);
  fl_snapshot((target_dir + "wp_gridc_tab.png").c_str(), widget_tab_grid_child, tab_margin, row_blend);
}


#endif // NDEBUG
