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

#include <config.h>
#include <FL/Fl_Native_File_Chooser.H>
#include "Fl_Native_File_Chooser_Kdialog.H"
#include "Fl_Window_Driver.H"
#include "drivers/Unix/Fl_Unix_System_Driver.H"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


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


void Fl_Kdialog_Native_File_Chooser_Driver::fnfc_fd_cb(int fd,
                      Fl_Kdialog_Native_File_Chooser_Driver::fnfc_pipe_struct *data) {
  char tmp[FL_PATH_MAX];
  int l = read(fd, tmp, sizeof(tmp)-1);
  if (l > 0) {
    tmp[l] = 0;
    data->all_files = Fl_Native_File_Chooser_Driver::strapp(data->all_files, tmp);
  } else {
    data->fd = -1;
  }
}


static int fnfc_dispatch(int /*event*/, Fl_Window* /*win*/) {
  return 0;
}


int Fl_Kdialog_Native_File_Chooser_Driver::show() {
  const char *option;
  switch (_btype) {
    case Fl_Native_File_Chooser::BROWSE_MULTI_DIRECTORY: {
      // BROWSE_MULTI_DIRECTORY is not supported by kdialog, run GTK chooser instead
      Fl_Native_File_Chooser fnfc(Fl_Native_File_Chooser::BROWSE_MULTI_DIRECTORY);
      fnfc.title( title() );
      fnfc.directory(directory());
      fnfc.preset_file(preset_file());
      fnfc.filter(filter());
      fnfc.options(options());
      int retval = fnfc.show();
      for (int i = 0; i < _tpathnames; i++) delete[] _pathnames[i];
      delete[] _pathnames; _pathnames = NULL;
      _tpathnames = fnfc.count();
      if (_tpathnames && retval == 0) {
        _pathnames = new char*[_tpathnames];
        for (int i = 0; i < _tpathnames; i++) {
          _pathnames[i] = new char[strlen(fnfc.filename(i))+1];
          strcpy(_pathnames[i], fnfc.filename(i));
        }
      }
      return retval;
    }
      break;
    case Fl_Native_File_Chooser::BROWSE_DIRECTORY:
      option = "--getexistingdirectory";
      break;

    case Fl_Native_File_Chooser::BROWSE_SAVE_FILE:
    case Fl_Native_File_Chooser::BROWSE_SAVE_DIRECTORY:
      option = "--getsavefilename";
      break;

    case Fl_Native_File_Chooser::BROWSE_MULTI_FILE:
      option = "--multiple --getopenfilename --separate-output";
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
  fnfc_pipe_struct data;
  data.all_files = NULL;
  if (pipe) {
    data.fd = fileno(pipe);
    Fl::add_fd(data.fd, FL_READ, (Fl_FD_Handler)fnfc_fd_cb, &data);
    Fl_Event_Dispatch old_dispatch = Fl::event_dispatch();
    // prevent FLTK from processing any event
    Fl::event_dispatch(fnfc_dispatch);
    void *control = ((Fl_Unix_System_Driver*)Fl::system_driver())->control_maximize_button(NULL);
    // run event loop until pipe finishes
    while (data.fd >= 0) Fl::wait();
    Fl::remove_fd(fileno(pipe));
    pclose(pipe);
    // return to previous event processing by FLTK
    Fl::event_dispatch(old_dispatch);
    if (control) ((Fl_Unix_System_Driver*)Fl::system_driver())->control_maximize_button(control);
    if (data.all_files) {
      // process text received from pipe
      if (data.all_files[strlen(data.all_files)-1] == '\n') data.all_files[strlen(data.all_files)-1] = 0;
      for (int i = 0; i < _tpathnames; i++) delete[] _pathnames[i];
      delete[] _pathnames;
      char *p = data.all_files;
      int count = 1;
      while ((p = strchr(p+1, '\n'))) count++;
      _pathnames = new char*[count];
      _tpathnames = 0;
      char *q = strtok(data.all_files, "\n");
      while (q) {
        _pathnames[_tpathnames] = new char[strlen(q)+1];
        strcpy(_pathnames[_tpathnames], q);
        _tpathnames++;
        q = strtok(NULL, "\n");
      }
    }
  }
  delete[] command;
  if (_title) { free(_title); _title = NULL; }
  if (!pipe) return -1;
  return (data.all_files == NULL ? 1 : 0);
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
