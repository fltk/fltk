//
// fltk-admin for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022 by Bill Spitzak and others.
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

#include "fltk-admin.h"
#include "fltk-admin-ui.h"

#include <FL/Fl.H>
#include <FL/filename.H>
#include "../src/flstring.h"

#include <stdlib.h>
#include <stdio.h>


Fl_Option_Data g_option[Fl::OPTION_LAST];

int g_verbose = 0;

int g_batch_mode = 0;


void set_user_option(const char *name, int value) {
  Fl_Preferences prefs(Fl_Preferences::USER_L, "fltk.org", "fltk");
  Fl_Preferences options(prefs, "options");
  if (value==-1)
    options.deleteEntry(name);
  else
    options.set(name, value);
}

int get_user_option(const char *name) {
  int value = -1;
  Fl_Preferences prefs(Fl_Preferences::USER_L, "fltk.org", "fltk");
  Fl_Preferences options(prefs, "options");
  options.get(name, value, -1);
  return value;
}

void clear_user_option(const char *name) {
  set_user_option(name, -1);
}


void set_system_option(const char *name, int value) {
  Fl_Preferences prefs(Fl_Preferences::SYSTEM_L, "fltk.org", "fltk");
  Fl_Preferences options(prefs, "options");
  if (value==-1)
    options.deleteEntry(name);
  else
    options.set(name, value);
}

int get_system_option(const char *name) {
  int value = -1;
  Fl_Preferences prefs(Fl_Preferences::SYSTEM_L, "fltk.org", "fltk");
  Fl_Preferences options(prefs, "options");
  options.get(name, value, -1);
  return value;
}

void clear_system_option(const char *name) {
  set_system_option(name, -1);
}


void print_usage(const char *argv0) {
  const char *app_name = NULL;
  if (argv0 && argv0[0])
    app_name = fl_filename_name(argv0);
  if (!app_name || !app_name[0])
    app_name = "fltk-admin";
  fprintf(stderr, "FLTK %d.%d.%d. Usage:\n", FL_MAJOR_VERSION, FL_MINOR_VERSION, FL_PATCH_VERSION);
  fprintf(stderr, "%s [-Soption[=val]] [-Uoption[=val]] [-h] [-fltk_option]\n", app_name);
  fprintf(stderr, "  Calling \"%s\" without options will launch in interactive mode.\n", app_name);
  fprintf(stderr, "  Options start \"-S\" for system wide settings or \"-U\" for\n"
                  "  user settings in current account.\n"
                  "  Values can be \"0\" or \"OFF\" to clear, or \"1\" or \"OFF\" to set the option.\n"
                  "  A values \"-1\" or \"DEFAULT\" set the option to its default value.\n"
                  "  Values can be \"0\" and \"1\", or \"ON\" and \"OFF\".\n"
                  "  If no value is given the current setting is returned as \"0\" or \"1\".\n");
  fprintf(stderr, "  This version of FLTK supports the following options:\n");
  for (int i=0; i<Fl::OPTION_LAST; i++) {
    if (g_option[i].name) {
      if (g_option[i].brief)
        fprintf(stderr, "    %-16s %s\n", g_option[i].name, g_option[i].brief);
      else
        fprintf(stderr, "    %s\n", g_option[i].name);
    }
  }
}


/*
 Command line arguments start with "-S" for system wide settings or "-U" for
 user settings, followed by the option name, followed by a value "=0", "=1",
 "=ON", or "=OFF". If no value is give, we will print a "0" or "1" to stdout.
 */
static int read_command_line_args(int argc, char** argv, int& i) {
  char cmd = 0;
  char opt[64] = "";
  char val[32] = "";
  int ival = -999;
  const char *arg = argv[i++];

  if (arg[0] == '-') {
    cmd = arg[1];
    if (cmd == 'U' || cmd == 'S') {
      const char *eq = strchr(arg+2, '=');
      if (eq) {
        size_t n = (eq - (arg+2));
        if (n==0) {
          i--;
          return 0;
        }
        if (n > sizeof(opt)-1) n = sizeof(opt)-1;
        strlcpy(opt, arg+2, n+1);
        strlcpy(val, eq+1, sizeof(val));
        if (fl_ascii_strcasecmp(val, "ON")==0)
          ival = 1;
        else if (fl_ascii_strcasecmp(val, "OFF")==0)
          ival = 0;
        else if (fl_ascii_strcasecmp(val, "DEFAULT")==0)
          ival = -1;
        else if (strcmp(val, "1")==0)
          ival = 1;
        else if (strcmp(val, "0")==0)
          ival = 0;
        else if (strcmp(val, "-1")==0)
          ival = -1;
        else {
          fprintf(stderr, "Warning: Unrecognized value \"%s\" for option \"%s\".\n", opt, val);
          return 1;
        }
      } else {
        strlcpy(opt, arg+2, sizeof(opt));
      }
    }
  }
  if ((cmd == 'h') && (arg[2] == 0)) {
    print_usage(argv[0]);
    g_batch_mode = 1;
    return 1;
  }
  if ((cmd == 'v') && (arg[2] == 0)) {
    g_verbose = 1;
    return 1;
  }
  if (cmd == 'U') { // user setting
    int i;
    for (i=0; i<Fl::OPTION_LAST; i++) {
      if (g_option[i].name && strcmp(g_option[i].name, opt)==0) {
        if (ival == -999) {
          int value = get_user_option(g_option[i].name);
          if (g_verbose) fprintf(stderr, "Current value for user option %s is %d\n", opt, value);
          printf("%d\n", value);
        } else if (ival ==-1) {
          if (g_verbose) fprintf(stderr, "Reset user option %s to default\n", opt);
          clear_user_option(g_option[i].name);
        } else {
          if (g_verbose) fprintf(stderr, "Set user option %s to %d\n", opt, ival);
          set_user_option(g_option[i].name, ival);
        }
        break;
      }
    }
    if (i==Fl::OPTION_LAST)
      fprintf(stderr, "Warning: Unrecognized user option \"%s\".\n", opt);
    g_batch_mode = 1;
    return 1;
  }
  if (cmd == 'S') { // system setting
    int i;
    for (i=0; i<Fl::OPTION_LAST; i++) {
      if (g_option[i].name && strcmp(g_option[i].name, opt)==0) {
        if (ival == -999) {
          int value = get_system_option(g_option[i].name);
          if (g_verbose) fprintf(stderr, "Current value for system option %s is %d\n", opt, value);
          printf("%d\n", value);
        } else if (ival ==-1) {
          if (g_verbose) fprintf(stderr, "Reset system option %s to default\n", opt);
          clear_system_option(g_option[i].name);
        } else {
          if (g_verbose) fprintf(stderr, "Set system option %s to %d\n", opt, ival);
          set_system_option(g_option[i].name, ival);
        }
        break;
      }
    }
    if (i==Fl::OPTION_LAST)
      fprintf(stderr, "Warning: Unrecognized user option \"%s\".\n", opt);
    g_batch_mode = 1;
    return 1;
  }
  i--;
  return 0;
}

// Fill the option names array with the names. Some entries may stay blank.
static void init_option_data() {
  static Fl_Option_Data arrow_focus = {
    "ArrowFocus", 0,
    "arrow keys will move focus beyond text input field"
  };
  g_option[Fl::OPTION_ARROW_FOCUS] = arrow_focus;

  static Fl_Option_Data visible_focus = {
    "VisibleFocus", 1,
    "draw a dotted rectangle in widget with keyboard focus"
  };
  g_option[Fl::OPTION_VISIBLE_FOCUS] = visible_focus;

  static Fl_Option_Data dnd_text = {
    "DNDText", 1,
    "user can drag text from FLTK into other apps"
  };
  g_option[Fl::OPTION_DND_TEXT] = dnd_text;

  static Fl_Option_Data show_tooltips = {
    "ShowTooltips", 1,
    "show or hide tooltips"
  };
  g_option[Fl::OPTION_SHOW_TOOLTIPS] = show_tooltips;

  static Fl_Option_Data fnfc_uses_gtk = {
    "FNFCUsesGTK", 1,
    "use GTK file chooser instead of FLTK if available"
  };
  g_option[Fl::OPTION_FNFC_USES_GTK] = fnfc_uses_gtk;

  static Fl_Option_Data print_use_gtk = {
    "PrintUsesGTK", 1,
    "use GTK printer dialog instead of FLTK if available"
  };
  g_option[Fl::OPTION_PRINTER_USES_GTK] = print_use_gtk;

  static Fl_Option_Data show_zoom_factor = {
    "ShowZoomFactor", 1,
    "show the zoom factor in a transient yellow window"
  };
  g_option[Fl::OPTION_SHOW_SCALING] = show_zoom_factor;
}


int main(int argc,char **argv) {
  init_option_data();

  int i = 1;
  if (Fl::args(argc, argv, i, read_command_line_args)==0) {
    fprintf(stderr, "ERROR: Unrecognized command line option \"%s\".\n", argv[i]);
    return 1;
  }
  if (g_batch_mode)
    return 0;

  Fl_Window *win = create_options_editor();
  win->show(argc, argv);
  Fl::run();

  return 0;
}
//
///// \}
//
