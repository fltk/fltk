// filename_list.C

// Wrapper for scandir with const-correct function prototypes.

#include <config.h>
#include <FL/filename.H>

extern "C" {
  int numericsort(const dirent **, const dirent **);
#if HAVE_SCANDIR
#else
  int alphasort(const dirent **, const dirent **);
  int scandir (const char *dir, dirent ***namelist,
	       int (*select)(const dirent *),
	       int (*compar)(const dirent **, const dirent **));
#endif
}

int filename_list(const char *d, dirent ***list) {
#if defined(_AIX) || defined(CRAY)
  // on some systems you may need to do this, due to a rather common
  // error in the prototype for the sorting function, where a level
  // of pointer indirection is missing:
  return scandir(d, list, 0, (int(*)(const void*,const void*))numericsort);
#else
#if HAVE_SCANDIR
  return scandir(d, list, 0, (int(*)(dirent**,dirent**))numericsort);
#else // built-in scandir is const-correct:
  return scandir(d, list, 0, numericsort);
#endif
#endif
}
