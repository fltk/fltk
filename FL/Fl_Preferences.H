//
// "$Id: Fl_Preferences.H,v 1.1.2.7 2002/05/07 00:43:20 easysw Exp $"
//
// Preferences definitions for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Preferences_H
#  define Fl_Preferences_H

#  ifdef WIN32
#    include <windows.h>
#  endif // WIN32

#include <stdio.h>



/**
 * Preferences are a data tree containing a root, branches and leafs
 */
class Fl_Preferences 
{

public:

  enum Root { SYSTEM=0, USER };
  // enum Type { win32, macos, fltk };

  FL_EXPORT Fl_Preferences( Root root, const char *vendor, const char *application );
  FL_EXPORT Fl_Preferences( Fl_Preferences&, const char *group );
  FL_EXPORT Fl_Preferences( Fl_Preferences*, const char *group );
  FL_EXPORT ~Fl_Preferences();

  FL_EXPORT int groups();
  FL_EXPORT const char *group( int );
  FL_EXPORT char groupExists( const char *group );
  FL_EXPORT char deleteGroup( const char *group );

  FL_EXPORT int entries();
  FL_EXPORT const char *entry( int );
  FL_EXPORT char entryExists( const char *entry );
  FL_EXPORT char deleteEntry( const char *entry );

  FL_EXPORT char set( const char *entry, char value );
  FL_EXPORT char set( const char *entry, int value );
  FL_EXPORT char set( const char *entry, float value );
  FL_EXPORT char set( const char *entry, double value );
  FL_EXPORT char set( const char *entry, const char *value );
  FL_EXPORT char set( const char *entry, const void *value, int size ); 

  FL_EXPORT char get( const char *entry, char &value,   char defaultValue );
  FL_EXPORT char get( const char *entry, int &value,    int defaultValue );
  FL_EXPORT char get( const char *entry, float &value,  float defaultValue );
  FL_EXPORT char get( const char *entry, double &value, double defaultValue );
  FL_EXPORT char get( const char *entry, char *&value,  const char *defaultValue );
  FL_EXPORT char get( const char *entry, char *value,   const char *defaultValue, int maxSize );
  FL_EXPORT char get( const char *entry, void *&value,  const void *defaultValue, int defaultSize );
  FL_EXPORT char get( const char *entry, void *value,   const void *defaultValue, int defaultSize, int maxSize );
  FL_EXPORT int size( const char *entry );

  FL_EXPORT char getUserdataPath( char *path, int pathlen );

  FL_EXPORT void flush();

  // FL_EXPORT char export( const char *filename, Type fileFormat );
  // FL_EXPORT char import( const char *filename );

  class Name {
    char *data_;
  public:
    FL_EXPORT Name( unsigned int n );
    FL_EXPORT Name( const char *format, ... );
    FL_EXPORT operator const char *() { return data_; }
    FL_EXPORT ~Name();
  };

  struct Entry
  {
    char *name, *value;
  };

private:

  static char nameBuffer[128];

  class Node // a node contains a list to all its entries 
  {          // and all means to manage the tree structure
    Node *child_, *next_, *parent_;
    char *path_;
    char dirty_;
  public:
    Node( const char *path );
    ~Node();
    // node methods
    int write( FILE *f );
    Node *find( const char *path );
    Node *search( const char *path );
    Node *addChild( const char *path );
    void setParent( Node *parent );
    char remove();
    char dirty();
    // entry methods
    int nChildren();
    const char *child( int ix );
    void set( const char *name, const char *value );
    void set( const char *line );
    void add( const char *line );
    const char *get( const char *name );
    int getEntry( const char *name );
    char deleteEntry( const char *name );
    // public values
    Entry *entry;
    int nEntry, NEntry;
    static int lastEntrySet;
  };
  friend class Node;

  class RootNode  // the root node manages file paths and basic reading and writing
  {
    Fl_Preferences *prefs_;
    char *filename_;
    char *vendor_, *application_;
  public:
    RootNode( Fl_Preferences *, Root root, const char *vendor, const char *application );
    ~RootNode();
    int read();
    int write();
    char getPath( char *path, int pathlen );
  };
  friend class RootNode;

  Node *node;
  RootNode *rootNode;
  
};


#endif // !Fl_Preferences_H

//
// End of "$Id: Fl_Preferences.H,v 1.1.2.7 2002/05/07 00:43:20 easysw Exp $".
//
