//
// "$Id: Fl_Preferences.H,v 1.1.2.1 2002/04/28 16:41:16 easysw Exp $"
//
// Preferences header file for the Fast Light Tool Kit (FLTK).
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
// Please report all bugs and problems to "fltk-bugs@matthiasm.com".
//

#ifndef Fl_Preferences_H
#define Fl_Preferences_H

#  include <stdio.h>

/* missing functions:

  Fl_Preferences should return functions to create and access User Data directories
  Fl_Preferences could offer functions that return the value instead off an error code to write directly into widgets
*/

/**
 * Preferences are a data tree containing a root, branches and leafs
 */
class Fl_Preferences 
{

public:

  typedef enum { SYSTEM=0, USER } Root;
  // typedef enum { win32, macos, fltk } Type;

  FL_EXPORT Fl_Preferences( enum Root root, const char *vendor, const char *application );
  FL_EXPORT Fl_Preferences( Fl_Preferences&, const char *group );
  FL_EXPORT Fl_Preferences( Fl_Preferences*, const char *group );
  FL_EXPORT ~Fl_Preferences();

  FL_EXPORT int groups();
  FL_EXPORT const char *group( int );
  FL_EXPORT int groupExists( const char *group );
  FL_EXPORT int deleteGroup( const char *group );

  FL_EXPORT int entries();
  FL_EXPORT const char *entry( int );
  FL_EXPORT int entryExists( const char *entry );
  FL_EXPORT int deleteEntry( const char *entry );

  FL_EXPORT int set( const char *entry, int value );
  FL_EXPORT int set( const char *entry, float value );
  FL_EXPORT int set( const char *entry, double value );
  FL_EXPORT int set( const char *entry, const char *value );
  // FL_EXPORT int set( const char *entry, const void *value, int size );

  FL_EXPORT int get( const char *entry, int &value,    int defaultValue );
  FL_EXPORT int get( const char *entry, float &value,  float defaultValue );
  FL_EXPORT int get( const char *entry, double &value, double defaultValue );
  FL_EXPORT int get( const char *entry, char *&value,  const char *defaultValue );
  FL_EXPORT int get( const char *entry, char *value,   const char *defaultValue, int maxSize );
  // FL_EXPORT int get( const char *entry, void *&value,  const char *defaultValue );
  // FL_EXPORT int get( const char *entry, void *value,   const char *defaultValue, int maxSize );
  FL_EXPORT int size( const char *entry );

  FL_EXPORT int getUserdataPath( char *path );

  FL_EXPORT void flush();

  // FL_EXPORT int export( const char *filename, enum Type fileFormat );
  // FL_EXPORT int import( const char *filename );

  // FL_EXPORT const char *namef( const char *, ... );

private:

  static char nameBuffer[128];

  struct Entry
  {
    char *name, *value;
  };

  class Node 
  {
    Node *child_, *next_, *parent_;
    char *path_;
    int dirty_;
  public:
    Node( const char *path );
    ~Node();
    Node *find( const char *path );
    Node *search( const char *path );
    int write( FILE *f );
    void setParent( Node *parent );
    Node *addChild( const char *path );
    void set( const char *name, const char *value );
    void set( const char *line );
    void add( const char *line );
    const char *get( const char *name );
    int getEntry( const char *name );
    int deleteEntry( const char *name );
    int remove();
    int dirty();
    int nChildren();
    const char *child( int ix );
    Entry *entry;
    int nEntry, NEntry;
    static int lastEntrySet;
  };
  friend class Node;

  class RootNode
  {
    Fl_Preferences *prefs_; //++ maybe we should hold a reference to the first node
    char *filename_;
    char *vendor_, *application_;
  public:
    RootNode( Fl_Preferences *, enum Root root, const char *vendor, const char *application );
    ~RootNode();
    int read();
    int write();
    int getPath( char *path );
  };
  friend class RootNode;

  Node *node;
  RootNode *rootNode;
};


#endif

//
// End of "$Id: Fl_Preferences.H,v 1.1.2.1 2002/04/28 16:41:16 easysw Exp $".
//
