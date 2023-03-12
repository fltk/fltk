//
// Setting and shell dialogs for the Fast Light Tool Kit (FLTK).
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

#include "alignment_panel.h"
#include <FL/Fl_Preferences.H>
#include <FL/fl_ask.H>
#include <string.h>

Fl_Double_Window *project_window=(Fl_Double_Window *)0;

static void cb_Close(Fl_Button*, void*) {
  project_window->hide();
  set_modflag(-1, -1);
}

Fl_Input *header_file_input=(Fl_Input *)0;

Fl_Input *code_file_input=(Fl_Input *)0;

Fl_Check_Button *include_H_from_C_button=(Fl_Check_Button *)0;

Fl_Check_Button *use_FL_COMMAND_button=(Fl_Check_Button *)0;

Fl_Check_Button *utf8_in_src_button=(Fl_Check_Button *)0;

Fl_Check_Button *avoid_early_includes_button=(Fl_Check_Button *)0;

Fl_Choice *i18n_type_chooser=(Fl_Choice *)0;

Fl_Menu_Item menu_i18n_type_chooser[] = {
 {"None", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 14, 0},
 {"GNU gettext", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 14, 0},
 {"POSIX catgets", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 14, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_Input *i18n_include_input=(Fl_Input *)0;

Fl_Input *i18n_conditional_input=(Fl_Input *)0;

Fl_Input *i18n_file_input=(Fl_Input *)0;

Fl_Int_Input *i18n_set_input=(Fl_Int_Input *)0;

Fl_Input *i18n_function_input=(Fl_Input *)0;

Fl_Input *i18n_static_function_input=(Fl_Input *)0;

Fl_Double_Window* make_project_window() {
  { project_window = new Fl_Double_Window(399, 298, "Project Settings");
    { Fl_Button* o = new Fl_Button(328, 267, 60, 25, "Close");
      o->tooltip("Close this dialog.");
      o->callback((Fl_Callback*)cb_Close);
    } // Fl_Button* o
    { Fl_Tabs* o = new Fl_Tabs(10, 10, 379, 246);
      o->selection_color((Fl_Color)12);
      o->labelcolor(FL_BACKGROUND2_COLOR);
      { Fl_Group* o = new Fl_Group(10, 36, 379, 220, "Output");
        o->hide();
        { Fl_Box* o = new Fl_Box(20, 49, 340, 49, "Use \"name.ext\" to set a file name or just \".ext\" to set extension.");
          o->align(Fl_Align(132|FL_ALIGN_INSIDE));
        } // Fl_Box* o
        { header_file_input = new Fl_Input(119, 103, 252, 20, "Header File:");
          header_file_input->tooltip("The name of the generated header file.");
          header_file_input->box(FL_THIN_DOWN_BOX);
          header_file_input->labelfont(1);
          header_file_input->textfont(4);
          header_file_input->callback((Fl_Callback*)header_input_cb, (void*)(1));
          header_file_input->when(FL_WHEN_CHANGED);
        } // Fl_Input* header_file_input
        { code_file_input = new Fl_Input(119, 128, 252, 20, "Code File:");
          code_file_input->tooltip("The name of the generated code file.");
          code_file_input->box(FL_THIN_DOWN_BOX);
          code_file_input->labelfont(1);
          code_file_input->textfont(4);
          code_file_input->callback((Fl_Callback*)code_input_cb, (void*)(1));
          code_file_input->when(FL_WHEN_CHANGED);
        } // Fl_Input* code_file_input
        { include_H_from_C_button = new Fl_Check_Button(117, 153, 272, 20, "Include Header from Code");
          include_H_from_C_button->tooltip("Include the header file from the code file.");
          include_H_from_C_button->down_box(FL_DOWN_BOX);
          include_H_from_C_button->callback((Fl_Callback*)include_H_from_C_button_cb);
        } // Fl_Check_Button* include_H_from_C_button
        { use_FL_COMMAND_button = new Fl_Check_Button(117, 176, 272, 20, "Menu shortcuts use FL_COMMAND");
          use_FL_COMMAND_button->tooltip("Replace FL_CTRL and FL_META with FL_COMMAND when generating menu shortcuts");
          use_FL_COMMAND_button->down_box(FL_DOWN_BOX);
          use_FL_COMMAND_button->callback((Fl_Callback*)use_FL_COMMAND_button_cb);
        } // Fl_Check_Button* use_FL_COMMAND_button
        { utf8_in_src_button = new Fl_Check_Button(117, 199, 272, 20, "allow Unicode UTF-8 in source code");
          utf8_in_src_button->tooltip("For older compilers, characters outside of the printable ASCII range are esca\
ped using octal notation `\\0123`. If this option is checked, Fluid will write\
 UTF-8 characters unchanged.");
          utf8_in_src_button->down_box(FL_DOWN_BOX);
          utf8_in_src_button->callback((Fl_Callback*)utf8_in_src_cb);
        } // Fl_Check_Button* utf8_in_src_button
        { avoid_early_includes_button = new Fl_Check_Button(117, 222, 272, 20, "avoid early include of Fl.H");
          avoid_early_includes_button->tooltip("Do not emit #include <FL//Fl.H> until it is needed by another include file.");
          avoid_early_includes_button->down_box(FL_DOWN_BOX);
          avoid_early_includes_button->callback((Fl_Callback*)avoid_early_includes_cb);
        } // Fl_Check_Button* avoid_early_includes_button
        o->end();
      } // Fl_Group* o
      { Fl_Group* o = new Fl_Group(10, 36, 378, 220, "Internationalization");
        { i18n_type_chooser = new Fl_Choice(128, 48, 136, 25, "Use:");
          i18n_type_chooser->tooltip("Type of internationalization to use.");
          i18n_type_chooser->box(FL_THIN_UP_BOX);
          i18n_type_chooser->down_box(FL_BORDER_BOX);
          i18n_type_chooser->labelfont(1);
          i18n_type_chooser->callback((Fl_Callback*)i18n_type_cb);
          i18n_type_chooser->menu(menu_i18n_type_chooser);
        } // Fl_Choice* i18n_type_chooser
        { i18n_include_input = new Fl_Input(128, 78, 243, 20, "#include:");
          i18n_include_input->tooltip("The include file for internationalization.");
          i18n_include_input->box(FL_THIN_DOWN_BOX);
          i18n_include_input->labelfont(1);
          i18n_include_input->textfont(4);
          i18n_include_input->callback((Fl_Callback*)i18n_text_cb);
        } // Fl_Input* i18n_include_input
        { i18n_conditional_input = new Fl_Input(128, 103, 243, 20, "Conditional:");
          i18n_conditional_input->tooltip("only include the header file if this preprocessor macro is defined, for examp\
le FLTK_GETTEXT_FOUND");
          i18n_conditional_input->box(FL_THIN_DOWN_BOX);
          i18n_conditional_input->labelfont(1);
          i18n_conditional_input->textfont(4);
          i18n_conditional_input->callback((Fl_Callback*)i18n_text_cb);
        } // Fl_Input* i18n_conditional_input
        { i18n_file_input = new Fl_Input(128, 128, 243, 20, "File:");
          i18n_file_input->tooltip("The name of the message catalog.");
          i18n_file_input->box(FL_THIN_DOWN_BOX);
          i18n_file_input->labelfont(1);
          i18n_file_input->textfont(4);
          i18n_file_input->callback((Fl_Callback*)i18n_text_cb);
        } // Fl_Input* i18n_file_input
        { i18n_set_input = new Fl_Int_Input(128, 153, 243, 20, "Set:");
          i18n_set_input->tooltip("The message set number.");
          i18n_set_input->type(2);
          i18n_set_input->box(FL_THIN_DOWN_BOX);
          i18n_set_input->labelfont(1);
          i18n_set_input->textfont(4);
          i18n_set_input->callback((Fl_Callback*)i18n_int_cb);
        } // Fl_Int_Input* i18n_set_input
        { i18n_function_input = new Fl_Input(128, 128, 243, 20, "Function:");
          i18n_function_input->tooltip("The function to call to translate labels and tooltips, usually \"gettext\" or\
 \"_\"");
          i18n_function_input->box(FL_THIN_DOWN_BOX);
          i18n_function_input->labelfont(1);
          i18n_function_input->textfont(4);
          i18n_function_input->callback((Fl_Callback*)i18n_text_cb);
        } // Fl_Input* i18n_function_input
        { i18n_static_function_input = new Fl_Input(128, 153, 243, 20, "Static Function:");
          i18n_static_function_input->tooltip("function to call to translate static text, The function to call to internatio\
nalize labels and tooltips, usually \"gettext_noop\" or \"N_\"");
          i18n_static_function_input->box(FL_THIN_DOWN_BOX);
          i18n_static_function_input->labelfont(1);
          i18n_static_function_input->textfont(4);
          i18n_static_function_input->callback((Fl_Callback*)i18n_text_cb);
        } // Fl_Input* i18n_static_function_input
        o->end();
      } // Fl_Group* o
      o->end();
    } // Fl_Tabs* o
    project_window->set_modal();
    project_window->end();
  } // Fl_Double_Window* project_window
  return project_window;
}
void scheme_cb(Fl_Scheme_Choice *, void *);

Fl_Double_Window *settings_window=(Fl_Double_Window *)0;

Fl_Scheme_Choice *scheme_choice=(Fl_Scheme_Choice *)0;

Fl_Check_Button *tooltips_button=(Fl_Check_Button *)0;

static void cb_tooltips_button(Fl_Check_Button*, void*) {
  Fl_Tooltip::enable(tooltips_button->value());
  fluid_prefs.set("show_tooltips", tooltips_button->value());
}

Fl_Check_Button *completion_button=(Fl_Check_Button *)0;

static void cb_completion_button(Fl_Check_Button*, void*) {
  fluid_prefs.set("show_completion_dialogs", completion_button->value());
}

Fl_Check_Button *openlast_button=(Fl_Check_Button *)0;

static void cb_openlast_button(Fl_Check_Button*, void*) {
  fluid_prefs.set("open_previous_file", openlast_button->value());
}

Fl_Check_Button *prevpos_button=(Fl_Check_Button *)0;

static void cb_prevpos_button(Fl_Check_Button*, void*) {
  fluid_prefs.set("prev_window_pos", prevpos_button->value());
}

Fl_Check_Button *show_comments_button=(Fl_Check_Button *)0;

static void cb_show_comments_button(Fl_Check_Button*, void*) {
  show_comments = show_comments_button->value();
  fluid_prefs.set("show_comments", show_comments);
  redraw_browser();
}

Fl_Spinner *recent_spinner=(Fl_Spinner *)0;

static void cb_recent_spinner(Fl_Spinner*, void*) {
  fluid_prefs.set("recent_files", recent_spinner->value());
  load_history();
}

Fl_Check_Button *use_external_editor_button=(Fl_Check_Button *)0;

static void cb_use_external_editor_button(Fl_Check_Button*, void*) {
  G_use_external_editor = use_external_editor_button->value();
  fluid_prefs.set("use_external_editor", G_use_external_editor);
  redraw_browser();
}

Fl_Input *editor_command_input=(Fl_Input *)0;

static void cb_editor_command_input(Fl_Input*, void*) {
  strncpy(G_external_editor_command, editor_command_input->value(), sizeof(G_external_editor_command)-1);
  G_external_editor_command[sizeof(G_external_editor_command)-1] = 0;
  fluid_prefs.set("external_editor_command", G_external_editor_command);
  redraw_browser();
}

static void cb_Close1(Fl_Button*, void*) {
  settings_window->hide();
}

Fl_Double_Window* make_settings_window() {
  { Fl_Double_Window* o = settings_window = new Fl_Double_Window(360, 355, "GUI Settings");
    { scheme_choice = new Fl_Scheme_Choice(140, 10, 115, 25, "Scheme: ");
      scheme_choice->box(FL_FLAT_BOX);
      scheme_choice->down_box(FL_BORDER_BOX);
      scheme_choice->color(FL_BACKGROUND_COLOR);
      scheme_choice->selection_color(FL_SELECTION_COLOR);
      scheme_choice->labeltype(FL_NORMAL_LABEL);
      scheme_choice->labelfont(1);
      scheme_choice->labelsize(14);
      scheme_choice->labelcolor(FL_FOREGROUND_COLOR);
      scheme_choice->callback((Fl_Callback*)scheme_cb);
      scheme_choice->align(Fl_Align(FL_ALIGN_LEFT));
      scheme_choice->when(FL_WHEN_RELEASE);
      init_scheme();
    } // Fl_Scheme_Choice* scheme_choice
    { Fl_Group* o = new Fl_Group(20, 43, 330, 161);
      o->labelfont(1);
      o->align(Fl_Align(FL_ALIGN_CENTER));
      { Fl_Box* o = new Fl_Box(140, 43, 1, 25, "Options: ");
        o->labelfont(1);
        o->align(Fl_Align(FL_ALIGN_LEFT));
      } // Fl_Box* o
      { tooltips_button = new Fl_Check_Button(138, 43, 113, 25, "Show Tooltips");
        tooltips_button->down_box(FL_DOWN_BOX);
        tooltips_button->labelsize(12);
        tooltips_button->callback((Fl_Callback*)cb_tooltips_button);
        int b;
        fluid_prefs.get("show_tooltips", b, 1);
        tooltips_button->value(b);
        Fl_Tooltip::enable(b);
      } // Fl_Check_Button* tooltips_button
      { completion_button = new Fl_Check_Button(138, 68, 186, 25, "Show Completion Dialogs");
        completion_button->down_box(FL_DOWN_BOX);
        completion_button->labelsize(12);
        completion_button->callback((Fl_Callback*)cb_completion_button);
        int b;
        fluid_prefs.get("show_completion_dialogs", b, 1);
        completion_button->value(b);
      } // Fl_Check_Button* completion_button
      { openlast_button = new Fl_Check_Button(138, 93, 214, 25, "Open Previous File on Startup");
        openlast_button->down_box(FL_DOWN_BOX);
        openlast_button->labelsize(12);
        openlast_button->callback((Fl_Callback*)cb_openlast_button);
        int b;
        fluid_prefs.get("open_previous_file", b, 0);
        openlast_button->value(b);
      } // Fl_Check_Button* openlast_button
      { prevpos_button = new Fl_Check_Button(138, 118, 209, 25, "Remember Window Positions");
        prevpos_button->down_box(FL_DOWN_BOX);
        prevpos_button->labelsize(12);
        prevpos_button->callback((Fl_Callback*)cb_prevpos_button);
        int b;
        fluid_prefs.get("prev_window_pos", b, 1);
        prevpos_button->value(b);
      } // Fl_Check_Button* prevpos_button
      { show_comments_button = new Fl_Check_Button(138, 143, 209, 25, "Show Comments in Browser");
        show_comments_button->down_box(FL_DOWN_BOX);
        show_comments_button->labelsize(12);
        show_comments_button->callback((Fl_Callback*)cb_show_comments_button);
        fluid_prefs.get("show_comments", show_comments, 1);
        show_comments_button->value(show_comments);
      } // Fl_Check_Button* show_comments_button
      o->end();
    } // Fl_Group* o
    { recent_spinner = new Fl_Spinner(140, 173, 40, 25, "# Recent Files: ");
      recent_spinner->labelfont(1);
      recent_spinner->callback((Fl_Callback*)cb_recent_spinner);
      recent_spinner->when(FL_WHEN_CHANGED);
      int c;
      fluid_prefs.get("recent_files", c, 5);
      recent_spinner->maximum(10);
      recent_spinner->value(c);
    } // Fl_Spinner* recent_spinner
    { Fl_Group* o = new Fl_Group(10, 210, 337, 95);
      o->box(FL_THIN_UP_BOX);
      o->color(FL_DARK1);
      { use_external_editor_button = new Fl_Check_Button(25, 218, 209, 22, "Use external editor?");
        use_external_editor_button->down_box(FL_DOWN_BOX);
        use_external_editor_button->labelsize(12);
        use_external_editor_button->callback((Fl_Callback*)cb_use_external_editor_button);
        fluid_prefs.get("use_external_editor", G_use_external_editor, 0);
        use_external_editor_button->value(G_use_external_editor);
      } // Fl_Check_Button* use_external_editor_button
      { editor_command_input = new Fl_Input(25, 264, 305, 21, "Editor Command");
        editor_command_input->tooltip("The editor command to open your external text editor.\nInclude any necessary \
flags to ensure your editor does not background itself.\nExamples:\n    gvim -\
f\n    gedit\n emacs");
        editor_command_input->labelsize(12);
        editor_command_input->textsize(12);
        editor_command_input->callback((Fl_Callback*)cb_editor_command_input);
        editor_command_input->align(Fl_Align(FL_ALIGN_TOP_LEFT));
        editor_command_input->when(FL_WHEN_CHANGED);
        fluid_prefs.get("external_editor_command", G_external_editor_command, "", sizeof(G_external_editor_command)-1);
        editor_command_input->value(G_external_editor_command);
      } // Fl_Input* editor_command_input
      o->end();
      Fl_Group::current()->resizable(o);
    } // Fl_Group* o
    { Fl_Button* o = new Fl_Button(285, 320, 64, 25, "Close");
      o->tooltip("Close this dialog.");
      o->callback((Fl_Callback*)cb_Close1);
    } // Fl_Button* o
    o->size_range(o->w(), o->h());
    settings_window->set_non_modal();
    settings_window->end();
  } // Fl_Double_Window* settings_window
  return settings_window;
}

Fl_Double_Window *shell_window=(Fl_Double_Window *)0;

Fl_Input *shell_command_input=(Fl_Input *)0;

Fl_Check_Button *shell_savefl_button=(Fl_Check_Button *)0;

Fl_Check_Button *shell_writecode_button=(Fl_Check_Button *)0;

Fl_Check_Button *shell_writemsgs_button=(Fl_Check_Button *)0;

Fl_Check_Button *shell_use_fl_button=(Fl_Check_Button *)0;

static void cb_shell_use_fl_button(Fl_Check_Button*, void*) {
  g_shell_use_fl_settings = shell_use_fl_button->value();
  fluid_prefs.set("shell_use_fl", g_shell_use_fl_settings);
  if (g_shell_use_fl_settings) {
    shell_settings_read();
  } else {
    shell_prefs_get();
  }
  update_shell_window();
}

static void cb_save(Fl_Button*, void*) {
  apply_shell_window();
  shell_prefs_set();
}

static void cb_Run(Fl_Return_Button*, void*) {
  apply_shell_window();
  do_shell_command(NULL, NULL);
}

static void cb_Cancel(Fl_Button*, void*) {
  shell_command_input->value(g_shell_command);
  shell_window->hide();
}

Fl_Double_Window *shell_run_window=(Fl_Double_Window *)0;

Fl_Simple_Terminal *shell_run_terminal=(Fl_Simple_Terminal *)0;

Fl_Return_Button *shell_run_button=(Fl_Return_Button *)0;

static void cb_shell_run_button(Fl_Return_Button*, void*) {
  Fl_Preferences pos(fluid_prefs, "shell_run_Window_pos");
  pos.set("x", shell_run_window->x());
  pos.set("y", shell_run_window->y());
  pos.set("w", shell_run_window->w());
  pos.set("h", shell_run_window->h());
  shell_run_window->hide();
}

Fl_Double_Window* make_shell_window() {
  { shell_window = new Fl_Double_Window(375, 208, "Shell Command");
    { Fl_Group* o = new Fl_Group(0, 0, 375, 165);
      { shell_command_input = new Fl_Input(82, 14, 277, 20, "Command:");
        shell_command_input->tooltip("external shell command");
        shell_command_input->labelfont(1);
        shell_command_input->labelsize(12);
        shell_command_input->textfont(4);
        shell_command_input->textsize(12);
        Fl_Group::current()->resizable(shell_command_input);
      } // Fl_Input* shell_command_input
      { shell_savefl_button = new Fl_Check_Button(82, 39, 136, 19, "save .fl project file");
        shell_savefl_button->tooltip("save the project to the .fl file before running the command");
        shell_savefl_button->down_box(FL_DOWN_BOX);
        shell_savefl_button->labelsize(12);
      } // Fl_Check_Button* shell_savefl_button
      { shell_writecode_button = new Fl_Check_Button(82, 59, 120, 19, "save source code");
        shell_writecode_button->tooltip("generate the source code and header file before running the command");
        shell_writecode_button->down_box(FL_DOWN_BOX);
        shell_writecode_button->labelsize(12);
      } // Fl_Check_Button* shell_writecode_button
      { shell_writemsgs_button = new Fl_Check_Button(82, 79, 126, 19, "save i18n strings");
        shell_writemsgs_button->tooltip("save the internationalisation string before running the command");
        shell_writemsgs_button->down_box(FL_DOWN_BOX);
        shell_writemsgs_button->labelsize(12);
      } // Fl_Check_Button* shell_writemsgs_button
      { shell_use_fl_button = new Fl_Check_Button(82, 110, 180, 19, "use settings in .fl project files");
        shell_use_fl_button->tooltip("check to read and write shell command from and to .fl files");
        shell_use_fl_button->down_box(FL_DOWN_BOX);
        shell_use_fl_button->labelsize(12);
        shell_use_fl_button->callback((Fl_Callback*)cb_shell_use_fl_button);
      } // Fl_Check_Button* shell_use_fl_button
      { Fl_Box* o = new Fl_Box(82, 103, 275, 1);
        o->box(FL_BORDER_FRAME);
        o->color(FL_FOREGROUND_COLOR);
      } // Fl_Box* o
      { Fl_Group* o = new Fl_Group(82, 134, 273, 20);
        { Fl_Button* o = new Fl_Button(82, 134, 104, 20, "save as default");
          o->tooltip("update the Fluid app settings for external shell commands to the current sett\
ings");
          o->labelsize(12);
          o->callback((Fl_Callback*)cb_save);
        } // Fl_Button* o
        { Fl_Box* o = new Fl_Box(186, 136, 169, 15);
          Fl_Group::current()->resizable(o);
        } // Fl_Box* o
        o->end();
      } // Fl_Group* o
      o->end();
      Fl_Group::current()->resizable(o);
    } // Fl_Group* o
    { Fl_Group* o = new Fl_Group(0, 160, 375, 48);
      { Fl_Box* o = new Fl_Box(10, 167, 135, 25);
        Fl_Group::current()->resizable(o);
      } // Fl_Box* o
      { Fl_Return_Button* o = new Fl_Return_Button(145, 167, 100, 25, "Run");
        o->tooltip("save selected files and run the command");
        o->labelsize(12);
        o->callback((Fl_Callback*)cb_Run);
      } // Fl_Return_Button* o
      { Fl_Button* o = new Fl_Button(255, 167, 100, 25, "Cancel");
        o->labelsize(12);
        o->callback((Fl_Callback*)cb_Cancel);
      } // Fl_Button* o
      o->end();
    } // Fl_Group* o
    shell_window->set_modal();
    shell_window->size_range(375, 208, 1024, 208);
    shell_window->end();
  } // Fl_Double_Window* shell_window
  { shell_run_window = new Fl_Double_Window(555, 430, "Shell Command Output");
    { shell_run_terminal = new Fl_Simple_Terminal(10, 10, 535, 375);
      Fl_Group::current()->resizable(shell_run_terminal);
    } // Fl_Simple_Terminal* shell_run_terminal
    { Fl_Group* o = new Fl_Group(10, 395, 535, 25);
      { Fl_Box* o = new Fl_Box(10, 395, 435, 25);
        o->hide();
        Fl_Group::current()->resizable(o);
      } // Fl_Box* o
      { shell_run_button = new Fl_Return_Button(445, 395, 100, 25, "Close");
        shell_run_button->callback((Fl_Callback*)cb_shell_run_button);
      } // Fl_Return_Button* shell_run_button
      o->end();
    } // Fl_Group* o
    shell_run_window->end();
  } // Fl_Double_Window* shell_run_window
  return shell_run_window;
}

Fl_Double_Window *grid_window=(Fl_Double_Window *)0;

static void cb_grid_window(Fl_Double_Window* o, void* v) {
  propagate_load(o, v);
}

static void cb_Close2(Fl_Button*, void* v) {
  if (v != LOAD) grid_window->hide();
}

Fl_Round_Button *def_widget_size[6]={(Fl_Round_Button *)0};

static void cb_left(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->left_window_margin);
  } else {
    layout->left_window_margin = (int)o->value();
  }
}

static void cb_right(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->right_window_margin);
  } else {
    layout->right_window_margin = (int)o->value();
  }
}

static void cb_top(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->top_window_margin);
  } else {
    layout->top_window_margin = (int)o->value();
  }
}

static void cb_bottom(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->bottom_window_margin);
  } else {
    layout->bottom_window_margin = (int)o->value();
  }
}

static void cb_window(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->window_grid_x);
  } else {
    layout->window_grid_x = (int)o->value();
  }
}

static void cb_widget(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->widget_min_w);
  } else {
    layout->widget_min_w = (int)o->value();
  }
}

static void cb_widget1(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->widget_inc_w);
  } else {
    layout->widget_inc_w = (int)o->value();
  }
}

static void cb_widget2(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->widget_gap_x);
  } else {
    layout->widget_gap_x = (int)o->value();
  }
}

static void cb_widget3(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->widget_min_h);
  } else {
    layout->widget_min_h = (int)o->value();
  }
}

static void cb_widget4(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->widget_inc_h);
  } else {
    layout->widget_inc_h = (int)o->value();
  }
}

static void cb_widget5(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->widget_gap_y);
  } else {
    layout->widget_gap_y = (int)o->value();
  }
}

static void cb_window1(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->window_grid_y);
  } else {
    layout->window_grid_y = (int)o->value();
  }
}

static void cb_left1(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->left_group_margin);
  } else {
    layout->left_group_margin = (int)o->value();
  }
}

static void cb_right1(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->right_group_margin);
  } else {
    layout->right_group_margin = (int)o->value();
  }
}

static void cb_top1(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->top_group_margin);
  } else {
    layout->top_group_margin = (int)o->value();
  }
}

static void cb_bottom1(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->bottom_group_margin);
  } else {
    layout->bottom_group_margin = (int)o->value();
  }
}

static void cb_top2(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->top_tabs_margin);
  } else {
    layout->top_tabs_margin = (int)o->value();
  }
}

static void cb_bottom2(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->bottom_tabs_margin);
  } else {
    layout->bottom_tabs_margin = (int)o->value();
  }
}

static void cb_group(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->group_grid_x);
  } else {
    layout->group_grid_x = (int)o->value();
  }
}

static void cb_group1(Fl_Value_Input* o, void* v) {
  if (v == LOAD) {
    o->value((double)layout->group_grid_y);
  } else {
    layout->group_grid_y = (int)o->value();
  }
}

Fl_Choice *layout_choice=(Fl_Choice *)0;

Fl_Menu_Item menu_layout_choice[] = {
 {"FLTK", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 14, 0},
 {"Grid", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 14, 0},
 {0,0,0,0,0,0,0,0,0}
};

static void cb_(Fl_Button*, void* v) {
  // Clone the current layout suite

  if (v == LOAD) return;

  Fl_String old_name = "Copy of ";
  old_name.append(g_layout_list[g_layout_list.current_suite()].name);
  const char *new_name = fl_input("Enter a name for the new layout:", old_name.c_str());
  if (new_name == NULL)
    return; 

  g_layout_list.add(new_name);
  g_layout_list.update_dialogs();
}

Fl_Menu_Button *w_layout_menu=(Fl_Menu_Button *)0;

static void cb_w_layout_menu(Fl_Menu_Button*, void* v) {
  if (v == LOAD) {
    Fd_Layout_Suite &suite = g_layout_list[g_layout_list.current_suite()];
    if (suite.is_static) {
      w_layout_menu_rename->deactivate();
      w_layout_menu_user_setting->deactivate();
      w_layout_menu_delete->deactivate();
    } else {
      w_layout_menu_rename->activate();
      w_layout_menu_user_setting->activate();
      w_layout_menu_delete->activate();
    }
    w_layout_menu_user_setting->value(suite.is_user_setting);
    w_layout_menu_project_setting->value(suite.is_project_setting);
  }
}

static void cb_w_layout_menu_rename(Fl_Menu_*, void*) {
  // Rename the current layout suite

  Fl_String old_name = g_layout_list[g_layout_list.current_suite()].name;
  const char *new_name = fl_input("Enter a new name for the layout:", old_name.c_str());
  if (new_name == NULL)
    return; 

  g_layout_list.rename(new_name);
  g_layout_list.update_dialogs();
}

static void cb_w_layout_menu_user_setting(Fl_Menu_* o, void*) {
  Fd_Layout_Suite &suite = g_layout_list[g_layout_list.current_suite()];
  if (!suite.is_static)
    suite.is_user_setting = o->value();
}

static void cb_w_layout_menu_project_setting(Fl_Menu_* o, void*) {
  Fd_Layout_Suite &suite = g_layout_list[g_layout_list.current_suite()];
  suite.is_project_setting = o->value();
}

static void cb_w_layout_menu_delete(Fl_Menu_*, void*) {
  // remove the current suite

  g_layout_list.remove(g_layout_list.current_suite());
  g_layout_list.update_dialogs();
}

Fl_Menu_Item menu_w_layout_menu[] = {
 {"Rename...", 0,  (Fl_Callback*)cb_w_layout_menu_rename, 0, 128, (uchar)FL_NORMAL_LABEL, 0, 14, 0},
 {"Load...", 0,  0, 0, 1, (uchar)FL_NORMAL_LABEL, 0, 14, 0},
 {"Save...", 0,  0, 0, 129, (uchar)FL_NORMAL_LABEL, 0, 14, 0},
 {"Save in User Settings", 0,  (Fl_Callback*)cb_w_layout_menu_user_setting, 0, 2, (uchar)FL_NORMAL_LABEL, 0, 14, 0},
 {"Save in Project File", 0,  (Fl_Callback*)cb_w_layout_menu_project_setting, 0, 130, (uchar)FL_NORMAL_LABEL, 0, 14, 0},
 {"Delete", 0,  (Fl_Callback*)cb_w_layout_menu_delete, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 14, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_Button *preset_choice[3]={(Fl_Button *)0};

Fl_Double_Window* make_layout_window() {
  { grid_window = new Fl_Double_Window(520, 573, "Layout Settings");
    grid_window->callback((Fl_Callback*)cb_grid_window);
    { Fl_Button* o = new Fl_Button(435, 515, 60, 25, "Close");
      o->tooltip("Close this dialog.");
      o->callback((Fl_Callback*)cb_Close2);
    } // Fl_Button* o
    { Fl_Box* o = new Fl_Box(241, 367, 107, 25, "Widget Size:");
      o->labelfont(1);
      o->align(Fl_Align(FL_ALIGN_RIGHT|FL_ALIGN_INSIDE));
    } // Fl_Box* o
    { Fl_Group* o = new Fl_Group(305, 393, 192, 75);
      { def_widget_size[0] = new Fl_Round_Button(315, 393, 70, 25);
        def_widget_size[0]->type(102);
        def_widget_size[0]->down_box(FL_ROUND_DOWN_BOX);
        def_widget_size[0]->callback((Fl_Callback*)default_widget_size_cb, (void*)(8));
      } // Fl_Round_Button* def_widget_size[0]
      { Fl_Box* o = new Fl_Box(330, 393, 50, 25, "tiny");
        o->labelsize(8);
        o->align(Fl_Align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE));
      } // Fl_Box* o
      { def_widget_size[1] = new Fl_Round_Button(402, 393, 70, 25);
        def_widget_size[1]->type(102);
        def_widget_size[1]->down_box(FL_ROUND_DOWN_BOX);
        def_widget_size[1]->callback((Fl_Callback*)default_widget_size_cb, (void*)(11));
      } // Fl_Round_Button* def_widget_size[1]
      { Fl_Box* o = new Fl_Box(418, 393, 50, 25, "small");
        o->labelsize(11);
        o->align(Fl_Align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE));
      } // Fl_Box* o
      { def_widget_size[2] = new Fl_Round_Button(315, 418, 70, 25);
        def_widget_size[2]->type(102);
        def_widget_size[2]->down_box(FL_ROUND_DOWN_BOX);
        def_widget_size[2]->callback((Fl_Callback*)default_widget_size_cb, (void*)(14));
      } // Fl_Round_Button* def_widget_size[2]
      { Fl_Box* o = new Fl_Box(330, 418, 50, 25, "normal");
        o->align(Fl_Align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE));
      } // Fl_Box* o
      { def_widget_size[3] = new Fl_Round_Button(402, 418, 90, 25);
        def_widget_size[3]->type(102);
        def_widget_size[3]->down_box(FL_ROUND_DOWN_BOX);
        def_widget_size[3]->callback((Fl_Callback*)default_widget_size_cb, (void*)(18));
      } // Fl_Round_Button* def_widget_size[3]
      { Fl_Box* o = new Fl_Box(418, 418, 68, 25, "medium");
        o->labelsize(18);
        o->align(Fl_Align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE));
      } // Fl_Box* o
      { def_widget_size[4] = new Fl_Round_Button(315, 443, 75, 25);
        def_widget_size[4]->type(102);
        def_widget_size[4]->down_box(FL_ROUND_DOWN_BOX);
        def_widget_size[4]->callback((Fl_Callback*)default_widget_size_cb, (void*)(24));
      } // Fl_Round_Button* def_widget_size[4]
      { Fl_Box* o = new Fl_Box(330, 443, 64, 25, "large");
        o->labelsize(24);
        o->align(Fl_Align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE));
      } // Fl_Box* o
      { def_widget_size[5] = new Fl_Round_Button(402, 443, 95, 25);
        def_widget_size[5]->type(102);
        def_widget_size[5]->down_box(FL_ROUND_DOWN_BOX);
        def_widget_size[5]->callback((Fl_Callback*)default_widget_size_cb, (void*)(32));
      } // Fl_Round_Button* def_widget_size[5]
      { Fl_Box* o = new Fl_Box(418, 443, 76, 25, "huge");
        o->labelsize(32);
        o->align(Fl_Align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE));
      } // Fl_Box* o
      o->end();
    } // Fl_Group* o
    { Fl_Value_Input* o = new Fl_Value_Input(167, 82, 50, 22, "left window margin:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_left);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(167, 112, 50, 22, "right window margin:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_right);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(167, 142, 50, 22, "top window margin:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_top);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(167, 172, 50, 22, "bottom window margin:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_bottom);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(167, 202, 50, 22, "window grid x:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_window);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(167, 270, 50, 22, "widget x min:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_widget);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(167, 300, 50, 22, "widget x incr:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_widget1);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(167, 330, 50, 22, "widget x gap:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_widget2);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(167, 371, 50, 22, "widget y min:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_widget3);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(167, 401, 50, 22, "widget y incr:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_widget4);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(167, 431, 50, 22, "widget y gap:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_widget5);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(167, 232, 50, 22, "window grid y:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_window1);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(374, 82, 50, 22, "left group margin:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_left1);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(374, 112, 50, 22, "right group margin:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_right1);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(374, 142, 50, 22, "top group margin:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_top1);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(374, 172, 50, 22, "bottom group margin:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_bottom1);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(374, 270, 50, 22, "top tabs margin:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_top2);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(374, 300, 50, 22, "bottom tabs margin:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_bottom2);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(374, 202, 50, 22, "group grid x:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_group);
    } // Fl_Value_Input* o
    { Fl_Value_Input* o = new Fl_Value_Input(374, 232, 50, 22, "group grid y:");
      o->maximum(32767);
      o->step(1);
      o->callback((Fl_Callback*)cb_group1);
    } // Fl_Value_Input* o
    { Fl_Group* o = new Fl_Group(107, 479, 200, 20, "Label Font:");
      o->labelfont(1);
      o->labelsize(11);
      o->callback((Fl_Callback*)propagate_load);
      o->align(Fl_Align(FL_ALIGN_LEFT));
      { Fl_Choice* o = new Fl_Choice(107, 479, 152, 20);
        o->tooltip("The style of the label text.");
        o->box(FL_THIN_UP_BOX);
        o->down_box(FL_BORDER_BOX);
        o->labelfont(1);
        o->labelsize(11);
        o->textsize(11);
        Fl_Group::current()->resizable(o);
        o->menu(fontmenu);
      } // Fl_Choice* o
      { Fl_Value_Input* o = new Fl_Value_Input(258, 479, 49, 20);
        o->tooltip("The size of the label text.");
        o->labelsize(11);
        o->maximum(100);
        o->step(1);
        o->value(14);
        o->textsize(11);
      } // Fl_Value_Input* o
      o->end();
    } // Fl_Group* o
    { Fl_Group* o = new Fl_Group(107, 507, 200, 20, "Text Font:");
      o->labelfont(1);
      o->labelsize(11);
      o->callback((Fl_Callback*)propagate_load);
      o->align(Fl_Align(FL_ALIGN_LEFT));
      { Fl_Choice* o = new Fl_Choice(107, 507, 152, 20);
        o->tooltip("The value text style.");
        o->box(FL_DOWN_BOX);
        o->down_box(FL_BORDER_BOX);
        o->labelfont(1);
        o->labelsize(11);
        o->textsize(11);
        Fl_Group::current()->resizable(o);
        o->menu(fontmenu);
      } // Fl_Choice* o
      { Fl_Value_Input* o = new Fl_Value_Input(258, 507, 49, 20);
        o->tooltip("The value text size.");
        o->labelsize(11);
        o->maximum(100);
        o->step(1);
        o->value(14);
        o->textsize(11);
      } // Fl_Value_Input* o
      o->end();
    } // Fl_Group* o
    { layout_choice = new Fl_Choice(170, 11, 130, 24);
      layout_choice->down_box(FL_BORDER_BOX);
      layout_choice->callback((Fl_Callback*)edit_layout_suite_cb);
      layout_choice->menu(menu_layout_choice);
    } // Fl_Choice* layout_choice
    { Fl_Button* o = new Fl_Button(300, 11, 24, 24, "+");
      o->callback((Fl_Callback*)cb_);
    } // Fl_Button* o
    { w_layout_menu = new Fl_Menu_Button(324, 11, 24, 24);
      w_layout_menu->callback((Fl_Callback*)cb_w_layout_menu);
      w_layout_menu->menu(menu_w_layout_menu);
    } // Fl_Menu_Button* w_layout_menu
    { Fl_Group* o = new Fl_Group(121, 48, 270, 20);
      o->callback((Fl_Callback*)propagate_load);
      { preset_choice[0] = new Fl_Button(121, 48, 90, 20, "Application");
        preset_choice[0]->type(102);
        preset_choice[0]->value(1);
        preset_choice[0]->selection_color(FL_DARK2);
        preset_choice[0]->labelsize(12);
        preset_choice[0]->callback((Fl_Callback*)edit_layout_preset_cb);
      } // Fl_Button* preset_choice[0]
      { preset_choice[1] = new Fl_Button(211, 48, 90, 20, "Dialog");
        preset_choice[1]->type(102);
        preset_choice[1]->selection_color(FL_DARK2);
        preset_choice[1]->labelsize(12);
        preset_choice[1]->callback((Fl_Callback*)edit_layout_preset_cb);
      } // Fl_Button* preset_choice[1]
      { preset_choice[2] = new Fl_Button(301, 48, 90, 20, "Toolbox");
        preset_choice[2]->type(102);
        preset_choice[2]->selection_color(FL_DARK2);
        preset_choice[2]->labelsize(12);
        preset_choice[2]->callback((Fl_Callback*)edit_layout_preset_cb);
      } // Fl_Button* preset_choice[2]
      o->end();
    } // Fl_Group* o
    grid_window->set_non_modal();
    grid_window->end();
  } // Fl_Double_Window* grid_window
  grid_window->do_callback(grid_window, LOAD);
  return grid_window;
}
