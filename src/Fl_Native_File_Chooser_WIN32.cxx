// "$Id$"
//
// FLTK native OS file chooser widget
//
// Copyright 1998-2014 by Bill Spitzak and others.
// Copyright 2004 Greg Ercolano.
// API changes + filter improvements by Nathan Vander Wilt 2005
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

// Any application to multi-folder implementation:
//     http://www.codeproject.com/dialog/selectfolder.asp
//

#ifndef FL_DOXYGEN		// PREVENT DOXYGEN'S USE OF THIS FILE
#include <FL/Enumerations.H>

#if FLTK_ABI_VERSION < 10304
#define _ofn_ptr (&_ofn)
#define _binf_ptr (&_binf)
#endif

#  include <stdlib.h>		// malloc
#  include <stdio.h>		// sprintf
#include <wchar.h>

#include "Fl_Native_File_Chooser_common.cxx"		// strnew/strfree/strapp/chrcat

#define FNFC_MAX_PATH 32768		// XXX: MAX_PATH under win32 is 260, too small for modern use

#include <FL/Fl_Native_File_Chooser.H>
static LPCWSTR utf8towchar(const char *in);
static char *wchartoutf8(LPCWSTR in);

#include <FL/x.H> // for fl_open_display

#define LCURLY_CHR	'{'
#define RCURLY_CHR	'}'
#define LBRACKET_CHR	'['
#define RBRACKET_CHR	']'

// STATIC: PRINT WINDOWS 'DOUBLE NULL' STRING (DEBUG)
#ifdef DEBUG
#include <stdio.h>
static void dnullprint(char *wp) {
  if ( ! wp ) return;
  for ( int t=0; true; t++ ) {
    if ( wp[t] == '\0' && wp[t+1] == '\0' ) {
      printf("\\0\\0");
      fflush(stdout);
      return;
    } else if ( wp[t] == '\0' ) {
      printf("\\0");
    } else {
      printf("%c",wp[t]);
    }
  }
}
#endif

// RETURN LENGTH OF DOUBLENULL STRING
//    Includes single nulls in count, excludes trailing doublenull.
//
//         1234 567
//         |||/\|||
//    IN: "one\0two\0\0"
//   OUT: 7
//
static int dnulllen(const char *wp) {
  int len = 0;
  while ( ! ( *(wp+0) == 0 && *(wp+1) == 0 ) ) {
    ++wp;
    ++len;
  }
  return(len);
}

// STATIC: Append a string to another, leaving terminated with DOUBLE NULL.
//     Automatically handles extending length of string.
//     wp can be NULL (a new wp will be allocated and initialized).
//     string must be NULL terminated.
//     The pointer wp may be modified on return.
//
static void dnullcat(char*&wp, const char *string, int n = -1 ) {
  //DEBUG printf("DEBUG: dnullcat IN: <"); dnullprint(wp); printf(">\n");
  size_t inlen = ( n < 0 ) ? strlen(string) : n;
  if ( ! wp ) {
    wp = new char[inlen + 4];
    *(wp+0) = '\0';
    *(wp+1) = '\0';
  } else {
    int wplen = dnulllen(wp);
    // Make copy of wp into larger buffer
    char *tmp = new char[wplen + inlen + 4];
    memcpy(tmp, wp, wplen+2);	// copy of wp plus doublenull
    delete[] wp;		// delete old wp
    wp = tmp;			// use new copy
    //DEBUG printf("DEBUG: dnullcat COPY: <"); dnullprint(wp); printf("> (wplen=%d)\n", wplen);
  }

  // Find end of double null string
  //     *wp2 is left pointing at second null.
  //
  char *wp2 = wp;
  if ( *(wp2+0) != '\0' && *(wp2+1) != '\0' ) {
    for ( ; 1; wp2++ ) {
      if ( *(wp2+0) == '\0' && *(wp2+1) == '\0' ) {
        wp2++;
        break;
      }
    }
  }

  memcpy(wp2, string, inlen); // use memcpy to avoid compiler warning

  // Leave string double-null terminated
  *(wp2+inlen+0) = '\0';
  *(wp2+inlen+1) = '\0';
  //DEBUG printf("DEBUG: dnullcat OUT: <"); dnullprint(wp); printf(">\n\n");
}

// CTOR
Fl_Native_File_Chooser::Fl_Native_File_Chooser(int val) {
  _btype           = val;
  _options         = NO_OPTIONS;
#if FLTK_ABI_VERSION >= 10304
  _ofn_ptr = new OPENFILENAMEW;
  _binf_ptr = new BROWSEINFOW;
  _wpattern = 0;
#endif
  memset((void*)_ofn_ptr, 0, sizeof(OPENFILENAMEW));
  _ofn_ptr->lStructSize = sizeof(OPENFILENAMEW);
  _ofn_ptr->hwndOwner   = NULL;
  memset((void*)_binf_ptr, 0, sizeof(BROWSEINFOW));
  _pathnames       = NULL;
  _tpathnames      = 0;
  _directory       = NULL;
  _title           = NULL;
  _filter          = NULL;
  _parsedfilt      = NULL;
  _nfilters        = 0;
  _preset_file     = NULL;
  _errmsg          = NULL;
}

// DTOR
Fl_Native_File_Chooser::~Fl_Native_File_Chooser() {
  //_pathnames                // managed by clear_pathnames()
  //_tpathnames               // managed by clear_pathnames()
  _directory   = strfree(_directory);
  _title       = strfree(_title);
  _filter      = strfree(_filter);
  //_parsedfilt               // managed by clear_filters()
  //_nfilters                 // managed by clear_filters()
  _preset_file = strfree(_preset_file);
  _errmsg      = strfree(_errmsg);
  clear_filters();
  clear_pathnames();
  ClearOFN();
  ClearBINF();
#if FLTK_ABI_VERSION >= 10304
  delete _binf_ptr;
  delete _ofn_ptr;
  if ( _wpattern ) delete[] _wpattern;
#endif
}

// SET TYPE OF BROWSER
void Fl_Native_File_Chooser::type(int val) {
  _btype = val;
}

// GET TYPE OF BROWSER
int Fl_Native_File_Chooser::type() const {
  return( _btype );
}

// SET OPTIONS
void Fl_Native_File_Chooser::options(int val) {
  _options = val;
}

// GET OPTIONS
int Fl_Native_File_Chooser::options() const {
  return(_options);
}

// PRIVATE: SET ERROR MESSAGE
void Fl_Native_File_Chooser::errmsg(const char *val) {
  _errmsg = strfree(_errmsg);
  _errmsg = strnew(val);
}

// FREE PATHNAMES ARRAY, IF IT HAS ANY CONTENTS
void Fl_Native_File_Chooser::clear_pathnames() {
  if ( _pathnames ) {
    while ( --_tpathnames >= 0 ) {
      _pathnames[_tpathnames] = strfree(_pathnames[_tpathnames]);
    }
    delete[] _pathnames;
    _pathnames = NULL;
  }
  _tpathnames = 0;
}

// SET A SINGLE PATHNAME
void Fl_Native_File_Chooser::set_single_pathname(const char *s) {
  clear_pathnames();
  _pathnames = new char*[1];
  _pathnames[0] = strnew(s);
  _tpathnames = 1;
}

// ADD PATHNAME TO EXISTING ARRAY
void Fl_Native_File_Chooser::add_pathname(const char *s) {
  if ( ! _pathnames ) {
    // Create first element in array
    ++_tpathnames;
    _pathnames = new char*[_tpathnames];
  } else {
    // Grow array by 1
    char **tmp = new char*[_tpathnames+1];		// create new buffer
    memcpy((void*)tmp, (void*)_pathnames,
		       sizeof(char*)*_tpathnames);	// copy old
    delete[] _pathnames;				// delete old
    _pathnames = tmp;					// use new
    ++_tpathnames;
  }
  _pathnames[_tpathnames-1] = strnew(s);
}

// FREE A PIDL (Pointer to IDentity List)
static void FreePIDL(LPITEMIDLIST pidl) {
  IMalloc *imalloc = NULL;
  if ( SUCCEEDED(SHGetMalloc(&imalloc)) ) {
    imalloc->Free(pidl);
    imalloc->Release();
    imalloc = NULL;
  }
}

// CLEAR MICROSOFT OFN (OPEN FILE NAME) CLASS
void Fl_Native_File_Chooser::ClearOFN() {
  // Free any previously allocated lpstrFile before zeroing out _ofn_ptr
  if ( _ofn_ptr->lpstrFile ) {
    delete[] _ofn_ptr->lpstrFile;
    _ofn_ptr->lpstrFile = NULL;
  }
  if ( _ofn_ptr->lpstrInitialDir ) {
    delete[] (TCHAR*) _ofn_ptr->lpstrInitialDir; //msvc6 compilation fix
    _ofn_ptr->lpstrInitialDir = NULL;
  }
  _ofn_ptr->lpstrFilter = NULL;		// (deleted elsewhere)
  int temp = _ofn_ptr->nFilterIndex;		// keep the filter_value
  memset((void*)_ofn_ptr, 0, sizeof(OPENFILENAMEW));
  _ofn_ptr->lStructSize  = sizeof(OPENFILENAMEW);
  _ofn_ptr->nFilterIndex = temp;
}

// CLEAR MICROSOFT BINF (BROWSER INFO) CLASS
void Fl_Native_File_Chooser::ClearBINF() {
  if ( _binf_ptr->pidlRoot ) {
    FreePIDL((ITEMIDLIST*)_binf_ptr->pidlRoot);
    _binf_ptr->pidlRoot = NULL;
  }
  memset((void*)_binf_ptr, 0, sizeof(BROWSEINFOW));
}

// CONVERT WINDOWS BACKSLASHES TO UNIX FRONTSLASHES
void Fl_Native_File_Chooser::Win2Unix(char *s) {
  for ( ; *s; s++ )
    if ( *s == '\\' ) *s = '/';
}

// CONVERT UNIX FRONTSLASHES TO WINDOWS BACKSLASHES
void Fl_Native_File_Chooser::Unix2Win(char *s) {
  for ( ; *s; s++ )
    if ( *s == '/' ) *s = '\\';
}

// SAVE THE CURRENT WORKING DIRECTORY
//     Returns a malloc()ed copy of the cwd that can
//     later be freed with RestoreCWD(). May return 0 on error.
//
static char *SaveCWD() {
  char *thecwd = 0;
  DWORD thecwdsz = GetCurrentDirectory(0,0);
  if ( thecwdsz > 0 ) {
    thecwd = (char*)malloc(thecwdsz);
    if (GetCurrentDirectory(thecwdsz, thecwd) == 0 ) {
      free(thecwd); thecwd = 0;
    }
  }
  return thecwd;
}

// RESTORES THE CWD SAVED BY SaveCWD(), FREES STRING
//    Always returns NULL (string was freed).
//
static void RestoreCWD(char *thecwd) {
  if ( !thecwd ) return;
  SetCurrentDirectory(thecwd);
  free(thecwd);
}

// SHOW FILE BROWSER
int Fl_Native_File_Chooser::showfile() {
  ClearOFN();
  clear_pathnames();
  size_t fsize = FNFC_MAX_PATH;
  _ofn_ptr->Flags |= OFN_NOVALIDATE;	// prevent disabling of front slashes
  _ofn_ptr->Flags |= OFN_HIDEREADONLY;	// hide goofy readonly flag
  // USE NEW BROWSER
  _ofn_ptr->Flags |= OFN_EXPLORER;	// use newer explorer windows
  _ofn_ptr->Flags |= OFN_ENABLESIZING;	// allow window to be resized (hey, why not?)
  _ofn_ptr->Flags |= OFN_NOCHANGEDIR;	// XXX: docs say ineffective on XP/2K/NT, but set it anyway..

  switch ( _btype ) {
    case BROWSE_DIRECTORY:
    case BROWSE_MULTI_DIRECTORY:
    case BROWSE_SAVE_DIRECTORY:
      abort();				// never happens: handled by showdir()
    case BROWSE_FILE:
      break;
    case BROWSE_MULTI_FILE:
      _ofn_ptr->Flags |= OFN_ALLOWMULTISELECT;
      break;
    case BROWSE_SAVE_FILE:
      if ( options() & SAVEAS_CONFIRM && type() == BROWSE_SAVE_FILE ) {
	  _ofn_ptr->Flags |= OFN_OVERWRITEPROMPT;
      }
      break;
  }
  // SPACE FOR RETURNED FILENAME
  _ofn_ptr->lpstrFile    = new WCHAR[fsize];
  _ofn_ptr->nMaxFile     = (DWORD) fsize-1;
  _ofn_ptr->lpstrFile[0] = 0;
  _ofn_ptr->lpstrFile[1] = 0;		// dnull
  // PARENT WINDOW
  _ofn_ptr->hwndOwner = GetForegroundWindow();
  // DIALOG TITLE
  if (_title) {
    static WCHAR wtitle[200];
    wcsncpy(wtitle, utf8towchar(_title), 200);
    wtitle[200-1] = 0;
    _ofn_ptr->lpstrTitle =  wtitle;
  } else {
    _ofn_ptr->lpstrTitle = NULL;
  }
  // FILTER
  if (_parsedfilt != NULL) {	// to convert a null-containing char string into a widechar string
#if FLTK_ABI_VERSION >= 10304
    // NEW
    if ( !_wpattern ) _wpattern = new WCHAR[FNFC_MAX_PATH];
#else
    // OLD
    static WCHAR _wpattern[FNFC_MAX_PATH];	// yuck -- replace with managed class member
#endif
    const char *p = _parsedfilt;
    while(*(p + strlen(p) + 1) != 0) p += strlen(p) + 1;
    p += strlen(p) + 2;
    MultiByteToWideChar(CP_UTF8, 0, _parsedfilt, (int) (p - _parsedfilt), _wpattern, FNFC_MAX_PATH);
    _ofn_ptr->lpstrFilter = _wpattern;
  } else {
    _ofn_ptr->lpstrFilter = NULL;
  }
  // PRESET FILE
  //     If set, supercedes _directory. See KB Q86920 for details
  //     XXX: this doesn't preselect the item in the listview.. why?
  //
  if ( _preset_file ) {
    size_t len = strlen(_preset_file);
    if ( len >= _ofn_ptr->nMaxFile ) {
      char msg[80];
      sprintf(msg, "preset_file() filename is too long: %ld is >=%ld", (long)len, (long)fsize);
      return(-1);
    }
    wcscpy(_ofn_ptr->lpstrFile, utf8towchar(_preset_file));
    // Unix2Win(_ofn_ptr->lpstrFile);
    len = wcslen(_ofn_ptr->lpstrFile);
    _ofn_ptr->lpstrFile[len+0] = 0;	// multiselect needs dnull
    _ofn_ptr->lpstrFile[len+1] = 0;
  }
  if ( _directory ) {
    // PRESET DIR
    //     XXX: See KB Q86920 for doc bug:
    //     http://support.microsoft.com/default.aspx?scid=kb;en-us;86920
    //
    _ofn_ptr->lpstrInitialDir    = new WCHAR[FNFC_MAX_PATH];
    wcscpy((WCHAR *)_ofn_ptr->lpstrInitialDir, utf8towchar(_directory));
    // Unix2Win((char*)_ofn_ptr->lpstrInitialDir);
  }
  // SAVE THE CURRENT DIRECTORY
  //     See above warning (XXX) for OFN_NOCHANGEDIR
  //
  char *save_cwd = SaveCWD();		// must be freed with RestoreCWD()
  // OPEN THE DIALOG WINDOW
  int err;
  if ( _btype == BROWSE_SAVE_FILE ) {
    err = GetSaveFileNameW(_ofn_ptr);
  } else {
    err = GetOpenFileNameW(_ofn_ptr);
  }
  // GET EXTENDED ERROR
  int exterr = CommDlgExtendedError();
  // RESTORE CURRENT DIRECTORY
  RestoreCWD(save_cwd); save_cwd = 0;	// also frees save_cwd
  // ERROR OR CANCEL?
  if ( err == 0 ) {
    if ( exterr == 0 ) return(1);	// user hit cancel
    // Otherwise, an error occurred..
    char msg[80];
    sprintf(msg, "CommDlgExtendedError() code=%d", exterr);
    errmsg(msg);
    return(-1);
  }
  // PREPARE PATHNAMES FOR RETURN
  switch ( _btype ) {
    case BROWSE_FILE:
    case BROWSE_SAVE_FILE:
      set_single_pathname(wchartoutf8(_ofn_ptr->lpstrFile));
      // Win2Unix(_pathnames[_tpathnames-1]);
      break;
    case BROWSE_MULTI_FILE: {
      // EXTRACT MULTIPLE FILENAMES
      const WCHAR *dirname = _ofn_ptr->lpstrFile;
      size_t dirlen = wcslen(dirname);
      if ( dirlen > 0 ) {
	// WALK STRING SEARCHING FOR 'DOUBLE-NULL'
	//     eg. "/dir/name\0foo1\0foo2\0foo3\0\0"
	//
	char pathname[FNFC_MAX_PATH];
	for ( const WCHAR *s = dirname + dirlen + 1;
		 *s; s+= (wcslen(s)+1)) {
		strcpy(pathname, wchartoutf8(dirname));
		strcat(pathname, "\\");
		strcat(pathname, wchartoutf8(s));
		add_pathname(pathname);
	}
      }
      // XXX
      //    Work around problem where pasted forward-slash pathname
      //    into the file browser causes new "Explorer" interface
      //    not to grok forward slashes, passing back as a 'filename'..!
      //
      if ( _tpathnames == 0 ) {
	add_pathname(wchartoutf8(dirname));
	// Win2Unix(_pathnames[_tpathnames-1]);
      }
      break;
    }
    case BROWSE_DIRECTORY:
    case BROWSE_MULTI_DIRECTORY:
    case BROWSE_SAVE_DIRECTORY:
      abort();			// never happens: handled by showdir()
  }
  return(0);
}

// Used by SHBrowseForFolder(), sets initial selected dir.
// Ref: Usenet: microsoft.public.vc.mfc, Dec 8 2000, 1:38p David Lowndes
//              Subject: How to specify to select an initial folder .."
//
static int CALLBACK Dir_CB(HWND win, UINT msg, LPARAM param, LPARAM data) {
  switch (msg) {
    case BFFM_INITIALIZED:
      if (data) ::SendMessageW(win, BFFM_SETSELECTIONW, TRUE, data);
      break;
    case BFFM_SELCHANGED:
      TCHAR path[FNFC_MAX_PATH];
      if ( SHGetPathFromIDList((ITEMIDLIST*)param, path) ) {
	::SendMessage(win, BFFM_ENABLEOK, 0, 1);
      } else {
	// disable ok button if not a path
	::SendMessage(win, BFFM_ENABLEOK, 0, 0);
      }
      break;
    case BFFM_VALIDATEFAILED:
      // we could pop up an annoying message here.
      // also needs set ulFlags |= BIF_VALIDATE
      break;
    default:
      break;
  }
  return(0);
}

// SHOW DIRECTORY BROWSER
int Fl_Native_File_Chooser::showdir() {
  // initialize OLE only once
  fl_open_display();		// init needed by BIF_USENEWUI
  ClearBINF();
  clear_pathnames();
  // PARENT WINDOW
  _binf_ptr->hwndOwner = GetForegroundWindow();
  // DIALOG TITLE
  //_binf_ptr->lpszTitle = _title ? _title : NULL;
  if (_title) {
    static WCHAR wtitle[256];
    wcsncpy(wtitle, utf8towchar(_title), 256);
    wtitle[255] = 0;
    _binf_ptr->lpszTitle =  wtitle;
  } else {
    _binf_ptr->lpszTitle = NULL;
  }

  // FLAGS
  _binf_ptr->ulFlags = 0; 		// initialize

  // TBD: make sure matches to runtime system, if need be.
  //(what if _WIN32_IE doesn't match system? does the program not run?)
  //
  // TBD: match all 3 types of directories
  //
  // NOTE: *Don't* use BIF_SHAREABLE. It /disables/ mapped network shares
  //       from being visible in BROWSE_DIRECTORY mode. Walter Garm's comments:
  //
  //       --- Garms, Walter (GE EntSol, Security) wrote:
  //       With your help I was able to solve the problem of the network drives.
  //       For Version 6.0, at least, the BIF_SHAREABLE flag seems to have the
  //       opposite sense:  With BIF_SHAREABLE not set I see the mapped network
  //       drives, and with BIF_SHAREABLE set I do not.
  //       ---

#if defined(BIF_NONEWFOLDERBUTTON)				// Version 6.0
  if ( _btype == BROWSE_DIRECTORY ) _binf_ptr->ulFlags |= BIF_NONEWFOLDERBUTTON;
  _binf_ptr->ulFlags |= BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
#elif defined(BIF_USENEWUI)					// Version 5.0
  if ( _btype == BROWSE_DIRECTORY ) _binf_ptr->ulFlags |= BIF_EDITBOX;
  else if ( _btype == BROWSE_SAVE_DIRECTORY ) _binf_ptr->ulFlags |= BIF_USENEWUI;
  _binf_ptr->ulFlags |= BIF_RETURNONLYFSDIRS;
#elif defined(BIF_EDITBOX)					// Version 4.71
  _binf_ptr->ulFlags |= BIF_RETURNONLYFSDIRS | BIF_EDITBOX;
#else								// Version Old
  _binf_ptr->ulFlags |= BIF_RETURNONLYFSDIRS;
#endif

  // BUFFER
  //char displayname[FNFC_MAX_PATH];
  WCHAR displayname[FNFC_MAX_PATH];
  _binf_ptr->pszDisplayName = displayname;

  // PRESET DIR
  WCHAR presetname[FNFC_MAX_PATH];
  if ( _directory ) {
    // Unix2Win(presetname);
    wcsncpy(presetname, utf8towchar(_directory), FNFC_MAX_PATH);
    presetname[FNFC_MAX_PATH-1] = 0;
    _binf_ptr->lParam = (LPARAM)presetname;
  }
  else _binf_ptr->lParam = 0;
  _binf_ptr->lpfn = Dir_CB;
  // OPEN BROWSER
  LPITEMIDLIST pidl = SHBrowseForFolderW(_binf_ptr);
  // CANCEL?
  if ( pidl == NULL ) return(1);

  // GET THE PATHNAME(S) THE USER SELECTED
  // TBD: expand NetHood shortcuts from this PIDL??
  // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/functions/shbrowseforfolder.asp

  WCHAR path[FNFC_MAX_PATH];
  if ( SHGetPathFromIDListW(pidl, path) ) {
    // Win2Unix(path);
    //add_pathname(path);
    add_pathname(wchartoutf8(path));
  }
  FreePIDL(pidl);
  if ( !wcslen(path) ) return(1);             // don't return empty pathnames
  return(0);
}

// RETURNS:
//    0 - user picked a file
//    1 - user cancelled
//   -1 - failed; errmsg() has reason
//
int Fl_Native_File_Chooser::show() {
  int retval;
  if ( _btype == BROWSE_DIRECTORY ||
       _btype == BROWSE_MULTI_DIRECTORY ||
       _btype == BROWSE_SAVE_DIRECTORY ) {
    retval = showdir();
  } else {
    retval = showfile();
  }
  // restore the correct state of mouse buttons and keyboard modifier keys (STR #3221)
  HWND h = GetForegroundWindow();
  if (h) {
    WNDPROC windproc = (WNDPROC)GetWindowLongPtrW(h, GWLP_WNDPROC);
    CallWindowProc(windproc, h, WM_ACTIVATEAPP, 1, 0);
  }
  return retval;
}

// RETURN ERROR MESSAGE
const char *Fl_Native_File_Chooser::errmsg() const {
  return(_errmsg ? _errmsg : "No error");
}

// GET FILENAME
const char* Fl_Native_File_Chooser::filename() const {
  if ( _pathnames && _tpathnames > 0 ) return(_pathnames[0]);
  return("");
}

// GET FILENAME FROM LIST OF FILENAMES
const char* Fl_Native_File_Chooser::filename(int i) const {
  if ( _pathnames && i < _tpathnames ) return(_pathnames[i]);
  return("");
}

// GET TOTAL FILENAMES CHOSEN
int Fl_Native_File_Chooser::count() const {
  return(_tpathnames);
}

// PRESET PATHNAME
//     Can be NULL if no preset is desired.
//
void Fl_Native_File_Chooser::directory(const char *val) {
  _directory = strfree(_directory);
  _directory = strnew(val);
}

// GET PRESET PATHNAME
//    Can return NULL if none set.
//
const char *Fl_Native_File_Chooser::directory() const {
  return(_directory);
}

// SET TITLE
//     Can be NULL if no title desired.
//
void Fl_Native_File_Chooser::title(const char *val) {
  _title = strfree(_title);
  _title = strnew(val);
}

// GET TITLE
//    Can return NULL if none set.
//
const char *Fl_Native_File_Chooser::title() const {
  return(_title);
}

// SET FILTER
//     Can be NULL if no filter needed
//
void Fl_Native_File_Chooser::filter(const char *val) {
  _filter = strfree(_filter);
  clear_filters();
  if ( val ) {
    _filter = strnew(val);
    parse_filter(_filter);
  }
  add_filter("All Files", "*.*");	// always include 'all files' option

#ifdef DEBUG
  dnullprint(_parsedfilt);
#endif /*DEBUG*/
}

// GET FILTER
//    Can return NULL if none set.
//
const char *Fl_Native_File_Chooser::filter() const {
  return(_filter);
}

// CLEAR FILTERS
void Fl_Native_File_Chooser::clear_filters() {
  _nfilters = 0;
  _parsedfilt = strfree(_parsedfilt);
}

// ADD A FILTER
void Fl_Native_File_Chooser::add_filter(const char *name_in,	// name of filter (optional: can be null)
	                    const char *winfilter) {    	// windows style filter (eg. "*.cxx;*.h")
  // No name? Make one..
  char name[1024];
  if ( !name_in || name_in[0] == '\0' ) {
    snprintf(name, sizeof(name), "%.*s Files", int(sizeof(name)-10), winfilter);
  } else {
    if ((strlen(name_in)+strlen(winfilter)+3) < sizeof(name)) {
      snprintf(name, sizeof(name), "%s (%s)", name_in, winfilter);
    } else {
      snprintf(name, sizeof(name), "%.*s", int(sizeof(name))-1, name_in);
    }
  }
  dnullcat(_parsedfilt, name);
  dnullcat(_parsedfilt, winfilter);
  _nfilters++;
  //DEBUG printf("DEBUG: ADD FILTER name=<%s> winfilter=<%s>\n", name, winfilter);
}

// RETURN HOW MANY DIFFERENT FILTERS WERE SPECIFIED
//   In: "foo.[CH]" or "foo.{C,H}"
//   Out: 2
//
static int count_filters(const char *filter) {
  int count = 0;
  char mode = 0;
  const char *in = filter;
  while (*in) {
    switch(*in) {
      case '\\':			// escape next character
        ++in; if ( *in == 0 ) continue;	// skip escape. EOL? done
	++in;				// skip escaped char
	continue;
      case LCURLY_CHR:			// start "{aaa,bbb}"
	mode = *in;			// set mode, parse over curly
        ++count;			// at least +1 wildcard
	break;
      case RCURLY_CHR:			// end "{aaa,bbb}"
	if ( mode == LCURLY_CHR )	// disable curly mode (if on)
	  mode = 0;
	break;
      case LBRACKET_CHR:		// start "[xyz]"
        mode = *in;			// set mode, parse over bracket
	break;
      case RBRACKET_CHR:		// end "[xyz]"
	if ( mode == LBRACKET_CHR )	// disable bracket mode (if on)
	  mode = 0;
	break;
      default:				// any other char
        switch (mode) {			// handle {} or [] modes
	  case LCURLY_CHR:		// handle "{aaa,bbb}"
	    if (*in==',' || *in=='|')	// ',' and '|' adds filters
	      ++count;
	    break;
	  case LBRACKET_CHR:		// handle "[xyz]"
	    ++count;			// all chars in []'s add new filter
	    break;
	}
	break;
    }
    ++in;				// parse past char
  }
  return count > 0 ? count : 1;		// return at least 1
}

// CONVERT FLTK STYLE PATTERN MATCHES TO WINDOWS 'DOUBLENULL' PATTERN
// Returns with the parsed double-null result in '_parsedfilt'.
//
//    Handles:
//        IN              OUT
//        -----------     -----------------------------
//        *.{ma,mb}       "*.{ma,mb} Files\0*.ma;*.mb\0\0"
//        *.[abc]         "*.[abc] Files\0*.a;*.b;*.c\0\0"
//        *.txt           "*.txt Files\0*.txt\0\0"
//        C Files\t*.[ch] "C Files\0*.c;*.h\0\0"
//
//    Example:
//         IN: "*.{ma,mb}"
//        OUT: "*.ma;*.mb Files\0*.ma;*.mb\0All Files\0*.*\0\0"
//              ---------------  ---------  ---------  ---
//                     |             |          |       |
//                   Title       Wildcards    Title    Wildcards
//
// Parsing Mode:
//         IN:"C Files\t*.{cxx,h}"
//             |||||||  |||||||||
//       mode: nnnnnnn  ww{{{{{{{
//             \_____/  \_______/
//              Name     Wildcard
//
void Fl_Native_File_Chooser::parse_filter(const char *in) {
  clear_filters();
  if ( ! in || in[0] == '\0' ) return;

  int has_name = strchr(in, '\t') ? 1 : 0;
  char mode = has_name ? 'n' : 'w';		// parse mode: n=name, w=wildcard

  // whatever input string is, our output won't be much longer in length..
  // use double length just for safety.
  size_t slen = strlen(in);
  char *wildprefix = new char[(slen+1)*2]; wildprefix[0] = 0;
  char *comp       = new char[(slen+1)*2]; comp[0] = 0;
  char *name       = new char[(slen+1)*2]; name[0] = 0;

  // Init
  int nwildcards = 0;
  int maxfilters = count_filters(in) + 1;	// count wildcard seps
  char **wildcards = new char*[maxfilters];	// parsed wildcards (can be several)
  int t;
  for ( t=0; t<maxfilters; t++ ) {
    wildcards[t] = new char[slen+1];
    wildcards[t][0] = '\0';
  }

  // Parse
  for ( ; 1; in++ ) {

    //// DEBUG
    //// printf("WORKING ON '%c': mode=<%c> name=<%s> wildprefix=<%s> nwildcards=%d wildcards[n]=<%s>\n",
    ////        *in, mode, name, wildprefix, nwildcards, wildcards[nwildcards]);

    switch (*in) {
      case ',':
      case '|':
	if ( mode == LCURLY_CHR ) {
	  // create new wildcard, copy in prefix
	  strcat(wildcards[nwildcards++], wildprefix);
	  continue;
	} else {
	  goto regchar;
	}
	continue;

      // FINISHED PARSING A NAME?
      case '\t':
	if ( mode != 'n' ) goto regchar;
	// finish parsing name? switch to wildcard mode
	mode = 'w';
	break;

      // ESCAPE NEXT CHAR
      case '\\':
	++in;
	goto regchar;

      // FINISHED PARSING ONE OF POSSIBLY SEVERAL FILTERS?
      case '\r':
      case '\n':
      case '\0':
      {
	if ( mode == 'w' ) {		// finished parsing wildcard?
	  if ( nwildcards == 0 ) {
	    strcpy(wildcards[nwildcards++], wildprefix);
	  }
	  // Append wildcards in Microsoft's "*.one;*.two" format
	  comp[0] = 0;
	  for ( t=0; t<nwildcards; t++ ) {
	    if ( t != 0 ) strcat(comp, ";");
	    strcat(comp, wildcards[t]);
	  }
	  // Add if not empty
	  if ( comp[0] ) {
	    add_filter(name, comp);
	  }
	}
	// RESET
	for ( t=0; t<maxfilters; t++ ) {
	  wildcards[t][0] = '\0';
	}
	nwildcards = 0;
	wildprefix[0] = name[0] = '\0';
	mode = strchr(in,'\t') ? 'n' : 'w';
	// DONE?
	if ( *in == '\0' ) {		// done
	  // Free everything
	  delete[] wildprefix;
	  delete[] comp;
	  delete[] name;
	  for ( t=0; t<maxfilters; t++ ) delete[] wildcards[t];
	  delete[] wildcards;
	  return;
	}
	continue;			// not done yet, more filters
      }

      // STARTING A WILDCARD?
      case LBRACKET_CHR:
      case LCURLY_CHR:
	mode = *in;
	if ( *in == LCURLY_CHR ) {
	  // create new wildcard
	  strcat(wildcards[nwildcards++], wildprefix);
	}
	continue;

      // ENDING A WILDCARD?
      case RBRACKET_CHR:
      case RCURLY_CHR:
	mode = 'w';	// back to wildcard mode
	continue;

      // ALL OTHER NON-SPECIAL CHARACTERS
      default:
      regchar:		// handle regular char
	switch ( mode ) {
	  case LBRACKET_CHR:
	    // create new wildcard
	    ++nwildcards;
	    // copy in prefix
	    strcpy(wildcards[nwildcards-1], wildprefix);
	    // append search char
	    chrcat(wildcards[nwildcards-1], *in);
	    continue;

	  case LCURLY_CHR:
	    if ( nwildcards > 0 ) {
	      chrcat(wildcards[nwildcards-1], *in);
	    }
	    continue;

	  case 'n':
	    chrcat(name, *in);
	    continue;

	  case 'w':
	    chrcat(wildprefix, *in);
	    for ( t=0; t<nwildcards; t++ ) {
	      chrcat(wildcards[t], *in);
	    }
	    continue;
	}
	break;
    }
  }
}

// SET 'CURRENTLY SELECTED FILTER'
void Fl_Native_File_Chooser::filter_value(int i) {
  _ofn_ptr->nFilterIndex = i + 1;
}

// RETURN VALUE OF 'CURRENTLY SELECTED FILTER'
int Fl_Native_File_Chooser::filter_value() const {
  return(_ofn_ptr->nFilterIndex ? _ofn_ptr->nFilterIndex-1 : _nfilters+1);
}

// PRESET FILENAME FOR 'SAVE AS' CHOOSER
void Fl_Native_File_Chooser::preset_file(const char* val) {
  _preset_file = strfree(_preset_file);
  _preset_file = strnew(val);
}

// GET PRESET FILENAME FOR 'SAVE AS' CHOOSER
const char* Fl_Native_File_Chooser::preset_file() const {
  return(_preset_file);
}

int Fl_Native_File_Chooser::filters() const {
  return(_nfilters);
}

static char *wchartoutf8(LPCWSTR in) {
  static char *out = NULL;
  static int lchar = 0;
  if (in == NULL)return NULL;
  int utf8len  = WideCharToMultiByte(CP_UTF8, 0, in, -1, NULL, 0, NULL, NULL);
  if (utf8len > lchar) {
    lchar = utf8len;
    out = (char *)realloc(out, lchar * sizeof(char));
  }
  WideCharToMultiByte(CP_UTF8, 0, in, -1, out, utf8len, NULL, NULL);
  return out;
}

static LPCWSTR utf8towchar(const char *in) {
  static WCHAR *wout = NULL;
  static int lwout = 0;
  if (in == NULL)return NULL;
  int wlen = MultiByteToWideChar(CP_UTF8, 0, in, -1, NULL, 0);
  if (wlen > lwout) {
    lwout = wlen;
    wout = (WCHAR *)realloc(wout, lwout * sizeof(WCHAR));
  }
  MultiByteToWideChar(CP_UTF8, 0, in, -1, wout, wlen);
  return wout;
}

#endif /*!FL_DOXYGEN*/

//
// End of "$Id$".
//
