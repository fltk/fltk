// fselect.C

// Emulate the Forms file chooser using the fltk file chooser.

#include <FL/forms.H>
#include <string.h>

static char fl_directory[1024];
static const char *fl_pattern;  // assummed passed value is static
static char fl_filename[256];

char* fl_show_file_selector(const char *message,const char *dir,
			    const char *pat,const char *fname) {
  if (dir && dir[0]) strncpy(fl_directory,dir,1023);
  if (pat && pat[0]) fl_pattern = pat;
  if (fname && fname[0]) strncpy(fl_filename,fname,255);
  char *p = fl_directory+strlen(fl_directory);
  if (p > fl_directory && *(p-1)!='/'
#ifdef WIN32
      && *(p-1)!='\\' && *(p-1)!=':'
#endif
      ) *p++ = '/';
  strcpy(p,fl_filename);
  const char *q = fl_file_chooser(message,fl_pattern,fl_directory);
  if (!q) return 0;
  strcpy(fl_directory, q);
  p = (char *)filename_name(fl_directory);
  strcpy(fl_filename, p);
  if (p > fl_directory+1) p--;
  *p = 0;
  return (char *)q;
}

char*	fl_get_directory() {return fl_directory;}

char*	fl_get_pattern() {return (char *)fl_pattern;}

char*	fl_get_filename() {return fl_filename;}
