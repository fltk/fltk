//
// "$Id$"
//
// Definition of Apple Darwin system driver.
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

#include "Fl_Darwin_System_Driver.H"
#include <FL/platform.H>
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

// This key table is used for the Darwin system driver. It is defined here
// "static" and assigned in the constructor to avoid static initialization
// race conditions. It is used in fl_shortcut.cxx.
//
// This table must be in numeric order by fltk (X) keysym number:

Fl_System_Driver::Keyname darwin_key_table[] = {
  //              v - this column may contain UTF-8 characters
  {' ',           "Space"},
  {FL_BackSpace,  "\xe2\x8c\xab"}, // erase to the left
  {FL_Tab,        "\xe2\x87\xa5"}, // rightwards arrow to bar
  {0xff0b,        "\xe2\x8c\xa6"}, // erase to the right
  {FL_Enter,      "\xe2\x86\xa9"}, // leftwards arrow with hook
  {FL_Pause,      "Pause"},
  {FL_Scroll_Lock, "Scroll_Lock"},
  {FL_Escape,     "\xe2\x90\x9b"},
  {FL_Home,       "\xe2\x86\x96"}, // north west arrow
  {FL_Left,       "\xe2\x86\x90"}, // leftwards arrow
  {FL_Up,         "\xe2\x86\x91"}, // upwards arrow
  {FL_Right,      "\xe2\x86\x92"}, // rightwards arrow
  {FL_Down,       "\xe2\x86\x93"}, // downwards arrow
  {FL_Page_Up,    "\xe2\x87\x9e"}, // upwards arrow with double stroke
  {FL_Page_Down,  "\xe2\x87\x9f"}, // downwards arrow with double stroke
  {FL_End,        "\xe2\x86\x98"}, // south east arrow
  {FL_Print,      "Print"},
  {FL_Insert,     "Insert"},
  {FL_Menu,       "Menu"},
  {FL_Num_Lock,   "Num_Lock"},
  {FL_KP_Enter,   "\xe2\x8c\xa4"}, // up arrow head between two horizontal bars
  {FL_Shift_L,    "Shift_L"},
  {FL_Shift_R,    "Shift_R"},
  {FL_Control_L,  "Control_L"},
  {FL_Control_R,  "Control_R"},
  {FL_Caps_Lock,  "\xe2\x87\xaa"}, // upwards white arrow from bar
  {FL_Meta_L,     "Meta_L"},
  {FL_Meta_R,     "Meta_R"},
  {FL_Alt_L,      "Alt_L"},
  {FL_Alt_R,      "Alt_R"},
  {FL_Delete,     "\xe2\x8c\xa7"}  // x in a rectangle box
};

const char *Fl_Darwin_System_Driver::shift_name() {
  return "⇧\\"; // "\xe2\x87\xa7\\"; // U+21E7 (upwards white arrow)
}
const char *Fl_Darwin_System_Driver::meta_name() {
  return "⌘\\"; // "\xe2\x8c\x98\\"; // U+2318 (place of interest sign)
}
const char *Fl_Darwin_System_Driver::alt_name() {
  return "⌥\\"; // "\xe2\x8c\xa5\\"; // U+2325 (option key)
}
const char *Fl_Darwin_System_Driver::control_name() {
  return "⌃\\"; // "\xe2\x8c\x83\\"; // U+2303 (up arrowhead)
}

/*
 Creates a driver that manages all system related calls.
 
 This function must be implemented once for every platform.
 */
Fl_System_Driver *Fl_System_Driver::newSystemDriver()
{
  return new Fl_Darwin_System_Driver();
}

Fl_Darwin_System_Driver::Fl_Darwin_System_Driver() : Fl_Posix_System_Driver() {
  if (fl_mac_os_version == 0) fl_mac_os_version = calc_mac_os_version();
  // initialize key table
  key_table = darwin_key_table;
  key_table_size = sizeof(darwin_key_table)/sizeof(*darwin_key_table);
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
 PMSessionPageSetupDialog and PMSessionPrintDialog used by 10.4 or before (in Fl_Printer::begin_job())
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


int Fl_Darwin_System_Driver::open_uri(const char *uri, char *msg, int msglen)
{
  char	*argv[3];			// Command-line arguments
  argv[0] = (char*)"open";
  argv[1] = (char*)uri;
  argv[2] = (char*)0;
  if (msg) snprintf(msg, msglen, "open %s", uri);
  return run_program("/usr/bin/open", argv, msg, msglen) != 0;
}

int Fl_Darwin_System_Driver::file_browser_load_filesystem(Fl_File_Browser *browser, char *filename, int lname, Fl_File_Icon *icon)
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
        snprintf(filename, lname, "%s/", fs[i].f_mntonname);
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
  // (On Windows, this frequently leads to issues with internationalized systems)
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

/*
 * returns pointer to the filename, or null if name ends with ':'
 */
const char *Fl_Darwin_System_Driver::filename_name( const char *name )
{
  const char *p, *q;
  if (!name) return (0);
  for ( p = q = name ; *p ; ) {
    if ( ( p[0] == ':' ) && ( p[1] == ':' ) ) {
      q = p+2;
      p++;
    }
    else if (p[0] == '/') {
      q = p + 1;
    }
    p++;
  }
  return q;
}

//
// End of "$Id$".
//
