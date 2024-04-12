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

// generated by Fast Light User Interface Designer (fluid) version 1.0400

#include "sourceview_panel.h"
#include "fluid.h"
#include "file.h"
#include "../src/flstring.h"
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Button.H>
static char *sv_source_filename = NULL;
static char *sv_header_filename = NULL;
static char *sv_design_filename = NULL;
int sv_code_choice;
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
void update_sourceview_position() {
  if (!sourceview_panel || !sourceview_panel->visible())
      return;
    if (sv_autoposition->value()==0)
      return;
    if (sourceview_panel && sourceview_panel->visible() && Fl_Type::current) {
      int pos0 = 0, pos1 = 0;
      if (sv_source->visible_r()) {
        switch (sv_code_choice) {
          case 0: // prolog: not yet (include statements)
            pos0 = Fl_Type::current->code1_start;
            pos1 = Fl_Type::current->code2_end;
            break;
          case 1: // static: callbacks, menu declarations
            pos0 = Fl_Type::current->code_static_start;
            pos1 = Fl_Type::current->code_static_end;
            break;
          case 2: // code: entire implementation block including children
            pos0 = Fl_Type::current->code1_start;
            pos1 = Fl_Type::current->code2_end;
            break;
          case 3: // code1: all implementation code before the children
            pos0 = Fl_Type::current->code1_start;
            pos1 = Fl_Type::current->code1_end;
            break;
          case 4: // code1: all implementation code before the children
            pos0 = Fl_Type::current->code2_start;
            pos1 = Fl_Type::current->code2_end;
            break;
        }
        if (pos0>=0) {
          if (pos1<pos0)
            pos1 = sv_source->buffer()->line_end(pos0);
          sv_source->buffer()->highlight(pos0, pos1);
          int line = sv_source->buffer()->count_lines(0, pos0);
          sv_source->scroll(line, 0);
        }
      }
      if (sv_header->visible_r()) {
        switch (sv_code_choice) {
          case 0: // prolog: not yet (include statements)
          case 1: // static: callbacks, menu declarations
            pos0 = Fl_Type::current->header_static_start;
            pos1 = Fl_Type::current->header_static_end;
            break;
          case 2: // code: entire implementation block including children
            pos0 = Fl_Type::current->header1_start;
            pos1 = Fl_Type::current->header2_end;
            break;
          case 3: // code1: all implementation code before the children
            pos0 = Fl_Type::current->header1_start;
            pos1 = Fl_Type::current->header1_end;
            break;
          case 4: // code1: all implementation code before the children
            pos0 = Fl_Type::current->header2_start;
            pos1 = Fl_Type::current->header2_end;
            break;
        }
        if (pos0>=0) {
          if (pos1<pos0)
            pos1 = sv_header->buffer()->line_end(pos0);
          sv_header->buffer()->highlight(pos0, pos1);
          int line = sv_header->buffer()->count_lines(0, pos0);
          sv_header->scroll(line, 0);
        }
      }
      if (sv_project->visible_r()) {
        switch (sv_code_choice) {
          case 0: // prolog: not yet (include statements)
          case 1: // static: callbacks, menu declarations
          case 2: // code: entire implementation block including children
            pos0 = Fl_Type::current->proj1_start;
            pos1 = Fl_Type::current->proj2_end;
            break;
          case 3: // code1: all implementation code before the children
            pos0 = Fl_Type::current->proj1_start;
            pos1 = Fl_Type::current->proj1_end;
            break;
          case 4: // code1: all implementation code before the children
            pos0 = Fl_Type::current->proj2_start;
            pos1 = Fl_Type::current->proj2_end;
            break;
        }
        if (pos0>=0) {
          if (pos1<pos0)
            pos1 = sv_project->buffer()->line_end(pos0);
          sv_project->buffer()->highlight(pos0, pos1);
          int line = sv_project->buffer()->count_lines(0, pos0);
          sv_project->scroll(line, 0);
        }
      }
    }
}

/**
 Callback to update the sourceview position.
*/
void update_sourceview_position_cb(class Fl_Tabs*, void*) {
  // make sure that the selected tab shows the current view
    update_sourceview_cb(0,0);
    // highlight the selected widget in the selected tab
    update_sourceview_position();
}

/**
 Generate a header, source, strings, or design file in a temporary directory
 and load those into the Code Viewer widgets.
*/
void update_sourceview_cb(class Fl_Button*, void*) {
  if (!sourceview_panel || !sourceview_panel->visible())
      return;

    if (!sv_source_filename) {
      sv_source_filename = (char*)malloc(FL_PATH_MAX);
      fl_strlcpy(sv_source_filename, get_tmpdir().c_str(), FL_PATH_MAX);
      fl_strlcat(sv_source_filename, "source_view_tmp.cxx", FL_PATH_MAX);
    }
    if (!sv_header_filename) {
      sv_header_filename = (char*)malloc(FL_PATH_MAX);
      fl_strlcpy(sv_header_filename, get_tmpdir().c_str(), FL_PATH_MAX);
      fl_strlcat(sv_header_filename, "source_view_tmp.h", FL_PATH_MAX);
    }
    if (!sv_design_filename) {
      sv_design_filename = (char*)malloc(FL_PATH_MAX);
      fl_strlcpy(sv_design_filename, get_tmpdir().c_str(), FL_PATH_MAX);
      fl_strlcat(sv_design_filename, "source_view_tmp.fl", FL_PATH_MAX);
    }

    if (sv_project->visible_r()) {
      write_file(sv_design_filename, false, true);
      int top = sv_project->top_line();
      sv_project->buffer()->loadfile(sv_design_filename);
      sv_project->scroll(top, 0);
    } else if (sv_strings->visible_r()) {
      static const char *exts[] = { ".txt", ".po", ".msg" };
      char fn[FL_PATH_MAX+1];
      fl_strlcpy(fn, get_tmpdir().c_str(), FL_PATH_MAX);
      fl_strlcat(fn, "strings", FL_PATH_MAX);
      fl_filename_setext(fn, FL_PATH_MAX, exts[g_project.i18n_type]);
      write_strings(fn);
      int top = sv_strings->top_line();
      sv_strings->buffer()->loadfile(fn);
      sv_strings->scroll(top, 0);
    } else if (sv_source->visible_r() || sv_header->visible_r()) {
      Fl_String code_file_name_bak = g_project.code_file_name;
      g_project.code_file_name = sv_source_filename;
      Fl_String header_file_name_bak = g_project.header_file_name;
      g_project.header_file_name = sv_header_filename;

      // generate the code and load the files
      Fd_Code_Writer f;
      // generate files
      if (f.write_code(sv_source_filename, sv_header_filename, true))
      {
        // load file into source editor
        int pos = sv_source->top_line();
        sv_source->buffer()->loadfile(sv_source_filename);
        sv_source->scroll(pos, 0);
        // load file into header editor
        pos = sv_header->top_line();
        sv_header->buffer()->loadfile(sv_header_filename);
        sv_header->scroll(pos, 0);
        // update the source code highlighting
        update_sourceview_position();
      }

      g_project.code_file_name = code_file_name_bak;
      g_project.header_file_name = header_file_name_bak;
    }
}

/**
 This is called by the timer itself
*/
void update_sourceview_timer(void*) {
  update_sourceview_cb(0,0);
}

void sourceview_defer_update() {
  // we will only update earliest 0.5 seconds after the last change, and only
      // if no other change was made, so dragging a widget will not generate any
      // CPU load
      Fl::remove_timeout(update_sourceview_timer, 0);
      Fl::add_timeout(0.5, update_sourceview_timer, 0);
}

/**
 Show or hide the source code preview.
 The state is stored in the app preferences.
*/
void sourceview_toggle_visibility() {
  if (!sourceview_panel) {
      make_sourceview();
      sourceview_panel->callback((Fl_Callback*)toggle_sourceview_cb);
      Fl_Preferences svp(fluid_prefs, "sourceview");
      int autorefresh;
      svp.get("autorefresh", autorefresh, 1);
      sv_autorefresh->value(autorefresh);
      int autoposition;
      svp.get("autoposition", autoposition, 1);
      sv_autoposition->value(autoposition);
      int tab;
      svp.get("tab", tab, 0);
      if (tab>=0 && tab<sv_tab->children()) sv_tab->value(sv_tab->child(tab));
      svp.get("code_choice", sv_code_choice, 2);
      sv_code_choice_w->value(sv_code_choice_w->find_item_with_argument(sv_code_choice));
      if (!position_window(sourceview_panel,"sourceview_pos", 0, 320, 120, 550, 500)) return;
    }

    if (sourceview_panel->visible()) {
      sourceview_panel->hide();
      sourceview_item->label("Show Code View");
    } else {
      sourceview_panel->show();
      sourceview_item->label("Hide Code View");
      update_sourceview_cb(0,0);
    }
}

Fl_Double_Window *sourceview_panel=(Fl_Double_Window *)0;

Fl_Tabs *sv_tab=(Fl_Tabs *)0;

Fl_Group *sv_source_tab=(Fl_Group *)0;

CodeViewer *sv_source=(CodeViewer *)0;

CodeViewer *sv_header=(CodeViewer *)0;

TextViewer *sv_strings=(TextViewer *)0;

TextViewer *sv_project=(TextViewer *)0;

Fl_Group *cv_find_row=(Fl_Group *)0;

Fl_Button *sv_find_text_case=(Fl_Button *)0;

Fl_Input *sv_find_text=(Fl_Input *)0;

static void cb_sv_find_text(Fl_Input* o, void*) {
  Fl_Text_Display *e = NULL;
  if (sv_source->visible_r()) {
    e = sv_source;
  } else if (sv_header->visible_r()) {
    e = sv_header;
  } else if (sv_project->visible_r()) {
    e = sv_project;
  }
  if (e) {
    Fl_Text_Buffer *b = e->buffer();
    int pos = e->insert_position();
    int found = b->search_forward(pos, o->value(), &pos, sv_find_text_case->value());
    if (found) {
      b->select(pos, pos + (int)strlen(o->value()));
      e->insert_position(pos);
      e->show_insert_position();
    }
  }
}

static void cb_(Fl_Button*, void*) {
  Fl_Text_Display *e = NULL;
  if (sv_source->visible_r()) {
    e = sv_source;
  } else if (sv_header->visible_r()) {
    e = sv_header;
  } else if (sv_project->visible_r()) {
    e = sv_project;
  }
  if (e) {
    const char *needle = sv_find_text->value();
    Fl_Text_Buffer *b = e->buffer();
    int pos = e->insert_position()-1;
    if (pos < 0) pos = b->length()-1;
    int found = b->search_backward(pos, needle, &pos, sv_find_text_case->value());
    if (!found)
      found = b->search_backward(b->length()-1, needle, &pos, sv_find_text_case->value());
    if (found) {
      b->select(pos, pos + (int)strlen(needle));
      e->insert_position(pos);
      e->show_insert_position();
    }
  }
}

static void cb_1(Fl_Button*, void*) {
  Fl_Text_Display *e = NULL;
  if (sv_source->visible_r()) {
    e = sv_source;
  } else if (sv_header->visible_r()) {
    e = sv_header;
  } else if (sv_project->visible_r()) {
    e = sv_project;
  }
  if (e) {
    const char *needle = sv_find_text->value();
    Fl_Text_Buffer *b = e->buffer();
    int pos = e->insert_position() + 1;
    if (pos+1 >= b->length()) pos = 0;
    int found = b->search_forward(pos, needle, &pos, sv_find_text_case->value());
    if (!found && (pos > 0))
      found = b->search_forward(0, needle, &pos, sv_find_text_case->value());
    if (found) {
      b->select(pos, pos + (int)strlen(needle));
      e->insert_position(pos);
      e->show_insert_position();
    }
  }
}

static void cb_Reveal(Fl_Button*, void*) {
  if (sourceview_panel && sourceview_panel->visible()) {
    Fl_Type *node = NULL;
    if (sv_source->visible_r())
      node = Fl_Type::find_in_text(0, sv_source->insert_position());
    else if (sv_header->visible_r())
      node = Fl_Type::find_in_text(1, sv_header->insert_position());
    else if (sv_project->visible_r())
      node = Fl_Type::find_in_text(2, sv_project->insert_position());
    if (node) {
      select_only(node);
      reveal_in_browser(node);
      if (Fl::event_clicks()==1) // double click
        node->open();
    }
  }
}

Fl_Group *cv_settings_row=(Fl_Group *)0;

Fl_Light_Button *sv_autorefresh=(Fl_Light_Button *)0;

Fl_Light_Button *sv_autoposition=(Fl_Light_Button *)0;

Fl_Choice *sv_code_choice_w=(Fl_Choice *)0;

static void cb_sv_code_choice_w(Fl_Choice* o, void*) {
  sv_code_choice = (int)o->mvalue()->argument();
  update_sourceview_position();
}

Fl_Menu_Item menu_sv_code_choice_w[] = {
 {"prolog", 0,  0, (void*)(0), 16, (uchar)FL_NORMAL_LABEL, 0, 11, 0},
 {"static", 0,  0, (void*)(1), 0, (uchar)FL_NORMAL_LABEL, 0, 11, 0},
 {"code", 0,  0, (void*)(2), 0, (uchar)FL_NORMAL_LABEL, 0, 11, 0},
 {"code 1", 0,  0, (void*)(3), 0, (uchar)FL_NORMAL_LABEL, 0, 11, 0},
 {"code 2", 0,  0, (void*)(4), 0, (uchar)FL_NORMAL_LABEL, 0, 11, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_Double_Window* make_sourceview() {
  { sourceview_panel = new Fl_Double_Window(520, 515, "Code View");
    sourceview_panel->callback((Fl_Callback*)toggle_sourceview_cb);
    sourceview_panel->align(Fl_Align(FL_ALIGN_CLIP|FL_ALIGN_INSIDE));
    { sv_tab = new Fl_Tabs(10, 10, 500, 440);
      sv_tab->selection_color((Fl_Color)4);
      sv_tab->labelcolor(FL_BACKGROUND2_COLOR);
      sv_tab->callback((Fl_Callback*)update_sourceview_position_cb);
      { sv_source_tab = new Fl_Group(10, 35, 500, 415, "Source");
        sv_source_tab->labelsize(13);
        { CodeViewer* o = sv_source = new CodeViewer(10, 40, 500, 410);
          sv_source->box(FL_DOWN_FRAME);
          sv_source->color(FL_BACKGROUND2_COLOR);
          sv_source->selection_color(FL_SELECTION_COLOR);
          sv_source->labeltype(FL_NORMAL_LABEL);
          sv_source->labelfont(0);
          sv_source->labelsize(14);
          sv_source->labelcolor(FL_FOREGROUND_COLOR);
          sv_source->textfont(4);
          sv_source->textsize(11);
          sv_source->align(Fl_Align(FL_ALIGN_TOP));
          sv_source->when(FL_WHEN_RELEASE);
          Fl_Group::current()->resizable(sv_source);
          o->linenumber_width(60);
          o->linenumber_size(o->Fl_Text_Display::textsize());
        } // CodeViewer* sv_source
        sv_source_tab->end();
        Fl_Group::current()->resizable(sv_source_tab);
      } // Fl_Group* sv_source_tab
      { Fl_Group* o = new Fl_Group(10, 35, 500, 415, "Header");
        o->labelsize(13);
        o->hide();
        { CodeViewer* o = sv_header = new CodeViewer(10, 40, 500, 410);
          sv_header->box(FL_DOWN_FRAME);
          sv_header->color(FL_BACKGROUND2_COLOR);
          sv_header->selection_color(FL_SELECTION_COLOR);
          sv_header->labeltype(FL_NORMAL_LABEL);
          sv_header->labelfont(0);
          sv_header->labelsize(14);
          sv_header->labelcolor(FL_FOREGROUND_COLOR);
          sv_header->textfont(4);
          sv_header->textsize(11);
          sv_header->align(Fl_Align(FL_ALIGN_TOP));
          sv_header->when(FL_WHEN_RELEASE);
          Fl_Group::current()->resizable(sv_header);
          o->linenumber_width(60);
          o->linenumber_size(o->Fl_Text_Display::textsize());
        } // CodeViewer* sv_header
        o->end();
      } // Fl_Group* o
      { Fl_Group* o = new Fl_Group(10, 35, 500, 415, "Strings");
        o->labelsize(13);
        o->hide();
        { TextViewer* o = sv_strings = new TextViewer(10, 40, 500, 410);
          sv_strings->box(FL_DOWN_FRAME);
          sv_strings->color(FL_BACKGROUND2_COLOR);
          sv_strings->selection_color(FL_SELECTION_COLOR);
          sv_strings->labeltype(FL_NORMAL_LABEL);
          sv_strings->labelfont(0);
          sv_strings->labelsize(14);
          sv_strings->labelcolor(FL_FOREGROUND_COLOR);
          sv_strings->textfont(4);
          sv_strings->textsize(11);
          sv_strings->align(Fl_Align(FL_ALIGN_TOP));
          sv_strings->when(FL_WHEN_RELEASE);
          Fl_Group::current()->resizable(sv_strings);
          o->linenumber_width(60);
          o->linenumber_size(o->Fl_Text_Display::textsize());
        } // TextViewer* sv_strings
        o->end();
      } // Fl_Group* o
      { Fl_Group* o = new Fl_Group(10, 35, 500, 415, "Project");
        o->labelsize(13);
        o->hide();
        { TextViewer* o = sv_project = new TextViewer(10, 40, 500, 410);
          sv_project->box(FL_DOWN_FRAME);
          sv_project->color(FL_BACKGROUND2_COLOR);
          sv_project->selection_color(FL_SELECTION_COLOR);
          sv_project->labeltype(FL_NORMAL_LABEL);
          sv_project->labelfont(0);
          sv_project->labelsize(14);
          sv_project->labelcolor(FL_FOREGROUND_COLOR);
          sv_project->textfont(4);
          sv_project->textsize(11);
          sv_project->align(Fl_Align(FL_ALIGN_TOP));
          sv_project->when(FL_WHEN_RELEASE);
          Fl_Group::current()->resizable(sv_project);
          o->linenumber_width(60);
          o->linenumber_size(o->Fl_Text_Display::textsize());
        } // TextViewer* sv_project
        o->end();
      } // Fl_Group* o
      sv_tab->end();
      Fl_Group::current()->resizable(sv_tab);
    } // Fl_Tabs* sv_tab
    { cv_find_row = new Fl_Group(10, 460, 500, 20);
      { sv_find_text_case = new Fl_Button(244, 460, 25, 20, "aA");
        sv_find_text_case->type(1);
        sv_find_text_case->labelsize(11);
      } // Fl_Button* sv_find_text_case
      { sv_find_text = new Fl_Input(40, 460, 200, 20, "Find:");
        sv_find_text->labelsize(11);
        sv_find_text->textsize(11);
        sv_find_text->callback((Fl_Callback*)cb_sv_find_text);
        sv_find_text->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY_CHANGED);
      } // Fl_Input* sv_find_text
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
        o->callback((Fl_Callback*)update_sourceview_cb);
      } // Fl_Button* o
      { Fl_Light_Button* o = sv_autorefresh = new Fl_Light_Button(77, 485, 91, 20, "Auto-Refresh");
        sv_autorefresh->labelsize(11);
        o->callback((Fl_Callback*)update_sourceview_cb);
      } // Fl_Light_Button* sv_autorefresh
      { sv_autoposition = new Fl_Light_Button(172, 485, 89, 20, "Auto-Position");
        sv_autoposition->labelsize(11);
      } // Fl_Light_Button* sv_autoposition
      { sv_code_choice_w = new Fl_Choice(265, 485, 70, 20);
        sv_code_choice_w->down_box(FL_BORDER_BOX);
        sv_code_choice_w->labelsize(11);
        sv_code_choice_w->textsize(11);
        sv_code_choice_w->callback((Fl_Callback*)cb_sv_code_choice_w);
        sv_code_choice_w->menu(menu_sv_code_choice_w);
      } // Fl_Choice* sv_code_choice_w
      { Fl_Box* o = new Fl_Box(375, 485, 80, 20);
        Fl_Group::current()->resizable(o);
      } // Fl_Box* o
      { Fl_Button* o = new Fl_Button(460, 485, 50, 20, "Close");
        o->labelsize(11);
        o->callback((Fl_Callback*)toggle_sourceview_b_cb);
      } // Fl_Button* o
      cv_settings_row->end();
    } // Fl_Group* cv_settings_row
    sourceview_panel->size_range(384, 120);
    sourceview_panel->end();
  } // Fl_Double_Window* sourceview_panel
  return sourceview_panel;
}

//
