//
// "$Id: editor.cxx,v 1.2.2.3 2001/01/22 15:13:41 easysw Exp $"
//
// A simple text editor program for the Fast Light Tool Kit (FLTK).
//
// This program is described in Chapter 4 of the FLTK Programmer's Guide.
//
// Copyright 1998-2001 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

//
// Include necessary headers...
//

#include <stdio.h>			// Standard library files
#include <stdlib.h>
#include <string.h>

#include <FL/Fl.H>			// Main FLTK header file
#include <FL/Fl_Group.H>		// Fl_Group header file
#include <FL/Fl_Window.H>		// Fl_Window header file
#include <FL/fl_ask.H>			// FLTK convenience functions
#include <FL/fl_file_chooser.H>		// FLTK file chooser
#include <FL/Fl_Menu_Bar.H>		// Fl_Menu_Bar header file
#include <FL/Fl_Input.H>		// Fl_Input header file
#include <FL/Fl_Multiline_Input.H>	// Fl_Multiline_Input header file
#include <FL/Fl_Button.H>		// Fl_Button header file
#include <FL/Fl_Return_Button.H>	// Fl_Return_Button header file


Fl_Window          *window;
Fl_Menu_Bar        *menubar;
Fl_Multiline_Input *input;
Fl_Window          *replace_dlg;
Fl_Input           *replace_find;
Fl_Input           *replace_with;
Fl_Button          *replace_all;
Fl_Return_Button   *replace_next;
Fl_Button          *replace_cancel;

int                changed = 0;
char               filename[1024] = "";
char               search[256] = "";


void set_changed(int);
void save_cb(void);
void saveas_cb(void);
void find2_cb(void);

int check_save(void) {
  if (!changed) return 1;

  if (fl_ask("The current file has not been saved.\n"
             "Would you like to save it now?")) {
    // Save the file...
    save_cb();

    return !changed;
  }
  else return (1);
}

void load_file(char *newfile) {
  FILE *fp;
  char buffer[8192];
  int  nbytes;
  int  pos;

  input->value("");

  fp = fopen(newfile, "r");
  if (fp != NULL) {
    // Was able to open file; let's read from it...
    strcpy(filename, newfile);
    pos = 0;

    while ((nbytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
      input->replace(pos, pos, buffer, nbytes);
      pos += nbytes;
    }

    fclose(fp);
    input->position(0);
    set_changed(0);
  } else {
    // Couldn't open file - say so...
    fl_alert("Unable to open \'%s\' for reading!");
  }
}

void save_file(char *newfile) {
  FILE *fp;

  fp = fopen(newfile, "w");
  if (fp != NULL) {
    // Was able to create file; let's write to it...
    strcpy(filename, newfile);

    if (fwrite(input->value(), 1, input->size(), fp) < 1) {
      fl_alert("Unable to write file!");
      fclose(fp);
      return;
    }

    fclose(fp);
    changed = 1;
    set_changed(0);
  } else {
    // Couldn't open file - say so...
    fl_alert("Unable to create \'%s\' for writing!");
  }
}

void set_changed(int c) {
  if (c != changed) {
    char title[1024];
    char *slash;

    changed = c;

    if (filename[0] == '\0') strcpy(title, "Untitled");
    else {
      slash = strrchr(filename, '/');
      if (slash == NULL) slash = strrchr(filename, '\\');

      if (slash != NULL) strcpy(title, slash + 1);
      else strcpy(title, filename);
    }

    if (changed) strcat(title, " (modified)");

    window->label(title);
  }
}

void changed_cb(void) {
  set_changed(1);
}

void copy_cb(void) {
  input->copy();
}

void cut_cb(void) {
  input->copy();
  input->cut();
}

void delete_cb(void) {
  input->cut();
}

void find_cb(void) {
  const char *val;

  val = fl_input("Search String:", search);
  if (val != NULL) {
    // User entered a string - go find it!
    strcpy(search, val);
    find2_cb();
  }
}

void find2_cb(void) {
  const char *val, *found;
  int pos;

  if (search[0] == '\0') {
    // Search string is blank; get a new one...
    find_cb();
    return;
  }

  val   = input->value() + input->mark();
  found = strstr(val, search);

  if (found != NULL) {
    // Found a match; update the position and mark...
    pos = input->mark() + found - val;
    input->position(pos, pos + strlen(search));
  }
  else fl_alert("No occurrences of \'%s\' found!", search);
}

void new_cb(void) {
  if (changed)
    if (!check_save()) return;

  filename[0] = '\0';
  input->value("");
  set_changed(0);
}

void open_cb(void) {
  char *newfile;

  if (changed)
    if (!check_save()) return;

  newfile = fl_file_chooser("Open File?", "*", filename);
  if (newfile != NULL) load_file(newfile);
}

void paste_cb(void) {
  Fl::paste(*input);
}

void quit_cb(void) {
  if (changed)
    if (!check_save())
      return;

  window->hide();
}

void replace_cb(void) {
  replace_dlg->show();
}

void replace2_cb() {
  const char *find, *val, *found;
  int pos;

  find = replace_find->value();
  if (find[0] == '\0') {
    // Search string is blank; get a new one...
    replace_dlg->show();
    return;
  }

  replace_dlg->hide();

  val   = input->value() + input->position();
  found = strstr(val, find);

  if (found != NULL) {
    // Found a match; update the position and replace text...
    pos = input->position() + found - val;
    input->replace(pos, pos + strlen(find), replace_with->value());
    input->position(pos + strlen(replace_with->value()));
  }
  else fl_alert("No occurrences of \'%s\' found!", find);
}

void replall_cb() {
  const char *find, *val, *found;
  int pos;
  int times;

  find = replace_find->value();
  if (find[0] == '\0') {
    // Search string is blank; get a new one...
    replace_dlg->show();
    return;
  }

  replace_dlg->hide();

  input->position(0);
  times = 0;

  // Loop through the whole string
  do {
    val   = input->value() + input->position();
    found = strstr(val, find);

    if (found != NULL) {
      // Found a match; update the position and replace text...
      times ++;
      pos = input->position() + found - val;
      input->replace(pos, pos + strlen(find), replace_with->value());
      input->position(pos + strlen(replace_with->value()));
    }
  } while (found != NULL);

  if (times > 0) fl_message("Replaced %d occurrences.", times);
  else fl_alert("No occurrences of \'%s\' found!", find);
}

void replcan_cb() {
  replace_dlg->hide();
}

void save_cb(void) {
  if (filename[0] == '\0') {
    // No filename - get one!
    saveas_cb();
    return;
  }
  else save_file(filename);
}

void saveas_cb(void) {
  char *newfile;

  newfile = fl_file_chooser("Save File As?", "*", filename);
  if (newfile != NULL) save_file(newfile);
}

void undo_cb(void) {
  input->undo();
}

Fl_Menu_Item menuitems[] = {
  { "&File", 0, 0, 0, FL_SUBMENU },
    { "&New",        FL_ALT + 'n', (Fl_Callback *)new_cb },
    { "&Open...",    FL_ALT + 'o', (Fl_Callback *)open_cb, 0, FL_MENU_DIVIDER },
    { "&Save",       FL_ALT + 's', (Fl_Callback *)save_cb },
    { "Save &As...", FL_ALT + FL_SHIFT + 's', (Fl_Callback *)saveas_cb, 0, FL_MENU_DIVIDER },
    { "&Quit", FL_ALT + 'q', (Fl_Callback *)quit_cb },
    { 0 },

  { "&Edit", 0, 0, 0, FL_SUBMENU },
    { "&Undo",       FL_ALT + 'z', (Fl_Callback *)undo_cb, 0, FL_MENU_DIVIDER },
    { "Cu&t",        FL_ALT + 'x', (Fl_Callback *)cut_cb },
    { "&Copy",       FL_ALT + 'c', (Fl_Callback *)copy_cb },
    { "&Paste",      FL_ALT + 'v', (Fl_Callback *)paste_cb },
    { "&Delete",     0, (Fl_Callback *)delete_cb },
    { 0 },

  { "&Search", 0, 0, 0, FL_SUBMENU },
    { "&Find...",       FL_ALT + 'f', (Fl_Callback *)find_cb },
    { "F&ind Again",    FL_ALT + 'g', (Fl_Callback *)find2_cb },
    { "&Replace...",    FL_ALT + 'r', (Fl_Callback *)replace_cb },
    { "Re&place Again", FL_ALT + 't', (Fl_Callback *)replace2_cb },
    { 0 },

  { 0 }
};

int main(int argc, char **argv) {
  window = new Fl_Window(640, 480, "Untitled");
    menubar = new Fl_Menu_Bar(0, 0, 640, 30);
    menubar->menu(menuitems);

    input = new Fl_Multiline_Input(0, 30, 640, 450);
    input->callback((Fl_Callback *)changed_cb);
    input->when(FL_WHEN_CHANGED);
    input->textfont(FL_COURIER);
  window->end();
  window->resizable(input);
  window->callback((Fl_Callback *)quit_cb);

  replace_dlg = new Fl_Window(300, 105, "Replace");
    replace_find = new Fl_Input(70, 10, 210, 25, "Find:");
    replace_find->align(FL_ALIGN_LEFT);

    replace_with = new Fl_Input(70, 40, 210, 25, "Replace:");
    replace_with->align(FL_ALIGN_LEFT);

    replace_all = new Fl_Button(10, 70, 90, 25, "Replace All");
    replace_all->callback((Fl_Callback *)replall_cb);

    replace_next = new Fl_Return_Button(105, 70, 120, 25, "Replace Next");
    replace_next->callback((Fl_Callback *)replace2_cb);

    replace_cancel = new Fl_Button(230, 70, 60, 25, "Cancel");
    replace_cancel->callback((Fl_Callback *)replcan_cb);
  replace_dlg->end();
  replace_dlg->set_modal();

  window->show(1, argv);

  if (argc > 1) load_file(argv[1]);

  return Fl::run();
}

//
// End of "$Id: editor.cxx,v 1.2.2.3 2001/01/22 15:13:41 easysw Exp $".
//
