// filename.H

// fltk filename utilities.  These are used by the file chooser but may
// be useful to other programs.

#ifndef FL_PATH_MAX

#define FL_PATH_MAX 256 // all buffers are this length

const char *filename_name(const char *); // return pointer to name
const char *filename_ext(const char *); // return pointer to .ext
char *filename_setext(char *,const char *ext); // clobber .ext
int filename_expand(char *, const char *from); // do $x and ~x
int filename_absolute(char *, const char *from); // prepend getcwd()
int filename_match(const char *, const char *pattern); // glob match
int filename_isdir(const char*);

// Portable "scandir" function.  Ugly but apparently necessary...

#ifdef WIN32

struct dirent {char d_name[1];};

#else

#include <sys/types.h>
#include <dirent.h>
// warning: on some systems (very few nowadays?) <dirent.h> may not exist.
// The correct information is in one of these files:
//#include <sys/ndir.h>
//#include <sys/dir.h>
//#include <ndir.h>
// plus you must do the following #define:
//#define dirent direct
// It would be best to create a <dirent.h> file that does this...

#endif

int filename_list(const char *d, dirent ***list);

#endif
