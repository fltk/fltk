/* expand a file name by substuting environment variables and
   home directories.  Returns true if any changes were made.
   to & from may be the same buffer.
*/

#include <FL/filename.H>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#else
# include <unistd.h>
# include <pwd.h>
#endif

#if defined(WIN32) || defined(__EMX__)
static inline int isdirsep(char c) {return c=='/' || c=='\\';}
#else
#define isdirsep(c) ((c)=='/')
#endif

int filename_expand(char *to,const char *from) {

  char temp[FL_PATH_MAX];
  strcpy(temp,from);
  const char *start = temp;
  const char *end = temp+strlen(temp);

  int ret = 0;

  for (char *a=temp; a<end; ) {	// for each slash component
    char *e; for (e=a; e<end && !isdirsep(*e); e++); // find next slash
    const char *value = 0; // this will point at substitute value
    switch (*a) {
    case '~':	// a home directory name
      if (e <= a+1) {	// current user's directory
	value = getenv("HOME");
#ifndef WIN32
      } else {	// another user's directory
	struct passwd *pwd;
	char t = *e; *(char *)e = 0; 
        pwd = getpwnam(a+1); 
        *(char *)e = t;
	    if (pwd) value = pwd->pw_dir;
#endif
      }
      break;
    case '$':		/* an environment variable */
      {char t = *e; *(char *)e = 0; value = getenv(a+1); *(char *)e = t;}
      break;
    }
    if (value) {
      // substitutions that start with slash delete everything before them:
      if (isdirsep(value[0])) start = a;
#if defined(WIN32) || defined(__EMX__)
      // also if it starts with "A:"
      if (value[0] && value[1]==':') start = a;
#endif
      int t = strlen(value); if (isdirsep(value[t-1])) t--;
      memmove(a+t, e, end+1-e);
      end = a+t+(end-e);
      memcpy(a, value, t);
      ret++;
    } else {
      a = e+1;
#if defined(WIN32) || defined(__EMX__)
      if (*e == '\\') {*e = '/'; ret++;} // ha ha!
#endif
    }
  }
  strcpy(to,start);
  return ret;
}
