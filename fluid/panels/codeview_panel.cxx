//
// Code dialogs for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

// generated by Fast Light User Interface Designer (fluid) version 1.0500

#include "codeview_panel.h"
#include "Fluid.h"
#include "Project.h"
#include "io/Project_Reader.h"
#include "io/Project_Writer.h"
#include "io/String_Writer.h"
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Button.H>
#include "../src/flstring.h"
static char *cv_source_filename = nullptr;
static char *cv_header_filename = nullptr;
static char *cv_design_filename = nullptr;
int cv_code_choice;
extern void select_only(Fl_Type *o);
extern void reveal_in_browser(Fl_Type *t);

/**
 Update the header and source code highlighting depending on the
 currently selected object

 The Code View system offers an immediate preview of the code
 files that will be generated by FLUID. It also marks the code
 generated for the last selected item in the header and the source
 file.
*/
void update_codeview_position() {
  if (!codeview_panel || !codeview_panel->visible())
      return;
    if (cv_autoposition->value()==0)
      return;
    if (codeview_panel && codeview_panel->visible() && Fluid.proj.tree.current) {
      int pos0 = 0, pos1 = 0;
      if (cv_source->visible_r()) {
        switch (cv_code_choice) {
          case 0: // prolog: not yet (include statements)
            pos0 = Fluid.proj.tree.current->code1_start;
            pos1 = Fluid.proj.tree.current->code2_end;
            break;
          case 1: // static: callbacks, menu declarations
            pos0 = Fluid.proj.tree.current->code_static_start;
            pos1 = Fluid.proj.tree.current->code_static_end;
            break;
          case 2: // code: entire implementation block including children
            pos0 = Fluid.proj.tree.current->code1_start;
            pos1 = Fluid.proj.tree.current->code2_end;
            break;
          case 3: // code1: all implementation code before the children
            pos0 = Fluid.proj.tree.current->code1_start;
            pos1 = Fluid.proj.tree.current->code1_end;
            break;
          case 4: // code1: all implementation code before the children
            pos0 = Fluid.proj.tree.current->code2_start;
            pos1 = Fluid.proj.tree.current->code2_end;
            break;
        }
        if (pos0>=0) {
          if (pos1<pos0)
            pos1 = cv_source->buffer()->line_end(pos0);
          cv_source->buffer()->highlight(pos0, pos1);
          int line = cv_source->buffer()->count_lines(0, pos0);
          cv_source->scroll(line, 0);
        }
      }
      if (cv_header->visible_r()) {
        switch (cv_code_choice) {
          case 0: // prolog: not yet (include statements)
          case 1: // static: callbacks, menu declarations
            pos0 = Fluid.proj.tree.current->header_static_start;
            pos1 = Fluid.proj.tree.current->header_static_end;
            break;
          case 2: // code: entire implementation block including children
            pos0 = Fluid.proj.tree.current->header1_start;
            pos1 = Fluid.proj.tree.current->header2_end;
            break;
          case 3: // code1: all implementation code before the children
            pos0 = Fluid.proj.tree.current->header1_start;
            pos1 = Fluid.proj.tree.current->header1_end;
            break;
          case 4: // code1: all implementation code before the children
            pos0 = Fluid.proj.tree.current->header2_start;
            pos1 = Fluid.proj.tree.current->header2_end;
            break;
        }
        if (pos0>=0) {
          if (pos1<pos0)
            pos1 = cv_header->buffer()->line_end(pos0);
          cv_header->buffer()->highlight(pos0, pos1);
          int line = cv_header->buffer()->count_lines(0, pos0);
          cv_header->scroll(line, 0);
        }
      }
      if (cv_project->visible_r()) {
        switch (cv_code_choice) {
          case 0: // prolog: not yet (include statements)
          case 1: // static: callbacks, menu declarations
          case 2: // code: entire implementation block including children
            pos0 = Fluid.proj.tree.current->proj1_start;
            pos1 = Fluid.proj.tree.current->proj2_end;
            break;
          case 3: // code1: all implementation code before the children
            pos0 = Fluid.proj.tree.current->proj1_start;
            pos1 = Fluid.proj.tree.current->proj1_end;
            break;
          case 4: // code1: all implementation code before the children
            pos0 = Fluid.proj.tree.current->proj2_start;
            pos1 = Fluid.proj.tree.current->proj2_end;
            break;
        }
        if (pos0>=0) {
          if (pos1<pos0)
            pos1 = cv_project->buffer()->line_end(pos0);
          cv_project->buffer()->highlight(pos0, pos1);
          int line = cv_project->buffer()->count_lines(0, pos0);
          cv_project->scroll(line, 0);
        }
      }
    }
}

/**
 Callback to update the codeview position.
*/
void update_codeview_position_cb(class Fl_Tabs*, void*) {
  // make sure that the selected tab shows the current view
    update_codeview_cb(0,0);
    // highlight the selected widget in the selected tab
    update_codeview_position();
}

/**
 Generate a header, source, strings, or design file in a temporary directory
 and load those into the Code Viewer widgets.
*/
void update_codeview_cb(class Fl_Button*, void*) {
  if (!codeview_panel || !codeview_panel->visible())
      return;

    if (!cv_source_filename) {
      cv_source_filename = (char*)malloc(FL_PATH_MAX);
      fl_strlcpy(cv_source_filename, Fluid.get_tmpdir().c_str(), FL_PATH_MAX);
      fl_strlcat(cv_source_filename, "codeview_tmp.cxx", FL_PATH_MAX);
    }
    if (!cv_header_filename) {
      cv_header_filename = (char*)malloc(FL_PATH_MAX);
      fl_strlcpy(cv_header_filename, Fluid.get_tmpdir().c_str(), FL_PATH_MAX);
      fl_strlcat(cv_header_filename, "codeview_tmp.h", FL_PATH_MAX);
    }
    if (!cv_design_filename) {
      cv_design_filename = (char*)malloc(FL_PATH_MAX);
      fl_strlcpy(cv_design_filename, Fluid.get_tmpdir().c_str(), FL_PATH_MAX);
      fl_strlcat(cv_design_filename, "codeview_tmp.fl", FL_PATH_MAX);
    }

    if (cv_project->visible_r()) {
      fld::io::write_file(Fluid.proj, cv_design_filename, false, true);
      int top = cv_project->top_line();
      cv_project->buffer()->loadfile(cv_design_filename);
      cv_project->scroll(top, 0);
    } else if (cv_strings->visible_r()) {
      static const char *exts[] = { ".txt", ".po", ".msg" };
      char fn[FL_PATH_MAX+1];
      fl_strlcpy(fn, Fluid.get_tmpdir().c_str(), FL_PATH_MAX);
      fl_strlcat(fn, "strings", FL_PATH_MAX);
      fl_filename_setext(fn, FL_PATH_MAX, exts[static_cast<int>(Fluid.proj.i18n_type)]);
      fld::io::write_strings(Fluid.proj, fn);
      int top = cv_strings->top_line();
      cv_strings->buffer()->loadfile(fn);
      cv_strings->scroll(top, 0);
    } else if (cv_source->visible_r() || cv_header->visible_r()) {
      std::string code_file_name_bak = Fluid.proj.code_file_name;
      Fluid.proj.code_file_name = cv_source_filename;
      std::string header_file_name_bak = Fluid.proj.header_file_name;
      Fluid.proj.header_file_name = cv_header_filename;

      // generate the code and load the files
      fld::io::Code_Writer f(Fluid.proj);
      // generate files
      if (f.write_code(cv_source_filename, cv_header_filename, true))
      {
        // load file into source editor
        int pos = cv_source->top_line();
        cv_source->buffer()->loadfile(cv_source_filename);
        cv_source->scroll(pos, 0);
        // load file into header editor
        pos = cv_header->top_line();
        cv_header->buffer()->loadfile(cv_header_filename);
        cv_header->scroll(pos, 0);
        // update the source code highlighting
        update_codeview_position();
      }

      Fluid.proj.code_file_name = code_file_name_bak;
      Fluid.proj.header_file_name = header_file_name_bak;
    }
}

/**
 This is called by the timer itself

*/
void update_codeview_timer(void*) {
  update_codeview_cb(0,0);
}

void codeview_defer_update() {
  // we will only update earliest 0.5 seconds after the last change, and only
      // if no other change was made, so dragging a widget will not generate any
      // CPU load
      Fl::remove_timeout(update_codeview_timer, 0);
      Fl::add_timeout(0.5, update_codeview_timer, 0);
}

/**
 Show or hide the source code preview.
 The state is stored in the app preferences.

*/
void codeview_toggle_visibility() {
  if (!codeview_panel) {
      make_codeview();
      codeview_panel->callback((Fl_Callback*)toggle_codeview_cb);
      Fl_Preferences svp(Fluid.preferences, "codeview");
      int autorefresh;
      svp.get("autorefresh", autorefresh, 1);
      cv_autorefresh->value(autorefresh);
      int autoposition;
      svp.get("autoposition", autoposition, 1);
      cv_autoposition->value(autoposition);
      int tab;
      svp.get("tab", tab, 0);
      if (tab>=0 && tab<cv_tab->children()) cv_tab->value(cv_tab->child(tab));
      svp.get("code_choice", cv_code_choice, 2);
      cv_code_choice_w->value(cv_code_choice_w->find_item_with_argument(cv_code_choice));
      if (!Fluid.position_window(codeview_panel,"codeview_pos", 0, 320, 120, 550, 500)) return;
    }

    if (codeview_panel->visible()) {
      codeview_panel->hide();
      Fluid.codeview_item->label("Show Code View");
    } else {
      codeview_panel->show();
      Fluid.codeview_item->label("Hide Code View");
      update_codeview_cb(0,0);
    }
}

Fl_Double_Window *codeview_panel=(Fl_Double_Window *)0;

Fl_Tabs *cv_tab=(Fl_Tabs *)0;

Fl_Group *cv_source_tab=(Fl_Group *)0;

fld::widget::Code_Viewer *cv_source=(fld::widget::Code_Viewer *)0;

fld::widget::Code_Viewer *cv_header=(fld::widget::Code_Viewer *)0;

fld::widget::Text_Viewer *cv_strings=(fld::widget::Text_Viewer *)0;

fld::widget::Text_Viewer *cv_project=(fld::widget::Text_Viewer *)0;

Fl_Group *cv_find_row=(Fl_Group *)0;

Fl_Button *cv_find_text_case=(Fl_Button *)0;

Fl_Input *cv_find_text=(Fl_Input *)0;

static void cb_cv_find_text(Fl_Input* o, void*) {
  Fl_Text_Display *e = nullptr;
  if (cv_source->visible_r()) {
    e = cv_source;
  } else if (cv_header->visible_r()) {
    e = cv_header;
  } else if (cv_project->visible_r()) {
    e = cv_project;
  }
  if (e) {
    Fl_Text_Buffer *b = e->buffer();
    int pos = e->insert_position();
    int found = b->search_forward(pos, o->value(), &pos, cv_find_text_case->value());
    if (found) {
      b->select(pos, pos + (int)strlen(o->value()));
      e->insert_position(pos);
      e->show_insert_position();
    }
  }
}

static void cb_(Fl_Button*, void*) {
  Fl_Text_Display *e = nullptr;
  if (cv_source->visible_r()) {
    e = cv_source;
  } else if (cv_header->visible_r()) {
    e = cv_header;
  } else if (cv_project->visible_r()) {
    e = cv_project;
  }
  if (e) {
    const char *needle = cv_find_text->value();
    Fl_Text_Buffer *b = e->buffer();
    int pos = e->insert_position()-1;
    if (pos < 0) pos = b->length()-1;
    int found = b->search_backward(pos, needle, &pos, cv_find_text_case->value());
    if (!found)
      found = b->search_backward(b->length()-1, needle, &pos, cv_find_text_case->value());
    if (found) {
      b->select(pos, pos + (int)strlen(needle));
      e->insert_position(pos);
      e->show_insert_position();
    }
  }
}

static void cb_1(Fl_Button*, void*) {
  Fl_Text_Display *e = nullptr;
  if (cv_source->visible_r()) {
    e = cv_source;
  } else if (cv_header->visible_r()) {
    e = cv_header;
  } else if (cv_project->visible_r()) {
    e = cv_project;
  }
  if (e) {
    const char *needle = cv_find_text->value();
    Fl_Text_Buffer *b = e->buffer();
    int pos = e->insert_position() + 1;
    if (pos+1 >= b->length()) pos = 0;
    int found = b->search_forward(pos, needle, &pos, cv_find_text_case->value());
    if (!found && (pos > 0))
      found = b->search_forward(0, needle, &pos, cv_find_text_case->value());
    if (found) {
      b->select(pos, pos + (int)strlen(needle));
      e->insert_position(pos);
      e->show_insert_position();
    }
  }
}

static void cb_Reveal(Fl_Button*, void*) {
  if (codeview_panel && codeview_panel->visible()) {
    Fl_Type *node = nullptr;
    if (cv_source->visible_r())
      node = Fluid.proj.tree.find_in_text(0, cv_source->insert_position());
    else if (cv_header->visible_r())
      node = Fluid.proj.tree.find_in_text(1, cv_header->insert_position());
    else if (cv_project->visible_r())
      node = Fluid.proj.tree.find_in_text(2, cv_project->insert_position());
    if (node) {
      select_only(node);
      reveal_in_browser(node);
      if (Fl::event_clicks()==1) // double click
        node->open();
    }
  }
}

Fl_Group *cv_settings_row=(Fl_Group *)0;

Fl_Light_Button *cv_autorefresh=(Fl_Light_Button *)0;

Fl_Light_Button *cv_autoposition=(Fl_Light_Button *)0;

Fl_Choice *cv_code_choice_w=(Fl_Choice *)0;

static void cb_cv_code_choice_w(Fl_Choice* o, void*) {
  cv_code_choice = (int)o->mvalue()->argument();
  update_codeview_position();
}

Fl_Menu_Item menu_cv_code_choice_w[] = {
 {"prolog", 0,  0, (void*)(0), 16, (uchar)FL_NORMAL_LABEL, 0, 11, 0},
 {"static", 0,  0, (void*)(1), 0, (uchar)FL_NORMAL_LABEL, 0, 11, 0},
 {"code", 0,  0, (void*)(2), 0, (uchar)FL_NORMAL_LABEL, 0, 11, 0},
 {"code 1", 0,  0, (void*)(3), 0, (uchar)FL_NORMAL_LABEL, 0, 11, 0},
 {"code 2", 0,  0, (void*)(4), 0, (uchar)FL_NORMAL_LABEL, 0, 11, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_Double_Window* make_codeview() {
  { codeview_panel = new Fl_Double_Window(520, 515, "Code View");
    codeview_panel->callback((Fl_Callback*)toggle_codeview_cb);
    codeview_panel->align(Fl_Align(FL_ALIGN_CLIP|FL_ALIGN_INSIDE));
    { cv_tab = new Fl_Tabs(10, 10, 500, 440);
      cv_tab->selection_color((Fl_Color)4);
      cv_tab->labelcolor(FL_BACKGROUND2_COLOR);
      cv_tab->callback((Fl_Callback*)update_codeview_position_cb);
      { cv_source_tab = new Fl_Group(10, 35, 500, 415, "Source");
        cv_source_tab->labelsize(13);
        { fld::widget::Code_Viewer* o = cv_source = new fld::widget::Code_Viewer(10, 40, 500, 410);
          cv_source->box(FL_DOWN_FRAME);
          cv_source->color(FL_BACKGROUND2_COLOR);
          cv_source->selection_color(FL_SELECTION_COLOR);
          cv_source->labeltype(FL_NORMAL_LABEL);
          cv_source->labelfont(0);
          cv_source->labelsize(14);
          cv_source->labelcolor(FL_FOREGROUND_COLOR);
          cv_source->textfont(4);
          cv_source->textsize(11);
          cv_source->align(Fl_Align(FL_ALIGN_TOP));
          cv_source->when(FL_WHEN_RELEASE);
          Fl_Group::current()->resizable(cv_source);
          o->linenumber_width(60);
          o->linenumber_size(o->Fl_Text_Display::textsize());
        } // fld::widget::Code_Viewer* cv_source
        cv_source_tab->end();
        Fl_Group::current()->resizable(cv_source_tab);
      } // Fl_Group* cv_source_tab
      { Fl_Group* o = new Fl_Group(10, 35, 500, 415, "Header");
        o->labelsize(13);
        o->hide();
        { fld::widget::Code_Viewer* o = cv_header = new fld::widget::Code_Viewer(10, 40, 500, 410);
          cv_header->box(FL_DOWN_FRAME);
          cv_header->color(FL_BACKGROUND2_COLOR);
          cv_header->selection_color(FL_SELECTION_COLOR);
          cv_header->labeltype(FL_NORMAL_LABEL);
          cv_header->labelfont(0);
          cv_header->labelsize(14);
          cv_header->labelcolor(FL_FOREGROUND_COLOR);
          cv_header->textfont(4);
          cv_header->textsize(11);
          cv_header->align(Fl_Align(FL_ALIGN_TOP));
          cv_header->when(FL_WHEN_RELEASE);
          Fl_Group::current()->resizable(cv_header);
          o->linenumber_width(60);
          o->linenumber_size(o->Fl_Text_Display::textsize());
        } // fld::widget::Code_Viewer* cv_header
        o->end();
      } // Fl_Group* o
      { Fl_Group* o = new Fl_Group(10, 35, 500, 415, "Strings");
        o->labelsize(13);
        o->hide();
        { fld::widget::Text_Viewer* o = cv_strings = new fld::widget::Text_Viewer(10, 40, 500, 410);
          cv_strings->box(FL_DOWN_FRAME);
          cv_strings->color(FL_BACKGROUND2_COLOR);
          cv_strings->selection_color(FL_SELECTION_COLOR);
          cv_strings->labeltype(FL_NORMAL_LABEL);
          cv_strings->labelfont(0);
          cv_strings->labelsize(14);
          cv_strings->labelcolor(FL_FOREGROUND_COLOR);
          cv_strings->textfont(4);
          cv_strings->textsize(11);
          cv_strings->align(Fl_Align(FL_ALIGN_TOP));
          cv_strings->when(FL_WHEN_RELEASE);
          Fl_Group::current()->resizable(cv_strings);
          o->linenumber_width(60);
          o->linenumber_size(o->Fl_Text_Display::textsize());
        } // fld::widget::Text_Viewer* cv_strings
        o->end();
      } // Fl_Group* o
      { Fl_Group* o = new Fl_Group(10, 35, 500, 415, "Project");
        o->labelsize(13);
        o->hide();
        { fld::widget::Text_Viewer* o = cv_project = new fld::widget::Text_Viewer(10, 40, 500, 410);
          cv_project->box(FL_DOWN_FRAME);
          cv_project->color(FL_BACKGROUND2_COLOR);
          cv_project->selection_color(FL_SELECTION_COLOR);
          cv_project->labeltype(FL_NORMAL_LABEL);
          cv_project->labelfont(0);
          cv_project->labelsize(14);
          cv_project->labelcolor(FL_FOREGROUND_COLOR);
          cv_project->textfont(4);
          cv_project->textsize(11);
          cv_project->align(Fl_Align(FL_ALIGN_TOP));
          cv_project->when(FL_WHEN_RELEASE);
          Fl_Group::current()->resizable(cv_project);
          o->linenumber_width(60);
          o->linenumber_size(o->Fl_Text_Display::textsize());
        } // fld::widget::Text_Viewer* cv_project
        o->end();
      } // Fl_Group* o
      cv_tab->end();
      Fl_Group::current()->resizable(cv_tab);
    } // Fl_Tabs* cv_tab
    { cv_find_row = new Fl_Group(10, 460, 500, 20);
      { cv_find_text_case = new Fl_Button(244, 460, 25, 20, "aA");
        cv_find_text_case->type(1);
        cv_find_text_case->labelsize(11);
      } // Fl_Button* cv_find_text_case
      { cv_find_text = new Fl_Input(40, 460, 200, 20, "Find:");
        cv_find_text->labelsize(11);
        cv_find_text->textsize(11);
        cv_find_text->callback((Fl_Callback*)cb_cv_find_text);
        cv_find_text->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY_CHANGED);
      } // Fl_Input* cv_find_text
      { Fl_Button* o = new Fl_Button(273, 460, 25, 20, "<<");
        o->labelsize(11);
        o->callback((Fl_Callback*)cb_);
      } // Fl_Button* o
      { Fl_Button* o = new Fl_Button(298, 460, 25, 20, ">>");
        o->labelsize(11);
        o->callback((Fl_Callback*)cb_1);
      } // Fl_Button* o
      { Fl_Button* o = new Fl_Button(327, 460, 61, 20, "Reveal");
        o->labelsize(11);
        o->callback((Fl_Callback*)cb_Reveal);
      } // Fl_Button* o
      { Fl_Box* o = new Fl_Box(490, 460, 20, 20);
        Fl_Group::current()->resizable(o);
      } // Fl_Box* o
      cv_find_row->end();
    } // Fl_Group* cv_find_row
    { cv_settings_row = new Fl_Group(10, 485, 500, 20);
      { Fl_Button* o = new Fl_Button(10, 485, 61, 20, "Refresh");
        o->labelsize(11);
        o->callback((Fl_Callback*)update_codeview_cb);
      } // Fl_Button* o
      { Fl_Light_Button* o = cv_autorefresh = new Fl_Light_Button(77, 485, 91, 20, "Auto-Refresh");
        cv_autorefresh->labelsize(11);
        o->callback((Fl_Callback*)update_codeview_cb);
      } // Fl_Light_Button* cv_autorefresh
      { cv_autoposition = new Fl_Light_Button(172, 485, 89, 20, "Auto-Position");
        cv_autoposition->labelsize(11);
      } // Fl_Light_Button* cv_autoposition
      { cv_code_choice_w = new Fl_Choice(265, 485, 70, 20);
        cv_code_choice_w->down_box(FL_BORDER_BOX);
        cv_code_choice_w->labelsize(11);
        cv_code_choice_w->textsize(11);
        cv_code_choice_w->callback((Fl_Callback*)cb_cv_code_choice_w);
        cv_code_choice_w->menu(menu_cv_code_choice_w);
      } // Fl_Choice* cv_code_choice_w
      { Fl_Box* o = new Fl_Box(375, 485, 80, 20);
        Fl_Group::current()->resizable(o);
      } // Fl_Box* o
      { Fl_Button* o = new Fl_Button(460, 485, 50, 20, "Close");
        o->labelsize(11);
        o->callback((Fl_Callback*)toggle_codeview_b_cb);
      } // Fl_Button* o
      cv_settings_row->end();
    } // Fl_Group* cv_settings_row
    codeview_panel->size_range(384, 120);
    codeview_panel->end();
  } // Fl_Double_Window* codeview_panel
  return codeview_panel;
}

//
