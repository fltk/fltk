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

void show_shell_window();
void do_shell_command(class Fl_Return_Button*, void*);

typedef struct {
  char *command;
  int flags;
} Shell_Settings;

extern Shell_Settings shell_settings_windows;
extern Shell_Settings shell_settings_linux;
extern Shell_Settings shell_settings_macos;

extern Fl_String g_shell_command;
extern int g_shell_save_fl;
extern int g_shell_save_code;
extern int g_shell_save_strings;
extern int g_shell_use_fl_settings;

void shell_prefs_get();
void shell_prefs_set();
void shell_settings_read();
void shell_settings_write();


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

#endif // _FLUID_SHELL_COMMAND_H
