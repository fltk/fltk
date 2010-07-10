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
 
 VisualC 6 IDE FILES
 
 This module generates all files needed for a VisualC++ 6.0 description of the 
 FLTK project. Thanks to the developers of Visual Studion, VC6 files can be read
 and converted by all newer versions of VisualC++ as well (tested up to VC2008). 
 There is no need to write any newer file formats.
 
 The VC6 file formats are very simple. The project is described in a .dsw file
 which basically just lists all targets and their interdependencies. The targets
 are then described in .dsp files which list the included source files and the 
 build options.
 
 */

#include "ide_support.h"

#include <FL/filename.H>
#include <FL/fl_ask.H>
#include "../src/flstring.h"

#include "Fl_Type.h"


static void DOSPath(char *s) {
  for (;;) {
    switch (*s) {
      case 0: return;
      case '/': *s = '\\';
    }
    s++;
  }
}

/*
 * This class creates all VisualC++ 6.0 IDE files.
 */
class VisualC6_IDE {
  char *rootDir;
  char projectName[80];
  Fl_Preferences tgtAppsDB;
  int nTgtApps;
  Fl_Preferences tgtLibsDB;
  int nTgtLibs;
  Fl_Preferences tgtTestsDB;
  int nTgtTests;
  Fl_Preferences filesDB;
  int nFiles;
  Fl_Preferences ideDB;
public:
  VisualC6_IDE(Fl_Preferences &db, const char *rootDirA)
  : rootDir(strdup(rootDirA)),
    tgtAppsDB(db, "targets/apps"),
    tgtLibsDB(db, "targets/libs"),
    tgtTestsDB(db, "targets/tests"),
    filesDB(db, "files"),
    ideDB(db, "ide/VisualC")
  {
    db.get("projectName", projectName, "Unnamed", 80);
    nTgtApps = tgtAppsDB.groups();
    nTgtLibs = tgtLibsDB.groups();
    nTgtTests = tgtTestsDB.groups();
    nFiles = filesDB.groups();
  }
  
  ~VisualC6_IDE() {
    if (rootDir) free(rootDir);
  }

  /*
   * Write a project description
   */
  int writeProjectSection(FILE *f, Fl_Preferences &targetDB, char dll=0) {
    char name[80]; targetDB.get("name", name, "DBERROR", 80);
    if (dll) strcat(name, "dll");
    fprintf(f, "###############################################################################\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "Project: \"%s\"=\".\\%s.dsp\" - Package Owner=<4>\r\n", name, name);
    fprintf(f, "\r\n");
    fprintf(f, "Package=<5>\r\n");
    fprintf(f, "{{{\r\n");
    fprintf(f, "}}}\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "Package=<4>\r\n");
    fprintf(f, "{{{\r\n");
    Fl_Preferences depsDB(targetDB, "deps");
    int i, n = depsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences depDB(depsDB, i);
      GET_UUID(refUUID, depDB);
      Fl_Preferences *depTgtDBp = 0;
      if (tgtAppsDB.groupExists(refUUID))
        depTgtDBp = new Fl_Preferences(tgtAppsDB, refUUID);
      else if (tgtLibsDB.groupExists(refUUID))
        depTgtDBp = new Fl_Preferences(tgtLibsDB, refUUID);
      else if (tgtTestsDB.groupExists(refUUID))
        depTgtDBp = new Fl_Preferences(tgtTestsDB, refUUID);
      if (depTgtDBp) {
        char depName[80]; depTgtDBp->get("name", depName, "DBERROR", 80);
        fprintf(f, "    Begin Project Dependency\r\n");
        fprintf(f, "    Project_Dep_Name %s\r\n", depName);
        fprintf(f, "    End Project Dependency\r\n");
      }
    }    
    fprintf(f, "}}}\r\n");
    fprintf(f, "\r\n");
    return 0;
  }

  /*
   * Write the .dsw project file
   */
  int writeProjectFile(const char *filepath) {
    char filename[2048];
    fl_snprintf(filename, 2047, "%s/%s.dsw", filepath, "fltk");
    FILE *f = fopen(filename, "wb");
    if (!f) {
      fl_alert("Can't open file:\n%s", filename);
      return -1;
    }
    fprintf(f, "Microsoft Developer Studio Workspace File, Format Version 6.00\r\n");
    fprintf(f, "# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!\r\n");
    fprintf(f, "\r\n");
    int i;
    // Applications
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      writeProjectSection(f, targetDB);
    }
    // Static Libraries
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeProjectSection(f, targetDB);
    }
    // Dynamically Linking Libraries
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeProjectSection(f, targetDB, 1);
    }
    // Tests
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      writeProjectSection(f, targetDB);
    }
    fprintf(f, "###############################################################################\r\n");
    fprintf(f, "\r\n");
    fclose(f);
    return 0;
  }
      
  /*
   * Write a source file entry.
   */
  int writeSourceFile(FILE *f, Fl_File_Prefs &fileDB) {
    char pathAndName[1024]; fileDB.get("pathAndName", pathAndName, "DBERROR/DBERROR.DBERR", 1024);
    DOSPath(pathAndName);
    fprintf(f, "# Begin Source File\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "SOURCE=..\\..\\%s\r\n", pathAndName);
    fprintf(f, "# End Source File\r\n");
    return 0;
  }
  
  
  /*
   * Write a Fluid file entry.
   */
  int writeFluidFile(FILE *f, Fl_File_Prefs &fileDB, const char *name) {
    char pathAndName[1024]; fileDB.get("pathAndName", pathAndName, "DBERROR/DBERROR.DBERR", 1024);
    char cxx_pathname[1024]; strcpy(cxx_pathname, pathAndName);
    char path_fl[1024]; strcpy(path_fl, fileDB.filePath());
    DOSPath(pathAndName);
    fl_filename_setext(cxx_pathname, 1024, ".cxx");
    DOSPath(cxx_pathname);
    DOSPath(path_fl);
    fprintf(f, "# Begin Source File\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "SOURCE=..\\..\\%s\r\n", cxx_pathname);
    fprintf(f, "# End Source File\r\n");
    fprintf(f, "# Begin Source File\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "SOURCE=..\\..\\%s\r\n", pathAndName);
    fprintf(f, "\r\n");
    fprintf(f, "!IF  \"$(CFG)\" == \"%s - Win32 Release\"\r\n", name);
    fprintf(f, "\r\n");
    fprintf(f, "# Begin Custom Build - Create .cxx and .h file with fluid\r\n");
    fprintf(f, "InputPath=..\\..\\%s\r\n", pathAndName);
    fprintf(f, "\r\n");
    fprintf(f, "\"..\\..\\%s\" : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\r\n", cxx_pathname);
    fprintf(f, "\tpushd ..\\..\\%s \r\n", path_fl);
    fprintf(f, "\t..\\fluid\\fluid -c %s\r\n", fileDB.fullName());
    fprintf(f, "\tpopd \r\n");
    fprintf(f, "\t\r\n");
    fprintf(f, "# End Custom Build\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "!ELSEIF  \"$(CFG)\" == \"%s - Win32 Debug\"\r\n", name);
    fprintf(f, "\r\n");
    fprintf(f, "# Begin Custom Build - Create .cxx and .h file with fluidd\r\n");
    fprintf(f, "InputPath=..\\..\\%s\r\n", pathAndName);
    fprintf(f, "\r\n");
    fprintf(f, "\"..\\..\\%s\" : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\r\n", cxx_pathname);
    fprintf(f, "\tpushd ..\\..\\%s \r\n", path_fl);
    fprintf(f, "\t..\\fluid\\fluidd -c %s \r\n", fileDB.fullName());
    fprintf(f, "\tpopd \r\n");
    fprintf(f, "\t\r\n");
    fprintf(f, "# End Custom Build\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "!ENDIF \r\n");
    fprintf(f, "\r\n");
    fprintf(f, "# End Source File\r\n");
    return 0;
  }
  
  
  /*
   * Write a .dsp target file
   */
  int writeApplicationTarget(const char *filepath, Fl_Preferences &targetDB, const char *dir=0) {
    char name[80]; targetDB.get("name", name, "DBERROR", 80);
    if (!dir) dir = name;
    char filename[2048];
    fl_snprintf(filename, 2047, "%s/%s.dsp", filepath, name);
    FILE *f = fopen(filename, "wb");
    if (!f) {
      fl_alert("Can't open file:\n%s", filename);
      return -1;
    }

    fprintf(f, "# Microsoft Developer Studio Project File - Name=\"%s\" - Package Owner=<4>\r\n", name);
    fprintf(f, "# Microsoft Developer Studio Generated Build File, Format Version 6.00\r\n");
    fprintf(f, "# ** DO NOT EDIT **\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "# TARGTYPE \"Win32 (x86) Application\" 0x0101\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "CFG=%s - Win32 Debug\r\n", name);
    fprintf(f, "!MESSAGE This is not a valid makefile. To build this project using NMAKE,\r\n");
    fprintf(f, "!MESSAGE use the Export Makefile command and run\r\n");
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "!MESSAGE NMAKE /f \"%s.mak\".\r\n", name);
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "!MESSAGE You can specify a configuration when running NMAKE\r\n");
    fprintf(f, "!MESSAGE by defining the macro CFG on the command line. For example:\r\n");
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "!MESSAGE NMAKE /f \"%s.mak\" CFG=\"%s - Win32 Debug\"\r\n", name, name);
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "!MESSAGE Possible choices for configuration are:\r\n");
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "!MESSAGE \"%s - Win32 Release\" (based on \"Win32 (x86) Application\")\r\n", name);
    fprintf(f, "!MESSAGE \"%s - Win32 Debug\" (based on \"Win32 (x86) Application\")\r\n", name);
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "\r\n");
    fprintf(f, "# Begin Project\r\n");
    fprintf(f, "# PROP AllowPerConfigDependencies 0\r\n");
    fprintf(f, "# PROP Scc_ProjName \"\"\r\n");
    fprintf(f, "# PROP Scc_LocalPath \"\"\r\n");
    fprintf(f, "CPP=cl.exe\r\n");
    fprintf(f, "MTL=midl.exe\r\n");
    fprintf(f, "RSC=rc.exe\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "!IF  \"$(CFG)\" == \"%s - Win32 Release\"\r\n", name);
    fprintf(f, "\r\n");
    fprintf(f, "# PROP BASE Use_MFC 0\r\n");
    fprintf(f, "# PROP BASE Use_Debug_Libraries 0\r\n");
    fprintf(f, "# PROP BASE Output_Dir \"Release/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP BASE Intermediate_Dir \"Release/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP BASE Target_Dir \"\"\r\n");
    fprintf(f, "# PROP Use_MFC 0\r\n");
    fprintf(f, "# PROP Use_Debug_Libraries 0\r\n");
    fprintf(f, "# PROP Output_Dir \"Release/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP Intermediate_Dir \"Release/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP Ignore_Export_Lib 0\r\n");
    fprintf(f, "# PROP Target_Dir \"\"\r\n");
    fprintf(f, "# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /YX /FD /c\r\n");
    fprintf(f, "# ADD CPP /nologo /MD /GX /Os /Ob2 /I \".\" /I \"../..\" /I \"../../zlib\" /I \"../../png\" /I \"../../jpeg\" /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"WIN32_LEAN_AND_MEAN\" /D \"VC_EXTRA_LEAN\" /D \"WIN32_EXTRA_LEAN\" /YX /FD /c\r\n");
    fprintf(f, "# ADD BASE MTL /nologo /D \"NDEBUG\" /mktyplib203 /o \"NUL\" /win32\r\n");
    fprintf(f, "# ADD MTL /nologo /D \"NDEBUG\" /mktyplib203 /o \"NUL\" /win32\r\n");
    fprintf(f, "# ADD BASE RSC /l 0x409 /d \"NDEBUG\"\r\n");
    fprintf(f, "# ADD RSC /l 0x409 /d \"NDEBUG\"\r\n");
    fprintf(f, "BSC32=bscmake.exe\r\n");
    fprintf(f, "# ADD BASE BSC32 /nologo\r\n");
    fprintf(f, "# ADD BSC32 /nologo\r\n");
    fprintf(f, "LINK32=link.exe\r\n");
    fprintf(f, "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386\r\n");
    
    fprintf(f, "# ADD LINK32 ");    
    Fl_Preferences libsDB(targetDB, "libs");
    int i, n = libsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences libDB(libsDB, i);
      GET_UUID(refUUID, libDB);
      Fl_Preferences tgtLibDB(tgtLibsDB, refUUID);
      char name[80]; tgtLibDB.get("name", name, "DBERROR", 80);
      fprintf(f, "%s.lib ", name);
    }
    Fl_Preferences extsDB(targetDB, "externals");
    n = extsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences extDB(extsDB, i);
      if (with_visualc(extDB.id())) {
        GET_UUID(refUUID, extDB);
        Fl_File_Prefs fileDB(filesDB, refUUID);
        char pathAndName[1024]; fileDB.get("pathAndName", pathAndName, "DBERROR/DBERROR.DBERR", 1024);
        fprintf(f, "%s ", pathAndName);
      }
    }
    fprintf(f, "comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:\"libcd\" /out:\"../../%s/%s.exe\" /pdbtype:sept /libpath:\"..\\..\\lib\"\r\n", dir, name);
    
    fprintf(f, "# SUBTRACT LINK32 /pdb:none /incremental:yes\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "!ELSEIF  \"$(CFG)\" == \"%s - Win32 Debug\"\r\n", name);
    fprintf(f, "\r\n");
    fprintf(f, "# PROP BASE Use_MFC 0\r\n");
    fprintf(f, "# PROP BASE Use_Debug_Libraries 1\r\n");
    fprintf(f, "# PROP BASE Output_Dir \"Debug/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP BASE Intermediate_Dir \"Debug/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP BASE Target_Dir \"\"\r\n");
    fprintf(f, "# PROP Use_MFC 0\r\n");
    fprintf(f, "# PROP Use_Debug_Libraries 1\r\n");
    fprintf(f, "# PROP Output_Dir \"Debug/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP Intermediate_Dir \"Debug/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP Ignore_Export_Lib 0\r\n");
    fprintf(f, "# PROP Target_Dir \"\"\r\n");
    fprintf(f, "# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /YX /FD /c\r\n");
    fprintf(f, "# ADD CPP /nologo /MDd /Gm /GX /ZI /Od /I \".\" /I \"../..\" /I \"../../zlib\" /I \"../../png\" /I \"../../jpeg\" /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"WIN32_LEAN_AND_MEAN\" /D \"VC_EXTRA_LEAN\" /D \"WIN32_EXTRA_LEAN\" /YX /FD /c\r\n");
    fprintf(f, "# ADD BASE MTL /nologo /D \"_DEBUG\" /mktyplib203 /o \"NUL\" /win32\r\n");
    fprintf(f, "# ADD MTL /nologo /D \"_DEBUG\" /mktyplib203 /o \"NUL\" /win32\r\n");
    fprintf(f, "# ADD BASE RSC /l 0x409 /d \"_DEBUG\"\r\n");
    fprintf(f, "# ADD RSC /l 0x409 /d \"_DEBUG\"\r\n");
    fprintf(f, "BSC32=bscmake.exe\r\n");
    fprintf(f, "# ADD BASE BSC32 /nologo\r\n");
    fprintf(f, "# ADD BSC32 /nologo\r\n");
    fprintf(f, "LINK32=link.exe\r\n");
    fprintf(f, "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept\r\n");
    
    fprintf(f, "# ADD LINK32 ");    
    n = libsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences libDB(libsDB, i);
      GET_UUID(refUUID, libDB);
      Fl_Preferences tgtLibDB(tgtLibsDB, refUUID);
      char name[80]; tgtLibDB.get("name", name, "DBERROR", 80);
      fprintf(f, "%sd.lib ", name);
    }
    n = extsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences extDB(extsDB, i);
      if (with_visualc(extDB.id())) {
        GET_UUID(refUUID, extDB);
        Fl_File_Prefs fileDB(filesDB, refUUID);
        char pathAndName[1024]; fileDB.get("pathAndName", pathAndName, "DBERROR/DBERROR.DBERR", 1024);
        fprintf(f, "%s ", pathAndName);
      }
    }
    fprintf(f, "comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:\"libcd\" /out:\"../../%s/%sd.exe\" /pdbtype:sept /libpath:\"..\\..\\lib\"\r\n", dir, name);
    
    fprintf(f, "# SUBTRACT LINK32 /pdb:none /incremental:no\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "!ENDIF \r\n");
    fprintf(f, "\r\n");
    fprintf(f, "# Begin Target\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "# Name \"%s - Win32 Release\"\r\n", name);
    fprintf(f, "# Name \"%s - Win32 Debug\"\r\n", name);
    
    Fl_Preferences sourcesDB(targetDB, "sources");
    n = sourcesDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences sourceDB(sourcesDB, i);
      GET_UUID(refUUID, sourceDB);
      Fl_File_Prefs fileDB(filesDB, refUUID);
      writeSourceFile(f, fileDB);
    }

    Fl_Preferences flsDB(targetDB, "fl");
    n = flsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences flDB(flsDB, i);
      GET_UUID(refUUID, flDB);
      Fl_File_Prefs fileDB(filesDB, refUUID);
      writeFluidFile(f, fileDB, name);
    }
    
    fprintf(f, "# End Target\r\n");
    fprintf(f, "# End Project\r\n");
    fclose(f);
    return 0;
  }
  
  /*
   * Write a .dsp target file
   * FIXME: this is somehow seen as an application, not a static library 
   * FIXME: the library is written into ide/VisualC6/Debug, and not into lib
   */
  int writeStaticLibTarget(const char *filepath, Fl_Preferences &targetDB, const char *dir=0) {
    char name[80]; targetDB.get("name", name, "DBERROR", 80);
    if (!dir) dir = name;
    char filename[2048];
    fl_snprintf(filename, 2047, "%s/%s.dsp", filepath, name);
    FILE *f = fopen(filename, "wb");
    if (!f) {
      fl_alert("Can't open file:\n%s", filename);
      return -1;
    }
    
    fprintf(f, "# Microsoft Developer Studio Project File - Name=\"%s\" - Package Owner=<4>\r\n", name);
    fprintf(f, "# Microsoft Developer Studio Generated Build File, Format Version 6.00\r\n");
    fprintf(f, "# ** DO NOT EDIT **\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "# TARGTYPE \"Win32 (x86) Static Library\" 0x0104\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "CFG=%s - Win32 Debug\r\n", name);
    fprintf(f, "!MESSAGE This is not a valid makefile. To build this project using NMAKE,\r\n");
    fprintf(f, "!MESSAGE use the Export Makefile command and run\r\n");
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "!MESSAGE NMAKE /f \"%s.mak\".\r\n", name);
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "!MESSAGE You can specify a configuration when running NMAKE\r\n");
    fprintf(f, "!MESSAGE by defining the macro CFG on the command line. For example:\r\n");
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "!MESSAGE NMAKE /f \"%s.mak\" CFG=\"%s - Win32 Debug\"\r\n", name, name);
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "!MESSAGE Possible choices for configuration are:\r\n");
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "!MESSAGE \"%s - Win32 Release\" (based on \"Win32 (x86) Static Library\")\r\n", name);
    fprintf(f, "!MESSAGE \"%s - Win32 Debug\" (based on \"Win32 (x86) Static Library\")\r\n", name);
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "\r\n");
    fprintf(f, "# Begin Project\r\n");
    fprintf(f, "# PROP AllowPerConfigDependencies 0\r\n");
    fprintf(f, "# PROP Scc_ProjName \"\"\r\n");
    fprintf(f, "# PROP Scc_LocalPath \"\"\r\n");
    fprintf(f, "CPP=cl.exe\r\n");
    fprintf(f, "RSC=rc.exe\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "!IF  \"$(CFG)\" == \"%s - Win32 Release\"\r\n", name);
    fprintf(f, "\r\n");
    fprintf(f, "# PROP BASE Use_MFC 0\r\n");
    fprintf(f, "# PROP BASE Use_Debug_Libraries 0\r\n");
    fprintf(f, "# PROP BASE Output_Dir \"Release/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP BASE Intermediate_Dir \"Release/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP BASE Target_Dir \"\"\r\n");
    fprintf(f, "# PROP Use_MFC 0\r\n");
    fprintf(f, "# PROP Use_Debug_Libraries 0\r\n");
    fprintf(f, "# PROP Output_Dir \"Release/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP Intermediate_Dir \"Release/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP Target_Dir \"\"\r\n");
    fprintf(f, "# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /YX /FD /c\r\n");
    fprintf(f, "# ADD CPP /nologo /MD /GX /Ot /Op /Ob2 /I \".\" /I \"../..\" /I \"../../zlib\" /I \"../../png\" /I \"../../jpeg\" /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"WIN32_LEAN_AND_MEAN\" /D \"VC_EXTRA_LEAN\" /D \"WIN32_EXTRA_LEAN\" /YX /FD /c\r\n");
    fprintf(f, "# SUBTRACT CPP /Os\r\n");
    fprintf(f, "# ADD BASE RSC /l 0x409\r\n");
    fprintf(f, "# ADD RSC /l 0x409\r\n");
    fprintf(f, "BSC32=bscmake.exe\r\n");
    fprintf(f, "# ADD BASE BSC32 /nologo\r\n");
    fprintf(f, "# ADD BSC32 /nologo\r\n");
    fprintf(f, "LIB32=link.exe -lib\r\n");
    fprintf(f, "# ADD BASE LIB32 /nologo\r\n");
    fprintf(f, "# ADD LIB32 /nologo /out:\"..\\..\\lib\\%s.lib\"\r\n", name);
    fprintf(f, "\r\n");
    fprintf(f, "!ELSEIF  \"$(CFG)\" == \"%s - Win32 Debug\"\r\n", name);
    fprintf(f, "\r\n");
    fprintf(f, "# PROP BASE Use_MFC 0\r\n");
    fprintf(f, "# PROP BASE Use_Debug_Libraries 1\r\n");
    fprintf(f, "# PROP BASE Output_Dir \"Debug/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP BASE Intermediate_Dir \"Debug/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP BASE Target_Dir \"\"\r\n");
    fprintf(f, "# PROP Use_MFC 0\r\n");
    fprintf(f, "# PROP Use_Debug_Libraries 1\r\n");
    fprintf(f, "# PROP Output_Dir \"Debug/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP Intermediate_Dir \"Debug/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP Target_Dir \"\"\r\n");
    fprintf(f, "# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /YX /FD /c\r\n");
    fprintf(f, "# ADD CPP /nologo /MDd /GX /Z7 /Od /I \".\" /I \"../..\" /I \"../../zlib\" /I \"../../png\" /I \"../../jpeg\" /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"WIN32_LEAN_AND_MEAN\" /D \"VC_EXTRA_LEAN\" /D \"WIN32_EXTRA_LEAN\" /FR /YX /FD /c\r\n");
    fprintf(f, "# ADD BASE RSC /l 0x409\r\n");
    fprintf(f, "# ADD RSC /l 0x409\r\n");
    fprintf(f, "BSC32=bscmake.exe\r\n");
    fprintf(f, "# ADD BASE BSC32 /nologo\r\n");
    fprintf(f, "# ADD BSC32 /nologo\r\n");
    fprintf(f, "LIB32=link.exe -lib\r\n");
    fprintf(f, "# ADD BASE LIB32 /nologo\r\n");
    fprintf(f, "# ADD LIB32 /nologo /out:\"..\\..\\lib\\%sd.lib\"\r\n", name);
    fprintf(f, "\r\n");
    fprintf(f, "!ENDIF \r\n");
    fprintf(f, "\r\n");
    fprintf(f, "# Begin Target\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "# Name \"%s - Win32 Release\"\r\n", name);
    fprintf(f, "# Name \"%s - Win32 Debug\"\r\n", name);
    
    Fl_Preferences sourcesDB(targetDB, "sources");
    int i, n = sourcesDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences sourceDB(sourcesDB, i);
      GET_UUID(refUUID, sourceDB);
      Fl_File_Prefs fileDB(filesDB, refUUID);
      writeSourceFile(f, fileDB);
    }
    
    Fl_Preferences flsDB(targetDB, "fl");
    n = flsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences flDB(flsDB, i);
      GET_UUID(refUUID, flDB);
      Fl_File_Prefs fileDB(filesDB, refUUID);
      writeFluidFile(f, fileDB, name);
    }
    
    fprintf(f, "# End Target\r\n");
    fprintf(f, "# End Project\r\n");
    fclose(f);
    return 0;
  }
  
  /*
   *  Write a .dsp target file
   */
  int writeDynamicLibTarget(const char *filepath, Fl_Preferences &targetDB, const char *dir=0) {
    char name[84]; targetDB.get("name", name, "DBERROR", 80); strcat(name, "dll");
    if (!dir) dir = name;
    char filename[2048];
    fl_snprintf(filename, 2047, "%s/%s.dsp", filepath, name);
    FILE *f = fopen(filename, "wb");
    if (!f) {
      fl_alert("Can't open file:\n%s", filename);
      return -1;
    }
    
    fprintf(f, "# Microsoft Developer Studio Project File - Name=\"%s\" - Package Owner=<4>\r\n", name);
    fprintf(f, "# Microsoft Developer Studio Generated Build File, Format Version 6.00\r\n");
    fprintf(f, "# ** DO NOT EDIT **\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "# TARGTYPE \"Win32 (x86) Dynamic-Link Library\" 0x0102\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "CFG=%s - Win32 Debug\r\n", name);
    fprintf(f, "!MESSAGE This is not a valid makefile. To build this project using NMAKE,\r\n");
    fprintf(f, "!MESSAGE use the Export Makefile command and run\r\n");
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "!MESSAGE NMAKE /f \"%s.mak\".\r\n", name);
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "!MESSAGE You can specify a configuration when running NMAKE\r\n");
    fprintf(f, "!MESSAGE by defining the macro CFG on the command line. For example:\r\n");
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "!MESSAGE NMAKE /f \"%s.mak\" CFG=\"%s - Win32 Debug\"\r\n", name, name);
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "!MESSAGE Possible choices for configuration are:\r\n");
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "!MESSAGE \"%s - Win32 Release\" (based on \"Win32 (x86) Dynamic-Link Library\")\r\n", name);
    fprintf(f, "!MESSAGE \"%s - Win32 Debug\" (based on \"Win32 (x86) Dynamic-Link Library\")\r\n", name);
    fprintf(f, "!MESSAGE \r\n");
    fprintf(f, "\r\n");
    fprintf(f, "# Begin Project\r\n");
    fprintf(f, "# PROP AllowPerConfigDependencies 0\r\n");
    fprintf(f, "# PROP Scc_ProjName \"\"\r\n");
    fprintf(f, "# PROP Scc_LocalPath \"\"\r\n");
    fprintf(f, "CPP=cl.exe\r\n");
    fprintf(f, "MTL=midl.exe\r\n");
    fprintf(f, "RSC=rc.exe\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "!IF  \"$(CFG)\" == \"%s - Win32 Release\"\r\n", name);
    fprintf(f, "\r\n");
    fprintf(f, "# PROP BASE Use_MFC 0\r\n");
    fprintf(f, "# PROP BASE Use_Debug_Libraries 0\r\n");
    fprintf(f, "# PROP BASE Output_Dir \"Release/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP BASE Intermediate_Dir \"Release/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP BASE Target_Dir \"\"\r\n");
    fprintf(f, "# PROP Use_MFC 0\r\n");
    fprintf(f, "# PROP Use_Debug_Libraries 0\r\n");
    fprintf(f, "# PROP Output_Dir \"Release/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP Intermediate_Dir \"Release/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP Ignore_Export_Lib 0\r\n");
    fprintf(f, "# PROP Target_Dir \"\"\r\n");
    fprintf(f, "# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /YX /FD /c\r\n");
    fprintf(f, "# ADD CPP /nologo /MD /W3 /GX /Os /Ob2 /I \"../../zlib\" /I \"../../png\" /I \"../../jpeg\" /I \".\" /I \"../..\" /D \"FL_DLL\" /D \"FL_LIBRARY\" /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"WIN32_LEAN_AND_MEAN\" /D \"VC_EXTRA_LEAN\" /D \"WIN32_EXTRA_LEAN\" /YX /c\r\n");
    fprintf(f, "# ADD BASE MTL /nologo /D \"NDEBUG\" /mktyplib203 /o /win32 \"NUL\"\r\n");
    fprintf(f, "# ADD MTL /nologo /D \"NDEBUG\" /mktyplib203 /o /win32 \"NUL\"\r\n");
    fprintf(f, "# ADD BASE RSC /l 0x409 /d \"NDEBUG\"\r\n");
    fprintf(f, "# ADD RSC /l 0x409 /d \"NDEBUG\"\r\n");
    fprintf(f, "BSC32=bscmake.exe\r\n");
    fprintf(f, "# ADD BASE BSC32 /nologo\r\n");
    fprintf(f, "# ADD BSC32 /nologo\r\n");
    fprintf(f, "LINK32=link.exe\r\n");
    fprintf(f, "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386\r\n");
    fprintf(f, "# ADD LINK32 opengl32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:\"%s.pdb\" /machine:I386\r\n", name);
    fprintf(f, "# SUBTRACT LINK32 /pdb:none\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "!ELSEIF  \"$(CFG)\" == \"%s - Win32 Debug\"\r\n", name);
    fprintf(f, "\r\n");
    fprintf(f, "# PROP BASE Use_MFC 0\r\n");
    fprintf(f, "# PROP BASE Use_Debug_Libraries 1\r\n");
    fprintf(f, "# PROP BASE Output_Dir \"Debug/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP BASE Intermediate_Dir \"Debug/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP BASE Target_Dir \"\"\r\n");
    fprintf(f, "# PROP Use_MFC 0\r\n");
    fprintf(f, "# PROP Use_Debug_Libraries 1\r\n");
    fprintf(f, "# PROP Output_Dir \"Debug/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP Intermediate_Dir \"Debug/$(ProjectName)\"\r\n");
    fprintf(f, "# PROP Ignore_Export_Lib 0\r\n");
    fprintf(f, "# PROP Target_Dir \"\"\r\n");
    fprintf(f, "# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /YX /FD /c\r\n");
    fprintf(f, "# ADD CPP /nologo /MDd /GX /ZI /Od /I \".\" /I \"../..\" /I \"..\\..\\zlib\" /I \"..\\..\\png\" /I \"..\\..\\jpeg\" /D \"FL_DLL\" /D \"FL_LIBRARY\" /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"WIN32_LEAN_AND_MEAN\" /D \"VC_EXTRA_LEAN\" /D \"WIN32_EXTRA_LEAN\" /YX /c\r\n");
    fprintf(f, "# ADD BASE MTL /nologo /D \"_DEBUG\" /mktyplib203 /o /win32 \"NUL\"\r\n");
    fprintf(f, "# ADD MTL /nologo /D \"_DEBUG\" /mktyplib203 /o /win32 \"NUL\"\r\n");
    fprintf(f, "# ADD BASE RSC /l 0x409 /d \"_DEBUG\"\r\n");
    fprintf(f, "# ADD RSC /l 0x409 /d \"_DEBUG\"\r\n");
    fprintf(f, "BSC32=bscmake.exe\r\n");
    fprintf(f, "# ADD BASE BSC32 /nologo\r\n");
    fprintf(f, "# ADD BSC32 /nologo\r\n");
    fprintf(f, "LINK32=link.exe\r\n");
    fprintf(f, "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept\r\n");
    fprintf(f, "# ADD LINK32 opengl32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:\"%sd.pdb\" /debug /machine:I386 /out:\"../../test/%sd.dll\" /pdbtype:sept\r\n", name, name);
    fprintf(f, "# SUBTRACT LINK32 /pdb:none /incremental:no\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "!ENDIF \r\n");
    fprintf(f, "\r\n");
    fprintf(f, "# Begin Target\r\n");
    fprintf(f, "\r\n");
    fprintf(f, "# Name \"%s - Win32 Release\"\r\n", name);
    fprintf(f, "# Name \"%s - Win32 Debug\"\r\n", name);
    
    Fl_Preferences sourcesDB(targetDB, "sources");
    int i, n = sourcesDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences sourceDB(sourcesDB, i);
      GET_UUID(refUUID, sourceDB);
      Fl_File_Prefs fileDB(filesDB, refUUID);
      writeSourceFile(f, fileDB);
    }
    
    Fl_Preferences flsDB(targetDB, "fl");
    n = flsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences flDB(flsDB, i);
      GET_UUID(refUUID, flDB);
      Fl_File_Prefs fileDB(filesDB, refUUID);
      writeFluidFile(f, fileDB, name);
    }
    
    fprintf(f, "# End Target\r\n");
    fprintf(f, "# End Project\r\n");
    fclose(f);
    return 0;
  }
  
  /*
   * Write the .dsp target files
   */
  int writeTargetFiles(const char *filepath) {
    int i;
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      writeApplicationTarget(filepath, targetDB);
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeStaticLibTarget(filepath, targetDB, "src");
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeDynamicLibTarget(filepath, targetDB, "src");
    }
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      writeApplicationTarget(filepath, targetDB, "test");
    }
    return 0;
  }
  
  int writeConfigH(const char *filename) {
    FILE *f = fopen(filename, "wb");
    fputs("/*\n", f);
    fputs(" * \"$Id$\"\n", f);
    fputs(" */\n", f);
    fputs("#define FLTK_DATADIR    \"C:/FLTK\"\n", f);
    fputs("#define FLTK_DOCDIR     \"C:/FLTK/DOC\"\n", f);
    fputs("#define BORDER_WIDTH 2\n", f);
    fputs("#define HAVE_GL 1\n", f);
    fputs("#define HAVE_GL_GLU_H 1\n", f);
    fputs("#define USE_COLORMAP 1\n", f);
    fputs("#define HAVE_XDBE 0\n", f);
    fputs("#define USE_XDBE HAVE_XDBE\n", f);
    fputs("#define HAVE_OVERLAY 0\n", f);
    fputs("#define HAVE_GL_OVERLAY 1\n", f);
    fputs("#define WORDS_BIGENDIAN 0\n", f);
    fputs("#define U16 unsigned short\n", f);
    fputs("#define U32 unsigned\n", f);
    fputs("#undef U64\n", f);
    fputs("#define HAVE_VSNPRINTF 0\n", f);
    fputs("#define HAVE_SNPRINTF 0\n", f);
    fputs("#define HAVE_STRCASECMP 1\n", f);
    fputs("#define HAVE_LOCALE_H 1\n", f);
    fputs("#define HAVE_LOCALECONV 1\n", f);
    fputs("#define HAVE_POLL 0\n", f);
    fputs("#define HAVE_LIBPNG\n", f);
    fputs("#define HAVE_LIBZ\n", f);
    fputs("#define HAVE_LIBJPEG\n", f);
    fputs("#define HAVE_PNG_H\n", f);
    fputs("#undef HAVE_LIBPNG_PNG_H\n", f);
    fputs("#define HAVE_PNG_GET_VALID\n", f);
    fputs("#define HAVE_PNG_SET_TRNS_TO_ALPHA\n", f);
    fputs("/*\n", f);
    fputs(" * End of \"$Id$\".\n", f);
    fputs(" */\n", f);
    fclose(f);
    return 0;
  }
  
  /*
   * Write the entire system of files.
   */
  int write() {
    char filepath[2048];
    // --- create directory structure ide/VisualC6
    sprintf(filepath, "%s/ide", rootDir); fl_mkdir(filepath, 0777);
    sprintf(filepath, "%s/ide/VisualC6", rootDir); fl_mkdir(filepath, 0777);
    // --- create project database (.dsw)
    writeProjectFile(filepath);
    // --- create all targets (.dsp)
    writeTargetFiles(filepath);
    // --- create a valid config.h
    sprintf(filepath, "%s/ide/VisualC6/config.h", rootDir);
    writeConfigH(filepath);
    // --- close and finish
    return 0;
  }
};


// This creates all files needed to compile FLTK using VisualC++ 6.0 and up.
// create directory structure ide/VisualC6
// create .dsw file
// create .dsp files
// create config.h
// create icons
void generate_fltk_VisualC6_support(const char *filename, const char *targetpath)
{
  Fl_Preferences *db = 
    new Fl_Preferences(filename, "fltk.org", 0);
  VisualC6_IDE ide(*db, targetpath);
  ide.write();
  delete db;
  return;
}


extern int exit_early;

class Fl_IDE_VisualC_Plugin : public Fl_Commandline_Plugin
{
public:
  Fl_IDE_VisualC_Plugin() : Fl_Commandline_Plugin(name()) { }
  const char *name() { return "ideVisualC.fluid.fltk.org"; }
  const char *help() { return
    " --dbvisualc6 <dbname> <targetpath> : create IDE files for MS IDE's able to read VC6 projects"; } 
  int arg(int argc, char **argv, int &i) {
    if (argc>=i+1 && strcmp(argv[i], "--dbvisualc6")==0) {
      if (argc>=i+3 && argv[i+1][0]!='-' && argv[i+2][0]!='-') {
        fprintf(stderr, "Creating VisualC++ 6.0 IDE from %s in %s\n", argv[i+1], argv[i+2]);
        exit_early = 1;
        generate_fltk_VisualC6_support(argv[i+1], argv[i+2]);
        i = i+3;
        return 3;
      } else {
        fprintf(stderr, "Missing argument: --dbvisualc6 <dbname> <targetpath>\n");
        return 1;
      }
    }
    return 0;
  }
  int test(const char *a1, const char *a2, const char *a3) {
    generate_fltk_VisualC6_support(a1, a2);
    return 0;
  }
};
Fl_IDE_VisualC_Plugin IDE_VisualC_Plugin;

//
// End of "$Id$".
//

