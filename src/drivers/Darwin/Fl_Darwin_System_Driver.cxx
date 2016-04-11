//
// "$Id$"
//
// Definition of Apple Darwin system driver.
//
// Copyright 1998-2016 by Bill Spitzak and others.
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


#include "../../config_lib.h"
#include "Fl_Darwin_System_Driver.H"
#include <FL/Fl.H>
#include <FL/Fl_File_Browser.H>
#include <FL/filename.H>
#include <FL/Fl_File_Icon.H>
#include <FL/Fl_Preferences.H>
#include <string.h>
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
#include <xlocale.h>
#endif
#include <locale.h>
#include <stdio.h>
#include <dlfcn.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <ApplicationServices/ApplicationServices.h>


extern int fl_mac_os_version;	// the version number of the running Mac OS X

//const char* fl_local_alt   = "\xe2\x8c\xa5\\"; // U+2325 (option key)
const char* fl_local_alt   = "⌥\\"; // U+2325 (option key)
//const char* fl_local_ctrl  = "\xe2\x8c\x83\\"; // U+2303 (up arrowhead)
const char* fl_local_ctrl  = "⌃\\"; // U+2303 (up arrowhead)
//const char* fl_local_meta  = "\xe2\x8c\x98\\"; // U+2318 (place of interest sign)
const char* fl_local_meta  = "⌘\\"; // U+2318 (place of interest sign)
//const char* fl_local_shift = "\xe2\x87\xa7\\"; // U+21E7 (upwards white arrow)
const char* fl_local_shift = "⇧\\"; // U+21E7 (upwards white arrow)


/**
 Creates a driver that manages all screen and display related calls.
 
 This function must be implemented once for every platform.
 */
Fl_System_Driver *Fl_System_Driver::newSystemDriver()
{
  return new Fl_Darwin_System_Driver();
}

Fl_Darwin_System_Driver::Fl_Darwin_System_Driver() {
  if (fl_mac_os_version == 0) fl_mac_os_version = calc_mac_os_version();
}

int Fl_Darwin_System_Driver::single_arg(const char *arg) {
  // The Finder application in MacOS X passes the "-psn_N_NNNNN" option to all apps.
  return (strncmp(arg, "psn_", 4) == 0);
}

int Fl_Darwin_System_Driver::arg_and_value(const char *name, const char *value) {
  // Xcode in MacOS X may pass "-NSDocumentRevisionsDebugMode YES"
  return strcmp(name, "NSDocumentRevisionsDebugMode") == 0;
}

int Fl_Darwin_System_Driver::clocale_printf(FILE *output, const char *format, va_list args) {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (fl_mac_os_version >= 100400) {
    static locale_t postscript_locale = newlocale(LC_NUMERIC_MASK, "C", (locale_t)0);
    return vfprintf_l(output, postscript_locale, format, args);
  }
#endif
  char *saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  int retval = vfprintf(output, format, args);
  setlocale(LC_NUMERIC, saved_locale);
  return retval;
}

/* Returns the address of a Carbon function after dynamically loading the Carbon library if needed.
 Supports old Mac OS X versions that may use a couple of Carbon calls:
 GetKeys used by OS X 10.3 or before (in Fl::get_key())
 PMSessionPageSetupDialog and PMSessionPrintDialog used by 10.4 or before (in Fl_Printer::start_job())
 */
void *Fl_Darwin_System_Driver::get_carbon_function(const char *function_name) {
  static void *carbon = ::dlopen("/System/Library/Frameworks/Carbon.framework/Carbon", RTLD_LAZY);
  return (carbon ? dlsym(carbon, function_name) : NULL);
}

int Fl_Darwin_System_Driver::filename_list(const char *d, dirent ***list, int (*sort)(struct dirent **, struct dirent **) ) {
  int dirlen;
  char *dirloc;
  // Assume that locale encoding is no less dense than UTF-8
  dirlen = strlen(d);
  dirloc = (char *)d;
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_8
  int n = scandir(dirloc, list, 0, (int(*)(const struct dirent**,const struct dirent**))sort);
# else
  int n = scandir(dirloc, list, 0, (int(*)(const void*,const void*))sort);
# endif
  // convert every filename to UTF-8, and append a '/' to all
  // filenames that are directories
  int i;
  char *fullname = (char*)malloc(dirlen+FL_PATH_MAX+3); // Add enough extra for two /'s and a nul
  // Use memcpy for speed since we already know the length of the string...
  memcpy(fullname, d, dirlen+1);
  char *name = fullname + dirlen;
  if (name!=fullname && name[-1]!='/') *name++ = '/';  
  for (i=0; i<n; i++) {
    int newlen;
    dirent *de = (*list)[i];
    int len = strlen(de->d_name);
    newlen = len;
    dirent *newde = (dirent*)malloc(de->d_name - (char*)de + newlen + 2); // Add space for a / and a nul
    // Conversion to UTF-8
    memcpy(newde, de, de->d_name - (char*)de);
    strcpy(newde->d_name, de->d_name);
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


const char *Fl_Darwin_System_Driver::getpwnam(const char *login) {
  struct passwd *pwd;
  pwd = ::getpwnam(login);
  return pwd ? pwd->pw_dir : NULL;
}

int Fl_Darwin_System_Driver::open_uri(const char *uri, char *msg, int msglen)
{
  char	*argv[3];			// Command-line arguments
  argv[0] = (char*)"open";
  argv[1] = (char*)uri;
  argv[2] = (char*)0;
  if (msg) snprintf(msg, msglen, "open %s", uri);
  return run_program("/usr/bin/open", argv, msg, msglen) != 0;
}

int Fl_Darwin_System_Driver::file_browser_load_filesystem(Fl_File_Browser *browser, char *filename, Fl_File_Icon *icon)
{
  // MacOS X and Darwin use getfsstat() system call...
  int			numfs;	// Number of file systems
  struct statfs	*fs;	// Buffer for file system info
  int num_files = 0;
  
  // We always have the root filesystem.
  browser->add("/", icon);
  
  // Get the mounted filesystems...
  numfs = getfsstat(NULL, 0, MNT_NOWAIT);
  if (numfs > 0) {
    // We have file systems, get them...
    fs = new struct statfs[numfs];
    getfsstat(fs, sizeof(struct statfs) * numfs, MNT_NOWAIT);
    
    // Add filesystems to the list...
    for (int i = 0; i < numfs; i ++) {
      // Ignore "/", "/dev", and "/.vol"...
      if (fs[i].f_mntonname[1] && strcmp(fs[i].f_mntonname, "/dev") &&
          strcmp(fs[i].f_mntonname, "/.vol")) {
        snprintf(filename, sizeof(filename), "%s/", fs[i].f_mntonname);
        browser->add(filename, icon);
      }
      num_files ++;
    }
    
    // Free the memory used for the file system info array...
    delete[] fs;
  }
  return num_files;
}

void Fl_Darwin_System_Driver::newUUID(char *uuidBuffer)
{
  CFUUIDRef theUUID = CFUUIDCreate(NULL);
  CFUUIDBytes b = CFUUIDGetUUIDBytes(theUUID);
  sprintf(uuidBuffer, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
          b.byte0, b.byte1, b.byte2, b.byte3, b.byte4, b.byte5, b.byte6, b.byte7,
          b.byte8, b.byte9, b.byte10, b.byte11, b.byte12, b.byte13, b.byte14, b.byte15);
  CFRelease(theUUID);
}

char *Fl_Darwin_System_Driver::preference_rootnode(Fl_Preferences *prefs, Fl_Preferences::Root root,
                                                  const char *vendor, const char *application)
{
  static char filename[ FL_PATH_MAX ];
  // TODO: verify that this is the Apple sanctioned way of finding these folders
  // (On MSWindows, this frequently leads to issues with internationalized systems)
  // Carbon: err = FindFolder( kLocalDomain, kPreferencesFolderType, 1, &spec.vRefNum, &spec.parID );
  switch (root) {
    case Fl_Preferences::SYSTEM:
      strcpy(filename, "/Library/Preferences");
      break;
    case Fl_Preferences::USER:
      sprintf(filename, "%s/Library/Preferences", getenv("HOME"));
      break;
  }
  snprintf(filename + strlen(filename), sizeof(filename) - strlen(filename),
           "/%s/%s.prefs", vendor, application);
  return filename;
}

void *Fl_Darwin_System_Driver::dlopen(const char *filename) {
  return ::dlopen(filename, RTLD_LAZY);
}

int Fl_Darwin_System_Driver::file_type(const char *filename)
{
  int filetype;
  struct stat fileinfo;		// Information on file
  if (!stat(filename, &fileinfo))
  {
    if (S_ISDIR(fileinfo.st_mode))
      filetype = Fl_File_Icon::DIRECTORY;
#  ifdef S_ISFIFO
    else if (S_ISFIFO(fileinfo.st_mode))
      filetype = Fl_File_Icon::FIFO;
#  endif // S_ISFIFO
#  if defined(S_ISCHR) && defined(S_ISBLK)
    else if (S_ISCHR(fileinfo.st_mode) || S_ISBLK(fileinfo.st_mode))
      filetype = Fl_File_Icon::DEVICE;
#  endif // S_ISCHR && S_ISBLK
#  ifdef S_ISLNK
    else if (S_ISLNK(fileinfo.st_mode))
      filetype = Fl_File_Icon::LINK;
#  endif // S_ISLNK
    else
      filetype = Fl_File_Icon::PLAIN;
  }
  else
    filetype = Fl_File_Icon::PLAIN;
  return filetype;
}

//
// End of "$Id$".
//
