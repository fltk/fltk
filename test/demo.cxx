//
// Main demo program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

/*
  General information on directory structure and file handling.

  The "classic" autotools/make system creates executables in their source
  folders, i.e. fluid/fluid, test/demo and test/xyz, resp.. The menu file is
  in folder test/, as is the main demo(.exe) program. In the following text
  and directory lists all test and demo executables are represented by "demo"
  and the fluid executable by "fluid", no matter what OS (under Windows: *.exe).

  The CMake build system generates all executables in the build tree and copies
  the supporting test data files to the build tree as well. This structure is
  different and needs to be handled separately in this program.

  Additionally, different OS platforms create different types of files, for
  instance "app bundles" on macOS. All this needs to be considered.

  The overall structure, relative to the FLTK source dir (fltk) and the build
  tree (build):

  (1) Autotools / Make:

    fltk/fluid              fluid (../fluid/fluid)
    fltk/test               demo, demo.menu, working directory, data files
    fltk/test/images        images for help_dialog(.html)

  (2) CMake + make (e.g. Unix)

    build/bin               fluid
    build/bin/test          test and demo programs
    build/data              demo.menu, working directory, data files
    build/data/images       images for help_dialog(.html)

  (3) CMake + Visual Studio (TYPE == build type: Debug, Release, ...)

    build/bin/TYPE          fluid
    build/bin/test/TYPE     test and demo programs
    build/data              demo.menu, working directory, data files
    build/data/images       images for help_dialog(.html)

  (4) macOS                 The setup is similar to Windows and Linux:
                            Makefiles: like (1) or (2)
                            Xcode: like (3), i.e. similar to VS layout

  The built executable 'demo' can also be executed with the menu filename
  as commandline argument. In this case all the support (data) files are
  expected to be in the same directory as the menu file or relative paths
  as needed by the test programs, for instance help_dialog which needs
  help_dialog.html and related image files.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Button.H>      // right click popup menu
#include <FL/Fl_Scheme_Choice.H>
#include <FL/Fl_Terminal.H>         // tty
#include <FL/filename.H>
#include <FL/platform.H>
#include <FL/fl_ask.H>              // fl_alert()
#include <FL/fl_utf8.h>             // fl_getcwd()

// Define USE_MAC_OS for convenience (below). We use macOS specific features (bundles
// and paths) if USE_MAC_OS is defined, otherwise we're using X11 (XQuartz) on macOS

#if defined __APPLE__ && !defined(FLTK_USE_X11)
#define USE_MAC_OS
#include <ApplicationServices/ApplicationServices.h>
#endif

#define FORM_W 350
#define FORM_H 440
#define TTY_W  780
#define TTY_H  200

/* The form description */

void doexit(Fl_Widget *, void *);
void doback(Fl_Widget *, void *);
void dobut(Fl_Widget *, long);

Fl_Double_Window *form = 0;
Fl_Group *demogrp = 0;
Fl_Terminal *tty = 0;
Fl_Scheme_Choice *scheme_choice = 0;
Fl_Button *but[9];
Fl_Button *exit_button;

// Allocate space to edit commands and arguments from demo.menu.
// We "trust demo.menu" that strings don't overflow

char cmdbuf[256];           // commandline w/o arguments
char params[256];           // commandline arguments

// Global path variables for all platforms and build systems
// to avoid duplication and dynamic allocation

char app_path     [FL_PATH_MAX];          // directory of all demo binaries
char fluid_path   [FL_PATH_MAX];          // binary directory of fluid
char options_path [FL_PATH_MAX];          // binary directory of fltk-options
char data_path    [FL_PATH_MAX];          // working directory of all demos
char command      [2 * FL_PATH_MAX + 40]; // command to be executed

// platform specific suffix for executable files

#ifdef _WIN32
const char *suffix = ".exe";
#elif defined USE_MAC_OS
const char *suffix = ".app";
#else
const char *suffix = "";
#endif

// CMake defines the "build type" subdirectory for multi configuration
// build setups like Visual Studio and Xcode

#ifdef CMAKE_INTDIR
const char *cmake_intdir = "/" CMAKE_INTDIR;
#else
const char *cmake_intdir = 0;
#endif

// debug output function
void debug_var(const char *varname, const char *value) {
  tty->printf("%-10s = %s\n", varname, value);
}

// Show or hide the tty window. Generally this could be much simpler
// but the extra space (10 px) at the bottom needs "special care"

void show_tty(int val) {
  if (val) {
    tty->show();                                        // show debug terminal
    form->size_range(FORM_W, FORM_H + TTY_H/2, 0, 0);   // allow resizing
    form->size(TTY_W + 20, FORM_H + TTY_H + 10);        // demo + height for tty + space (10)
    tty->size(TTY_W, TTY_H);                            // force tty size
  } else {
    tty->hide();                                        // hide debug terminal
    form->size_range(FORM_W, FORM_H, FORM_W, FORM_H);   // no resizing
    form->size(FORM_W, FORM_H);                         // normal demo size
    tty->resize(10, FORM_H - 1, FORM_W - 20, 1);        // restore original position and size
  }
  form->init_sizes();
  exit_button->take_focus();
}

// Right click popup menu handler (1 = show, 0 = hide)
void popup_menu_cb(Fl_Widget*, void *userdata) {
  show_tty(fl_int(userdata));
}

void create_the_forms() {
  Fl_Widget *obj;
  Fl_Menu_Button *popup;
  form = new Fl_Double_Window(FORM_W, FORM_H, "FLTK Demonstration");

  // Parent group for demo
  demogrp = new Fl_Group(0, 0, FORM_W, FORM_H - 1);

  // Top demo button
  obj = new Fl_Box(FL_FRAME_BOX, 10, 15, 330, 40, "FLTK Demonstration");
  obj->color(FL_GRAY-4);
  obj->labelsize(24);
  obj->labelfont(FL_BOLD);
  obj->labeltype(FL_ENGRAVED_LABEL);

  obj = new Fl_Box(FL_FRAME_BOX, 10, 65, 330, 330, 0);
  obj->color(FL_GRAY-8);

  scheme_choice = new Fl_Scheme_Choice(90, 405, 100, 25, "Scheme:");
  scheme_choice->labelfont(FL_HELVETICA_BOLD);

  exit_button = new Fl_Button(280, 405, 60, 25, "Exit");
  exit_button->callback(doexit);
  exit_button->take_focus();

  obj = new Fl_Button(10, 15, 330, 380);
  obj->type(FL_HIDDEN_BUTTON);
  obj->callback(doback);
  obj->tooltip("Use right mouse button to show/hide debug terminal");

  but[0] = new Fl_Button( 30, 85,90,90);
  but[1] = new Fl_Button(130, 85,90,90);
  but[2] = new Fl_Button(230, 85,90,90);
  but[3] = new Fl_Button( 30,185,90,90);
  but[4] = new Fl_Button(130,185,90,90);
  but[5] = new Fl_Button(230,185,90,90);
  but[6] = new Fl_Button( 30,285,90,90);
  but[7] = new Fl_Button(130,285,90,90);
  but[8] = new Fl_Button(230,285,90,90);

  for (int i=0; i<9; i++) {
    but[i]->align(FL_ALIGN_WRAP);
    but[i]->callback(dobut, i);
  }

  // Right click popup menu (inside demogrp)
  popup = new Fl_Menu_Button(0,0,FORM_W,FORM_H);
  popup->box(FL_NO_BOX);
  popup->type(Fl_Menu_Button::POPUP3); // pop menu on right-click
  popup->add("Show debug terminal", 0, popup_menu_cb, (void*)1);
  popup->add("Hide debug terminal", 0, popup_menu_cb, (void*)0);

  // The resizable box of 'demogrp' ensures that the demo form is not resized
  // if the user resizes the window while the debug terminal (tty) is shown
  obj = new Fl_Box(FORM_W - 1, 0, 1, FORM_H);
  obj->box(FL_NO_BOX);
  demogrp->resizable(obj);

  demogrp->end();

  // Small debug terminal window parented to window, not demogrp
  //    To show/hide debug terminal, use demo's right-click menu
  //
  tty = new Fl_Terminal(10, FORM_H - 1, FORM_W - 20, 1);
  tty->history_lines(50);
  tty->display_rows(2);       // make display at least 2 rows high, even if not seen
  tty->display_columns(100);  // make display at least 100 cols wide, even if not seen
  tty->ansi(true);
  tty->hide();
  tty->textsize(12);

  // End window
  form->end();
  form->resizable(tty);

  // Note: do not set size_range() before show() or window can't be made resizable
  // later (macOS and Windows only, works on Linux though)
  // form->size_range(FORM_W, FORM_H, FORM_W, FORM_H);
}

/* Maintaining and building up the menus. */

typedef struct {
  char name[64];
  int numb;
  char iname[9][64];
  char icommand[9][64];
} MENU;

#define MAXMENU 32

MENU menus[MAXMENU];
int mennumb = 0;

/* Return the number of a given menu name. */
int find_menu(const char* nnn) {
  int i;
  for (i=0; i<mennumb; i++)
    if (strcmp(menus[i].name,nnn) == 0) return i;
  return -1;
}

/* Create a new menu with name nnn */
void create_menu(const char* nnn) {
  if (mennumb == MAXMENU -1) return;
  strcpy(menus[mennumb].name,nnn);
  menus[mennumb].numb = 0;
  mennumb++;
}

/* Add an item to a menu */
void addto_menu(const char* men, const char* item, const char* comm) {
  int n = find_menu(men);
  if (n<0) { create_menu(men); n = find_menu(men); }
  if (menus[n].numb == 9) return;
  strcpy(menus[n].iname[menus[n].numb],item);
  strcpy(menus[n].icommand[menus[n].numb],comm);
  menus[n].numb++;
}

/* Button to Item conversion and back. */

int b2n[][9] = {
        { -1, -1, -1, -1,  0, -1, -1, -1, -1},
        { -1, -1, -1,  0, -1,  1, -1, -1, -1},
        {  0, -1, -1, -1,  1, -1, -1, -1,  2},
        {  0, -1,  1, -1, -1, -1,  2, -1,  3},
        {  0, -1,  1, -1,  2, -1,  3, -1,  4},
        {  0, -1,  1,  2, -1,  3,  4, -1,  5},
        {  0, -1,  1,  2,  3,  4,  5, -1,  6},
        {  0,  1,  2,  3, -1,  4,  5,  6,  7},
        {  0,  1,  2,  3,  4,  5,  6,  7,  8}
};
int n2b[][9] = {
        {  4, -1, -1, -1, -1, -1, -1, -1, -1},
        {  3,  5, -1, -1, -1, -1, -1, -1, -1},
        {  0,  4,  8, -1, -1, -1, -1, -1, -1},
        {  0,  2,  6,  8, -1, -1, -1, -1, -1},
        {  0,  2,  4,  6,  8, -1, -1, -1, -1},
        {  0,  2,  3,  5,  6,  8, -1, -1, -1},
        {  0,  2,  3,  4,  5,  6,  8, -1, -1},
        {  0,  1,  2,  3,  5,  6,  7,  8, -1},
        {  0,  1,  2,  3,  4,  5,  6,  7,  8}
};

/* Transform a button number to an item number when there are
  maxnumb items in total. -1 if the button should not exist. */
int but2numb(int bnumb, int maxnumb)
{ return b2n[maxnumb][bnumb]; }

/* Transform an item number to a button number when there are
  maxnumb items in total. -1 if the item should not exist. */
int numb2but(int inumb, int maxnumb)
{ return n2b[maxnumb][inumb]; }

/* Pushing and Popping menus */

char stack[64][32];
int stsize = 0;

/* Push a menu to be visible */
void push_menu(const char* nnn) {
  int n,i,bn;
  int men = find_menu(nnn);
  if (men < 0) return;
  n = menus[men].numb;
  for (i=0; i<9; i++) but[i]->hide();
  for (i=0; i<n; i++)
  {
    bn = numb2but(i,n-1);
    but[bn]->show();
    but[bn]->label(menus[men].iname[i]);
    if (menus[men].icommand[i][0] != '@') but[bn]->tooltip(menus[men].icommand[i]);
    else but[bn]->tooltip(0);
  }
  if (stack[stsize]!=nnn)
    strcpy(stack[stsize],nnn);
  stsize++;
}

/* Pop a menu */
void pop_menu() {
  if (stsize<=1) return;
  stsize -= 2;
  push_menu(stack[stsize]);
}

/* The callback Routines */

/* Handle a button push */
void dobut(Fl_Widget *, long arg) {
  int men = find_menu(stack[stsize-1]);
  int n = menus[men].numb;
  int bn = but2numb( (int) arg, n-1);

  // menu ?

  if (menus[men].icommand[bn][0] == '@') {
    push_menu(menus[men].icommand[bn]);
    return;
  }

  // not a menu: run test/demo/fluid executable
  // find and separate "command" and "params"

  // skip leading spaces in command
  char *start_command = menus[men].icommand[bn];
  while (*start_command == ' ') ++start_command;

  strcpy(cmdbuf, start_command); // here still full command w/params

  // find the space between the command and parameters if one exists
  char *start_params = strchr(cmdbuf, ' ');
  if (start_params) {
    *start_params = '\0';         // terminate command
    start_params++;               // skip space
    strcpy(params, start_params); // copy parameters
  } else {
    params[0] = '\0';             // empty string
  }

  // select application path: either app_path or fluid_path

  const char *path = app_path;
  if (!strncmp(cmdbuf, "fluid", 5))
    path = fluid_path;
  else if (!strncmp(cmdbuf, "fltk-options", 5))
    path = options_path;

  // format commandline with optional parameters

#if defined(USE_MAC_OS) // macOS

  if (params[0]) {
    // we assume that we have only one argument which is a filename in 'data_path'
    snprintf(command, sizeof(command), "open '%s/%s%s' --args '%s/%s'", path, cmdbuf, suffix, data_path, params);
  } else {
    snprintf(command, sizeof(command), "open '%s/%s%s'", path, cmdbuf, suffix);
  }

#else // other platforms

  if (params[0])
    snprintf(command, sizeof(command), "%s/%s%s %s", path, cmdbuf, suffix, params);
  else
    snprintf(command, sizeof(command), "%s/%s%s", path, cmdbuf, suffix);

#endif

  // finally, execute program (the system specific part)

#ifdef _WIN32

  STARTUPINFO         suInfo;         // Process startup information
  PROCESS_INFORMATION prInfo;         // Process information

  memset(&suInfo, 0, sizeof(suInfo));
  suInfo.cb = sizeof(suInfo);

  debug_var("Command", command);

  BOOL stat = CreateProcess(NULL, command, NULL, NULL, FALSE,
                            NORMAL_PRIORITY_CLASS, NULL,
                            NULL, &suInfo, &prInfo);
  if (!stat) {
    DWORD err = GetLastError();
    fl_alert("Error starting process, error #%lu\n'%s'", err, command);
  }

#elif defined USE_MAC_OS

  debug_var("Command", command);

  system(command);

#else // other platforms (Unix, Linux, X11, and XQuartz on macOS)

  strcat(command, " &"); // run in background

  debug_var("Command", command);

  if (system(command) == -1) {
    fl_alert("Could not start program, errno = %d\n'%s'", errno, command);
  }

#endif // _WIN32

}

void doback(Fl_Widget *, void *) {pop_menu();}

void doexit(Fl_Widget *, void *) {exit(0);}

/*
  Load the menu file. Returns whether successful.
*/
int load_the_menu(const char * const menu) {
  FILE *fin = 0;
  char line[256], mname[64],iname[64],cname[64];
  int i, j;

  fin = fl_fopen(menu, "r");

  if (fin == NULL)
    return 0;

  for (;;) {
    if (fgets(line,256,fin) == NULL) break;
    // remove all carriage returns that Cygwin may have inserted
    char *s = line, *d = line;
    for (;;++d) {
      while (*s=='\r') s++;
      *d = *s++;
      if (!*d) break;
    }
    // interpret the line
    j = 0; i = 0;
    while (line[i] == ' ' || line[i] == '\t') i++;
    if (line[i] == '\n') continue;
    if (line[i] == '#') continue;
    while (line[i] != ':' && line[i] != '\n') mname[j++] = line[i++];
    mname[j] = '\0';
    if (line[i] == ':') i++;
    j = 0;
    while (line[i] != ':' && line[i] != '\n') {
      if (line[i] == '\\') {
        i++;
        if (line[i] == 'n') iname[j++] = '\n';
        else iname[j++] = line[i];
        i++;
      } else
        iname[j++] = line[i++];
    }
    iname[j] = '\0';
    if (line[i] == ':') i++;
    j = 0;
    while (line[i] != ':' && line[i] != '\n') cname[j++] = line[i++];
    cname[j] = '\0';
    addto_menu(mname,iname,cname);
  }
  fclose(fin);
  return 1;
}

// Fix '\' in Windows paths (convert to '/') and cut off filename (optional, default)
void fix_path(char *path, int strip_filename = 1) {
  if (!path[0])
    return;
#ifdef _WIN32 // convert '\' to '/'
  char *p = path;
  while (*p) {
    if (*p == '\\')
      *p = '/';
    p++;
  }
#endif // _WIN32
  if (strip_filename) {
    char *pos = strrchr(path, '/');
    if (pos)
      *pos = 0;
  }
}

int main(int argc, char **argv) {

  fl_putenv("FLTK_DOCDIR=../documentation/html"); // used by fluid

  char menu[FL_PATH_MAX];

  // construct app_path for all executable files

  fl_filename_absolute(app_path, sizeof(app_path), argv[0]);
#if defined(USE_MAC_OS)
    char *q = strstr(app_path, "/Contents/MacOS/");
    if (q) *q = 0;
#endif
  fix_path(app_path);

  // fluid's path is relative to app_path:
  // - "../fluid" for autoconf/make builds
  // - ".." (parent directory) for single configuration CMake builds
  // - "../../$CMAKE_INTDIR" for multi-config (Visual Studio or Xcode) CMake builds

  strcpy(fluid_path, app_path);
  strcpy(options_path, app_path);

  if (cmake_intdir) {
    fix_path(fluid_path); // remove intermediate (build type) folder, e.g. "/Debug"
    fix_path(options_path);
  }

  fix_path(fluid_path); // remove folder name ("test")
  fix_path(options_path);

#if !defined(GENERATED_BY_CMAKE)
  strcat(fluid_path, "/fluid");
  strcat(options_path, "/fltk-options");
#else
  // CMake: potentially Visual Studio or Xcode (multi config)
  if (cmake_intdir) {
    strcat(fluid_path, cmake_intdir); // append e.g. "/Debug"
    strcat(options_path, cmake_intdir);
  }
#endif // GENERATED_BY_CMAKE

  // construct data_path for the menu file and all resources (data files)
  // CMake: replace "/bin/test/*" with "/data"
  // autotools: use app_path directly

  strcpy(data_path, app_path);

#if defined(GENERATED_BY_CMAKE)
  {
    char *pos = strstr(data_path, "/bin/test");
    if (pos)
      strcpy(pos, "/data");
  }
#endif // GENERATED_BY_CMAKE

  // Construct the menu file name, optionally overridden by command args.
  // Use data_path and append "/<exe-file-name>.menu"

  const char *fn = fl_filename_name(argv[0]);
  strcpy(menu, data_path);
  strcat(menu, "/");
  strcat(menu, fn);
  fl_filename_setext(menu, sizeof(menu), ".menu");

  // parse commandline

  int i = 0;
  Fl::args_to_utf8(argc, argv); // for MSYS2/MinGW
  if (!Fl::args(argc, argv, i) || i < argc-1)
    Fl::fatal("Usage: %s <switches> <menufile>\n%s", argv[0], Fl::help);
  if (i < argc) {
    // override menu file *and* data path !
    fl_filename_absolute(menu, sizeof(menu), (const char *)argv[i]);
    strcpy(data_path, menu);
    fix_path(data_path);
  }

  // set current work directory to 'data_path'

  if (fl_chdir(data_path) == -1) { /* ignore */ }

  // Create forms first
  //    tty needs to exist before we can print debug msgs
  //
  create_the_forms();

  {
    char cwd[1024];
    fl_getcwd(cwd, sizeof(cwd));
    fix_path(cwd, 0);

    debug_var("app_path",     app_path);
    debug_var("fluid_path",   fluid_path);
    debug_var("options_path", options_path);
    debug_var("data_path",    data_path);
    debug_var("menu file",    menu);
    debug_var("cwd",          cwd);
    tty->printf("\n");
  }

  if (!load_the_menu(menu))
    Fl::fatal("Can't open menu file '%s'", menu);

  push_menu("@main");
  form->show(argc, argv);

  // set size_range() after show() so the window can be resizable (Win + macOS)
  form->size_range(FORM_W, FORM_H, FORM_W, FORM_H);

#if (0) // DEBUG (remove after testing)
  {
    const char *const *scheme_names = Fl_Scheme_Choice::scheme_names();
    int ni = 0;
    while (scheme_names[ni]) {
      printf("scheme[%2d] = '%s'\n", ni, scheme_names[ni]);
      ni++;
    }
  }
#endif // End of debug and test statements

  Fl::run();
  return 0;
}
