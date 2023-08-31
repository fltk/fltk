//
// FLUID main entry for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

#include <stdio.h>
#include <stdlib.h>

#include <FL/Fl_String.H>
#include <FL/Enumerations.H>

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

void run_shell_command(const Fl_String &cmd, int flags);

extern Fl_Menu_Item default_shell_menu[];
void shell_submenu_marker(Fl_Widget*, void*);

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
  enum { SAVE_PROJECT = 1, SAVE_SOURCECODE = 2, SAVE_STRINGS = 4, SAVE_ALL = 7 }; // flags
  Fd_Shell_Command(const Fd_Shell_Command *rhs);
  Fd_Shell_Command(const Fl_String &in_name);
  Fd_Shell_Command(const Fl_String &in_name,
                   const Fl_String &in_label,
                   Fl_Shortcut in_shortcut,
                   int in_condition,
                   const Fl_String &in_condition_data,
                   const Fl_String &in_command,
                   int in_flags);
  Fl_String name;
  Fl_String label;
  Fl_Shortcut shortcut;
  int condition; // always, hide, windows only, linux only, mac only, user, machine
  Fl_String condition_data; // user name, machine name
  Fl_String command;
  int flags; // save_project, save_code, save_string, ...
  Fl_Menu_Item *shell_menu_item_;
  void run();
  void read(class Fd_Project_Reader*);
  void write(class Fd_Project_Writer*);
  void update_shell_menu();
  bool is_active();
};

class Fd_Shell_Command_List {
public:
  bool is_default_;
  Fd_Shell_Command **list;
  int list_size;
  int list_capacity;
  Fl_Menu_Item *shell_menu_;
public:
  Fd_Shell_Command_List();
  ~Fd_Shell_Command_List();
  Fd_Shell_Command *at(int index) const;
  void select(int n);
  void deselect();
  void add(Fd_Shell_Command *cmd);
  void insert(int index, Fd_Shell_Command *cmd);
  void duplicate();
  void remove(int index);
  void clear();
  void restore_defaults();
  bool is_default();
  void move_up();
  void move_down();
  void read(class Fd_Project_Reader*);
  void write(class Fd_Project_Writer*);
  void rebuild_shell_menu();
  void update_settings_dialog();
};

extern Fd_Shell_Command_List *g_shell_config;


#endif // _FLUID_SHELL_COMMAND_H
