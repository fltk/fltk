//
// FLUID main entry for the Fast Light Tool Kit (FLTK).
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

#ifndef _FLUID_SHELL_COMMAND_H
#define _FLUID_SHELL_COMMAND_H

#include "fluid.h"

#include "../src/Fl_String.H"
#include <FL/Enumerations.H>

#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <direct.h>
#  include <windows.h>
#  include <io.h>
#  include <fcntl.h>
#  include <commdlg.h>
#  include <FL/platform.H>
#else
#  include <unistd.h>
#endif

struct Fl_Menu_Item;
class Fl_Widget;
class Fl_Preferences;

char preferences_get(Fl_Preferences &prefs, const char *key, Fl_String &value, const Fl_String &defaultValue);
char preferences_set(Fl_Preferences &prefs, const char *key, const Fl_String &value);

void show_terminal_window();
void run_shell_command(const Fl_String &cmd, int flags);
bool shell_command_running(void);

class Fl_Process {
public:
  Fl_Process();
  ~Fl_Process();

  FILE *popen(const char *cmd, const char *mode="r");
  int  close();

  FILE * desc() const;
  char * get_line(char * line, size_t s) const;

  int get_fileno() const;

#if defined(_WIN32) && !defined(__CYGWIN__)
protected:
  HANDLE pin[2], pout[2], perr[2];
  char ptmode;
  PROCESS_INFORMATION pi;
  STARTUPINFO si;

  static bool createPipe(HANDLE * h, BOOL bInheritHnd=TRUE);

private:
  FILE * freeHandles();
  static void clean_close(HANDLE& h);
#endif

protected:
  FILE * _fpt;
};

class Fd_Shell_Command {
public:
  enum { ALWAYS, NEVER, MAC_ONLY, UX_ONLY, WIN_ONLY, MAC_AND_UX_ONLY, USER_ONLY, HOST_ONLY, ENV_ONLY }; // conditions
  enum { SAVE_PROJECT = 1, SAVE_SOURCECODE = 2, SAVE_STRINGS = 4, SAVE_ALL = 7,
    DONT_SHOW_TERMINAL = 8, CLEAR_TERMINAL = 16, CLEAR_HISTORY = 32 }; // flags
  Fd_Shell_Command();
  Fd_Shell_Command(const Fd_Shell_Command *rhs);
  Fd_Shell_Command(const Fl_String &in_name);
  Fd_Shell_Command(const Fl_String &in_name,
                   const Fl_String &in_label,
                   Fl_Shortcut in_shortcut,
                   Fd_Tool_Store in_storage,
                   int in_condition,
                   const Fl_String &in_condition_data,
                   const Fl_String &in_command,
                   int in_flags);
  Fl_String name;
  Fl_String label;
  Fl_Shortcut shortcut;
  Fd_Tool_Store storage;
  int condition; // always, hide, windows only, linux only, mac only, user, machine
  Fl_String condition_data; // user name, machine name
  Fl_String command;
  int flags; // save_project, save_code, save_string, ...
  Fl_Menu_Item *shell_menu_item_;
  void run();
  void read(Fl_Preferences &prefs);
  void write(Fl_Preferences &prefs, bool save_location = false);
  void read(class Fd_Project_Reader*);
  void write(class Fd_Project_Writer*);
  void update_shell_menu();
  bool is_active();
};

class Fd_Shell_Command_List {
public:
  Fd_Shell_Command **list;
  int list_size;
  int list_capacity;
  Fl_Menu_Item *shell_menu_;
public:
  Fd_Shell_Command_List();
  ~Fd_Shell_Command_List();
  Fd_Shell_Command *at(int index) const;
  void add(Fd_Shell_Command *cmd);
  void insert(int index, Fd_Shell_Command *cmd);
  void remove(int index);
  void clear();
  void clear(Fd_Tool_Store store);
//  void move_up();
//  void move_down();
//  int load(const Fl_String &filename);
//  int save(const Fl_String &filename);
  void read(Fl_Preferences &prefs, Fd_Tool_Store storage);
  void write(Fl_Preferences &prefs, Fd_Tool_Store storage);
  void read(class Fd_Project_Reader*);
  void write(class Fd_Project_Writer*);
  void rebuild_shell_menu();
  void update_settings_dialog();

  static Fl_Menu_Item default_menu[];
  static void menu_marker(Fl_Widget*, void*);
  static void export_selected();
  static void import_from_file();
};

extern Fd_Shell_Command_List *g_shell_config;

#endif // _FLUID_SHELL_COMMAND_H
