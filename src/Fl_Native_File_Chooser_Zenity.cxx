//
// FLTK native file chooser widget : Zenity version
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

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

#include <config.h>
#include "Fl_Native_File_Chooser_Zenity.H"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Fl_Zenity_Native_File_Chooser_Driver : file chooser based on the "zenity" command

bool Fl_Zenity_Native_File_Chooser_Driver::did_find_zenity = false;
bool Fl_Zenity_Native_File_Chooser_Driver::have_looked_for_zenity = false;


Fl_Zenity_Native_File_Chooser_Driver::Fl_Zenity_Native_File_Chooser_Driver(int val) :  Fl_Kdialog_Native_File_Chooser_Driver(val) {
}

void Fl_Zenity_Native_File_Chooser_Driver::append_filter(Fl_String& ret_command) {
  // TODO: This could probably be simplified + toughened with Fl_String -erco 1/10/24
  int l;
  int lcommand = 10000;
  char *command = new char[lcommand];
  command[0] = 0;
  char *parsed_filter_copy = strdup(_parsedfilt); // keep _parsedfilt unchanged for re-use
  char *p = strtok(parsed_filter_copy, "\n");
  while (p) {
    char *op = strchr(p, '(');
    l = strlen(command);
    snprintf(command+l, lcommand-l, " --file-filter='%s|", p);
    char *cp = strchr(p, ')');
    *cp = 0;
    char *ob = strchr(op+1, '[');
    if (ob) { // process [xyz] patterns
      *ob = 0;
      char *cb = strchr(ob+1, ']');
      char aux[100];
      for (char *q = ob+1; q < cb; q++) {
        strcpy(aux, op+1);
        int la = strlen(aux);
        aux[la++] = *q;
        if (cb < cp-1) { strcpy(aux+la, cb+1); la += strlen(cb+1); }
        aux[la] = 0;
        l = strlen(command);
        snprintf(command+l, lcommand-l, " %s", aux);
      }
      strcat(command, "'");
    } else {
      l = strlen(command);
      snprintf(command+l, lcommand-l, "%s'", op+1);
    }
    p = strtok(NULL, "\n");
  }
  free(parsed_filter_copy);
  ret_command += command;        // append to parent's Fl_String
  delete [] command;
}

void Fl_Zenity_Native_File_Chooser_Driver::build_command(Fl_String& command) {
  const char *option;
  switch (_btype) {
    case Fl_Native_File_Chooser::BROWSE_DIRECTORY:
    case Fl_Native_File_Chooser::BROWSE_SAVE_DIRECTORY:
      option = "--file-selection --directory";
      break;

    case Fl_Native_File_Chooser::BROWSE_SAVE_FILE:
      if (options() & Fl_Native_File_Chooser::SAVEAS_CONFIRM)
        option = "--file-selection --save --confirm-overwrite";
      else
        option = "--file-selection --save";
      break;

    case Fl_Native_File_Chooser::BROWSE_MULTI_FILE:
      option = "--file-selection --multiple --separator='\n'";
      break;

    default:
      option = "--file-selection";
  }

  // Build preset
  Fl_String preset;
  if (_preset_file) {
    Fl_String quoted_filename = _preset_file; shell_quote(quoted_filename);
    preset = "--filename=";
    preset += quoted_filename;
  } else if (_directory) {
    // This doesn't actually seem to do anything, but supply it anyway.
    Fl_String quoted_dir = _directory; shell_quote(quoted_dir);
    preset = "--filename=";
    preset += quoted_dir;
  }

  // Build command
  command = "zenity";
  if (_title) {
    Fl_String quoted_title = _title; shell_quote(quoted_title);
    command += " --title ";
    command += quoted_title;
  }
  command += " ";
  command += option;
  if (preset != "") {
    command += " ";
    command += preset;
  }
  if (_parsedfilt) append_filter(command);
  command += " 2> /dev/null";   // get rid of stderr
  //printf("command = %s\n", command.c_str());
}

/**
\}
\endcond
*/
