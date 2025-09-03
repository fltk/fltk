//
// fltk-options for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022-2024 by Bill Spitzak and others.
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

#include <FL/Fl_Preferences.H>
#include <FL/Fl.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/filename.H>
#include <FL/fl_draw.H>
#include "../src/flstring.h"

#include <stdlib.h>
#include <stdio.h>

// user interface sizes
// |<->|<-  group browser ->|<->|<- options ->|<->|
const int FO_GAP = 10;
const int FO_BROWSER_W = 220;
const int FO_SCROLL_W = 16 + 4; // Fl::scrollbar_size() + Fl::box_dw(FL_DOWN_BOX);
const int FO_CHOICE_W = 80;
const int FO_OPTIONS_W = 420;
const int FO_BUTTON_W = 75;
const int FO_WINDOW_W = FO_GAP + FO_BROWSER_W + FO_GAP + FO_SCROLL_W + FO_OPTIONS_W + FO_SCROLL_W + FO_GAP;
const int FO_SYSTEM_X = FO_OPTIONS_W - 2*FO_GAP - 2*FO_CHOICE_W;
const int FO_USER_X = FO_OPTIONS_W - FO_GAP - FO_CHOICE_W;

const int FO_TITLE_H = 20;
const int FO_BROWSER_H = 370;
const int FO_BUTTON_H = 25;
const int FO_CHOICE_H = 22;
const int FO_WINDOW_H = FO_GAP + FO_BROWSER_H + FO_GAP + FO_BUTTON_H + FO_GAP;

Fl_Window* g_window = NULL;
Fl_Hold_Browser* g_headline_browser = NULL;
Fl_Scroll* g_option_scroll = NULL;

// command line options
int g_verbose = 0;
int g_batch_mode = 0;
int g_system_write_ok = 0;
int g_user_write_ok = 0;

// Magic value when calling the handle function.
const int FO_PRINT_VALUE = -999;

// User or system setting
typedef enum {
  FO_SYSTEM,
  FO_USER
} Fo_Context;

// Available option types.
// We can add more types later as needed.
// See: Fl_Group* add_option( Fo_Option_Descr* opt)
// See: Fl_Group* add_option_bool(Fo_Option_Descr* opt)
typedef enum {
  FO_END_OF_LIST = 0,
  FO_HEADLINE,
  FO_OPTION_BOOL
  // FO_OPTION_INT,     // not yet implemented
  // FO_OPTION_TEXT,    // not yet implemented
  // FO_OPTION_SCHEME,  // not yet implemented
} Fo_Type;

// Record holding the information for headlines and options.
typedef struct {
  Fo_Type type;
  const char* text;
  Fl::Fl_Option id;
  const char* name;
  const char* prefs_name;
  bool bool_default;
  const char* brief;
  const char* tooltip;
  Fl_Group* ui;
} Fo_Option_Descr;

// List of all entries in the user interface, including headlines and options.
// This list can be arbitrarily extended. The app will generate a scrollable
// area if the list does not fit the app window.
Fo_Option_Descr g_option_list[] = {
  { FO_HEADLINE, "Keyboard Focus Options" },
  { FO_OPTION_BOOL, "Visible Keyboard Focus:",
    Fl::OPTION_VISIBLE_FOCUS, "OPTION_VISIBLE_FOCUS", "VisibleFocus", true,
    "Draw a dotted rectangle in widget with keyboard focus.",
    "If visible focus is switched on, FLTK will draw a dotted rectangle inside "
    "the widget that will receive the next keystroke. If switched off, no such "
    "indicator will be drawn and keyboard navigation is disabled." },
  { FO_OPTION_BOOL, "Arrow Keys move Focus:",
    Fl::OPTION_ARROW_FOCUS, "OPTION_ARROW_FOCUS", "ArrowFocus", false,
    "Arrow keys will move focus beyond text input field.",
    "When switched on, moving the text cursor beyond the start or end of the "
    "text in a text widget will change focus to the next widget. When switched "
    "off, the cursor will stop at the end of the text. Pressing Tab or Ctrl-Tab "
    "will advance the keyboard focus. Switch this on, if you want the old "
    "behavior of FLTK 1.1." },
  { FO_HEADLINE, "Tooltip Options" },
  { FO_OPTION_BOOL, "Show Tooltips:",
    Fl::OPTION_SHOW_TOOLTIPS, "OPTION_SHOW_TOOLTIPS", "ShowTooltips", true,
    "Show or hide tooltips.",
    "If tooltips are enabled, hovering the mouse over a widget with a tooltip "
    "text will open a little tooltip window until the mouse leaves the widget. "
    "If disabled, no tooltip is shown." },
  { FO_HEADLINE, "Drag And Drop Options" },
  { FO_OPTION_BOOL, "Allow dragging Text:",
    Fl::OPTION_DND_TEXT, "OPTION_DND_TEXT", "DNDText", true,
    "User can drag text from FLTK into other apps.",
    "If text drag-and-drop is enabled, the user can select and drag text from "
    "any text widget. If disabled, no dragging is possible, however dropping "
    "text from other applications still works." },
  { FO_HEADLINE, "Native File Chooser Options" },
  { FO_OPTION_BOOL, "Native File Chooser uses GTK:",
    Fl::OPTION_FNFC_USES_GTK, "OPTION_FNFC_USES_GTK", "FNFCUsesGTK", true,
    "Use GTK file chooser instead of FLTK if available.",
    "If 'Native File Chooser uses GTK' is enabled, the Fl_Native_File_Chooser "
    "class calls the GTK open/save file dialogs when they are available on the "
    "platfom. If disabled, the Fl_Native_File_Chooser class always uses FLTK's "
    "own file dialog (i.e., Fl_File_Chooser) even if GTK is available." },
  { FO_OPTION_BOOL, "Native File Chooser uses Zenity:",
    Fl::OPTION_FNFC_USES_ZENITY, "OPTION_FNFC_USES_ZENITY", "UseZenity", false,
    "Fl_Native_File_Chooser uses the 'zenity' command if possible.",
    "Meaningful for the Wayland/X11 platform only. When switched on, "
    "the library uses a Zenity-based file dialog if command 'zenity' is available. "
    "When switched off (default), command 'zenity' is not used."},
  { FO_OPTION_BOOL, "Native File Chooser uses Kdialog:",
    Fl::OPTION_FNFC_USES_KDIALOG, "OPTION_FNFC_USES_KDIALOG", "UseKdialog", false,
    "Fl_Native_File_Chooser uses the 'kdialog' command if possible.",
    "Meaningful for the Wayland/X11 platform. "
    "When switched on, the library uses a kdialog-based file dialog if command 'kdialog' is "
    "available. When switched off (default), command 'kdialog' is not used." },
  { FO_HEADLINE, "Print dialog Options" },
  { FO_OPTION_BOOL, "Print dialog uses GTK:",
    Fl::OPTION_PRINTER_USES_GTK, "OPTION_PRINTER_USES_GTK", "PrintUsesGTK", true,
    "Use GTK printer dialog instead of FLTK if available.",
    "If 'Print dialog uses GTK' is enabled, the Fl_Printer class calls the "
    "GTK print dialog when it's available on the platfom. If disabled, the "
    "Fl_Printer class always uses FLTK's own print dialog even "
    "if GTK is available." },
  { FO_HEADLINE, "Scaling Factor Options" },
  { FO_OPTION_BOOL, "Transiently show scaling factor:",
    Fl::OPTION_SHOW_SCALING, "OPTION_SHOW_SCALING", "ShowZoomFactor", true,
    "Show the zoom factor in a transient popup window.",
    "If 'Transiently show scaling factor' is enabled, the library shows in a "
    "transient popup window the display scaling factor value when it is "
    "changed. If disabled, no such transient window is used." },
  { FO_OPTION_BOOL, "Allow simple zoom-in shortcut:",
    Fl::OPTION_SIMPLE_ZOOM_SHORTCUT, "OPTION_SIMPLE_ZOOM_SHORTCUT", "SimpleZoomShortcut", false,
    "Fine tune the shortcut that triggers the zoom-in operation.",
    "When the keyboard in use has '+' in the shifted position of its key, "
    "pressing that key and ctrl triggers the zoom-in operation. "
    "If disabled, the zoom-in operation requires the shift key to be pressed also "
    "with such a keyboard." },
  // -- When adding new options here, please make sure that you also update
  // --   documentation/src/fltk-options.dox
  // -- and
  // --   documentation/src/fltk-options.man
  { FO_END_OF_LIST }
};

/** Check for write permission.
 \param[in] ctx settings context
 \return true if writing to the requested preferences is ok
 */
bool write_permission(Fo_Context ctx) {
  if (ctx == FO_SYSTEM)
    return g_system_write_ok;
  else
    return g_user_write_ok;
}

/** Write an option into the system or user preferences file.
 \param[in] ctx settings context
 \param[in] name the name of the option
 \param[in] value 0 or 1, or -1 to remove the entry, so it will be reset to
    its default value
 */
void set_option(Fo_Context ctx, const char *name, int value) {
  enum Fl_Preferences::Root context =
    (ctx == FO_SYSTEM) ? Fl_Preferences::CORE_SYSTEM : Fl_Preferences::CORE_USER;
  Fl_Preferences prefs(context, "fltk.org", "fltk");
  Fl_Preferences options(prefs, "options");
  if (value == -1)
    options.deleteEntry(name);
  else
    options.set(name, value);
}

/** Read an option from the system or user preferences file.
 \param[in] ctx settings context
 \param[in] name the name of the option
 \return 0 or 1, or -1 if the value is not set
 */
int get_option(Fo_Context ctx, const char *name) {
  int value = -1;
  enum Fl_Preferences::Root context =
    (ctx == FO_SYSTEM) ? Fl_Preferences::SYSTEM_L : Fl_Preferences::USER_L;
  Fl_Preferences prefs(context, "fltk.org", "fltk");
  Fl_Preferences options(prefs, "options");
  options.get(name, value, -1);
  return value;
}

/** Delete an option in the system or user preferences file.
 \param[in] ctx settings context
 \param[in] name the name of the option
 */
void clear_option(Fo_Context ctx, const char *name) {
  set_option(ctx, name, -1);
}

/** Print the Usage: text and list all system and user options.
 \param[in] argv0 application name when called from the shell
 */
void print_usage(const char *argv0) {
  const char *app_name = NULL;
  if (argv0 && argv0[0])
    app_name = fl_filename_name(argv0);
  if (!app_name || !app_name[0])
    app_name = "fltk-options";
  fprintf(stdout, "FLTK %d.%d.%d. Usage:\n", FL_MAJOR_VERSION, FL_MINOR_VERSION, FL_PATCH_VERSION);
  fprintf(stdout, "%s [-Soption[=val]] [-Uoption[=val]] [-L] [-LS] [-LU] [-f] [-v] [-h[option]]\n", app_name);
  fprintf(stdout, "  -Soption[=value]  change or print system wide option\n");
  fprintf(stdout, "  -Uoption[=value]  change or print user option\n");
  fprintf(stdout, "      Values can be 0 or OFF to clear, or 1 or ON to set the option.\n"
                  "      The value -1 or DEFAULT sets the option to its default value.\n"
                  "      If no value is given, the current setting is returned as -1, 0, or 1.\n");
  fprintf(stdout, "  -L, -LS, -LU  list the value of all options, of all system settings, \n"
                  "      or of all user setting\n");
  fprintf(stdout, "  -f  suppresses error messages concerning file access permissions\n");
  fprintf(stdout, "  -v, --verbose  prints additional information in command line mode\n");
  fprintf(stdout, "  -h[option], --help [option]  general help, or info for the given option\n\n");
  fprintf(stdout, "    This version of %s supports the following options:\n", app_name);
  Fo_Option_Descr *opt;
  for (opt = g_option_list; opt->type != FO_END_OF_LIST; ++opt) {
    if (opt->name) {
      if (opt->brief)
        fprintf(stdout, "  %-28s %s\n", opt->name, opt->brief);
      else
        fprintf(stdout, "  %s\n", opt->name);
    }
  }
  fprintf(stdout, "\n  Calling %s without options will launch %s interactive mode.\n", app_name, app_name);
}

/** Print more information for a given option.
 \param[in] option the name of the option, case insensitive
 */
void print_info(const char *option) {
  Fo_Option_Descr *opt;
  for (opt = g_option_list; opt->type != FO_END_OF_LIST; ++opt) {
    if ( opt->name && (fl_ascii_strcasecmp(opt->name, option) == 0) ) {
      if (opt->brief)
        fprintf(stdout, "%s: %s\n", opt->name, opt->brief);
      else
        fprintf(stdout, "%s: see FLTK manual for details\n", opt->name);
      if (opt->tooltip)
        fprintf(stdout, "\n%s\n", opt->tooltip);
      fprintf(stdout, "\nDefault is %s.\n", opt->bool_default ? "on" : "off");
      break;
    }
  }
  if (opt->type == FO_END_OF_LIST)
    fprintf(stderr, "Warning: Unrecognized option \"%s\".\n", option);
}

/** List the current value of all options.
 \param[in] cmd 'S' lists system options, 'U' lists user options, 0 lists both
 */
void list_options(char cmd) {
  Fo_Option_Descr *opt;
  for (opt = g_option_list; opt->type != FO_END_OF_LIST; ++opt) {
    if (opt->name) {
      printf("%-28s", opt->name);
      if (cmd == 'S' || cmd == 0) {
        int value = get_option(FO_SYSTEM, opt->prefs_name);
        printf(" system: %2d", value);
      }
      if (cmd == 0)
        printf(",");
      if (cmd == 'U' || cmd == 0) {
        int value = get_option(FO_USER, opt->prefs_name);
        printf(" user: %2d", value);
      }
      printf("\n");
    }
  }
}

/** Handle a commmand line argument for system or user options.
 \param[in] ctx settings context
 \param[in] name the name of the option
 \param[in] ival 0 or 1 to set, -1 to reset to default, and FO_PRINT_VALUE to
    print the current value
 */
void handle_option(Fo_Context ctx, const char *name, int ival) {
  const char *ctx_name = (ctx == FO_SYSTEM) ? "system" : "user";
  Fo_Option_Descr *opt;
  for (opt = g_option_list; opt->type != FO_END_OF_LIST; ++opt) {
    if ( opt->name && (fl_ascii_strcasecmp(opt->name, name) == 0) ) {
      if (ival == FO_PRINT_VALUE) {
        int value = get_option(ctx, opt->prefs_name);
        if (g_verbose)
          printf("Current value for %s option %s is %d\n", ctx_name, name, value);
        else
          printf("%d\n", value);
      } else if (ival == -1) {
        if (g_verbose) printf("Reset %s option %s to default\n", ctx_name, name);
        clear_option(ctx, opt->prefs_name);
      } else {
        if (g_verbose) printf("Set %s option %s to %d\n", ctx_name, name, ival);
        set_option(ctx, opt->prefs_name, ival);
      }
      if ( (ival != FO_PRINT_VALUE) && !write_permission(ctx) ) {
        fprintf(stderr, "ERROR: No write permission for %s options\n", ctx_name);
        exit(1);
      }
      break;
    }
  }
  if (opt->type == FO_END_OF_LIST)
    fprintf(stderr, "Warning: Unrecognized %s option \"%s\".\n", ctx_name, name);
}

/** FLTK callback for every item in the command line.

 Command line arguments start with "-S" for system wide settings or "-U" for
 user settings, followed by the option name, followed by a value "=0", "=1",
 "=ON", or "=OFF". If no value is give, we will print a "0" or "1" to stdout.

 \param[in] argc number of arguments in the list
 \param[in] argv pointer to the list of arguments
 \param[inout] index into current argument, increment for every used entry
 \return 1 if the entry was correct, 0 if it was not understood
 */
static int read_command_line_args(int argc, char** argv, int& i) {
  char cmd = 0;
  char opt[64] = "";
  char val[32] = "";
  int ival = FO_PRINT_VALUE;
  const char *arg = argv[i++];

  if ( (strcmp(arg, "--help") == 0)) {
    if (argv[i] && argv[i][0]) {
      print_info(argv[i]);
      i++;
    } else {
      print_usage(argv[0]);
    }
    g_batch_mode = 1;
    return 1;
  }
  if ( (strncmp(arg, "-h", 2) == 0) ) {
    if (arg[2]) {
      print_info(arg+2);
      i++;
    } else {
      print_usage(argv[0]);
    }
    g_batch_mode = 1;
    return 1;
  }
  if ( (strcmp(arg, "--help") == 0) || (strcmp(arg, "-h") == 0) ) {
    print_usage(argv[0]);
    g_batch_mode = 1;
    return 1;
  }
  if ( (strcmp(arg, "--verbose") == 0) || (strcmp(arg, "-v") == 0) ) {
    g_verbose = 1;
    return 1;
  }
  if (strcmp(arg, "-f") == 0) { // suppress write access error
    g_system_write_ok = g_user_write_ok = 1;
    return 1;
  }
  if (arg[0] == '-') {
    cmd = arg[1];
    if (cmd == 'U' || cmd == 'S') {
      const char *eq = strchr(arg+2, '=');
      if (eq) {
        size_t n = (eq - (arg+2));
        if (n == 0) {
          i--;
          return 0;
        }
        if (n > sizeof(opt)-1) n = sizeof(opt)-1;
        strlcpy(opt, arg+2, n+1);
        strlcpy(val, eq+1, sizeof(val));
        if (fl_ascii_strcasecmp(val, "ON") == 0)
          ival = 1;
        else if (fl_ascii_strcasecmp(val, "OFF") == 0)
          ival = 0;
        else if (fl_ascii_strcasecmp(val, "DEFAULT") == 0)
          ival = -1;
        else if (strcmp(val, "1") == 0)
          ival = 1;
        else if (strcmp(val, "0") == 0)
          ival = 0;
        else if (strcmp(val, "-1") == 0)
          ival = -1;
        else {
          fprintf(stderr, "Warning: Unrecognized value \"%s\" for option \"%s\".\n", val, opt);
          g_batch_mode = 1;
          return 1;
        }
      } else {
        strlcpy(opt, arg+2, sizeof(opt));
      }
    }
  }
  if (cmd == 'S') { // system setting
    handle_option(FO_SYSTEM, opt, ival);
    g_batch_mode = 1;
    return 1;
  }
  if (cmd == 'U') { // user setting
    handle_option(FO_USER, opt, ival);
    g_batch_mode = 1;
    return 1;
  }
  // check for -L, -LS, or -LU
  if (strcmp(arg, "-L") == 0 || strcmp(arg, "-LS") == 0 || strcmp(arg, "-LU") == 0) {
    list_options(arg[2]);
    g_batch_mode = 1;
    return 1;
  }
  i--;
  return 0;
}

/** Check if we have write permission for either database.

 If there is no write permission for a set of options, the UI will show
 them grayed out.

 \param[out] sys system preferences write permission
 \param[out] user user preferences write permission
 */
void check_write_permissions(int &sys, int &user) {
  char path[FL_PATH_MAX+1];
  sys = 0;
  Fl_Preferences sys_prefs(Fl_Preferences::SYSTEM_L, "fltk.org", "fltk");
  if (sys_prefs.file_access() & Fl_Preferences::SYSTEM_WRITE_OK) {
    path[0] = 0;
    sys_prefs.filename(path, FL_PATH_MAX);
    if ( path[0] && (fl_access(path, 2) == 0) ) // W_OK
      sys = 1;
  }
  user = 0;
  Fl_Preferences user_prefs(Fl_Preferences::USER_L, "fltk.org", "fltk");
  if (user_prefs.file_access() & Fl_Preferences::USER_WRITE_OK) {
    path[0] = 0;
    user_prefs.filename(path, FL_PATH_MAX);
    if ( path[0] && (fl_access(path, 2) == 0) ) // W_OK
      user = 1;
  }
}

/** Called when the UI Close button is clicked.
 */
static void close_cb(Fl_Widget*, void*) {
  Fl::hide_all_windows();
}

/** Called when a boolenan system option is changed.
 */
void set_system_option_cb(Fl_Widget* w, void* user_data) {
  Fl_Choice* choice = (Fl_Choice*)w;
  Fo_Option_Descr* opt = (Fo_Option_Descr*)user_data;
  const Fl_Menu_Item* mi = choice->mvalue();
  if (!mi) return;
  int value = (int)mi->argument();
  set_option(FO_SYSTEM, opt->prefs_name, value);
}

/** Called when a boolenan user option is changed.
 */
void set_user_option_cb(Fl_Widget* w, void* user_data) {
  Fl_Choice* choice = (Fl_Choice*)w;
  Fo_Option_Descr* opt = (Fo_Option_Descr*)user_data;
  const Fl_Menu_Item* mi = choice->mvalue();
  if (!mi) return;
  int value = (int)mi->argument();
  set_option(FO_USER, opt->prefs_name, value);
}

/** Add an option group to the pack group.
 */
void add_option(Fl_Pack* pack, Fo_Option_Descr* opt) {
  static Fl_Menu_Item bool_option_menu[] = {
    { "off",      0, 0, (void*)(0),   0},
    { "on",       0, 0, (void*)(1),   FL_MENU_DIVIDER},
    { "default",  0, 0, (void*)(-1),  0},
    { NULL }
  };
  // -- get the height of the tooltip text, so we can create the correct group size
  int tooltip_h = 0;
  if (opt->tooltip) {
    int ww = FO_OPTIONS_W - 2*FO_GAP - 6; // 6 is the FL_ALIGN_LEFT margin
    int hh = 100;
    fl_font(FL_HELVETICA, 11);
    fl_measure(opt->tooltip, ww, hh);
    tooltip_h = hh+5;
  }
  // -- create a group that contains all the UI elements for the option
  int yy = 0;
  Fl_Group* option_group = new Fl_Group(0, 0,
                                        FO_OPTIONS_W,
                                        2*FO_GAP + FO_BUTTON_H + tooltip_h + 2*FO_GAP + FO_CHOICE_H +17);
  opt->ui = option_group;
  yy += FO_GAP;
  // -- name of the options
  Fl_Box* name = new Fl_Box(0, yy, FO_OPTIONS_W, 21);
  name->copy_label(opt->name);
  name->align(FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
  name->labelfont(FL_HELVETICA_BOLD);
  yy += name->h();
  // -- description
  Fl_Box* brief = new Fl_Box(FO_GAP, yy,
                               FO_OPTIONS_W - 2*FO_GAP, tooltip_h,
                               opt->brief);
  brief->labelsize(11);
  brief->labelcolor(fl_lighter(FL_FOREGROUND_COLOR));
  brief->align(FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
  yy += 17;
  // -- tooltip, if one exists
  Fl_Box* tooltip = new Fl_Box(FO_GAP, yy,
                               FO_OPTIONS_W - 2*FO_GAP, tooltip_h,
                               opt->tooltip);
  tooltip->labelfont(FL_HELVETICA);
  tooltip->labelsize(11);
  tooltip->labelcolor(fl_lighter(FL_FOREGROUND_COLOR));
  tooltip->align(FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
  yy += tooltip_h;
  // -- default setting
  Fl_Box* default_setting = new Fl_Box(FO_GAP, yy, FO_SYSTEM_X - FO_GAP, 14);
  char buf[64];
  fl_snprintf(buf, 64, "Default is %s.",opt->bool_default ? "on" : "off");  // -- label the pulldown menus
  default_setting->copy_label(buf);
  default_setting->labelsize(11);
  default_setting->labelcolor(fl_lighter(FL_FOREGROUND_COLOR));
  default_setting->align(FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
  yy += 18;
  // -- option text
  Fl_Box* text = new Fl_Box(0, yy, FO_SYSTEM_X, FO_CHOICE_H);
  text->copy_label(opt->text);
  text->align(FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
  // -- system choice
  Fl_Choice* system_choice = new Fl_Choice(FO_SYSTEM_X, yy, FO_CHOICE_W, FO_CHOICE_H, "System:");
  system_choice->down_box(FL_BORDER_BOX);
  system_choice->labelsize(11);
  system_choice->align(FL_ALIGN_TOP);
  system_choice->callback((Fl_Callback*)set_system_option_cb, (void*)opt);
  system_choice->menu(bool_option_menu);
  switch (get_option(FO_SYSTEM, opt->prefs_name)) {
    case 0: system_choice->value(0); break;
    case 1: system_choice->value(1); break;
    default: system_choice->value(2); break;
  }
  if (!g_system_write_ok) system_choice->deactivate();
  // -- user choice
  Fl_Choice* user_choice = new Fl_Choice(FO_USER_X, yy, FO_CHOICE_W, FO_CHOICE_H, "User:");
  user_choice->down_box(FL_BORDER_BOX);
  user_choice->labelsize(11);
  user_choice->align(FL_ALIGN_TOP);
  user_choice->callback((Fl_Callback*)set_user_option_cb, (void*)opt);
  user_choice->menu(bool_option_menu);
  switch (get_option(FO_USER, opt->prefs_name)) {
    case 0: user_choice->value(0); break;
    case 1: user_choice->value(1); break;
    default: user_choice->value(2); break;
  }
  if (!g_user_write_ok) user_choice->deactivate();
      option_group->end();
}

/** Fill the options list and make the unused options invisible.
 */
void add_options(Fl_Pack* pack) {
  Fo_Option_Descr *opt;
  for (opt = g_option_list; opt->type != FO_END_OF_LIST; ++opt) {
    if (opt->type != FO_HEADLINE)
      add_option(pack, opt);
  }
}

void select_headline_cb(Fl_Widget*, void*) {
  int line = g_headline_browser->value();
  if (line) {
    Fo_Option_Descr* sel = (Fo_Option_Descr*)g_headline_browser->data(line);
    Fo_Option_Descr *opt;
    bool show = false;
    for (opt = g_option_list; opt->type != FO_END_OF_LIST; ++opt) {
      if (opt->type == FO_HEADLINE) show = false;
      if (opt == sel) {
        show = true;
        g_option_scroll->label(opt->text);
      }
      if (opt->ui) {
        if (show)
          opt->ui->show();
        else
          opt->ui->hide();
      }
    }
  } else {
    Fo_Option_Descr *opt;
    for (opt = g_option_list; opt->type != FO_END_OF_LIST; ++opt) {
      if (opt->ui)
        opt->ui->show();
    }
    g_option_scroll->label("All Options");
  }
  g_option_scroll->scroll_to(-FO_SCROLL_W+Fl::box_dy(g_option_scroll->box()), 0);
  g_window->redraw();
}

/** Fill the headline browser and set callbacks.
 */
void add_headlines(Fl_Hold_Browser* browser) {
  Fo_Option_Descr *opt;
  for (opt = g_option_list; opt->type != FO_END_OF_LIST; ++opt) {
    if (opt->type == FO_HEADLINE)
      browser->add(opt->text, (void*)opt);
  }
  browser->callback(select_headline_cb, NULL);
}

/** Build the modular user interface (ALT VERSION).
 */
Fl_Window* build_ui() {
  Fl_Tooltip::size(11);
  // -- the main application window
  g_window = new Fl_Double_Window(FO_WINDOW_W, FO_WINDOW_H,
                                  "FLTK Options");
  // -- a clickable browser for all headlines
  g_headline_browser = new Fl_Hold_Browser(FO_GAP, FO_GAP + FO_TITLE_H,
                                           FO_BROWSER_W, FO_BROWSER_H - FO_TITLE_H,
                                           "Groups");
  g_headline_browser->align(FL_ALIGN_TOP);
  g_headline_browser->textsize(FL_NORMAL_SIZE+1);
  g_headline_browser->linespacing(4);
  add_headlines(g_headline_browser);
  // -- scrollable area for all options inside a group
  g_option_scroll = new Fl_Scroll(FO_GAP + FO_BROWSER_W + FO_GAP, FO_GAP + FO_TITLE_H,
                                  FO_OPTIONS_W +2*FO_SCROLL_W, FO_BROWSER_H - FO_TITLE_H,
                                  "All Options");
  g_option_scroll->box(FL_DOWN_BOX);
  g_option_scroll->type(Fl_Scroll::VERTICAL);
  Fl_Pack* option_list = new Fl_Pack(g_option_scroll->x() + FO_SCROLL_W,
                                     g_option_scroll->y() + Fl::box_dy(FL_DOWN_BOX),
                                     FO_OPTIONS_W, 10);
  add_options(option_list);
  // -- end all groups
  g_option_scroll->end();
  g_window->resizable(g_option_scroll);
  // -- close button
  Fl_Button* close = new Fl_Button(FO_WINDOW_W - 2*FO_GAP-FO_BUTTON_W,
                                   FO_WINDOW_H - FO_GAP - FO_BUTTON_H,
                                   FO_BUTTON_W, FO_BUTTON_H,
                                   "Close");
  close->callback(close_cb);
  g_window->end();
  g_window->size_range(FO_WINDOW_W, FO_WINDOW_H);
  return g_window;
}

/** This is where it all begins.
 */
int main(int argc,char **argv) {
  check_write_permissions(g_system_write_ok, g_user_write_ok);

  int i = 1;
  Fl::args_to_utf8(argc, argv); // for MSYS2/MinGW
  int args_processed = Fl::args(argc, argv, i, read_command_line_args);
  if (args_processed < argc) {
    fprintf(stderr, "ERROR: Unrecognized command line option \"%s\".\n", argv[i]);
    return 1;
  }
  if (g_batch_mode)
    return 0;

  Fl_Window *win = build_ui();
  win->show(argc, argv);
  Fl::run();

  return 0;
}
