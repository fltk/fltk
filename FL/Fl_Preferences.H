//
// "$Id$"
//
// Preferences .
//
// Copyright 2002-2005 by Matthias Melcher.
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
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

/** \file
   Fl_Preferences class . */

#ifndef Fl_Preferences_H
#  define Fl_Preferences_H

#  ifdef WIN32
#    include <windows.h>
#  endif // WIN32

#  include <stdio.h>
#  include "Fl_Export.H"


/**
   Fl_Preferences provides methods to store user
   settings between application starts. It is similar to the
   Registry on WIN32 and Preferences on MacOS, and provides a
   simple configuration mechanism for UNIX.
   
   Fl_Preferences uses a hierarchy to store data. It
   bundles similar data into groups and manages entries into those
   groups as name/value pairs.
   
   Preferences are stored in text files that can be edited
   manually. The file format is easy to read and relatively
   forgiving. Preferences files are the same on all platforms. User
   comments in preference files are preserved. Filenames are unique
   for each application by using a vendor/application naming
   scheme. The user must provide default values for all entries to
   ensure proper operation should preferences be corrupted or not
   yet exist.
   
   Entries can be of any length. However, the size of each
   preferences file should be kept under 100k for performance
   reasons. One application can have multiple preferences files.
   Extensive binary data however should be stored in separate
   files; see getUserdataPath().
 */
class FL_EXPORT Fl_Preferences 
{

public:

  /**
     Define the scope of the preferences.
   */
  enum Root { 
    SYSTEM=0,   ///< Preferences are used system-wide
    USER        ///< Preferences apply only to the current user
  };
  // enum Type { win32, macos, fltk };

  /**
     The constructor creates a group that manages name/value pairs and
     child groups. Groups are ready for reading and writing at any time.
     The root argument is either Fl_Preferences::USER
     or Fl_Preferences::SYSTEM.
     
     This constructor creates the <i>base</i> instance for all
     following entries and reads existing databases into memory. The
     vendor argument is a unique text string identifying the
     development team or vendor of an application.  A domain name or
     an EMail address are great unique names, e.g.
     "researchATmatthiasm.com" or "fltk.org". The
     application argument can be the working title or final
     name of your application. Both vendor and
     application must be valid relative UNIX pathnames and
     may contain '/'s to create deeper file structures.
    
     \param[in] root can be USER or SYSTEM for user specific or system wide preferences
     \param[in] vendor unique text describing the company or author of this file
     \param[in] application unique text describing the application
   */
  Fl_Preferences( Root root, const char *vendor, const char *application );

  /**
     This constructor is used to create or read a preferences file at an
     arbitrary position in the file system. The file name is generated
     as <i>path</i>/<i>application</i>.prefs. If <i>application</i>
     is 0, <i>path</i> must contain the full file name.
     
     \param[in] path path to the directory that contains the preferences file
     \param[in] vendor unique text describing the company or author of this file
     \param[in] application unique text describing the application
   */
  Fl_Preferences( const char *path, const char *vendor, const char *application );

  /**
     This constructor generates a new group of preference entries
     inside the group or file <i>parent</i>. The group argument 
     identifies a group of entries. It can contain '/'s to get quick 
     access to individual elements inside the hierarchy.
    
     \param[in] parent reference object for the new group
     \param[in] group name of the group to access (may contain '/'s)
   */
  Fl_Preferences( Fl_Preferences &parent, const char *group );

  /**
     \see Fl_Preferences( Fl_Preferences&, const char *group )
   */
  Fl_Preferences( Fl_Preferences*, const char *group );

  /**
     The destructor removes allocated resources. When used on the
     <i>base</i> preferences group, the destructor flushes all
     changes to the preferences file and deletes all internal
     databases.
   */
  ~Fl_Preferences();


  /**
     Returns the number of groups that are contained within a
     group.
    
     \return 0 for no groups at all
   */
  int groups();

  /**
     Returns the name of the Nth group. There is no guaranteed
     order  of group names. The index must be within the range given
     by groups().
     
     \param[in] index number indexing the requested group
     \return cstring pointer to the group name
   */
  const char *group( int index );
  
  /**
     Returns non-zero if a group with this name exists.
     Groupnames are relative to the Preferences node and can contain a path.
     "." describes the current node, "./" describes the topmost node.
     By preceding a groupname with a "./", its path becomes relative to the topmost node.
     
     \param[in] group name of group that is searched for
     \return 0 if group was not found
   */
  char groupExists( const char *group );

  /**
     Deletes a group.
    
     \param[in] group name of the group to delete
     \return 0 if call failed
   */
  char deleteGroup( const char *group );


  /**
     Returns the number of entries (name/value pairs) in a group.
    
     \return number of entries
   */
  int entries();

  /**
     Returns the name of an entry. There is no guaranteed order of
     entry names. The index must be within the range given by
     entries().
    
     \param[in] index number indexing the requested entry
     \return pointer to value cstring
   */
  const char *entry( int index );

  /**
     Returns non-zero if an entry with this name exists.
     
     \param[in] entry name of entry that is searched for
     \return 0 if entry was not found
   */
  char entryExists( const char *entry );
  
  /**
     Removes a single entry (name/value pair).
    
     \param[in] entry name of entry to delete
     \return 0 if deleting the entry failed
   */
  char deleteEntry( const char *entry );

  /** 
     Sets an entry (name/value pair). The return value indicates if there
     was a problem storing the data in memory. However it does not
     reflect if the value was actually stored in the preferences
     file.
    
     \param[in] entry name of entry
     \param[in] value set this entry to \a value
     \return 0 if setting the value failed
   */
  char set( const char *entry, int value );

  /** 
     Sets an entry (name/value pair). The return value indicates if there
     was a problem storing the data in memory. However it does not
     reflect if the value was actually stored in the preferences
     file.
    
     \param[in] entry name of entry
     \param[in] value set this entry to \a value
     \return 0 if setting the value failed
   */
  char set( const char *entry, float value );

  /** 
     Sets an entry (name/value pair). The return value indicates if there
     was a problem storing the data in memory. However it does not
     reflect if the value was actually stored in the preferences
     file.
    
     \param[in] entry name of entry
     \param[in] value set this entry to \a value
     \param[in] precision number of decimal digits to represent value
     \return 0 if setting the value failed
   */
  char set( const char *entry, float value, int precision );

  /** 
     Sets an entry (name/value pair). The return value indicates if there
     was a problem storing the data in memory. However it does not
     reflect if the value was actually stored in the preferences
     file.
    
     \param[in] entry name of entry
     \param[in] value set this entry to \a value
     \return 0 if setting the value failed
   */
  char set( const char *entry, double value );

  /** 
     Sets an entry (name/value pair). The return value indicates if there
     was a problem storing the data in memory. However it does not
     reflect if the value was actually stored in the preferences
     file.
    
     \param[in] entry name of entry
     \param[in] value set this entry to \a value
     \param[in] precision number of decimal digits to represent value
     \return 0 if setting the value failed
   */
  char set( const char *entry, double value, int precision );

  /** 
     Sets an entry (name/value pair). The return value indicates if there
     was a problem storing the data in memory. However it does not
     reflect if the value was actually stored in the preferences
     file.
    
     \param[in] entry name of entry
     \param[in] value set this entry to \a value
     \return 0 if setting the value failed
   */
  char set( const char *entry, const char *value );

  /** 
     Sets an entry (name/value pair). The return value indicates if there
     was a problem storing the data in memory. However it does not
     reflect if the value was actually stored in the preferences
     file.
    
     \param[in] entry name of entry
     \param[in] value set this entry to \a value
     \param[in] size of data array
     \return 0 if setting the value failed
   */
  char set( const char *entry, const void *value, int size ); 


  /**
     Reads an entry from the group. A default value must be
     supplied. The return value indicates if the value was available
     (non-zero) or the default was used (0).
    
     \param[in] entry name of entry
     \param[out] value returned from preferences or default value if none was set
     \param[in] defaultValue default value to be used if no preference was set
     \return 0 if the default value was used 
   */
  char get( const char *entry, int &value,    int defaultValue );

  /**
     Reads an entry from the group. A default value must be
     supplied. The return value indicates if the value was available
     (non-zero) or the default was used (0). 
    
     \param[in] entry name of entry
     \param[out] value returned from preferences or default value if none was set
     \param[in] defaultValue default value to be used if no preference was set
     \return 0 if the default value was used 
   */
  char get( const char *entry, float &value,  float defaultValue );

  /**
     Reads an entry from the group. A default value must be
     supplied. The return value indicates if the value was available
     (non-zero) or the default was used (0). 
    
     \param[in] entry name of entry
     \param[out] value returned from preferences or default value if none was set
     \param[in] defaultValue default value to be used if no preference was set
     \return 0 if the default value was used 
   */
  char get( const char *entry, double &value, double defaultValue );

  /**
     Reads an entry from the group. A default value must be
     supplied. The return value indicates if the value was available
     (non-zero) or the default was used (0). get() allocates memory of
     sufficient size to hold the value. The buffer must be free'd by 
     the developer using 'free(value)'. 
    
     \param[in] entry name of entry
     \param[out] value returned from preferences or default value if none was set
     \param[in] defaultValue default value to be used if no preference was set
     \return 0 if the default value was used 
   */
  char get( const char *entry, char *&value,  const char *defaultValue );

  /**
     Reads an entry from the group. A default value must be
     supplied. The return value indicates if the value was available
     (non-zero) or the default was used (0). 
     'maxSize' is the maximum length of text that will be read. 
     The text buffer must allow for one additional byte for a trailling zero.
    
     \param[in] entry name of entry
     \param[out] value returned from preferences or default value if none was set
     \param[in] defaultValue default value to be used if no preference was set
     \param[in] maxSize maximum length of value plus one byte for a trailing zero
     \return 0 if the default value was used 
   */
  char get( const char *entry, char *value,   const char *defaultValue, int maxSize );

  /**
     Reads an entry from the group. A default value must be
     supplied. The return value indicates if the value was available
     (non-zero) or the default was used (0). get() allocates memory of
     sufficient size to hold the value. The buffer must be free'd by 
     the developer using 'free(value)'. 
    
     \param[in] entry name of entry
     \param[out] value returned from preferences or default value if none was set
     \param[in] defaultValue default value to be used if no preference was set
     \param[in] defaultSize size of default value array
     \return 0 if the default value was used 
   */
  char get( const char *entry, void *&value,  const void *defaultValue, int defaultSize );

  /**
     Reads an entry from the group. A default value must be
     supplied. The return value indicates if the value was available
     (non-zero) or the default was used (0). 
     'maxSize' is the maximum length of text that will be read. 
    
     \param[in] entry name of entry
     \param[out] value returned from preferences or default value if none was set
     \param[in] defaultValue default value to be used if no preference was set
     \param[in] defaultSize size of default value array
     \param[in] maxSize maximum length of value
     \return 0 if the default value was used 
    
     \todo maxSize should receive the number of bytes that were read.
   */
  char get( const char *entry, void *value,   const void *defaultValue, int defaultSize, int maxSize );

  /**
     Returns the size of the value part of an entry.
    
     \return size of value
   */
  int size( const char *entry );


  /**
     Creates a path that is related to the preferences file and
     that is usable for application data beyond what is covered by
     Fl_Preferences.
    
     \param[out] path buffer for user data path
     \param[in] pathlen size of path buffer
     \return 0 if path was not created or pathname can't fit into buffer
   */
  char getUserdataPath( char *path, int pathlen );


  /**
     Write all preferences to disk. This function works only with
     the base preference group. This function is rarely used as
     deleting the base preferences flushes automatically.
   */
  void flush();

  // char export( const char *filename, Type fileFormat );
  // char import( const char *filename );

  /**
     'Name' provides a simple method to create numerical or more complex 
     procedural names for entries and groups on the fly.
     
     Example: prefs.set(Fl_Preferences::Name("File%d",i),file[i]);. 
    
     See test/preferences.cxx as a sample for writing arrays into preferences.<p>
     'Name' is actually implemented as a class inside Fl_Preferences. It casts
     into const char* and gets automatically destroyed after the enclosing call
     ends.
   */
  class FL_EXPORT Name {

    char *data_;

  public:

    /**
       Create a numerical name.
     */
    Name( unsigned int n );

    /**
       Create a name using 'printf' style formatting.
     */
    Name( const char *format, ... );

    /**
       Return the Name as a c-string.
       \internal
     */
    operator const char *() { return data_; }
    ~Name();
  };

  struct Entry
  {
    char *name, *value;
  };

private:

  // make the following functions unavailable
  Fl_Preferences(); 
  Fl_Preferences(const Fl_Preferences&); 
  Fl_Preferences &operator=(const Fl_Preferences&);

  static char nameBuffer[128];

  class FL_EXPORT Node // a node contains a list to all its entries 
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
    Node *search( const char *path, int offset=0 );
    Node *addChild( const char *path );
    void setParent( Node *parent );
    Node *parent() { return parent_; }
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

  class FL_EXPORT RootNode  // the root node manages file paths and basic reading and writing
  {
    Fl_Preferences *prefs_;
    char *filename_;
    char *vendor_, *application_;
  public:
    RootNode( Fl_Preferences *, Root root, const char *vendor, const char *application );
    RootNode( Fl_Preferences *, const char *path, const char *vendor, const char *application );
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
// End of "$Id$".
//
