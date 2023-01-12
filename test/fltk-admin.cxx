//
// fltk-option for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022-2023 by Bill Spitzak and others.
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
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/filename.H>
#include "../src/flstring.h"

#include <stdlib.h>
#include <stdio.h>

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

// Avalable option types.
// We can add more types later as needed.
// See: Fl_Group* add_option( Fo_Option_Descr* opt)
// See: Fl_Group* add_option_bool(Fo_Option_Descr* opt)
typedef enum {
  FO_END_OF_LIST = 0,
  FO_HEADLINE,
  FO_OPTION_BOOL,
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
  bool bool_default;
  const char* brief;
  const char* tooltip;
} Fo_Option_Descr;

// List of all entries in the user interface, including headlines and options.
// This list can be arbitrarily extended. The app will generate a scrollable
// area if the list does not fit the app window.
Fo_Option_Descr g_option_list[] = {
  { FO_HEADLINE, "Keyboard Focus Options" },
  { FO_OPTION_BOOL, "Visible Keyboard Focus:",
    Fl::OPTION_VISIBLE_FOCUS, "OPTION_VISIBLE_FOCUS", true,
    "draw a dotted rectangle in widget with keyboard focus",
    "If visible focus is switched on, FLTK will draw a dotted rectangle inside "
    "the widget that will receive the next keystroke. If switched off, no such "
    "indicator will be drawn and keyboard navigation is disabled." },
  { FO_OPTION_BOOL, "Arrow Keys move Focus:",
    Fl::OPTION_ARROW_FOCUS, "OPTION_ARROW_FOCUS", false,
    "arrow keys will move focus beyond text input field",
    "When switched on, moving the text cursor beyond the start or end of the "
    "text in a text widget will change focus to the next widget. When switched "
    "off, the cursor will stop at the end of the text. Pressing Tab or Ctrl-Tab "
    "will advance the keyboard focus. Switch this on, if you want the old "
    "behavior of FLTK 1.1." },
  { FO_HEADLINE, "Tooltip Options" },
  { FO_OPTION_BOOL, "Show Tooltips:",
    Fl::OPTION_SHOW_TOOLTIPS, "OPTION_SHOW_TOOLTIPS", true,
    "show or hide tooltips",
    "If tooltips are enabled, hovering the mouse over a widget with a tooltip "
    "text will open a little tooltip window until the mouse leaves the widget. "
    "If disabled, no tooltip is shown." },
  { FO_HEADLINE, "Drag And Drop Options" },
  { FO_OPTION_BOOL, "Allow dragging Text:",
    Fl::OPTION_DND_TEXT, "OPTION_DND_TEXT", true,
    "user can drag text from FLTK into other apps",
    "If text drag-and-drop is enabled, the user can select and drag text from "
    "any text widget. If disabled, no dragging is possible, however dropping "
    "text from other applications still works." },
  { FO_HEADLINE, "Native File Chooser Options" },
  { FO_OPTION_BOOL, "Native File Chooser uses GTK:",
    Fl::OPTION_FNFC_USES_GTK, "OPTION_FNFC_USES_GTK", true,
    "use GTK file chooser instead of FLTK if available",
    "If 'Native File Chooser uses GTK' is enabled, the Fl_Native_File_Chooser "
    "class calls the GTK open/save file dialogs when they are available on the "
    "platfom. If disabled, the Fl_Native_File_Chooser class always uses FLTK's "
    "own file dialog (i.e., Fl_File_Chooser) even if GTK is available." },
  { FO_HEADLINE, "Print dialog Options" },
  { FO_OPTION_BOOL, "Print dialog uses GTK:",
    Fl::OPTION_PRINTER_USES_GTK, "OPTION_PRINTER_USES_GTK", true,
    "use GTK printer dialog instead of FLTK if available",
    "If 'Print dialog uses GTK' is enabled, the Fl_Printer class calls the "
    "GTK print dialog when it's available on the platfom. If disabled, the "
    "Fl_Printer class always uses FLTK's own print dialog even "
    "if GTK is available." },
  { FO_HEADLINE, "Scaling Factor Options" },
  { FO_OPTION_BOOL, "Transiently show scaling factor:",
    Fl::OPTION_SHOW_SCALING, "OPTION_SHOW_SCALING", true,
    "show the zoom factor in a transient popup window",
    "If 'Transiently show scaling factor' is enabled, the library shows in a "
    "transient popup window the display scaling factor value when it is "
    "changed. If disabled, no such transient window is used." },
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
    (ctx==FO_SYSTEM) ? Fl_Preferences::SYSTEM_L : Fl_Preferences::USER_L;
  Fl_Preferences prefs(context, "fltk.org", "fltk");
  Fl_Preferences options(prefs, "options");
  if (value==-1)
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
    (ctx==FO_SYSTEM) ? Fl_Preferences::SYSTEM_L : Fl_Preferences::USER_L;
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
 \param[in] argv application name when called from the shell
 */
void print_usage(const char *argv0) {
  const char *app_name = NULL;
  if (argv0 && argv0[0])
    app_name = fl_filename_name(argv0);
  if (!app_name || !app_name[0])
    app_name = "fltk-option";
  fprintf(stderr, "FLTK %d.%d.%d. Usage:\n", FL_MAJOR_VERSION, FL_MINOR_VERSION, FL_PATCH_VERSION);
  fprintf(stderr, "%s [-Soption[=val]] [-Uoption[=val]] [-L] [-LS] [-LU] [-f] [-v] [-h]\n", app_name);
  fprintf(stderr, "  -Soption[=value]  change or print system wide option\n");
  fprintf(stderr, "  -Uoption[=value]  change or print user option\n");
  fprintf(stderr, "      Values can be 0 or OFF to clear, or 1 or ON to set the option.\n"
                  "      The value -1 or DEFAULT sets the option to its default value.\n"
                  "      If no value is given, the current setting is returned as -1, 0, or 1.\n");
  fprintf(stderr, "  -L, -LS, -LU  lists all current settings, system settings, or user setting\n");
  fprintf(stderr, "  -f  suppresses error messages concerning file access permissions\n");
  fprintf(stderr, "  -v, --verbose  prints additional informatin in command line mode\n");
  fprintf(stderr, "  -h, --help  prints this page\n\n");
  fprintf(stderr, "    This version of %s supports the following options:\n", app_name);
  Fo_Option_Descr *opt;
  for (opt = g_option_list; opt->type!=FO_END_OF_LIST; ++opt) {
    if (opt->name) {
      if (opt->brief)
        fprintf(stderr, "  %-24s %s\n", opt->name, opt->brief);
      else
        fprintf(stderr, "  %s\n", opt->name);
    }
  }
  fprintf(stderr, "\n  Calling %s without options will launch %s interactive mode.\n", app_name, app_name);
}

/** List the current value of all options.
 \param[in] cmd 'S' lists system options, 'U' lists user options, 0 lists both
 */
void list_options(char cmd) {
  Fo_Option_Descr *opt;
  for (opt = g_option_list; opt->type!=FO_END_OF_LIST; ++opt) {
    if (opt->name) {
      printf("%-24s", opt->name);
      if (cmd == 'S' || cmd == 0) {
        int value = get_option(FO_SYSTEM, opt->name);
        printf(" system:%2d", value);
      }
      if (cmd == 0)
        printf(",");
      if (cmd == 'U' || cmd == 0) {
        int value = get_option(FO_USER, opt->name);
        printf(" user:%2d", value);
      }
      printf("\n");
    }
  }
}

/** Handle a commmand line argument for system or user options.
 \param[in] ctx settings context
 \param[in] name the name of the option
 \param ival 0 or 1 to set, -1 to reset to default, and FO_PRINT_VALUE to
    print the current value
 */
void handle_option(Fo_Context ctx, const char *name, int ival) {
  const char *ctx_name = (ctx==FO_SYSTEM) ? "system" : "user";
  Fo_Option_Descr *opt;
  for (opt = g_option_list; opt->type!=FO_END_OF_LIST; ++opt) {
    if ( opt->name && (fl_ascii_strcasecmp(opt->name, name) == 0) ) {
      if (ival == FO_PRINT_VALUE) {
        int value = get_option(ctx, opt->name);
        if (g_verbose)
          printf("Current value for %s option %s is %d\n", ctx_name, name, value);
        else
          printf("%d\n", value);
      } else if (ival ==-1) {
        if (g_verbose) printf("Reset %s option %s to default\n", ctx_name, name);
        clear_option(ctx, opt->name);
      } else {
        if (g_verbose) printf("Set %s option %s to %d\n", ctx_name, name, ival);
        set_option(ctx, opt->name, ival);
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
  if (strcmp(arg, "-L")==0 || strcmp(arg, "-LS")==0 || strcmp(arg, "-LU")==0) {
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
  set_option(FO_SYSTEM, opt->name, value);
}

/** Called when a boolenan user option is changed.
 */
void set_user_option_cb(Fl_Widget* w, void* user_data) {
  Fl_Choice* choice = (Fl_Choice*)w;
  Fo_Option_Descr* opt = (Fo_Option_Descr*)user_data;
  const Fl_Menu_Item* mi = choice->mvalue();
  if (!mi) return;
  int value = (int)mi->argument();
  set_option(FO_USER, opt->name, value);
}

/** Add a headline to a group of options.
 \param[in] opt pointer to the headline record
 \param[in] first true, if this is the first headline to generate
 */
Fl_Group* add_headline(Fo_Option_Descr* opt, bool first) {
  if (!first) {
    Fl_Group* divider_container = new Fl_Group(0, 0, 385, 10);
    Fl_Box *divider = new Fl_Box(0, 4, 385, 2);
    divider->box(FL_THIN_DOWN_BOX);
    divider_container->end();
  }

  Fl_Group* container = new Fl_Group(0, 0, 385, 26);

  Fl_Box* headline = new Fl_Box(0, 0, 215, 26, opt->text);
  headline->labelfont(FL_HELVETICA_ITALIC);
  headline->labelsize(15);
  headline->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

  Fl_Box* system = new Fl_Box(210, 5, 75, 20, "System:");
  system->labelsize(11);
  system->align(FL_ALIGN_BOTTOM|FL_ALIGN_INSIDE);

  Fl_Box* user = new Fl_Box(300, 5, 75, 20, "User:");
  user->labelsize(11);
  user->align(FL_ALIGN_BOTTOM|FL_ALIGN_INSIDE);

  container->end();
  return container;
}

/** Add a boolean option to the UI.
 \param[in] opt pointer to the bool option record
 */
Fl_Group* add_option_bool(Fo_Option_Descr* opt) {
  static Fl_Menu_Item bool_option_menu[] = {
    { "off",      0, 0, (void*)(0),   0},
    { "on",       0, 0, (void*)(1),   FL_MENU_DIVIDER},
    { "default",  0, 0, (void*)(-1),  0},
    { NULL }
  };

  Fl_Group* container = new Fl_Group(0, 0, 385, 30);
  if (opt->tooltip) {
    size_t tooltip_len = strlen(opt->name) + strlen(opt->tooltip) + 32;
    char *tooltip_str = (char*)malloc(tooltip_len); // tollerable memeory leak
    fl_snprintf(tooltip_str, tooltip_len-1, "%s\n\n%s\n\nDefault is %s.",
                opt->name, opt->tooltip, opt->bool_default ? "on" : "off");
    container->tooltip(tooltip_str);
  }

  Fl_Choice* system_choice = new Fl_Choice(210, 4, 75, 22, opt->text);
  system_choice->down_box(FL_BORDER_BOX);
  system_choice->labelsize(13);
  system_choice->callback((Fl_Callback*)set_system_option_cb, (void*)opt);
  system_choice->menu(bool_option_menu);
  switch (get_option(FO_SYSTEM, opt->name)) {
    case 0: system_choice->value(0); break;
    case 1: system_choice->value(1); break;
    default: system_choice->value(2); break;
  }
  if (!g_system_write_ok) system_choice->deactivate();

  Fl_Choice* user_choice = new Fl_Choice(300, 4, 75, 22);
  user_choice->down_box(FL_BORDER_BOX);
  user_choice->labelsize(13);
  user_choice->callback((Fl_Callback*)set_user_option_cb, (void*)opt);
  user_choice->menu(bool_option_menu);
  switch (get_option(FO_USER, opt->name)) {
    case 0: user_choice->value(0); break;
    case 1: user_choice->value(1); break;
    default: user_choice->value(2); break;
  }
  if (!g_user_write_ok) user_choice->deactivate();

  container->end();
  return container;
}

/** Add any option record to the UI.
 \param[in] opt pointer to the bool option record
 */
Fl_Group* add_option( Fo_Option_Descr* opt) {
  Fl_Group* ret = NULL;
  switch (opt->type) {
    case FO_HEADLINE:
      ret = add_headline(opt, (opt == g_option_list) );
      break;
    case FO_OPTION_BOOL:
      ret = add_option_bool(opt);
      break;
    case FO_END_OF_LIST:
      break;
  }
  return ret;
}

/** Build the modular user interface.
 */
Fl_Window* build_ui() {
  int sb = Fl::scrollbar_size();
  int cw = 385;
  int ww = 10+sb+cw+sb+10;
  int sh = 5*10 + 6*26 + 7*30;
  Fl_Tooltip::size(11);
  Fl_Window* window = new Fl_Double_Window(ww, 10+sh+10+25+10, "FLTK Options");
  Fl_Scroll* option_scroll = new Fl_Scroll(10, 10, ww-20, sh);
  option_scroll->type(Fl_Scroll::VERTICAL);
  Fl_Pack* option_list = new Fl_Pack(10+sb, 10, ww-20-2*sb, sh);
  Fo_Option_Descr *opt;
  for (opt = g_option_list; opt->type!=FO_END_OF_LIST; ++opt) {
    add_option(opt);
  }
  option_list->end();
  option_scroll->end();
  window->resizable(option_scroll);
  Fl_Button* close = new Fl_Button(ww-75-20, 10+sh+10, 75, 25, "Close");
  close->callback(close_cb);
  window->end();
  window->size_range(ww, 120, ww, 32785);
  return window;
}

/** This is where it all begins.
 */
int main(int argc,char **argv) {
  check_write_permissions(g_system_write_ok, g_user_write_ok);

  int i = 1;
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
