// filename_isdir.C

// Used by fl_file_chooser

#include <config.h>
#include <FL/filename.H>
#include <sys/stat.h>

int filename_isdir(const char* n) {
  struct stat s;
  return !stat(n, &s) && (s.st_mode&0170000)==0040000;
}
