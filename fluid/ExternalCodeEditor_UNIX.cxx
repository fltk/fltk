//
// "$Id$".
//
//       External code editor management class for Unix
//
#ifndef WIN32           /* This entire file unix only */

#include <errno.h>      /* errno */
#include <string.h>     /* strerror() */
#include <sys/types.h>  /* stat().. */
#include <sys/stat.h>
#include <sys/wait.h>   /* waitpid().. */
#include <fcntl.h>      /* open().. */
#include <signal.h>     /* kill().. */
#include <unistd.h>
#include <stdlib.h>     /* free().. */
#include <stdio.h>      /* snprintf().. */

#include <FL/Fl.H>      /* Fl_Timeout_Handler.. */
#include <FL/fl_ask.H>  /* fl_alert() */

#include "ExternalCodeEditor_UNIX.h"

extern int G_debug;     // defined in fluid.cxx

// Static local data
static int L_editors_open = 0;                          // keep track of #editors open
static Fl_Timeout_Handler L_update_timer_cb = 0;        // app's update timer callback

// [Static/Local] See if file exists
static int is_file(const char *filename) {
  struct stat buf;
  if ( stat(filename, &buf) < 0 ) return(0);
  return(S_ISREG(buf.st_mode) ? 1 : 0);     // regular file?
}

// [Static/Local] See if dir exists
static int is_dir(const char *dirname) {
  struct stat buf;
  if ( stat(dirname, &buf) < 0 ) return(0);
  return(S_ISDIR(buf.st_mode) ? 1 : 0);     // a dir?
}

// CTOR
ExternalCodeEditor::ExternalCodeEditor() {
  pid_        = -1;
  filename_   = 0;
  file_mtime_ = 0;
  file_size_  = 0;
}

// DTOR
ExternalCodeEditor::~ExternalCodeEditor() {
  if ( G_debug )
    printf("ExternalCodeEditor() DTOR CALLED (this=%p, pid=%ld)\n",
           (void*)this, (long)pid_);
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
  return( (pid_ != -1) ? 1 : 0 );
}

// [Protected] Wait for editor to close
void ExternalCodeEditor::close_editor() {
  if ( G_debug ) printf("close_editor() called: pid=%ld\n", long(pid_));
  // Wait until editor is closed + reaped
  while ( is_editing() ) {
    switch ( reap_editor() ) {
      case -2:  // no editor running (unlikely to happen)
        return;
      case -1:  // error
        fl_alert("Error reaping external editor\n"
                 "pid=%ld file=%s", long(pid_), filename());
        break;
      case 0:   // process still running
	switch ( fl_choice("Please close external editor\npid=%ld file=%s",
			   "Force Close",	// button 0
			   "Closed",		// button 1
			   0,			// button 2
			   long(pid_), filename() ) ) {
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

// [Protected] Kill the running editor (if any)
//   Kills the editor, reaps the process, and removes the tmp file.
//   The dtor calls this to ensure no editors remain running when fluid exits.
//
void ExternalCodeEditor::kill_editor() {
  if ( G_debug ) printf("kill_editor() called: pid=%ld\n", (long)pid_);
  if ( !is_editing() ) return;  // editor not running? return..
  kill(pid_, SIGTERM);          // kill editor
  int wcount = 0;
  while ( is_editing() ) {      // and wait for editor to finish..
    usleep(100000);             // 1/10th sec delay gives editor time to close itself
    pid_t pid_reaped;
    switch ( reap_editor(&pid_reaped) ) {
      case -2:  // editor not running (unlikely to happen)
        return;
      case -1:  // error
        fl_alert("Can't seem to close editor of file: %s\n"
                 "waitpid() returned: %s\n"
                 "Please close editor and hit OK",
                 filename(), strerror(errno));
        continue;
      case 0:   // process still running
        if ( ++wcount > 3 ) {   // retry 3x with 1/10th delay before showing dialog
          fl_alert("Can't seem to close editor of file: %s\n"
                   "Please close editor and hit OK", filename());
        }
        continue;
      case 1:  // process reaped (reap_editor() sets pid_ to -1)
        if ( G_debug )
          printf("*** REAPED KILLED EXTERNAL EDITOR: PID %ld\n", (long)pid_reaped);
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
//    -1 -- error getting file info (strerror() has reason)
//
int ExternalCodeEditor::handle_changes(const char **code, int force) {
  code[0] = 0;
  if ( !is_editing() ) return 0;
  // Get current time/size info, see if file changed
  int changed = 0;
  {
    struct stat sbuf;
    if ( stat(filename(), &sbuf) < 0 ) return(-1);  // TODO: show fl_alert(), do this in win32 too, adjust func call docs above
    time_t now_mtime = sbuf.st_mtime;
    size_t now_size  = sbuf.st_size;
    // OK, now see if file changed; update records if so
    if ( now_mtime != file_mtime_ ) { changed = 1; file_mtime_ = now_mtime; }
    if ( now_size  != file_size_  ) { changed = 1; file_size_  = now_size; }
  }
  // No changes? done
  if ( !changed && !force ) return 0;
  // Changes? Load file, and fallthru to close()
  int fd = open(filename(), O_RDONLY);
  if ( fd < 0 ) {
    fl_alert("ERROR: can't open '%s': %s", filename(), strerror(errno));
    return -1;
  }
  int ret = 0;
  char *buf = (char*)malloc(file_size_ + 1);
  ssize_t count = read(fd, buf, file_size_);
  if ( count == -1 ) {
    fl_alert("ERROR: read() %s: %s", filename(), strerror(errno));
    free((void*)buf);
    ret = -1;
  } else if ( (long)count != (long)file_size_ ) {
    fl_alert("ERROR: read() failed for %s:\n"
             "expected %ld bytes, only got %ld",
             filename(), long(file_size_), long(count));
    ret = -1;
  } else {
    // Success -- file loaded OK
    buf[count] = '\0';
    code[0] = buf;        // return pointer to allocated buffer
    ret = 1;
  }
  close(fd);
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
  if ( !tmpfile ) return 0;
  // Filename set? remove (if exists) and zero filename/mtime/size
  if ( is_file(tmpfile) ) {
    if ( G_debug ) printf("Removing tmpfile '%s'\n", tmpfile);
    if ( remove(tmpfile) < 0 ) {
      fl_alert("WARNING: Can't remove() '%s': %s", tmpfile, strerror(errno));
      return -1;
    }
  }
  set_filename(0);
  file_mtime_ = 0;
  file_size_  = 0;
  return 1;
}

// [Static/Public] Return tmpdir name for this fluid instance.
//     Returns pointer to static memory.
//
const char* ExternalCodeEditor::tmpdir_name() {
  static char dirname[100];
  snprintf(dirname, sizeof(dirname), "/tmp/.fluid-%ld", (long)getpid());
  return dirname;
}

// [Static/Public] Clear the external editor's tempdir
//    Static so that the main program can call it on exit to clean up.
//
void ExternalCodeEditor::tmpdir_clear() {
  const char *tmpdir = tmpdir_name();
  if ( is_dir(tmpdir) ) {
    if ( G_debug ) printf("Removing tmpdir '%s'\n", tmpdir);
    if ( rmdir(tmpdir) < 0 ) {
      fl_alert("WARNING: Can't rmdir() '%s': %s", tmpdir, strerror(errno));
    }
  }
}

// [Protected] Creates temp dir (if doesn't exist) and returns the dirname
// as a static string. Returns NULL on error, dialog shows reason.
//
const char* ExternalCodeEditor::create_tmpdir() {
  const char *dirname = tmpdir_name();
  if ( ! is_dir(dirname) ) {
    if ( mkdir(dirname, 0777) < 0 ) {
      fl_alert("can't create directory '%s': %s",
        dirname, strerror(errno));
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
  extern const char *code_file_name;   // fluid's global
  const char *ext  = code_file_name;   // e.g. ".cxx"
  snprintf(path, sizeof(path), "%s/%p%s", tmpdir, (void*)this, ext);
  path[sizeof(path)-1] = 0;
  return path;
}

// [Static/Local] Save string 'code' to 'filename', returning file's mtime/size
// 'code' can be NULL -- writes an empty file if so.
// Returns:
//    0 on success
//   -1 on error (posts dialog with reason)
//
static int save_file(const char *filename, const char *code) {
  int fd = open(filename, O_WRONLY|O_CREAT, 0666);
  if ( fd == -1 ) {
    fl_alert("ERROR: open() '%s': %s", filename, strerror(errno));
    return -1;
  }
  ssize_t clen = strlen(code);
  ssize_t count = write(fd, code, clen);
  int ret = 0;
  if ( count == -1 ) {
    fl_alert("ERROR: write() '%s': %s", filename, strerror(errno));
    ret = -1; // fallthru to close()
  } else if ( count != clen ) {
    fl_alert("ERROR: write() '%s': wrote only %lu bytes, expected %lu",
             filename, (unsigned long)count, (unsigned long)clen);
    ret = -1; // fallthru to close()
  }
  close(fd);
  return(ret);
}

// [Static/Local] Convert string 's' to array of argv[], useful for execve()
//     o 's' will be modified (words will be NULL separated)
//     o argv[] will end up pointing to the words of 's'
//     o Caller must free argv with: free(argv);
//
static int make_args(char *s,         // string containing words (gets trashed!)
                     int *aargc,      // pointer to argc
                     char ***aargv) { // pointer to argv
  char *ss, **argv;
  if ((argv=(char**)malloc(sizeof(char*) * (strlen(s)/2)))==NULL) {
    return -1;
  }
  int t;
  for(t=0; (t==0)?(ss=strtok(s," \t")):(ss=strtok(0," \t")); t++) {
    argv[t] = ss;
  }
  argv[t] = 0;
  aargv[0] = argv;
  aargc[0] = t;
  return(t);
}

// [Protected] Start editor in background (fork/exec)
// Returns:
//    >  0 on success, leaves editor child process running as 'pid_'
//    > -1 on error, posts dialog with reason (child exits)
//
int ExternalCodeEditor::start_editor(const char *editor_cmd,
                                     const char *filename) {
  if ( G_debug ) printf("start_editor() cmd='%s', filename='%s'\n",
                        editor_cmd, filename);
  char cmd[1024];
  snprintf(cmd, sizeof(cmd), "%s %s", editor_cmd, filename);
  // Fork editor to background..
  switch ( pid_ = fork() ) {
    case -1:    // error
      fl_alert("couldn't fork(): %s", strerror(errno));
      return -1;
    case 0: {   // child
      // NOTE: OSX wants minimal code between fork/exec, see Apple TN2083
      int nargs;
      char **args = 0;
      make_args(cmd, &nargs, &args);
      execvp(args[0], args);  // run command - doesn't return if succeeds
      fl_alert("couldn't exec() '%s': %s", cmd, strerror(errno));
      exit(1);
    }
    default:    // parent
      if ( L_editors_open++ == 0 )  // first editor? start timers
        { start_update_timer(); }
      if ( G_debug )
        printf("--- EDITOR STARTED: pid_=%ld #open=%d\n", (long)pid_, L_editors_open);
      break;
  }
  return 0;
}

// [Public] Try to reap external editor process
// If 'pid_reaped' not NULL, returns PID of reaped editor.
// Returns:
//   -2 -- editor not open
//   -1 -- waitpid() failed (errno has reason)
//    0 -- process still running
//    1 -- process finished + reaped ('pid_reaped' has pid), pid_ set to -1.
//         Handles removing tmpfile/zeroing file_mtime/file_size/filename
//
// If return value <=0, 'pid_reaped' is set to zero.
//
int ExternalCodeEditor::reap_editor(pid_t *pid_reaped) {
  if ( pid_reaped ) *pid_reaped = 0;
  if ( !is_editing() ) return -2;
  int status = 0;
  pid_t wpid;
  switch (wpid = waitpid(pid_, &status, WNOHANG)) {
    case -1:    // waitpid() failed
      return -1;
    case 0:     // process didn't reap, still running
      return 0;
    default:    // process reaped
      if ( pid_reaped ) *pid_reaped = wpid;  // return pid to caller
      remove_tmpfile(); // also zeroes mtime/size
      pid_ = -1;
      if ( --L_editors_open <= 0 )
        { stop_update_timer(); }
      break;
  }
  if ( G_debug ) 
    printf("*** EDITOR REAPED: pid=%ld #open=%d\n", long(wpid), L_editors_open);
  return 1;
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
      pid_t wpid;
      switch ( reap_editor(&wpid) ) {
        case -2:        // no editor running? (unlikely if is_editing() true)
	  break;
        case -1:        // waitpid() failed
          fl_alert("ERROR: waitpid() failed: %s\nfile='%s', pid=%ld",
            strerror(errno), filename(), (long)pid_);
          return -1;
        case 0:         // process still running
          fl_alert("Editor Already Open\n  file='%s'\n  pid=%ld",
            filename(), (long)pid_);
          return 0;
        case 1:        // process reaped, wpid is pid reaped
          if ( G_debug )
            printf("*** REAPED EXTERNAL EDITOR: PID %ld\n", (long)wpid);
          break;        // fall thru to open new editor instance
      }
      // Reinstate tmp filename (reap_editor() clears it)
      set_filename(tmp_filename());
    }
  }
  if ( save_file(filename(), code) < 0 ) {
    return -1;  // errors were shown in dialog
  }
  // Update mtime/size from closed file
  struct stat sbuf;
  if ( stat(filename(), &sbuf) < 0 ) {
    fl_alert("ERROR: can't stat('%s'): %s", filename(), strerror(errno));
    return -1;
  }
  file_mtime_ = sbuf.st_mtime;
  file_size_  = sbuf.st_size;
  if ( start_editor(editor_cmd, filename()) < 0 ) { // open file in external editor
    if ( G_debug ) printf("Editor failed to start\n");
    return -1;  // errors were shown in dialog
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

#endif /* !WIN32 */
//
// End of "$Id$".
//
