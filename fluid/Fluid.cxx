//
// FLUID main entry for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

#include "Fluid.h"

#include "Project.h"
#include "app/mergeback.h"
#include "app/undo.h"
#include "io/Project_Reader.h"
#include "io/Project_Writer.h"
#include "io/Code_Writer.h"
#include "nodes/Fl_Type.h"
#include "nodes/Fl_Function_Type.h"
#include "nodes/Fl_Group_Type.h"
#include "nodes/Fl_Window_Type.h"
#include "nodes/factory.h"
#include "panels/settings_panel.h"
#include "panels/function_panel.h"
#include "panels/codeview_panel.h"
#include "panels/template_panel.h"
#include "panels/about_panel.h"
#include "rsrcs/pixmaps.h"
#include "app/shell_command.h"
#include "tools/autodoc.h"
#include "widgets/Node_Browser.h"

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

fld::Application Fluid;

using namespace fld;

Application::Application()
: preferences( Fl_Preferences::USER_L, "fltk.org", "fluid" ) 
{ }


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



// File history info...

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

/// Set if the current design has been modified compared to the associated .fl design file.
int modflag = 0;

/// Set if the code files are older than the current design.
int modflag_c = 0;

/// Set, if Fluid was started with the command line argument -v
int show_version = 0;        // fluid -v

/// Offset in pixels when adding widgets from an .fl file.
int pasteoffset = 0;

/// Paste offset incrementing at every paste command.
static int ipasteoffset = 0;


/**
 Generate a path to a directory for temporary data storage.
 The path is stored in g_tmpdir.
 TODO: see ExternalCodeEditor::create_tmpdir()!
 */
void Application::create_tmpdir() {
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
  std::string name = buf;
  wchar_t tempdirW[FL_PATH_MAX+1];
  char tempdir[FL_PATH_MAX+1];
  unsigned len = GetTempPathW(FL_PATH_MAX, tempdirW);
  if (len == 0) {
    strcpy(tempdir, "c:/windows/temp/");
  } else {
    unsigned wn = fl_utf8fromwc(tempdir, FL_PATH_MAX, tempdirW, len);
    tempdir[wn] = 0;
  }
  std::string path = tempdir;
  end_with_slash(path);
  path += name;
  fl_make_path(path.c_str());
  if (fl_access(path.c_str(), 6) == 0) tmpdir_path = path;
#else
  fl_snprintf(buf, sizeof(buf)-1, "fluid-%d/", getpid());
  std::string name = buf;
  auto path_temp = fl_getenv("TMPDIR");
  std::string path = path_temp ? path_temp : "";
  if (!path.empty()) {
    end_with_slash(path);
    path += name;
    fl_make_path(path.c_str());
    if (fl_access(path.c_str(), 6) == 0) tmpdir_path = path;
  }
  if (tmpdir_path.empty()) {
    path = std::string("/tmp/") + name;
    fl_make_path(path.c_str());
    if (fl_access(path.c_str(), 6) == 0) tmpdir_path = path;
  }
#endif
  if (tmpdir_path.empty()) {
    char pbuf[FL_PATH_MAX+1];
    preferences.get_userdata_path(pbuf, FL_PATH_MAX);
    path = std::string(pbuf);
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
void Application::delete_tmpdir() {
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
      std::string path = tmpdir_path + de[i]->d_name;
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
const std::string &Application::get_tmpdir() {
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
      Fluid.save_project_file(nullptr);
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
  Fl_Preferences pos(Fluid.preferences, prefsName);
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
  Fl_Preferences pos(Fluid.preferences, prefsName);
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
    Fluid.preferences.getUserdataPath(name[0], sizeof(name[0]));
    strlcat(name[0], "cut_buffer", sizeof(name[0]));
    Fluid.preferences.getUserdataPath(name[1], sizeof(name[1]));
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
  if ( Fluid.debug_external_editor ) printf("--- TIMER --- External editors open=%d\n", editors_open);
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
void Application::save_project_file(void *v) {
  flush_text_widgets();
  Fl_Native_File_Chooser fnfc;
  const char *c = proj.proj_filename;
  if (v || !c || !*c) {
    fnfc.title("Save To:");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fnfc.filter("FLUID Files\t*.f[ld]");
    if (fnfc.show() != 0) return;
    c = fnfc.filename();
    if (!fl_access(c, 0)) {
      std::string basename = fl_filename_name_str(std::string(c));
      if (fl_choice("The file \"%s\" already exists.\n"
                    "Do you want to replace it?", "Cancel",
                    "Replace", NULL, basename.c_str()) == 0) return;
    }

    if (v != (void *)2) set_filename(c);
  }
  if (!fld::io::write_file(c)) {
    fl_alert("Error writing %s: %s", c, strerror(errno));
    return;
  }

  if (v != (void *)2) {
    set_modflag(0, 1);
    undo_save = undo_current;
  }
}


/**
 Reload the file set by \c filename, replacing the current design.
 If the design was modified, a dialog will ask for confirmation.
 */
void Application::revert_project() {
  if (modflag) {
    if (!fl_choice("This user interface has been changed. Really revert?",
                   "Cancel", "Revert", NULL)) return;
  }
  undo_suspend();
  if (!fld::io::read_file(proj.proj_filename, 0)) {
    undo_resume();
    widget_browser->rebuild();
    proj.update_settings_dialog();
    fl_message("Can't read %s: %s", proj.proj_filename, strerror(errno));
    return;
  }
  widget_browser->rebuild();
  undo_resume();
  set_modflag(0, 0);
  undo_clear();
  proj.update_settings_dialog();
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
    Fl_Preferences svp(Fluid.preferences, "codeview");
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
    g_shell_config->write(Fluid.preferences, FD_STORE_USER);
  g_layout_list.write(Fluid.preferences, FD_STORE_USER);

  undo_clear();

  // Destroy tree
  //    Doing so causes dtors to automatically close all external editors
  //    and cleans up editor tmp files. Then remove fluid tmpdir /last/.
  Fluid.proj.reset();
  ExternalCodeEditor::tmpdir_clear();
  Fluid.delete_tmpdir();

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
bool Application::new_project(bool user_must_confirm) {
  // verify user intention
  if ((user_must_confirm) &&  (confirm_project_clear() == false))
    return false;

  // clear the current project
  proj.reset();
  set_filename(NULL);
  set_modflag(0, 0);
  widget_browser->rebuild();
  proj.update_settings_dialog();

  // all is clear to continue
  return true;
}

/**
 Open the template browser and load a new file from templates.

 If the current project was modified, FLUID will give the user the opportunity
 to save the old project first.

 \return false if the operation was canceled or failed otherwise
 */
bool Application::new_project_from_template() {
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
      fld::io::read_file(cutfname(1), 0);
      fl_unlink(cutfname(1));
      undo_resume();
    } else {
      // No instance name, so read the template without replacements...
      undo_suspend();
      fld::io::read_file(tname, 0);
      undo_resume();
    }
  }

  widget_browser->rebuild();
  proj.update_settings_dialog();
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
std::string open_project_filechooser(const std::string &title) {
  Fl_Native_File_Chooser dialog;
  dialog.title(title.c_str());
  dialog.type(Fl_Native_File_Chooser::BROWSE_FILE);
  dialog.filter("FLUID Files\t*.f[ld]\n");
  if (Fluid.proj.proj_filename) {
    std::string current_project_file = Fluid.proj.proj_filename;
    dialog.directory(fl_filename_path_str(current_project_file).c_str());
    dialog.preset_file(fl_filename_name_str(current_project_file).c_str());
  }
  if (dialog.show() != 0)
    return std::string();
  return std::string(dialog.filename());
}

/**
 Load a project from the give file name and path.

 The project file is inserted at the currently selected type.

 If no filename is given, FLUID will open a file chooser dialog.

 \param[in] filename_arg path and name of the new project file
 \return false if the operation failed
 */
bool Application::merge_project_file(const std::string &filename_arg) {
  bool is_a_merge = (Fl_Type::first != NULL);
  std::string title = is_a_merge ? "Merge Project File" : "Open Project File";

  // ask for a filename if none was given
  std::string new_filename = filename_arg;
  if (new_filename.empty()) {
    new_filename = open_project_filechooser(title);
    if (new_filename.empty()) {
      return false;
    }
  }

  const char *c = new_filename.c_str();
  const char *oldfilename = proj.proj_filename;
  proj.proj_filename    = NULL;
  set_filename(c);
  if (is_a_merge) undo_checkpoint();
  undo_suspend();
  if (!fld::io::read_file(c, is_a_merge)) {
    undo_resume();
    widget_browser->rebuild();
    proj.update_settings_dialog();
    fl_message("Can't read %s: %s", c, strerror(errno));
    free((void *)proj.proj_filename);
    proj.proj_filename = oldfilename;
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
bool Application::open_project_file(const std::string &filename_arg) {
  // verify user intention
  if (confirm_project_clear() == false)
    return false;

  // ask for a filename if none was given
  std::string new_filename = filename_arg;
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
  Fluid.open_project_file(std::string(c));
}
#endif // __APPLE__

/**
 Generate the C++ source and header filenames and write those files.

 This function creates the source filename by setting the file
 extension to \c code_file_name and a header filename
 with the extension \c code_file_name which are both
 settable by the user.

 If the code filename has not been set yet, a "save file as" dialog will be
 presented to the user.

 In Fluid.batch_mode, the function will either be silent, or, if opening or writing
 the files fails, write an error message to \c stderr and exit with exit code 1.

 In interactive mode, it will pop up an error message, or, if the user
 hasn't disabled that, pop up a confirmation message.

 \param[in] dont_show_completion_dialog don't show the completion dialog
 \return 1 if the operation failed, 0 if it succeeded
 */
int Application::write_code_files(bool dont_show_completion_dialog)
{
  // -- handle user interface issues
  flush_text_widgets();
  if (!proj.proj_filename) {
    save_project_file(nullptr);
    if (!proj.proj_filename) return 1;
  }

  // -- generate the file names with absolute paths
  fld::io::Code_Writer f;
  std::string code_filename = proj.codefile_path() + proj.codefile_name();
  std::string header_filename = proj.headerfile_path() + proj.headerfile_name();

  // -- write the code and header files
  if (!batch_mode) proj.enter_project_dir();
  int x = f.write_code(code_filename.c_str(), header_filename.c_str());
  std::string code_filename_rel = fl_filename_relative_str(code_filename);
  std::string header_filename_rel = fl_filename_relative_str(header_filename);
  if (!batch_mode) proj.leave_project_dir();

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

#if 0
// Matt: disabled
/**
 Merge the possibly modified content of code files back into the project.
 */
int mergeback_code_files()
{
  flush_text_widgets();
  if (!filename) return 1;
  if (!Fluid.proj.write_mergeback_data) {
    fl_message("MergeBack is not enabled for this project.\n"
               "Please enable MergeBack in the project settings\n"
               "dialog and re-save the project file and the code.");
    return 0;
  }

  std::string proj_filename = Fluid.proj.projectfile_path() + Fluid.proj.projectfile_name();
  std::string code_filename;
#if 1
  if (!Fluid.batch_mode) {
    Fl_Preferences build_records(Fl_Preferences::USER_L, "fltk.org", "fluid-build");
    Fl_Preferences path(build_records, proj_filename.c_str());
    int i, n = proj_filename.size();
    for (i=0; i<n; i++) if (proj_filename[i]=='\\') proj_filename[i] = '/';
    preferences_get(path, "code", code_filename, "");
  }
#endif
  if (code_filename.empty())
    code_filename = Fluid.proj.codefile_path() + Fluid.proj.codefile_name();
  if (!Fluid.batch_mode) proj.enter_project_dir();
  int c = merge_back(code_filename, proj_filename, FD_MERGEBACK_INTERACTIVE);
  if (!Fluid.batch_mode) proj.leave_project_dir();

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
  if (!Fluid.proj.proj_filename) {
    Fluid.save_project_file(nullptr);
    if (!Fluid.proj.proj_filename) return;
  }
  std::string filename = Fluid.proj.stringsfile_path() + Fluid.proj.stringsfile_name();
  int x = write_strings(filename);
  if (Fluid.batch_mode) {
    if (x) {
      fprintf(stderr, "%s : %s\n", filename.c_str(), strerror(errno));
      exit(1);
    }
  } else {
    if (x) {
      fl_message("Can't write %s: %s", filename.c_str(), strerror(errno));
    } else if (completion_button->value()) {
      fl_message("Wrote %s", Fluid.proj.stringsfile_name().c_str());
    }
  }
}

/**
 Show the editor for the \c current Fl_Type.
 */
void Application::edit_selected() {
  if (!Fl_Type::current) {
    fl_message("Please select a widget");
    return;
  }
  Fl_Type::current->open();
}

/**
 User chose to copy the currently selected widgets.
 */
void Application::copy_selected() {
  flush_text_widgets();
  if (!Fl_Type::current) {
    fl_beep();
    return;
  }
  flush_text_widgets();
  ipasteoffset = 10;
  if (!fld::io::write_file(cutfname(),1)) {
    fl_message("Can't write %s: %s", cutfname(), strerror(errno));
    return;
  }
}

/**
 User chose to cut the currently selected widgets.
 */
void Application::cut_selected() {
  if (!Fl_Type::current) {
    fl_beep();
    return;
  }
  flush_text_widgets();
  if (!fld::io::write_file(cutfname(),1)) {
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
void Application::delete_selected() {
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
void Application::paste_from_clipboard() {
  pasteoffset = ipasteoffset;
  undo_checkpoint();
  undo_suspend();
  Strategy strategy = Strategy::FROM_FILE_AFTER_CURRENT;
  if (Fl_Type::current && Fl_Type::current->can_have_children()) {
    if (Fl_Type::current->folded_ == 0) {
      // If the current widget is a group widget and it is not folded,
      // add the new widgets inside the group.
      strategy = Strategy::FROM_FILE_AS_LAST_CHILD;
      // The following alternative also works quite nicely
      //strategy = Strategy::FROM_FILE_AS_FIRST_CHILD;
    }
  }
  if (!fld::io::read_file(cutfname(), 1, strategy)) {
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
void Application::duplicate_selected() {
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
  if (!fld::io::write_file(cutfname(1),1)) {
    fl_message("Can't write %s: %s", cutfname(1), strerror(errno));
    return;
  }

  // read the file and add the widgets after the current one:
  pasteoffset  = 0;
  undo_checkpoint();
  undo_suspend();
  if (!fld::io::read_file(cutfname(1), 1, Strategy::FROM_FILE_AFTER_CURRENT)) {
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
void Application::sort_selected() {
  undo_checkpoint();
  sort((Fl_Type*)NULL);
  widget_browser->rebuild();
  set_modflag(1);
}

/**
 Open the "About" dialog.
 */
void Application::about() {
  if (!about_panel) make_about_panel();
  about_panel->show();
}

/**
 Open a dialog to show the HTML help page form the FLTK documentation folder.
 \param[in] name name of the HTML help file.
 */
void Application::show_help(const char *name) {
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
       "\"https://www.fltk.org/doc-1.5/fluid.html\">https://www.fltk.org/</a>"
       "</body></html>"
       );
    } else if (strcmp(name, "license.html")==0) {
      fl_open_uri("https://www.fltk.org/doc-1.5/license.html");
      return;
    } else if (strcmp(name, "index.html")==0) {
      fl_open_uri("https://www.fltk.org/doc-1.5/index.html");
      return;
    } else {
      snprintf(helpname, sizeof(helpname), "https://www.fltk.org/%s", name);
      fl_open_uri(helpname);
      return;
    }
  }
  help_dialog->show();
}


// ---- Printing

/**
 Open the dialog to allow the user to print the current window.
 */
void Application::print_snapshots() {
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
    std::string basename = fl_filename_name_str(std::string(proj.proj_filename));
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
  if (Fluid.batch_mode)
    return;

  // set the new scheme only if the scheme was changed
  const char *new_scheme = choice->text(choice->value());

  if (Fl::is_scheme(new_scheme))
    return;

  Fl::scheme(new_scheme);
  Fluid.preferences.set("scheme_name", new_scheme);

  // Backwards compatibility: store 1.3 scheme index (1-4).
  // We assume that index 0-3 (base, plastic, gtk+, gleam) are in the
  // same order as in 1.3.x (index 1-4), higher values are ignored

  int scheme_index = scheme_choice->value();
  if (scheme_index <= 3)                          // max. index for 1.3.x (Gleam)
    Fluid.preferences.set("scheme", scheme_index + 1);  // compensate for different indexing
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
  Fluid.preferences.get("scheme_name", scheme_name, "XXX"); // XXX means: not set => fallback 1.3.x
  if (!strcmp(scheme_name, "XXX")) {
    Fluid.preferences.get("scheme", scheme_index, 0);
    if (scheme_index > 0) {
      scheme_index--;
      scheme_choice->value(scheme_index);   // set the choice value
    }
    if (scheme_index < 0)
      scheme_index = 0;
    else if (scheme_index > scheme_choice->size() - 1)
      scheme_index = 0;
    scheme_name = const_cast<char *>(scheme_choice->text(scheme_index));
    Fluid.preferences.set("scheme_name", scheme_name);
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


extern void menu_file_save_cb(Fl_Widget *, void *arg);
extern void menu_file_open_history_cb(Fl_Widget *, void *v);

/**
 Build the main app window and create a few other dialogs.
 */
void make_main_window() {
  if (!Fluid.batch_mode) {
    Fluid.preferences.get("Fluid.show_guides", Fluid.show_guides, 1);
    Fluid.preferences.get("Fluid.show_restricted", Fluid.show_restricted, 1);
    Fluid.preferences.get("Fluid.show_ghosted_outline", Fluid.show_ghosted_outline, 0);
    Fluid.preferences.get("Fluid.show_comments", Fluid.show_comments, 1);
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
    save_item = (Fl_Menu_Item*)main_menubar->find_item(menu_file_save_cb);
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

  if (!Fluid.batch_mode) {
    Fluid.load_project_history();
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
void Application::load_project_history() {
  int   i;              // Looping var
  int   max_files;

  preferences.get("recent_files", max_files, 5);
  if (max_files > 10) max_files = 10;

  for (i = 0; i < max_files; i ++) {
    preferences.get( Fl_Preferences::Name("file%d", i), project_history_abspath[i], "", sizeof(project_history_abspath[i]));
    if (project_history_abspath[i][0]) {
      // Make a shortened version of the filename for the menu...
      std::string fn = fl_filename_shortened(project_history_abspath[i], 48);
      strncpy(project_history_relpath[i], fn.c_str(), sizeof(project_history_relpath[i]) - 1);
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
void Application::update_project_history(const char *flname) {
  int   i;              // Looping var
  char  absolute[FL_PATH_MAX];
  int   max_files;


  preferences.get("recent_files", max_files, 5);
  if (max_files > 10) max_files = 10;

  fl_filename_absolute(absolute, sizeof(absolute), flname);
#ifdef _WIN32
  // Make path canonical.
  for (char *s = absolute; *s; s++) {
    if (*s == '\\')
      *s = '/';
  }
#endif


  for (i = 0; i < max_files; i ++)
#if defined(_WIN32) || defined(__APPLE__)
    if (!strcasecmp(absolute, project_history_abspath[i])) break;
#else
    if (!strcmp(absolute, project_history_abspath[i])) break;
#endif // _WIN32 || __APPLE__

  if (i == 0) return;

  if (i >= max_files) i = max_files - 1;

  // Move the other flnames down in the list...
  memmove(project_history_abspath + 1, project_history_abspath,
          i * sizeof(project_history_abspath[0]));
  memmove(project_history_relpath + 1, project_history_relpath,
          i * sizeof(project_history_relpath[0]));

  // Put the new file at the top...
  strlcpy(project_history_abspath[0], absolute, sizeof(project_history_abspath[0]));
  std::string fn = fl_filename_shortened(project_history_abspath[0], 48);
  strncpy(project_history_relpath[0], fn.c_str(), sizeof(project_history_relpath[0]) - 1);

  // Update the menu items as needed...
  for (i = 0; i < max_files; i ++) {
    preferences.set( Fl_Preferences::Name("file%d", i), project_history_abspath[i]);
    if (project_history_abspath[i][0]) {
      if (i == 9) history_item[i].flags = FL_MENU_DIVIDER;
      else history_item[i].flags = 0;
    } else break;
  }

  for (; i < 10; i ++) {
    preferences.set( Fl_Preferences::Name("file%d", i), "");
    if (i) history_item[i-1].flags |= FL_MENU_DIVIDER;
    history_item[i].hide();
  }
  preferences.flush();
}

/**
 Set the filename of the current .fl design.
 \param[in] c the new absolute filename and path
 */
void set_filename(const char *c) {
  if (Fluid.proj.proj_filename) free((void *)Fluid.proj.proj_filename);
  Fluid.proj.proj_filename = c ? fl_strdup(c) : NULL;

  if (Fluid.proj.proj_filename && !Fluid.batch_mode)
    Fluid.update_project_history(Fluid.proj.proj_filename);

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
    std::string basename;
    if (!Fluid.proj.proj_filename) basename = "Untitled.fl";
    else basename = fl_filename_name_str(std::string(Fluid.proj.proj_filename));
    code_ext = fl_filename_ext(Fluid.proj.code_file_name.c_str());
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


int Application::arg_cb(int argc, char** argv, int& i) {
  return Fluid.arg(argc, argv, i);
}

/**
 Handle command line arguments.
 \param[in] argc number of arguments in the list
 \param[in] argv pointer to an array of arguments
 \param[inout] i current argument index
 \return number of arguments used; if 0, the argument is not supported
 */
int Application::arg(int argc, char** argv, int& i) {
  if (argv[i][0] != '-')
    return 0;
  if (argv[i][1] == 'd' && !argv[i][2]) {
    debug_external_editor=1;
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
  if ((strcmp(argv[i], "-v")==0) || (strcmp(argv[i], "--version")==0)) {
    show_version = 1;
    i++; return 1;
  }
  if (argv[i][1] == 'c' && argv[i][2] == 's' && !argv[i][3]) {
    compile_file++;
    compile_strings++;
    batch_mode++;
    i++; return 1;
  }
  if (argv[i][1] == 'o' && !argv[i][2] && i+1 < argc) {
    code_filename_arg = argv[i+1];
    batch_mode++;
    i += 2; return 2;
  }
#ifndef NDEBUG
  if ((i+1 < argc) && (strcmp(argv[i], "--autodoc") == 0)) {
    autodoc_path = argv[i+1];
    i += 2; return 2;
  }
#endif
  if (strcmp(argv[i], "--help")==0) {
    return 0;
  }
  if (argv[i][1] == 'h' && !argv[i][2]) {
    if ( (i+1 < argc) && (argv[i+1][0] != '-') ) {
      header_filename_arg = argv[i+1];
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
 \todo On Windows, Fluid can under certain conditions open a dialog box, even
    in batch mode. Is that intentional? Does it circumvent issues with Windows'
    stderr and stdout?
 */
int Application::run(int argc,char **argv) {
  int i = 1;

  setlocale(LC_ALL, "");      // enable multi-language errors in file chooser
  setlocale(LC_NUMERIC, "C"); // make sure numeric values are written correctly
  launch_path = end_with_slash(fl_getcwd_str()); // store the current path at launch

  Fl::args_to_utf8(argc, argv); // for MSYS2/MinGW
  if (   (Fl::args(argc,argv,i,arg_cb) == 0)     // unsupported argument found
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
      " --help : brief usage information\n"
      " --version, -v : print fluid version number\n"
      " -d : enable internal debugging\n";
    const char *app_name = NULL;
    if ( (argc > 0) && argv[0] && argv[0][0] )
      app_name = fl_filename_name(argv[0]);
    if ( !app_name || !app_name[0])
      app_name = "fluid";
#ifdef _MSC_VER
    // TODO: if this is fluid-cmd, use stderr and not fl_message
    fl_message(msg, app_name);
#else
    fprintf(stderr, msg, app_name);
#endif
    return 1;
  }
  if (show_version) {
    printf("fluid v%d.%d.%d\n", FL_MAJOR_VERSION, FL_MINOR_VERSION, FL_PATCH_VERSION);
    ::exit(0);
  }

  const char *c = NULL;
  if (autodoc_path.empty())
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
      g_shell_config->read(preferences, FD_STORE_USER);
      g_shell_config->update_settings_dialog();
      g_shell_config->rebuild_shell_menu();
    }
    g_layout_list.read(preferences, FD_STORE_USER);
    main_window->show(argc,argv);
    toggle_widgetbin_cb(0,0);
    toggle_codeview_cb(0,0);
    if (!c && openlast_button->value() && project_history_abspath[0][0] && autodoc_path.empty()) {
      // Open previous file when no file specified...
      open_project_file(project_history_abspath[0]);
    }
  }
  undo_suspend();
  if (c && !fld::io::read_file(c,0)) {
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
    if (!code_filename_arg.empty()) {
      proj.code_file_set = 1;
      proj.code_file_name = code_filename_arg;
    }
    if (!header_filename_arg.empty()) {
      proj.header_file_set = 1;
      proj.header_file_name = header_filename_arg;
    }
  }

  if (update_file) {            // fluid -u
    fld::io::write_file(c,0);
    if (!compile_file)
      exit(0);
  }

  if (compile_file) {           // fluid -c[s]
    if (compile_strings)
      write_strings_cb(0,0);
    write_code_files();
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
  if (!autodoc_path.empty()) {
    run_autodoc(autodoc_path);
    set_modflag(0, 0);
    exit_cb(0,0);
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

