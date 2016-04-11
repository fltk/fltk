//
// "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $"
//
// Definition of Posix system driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include "Fl_X11_System_Driver.H"
#include <FL/Fl_File_Browser.H>

#include <X11/Xlib.h>
#include <locale.h>
#include <time.h>

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
                 int (*compar)(struct dirent **, struct dirent **));
}
#endif

// Pointers you can use to change FLTK to another language.
// Note: Similar pointers are defined in FL/fl_ask.H and src/fl_ask.cxx
const char* fl_local_alt   = "Alt";
const char* fl_local_ctrl  = "Ctrl";
const char* fl_local_meta  = "Meta";
const char* fl_local_shift = "Shift";

/**
 Creates a driver that manages all screen and display related calls.
 
 This function must be implemented once for every platform.
 */
Fl_System_Driver *Fl_System_Driver::newSystemDriver()
{
  return new Fl_X11_System_Driver();
}


int Fl_X11_System_Driver::clocale_printf(FILE *output, const char *format, va_list args) {
  char *saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  int retval = vfprintf(output, format, args);
  setlocale(LC_NUMERIC, saved_locale);
  return retval;
}


// Find a program in the path...
static char *path_find(const char *program, char *filename, int filesize) {
  const char	*path;			// Search path
  char		*ptr,			// Pointer into filename
		*end;			// End of filename buffer
  
  
  if ((path = getenv("PATH")) == NULL) path = "/bin:/usr/bin";
  
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


int Fl_X11_System_Driver::open_uri(const char *uri, char *msg, int msglen)
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
  char	command[FL_PATH_MAX],		// Command to run...
  *argv[4],			// Command-line arguments
  remote[1024];			// Remote-mode command...
  const char * const *commands;		// Array of commands to check...
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


int Fl_X11_System_Driver::file_browser_load_filesystem(Fl_File_Browser *browser, char *filename, Fl_File_Icon *icon)
{
  int num_files = 0;
#if defined(_AIX)
  // AIX don't write the mounted filesystems to a file like '/etc/mnttab'.
  // But reading the list of mounted filesystems from the kernel is possible:
  // http://publib.boulder.ibm.com/infocenter/pseries/v5r3/topic/com.ibm.aix.basetechref/doc/basetrf1/mntctl.htm
  int res = -1, len;
  char *list = NULL, *name;
  struct vmount *vp;
  
  // We always have the root filesystem
  add("/", icon);
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
        for (i = 0, vp = (struct vmount *) list; i < res; ++i) {
          name = (char *) vp + vp->vmt_data[VMT_STUB].vmt_off;
          strlcpy(filename, name, sizeof(filename));
          // Skip the already added root filesystem
          if (strcmp("/", filename) != 0) {
            strlcat(filename, "/", sizeof(filename));
            browser->add(filename, icon);
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
#  ifdef HAVE_PTHREAD
  // Lock mutex for thread safety
  if (!pthread_mutex_lock(&getvfsstat_mutex)) {
#  endif  // HAVE_PTHREAD
    // Get list of statvfs structures
    res = getmntinfo(&list, ST_WAIT);
    if(0 < res) {
      for (i = 0;  i < res; ++i) {
        strlcpy(filename, list[i].f_mntonname, sizeof(filename));
        // Skip the already added root filesystem
        if (strcmp("/", filename) != 0) {
          strlcat(filename, "/", sizeof(filename));
          browser->add(filename, icon);
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
  FILE	*mtab;		// /etc/mtab or /etc/mnttab file
  char	line[FL_PATH_MAX];	// Input line
  
  //
  // Open the file that contains a list of mounted filesystems...
  //
  
  mtab = fopen("/etc/mnttab", "r");	// Fairly standard
  if (mtab == NULL)
    mtab = fopen("/etc/mtab", "r");	// More standard
  if (mtab == NULL)
    mtab = fopen("/etc/fstab", "r");	// Otherwise fallback to full list
  if (mtab == NULL)
    mtab = fopen("/etc/vfstab", "r");	// Alternate full list file
  
  if (mtab != NULL)
  {
    while (fgets(line, sizeof(line), mtab) != NULL)
    {
      if (line[0] == '#' || line[0] == '\n')
        continue;
      if (sscanf(line, "%*s%4095s", filename) != 1)
        continue;
      
      // Add a trailing slash (except for the root filesystem)
      if (strcmp("/", filename) != 0) {
        strlcat(filename, "/", sizeof(filename));
      }
      
      //        printf("Fl_File_Browser::load() - adding \"%s\" to list...\n", filename);
      browser->add(filename, icon);
      num_files ++;
    }
    
    fclose(mtab);
  } else {
    // Every Unix has a root filesystem '/'.
    // This last stage fallback ensures that the user don't get an empty
    // window after requesting filesystem list.
    browser->add("/", icon);
  }
#endif // _AIX || ...
  return num_files;
}

void Fl_X11_System_Driver::newUUID(char *uuidBuffer)
{
  // warning Unix implementation of Fl_Preferences::newUUID() incomplete!
  // #include <uuid/uuid.h>
  // void uuid_generate(uuid_t out);
  unsigned char b[16];
  time_t t = time(0);			// first 4 byte
  b[0] = (unsigned char)t;
  b[1] = (unsigned char)(t>>8);
  b[2] = (unsigned char)(t>>16);
  b[3] = (unsigned char)(t>>24);
  int r = rand(); 			// four more bytes
  b[4] = (unsigned char)r;
  b[5] = (unsigned char)(r>>8);
  b[6] = (unsigned char)(r>>16);
  b[7] = (unsigned char)(r>>24);
  unsigned long a = (unsigned long)&t;	// four more bytes
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
  char name[80];			// last four bytes
  gethostname(name, 79);
  memcpy(b+12, name, 4);
  sprintf(uuidBuffer, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
          b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7],
          b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
}

char *Fl_X11_System_Driver::preference_rootnode(Fl_Preferences *prefs, Fl_Preferences::Root root, const char *vendor,
                                                const char *application)
{
  static char filename[ FL_PATH_MAX ]; filename[0] = 0;
  const char *e;
  switch (root) {
    case Fl_Preferences::USER:
      if ((e = ::getenv("HOME")) != NULL) {
        strlcpy(filename, e, sizeof(filename));
        
        if (filename[strlen(filename)-1] != '/') {
          strlcat(filename, "/.fltk/", sizeof(filename));
        } else {
          strlcat(filename, ".fltk/", sizeof(filename));
        }
        break;
      }
    case Fl_Preferences::SYSTEM:
      strcpy(filename, "/etc/fltk/");
      break;
  }
  snprintf(filename + strlen(filename), sizeof(filename) - strlen(filename),
           "%s/%s.prefs", vendor, application);
  return filename;
}

void Fl_X11_System_Driver::display_arg(const char *arg) {
  Fl::display(arg);
}

int Fl_X11_System_Driver::XParseGeometry(const char* string, int* x, int* y,
                                         unsigned int* width, unsigned int* height) {
  return ::XParseGeometry(string, x, y, width, height);
}

int Fl_X11_System_Driver::filename_list(const char *d, dirent ***list, int (*sort)(struct dirent **, struct dirent **) ) {
  int dirlen;
  char *dirloc;
  
  // Assume that locale encoding is no less dense than UTF-8
  dirlen = strlen(d);
  dirloc = (char *)malloc(dirlen + 1);
  fl_utf8to_mb(d, dirlen, dirloc, dirlen + 1);
  
#ifndef HAVE_SCANDIR
  // This version is when we define our own scandir
  int n = fl_scandir(dirloc, list, 0, sort);
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

//
// End of "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $".
//
