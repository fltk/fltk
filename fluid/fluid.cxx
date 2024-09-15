//
// FLUID main entry for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

#include "fluid.h"

#include "Fl_Type.h"
#include "Fl_Function_Type.h"
#include "Fl_Group_Type.h"
#include "Fl_Window_Type.h"
#include "widget_browser.h"
#include "shell_command.h"
#include "factory.h"
#include "pixmaps.h"
#include "undo.h"
#include "file.h"
#include "code.h"
#include "mergeback.h"

#include "settings_panel.h"
#include "function_panel.h"
#include "codeview_panel.h"
#include "template_panel.h"
#include "about_panel.h"
#include "autodoc.h"

#include <FL/Fl.H>
#ifdef __APPLE__
#include <FL/platform.H> // for fl_open_callback
#endif
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Printer.H>
#include <FL/fl_string_functions.h>
#include <locale.h>     // setlocale()..
#include "../src/flstring.h"

extern "C"
{
#if defined(HAVE_LIBPNG) && defined(HAVE_LIBZ)
#  include <zlib.h>
#  ifdef HAVE_PNG_H
#    include <png.h>
#  else
#    include <libpng/png.h>
#  endif // HAVE_PNG_H
#endif // HAVE_LIBPNG && HAVE_LIBZ
}

/// \defgroup globals Fluid Global Variables, Functions and Callbacks
/// \{

//
// Globals..
//

/// FLUID-wide help dialog.
static Fl_Help_Dialog *help_dialog = NULL;

/// Main app window menu bar.
Fl_Menu_Bar *main_menubar = NULL;

/// Main app window.
Fl_Window *main_window;

/// Fluid application preferences, always accessible, will be flushed when app closes.
Fl_Preferences fluid_prefs(Fl_Preferences::USER_L, "fltk.org", "fluid");

/// Show guides in the design window when positioning widgets, saved in app preferences.
int show_guides = 1;

/// Show areas of restricted use in overlay plane.
/// Restricted areas are widget that overlap each other, widgets that are outside
/// of their parent's bounds (except children of Scroll groups), and areas
/// within an Fl_Tile that are not covered by children.
int show_restricted = 1;

/// Show a ghosted outline for groups that have very little contrast.
/// This makes groups with NO_BOX or FLAT_BOX better editable.
int show_ghosted_outline = 1;

/// Show widget comments in the browser, saved in app preferences.
int show_comments = 1;

/// Use external editor for editing Fl_Code_Type, saved in app preferences.
int G_use_external_editor = 0;

/// Debugging help for external Fl_Code_Type editor.
int G_debug = 0;

/// Run this command to load an Fl_Code_Type into an external editor, save in app preferences.
char G_external_editor_command[512];


/// This is set to create different labels when creating new widgets.
/// \todo Details unclear.
int reading_file = 0;


// File history info...

/// Stores the absolute filename of the last 10 design files, saved in app preferences.
char absolute_history[10][FL_PATH_MAX];

/// This list of filenames is computed from \c absolute_history and displayed in the main menu.
char relative_history[10][FL_PATH_MAX];

/// Menuitem to save a .fl design file, will be deactivated if the design is unchanged.
Fl_Menu_Item *save_item = NULL;

/// First Menuitem that shows the .fl design file history.
Fl_Menu_Item *history_item = NULL;

/// Menuitem to show or hide the widget bin, label will change if bin is visible.
Fl_Menu_Item *widgetbin_item = NULL;

/// Menuitem to show or hide the code view, label will change if view is visible.
Fl_Menu_Item *codeview_item = NULL;

/// Menuitem to show or hide the editing overlay, label will change if overlay visibility changes.
Fl_Menu_Item *overlay_item = NULL;

/// Menuitem to show or hide the editing guides, label will change if overlay visibility changes.
Fl_Menu_Item *guides_item = NULL;

/// Menuitem to show or hide the restricted area overlys, label will change if overlay visibility changes.
Fl_Menu_Item *restricted_item = NULL;

////////////////////////////////////////////////////////////////

/// Filename of the current .fl project file
static const char *filename = NULL;

/// Set if the current design has been modified compared to the associated .fl design file.
int modflag = 0;

/// Set if the code files are older than the current design.
int modflag_c = 0;

/// Application work directory, stored here when temporarily changing to the source code directory.
/// \see goto_source_dir()
static Fl_String app_work_dir;

/// Used as a counter to set the .fl project dir as the current directory.
/// \see enter_project_dir(), leave_project_dir()
static char in_project_dir = 0;

/// Set, if Fluid was started with the command line argument -u
int update_file = 0;            // fluid -u

/// Set, if Fluid was started with the command line argument -c
int compile_file = 0;           // fluid -c

/// Set, if Fluid was started with the command line argument -cs
int compile_strings = 0;        // fluid -cs

/// Set, if Fluid runs in batch mode, and no user interface is activated.
int batch_mode = 0;             // if set (-c, -u) don't open display

/// command line arguments that overrides the generate code file extension or name
Fl_String g_code_filename_arg;

/// command line arguments that overrides the generate header file extension or name
Fl_String g_header_filename_arg;

/// current directory path at application launch
Fl_String g_launch_path;

/// if set, generate images for automatic documentation in this directory
Fl_String g_autodoc_path;

/// path to store temporary files during app run
/// \see tmpdir_create_called
Fl_String tmpdir_path;

/// true if the temporary file path was already created
/// \see tmpdir_path
bool tmpdir_create_called = false;


/// Offset in pixels when adding widgets from an .fl file.
int pasteoffset = 0;

/// Paste offset incrementing at every paste command.
static int ipasteoffset = 0;

// ---- project settings

/// The current project, possibly a new, empty roject
Fluid_Project g_project;

/**
 Initialize a new project.
 */
Fluid_Project::Fluid_Project() :
  i18n_type(FD_I18N_NONE),
  include_H_from_C(1),
  use_FL_COMMAND(0),
  utf8_in_src(0),
  avoid_early_includes(0),
  header_file_set(0),
  code_file_set(0),
  write_mergeback_data(0),
  header_file_name(".h"),
  code_file_name(".cxx")
{ }

/**
 Clear all project resources.
 Not implemented.
 */
Fluid_Project::~Fluid_Project() {
}

/**
 Reset all project setting to create a new empty project.
 */
void Fluid_Project::reset() {
  ::delete_all();
  i18n_type = FD_I18N_NONE;

  i18n_gnu_include = "<libintl.h>";
  i18n_gnu_conditional = "";
  i18n_gnu_function = "gettext";
  i18n_gnu_static_function = "gettext_noop";

  i18n_pos_include = "<nl_types.h>";
  i18n_pos_conditional = "";
  i18n_pos_file = "";
  i18n_pos_set = "1";

  include_H_from_C = 1;
  use_FL_COMMAND = 0;
  utf8_in_src = 0;
  avoid_early_includes = 0;
  header_file_set = 0;
  code_file_set = 0;
  header_file_name = ".h";
  code_file_name = ".cxx";
  write_mergeback_data = 0;
}

/**
 Tell the project and i18n tab of the settings dialog to refresh themselves.
 */
void Fluid_Project::update_settings_dialog() {
  if (settings_window) {
    w_settings_project_tab->do_callback(w_settings_project_tab, LOAD);
    w_settings_i18n_tab->do_callback(w_settings_i18n_tab, LOAD);
  }
}

/**
 Make sure that a path name ends with a forward slash.
 \param[in] str directory or path name
 \return a new string, ending with a '/'
 */
static Fl_String end_with_slash(const Fl_String &str) {
  char last = str[str.size()-1];
  if (last !='/' && last != '\\')
    return str + "/";
  else
    return str;
}

/**
 Generate a path to a directory for temporary data storage.
 The path is stored in g_tmpdir.
 */
static void create_tmpdir() {
  if (tmpdir_create_called)
    return;
  tmpdir_create_called = true;

  char buf[128];
#if _WIN32
  // The usual temp file locations on Windows are
  //    %system%\Windows\Temp
  //    %userprofiles%\AppData\Local
  // usually resolving into
  //    C:/Windows/Temp/
  //    C:\Users\<username>\AppData\Local\Temp
  fl_snprintf(buf, sizeof(buf)-1, "fluid-%d/", (long)GetCurrentProcessId());
  Fl_String name = buf;
  wchar_t tempdirW[FL_PATH_MAX+1];
  char tempdir[FL_PATH_MAX+1];
  unsigned len = GetTempPathW(FL_PATH_MAX, tempdirW);
  if (len == 0) {
    strcpy(tempdir, "c:/windows/temp/");
  } else {
    unsigned wn = fl_utf8fromwc(tempdir, FL_PATH_MAX, tempdirW, len);
    tempdir[wn] = 0;
  }
  Fl_String path = tempdir;
  end_with_slash(path);
  path += name;
  fl_make_path(path.c_str());
  if (fl_access(path.c_str(), 6) == 0) tmpdir_path = path;
#else
  fl_snprintf(buf, sizeof(buf)-1, "fluid-%d/", getpid());
  Fl_String name = buf;
  Fl_String path = fl_getenv("TMPDIR");
  if (!path.empty()) {
    end_with_slash(path);
    path += name;
    fl_make_path(path.c_str());
    if (fl_access(path.c_str(), 6) == 0) tmpdir_path = path;
  }
  if (tmpdir_path.empty()) {
    path = Fl_String("/tmp/") + name;
    fl_make_path(path.c_str());
    if (fl_access(path.c_str(), 6) == 0) tmpdir_path = path;
  }
#endif
  if (tmpdir_path.empty()) {
    char pbuf[FL_PATH_MAX+1];
    fluid_prefs.get_userdata_path(pbuf, FL_PATH_MAX);
    path = Fl_String(pbuf);
    end_with_slash(path);
    path += name;
    fl_make_path(path.c_str());
    if (fl_access(path.c_str(), 6) == 0) tmpdir_path = path;
  }
  if (tmpdir_path.empty()) {
    if (batch_mode) {
      fprintf(stderr, "ERROR: Can't create directory for temporary data storage.\n");
    } else {
      fl_alert("Can't create directory for temporary data storage.");
    }
  }
}

/**
 Delete the temporary directory that was created in set_tmpdir.
 */
static void delete_tmpdir() {
  // was a temporary directory created
  if (!tmpdir_create_called)
    return;
  if (tmpdir_path.empty())
    return;

  // first delete all files that may still be left in the temp directory
  struct dirent **de;
  int n_de = fl_filename_list(tmpdir_path.c_str(), &de);
  if (n_de >= 0) {
    for (int i=0; i<n_de; i++) {
      Fl_String path = tmpdir_path + de[i]->d_name;
      fl_unlink(path.c_str());
    }
    fl_filename_free_list(&de, n_de);
  }

  // then delete the directory itself
  if (fl_rmdir(tmpdir_path.c_str()) < 0) {
    if (batch_mode) {
      fprintf(stderr, "WARNING: Can't delete tmpdir '%s': %s", tmpdir_path.c_str(), strerror(errno));
    } else {
      fl_alert("WARNING: Can't delete tmpdir '%s': %s", tmpdir_path.c_str(), strerror(errno));
    }
  }
}

/**
 Return the path to a temporary directory for this instance of FLUID.
 Fluid will do its best to clear and delete this directory when exiting.
 \return the path to the temporary directory, ending in a '/', or and empty
      string if no directory could be created.
 */
const Fl_String &get_tmpdir() {
  if (!tmpdir_create_called)
    create_tmpdir();
  return tmpdir_path;
}

/**
 Give the user the opportunity to save a project before clearing it.

 If the project has unsaved changes, this function pops up a dialog, that
 allows the user to save the project, continue without saving the project,
 or to cancel the operation.

 If the user chooses to save, and no filename was set, a file dialog allows
 the user to pick a name and location, or to cancel the operation.

 \return false if the user aborted the operation and the calling function
 should abort as well
 */
bool confirm_project_clear() {
  if (modflag == 0) return true;
  switch (fl_choice("This project has unsaved changes. Do you want to save\n"
                    "the project file before proceeding?",
                    "Cancel", "Save", "Don't Save"))
  {
    case 0 : /* Cancel */
      return false;
    case 1 : /* Save */
      save_cb(NULL, NULL);
      if (modflag) return false;  // user canceled the "Save As" dialog
  }
  return true;
}

// ----

extern Fl_Window *the_panel;

/**
 Ensure that text widgets in the widget panel propagates apply current changes.
 By temporarily clearing the text focus, all text widgets with changed text
 will unfocus and call their respective callbacks, propagating those changes to
 their data set.
 */
void flush_text_widgets() {
  if (Fl::focus() && (Fl::focus()->top_window() == the_panel)) {
    Fl_Widget *old_focus = Fl::focus();
    Fl::focus(NULL); // trigger callback of the widget that is losing focus
    Fl::focus(old_focus);
  }
}

// ----

/**
 Change the current working directory to the .fl project directory.

 Every call to enter_project_dir() must have a corresponding leave_project_dir()
 call. Enter and leave calls can be nested.

 The first call to enter_project_dir() remembers the original directory, usually
 the launch directory of the application. Nested calls will increment a nesting
 counter. When the nesting counter is back to 0, leave_project_dir() will return
 to the original directory.

 The global variable 'filename' must be set to the current project file with
 absolute or relative path information.

 \see leave_project_dir(), pwd, in_project_dir
 */
void enter_project_dir() {
  if (in_project_dir<0) {
    fprintf(stderr, "** Fluid internal error: enter_project_dir() calls unmatched\n");
    return;
  }
  in_project_dir++;
  // check if we are already in the project dir and do nothing if so
  if (in_project_dir>1) return;
  // check if there is an active project, and do nothing if there is none
  if (!filename || !*filename) {
    fprintf(stderr, "** Fluid internal error: enter_project_dir() no filename set\n");
    return;
  }
  // store the current working directory for later
  app_work_dir = fl_getcwd();
  // set the current directory to the path of our .fl file
  Fl_String project_path = fl_filename_path(fl_filename_absolute(filename));
  if (fl_chdir(project_path.c_str()) == -1) {
    fprintf(stderr, "** Fluid internal error: enter_project_dir() can't chdir to %s: %s\n",
            project_path.c_str(), strerror(errno));
    return;
  }
  //fprintf(stderr, "chdir from %s to %s\n", app_work_dir.c_str(), fl_getcwd().c_str());
}

/**
 Change the current working directory to the previous directory.
 \see enter_project_dir(), pwd, in_project_dir
 */
void leave_project_dir() {
  if (in_project_dir == 0) {
    fprintf(stderr, "** Fluid internal error: leave_project_dir() calls unmatched\n");
    return;
  }
  in_project_dir--;
  // still nested, stay in the project directory
  if (in_project_dir > 0) return;
  // no longer nested, return to the original, usually the application working directory
  if (fl_chdir(app_work_dir.c_str()) < 0) {
    fprintf(stderr, "** Fluid internal error: leave_project_dir() can't chdir back to %s : %s\n",
            app_work_dir.c_str(), strerror(errno));
  }
}

/**
 Position the given window window based on entries in the app preferences.
 Customisable by user; feature can be switched off.
 The window is not shown or hidden by this function, but a value is returned
 to indicate the state to the caller.
 \param[in] w position this window
 \param[in] prefsName name of the preferences item that stores the window settings
 \param[in] Visible default value if window is hidden or shown
 \param[in] X, Y, W, H default size and position if nothing is specified in the preferences
 \return 1 if the caller should make the window visible, 0 if hidden.
 */
char position_window(Fl_Window *w, const char *prefsName, int Visible, int X, int Y, int W, int H) {
  Fl_Preferences pos(fluid_prefs, prefsName);
  if (prevpos_button->value()) {
    pos.get("x", X, X);
    pos.get("y", Y, Y);
    if ( W!=0 ) {
      pos.get("w", W, W);
      pos.get("h", H, H);
      w->resize( X, Y, W, H );
    }
    else
      w->position( X, Y );
  }
  pos.get("visible", Visible, Visible);
  return Visible;
}

/**
 Save the position and visibility state of a window to the app preferences.
 \param[in] w save this window data
 \param[in] prefsName name of the preferences item that stores the window settings
 */
void save_position(Fl_Window *w, const char *prefsName) {
  Fl_Preferences pos(fluid_prefs, prefsName);
  pos.set("x", w->x());
  pos.set("y", w->y());
  pos.set("w", w->w());
  pos.set("h", w->h());
  pos.set("visible", (int)(w->shown() && w->visible()));
}

/**
 Return the path and filename of a temporary file for cut or duplicated data.
 \param[in] which 0 gets the cut/copy/paste buffer, 1 gets the duplication buffer
 \return a pointer to a string in a static buffer
 */
static char* cutfname(int which = 0) {
  static char name[2][FL_PATH_MAX];
  static char beenhere = 0;

  if (!beenhere) {
    beenhere = 1;
    fluid_prefs.getUserdataPath(name[0], sizeof(name[0]));
    strlcat(name[0], "cut_buffer", sizeof(name[0]));
    fluid_prefs.getUserdataPath(name[1], sizeof(name[1]));
    strlcat(name[1], "dup_buffer", sizeof(name[1]));
  }

  return name[which];
}

/**
 Timer to watch for external editor modifications.

 If one or more external editors open, check if their files were modified.
 If so: reload to ram, update size/mtime records, and change fluid's
 'modified' state.
 */
static void external_editor_timer(void*) {
  int editors_open = ExternalCodeEditor::editors_open();
  if ( G_debug ) printf("--- TIMER --- External editors open=%d\n", editors_open);
  if ( editors_open > 0 ) {
    // Walk tree looking for files modified by external editors.
    int modified = 0;
    for (Fl_Type *p = Fl_Type::first; p; p = p->next) {
      if ( p->is_a(ID_Code) ) {
        Fl_Code_Type *code = (Fl_Code_Type*)p;
        // Code changed by external editor?
        if ( code->handle_editor_changes() ) {  // updates ram, file size/mtime
          modified++;
        }
        if ( code->is_editing() ) {             // editor open?
          code->reap_editor();                  // Try to reap; maybe it recently closed
        }
      }
    }
    if ( modified ) set_modflag(1);
  }
  // Repeat timeout if editors still open
  //    The ExternalCodeEditor class handles start/stopping timer, we just
  //    repeat_timeout() if it's already on. NOTE: above code may have reaped
  //    only open editor, which would disable further timeouts. So *recheck*
  //    if editors still open, to ensure we don't accidentally re-enable them.
  //
  if ( ExternalCodeEditor::editors_open() ) {
    Fl::repeat_timeout(2.0, external_editor_timer);
  }
}

/**
 Save the current design to the file given by \c filename.
 If automatic, this overwrites an existing file. If interactive, if will
 verify with the user.
 \param[in] v if v is not NULL, or no filename is set, open a filechooser.
 */
void save_cb(Fl_Widget *, void *v) {
  flush_text_widgets();
  Fl_Native_File_Chooser fnfc;
  const char *c = filename;
  if (v || !c || !*c) {
    fnfc.title("Save To:");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fnfc.filter("FLUID Files\t*.f[ld]");
    if (fnfc.show() != 0) return;
    c = fnfc.filename();
    if (!fl_access(c, 0)) {
      Fl_String basename = fl_filename_name(Fl_String(c));
      if (fl_choice("The file \"%s\" already exists.\n"
                    "Do you want to replace it?", "Cancel",
                    "Replace", NULL, basename.c_str()) == 0) return;
    }

    if (v != (void *)2) set_filename(c);
  }
  if (!write_file(c)) {
    fl_alert("Error writing %s: %s", c, strerror(errno));
    return;
  }

  if (v != (void *)2) {
    set_modflag(0, 1);
    undo_save = undo_current;
  }
}

/**
 Save a design template.
 \todo We should document the concept of templates.
 */
void save_template_cb(Fl_Widget *, void *) {
  // Setup the template panel...
  if (!template_panel) make_template_panel();

  template_clear();
  template_browser->add("New Template");
  template_load();

  template_name->show();
  template_name->value("");

  template_instance->hide();

  template_delete->show();
  template_delete->deactivate();

  template_submit->label("Save");
  template_submit->deactivate();

  template_panel->label("Save Template");

  // Show the panel and wait for the user to do something...
  template_panel->show();
  while (template_panel->shown()) Fl::wait();

  // Get the template name, return if it is empty...
  const char *c = template_name->value();
  if (!c || !*c) return;

  // Convert template name to filename_with_underscores
  char savename[FL_PATH_MAX], *saveptr;
  strlcpy(savename, c, sizeof(savename));
  for (saveptr = savename; *saveptr; saveptr ++) {
    if (isspace(*saveptr)) *saveptr = '_';
  }

  // Find the templates directory...
  char filename[FL_PATH_MAX];
  fluid_prefs.getUserdataPath(filename, sizeof(filename));

  strlcat(filename, "templates", sizeof(filename));
  if (fl_access(filename, 0)) fl_make_path(filename);

  strlcat(filename, "/", sizeof(filename));
  strlcat(filename, savename, sizeof(filename));

  char *ext = filename + strlen(filename);
  if (ext >= (filename + sizeof(filename) - 5)) {
    fl_alert("The template name \"%s\" is too long!", c);
    return;
  }

  // Save the .fl file...
  strcpy(ext, ".fl");

  if (!fl_access(filename, 0)) {
    if (fl_choice("The template \"%s\" already exists.\n"
                  "Do you want to replace it?", "Cancel",
                  "Replace", NULL, c) == 0) return;
  }

  if (!write_file(filename)) {
    fl_alert("Error writing %s: %s", filename, strerror(errno));
    return;
  }

#if defined(HAVE_LIBPNG) && defined(HAVE_LIBZ)
  // Get the screenshot, if any...
  Fl_Type *t;

  for (t = Fl_Type::first; t; t = t->next) {
    // Find the first window...
    if (t->is_a(ID_Window)) break;
  }

  if (!t) return;

  // Grab a screenshot...
  Fl_Window_Type *wt = (Fl_Window_Type *)t;
  uchar *pixels;
  int w, h;

  if ((pixels = wt->read_image(w, h)) == NULL) return;

  // Save to a PNG file...
  strcpy(ext, ".png");

  errno = 0;
  if (fl_write_png(filename, pixels, w, h, 3) != 0) {
    delete[] pixels;
    fl_alert("Error writing %s: %s", filename, strerror(errno));
    return;
  }

#  if 0 // The original PPM output code...
  strcpy(ext, ".ppm");
  fp = fl_fopen(filename, "wb");
  fprintf(fp, "P6\n%d %d 255\n", w, h);
  fwrite(pixels, w * h, 3, fp);
  fclose(fp);
#  endif // 0

  delete[] pixels;
#endif // HAVE_LIBPNG && HAVE_LIBZ
}

/**
 Reload the file set by \c filename, replacing the current design.
 If the design was modified, a dialog will ask for confirmation.
 */
void revert_cb(Fl_Widget *,void *) {
  if (modflag) {
    if (!fl_choice("This user interface has been changed. Really revert?",
                   "Cancel", "Revert", NULL)) return;
  }
  undo_suspend();
  if (!read_file(filename, 0)) {
    undo_resume();
    widget_browser->rebuild();
    g_project.update_settings_dialog();
    fl_message("Can't read %s: %s", filename, strerror(errno));
    return;
  }
  widget_browser->rebuild();
  undo_resume();
  set_modflag(0, 0);
  undo_clear();
  g_project.update_settings_dialog();
}

/**
 Exit Fluid; we hope you had a nice experience.
 If the design was modified, a dialog will ask for confirmation.
 */
void exit_cb(Fl_Widget *,void *) {
  if (shell_command_running()) {
    int choice = fl_choice("Previous shell command still running!",
                           "Cancel",
                           "Exit",
                           NULL);
    if (choice == 0) { // user chose to cancel the exit operation
      return;
    }
  }

  flush_text_widgets();

  // verify user intention
  if (confirm_project_clear() == false)
    return;

  // Stop any external editor update timers
  ExternalCodeEditor::stop_update_timer();

  save_position(main_window,"main_window_pos");

  if (widgetbin_panel) {
    save_position(widgetbin_panel,"widgetbin_pos");
    delete widgetbin_panel;
  }
  if (codeview_panel) {
    Fl_Preferences svp(fluid_prefs, "codeview");
    svp.set("autorefresh", cv_autorefresh->value());
    svp.set("autoposition", cv_autoposition->value());
    svp.set("tab", cv_tab->find(cv_tab->value()));
    svp.set("code_choice", cv_code_choice);
    save_position(codeview_panel,"codeview_pos");
    delete codeview_panel;
    codeview_panel = 0;
  }
  if (shell_run_window) {
    save_position(shell_run_window,"shell_run_Window_pos");
  }

  if (about_panel)
    delete about_panel;
  if (help_dialog)
    delete help_dialog;

  if (g_shell_config)
    g_shell_config->write(fluid_prefs, FD_STORE_USER);
  g_layout_list.write(fluid_prefs, FD_STORE_USER);

  undo_clear();

  // Destroy tree
  //    Doing so causes dtors to automatically close all external editors
  //    and cleans up editor tmp files. Then remove fluid tmpdir /last/.
  g_project.reset();
  ExternalCodeEditor::tmpdir_clear();
  delete_tmpdir();

  exit(0);
}

/**
 Clear the current project and create a new, empty one.

 If the current project was modified, FLUID will give the user the opportunity
 to save the old project first.

 \param[in] user_must_confirm if set, a confimation dialog is presented to the
    user before resetting the project. Default is `true`.
 \return false if the operation was canceled
 */
bool new_project(bool user_must_confirm) {
  // verify user intention
  if ((user_must_confirm) &&  (confirm_project_clear() == false))
    return false;

  // clear the current project
  g_project.reset();
  set_filename(NULL);
  set_modflag(0, 0);
  widget_browser->rebuild();
  g_project.update_settings_dialog();

  // all is clear to continue
  return true;
}

/**
 Open the template browser and load a new file from templates.

 If the current project was modified, FLUID will give the user the opportunity
 to save the old project first.

 \return false if the operation was canceled or failed otherwise
 */
bool new_project_from_template() {
  // clear the current project first
  if (new_project() == false)
    return false;

  // Setup the template panel...
  if (!template_panel) make_template_panel();

  template_clear();
  template_browser->add("Blank");
  template_load();

  template_name->hide();
  template_name->value("");

  template_instance->show();
  template_instance->deactivate();
  template_instance->value("");

  template_delete->show();

  template_submit->label("New");
  template_submit->deactivate();

  template_panel->label("New");

  //if ( template_browser->size() == 1 ) { // only one item?
  template_browser->value(1);          // select it
  template_browser->do_callback();
  //}

  // Show the panel and wait for the user to do something...
  template_panel->show();
  while (template_panel->shown()) Fl::wait();

  // See if the user chose anything...
  int item = template_browser->value();
  if (item < 1) return false;

  // Load the template, if any...
  const char *tname = (const char *)template_browser->data(item);

  if (tname) {
    // Grab the instance name...
    const char *iname = template_instance->value();

    if (iname && *iname) {
      // Copy the template to a temp file, then read it in...
      char line[1024], *ptr, *next;
      FILE *infile, *outfile;

      if ((infile = fl_fopen(tname, "rb")) == NULL) {
        fl_alert("Error reading template file \"%s\":\n%s", tname,
                 strerror(errno));
        set_modflag(0);
        undo_clear();
        return false;
      }

      if ((outfile = fl_fopen(cutfname(1), "wb")) == NULL) {
        fl_alert("Error writing buffer file \"%s\":\n%s", cutfname(1),
                 strerror(errno));
        fclose(infile);
        set_modflag(0);
        undo_clear();
        return false;
      }

      while (fgets(line, sizeof(line), infile)) {
        // Replace @INSTANCE@ with the instance name...
        for (ptr = line; (next = strstr(ptr, "@INSTANCE@")) != NULL; ptr = next + 10) {
          fwrite(ptr, next - ptr, 1, outfile);
          fputs(iname, outfile);
        }

        fputs(ptr, outfile);
      }

      fclose(infile);
      fclose(outfile);

      undo_suspend();
      read_file(cutfname(1), 0);
      fl_unlink(cutfname(1));
      undo_resume();
    } else {
      // No instance name, so read the template without replacements...
      undo_suspend();
      read_file(tname, 0);
      undo_resume();
    }
  }

  widget_browser->rebuild();
  g_project.update_settings_dialog();
  set_modflag(0);
  undo_clear();

  return true;
}

/**
 Open a native file chooser to allow choosing a project file for reading.

 Path and filename are preset with the current project filename, if there
 is one.

 \param title a text describing the action after selecting a file (load, merge, ...)
 \return the file path and name, or an empty string if the operation was canceled
 */
Fl_String open_project_filechooser(const Fl_String &title) {
  Fl_Native_File_Chooser dialog;
  dialog.title(title.c_str());
  dialog.type(Fl_Native_File_Chooser::BROWSE_FILE);
  dialog.filter("FLUID Files\t*.f[ld]\n");
  if (filename) {
    Fl_String current_project_file = filename;
    dialog.directory(fl_filename_path(current_project_file).c_str());
    dialog.preset_file(fl_filename_name(current_project_file).c_str());
  }
  if (dialog.show() != 0)
    return Fl_String();
  return Fl_String(dialog.filename());
}

/**
 Load a project from the give file name and path.

 The project file is inserted at the currently selected type.

 If no filename is given, FLUID will open a file chooser dialog.

 \param[in] filename_arg path and name of the new project file
 \return false if the operation failed
 */
bool merge_project_file(const Fl_String &filename_arg) {
  bool is_a_merge = (Fl_Type::first != NULL);
  Fl_String title = is_a_merge ? "Merge Project File" : "Open Project File";

  // ask for a filename if none was given
  Fl_String new_filename = filename_arg;
  if (new_filename.empty()) {
    new_filename = open_project_filechooser(title);
    if (new_filename.empty()) {
      return false;
    }
  }

  const char *c = new_filename.c_str();
  const char *oldfilename = filename;
  filename    = NULL;
  set_filename(c);
  if (is_a_merge) undo_checkpoint();
  undo_suspend();
  if (!read_file(c, is_a_merge)) {
    undo_resume();
    widget_browser->rebuild();
    g_project.update_settings_dialog();
    fl_message("Can't read %s: %s", c, strerror(errno));
    free((void *)filename);
    filename = oldfilename;
    if (main_window) set_modflag(modflag);
    return false;
  }
  undo_resume();
  widget_browser->rebuild();
  if (is_a_merge) {
    // Inserting a file; restore the original filename...
    set_filename(oldfilename);
    set_modflag(1);
  } else {
    // Loaded a file; free the old filename...
    set_modflag(0, 0);
    undo_clear();
  }
  if (oldfilename) free((void *)oldfilename);
  return true;
}

/**
 Open a file chooser and load an exiting project file.

 If the current project was modified, FLUID will give the user the opportunity
 to save the old project first.

 If no filename is given, FLUID will open a file chooser dialog.

 \param[in] filename_arg load from this file, or show file chooser if empty
 \return false if the operation was canceled or failed otherwise
 */
bool open_project_file(const Fl_String &filename_arg) {
  // verify user intention
  if (confirm_project_clear() == false)
    return false;

  // ask for a filename if none was given
  Fl_String new_filename = filename_arg;
  if (new_filename.empty()) {
    new_filename = open_project_filechooser("Open Project File");
    if (new_filename.empty()) {
      return false;
    }
  }

  // clear the project and merge a file by the given name
  new_project(false);
  return merge_project_file(new_filename);
}

#ifdef __APPLE__
/**
 Handle app launch with an associated filename (macOS only).
 Should there be a modified design already, Fluid asks for user confirmation.
 \param[in] c the filename of the new design
 */
void apple_open_cb(const char *c) {
  open_project_file(Fl_String(c));
}
#endif // __APPLE__

/**
 Get the absolute path of the project file, for example `/Users/matt/dev/`.
 \return the path ending in '/'
 */
Fl_String Fluid_Project::projectfile_path() const {
  return end_with_slash(fl_filename_absolute(fl_filename_path(filename), g_launch_path));
}

/**
 Get the project file name including extension, for example `test.fl`.
 \return the file name without path
 */
Fl_String Fluid_Project::projectfile_name() const {
  return fl_filename_name(filename);
}

/**
 Get the absolute path of the generated C++ code file, for example `/Users/matt/dev/src/`.
 \return the path ending in '/'
 */
Fl_String Fluid_Project::codefile_path() const {
  Fl_String path = fl_filename_path(code_file_name);
  if (batch_mode)
    return end_with_slash(fl_filename_absolute(path, g_launch_path));
  else
    return end_with_slash(fl_filename_absolute(path, projectfile_path()));
}

/**
 Get the generated C++ code file name including extension, for example `test.cxx`.
 \return the file name without path
 */
Fl_String Fluid_Project::codefile_name() const {
  Fl_String name = fl_filename_name(code_file_name);
  if (name.empty()) {
    return fl_filename_setext(fl_filename_name(filename), ".cxx");
  } else if (name[0] == '.') {
    return fl_filename_setext(fl_filename_name(filename), code_file_name);
  } else {
    return name;
  }
}

/**
 Get the absolute path of the generated C++ header file, for example `/Users/matt/dev/src/`.
 \return the path ending in '/'
 */
Fl_String Fluid_Project::headerfile_path() const {
  Fl_String path = fl_filename_path(header_file_name);
  if (batch_mode)
    return end_with_slash(fl_filename_absolute(path, g_launch_path));
  else
    return end_with_slash(fl_filename_absolute(path, projectfile_path()));
}

/**
 Get the generated C++ header file name including extension, for example `test.cxx`.
 \return the file name without path
 */
Fl_String Fluid_Project::headerfile_name() const {
  Fl_String name = fl_filename_name(header_file_name);
  if (name.empty()) {
    return fl_filename_setext(fl_filename_name(filename), ".h");
  } else if (name[0] == '.') {
    return fl_filename_setext(fl_filename_name(filename), header_file_name);
  } else {
    return name;
  }
}

/**
 Get the absolute path of the generated i18n strings file, for example `/Users/matt/dev/`.
 Although it may be more useful to put the text file into the same directory
 with the source and header file, historically, the text is always saved with
 the project file in interactive mode, and in the FLUID launch directory in
 batch mode.
 \return the path ending in '/'
 */
Fl_String Fluid_Project::stringsfile_path() const {
  if (batch_mode)
    return g_launch_path;
  else
    return projectfile_path();
}

/**
 Get the generated i18n text file name including extension, for example `test.po`.
 \return the file name without path
 */
Fl_String Fluid_Project::stringsfile_name() const {
  switch (i18n_type) {
    default: return fl_filename_setext(fl_filename_name(filename), ".txt");
    case FD_I18N_GNU: return fl_filename_setext(fl_filename_name(filename), ".po");
    case FD_I18N_POSIX: return fl_filename_setext(fl_filename_name(filename), ".msg");
  }
}

/**
 Get the name of the project file without the filename extension.
 \return the file name without path or extension
 */
Fl_String Fluid_Project::basename() const {
  return fl_filename_setext(fl_filename_name(filename), "");
}

/**
 Generate the C++ source and header filenames and write those files.

 This function creates the source filename by setting the file
 extension to \c code_file_name and a header filename
 with the extension \c code_file_name which are both
 settable by the user.

 If the code filename has not been set yet, a "save file as" dialog will be
 presented to the user.

 In batch_mode, the function will either be silent, or, if opening or writing
 the files fails, write an error message to \c stderr and exit with exit code 1.

 In interactive mode, it will pop up an error message, or, if the user
 hasn't disabled that, pop up a confirmation message.

 \param[in] dont_show_completion_dialog don't show the completion dialog
 \return 1 if the operation failed, 0 if it succeeded
 */
int write_code_files(bool dont_show_completion_dialog)
{
  // -- handle user interface issues
  flush_text_widgets();
  if (!filename) {
    save_cb(0,0);
    if (!filename) return 1;
  }

  // -- generate the file names with absolute paths
  Fd_Code_Writer f;
  Fl_String code_filename = g_project.codefile_path() + g_project.codefile_name();
  Fl_String header_filename = g_project.headerfile_path() + g_project.headerfile_name();

  // -- write the code and header files
  if (!batch_mode) enter_project_dir();
  int x = f.write_code(code_filename.c_str(), header_filename.c_str());
  Fl_String code_filename_rel = fl_filename_relative(code_filename);
  Fl_String header_filename_rel = fl_filename_relative(header_filename);
  if (!batch_mode) leave_project_dir();

  // -- print error message in batch mode or pop up an error or confirmation dialog box
  if (batch_mode) {
    if (!x) {
      fprintf(stderr, "%s and %s: %s\n",
              code_filename_rel.c_str(),
              header_filename_rel.c_str(),
              strerror(errno));
      exit(1);
    }
  } else {
    if (!x) {
      fl_message("Can't write %s or %s: %s",
                 code_filename_rel.c_str(),
                 header_filename_rel.c_str(),
                 strerror(errno));
    } else {
      set_modflag(-1, 0);
      if (dont_show_completion_dialog==false && completion_button->value()) {
        fl_message("Wrote %s and %s",
                   code_filename_rel.c_str(),
                   header_filename_rel.c_str());
      }
    }
  }
  return 0;
}

/**
 Callback to write C++ code and header files.
 */
void write_cb(Fl_Widget *, void *) {
    write_code_files();
}

#if 0
// Matt: disabled
/**
 Merge the possibly modified content of code files back into the project.
 */
int mergeback_code_files()
{
  flush_text_widgets();
  if (!filename) return 1;
  if (!g_project.write_mergeback_data) {
    fl_message("MergeBack is not enabled for this project.\n"
               "Please enable MergeBack in the project settings\n"
               "dialog and re-save the project file and the code.");
    return 0;
  }

  Fl_String proj_filename = g_project.projectfile_path() + g_project.projectfile_name();
  Fl_String code_filename;
#if 1
  if (!batch_mode) {
    Fl_Preferences build_records(Fl_Preferences::USER_L, "fltk.org", "fluid-build");
    Fl_Preferences path(build_records, proj_filename.c_str());
    int i, n = proj_filename.size();
    for (i=0; i<n; i++) if (proj_filename[i]=='\\') proj_filename[i] = '/';
    preferences_get(path, "code", code_filename, "");
  }
#endif
  if (code_filename.empty())
    code_filename = g_project.codefile_path() + g_project.codefile_name();
  if (!batch_mode) enter_project_dir();
  int c = merge_back(code_filename, proj_filename, FD_MERGEBACK_INTERACTIVE);
  if (!batch_mode) leave_project_dir();

  if (c==0) fl_message("Comparing\n  \"%s\"\nto\n  \"%s\"\n\n"
                       "MergeBack found no external modifications\n"
                       "in the source code.",
                       code_filename.c_str(), proj_filename.c_str());
  if (c==-2) fl_message("No corresponding source code file found.");
  return c;
}

void mergeback_cb(Fl_Widget *, void *) {
  mergeback_code_files();
}
#endif

/**
 Write the strings that are used in i18n.
 */
void write_strings_cb(Fl_Widget *, void *) {
  flush_text_widgets();
  if (!filename) {
    save_cb(0,0);
    if (!filename) return;
  }
  Fl_String filename = g_project.stringsfile_path() + g_project.stringsfile_name();
  int x = write_strings(filename);
  if (batch_mode) {
    if (x) {
      fprintf(stderr, "%s : %s\n", filename.c_str(), strerror(errno));
      exit(1);
    }
  } else {
    if (x) {
      fl_message("Can't write %s: %s", filename.c_str(), strerror(errno));
    } else if (completion_button->value()) {
      fl_message("Wrote %s", g_project.stringsfile_name().c_str());
    }
  }
}

/**
 Show the editor for the \c current Fl_Type.
 */
void openwidget_cb(Fl_Widget *, void *) {
  if (!Fl_Type::current) {
    fl_message("Please select a widget");
    return;
  }
  Fl_Type::current->open();
}

/**
 User chose to copy the currently selected widgets.
 */
void copy_cb(Fl_Widget*, void*) {
  flush_text_widgets();
  if (!Fl_Type::current) {
    fl_beep();
    return;
  }
  flush_text_widgets();
  ipasteoffset = 10;
  if (!write_file(cutfname(),1)) {
    fl_message("Can't write %s: %s", cutfname(), strerror(errno));
    return;
  }
}

/**
 User chose to cut the currently selected widgets.
 */
void cut_cb(Fl_Widget *, void *) {
  if (!Fl_Type::current) {
    fl_beep();
    return;
  }
  flush_text_widgets();
  if (!write_file(cutfname(),1)) {
    fl_message("Can't write %s: %s", cutfname(), strerror(errno));
    return;
  }
  undo_checkpoint();
  set_modflag(1);
  ipasteoffset = 0;
  Fl_Type *p = Fl_Type::current->parent;
  while (p && p->selected) p = p->parent;
  delete_all(1);
  if (p) select_only(p);
  widget_browser->rebuild();
}

/**
 User chose to delete the currently selected widgets.
 */
void delete_cb(Fl_Widget *, void *) {
  if (!Fl_Type::current) {
    fl_beep();
    return;
  }
  undo_checkpoint();
  set_modflag(1);
  ipasteoffset = 0;
  Fl_Type *p = Fl_Type::current->parent;
  while (p && p->selected) p = p->parent;
  delete_all(1);
  if (p) select_only(p);
  widget_browser->rebuild();
}

/**
 User chose to paste the widgets from the cut buffer.

 This function will paste the widgets in the cut buffer after the currently
 selected widget. If the currently selected widget is a group widget and
 it is not folded, the new widgets will be added inside the group.
 */
void paste_cb(Fl_Widget*, void*) {
  pasteoffset = ipasteoffset;
  undo_checkpoint();
  undo_suspend();
  Strategy strategy = kAddAfterCurrent;
  if (Fl_Type::current && Fl_Type::current->can_have_children()) {
    if (Fl_Type::current->folded_ == 0) {
      // If the current widget is a group widget and it is not folded,
      // add the new widgets inside the group.
      strategy = kAddAsLastChild;
      // The following alternative also works quite nicely
      //strategy = kAddAsFirstChild;
    }
  }
  if (!read_file(cutfname(), 1, strategy)) {
    widget_browser->rebuild();
    fl_message("Can't read %s: %s", cutfname(), strerror(errno));
  }
  undo_resume();
  widget_browser->display(Fl_Type::current);
  widget_browser->rebuild();
  pasteoffset = 0;
  ipasteoffset += 10;
}

/**
 Duplicate the selected widgets.

 This code is a bit complex because it needs to find the last selected
 widget with the lowest level, so that the new widgets are inserted after
 this one.
 */
void duplicate_cb(Fl_Widget*, void*) {
  if (!Fl_Type::current) {
    fl_beep();
    return;
  }

  // flush the text widgets to make sure the user's changes are saved:
  flush_text_widgets();

  // find the last selected node with the lowest level:
  int lowest_level = 9999;
  Fl_Type *new_insert = NULL;
  if (Fl_Type::current->selected) {
    for (Fl_Type *t = Fl_Type::first; t; t = t->next) {
      if (t->selected && (t->level <= lowest_level)) {
        lowest_level = t->level;
        new_insert = t;
      }
    }
  }
  if (new_insert)
    Fl_Type::current = new_insert;

  // write the selected widgets to a file:
  if (!write_file(cutfname(1),1)) {
    fl_message("Can't write %s: %s", cutfname(1), strerror(errno));
    return;
  }

  // read the file and add the widgets after the current one:
  pasteoffset  = 0;
  undo_checkpoint();
  undo_suspend();
  if (!read_file(cutfname(1), 1, kAddAfterCurrent)) {
    fl_message("Can't read %s: %s", cutfname(1), strerror(errno));
  }
  fl_unlink(cutfname(1));
  widget_browser->display(Fl_Type::current);
  widget_browser->rebuild();
  undo_resume();
}

/**
 User wants to sort selected widgets by y coordinate.
 */
static void sort_cb(Fl_Widget *,void *) {
  undo_checkpoint();
  sort((Fl_Type*)NULL);
  widget_browser->rebuild();
  set_modflag(1);
}

/**
 Open the "About" dialog.
 */
void about_cb(Fl_Widget *, void *) {
  if (!about_panel) make_about_panel();
  about_panel->show();
}

/**
 Open a dialog to show the HTML help page form the FLTK documentation folder.
 \param[in] name name of the HTML help file.
 */
void show_help(const char *name) {
  const char    *docdir;
  char          helpname[FL_PATH_MAX];

  if (!help_dialog) help_dialog = new Fl_Help_Dialog();

  if ((docdir = fl_getenv("FLTK_DOCDIR")) == NULL) {
    docdir = FLTK_DOCDIR;
  }
  snprintf(helpname, sizeof(helpname), "%s/%s", docdir, name);

  // make sure that we can read the file
  FILE *f = fopen(helpname, "rb");
  if (f) {
    fclose(f);
    help_dialog->load(helpname);
  } else {
    // if we can not read the file, we display the canned version instead
    // or ask the native browser to open the page on www.fltk.org
    if (strcmp(name, "fluid.html")==0) {
      if (!Fl_Shared_Image::find("embedded:/fluid_flow_chart_800.png"))
        new Fl_PNG_Image("embedded:/fluid_flow_chart_800.png", fluid_flow_chart_800_png, sizeof(fluid_flow_chart_800_png));
      help_dialog->value
      (
       "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
       "<html><head><title>FLTK: Programming with FLUID</title></head><body>\n"
       "<h2>What is FLUID?</h2>\n"
       "The Fast Light User Interface Designer, or FLUID, is a graphical editor "
       "that is used to produce FLTK source code. FLUID edits and saves its state "
       "in <code>.fl</code> files. These files are text, and you can (with care) "
       "edit them in a text editor, perhaps to get some special effects.<p>\n"
       "FLUID can \"compile\" the <code>.fl</code> file into a <code>.cxx</code> "
       "and a <code>.h</code> file. The <code>.cxx</code> file defines all the "
       "objects from the <code>.fl</code> file and the <code>.h</code> file "
       "declares all the global ones. FLUID also supports localization "
       "(Internationalization) of label strings using message files and the GNU "
       "gettext or POSIX catgets interfaces.<p>\n"
       "A simple program can be made by putting all your code (including a <code>"
       "main()</code> function) into the <code>.fl</code> file and thus making the "
       "<code>.cxx</code> file a single source file to compile. Most programs are "
       "more complex than this, so you write other <code>.cxx</code> files that "
       "call the FLUID functions. These <code>.cxx</code> files must <code>"
       "#include</code> the <code>.h</code> file or they can <code>#include</code> "
       "the <code>.cxx</code> file so it still appears to be a single source file.<p>"
       "<img src=\"embedded:/fluid_flow_chart_800.png\"></p>"
       "<p>More information is available online at <a href="
       "\"https://www.fltk.org/doc-1.4/fluid.html\">https://www.fltk.org/</a>"
       "</body></html>"
       );
    } else if (strcmp(name, "license.html")==0) {
      fl_open_uri("https://www.fltk.org/doc-1.4/license.html");
      return;
    } else if (strcmp(name, "index.html")==0) {
      fl_open_uri("https://www.fltk.org/doc-1.4/index.html");
      return;
    } else {
      snprintf(helpname, sizeof(helpname), "https://www.fltk.org/%s", name);
      fl_open_uri(helpname);
      return;
    }
  }
  help_dialog->show();
}

/**
 User wants help on Fluid.
 */
void help_cb(Fl_Widget *, void *) {
  show_help("fluid.html");
}

/**
 User wants to see the Fluid manual.
 */
void manual_cb(Fl_Widget *, void *) {
  show_help("index.html");
}

// ---- Printing

/**
 Open the dialog to allow the user to print the current window.
 */
void print_menu_cb(Fl_Widget *, void *) {
  int w, h, ww, hh;
  int frompage, topage;
  Fl_Type       *t;                     // Current widget
  int           num_windows;            // Number of windows
  Fl_Window_Type *windows[1000];        // Windows to print
  int           winpage;                // Current window page
  Fl_Window *win;

  for (t = Fl_Type::first, num_windows = 0; t; t = t->next) {
    if (t->is_a(ID_Window)) {
      windows[num_windows] = (Fl_Window_Type *)t;
      if (!((Fl_Window*)(windows[num_windows]->o))->shown()) continue;
      num_windows ++;
    }
  }

  Fl_Printer printjob;
  if ( printjob.start_job(num_windows, &frompage, &topage) ) return;
  int pagecount = 0;
  for (winpage = 0; winpage < num_windows; winpage++) {
    float scale = 1, scale_x = 1, scale_y = 1;
    if (winpage+1 < frompage || winpage+1 > topage) continue;
    printjob.start_page();
    printjob.printable_rect(&w, &h);

    // Get the time and date...
    time_t curtime = time(NULL);
    struct tm *curdate = localtime(&curtime);
    char date[1024];
    strftime(date, sizeof(date), "%c", curdate);
    fl_font(FL_HELVETICA, 12);
    fl_color(FL_BLACK);
    fl_draw(date, (w - (int)fl_width(date))/2, fl_height());
    sprintf(date, "%d/%d", ++pagecount, topage-frompage+1);
    fl_draw(date, w - (int)fl_width(date), fl_height());

    // Get the base filename...
    Fl_String basename = fl_filename_name(Fl_String(filename));
    fl_draw(basename.c_str(), 0, fl_height());

    // print centered and scaled to fit in the page
    win = (Fl_Window*)windows[winpage]->o;
    ww = win->decorated_w();
    if(ww > w) scale_x = float(w)/ww;
    hh = win->decorated_h();
    if(hh > h) scale_y = float(h)/hh;
    if (scale_x < scale) scale = scale_x;
    if (scale_y < scale) scale = scale_y;
    if (scale < 1) {
      printjob.scale(scale);
      printjob.printable_rect(&w, &h);
    }
    printjob.origin(w/2, h/2);
    printjob.print_window(win, -ww/2, -hh/2);
    printjob.end_page();
  }
  printjob.end_job();
}

// ---- Main menu bar

extern void select_layout_preset_cb(Fl_Widget *, void *user_data);
extern void layout_suite_marker(Fl_Widget *, void *user_data);

static void menu_file_new_cb(Fl_Widget *, void *) { new_project(); }
static void menu_file_new_from_template_cb(Fl_Widget *, void *) { new_project_from_template(); }
static void menu_file_open_cb(Fl_Widget *, void *) { open_project_file(""); }
static void menu_file_insert_cb(Fl_Widget *, void *) { merge_project_file(""); }
static void menu_file_open_history_cb(Fl_Widget *, void *v) { open_project_file(Fl_String((const char*)v)); }
static void menu_layout_sync_resize_cb(Fl_Menu_ *m, void*) {
  if (m->mvalue()->value()) Fl_Type::allow_layout = 1; else Fl_Type::allow_layout = 0; }
/**
 This is the main Fluid menu.

 Design history is manipulated right inside this menu structure.
 Some menu items change or deactivate correctly, but most items just trigger
 various callbacks.

 \c New_Menu creates new widgets and is explained in detail in another location.

 \see New_Menu
 \todo This menu needs some major modernization. Menus are too long and their
    sorting is not always obvious.
 \todo Shortcuts are all over the place (Alt, Ctrl, Command, Shift-Ctrl,
    function keys), and there should be a help page listing all shortcuts.
 */
Fl_Menu_Item Main_Menu[] = {
{"&File",0,0,0,FL_SUBMENU},
  {"&New", FL_COMMAND+'n', menu_file_new_cb},
  {"&Open...", FL_COMMAND+'o', menu_file_open_cb},
  {"&Insert...", FL_COMMAND+'i', menu_file_insert_cb, 0, FL_MENU_DIVIDER},
  {"&Save", FL_COMMAND+'s', save_cb, 0},
  {"Save &As...", FL_COMMAND+FL_SHIFT+'s', save_cb, (void*)1},
  {"Sa&ve A Copy...", 0, save_cb, (void*)2},
  {"&Revert...", 0, revert_cb, 0, FL_MENU_DIVIDER},
  {"New &From Template...", FL_COMMAND+'N', menu_file_new_from_template_cb, 0},
  {"Save As &Template...", 0, save_template_cb, 0, FL_MENU_DIVIDER},
  {"&Print...", FL_COMMAND+'p', print_menu_cb},
  {"Write &Code", FL_COMMAND+FL_SHIFT+'c', write_cb, 0},
// Matt: disabled {"MergeBack Code", FL_COMMAND+FL_SHIFT+'m', mergeback_cb, 0},
  {"&Write Strings", FL_COMMAND+FL_SHIFT+'w', write_strings_cb, 0, FL_MENU_DIVIDER},
  {relative_history[0], FL_COMMAND+'1', menu_file_open_history_cb, absolute_history[0]},
  {relative_history[1], FL_COMMAND+'2', menu_file_open_history_cb, absolute_history[1]},
  {relative_history[2], FL_COMMAND+'3', menu_file_open_history_cb, absolute_history[2]},
  {relative_history[3], FL_COMMAND+'4', menu_file_open_history_cb, absolute_history[3]},
  {relative_history[4], FL_COMMAND+'5', menu_file_open_history_cb, absolute_history[4]},
  {relative_history[5], FL_COMMAND+'6', menu_file_open_history_cb, absolute_history[5]},
  {relative_history[6], FL_COMMAND+'7', menu_file_open_history_cb, absolute_history[6]},
  {relative_history[7], FL_COMMAND+'8', menu_file_open_history_cb, absolute_history[7]},
  {relative_history[8], FL_COMMAND+'9', menu_file_open_history_cb, absolute_history[8]},
  {relative_history[9], 0, menu_file_open_history_cb, absolute_history[9], FL_MENU_DIVIDER},
  {"&Quit", FL_COMMAND+'q', exit_cb},
  {0},
{"&Edit",0,0,0,FL_SUBMENU},
  {"&Undo", FL_COMMAND+'z', undo_cb},
  {"&Redo", FL_COMMAND+FL_SHIFT+'z', redo_cb, 0, FL_MENU_DIVIDER},
  {"C&ut", FL_COMMAND+'x', cut_cb},
  {"&Copy", FL_COMMAND+'c', copy_cb},
  {"&Paste", FL_COMMAND+'v', paste_cb},
  {"Dup&licate", FL_COMMAND+'u', duplicate_cb},
  {"&Delete", FL_Delete, delete_cb, 0, FL_MENU_DIVIDER},
  {"Select &All", FL_COMMAND+'a', select_all_cb},
  {"Select &None", FL_COMMAND+FL_SHIFT+'a', select_none_cb, 0, FL_MENU_DIVIDER},
  {"Pr&operties...", FL_F+1, openwidget_cb},
  {"&Sort",0,sort_cb},
  {"&Earlier", FL_F+2, earlier_cb},
  {"&Later", FL_F+3, later_cb},
  {"&Group", FL_F+7, group_cb},
  {"Ung&roup", FL_F+8, ungroup_cb,0, FL_MENU_DIVIDER},
  {"Hide O&verlays",FL_COMMAND+FL_SHIFT+'o',toggle_overlays},
  {"Hide Guides",FL_COMMAND+FL_SHIFT+'g',toggle_guides},
  {"Hide Restricted",FL_COMMAND+FL_SHIFT+'r',toggle_restricted},
  {"Show Widget &Bin...",FL_ALT+'b',toggle_widgetbin_cb},
  {"Show Code View",FL_ALT+'c', (Fl_Callback*)toggle_codeview_cb, 0, FL_MENU_DIVIDER},
  {"Settings...",FL_ALT+'p',show_settings_cb},
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
  {"&Center In Group",0,0,0,FL_SUBMENU},
    {"&Horizontal",0,(Fl_Callback *)align_widget_cb,(void*)40},
    {"&Vertical",0,(Fl_Callback *)align_widget_cb,(void*)41},
    {0},
  {"Synchronized Resize", 0, (Fl_Callback*)menu_layout_sync_resize_cb, NULL, FL_MENU_TOGGLE|FL_MENU_DIVIDER },
  {"&Grid and Size Settings...",FL_COMMAND+'g',show_grid_cb, NULL, FL_MENU_DIVIDER},
  {"Presets", 0, layout_suite_marker, (void*)main_layout_submenu_, FL_SUBMENU_POINTER },
  {"Application", 0, select_layout_preset_cb, (void*)0, FL_MENU_RADIO|FL_MENU_VALUE },
  {"Dialog",      0, select_layout_preset_cb, (void*)1, FL_MENU_RADIO },
  {"Toolbox",     0, select_layout_preset_cb, (void*)2, FL_MENU_RADIO },
  {0},
{"&Shell", 0, Fd_Shell_Command_List::menu_marker, (void*)Fd_Shell_Command_List::default_menu, FL_SUBMENU_POINTER},
{"&Help",0,0,0,FL_SUBMENU},
  {"&Rapid development with FLUID...",0,help_cb},
  {"&FLTK Programmers Manual...",0,manual_cb, 0, FL_MENU_DIVIDER},
  {"&About FLUID...",0,about_cb},
  {0},
{0}};

/**
 Change the app's and hence preview the design's scheme.

 The scheme setting is stored in the app preferences
 - in key \p 'scheme_name' since 1.4.0
 - in key \p 'scheme' (index: 0 - 4) in 1.3.x

 This callback is triggered by changing the scheme in the
 Fl_Scheme_Choice widget (\p Edit/GUI Settings).

 \param[in] choice the calling widget

 \see init_scheme() for choice values and backwards compatibility
 */
void scheme_cb(Fl_Scheme_Choice *choice, void *) {
  if (batch_mode)
    return;

  // set the new scheme only if the scheme was changed
  const char *new_scheme = choice->text(choice->value());

  if (Fl::is_scheme(new_scheme))
    return;

  Fl::scheme(new_scheme);
  fluid_prefs.set("scheme_name", new_scheme);

  // Backwards compatibility: store 1.3 scheme index (1-4).
  // We assume that index 0-3 (base, plastic, gtk+, gleam) are in the
  // same order as in 1.3.x (index 1-4), higher values are ignored

  int scheme_index = scheme_choice->value();
  if (scheme_index <= 3)                          // max. index for 1.3.x (Gleam)
    fluid_prefs.set("scheme", scheme_index + 1);  // compensate for different indexing
}

/**
  Read Fluid's scheme preferences and set the app's scheme.

  Since FLTK 1.4.0 the scheme \b name is stored as a character string
  with key "scheme_name" in the preference database.

  In FLTK 1.3.x the scheme preference was stored as an integer index
  with key "scheme" in the database. The known schemes were hardcoded in
  Fluid's sources (here for reference):

    | Index | 1.3 Scheme Name | Choice | 1.4 Scheme Name |
    |-------|-----------------|-------|-----------------|
    | 0 | Default (same as None) | n/a | n/a |
    | 1 | None (same as Default) | 0 | base |
    | 2 | Plastic | 1 | plastic |
    | 3 | GTK+ | 2 | gtk+ |
    | 4 | Gleam | 3 | gleam |
    | n/a | n/a | 4 | oxy |

  The new Fluid tries to keep backwards compatibility and reads both
  keys (\p scheme and \p scheme_name). If the latter is defined, it is used.
  If not the old \p scheme (index) is used - but we need to subtract one to
  get the new Fl_Scheme_Choice index (column "Choice" above).
*/
void init_scheme() {
  int scheme_index = 0;                     // scheme index for backwards compatibility (1.3.x)
  char *scheme_name = 0;                    // scheme name since 1.4.0
  fluid_prefs.get("scheme_name", scheme_name, "XXX"); // XXX means: not set => fallback 1.3.x
  if (!strcmp(scheme_name, "XXX")) {
    fluid_prefs.get("scheme", scheme_index, 0);
    if (scheme_index > 0) {
      scheme_index--;
      scheme_choice->value(scheme_index);   // set the choice value
    }
    if (scheme_index < 0)
      scheme_index = 0;
    else if (scheme_index > scheme_choice->size() - 1)
      scheme_index = 0;
    scheme_name = const_cast<char *>(scheme_choice->text(scheme_index));
    fluid_prefs.set("scheme_name", scheme_name);
  }
  // Set the new scheme only if it was not overridden by the -scheme
  // command line option
  if (Fl::scheme() == NULL) {
    Fl::scheme(scheme_name);
  }
  free(scheme_name);
}

/**
 Show or hide the widget bin.
 The state is stored in the app preferences.
 */
void toggle_widgetbin_cb(Fl_Widget *, void *) {
  if (!widgetbin_panel) {
    make_widgetbin();
    if (!position_window(widgetbin_panel,"widgetbin_pos", 1, 320, 30)) return;
  }

  if (widgetbin_panel->visible()) {
    widgetbin_panel->hide();
    widgetbin_item->label("Show Widget &Bin...");
  } else {
    widgetbin_panel->show();
    widgetbin_item->label("Hide Widget &Bin");
  }
}

/**
 Show or hide the code preview window.
 */
void toggle_codeview_cb(Fl_Double_Window *, void *) {
  codeview_toggle_visibility();
}

/**
 Show or hide the code preview window, button callback.
 */
void toggle_codeview_b_cb(Fl_Button*, void *) {
  codeview_toggle_visibility();
}

/**
 Build the main app window and create a few other dialogs.
 */
void make_main_window() {
  if (!batch_mode) {
    fluid_prefs.get("show_guides", show_guides, 1);
    fluid_prefs.get("show_restricted", show_restricted, 1);
    fluid_prefs.get("show_ghosted_outline", show_ghosted_outline, 0);
    fluid_prefs.get("show_comments", show_comments, 1);
    make_shell_window();
  }

  if (!main_window) {
    Fl_Widget *o;
    loadPixmaps();
    main_window = new Fl_Double_Window(WINWIDTH,WINHEIGHT,"fluid");
    main_window->box(FL_NO_BOX);
    o = make_widget_browser(0,MENUHEIGHT,BROWSERWIDTH,BROWSERHEIGHT);
    o->box(FL_FLAT_BOX);
    o->tooltip("Double-click to view or change an item.");
    main_window->resizable(o);
    main_menubar = new Fl_Menu_Bar(0,0,BROWSERWIDTH,MENUHEIGHT);
    main_menubar->menu(Main_Menu);
    // quick access to all dynamic menu items
    save_item = (Fl_Menu_Item*)main_menubar->find_item(save_cb);
    history_item = (Fl_Menu_Item*)main_menubar->find_item(menu_file_open_history_cb);
    widgetbin_item = (Fl_Menu_Item*)main_menubar->find_item(toggle_widgetbin_cb);
    codeview_item = (Fl_Menu_Item*)main_menubar->find_item((Fl_Callback*)toggle_codeview_cb);
    overlay_item = (Fl_Menu_Item*)main_menubar->find_item((Fl_Callback*)toggle_overlays);
    guides_item = (Fl_Menu_Item*)main_menubar->find_item((Fl_Callback*)toggle_guides);
    restricted_item = (Fl_Menu_Item*)main_menubar->find_item((Fl_Callback*)toggle_restricted);
    main_menubar->global();
    fill_in_New_Menu();
    main_window->end();
  }

  if (!batch_mode) {
    load_history();
    g_shell_config = new Fd_Shell_Command_List;
    widget_browser->load_prefs();
    make_settings_window();
  }
}

/**
 Load file history from preferences.

 This loads the absolute filepaths of the last 10 used design files.
 It also computes and stores the relative filepaths for display in
 the main menu.
 */
void load_history() {
  int   i;              // Looping var
  int   max_files;


  fluid_prefs.get("recent_files", max_files, 5);
  if (max_files > 10) max_files = 10;

  for (i = 0; i < max_files; i ++) {
    fluid_prefs.get( Fl_Preferences::Name("file%d", i), absolute_history[i], "", sizeof(absolute_history[i]));
    if (absolute_history[i][0]) {
      // Make a relative version of the filename for the menu...
      fl_filename_relative(relative_history[i], sizeof(relative_history[i]),
                           absolute_history[i]);

      if (i == 9) history_item[i].flags = FL_MENU_DIVIDER;
      else history_item[i].flags = 0;
    } else break;
  }

  for (; i < 10; i ++) {
    if (i) history_item[i-1].flags |= FL_MENU_DIVIDER;
    history_item[i].hide();
  }
}

/**
 Update file history from preferences.

 Add this new filepath to the history and update the main menu.
 Writes the new file history to the app preferences.

 \param[in] flname path or filename of .fl file, will be converted into an
    absolute file path based on the current working directory.
 */
void update_history(const char *flname) {
  int   i;              // Looping var
  char  absolute[FL_PATH_MAX];
  int   max_files;


  fluid_prefs.get("recent_files", max_files, 5);
  if (max_files > 10) max_files = 10;

  fl_filename_absolute(absolute, sizeof(absolute), flname);

  for (i = 0; i < max_files; i ++)
#if defined(_WIN32) || defined(__APPLE__)
    if (!strcasecmp(absolute, absolute_history[i])) break;
#else
    if (!strcmp(absolute, absolute_history[i])) break;
#endif // _WIN32 || __APPLE__

  if (i == 0) return;

  if (i >= max_files) i = max_files - 1;

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
  for (i = 0; i < max_files; i ++) {
    fluid_prefs.set( Fl_Preferences::Name("file%d", i), absolute_history[i]);
    if (absolute_history[i][0]) {
      if (i == 9) history_item[i].flags = FL_MENU_DIVIDER;
      else history_item[i].flags = 0;
    } else break;
  }

  for (; i < 10; i ++) {
    fluid_prefs.set( Fl_Preferences::Name("file%d", i), "");
    if (i) history_item[i-1].flags |= FL_MENU_DIVIDER;
    history_item[i].hide();
  }
  fluid_prefs.flush();
}

/**
 Set the filename of the current .fl design.
 \param[in] c the new absolute filename and path
 */
void set_filename(const char *c) {
  if (filename) free((void *)filename);
  filename = c ? fl_strdup(c) : NULL;

  if (filename && !batch_mode)
    update_history(filename);

  set_modflag(modflag);
}


/**
 Set the "modified" flag and update the title of the main window.

 The first argument sets the modification state of the current design against
 the corresponding .fl design file. Any change to the widget tree will mark
 the design 'modified'. Saving the design will mark it clean.

 The second argument is optional and set the modification state of the current
 design against the source code and header file. Any change to the tree,
 including saving the tree, will mark the code 'outdated'. Generating source
 code and header files will clear this flag until the next modification.

 \param[in] mf 0 to clear the modflag, 1 to mark the design "modified", -1 to
    ignore this parameter
 \param[in] mfc default -1 to let \c mf control \c modflag_c, 0 to mark the
    code files current, 1 to mark it out of date. -2 to ignore changes to mf.
 */
void set_modflag(int mf, int mfc) {
  const char *code_ext = NULL;
  char new_title[FL_PATH_MAX];

  // Update the modflag_c to the worst possible condition. We could be a bit
  // more graceful and compare modification times of the files, but C++ has
  // no API for that until C++17.
  if (mf!=-1) {
    modflag = mf;
    if (mfc==-1 && mf==1)
      mfc = mf;
  }
  if (mfc>=0) {
    modflag_c = mfc;
  }

  if (main_window) {
    Fl_String basename;
    if (!filename) basename = "Untitled.fl";
    else basename = fl_filename_name(Fl_String(filename));
    code_ext = fl_filename_ext(g_project.code_file_name.c_str());
    char mod_star = modflag ? '*' : ' ';
    char mod_c_star = modflag_c ? '*' : ' ';
    snprintf(new_title, sizeof(new_title), "%s%c  %s%c",
             basename.c_str(), mod_star, code_ext, mod_c_star);
    const char *old_title = main_window->label();
    // only update the title if it actually changed
    if (!old_title || strcmp(old_title, new_title))
      main_window->copy_label(new_title);
  }
  // if the UI was modified in any way, update the Code View panel
  if (codeview_panel && codeview_panel->visible() && cv_autorefresh->value())
    codeview_defer_update();
}

// ---- Main program entry point

/**
 Handle command line arguments.
 \param[in] argc number of arguments in the list
 \param[in] argv pointer to an array of arguments
 \param[inout] i current argument index
 \return number of arguments used; if 0, the argument is not supported
 */
static int arg(int argc, char** argv, int& i) {
  if (argv[i][0] != '-')
    return 0;
  if (argv[i][1] == 'd' && !argv[i][2]) {
    G_debug=1;
    i++; return 1;
  }
  if (argv[i][1] == 'u' && !argv[i][2]) {
    update_file++;
    batch_mode++;
    i++; return 1;
  }
  if (argv[i][1] == 'c' && !argv[i][2]) {
    compile_file++;
    batch_mode++;
    i++; return 1;
  }
  if (argv[i][1] == 'c' && argv[i][2] == 's' && !argv[i][3]) {
    compile_file++;
    compile_strings++;
    batch_mode++;
    i++; return 1;
  }
  if (argv[i][1] == 'o' && !argv[i][2] && i+1 < argc) {
    g_code_filename_arg = argv[i+1];
    batch_mode++;
    i += 2; return 2;
  }
#ifndef NDEBUG
  if ((i+1 < argc) && (strcmp(argv[i], "--autodoc") == 0)) {
    g_autodoc_path = argv[i+1];
    i += 2; return 2;
  }
#endif
  if (argv[i][1] == 'h' && !argv[i][2]) {
    if ( (i+1 < argc) && (argv[i+1][0] != '-') ) {
      g_header_filename_arg = argv[i+1];
      batch_mode++;
      i += 2;
      return 2;
    } else {
      // a lone "-h" without a filename will output the help string
      return 0;
    }
  }
  return 0;
}

#if ! (defined(_WIN32) && !defined (__CYGWIN__))

int quit_flag = 0;
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
  quit_flag = 1;
}
}

#endif

/**
 Start Fluid.

 Fluid can run in interactive mode with a full user interface to design new
 user interfaces and write the C++ files to manage them,

 Fluid can run form the command line in batch mode to convert .fl design files
 into C++ source and header files. In batch mode, no display is needed,
 particularly no X11 connection will be attempted on Linux/Unix.

 \param[in] argc number of arguments in the list
 \param[in] argv pointer to an array of arguments
 \return in batch mode, an error code will be returned via \c exit() . This
    function return 1, if there was an error in the parameters list.
 \todo On MSWindows, Fluid can under certain conditions open a dialog box, even
    in batch mode. Is that intentional? Does it circumvent issues with Windows'
 stderr and stdout?
 */
int main(int argc,char **argv) {
  int i = 1;

  setlocale(LC_ALL, "");      // enable multi-language errors in file chooser
  setlocale(LC_NUMERIC, "C"); // make sure numeric values are written correctly
  g_launch_path = end_with_slash(fl_getcwd()); // store the current path at launch

  Fl::args_to_utf8(argc, argv); // for MSYS2/MinGW
  if (   (Fl::args(argc,argv,i,arg) == 0)     // unsupported argument found
      || (batch_mode && (i != argc-1))        // .fl filename missing
      || (!batch_mode && (i < argc-1))        // more than one filename found
      || (argv[i] && (argv[i][0] == '-'))) {  // unknown option
    static const char *msg =
      "usage: %s <switches> name.fl\n"
      " -u : update .fl file and exit (may be combined with '-c' or '-cs')\n"
      " -c : write .cxx and .h and exit\n"
      " -cs : write .cxx and .h and strings and exit\n"
      " -o <name> : .cxx output filename, or extension if <name> starts with '.'\n"
      " -h <name> : .h output filename, or extension if <name> starts with '.'\n"
      " -d : enable internal debugging\n";
    const char *app_name = NULL;
    if ( (argc > 0) && argv[0] && argv[0][0] )
      app_name = fl_filename_name(argv[0]);
    if ( !app_name || !app_name[0])
      app_name = "fluid";
#ifdef _MSC_VER
    fl_message(msg, app_name);
#else
    fprintf(stderr, msg, app_name);
#endif
    return 1;
  }

  const char *c = NULL;
  if (g_autodoc_path.empty())
    c = argv[i];

  fl_register_images();

  make_main_window();

  if (c) set_filename(c);
  if (!batch_mode) {
#ifdef __APPLE__
    fl_open_callback(apple_open_cb);
#endif // __APPLE__
    Fl::visual((Fl_Mode)(FL_DOUBLE|FL_INDEX));
    Fl_File_Icon::load_system_icons();
    main_window->callback(exit_cb);
    position_window(main_window,"main_window_pos", 1, 10, 30, WINWIDTH, WINHEIGHT );
    if (g_shell_config) {
      g_shell_config->read(fluid_prefs, FD_STORE_USER);
      g_shell_config->update_settings_dialog();
      g_shell_config->rebuild_shell_menu();
    }
    g_layout_list.read(fluid_prefs, FD_STORE_USER);
    main_window->show(argc,argv);
    toggle_widgetbin_cb(0,0);
    toggle_codeview_cb(0,0);
    if (!c && openlast_button->value() && absolute_history[0][0] && g_autodoc_path.empty()) {
      // Open previous file when no file specified...
      open_project_file(absolute_history[0]);
    }
  }
  undo_suspend();
  if (c && !read_file(c,0)) {
    if (batch_mode) {
      fprintf(stderr,"%s : %s\n", c, strerror(errno));
      exit(1);
    }
    fl_message("Can't read %s: %s", c, strerror(errno));
  }
  undo_resume();

  // command line args override code and header filenames from the project file
  // in batch mode only
  if (batch_mode) {
    if (!g_code_filename_arg.empty()) {
      g_project.code_file_set = 1;
      g_project.code_file_name = g_code_filename_arg;
    }
    if (!g_header_filename_arg.empty()) {
      g_project.header_file_set = 1;
      g_project.header_file_name = g_header_filename_arg;
    }
  }

  if (update_file) {            // fluid -u
    write_file(c,0);
    if (!compile_file)
      exit(0);
  }

  if (compile_file) {           // fluid -c[s]
    if (compile_strings)
      write_strings_cb(0,0);
    write_cb(0,0);
    exit(0);
  }

  // don't lock up if inconsistent command line arguments were given
  if (batch_mode)
    exit(0);

  set_modflag(0);
  undo_clear();
#ifndef _WIN32
  signal(SIGINT,sigint);
#endif

  // Set (but do not start) timer callback for external editor updates
  ExternalCodeEditor::set_update_timer_callback(external_editor_timer);

#ifndef NDEBUG
  // check if the user wants FLUID to generate image for the user documentation
  if (!g_autodoc_path.empty()) {
    run_autodoc(g_autodoc_path);
    return 0;
  }
#endif

#ifdef _WIN32
  Fl::run();
#else
  while (!quit_flag) Fl::wait();
  if (quit_flag) exit_cb(0,0);
#endif // _WIN32

  undo_clear();
  return (0);
}

/// \}

