//
// "$Id$"
//
// IDE and Build FIle generation for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010 by Matthias Melcher.
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

#ifndef FLUID_IDE_SUPPORT_H
#define FLUID_IDE_SUPPORT_H

#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>


typedef char Fl_UUID[40];
typedef char Fl_XCID[25];

extern void fl_getUUID(Fl_Preferences &db, const char *key, char *buffer);
extern void fl_getXCID(Fl_Preferences &db, const char *key, char *buffer);


/* Shortcut to retrieve or create a UUID from the database */
#define MAKE_UUID(name, db) \
  char name[40]; fl_getUUID(db, #name, name);                                          

/* Shortcut to retrieve, but not create a UUID from the database */
#define GET_UUID(name, db) \
  char name[40]; db.get(#name, name, "DBERROR", 40);

/* Shortcut to retrieve or create a UUID from the database */
#define MAKE_XCID(name, db) \
char name[25]; fl_getXCID(db, #name, name);                                          

/* Shortcut to retrieve, but not create a UUID from the database */
#define GET_XCID(name, db) \
char name[25]; db.get(#name, name, "DBERROR", 40);


/* A preferences node with some additional functionality */
class Fl_IDE_Prefs : public Fl_Preferences {
public:
  Fl_IDE_Prefs(Fl_Preferences &parent, const char *name);
  Fl_IDE_Prefs(Fl_Preferences &parent, int ix);
  Fl_IDE_Prefs(Fl_Preferences::ID id);
  Fl_Preferences::ID find_by_name(const char *name);
  Fl_Preferences::ID find_by_key(const char *key, const char *value);
  Fl_Preferences::ID add_with_key(const char *key, const char *value, const char *uuid=0);
};


class Fl_Target_Prefs : public Fl_IDE_Prefs {
public:
  Fl_Target_Prefs(Fl_Preferences::ID id);
  Fl_Preferences::ID add_source(Fl_IDE_Prefs &fdb, const char *pathAndName);
  Fl_Preferences::ID add_header(Fl_IDE_Prefs &fdb, const char *pathAndName);
  Fl_Preferences::ID add_fl(Fl_IDE_Prefs &fdb, const char *pathAndName);
  Fl_Preferences::ID depends_on(Fl_IDE_Prefs &dep);
  Fl_Preferences::ID add_lib(Fl_IDE_Prefs &lib);
  Fl_Preferences::ID add_external_lib(Fl_IDE_Prefs &fdb, const char *pathAndName);
};


class Fl_File_Prefs : public Fl_Preferences {
  char pPath[1024];
  char pName[80];
  char pFullName[100];
  char pExt[20];
public:
  Fl_File_Prefs(Fl_Preferences &parent, const char *name);
  Fl_File_Prefs(Fl_Preferences &parent, int ix);
  Fl_File_Prefs(Fl_Preferences::ID id);
  const char *filePath();
  const char *fileName();
  const char *fullName();
  const char *fileExt();
};

void xcode_only(Fl_Preferences::ID id);
void visualc_only(Fl_Preferences::ID id);
void makefile_only(Fl_Preferences::ID id);

char with_xcode(Fl_Preferences::ID id);
char with_visualc(Fl_Preferences::ID id);
char with_makefile(Fl_Preferences::ID id);

void ui_load_database(const char *filename);

#endif

//
// End of "$Id$".
//
