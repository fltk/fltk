// "$Id$"
//
// FLTK native OS file chooser widget
//
// Copyright 1998-2005 by Bill Spitzak and others.
// Copyright 2004 Greg Ercolano.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

// TODO:
//	o When doing 'open file', only dir is preset, not filename.
//        Possibly 'preset_file' could be used to select the filename.
//

#include "Fl_Native_File_Chooser_common.cxx"		// strnew/strfree/strapp/chrcat
#include <libgen.h>		// dirname(3)
#include <sys/types.h>		// stat(2)
#include <sys/stat.h>		// stat(2)


#include <FL/Fl.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/filename.H>
#define FNFC_CLASS Fl_Native_File_Chooser
#define FNFC_CTOR  Fl_Native_File_Chooser

#ifndef __APPLE_COCOA__
// TRY TO CONVERT AN AEDesc TO AN FSRef
//     As per Apple Technical Q&A QA1274
//     eg: http://developer.apple.com/qa/qa2001/qa1274.html
//     Returns 'noErr' if OK, or an 'OSX result code' on error.
//
static int AEDescToFSRef(const AEDesc* desc, FSRef* fsref) {
  OSStatus err = noErr;
  AEDesc coerceDesc;
  // If AEDesc isn't already an FSRef, convert it to one
  if ( desc->descriptorType != typeFSRef ) {
    if ( ( err = AECoerceDesc(desc, typeFSRef, &coerceDesc) ) == noErr ) {
      // Get FSRef out of AEDesc
      err = AEGetDescData(&coerceDesc, fsref, sizeof(FSRef));
      AEDisposeDesc(&coerceDesc);
    }
  } else {
    err = AEGetDescData(desc, fsref, sizeof(FSRef));
  }
  return( err );
}

// NAVREPLY: CTOR
FNFC_CLASS::NavReply::NavReply() {
  _valid_reply = 0;
}

// NAVREPLY: DTOR
FNFC_CLASS::NavReply::~NavReply() {
  if ( _valid_reply ) {
    NavDisposeReply(&_reply);
  }
}

// GET REPLY FROM THE NAV* DIALOG
int FNFC_CLASS::NavReply::get_reply(NavDialogRef& ref) {
  if ( _valid_reply ) {
    NavDisposeReply(&_reply);	// dispose of previous
    _valid_reply = 0;
  }
  if ( ref == NULL || NavDialogGetReply(ref, &_reply) != noErr ) {
    return(-1);
  }
  _valid_reply = 1;
  return(0);
}

// RETURN THE BASENAME USER WANTS TO 'Save As'
int FNFC_CLASS::NavReply::get_saveas_basename(char *s, int slen) {
  if (CFStringGetCString(_reply.saveFileName, s, slen-1, kCFStringEncodingUTF8) == false) {
    s[0] = '\0';
    return(-1);
  }
  return(0);
}

// RETURN THE DIRECTORY NAME
//    Returns 0 on success, -1 on error.
//
int FNFC_CLASS::NavReply::get_dirname(char *s, int slen) {
  FSRef fsref;
  if ( AEDescToFSRef(&_reply.selection, &fsref) != noErr ) {
    // Conversion failed? Return empty name
    s[0] = 0;
    return(-1);
  }
  FSRefMakePath(&fsref, (UInt8 *)s, slen);
  return(0);
}

// RETURN MULTIPLE DIRECTORIES
//     Returns: 0 on success with pathnames[] containing pathnames selected,
//             -1 on error
//
int FNFC_CLASS::NavReply::get_pathnames(char **&pathnames, int& tpathnames) {
  // How many items selected?
  long count = 0;
  if ( AECountItems(&_reply.selection, &count) != noErr ) {
    return(-1);
  }

  // Allocate space for that many pathnames
  pathnames = new char*[count];
  memset((void*)pathnames, 0, count*sizeof(char*));
  tpathnames = count;

  // Walk list of pathnames selected
  for (short index=1; index<=count; index++) {
    AEKeyword keyWord;
    AEDesc    desc;
    if (AEGetNthDesc(&_reply.selection, index, typeFSRef, &keyWord, &desc) != noErr) {
      pathnames[index-1] = strnew("");
      continue;
    }
    FSRef fsref;
    if (AEGetDescData(&desc, &fsref, sizeof(FSRef)) != noErr ) {
      pathnames[index-1] = strnew("");
      continue;
    }
    char s[4096];
    FSRefMakePath(&fsref, (UInt8 *)s, sizeof(s)-1);
    pathnames[index-1] = strnew(s);
    AEDisposeDesc(&desc);
  }
  return(0);
}
#endif /* !__APPLE_COCOA__ */

// FREE PATHNAMES ARRAY, IF IT HAS ANY CONTENTS
void FNFC_CLASS::clear_pathnames() {
  if ( _pathnames ) {
    while ( --_tpathnames >= 0 ) {
      _pathnames[_tpathnames] = strfree(_pathnames[_tpathnames]);
    }
    delete [] _pathnames;
    _pathnames = NULL;
  }
  _tpathnames = 0;
}

// SET A SINGLE PATHNAME
void FNFC_CLASS::set_single_pathname(const char *s) {
  clear_pathnames();
  _pathnames = new char*[1];
  _pathnames[0] = strnew(s);
  _tpathnames = 1;
}

#ifndef __APPLE_COCOA__
// GET THE 'Save As' FILENAME
//    Returns -1 on error, errmsg() has reason, filename == "".
//             0 if OK, filename() has filename chosen.
//
int FNFC_CLASS::get_saveas_basename(NavDialogRef& ref) {
  if ( ref == NULL ) {
    errmsg("get_saveas_basename: ref is NULL");
    return(-1);
  }
  NavReply reply;
  OSStatus err;
  if ((err = reply.get_reply(ref)) != noErr ) {
    errmsg("NavReply::get_reply() failed");
    clear_pathnames();
    return(-1);
  }

  char pathname[4096] = "";
  // Directory name..
  //    -2 leaves room to append '/'
  //
  if ( reply.get_dirname(pathname, sizeof(pathname)-2) < 0 ) {
    clear_pathnames();
    errmsg("NavReply::get_dirname() failed");
    return(-1);
  }
  // Append '/'
  int len = strlen(pathname);
  pathname[len++] = '/';
  pathname[len] = '\0';
  // Basename..
  if ( reply.get_saveas_basename(pathname+len, sizeof(pathname)-len) < 0 ) {
    clear_pathnames();
    errmsg("NavReply::get_saveas_basename() failed");
    return(-1);
  }
  set_single_pathname(pathname);
  return(0);
}

// GET (POTENTIALLY) MULTIPLE FILENAMES
//     Returns:
//         -1 -- error, errmsg() has reason, filename == ""
//          0 -- OK, pathnames()/filename() has pathname(s) chosen
//
int FNFC_CLASS::get_pathnames(NavDialogRef& ref) {
  if ( ref == NULL ) {
    errmsg("get_saveas_basename: ref is NULL");
    return(-1);
  }
  NavReply reply;
  OSStatus err;
  if ((err = reply.get_reply(ref)) != noErr ) { 
    errmsg("NavReply::get_reply() failed");
    clear_pathnames();
    return(-1);
  }
  // First, clear pathnames array of any previous contents
  clear_pathnames();
  if ( reply.get_pathnames(_pathnames, _tpathnames) < 0 ) {
    clear_pathnames();
    errmsg("NavReply::get_dirname() failed");
    return(-1);
  }
  return(0);
}

// IS PATHNAME A DIRECTORY?
//    1 - path is a dir
//    0 - path not a dir or error
//
static int IsDir(const char *pathname) {
  struct stat buf;
  if ( stat(pathname, &buf) != -1 ) {
    if ( buf.st_mode & S_IFDIR ) return(1);
  }
  return(0);
}

// PRESELECT PATHNAME IN BROWSER
static void PreselectPathname(NavCBRecPtr cbparm, const char *path) {
  // XXX: path must be a dir, or kNavCtlSetLocation fails with -50.
  //      Why, I don't know. Let me know with a bug report. -erco
  //
  if ( ! IsDir(path) ) {
    path = dirname((char*)path);
  }
  OSStatus err;
  FSRef fsref;
  err = FSPathMakeRef((const UInt8*)path, &fsref, NULL);
  if ( err != noErr) {
    fprintf(stderr, "FSPathMakeRef(%s) failed: err=%d\n", path, (int)err);
    return;
  }
  AEDesc desc;
  err = AECreateDesc(typeFSRef, &fsref, sizeof(FSRef), &desc);
  if ( err != noErr) {
    fprintf(stderr, "AECreateDesc() failed: err=%d\n", (int)err);
  }
  err = NavCustomControl(cbparm->context, kNavCtlSetLocation, &desc);
  if ( err != noErr) {
    fprintf(stderr, "NavCustomControl() failed: err=%d\n", (int)err);
  }
  AEDisposeDesc(&desc);
}

// NAV CALLBACK EVENT HANDLER
void FNFC_CLASS::event_handler(NavEventCallbackMessage callBackSelector, 
			       NavCBRecPtr cbparm,
			       void *data) {
  FNFC_CLASS *nfb = (FNFC_CLASS*)data;
  switch (callBackSelector) {
    case kNavCBStart:
    {
      if ( nfb->directory() ) {				// dir specified?
	PreselectPathname(cbparm, nfb->directory());	// use it first
      } else if ( nfb->preset_file() ) {		// file specified?
	PreselectPathname(cbparm, nfb->preset_file());	// use if no dir
      }
      if ( nfb->_btype == BROWSE_SAVE_FILE && nfb->preset_file() ) {
	const char *p, *q;
	p = nfb->preset_file();//don't use the path part of preset_file
	q = strrchr(p, '/');
	if(q == NULL) q = p; else q++;
	CFStringRef namestr = CFStringCreateWithCString(NULL,
							q,
							kCFStringEncodingUTF8);
	NavDialogSetSaveFileName(cbparm->context, namestr);
	CFRelease(namestr);
      }
      NavCustomControl(cbparm->context,
		       kNavCtlSetActionState,
		       &nfb->_keepstate);
      // Select the right filter in pop-up menu
      if ( nfb->_filt_value == nfb->_filt_total ) {
	// Select All Documents
	NavPopupMenuItem kAll = kNavAllFiles;
	NavCustomControl(cbparm->context, kNavCtlSelectAllType, &kAll);
      } else if (nfb->_filt_value < nfb->_filt_total) {
	// Select custom filter
	nfb->_tempitem.version = kNavMenuItemSpecVersion;
	nfb->_tempitem.menuCreator = 'extn';
	nfb->_tempitem.menuType = nfb->_filt_value;
	*nfb->_tempitem.menuItemName = '\0';	// needed on 10.3+
	NavCustomControl(cbparm->context,
			 kNavCtlSelectCustomType,
			 &(nfb->_tempitem));
      }
      break;
    }
    case kNavCBPopupMenuSelect:
    {
      NavMenuItemSpecPtr ptr;
      // they really buried this one!
      ptr = (NavMenuItemSpecPtr)cbparm->eventData.eventDataParms.param;
      if ( ptr->menuCreator ) {
	// Gets index to filter ( menuCreator = 'extn' )
	nfb->_filt_value = ptr->menuType;
      } else {
	// All docs filter selected ( menuCreator = '\0\0\0\0' )
	nfb->_filt_value = nfb->_filt_total;
      }
      break;
    }
    case kNavCBSelectEntry:
    {
      NavActionState astate;
      switch ( nfb->_btype ) {
	// These don't need selection override
	case BROWSE_MULTI_FILE:
	case BROWSE_MULTI_DIRECTORY:
	case BROWSE_SAVE_FILE:
	  break;

	// These need to allow only one item, so disable
	// Open button if user tries to select multiple files
	case BROWSE_SAVE_DIRECTORY:
	case BROWSE_DIRECTORY:
	case BROWSE_FILE:
	{
	  long selectcount;
	  AECountItems((AEDescList*)cbparm->
		       eventData.eventDataParms.param,
		       &selectcount);
	  if ( selectcount > 1 ) {
	    NavCustomControl(cbparm->context,
			     kNavCtlSetSelection,
			     NULL);
	    astate = nfb->_keepstate |
		     kNavDontOpenState |
		     kNavDontChooseState;
	    NavCustomControl(cbparm->context,
			     kNavCtlSetActionState,
			     &astate );
	  } else {
	    astate= nfb->_keepstate | kNavNormalState;
	    NavCustomControl(cbparm->context,
			     kNavCtlSetActionState,
			     &astate );
	  }
	  break;
	}
      }
    }
    break;
  }
}
#endif /* !__APPLE_COCOA__ */

// CONSTRUCTOR
FNFC_CLASS::FNFC_CTOR(int val) {
  _btype          = val;
#ifdef __APPLE_COCOA__
  _panel = NULL;
#else
  NavGetDefaultDialogCreationOptions(&_opts);
  _opts.optionFlags |= kNavDontConfirmReplacement;	// no confirms for "save as"
  _ref            = NULL;
#endif
  _options        = NO_OPTIONS;
  memset(&_tempitem, 0, sizeof(_tempitem));
  _pathnames      = NULL;
  _tpathnames     = 0;
  _title          = NULL;
  _filter         = NULL;
  _filt_names     = NULL;
  memset(_filt_patt, 0, sizeof(char*) * MAXFILTERS);
  _filt_total     = 0;
  _filt_value     = 0;
  _directory      = NULL;
  _preset_file    = NULL;
  _errmsg         = NULL;
  _keepstate      = kNavNormalState;
}

// DESTRUCTOR
FNFC_CLASS::~FNFC_CTOR() {
  // _opts			// nothing to manage
#ifndef __APPLE_COCOA__
  if (_ref) { NavDialogDispose(_ref); _ref = NULL; }
#endif
  // _options			// nothing to manage
  // _keepstate		// nothing to manage
  // _tempitem		// nothing to manage
  clear_pathnames();
  _directory   = strfree(_directory);
  _title       = strfree(_title);
  _preset_file = strfree(_preset_file);
  _filter      = strfree(_filter);
  //_filt_names		// managed by clear_filters()
  //_filt_patt[i]		// managed by clear_filters()
  //_filt_total		// managed by clear_filters()
  clear_filters();
  //_filt_value		// nothing to manage
  _errmsg = strfree(_errmsg);
}

#ifndef __APPLE_COCOA__
// SET THE TYPE OF BROWSER
void FNFC_CLASS::type(int val) {
  _btype = val;
}
#endif /* !__APPLE_COCOA__ */

// GET TYPE OF BROWSER
int FNFC_CLASS::type() const {
  return(_btype);
}

// SET OPTIONS
void FNFC_CLASS::options(int val) {
  _options = val;
}

// GET OPTIONS
int FNFC_CLASS::options() const {
  return(_options);
}

// SHOW THE BROWSER WINDOW
//     Returns:
//         0 - user picked a file
//         1 - user cancelled
//        -1 - failed; errmsg() has reason
//
int FNFC_CLASS::show() {

  // Make sure fltk interface updates before posting our dialog
  Fl::flush();

  _keepstate = kNavNormalState;
#ifndef __APPLE_COCOA__
  // BROWSER TITLE
  CFStringRef cfs_title;
  cfs_title = CFStringCreateWithCString(NULL,
					_title ? _title : "No Title",
					kCFStringEncodingUTF8);
  _opts.windowTitle = cfs_title;
  // BROWSER FILTERS
  CFArrayRef filter_array = NULL;

  // One or more filters specified?
  if ( _filt_total ) {
    // NAMES -> CFArrayRef
    CFStringRef tab = CFSTR("\t");
    CFStringRef tmp_cfs;
    tmp_cfs = CFStringCreateWithCString(NULL,
					_filt_names,
					kCFStringEncodingUTF8);
    filter_array = CFStringCreateArrayBySeparatingStrings(NULL,
							  tmp_cfs,
							  tab);
    CFRelease(tmp_cfs);
    CFRelease(tab);
    _opts.popupExtension = filter_array;
    _opts.optionFlags |= kNavAllFilesInPopup;
  } else {
    filter_array = NULL;
    _opts.popupExtension = NULL;
    _opts.optionFlags |= kNavAllFilesInPopup;
  }

  // HANDLE OPTIONS WE SUPPORT
  if ( _options & SAVEAS_CONFIRM ) {
    _opts.optionFlags &= ~kNavDontConfirmReplacement;	// enables confirm
  } else {
    _opts.optionFlags |= kNavDontConfirmReplacement;	// disables confirm
  }
  // _opts.optionFlags |= kNavSupportPackages;		// enables *.app TODO: Make a flag to control this
#endif /* !__APPLE_COCOA__ */

  // POST BROWSER
  int err = post();

#ifndef __APPLE_COCOA__
  // RELEASE _FILT_ARR
  if ( filter_array ) CFRelease(filter_array);
  filter_array = NULL;
  _opts.popupExtension = NULL;
  // RELEASE TITLE
  if ( cfs_title ) CFRelease(cfs_title);
  cfs_title = NULL;
#endif /* !__APPLE_COCOA__ */
  _filt_total = 0;

  return(err);
}

#ifndef __APPLE_COCOA__
int FNFC_CLASS::post() {

  // INITIALIZE BROWSER
  OSStatus err;
  if ( _filt_total == 0 ) {		// Make sure they match
    _filt_value = 0;			// TBD: move to someplace more logical?
  }

  if ( ! ( _options & NEW_FOLDER ) ) {
    _keepstate |= kNavDontNewFolderState;
  }

  switch (_btype) {
    case BROWSE_FILE:
    case BROWSE_MULTI_FILE:
      // Prompt user for one or more files
      if ((err = NavCreateGetFileDialog(&_opts,			// options
					0, 			// file types
					event_handler,		// event handler
					0,			// preview callback
					filter_proc_cb,		// filter callback
					(void*)this,		// callback data
					&_ref)) != noErr ) {	// dialog ref
	errmsg("NavCreateGetFileDialog: failed");
	return(-1);
      }
      break;

    case BROWSE_DIRECTORY:
    case BROWSE_MULTI_DIRECTORY:
    case BROWSE_SAVE_DIRECTORY:
      // Prompts user for one or more files or folders
      if ((err = NavCreateChooseFolderDialog(&_opts,			// options
					     event_handler,		// event callback
					     0,				// filter callback
					     (void*)this,		// callback data
					     &_ref)) != noErr ) {	// dialog ref
	errmsg("NavCreateChooseFolderDialog: failed");
	return(-1);
      }
      break;

    case BROWSE_SAVE_FILE:
      // Prompt user for filename to 'save as'
      if ((err = NavCreatePutFileDialog(&_opts,			// options
					0,			// file types
					kNavGenericSignature,	//file creator
					event_handler,		// event handler
					(void*)this,		// callback data
					&_ref)) != noErr ) {	// dialog ref
	errmsg("NavCreatePutFileDialog: failed");
	return(-1);
      }
      break;
  }

  // SHOW THE DIALOG
  if ( ( err = NavDialogRun(_ref) ) != 0 ) {
    char msg[80];
    sprintf(msg, "NavDialogRun: failed (err=%d)", (int)err);
    errmsg(msg);
    return(-1);
  }

  // WHAT ACTION DID USER CHOOSE?
  NavUserAction act = NavDialogGetUserAction(_ref);
  if ( act == kNavUserActionNone ) {
    errmsg("Nothing happened yet (dialog still open)");
    return(-1);
  }
  else if ( act == kNavUserActionCancel ) { 	// user chose 'cancel'
    return(1);
  }
  else if ( act == kNavUserActionSaveAs ) {	// user chose 'save as'
    return(get_saveas_basename(_ref));
  }

  // TOO MANY FILES CHOSEN?
  int ret = get_pathnames(_ref);
  if ( _btype == BROWSE_FILE && ret == 0 && _tpathnames != 1 ) {
    char msg[80];
    sprintf(msg, "Expected only one file to be chosen.. you chose %d.", (int)_tpathnames);
    errmsg(msg);
    return(-1);
  }
  return(err);
}
#endif /* !__APPLE_COCOA__ */

// SET ERROR MESSAGE
//     Internal use only.
//
void FNFC_CLASS::errmsg(const char *msg) {
  _errmsg = strfree(_errmsg);
  _errmsg = strnew(msg);
}

// RETURN ERROR MESSAGE
const char *FNFC_CLASS::errmsg() const {
  return(_errmsg ? _errmsg : "No error");
}

// GET FILENAME
const char* FNFC_CLASS::filename() const {
  if ( _pathnames && _tpathnames > 0 ) return(_pathnames[0]);
  return("");
}

// GET FILENAME FROM LIST OF FILENAMES
const char* FNFC_CLASS::filename(int i) const {
  if ( _pathnames && i < _tpathnames ) return(_pathnames[i]);
  return("");
}

// GET TOTAL FILENAMES CHOSEN
int FNFC_CLASS::count() const {
  return(_tpathnames);
}

// PRESET PATHNAME
//     Value can be NULL for none.
//
void FNFC_CLASS::directory(const char *val) {
  _directory = strfree(_directory);
  _directory = strnew(val);
}

// GET PRESET PATHNAME
//     Returned value can be NULL if none set.
//
const char* FNFC_CLASS::directory() const {
  return(_directory);
}

// SET TITLE
//     Value can be NULL if no title desired.
//
void FNFC_CLASS::title(const char *val) {
  _title = strfree(_title);
  _title = strnew(val);
}

// GET TITLE
//     Returned value can be NULL if none set.
//
const char *FNFC_CLASS::title() const {
  return(_title);
}

// SET FILTER
//     Can be NULL if no filter needed
//
void FNFC_CLASS::filter(const char *val) {
  _filter = strfree(_filter);
  _filter = strnew(val);

  // Parse filter user specified
  //     IN: _filter = "C Files\t*.{cxx,h}\nText Files\t*.txt"
  //    OUT: _filt_names   = "C Files\tText Files"
  //         _filt_patt[0] = "*.{cxx,h}"
  //         _filt_patt[1] = "*.txt"
  //         _filt_total   = 2
  //
  parse_filter(_filter);
}

// GET FILTER
//     Returned value can be NULL if none set.
//
const char *FNFC_CLASS::filter() const {
  return(_filter);
}

// CLEAR ALL FILTERS
//    Internal use only.
//
void FNFC_CLASS::clear_filters() {
  _filt_names = strfree(_filt_names);
  for (int i=0; i<_filt_total; i++) {
    _filt_patt[i] = strfree(_filt_patt[i]);
  }
  _filt_total = 0;
}

// PARSE USER'S FILTER SPEC
//    Parses user specified filter ('in'),
//    breaks out into _filt_patt[], _filt_names, and _filt_total.
//
//    Handles:
//    IN:                                   OUT:_filt_names    OUT: _filt_patt
//    ------------------------------------  ------------------ ---------------
//    "*.{ma,mb}"                           "*.{ma,mb} Files"  "*.{ma,mb}"
//    "*.[abc]"                             "*.[abc] Files"    "*.[abc]"
//    "*.txt"                               "*.txt Files"      "*.c"
//    "C Files\t*.[ch]"                     "C Files"          "*.[ch]"
//    "C Files\t*.[ch]\nText Files\t*.cxx"  "C Files"          "*.[ch]"
//
//    Parsing Mode:
//         IN:"C Files\t*.{cxx,h}"
//             |||||||  |||||||||
//       mode: nnnnnnn  wwwwwwwww
//             \_____/  \_______/
//              Name     Wildcard
//
void FNFC_CLASS::parse_filter(const char *in) {
  clear_filters();
  if ( ! in ) return;
  int has_name = strchr(in, '\t') ? 1 : 0;

  char mode = has_name ? 'n' : 'w';	// parse mode: n=title, w=wildcard
  char wildcard[1024] = "";		// parsed wildcard
  char name[1024] = "";

  // Parse filter user specified
  for ( ; 1; in++ ) {

    //// DEBUG
    //// printf("WORKING ON '%c': mode=<%c> name=<%s> wildcard=<%s>\n",
    ////                    *in,  mode,     name,     wildcard);
    
    switch (*in) {
      // FINISHED PARSING NAME?
      case '\t':
	if ( mode != 'n' ) goto regchar;
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
	// TITLE
	//     If user didn't specify a name, make one
	//
	if ( name[0] == '\0' ) {
	  sprintf(name, "%.*s Files", (int)sizeof(name)-10, wildcard);
	}
	// APPEND NEW FILTER TO LIST
	if ( wildcard[0] ) {
	  // Add to filtername list
	  //     Tab delimit if more than one. We later break
	  //     tab delimited string into CFArray with 
	  //     CFStringCreateArrayBySeparatingStrings()
	  //
	  if ( _filt_total ) {
	      _filt_names = strapp(_filt_names, "\t");
	  }
	  _filt_names = strapp(_filt_names, name);

	  // Add filter to the pattern array
	  _filt_patt[_filt_total++] = strnew(wildcard);
	}
	// RESET
	wildcard[0] = name[0] = '\0';
	mode = strchr(in, '\t') ? 'n' : 'w';
	// DONE?
	if ( *in == '\0' ) return;	// done
	else continue;			// not done yet, more filters

      // Parse all other chars
      default:				// handle all non-special chars
      regchar:				// handle regular char
	switch ( mode ) {
	  case 'n': chrcat(name, *in);     continue;
	  case 'w': chrcat(wildcard, *in); continue;
	}
	break;
    }
  }
  //NOTREACHED
}

#ifndef __APPLE_COCOA__
// STATIC: FILTER CALLBACK
Boolean FNFC_CLASS::filter_proc_cb(AEDesc *theItem,
				   void *info,
				   void *callBackUD,
				   NavFilterModes filterMode) {
  return((FNFC_CLASS*)callBackUD)->filter_proc_cb2(theItem,
						   info,
						   callBackUD,
						   filterMode);
}

// FILTER CALLBACK
//     Return true if match,
//            false if no match.
//
Boolean FNFC_CLASS::filter_proc_cb2(AEDesc *theItem,
				    void *info,
				    void *callBackUD,
				    NavFilterModes filterMode) {
  // All files chosen or no filters
  if ( _filt_value == _filt_total ) return(true);
  
  FSRef fsref;
  char pathname[4096];
  
  // On fail, filter should return true by default
  if ( AEDescToFSRef(theItem, &fsref) != noErr ) {
    return(true);
  }
  FSRefMakePath(&fsref, (UInt8 *)pathname, sizeof(pathname)-1);

  if ( fl_filename_isdir(pathname) ) return(true);
  if ( fl_filename_match(pathname, _filt_patt[_filt_value]) ) return(true);
  else return(false);
}
#endif

// SET PRESET FILE
//     Value can be NULL for none.
//
void FNFC_CLASS::preset_file(const char* val) {
  _preset_file = strfree(_preset_file);
  _preset_file = strnew(val);
}

// PRESET FILE
//     Returned value can be NULL if none set.
//
const char* FNFC_CLASS::preset_file() {
  return(_preset_file);
}

#ifdef __APPLE_COCOA__
#import <Cocoa/Cocoa.h>
#define UNLIKELYPREFIX "___fl_very_unlikely_prefix_"

int FNFC_CLASS::get_saveas_basename(void) {
  char *q = strdup( [[(NSSavePanel*)_panel filename] fileSystemRepresentation] );
  id delegate = [(NSSavePanel*)_panel delegate];
  if(delegate != nil) {
    const char *d = [[(NSSavePanel*)_panel directory] fileSystemRepresentation];
    int l = strlen(d) + 1;
    int lu = strlen(UNLIKELYPREFIX);
    //remove UNLIKELYPREFIX between directory and filename parts
    memmove(q + l, q + l + lu, strlen(q + l + lu) + 1);
  }
  set_single_pathname( q );
  free(q);
  return 0;
}

// SET THE TYPE OF BROWSER
void FNFC_CLASS::type(int val) {
  _btype = val;
  switch (_btype) {
    case BROWSE_FILE:
    case BROWSE_MULTI_FILE:
    case BROWSE_DIRECTORY:
    case BROWSE_MULTI_DIRECTORY:
      _panel =  [NSOpenPanel openPanel];
      break;	  
    case BROWSE_SAVE_DIRECTORY:
    case BROWSE_SAVE_FILE:
      _panel =  [NSSavePanel savePanel];
      break;
  }
}
  
@interface FLopenDelegate : NSObject {
  NSPopUpButton *nspopup;
  char **filter_pattern;
}
- (FLopenDelegate*)setPopup:(NSPopUpButton*)popup filter_pattern:(char**)pattern;
- (BOOL)panel:(id)sender shouldShowFilename:(NSString *)filename;
@end
@implementation FLopenDelegate
- (FLopenDelegate*)setPopup:(NSPopUpButton*)popup filter_pattern:(char**)pattern
{
  nspopup = popup;
  filter_pattern = pattern;
  return self;
}
- (BOOL)panel:(id)sender shouldShowFilename:(NSString *)filename
{
  if( [nspopup indexOfSelectedItem] == [nspopup numberOfItems] - 1) return YES;
  const char *pathname = [filename fileSystemRepresentation];
  if ( fl_filename_isdir(pathname) ) return YES;
  if ( fl_filename_match(pathname, filter_pattern[ [nspopup indexOfSelectedItem] ]) ) return YES;
  return NO;
}
@end

@interface FLsaveDelegate : NSObject {
}
- (NSString *)panel:(id)sender userEnteredFilename:(NSString *)filename confirmed:(BOOL)okFlag;
@end
@implementation FLsaveDelegate
- (NSString *)panel:(id)sender userEnteredFilename:(NSString *)filename confirmed:(BOOL)okFlag
{
  if(! okFlag) return filename;
  // User has clicked save, and no overwrite confirmation should occur.
  // To get the latter, we need to change the name we return (hence the prefix):
  return [@ UNLIKELYPREFIX stringByAppendingString:filename];
}
@end
  
static NSPopUpButton *createPopupAccessory(NSSavePanel *panel, const char *filter, const char *title, int rank)
{
  NSPopUpButton *popup;
  NSRect rectview = NSMakeRect(5, 5, 350, 30 );
  NSView *view = [[[NSView alloc] initWithFrame:rectview] autorelease];
  NSRect rectbox = NSMakeRect(0, 3, 50, 1 );
  NSBox *box = [[[NSBox alloc] initWithFrame:rectbox] autorelease];
  NSRect rectpop = NSMakeRect(60, 0, 250, 30 );
  popup = [[[NSPopUpButton alloc ] initWithFrame:rectpop pullsDown:NO] autorelease];
  [view addSubview:box];
  [view addSubview:popup];
  [box setBorderType:NSNoBorder];
  NSString *nstitle = [[NSString alloc] initWithUTF8String:title];
  [box setTitle:nstitle];
  [nstitle release];
  NSFont *font = [NSFont controlContentFontOfSize:NSRegularControlSize];
  [box setTitleFont:font];
  [box sizeToFit];
  CFStringRef tab = CFSTR("\n");
  CFStringRef tmp_cfs;
  tmp_cfs = CFStringCreateWithCString(NULL, filter, kCFStringEncodingASCII);
  CFArrayRef array = CFStringCreateArrayBySeparatingStrings(NULL, tmp_cfs, tab);
  CFRelease(tmp_cfs);
  CFRelease(tab);
  [popup addItemsWithTitles:(NSArray*)array];
  NSMenuItem *item = [popup itemWithTitle:@""];
  if(item) [popup removeItemWithTitle:@""];
  CFRelease(array);
  [popup selectItemAtIndex:rank];
  [panel setAccessoryView:view];
  return popup;
}
  
// POST BROWSER
//     Internal use only.
//     Assumes '_opts' has been initialized.
//
//     Returns:
//         0 - user picked a file
//         1 - user cancelled
//        -1 - failed; errmsg() has reason
//     
int FNFC_CLASS::post() {
  // INITIALIZE BROWSER
  if ( _filt_total == 0 ) {	// Make sure they match
    _filt_value = 0;		// TBD: move to someplace more logical?
  }
  NSAutoreleasePool *localPool;
  localPool = [[NSAutoreleasePool alloc] init];
  int retval;
  NSString *nstitle = [NSString stringWithUTF8String: (_title ? _title : "No Title")];
  [(NSSavePanel*)_panel setTitle:nstitle];
  switch (_btype) {
    case BROWSE_MULTI_FILE:
      [(NSOpenPanel*)_panel setAllowsMultipleSelection:YES];
      break;
    case BROWSE_MULTI_DIRECTORY:
      [(NSOpenPanel*)_panel setAllowsMultipleSelection:YES];
    case BROWSE_DIRECTORY:
      [(NSOpenPanel*)_panel setCanChooseDirectories:YES];
      break;
    case BROWSE_SAVE_DIRECTORY:
      [(NSSavePanel*)_panel setCanCreateDirectories:YES];
      break;
  }
  
  // SHOW THE DIALOG
  if( [(NSSavePanel*)_panel isKindOfClass:[NSOpenPanel class]] ) {
    NSPopUpButton *popup = nil;
    if(_filt_total) {
      char *p; p = _filter;
      char *q; q = new char[strlen(p) + 1];
      char *r, *s, *t;
      t = q;
      do {	//copy to t what is in _filter removing what is between \t and \n, if any
	r = strchr(p, '\n');
	if(!r) r = p + strlen(p) - 1;
	s = strchr(p, '\t');
	if(s && s < r) { memcpy(q, p, s - p); q += s - p; *(q++) = '\n'; }
	else { memcpy(q, p, r - p + 1); q += r - p + 1; }
	*q = 0;
	p = r + 1;
      } while(*p);
      popup = createPopupAccessory((NSSavePanel*)_panel, t, "Enable:", 0);
      delete t;
      [[popup menu] addItem:[NSMenuItem separatorItem]];
      [popup addItemWithTitle:@"All Documents"];
      [popup setAction:@selector(validateVisibleColumns)];
      [popup setTarget:(NSObject*)_panel];
      static FLopenDelegate *openDelegate = nil;
      if(openDelegate == nil) {
	// not to be ever freed
	openDelegate = [[FLopenDelegate alloc] init];
      }
      [openDelegate setPopup:popup filter_pattern:_filt_patt];
      [(NSOpenPanel*)_panel setDelegate:openDelegate];
    }
    NSString *dir = nil;
    NSString *fname = nil;
    NSString *preset = nil;
    if(_preset_file) {
      preset = [[NSString alloc] initWithUTF8String:_preset_file];
      if(strchr(_preset_file, '/') != NULL) 
	dir = [[NSString alloc] initWithString:[preset stringByDeletingLastPathComponent]];
      fname = [preset lastPathComponent];
    }
    if(_directory && !dir) dir = [[NSString alloc] initWithUTF8String:_directory];
    retval = [(NSOpenPanel*)_panel runModalForDirectory:dir file:fname types:nil];	
    [dir release];
    [preset release];
    if(_filt_total) {
      _filt_value = [popup indexOfSelectedItem];
    }
    if ( retval == NSOKButton ) {
      clear_pathnames();
      NSArray *array = [(NSOpenPanel*)_panel filenames];
      _tpathnames = [array count];
      _pathnames = new char*[_tpathnames];
      for(int i = 0; i < _tpathnames; i++) {
	_pathnames[i] = strnew([(NSString*)[array objectAtIndex:i] fileSystemRepresentation]);
      }
    }
  }
  else {
    NSString *dir = nil;
    NSString *fname = nil;
    NSString *preset = nil;
    NSPopUpButton *popup = nil;
    if( !(_options & SAVEAS_CONFIRM) ) {
      static FLsaveDelegate *saveDelegate = nil;
      if(saveDelegate == nil)saveDelegate = [[FLsaveDelegate alloc] init];//not to be ever freed
      [(NSSavePanel*)_panel setDelegate:saveDelegate];
    }
    if(_preset_file) {
      preset = [[NSString alloc] initWithUTF8String:_preset_file];
      if(strchr(_preset_file, '/') != NULL) {
	dir = [[NSString alloc] initWithString:[preset stringByDeletingLastPathComponent]];
      }
      fname = [preset lastPathComponent];
    }
    if(_directory && !dir) dir = [[NSString alloc] initWithUTF8String:_directory];
    if(_filt_total) {
      popup = createPopupAccessory((NSSavePanel*)_panel, _filter, "Format:", _filt_value);
    }
    retval = [(NSSavePanel*)_panel runModalForDirectory:dir file:fname];
    if(_filt_total) {
      _filt_value = [popup indexOfSelectedItem];
    }
    [dir release];
    [preset release];
    if ( retval == NSOKButton ) get_saveas_basename();
  }
  [localPool release];
  return (retval == NSOKButton ? 0 : 1);
}

#endif //__APPLE_COCOA__

//
// End of "$Id$".
//
