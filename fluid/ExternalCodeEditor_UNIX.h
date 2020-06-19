//
// "$Id$".
//
//       External code editor management class for Unix
//
//       Handles starting and keeping track of an external text editor,
//       including process start, temp file creation/removal, bookkeeping, killing..
//
#ifndef _EXTCODEEDITOR_H
#define _EXTCODEEDITOR_H

#include <errno.h>      /* errno */
#include <string.h>     /* strerror() */

#include <sys/types.h>  /* stat().. */
#include <sys/stat.h>
#include <unistd.h>

class ExternalCodeEditor {
  int pid_;
  time_t file_mtime_;                   // last modify time of the file (used to determine if file changed)
  size_t file_size_;                    // last file size (used to determine if changed)
  const char *filename_;
protected:
  void kill_editor();
  const char *create_tmpdir();
  const char *tmp_filename();
  int start_editor(const char *cmd, const char *filename);
  void set_filename(const char *val);
public:
  ExternalCodeEditor();
  ~ExternalCodeEditor();
  int is_editing();
  int reap_editor(pid_t *pid_reaped=NULL);
  void close_editor();
  const char *filename() { return filename_; }
  int open_editor(const char *editor_cmd, const char *code);
  int handle_changes(const char **code, int force=0);
  int remove_tmpfile();
  // Public static methods
  static void start_update_timer();
  static void stop_update_timer();
  static const char* tmpdir_name();
  static void tmpdir_clear();
  static int editors_open();
  static void set_update_timer_callback(Fl_Timeout_Handler);
};

#endif /*_EXTCODEEDITOR_H */
//
// End of "$Id$".
//
