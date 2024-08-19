//
// A simple text editor program for the Fast Light Tool Kit (FLTK).
//
// This program is described in chapter "Designing a Simple Text Editor"
// of the FLTK Programmer's Guide.
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

// Enable tutorial code for each chapter by adjusting this macro to match
// the chapter number.
#define TUTORIAL_CHAPTER 10

// ---- Tutorial Chapter 1 -----------------------------------------------------
#if TUTORIAL_CHAPTER >= 1

#include <FL/Fl_Double_Window.H>
#include <FL/Fl.H>

Fl_Double_Window *app_window = NULL;

void tut1_build_app_window() {
  app_window = new Fl_Double_Window(640, 480, "FLTK Editor");
}

#endif
#if TUTORIAL_CHAPTER == 1

int main (int argc, char **argv) {
  tut1_build_app_window();
  app_window->show(argc, argv);
  return Fl::run();
}

#endif
// ---- Tutorial Chapter 2 -----------------------------------------------------
#if TUTORIAL_CHAPTER >= 2

#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_ask.H>
#include <FL/filename.H>
#include <FL/fl_string_functions.h>

Fl_Menu_Bar *app_menu_bar = NULL;
bool text_changed = false;
char app_filename[FL_PATH_MAX] = "";

void update_title() {
  const char *fname = NULL;
  if (app_filename[0])
    fname = fl_filename_name(app_filename);
  if (fname) {
    char buf[FL_PATH_MAX + 3];
    if (text_changed) {
      snprintf(buf, FL_PATH_MAX+2, "%s *", fname);
    } else {
      snprintf(buf, FL_PATH_MAX+2, "%s", fname);
    }
    app_window->copy_label(buf);
  } else {
    app_window->label("FLTK Editor");
  }
}

void set_changed(bool v) {
  if (v != text_changed) {
    text_changed = v;
    update_title();
  }
}

void set_filename(const char *new_filename) {
  if (new_filename) {
    fl_strlcpy(app_filename, new_filename, FL_PATH_MAX);
  } else {
    app_filename[0] = 0;
  }
  update_title();
}

#if TUTORIAL_CHAPTER < 4
void menu_quit_callback(Fl_Widget *, void *) {
  if (text_changed) {
    int c = fl_choice("Changes in your text have not been saved.\n"
                      "Do you want to quit the editor anyway?",
                      "Quit", "Cancel", NULL);
    if (c == 1) return;
  }
  Fl::hide_all_windows();
}
#else
void menu_quit_callback(Fl_Widget *, void *);
#endif

void tut2_build_app_menu_bar() {
  app_window->begin();
  app_menu_bar = new Fl_Menu_Bar(0, 0, app_window->w(), 25);
  app_menu_bar->add("File/Quit Editor", FL_COMMAND|'q', menu_quit_callback);
  app_window->callback(menu_quit_callback);
  app_window->end();
}

#endif
#if TUTORIAL_CHAPTER == 2

int main (int argc, char **argv) {
  tut1_build_app_window();
  tut2_build_app_menu_bar();
  app_window->show(argc, argv);
  return Fl::run();
}

#endif
// ---- Tutorial Chapter 3 -----------------------------------------------------
#if TUTORIAL_CHAPTER >= 3

#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>

Fl_Text_Editor *app_editor = NULL;
Fl_Text_Editor *app_split_editor = NULL; // for later
Fl_Text_Buffer *app_text_buffer = NULL;

void text_changed_callback(int, int n_inserted, int n_deleted, int, const char*, void*) {
  if (n_inserted || n_deleted)
    set_changed(true);
}

void menu_new_callback(Fl_Widget*, void*) {
  if (text_changed) {
    int c = fl_choice("Changes in your text have not been saved.\n"
                      "Do you want to start a new text anyway?",
                      "New", "Cancel", NULL);
    if (c == 1) return;
  }
  app_text_buffer->text("");
  set_filename(NULL);
  set_changed(false);
}

void tut3_build_main_editor() {
  app_window->begin();
  app_text_buffer = new Fl_Text_Buffer();
  app_text_buffer->add_modify_callback(text_changed_callback, NULL);
  app_editor = new Fl_Text_Editor(0, app_menu_bar->h(),
    app_window->w(), app_window->h() - app_menu_bar->h());
  app_editor->buffer(app_text_buffer);
  app_editor->textfont(FL_COURIER);
  app_window->resizable(app_editor);
  app_window->end();
  // find the Quit menu and insert the New menu there
  int ix = app_menu_bar->find_index(menu_quit_callback);
  app_menu_bar->insert(ix, "New", FL_COMMAND+'n', menu_new_callback);
}

#endif
#if TUTORIAL_CHAPTER == 3

int main (int argc, char **argv) {
  tut1_build_app_window();
  tut2_build_app_menu_bar();
  tut3_build_main_editor();
  app_window->show(argc, argv);
  return Fl::run();
}

#endif
// ---- Tutorial Chapter 4 -----------------------------------------------------
#if TUTORIAL_CHAPTER >= 4

#include <FL/Fl_Native_File_Chooser.H>
#include <FL/platform.H>
#include <errno.h>

void menu_save_as_callback(Fl_Widget*, void*) {
  Fl_Native_File_Chooser file_chooser;
  file_chooser.title("Save File As...");
  file_chooser.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
  if (app_filename[0]) {
    char temp_filename[FL_PATH_MAX];
    fl_strlcpy(temp_filename, app_filename, FL_PATH_MAX);
    const char *name = fl_filename_name(temp_filename);
    if (name) {
      file_chooser.preset_file(name);
      temp_filename[name - temp_filename] = 0;
      file_chooser.directory(temp_filename);
    }
  }
  if (file_chooser.show() == 0) {
    if (app_text_buffer->savefile(file_chooser.filename()) == 0) {
      set_filename(file_chooser.filename());
      set_changed(false);
    } else {
      fl_alert("Failed to save file\n%s\n%s",
               file_chooser.filename(),
               strerror(errno));
    }
  }
}

void menu_save_callback(Fl_Widget*, void*) {
  if (!app_filename[0]) {
    menu_save_as_callback(NULL, NULL);
  } else {
    if (app_text_buffer->savefile(app_filename) == 0) {
      set_changed(false);
    } else {
      fl_alert("Failed to save file\n%s\n%s",
               app_filename,
               strerror(errno));
    }
  }
}

void menu_quit_callback(Fl_Widget *, void *) {
  if (text_changed) {
      int r = fl_choice("The current file has not been saved.\n"
                        "Would you like to save it now?",
                        "Cancel", "Save", "Don't Save");
      if (r == 0)   // cancel
        return;
      if (r == 1) { // save
        menu_save_callback(NULL, NULL);
        return;
      }
  }
  Fl::hide_all_windows();
}

void load(const char *filename) {
  if (app_text_buffer->loadfile(filename) == 0) {
    set_filename(filename);
    set_changed(false);
  } else {
    fl_alert("Failed to load file\n%s\n%s",
             filename,
             strerror(errno));
  }
}

void menu_open_callback(Fl_Widget*, void*) {
  if (text_changed) {
    int r = fl_choice("The current file has not been saved.\n"
                      "Would you like to save it now?",
                      "Cancel", "Save", "Don't Save");
    if (r == 0) // cancel
      return;
    if (r == 1) // save
      menu_save_callback(NULL, NULL);
  }
  Fl_Native_File_Chooser file_chooser;
  file_chooser.title("Open File...");
  file_chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
  if (app_filename[0]) {
    char temp_filename[FL_PATH_MAX];
    fl_strlcpy(temp_filename, app_filename, FL_PATH_MAX);
    const char *name = fl_filename_name(temp_filename);
    if (name) {
      file_chooser.preset_file(name);
      temp_filename[name - temp_filename] = 0;
      file_chooser.directory(temp_filename);
    }
  }
  if (file_chooser.show() == 0)
    load(file_chooser.filename());
}

void tut4_add_file_support() {
  int ix = app_menu_bar->find_index(menu_quit_callback);
  app_menu_bar->insert(ix, "Open", FL_COMMAND+'o', menu_open_callback, NULL, FL_MENU_DIVIDER);
  app_menu_bar->insert(ix+1, "Save", FL_COMMAND+'s', menu_save_callback);
  app_menu_bar->insert(ix+2, "Save as...", FL_COMMAND+'S', menu_save_as_callback, NULL, FL_MENU_DIVIDER);
}

int args_handler(int argc, char **argv, int &i) {
  if (argv && argv[i] && argv[i][0]!='-') {
    load(argv[i]);
    i++;
    return 1;
  }
  return 0;
}

int tut4_handle_commandline_and_run(int &argc, char **argv) {
  int i = 0;
  Fl::args_to_utf8(argc, argv);
  Fl::args(argc, argv, i, args_handler);
  fl_open_callback(load);
  app_window->show(argc, argv);
  return Fl::run();
}

#endif
#if TUTORIAL_CHAPTER == 4

int main (int argc, char **argv) {
  tut1_build_app_window();
  tut2_build_app_menu_bar();
  tut3_build_main_editor();
  tut4_add_file_support();
  return tut4_handle_commandline_and_run(argc, argv);
}

#endif
// ---- Tutorial Chapter 5 -----------------------------------------------------
#if TUTORIAL_CHAPTER >= 5

void menu_undo_callback(Fl_Widget*, void* v) {
  Fl_Widget *e = Fl::focus();
  if (e && (e == app_editor || e == app_split_editor))
    Fl_Text_Editor::kf_undo(0, (Fl_Text_Editor*)e);
}

void menu_redo_callback(Fl_Widget*, void* v) {
  Fl_Widget *e = Fl::focus();
  if (e && (e == app_editor || e == app_split_editor))
    Fl_Text_Editor::kf_redo(0, (Fl_Text_Editor*)e);
}

void menu_cut_callback(Fl_Widget*, void* v) {
  Fl_Widget *e = Fl::focus();
  if (e && (e == app_editor || e == app_split_editor))
    Fl_Text_Editor::kf_cut(0, (Fl_Text_Editor*)e);
}

void menu_copy_callback(Fl_Widget*, void* v) {
  Fl_Widget *e = Fl::focus();
  if (e && (e == app_editor || e == app_split_editor))
    Fl_Text_Editor::kf_copy(0, (Fl_Text_Editor*)e);
}

void menu_paste_callback(Fl_Widget*, void* v) {
  Fl_Widget *e = Fl::focus();
  if (e && (e == app_editor || e == app_split_editor))
    Fl_Text_Editor::kf_paste(0, (Fl_Text_Editor*)e);
}

void menu_delete_callback(Fl_Widget*, void*) {
  Fl_Widget *e = Fl::focus();
  if (e && (e == app_editor || e == app_split_editor))
    Fl_Text_Editor::kf_delete(0, (Fl_Text_Editor*)e);
}

void tut5_cut_copy_paste() {
  app_menu_bar->add("Edit/Undo",   FL_COMMAND+'z', menu_undo_callback);
  app_menu_bar->add("Edit/Redo",   FL_COMMAND+'Z', menu_redo_callback, NULL, FL_MENU_DIVIDER);
  app_menu_bar->add("Edit/Cut",    FL_COMMAND+'x', menu_cut_callback);
  app_menu_bar->add("Edit/Copy",   FL_COMMAND+'c', menu_copy_callback);
  app_menu_bar->add("Edit/Paste",  FL_COMMAND+'v', menu_paste_callback);
  app_menu_bar->add("Edit/Delete", 0,              menu_delete_callback);
}

#endif
#if TUTORIAL_CHAPTER == 5

int main (int argc, char **argv) {
  tut1_build_app_window();
  tut2_build_app_menu_bar();
  tut3_build_main_editor();
  tut4_add_file_support();
  tut5_cut_copy_paste();
  return tut4_handle_commandline_and_run(argc, argv);
}

#endif
// ---- Tutorial Chapter 6 -----------------------------------------------------
#if TUTORIAL_CHAPTER >= 6

char last_find_text[1024] = "";

bool find_next(const char *needle) {
  Fl_Text_Editor *editor = app_editor;
  Fl_Widget *e = Fl::focus();
  if (e && e == app_split_editor)
    editor = app_split_editor;
  int pos = editor->insert_position();
  int found = app_text_buffer->search_forward(pos, needle, &pos);
  if (found) {
    app_text_buffer->select(pos, pos + (int)strlen(needle));
    editor->insert_position(pos + (int)strlen(needle));
    editor->show_insert_position();
    return true;
  } else {
    fl_alert("No further occurrences of '%s' found!", needle);
    return false;
  }
}

void menu_find_callback(Fl_Widget*, void* v) {
  const char *find_text = fl_input("Find in text:", last_find_text);
  if (find_text) {
    fl_strlcpy(last_find_text, find_text, sizeof(last_find_text));
    find_next(find_text);
  }
}

void menu_find_next_callback(Fl_Widget*, void* v) {
  if (last_find_text[0]) {
    find_next(last_find_text);
  } else {
    menu_find_callback(NULL, NULL);
  }
}

void tut6_implement_find() {
  app_menu_bar->add("Find/Find...",   FL_COMMAND+'f', menu_find_callback);
  app_menu_bar->add("Find/Find Next", FL_COMMAND+'g', menu_find_next_callback, NULL, FL_MENU_DIVIDER);
}

#endif
#if TUTORIAL_CHAPTER == 6

int main (int argc, char **argv) {
  tut1_build_app_window();
  tut2_build_app_menu_bar();
  tut3_build_main_editor();
  tut4_add_file_support();
  tut5_cut_copy_paste();
  tut6_implement_find();
  return tut4_handle_commandline_and_run(argc, argv);
}

#endif
// ---- Tutorial Chapter 7 -----------------------------------------------------
#if TUTORIAL_CHAPTER >= 7

#include <FL/Fl_Flex.H>

char last_replace_text[1024] = "";

void replace_selection(const char *new_text) {
  Fl_Text_Editor *editor = app_editor;
  Fl_Widget *e = Fl::focus();
  if (e && e == app_split_editor)
    editor = app_split_editor;
  int start, end;
  if (app_text_buffer->selection_position(&start, &end)) {
    app_text_buffer->remove_selection();
    app_text_buffer->insert(start, new_text);
    app_text_buffer->select(start, start + (int)strlen(new_text));
    editor->insert_position(start + (int)strlen(new_text));
    editor->show_insert_position();
  }
}

class Replace_Dialog : public Fl_Double_Window {
  Fl_Input *find_text_input;
  Fl_Input *replace_text_input;
  Fl_Button *find_next_button;
  Fl_Button *replace_and_find_button;
  Fl_Button *close_button;
public:
  Replace_Dialog(const char *label);
  void show() FL_OVERRIDE;
private:
  static void find_next_callback(Fl_Widget*, void*);
  static void replace_and_find_callback(Fl_Widget*, void*);
  static void close_callback(Fl_Widget*, void*);
};

Replace_Dialog *replace_dialog = NULL;

Replace_Dialog::Replace_Dialog(const char *label)
: Fl_Double_Window(430, 110, label)
{
  find_text_input = new Fl_Input(100, 10, 320, 25, "Find:");
  replace_text_input = new Fl_Input(100, 40, 320, 25, "Replace:");
  Fl_Flex* button_field = new Fl_Flex(100, 70, w()-100, 40);
  button_field->type(Fl_Flex::HORIZONTAL);
  button_field->margin(0, 5, 10, 10);
  button_field->gap(10);
  find_next_button = new Fl_Button(0, 0, 0, 0, "Next");
  find_next_button->callback(find_next_callback, this);
  replace_and_find_button = new Fl_Button(0, 0, 0, 0, "Replace");
  replace_and_find_button->callback(replace_and_find_callback, this);
  close_button = new Fl_Button(0, 0, 0, 0, "Close");
  close_button->callback(close_callback, this);
  button_field->end();
  set_non_modal();
}

void Replace_Dialog::show() {
  find_text_input->value(last_find_text);
  replace_text_input->value(last_replace_text);
  Fl_Double_Window::show();
}

void Replace_Dialog::find_next_callback(Fl_Widget*, void* my_dialog) {
  Replace_Dialog *dlg = static_cast<Replace_Dialog*>(my_dialog);
  fl_strlcpy(last_find_text, dlg->find_text_input->value(), sizeof(last_find_text));
  fl_strlcpy(last_replace_text, dlg->replace_text_input->value(), sizeof(last_replace_text));
  if (last_find_text[0])
    find_next(last_find_text);
}

void Replace_Dialog::replace_and_find_callback(Fl_Widget*, void* my_dialog) {
  Replace_Dialog *dlg = static_cast<Replace_Dialog*>(my_dialog);
  replace_selection(dlg->replace_text_input->value());
  find_next_callback(NULL, my_dialog);
}

void Replace_Dialog::close_callback(Fl_Widget*, void* my_dialog) {
  Replace_Dialog *dlg = static_cast<Replace_Dialog*>(my_dialog);
  dlg->hide();
}

void menu_replace_callback(Fl_Widget*, void*) {
  if (!replace_dialog)
    replace_dialog = new Replace_Dialog("Find and Replace");
  replace_dialog->show();
}

void menu_replace_next_callback(Fl_Widget*, void*) {
  if (!last_find_text[0]) {
    menu_replace_callback(NULL, NULL);
  } else {
    replace_selection(last_replace_text);
    find_next(last_find_text);
  }
}

void tut7_implement_replace() {
  app_menu_bar->add("Find/Replace...",   FL_COMMAND+'r', menu_replace_callback);
  app_menu_bar->add("Find/Replace Next", FL_COMMAND+'t', menu_replace_next_callback);
}

#endif
#if TUTORIAL_CHAPTER == 7

int main (int argc, char **argv) {
  tut1_build_app_window();
  tut2_build_app_menu_bar();
  tut3_build_main_editor();
  tut4_add_file_support();
  tut5_cut_copy_paste();
  tut6_implement_find();
  tut7_implement_replace();
  return tut4_handle_commandline_and_run(argc, argv);
}

#endif
// ---- Tutorial Chapter 8 -----------------------------------------------------
#if TUTORIAL_CHAPTER >= 8

#include <FL/Fl_Menu_Item.H>

void menu_linenumbers_callback(Fl_Widget* w, void*) {
  Fl_Menu_Bar* menu = static_cast<Fl_Menu_Bar*>(w);
  const Fl_Menu_Item* linenumber_item = menu->mvalue();
  if (linenumber_item->value()) {
    app_editor->linenumber_width(40);
  } else {
    app_editor->linenumber_width(0);
  }
  app_editor->redraw();
  if (app_split_editor) {
    if (linenumber_item->value()) {
      app_split_editor->linenumber_width(40);
    } else {
      app_split_editor->linenumber_width(0);
    }
    app_split_editor->redraw();
  }
}

void menu_wordwrap_callback(Fl_Widget* w, void*) {
  Fl_Menu_Bar* menu = static_cast<Fl_Menu_Bar*>(w);
  const Fl_Menu_Item* wordwrap_item = menu->mvalue();
  if (wordwrap_item->value()) {
    app_editor->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
  } else {
    app_editor->wrap_mode(Fl_Text_Display::WRAP_NONE, 0);
  }
  app_editor->redraw();
  if (app_split_editor) {
    if (wordwrap_item->value()) {
      app_split_editor->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
    } else {
      app_split_editor->wrap_mode(Fl_Text_Display::WRAP_NONE, 0);
    }
    app_split_editor->redraw();
  }
}

void tut8_editor_features() {
  app_menu_bar->add("Window/Line Numbers", FL_COMMAND+'l', menu_linenumbers_callback, NULL, FL_MENU_TOGGLE);
  app_menu_bar->add("Window/Word Wrap", 0, menu_wordwrap_callback, NULL, FL_MENU_TOGGLE);
}

#endif
#if TUTORIAL_CHAPTER == 8

int main (int argc, char **argv) {
  tut1_build_app_window();
  tut2_build_app_menu_bar();
  tut3_build_main_editor();
  tut4_add_file_support();
  tut5_cut_copy_paste();
  tut6_implement_find();
  tut7_implement_replace();
  tut8_editor_features();
  return tut4_handle_commandline_and_run(argc, argv);
}

#endif
// ---- Tutorial Chapter 9 -----------------------------------------------------
#if TUTORIAL_CHAPTER >= 9

#include <FL/Fl_Tile.H>

Fl_Tile *app_tile = NULL;

void menu_split_callback(Fl_Widget* w, void*) {
  Fl_Menu_Bar* menu = static_cast<Fl_Menu_Bar*>(w);
  const Fl_Menu_Item* splitview_item = menu->mvalue();
  if (splitview_item->value()) {
    int h_split = app_tile->h()/2;
    app_editor->size(app_tile->w(), h_split);
    app_split_editor->resize(app_tile->x(), app_tile->y() + h_split,
                             app_tile->w(), app_tile->h() - h_split);
    app_split_editor->show();
  } else {
    app_editor->size(app_tile->w(), app_tile->h());
    app_split_editor->resize(app_tile->x(), app_tile->y()+app_tile->h(),
                             app_tile->w(), 0);
    app_split_editor->hide();
  }
  app_tile->resizable(app_editor);
  app_tile->init_sizes();
  app_tile->redraw();
}

void tut9_split_editor() {
  app_window->begin();
  app_tile = new Fl_Tile(app_editor->x(), app_editor->y(),
                         app_editor->w(), app_editor->h());
  app_window->remove(app_editor);
  app_tile->add(app_editor);
  app_split_editor = new Fl_Text_Editor(app_tile->x(), app_tile->y()+app_tile->h(),
                                        app_tile->w(), 0);
  app_split_editor->buffer(app_text_buffer);
  app_split_editor->textfont(FL_COURIER);
  app_split_editor->hide();
  app_tile->end();
  app_tile->size_range(0, 25, 25);
  app_tile->size_range(1, 25, 25);
  app_tile->init_sizes();
  app_window->end();
  app_window->resizable(app_tile);
  app_tile->resizable(app_editor);
  app_menu_bar->add("Window/Split", FL_COMMAND+'i', menu_split_callback, NULL, FL_MENU_TOGGLE);
}

#endif
#if TUTORIAL_CHAPTER == 9

int main (int argc, char **argv) {
  tut1_build_app_window();
  tut2_build_app_menu_bar();
  tut3_build_main_editor();
  tut4_add_file_support();
  tut5_cut_copy_paste();
  tut6_implement_find();
  tut7_implement_replace();
  tut8_editor_features();
  tut9_split_editor();
  return tut4_handle_commandline_and_run(argc, argv);
}

#endif
// ---- Tutorial Chapter 10 ----------------------------------------------------
#if TUTORIAL_CHAPTER >= 10

#include <ctype.h>
#include <stdlib.h>

Fl_Text_Buffer *app_style_buffer = NULL;

// Syntax highlighting stuff...
#define TS 14 // default editor textsize
Fl_Text_Display::Style_Table_Entry
                   styletable[] = {     // Style table
#ifdef TESTING_ATTRIBUTES
                     { FL_BLACK,      FL_COURIER,           TS }, // A - Plain
                     { FL_DARK_GREEN, FL_HELVETICA_ITALIC,  TS, Fl_Text_Display::ATTR_BGCOLOR, FL_LIGHT2  }, // B - Line comments
                     { FL_DARK_GREEN, FL_HELVETICA_ITALIC,  TS, Fl_Text_Display::ATTR_BGCOLOR_EXT, FL_LIGHT2 }, // C - Block comments
                     { FL_BLUE,       FL_COURIER,           TS, Fl_Text_Display::ATTR_UNDERLINE }, // D - Strings
                     { FL_DARK_RED,   FL_COURIER,           TS, Fl_Text_Display::ATTR_GRAMMAR }, // E - Directives
                     { FL_DARK_RED,   FL_COURIER_BOLD,      TS, Fl_Text_Display::ATTR_STRIKE_THROUGH }, // F - Types
                     { FL_BLUE,       FL_COURIER_BOLD,      TS, Fl_Text_Display::ATTR_SPELLING }, // G - Keywords
#else
                     { FL_BLACK,      FL_COURIER,           TS }, // A - Plain
                     { FL_DARK_GREEN, FL_HELVETICA_ITALIC,  TS }, // B - Line comments
                     { FL_DARK_GREEN, FL_HELVETICA_ITALIC,  TS }, // C - Block comments
                     { FL_BLUE,       FL_COURIER,           TS }, // D - Strings
                     { FL_DARK_RED,   FL_COURIER,           TS }, // E - Directives
                     { FL_DARK_RED,   FL_COURIER_BOLD,      TS }, // F - Types
                     { FL_BLUE,       FL_COURIER_BOLD,      TS }, // G - Keywords
#endif
                   };
const char         *code_keywords[] = { // List of known C/C++ keywords...
                     "and",
                     "and_eq",
                     "asm",
                     "bitand",
                     "bitor",
                     "break",
                     "case",
                     "catch",
                     "compl",
                     "continue",
                     "default",
                     "delete",
                     "do",
                     "else",
                     "false",
                     "for",
                     "goto",
                     "if",
                     "new",
                     "not",
                     "not_eq",
                     "operator",
                     "or",
                     "or_eq",
                     "return",
                     "switch",
                     "template",
                     "this",
                     "throw",
                     "true",
                     "try",
                     "while",
                     "xor",
                     "xor_eq"
                   };
const char         *code_types[] = {    // List of known C/C++ types...
                     "auto",
                     "bool",
                     "char",
                     "class",
                     "const",
                     "const_cast",
                     "double",
                     "dynamic_cast",
                     "enum",
                     "explicit",
                     "extern",
                     "float",
                     "friend",
                     "inline",
                     "int",
                     "long",
                     "mutable",
                     "namespace",
                     "private",
                     "protected",
                     "public",
                     "register",
                     "short",
                     "signed",
                     "sizeof",
                     "static",
                     "static_cast",
                     "struct",
                     "template",
                     "typedef",
                     "typename",
                     "union",
                     "unsigned",
                     "virtual",
                     "void",
                     "volatile"
                   };


//
// 'compare_keywords()' - Compare two keywords...
//

extern "C" {
  int
  compare_keywords(const void *a,
                   const void *b) {
    return (strcmp(*((const char **)a), *((const char **)b)));
  }
}

//
// 'style_parse()' - Parse text and produce style data.
//

void
style_parse(const char *text,
            char       *style,
            int        length) {
  char       current;
  int        col;
  int        last;
  char       buf[255],
             *bufptr;
  const char *temp;

  // Style letters:
  //
  // A - Plain
  // B - Line comments
  // C - Block comments
  // D - Strings
  // E - Directives
  // F - Types
  // G - Keywords

  for (current = *style, col = 0, last = 0; length > 0; length --, text ++) {
    if (current == 'B' || current == 'F' || current == 'G') current = 'A';
    if (current == 'A') {
      // Check for directives, comments, strings, and keywords...
      if (col == 0 && *text == '#') {
        // Set style to directive
        current = 'E';
      } else if (strncmp(text, "//", 2) == 0) {
        current = 'B';
        for (; length > 0 && *text != '\n'; length --, text ++) *style++ = 'B';
        if (length == 0) break;
      } else if (strncmp(text, "/*", 2) == 0) {
        current = 'C';
      } else if (strncmp(text, "\\\"", 2) == 0) {
        // Quoted quote...
        *style++ = current;
        *style++ = current;
        text ++;
        length --;
        col += 2;
        continue;
      } else if (*text == '\"') {
        current = 'D';
      } else if (!last && (islower((*text)&255) || *text == '_')) {
        // Might be a keyword...
        for (temp = text, bufptr = buf;
             (islower((*temp)&255) || *temp == '_') && bufptr < (buf + sizeof(buf) - 1);
             *bufptr++ = *temp++) {
          // nothing
        }

        if (!islower((*temp)&255) && *temp != '_') {
          *bufptr = '\0';

          bufptr = buf;

          if (bsearch(&bufptr, code_types,
                      sizeof(code_types) / sizeof(code_types[0]),
                      sizeof(code_types[0]), compare_keywords)) {
            while (text < temp) {
              *style++ = 'F';
              text ++;
              length --;
              col ++;
            }

            text --;
            length ++;
            last = 1;
            continue;
          } else if (bsearch(&bufptr, code_keywords,
                             sizeof(code_keywords) / sizeof(code_keywords[0]),
                             sizeof(code_keywords[0]), compare_keywords)) {
            while (text < temp) {
              *style++ = 'G';
              text ++;
              length --;
              col ++;
            }

            text --;
            length ++;
            last = 1;
            continue;
          }
        }
      }
    } else if (current == 'C' && strncmp(text, "*/", 2) == 0) {
      // Close a C comment...
      *style++ = current;
      *style++ = current;
      text ++;
      length --;
      current = 'A';
      col += 2;
      continue;
    } else if (current == 'D') {
      // Continuing in string...
      if (strncmp(text, "\\\"", 2) == 0) {
        // Quoted end quote...
        *style++ = current;
        *style++ = current;
        text ++;
        length --;
        col += 2;
        continue;
      } else if (*text == '\"') {
        // End quote...
        *style++ = current;
        col ++;
        current = 'A';
        continue;
      }
    }

    // Copy style info...
    if (current == 'A' && (*text == '{' || *text == '}')) *style++ = 'G';
    else *style++ = current;
    col ++;

    last = isalnum((*text)&255) || *text == '_' || *text == '.';

    if (*text == '\n') {
      // Reset column and possibly reset the style
      col = 0;
      if (current == 'B' || current == 'E') current = 'A';
    }
  }
}


//
// 'style_init()' - Initialize the style buffer...
//

void
style_init(void) {
  char *style = new char[app_text_buffer->length() + 1];
  char *text = app_text_buffer->text();

  memset(style, 'A', app_text_buffer->length());
  style[app_text_buffer->length()] = '\0';

  if (!app_style_buffer) app_style_buffer = new Fl_Text_Buffer(app_text_buffer->length());

  style_parse(text, style, app_text_buffer->length());

  app_style_buffer->text(style);
  delete[] style;
  free(text);
}


//
// 'style_unfinished_cb()' - Update unfinished styles.
//

void
style_unfinished_cb(int, void*) {
}


//
// 'style_update()' - Update the style buffer...
//

void
style_update(int        pos,            // I - Position of update
             int        nInserted,      // I - Number of inserted chars
             int        nDeleted,       // I - Number of deleted chars
             int        /*nRestyled*/,  // I - Number of restyled chars
             const char * /*deletedText*/,// I - Text that was deleted
             void       *cbArg) {       // I - Callback data
  int   start,                          // Start of text
        end;                            // End of text
  char  last,                           // Last style on line
        *style,                         // Style data
        *text;                          // Text data


  // If this is just a selection change, just unselect the style buffer...
  if (nInserted == 0 && nDeleted == 0) {
    app_style_buffer->unselect();
    return;
  }

  // Track changes in the text buffer...
  if (nInserted > 0) {
    // Insert characters into the style buffer...
    style = new char[nInserted + 1];
    memset(style, 'A', nInserted);
    style[nInserted] = '\0';

    app_style_buffer->replace(pos, pos + nDeleted, style);
    delete[] style;
  } else {
    // Just delete characters in the style buffer...
    app_style_buffer->remove(pos, pos + nDeleted);
  }

  // Select the area that was just updated to avoid unnecessary
  // callbacks...
  app_style_buffer->select(pos, pos + nInserted - nDeleted);

  // Re-parse the changed region; we do this by parsing from the
  // beginning of the previous line of the changed region to the end of
  // the line of the changed region...  Then we check the last
  // style character and keep updating if we have a multi-line
  // comment character...
  start = app_text_buffer->line_start(pos);
//  if (start > 0) start = app_text_buffer->line_start(start - 1);
  end   = app_text_buffer->line_end(pos + nInserted);
  text  = app_text_buffer->text_range(start, end);
  style = app_style_buffer->text_range(start, end);
  if (start==end)
    last = 0;
  else
    last  = style[end - start - 1];

//  printf("start = %d, end = %d, text = \"%s\", style = \"%s\", last='%c'...\n",
//         start, end, text, style, last);

  style_parse(text, style, end - start);

//  printf("new style = \"%s\", new last='%c'...\n",
//         style, style[end - start - 1]);

  app_style_buffer->replace(start, end, style);
  ((Fl_Text_Editor *)cbArg)->redisplay_range(start, end);

  if (start==end || last != style[end - start - 1]) {
//    printf("Recalculate the rest of the buffer style\n");
    // Either the user deleted some text, or the last character
    // on the line changed styles, so reparse the
    // remainder of the buffer...
    free(text);
    free(style);

    end   = app_text_buffer->length();
    text  = app_text_buffer->text_range(start, end);
    style = app_style_buffer->text_range(start, end);

    style_parse(text, style, end - start);

    app_style_buffer->replace(start, end, style);
    ((Fl_Text_Editor *)cbArg)->redisplay_range(start, end);
  }

  free(text);
  free(style);
}

void menu_syntaxhighlight_callback(Fl_Widget* w, void*) {
  Fl_Menu_Bar* menu = static_cast<Fl_Menu_Bar*>(w);
  const Fl_Menu_Item* syntaxt_item = menu->mvalue();
  if (syntaxt_item->value()) {
    style_init();
    app_editor->highlight_data(app_style_buffer, styletable,
                               sizeof(styletable) / sizeof(styletable[0]),
                               'A', style_unfinished_cb, 0);
    app_text_buffer->add_modify_callback(style_update, app_editor);
  } else {
    app_text_buffer->remove_modify_callback(style_update, app_editor);
    app_editor->highlight_data(NULL, NULL, 0, 'A', NULL, 0);
  }
  app_editor->redraw();
  if (app_split_editor) {
    if (syntaxt_item->value()) {
      app_split_editor->highlight_data(app_style_buffer, styletable,
                                 sizeof(styletable) / sizeof(styletable[0]),
                                 'A', style_unfinished_cb, 0);
    } else {
      app_split_editor->highlight_data(NULL, NULL, 0, 'A', NULL, 0);
    }
    app_split_editor->redraw();
  }
}

void tut10_syntax_highlighting() {
  app_menu_bar->add("Window/Syntax Highlighting", 0, menu_syntaxhighlight_callback, NULL, FL_MENU_TOGGLE);
}

#endif
#if TUTORIAL_CHAPTER == 10

int main (int argc, char **argv) {
  tut1_build_app_window();
  tut2_build_app_menu_bar();
  tut3_build_main_editor();
  tut4_add_file_support();
  tut5_cut_copy_paste();
  tut6_implement_find();
  tut7_implement_replace();
  tut8_editor_features();
  tut9_split_editor();
  tut10_syntax_highlighting();
  return tut4_handle_commandline_and_run(argc, argv);
}

#endif
