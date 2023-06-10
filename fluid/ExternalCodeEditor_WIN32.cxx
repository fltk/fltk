//
// External code editor management class for Windows
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

//      Note: This entire file Windows only.

#ifdef WIN32

#include <stdio.h>      // snprintf()

#include <FL/Fl.H>      // Fl_Timeout_Handler..
#include <FL/fl_ask.H>  // fl_alert()

#include "ExternalCodeEditor_WIN32.h"

extern int G_debug;     // defined in fluid.cxx

// Static local data
static int L_editors_open = 0;                          // keep track of #editors open
static Fl_Timeout_Handler L_update_timer_cb = 0;        // app's update timer callback

// [Static/Local] Get error message string for last failed WIN32 function.
// Returns a string pointing to static memory.
//
//     TODO: Is more code needed here to convert returned string to utf8? -erco
//
static const char *get_ms_errmsg() {
  static char emsg[1024];
  DWORD lastErr = GetLastError();
  DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_IGNORE_INSERTS  |
                FORMAT_MESSAGE_FROM_SYSTEM;
  LPSTR mbuf = 0;
  DWORD size = FormatMessageA(flags, 0, lastErr, MAKELANGID(LANG_NEUTRAL,
                              SUBLANG_DEFAULT), (LPSTR)&mbuf, 0, NULL);
  if ( size == 0 ) {
    _snprintf(emsg, sizeof(emsg), "Error Code %ld", long(lastErr));
  } else {
    // Copy mbuf -> emsg (with '\r's removed -- they screw up fl_alert())
    for ( char *src=mbuf, *dst=emsg; 1; src++ ) {
      if ( *src == '\0' ) { *dst = '\0'; break; }
      if ( *src != '\r' ) { *dst++ = *src; }
    }
    LocalFree(mbuf);    // Free the buffer allocated by the system
  }
  return emsg;
}

// [Static/Local] See if file exists
static int is_file(const char *filename) {
  DWORD att = GetFileAttributesA(filename);
  if (att == INVALID_FILE_ATTRIBUTES) return 0;
  if ( (att & FILE_ATTRIBUTE_DIRECTORY) == 0 ) return 1;        // not a dir == file
  return 0;
}

// [Static/Local] See if dir exists
static int is_dir(const char *dirname) {
  DWORD att = GetFileAttributesA(dirname);
  if (att == INVALID_FILE_ATTRIBUTES) return 0;
  if (att & FILE_ATTRIBUTE_DIRECTORY) return 1;
  return 0;
}

// CTOR
ExternalCodeEditor::ExternalCodeEditor() {
  memset(&pinfo_, 0, sizeof(pinfo_));
  memset(&file_mtime_, 0, sizeof(file_mtime_));
  memset(&file_size_, 0, sizeof(file_size_));
  filename_ = 0;
}

// DTOR
ExternalCodeEditor::~ExternalCodeEditor() {
  close_editor();   // close editor, delete tmp file
  set_filename(0);  // free()s filename
}

// [Protected] Set the filename. Handles memory allocation/free
//     If set to NULL, frees memory.
//
void ExternalCodeEditor::set_filename(const char *val) {
  if ( filename_ ) free((void*)filename_);
  filename_ = val ? strdup(val) : 0;
}

// [Public] Is editor running?
int ExternalCodeEditor::is_editing() {
  return( (pinfo_.dwProcessId != 0) ? 1 : 0 );
}

// [Static/Local] Terminate_app()'s callback to send WM_CLOSE to a single window. 
static BOOL CALLBACK terminate_app_enum(HWND hwnd, LPARAM lParam) {
  DWORD dwID;
  GetWindowThreadProcessId(hwnd, &dwID);
  if (dwID == (DWORD)lParam) {
    PostMessage(hwnd, WM_CLOSE, 0, 0);
    if ( G_debug )
      printf("terminate_app_enum() sends WIN_CLOSE to hwnd=%p\n", (void*)hwnd);
  }
  return TRUE;
}

// [Static/Local] Handle sending WIN_CLOSE to /all/ windows matching specified pid.
// Wait up to msecTimeout for process to close, and if it doesn't, use TerminateProcess().
//
static int terminate_app(DWORD pid, DWORD msecTimeout) {
  HANDLE hProc = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, FALSE, pid);
  if ( !hProc ) return -1;
  // terminate_app_enum() posts WM_CLOSE to all windows matching pid
  EnumWindows((WNDENUMPROC)terminate_app_enum, (LPARAM) pid);
  // Wait on handle. If it closes, great. If it times out, use TerminateProcess()
  int ret = 0;
  if ( WaitForSingleObject(hProc, msecTimeout) != WAIT_OBJECT_0 ) {
    if ( G_debug ) {
      printf("WARNING: sent WIN_CLOSE, but timeout after %ld msecs.."
             "trying TerminateProcess\n", msecTimeout);
    }
    if ( TerminateProcess(hProc, 0) == 0 ) {
      if ( G_debug ) {
        printf("ERROR: TerminateProcess() for pid=%ld failed: %s\n",
               long(pid), get_ms_errmsg());
      }
      ret = -1;
    } else {
      ret = 0;      // TerminateProcess succeeded
    }
  } else {
    ret = 0;        // WaitForSingleObject() confirmed WIN_CLOSE succeeded
  }
  CloseHandle(hProc);
  return ret;
}

// [Protected] Wait for editor to close
void ExternalCodeEditor::close_editor() {
  if ( G_debug ) printf("close_editor() called: pid=%ld\n", long(pinfo_.dwProcessId));
  // Wait until editor is closed + reaped
  while ( is_editing() ) {
    switch ( reap_editor() ) {
      case -2:  // no editor running (unlikely to happen)
        return;
      case -1:  // error
        fl_alert("Error reaping external editor\n"
                 "pid=%ld file=%s", long(pinfo_.dwProcessId), filename());
        break;
      case 0:   // process still running
	switch ( fl_choice("Please close external editor\npid=%ld file=%s",
			   "Force Close",	// button 0
			   "Closed",		// button 1
			   0,			// button 2
			   long(pinfo_.dwProcessId), filename() ) ) {
	  case 0: 	// Force Close
	    kill_editor();
	    continue;
	  case 1: 	// Closed? try to reap
	    continue;
	}
        break;
      case 1:   // process reaped
        return;
    }
  }
}

// [Protected] Kill the running editor (if any) and cleanup
//   Kills the editor, reaps the process, and removes the tmp file.
//   The dtor calls this to ensure no editors remain running when fluid exits.
//
void ExternalCodeEditor::kill_editor() {
  if ( G_debug )
    printf("kill_editor() called: pid=%ld\n", (long)pinfo_.dwProcessId);
  if ( !is_editing() ) return;
  switch ( terminate_app(pinfo_.dwProcessId, 500) ) {  // kill editor, wait up to 1/2 sec to die
    case -1: { // error
      fl_alert("Can't seem to close editor of file: %s\n"
               "Please close editor and hit OK", filename());
      break;
    }
    case 0: {  // success -- process reaped
      DWORD pid = pinfo_.dwProcessId;     // save pid
      reap_cleanup();			  // clears pinfo_
      if ( G_debug )
        printf("*** kill_editor() REAP pid=%ld #open=%ld\n",
               long(pid), long(L_editors_open));
      break;
    }
  }
  return;
}

// [Public] Handle if file changed since last check, and update records if so.
// Load new data into 'code', which caller must free().
// If 'force' set, forces reload even if file size/time didn't change.
//
// Returns:
//     0 -- file unchanged or not editing
//     1 -- file changed, internal records updated, 'code' has new content
//    -1 -- error getting file info (get_ms_errmsg() has reason)
//
// OPTIONAL TODO:
//   Ignore changes made within the last 2 seconds,
//   to give editor time to fully write out the file.
//
int ExternalCodeEditor::handle_changes(const char **code, int force) {
  code[0] = 0;
  if ( !is_editing() ) return 0;
  // Sigh, have to open file to get file time/size :/
  HANDLE fh = CreateFile(filename(),      // file to read
                         GENERIC_READ,    // reading only
                         FILE_SHARE_READ, // sharing -- allow read share; just getting file size
                         NULL,            // security
                         OPEN_EXISTING,   // create flags -- must exist
                         0,               // misc flags
                         NULL);           // templates
  if ( fh == INVALID_HANDLE_VALUE ) return -1;
  LARGE_INTEGER fsize;
  // Get file size
  if ( GetFileSizeEx(fh, &fsize) == 0 ) {
    DWORD err = GetLastError();
    CloseHandle(fh); 
    SetLastError(err);  // return error from GetFileSizeEx(), not CloseHandle()
    return -1;
  }
  // Get file time
  FILETIME ftCreate, ftAccess, ftWrite;
  if ( GetFileTime(fh, &ftCreate, &ftAccess, &ftWrite) == 0 ) {
    DWORD err = GetLastError();
    CloseHandle(fh); 
    SetLastError(err);  // return error from GetFileTime(), not CloseHandle()
    return -1;
  }
  // OK, now see if file changed; update records if so
  int changed = 0;
  if ( fsize.QuadPart != file_size_.QuadPart )
    { changed = 1; file_size_ = fsize; }
  if ( CompareFileTime(&ftWrite, &file_mtime_) != 0 )
    { changed = 1; file_mtime_ = ftWrite; }
  // Changes? Load file. Be sure to fallthru to CloseHandle()
  int ret = 0;
  if ( changed || force ) {
    char *buf = (char*)malloc(fsize.QuadPart + 1);
    DWORD count;
    if ( ReadFile(fh, buf, fsize.QuadPart, &count, 0) == 0 ) {
      fl_alert("ERROR: ReadFile() failed for %s: %s",
               filename(), get_ms_errmsg());
      free((void*)buf); buf = 0;
      ret = -1;      // fallthru to CloseHandle()
    } else if ( count != fsize.QuadPart ) {
      fl_alert("ERROR: ReadFile() failed for %s:\n"
               "expected %ld bytes, got %ld",
               filename(), long(fsize.QuadPart), long(count));
      free((void*)buf); buf = 0;
      ret = -1;      // fallthru to CloseHandle()
    } else {
      // Successfully read changed file
      buf[count] = '\0';
      code[0] = buf; // return pointer to allocated buffer
      ret = 1;       // fallthru to CloseHandle()
    }
  }
  CloseHandle(fh);
  return ret;
}

// [Public] Remove the tmp file (if it exists), and zero out filename/mtime/size
// Returns:
//    -1 -- on error (dialog is posted as to why)
//     0 -- no file to remove
//     1 -- file was removed
//
int ExternalCodeEditor::remove_tmpfile() {
  const char *tmpfile = filename();
  if ( G_debug ) printf("remove_tmpfile() '%s'\n", tmpfile ? tmpfile : "(empty)");
  if ( !tmpfile ) return 0;
  // Filename set? remove (if exists) and zero filename/mtime/size
  if ( is_file(tmpfile) ) {
    if ( G_debug ) printf("Removing tmpfile '%s'\n", tmpfile);
    if ( DeleteFile(tmpfile) == 0 ) {
      fl_alert("WARNING: Can't DeleteFile() '%s': %s", tmpfile, get_ms_errmsg());
      return -1;
    }
  } else {
    if ( G_debug ) printf("remove_tmpfile(): is_file(%s) failed\n", tmpfile);
  }
  set_filename(0);
  memset(&file_mtime_, 0, sizeof(file_mtime_));
  memset(&file_size_, 0, sizeof(file_size_));
  return 1;
}

// [Static/Public] Return tmpdir name for this fluid instance.
//     Returns pointer to static memory.
//
const char* ExternalCodeEditor::tmpdir_name() {
  char tempdir[100];
  if (GetTempPath(sizeof(tempdir), tempdir) == 0 ) {
    strcpy(tempdir, "c:\\windows\\temp");      // fallback
  }
  static char dirname[100];
  _snprintf(dirname, sizeof(dirname), "%s.fluid-%ld",
    tempdir, (long)GetCurrentProcessId());
  if ( G_debug ) printf("tmpdir_name(): '%s'\n", dirname);
  return dirname;
}

// [Static/Public] Clear the external editor's tempdir
//    Static so that the main program can call it on exit to clean up.
//
void ExternalCodeEditor::tmpdir_clear() {
  const char *tmpdir = tmpdir_name();
  if ( is_dir(tmpdir) ) {
    if ( G_debug ) printf("Removing tmpdir '%s'\n", tmpdir);
    if ( RemoveDirectory(tmpdir) == 0 ) {
      fl_alert("WARNING: Can't RemoveDirectory() '%s': %s",
               tmpdir, get_ms_errmsg());
    }
  }
}

// [Protected] Creates temp dir (if doesn't exist) and returns the dirname
// as a static string. Returns NULL on error, dialog shows reason.
//
const char* ExternalCodeEditor::create_tmpdir() {
  const char *dirname = tmpdir_name();
  if ( ! is_dir(dirname) ) {
    if ( CreateDirectory(dirname,0) == 0 ) {
      fl_alert("can't create directory '%s': %s",
        dirname, get_ms_errmsg());
      return NULL;
    }
  }
  return dirname;
}

// [Protected] Returns temp filename in static buffer.
//    Returns NULL if can't, posts dialog explaining why.
//
const char* ExternalCodeEditor::tmp_filename() {
  static char path[512];
  const char *tmpdir = create_tmpdir();
  if ( !tmpdir ) return 0;
  extern const char *code_file_name;    // fluid's global
  const char *ext  = code_file_name;    // e.g. ".cxx"
  _snprintf(path, sizeof(path), "%s\\%p%s", tmpdir, (void*)this, ext);
  path[sizeof(path)-1] = 0;
  return path;
}

// [Static/Local] Save string 'code' to 'filename', returning file's mtime/size
// 'code' can be NULL -- writes an empty file if so.
// Returns:
//    0 on success
//   -1 on error (posts dialog with reason)
//
static int save_file(const char *filename,
                     const char *code,
                     FILETIME &file_mtime,        // return these since in win32 it's..
                     LARGE_INTEGER &file_size) {  // ..efficient to get while file open
  if ( code == 0 ) code = "";   // NULL? write an empty file
  memset(&file_mtime, 0, sizeof(file_mtime));
  memset(&file_size, 0, sizeof(file_size));
  HANDLE fh = CreateFile(filename,                // filename
                         GENERIC_WRITE,           // write only
                         0,                       // sharing -- no share during write
                         NULL,                    // security
                         CREATE_ALWAYS,           // create flags -- recreate
                         FILE_ATTRIBUTE_NORMAL,   // misc flags
                         NULL);                   // templates
  if ( fh == INVALID_HANDLE_VALUE ) {
    fl_alert("ERROR: couldn't create file '%s': %s",
             filename, get_ms_errmsg());
    return(-1);
  }
  // Write the file, being careful to CloseHandle() even on errs
  DWORD clen = strlen(code);
  DWORD count = 0;
  int ret = 0;
  if ( WriteFile(fh, code, clen, &count, NULL) == 0 ) {
    fl_alert("ERROR: WriteFile() '%s': %s", filename, get_ms_errmsg());
    ret = -1; // fallthru to CloseHandle()
  } else if ( count != clen ) {
    fl_alert("ERROR: WriteFile() '%s': wrote only %lu bytes, expected %lu",
             filename, (unsigned long)count, (unsigned long)clen);
    ret = -1; // fallthru to CloseHandle()
  }
  // Get mtime/size before closing
  {
    FILETIME ftCreate, ftAccess, ftWrite;
    if ( GetFileSizeEx(fh, &file_size) == 0 ) {
      fl_alert("ERROR: save_file(%s): GetFileSizeEx() failed: %s\n",
               filename, get_ms_errmsg());
    }
    if ( GetFileTime(fh, &ftCreate, &ftAccess, &ftWrite) == 0 ) {
      fl_alert("ERROR: save_file(%s): GetFileTime() failed: %s\n",
               filename, get_ms_errmsg());
    }
    file_mtime = ftWrite;
  }
  // Close, done
  CloseHandle(fh);
  return(ret);
}

// [Protected] Start editor
// Returns:
//    >  0 on success, leaves editor child process running as 'pinfo_'
//    > -1 on error, posts dialog with reason (child exits)
//
int ExternalCodeEditor::start_editor(const char *editor_cmd,
                                     const char *filename) {
  if ( G_debug ) printf("start_editor() cmd='%s', filename='%s'\n",
                        editor_cmd, filename);
  // Startup info
  STARTUPINFO sinfo;
  memset(&sinfo, 0, sizeof(sinfo));
  sinfo.cb          = sizeof(sinfo);
  sinfo.dwFlags     = 0;
  sinfo.wShowWindow = 0;
  // Process info
  memset(&pinfo_, 0, sizeof(pinfo_));
  // Command
  char cmd[1024];
  _snprintf(cmd, sizeof(cmd), "%s %s", editor_cmd, filename);
  // Start editor process
  if (CreateProcess(NULL,               // app name
                    (char*)cmd,         // command to exec
                    NULL,               // secure attribs
                    NULL,               // thread secure attribs
                    FALSE,              // handle inheritance
                    0,                  // creation flags
                    NULL,               // environ block
                    NULL,               // current dir
                    &sinfo,             // startup info
                    &pinfo_) == 0 ) {   // process info
    fl_alert("CreateProcess() failed to start '%s': %s",
             cmd, get_ms_errmsg());
    return(-1);
  }
  if ( L_editors_open++ == 0 )  // first editor? start timers
    { start_update_timer(); }
  if ( G_debug )
    printf("--- EDITOR STARTED: pid_=%ld #open=%d\n",
           (long)pinfo_.dwProcessId, L_editors_open);
  return 0;
}

// [Protected] Cleanup after editor reaped:
//    > Remove tmpfile, zeroes mtime/size/filename
//    > Close process handles
//    > Zeroes out pinfo_
//    > Decrease editor count
//
void ExternalCodeEditor::reap_cleanup() {
  remove_tmpfile();                    // also zeroes mtime/size/filename
  CloseHandle(pinfo_.hProcess);        // close process handle
  CloseHandle(pinfo_.hThread);         // close thread handle
  memset(&pinfo_, 0, sizeof(pinfo_));  // clear pinfo_
  if ( --L_editors_open <= 0 )
    { stop_update_timer(); }
}

// [Public] Try to reap external editor process
// If 'pid_reaped' not NULL, returns PID of reaped editor.
// Returns:
//   -2 -- editor not open
//   -1 -- WaitForSingleObject() failed (get_ms_errmsg() has reason)
//    0 -- process still running
//    1 -- process finished + reaped ('pid_reaped' has pid), pinfo_ set to 0.
//         Handles removing tmpfile/zeroing file_mtime/file_size/filename
//
// If return value <=0, 'pid_reaped' is set to zero.
//
int ExternalCodeEditor::reap_editor(DWORD *pid_reaped) {
  if ( pid_reaped ) *pid_reaped = 0;
  if ( !is_editing() ) return -2;
  DWORD msecs_wait = 50;   // .05 sec
  switch ( WaitForSingleObject(pinfo_.hProcess, msecs_wait) ) {
    case WAIT_TIMEOUT: {   // process didn't reap, still running
      return 0;
    }
    case WAIT_OBJECT_0: {  // reaped
      DWORD wpid = pinfo_.dwProcessId;      // save pid
      reap_cleanup();			    // clears pinfo_
      if ( pid_reaped ) *pid_reaped = wpid; // return pid to caller
      if ( G_debug ) printf("*** EDITOR REAPED: pid=%ld #open=%d\n",
                            long(wpid), L_editors_open);
      return 1;
    }
    case WAIT_FAILED: {    // failed
      return -1;
    }
  }
  return -1;               // any other return unexpected
}

// [Public] Open external editor using 'editor_cmd' to edit 'code'
// 'code' contains multiline code to be edited as a temp file.
//
// Returns:
//   0 if succeeds
//  -1 if can't open editor (already open, etc),
//     errors were shown to user in a dialog
//
int ExternalCodeEditor::open_editor(const char *editor_cmd,
                                    const char *code) {
  // Make sure a temp filename exists
  if ( !filename() ) {
    set_filename(tmp_filename());
    if ( !filename() ) return -1;
  }
  // See if tmpfile already exists or editor already open
  if ( is_file(filename()) ) {
    if ( is_editing() ) {
      // See if editor recently closed but not reaped; try to reap
      DWORD wpid;
      switch ( reap_editor(&wpid) ) {
        case -2:	// no editor running (unlikely to happen)
	  break;
        case -1:        // wait failed
          fl_alert("ERROR: WaitForSingleObject() failed: %s\nfile='%s', pid=%ld",
            get_ms_errmsg(), filename(), long(pinfo_.dwProcessId));
          return -1;
        case 0:         // process still running
          fl_alert("Editor Already Open\n  file='%s'\n  pid=%ld",
            filename(), long(pinfo_.dwProcessId));
          return 0;
        case 1:         // process reaped, wpid is pid reaped
          if ( G_debug )
            printf("*** REAPED EXTERNAL EDITOR: PID %ld\n", long(wpid));
          break;        // fall thru to open new editor instance
      }
      // Reinstate tmp filename (reap_editor() clears it)
      set_filename(tmp_filename());
    }
  }
  // Save code to tmpfile, getting mtime/size
  if ( save_file(filename(), code, file_mtime_, file_size_) < 0 ) {
    return -1;  // errors were shown in dialog
  }
  if ( start_editor(editor_cmd, filename()) < 0 ) { // open file in external editor
    if ( G_debug ) printf("Editor failed to start\n");
    return -1;  // errors were shown in dialog
  }
  // New editor opened -- start update timer (if not already)
  if ( L_update_timer_cb && !Fl::has_timeout(L_update_timer_cb) ) {
    if ( G_debug ) printf("--- Editor opened: STARTING UPDATE TIMER\n");
    Fl::add_timeout(2.0, L_update_timer_cb);
  }
  return 0;
}

// [Public/Static] Start update timer
void ExternalCodeEditor::start_update_timer() {
  if ( !L_update_timer_cb ) return;
  if ( G_debug ) printf("--- TIMER: STARTING UPDATES\n");
  Fl::add_timeout(2.0, L_update_timer_cb);
}

// [Public/Static] Stop update timer
void ExternalCodeEditor::stop_update_timer() {
  if ( !L_update_timer_cb ) return;
  if ( G_debug ) printf("--- TIMER: STOPPING UPDATES\n");
  Fl::remove_timeout(L_update_timer_cb);
}

// [Public/Static] Set app's external editor update timer callback
//   This is the app's callback callback we start while editors are open,
//   and stop when all editors are closed.
//
void ExternalCodeEditor::set_update_timer_callback(Fl_Timeout_Handler cb) {
  L_update_timer_cb = cb;
}

// [Static/Public] See if any external editors are open.
//   App's timer cb can see if any editors need checking..
//
int ExternalCodeEditor::editors_open() {
  return L_editors_open;
}

#endif /* WIN32 */
//
// End of "$Id$".
//
