//
// FLTK native file chooser widget : KDE version
//
// Copyright 2021 by Bill Spitzak and others.
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

#include <config.h>
#include <FL/Fl_Native_File_Chooser.H>
#include "Fl_Native_File_Chooser_Kdialog.H"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Fl_Kdialog_Native_File_Chooser_Driver : file chooser based on the "kdialog" command */

bool Fl_Kdialog_Native_File_Chooser_Driver::did_find_kdialog = false;
bool Fl_Kdialog_Native_File_Chooser_Driver::have_looked_for_kdialog = false;


Fl_Kdialog_Native_File_Chooser_Driver::Fl_Kdialog_Native_File_Chooser_Driver(int val) :  Fl_Native_File_Chooser_FLTK_Driver(val) {
  _tpathnames = 0;
  _pathnames = NULL;
  _directory = NULL;
  _preset_file = NULL;
  _title = NULL;
}


Fl_Kdialog_Native_File_Chooser_Driver::~Fl_Kdialog_Native_File_Chooser_Driver() {
  for (int i = 0; i < _tpathnames; i++) delete[] _pathnames[i];
  delete[] _pathnames;
  if (_preset_file) free(_preset_file);
  if (_directory) free(_directory);
  if (_title) free(_title);
}


int Fl_Kdialog_Native_File_Chooser_Driver::show() {
  Fl::flush(); // to close menus if necessary
  const char *option;
  switch (_btype) {
    case Fl_Native_File_Chooser::BROWSE_DIRECTORY:
    case Fl_Native_File_Chooser::BROWSE_MULTI_DIRECTORY: // not supported
      option = "--getexistingdirectory";
      break;

    case Fl_Native_File_Chooser::BROWSE_SAVE_FILE:
    case Fl_Native_File_Chooser::BROWSE_SAVE_DIRECTORY:
      option = "--getsavefilename";
      break;

    case Fl_Native_File_Chooser::BROWSE_MULTI_FILE:
      option = "--multiple --getopenfilename";
      break;

    default:
      option = "--getopenfilename";
  }
  const char *preset = ".";
  if (_preset_file) preset = _preset_file;
  else if (_directory) preset = _directory;
  char *command = new char[strlen(option) + strlen(preset) + (_title?strlen(_title)+11:0) +
                           (_parsedfilt?strlen(_parsedfilt):0) + 50];
  strcpy(command, "kdialog ");
  if (_title) {
    sprintf(command+strlen(command), " --title '%s'", _title);
  }
  sprintf(command+strlen(command), " %s %s ", option, preset);
  if (_parsedfilt) sprintf(command+strlen(command), " \"%s\" ", _parsedfilt);
  strcat(command, "2> /dev/null"); // get rid of stderr output
//puts(command);
  FILE *pipe = popen(command, "r");
  char *all_files = NULL;
  if (pipe) {
    char *p, tmp[FL_PATH_MAX];
    do {
      p = fgets(tmp, sizeof(tmp), pipe);
      if (p) all_files = strapp(all_files, p);
    }
    while (p);
    pclose(pipe);
    if (all_files) {
      if (all_files[strlen(all_files)-1] == '\n') all_files[strlen(all_files)-1] = 0;
      for (int i = 0; i < _tpathnames; i++) delete[] _pathnames[i];
      delete[] _pathnames;
      p = all_files;
      int count = 1;
      while ((p = strchr(p+1, ' '))) count++;
      _pathnames = new char*[count];
      _tpathnames = 0;
      char *q = strtok(all_files, " ");
      while (q) {
        _pathnames[_tpathnames] = new char[strlen(q)+1];
        strcpy(_pathnames[_tpathnames], q);
        _tpathnames++;
        q = strtok(NULL, " ");
      }
    }
  }
  delete[] command;
  if (_title) { free(_title); _title = NULL; }
  if (!pipe) return -1;
  return (all_files == NULL ? 1 : 0);
}


const char *Fl_Kdialog_Native_File_Chooser_Driver::filename() const {
  return _tpathnames >= 1 ? _pathnames[0] : NULL;
}

const char *Fl_Kdialog_Native_File_Chooser_Driver::filename (int i) const {
  return _tpathnames > i ? _pathnames[i] : NULL;
}

const char *Fl_Kdialog_Native_File_Chooser_Driver::filter() const  {
  return _filter;
}

int Fl_Kdialog_Native_File_Chooser_Driver::filters() const {
  return (_nfilters ? _nfilters - 1 : 0);
}

int Fl_Kdialog_Native_File_Chooser_Driver::count() const {
  return _tpathnames;
}

char *Fl_Kdialog_Native_File_Chooser_Driver::parse_filter(const char *f) {
  //In: "*.H\n" or "*.H"        Out: "(*.H)"
  //In: "Headers\t*.H\n"        Out: "Headers (*.H)"
  //In: "Headers\t*.{H,h}\n"    Out: "Headers (*.H *.h)"
  const char *p = strchr(f, '\t');
  if (!p) p = f - 1;
  const char *q = strchr(f, '\n'); if (!q) q = f + strlen(f);
  const char *r = strchr(f, '{');
  char *developed = NULL;
  if (r) { // with {}
    char *lead = new char[r-p];
    memcpy(lead, p+1, (r-p)-1); lead[(r-p)-1] = 0;
    const char *r2 = strchr(r, '}');
    char *ends = new char[r2-r];
    memcpy(ends, r+1, (r2-r)-1); ends[(r2-r)-1] = 0;
    char *ptr;
    char *part = strtok_r(ends, ",", &ptr);
    while (part) {
      developed = strapp(developed, lead);
      developed = strapp(developed, part);
      developed = strapp(developed, " ");
      part = strtok_r(NULL, ",", &ptr);
    }
    if (developed[strlen(developed)-1] == ' ') developed[strlen(developed)-1] = 0;
    delete[] lead;
    delete[] ends;
  }
  int lout = (p>f?p-f:0) + 2 + (r?strlen(developed):((q-p)-1)) + 2;
  char *out = new char[lout]; *out = 0;
  if (p > f) {memcpy(out, f, p-f); out[p-f] = 0; }
  strcat(out, " (");
  if (r) {
    strcpy(out+strlen(out), developed);
    strfree(developed);
  }
  else memcpy(out+strlen(out), p+1, (q-p));
  strcat(out, ")");
//puts(out);
  return out;
}


void Fl_Kdialog_Native_File_Chooser_Driver::filter(const char *f) {
  _parsedfilt = strfree(_parsedfilt);   // clear previous parsed filter (if any)
  _nfilters = 0;
  if (!f) return;
  _filter = strdup(f);
  char *f2 = strdup(f);
  char *ptr;
  char *part = strtok_r(f2, "\n", &ptr);
  while (part) {
    char *p = parse_filter(part);
    _parsedfilt = strapp(_parsedfilt, p);
    _parsedfilt = strapp(_parsedfilt, "\\n");
    delete[] p;
    _nfilters++;
    part = strtok_r(NULL, "\n", &ptr);
  }
  free(f2);
  _parsedfilt = strapp(_parsedfilt, "All files (*)");
  _nfilters++;
//puts(_parsedfilt);
}

void Fl_Kdialog_Native_File_Chooser_Driver::preset_file(const char *val) {
  if (_preset_file) free(_preset_file);
  _preset_file = strdup(val);
}

const char *Fl_Kdialog_Native_File_Chooser_Driver::preset_file() const {
  return _preset_file;
}

void Fl_Kdialog_Native_File_Chooser_Driver::directory(const char *val) {
  if (_directory) free(_directory);
  _directory = strdup(val);
}

const char *Fl_Kdialog_Native_File_Chooser_Driver::directory() const {
  return _directory;
}

void Fl_Kdialog_Native_File_Chooser_Driver::title(const char *val)
{
  if (_title) free(_title);
  _title = strdup(val);
}

const char *Fl_Kdialog_Native_File_Chooser_Driver::title() const {
  return _title;
}
