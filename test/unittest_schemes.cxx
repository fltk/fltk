//
// "$Id$"
//
// Unit tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// Nods to Edmanuel Torres for the widget layout (STR#2672)
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

#include <FL/Fl_Choice.H>

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

class SchemesTest : public Fl_Group {
  Fl_Choice *schemechoice;
  static void SchemeChoice_CB(Fl_Widget*,void *data) {
    SchemesTest *st = (SchemesTest*)data;
    const char *name = st->schemechoice->text();
    if ( name ) {
      Fl::scheme(name);		// change scheme
      st->window()->redraw();	// redraw window
    }
  }
public:
  static Fl_Widget *create() {
    return new SchemesTest(TESTAREA_X, TESTAREA_Y, TESTAREA_W, TESTAREA_H);
  }
  SchemesTest(int X,int Y,int W,int H) : Fl_Group(X,Y,W,H) {
    schemechoice = new Fl_Choice(X+125,Y,140,25,"FLTK Scheme");
    schemechoice->add("none");
    schemechoice->add("plastic");
    schemechoice->add("gtk+");
    schemechoice->add("gleam");
    schemechoice->value(0);
    schemechoice->labelfont(FL_HELVETICA_BOLD);
    const char *name = Fl::scheme();
    if ( name ) {
           if ( strcmp(name, "plastic") == 0) { schemechoice->value(1); }
      else if ( strcmp(name, "gtk+")    == 0) { schemechoice->value(2); }
      else if ( strcmp(name, "gleam")   == 0) { schemechoice->value(3); }
    }
    schemechoice->callback(SchemeChoice_CB, (void*)this);

    Fl_Window *subwin = new Fl_Window(X,Y+30,W,H-30);
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
	  { Fl_Clock* o = new Fl_Clock(24, 166, 130, 124);
	    o->box(FL_THIN_UP_BOX);
	    o->color((Fl_Color)12);
	    o->selection_color(FL_BACKGROUND2_COLOR);
	    o->labelcolor(FL_BACKGROUND2_COLOR);
	  } // Fl_Clock* o
	  { new Fl_Progress(22, 306, 290, 20);
	  } // Fl_Progress* o
	  { Fl_Clock* o = new Fl_Clock(179, 166, 130, 130);
	    o->box(FL_THIN_DOWN_BOX);
	    o->color((Fl_Color)26);
	  } // Fl_Clock* o
	  o->end();
	} // Fl_Group* o
	{ Fl_Group* o = new Fl_Group(15, 140, 310, 190, "tab2");
	  //o->box(FL_THIN_UP_BOX);
	  o->color(FL_DARK1);
	  { Fl_Slider* o = new Fl_Slider(20, 161, 25, 155);
	    o->box(FL_DOWN_BOX);
	  } // Fl_Slider* o
	  { Fl_Scrollbar* o = new Fl_Scrollbar(50, 161, 25, 155);
	    o->box(FL_DOWN_BOX);
	  } // Fl_Scrollbar* o
	  { Fl_Value_Slider* o = new Fl_Value_Slider(115, 161, 25, 155);
	    o->box(FL_DOWN_BOX);
	  } // Fl_Value_Slider* o
	  { Fl_Value_Output* o = new Fl_Value_Output(240, 265, 75, 25);
	    o->box(FL_DOWN_BOX);
	  } // Fl_Value_Output* o
	  { new Fl_Adjuster(185, 210, 100, 25);
	  } // Fl_Adjuster* o
	  { Fl_Counter* o = new Fl_Counter(185, 180, 100, 25);
	    o->box(FL_DOWN_BOX);
	  } // Fl_Counter* o
	  { Fl_Roller* o = new Fl_Roller(85, 161, 25, 155);
	    o->box(FL_UP_BOX);
	  } // Fl_Roller* o
	  { Fl_Value_Input* o = new Fl_Value_Input(155, 265, 75, 25);
	    o->box(FL_DOWN_BOX);
	  } // Fl_Value_Input* o
	  o->end();
	} // Fl_Group* o
	{ Fl_Group* o = new Fl_Group(15, 140, 310, 190, "tab3");
	  //o->box(FL_THIN_UP_BOX);
	  o->color(FL_DARK1);
	  o->hide();
	  { Fl_Input* o = new Fl_Input(40, 230, 120, 25);
	    o->box(FL_DOWN_BOX);
	  } // Fl_Input* o
	  { Fl_Output* o = new Fl_Output(40, 260, 120, 25);
	    o->box(FL_DOWN_BOX);
	  } // Fl_Output* o
	  { Fl_Text_Editor* o = new Fl_Text_Editor(220, 160, 90, 55);
	    o->box(FL_DOWN_FRAME);
	    o->color((Fl_Color)80);
	  } // Fl_Text_Editor* o
	  { Fl_Text_Display* o = new Fl_Text_Display(220, 230, 90, 55);
	    o->box(FL_DOWN_FRAME);
	    o->color((Fl_Color)12);
	  } // Fl_Text_Display* o
	  { Fl_File_Input* o = new Fl_File_Input(40, 290, 265, 30);
	    o->box(FL_DOWN_BOX);
	  } // Fl_File_Input* o
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
      } // Fl_Light_Button* o
      { Fl_Check_Button* o = new Fl_Check_Button(110, 37, 105, 25, "Check");
	o->box(FL_DOWN_FRAME);
	o->down_box(FL_DOWN_BOX);
	o->color(FL_DARK1);
      } // Fl_Check_Button* o
      { Fl_Input* o = new Fl_Input(220, 10, 100, 25);
	o->box(FL_DOWN_BOX);
	o->color((Fl_Color)23);
      } // Fl_Input* o
      { Fl_Adjuster* o = new Fl_Adjuster(110, 65, 80, 43);
	o->box(FL_UP_BOX);
	o->color(FL_INACTIVE_COLOR);
	o->selection_color(FL_BACKGROUND2_COLOR);
	o->labelcolor((Fl_Color)55);
      } // Fl_Adjuster* o
      { Fl_Text_Editor* o = new Fl_Text_Editor(220, 53, 100, 29, "down frame");
	o->box(FL_DOWN_FRAME);
	o->color((Fl_Color)19);
	o->selection_color(FL_DARK1);
      } // Fl_Text_Editor* o
      { Fl_Text_Editor* o = new Fl_Text_Editor(220, 99, 100, 38, "up frame");
	o->box(FL_UP_FRAME);
	o->color((Fl_Color)19);
	o->selection_color(FL_DARK1);
      } // Fl_Text_Editor* o
    }
    subwin->end();
    subwin->resizable(subwin);
    subwin->show();
  }
};

UnitTest schemestest("schemes test", SchemesTest::create);

//
// End of "$Id$
//
