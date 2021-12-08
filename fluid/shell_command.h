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

class Fl_Process {
public:
  Fl_Process();
  ~Fl_Process();

  FILE *popen(const char *cmd, const char *mode="r");
  int  close();

  FILE * desc() const;
  char * get_line(char * line, size_t s) const;

  int get_fileno() const;

#if defined(_WIN32)  && !defined(__CYGWIN__)
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
