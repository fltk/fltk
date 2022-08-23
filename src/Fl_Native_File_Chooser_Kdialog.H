//
// FLTK native file chooser widget : KDE version
//
// Copyright 2021-2022 by Bill Spitzak and others.
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

#ifndef FL_KDIALOG_NATIVE_FILE_CHOOSER_H
#define FL_KDIALOG_NATIVE_FILE_CHOOSER_H 1

#include <FL/Fl_Native_File_Chooser.H>

class Fl_Kdialog_Native_File_Chooser_Driver : public Fl_Native_File_Chooser_FLTK_Driver {
  friend class Fl_Native_File_Chooser;
  struct fnfc_pipe_struct {
    char *all_files;
    int fd;
  };
  static void fnfc_fd_cb(int fd, fnfc_pipe_struct *data);
  char **_pathnames;
  int _tpathnames;
  char *_directory;
  char *_preset_file;
  char *_title;
  static bool did_find_kdialog;
  static bool have_looked_for_kdialog;
  Fl_Kdialog_Native_File_Chooser_Driver(int val);
  ~Fl_Kdialog_Native_File_Chooser_Driver();
  int count() const;
  const char *filename() const;
  const char *filename(int i) const;
  int show();
  char *parse_filter(const char *f);
  const char *filter() const;
  virtual void filter(const char *f);
  int filters() const;
  void preset_file(const char *val);
  const char *preset_file() const;
  void directory(const char *val);
  const char *directory() const;
  void title(const char *val);
  const char *title() const;
};

#endif // FL_KDIALOG_NATIVE_FILE_CHOOSER_H
