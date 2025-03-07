//
// Definition of Unix/Linux system driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2024 by Bill Spitzak and others.
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

#include "Fl_Unix_System_Driver.H"
#include "Fl_Unix_Screen_Driver.H"
#include <FL/Fl_File_Browser.H>
#include <FL/fl_string_functions.h>  // fl_strdup
#include <FL/platform.H>
#include "../../flstring.h"
#include "../../Fl_Timeout.h"

#include <locale.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>     // strerror(errno)
#include <errno.h>      // errno
#include <string>
#if HAVE_DLSYM && HAVE_DLFCN_H
#include <dlfcn.h>   // for dlsym
#endif


#if defined(_AIX)
extern "C" {
#  include <sys/vmount.h>
#  include <sys/mntctl.h>
  // Older AIX versions don't expose this prototype
  int mntctl(int, int, char *);
}
#endif  // _AIX

#if defined(__NetBSD__)
extern "C" {
#  include <sys/param.h>  // For '__NetBSD_Version__' definition
#  if defined(__NetBSD_Version__) && (__NetBSD_Version__ >= 300000000)
#    include <sys/types.h>
#    include <sys/statvfs.h>
#    if defined(HAVE_PTHREAD) && defined(HAVE_PTHREAD_H)
#      include <pthread.h>
#    endif  // HAVE_PTHREAD && HAVE_PTHREAD_H
#    ifdef HAVE_PTHREAD
  static pthread_mutex_t getvfsstat_mutex = PTHREAD_MUTEX_INITIALIZER;
#    endif  // HAVE_PTHREAD/
#  endif  // __NetBSD_Version__
}
#endif  // __NetBSD__

#ifndef HAVE_SCANDIR
extern "C" {
  int fl_scandir(const char *dirname, struct dirent ***namelist,
                 int (*select)(struct dirent *),
                 int (*compar)(struct dirent **, struct dirent **),
                 char *errmsg, int errmsg_sz);
}
#endif

#if defined(__linux__) && defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 700
static locale_t c_locale = NULL;
#endif


int Fl_Unix_System_Driver::clocale_vprintf(FILE *output, const char *format, va_list args) {
#if defined(__linux__) && defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 700
  if (!c_locale)
    c_locale = newlocale(LC_NUMERIC_MASK, "C", duplocale(LC_GLOBAL_LOCALE));
  locale_t previous_locale = uselocale(c_locale);
  int retval = vfprintf(output, format, args);
  uselocale(previous_locale);
#else
  char *saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  int retval = vfprintf(output, format, args);
  setlocale(LC_NUMERIC, saved_locale);
#endif
  return retval;
}

int Fl_Unix_System_Driver::clocale_vsnprintf(char *output, size_t output_size, const char *format, va_list args) {
#if defined(__linux__) && defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 700
  if (!c_locale)
    c_locale = newlocale(LC_NUMERIC_MASK, "C", duplocale(LC_GLOBAL_LOCALE));
  locale_t previous_locale = uselocale(c_locale);
  int retval = vsnprintf(output, output_size, format, args);
  uselocale(previous_locale);
#else
  char *saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  int retval = vsnprintf(output, output_size, format, args);
  setlocale(LC_NUMERIC, saved_locale);
#endif
  return retval;
}

int Fl_Unix_System_Driver::clocale_vsscanf(const char *input, const char *format, va_list args) {
#if defined(__linux__) && defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 700
  if (!c_locale)
    c_locale = newlocale(LC_NUMERIC_MASK, "C", duplocale(LC_GLOBAL_LOCALE));
  locale_t previous_locale = uselocale(c_locale);
  int retval = vsscanf(input, format, args);
  uselocale(previous_locale);
#else
  char *saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
#if defined(__hpux)
  // HP-UX 11.11 provides:
  //   int vsscanf(char *s, const char *format, va_list ap);
  int retval = vsscanf((char*)input, format, args);
#else  // defined(__hpux)
  int retval = vsscanf(input, format, args);
#endif  // defined(__hpux)
  setlocale(LC_NUMERIC, saved_locale);
#endif
  return retval;
}


// Find a program in the path...
static char *path_find(const char *program, char *filename, int filesize) {
  const char    *path;                  // Search path
  char          *ptr,                   // Pointer into filename
                *end;                   // End of filename buffer


  if ((path = fl_getenv("PATH")) == NULL) path = "/bin:/usr/bin";

  for (ptr = filename, end = filename + filesize - 1; *path; path ++) {
    if (*path == ':') {
      if (ptr > filename && ptr[-1] != '/' && ptr < end) *ptr++ = '/';

      strlcpy(ptr, program, end - ptr + 1);

      if (!access(filename, X_OK)) return filename;

      ptr = filename;
    } else if (ptr < end) *ptr++ = *path;
  }

  if (ptr > filename) {
    if (ptr[-1] != '/' && ptr < end) *ptr++ = '/';

    strlcpy(ptr, program, end - ptr + 1);

    if (!access(filename, X_OK)) return filename;
  }

  return 0;
}


int Fl_Unix_System_Driver::open_uri(const char *uri, char *msg, int msglen)
{
  // Run any of several well-known commands to open the URI.
  //
  // We give preference to the Portland group's xdg-utils
  // programs which run the user's preferred web browser, etc.
  // based on the current desktop environment in use.  We fall
  // back on older standards and then finally test popular programs
  // until we find one we can use.
  //
  // Note that we specifically do not support the MAILER and
  // BROWSER environment variables because we have no idea whether
  // we need to run the listed commands in a terminal program.
  char  command[FL_PATH_MAX],           // Command to run...
  *argv[4],                     // Command-line arguments
  remote[1024];                 // Remote-mode command...
  const char * const *commands;         // Array of commands to check...
  int i;
  static const char * const browsers[] = {
    "xdg-open", // Portland
    "htmlview", // Freedesktop.org
    "firefox",
    "mozilla",
    "netscape",
    "konqueror", // KDE
    "opera",
    "hotjava", // Solaris
    "mosaic",
    NULL
  };
  static const char * const readers[] = {
    "xdg-email", // Portland
    "thunderbird",
    "mozilla",
    "netscape",
    "evolution", // GNOME
    "kmailservice", // KDE
    NULL
  };
  static const char * const managers[] = {
    "xdg-open", // Portland
    "fm", // IRIX
    "dtaction", // CDE
    "nautilus", // GNOME
    "konqueror", // KDE
    NULL
  };

  // Figure out which commands to check for...
  if (!strncmp(uri, "file://", 7)) commands = managers;
  else if (!strncmp(uri, "mailto:", 7) ||
           !strncmp(uri, "news:", 5)) commands = readers;
  else commands = browsers;

  // Find the command to run...
  for (i = 0; commands[i]; i ++)
    if (path_find(commands[i], command, sizeof(command))) break;

  if (!commands[i]) {
    if (msg) {
      snprintf(msg, msglen, "No helper application found for \"%s\"", uri);
    }

    return 0;
  }

  // Handle command-specific arguments...
  argv[0] = (char *)commands[i];

  if (!strcmp(commands[i], "firefox") ||
      !strcmp(commands[i], "mozilla") ||
      !strcmp(commands[i], "netscape") ||
      !strcmp(commands[i], "thunderbird")) {
    // program -remote openURL(uri)
    snprintf(remote, sizeof(remote), "openURL(%s)", uri);

    argv[1] = (char *)"-remote";
    argv[2] = remote;
    argv[3] = 0;
  } else if (!strcmp(commands[i], "dtaction")) {
    // dtaction open uri
    argv[1] = (char *)"open";
    argv[2] = (char *)uri;
    argv[3] = 0;
  } else {
    // program uri
    argv[1] = (char *)uri;
    argv[2] = 0;
  }

  if (msg) {
    strlcpy(msg, argv[0], msglen);

    for (i = 1; argv[i]; i ++) {
      strlcat(msg, " ", msglen);
      strlcat(msg, argv[i], msglen);
    }
  }

  return run_program(command, argv, msg, msglen) != 0;
}


int Fl_Unix_System_Driver::file_browser_load_filesystem(Fl_File_Browser *browser, char *filename, int lname, Fl_File_Icon *icon)
{
  int num_files = 0;
#if defined(_AIX)
  // AIX don't write the mounted filesystems to a file like '/etc/mnttab'.
  // But reading the list of mounted filesystems from the kernel is possible:
  // http://publib.boulder.ibm.com/infocenter/pseries/v5r3/topic/com.ibm.aix.basetechref/doc/basetrf1/mntctl.htm
  int res = -1, len;
  char *list = NULL, *name;

  // We always have the root filesystem
  browser->add("/", icon);
  num_files++;
  // Get the required buffer size for the vmount structures
  res = mntctl(MCTL_QUERY, sizeof(len), (char *) &len);
  if (!res) {
    // Allocate buffer ...
    list = (char *) malloc((size_t) len);
    if (NULL == list) {
      res = -1;
    } else {
      // ... and read vmount structures from kernel
      res = mntctl(MCTL_QUERY, len, list);
      if (0 >= res) {
        res = -1;
      } else {
        struct vmount *vp = (struct vmount *) list;
        for (int i = 0; i < res; ++i) {
          name = (char *) vp + vp->vmt_data[VMT_STUB].vmt_off;
          strlcpy(filename, name, lname);
          // Skip the already added root filesystem
          if (strcmp("/", filename) != 0) {
            strlcat(filename, "/", lname);
            browser->add(filename, icon);
            num_files++;
          }
          vp = (struct vmount *) ((char *) vp + vp->vmt_length);
        }
      }
    }
  }
  // Note: Executing 'free(NULL)' is allowed and simply do nothing
  free((void *) list);
#elif defined(__NetBSD__) && defined(__NetBSD_Version__) && (__NetBSD_Version__ >= 300000000)
  // NetBSD don't write the mounted filesystems to a file like '/etc/mnttab'.
  // Since NetBSD 3.0 the system call getvfsstat(2) has replaced getfsstat(2)
  // that is used by getmntinfo(3):
  // http://www.daemon-systems.org/man/getmntinfo.3.html
  int res = -1;
  struct statvfs *list;

  // We always have the root filesystem
  browser->add("/", icon);
  num_files++;
#  ifdef HAVE_PTHREAD
  // Lock mutex for thread safety
  if (!pthread_mutex_lock(&getvfsstat_mutex)) {
#  endif  // HAVE_PTHREAD
    // Get list of statvfs structures
    res = getmntinfo(&list, ST_WAIT);
    if (0 < res) {
      for (int i = 0;  i < res; ++i) {
        strlcpy(filename, list[i].f_mntonname, lname);
        // Skip the already added root filesystem
        if (strcmp("/", filename) != 0) {
          strlcat(filename, "/", lname);
          browser->add(filename, icon);
          num_files++;
        }
      }
    } else {
      res = -1;
    }
#  ifdef HAVE_PTHREAD
    pthread_mutex_unlock(&getvfsstat_mutex);
  }
#  endif  // HAVE_PTHREAD
#else
  //
  // UNIX code uses /etc/fstab or similar...
  //
  FILE  *mtab;          // /etc/mtab or /etc/mnttab file
  char  line[FL_PATH_MAX];      // Input line

  // Every Unix has a root filesystem '/'.
  // This ensures that the user don't get an empty
  // window after requesting filesystem list.
  browser->add("/", icon);
  num_files ++;

  //
  // Open the file that contains a list of mounted filesystems...
  //
  // Note: this misses automounted filesystems on FreeBSD if absent from /etc/fstab
  //

  mtab = fopen("/etc/mnttab", "r");     // Fairly standard
  if (mtab == NULL)
    mtab = fopen("/etc/mtab", "r");     // More standard
  if (mtab == NULL)
    mtab = fopen("/etc/fstab", "r");    // Otherwise fallback to full list
  if (mtab == NULL)
    mtab = fopen("/etc/vfstab", "r");   // Alternate full list file

  if (mtab != NULL)
  {
    while (fgets(line, sizeof(line), mtab) != NULL)
    {
      if (line[0] == '#' || line[0] == '\n')
        continue;
      if (sscanf(line, "%*s%4095s", filename) != 1)
        continue;
      if (strcmp("/", filename) == 0)
        continue; // "/" was added before

      // Add a trailing slash (except for the root filesystem)
      strlcat(filename, "/", lname);

      //        printf("Fl_File_Browser::load() - adding \"%s\" to list...\n", filename);
      browser->add(filename, icon);
      num_files ++;
    }

    fclose(mtab);
  }
#endif // _AIX || ...
  return num_files;
}

void Fl_Unix_System_Driver::newUUID(char *uuidBuffer)
{
  unsigned char b[16];
#if HAVE_DLSYM && HAVE_DLFCN_H
  typedef void (*gener_f_type)(uchar*);
  static bool looked_for_uuid_generate = false;
  static gener_f_type uuid_generate_f = NULL;
  if (!looked_for_uuid_generate) {
    looked_for_uuid_generate = true;
    uuid_generate_f = (gener_f_type)dlopen_or_dlsym("libuuid", "uuid_generate");
  }
  if (uuid_generate_f) {
    uuid_generate_f(b);
  } else
#endif
  {
    time_t t = time(0);                   // first 4 byte
    b[0] = (unsigned char)t;
    b[1] = (unsigned char)(t>>8);
    b[2] = (unsigned char)(t>>16);
    b[3] = (unsigned char)(t>>24);
    int r = rand();                       // four more bytes
    b[4] = (unsigned char)r;
    b[5] = (unsigned char)(r>>8);
    b[6] = (unsigned char)(r>>16);
    b[7] = (unsigned char)(r>>24);
    unsigned long a = (unsigned long)&t;  // four more bytes
    b[8] = (unsigned char)a;
    b[9] = (unsigned char)(a>>8);
    b[10] = (unsigned char)(a>>16);
    b[11] = (unsigned char)(a>>24);
    // Now we try to find 4 more "random" bytes. We extract the
    // lower 4 bytes from the address of t - it is created on the
    // stack so *might* be in a different place each time...
    // This is now done via a union to make it compile OK on 64-bit systems.
    union { void *pv; unsigned char a[sizeof(void*)]; } v;
    v.pv = (void *)(&t);
    // NOTE: May need to handle big- or little-endian systems here
# if WORDS_BIGENDIAN
    b[8] = v.a[sizeof(void*) - 1];
    b[9] = v.a[sizeof(void*) - 2];
    b[10] = v.a[sizeof(void*) - 3];
    b[11] = v.a[sizeof(void*) - 4];
# else // data ordered for a little-endian system
    b[8] = v.a[0];
    b[9] = v.a[1];
    b[10] = v.a[2];
    b[11] = v.a[3];
# endif
    char name[80];                        // last four bytes
    gethostname(name, 79);
    memcpy(b+12, name, 4);
  }
  snprintf(uuidBuffer, 36+1, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
          b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7],
          b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
}

/*
 Create a buffer that holds the absolute file path and name of the preferences
 file for the given root, vendor, and application name.
 Note: `prefs` can be NULL!
 */
char *Fl_Unix_System_Driver::preference_rootnode(Fl_Preferences * /*prefs*/,
                                                 Fl_Preferences::Root root,
                                                 const char *vendor,
                                                 const char *application)
{
  // Create a static buffer for our filename
  static char *buffer = 0L;
  if (!buffer) buffer = (char*)::calloc(1, FL_PATH_MAX);
  buffer[0] = '\0';

  // Make sure that the parameters are not NULL
  if ( (vendor==NULL) || (vendor[0]==0) )
    vendor = "unknown";
  if ( (application==NULL) || (application[0]==0) )
    application = "unknown";

  // Dispatch to the various path creators for the requested root.
  char *prefs_path = NULL;
  int pref_type = root & Fl_Preferences::ROOT_MASK;
  switch (pref_type) {
    case Fl_Preferences::USER:
      prefs_path = preference_user_rootnode(vendor, application, buffer);
      break;
    case Fl_Preferences::SYSTEM:
      prefs_path = preference_system_rootnode(vendor, application, buffer);
      break;
    case Fl_Preferences::MEMORY:
    default:
      prefs_path = preference_memory_rootnode(vendor, application, buffer);
      break;
  }

  return prefs_path;
}

/*
 Memory based preferences are never saved to any file, so the path is
 just an empty string.
 */
char *Fl_Unix_System_Driver::preference_memory_rootnode(
    const char * /*vendor*/,
    const char * /*application*/,
    char *buffer)
{
  buffer[0] = 0;
  return buffer;
}

/*
 The path and file name for system preferences on Unix type
 systems is `/etc/fltk/{vendor}/{application}.prefs`.
 */
char *Fl_Unix_System_Driver::preference_system_rootnode(
    const char *vendor,
    const char *application,
    char *buffer)
{
  snprintf(buffer, FL_PATH_MAX, "/etc/fltk/%s/%s.prefs", vendor, application);
  return buffer;
}

/*
 The user path changed between FLTK 1.3 and 1.4. It is now calculated
 using XDG guidelines. It is `$XDG_CONFIG_HOME/{vendor}/{application}.prefs`
 if `$XDG_CONFIG_HOME` is set, and `$HOME/.config/{vendor}/{application}.prefs`
 if `$XDG_CONFIG_HOME` is not set or empty.

 For compatibility with 1.3 preferences, this function checks

 1. Does the XDG based top level folder '{vendor}' exist? If yes, use it.
 2. If not: does the old location $HOME/.fltk/{vendor} exist? If yes, use it.
 3. If neither: fall back to (1.) and use the XDG based folder (create it and
    the prefs file below it).
 */
char *Fl_Unix_System_Driver::preference_user_rootnode(
    const char *vendor,
    const char *application,
    char *buffer)
{
  // Find the path to the user's home directory.
  const char *home_path_c = getenv("HOME");
  std::string home_path = home_path_c ? home_path_c : "";
  if (home_path.empty()) {
    struct passwd *pw = getpwuid(getuid());
    if (pw)
      home_path = pw->pw_dir;
  }

  // 1: Generate the 1.4 path for this vendor and application.
  const char *prefs_path_14_c = getenv("XDG_CONFIG_HOME");
  std::string prefs_path_14 = prefs_path_14_c ? prefs_path_14_c : "";
  if (prefs_path_14.empty()) {
    prefs_path_14 = home_path + "/.config";
  } else {
    if (prefs_path_14[prefs_path_14.size()-1]!='/')
      prefs_path_14.append("/");
    if (prefs_path_14.find("~/")==0) // starts with "~"
      prefs_path_14.replace(0, 1, home_path);
    int h_env = prefs_path_14.find("${HOME}");
    if (h_env!=(int)prefs_path_14.npos)
      prefs_path_14.replace(h_env, 7, home_path);
    h_env = prefs_path_14.find("$HOME/");
    if (h_env!=(int)prefs_path_14.npos)
      prefs_path_14.replace(h_env, 5, home_path);
  }
  if (prefs_path_14[prefs_path_14.size()-1]!='/')
    prefs_path_14.append("/");
  prefs_path_14.append(vendor);

  // 2: If this base path does not exist, try the 1.3 path
  if (::access(prefs_path_14.c_str(), F_OK) == -1) {
    std::string prefs_path_13 = home_path + "/.fltk/" + vendor;
    if (::access(prefs_path_13.c_str(), F_OK) == 0) {
      prefs_path_13.append("/");
      prefs_path_13.append(application);
      prefs_path_13.append(".prefs");
      strlcpy(buffer, prefs_path_13.c_str(), FL_PATH_MAX);
      return buffer;
    }
  }

  // 3: neither path exists, return the 1.4 file path and name
  prefs_path_14.append("/");
  prefs_path_14.append(application);
  prefs_path_14.append(".prefs");
  strlcpy(buffer, prefs_path_14.c_str(), FL_PATH_MAX);
  return buffer;
}

//
// Needs some docs
// Returns -1 on error, errmsg will contain OS error if non-NULL.
//
int Fl_Unix_System_Driver::filename_list(const char *d,
                                        dirent ***list,
                                        int (*sort)(struct dirent **, struct dirent **),
                                        char *errmsg, int errmsg_sz) {
  int dirlen;
  char *dirloc;

  if (errmsg && errmsg_sz>0) errmsg[0] = '\0';

  // Assume that locale encoding is no less dense than UTF-8
  dirlen = strlen(d);
  dirloc = (char *)malloc(dirlen + 1);
  fl_utf8to_mb(d, dirlen, dirloc, dirlen + 1);

#ifndef HAVE_SCANDIR
  // This version is when we define our own scandir. Note it updates errmsg on errors.
  int n = fl_scandir(dirloc, list, 0, sort, errmsg, errmsg_sz);
#elif defined(HAVE_SCANDIR_POSIX)
  // POSIX (2008) defines the comparison function like this:
  int n = scandir(dirloc, list, 0, (int(*)(const dirent **, const dirent **))sort);
#elif defined(__osf__)
  // OSF, DU 4.0x
  int n = scandir(dirloc, list, 0, (int(*)(dirent **, dirent **))sort);
#elif defined(_AIX)
  // AIX is almost standard...
  int n = scandir(dirloc, list, 0, (int(*)(void*, void*))sort);
#elif defined(__sgi)
  int n = scandir(dirloc, list, 0, sort);
#else
  // The vast majority of UNIX systems want the sort function to have this
  // prototype, most likely so that it can be passed to qsort without any
  // changes:
  int n = scandir(dirloc, list, 0, (int(*)(const void*,const void*))sort);
#endif

  free(dirloc);

  if (n==-1) {
    // Don't write to errmsg if FLTK's fl_scandir() already set it.
    // If OS's scandir() was used (HAVE_SCANDIR), we return its error in errmsg here..
#ifdef HAVE_SCANDIR
    if (errmsg) fl_snprintf(errmsg, errmsg_sz, "%s", strerror(errno));
#endif
    return -1;
  }

  // convert every filename to UTF-8, and append a '/' to all
  // filenames that are directories
  int i;
  char *fullname = (char*)malloc(dirlen+FL_PATH_MAX+3); // Add enough extra for two /'s and a nul
  // Use memcpy for speed since we already know the length of the string...
  memcpy(fullname, d, dirlen+1);

  char *name = fullname + dirlen;
  if (name!=fullname && name[-1]!='/')
    *name++ = '/';

  for (i=0; i<n; i++) {
    int newlen;
    dirent *de = (*list)[i];
    int len = strlen(de->d_name);
    newlen = fl_utf8from_mb(NULL, 0, de->d_name, len);
    dirent *newde = (dirent*)malloc(de->d_name - (char*)de + newlen + 2); // Add space for a / and a nul

    // Conversion to UTF-8
    memcpy(newde, de, de->d_name - (char*)de);
    fl_utf8from_mb(newde->d_name, newlen + 1, de->d_name, len);

    // Check if dir (checks done on "old" name as we need to interact with
    // the underlying OS)
    if (de->d_name[len-1]!='/' && len<=FL_PATH_MAX) {
      // Use memcpy for speed since we already know the length of the string...
      memcpy(name, de->d_name, len+1);
      if (fl_filename_isdir(fullname)) {
        char *dst = newde->d_name + newlen;
        *dst++ = '/';
        *dst = 0;
      }
    }

    free(de);
    (*list)[i] = newde;
  }
  free(fullname);

  return n;
}

int Fl_Unix_System_Driver::utf8locale() {
  static int ret = 2;
  if (ret == 2) {
    char* s;
    ret = 1; /* assume UTF-8 if no locale */
    if (((s = getenv("LC_CTYPE")) && *s) ||
        ((s = getenv("LC_ALL"))   && *s) ||
        ((s = getenv("LANG"))     && *s)) {
      ret = (strstr(s,"utf") || strstr(s,"UTF"));
    }
  }
  return ret;
}


// returns pointer to the filename, or null if name ends with '/'
const char *Fl_Unix_System_Driver::filename_name(const char *name) {
  const char *p,*q;
  if (!name) return (0);
  for (p=q=name; *p;) if (*p++ == '/') q = p;
  return q;
}


static int fd_array_size = 0;

void Fl_Unix_System_Driver::add_fd(int n, int events, void (*cb)(int, void*), void *v) {
  remove_fd(n,events);
  int i = Fl_Unix_Screen_Driver::nfds++;
  if (i >= fd_array_size) {
    Fl_Unix_Screen_Driver::FD *temp;
    fd_array_size = 2*fd_array_size+1;

    if (!Fl_Unix_Screen_Driver::fd) temp =
      (Fl_Unix_Screen_Driver::FD*)malloc(fd_array_size*sizeof(Fl_Unix_Screen_Driver::FD));
    else temp = (Fl_Unix_Screen_Driver::FD*)realloc(Fl_Unix_Screen_Driver::fd,
                          fd_array_size*sizeof(Fl_Unix_Screen_Driver::FD));

    if (!temp) return;
    Fl_Unix_Screen_Driver::fd = temp;

#  if USE_POLL
    pollfd *tpoll;

    if (!pollfds) tpoll = (pollfd*)malloc(fd_array_size*sizeof(pollfd));
    else tpoll = (pollfd*)realloc(pollfds, fd_array_size*sizeof(pollfd));

    if (!tpoll) return;
    pollfds = tpoll;
#  endif
  }
  Fl_Unix_Screen_Driver::fd[i].cb = cb;
  Fl_Unix_Screen_Driver::fd[i].arg = v;
#  if USE_POLL
  pollfds[i].fd = n;
  pollfds[i].events = events;
#  else
  Fl_Unix_Screen_Driver::fd[i].fd = n;
  Fl_Unix_Screen_Driver::fd[i].events = events;
  if (events & POLLIN) FD_SET(n, &Fl_Unix_Screen_Driver::fdsets[0]);
  if (events & POLLOUT) FD_SET(n, &Fl_Unix_Screen_Driver::fdsets[1]);
  if (events & POLLERR) FD_SET(n, &Fl_Unix_Screen_Driver::fdsets[2]);
  if (n > Fl_Unix_Screen_Driver::maxfd) Fl_Unix_Screen_Driver::maxfd = n;
#  endif
}

void Fl_Unix_System_Driver::add_fd(int n, void (*cb)(int, void*), void* v) {
  add_fd(n, POLLIN, cb, v);
}

void Fl_Unix_System_Driver::remove_fd(int n, int events) {
  int i,j;
# if !USE_POLL
  Fl_Unix_Screen_Driver::maxfd = -1; // recalculate maxfd on the fly
# endif
  for (i=j=0; i<Fl_Unix_Screen_Driver::nfds; i++) {
#  if USE_POLL
    if (pollfds[i].fd == n) {
      int e = pollfds[i].events & ~events;
      if (!e) continue; // if no events left, delete this fd
      pollfds[j].events = e;
    }
#  else
    if (Fl_Unix_Screen_Driver::fd[i].fd == n) {
      int e = Fl_Unix_Screen_Driver::fd[i].events & ~events;
      if (!e) continue; // if no events left, delete this fd
      Fl_Unix_Screen_Driver::fd[i].events = e;
    }
    if (Fl_Unix_Screen_Driver::fd[i].fd > Fl_Unix_Screen_Driver::maxfd) Fl_Unix_Screen_Driver::maxfd = Fl_Unix_Screen_Driver::fd[i].fd;
#  endif
    // move it down in the array if necessary:
    if (j<i) {
      Fl_Unix_Screen_Driver::fd[j] = Fl_Unix_Screen_Driver::fd[i];
#  if USE_POLL
      pollfds[j] = pollfds[i];
#  endif
    }
    j++;
  }
  Fl_Unix_Screen_Driver::nfds = j;
#  if !USE_POLL
  if (events & POLLIN) FD_CLR(n, &Fl_Unix_Screen_Driver::fdsets[0]);
  if (events & POLLOUT) FD_CLR(n, &Fl_Unix_Screen_Driver::fdsets[1]);
  if (events & POLLERR) FD_CLR(n, &Fl_Unix_Screen_Driver::fdsets[2]);
#  endif
}

void Fl_Unix_System_Driver::remove_fd(int n) {
  remove_fd(n, -1);
}


double Fl_Unix_System_Driver::wait(double time_to_wait)
{
  Fl_Unix_Screen_Driver *scr_dr = (Fl_Unix_Screen_Driver*)Fl::screen_driver();
  time_to_wait = Fl_System_Driver::wait(time_to_wait);
  if (time_to_wait <= 0.0) {
    // do flush second so that the results of events are visible:
    int ret = scr_dr->poll_or_select_with_delay(0.0);
    Fl::flush();
    return ret;
  } else {
    // do flush first so that user sees the display:
    Fl::flush();
    if (Fl::idle) // 'idle' may have been set within flush()
      time_to_wait = 0.0;
    else {
      Fl_Timeout::elapse_timeouts();
      time_to_wait = Fl_Timeout::time_to_wait(time_to_wait);
    }
    return scr_dr->poll_or_select_with_delay(time_to_wait);
  }
}

int Fl_Unix_System_Driver::ready()
{
  Fl_Unix_Screen_Driver *scr_dr = (Fl_Unix_Screen_Driver*)Fl::screen_driver();
  Fl_Timeout::elapse_timeouts();
  if (Fl_Timeout::time_to_wait(1.0) <= 0.0) return 1;
  return scr_dr->poll_or_select();
}


static void write_short(unsigned char **cp, short i) {
  unsigned char *c = *cp;
  *c++ = i & 0xFF; i >>= 8;
  *c++ = i & 0xFF;
  *cp = c;
}

static void write_int(unsigned char **cp, int i) {
  unsigned char *c = *cp;
  *c++ = i & 0xFF; i >>= 8;
  *c++ = i & 0xFF; i >>= 8;
  *c++ = i & 0xFF; i >>= 8;
  *c++ = i & 0xFF;
  *cp = c;
}


unsigned char *Fl_Unix_System_Driver::create_bmp(const unsigned char *data, int W, int H, int *return_size) {
  int R = ((3*W+3)/4) * 4; // the number of bytes per row, rounded up to multiple of 4
  int s=H*R;
  int fs=14+40+s;
  unsigned char *b=new unsigned char[fs];
  unsigned char *c=b;
  // BMP header
  *c++='B';
  *c++='M';
  write_int(&c,fs);
  write_int(&c,0);
  write_int(&c,14+40);
  // DIB header:
  write_int(&c,40);
  write_int(&c,W);
  write_int(&c,H);
  write_short(&c,1);
  write_short(&c,24);//bits ber pixel
  write_int(&c,0);//RGB
  write_int(&c,s);
  write_int(&c,0);// horizontal resolution
  write_int(&c,0);// vertical resolution
  write_int(&c,0);//number of colors. 0 -> 1<<bits_per_pixel
  write_int(&c,0);
  // Pixel data
  data+=3*W*H;
  for (int y=0;y<H;++y){
    data-=3*W;
    const unsigned char *s=data;
    unsigned char *p=c;
    for (int x=0;x<W;++x){
      *p++=s[2];
      *p++=s[1];
      *p++=s[0];
      s+=3;
    }
    c+=R;
  }
  *return_size = fs;
  return b;
}


void Fl_Unix_System_Driver::read_int(uchar *c, int& i) {
  i = *c;
  i |= (*(++c))<<8;
  i |= (*(++c))<<16;
  i |= (*(++c))<<24;
}


// turn BMP image FLTK produced by create_bmp() back to Fl_RGB_Image
Fl_RGB_Image *Fl_Unix_System_Driver::own_bmp_to_RGB(char *bmp) {
  int w, h;
  read_int((uchar*)bmp + 18, w);
  read_int((uchar*)bmp + 22, h);
  int R=((3*w+3)/4) * 4; // the number of bytes per row, rounded up to multiple of 4
  bmp +=  54;
  uchar *data = new uchar[w*h*3];
  uchar *p = data;
  for (int i = h-1; i >= 0; i--) {
    char *s = bmp + i * R;
    for (int j = 0; j < w; j++) {
      *p++=s[2];
      *p++=s[1];
      *p++=s[0];
      s+=3;
    }
  }
  Fl_RGB_Image *img = new Fl_RGB_Image(data, w, h, 3);
  img->alloc_array = 1;
  return img;
}
