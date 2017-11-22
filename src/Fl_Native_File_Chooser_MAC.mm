// "$Id$"
//
// FLTK native OS file chooser widget
//
// Copyright 1998-2010 by Bill Spitzak and others.
// Copyright 2004 Greg Ercolano.
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

// TODO:
//	o When doing 'open file', only dir is preset, not filename.
//        Possibly 'preset_file' could be used to select the filename.
//

#ifdef __APPLE__

#include <FL/Fl.H>
#include <FL/x.H> // for fl_mac_os_version
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/filename.H>
#define MAXFILTERS	80

class Fl_Quartz_Native_File_Chooser_Driver : public Fl_Native_File_Chooser_Driver {
private:
  int             _btype;		// kind-of browser to show()
  int             _options;		// general options
  void 	         *_panel;
  char          **_pathnames;		// array of pathnames
  int             _tpathnames;	        // total pathnames
  char           *_directory;		// default pathname to use
  char           *_title;		// title for window
  char           *_preset_file;	        // the 'save as' filename
  
  char           *_filter;		// user-side search filter, eg:
  // C Files\t*.[ch]\nText Files\t*.txt"
  
  char           *_filt_names;		// filter names (tab delimited)
  // eg. "C Files\tText Files"
  
  char           *_filt_patt[MAXFILTERS];
  // array of filter patterns, eg:
  //     _filt_patt[0]="*.{cxx,h}"
  //     _filt_patt[1]="*.txt"
  
  int             _filt_total;		// parse_filter() # of filters loaded
  int             _filt_value;		// index of the selected filter
  char           *_errmsg;		// error message
  
  // Private methods
  void errmsg(const char *msg);
  void clear_pathnames();
  void set_single_pathname(const char *s);
  int get_saveas_basename(void);
  void clear_filters();
  void parse_filter(const char *from);
  int post();
  int runmodal();
public:
  Fl_Quartz_Native_File_Chooser_Driver(int val);
  ~Fl_Quartz_Native_File_Chooser_Driver();
  virtual void type(int t);
  virtual int type() const ;
  virtual void options(int o);
  virtual int options() const;
  virtual int count() const;
  virtual const char *filename() const ;
  virtual const char *filename(int i) const ;
  virtual void directory(const char *val) ;
  virtual const char *directory() const;
  virtual void title(const char *t);
  virtual const char* title() const;
  virtual const char *filter() const ;
  virtual void filter(const char *f);
  virtual int filters() const ;
  virtual void filter_value(int i) ;
  virtual int filter_value() const ;
  virtual void preset_file(const char*f) ;
  virtual const char* preset_file() const;
  virtual const char *errmsg() const ;
  virtual int show() ;
};

Fl_Native_File_Chooser::Fl_Native_File_Chooser(int val) {
  platform_fnfc = new Fl_Quartz_Native_File_Chooser_Driver(val);
}

// FREE PATHNAMES ARRAY, IF IT HAS ANY CONTENTS
void Fl_Quartz_Native_File_Chooser_Driver::clear_pathnames() {
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
void Fl_Quartz_Native_File_Chooser_Driver::set_single_pathname(const char *s) {
  clear_pathnames();
  _pathnames = new char*[1];
  _pathnames[0] = strnew(s);
  _tpathnames = 1;
}

// CONSTRUCTOR
Fl_Quartz_Native_File_Chooser_Driver::Fl_Quartz_Native_File_Chooser_Driver(int val) :
  Fl_Native_File_Chooser_Driver(val) {
  _btype          = val;
  _panel = NULL;
  _options        = Fl_Native_File_Chooser::NO_OPTIONS;
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
}

// DESTRUCTOR
Fl_Quartz_Native_File_Chooser_Driver::~Fl_Quartz_Native_File_Chooser_Driver() {
  // _opts		// nothing to manage
  // _options		// nothing to manage
  // _keepstate		// nothing to manage
  // _tempitem		// nothing to manage
  clear_pathnames();
  _directory   = strfree(_directory);
  _title       = strfree(_title);
  _preset_file = strfree(_preset_file);
  _filter      = strfree(_filter);
  //_filt_names		// managed by clear_filters()
  //_filt_patt[i]	// managed by clear_filters()
  //_filt_total		// managed by clear_filters()
  clear_filters();
  //_filt_value		// nothing to manage
  _errmsg = strfree(_errmsg);
}

// GET TYPE OF BROWSER
int Fl_Quartz_Native_File_Chooser_Driver::type() const {
  return(_btype);
}

// SET OPTIONS
void Fl_Quartz_Native_File_Chooser_Driver::options(int val) {
  _options = val;
}

// GET OPTIONS
int Fl_Quartz_Native_File_Chooser_Driver::options() const {
  return(_options);
}

// SHOW THE BROWSER WINDOW
//     Returns:
//         0 - user picked a file
//         1 - user cancelled
//        -1 - failed; errmsg() has reason
//
int Fl_Quartz_Native_File_Chooser_Driver::show() {

  // Make sure fltk interface updates before posting our dialog
  Fl::flush();
  
  // POST BROWSER
  int err = post();

  return(err);
}

// SET ERROR MESSAGE
//     Internal use only.
//
void Fl_Quartz_Native_File_Chooser_Driver::errmsg(const char *msg) {
  _errmsg = strfree(_errmsg);
  _errmsg = strnew(msg);
}

// RETURN ERROR MESSAGE
const char *Fl_Quartz_Native_File_Chooser_Driver::errmsg() const {
  return(_errmsg ? _errmsg : "No error");
}

// GET FILENAME
const char* Fl_Quartz_Native_File_Chooser_Driver::filename() const {
  if ( _pathnames && _tpathnames > 0 ) return(_pathnames[0]);
  return("");
}

// GET FILENAME FROM LIST OF FILENAMES
const char* Fl_Quartz_Native_File_Chooser_Driver::filename(int i) const {
  if ( _pathnames && i < _tpathnames ) return(_pathnames[i]);
  return("");
}

// GET TOTAL FILENAMES CHOSEN
int Fl_Quartz_Native_File_Chooser_Driver::count() const {
  return(_tpathnames);
}

// PRESET PATHNAME
//     Value can be NULL for none.
//
void Fl_Quartz_Native_File_Chooser_Driver::directory(const char *val) {
  _directory = strfree(_directory);
  _directory = strnew(val);
}

// GET PRESET PATHNAME
//     Returned value can be NULL if none set.
//
const char* Fl_Quartz_Native_File_Chooser_Driver::directory() const {
  return(_directory);
}

// SET TITLE
//     Value can be NULL if no title desired.
//
void Fl_Quartz_Native_File_Chooser_Driver::title(const char *val) {
  _title = strfree(_title);
  _title = strnew(val);
}

// GET TITLE
//     Returned value can be NULL if none set.
//
const char *Fl_Quartz_Native_File_Chooser_Driver::title() const {
  return(_title);
}

// SET FILTER
//     Can be NULL if no filter needed
//
void Fl_Quartz_Native_File_Chooser_Driver::filter(const char *val) {
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
const char *Fl_Quartz_Native_File_Chooser_Driver::filter() const {
  return(_filter);
}

// CLEAR ALL FILTERS
//    Internal use only.
//
void Fl_Quartz_Native_File_Chooser_Driver::clear_filters() {
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
void Fl_Quartz_Native_File_Chooser_Driver::parse_filter(const char *in) {
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

// SET PRESET FILE
//     Value can be NULL for none.
//
void Fl_Quartz_Native_File_Chooser_Driver::preset_file(const char* val) {
  _preset_file = strfree(_preset_file);
  _preset_file = strnew(val);
}

// PRESET FILE
//     Returned value can be NULL if none set.
//
const char* Fl_Quartz_Native_File_Chooser_Driver::preset_file() const {
  return(_preset_file);
}

void Fl_Quartz_Native_File_Chooser_Driver::filter_value(int val) {
  _filt_value = val;
}

int Fl_Quartz_Native_File_Chooser_Driver::filter_value() const {
  return(_filt_value);
}

int Fl_Quartz_Native_File_Chooser_Driver::filters() const {
  return(_filt_total);
}

#import <Cocoa/Cocoa.h>
#define UNLIKELYPREFIX "___fl_very_unlikely_prefix_"

int Fl_Quartz_Native_File_Chooser_Driver::get_saveas_basename(void) {
  char *q = strdup( [[[(NSSavePanel*)_panel URL] path] UTF8String] );
  if ( !(_options & Fl_Native_File_Chooser::SAVEAS_CONFIRM) ) {
    const char *d = [[[[(NSSavePanel*)_panel URL] path] stringByDeletingLastPathComponent] UTF8String];
    int l = (int)strlen(d) + 1;
    if (strcmp(d, "/") == 0) l = 1;
    int lu = strlen(UNLIKELYPREFIX);
    // Remove UNLIKELYPREFIX between directory and filename parts
    memmove(q + l, q + l + lu, strlen(q + l + lu) + 1);
  }
  set_single_pathname( q );
  free(q);
  return 0;
}

// SET THE TYPE OF BROWSER
void Fl_Quartz_Native_File_Chooser_Driver::type(int val) {
  _btype = val;
}

/* Input
 filter=  "C files\t*.{c,h}\nText files\t*.txt\n"
 patterns[0] = "*.{c,h}"
 patterns[1] = "*.txt"
 count = 2
 Return:
 "C files (*.{c,h})\nText files (*.txt)\n"
 */
static char *prepareMacFilter(int count, const char *filter, char **patterns) {
  int rank = 0, l = 0;
  for (int i = 0; i < count; i++) {
    l += strlen(patterns[i]) + 3;
    }
  const char *p = filter;
  char *q; q = new char[strlen(p) + l + 1];
  const char *r, *s;
  char *t;
  t = q;
  do {	// copy to t what is in filter removing what is between \t and \n, if any
    r = strchr(p, '\n');
    if (!r) r = p + strlen(p);
    s = strchr(p, '\t');
    if (s && s < r) { 
      memcpy(q, p, s - p); 
      q += s - p; 
      if (rank < count) { sprintf(q, " (%s)", patterns[rank]); q += strlen(q); }
    }
    else { 
      memcpy(q, p, r - p); 
      q += r - p; 
    }
    rank++;
    *(q++) = '\n'; 
    if (*r) p = r + 1; else p = r;
  } while(*p);
  *q = 0;
  return t;
}
  
@interface FLopenDelegate : NSObject 
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
<NSOpenSavePanelDelegate>
#endif
{
  NSPopUpButton *nspopup;
  char **filter_pattern;
}
- (FLopenDelegate*)setPopup:(NSPopUpButton*)popup filter_pattern:(char**)pattern;
- (BOOL)panel:(id)sender shouldShowFilename:(NSString *)filename;
- (BOOL)panel:(id)sender shouldEnableURL:(NSURL *)url;
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
  if ( [nspopup indexOfSelectedItem] == [nspopup numberOfItems] - 1) return YES;
  BOOL isdir = NO;
  [[NSFileManager defaultManager] fileExistsAtPath:filename isDirectory:&isdir];
  if (isdir) return YES;
  if ( fl_filename_match([filename fileSystemRepresentation], filter_pattern[ [nspopup indexOfSelectedItem] ]) ) return YES;
  return NO;
}
- (BOOL)panel:(id)sender shouldEnableURL:(NSURL *)url
{
  return [self panel:sender shouldShowFilename:[url path]];
}
@end

@interface FLsaveDelegate : NSObject 
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
<NSOpenSavePanelDelegate>
#endif
{
  NSSavePanel *dialog;
  BOOL saveas_confirm;
}
- (NSString *)panel:(id)sender userEnteredFilename:(NSString *)filename confirmed:(BOOL)okFlag;
- (void)changedPopup:(id)sender;
- (void)panel:(NSSavePanel*)p;
- (void)option:(BOOL)o;
@end
@implementation FLsaveDelegate
- (NSString *)panel:(id)sender userEnteredFilename:(NSString *)filename confirmed:(BOOL)okFlag
{
  if ( !okFlag || saveas_confirm ) return filename;
  // User has clicked save, and no overwrite confirmation should occur.
  // To get the latter, we need to change the name we return (hence the prefix):
  return [@ UNLIKELYPREFIX stringByAppendingString:filename];
}
- (void)changedPopup:(id)sender
// runs when the save panel popup menu changes output file type
// correspondingly changes the extension of the output file name
{
  if (fl_mac_os_version < 100600) return; // because of setNameFieldStringValue and nameFieldStringValue
  char *s = strdup([[(NSPopUpButton*)sender titleOfSelectedItem] UTF8String]);
  if (!s) return;
  char *p = strchr(s, '(');
  if (!p) p = s;
  p = strchr(p, '.');
  if (!p) {free(s); return;}
  p++;
  while (*p == ' ') p++;
  if (!p || *p == '{') {free(s); return;}
  char *q = p+1;
  while (*q != ' ' && *q != ')' && *q != 0) q++;
  *q = 0;
  NSString *ns = [NSString stringWithFormat:@"%@.%@", 
		  [[dialog performSelector:@selector(nameFieldStringValue)] stringByDeletingPathExtension],
		  [NSString stringWithUTF8String:p]];
  free(s);
  [dialog performSelector:@selector(setNameFieldStringValue:) withObject:ns];
}
- (void)panel:(NSSavePanel*)p
{
  dialog = p;
}
- (void) option:(BOOL)o
{
  saveas_confirm = o;
}
@end
  
static NSPopUpButton *createPopupAccessory(NSSavePanel *panel, const char *filter, const char *title, int rank)
{
  NSPopUpButton *popup;
  NSRect rectview = NSMakeRect(5, 5, 350, 30 );
  NSView *view = [[[NSView alloc] initWithFrame:rectview] autorelease];
  NSRect rectbox = NSMakeRect(0, 3, 140, 20 );
  NSBox *box = [[[NSBox alloc] initWithFrame:rectbox] autorelease];
  NSRect rectpop = NSMakeRect(105, 0, 246, 30 );
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
  // horizontally move box to fit the locale-dependent width of its title
  NSRect r=[box frame];
  NSPoint o = r.origin;
  o.x = rectpop.origin.x - r.size.width + 15;
  [box setFrameOrigin:o];
  CFStringRef tab = CFSTR("\n");
  CFStringRef tmp_cfs;
  tmp_cfs = CFStringCreateWithCString(NULL, filter, kCFStringEncodingUTF8);
  CFArrayRef array = CFStringCreateArrayBySeparatingStrings(NULL, tmp_cfs, tab);
  CFRelease(tmp_cfs);
  CFRelease(tab);
  [popup addItemsWithTitles:(NSArray*)array];
  NSMenuItem *item = [popup itemWithTitle:@""];
  if (item) [popup removeItemWithTitle:@""];
  CFRelease(array);
  [popup selectItemAtIndex:rank];
  [panel setAccessoryView:view];
  return popup;
}

int Fl_Quartz_Native_File_Chooser_Driver::runmodal()
{
  NSString *dir = nil;
  NSString *fname = nil;
  NSString *preset = nil;
  NSInteger retval;
  if (_preset_file) {
    preset = [[NSString alloc] initWithUTF8String:_preset_file];
    if (strchr(_preset_file, '/') != NULL) {
      dir = [[NSString alloc] initWithString:[preset stringByDeletingLastPathComponent]];
    }
    fname = [preset lastPathComponent];
  }
  if (_directory && !dir) dir = [[NSString alloc] initWithUTF8String:_directory];
  if (fl_mac_os_version >= 100600) {
    bool usepath = false;
    NSString *path = nil;
    if (dir && fname && [(NSSavePanel*)_panel isKindOfClass:[NSOpenPanel class]]) {
      // STR #3406: If both dir + fname specified, combine and pass to setDirectoryURL
      path = [[NSString alloc] initWithFormat:@"%@/%@", dir, fname];  // dir+fname -> path
      // See if full path to file exists
      //    If dir exists but fname doesn't, avoid using setDirectoryURL,
      //    otherwise NSSavePanel falls back to showing user's Documents dir.
      //
      if ( [[NSFileManager defaultManager] fileExistsAtPath:path] ) usepath = true;
    }
      if (usepath) {
        // Set only if full path exists
        [(NSSavePanel*)_panel performSelector:@selector(setDirectoryURL:) withObject:[NSURL fileURLWithPath:path]];
      } else { // didn't setDirectoryURL to full path? Set dir + fname separately..
        if (dir) [(NSSavePanel*)_panel performSelector:@selector(setDirectoryURL:) withObject:[NSURL fileURLWithPath:dir]];
        if (fname) [(NSSavePanel*)_panel performSelector:@selector(setNameFieldStringValue:) withObject:fname];
      }
      [path release];
    retval = [(NSSavePanel*)_panel runModal];
  }
  else {
    retval = [(id)_panel runModalForDirectory:dir file:fname];
  }
  [dir release];
  [preset release];
  return (retval == NSFileHandlingPanelOKButton ? 1 : 0);
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
int Fl_Quartz_Native_File_Chooser_Driver::post() {
  // INITIALIZE BROWSER
  if ( _filt_total == 0 ) {	// Make sure they match
    _filt_value = 0;		// TBD: move to someplace more logical?
  }
  fl_open_display();
  NSAutoreleasePool *localPool;
  localPool = [[NSAutoreleasePool alloc] init];
  switch (_btype) {
    case Fl_Native_File_Chooser::BROWSE_FILE:
    case Fl_Native_File_Chooser::BROWSE_MULTI_FILE:
    case Fl_Native_File_Chooser::BROWSE_DIRECTORY:
    case Fl_Native_File_Chooser::BROWSE_MULTI_DIRECTORY:
      _panel =  [NSOpenPanel openPanel];
      break;	  
    case Fl_Native_File_Chooser::BROWSE_SAVE_DIRECTORY:
    case Fl_Native_File_Chooser::BROWSE_SAVE_FILE:
      _panel =  [NSSavePanel savePanel];
      break;
  }
  BOOL is_open_panel = [(NSSavePanel*)_panel isKindOfClass:[NSOpenPanel class]];
  if (_title) {
    SEL title_or_message = (is_open_panel && fl_mac_os_version >= 101200) ?
          @selector(setMessage:) : @selector(setTitle:);
    [(NSSavePanel*)_panel performSelector:title_or_message withObject:[NSString stringWithUTF8String:_title]];
  }
  switch (_btype) {
    case Fl_Native_File_Chooser::BROWSE_MULTI_FILE:
      [(NSOpenPanel*)_panel setAllowsMultipleSelection:YES];
      break;
    case Fl_Native_File_Chooser::BROWSE_MULTI_DIRECTORY:
      [(NSOpenPanel*)_panel setAllowsMultipleSelection:YES];
      /* FALLTHROUGH */
    case Fl_Native_File_Chooser::BROWSE_DIRECTORY:
      [(NSOpenPanel*)_panel setCanChooseDirectories:YES];
      break;
    case Fl_Native_File_Chooser::BROWSE_SAVE_DIRECTORY:
      [(NSSavePanel*)_panel setCanCreateDirectories:YES];
      break;
  }
  
  // SHOW THE DIALOG
  NSWindow *key = [NSApp keyWindow];
  NSPopUpButton *popup = nil;
  if ( is_open_panel ) {
    if (_filt_total) {
      char *t = prepareMacFilter(_filt_total, _filter, _filt_patt);
      popup = createPopupAccessory((NSSavePanel*)_panel, t, Fl_File_Chooser::show_label, 0);
      delete[] t;
      [[popup menu] addItem:[NSMenuItem separatorItem]];
      [popup addItemWithTitle:[NSString stringWithUTF8String:Fl_File_Chooser::all_files_label]];
      [popup setAction:@selector(validateVisibleColumns)];
      [popup setTarget:(NSObject*)_panel];
      FLopenDelegate *openDelegate = [[[FLopenDelegate alloc] init] autorelease];
      [openDelegate setPopup:popup filter_pattern:_filt_patt];
      [(NSOpenPanel*)_panel setDelegate:openDelegate];
    }
  }
  else {
    FLsaveDelegate *saveDelegate = [[[FLsaveDelegate alloc] init] autorelease]; 
    [(NSSavePanel*)_panel setAllowsOtherFileTypes:YES];
    [(NSSavePanel*)_panel setDelegate:saveDelegate];
    [saveDelegate option:(_options & Fl_Native_File_Chooser::SAVEAS_CONFIRM)];
    if (_filt_total) {
      if (_filt_value >= _filt_total) _filt_value = _filt_total - 1;
      char *t = prepareMacFilter(_filt_total, _filter, _filt_patt);
      popup = createPopupAccessory((NSSavePanel*)_panel, t, [[(NSSavePanel*)_panel nameFieldLabel] UTF8String], _filt_value);
      delete[] t;
      if (_options & Fl_Native_File_Chooser::USE_FILTER_EXT) {
	[popup setAction:@selector(changedPopup:)];
	[popup setTarget:saveDelegate];
	[saveDelegate panel:(NSSavePanel*)_panel];
      }
      [(NSSavePanel*)_panel setCanSelectHiddenExtension:YES];
    }
  }
  int retval = runmodal();
  if (_filt_total) {
    _filt_value = (int)[popup indexOfSelectedItem];
  }
  if ( retval == 1 ) {
    if (is_open_panel) {
      clear_pathnames();
      NSArray *array = [(NSOpenPanel*)_panel URLs];
      _tpathnames = (int)[array count];
      _pathnames = new char*[_tpathnames];
      for(int i = 0; i < _tpathnames; i++) {
	_pathnames[i] = strnew([[(NSURL*)[array objectAtIndex:i] path] UTF8String]);
      }
    }
    else get_saveas_basename();
  }
  [key makeKeyWindow];
  [localPool release];
  return (retval == 1 ? 0 : 1);
}

#endif // __APPLE__

//
// End of "$Id$".
//
