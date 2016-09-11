// "$Id$"
//
// FLTK native file chooser widget wrapper for GTK's GtkFileChooserDialog 
//
// Copyright 1998-2014 by Bill Spitzak and others.
// Copyright 2012 IMM
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

#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_File_Icon.H>
#include <FL/Fl_System_Driver.H> // for struct stat
#include <string.h>



Fl_Native_File_Chooser_FLTK_Driver::Fl_Native_File_Chooser_FLTK_Driver(int val) :
  Fl_Native_File_Chooser_Driver(val) {
  _btype       = 0;
  _options     = 0;
  _filter      = NULL;
  _filtvalue   = 0;
  _parsedfilt  = NULL;
  _preset_file = NULL;
  _prevvalue   = NULL;
  _directory   = NULL;
  _errmsg      = NULL;
  _file_chooser= NULL;
  if (val >= 0) {
    _file_chooser = new Fl_File_Chooser(NULL, NULL, 0, NULL);
    type(val);			// do this after _file_chooser created
    }
  _nfilters    = 0;
} 

Fl_Native_File_Chooser_FLTK_Driver::~Fl_Native_File_Chooser_FLTK_Driver() {
  delete _file_chooser;
  _file_chooser = NULL;
  _filter      = strfree(_filter);
  _parsedfilt  = strfree(_parsedfilt);
  _preset_file = strfree(_preset_file);
  _prevvalue   = strfree(_prevvalue);
  _directory   = strfree(_directory);
  _errmsg      = strfree(_errmsg);
}


// PRIVATE: SET ERROR MESSAGE
void Fl_Native_File_Chooser_FLTK_Driver::errmsg(const char *msg) {
  _errmsg = strfree(_errmsg);
  _errmsg = strnew(msg);
}

// PRIVATE: translate Native types to Fl_File_Chooser types
int Fl_Native_File_Chooser_FLTK_Driver::type_fl_file(int val) {
  switch (val) {
    case Fl_Native_File_Chooser::BROWSE_FILE:
      return(Fl_File_Chooser::SINGLE);
    case Fl_Native_File_Chooser::BROWSE_DIRECTORY:
      return(Fl_File_Chooser::SINGLE | Fl_File_Chooser::DIRECTORY);
    case Fl_Native_File_Chooser::BROWSE_MULTI_FILE:
      return(Fl_File_Chooser::MULTI);
    case Fl_Native_File_Chooser::BROWSE_MULTI_DIRECTORY:
      return(Fl_File_Chooser::DIRECTORY | Fl_File_Chooser::MULTI);
    case Fl_Native_File_Chooser::BROWSE_SAVE_FILE:
      return(Fl_File_Chooser::SINGLE | Fl_File_Chooser::CREATE);
    case Fl_Native_File_Chooser::BROWSE_SAVE_DIRECTORY:
      return(Fl_File_Chooser::DIRECTORY | Fl_File_Chooser::MULTI | Fl_File_Chooser::CREATE);
    default:
      return(Fl_File_Chooser::SINGLE);
  }
}

void Fl_Native_File_Chooser_FLTK_Driver::type(int val) {
  _btype = val;
  _file_chooser->type(type_fl_file(val));
}

int Fl_Native_File_Chooser_FLTK_Driver::type() const {
  return(_btype);
}

void Fl_Native_File_Chooser_FLTK_Driver::options(int val) {
  _options = val;
}

int Fl_Native_File_Chooser_FLTK_Driver::options() const {
  return(_options);
}

int Fl_Native_File_Chooser_FLTK_Driver::show() {

  // FILTER
  if ( _parsedfilt ) {
    _file_chooser->filter(_parsedfilt);
  }

  // FILTER VALUE
  //     Set this /after/ setting the filter
  //
  _file_chooser->filter_value(_filtvalue);

  // DIRECTORY
  if ( _directory && _directory[0] ) {
    _file_chooser->directory(_directory);
  } else {
    _file_chooser->directory(_prevvalue);
  }

  // PRESET FILE
  if ( _preset_file ) {
    _file_chooser->value(_preset_file);
  }

  // OPTIONS: PREVIEW
  _file_chooser->preview( (options() & Fl_Native_File_Chooser::PREVIEW) ? 1 : 0);

  // OPTIONS: NEW FOLDER
  if ( options() & Fl_Native_File_Chooser::NEW_FOLDER )
    _file_chooser->type(_file_chooser->type() | Fl_File_Chooser::CREATE);	// on
  
  // SHOW
  _file_chooser->show();

  // BLOCK WHILE BROWSER SHOWN
  while ( _file_chooser->shown() ) {
    Fl::wait();
  }

  if ( _file_chooser->value() && _file_chooser->value()[0] ) {
    _prevvalue = strfree(_prevvalue);
    _prevvalue = strnew(_file_chooser->value());
    _filtvalue = _file_chooser->filter_value(); // update filter value

    // HANDLE SHOWING 'SaveAs' CONFIRM
    if ( options() & Fl_Native_File_Chooser::SAVEAS_CONFIRM && type() == Fl_Native_File_Chooser::BROWSE_SAVE_FILE ) {
      struct stat buf;
      if ( fl_stat(_file_chooser->value(), &buf) != -1 ) {
        if ( buf.st_mode & S_IFREG ) {    // Regular file + exists?
          if ( exist_dialog() == 0 ) {
            return(1);
          }
        }
      }
    }
  }

  if ( _file_chooser->count() ) return(0);
  else return(1);
}

const char *Fl_Native_File_Chooser_FLTK_Driver::errmsg() const {
  return(_errmsg ? _errmsg : "No error");
}

const char* Fl_Native_File_Chooser_FLTK_Driver::filename() const {
  if ( _file_chooser->count() > 0 ) {
    return(_file_chooser->value());
  }
  return("");
}

const char* Fl_Native_File_Chooser_FLTK_Driver::filename(int i) const {
  if ( i < _file_chooser->count() )
    return(_file_chooser->value(i+1));  // convert fltk 1 based to our 0 based
  return("");
}

void Fl_Native_File_Chooser_FLTK_Driver::title(const char *val) {
  _file_chooser->label(val);
}

const char *Fl_Native_File_Chooser_FLTK_Driver::title() const {
  return(_file_chooser->label());
}

void Fl_Native_File_Chooser_FLTK_Driver::filter(const char *val) {
  _filter = strfree(_filter);
  _filter = strnew(val);
  parse_filter();
}

const char *Fl_Native_File_Chooser_FLTK_Driver::filter() const {
  return(_filter);
}

int Fl_Native_File_Chooser_FLTK_Driver::filters() const {
  return(_nfilters);
}

void Fl_Native_File_Chooser_FLTK_Driver::filter_value(int val) {
  _filtvalue = val;
}

int Fl_Native_File_Chooser_FLTK_Driver::filter_value() const {
  return _filtvalue;
}

int Fl_Native_File_Chooser_FLTK_Driver::count() const {
  return _file_chooser->count();
}

void Fl_Native_File_Chooser_FLTK_Driver::directory(const char *val) {
  _directory = strfree(_directory);
  _directory = strnew(val);
}

const char *Fl_Native_File_Chooser_FLTK_Driver::directory() const {
  return _directory;
}

// PRIVATE: Convert our filter format to fltk's chooser format
//     FROM                                     TO (FLTK)
//     -------------------------                --------------------------
//     "*.cxx"                                  "*.cxx Files(*.cxx)"
//     "C Files\t*.{cxx,h}"                     "C Files(*.{cxx,h})"
//     "C Files\t*.{cxx,h}\nText Files\t*.txt"  "C Files(*.{cxx,h})\tText Files(*.txt)"
//
//     Returns a modified version of the filter that the caller is responsible
//     for freeing with strfree().
//
void Fl_Native_File_Chooser_FLTK_Driver::parse_filter() {
  _parsedfilt = strfree(_parsedfilt);	// clear previous parsed filter (if any)
  _nfilters = 0;
  char *in = _filter;
  if ( !in ) return;

  int has_name = strchr(in, '\t') ? 1 : 0;

  char mode = has_name ? 'n' : 'w';	// parse mode: n=title, w=wildcard
  char wildcard[1024] = "";		// parsed wildcard
  char name[1024] = "";

  // Parse filter user specified
  for ( ; 1; in++ ) {
    /*** DEBUG
    printf("WORKING ON '%c': mode=<%c> name=<%s> wildcard=<%s>\n",
			*in, mode,     name,     wildcard);
    ***/

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
	// APPEND NEW FILTER TO LIST
	if ( wildcard[0] ) {
	  // OUT: "name(wild)\tname(wild)"
	  char comp[2048];
	  sprintf(comp, "%s%.511s(%.511s)", ((_parsedfilt)?"\t":""),
					    name, wildcard);
	  _parsedfilt = strapp(_parsedfilt, comp);
	  _nfilters++;
	  //DEBUG printf("DEBUG: PARSED FILT NOW <%s>\n", _parsedfilt);
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

void Fl_Native_File_Chooser_FLTK_Driver::preset_file(const char* val) {
  _preset_file = strfree(_preset_file);
  _preset_file = strnew(val);
}

const char* Fl_Native_File_Chooser_FLTK_Driver::preset_file() const {
  return _preset_file;
}

int Fl_Native_File_Chooser_FLTK_Driver::exist_dialog() {
  return fl_choice("%s", fl_cancel, fl_ok, NULL, Fl_Native_File_Chooser::file_exists_message);
}

//
// End of "$Id$".
//
