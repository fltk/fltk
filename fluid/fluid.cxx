//
// "$Id: fluid.cxx,v 1.15.2.13.2.32 2002/09/02 04:33:10 spitzak Exp $"
//
// FLUID main entry for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2002 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Icon.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Input.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_message.H>
#include <FL/filename.H>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "../src/flstring.h"
#include "alignment_panel.h"

#if defined(WIN32) && !defined(__CYGWIN__)
#  include <direct.h>
#  include <windows.h>
#else
#  include <unistd.h>
#endif
#ifdef __EMX__
#  include <X11/Xlibint.h>
#endif

#include "about_panel.h"

#include "Fl_Type.h"

static Fl_Help_Dialog *help_dialog = 0;

Fl_Preferences	fluid_prefs(Fl_Preferences::USER, "fltk.org", "fluid");
int gridx = 5;
int gridy = 5;
int snap = 1;

// File history info...
char	absolute_history[10][1024];
char	relative_history[10][1024];

void	load_history();
void	update_history(const char *);

// Shell command support...
void	show_shell_window();

////////////////////////////////////////////////////////////////

void nyi(Fl_Widget *,void *) {
    fl_message("That's not yet implemented, sorry");
}

static const char *filename;
void set_filename(const char *c);
int modflag;

static char* pwd;
static char in_source_dir;
void goto_source_dir() {
  if (in_source_dir) return;
  if (!filename || !*filename) return;
  const char *p = fl_filename_name(filename);
  if (p <= filename) return; // it is in the current directory
  char buffer[1024];
  strlcpy(buffer, filename, sizeof(buffer));
  int n = p-filename; if (n>1) n--; buffer[n] = 0;
  if (!pwd) {
    pwd = getcwd(0,1024);
    if (!pwd) {fprintf(stderr,"getwd : %s\n",strerror(errno)); return;}
  }
  if (chdir(buffer)<0) {fprintf(stderr, "Can't chdir to %s : %s\n",
				buffer, strerror(errno)); return;}
  in_source_dir = 1;
}

void leave_source_dir() {
  if (!in_source_dir) return;
  if (chdir(pwd)<0) {fprintf(stderr, "Can't chdir to %s : %s\n",
			     pwd, strerror(errno));}
  in_source_dir = 0;
}
  
Fl_Window *main_window;

void save_cb(Fl_Widget *, void *v) {
  const char *c = filename;
  if (v || !c || !*c) {
    if (!(c=fl_file_chooser("Save to:", "FLUID Files (*.f[ld])", c))) return;
    set_filename(c);
  }
  if (!write_file(c)) {
    fl_message("Error writing %s: %s", c, strerror(errno));
    return;
  }
  modflag = 0;
}

void exit_cb(Fl_Widget *,void *) {
  if (modflag)
    switch (fl_choice("Save changes before exiting?", "Cancel", "No", "Yes"))
    {
      case 0 : /* Cancel */
          return;
      case 2 : /* Yes */
          save_cb(NULL, NULL);
	  if (modflag) return;	// Didn't save!
    }

  if (about_panel)
    delete about_panel;
  if (help_dialog)
    delete help_dialog;

  exit(0);
}

void open_cb(Fl_Widget *, void *v) {
  if (!v && modflag && !fl_ask("Discard changes?")) return;
  const char *c;
  const char *oldfilename;
  if (!(c = fl_file_chooser("Open:", "FLUID Files (*.f[ld])", filename))) return;
  oldfilename = filename;
  filename    = NULL;
  set_filename(c);
  if (!read_file(c, v!=0)) {
    fl_message("Can't read %s: %s", c, strerror(errno));
    free((void *)filename);
    filename = oldfilename;
    if (main_window) main_window->label(filename);
    return;
  }
  if (v) {
    // Inserting a file; restore the original filename...
    modflag = 1;
    free((void *)filename);
    filename = oldfilename;
    if (main_window) main_window->label(filename);
  } else {
    // Loaded a file; free the old filename...
    modflag = 0;
    if (oldfilename) free((void *)oldfilename);
  }
}

void open_history_cb(Fl_Widget *, void *v) {
  if (modflag && !fl_ask("Discard changes?")) return;
  const char *oldfilename = filename;
  filename = NULL;
  set_filename((char *)v);
  if (!read_file(filename, 0)) {
    fl_message("Can't read %s: %s", filename, strerror(errno));
    free((void *)filename);
    filename = oldfilename;
    if (main_window) main_window->label(filename);
    return;
  }
  modflag = 0;
  if (oldfilename) free((void *)oldfilename);
}

void new_cb(Fl_Widget *, void *v) {
  if (!v && modflag && !fl_ask("Discard changes?")) return;
  const char *c;
  if (!(c = fl_file_chooser("New:", "FLUID Files (*.f[ld])", 0))) return;
  delete_all();
  set_filename(c);
  modflag = 0;
}

int compile_only = 0;
int compile_strings = 0;
int header_file_set = 0;
int code_file_set = 0;
const char* header_file_name = ".h";
const char* code_file_name = ".cxx";
int i18n_type = 0;
const char* i18n_include = "";
const char* i18n_function = "";
const char* i18n_file = "";
const char* i18n_set = "";
char i18n_program[1024] = "";

void write_cb(Fl_Widget *, void *) {
  if (!filename) {
    save_cb(0,0);
    if (!filename) return;
  }
  char cname[1024];
  char hname[1024];
  strlcpy(i18n_program, fl_filename_name(filename), sizeof(i18n_program));
  fl_filename_setext(i18n_program, sizeof(i18n_program), "");
  if (*code_file_name == '.' && strchr(code_file_name, '/') == NULL) {
    strlcpy(cname, fl_filename_name(filename), sizeof(cname));
    fl_filename_setext(cname, sizeof(cname), code_file_name);
  } else {
    strlcpy(cname, code_file_name, sizeof(hname));
  }
  if (*header_file_name == '.' && strchr(header_file_name, '/') == NULL) {
    strlcpy(hname, fl_filename_name(filename), sizeof(hname));
    fl_filename_setext(hname, sizeof(hname), header_file_name);
  } else {
    strlcpy(hname, header_file_name, sizeof(hname));
  }
  if (!compile_only) goto_source_dir();
  int x = write_code(cname,hname);
  if (!compile_only) leave_source_dir();
  strlcat(cname, " and ", sizeof(cname));
  strlcat(cname, hname, sizeof(cname));
  if (compile_only) {
    if (!x) {fprintf(stderr,"%s : %s\n",cname,strerror(errno)); exit(1);}
  } else {
    if (!x) {
      fl_message("Can't write %s: %s", cname, strerror(errno));
    } else if (completion_button->value()) {
      fl_message("Wrote %s", cname, 0);
    }
  }
}

void write_strings_cb(Fl_Widget *, void *) {
  static const char *exts[] = { ".txt", ".po", ".msg" };
  if (!filename) {
    save_cb(0,0);
    if (!filename) return;
  }
  char sname[1024];
  strlcpy(sname, fl_filename_name(filename), sizeof(sname));
  fl_filename_setext(sname, sizeof(sname), exts[i18n_type]);
  if (!compile_only) goto_source_dir();
  int x = write_strings(sname);
  if (!compile_only) leave_source_dir();
  if (compile_only) {
    if (x) {fprintf(stderr,"%s : %s\n",sname,strerror(errno)); exit(1);}
  } else {
    if (x) {
      fl_message("Can't write %s: %s", sname, strerror(errno));
    } else if (completion_button->value()) {
      fl_message("Wrote %s", sname);
    }
  }
}

void openwidget_cb(Fl_Widget *, void *) {
  if (!Fl_Type::current) {
    fl_message("Please select a widget");
    return;
  }
  Fl_Type::current->open();
}

void toggle_overlays(Fl_Widget *,void *);

void select_all_cb(Fl_Widget *,void *);

void group_cb(Fl_Widget *, void *);

void ungroup_cb(Fl_Widget *, void *);

extern int pasteoffset;
static int ipasteoffset;

static char* cutfname() {
  static char name[1024];
  static char beenhere = 0;

  if (!beenhere) {
    beenhere = 1;
    fluid_prefs.getUserdataPath(name, sizeof(name));
    strlcat(name, "cut_buffer", sizeof(name));
    // getUserdataPath zeros the "name" buffer...
  }

  return name;
}

void copy_cb(Fl_Widget*, void*) {
  if (!Fl_Type::current) return;
  ipasteoffset = 10;
  if (!write_file(cutfname(),1)) {
    fl_message("Can't write %s: %s", cutfname(), strerror(errno));
    return;
  }
}

extern void select_only(Fl_Type *);
void cut_cb(Fl_Widget *, void *) {
  if (!Fl_Type::current) return;
  ipasteoffset = 0;
  Fl_Type *p = Fl_Type::current->parent;
  while (p && p->selected) p = p->parent;
  if (!write_file(cutfname(),1)) {
    fl_message("Can't write %s: %s", cutfname(), strerror(errno));
    return;
  }
  delete_all(1);
  if (p) select_only(p);
}

extern int force_parent;

void paste_cb(Fl_Widget*, void*) {
  if (ipasteoffset) force_parent = 1;
  pasteoffset = ipasteoffset;
  if (gridx>1) pasteoffset = ((pasteoffset-1)/gridx+1)*gridx;
  if (gridy>1) pasteoffset = ((pasteoffset-1)/gridy+1)*gridy;
  if (!read_file(cutfname(), 1)) {
    fl_message("Can't read %s: %s", cutfname(), strerror(errno));
  }
  pasteoffset = 0;
  ipasteoffset += 10;
  force_parent = 0;
}

void earlier_cb(Fl_Widget*,void*);

void later_cb(Fl_Widget*,void*);

Fl_Type *sort(Fl_Type *parent);

static void sort_cb(Fl_Widget *,void *) {
  sort((Fl_Type*)0);
}

void show_project_cb(Fl_Widget *, void *);
void show_grid_cb(Fl_Widget *, void *);
void show_settings_cb(Fl_Widget *, void *);

void align_widget_cb(Fl_Widget *, long);

void about_cb(Fl_Widget *, void *) {
  if (!about_panel) make_about_panel();
  display_group->show();
  about_panel->show();
}

void show_help(const char *name) {
  const char	*docdir;
  char		helpname[1024];
  
  if (!help_dialog) help_dialog = new Fl_Help_Dialog();

  if ((docdir = getenv("FLTK_DOCDIR")) == NULL) {
#ifdef __EMX__
    // Doesn't make sense to have a hardcoded fallback
    static char fltk_docdir[1024];

    strlcpy(fltk_docdir, __XOS2RedirRoot("/XFree86/lib/X11/fltk/doc"),
            sizeof(fltk_docdir));

    docdir = fltk_docdir;
#else
    docdir = FLTK_DOCDIR;
#endif // __EMX__
  }
  snprintf(helpname, sizeof(helpname), "%s/%s", docdir, name);  

  help_dialog->load(helpname);
  help_dialog->show();
}

void help_cb(Fl_Widget *, void *) {
  show_help("fluid.html");
}

void manual_cb(Fl_Widget *, void *) {
  show_help("index.html");
}

////////////////////////////////////////////////////////////////

extern Fl_Menu_Item New_Menu[];

Fl_Menu_Item Main_Menu[] = {
{"&File",0,0,0,FL_SUBMENU},
  {"&New", 0, new_cb, 0},
  {"&Open...", FL_CTRL+'o', open_cb, 0},
  {"Open &Previous",0,0,0,FL_SUBMENU},
    {relative_history[0], FL_CTRL+'0', open_history_cb, absolute_history[0]},
    {relative_history[1], FL_CTRL+'1', open_history_cb, absolute_history[1]},
    {relative_history[2], FL_CTRL+'2', open_history_cb, absolute_history[2]},
    {relative_history[3], FL_CTRL+'3', open_history_cb, absolute_history[3]},
    {relative_history[4], FL_CTRL+'4', open_history_cb, absolute_history[4]},
    {relative_history[5], FL_CTRL+'5', open_history_cb, absolute_history[5]},
    {relative_history[6], FL_CTRL+'6', open_history_cb, absolute_history[6]},
    {relative_history[7], FL_CTRL+'7', open_history_cb, absolute_history[7]},
    {relative_history[8], FL_CTRL+'8', open_history_cb, absolute_history[8]},
    {relative_history[9], FL_CTRL+'9', open_history_cb, absolute_history[9]},
    {0},
  {"&Insert...", FL_CTRL+'i', open_cb, (void*)1, FL_MENU_DIVIDER},
  {"&Save", FL_CTRL+'s', save_cb, 0},
  {"Save &As...", FL_CTRL+FL_SHIFT+'s', save_cb, (void*)1, FL_MENU_DIVIDER},
  {"Write &code", FL_CTRL+FL_SHIFT+'c', write_cb, 0},
  {"&Write strings", FL_CTRL+FL_SHIFT+'w', write_strings_cb, 0, FL_MENU_DIVIDER},
  {"&Quit", FL_CTRL+'q', exit_cb},
  {0},
{"&Edit",0,0,0,FL_SUBMENU},
  {"&Undo", FL_CTRL+'z', nyi},
  {"C&ut", FL_CTRL+'x', cut_cb},
  {"&Copy", FL_CTRL+'c', copy_cb},
  {"&Paste", FL_CTRL+'v', paste_cb},
  {"Select &All", FL_CTRL+'a', select_all_cb, 0, FL_MENU_DIVIDER},
  {"&Open...", FL_F+1, openwidget_cb},
  {"&Sort",0,sort_cb},
  {"&Earlier", FL_F+2, earlier_cb},
  {"&Later", FL_F+3, later_cb},
//{"Show", FL_F+5, show_cb},
//{"Hide", FL_F+6, hide_cb},
  {"&Group", FL_F+7, group_cb},
  {"U&ngroup", FL_F+8, ungroup_cb,0, FL_MENU_DIVIDER},
//{"Deactivate", 0, nyi},
//{"Activate", 0, nyi, 0, FL_MENU_DIVIDER},
  {"O&verlays on/off",FL_CTRL+FL_SHIFT+'o',toggle_overlays},
  {"Pro&ject Settings...",FL_CTRL+'p',show_project_cb},
  {"&GUI Settings...",FL_CTRL+FL_SHIFT+'p',show_settings_cb},
  {0},
{"&New", 0, 0, (void *)New_Menu, FL_SUBMENU_POINTER},
{"&Layout",0,0,0,FL_SUBMENU},
  {"&Align",0,0,0,FL_SUBMENU},
    {"&Left",0,(Fl_Callback *)align_widget_cb,(void*)10},
    {"&Center",0,(Fl_Callback *)align_widget_cb,(void*)11},
    {"&Right",0,(Fl_Callback *)align_widget_cb,(void*)12},
    {"&Top",0,(Fl_Callback *)align_widget_cb,(void*)13},
    {"&Middle",0,(Fl_Callback *)align_widget_cb,(void*)14},
    {"&Bottom",0,(Fl_Callback *)align_widget_cb,(void*)15},
    {0},
  {"&Space Evenly",0,0,0,FL_SUBMENU},
    {"&Across",0,(Fl_Callback *)align_widget_cb,(void*)20},
    {"&Down",0,(Fl_Callback *)align_widget_cb,(void*)21},
    {0},
  {"&Make Same Size",0,0,0,FL_SUBMENU},
    {"&Width",0,(Fl_Callback *)align_widget_cb,(void*)30},
    {"&Height",0,(Fl_Callback *)align_widget_cb,(void*)31},
    {"&Both",0,(Fl_Callback *)align_widget_cb,(void*)32},
    {0},
  {"&Center In Group",0,0,0,FL_SUBMENU|FL_MENU_DIVIDER},
    {"&Horizontal",0,(Fl_Callback *)align_widget_cb,(void*)40},
    {"&Vertical",0,(Fl_Callback *)align_widget_cb,(void*)41},
    {0},
  {"&Grid...",FL_CTRL+'g',show_grid_cb},
  {0},
{"&Shell",0,0,0,FL_SUBMENU},
  {"Execute &Command...",FL_ALT+'x',(Fl_Callback *)show_shell_window},
  {"Execute &Again",FL_ALT+'g',(Fl_Callback *)do_shell_command},
  {0},
{"&Help",0,0,0,FL_SUBMENU},
  {"&About FLUID...",0,about_cb},
  {"&On FLUID...",0,help_cb},
  {"&Manual...",0,manual_cb},
  {0},
{0}};

#define BROWSERWIDTH 300
#define BROWSERHEIGHT 500
#define WINWIDTH 300
#define MENUHEIGHT 25
#define WINHEIGHT (BROWSERHEIGHT+MENUHEIGHT)

extern void fill_in_New_Menu();

void make_main_window() {
  int i;

  fluid_prefs.get("snap", i, 1);
  snap = i;

  fluid_prefs.get("gridx", i, 5);
  gridx = i;

  fluid_prefs.get("gridy", i, 5);
  gridy = i;

  load_history();

  make_grid_window();
  make_settings_window();
  make_shell_window();

  if (!main_window) {
    Fl_Widget *o;
    main_window = new Fl_Double_Window(WINWIDTH,WINHEIGHT,"fluid");
    main_window->box(FL_NO_BOX);
    o = make_widget_browser(0,MENUHEIGHT,BROWSERWIDTH,BROWSERHEIGHT);
    o->box(FL_FLAT_BOX);
    o->tooltip("Double-click to view or change an item.");
    main_window->resizable(o);
    Fl_Menu_Bar *m = new Fl_Menu_Bar(0,0,BROWSERWIDTH,MENUHEIGHT);
    m->menu(Main_Menu);
    m->global();
    fill_in_New_Menu();
    main_window->end();
  }
}

// Load file history from preferences...
void load_history() {
  int	i;		// Looping var

  for (i = 0; i < 10; i ++) {
    fluid_prefs.get( Fl_Preferences::Name("file%d", i), absolute_history[i], "", sizeof(absolute_history[i]));
    if (absolute_history[i][0]) {
      // Make a relative version of the filename for the menu...
      fl_filename_relative(relative_history[i], sizeof(relative_history[i]),
                           absolute_history[i]);

      Main_Menu[i + 4].flags = 0;
    } else Main_Menu[i + 4].flags = FL_MENU_INVISIBLE;
  }

  if (!absolute_history[0][0]) Main_Menu[3].flags |= FL_MENU_INACTIVE;
}

// Update file history from preferences...
void update_history(const char *flname) {
  int	i;		// Looping var
  char	absolute[1024];

  fl_filename_absolute(absolute, sizeof(absolute), flname);

  for (i = 0; i < 10; i ++)
#if defined(WIN32) || defined(__APPLE__)
    if (!strcasecmp(absolute, absolute_history[i])) break;
#else
    if (!strcmp(absolute, absolute_history[i])) break;
#endif // WIN32 || __APPLE__

  if (i == 0) return;

  if (i >= 10) i = 9;

  // Move the other flnames down in the list...
  memmove(absolute_history + 1, absolute_history,
          i * sizeof(absolute_history[0]));
  memmove(relative_history + 1, relative_history,
          i * sizeof(relative_history[0]));

  // Put the new file at the top...
  strlcpy(absolute_history[0], absolute, sizeof(absolute_history[0]));

  fl_filename_relative(relative_history[0], sizeof(relative_history[0]),
                       absolute_history[0]);

  // Update the menu items as needed...
  for (i = 0; i < 10; i ++) {
    fluid_prefs.set( Fl_Preferences::Name("file%d", i), absolute_history[i]);
    if (absolute_history[i][0]) Main_Menu[i + 4].flags = 0;
    else Main_Menu[i + 4].flags = FL_MENU_INVISIBLE;
  }

  Main_Menu[3].flags &= ~FL_MENU_INACTIVE;
}

// Shell command support...
#if !defined(WIN32) || defined(__CYGWIN__)
// Support the full piped shell command...
static FILE *shell_pipe;

void
shell_pipe_cb(int, void*) {
  char	line[1024];		// Line from command output...

  if (fgets(line, sizeof(line), shell_pipe) != NULL) {
    // Add the line to the output list...
    shell_run_list->add(line);
    shell_run_list->make_visible(shell_run_list->size());
  } else {
    // End of file; tell the parent...
    Fl::remove_fd(fileno(shell_pipe));

    pclose(shell_pipe);
    shell_pipe = NULL;
    shell_run_list->add("... END SHELL COMMAND ...");
  }
}

void
do_shell_command(Fl_Return_Button*, void*) {
  const char	*command;	// Command to run


  shell_window->hide();

  if ((command = shell_command_input->value()) == NULL || !*command) {
    fl_alert("No shell command entered!");
    return;
  }

  if (shell_savefl_button->value()) {
    save_cb(0, 0);
  }

  if (shell_writecode_button->value()) {
    compile_only = 1;
    write_cb(0, 0);
    compile_only = 0;
  }

  if (shell_writemsgs_button->value()) {
    compile_only = 1;
    write_strings_cb(0, 0);
    compile_only = 0;
  }

  // Show the output window and clear things...
  shell_run_list->clear();
  shell_run_list->add(command);
  shell_run_window->label("Shell Command Running...");

  if ((shell_pipe = popen(command, "r")) == NULL) {
    fl_alert("Unable to run shell command: %s", strerror(errno));
    return;
  }

  shell_run_button->deactivate();
  shell_run_window->hotspot(shell_run_list);
  shell_run_window->show();

  Fl::add_fd(fileno(shell_pipe), shell_pipe_cb);

  while (shell_pipe) Fl::wait();

  shell_run_button->activate();
  shell_run_window->label("Shell Command Complete");
  fl_beep();

  while (shell_run_window->shown()) Fl::wait();
}
#else
// Just do basic shell command stuff, no status window...
void
do_shell_command(Fl_Return_Button*, void*) {
  const char	*command;	// Command to run
  int		status;		// Status from command...


  shell_window->hide();

  if ((command = shell_command_input->value()) == NULL || !*command) {
    fl_alert("No shell command entered!");
    return;
  }

  if (shell_savefl_button->value()) {
    save_cb(0, 0);
  }

  if (shell_writecode_button->value()) {
    compile_only = 1;
    write_cb(0, 0);
    compile_only = 0;
  }

  if (shell_writemsgs_button->value()) {
    compile_only = 1;
    write_strings_cb(0, 0);
    compile_only = 0;
  }

  if ((status = system(command)) != 0) {
    fl_alert("Shell command returned status %d!", status);
  } else if (completion_button->value()) {
    fl_message("Shell command completed successfully!");
  }
}
#endif // !WIN32 || __CYGWIN__


void
show_shell_window() {
  shell_window->hotspot(shell_command_input);
  shell_window->show();
}

void set_filename(const char *c) {
  if (filename) free((void *)filename);
  filename = strdup(c);
  if (main_window) main_window->label(filename);

  update_history(filename);
}

////////////////////////////////////////////////////////////////

static int arg(int argc, char** argv, int& i) {
  if (argv[i][1] == 'c' && !argv[i][2]) {compile_only = 1; i++; return 1;}
  if (argv[i][1] == 'c' && argv[i][2] == 's' && !argv[i][3]) {compile_only = 1; compile_strings = 1; i++; return 1;}
  if (argv[i][1] == 'o' && !argv[i][2] && i+1 < argc) {
    code_file_name = argv[i+1];
    code_file_set  = 1;
    i += 2;
    return 2;
  }
  if (argv[i][1] == 'h' && !argv[i][2]) {
    header_file_name = argv[i+1];
    header_file_set  = 1;
    i += 2;
    return 2;
  }
  return 0;
}

#if ! (defined(WIN32) && !defined (__CYGWIN__))

#include <signal.h>
#ifdef _sigargs
#define SIGARG _sigargs
#else
#ifdef __sigargs
#define SIGARG __sigargs
#else
#define SIGARG int // you may need to fix this for older systems
#endif
#endif

extern "C" {
static void sigint(SIGARG) {
  signal(SIGINT,sigint);
  exit_cb(0,0);
}
}
#endif

int main(int argc,char **argv) {
  int i = 1;
  if (!Fl::args(argc,argv,i,arg) || i < argc-1) {
    fprintf(stderr,"usage: %s <switches> name.fl\n"
" -c : write .cxx and .h and exit\n"
" -cs : write .cxx and .h and strings and exit\n"
" -o <name> : .cxx output filename, or extension if <name> starts with '.'\n"
" -h <name> : .h output filename, or extension if <name> starts with '.'\n"
"%s\n", argv[0], Fl::help);
    return 1;
  }
  const char *c = argv[i];

  fl_register_images();

  make_main_window();
  fl_register_images();
  if (c) set_filename(c);
  if (!compile_only) {
    Fl::visual((Fl_Mode)(FL_DOUBLE|FL_INDEX));
    Fl_File_Icon::load_system_icons();
    main_window->callback(exit_cb);
    main_window->show(argc,argv);
    if (!c && openlast_button->value() && absolute_history[0][0]) {
      // Open previous file when no file specified...
      open_history_cb(0, absolute_history[0]);
    }
  }
  if (c && !read_file(c,0)) {
    if (compile_only) {
      fprintf(stderr,"%s : %s\n", c, strerror(errno));
      exit(1);
    }
    fl_message("Can't read %s: %s", c, strerror(errno));
  }
  if (compile_only) {
    if (compile_strings) write_strings_cb(0,0);
    write_cb(0,0);
    exit(0);
  }
  modflag = 0;
#ifndef WIN32
  signal(SIGINT,sigint);
#endif

  grid_cb(horizontal_input, 0); // Makes sure that windows get snap params...

  return Fl::run();
}

//
// End of "$Id: fluid.cxx,v 1.15.2.13.2.32 2002/09/02 04:33:10 spitzak Exp $".
//
