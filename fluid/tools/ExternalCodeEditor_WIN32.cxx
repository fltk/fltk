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

#include "tools/ExternalCodeEditor_WIN32.h"
#include "Fluid.h"
#include "Project.h"

#include <FL/Fl.H>      // Fl_Timeout_Handler..
#include <FL/fl_ask.H>  // fl_alert()
#include <FL/fl_utf8.h> // fl_utf8fromwc()
#include <FL/fl_string_functions.h> // fl_strdup()

#include <stdio.h>      // snprintf()
#include <stdlib.h>

using namespace fld;

// Static local data
static int L_editors_open = 0;                          // keep track of #editors open
static Fl_Timeout_Handler L_update_timer_cb = 0;        // app's update timer callback
static wchar_t *wbuf = nullptr;
static char *abuf = nullptr;

static wchar_t *utf8_to_wchar(const char *utf8, wchar_t *&wbuf, int lg = -1) {
  unsigned len = (lg >= 0) ? (unsigned)lg : (unsigned)strlen(utf8);
  unsigned wn = fl_utf8toUtf16(utf8, len, nullptr, 0) + 1; // Query length
  wbuf = (wchar_t *)realloc(wbuf, sizeof(wchar_t) * wn);
  wn = fl_utf8toUtf16(utf8, len, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return wbuf;
}

static char *wchar_to_utf8(const wchar_t *wstr, char *&utf8) {
  unsigned len = (unsigned)wcslen(wstr);
  unsigned wn = fl_utf8fromwc(nullptr, 0, wstr, len) + 1; // query length
  utf8 = (char *)realloc(utf8, wn);
  wn = fl_utf8fromwc(utf8, wn, wstr, len); // convert string
  utf8[wn] = 0;
  return utf8;
}

// [Static/Local] Get error message string for last failed WIN32 function.
// Returns a string pointing to static memory.
//
static const char *get_ms_errmsg() {
  static char emsg[1024];
  DWORD lastErr = GetLastError();
  DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_IGNORE_INSERTS  |
                FORMAT_MESSAGE_FROM_SYSTEM;
  DWORD langid = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
  LPWSTR mbuf = 0;
  DWORD msize = 0;

  // Get error message from Windows
  msize = FormatMessageW(flags, 0, lastErr, langid, (LPWSTR)&mbuf, 0, nullptr);
  if ( msize == 0 ) {
    _snprintf(emsg, sizeof(emsg), "Error #%ld", (unsigned long)lastErr);
  } else {
    // Convert message to UTF-8
    fl_utf8fromwc(emsg, sizeof(emsg), mbuf, msize);
    // Remove '\r's -- they screw up fl_alert()
    char *src = emsg, *dst = emsg;
    for ( ; 1; src++ ) {
      if ( *src == '\0' ) { *dst = '\0'; break; }
      if ( *src != '\r' ) { *dst++ = *src; }
    }
    LocalFree(mbuf);    // Free the buffer allocated by the system
  }
  return emsg;
}

// [Static/Local] See if file exists
static int is_file(const char *filename) {
  utf8_to_wchar(filename, wbuf);
  DWORD att = GetFileAttributesW(wbuf);
  if (att == INVALID_FILE_ATTRIBUTES)
    return 0;
  if ( (att & FILE_ATTRIBUTE_DIRECTORY) == 0 ) return 1;        // not a dir == file
  return 0;
}

// [Static/Local] See if dir exists
static int is_dir(const char *dirname) {
  utf8_to_wchar(dirname, wbuf);
  DWORD att = GetFileAttributesW(wbuf);
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
//     If set to nullptr, frees memory.
//
void ExternalCodeEditor::set_filename(const char *val) {
  if ( filename_ ) free((void*)filename_);
  filename_ = val ? fl_strdup(val) : 0;
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
    if ( Fluid.debug_external_editor )
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
    if ( Fluid.debug_external_editor ) {
      printf("WARNING: sent WIN_CLOSE, but timeout after %ld msecs.."
             "trying TerminateProcess\n", msecTimeout);
    }
    if ( TerminateProcess(hProc, 0) == 0 ) {
      if ( Fluid.debug_external_editor ) {
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
  if ( Fluid.debug_external_editor ) printf("close_editor() called: pid=%ld\n", long(pinfo_.dwProcessId));
  // Wait until editor is closed + reaped
  while ( is_editing() ) {
    switch ( reap_editor() ) {
      case -2:  // no editor running (unlikely to happen)
        return;
      case -1:  // error
        fl_alert("Error reaping external editor\npid=%ld file=%s\nOS error message=%s",
                 long(pinfo_.dwProcessId), filename(), get_ms_errmsg());
        break;
      case 0:   // process still running
        switch ( fl_choice("Please close external editor\npid=%ld file=%s",
                           "Force Close",       // button 0
                           "Closed",            // button 1
                           0,                   // button 2
                           long(pinfo_.dwProcessId), filename() ) ) {
          case 0:       // Force Close
            kill_editor();
            continue;
          case 1:       // Closed? try to reap
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
  if ( Fluid.debug_external_editor )
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
      reap_cleanup();                     // clears pinfo_
      if ( Fluid.debug_external_editor )
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
  utf8_to_wchar(filename(), wbuf);
  HANDLE fh = CreateFileW(wbuf,           // file to read
                         GENERIC_READ,    // reading only
                         FILE_SHARE_READ, // sharing -- allow read share; just getting file size
                         nullptr,            // security
                         OPEN_EXISTING,   // create flags -- must exist
                         0,               // misc flags
                         nullptr);           // templates
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
    DWORD buflen = (DWORD)fsize.QuadPart;
    char *buf = (char*)malloc((size_t)buflen + 1);
    DWORD count;
    if ( ReadFile(fh, buf, buflen, &count, 0) == 0 ) {
      fl_alert("ERROR: ReadFile() failed for %s: %s",
               filename(), get_ms_errmsg());
      free((void*)buf); buf = 0;
      ret = -1;      // fallthru to CloseHandle()
    } else if ( count != buflen ) {
      fl_alert("ERROR: ReadFile() failed for %s:\n"
               "expected %ld bytes, got %ld",
               filename(), long(buflen), long(count));
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
  if ( Fluid.debug_external_editor ) printf("remove_tmpfile() '%s'\n", tmpfile ? tmpfile : "(empty)");
  if ( !tmpfile ) return 0;
  // Filename set? remove (if exists) and zero filename/mtime/size
  if ( is_file(tmpfile) ) {
    if ( Fluid.debug_external_editor ) printf("Removing tmpfile '%s'\n", tmpfile);
    utf8_to_wchar(tmpfile, wbuf);
    if (DeleteFileW(wbuf) == 0) {
      fl_alert("WARNING: Can't DeleteFile() '%s': %s", tmpfile, get_ms_errmsg());
      return -1;
    }
  } else {
    if ( Fluid.debug_external_editor ) printf("remove_tmpfile(): is_file(%s) failed\n", tmpfile);
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
  wchar_t tempdirW[FL_PATH_MAX+1];
  char tempdir[FL_PATH_MAX+1];
  if (GetTempPathW(FL_PATH_MAX, tempdirW) == 0) {
    strcpy(tempdir, "c:\\windows\\temp");      // fallback
  } else {
    strcpy(tempdir, wchar_to_utf8(tempdirW, abuf));
  }
  static char dirname[100];
  _snprintf(dirname, sizeof(dirname), "%s.fluid-%ld",
    tempdir, (long)GetCurrentProcessId());
  if ( Fluid.debug_external_editor ) printf("tmpdir_name(): '%s'\n", dirname);
  return dirname;
}

// [Static/Public] Clear the external editor's tempdir
//    Static so that the main program can call it on exit to clean up.
//
void ExternalCodeEditor::tmpdir_clear() {
  const char *tmpdir = tmpdir_name();
  if ( is_dir(tmpdir) ) {
    if ( Fluid.debug_external_editor ) printf("Removing tmpdir '%s'\n", tmpdir);
    utf8_to_wchar(tmpdir, wbuf);
    if ( RemoveDirectoryW(wbuf) == 0 ) {
      fl_alert("WARNING: Can't RemoveDirectory() '%s': %s",
               tmpdir, get_ms_errmsg());
    }
  }
}

// [Protected] Creates temp dir (if doesn't exist) and returns the dirname
// as a static string. Returns nullptr on error, dialog shows reason.
//
const char* ExternalCodeEditor::create_tmpdir() {
  const char *dirname = tmpdir_name();
  if ( ! is_dir(dirname) ) {
    utf8_to_wchar(dirname, wbuf);
    if (CreateDirectoryW(wbuf, 0) == 0) {
      fl_alert("can't create directory '%s': %s",
        dirname, get_ms_errmsg());
      return nullptr;
    }
  }
  return dirname;
}

// [Protected] Returns temp filename in static buffer.
//    Returns nullptr if can't, posts dialog explaining why.
//
const char* ExternalCodeEditor::tmp_filename() {
  static char path[512];
  const char *tmpdir = create_tmpdir();
  if ( !tmpdir ) return 0;
  const char *ext  = Fluid.proj.code_file_name.c_str();    // e.g. ".cxx"
  _snprintf(path, sizeof(path), "%s\\%p%s", tmpdir, (void*)this, ext);
  path[sizeof(path)-1] = 0;
  return path;
}

// [Static/Local] Save string 'code' to 'filename', returning file's mtime/size
// 'code' can be nullptr -- writes an empty file if so.
// Returns:
//    0 on success
//   -1 on error (posts dialog with reason)
//
static int save_file(const char *filename,
                     const char *code,
                     FILETIME &file_mtime,        // return these since in win32 it's..
                     LARGE_INTEGER &file_size) {  // ..efficient to get while file open
  if ( code == 0 ) code = "";   // nullptr? write an empty file
  memset(&file_mtime, 0, sizeof(file_mtime));
  memset(&file_size, 0, sizeof(file_size));
  utf8_to_wchar(filename, wbuf);
  HANDLE fh = CreateFileW(wbuf,                   // filename
                         GENERIC_WRITE,           // write only
                         0,                       // sharing -- no share during write
                         nullptr,                    // security
                         CREATE_ALWAYS,           // create flags -- recreate
                         FILE_ATTRIBUTE_NORMAL,   // misc flags
                         nullptr);                   // templates
  if ( fh == INVALID_HANDLE_VALUE ) {
    fl_alert("ERROR: couldn't create file '%s': %s",
             filename, get_ms_errmsg());
    return(-1);
  }
  // Write the file, being careful to CloseHandle() even on errs
  DWORD clen = (DWORD)strlen(code);
  DWORD count = 0;
  int ret = 0;
  if ( WriteFile(fh, code, clen, &count, nullptr) == 0 ) {
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
  if ( Fluid.debug_external_editor ) printf("start_editor() cmd='%s', filename='%s'\n",
                        editor_cmd, filename);
  // Startup info
  STARTUPINFOW sinfo;
  memset(&sinfo, 0, sizeof(sinfo));
  sinfo.cb          = sizeof(sinfo);
  sinfo.dwFlags     = 0;
  sinfo.wShowWindow = 0;
  // Process info
  memset(&pinfo_, 0, sizeof(pinfo_));
  // Command
  char cmd[1024];
  _snprintf(cmd, sizeof(cmd), "%s %s", editor_cmd, filename);
  utf8_to_wchar(cmd, wbuf);
  // Start editor process
  if (CreateProcessW(nullptr,              // app name
                    wbuf,               // command to exec
                    nullptr,               // secure attribs
                    nullptr,               // thread secure attribs
                    FALSE,              // handle inheritance
                    0,                  // creation flags
                    nullptr,               // environ block
                    nullptr,               // current dir
                    &sinfo,             // startup info
                    &pinfo_) == 0 ) {   // process info
    fl_alert("CreateProcess() failed to start '%s': %s",
             cmd, get_ms_errmsg());
    return(-1);
  }
  if ( L_editors_open++ == 0 )  // first editor? start timers
    { start_update_timer(); }
  if ( Fluid.debug_external_editor )
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
// If 'pid_reaped' not nullptr, returns PID of reaped editor.
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
      reap_cleanup();                       // clears pinfo_
      if ( pid_reaped ) *pid_reaped = wpid; // return pid to caller
      if ( Fluid.debug_external_editor ) printf("*** EDITOR REAPED: pid=%ld #open=%d\n",
                            long(wpid), L_editors_open);
      return 1;
    }
    case WAIT_FAILED: {    // failed
      return -1;
    }
  }
  return -1;               // any other return unexpected
}

// [Public] Open external editor using 'editor_cmd' to edit 'code'.
//
// 'code' contains multiline code to be edited as a temp file.
// 'code' can be nullptr -- edits an empty file if so.
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
        case -2:        // no editor running (unlikely to happen)
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
          if ( Fluid.debug_external_editor )
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
    if ( Fluid.debug_external_editor ) printf("Editor failed to start\n");
    return -1;  // errors were shown in dialog
  }
  // New editor opened -- start update timer (if not already)
  if ( L_update_timer_cb && !Fl::has_timeout(L_update_timer_cb) ) {
    if ( Fluid.debug_external_editor ) printf("--- Editor opened: STARTING UPDATE TIMER\n");
    Fl::add_timeout(2.0, L_update_timer_cb);
  }
  return 0;
}

// [Public/Static] Start update timer
void ExternalCodeEditor::start_update_timer() {
  if ( !L_update_timer_cb ) return;
  if ( Fluid.debug_external_editor ) printf("--- TIMER: STARTING UPDATES\n");
  Fl::add_timeout(2.0, L_update_timer_cb);
}

// [Public/Static] Stop update timer
void ExternalCodeEditor::stop_update_timer() {
  if ( !L_update_timer_cb ) return;
  if ( Fluid.debug_external_editor ) printf("--- TIMER: STOPPING UPDATES\n");
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
