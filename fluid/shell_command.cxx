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

// FLUID comes with default shell commands to build the current project file
// and run the project. This is accomplished by calling `fltk-config` on the
// files generated by FLUID, and by calling the executable directly.
//
// If the user wants more complex commands, he can add or modify them in the
// "Shell" settings panel. Modified shell commands are saved with the .fl
// file.

// The Shell panel has a list of shell commands in the upper half. Under the
// list are buttons to add, duplicate, and delete shell commands. A harder to
// reach button can be used to reset defaults. We may want to add up and down
// buttons, so the user can change the order of commands.

// Selecting any shell command in the list fills in and activates a list of
// options in the lower half of the panel. Those settings are:
//  - Name: the name of the shell command in the list
//  - Label: the label in the pulldown menu (could be the same as name?)
//  - Shortcut: shortcut key to launch the command
//  - Condition: pulldown menu to make the entry conditional for various
//    target platforms, for example, a "Windows only" entry would only be added
//    to the Shell menu on a Windows machine. Other options could be:
//     - Linux only, macOS only, never (to make a list header!?), inactive?
//  - Command: a multiline input for the actual shell command
//  - Variables: a pulldown menu that insert variable names like $<sourcefile>
//  - options to save project, code, and strings before running
//  - test-run button

// I don't think we need a way to change the FLUID defaults. Merging a different
// .fl file with different shell commands could (should) simply replace the
// current shell commands? Or two buttons to save and load shell commands?

// TODO: text module insert and replacement
// TODO: popup text editor
// TODO: action on new project (revert, load, merge)
// TODO: import export?
// TODO: refactor namespace use
// TODO: hostname, username, getenv support?
// TODO: add ownership to item, as in layout
//         FLUID presets, user, project file, external file, folder
// TODO: make settings dialog resizable
// TODO: get a macro to find `fltk-config`

#include "shell_command.h"

#include "fluid.h"
#include "file.h"
#include "alignment_panel.h"

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_message.H>
#include <FL/fl_string_functions.h>

#include <errno.h>

static Fl_Process s_proc;


/** \class Fl_Process
 Launch an external shell command.
 */

/**
 Create a process manager
 */
Fl_Process::Fl_Process() {
  _fpt= NULL;
}

/**
 Destroy the project manager.
 */
Fl_Process::~Fl_Process() {
  // TODO: check what we need to do if a task is still running
  if (_fpt) close();
}

/**
 Open a process.

 \param[in] cmd the shell command that we want to run
 \param[in] mode "r" or "w" for creating a stream that can read or write
 \return a stream that is redirected from the shell command stdout
 */
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

/**
 Close the current process.
 */
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

/**
 non-null if file is open.

 \return the current file descriptor of the process' stdout
 */
FILE *Fl_Process::desc() const {
  return _fpt;
}

/**
 Receive a single line from the current process.

 \param[out] line buffer to receive the line
 \param[in] s size of the provided buffer
 \return NULL if an orror occured, otherwise a pointer to the string
 */
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

/**
 Prepare FLUID for running a shell command according to the command flags.

 \param[in] flags set various flags to save the project, code, and string before running the command
 \return false if the previous command is still running
 */
static bool prepare_shell_command(int flags)  {
//  settings_window->hide();
  if (s_proc.desc()) {
    fl_alert("Previous shell command still running!");
    return false;
  }
  if (flags & Fd_Shell_Command::SAVE_PROJECT) {
    save_cb(0, 0);
  }
  if (flags & Fd_Shell_Command::SAVE_SOURCECODE) {
    write_code_files();
  }
  if (flags & Fd_Shell_Command::SAVE_STRINGS) {
    write_strings_cb(0, 0);
  }
  return true;
}

/**
 Called by the file handler when the command is finished.
 */
void shell_proc_done() {
  shell_run_terminal->append("... END SHELL COMMAND ...\n");
  shell_run_button->activate();
  shell_run_window->label("FLUID Shell");
  fl_beep();
}

void shell_timer_cb(void*) {
  if (!s_proc.desc()) {
    shell_proc_done();
  } else {
    Fl::add_timeout(0.25, shell_timer_cb);
  }
}

// Support the full piped shell command...
void shell_pipe_cb(FL_SOCKET, void*) {
  char  line[1024]="";          // Line from command output...

  if (s_proc.get_line(line, sizeof(line)) != NULL) {
    // Add the line to the output list...
    shell_run_terminal->append(line);
  } else {
    // End of file; tell the parent...
    Fl::remove_timeout(shell_timer_cb);
    Fl::remove_fd(s_proc.get_fileno());
    s_proc.close();
    shell_proc_done();
  }
}

static void expand_macro(Fl_String &cmd, const Fl_String &macro, const Fl_String &content) {
  for (int i=0;;) {
    i = cmd.find(macro, i);
    if (i==Fl_String::npos) break;
    cmd.replace(i, macro.size(), content);
  }
}

static void expand_macros(Fl_String &cmd) {
  expand_macro(cmd, "@BASENAME@",         g_project.get_basename());
  expand_macro(cmd, "@PROJECTFILE_PATH@", g_project.get_projectfile_path());
  expand_macro(cmd, "@PROJECTFILE_NAME@", g_project.get_projectfile_name());
  expand_macro(cmd, "@CODEFILE_PATH@",    g_project.get_codefile_path());
  expand_macro(cmd, "@CODEFILE_NAME@",    g_project.get_codefile_name());
  expand_macro(cmd, "@HEADERFILE_PATH@",  g_project.get_headerfile_path());
  expand_macro(cmd, "@HEADERFILE_NAME@",  g_project.get_headerfile_name());
  expand_macro(cmd, "@TEXTFILE_PATH@",    g_project.get_textfile_path());
  expand_macro(cmd, "@TEXTFILE_NAME@",    g_project.get_textfile_name());
}

/**
 Prepare for and run a shell command.

 \param[in] cmd the command that is sent to `/bin/sh -c ...` or `cmd.exe` on Windows machines
 \param[in] flags various flags in preparation of the command
 */
void run_shell_command(const Fl_String &cmd, int flags) {
  if (cmd.empty()) {
    fl_alert("No shell command entered!");
    return;
  }

  if (!prepare_shell_command(flags)) return;

  Fl_String expanded_cmd = cmd;
  expand_macros(expanded_cmd);

  if (!shell_run_window->visible()) {
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
  }

  // Show the output window and clear things...
  shell_run_terminal->printf("\033[0;32m%s\033[0m\n", expanded_cmd.c_str());
  shell_run_window->label(expanded_cmd.c_str());

  if (s_proc.popen((char *)expanded_cmd.c_str()) == NULL) {
    shell_run_terminal->printf("\033[1;31mUnable to run shell command: %s\033[0m\n",
                               strerror(errno));
    shell_run_window->label("FLUID Shell");
    return;
  }
  shell_run_button->deactivate();

  // if the function below does not for some reason, we will check periodically
  // to see if the command is done
  Fl::add_timeout(0.25, shell_timer_cb);
  // this will tell us when the shell command is done
  Fl::add_fd(s_proc.get_fileno(), shell_pipe_cb);
}

// name       all: build
// label      Build...
// shortcut   Cmd-B
// condition  always
// condition_data; NUL
// command    fltk-config --compile <<codefile>>
// flags      save_project | save_code | save_strings

/**
 Create an empty shell command structure.
 */
Fd_Shell_Command::Fd_Shell_Command()
: shortcut(0),
  condition(0),
  flags(0),
  shell_menu_item_(NULL)
{
}

/**
 Copy the aspects of a shell command dataset into a new shell command.

 \param[in] rhs copy from this prototype
 */
Fd_Shell_Command::Fd_Shell_Command(const Fd_Shell_Command *rhs)
: name(rhs->name),
  label(rhs->label),
  shortcut(rhs->shortcut),
  condition(rhs->condition),
  condition_data(rhs->condition_data),
  command(rhs->command),
  flags(rhs->flags),
  shell_menu_item_(NULL)
{
}

/**
 Create a default storage for a shell command and how it is accessible in FLUID.

 \param[in] name is used as a stand-in for the command name and label
 */
Fd_Shell_Command::Fd_Shell_Command(const Fl_String &in_name)
: name(in_name),
  label(in_name),
  shortcut(0),
  condition(Fd_Shell_Command::ALWAYS),
  command("echo \"Hello, FLUID!\""),
  flags(Fd_Shell_Command::SAVE_ALL),
  shell_menu_item_(NULL)
{
}

/**
 Create a default storage for a shell command and how it is accessible in FLUID.

 \param[in] in_name name of this command in the command list in the settings panel
 \param[in] in_label label text in the main pulldown menu
 \param[in] in_shortcut a keyboard shortcut that will also appear in the main menu
 \param[in] in_condition commands can be hidden for certain platforms by setting a condition
 \param[in] in_condition_data more details for future conditions, i.e. per user, per host, etc.
 \param[in] in_command the shell command that we want to run
 \param[in] in_flags some flags to tell FLUID to save the project, code, or strings before running the command
 */
Fd_Shell_Command::Fd_Shell_Command(const Fl_String &in_name,
                 const Fl_String &in_label,
                 Fl_Shortcut in_shortcut,
                 int in_condition,
                 const Fl_String &in_condition_data,
                 const Fl_String &in_command,
                 int in_flags)
: name(in_name),
  label(in_label),
  shortcut(in_shortcut),
  condition(in_condition),
  condition_data(in_condition_data),
  command(in_command),
  flags(in_flags),
  shell_menu_item_(NULL)
{
}

/**
 Run this command now.

 Will open the Shell Panel and execute the command if no other command is
 currently running.
 */
void Fd_Shell_Command::run() {
  if (!command.empty())
    run_shell_command(command, flags);
}

/**
 Update the shell submenu in main menu with the shortcut and a copy of the label.
 */
void Fd_Shell_Command::update_shell_menu() {
  if (shell_menu_item_) {
    const char *old_label = shell_menu_item_->label();  // can be NULL
    const char *new_label = label.c_str();              // never NULL
    if (!old_label || (old_label && strcmp(old_label, new_label))) {
      if (old_label) ::free((void*)old_label);
      shell_menu_item_->label(fl_strdup(new_label));
    }
    shell_menu_item_->shortcut(shortcut);
  }
}

/**
 Check if the set condition is met.

 \return true if this command appears in the main menu
 */
bool Fd_Shell_Command::is_active() {
  switch (condition) {
    case ALWAYS: return true;
    case NEVER: return false;
#ifdef _WIN32
    case MAC_ONLY: return false;
    case UX_ONLY: return false;
    case WIN_ONLY: return true;
    case MAC_AND_UX_ONLY: return false;
#elif defined(__APPLE__)
    case MAC_ONLY: return true;
    case UX_ONLY: return false;
    case WIN_ONLY: return false;
    case MAC_AND_UX_ONLY: return true;
#else
    case MAC_ONLY: return false;
    case UX_ONLY: return true;
    case WIN_ONLY: return false;
    case MAC_AND_UX_ONLY: return true;
#endif
    case USER_ONLY: return false; // TODO: get user name
    case HOST_ONLY: return false; // TODO: get host name
    case ENV_ONLY: {
      const char *value = fl_getenv(condition_data.c_str());
      if (value && *value) return true;
      return false;
    }
  }
  return false;
}

void Fd_Shell_Command::read(class Fd_Project_Reader *in) {
  const char *c = in->read_word(1);
  if (strcmp(c, "{")!=0) return; // expecting start of group
  for (;;) {
    c = in->read_word(1);
    if (strcmp(c, "}")==0) break; // end of command list
    else if (strcmp(c, "name")==0)
      name = in->read_word();
    else if (strcmp(c, "label")==0)
      label = in->read_word();
    else if (strcmp(c, "shortcut")==0)
      shortcut = in->read_int();
    else if (strcmp(c, "condition")==0)
      condition = in->read_int();
    else if (strcmp(c, "condition_data")==0)
      condition_data = in->read_word();
    else if (strcmp(c, "command")==0)
      command = in->read_word();
    else if (strcmp(c, "flags")==0)
      flags = in->read_int();
    else
      in->read_word(); // skip an unknown word
  }
}

void Fd_Shell_Command::write(class Fd_Project_Writer *out) {
  out->write_string("\n  command {");
  out->write_string("\n    name "); out->write_word(name.c_str());
  out->write_string("\n    label "); out->write_word(label.c_str());
  if (shortcut) out->write_string("\n    shortcut %d", shortcut);
  if (condition) out->write_string("\n    condition %d", condition);
  if (!condition_data.empty()) {
    out->write_string("\n    condition_data "); out->write_word(condition_data.c_str());
  }
  if (!command.empty()) {
    out->write_string("\n    command "); out->write_word(command.c_str());
  }
  if (flags) out->write_string("\n    flags %d", flags);
  out->write_string("\n  }");
}


/**
 Manage a list of shell commands and their parameters.
 */
Fd_Shell_Command_List::Fd_Shell_Command_List()
: is_default_(false),
  list(NULL),
  list_size(0),
  list_capacity(0),
  shell_menu_(NULL)
{
}

/**
 Release all shell commands and destroy this class.
 */
Fd_Shell_Command_List::~Fd_Shell_Command_List() {
  clear();
}

/**
 Return the shell command at the given index.

 \param[in] index must be between 0 and list_size-1
 \return a pointer to the shell command data
 */
Fd_Shell_Command *Fd_Shell_Command_List::at(int index) const {
  return list[index];
}

/**
 Clear all current shell commands and reset the FLUID defaults.
 */
void Fd_Shell_Command_List::restore_defaults() {
  clear();
  add(new Fd_Shell_Command("all: build", "Build...", FL_COMMAND+'b', Fd_Shell_Command::ALWAYS, NULL,
                           "fltk-config --compile @BASENAME@.cxx",
                           Fd_Shell_Command::SAVE_PROJECT|Fd_Shell_Command::SAVE_SOURCECODE));
  add(new Fd_Shell_Command("all: run", "Run...", FL_COMMAND+'r', Fd_Shell_Command::ALWAYS, NULL,
                           "fltk-config --compile @BASENAME@.cxx\n"
                           "./@BASENAME@",
                           Fd_Shell_Command::SAVE_PROJECT|Fd_Shell_Command::SAVE_SOURCECODE));
  is_default_ = true;
}

/**
 Clear all shell commands.
 */
void Fd_Shell_Command_List::clear() {
  if (list) {
    for (int i=0; i<list_size; i++) {
      delete list[i];
    }
    ::free(list);
    list_size = 0;
    list_capacity = 0;
    list = 0;
  }
}

/**
 Read shell configuration from a project file.
 */
void Fd_Shell_Command_List::read(Fd_Project_Reader *in) {
  const char *c = in->read_word(1);
  if (strcmp(c, "{")!=0) return; // expecting start of group
  clear();
  is_default_list(false);
  for (;;) {
    c = in->read_word(1);
    if (strcmp(c, "}")==0) break; // end of command list
    else if (strcmp(c, "command")==0) {
      Fd_Shell_Command *cmd = new Fd_Shell_Command();
      add(cmd);
      cmd->read(in);
    } else {
      in->read_word(); // skip an unknown group
    }
  }
}

/**
 Write shell configuration to a project file if it isn;t the FLUID default.
 */
void Fd_Shell_Command_List::write(Fd_Project_Writer *out) {
  if (!is_default_) {
    out->write_string("\nshell_commands {");
    for (int i=0; i<list_size; i++) {
      list[i]->write(out);
    }
    out->write_string("\n}");
  }
}

/**
 Add a previously created shell command to the end of the list.

 \param[in] cmd a pointer to the command that we want to add
 */
void Fd_Shell_Command_List::add(Fd_Shell_Command *cmd) {
  if (list_size == list_capacity) {
    list_capacity += 16;
    list = (Fd_Shell_Command**)::realloc(list, list_capacity * sizeof(Fd_Shell_Command**));
  }
  list[list_size++] = cmd;
}

/**
 Insert a newly created shell command at the given position in the list.

 \param[in] index must be between 0 and list_size-1
 \param[in] cmd a pointer to the command that we want to add
 */
void Fd_Shell_Command_List::insert(int index, Fd_Shell_Command *cmd) {
  if (list_size == list_capacity) {
    list_capacity += 16;
    list = (Fd_Shell_Command**)::realloc(list, list_capacity * sizeof(Fd_Shell_Command**));
  }
  ::memmove(list+index+1, list+index, (list_size-index)*sizeof(Fd_Shell_Command**));
  list_size++;
  list[index] = cmd;
}

/**
 Remove and delete the command at the given index.

 \param[in] index must be between 0 and list_size-1
 */
void Fd_Shell_Command_List::remove(int index) {
  delete list[index];
  list_size--;
  ::memmove(list+index, list+index+1, (list_size-index)*sizeof(Fd_Shell_Command**));
}

/**
 Used to find the shell submenu within the main menu tree.
 */
void shell_submenu_marker(Fl_Widget*, void*) {
  // intentionally left empty
}

/**
 This is called whenever the user clicks a shell command menu in the main menu.

 \param[in] u cast tp long to get the index of the shell command
 */
void menu_shell_cmd_cb(Fl_Widget*, void *u) {
  long index = (long)(fl_intptr_t)u;
  g_shell_config->list[index]->run();
}

/**
 This is called when the user selects the menu to edit the shell commands.
 It pops up the setting panel at the shell settings tab.
 */
void menu_shell_customize_cb(Fl_Widget*, void*) {
  settings_window->show();
  w_settings_tabs->value(w_settings_shell_tab);
}

/**
 Rebuild the entire shell submenu from scratch and replace the old menu.
 */
void Fd_Shell_Command_List::rebuild_shell_menu() {
  static Fl_Menu_Item *shell_submenu = NULL;
  if (!shell_submenu)
    shell_submenu = (Fl_Menu_Item*)main_menubar->find_item(shell_submenu_marker);

  int i, j, num_active_items = 0;
  // count the active commands
  for (i=0; i<list_size; i++) {
    if (list[i]->is_active()) num_active_items++;
  }
  // allocate a menu item array
  Fl_Menu_Item *m = (Fl_Menu_Item*)::calloc(num_active_items+2, sizeof(Fl_Menu_Item));
  // set the menu item pointer for all active commands
  for (i=j=0; i<list_size; i++) {
    Fd_Shell_Command *cmd = list[i];
    if (cmd->is_active()) {
      cmd->shell_menu_item_ = m + i;
      m[i].callback(menu_shell_cmd_cb);
      m[i].argument(i);
      cmd->update_shell_menu();
      j++;
    }
  }
  if (i>0) m[i-1].flags |= FL_MENU_DIVIDER;
  m[i].label(fl_strdup("Customize..."));
  m[i].callback(menu_shell_customize_cb);
  // replace the old menu array with the new one
  Fl_Menu_Item *m_old = shell_menu_;
  shell_menu_ = m;
  shell_submenu->user_data(shell_menu_);
  // free all resources from the old menu
  if (m_old) {
    for (i=0; ; i++) {
      const char *label = m_old[i].label();
      if (!label) break;
      ::free((void*)label);
    }
    ::free(m_old);
  }
}

/**
 Tell the settings dialog to query this list and update its GUI elements.
 */
void Fd_Shell_Command_List::update_settings_dialog() {
  if (w_settings_shell_tab)
    w_settings_shell_tab->do_callback(w_settings_shell_tab, LOAD);
}

/**
 Some settings changed, so tell the manager that this is no longer the FLUID default list.

 \param[in] v new value
 */
void Fd_Shell_Command_List::is_default_list(bool v) {
  if (v != is_default_) {
    is_default_ = v;
    if (w_settings_shell_tab)
      w_settings_shell_default->do_callback(w_settings_shell_default, LOAD);
  }
  set_modflag(1);
}

/**
 The default shell submenu in batch mode.
 */
Fl_Menu_Item default_shell_menu[] = {
  {   "Customize...", 0, menu_shell_customize_cb },
  { NULL }
};

/**
 A pointer to the list of shell commands if we are not in batch mode.
 */
Fd_Shell_Command_List *g_shell_config = NULL;

