//
// Another forms demo for the Fast Light Tool Kit (FLTK).
//
// This is an XForms program with some changes for FLTK.
//
// This demo show the different boxtypes. Note that some
// boxtypes are not appropriate for some objects
//
// Copyright 1998-2015 by Bill Spitzak and others.
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

#include <FL/forms.H> // changed for fltk

static int border = 1; // changed from FL_TRANSIENT for fltk
// (this is so the close box and Esc work to close the window)

typedef struct { Fl_Boxtype val; const char *name; } VN_struct;

static VN_struct btypes[]=
{
   {FL_NO_BOX,"no box"},
   {FL_UP_BOX, "up box"},
   {FL_DOWN_BOX,"down box"},
   {FL_BORDER_BOX,"border box"},
   {FL_SHADOW_BOX,"shadow box"},
   {FL_FLAT_BOX,"flat box"},
   {FL_FRAME_BOX,"frame box"},
   {FL_EMBOSSED_BOX,"embossed box"},
   {FL_ROUNDED_BOX,"rounded box"},
   {FL_RFLAT_BOX,"rflat box"},
   {FL_RSHADOW_BOX,"rshadow box"}, // renamed for fltk
   {FL_OVAL_BOX,"oval box"},
   {FL_ROUNDED3D_UPBOX,"rounded3d upbox"},
   {FL_ROUNDED3D_DOWNBOX,"rounded3d downbox"},
   {FL_OVAL3D_UPBOX,"oval3d upbox"},
   {FL_OVAL3D_DOWNBOX,"oval3d downbox"},
   {FL_PLASTIC_UP_BOX,"plastic upbox"},
   {FL_PLASTIC_DOWN_BOX,"plastic downbox"},
   {FL_GTK_UP_BOX,"GTK up box"},
   {FL_GTK_ROUND_UP_BOX,"GTK round up box"},
   {FL_GLEAM_UP_BOX,"Gleam up box"},
   /* sentinel */
   {(Fl_Boxtype)(-1)}
};

#include "pixmaps/sorceress.xbm"

/*************** Callback **********************/

FL_FORM *form;
Fl_Widget *tobj[18], *exitob, *btypeob, *modeob;

void
boxtype_cb (Fl_Widget * ob, long)
{
  int i, req_bt = fl_get_choice(ob) - 1;
  static int lastbt = -1;

  if(lastbt != req_bt)
  {
     fl_freeze_form (form);
     fl_redraw_form (form);
     for (i = 0; i < 18; i++)
        fl_set_object_boxtype (tobj[i], btypes[req_bt].val);
     fl_unfreeze_form (form);
     lastbt = req_bt;
     fl_redraw_form(form); // added for fltk
  }
}

void
mode_cb (Fl_Widget *, long) {
  // empty
}

/*************** Creation Routines *********************/

void
create_form_form (void)
{
  Fl_Widget *obj;

  form = fl_bgn_form(FL_NO_BOX, 720, 520);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 720, 520, "");
  fl_set_object_color(obj, FL_BLUE, FL_COL1);
  obj = fl_add_box(FL_DOWN_BOX, 10, 90, 700, 420, "");
  fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_box(FL_DOWN_BOX, 10, 10, 700, 70, "");
  fl_set_object_color(obj, FL_SLATEBLUE, FL_COL1);
  tobj[0] = obj = fl_add_box(FL_UP_BOX, 30, 110, 110, 110, "Box");
  tobj[1] = obj = fl_add_text(FL_NORMAL_TEXT, 30, 240, 110, 30, "Text");
  tobj[2] = obj = fl_add_bitmap(FL_NORMAL_BITMAP, 40, 280, 90, 80, "Bitmap");
  fl_set_object_lcol(obj, FL_BLUE);
  tobj[3] = obj = fl_add_chart(FL_BAR_CHART, 160, 110, 160, 110, "Chart");
  tobj[4] = obj = fl_add_clock(FL_ANALOG_CLOCK, 40, 390, 90, 90, "Clock");
//fl_set_object_dblbuffer(tobj[4],1); // removed for fltk
  tobj[5]=obj=fl_add_button(FL_NORMAL_BUTTON, 340, 110, 120, 30, "Button");
  tobj[6]=obj=fl_add_lightbutton(FL_PUSH_BUTTON,340,150,120,30,"Lightbutton");
  tobj[7]=obj=fl_add_roundbutton(FL_PUSH_BUTTON,340,190,120,30,"Roundbutton");
  tobj[8]=obj=fl_add_slider(FL_VERT_SLIDER, 160, 250, 40, 230, "Slider");
  tobj[9]=obj=fl_add_valslider(FL_VERT_SLIDER, 220, 250, 40, 230, "Valslider");
  tobj[10]=obj=fl_add_dial (FL_LINE_DIAL, 280, 250, 100, 100, "Dial");
  tobj[11]=obj=fl_add_positioner(FL_NORMAL_POSITIONER,280,380,150,100, "Positioner");
  tobj[12]=obj=fl_add_counter (FL_NORMAL_COUNTER,480,110,210,30, "Counter");
  tobj[13]=obj=fl_add_input (FL_NORMAL_INPUT, 520,170,170,30, "Input");
  tobj[14]=obj=fl_add_menu (FL_PUSH_MENU, 400, 240, 100, 30, "Menu");
  tobj[15]=obj=fl_add_choice (FL_NORMAL_CHOICE, 580, 250, 110, 30, "Choice");
  tobj[16]=obj=fl_add_timer (FL_VALUE_TIMER, 580, 210, 110, 30, "Timer");
//fl_set_object_dblbuffer(tobj[16], 1); // removed for fltk
  tobj[17]=obj=fl_add_browser (FL_NORMAL_BROWSER,450,300,240, 180, "Browser");
  exitob=obj= fl_add_button (FL_NORMAL_BUTTON, 590, 30, 100, 30, "Exit");
  btypeob=obj= fl_add_choice (FL_NORMAL_CHOICE,110,30, 130, 30, "Boxtype");
  fl_set_object_callback (obj, boxtype_cb, 0);
  modeob = obj=fl_add_choice(FL_NORMAL_CHOICE,370,30,130,30,"Graphics mode");
  fl_set_object_callback (obj, mode_cb, 0);
  fl_end_form ();
}
/*---------------------------------------*/

void
create_the_forms (void)
{
  create_form_form ();
}

/*************** Main Routine ***********************/

const char *browserlines[] = {
   " ", "@C1@c@l@bObjects Demo",   " ",
   "This demo shows you all",      "objects that currently",
   "exist in the Forms Library.",  " ",
   "You can change the boxtype",   "of the different objects",
   "using the buttons at the",     "top of the form. Note that",
   "some combinations might not",  "look too good. Also realize",
   "that for all object classes",  "many different types are",
   "available with different",     "behaviour.", " ",
   "With this demo you can also",  "see the effect of the drawing",
   "mode on the appearance of the","objects.",
   0
};


int
main (int argc, char *argv[])
{
  FL_COLOR c = FL_BLACK;
  const char **p;
  VN_struct *vn;

  fl_initialize(&argc, argv, "FormDemo", 0, 0);
  create_the_forms ();
  fl_set_bitmap_data (tobj[2], sorceress_width, sorceress_height, sorceress_bits);
  fl_add_chart_value (tobj[3], 15, "item 1", c++);
  fl_add_chart_value (tobj[3], 5, "item 2", c++);
  fl_add_chart_value (tobj[3], -10, "item 3", c++);
  fl_add_chart_value (tobj[3], 25, "item 4", c++);
  fl_set_menu (tobj[14], "item 1|item 2|item 3|item 4|item 5");
  fl_addto_choice (tobj[15], "item 1");
  fl_addto_choice (tobj[15], "item 2");
  fl_addto_choice (tobj[15], "item 3");
  fl_addto_choice (tobj[15], "item 4");
  fl_addto_choice (tobj[15], "item 5");
  fl_set_timer (tobj[16], 1000.0);

  for ( p = browserlines; *p; p++)
     fl_add_browser_line (tobj[17], *p);

  for ( vn = btypes; vn->val >= 0; vn++)
    fl_addto_choice(btypeob, vn->name);

  fl_show_form (form, FL_PLACE_MOUSE, border, "Box types");

  while (fl_do_forms () != exitob)
     ;

  return 0;
}
