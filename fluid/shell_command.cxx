//
// FLUID main entry for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

#include "shell_command.h"

#include "fluid.h"
#include "alignment_panel.h"

#include <FL/Fl_Double_Window.H>
#include <FL/fl_message.H>
#include <FL/fl_string_functions.h>

#include <errno.h>

static Fl_Process s_proc;

/// Shell settings in the .fl file
Shell_Settings shell_settings_windows = { };
Shell_Settings shell_settings_linux = { };
Shell_Settings shell_settings_macos = { };

/// Current shell command, stored in .fl file for each platform, and in app prefs
char *g_shell_command = NULL;

/// Save .fl file before running, stored in .fl file for each platform, and in app prefs
int g_shell_save_fl = 1;

/// Save code file before running, stored in .fl file for each platform, and in app prefs
int g_shell_save_code = 1;

/// Save strings file before running, stored in .fl file for each platform, and in app prefs
int g_shell_save_strings = 0;

/// Use these settings from .fl files, stored in app prefs
int g_shell_use_fl_settings = 1;

/**
 Read the default shell settings from the app preferences.
 */
void shell_prefs_get()
{
  fluid_prefs.get("shell_command", g_shell_command, "echo \"Custom Shell Command\"");
  fluid_prefs.get("shell_savefl", g_shell_save_fl, 1);
  fluid_prefs.get("shell_writecode", g_shell_save_code, 1);
  fluid_prefs.get("shell_writemsgs", g_shell_save_strings, 0);
  fluid_prefs.get("shell_use_fl", g_shell_use_fl_settings, 1);
}

/**
 Write the current shell settings to the app preferences.
 */
void shell_prefs_set()
{
  fluid_prefs.set("shell_command", g_shell_command);
  fluid_prefs.set("shell_savefl", g_shell_save_fl);
  fluid_prefs.set("shell_writecode", g_shell_save_code);
  fluid_prefs.set("shell_writemsgs", g_shell_save_strings);
  fluid_prefs.set("shell_use_fl", g_shell_use_fl_settings);
}

/**
 Copy shell settings from the .fl buffer if use_fl_settings is set.
 */
void shell_settings_read()
{
  if (g_shell_use_fl_settings==0)
    return;
#if defined(_WIN32)
  Shell_Settings &shell_settings = shell_settings_windows;
#elif defined(__APPLE__)
  Shell_Settings &shell_settings = shell_settings_macos;
#else
  Shell_Settings &shell_settings = shell_settings_linux;
#endif
  if (g_shell_command)
    free((void*)g_shell_command);
  g_shell_command = NULL;
  if (shell_settings.command)
    g_shell_command = fl_strdup(shell_settings.command);
  g_shell_save_fl = ((shell_settings.flags&1)==1);
  g_shell_save_code = ((shell_settings.flags&2)==2);
  g_shell_save_strings = ((shell_settings.flags&4)==4);
}

/**
 Copy current shell settings to the .fl buffer if use_fl_settings is set.
 */
void shell_settings_write()
{
  if (g_shell_use_fl_settings==0)
    return;
#if defined(_WIN32)
  Shell_Settings &shell_settings = shell_settings_windows;
#elif defined(__APPLE__)
  Shell_Settings &shell_settings = shell_settings_macos;
#else
  Shell_Settings &shell_settings = shell_settings_linux;
#endif
  if (shell_settings.command)
    free((void*)shell_settings.command);
  shell_settings.command = NULL;
  if (g_shell_command)
    shell_settings.command = fl_strdup(g_shell_command);
  shell_settings.flags = 0;
  if (g_shell_save_fl)
    shell_settings.flags |= 1;
  if (g_shell_save_code)
    shell_settings.flags |= 2;
  if (g_shell_save_strings)
    shell_settings.flags |= 4;
}

/** \class Fl_Process
 \todo Explain.
 */

Fl_Process::Fl_Process() {
  _fpt= NULL;
}

Fl_Process::~Fl_Process() {
  if (_fpt) close();
}

// FIXME: popen needs the UTF-8 equivalent fl_popen
// portable open process:
FILE * Fl_Process::popen(const char *cmd, const char *mode) {
#if defined(_WIN32)  && !defined(__CYGWIN__)
  // PRECONDITIONS
  if (!mode || !*mode || (*mode!='r' && *mode!='w') ) return NULL;
  if (_fpt) close(); // close first before reuse

  ptmode = *mode;
  pin[0] = pin[1] = pout[0] = pout[1] = perr[0] = perr[1] = INVALID_HANDLE_VALUE;
  // stderr to stdout wanted ?
  int fusion = (strstr(cmd,"2>&1") !=NULL);

  // Create windows pipes
  if (!createPipe(pin) || !createPipe(pout) || (!fusion && !createPipe(perr) ) )
    return freeHandles(); // error

  // Initialize Startup Info
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb           = sizeof(STARTUPINFO);
  si.dwFlags    = STARTF_USESTDHANDLES;
  si.hStdInput    = pin[0];
  si.hStdOutput   = pout[1];
  si.hStdError  = fusion ? pout[1] : perr [1];

  if ( CreateProcess(NULL, (LPTSTR) cmd,NULL,NULL,TRUE,
                     DETACHED_PROCESS,NULL,NULL, &si, &pi)) {
    // don't need theses handles inherited by child process:
    clean_close(pin[0]); clean_close(pout[1]); clean_close(perr[1]);
    HANDLE & h = *mode == 'r' ? pout[0] : pin[1];
    _fpt = _fdopen(_open_osfhandle((fl_intptr_t) h,_O_BINARY),mode);
    h= INVALID_HANDLE_VALUE;  // reset the handle pointer that is shared
    // with _fpt so we don't free it twice
  }

  if (!_fpt)  freeHandles();
  return _fpt;
#else
  _fpt=::popen(cmd,mode);
  return _fpt;
#endif
}

int Fl_Process::close() {
#if defined(_WIN32)  && !defined(__CYGWIN__)
  if (_fpt) {
    fclose(_fpt);
    clean_close(perr[0]);
    clean_close(pin[1]);
    clean_close(pout[0]);
    _fpt = NULL;
    return 0;
  }
  return -1;
#else
  int ret = ::pclose(_fpt);
  _fpt=NULL;
  return ret;
#endif
}

// non-null if file is open
FILE *Fl_Process::desc() const {
  return _fpt;
}

char *Fl_Process::get_line(char * line, size_t s) const {
  return _fpt ? fgets(line, (int)s, _fpt) : NULL;
}

// returns fileno(FILE*):
// (file must be open, i.e. _fpt must be non-null)
// *FIXME* we should find a better solution for the 'fileno' issue
// non null if file is open
int Fl_Process::get_fileno() const {
#ifdef _MSC_VER
    return _fileno(_fpt); // suppress MSVC warning
#else
    return fileno(_fpt);
#endif
}

#if defined(_WIN32)  && !defined(__CYGWIN__)

bool Fl_Process::createPipe(HANDLE * h, BOOL bInheritHnd) {
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = bInheritHnd;
  return CreatePipe (&h[0],&h[1],&sa,0) ? true : false;
}

FILE *Fl_Process::freeHandles()  {
  clean_close(pin[0]);    clean_close(pin[1]);
  clean_close(pout[0]);   clean_close(pout[1]);
  clean_close(perr[0]);   clean_close(perr[1]);
  return NULL; // convenient for error management
}

void Fl_Process::clean_close(HANDLE& h) {
  if (h!= INVALID_HANDLE_VALUE) CloseHandle(h);
  h = INVALID_HANDLE_VALUE;
}

#endif


// Shell command support...

static bool prepare_shell_command(const char * &command)  { // common pre-shell command code all platforms
  shell_window->hide();
  if (s_proc.desc()) {
    fl_alert("Previous shell command still running!");
    return false;
  }
  if ((command = g_shell_command) == NULL || !*command) {
    fl_alert("No shell command entered!");
    return false;
  }
  if (g_shell_save_fl) {
    save_cb(0, 0);
  }
  if (g_shell_save_code) {
    write_code_files();
  }
  if (g_shell_save_strings) {
    write_strings_cb(0, 0);
  }
  return true;
}

// Support the full piped shell command...
void shell_pipe_cb(FL_SOCKET, void*) {
  char  line[1024]="";          // Line from command output...

  if (s_proc.get_line(line, sizeof(line)) != NULL) {
    // Add the line to the output list...
    shell_run_terminal->append(line);
  } else {
    // End of file; tell the parent...
    Fl::remove_fd(s_proc.get_fileno());
    s_proc.close();
    shell_run_terminal->append("... END SHELL COMMAND ...\n");
  }
}

void do_shell_command(Fl_Return_Button*, void*) {
  const char    *command=NULL;  // Command to run

  if (!prepare_shell_command(command)) return;

  // Show the output window and clear things...
  shell_run_terminal->text("");
  shell_run_terminal->append(command);
  shell_run_terminal->append("\n");
  shell_run_window->label("Shell Command Running...");

  if (s_proc.popen((char *)command) == NULL) {
    fl_alert("Unable to run shell command: %s", strerror(errno));
    return;
  }

  shell_run_button->deactivate();

  Fl_Preferences pos(fluid_prefs, "shell_run_Window_pos");
  int x, y, w, h;
  pos.get("x", x, -1);
  pos.get("y", y, 0);
  pos.get("w", w, 640);
  pos.get("h", h, 480);
  if (x!=-1) {
    shell_run_window->resize(x, y, w, h);
  }
  shell_run_window->show();

  Fl::add_fd(s_proc.get_fileno(), shell_pipe_cb);

  while (s_proc.desc()) Fl::wait();

  shell_run_button->activate();
  shell_run_window->label("Shell Command Complete");
  fl_beep();

  while (shell_run_window->shown()) Fl::wait();
}

/**
 Show a dialog box to run an external shell command.

 Copies the current settings into the dialog box.

 This dialog box offers a field for a command line and three check buttons
 to generate and save various files before the command is run.

 If the fourth checkbox, "use settings in .fl design files" is checked,
 all shell settings will be store in the current .fl file, and they will
 be read and restored when the .fl is loaded again.

 Fluid will save different shell settings for different operating system as
 it is common that a different OS requires a different shell command.

 Fluid comes with default shell settings. Pressing the "save as default" button
 will store the current setting in the Fluid app settings and are used for new
 designs, or if the "use settings..." box is not checked.

 Fluid app settings are saved per user and per machine.
 */
void show_shell_window() {
  update_shell_window();
  shell_window->hotspot(shell_command_input);
  shell_window->show();
}

/**
 Update the shell properties dialog box.
 */
void update_shell_window() {
  shell_command_input->value(g_shell_command);
  shell_savefl_button->value(g_shell_save_fl);
  shell_writecode_button->value(g_shell_save_code);
  shell_writemsgs_button->value(g_shell_save_strings);
  shell_use_fl_button->value(g_shell_use_fl_settings);
}

/**
 Copy the sshe;l settings from the dialog box into the variables.
 */
void apply_shell_window() {
  if (g_shell_command)
    free((void*)g_shell_command);
  g_shell_command = fl_strdup(shell_command_input->value());
  g_shell_save_fl = shell_savefl_button->value();
  g_shell_save_code = shell_writecode_button->value();
  g_shell_save_strings = shell_writemsgs_button->value();
}

