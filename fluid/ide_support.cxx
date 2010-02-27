//
// "$Id$"
//
// IDE and Build File generation for the Fast Light Tool Kit (FLTK).
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

/*
 IDE SUPPORT IN FLUID
 
 One of the biggest issues in maintaining FLTK has been the maintenance of build 
 environments. Users are confused when new features of FLTK ar not available
 yet for their platform, and may never be unless a volunteer updates and 
 maintains the corresponding IDE support. 
 
 "ide support" will generate sets of IDE files for all supported build 
 environments, including Makefiles and CMake. Ading a new source file to the 
 FLTK build tree is as easy as adding the file to the database and 
 running Fluid.
 
 This module will give Fluid the power to create a database for FLTK based 
 projects. By linking further IDE support modules, the database can be used
 to create IDE build files for many platforms, compilers, and development 
 environments.
 
 This module creates a database specifically for the FLTK project itself. A
 user interface shold be created to create databases for arbitrary projects.
 
 > fluid -fltkdb <filename>     # creates the fltk database
 
 
 DATABASE FORMAT
 
 The database is constructed using Fl_Preferences. Objects in the database are
 defined by using a UUID as the group name. References to objects are done using 
 the same UUID as a value. The mandatory structure is defined in the first block
 below. When writing IDE files, additional optional key/value pairs may be 
 generated. Their key should be marked by a unique prefix.
 
 Standard database format:
 
 /projectName:                  # name of the project (FLTK)
 
 /ide/                          # contains IDE specific data
 /targets/                      # contains various groups for differen target types
 /./libs/                       # targets that will be buildt as libraries
 /././UUID/                     # [multiple] description of a library target
 /./././sources/                # all source files required to build the library
 /././././UUID/                 # [multiple] description of this dependency
 /./././././refUUID:            # reference to file definitin in /files/
 /./././libs/                   # all libraries required to build the library
 /././././....                  # see /targets/libs/UUID/sources/...
 /./././fluid/                  # all Fluid UI defs required to build the library
 /././././....                  # see /targets/libs/UUID/sources/...
 /./././externals/              # all external libraries required to build the library
 /././././....                  # see /targets/libs/UUID/sources/...
 /./././deps/                   # all dependencies on other targets in this project
 /././././....                  # see /targets/libs/UUID/sources/...
 /./apps/                       # full blown applications
 /././UUID/                     # [multiple] description of a application target
 /./././...                     # see /targets/libs/UUID/...
 /./tests/                      # smaller test applications
 /././UUID/                     # [multiple] description of a test app target
 /./././...                     # see /targets/libs/UUID/...
 /files/                        # more information on al referenced files
 /./UUID/                       # [multiple] description of a file
 /././pathAndName:              # path and file name relative to the archive root
 
 Xcode3 IDE FILES: see ide_xcode.cxx
 
 VISUALC 2005 IDE FILES: not yet implemented
 
 VISUALC 2008 IDE FILES: not yet implemented
 
 VISUALC 2010 IDE FILES: not yet implemented
 
 CMAKE FILES: not yet implemented
 
 MAKEFILE SYSTEM: not yet implemented
 
 others: not yet implemented
 
 */

#include "ide_support.h"
#include "ide_support_ui.h"

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Widget.H>
#include <FL/filename.H>

#include "Fl_Type.h"

/* 
 * Read a UUID from a database entry. If none exists, create one in the database. 
 */
void getUUID(Fl_Preferences &db, const char *key, char *buffer) {
  db.get(key, buffer, "", 37);
  if (buffer[0]==0) {
    strcpy(buffer, Fl_Preferences::newUUID());
    db.set(key, buffer);
  }
}

/* 
 * Read an Xcode ID from a database entry. If none exists, create one in the database. 
 * The Xcode ID contains 24 bytes of hexadecimal chracters.
 */
void getXCID(Fl_Preferences &db, const char *key, char *buffer) {
  db.get(key, buffer, "", 25);
  if (buffer[0]==0) {
    const char *uuid = Fl_Preferences::newUUID();
    // A UUID comes in blocks of 8, 4, 4, 4, and 12 characters
    unsigned int i0, i1, i2, i3, i4, i5;
    sscanf(uuid, "%8x-%4x-%4x-%4x-%4x%8x", &i0, &i1, &i2, &i3, &i4, &i5);
    sprintf(buffer, "%8.8X%4.4X%4.4X%8.8X", i0, i1^i2, i3^i4, i5);
    db.set(key, buffer);
  }
}

/* A preferences node with some additional functionality */
Fl_IDE_Prefs::Fl_IDE_Prefs(Fl_Preferences &parent, const char *name) 
: Fl_Preferences(parent, name) { 
}

Fl_IDE_Prefs::Fl_IDE_Prefs(Fl_Preferences &parent, int ix) 
: Fl_Preferences(parent, ix) { 
}

Fl_IDE_Prefs::Fl_IDE_Prefs(Fl_Preferences::ID id) 
: Fl_Preferences(id) { 
}

Fl_Preferences::ID Fl_IDE_Prefs::find_by_name(const char *name) {
  int i, n = groups();
  for (i=0; i<n; i++) {
    Fl_Preferences p(this, i);
    if (strcmp(name, p.name())==0) return p.id();
  }
  return 0;
}

Fl_Preferences::ID Fl_IDE_Prefs::find_by_key(const char *key, const char *value) {
  int i, n = groups();
  for (i=0; i<n; i++) {
    Fl_Preferences p(this, i);
    char *v;
    p.get(key, v, "");
    if (strcmp(value, v)==0) { free(v); return p.id(); }
    free(v);
  }
  return 0;
}

Fl_Preferences::ID Fl_IDE_Prefs::add_with_key(const char *key, const char *value, const char *uuid) {
  Fl_Preferences::ID ret = find_by_key(key, value);
  if (!ret) {
    if (!uuid) uuid = Fl_Preferences::newUUID();
    Fl_Preferences p(this, uuid);
    p.set(key, value);
    ret = p.id();
  }
  return ret;
}


Fl_Target_Prefs::Fl_Target_Prefs(Fl_Preferences::ID id) 
: Fl_IDE_Prefs(id) { 
}

Fl_Preferences::ID Fl_Target_Prefs::add_source(Fl_IDE_Prefs &fdb, const char *pathAndName) {
  Fl_IDE_Prefs file(fdb.add_with_key("pathAndName", pathAndName));
  Fl_IDE_Prefs p(*this, "sources");
  return p.add_with_key("refUUID", file.name());
}

Fl_Preferences::ID Fl_Target_Prefs::add_fl(Fl_IDE_Prefs &fdb, const char *pathAndName) {
  Fl_IDE_Prefs file(fdb.add_with_key("pathAndName", pathAndName));
  Fl_IDE_Prefs p(*this, "fl");
  return p.add_with_key("refUUID", file.name());
}

Fl_Preferences::ID Fl_Target_Prefs::depends_on(Fl_IDE_Prefs &dep) {
  Fl_IDE_Prefs p(*this, "deps");
  return p.add_with_key("refUUID", dep.name());
}

Fl_Preferences::ID Fl_Target_Prefs::add_lib(Fl_IDE_Prefs &lib) {
  this->depends_on(lib);
  Fl_IDE_Prefs p(*this, "libs");
  return p.add_with_key("refUUID", lib.name());
}

Fl_Preferences::ID Fl_Target_Prefs::add_external_lib(Fl_IDE_Prefs &fdb, const char *pathAndName) {
  Fl_IDE_Prefs file(fdb.add_with_key("pathAndName", pathAndName));
  Fl_IDE_Prefs p(*this, "externals");
  return p.add_with_key("refUUID", file.name());
}


Fl_File_Prefs::Fl_File_Prefs(Fl_Preferences &parent, const char *name) 
: Fl_Preferences(parent, name) { 
}

Fl_File_Prefs::Fl_File_Prefs(Fl_Preferences &parent, int ix) 
: Fl_Preferences(parent, ix) { 
}

Fl_File_Prefs::Fl_File_Prefs(Fl_Preferences::ID id) 
: Fl_Preferences(id) { 
}

const char *Fl_File_Prefs::filePath() { 
  char pan[1024]; get("pathAndName", pan, "DBERROR/DBERROR.DBERR", 1024);
  strcpy(pPath, pan);
  char *name = (char*)fl_filename_name(pPath);
  if (name) *name = 0;
  return pPath;
}

const char *Fl_File_Prefs::fileName() { 
  char pan[1024]; get("pathAndName", pan, "DBERROR/DBERROR.DBERR", 1024);
  char *name = (char*)fl_filename_name(pan);
  if (name) {
    strcpy(pName, name);
  } else {
    strcpy(pName, pan);
  }
  char *ext = (char*)fl_filename_ext(pName);
  if (ext) *ext = 0;
  return pName;
}

const char *Fl_File_Prefs::fullName() { 
  char pan[1024]; get("pathAndName", pan, "DBERROR/DBERROR.DBERR", 1024);
  char *name = (char*)fl_filename_name(pan);
  if (name) {
    strcpy(pFullName, name);
  } else {
    strcpy(pFullName, pan);
  }
  return pFullName;
}

const char *Fl_File_Prefs::fileExt() { 
  char pan[1024]; get("pathAndName", pan, "DBERROR/DBERROR.DBERR", 1024);
  char *ext = (char*)fl_filename_ext(pan);
  if (ext) strcpy(pExt, ext);
  else pExt[0] = 0;
  return pExt;
}

void xcode_only(Fl_Preferences::ID id) {
  Fl_Preferences p(id);
  p.set("only", "xcode");
}

char with_xcode(Fl_Preferences::ID id) {
  Fl_Preferences p(id);
  if (p.entryExists("only")) {
    char os[16];
    p.get("only", os, "xcode", 15);
    return (strcmp(os, "xcode")==0);
  } else {
    return 1;
  }
}

void visualc_only(Fl_Preferences::ID id) {
  Fl_Preferences p(id);
  p.set("only", "visualc");
}

char with_visualc(Fl_Preferences::ID id) {
  Fl_Preferences p(id);
  if (p.entryExists("only")) {
    char os[16];
    p.get("only", os, "visualc", 15);
    return (strcmp(os, "visualc")==0);
  } else {
    return 1;
  }
}

void makefile_only(Fl_Preferences::ID id) {
  Fl_Preferences p(id);
  p.set("only", "makefile");
}

char with_makefile(Fl_Preferences::ID id) {
  Fl_Preferences p(id);
  if (p.entryExists("only")) {
    char os[16];
    p.get("only", os, "makefile", 15);
    return (strcmp(os, "makefile")==0);
  } else {
    return 1;
  }
}


//==============================================================================


int create_new_database(const char *filename)
{
  Fl_Preferences *db = new Fl_Preferences(filename, "fltk.org", 0);
  // TODO: we do not clear the database anymore! This shoudl keep UUID's 
  // consistent and reduce the size of check-ins. When removing items from
  // the db, we have to do that manually!
  //db->clear();
  
  db->set("projectName", "fltk");
  
  Fl_Preferences targets_db(db, "targets");
  Fl_IDE_Prefs files_db(*db, "files");
  
  // --- create libraries
  Fl_IDE_Prefs libs_db(targets_db, "libs"); 
  
  Fl_Target_Prefs fltk_lib(libs_db.add_with_key("name", "fltk")); {
    fltk_lib.add_source(files_db, "src/Fl.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Adjuster.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Bitmap.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Box.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Browser.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Browser_.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Browser_load.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Button.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Chart.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Check_Browser.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Check_Button.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Choice.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Clock.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Color_Chooser.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Counter.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Dial.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Double_Window.cxx");
    fltk_lib.add_source(files_db, "src/Fl_File_Browser.cxx");
    fltk_lib.add_source(files_db, "src/Fl_File_Chooser.cxx");
    fltk_lib.add_source(files_db, "src/Fl_File_Chooser2.cxx");
    fltk_lib.add_source(files_db, "src/Fl_File_Icon.cxx");
    fltk_lib.add_source(files_db, "src/Fl_File_Input.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Group.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Help_View.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Image.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Input.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Input_.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Light_Button.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Menu.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Menu_.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Menu_Bar.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Menu_Button.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Menu_Window.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Menu_add.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Menu_global.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Multi_Label.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Native_File_Chooser.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Overlay_Window.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Pack.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Pixmap.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Positioner.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Preferences.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Progress.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Repeat_Button.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Return_Button.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Roller.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Round_Button.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Scroll.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Scrollbar.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Shared_Image.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Single_Window.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Slider.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Sys_Menu_Bar.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Table.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Table_Row.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Tabs.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Text_Buffer.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Text_Display.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Text_Editor.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Tile.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Tiled_Image.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Tooltip.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Tree.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Tree_Item.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Tree_Item_Array.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Tree_Prefs.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Valuator.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Value_Input.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Value_Output.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Value_Slider.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Widget.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Window.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Window_fullscreen.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Window_hotspot.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Window_iconize.cxx");
    fltk_lib.add_source(files_db, "src/Fl_Wizard.cxx");
    fltk_lib.add_source(files_db, "src/Fl_XBM_Image.cxx");
    fltk_lib.add_source(files_db, "src/Fl_XPM_Image.cxx");
    fltk_lib.add_source(files_db, "src/Fl_abort.cxx");
    fltk_lib.add_source(files_db, "src/Fl_add_idle.cxx");
    fltk_lib.add_source(files_db, "src/Fl_arg.cxx");
    fltk_lib.add_source(files_db, "src/Fl_compose.cxx");
    fltk_lib.add_source(files_db, "src/Fl_display.cxx");
    fltk_lib.add_source(files_db, "src/Fl_get_key.cxx");
    fltk_lib.add_source(files_db, "src/Fl_get_system_colors.cxx");
    fltk_lib.add_source(files_db, "src/Fl_grab.cxx");
    fltk_lib.add_source(files_db, "src/Fl_lock.cxx");
    fltk_lib.add_source(files_db, "src/Fl_own_colormap.cxx");
    fltk_lib.add_source(files_db, "src/Fl_visual.cxx");
    fltk_lib.add_source(files_db, "src/Fl_x.cxx");
    fltk_lib.add_source(files_db, "src/cmap.cxx");
    fltk_lib.add_source(files_db, "src/filename_absolute.cxx");
    fltk_lib.add_source(files_db, "src/filename_expand.cxx");
    fltk_lib.add_source(files_db, "src/filename_ext.cxx");
    fltk_lib.add_source(files_db, "src/filename_isdir.cxx");
    fltk_lib.add_source(files_db, "src/filename_list.cxx");
    fltk_lib.add_source(files_db, "src/filename_match.cxx");
    fltk_lib.add_source(files_db, "src/filename_setext.cxx");
    fltk_lib.add_source(files_db, "src/fl_arc.cxx");
    fltk_lib.add_source(files_db, "src/fl_arci.cxx");
    fltk_lib.add_source(files_db, "src/fl_ask.cxx");
    fltk_lib.add_source(files_db, "src/fl_boxtype.cxx");
    fltk_lib.add_source(files_db, "src/fl_call_main.c");
    fltk_lib.add_source(files_db, "src/fl_color.cxx");
    fltk_lib.add_source(files_db, "src/fl_cursor.cxx");
    fltk_lib.add_source(files_db, "src/fl_curve.cxx");
    fltk_lib.add_source(files_db, "src/fl_diamond_box.cxx");
    fltk_lib.add_source(files_db, "src/fl_dnd.cxx");
    fltk_lib.add_source(files_db, "src/fl_draw.cxx");
    fltk_lib.add_source(files_db, "src/fl_draw_image.cxx");
    fltk_lib.add_source(files_db, "src/fl_draw_pixmap.cxx");
    fltk_lib.add_source(files_db, "src/fl_encoding_latin1.cxx");
    fltk_lib.add_source(files_db, "src/fl_encoding_mac_roman.cxx");
    fltk_lib.add_source(files_db, "src/fl_engraved_label.cxx");
    fltk_lib.add_source(files_db, "src/fl_file_dir.cxx");
    fltk_lib.add_source(files_db, "src/fl_font.cxx");
    fltk_lib.add_source(files_db, "src/fl_gtk.cxx");
    fltk_lib.add_source(files_db, "src/fl_labeltype.cxx");
    fltk_lib.add_source(files_db, "src/fl_line_style.cxx");
    fltk_lib.add_source(files_db, "src/fl_open_uri.cxx");
    fltk_lib.add_source(files_db, "src/fl_oval_box.cxx");
    fltk_lib.add_source(files_db, "src/fl_overlay.cxx");
    fltk_lib.add_source(files_db, "src/fl_overlay_visual.cxx");
    fltk_lib.add_source(files_db, "src/fl_plastic.cxx");
    fltk_lib.add_source(files_db, "src/fl_read_image.cxx");
    fltk_lib.add_source(files_db, "src/fl_rect.cxx");
    fltk_lib.add_source(files_db, "src/fl_round_box.cxx");
    fltk_lib.add_source(files_db, "src/fl_rounded_box.cxx");
    fltk_lib.add_source(files_db, "src/fl_scroll_area.cxx");
    fltk_lib.add_source(files_db, "src/fl_set_font.cxx");
    fltk_lib.add_source(files_db, "src/fl_set_fonts.cxx");
    fltk_lib.add_source(files_db, "src/fl_shadow_box.cxx");
    fltk_lib.add_source(files_db, "src/fl_shortcut.cxx");
    fltk_lib.add_source(files_db, "src/fl_show_colormap.cxx");
    fltk_lib.add_source(files_db, "src/fl_symbols.cxx");
    fltk_lib.add_source(files_db, "src/fl_utf.c");
    fltk_lib.add_source(files_db, "src/fl_utf8.cxx");
    fltk_lib.add_source(files_db, "src/fl_vertex.cxx");
    fltk_lib.add_source(files_db, "src/flstring.c");
    fltk_lib.add_source(files_db, "src/numericsort.c");
    fltk_lib.add_source(files_db, "src/scandir.c");
    fltk_lib.add_source(files_db, "src/screen_xywh.cxx");
    fltk_lib.add_source(files_db, "src/vsnprintf.c");
    fltk_lib.add_source(files_db, "src/xutf8/case.c");
    fltk_lib.add_source(files_db, "src/xutf8/is_right2left.c");
    fltk_lib.add_source(files_db, "src/xutf8/is_spacing.c");
  }
  
  Fl_Target_Prefs fltk_gl_lib(libs_db.add_with_key("name", "fltkgl")); {
    fltk_gl_lib.add_source(files_db, "src/Fl_Gl_Choice.cxx");
    fltk_gl_lib.add_source(files_db, "src/Fl_Gl_Overlay.cxx");
    fltk_gl_lib.add_source(files_db, "src/Fl_Gl_Window.cxx");
    fltk_gl_lib.add_source(files_db, "src/freeglut_geometry.cxx");
    fltk_gl_lib.add_source(files_db, "src/freeglut_stroke_mono_roman.cxx");
    fltk_gl_lib.add_source(files_db, "src/freeglut_stroke_roman.cxx");
    fltk_gl_lib.add_source(files_db, "src/freeglut_teapot.cxx");
    fltk_gl_lib.add_source(files_db, "src/gl_draw.cxx");
    fltk_gl_lib.add_source(files_db, "src/glut_compatability.cxx");
    fltk_gl_lib.add_source(files_db, "src/glut_font.cxx");
    xcode_only(fltk_gl_lib.add_external_lib(files_db, "/System/Library/Frameworks/OpenGL.framework"));
    xcode_only(fltk_gl_lib.add_external_lib(files_db, "/System/Library/Frameworks/AGL.framework"));
    fltk_gl_lib.add_lib(fltk_lib);
  }
  
  Fl_Target_Prefs fltk_images_lib(libs_db.add_with_key("name", "fltkimages")); {
    fltk_images_lib.add_source(files_db, "src/Fl_BMP_Image.cxx");
    fltk_images_lib.add_source(files_db, "src/Fl_File_Icon2.cxx");
    fltk_images_lib.add_source(files_db, "src/Fl_GIF_Image.cxx");
    fltk_images_lib.add_source(files_db, "src/Fl_Help_Dialog.cxx");
    fltk_images_lib.add_source(files_db, "src/Fl_JPEG_Image.cxx");
    fltk_images_lib.add_source(files_db, "src/Fl_PNG_Image.cxx");
    fltk_images_lib.add_source(files_db, "src/Fl_PNM_Image.cxx");
    fltk_images_lib.add_source(files_db, "src/fl_images_core.cxx");
    fltk_images_lib.add_lib(fltk_lib);
  }
  
  Fl_Target_Prefs fltk_png_lib(libs_db.add_with_key("name", "fltkpng")); {
    fltk_png_lib.add_source(files_db, "png/png.c");
    fltk_png_lib.add_source(files_db, "png/pngerror.c");
    fltk_png_lib.add_source(files_db, "png/pngget.c");
    fltk_png_lib.add_source(files_db, "png/pngmem.c");
    fltk_png_lib.add_source(files_db, "png/pngpread.c");
    fltk_png_lib.add_source(files_db, "png/pngread.c");
    fltk_png_lib.add_source(files_db, "png/pngrio.c");
    fltk_png_lib.add_source(files_db, "png/pngrtran.c");
    fltk_png_lib.add_source(files_db, "png/pngrutil.c");
    fltk_png_lib.add_source(files_db, "png/pngset.c");
    fltk_png_lib.add_source(files_db, "png/pngtrans.c");
    fltk_png_lib.add_source(files_db, "png/pngwio.c");
    fltk_png_lib.add_source(files_db, "png/pngwrite.c");
    fltk_png_lib.add_source(files_db, "png/pngwtran.c");
    fltk_png_lib.add_source(files_db, "png/pngwutil.c");
    xcode_only(fltk_png_lib.add_external_lib(files_db, "/usr/lib/libz.dylib"));
    fltk_images_lib.add_lib(fltk_png_lib);
  }
  
  Fl_Target_Prefs fltk_jpeg_lib(libs_db.add_with_key("name", "fltkjpeg")); {
    fltk_jpeg_lib.add_source(files_db, "jpeg/jcapimin.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jcapistd.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jccoefct.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jccolor.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jcdctmgr.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jchuff.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jcinit.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jcmainct.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jcmarker.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jcmaster.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jcomapi.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jcparam.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jcphuff.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jcprepct.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jcsample.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jctrans.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdapimin.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdapistd.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdatadst.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdatasrc.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdcoefct.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdcolor.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jddctmgr.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdhuff.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdinput.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdmainct.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdmarker.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdmaster.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdmerge.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdphuff.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdpostct.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdsample.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jdtrans.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jerror.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jfdctflt.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jfdctfst.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jfdctint.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jidctflt.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jidctfst.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jidctint.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jidctred.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jmemmgr.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jmemnobs.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jquant1.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jquant2.c");
    fltk_jpeg_lib.add_source(files_db, "jpeg/jutils.c");
    fltk_jpeg_lib.add_lib(fltk_lib);
    fltk_images_lib.add_lib(fltk_jpeg_lib);
  }
  
  Fl_Target_Prefs fltk_forms_lib(libs_db.add_with_key("name", "fltkforms")); {
    fltk_forms_lib.add_source(files_db, "src/forms_bitmap.cxx");
    fltk_forms_lib.add_source(files_db, "src/forms_compatability.cxx");
    fltk_forms_lib.add_source(files_db, "src/forms_free.cxx");
    fltk_forms_lib.add_source(files_db, "src/forms_fselect.cxx");
    fltk_forms_lib.add_source(files_db, "src/forms_pixmap.cxx");
    fltk_forms_lib.add_source(files_db, "src/forms_timer.cxx");
    fltk_forms_lib.add_lib(fltk_lib);
  }
  
  Fl_Target_Prefs fltk_z_lib(libs_db.add_with_key("name", "zlib")); {
    fltk_z_lib.add_source(files_db, "zlib/adler32.c");
    fltk_z_lib.add_source(files_db, "zlib/compress.c");
    fltk_z_lib.add_source(files_db, "zlib/crc32.c");
    fltk_z_lib.add_source(files_db, "zlib/deflate.c");
    fltk_z_lib.add_source(files_db, "zlib/gzio.c");
    fltk_z_lib.add_source(files_db, "zlib/inffast.c");
    fltk_z_lib.add_source(files_db, "zlib/inflate.c");
    fltk_z_lib.add_source(files_db, "zlib/inftrees.c");
    fltk_z_lib.add_source(files_db, "zlib/trees.c");
    fltk_z_lib.add_source(files_db, "zlib/uncompr.c");
    fltk_z_lib.add_source(files_db, "zlib/zutil.c");
    fltk_png_lib.add_lib(fltk_z_lib);
  }
  
  // --- create applications
  Fl_IDE_Prefs apps_db(targets_db, "apps"); 
  
  Fl_Target_Prefs fluid_app(apps_db.add_with_key("name", "Fluid")); {
    fluid_app.add_source(files_db, "fluid/CodeEditor.cxx");
    fluid_app.add_source(files_db, "fluid/Fl_Function_Type.cxx");
    fluid_app.add_source(files_db, "fluid/Fl_Group_Type.cxx");
    fluid_app.add_source(files_db, "fluid/Fl_Menu_Type.cxx");
    fluid_app.add_source(files_db, "fluid/Fl_Type.cxx");
    fluid_app.add_source(files_db, "fluid/Fl_Widget_Type.cxx");
    fluid_app.add_source(files_db, "fluid/Fl_Window_Type.cxx");
    fluid_app.add_source(files_db, "fluid/Fluid_Image.cxx");
    fluid_app.add_source(files_db, "fluid/about_panel.cxx");
    fluid_app.add_source(files_db, "fluid/align_widget.cxx");
    fluid_app.add_source(files_db, "fluid/alignment_panel.cxx");
    fluid_app.add_source(files_db, "fluid/code.cxx");
    fluid_app.add_source(files_db, "fluid/factory.cxx");
    fluid_app.add_source(files_db, "fluid/file.cxx");
    fluid_app.add_source(files_db, "fluid/fluid.cxx");
    fluid_app.add_source(files_db, "fluid/function_panel.cxx");
    fluid_app.add_source(files_db, "fluid/ide_support.cxx");
    fluid_app.add_source(files_db, "fluid/ide_support_ui.cxx");
    fluid_app.add_source(files_db, "fluid/ide_visualc.cxx");
    fluid_app.add_source(files_db, "fluid/ide_xcode.cxx");
    fluid_app.add_source(files_db, "fluid/template_panel.cxx");
    fluid_app.add_source(files_db, "fluid/undo.cxx");
    fluid_app.add_source(files_db, "fluid/widget_panel.cxx");
    fluid_app.add_lib(fltk_lib);
    fluid_app.add_lib(fltk_forms_lib);
    fluid_app.add_lib(fltk_images_lib);
    fluid_app.add_lib(fltk_jpeg_lib);
    fluid_app.add_lib(fltk_png_lib);
    visualc_only(fluid_app.add_lib(fltk_z_lib));
    xcode_only(fluid_app.add_external_lib(files_db, "icons/fluid.icns"));
  }
  
  // --- create test applications
  Fl_IDE_Prefs tests_db(targets_db, "tests"); 
  
  Fl_Target_Prefs demo_db(tests_db.add_with_key("name", "Demo")); {
    demo_db.add_source(files_db, "test/demo.cxx");
    demo_db.add_lib(fltk_lib); 
    demo_db.depends_on(fluid_app); 
  }
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "adjuster"));
    db.add_source(files_db, "test/adjuster.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "arc"));
    db.add_source(files_db, "test/arc.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "ask"));
    db.add_source(files_db, "test/ask.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
    
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "bitmap"));
    db.add_source(files_db, "test/bitmap.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "blocks"));
    db.add_source(files_db, "test/blocks.cxx");
    db.add_lib(fltk_lib); 
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/CoreAudio.framework"));
    visualc_only(db.add_external_lib(files_db, "winmm.lib"));
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "boxtype"));
    db.add_source(files_db, "test/boxtype.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "browser"));
    db.add_source(files_db, "test/browser.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "button"));
    db.add_source(files_db, "test/button.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "buttons"));
    db.add_source(files_db, "test/buttons.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "checkers"));
    db.add_source(files_db, "test/checkers.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "clock"));
    db.add_source(files_db, "test/clock.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "colbrowser"));
    db.add_source(files_db, "test/colbrowser.cxx");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_forms_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "color_chooser"));
    db.add_source(files_db, "test/color_chooser.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "cube"));
    db.add_source(files_db, "test/cube.cxx");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_gl_lib); 
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/OpenGL.framework"));
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/AGL.framework"));
    visualc_only(db.add_external_lib(files_db, "glu32.lib"));
    visualc_only(db.add_external_lib(files_db, "opengl32.lib"));
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "CubeView"));
    db.add_source(files_db, "test/CubeView.cxx");
    db.add_source(files_db, "test/CubeMain.cxx");
    db.add_fl(files_db, "test/CubeViewUI.fl");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_gl_lib); 
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/OpenGL.framework"));
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/AGL.framework"));
    visualc_only(db.add_external_lib(files_db, "glu32.lib"));
    visualc_only(db.add_external_lib(files_db, "opengl32.lib"));
    db.depends_on(fluid_app);
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "cursor"));
    db.add_source(files_db, "test/cursor.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "curve"));
    db.add_source(files_db, "test/curve.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "doublebuffer"));
    db.add_source(files_db, "test/doublebuffer.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "editor"));
    db.add_source(files_db, "test/editor.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "fast_slow"));
    db.add_fl(files_db, "test/fast_slow.fl");
    db.add_lib(fltk_lib); 
    db.depends_on(fluid_app);
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "file_chooser"));
    db.add_source(files_db, "test/file_chooser.cxx");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_images_lib); 
    db.add_lib(fltk_jpeg_lib); 
    db.add_lib(fltk_png_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "fonts"));
    db.add_source(files_db, "test/fonts.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "forms"));
    db.add_source(files_db, "test/forms.cxx");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_forms_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "fractals"));
    db.add_source(files_db, "test/fractals.cxx");
    db.add_source(files_db, "test/fracviewer.cxx");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_gl_lib); 
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/OpenGL.framework"));
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/AGL.framework"));
    visualc_only(db.add_external_lib(files_db, "glu32.lib"));
    visualc_only(db.add_external_lib(files_db, "opengl32.lib"));
    db.add_lib(fltk_forms_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "fullscreen"));
    db.add_source(files_db, "test/fullscreen.cxx");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_gl_lib); 
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/OpenGL.framework"));
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/AGL.framework"));
    visualc_only(db.add_external_lib(files_db, "glu32.lib"));
    visualc_only(db.add_external_lib(files_db, "opengl32.lib"));
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "gl_overlay"));
    db.add_source(files_db, "test/gl_overlay.cxx");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_gl_lib); 
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/OpenGL.framework"));
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/AGL.framework"));
    visualc_only(db.add_external_lib(files_db, "glu32.lib"));
    visualc_only(db.add_external_lib(files_db, "opengl32.lib"));
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "glpuzzle"));
    db.add_source(files_db, "test/glpuzzle.cxx");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_gl_lib); 
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/OpenGL.framework"));
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/AGL.framework"));
    visualc_only(db.add_external_lib(files_db, "glu32.lib"));
    visualc_only(db.add_external_lib(files_db, "opengl32.lib"));
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "hello"));
    db.add_source(files_db, "test/hello.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "help"));
    db.add_source(files_db, "test/help.cxx");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_images_lib); 
    db.add_lib(fltk_jpeg_lib); 
    db.add_lib(fltk_png_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "iconize"));
    db.add_source(files_db, "test/iconize.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "image"));
    db.add_source(files_db, "test/image.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "inactive"));
    db.add_fl(files_db, "test/inactive.fl");
    db.add_lib(fltk_lib); 
    db.depends_on(fluid_app);
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "input"));
    db.add_source(files_db, "test/input.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "input_choice"));
    db.add_source(files_db, "test/input_choice.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "keyboard"));
    db.add_source(files_db, "test/keyboard.cxx");
    db.add_fl(files_db, "test/keyboard_ui.fl");
    db.add_lib(fltk_lib); 
    db.depends_on(fluid_app);
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "label"));
    db.add_source(files_db, "test/label.cxx");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_forms_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "line_style"));
    db.add_source(files_db, "test/line_style.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "list_visuals"));
    db.add_source(files_db, "test/list_visuals.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "mandelbrot"));
    db.add_source(files_db, "test/mandelbrot.cxx");
    db.add_fl(files_db, "test/mandelbrot_ui.fl");
    db.add_lib(fltk_lib); 
    db.depends_on(fluid_app);
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "menubar"));
    db.add_source(files_db, "test/menubar.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "message"));
    db.add_source(files_db, "test/message.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "minimum"));
    db.add_source(files_db, "test/minimum.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "navigation"));
    db.add_source(files_db, "test/navigation.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "native-filechooser"));
    db.add_source(files_db, "test/native-filechooser.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "output"));
    db.add_source(files_db, "test/output.cxx");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_forms_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "overlay"));
    db.add_source(files_db, "test/overlay.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "pack"));
    db.add_source(files_db, "test/pack.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "pixmap_browser"));
    db.add_source(files_db, "test/pixmap_browser.cxx");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_images_lib); 
    db.add_lib(fltk_jpeg_lib); 
    db.add_lib(fltk_png_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "pixmap"));
    db.add_source(files_db, "test/pixmap.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "preferences"));
    db.add_fl(files_db, "test/preferences.fl");
    db.add_lib(fltk_lib); 
    db.depends_on(fluid_app);
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "radio"));
    db.add_fl(files_db, "test/radio.fl");
    db.add_lib(fltk_lib); 
    db.depends_on(fluid_app);
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "resizebox"));
    db.add_source(files_db, "test/resizebox.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "resize"));
    db.add_fl(files_db, "test/resize.fl");
    db.add_lib(fltk_lib); 
    db.depends_on(fluid_app);
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "scroll"));
    db.add_source(files_db, "test/scroll.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "shape"));
    db.add_source(files_db, "test/shape.cxx");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_gl_lib); 
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/OpenGL.framework"));
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/AGL.framework"));
    visualc_only(db.add_external_lib(files_db, "glu32.lib"));
    visualc_only(db.add_external_lib(files_db, "opengl32.lib"));
    demo_db.depends_on(db);
  }  

  { Fl_Target_Prefs db(tests_db.add_with_key("name", "subwindow"));
    db.add_source(files_db, "test/subwindow.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "sudoku"));
    db.add_source(files_db, "test/sudoku.cxx");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_images_lib); 
    db.add_lib(fltk_jpeg_lib); 
    db.add_lib(fltk_png_lib); 
    xcode_only(db.add_external_lib(files_db, "/System/Library/Frameworks/CoreAudio.framework"));
    visualc_only(db.add_external_lib(files_db, "winmm.lib"));
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "symbols"));
    db.add_source(files_db, "test/symbols.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "table"));
    db.add_source(files_db, "test/table.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "tabs"));
    db.add_fl(files_db, "test/tabs.fl");
    db.add_lib(fltk_lib); 
    db.depends_on(fluid_app);
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "threads"));
    db.add_source(files_db, "test/threads.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "tile"));
    db.add_source(files_db, "test/tile.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "tiled_image"));
    db.add_source(files_db, "test/tiled_image.cxx");
    db.add_lib(fltk_lib); 
    db.add_lib(fltk_images_lib); 
    db.add_lib(fltk_jpeg_lib); 
    db.add_lib(fltk_png_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "tree"));
    db.add_source(files_db, "test/tree.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "utf8"));
    db.add_source(files_db, "test/utf8.cxx");
    db.add_lib(fltk_lib); 
    demo_db.depends_on(db);
  }  
  
  { Fl_Target_Prefs db(tests_db.add_with_key("name", "valuators"));
    db.add_fl(files_db, "test/valuators.fl");
    db.add_lib(fltk_lib); 
    db.depends_on(fluid_app);
    demo_db.depends_on(db);
  }  
    
  db->flush();
  delete db;
  return 0;
}


void ui_load_database(const char *filename)
{
  Fl_Preferences *db = new Fl_Preferences(filename, "fltk.org", 0);
  db->copyTo(dbmanager_tree);
  dbmanager_tree->redraw();
  delete db;
}


// Make this module into a plugin

extern int exit_early;

class Fl_FltkDB_Plugin : public Fl_Commandline_Plugin
{
public:
  Fl_FltkDB_Plugin() : Fl_Commandline_Plugin(name()) { }
  const char *name() { return "FltkDB.fluid.fltk.org"; }
  const char *help() { return
    " --fltkdb <filename> : create a database describing all FLTK project rules"; } 
  int arg(int argc, char **argv, int &i) {
    if (argc>=i+1 && strcmp(argv[i], "--fltkdb")==0) {
      if (argc>=i+2 && argv[i+1][0]!='-') {
        fprintf(stderr, "Creating Databse %s\n", argv[i+1]);
        exit_early = 1;
        create_new_database(argv[i+1]);
        i = i+2;
        return 2;
      } else {
        fprintf(stderr, "Missing argument: --fltkdb <filename>\n");
        return 1;
      }
    }
    return 0;
  }
  int test(const char *a1, const char *a2, const char *a3) {
    create_new_database(a1);
    return 0;
  }
  void show_panel() {
    if (!dbmanager_window)
      make_dbmanager_window();
    dbmanager_window->show();
  }
};

Fl_FltkDB_Plugin FltkDB_Plugin;

//
// End of "$Id$".
//
