//
// "$Id: Fl_Preferences.cxx,v 1.1.2.2 2002/04/29 15:12:23 easysw Exp $"
//
// Preferences file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2002 by Matthias Melcher.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//


#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>

#include <stdio.h>
#include <stdlib.h>
#include "flstring.h"

#include <sys/stat.h>


char Fl_Preferences::nameBuffer[];


/**
 * create the initial preferences base
 * i root: machine or user preferences
 * i vendor: unique identification of author or vendor of application
 *     Must be a valid directory name.
 * i application: vendor unique application name, i.e. "PreferencesTest"
 *     multiple preferences files can be created per application.
 *     Must be a valid file name.
 * example: Fl_Preferences base( Fl_Preferences::USER, "matthiasm.com", "test01");
 */
Fl_Preferences::Fl_Preferences( enum Root root, const char *vendor, const char *application )
{
  node     = new Node( "." );
  rootNode = new RootNode( this, root, vendor, application );
}


/**
 * create a Preferences node in relation to a parent node for reading and writing
 * i parent: base name for group
 * i group: group name (can contain '/' seperated group names)
 * example: Fl_Preferences colors( base, "setup/colors" );
 */
Fl_Preferences::Fl_Preferences( Fl_Preferences &parent, const char *key )
{
  rootNode = 0;
  node     = parent.node->addChild( key );
}


/**
 * create a Preferences node in relation to a parent node for reading and writing
 * i parent: base name for group
 * i group: group name (can contain '/' seperated group names)
 * example: Fl_Preferences colors( base, "setup/colors" );
 */
Fl_Preferences::Fl_Preferences( Fl_Preferences *parent, const char *key )
{
  rootNode = 0;
  node     = parent->node->addChild( key );
}


/**
 * destroy individual keys
 * - destroying the base preferences will flush changes to the prefs file
 * - after destroying the base, none of the depending preferences must be read or written
 */
Fl_Preferences::~Fl_Preferences()
{
  delete rootNode;
  // DO NOT delete nodes! The root node will do that after writing the preferences
}


/**
 * return the number of groups that are contained within a group
 * example: int n = base.groups();
 */
int Fl_Preferences::groups()
{
  return node->nChildren();
}


/**
 * return the group name of the n'th group
 * - there is no guaranteed order of group names
 * - the index must be within the range given by groups()
 * example: printf( "Group(%d)='%s'\n", ix, base.group(ix) );
 */
const char *Fl_Preferences::group( int ix )
{
  return node->child( ix );
}


/**
 * return 1, if a group with this name exists
 * example: if ( base.groupExists( "setup/colors" ) ) ...
 */
int Fl_Preferences::groupExists( const char *key )
{
  return node->search( key ) ? 1 : 0 ;
}


/**
 * delete a group
 * example: setup.deleteGroup( "colors/buttons" );
 */
int Fl_Preferences::deleteGroup( const char *key )
{
  Node *nd = node->search( key );
  if ( nd ) return nd->remove();
  return 0;
}


/**
 * return the number of entries (name/value) pairs for a group
 * example: int n = buttonColor.entries();
 */
int Fl_Preferences::entries()
{
  return node->nEntry;
}


/**
 * return the name of an entry
 * - there is no guaranteed order of entry names
 * - the index must be within the range given by entries()
 * example: printf( "Entry(%d)='%s'\n", ix, buttonColor.entry(ix) );
 */
const char *Fl_Preferences::entry( int ix )
{
  return node->entry[ix].name;
}


/**
 * return 1, if a group with this name exists
 * example: if ( buttonColor.entryExists( "red" ) ) ...
 */
int Fl_Preferences::entryExists( const char *key )
{
  return node->getEntry( key ) ? 1 : 0 ;
}


/**
 * remove a single entry (name/value pair)
 * example: buttonColor.deleteEntry( "red" );
 */
int Fl_Preferences::deleteEntry( const char *key )
{
  return node->deleteEntry( key );
}



/**
 * read an entry from the group
 * - a default value must be supplied
 * - the return value indicates, if the value was not available and the default was used (0)
 * example: button.get( "visible", b.visible, 1 );
 */
int Fl_Preferences::get( const char *key, int &value, int defaultValue )
{
  const char *v = node->get( key );
  value = v ? atoi( v ) : defaultValue;
  return 0;
}


/**
 * set an entry (name/value pair)
 */
int Fl_Preferences::set( const char *key, int value )
{
  sprintf( nameBuffer, "%d", value );
  node->set( key, nameBuffer );
  return 1;
}


/**
 * read an entry from the group
 */
int Fl_Preferences::get( const char *key, float &value, float defaultValue )
{
  const char *v = node->get( key );
  value = v ? (float)atof( v ) : defaultValue;
  return 0;
}


/**
 * set an entry (name/value pair)
 */
int Fl_Preferences::set( const char *key, float value )
{
  sprintf( nameBuffer, "%g", value );
  node->set( key, nameBuffer );
  return 1;
}


/**
 * read an entry from the group
 */
int Fl_Preferences::get( const char *key, double &value, double defaultValue )
{
  const char *v = node->get( key );
  value = v ? atof( v ) : defaultValue;
  return 0;
}


/**
 * set an entry (name/value pair)
 */
int Fl_Preferences::set( const char *key, double value )
{
  sprintf( nameBuffer, "%g", value );
  node->set( key, nameBuffer );
  return 1;
}


/**
 * read a text entry from the group
 * - the maximum size for text plus entry name is 2046 bytes plus the trailling 0
 * - the text must not contain special characters
 * the text will be movet into the given text buffer
 */
int Fl_Preferences::get( const char *key, char *text, const char *defaultValue, int maxSize )
{
  maxSize --;
  const char *v = node->get( key );
  if ( !v ) v = defaultValue;
  strncpy( text, v, maxSize );
  if ( (int)strlen(v) >= maxSize ) text[maxSize] = 0;
  return 0;
}


/**
 * read a text entry from the group
 * - the maximum size for text plus entry name is 2046 bytes plus the trailling 0
 * - the text must not contain special characters (no \n or \r, "quotes" are OK)
 * 'text' will be changed to point to a new text buffer
 * the text buffer must be deleted with 'free(text)' by the user.
 */
int Fl_Preferences::get( const char *key, char *&text, const char *defaultValue )
{
  const char *v = node->get( key );
  if ( !v ) v = defaultValue;
  text = strdup( v );
  return 0;
}


/**
 * set an entry (name/value pair)
 */
int Fl_Preferences::set( const char *key, const char *text )
{
  node->set( key, text );
  return 1;
}


/**
 * return the size of the value part of an entry
 */
int Fl_Preferences::size( const char *key )
{
  const char *v = node->get( key );
  return v ? strlen( v ) : 0 ;
}

/**
 * creates a path that is related to the preferences file
 * and that is usable for application data beyond what is covered 
 * by Fl_Preferences.
 * - 'getUserdataPath' actually creates the directory
 * - 'path' must be large enough to receive a complete file path
 * example:
 *   Fl_Preferences prefs( USER, "matthiasm.com", "test" );
 *   char path[MAX_PATH];
 *   prefs.getUserdataPath( path );
 * sample returns:
 *   Win32: c:/Documents and Settings/matt/Application Data/matthiasm.com/test/
 *   prefs: c:/Documents and Settings/matt/Application Data/matthiasm.com/test.prefs
 */
int Fl_Preferences::getUserdataPath( char *path )
{
  if ( rootNode )
    return rootNode->getPath( path );
  return 0;
}

/**
 * write all preferences to disk
 * - this function works only with the base preference group
 * - this function is rarely used as deleting the base preferences does that automatically
 */
void Fl_Preferences::flush()
{
  if ( rootNode && node->dirty() )
    rootNode->write();
}

/*
int Fl_Preferences::export( const char *filename, enum Type type )
{
//#pragma message ( "TODO: implement Fl_Preferences::export(filepath)" )
  switch ( type )
  {
  case win32:
    break;
  case macos:
    break;
  case fltk:
    break;
  }
  return 0;
}


int Fl_Preferences::import( const char *filename )
{
//#pragma message ( "TODO: implement Fl_Preferences::import(filepath)" )
  return 0;
}
*/

//-----------------------------------------------------------------------------
// internal methods, do not change or use as they will change without notice
//

int Fl_Preferences::Node::lastEntrySet = -1;

static int makePath( const char *path )
{
  struct stat stats;
  int ret = stat( path, &stats );
  if ( ret )
  {
    char *s = strrchr( path, '/' );
    if ( !s ) return 0;
    int len = s-path;
    char *p = (char*)malloc( len+1 );
    memcpy( p, path, len );
    p[len] = 0;
    makePath( p );
    free( p );
    return ( mkdir( path, 0777 ) == 0 );
  }
  return 1;
}

static void makePathForFile( const char *path )
{
  char *s = strrchr( path, '/' );
  if ( !s ) return;
  int len = s-path;
  char *p = (char*)malloc( len+1 );
  memcpy( p, path, len );
  p[len] = 0;
  makePath( p );
  free( p );
}

// create the root node
// - construct the name of the file that will hold our preferences
Fl_Preferences::RootNode::RootNode( Fl_Preferences *prefs, enum Root root, const char *vendor, const char *application )
{
  const char	*home;		// Home directory
  char		filename[1024];	// Filename
#ifdef WIN32
  HKEY		key;		// Registry key
  DWORD		size;		// Size of string
  char		data[1024];	// Home (profile) directory
#endif // WIN32

  // Find the home directory...
#ifdef WIN32
  // Open the registry...
  if (RegOpenKeyEx(HKEY_CURRENT_USER,
                   "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 0,
		   KEY_READ, &key)) {
    data[0] = '\0';
  } else {
    // Grab the current user's AppData directory...
    size = sizeof(data);
    if (RegQueryValueEx(key, "AppData", NULL, NULL, (unsigned char *)data, &size))
      data[0] = '\0';

    RegCloseKey(key);
  }

  if (data[0])
    home = data;
  else
    home = NULL;
#else
  home = getenv("HOME");
#endif // WIN32

  // Choose the appropriate path...
  if (root == SYSTEM || !home) {
    // Locate the preferences in the system data directory...
    snprintf(filename, sizeof(filename),
             FLTK_DATADIR "/%s/%s.prefs", vendor, application);
  } else {
    // Locate the preferences in the user data directory...
    snprintf(filename, sizeof(filename),
             "%s/.fltk/%s/%s.prefs", home, vendor, application);
  }

  makePathForFile( filename );
  prefs_ = prefs;
  filename_ = strdup( filename );
  vendor_ = strdup( vendor );
  application_ = strdup( application );
  read(); 
}

// destroy the root node and all depending nodes
Fl_Preferences::RootNode::~RootNode()
{
  if ( prefs_->node->dirty() )
    write();
  if ( filename_ ) 
    free( filename_ );
  delete vendor_;
  delete application_;
  delete prefs_->node;
}

// read a preferences file and construct the group tree and with all entry leafs
int Fl_Preferences::RootNode::read()
{
  char buf[1024];
  FILE *f = fopen( filename_, "rb" );
  if ( !f ) return 0;
  fgets( buf, 1024, f );
  fgets( buf, 1024, f );
  fgets( buf, 1024, f );
  Node *nd = prefs_->node;
  for (;;)
  {
    if ( !fgets( buf, 1024, f ) ) break;	// EOF or Error
    if ( buf[0]=='[' ) // read a new group
    {
      int end = strcspn( buf+1, "]\n\r" );
      buf[ end+1 ] = 0;
      nd = prefs_->node->find( buf+1 );
    }
    else if ( buf[0]=='+' ) // 
    { // value of previous name/value pair spans multiple lines
      int end = strcspn( buf+1, "\n\r" );
      if ( end != 0 ) // if entry is not empty
      {
	buf[ end+1 ] = 0;
	nd->add( buf+1 );
      }
    }
    else // read a name/value pair
    {
      int end = strcspn( buf, "\n\r" );
      if ( end != 0 ) // if entry is not empty
      {
	buf[ end ] = 0;
	nd->set( buf );
      }
    }
  }
  fclose( f );
  return 0;
}

// write the group tree and all entry leafs
int Fl_Preferences::RootNode::write()
{
  FILE *f = fopen( filename_, "wb" );
  if ( !f ) return 1;
  fprintf( f, "; FLTK preferences file format 1.0\n" );
  fprintf( f, "; vendor: %s\n", vendor_ );
  fprintf( f, "; application: %s\n", application_ );
  prefs_->node->write( f );
  fclose( f );
  return 0;
}

// get the path to the preferences directory
int Fl_Preferences::RootNode::getPath( char *path )
{
  char *s;

  strcpy( path, filename_ );
  for ( s = path; *s; s++ ) if ( *s == '\\' ) *s = '/';
  s = strrchr( path, '.' );
  if ( !s ) return 0;
  *s = 0;
  int ret = makePath( path );
  strcpy( s, "/" );
  return ret;
}

// create a node that represents a group
// - path must be a single word, prferable alnum(), dot and underscore only. Space is ok.
Fl_Preferences::Node::Node( const char *path )
{
  if ( path ) path_ = strdup( path ); else path_ = 0;
  child_ = 0; next_ = 0; parent_ = 0;
  entry = 0;
  nEntry = NEntry = 0;
  dirty_ = 0;
}

// delete this and all depending nodes
Fl_Preferences::Node::~Node()
{
  Node *nx;
  for ( Node *nd = child_; nd; nd = nx )
  {
    nx = nd->next_;
    delete nd;
  }
  for ( int i = 0; i < nEntry; i++ )
  {
    delete entry[i].name;
    delete entry[i].value;
  }
  delete[] entry;
  if ( path_ ) free( path_ );
}

// recursively check if any entry is dirty (was changed after loading a fresh prefs file)
int Fl_Preferences::Node::dirty()
{
  if ( dirty_ ) return 1;
  if ( next_ && next_->dirty() ) return 1;
  if ( child_ && child_->dirty() ) return 1;
  return 0;
}

// write this node (recursively from the last neighbor back to this)
// write all entries
// write all children
int Fl_Preferences::Node::write( FILE *f )
{
  if ( next_ ) next_->write( f );
  fprintf( f, "\n[%s]\n\n", path_ );
  for ( int i = 0; i < nEntry; i++ )
  {
    char *src = entry[i].value;
    if ( src )
    { // hack it into smaller pieces if needed
      fprintf( f, "%s:", entry[i].name );
      int cnt;
      for ( cnt = 0; cnt < 60; cnt++ )
	if ( src[cnt]==0 ) break;
      fwrite( src, cnt, 1, f );
      fprintf( f, "\n" );
      src += cnt;
      for (;*src;)
      {
	for ( cnt = 0; cnt < 80; cnt++ )
	  if ( src[cnt]==0 ) break;
        fputc( '+', f );
	fwrite( src, cnt, 1, f );
        fputc( '\n', f );
	src += cnt;
      }
    }
    else
      fprintf( f, "%s\n", entry[i].name );
  }
  if ( child_ ) child_->write( f );
  dirty_ = 0;
  return 0;
}

// set the parent node and create the full path
void Fl_Preferences::Node::setParent( Node *pn )
{
  parent_ = pn;
  next_ = pn->child_;
  pn->child_ = this;
  sprintf( nameBuffer, "%s/%s", pn->path_, path_ );
  free( path_ );
  path_ = strdup( nameBuffer );
}

// add a child to this node and set its path (try to find it first...)
Fl_Preferences::Node *Fl_Preferences::Node::addChild( const char *path )
{
  sprintf( nameBuffer, "%s/%s", path_, path );
  char *name = strdup( nameBuffer );
  Node *nd = find( nameBuffer );
  free( name );
  return nd;
}

// create and set, or change an entry within this node
void Fl_Preferences::Node::set( const char *name, const char *value )
{
  for ( int i=0; i<nEntry; i++ )
  {
    if ( strcmp( name, entry[i].name ) == 0 )
    {
      if ( !value ) return; // annotation
      if ( strcmp( value, entry[i].value ) != 0 )
      {
	delete entry[i].value;
	entry[i].value = strdup( value );
	dirty_ = 1;
      }
      lastEntrySet = i;
      return;
    }
  }
  if ( NEntry==nEntry )
  {
    NEntry = NEntry ? NEntry*2 : 10;
    entry = (Entry*)realloc( entry, NEntry * sizeof(Entry) );
  }
  entry[ nEntry ].name = strdup( name );
  entry[ nEntry ].value = value?strdup( value ):0;
  lastEntrySet = nEntry;
  nEntry++;
  dirty_ = 1;
}

// create or set a value (or annotation) from a single line in the file buffer
void Fl_Preferences::Node::set( const char *line )
{
  int dirty = dirty_; // hmm. If we assume that we always read yhis file in the beginning, we can handle the dirty flag 'quick and dirty'
  if ( line[0]==';' || line[0]==0 || line[0]=='#' )
  {
    set( line, 0 );
  }
  else
  {
    char *c = strchr( line, ':' );
    if ( c )
    {
      strncpy( nameBuffer, line, c-line );
      nameBuffer[ c-line ] = 0;
      set( nameBuffer, c+1 );
    }
    else
      set( line, "" );
  }
  dirty_ = dirty;
}

// add more data to an existing entry
void Fl_Preferences::Node::add( const char *line )
{
  if ( lastEntrySet<0 || lastEntrySet>=nEntry ) return;
  char *&dst = entry[ lastEntrySet ].value;
  int a = strlen( dst );
  int b = strlen( line );
  dst = (char*)realloc( dst, a+b+1 );
  memcpy( dst+a, line, b+1 );
  dirty_ = 1;
}

// get the value for a name, returns 0 if no such name
const char *Fl_Preferences::Node::get( const char *name )
{
  int i = getEntry( name );
  return i>=0 ? entry[i].value : 0 ;
}

// find the index of an entry, returns -1 if no such entry
int Fl_Preferences::Node::getEntry( const char *name )
{
  for ( int i=0; i<nEntry; i++ )
  {
    if ( strcmp( name, entry[i].name ) == 0 )
    {
      return i;
    }
  }
  return -1;
}

// remove one entry form this group
int Fl_Preferences::Node::deleteEntry( const char *name )
{
  int ix = getEntry( name );
  if ( ix == -1 ) return 0;
  memmove( entry+ix, entry+ix+1, (nEntry-ix-1) * sizeof(Entry) );
  nEntry--;
  return 1;
}

// find a group somewhere in the tree starting here
// - this method will always return a valid node (except for memory allocation problems)
// - if the node was not found, 'find' will create the required branch
Fl_Preferences::Node *Fl_Preferences::Node::find( const char *path )
{
  int len = strlen( path_ );
  if ( strncmp( path, path_, len ) == 0 )
  {
    if ( path[ len ] == 0 ) 
      return this;
    if ( path[ len ] == '/' )
    {
      Node *nd;
      for ( nd = child_; nd; nd = nd->next_ )
      {
	Node *nn = nd->find( path );
	if ( nn ) return nn;
      }
      const char *s = path+len+1;
      const char *e = strchr( s, '/' );
      if ( e ) { strncpy( nameBuffer, s, e-s ); nameBuffer[ e-s ] = 0; } else strcpy( nameBuffer, s );
      nd = new Node( nameBuffer );
      nd->setParent( this );
      return nd->find( path );
    }
  }
  return 0;
}

// find a group somewhere in the tree starting here
// - if the node does not exist, 'search' returns NULL
Fl_Preferences::Node *Fl_Preferences::Node::search( const char *path )
{
  int len = strlen( path_ );
  if ( strncmp( path, path_, len ) == 0 )
  {
    if ( path[ len ] == 0 ) 
      return this;
    if ( path[ len ] == '/' )
    {
      Node *nd;

      for ( nd = child_; nd; nd = nd->next_ )
      {
	Node *nn = nd->find( path );
	if ( nn ) return nn;
      }
      return 0;
    }
  }
  return 0;
}

// return the number of child nodes (groups)
int Fl_Preferences::Node::nChildren()
{
  int cnt = 0;
  for ( Node *nd = child_; nd; nd = nd->next_ )
    cnt++;
  return cnt;
}

// return the n'th child node
const char *Fl_Preferences::Node::child( int ix )
{
  Node *nd;
  for ( nd = child_; nd; nd = nd->next_ )
  {
    if ( !ix-- ) break;
  }
  if ( nd && nd->path_ )
  {
    char *r = strrchr( nd->path_, '/' );
    return r ? r+1 : nd->path_ ;
  }
  return 0L ;
}

// remove myself from the list and delete me (and all children)
int Fl_Preferences::Node::remove()
{
  Node *nd, *np;
  if ( parent_ ) {
    nd = parent_->child_; np = 0;
    for ( ; nd; nd = nd->next_ ) {
      if ( nd == this ) {
	if ( np ) 
	  np->next_ = nd->next_; 
	else 
	  parent_->child_ = nd->next_;
	break;
      }
    }
  } else nd = 0;

  delete this;

  return ( nd != 0 );
}


//
// End of "$Id: Fl_Preferences.cxx,v 1.1.2.2 2002/04/29 15:12:23 easysw Exp $".
//
