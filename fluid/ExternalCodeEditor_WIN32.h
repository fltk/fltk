//
// "$Id$".
//
//       External code editor management class for Windows
//
//       Handles starting and keeping track of an external text editor,
//       including process start, temp file creation/removal, bookkeeping, killing..
//
#ifndef _EXTCODEEDITOR_H
#define _EXTCODEEDITOR_H

/* We require at least Windows 2000 (WINVER == 0x0500) for GetFileSizeEx().  */
/* This must be defined before #include <windows.h> - MinGW doesn't do that. */
#if !defined(WINVER) || (WINVER < 0x0500)
# ifdef WINVER
#  undef WINVER
# endif
# define WINVER 0x0500
#endif
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0500)
# ifdef _WIN32_WINNT
#  undef _WIN32_WINNT
# endif
# define _WIN32_WINNT 0x0500
#endif

#include <windows.h>                /* CreateFile().. */
#include <string.h>                 /* sprintf().. */

class ExternalCodeEditor {
  PROCESS_INFORMATION pinfo_;       // CreateProcess() handle to running process 
  FILETIME            file_mtime_;  // last modify time of the file (used to determine if file changed)
  LARGE_INTEGER       file_size_;   // last file size (used to determine if changed)
  const char *        filename_;    // tmpfilename editor uses
protected:
  void kill_editor();
  void reap_cleanup();
  const char *create_tmpdir();
  const char *tmp_filename();
  int start_editor(const char *cmd, const char *filename);
  void set_filename(const char *val);
public:
  ExternalCodeEditor();
  ~ExternalCodeEditor();
  int is_editing();
  int reap_editor(DWORD *pid_reaped=NULL);
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
