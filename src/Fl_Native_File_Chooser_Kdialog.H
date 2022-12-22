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
  friend class Fl_Zenity_Native_File_Chooser_Driver;
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
  virtual int count() const FL_OVERRIDE;
  virtual const char *filename() const FL_OVERRIDE;
  virtual const char *filename(int i) const FL_OVERRIDE;
  virtual char *build_command();
  virtual int show() FL_OVERRIDE;
  char *parse_filter(const char *f);
  virtual const char *filter() const FL_OVERRIDE;
  virtual void filter(const char *f) FL_OVERRIDE;
  virtual int filters() const FL_OVERRIDE;
  virtual void preset_file(const char *val) FL_OVERRIDE;
  virtual const char *preset_file() const FL_OVERRIDE;
  virtual void directory(const char *val) FL_OVERRIDE;
  virtual const char *directory() const FL_OVERRIDE;
  virtual void title(const char *val) FL_OVERRIDE;
  virtual const char *title() const FL_OVERRIDE;
};

#endif // FL_KDIALOG_NATIVE_FILE_CHOOSER_H
