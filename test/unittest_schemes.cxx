//
// Unit tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
//
// Nods to Edmanuel Torres for the widget layout (STR#2672)
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

#include "unittests.h"

#include <FL/Fl_Scheme_Choice.H>

// needed by Edmanuel's test layout
#include <FL/Fl_Button.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Clock.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Adjuster.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_File_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Radio_Round_Button.H>

class Ut_Schemes_Test : public Fl_Group {
  Fl_Scheme_Choice *scheme_choice_;

  static void activate_subwin(Fl_Widget *w, void *data) {
    Fl_Window *win = (Fl_Window *)data;
    Fl_Light_Button *b = (Fl_Light_Button *)w;
    int active = b->value();
    if (active) b->label("active");
    else b->label("inactive");
    // Documentation of deactivate() states: "Currently you cannot deactivate Fl_Window widgets".
    // However, it seems to work in this case. AlbrechtS, FLTK 1.4, July 2022
#if (1)
  if (active) win->activate();
  else win->deactivate();
#else // alternative: deactivate all widgets (works definitely)
    for (int i = 0; i < win->children(); i++) {
      Fl_Widget *o = win->child(i);
      if (active) o->activate();
      else o->deactivate();
    }
#endif
  } // activate_subwin()

public:
  static Fl_Widget *create() {
    return new Ut_Schemes_Test(UT_TESTAREA_X, UT_TESTAREA_Y, UT_TESTAREA_W, UT_TESTAREA_H);
  }
  Ut_Schemes_Test(int X,int Y,int W,int H)
    : Fl_Group(X,Y,W,H) {

    scheme_choice_ = new Fl_Scheme_Choice(X+125,Y,140,25,"FLTK Scheme");
    scheme_choice_->labelfont(FL_HELVETICA_BOLD);

    Fl_Light_Button *active = new Fl_Light_Button(X + 300, Y, 100, 25, "active");
    active->value(1);
    active->selection_color(FL_RED);
    active->align(FL_ALIGN_CENTER);
    Fl_Window *subwin = new Fl_Window(X,Y+30,W,H-30);
    active->callback(activate_subwin, subwin);
    subwin->begin();
    {
      // Pasted from Edmanuel's gleam test app
      { Fl_Button* o = new Fl_Button(10, 9, 90, 25, "button");
        o->box(FL_UP_BOX);
        o->color((Fl_Color)101);
        o->tooltip("selection_color() = default");
        o->labelfont(5);
      } // Fl_Button* o
      { Fl_Button* o = new Fl_Button(10, 36, 90, 25, "button");
        o->box(FL_UP_BOX);
        o->color((Fl_Color)179);
        o->selection_color(o->color());
        o->tooltip("selection_color() = color()");
        o->labelfont(4);
        o->labelcolor(FL_BACKGROUND2_COLOR);
      } // Fl_Button* o
      { Fl_Button* o = new Fl_Button(10, 63, 90, 25, "button");
        o->box(FL_UP_BOX);
        o->color((Fl_Color)91);
        o->selection_color(fl_lighter(o->color()));
        o->tooltip("selection_color() = fl_lighter(color())");
      } // Fl_Button* o
      { Fl_Button* o = new Fl_Button(10, 90, 90, 25, "button");
        o->box(FL_UP_BOX);
        o->color(FL_INACTIVE_COLOR);
        o->selection_color(fl_darker(o->color()));
        o->tooltip("selection_color() = fl_darker(color())");
        o->labelcolor(FL_BACKGROUND2_COLOR);
      } // Fl_Button* o
      { Fl_Tabs* o = new Fl_Tabs(10, 120, 320, 215);
        o->color(FL_DARK1);
        o->selection_color(FL_DARK1);
        { Fl_Group* o = new Fl_Group(14, 141, 310, 190, "tab1");
          //o->box(FL_THIN_UP_BOX);
          o->color(FL_DARK1);
          o->selection_color((Fl_Color)23);
          o->hide();
          { Fl_Clock* o = new Fl_Clock(24, 166, 130, 130);
            o->box(FL_THIN_UP_BOX);
            o->color((Fl_Color)12);
            o->selection_color(FL_BACKGROUND2_COLOR);
            o->labelcolor(FL_BACKGROUND2_COLOR);
            o->tooltip("Fl_Clock with thin up box");
          } // Fl_Clock* o
          { new Fl_Progress(22, 306, 290, 20);
          } // Fl_Progress* o
          { Fl_Clock* o = new Fl_Clock(179, 166, 130, 130);
            o->box(FL_THIN_DOWN_BOX);
            o->color((Fl_Color)26);
            o->tooltip("Fl_Clock with thin down box");
          } // Fl_Clock* o
          o->end();
        } // Fl_Group* o
        { Fl_Group* o = new Fl_Group(15, 140, 310, 190, "tab2");
          //o->box(FL_THIN_UP_BOX);
          o->color(FL_DARK1);
          { Fl_Slider* o = new Fl_Slider(20, 161, 25, 155);
            o->box(FL_DOWN_BOX);
            o->tooltip("Fl_Slider with down box");
          } // Fl_Slider* o
          { Fl_Scrollbar* o = new Fl_Scrollbar(50, 161, 25, 155);
            o->value(0, 50, 1, 100);
            o->box(FL_DOWN_BOX);
            o->tooltip("Fl_Scrollbar with down box");
          } // Fl_Scrollbar* o
          { Fl_Value_Slider* o = new Fl_Value_Slider(115, 161, 25, 155);
            o->box(FL_DOWN_BOX);
          } // Fl_Value_Slider* o
          { Fl_Value_Output* o = new Fl_Value_Output(240, 265, 75, 25);
            o->box(FL_DOWN_BOX);
            o->tooltip("Fl_Value_Output with down box");
          } // Fl_Value_Output* o
          { Fl_Adjuster* o = new Fl_Adjuster(185, 210, 100, 25);
            o->tooltip("Fl_Adjuster");
          } // Fl_Adjuster* o
          { Fl_Counter* o = new Fl_Counter(185, 180, 100, 25);
            o->box(FL_DOWN_BOX);
            o->tooltip("Fl_Counter with down box");
          } // Fl_Counter* o
          { Fl_Roller* o = new Fl_Roller(85, 161, 25, 155);
            o->box(FL_UP_BOX);
            o->tooltip("Fl_Roller with up box");
          } // Fl_Roller* o
          { Fl_Value_Input* o = new Fl_Value_Input(155, 265, 75, 25);
            o->box(FL_DOWN_BOX);
            o->tooltip("Fl_Value_Input with down box");
          } // Fl_Value_Input* o
          o->end();
        } // Fl_Group* o
        { Fl_Group* o = new Fl_Group(15, 140, 310, 190, "tab3");
          //o->box(FL_THIN_UP_BOX);
          o->color(FL_DARK1);
          o->hide();
          { Fl_Input* o = new Fl_Input(40, 230, 120, 25);
            o->box(FL_DOWN_BOX);
            o->tooltip("Fl_Input with down box");
          } // Fl_Input* o
          { Fl_Output* o = new Fl_Output(40, 260, 120, 25);
            o->box(FL_DOWN_BOX);
            o->tooltip("Fl_Output with down box");
          } // Fl_Output* o
          { Fl_Text_Editor* o = new Fl_Text_Editor(180, 160, 125, 55);
            o->box(FL_DOWN_FRAME);
            o->color((Fl_Color)80);
            o->tooltip("Fl_Text_Editor with down frame");
            o->textsize(8);
            o->buffer(new Fl_Text_Buffer());
            o->buffer()->text("Text editor");
          } // Fl_Text_Editor* o
          { Fl_Text_Display* o = new Fl_Text_Display(180, 230, 125, 55);
            o->box(FL_DOWN_FRAME);
            o->color((Fl_Color)12);
            o->tooltip("Fl_Text_Display with down frame");
            o->textsize(8);
            o->buffer(new Fl_Text_Buffer());
            o->buffer()->text("Text display");
          } // Fl_Text_Display* o
          { Fl_File_Input* o = new Fl_File_Input(40, 290, 265, 30);
            o->box(FL_DOWN_BOX);
            o->tooltip("Fl_File_Input with down box");
          } // Fl_File_Input* o
          o->end();
        } // Fl_Group* o
        { Fl_Group* o = new Fl_Group(15, 140, 310, 190, "tab4");
          // o->box(FL_THIN_UP_BOX);
          o->color(FL_DARK1);
          o->hide();
          { Fl_Radio_Round_Button* o = new Fl_Radio_Round_Button(40, 160, 120, 25, "Choice 1");
            o->tooltip("Fl_Radio_Round_Button (default)");
            // don't set selection color (use default selection color)
            o->value(1); // selected
          } // Fl_Radio_Round_Button* o
          { Fl_Radio_Round_Button* o = new Fl_Radio_Round_Button(40, 190, 120, 25, "Choice 2");
            o->tooltip("Fl_Radio_Round_Button (red)");
            o->selection_color(FL_RED);
          } // Fl_Radio_Round_Button* o
          { Fl_Radio_Round_Button* o = new Fl_Radio_Round_Button(40, 220, 120, 25, "Choice 3");
            o->tooltip("Fl_Radio_Round_Button (green)");
            o->selection_color(fl_darker(FL_GREEN));
          } // Fl_Radio_Round_Button* o
          { Fl_Radio_Round_Button* o = new Fl_Radio_Round_Button(40, 250, 120, 25, "Choice 4");
            o->tooltip("Fl_Radio_Round_Button (blue)");
            o->selection_color(FL_BLUE);
          } // Fl_Radio_Round_Button* o
          o->end();
        } // Fl_Group* o
        o->end();
      } // Fl_Tabs* o
      { Fl_Box* o = new Fl_Box(341, 10, 80, 50, "thin box\ndown1");
        o->box(FL_THIN_DOWN_BOX);
        o->color((Fl_Color)20);
        o->labelsize(10);
      } // Fl_Box* o
      { Fl_Box* o = new Fl_Box(430, 10, 80, 50, "thin box\nup1");
        o->box(FL_THIN_UP_BOX);
        o->color(FL_SELECTION_COLOR);
        o->labelcolor((Fl_Color)6);
        o->labelsize(10);
      } // Fl_Box* o
      { Fl_Box* o = new Fl_Box(341, 71, 80, 44, "thin box\ndown2");
        o->box(FL_THIN_DOWN_BOX);
        o->color((Fl_Color)190);
        o->labelsize(10);
      } // Fl_Box* o
      { Fl_Box* o = new Fl_Box(430, 71, 80, 44, "thin box\nup2");
        o->box(FL_THIN_UP_BOX);
        o->color((Fl_Color)96);
        o->labelcolor(FL_BACKGROUND2_COLOR);
        o->labelsize(10);
      } // Fl_Box* o
      { Fl_Box* o = new Fl_Box(341, 127, 80, 50, "box down3");
        o->box(FL_DOWN_BOX);
        o->color((Fl_Color)3);
        o->labelsize(10);
      } // Fl_Box* o
      { Fl_Box* o = new Fl_Box(430, 127, 80, 50, "box up3");
        o->box(FL_UP_BOX);
        o->color((Fl_Color)104);
        o->labelcolor((Fl_Color)3);
        o->labelsize(10);
      } // Fl_Box* o
      { Fl_Box* o = new Fl_Box(341, 189, 80, 50, "box down4");
        o->box(FL_DOWN_BOX);
        o->color((Fl_Color)42);
        o->labelcolor(FL_DARK_RED);
        o->labelsize(10);
      } // Fl_Box* o
      { Fl_Box* o = new Fl_Box(430, 189, 80, 50, "box up4");
        o->box(FL_UP_BOX);
        o->color((Fl_Color)30);
        o->labelcolor((Fl_Color)26);
        o->labelsize(10);
      } // Fl_Box* o
      { Fl_Box* o = new Fl_Box(341, 251, 80, 82, "box down5");
        o->box(FL_DOWN_BOX);
        o->color((Fl_Color)19);
        o->labelcolor((Fl_Color)4);
        o->labelsize(10);
      } // Fl_Box* o
      { Fl_Box* o = new Fl_Box(430, 251, 80, 82, "box up5");
        o->box(FL_UP_BOX);
        o->color(FL_FOREGROUND_COLOR);
        o->labelcolor(FL_BACKGROUND2_COLOR);
        o->labelsize(10);
      } // Fl_Box* o
      { Fl_Light_Button* o = new Fl_Light_Button(110, 10, 105, 25, "Light");
        o->box(FL_DOWN_BOX);
        o->color(FL_BACKGROUND2_COLOR);
        o->selection_color((Fl_Color)30);
        o->selection_color((Fl_Color)FL_RED);
        o->tooltip("Fl_Light_Button with down box");
      } // Fl_Light_Button* o
      { Fl_Check_Button* o = new Fl_Check_Button(110, 37, 105, 25, "Check");
        o->box(FL_DOWN_FRAME);
        o->down_box(FL_DOWN_BOX);
        o->color(FL_DARK1);
        o->selection_color(fl_darker(FL_GREEN));
        o->tooltip("Fl_Check_Button with down frame");
      } // Fl_Check_Button* o
      { Fl_Input* o = new Fl_Input(220, 10, 100, 25);
        o->box(FL_DOWN_BOX);
        o->color((Fl_Color)23);
        o->tooltip("Fl_Input with down box");
      } // Fl_Input* o
      { Fl_Adjuster* o = new Fl_Adjuster(110, 65, 80, 43);
        o->box(FL_UP_BOX);
        o->color(FL_INACTIVE_COLOR);
        o->selection_color(FL_BACKGROUND2_COLOR);
        o->labelcolor((Fl_Color)55);
        o->tooltip("Fl_Adjuster with up box");
      } // Fl_Adjuster* o
      { Fl_Text_Editor* o = new Fl_Text_Editor(220, 40, 100, 25);
        o->box(FL_DOWN_FRAME);
        o->color((Fl_Color)19);
        o->selection_color(FL_DARK1);
        o->buffer(new Fl_Text_Buffer);
        o->tooltip("Fl_Text_Editor with down frame");
      } // Fl_Text_Editor* o
      { Fl_Text_Editor* o = new Fl_Text_Editor(220, 70, 100, 25);
        o->box(FL_UP_FRAME);
        o->color((Fl_Color)19);
        o->selection_color(FL_DARK1);
        o->buffer(new Fl_Text_Buffer);
        o->tooltip("Fl_Text_Editor with up frame");
      } // Fl_Text_Editor* o
    }
    subwin->end();
    subwin->resizable(subwin);
    subwin->show();
  }
};

UnitTest schemestest(UT_TEST_SCHEMES, "Schemes Test", Ut_Schemes_Test::create);
