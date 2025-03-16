//
// Shell Command database header for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

#include "Fluid.h"

#include <FL/Enumerations.H>

#include <stdio.h>
#include <stdlib.h>
#include <string>
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

namespace fld {
namespace io {

class Project_Reader;
class Project_Writer;

} // namespace io
} // namespace fld

struct Fl_Menu_Item;
class Fl_Widget;
class Fl_Preferences;

void show_terminal_window();
void run_shell_command(const std::string &cmd, int flags);
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
  FILE * _fpt = nullptr;
};


class Fd_Shell_Command {
public:
  enum { ALWAYS, NEVER, MAC_ONLY, UX_ONLY, WIN_ONLY, MAC_AND_UX_ONLY, USER_ONLY, HOST_ONLY, ENV_ONLY }; // conditions
  enum { SAVE_PROJECT = 1, SAVE_SOURCECODE = 2, SAVE_STRINGS = 4, SAVE_ALL = 7,
    DONT_SHOW_TERMINAL = 8, CLEAR_TERMINAL = 16, CLEAR_HISTORY = 32 }; // flags
  Fd_Shell_Command();
  Fd_Shell_Command(const Fd_Shell_Command *rhs);
  Fd_Shell_Command(const std::string &in_name);
  Fd_Shell_Command(const std::string &in_name,
                   const std::string &in_label,
                   Fl_Shortcut in_shortcut,
                   fld::Tool_Store in_storage,
                   int in_condition,
                   const std::string &in_condition_data,
                   const std::string &in_command,
                   int in_flags);
  std::string name { };
  std::string label { };
  Fl_Shortcut shortcut = 0;
  fld::Tool_Store storage = fld::Tool_Store::USER;
  int condition = ALWAYS; // always, hide, windows only, linux only, mac only, user, machine
  std::string condition_data { }; // user name, machine name
  std::string command { };
  int flags = 0; // save_project, save_code, save_string, ...
  Fl_Menu_Item *shell_menu_item_;
  void run();
  void read(Fl_Preferences &prefs);
  void write(Fl_Preferences &prefs, bool save_location = false);
  void read(class fld::io::Project_Reader*);
  void write(class fld::io::Project_Writer*);
  void update_shell_menu();
  bool is_active();
};


class Fd_Shell_Command_List {
public:
  Fd_Shell_Command **list = nullptr;
  int list_size = 0;
  int list_capacity = 0;
  Fl_Menu_Item *shell_menu_ = nullptr;
public:
  Fd_Shell_Command_List();
  ~Fd_Shell_Command_List();
  Fd_Shell_Command *at(int index) const;
  void add(Fd_Shell_Command *cmd);
  void insert(int index, Fd_Shell_Command *cmd);
  void remove(int index);
  void clear();
  void clear(fld::Tool_Store store);
//  void move_up();
//  void move_down();
//  int load(const std::string &filename);
//  int save(const std::string &filename);
  void read(Fl_Preferences &prefs, fld::Tool_Store storage);
  void write(Fl_Preferences &prefs, fld::Tool_Store storage);
  void read(class fld::io::Project_Reader*);
  void write(class fld::io::Project_Writer*);
  void rebuild_shell_menu();
  void update_settings_dialog();

  static Fl_Menu_Item default_menu[];
  static void menu_marker(Fl_Widget*, void*);
  static void export_selected();
  static void import_from_file();
};

extern Fd_Shell_Command_List *g_shell_config;

#endif // _FLUID_SHELL_COMMAND_H
