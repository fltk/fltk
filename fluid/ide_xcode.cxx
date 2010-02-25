//
// "$Id$"
//
// IDE and Build FIle generation for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
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

#define XCODE_DEFAULT 1

/*
 
    XCODE 3.0 IDE FILES
 
 The Xcode 3.0 IDE file format is using a quite comlex tree of multiply linked
 entries to leave as much possibilities to developers as somehow possible. To 
 write this format, we will need to generate a bunch of new unique IDs that will
 be stored in the DB.
 
  "rootObject" is a PBXProject [/ide/Xcode/xcRootObjectID]
    link to "buildConfigurationList" [/ide/Xcode/xcBuildConfigurationListID]
    link to "mainGroup" PBXGroup [/ide/Xcode/xcMainGroupID]
    link to "productRefGroup" PBXGroup [/ide/Xcode/xcProductsGroupID]
    array of links to "target" PBXNativeTarget [/targets/apps|libs|tests/#/xcTargetID]
 
  "buildConfigurationList" is a XCConfigurationList
    array of links to buildConfiguration
      "Debug" [/ide/Xcode/xcBuildConfigurationDebugID]
      "Release" [/ide/Xcode/xcBuildConfigurationReleaseID]
      ... but also [/targets/apps|libs|tests/#/xcBuildConfigurationListID]
                   [/targets/apps|libs|tests/#/xcBuildConfigurationDebugID]
                   [/targets/apps|libs|tests/#/xcBuildConfigurationReleaseID]
 
  "buildConfiguration" is a XCBuildConfiguration
    no links
 
  "mainGroup" is a PBXGroup 
    array of links to PBXFileReference and PBXGroup [/ide/Xcode/xc...GroupID]
      and also [/targets/apps|libs/#/xcGroupID]
 
  "target" is a "PBXNaticeTarget" [/targets/apps|libs|tests/#/xcTargetID]
    link to buildConfigurationList [/targets/apps|libs|tests/#/xcBuildConfigurationListID]
    array of links to buildPhases
      Headers     [/targets/apps|libs|tests/#/xcBuildHeadersID] (libs only)
      Resources   [/targets/apps|libs|tests/#/xcBuildResourcesID]
      Sources     [/targets/apps|libs|tests/#/xcBuildSourcesID]
      Frameworks  [/targets/apps|libs|tests/#/xcBuildFrameworksID]
      CopyFiles   [/targets/apps|libs|tests/#/xcBuildCopyFilesID] (not for libs)
    array of links to buildRules [/targets/apps|libs|tests/#/xcBuildRuleFlID] etc.
    array of links to dependencies [/targets/apps|libs|tests/#/dependencies/#/xcDependencyID]
    link to productReference [/targets/apps|libs|tests/#/xcProductID]
 
  "buildPhase" is a PBX...BuildPhase [/targets/apps|libs|tests/#/xcBuild...ID]
    array of links to buildFile [/targets/apps|libs|tests/#/sources|libs|fl/#/xcBuildFileID]
 
  "buildFile" is a PBXBuildFile [/targets/apps|libs|tests/#/sources|libs|fl/#/xcBuildFileID]
    links to file (PBXFileReference) [/files/#/xcFileID]
 
  "buildRule" is a PBXBuildRule
    [/targets/apps|libs|tests/#/xcBuildRuleFlID] etc.
    no links
 
  "dependency" is a PBXTargetDependency [/targets/apps|libs|tests/#/dependencies/#/xcDependencyID]
    link to target "PBXNativeTarget" (see above)
    link to targetProxy "PBXContainerItemProxy" /targets/apps|libs|tests/#/dependencies/#/xcProxyID]
 
  "file" "productReference" is a PBXFileReference
    no links
 
  "targetProxy" is a PBXContainerItemProxy
    links to containerPortal (=rootObject) [/ide/Xcode/xcRootObjectID]
    links to remoteGlobalIDString "PBXNativeTarget" (see above) [/targets/apps|libs|tests/#/xcTargetID]
 
*/

#include "ide_support.h"

#include <FL/filename.H>
#include <FL/fl_ask.H>
#include "../src/flstring.h"

#include "Fl_Type.h"

/*
 * This class creates all Xcode 3.0 IDE files.
 */
class Xcode3_IDE {
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
  XCID xcRootNodeID;
  XCID xcBuildConfigurationListID;
  XCID xcMainGroupID;
  XCID xcProductsGroupID;
  XCID xcAppsGroupID;
  XCID xcLibsGroupID;
  XCID xcTestsGroupID;
  XCID xcBuildConfigurationDebugID;
  XCID xcBuildConfigurationReleaseID;
public:
  Xcode3_IDE(Fl_Preferences &db, const char *rootDirA)
  : rootDir(strdup(rootDirA)),
    tgtAppsDB(db, "targets/apps"),
    tgtLibsDB(db, "targets/libs"),
    tgtTestsDB(db, "targets/tests"),
    filesDB(db, "files"),
    ideDB(db, "ide/Xcode")
  {
    db.get("projectName", projectName, "Unnamed", 80);
    nTgtApps = tgtAppsDB.groups();
    nTgtLibs = tgtLibsDB.groups();
    nTgtTests = tgtTestsDB.groups();
    nFiles = filesDB.groups();
    getXCID(ideDB, "xcRootNodeID", xcRootNodeID);
    getXCID(ideDB, "xcBuildConfigurationListID", xcBuildConfigurationListID);
    getXCID(ideDB, "xcMainGroupID", xcMainGroupID);
    getXCID(ideDB, "xcProductsGroupID", xcProductsGroupID);
    getXCID(ideDB, "xcAppsGroupID", xcAppsGroupID);
    getXCID(ideDB, "xcLibsGroupID", xcLibsGroupID);
    getXCID(ideDB, "xcTestsGroupID", xcTestsGroupID);    
    getXCID(ideDB, "xcBuildConfigurationDebugID", xcBuildConfigurationDebugID);
    getXCID(ideDB, "xcBuildConfigurationReleaseID", xcBuildConfigurationReleaseID);
  }
  ~Xcode3_IDE() {
    if (rootDir) free(rootDir);
  }
  
  /*
   * Write all files required during the actual build.
   * These are actually forwarding links from the build setup into the 
   * files section.
   */
  int writeBuildFiles(FILE *out, Fl_Preferences &targetDB) {
    // FIXME: also write .app, .plist, and maybe headers
    // --- write all references to sources from the given target
    Fl_Preferences sourcesDB(targetDB, "sources");
    int i, n = sourcesDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences sourceDB(sourcesDB, i);
      GET_UUID(refUUID, sourceDB);
      MAKE_XCID(xcBuildFileID, sourceDB);
      Fl_File_Prefs fileDB(filesDB, refUUID);
      MAKE_XCID(xcFileID, fileDB);
      const char *fullName = fileDB.fullName();
      fprintf(out, "\t\t%s /* %s in Sources */ = {isa = PBXBuildFile; fileRef = %s /* %s */; };\n", xcBuildFileID, fullName, xcFileID, fullName);
    }
    // --- write all references to Fluid UI files from the given target
    Fl_Preferences flsDB(targetDB, "fl");
    n = flsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences flDB(flsDB, i);
      GET_UUID(refUUID, flDB);
      MAKE_XCID(xcBuildFileID, flDB);
      Fl_File_Prefs fileDB(filesDB, refUUID);
      MAKE_XCID(xcFileID, fileDB);
      const char *fullName = fileDB.fullName();
      fprintf(out, "\t\t%s /* %s in Sources */ = {isa = PBXBuildFile; fileRef = %s /* %s */; };\n", xcBuildFileID, fullName, xcFileID, fullName);
    }
    Fl_Preferences libsDB(targetDB, "libs"); n = libsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences libDB(libsDB, i);
      GET_UUID(refUUID, libDB);
      MAKE_XCID(xcBuildFrameworkID, libDB);
      MAKE_XCID(xcCopyFrameworkID, libDB);
      Fl_Preferences tgtLibDB(tgtLibsDB, refUUID);
      MAKE_XCID(xcProductID, tgtLibDB);
      char name[80]; tgtLibDB.get("name", name, "DBERROR", 80);;
      fprintf(out, "\t\t%s /* %s.framework in Frameworks */ = {isa = PBXBuildFile; fileRef = %s /* %s.framework */; };\n", xcBuildFrameworkID, name, xcProductID, name);
      fprintf(out, "\t\t%s /* %s.framework in CopyFiles */ = {isa = PBXBuildFile; fileRef = %s /* %s.framework */; };\n", xcCopyFrameworkID, name, xcProductID, name);
    }
    Fl_Preferences extsDB(targetDB, "externals"); n = extsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences extDB(extsDB, i);
      if (with_xcode(extDB.id())) {
        GET_UUID(refUUID, extDB);
        Fl_File_Prefs fileDB(filesDB, refUUID);
        MAKE_XCID(xcFileID, fileDB);
        const char *fullName = fileDB.fullName();
        if (strcmp(fileDB.fileExt(), ".icns")==0) {
          MAKE_XCID(xcCopyResourceID, extDB);
          fprintf(out, "\t\t%s /* %s in Resources */ = {isa = PBXBuildFile; fileRef = %s /* %s */; };\n", xcCopyResourceID, fullName, xcFileID, fullName);
        } else {
          MAKE_XCID(xcBuildFrameworkID, extDB);
          fprintf(out, "\t\t%s /* %s in Frameworks */ = {isa = PBXBuildFile; fileRef = %s /* %s */; };\n", xcBuildFrameworkID, fullName, xcFileID, fullName);
        }
      }
    }
    return 0;
  }
  
  /*
   * Writes the section that links build components to a file.
   */
  int writeBuildFileSection(FILE *out) {
    fprintf(out, "/* Begin PBXBuildFile section */\n");
    int i;
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      writeBuildFiles(out, targetDB);
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeBuildFiles(out, targetDB);
    }
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      writeBuildFiles(out, targetDB);
    }
    fprintf(out, "/* End PBXBuildFile section */\n\n");
    return 0;
  }

  /*
   * Write the build rule for .fl files.
   */
  int writeBuildRule(FILE *out, Fl_Preferences &targetDB) {
    MAKE_XCID(xcBuildRuleFlID, targetDB);    
    fprintf(out, "\t\t%s /* PBXBuildRule */ = {\n", xcBuildRuleFlID);
    fprintf(out, "\t\t\tisa = PBXBuildRule;\n");
    fprintf(out, "\t\t\tcompilerSpec = com.apple.compilers.proxy.script;\n");
    fprintf(out, "\t\t\tfilePatterns = \"*.fl\";\n");
    fprintf(out, "\t\t\tfileType = pattern.proxy;\n");
    fprintf(out, "\t\t\tisEditable = 1;\n");
    fprintf(out, "\t\t\toutputFiles = (\n");
    fprintf(out, "\t\t\t\t\"${INPUT_FILE_DIR}/${INPUT_FILE_BASE}.cxx\",\n");
    fprintf(out, "\t\t\t\t\"${INPUT_FILE_DIR}/${INPUT_FILE_BASE}.h\",\n");
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\tscript = \"export DYLD_FRAMEWORK_PATH=${TARGET_BUILD_DIR} && cd ${INPUT_FILE_DIR} && ${TARGET_BUILD_DIR}/Fluid.app/Contents/MacOS/Fluid -c ${INPUT_FILE_NAME}\";\n");
    fprintf(out, "\t\t};\n");
    return 0;
  }

  /*
   * Additional build rules. Here we teach Xcode how to handle .fl files.
   */
  int writeBuildRuleSection(FILE *out) {
    int i;
    fprintf(out, "/* Begin PBXBuildRule section */\n");
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      writeBuildRule(out, targetDB);
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeBuildRule(out, targetDB);
    }
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      writeBuildRule(out, targetDB);
    }
    fprintf(out, "/* End PBXBuildRule section */\n\n");
    return 0;
  }
  
  /*
   * Write all target proxies for a single target.
   */
  int writeContainerItemProxy(FILE *out, Fl_Preferences &targetDB) {
    Fl_Preferences depsDB(targetDB, "deps");
    int i, n = depsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences depDB(depsDB, i);
      GET_UUID(refUUID, depDB);
      //      MAKE_XCID(xcDependencyID, depDB);
      MAKE_XCID(xcProxyID, depDB);
      Fl_Preferences *depTgtDBp = 0;
      if (tgtAppsDB.groupExists(refUUID))
        depTgtDBp = new Fl_Preferences(tgtAppsDB, refUUID);
      else if (tgtLibsDB.groupExists(refUUID))
        depTgtDBp = new Fl_Preferences(tgtLibsDB, refUUID);
      else if (tgtTestsDB.groupExists(refUUID))
        depTgtDBp = new Fl_Preferences(tgtTestsDB, refUUID);
      if (depTgtDBp) {
        Fl_Preferences &depTgtDB = *depTgtDBp;
        MAKE_XCID(xcTargetID, depTgtDB);
        char name[80]; depTgtDB.get("name", name, "DBERROR", 80);        
        fprintf(out, "\t\t%s /* PBXContainerItemProxy */ = {\n", xcProxyID);
        fprintf(out, "\t\t\tisa = PBXContainerItemProxy;\n");
        fprintf(out, "\t\t\tcontainerPortal = %s /* Project object */;\n", xcRootNodeID);
        fprintf(out, "\t\t\tproxyType = 1;\n");
        fprintf(out, "\t\t\tremoteGlobalIDString = %s;\n", xcTargetID);
        fprintf(out, "\t\t\tremoteInfo = %s;\n", name);
        fprintf(out, "\t\t};\n");
        delete depTgtDBp;
      }
    }
    return 0;
  }
  
  /*
   * Write a proxy for all target dependencies of all targets.
   * (I am not entirely sure why these proxies exist, but Apple will know)
   */
  int writeContainerItemProxySection(FILE *out) {
    fprintf(out, "/* Begin PBXContainerItemProxy section */\n");
    int i;
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      writeContainerItemProxy(out, targetDB);
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeContainerItemProxy(out, targetDB);
    }
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      writeContainerItemProxy(out, targetDB);
    }
    fprintf(out, "/* End PBXContainerItemProxy section */\n\n");
    return 0;
  }
  
  /*
   * List the files that will be copied into the final application.
   */
  int writeCopyFilesBuildPhase(FILE *out, Fl_Preferences &targetDB) {
    MAKE_XCID(xcBuildCopyFilesID, targetDB);
    fprintf(out, "\t\t%s /* CopyFiles */ = {\n", xcBuildCopyFilesID);
    fprintf(out, "\t\t\tisa = PBXCopyFilesBuildPhase;\n");
    fprintf(out, "\t\t\tbuildActionMask = 2147483647;\n");
    fprintf(out, "\t\t\tdstPath = \"\";\n");
    fprintf(out, "\t\t\tdstSubfolderSpec = 10;\n");
    fprintf(out, "\t\t\tfiles = (\n");
    // ---
    Fl_Preferences libsDB(targetDB, "libs");
    int i, n = libsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences libDB(libsDB, i);
      GET_UUID(refUUID, libDB);
      MAKE_XCID(xcCopyFrameworkID, libDB);
      Fl_Preferences tgtLibDB(tgtLibsDB, refUUID);
      char name[80]; tgtLibDB.get("name", name, "DBERROR", 80);;
      fprintf(out, "\t\t\t\t%s /* %s.framework in CopyFiles */,\n", xcCopyFrameworkID, name);
    }
    // ---
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\trunOnlyForDeploymentPostprocessing = 0;\n");
    fprintf(out, "\t\t};\n");
    return 0;
  };
  
  /*
   * List the files that will be copied into the final application.
   */
  int writeCopyFilesBuildPhaseSection(FILE *out) {
    fprintf(out, "/* Begin PBXCopyFilesBuildPhase section */\n");
    int i;
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      writeCopyFilesBuildPhase(out, targetDB);
    }
    // TgtLibsDB has no CopyFile build phase
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      writeCopyFilesBuildPhase(out, targetDB);
    }
    fprintf(out, "/* End PBXCopyFilesBuildPhase section */\n\n");
    return 0;
  }
  
  /*
   * A list of all files that are somehow referenced in this project
   */
  int writeFileReferenceSection(FILE *out) {
    int i;
    fprintf(out, "/* Begin PBXFileReference section */\n");  
    // --- list of all Application target results
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      // write a reference to the target.app file
      char name[80]; targetDB.get("name", name, "DBERROR", 80);
      MAKE_XCID(xcProductID, targetDB);
      fprintf(out, "\t\t%s /* %s.app */ = {isa = PBXFileReference; "
              "explicitFileType = wrapper.application; includeInIndex = 0; "
              "path = %s.app; sourceTree = BUILT_PRODUCTS_DIR; };\n", 
              xcProductID, name, name);
      // FIXME: write .plist reference
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      // write a reference to the target.app file
      char name[80]; targetDB.get("name", name, "DBERROR", 80);
      MAKE_XCID(xcProductID, targetDB);
      fprintf(out, "\t\t%s /* %s.framework */ = {isa = PBXFileReference; "
              "explicitFileType = wrapper.framework; includeInIndex = 0; "
              "path = %s.framework; sourceTree = BUILT_PRODUCTS_DIR; };\n", 
              xcProductID, name, name);
      // FIXME: write .plist reference
    }
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      // write a reference to the target.app file
      char name[80]; targetDB.get("name", name, "DBERROR", 80);
      MAKE_XCID(xcProductID, targetDB);
      fprintf(out, "\t\t%s /* %s.app */ = {isa = PBXFileReference; "
              "explicitFileType = wrapper.application; includeInIndex = 0; "
              "path = %s.app; sourceTree = BUILT_PRODUCTS_DIR; };\n", 
              xcProductID, name, name);
      // FIXME: write .plist reference
    }
    // write all source file
    for (i=0; i<nFiles; i++) {
      Fl_File_Prefs fileDB(filesDB, i);
      MAKE_XCID(xcFileID, fileDB);
      const char *fullName = fileDB.fullName();
      char pathAndName[1024]; fileDB.get("pathAndName", pathAndName, "DBERROR", 1024);
      const char *filetype = "test";
      const char *ext = fileDB.fileExt();
      if (!ext) {
      } else if (strcmp(pathAndName, "src/Fl.cxx")==0
               ||strcmp(pathAndName, "src/Fl_Native_File_Chooser.cxx")==0) { // FIXME: bad hack!
        filetype = "sourcecode.cpp.objcpp";
      } else if (strcmp(ext, ".cxx")==0) {
        filetype = "sourcecode.cpp.cpp";
      } else if (strcmp(ext, ".c")==0) {
        filetype = "sourcecode.c.c";
      } else if (strcmp(ext, ".mm")==0) {
        filetype = "sourcecode.cpp.objcpp";
      } else if (strcmp(ext, ".dylib")==0) {
        fprintf(out,
                "\t\t%s /* %s */ = {isa = PBXFileReference; "
                "lastKnownFileType = \"compiled.mach-o.dylib\"; "
                "name = %s; path = %s; "
                "sourceTree = \"<absolute>\"; };\n", 
                xcFileID, fullName, fullName, pathAndName );
        filetype = 0L;
      } else if (strcmp(ext, ".framework")==0) {
        fprintf(out,
                "\t\t%s /* %s */ = {isa = PBXFileReference; "
                "lastKnownFileType = \"wrapper.framework\"; "
                "name = %s; path = %s; "
                "sourceTree = \"<absolute>\"; };\n", 
                xcFileID, fullName, fullName, pathAndName );
        filetype = 0L;
      } else if (strcmp(ext, ".icns")==0) {
        fprintf(out,
                "\t\t%s /* %s */ = {isa = PBXFileReference; "
                "lastKnownFileType = \"image.icns\"; "
                "name = %s; path = %s; "
                "sourceTree = \"<group>\"; };\n", 
                xcFileID, fullName, fullName, pathAndName );
        filetype = 0L;
      } else if (strcmp(ext, ".plist")==0) {
        filetype = "text.plist.xml";
      }
      if (filetype)
        fprintf(out,
              "\t\t%s /* %s */ = {isa = PBXFileReference; fileEncoding = 4; "
              "lastKnownFileType = %s; name = %s; "
              "path = ../../%s; sourceTree = SOURCE_ROOT; };\n", 
              xcFileID, fullName, filetype, fullName, pathAndName);
    }
    fprintf(out, "/* End PBXFileReference section */\n\n");
    return 0;
  }
  
  /*
   * List all framework build phases
   */
  int writeFrameworksBuildPhase(FILE *out, Fl_Preferences &targetDB) {
    MAKE_XCID(xcBuildFrameworksID, targetDB);
    fprintf(out, "\t\t%s /* Frameworks */ = {\n", xcBuildFrameworksID);
    fprintf(out, "\t\t\tisa = PBXFrameworksBuildPhase;\n");
    fprintf(out, "\t\t\tbuildActionMask = 2147483647;\n");
    fprintf(out, "\t\t\tfiles = (\n");
    Fl_Preferences libsDB(targetDB, "libs");
    int i, n = libsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences libDB(libsDB, i);
      GET_UUID(refUUID, libDB);
      MAKE_XCID(xcBuildFrameworkID, libDB);
      Fl_Preferences tgtLibDB(tgtLibsDB, refUUID);
      char name[80]; tgtLibDB.get("name", name, "DBERROR", 80);
      fprintf(out, "\t\t\t\t%s /* %s.framework in Frameworks */,\n", xcBuildFrameworkID, name);
    }
    Fl_Preferences extsDB(targetDB, "externals");
    n = extsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences extDB(extsDB, i);
      if (with_xcode(extDB.id())) {
        GET_UUID(refUUID, extDB);
        MAKE_XCID(xcBuildFrameworkID, extDB);
        Fl_File_Prefs fileDB(filesDB, refUUID);
        const char *fullName = fileDB.fullName();
        if (strcmp(fileDB.fileExt(), ".icns")!=0) {
          fprintf(out, "\t\t\t\t%s /* %s in Frameworks */,\n", xcBuildFrameworkID, fullName);
        }
      }
    }
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\trunOnlyForDeploymentPostprocessing = 0;\n");
    fprintf(out, "\t\t};\n");
    return 0;
  };
  
  /*
   * Write all build phases.
   */
  int writeFrameworksBuildPhaseSection(FILE *out) {
    fprintf(out, "/* Begin PBXFrameworksBuildPhase section */\n");
    int i;
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      writeFrameworksBuildPhase(out, targetDB);
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeFrameworksBuildPhase(out, targetDB);
    }
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      writeFrameworksBuildPhase(out, targetDB);
    }
    fprintf(out, "/* End PBXFrameworksBuildPhase section */\n\n");
    return 0;
  }
  
  /*
   *
   */
  int writeTargetFiles(FILE *out, Fl_Preferences &targetDB) {
    char name[80];
    MAKE_XCID(xcTargetGroupID, targetDB);    
    targetDB.get("name", name, "DBERROR", 80);
    fprintf(out, "\t\t%s /* %s */ = {\n", xcTargetGroupID, name);
    fprintf(out, "\t\t\tisa = PBXGroup;\n");
    fprintf(out, "\t\t\tchildren = (\n");
    
    MAKE_XCID(xcProductID, targetDB);
    Fl_Preferences sourcesDB(targetDB, "sources");
    int j, n = sourcesDB.groups();
    for (j=0; j<n; j++) {
      Fl_Preferences sourceDB(sourcesDB, j);
      GET_UUID(refUUID, sourceDB);
      Fl_File_Prefs fileDB(filesDB, refUUID);
      MAKE_XCID(xcFileID, fileDB);
      const char *fullName = fileDB.fullName();
      fprintf(out, "\t\t\t\t%s /* %s */,\n", xcFileID, fullName);
    }
    Fl_Preferences flsDB(targetDB, "fl");
    n = flsDB.groups();
    for (j=0; j<n; j++) {
      Fl_Preferences flDB(flsDB, j);
      GET_UUID(refUUID, flDB);
      Fl_File_Prefs fileDB(filesDB, refUUID);
      MAKE_XCID(xcFileID, fileDB);
      const char *fullName = fileDB.fullName();
      fprintf(out, "\t\t\t\t%s /* %s */,\n", xcFileID, fullName);
    }
    Fl_Preferences extsDB(targetDB, "externals");
    n = extsDB.groups();
    for (j=0; j<n; j++) {
      Fl_Preferences extDB(extsDB, j);
      if (with_xcode(extDB.id())) {
        GET_UUID(refUUID, extDB);
        Fl_File_Prefs fileDB(filesDB, refUUID);
        MAKE_XCID(xcFileID, fileDB);
        const char *fullName = fileDB.fullName();
        fprintf(out, "\t\t\t\t%s /* %s */,\n", xcFileID, fullName);
      }
    }
    
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\tname = %s;\n", name);
    fprintf(out, "\t\t\tsourceTree = \"<group>\";\n");
    fprintf(out, "\t\t};\n");
    return 0;
  }
  
  /*
   * Groups define the folder hierarchy in the "Groups & Files" panel
   */
  int writeGroupSection(FILE *out) {
    int i;
    char name[80];
    fprintf(out, "/* Begin PBXGroup section */\n");
    // --- FIXME: missing "icons" group
    // --- main group
    fprintf(out, "\t\t%s = {\n", xcMainGroupID);
    fprintf(out, "\t\t\tisa = PBXGroup;\n");
    fprintf(out, "\t\t\tchildren = (\n");
    fprintf(out, "\t\t\t\t%s /* Applications */,\n", xcAppsGroupID);
    fprintf(out, "\t\t\t\t%s /* Frameworks */,\n", xcLibsGroupID);
    fprintf(out, "\t\t\t\t%s /* Tests */,\n", xcTestsGroupID);
    fprintf(out, "\t\t\t\t%s /* Products */,\n", xcProductsGroupID); // link to "Products" group
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\tsourceTree = \"<group>\";\n");
    fprintf(out, "\t\t};\n");
    // --- "Products" group
    fprintf(out, "\t\t%s /* Products */ = {\n", xcProductsGroupID);
    fprintf(out, "\t\t\tisa = PBXGroup;\n");
    fprintf(out, "\t\t\tchildren = (\n");
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      char name[80]; targetDB.get("name", name, "DBERROR", 80);
      MAKE_XCID(xcProductID, targetDB);
      fprintf(out, "\t\t\t\t%s /* %s.app */,\n", xcProductID, name);
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      char name[80]; targetDB.get("name", name, "DBERROR", 80);
      MAKE_XCID(xcProductID, targetDB);
      fprintf(out, "\t\t\t\t%s /* %s.framework */,\n", xcProductID, name);
    }
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      char name[80]; targetDB.get("name", name, "DBERROR", 80);
      MAKE_XCID(xcProductID, targetDB);
      fprintf(out, "\t\t\t\t%s /* %s.app */,\n", xcProductID, name);
    }
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\tname = Products;\n");
    fprintf(out, "\t\t\tsourceTree = \"<group>\";\n");
    fprintf(out, "\t\t};\n");
    // --- FIXME: missing "plists" group

    // --- "Applications" group
    fprintf(out, "\t\t%s /* Applications */ = {\n", xcAppsGroupID);
    fprintf(out, "\t\t\tisa = PBXGroup;\n");
    fprintf(out, "\t\t\tchildren = (\n");
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      MAKE_XCID(xcTargetGroupID, targetDB);
      targetDB.get("name", name, "DBERROR", 80);
      fprintf(out, "\t\t\t\t%s /* %s */,\n", xcTargetGroupID, name);
    }
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\tname = Applications;\n");
    fprintf(out, "\t\t\tsourceTree = \"<group>\";\n");
    fprintf(out, "\t\t};\n");
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      writeTargetFiles(out, targetDB);
    }
    // --- "Frameworks" group
    fprintf(out, "\t\t%s /* Frameworks */ = {\n", xcLibsGroupID);
    fprintf(out, "\t\t\tisa = PBXGroup;\n");
    fprintf(out, "\t\t\tchildren = (\n");
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      MAKE_XCID(xcTargetGroupID, targetDB);
      targetDB.get("name", name, "DBERROR", 80);
      fprintf(out, "\t\t\t\t%s /* %s */,\n", xcTargetGroupID, name);
    }
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\tname = Frameworks;\n");
    fprintf(out, "\t\t\tsourceTree = \"<group>\";\n");
    fprintf(out, "\t\t};\n");
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeTargetFiles(out, targetDB);
    }
    // --- "Tests" group
    fprintf(out, "\t\t%s /* Tests */ = {\n", xcTestsGroupID);
    fprintf(out, "\t\t\tisa = PBXGroup;\n");
    fprintf(out, "\t\t\tchildren = (\n");
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      MAKE_XCID(xcTargetGroupID, targetDB);
      targetDB.get("name", name, "DBERROR", 80);
      fprintf(out, "\t\t\t\t%s /* %s */,\n", xcTargetGroupID, name);
    }
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\tname = Tests;\n");
    fprintf(out, "\t\t\tsourceTree = \"<group>\";\n");
    fprintf(out, "\t\t};\n");
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      writeTargetFiles(out, targetDB);
    }
    // --- done
    
    fprintf(out, "/* End PBXGroup section */\n\n");
    return 0;
  }
  
  /*
   *
   */
  int writeHeadersBuildPhase(FILE *out, Fl_Preferences &targetDB) {
    MAKE_XCID(xcBuildHeadersID, targetDB);
    fprintf(out, "\t\t%s /* Headers */ = {\n", xcBuildHeadersID);
    fprintf(out, "\t\t\tisa = PBXHeadersBuildPhase;\n");
    fprintf(out, "\t\t\tbuildActionMask = 2147483647;\n");
    fprintf(out, "\t\t\tfiles = (\n");
#if 0
    // FIXME: list all required headers
    Fl_Preferences libsDB(targetDB, "libs");
    int i, n = libsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences libDB(libsDB, i);
      GET_UUID(refUUID, libDB);
      MAKE_XCID(xcCopyFrameworkID, libDB);
      Fl_Preferences tgtLibDB(tgtLibsDB, refUUID);
      char name[80]; tgtLibDB.get("name", name, "DBERROR", 80);;
      fprintf(out, "\t\t\t\t%s /* %s.framework in CopyFiles */,\n", xcCopyFrameworkID, name);
    }
#endif
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\trunOnlyForDeploymentPostprocessing = 0;\n");
    fprintf(out, "\t\t};\n");
    return 0;
  };
  
  /*
   *
   */
  int writeHeadersBuildPhaseSection(FILE *out) {
    int i;
    fprintf(out, "/* Begin PBXHeadersBuildPhase section */\n");
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeHeadersBuildPhase(out, targetDB);
    }
    fprintf(out, "/* End PBXHeadersBuildPhase section */\n\n");
    return 0;
  }
  
  /*
   * Write build information for this target
   */
  int writeNativeTarget(FILE *out, Fl_Preferences &targetDB, int fmwk=0) {
    char name[80]; targetDB.get("name", name, "DBERROR", 80);
    MAKE_XCID(xcTargetID, targetDB);
    MAKE_XCID(xcBuildConfigurationListID, targetDB);
    char xcBuildHeadersID[25], xcBuildCopyFilesID[25];
    if (fmwk) getXCID(targetDB, "xcBuildHeadersID", xcBuildHeadersID);
    MAKE_XCID(xcBuildResourcesID, targetDB);
    MAKE_XCID(xcBuildSourcesID, targetDB);
    MAKE_XCID(xcBuildFrameworksID, targetDB);
    if (!fmwk) getXCID(targetDB, "xcBuildCopyFilesID", xcBuildCopyFilesID);
    MAKE_XCID(xcProductID, targetDB);
    MAKE_XCID(xcBuildRuleFlID, targetDB);
    fprintf(out, "\t\t%s /* %s */ = {\n", xcTargetID, name);
    fprintf(out, "\t\t\tisa = PBXNativeTarget;\n");
    fprintf(out, "\t\t\tbuildConfigurationList = %s /* Build configuration list for PBXNativeTarget \"%s\" */;\n", xcBuildConfigurationListID, name);
    fprintf(out, "\t\t\tbuildPhases = (\n");
    if (fmwk) fprintf(out, "\t\t\t\t%s /* Headers */,\n", xcBuildHeadersID);
    fprintf(out, "\t\t\t\t%s /* Resources */,\n", xcBuildResourcesID);
    fprintf(out, "\t\t\t\t%s /* Sources */,\n", xcBuildSourcesID);    
    fprintf(out, "\t\t\t\t%s /* Frameworks */,\n", xcBuildFrameworksID);
    if (!fmwk) fprintf(out, "\t\t\t\t%s /* CopyFiles */,\n", xcBuildCopyFilesID);  
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\tbuildRules = (\n");
    fprintf(out, "\t\t\t\t%s /* PBXBuildRule */,\n", xcBuildRuleFlID);
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\tdependencies = (\n"); {
      Fl_Preferences depsDB(targetDB, "deps");
      int i, n = depsDB.groups();
      for (i=0; i<n; i++) {
        Fl_Preferences depDB(depsDB, i);
        MAKE_XCID(xcDependencyID, depDB);
        fprintf(out, "\t\t\t\t%s /* PBXTargetDependency */,\n", xcDependencyID);
      }
    }
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\tname = %s;\n", name);
    fprintf(out, "\t\t\tproductName = %s;\n", name);
    if (fmwk) {
      fprintf(out, "\t\t\tproductReference = %s /* %s.framework */;\n", xcProductID, name);
      fprintf(out, "\t\t\tproductType = \"com.apple.product-type.framework\";\n");
    } else {
      fprintf(out, "\t\t\tproductReference = %s /* %s.app */;\n", xcProductID, name);
      fprintf(out, "\t\t\tproductType = \"com.apple.product-type.application\";\n");
    }
    fprintf(out, "\t\t};\n");
    return 0;
  }
  
  /*
   * Write the build information for all targets
   */
  int writeNativeTargetSection(FILE *out) {
    fprintf(out, "/* Begin PBXNativeTarget section */\n");
    int i;
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      writeNativeTarget(out, targetDB, 0);
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeNativeTarget(out, targetDB, 1);
    }
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      writeNativeTarget(out, targetDB, 0);
    }
    fprintf(out, "/* End PBXNativeTarget section */\n\n");
    return 0;
  }
  
  /*
   * This section describes the file layout in "Grous & Files"
   */
  int writeProjectSection(FILE *out) {
    int i;
    fprintf(out, "/* Begin PBXProject section */\n");
    fprintf(out, "\t\t%s /* Project object */ = {\n", xcRootNodeID);
    fprintf(out, "\t\t\tisa = PBXProject;\n");
    fprintf(out, "\t\t\tbuildConfigurationList = %s /* Build configuration list for PBXProject \"%s\" */;\n", xcBuildConfigurationListID, projectName);
    fprintf(out, "\t\t\tcompatibilityVersion = \"Xcode 3.0\";\n");
    fprintf(out, "\t\t\thasScannedForEncodings = 0;\n");
    fprintf(out, "\t\t\tmainGroup = %s;\n", xcMainGroupID);
    fprintf(out, "\t\t\tproductRefGroup = %s /* Products */;\n", xcProductsGroupID);
    fprintf(out, "\t\t\tprojectDirPath = \"\";\n");
    fprintf(out, "\t\t\tprojectRoot = \"\";\n");
    fprintf(out, "\t\t\ttargets = (\n");
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      char name[80]; targetDB.get("name", name, "DBERROR", 80);
      MAKE_XCID(xcTargetID, targetDB);
      fprintf(out, "\t\t\t\t%s /* %s */,\n", xcTargetID, name);
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      char name[80]; targetDB.get("name", name, "DBERROR", 80);
      MAKE_XCID(xcTargetID, targetDB);
      fprintf(out, "\t\t\t\t%s /* %s */,\n", xcTargetID, name);
    }
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      char name[80]; targetDB.get("name", name, "DBERROR", 80);
      MAKE_XCID(xcTargetID, targetDB);
      fprintf(out, "\t\t\t\t%s /* %s */,\n", xcTargetID, name);
    }
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t};\n");
    fprintf(out, "/* End PBXProject section */\n\n");
    return 0;
  }
  
  /*
   * Write the resource build pahse for a target.
   * Currently we do not include any resources, but we will eventually allow
   * icons for applications.
   */
  int writeResourcesBuildPhase(FILE *out, Fl_Preferences &targetDB) {
    MAKE_XCID(xcBuildResourcesID, targetDB);
    fprintf(out, "\t\t%s /* Resources */ = {\n", xcBuildResourcesID);
    fprintf(out, "\t\t\tisa = PBXResourcesBuildPhase;\n");
    fprintf(out, "\t\t\tbuildActionMask = 2147483647;\n");
    fprintf(out, "\t\t\tfiles = (\n");
    Fl_Preferences extsDB(targetDB, "externals");
    int i, n = extsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences extDB(extsDB, i);
      if (with_xcode(extDB.id())) {
        GET_UUID(refUUID, extDB);
        Fl_File_Prefs fileDB(filesDB, refUUID);
        if (strcmp(fileDB.fileExt(), ".icns")==0) {
          MAKE_XCID(xcCopyResourceID, extDB);
          fprintf(out, "\t\t\t\t%s /* %s in Resources */,\n", xcCopyResourceID, fileDB.fullName());
        }
      }
    }
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\trunOnlyForDeploymentPostprocessing = 0;\n");
    fprintf(out, "\t\t};\n");
    return 0;
  }
  
  /*
   * Write all resource build phases for all targets.
   */
  int writeResourcesBuildPhaseSection(FILE *out) {
    int i;
    fprintf(out, "/* Begin PBXResourcesBuildPhase section */\n");
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      writeResourcesBuildPhase(out, targetDB);
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeResourcesBuildPhase(out, targetDB);
    }
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      writeResourcesBuildPhase(out, targetDB);
    }
    fprintf(out, "/* End PBXResourcesBuildPhase section */\n\n");
    return 0;
  }
  
  /*
   *
   */
  int writeSourcesBuildPhase(FILE *out, Fl_Preferences &targetDB) {
    MAKE_XCID(xcBuildSourcesID, targetDB);
    fprintf(out, "\t\t%s /* Sources */ = {\n", xcBuildSourcesID);
    fprintf(out, "\t\t\tisa = PBXSourcesBuildPhase;\n");
    fprintf(out, "\t\t\tbuildActionMask = 2147483647;\n");
    fprintf(out, "\t\t\tfiles = (\n");
    // write the array of source code files
    Fl_Preferences sourcesDB(targetDB, "sources");
    int i, n = sourcesDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences sourceDB(sourcesDB, i);
      GET_UUID(refUUID, sourceDB);
      MAKE_XCID(xcBuildFileID, sourceDB);
      Fl_File_Prefs fileDB(filesDB, refUUID);
      fprintf(out, "\t\t\t\t%s /* %s in Sources */,\n", xcBuildFileID, fileDB.fullName());
    }
    Fl_Preferences flsDB(targetDB, "fl");
    n = flsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences flDB(flsDB, i);
      GET_UUID(refUUID, flDB);
      MAKE_XCID(xcBuildFileID, flDB);
      Fl_File_Prefs fileDB(filesDB, refUUID);
      fprintf(out, "\t\t\t\t%s /* %s in Sources */,\n", xcBuildFileID, fileDB.fullName());
    }
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\trunOnlyForDeploymentPostprocessing = 0;\n");
    fprintf(out, "\t\t};\n");
    return 0;
  };
  
  /*
   *
   */
  int writeSourcesBuildPhaseSection(FILE *out) {
    fprintf(out, "/* Begin PBXSourcesBuildPhase section */\n");
    int i;
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      writeSourcesBuildPhase(out, targetDB);
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeSourcesBuildPhase(out, targetDB);
    }
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      writeSourcesBuildPhase(out, targetDB);
    }
    fprintf(out, "/* End PBXSourcesBuildPhase section */\n\n");
    return 0;
  }
  
  /*
   * Write all target dependencies of a single target.
   */
  int writeTargetDependency(FILE *out, Fl_Preferences &targetDB) {
    Fl_Preferences depsDB(targetDB, "deps");
    int i, n = depsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences depDB(depsDB, i);
      GET_UUID(refUUID, depDB);
      MAKE_XCID(xcDependencyID, depDB);
      MAKE_XCID(xcProxyID, depDB);
      Fl_Preferences *depTgtDBp = 0;
      if (tgtAppsDB.groupExists(refUUID))
        depTgtDBp = new Fl_Preferences(tgtAppsDB, refUUID);
      else if (tgtLibsDB.groupExists(refUUID))
        depTgtDBp = new Fl_Preferences(tgtLibsDB, refUUID);
      else if (tgtTestsDB.groupExists(refUUID))
        depTgtDBp = new Fl_Preferences(tgtTestsDB, refUUID);
      if (depTgtDBp) {
        Fl_Preferences &depTgtDB = *depTgtDBp;
        MAKE_XCID(xcTargetID, depTgtDB);
        char name[80]; depTgtDB.get("name", name, "DBERROR", 80);
        fprintf(out, "\t\t%s /* PBXTargetDependency */ = {\n", xcDependencyID);
        fprintf(out, "\t\t\tisa = PBXTargetDependency;\n");
        fprintf(out, "\t\t\ttarget = %s /* %s */;\n", xcTargetID, name);
        fprintf(out, "\t\t\ttargetProxy = %s /* PBXContainerItemProxy */;\n", xcProxyID);
        fprintf(out, "\t\t};\n");
        delete depTgtDBp;
      }
    }
    return 0;
  }
    
  /*
   * Collect all the target dependencies from all targets and write them into 
   * this section.
   */
  int writeTargetDependencySection(FILE *out) {
    int i;
    fprintf(out, "/* Begin PBXTargetDependency section */\n");
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      writeTargetDependency(out, targetDB);      
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeTargetDependency(out, targetDB);      
    }
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      writeTargetDependency(out, targetDB);      
    }
    fprintf(out, "/* End PBXTargetDependency section */\n\n");
    return 0;
  }
  
  /*
   *
   */
  int writeProjectBuildConfiguration(FILE *out) {
    // --- project build configuration (Debug)
    fprintf(out, "\t\t%s /* Debug */ = {\n", xcBuildConfigurationDebugID);
    fprintf(out, "\t\t\tisa = XCBuildConfiguration;\n");
    fprintf(out, "\t\t\tbuildSettings = {\n");
#ifdef XCODE_DEFAULT
    fprintf(out, "\t\t\t\tARCHS = \"$(ARCHS_STANDARD_32_64_BIT)\";\n");
    fprintf(out, "\t\t\t\tGCC_C_LANGUAGE_STANDARD = gnu99;\n");
    fprintf(out, "\t\t\t\tGCC_OPTIMIZATION_LEVEL = 0;\n");
    fprintf(out, "\t\t\t\tGCC_WARN_ABOUT_RETURN_TYPE = YES;\n");
    fprintf(out, "\t\t\t\tGCC_WARN_UNUSED_VARIABLE = YES;\n");
    fprintf(out, "\t\t\t\tONLY_ACTIVE_ARCH = YES;\n");
    fprintf(out, "\t\t\t\tPREBINDING = NO;\n");
    fprintf(out, "\t\t\t\tSDKROOT = macosx10.5;\n");    
#else
    fprintf(out, "\t\t\t\tCOPY_PHASE_STRIP = NO;\n");
    fprintf(out, "\t\t\t\tGCC_ENABLE_TRIGRAPHS = YES;\n");
    fprintf(out, "\t\t\t\tGCC_OPTIMIZATION_LEVEL = 0;\n");
    fprintf(out, "\t\t\t\tGCC_PFE_FILE_C_DIALECTS = \"c c++\";\n");
    fprintf(out, "\t\t\t\tGCC_PRECOMPILE_PREFIX_HEADER = YES;\n");
    fprintf(out, "\t\t\t\tGCC_PREFIX_HEADER = \"\";\n");
    fprintf(out, "\t\t\t\tGCC_WARN_ABOUT_DEPRECATED_FUNCTIONS = NO;\n");
    fprintf(out, "\t\t\t\tMACOSX_DEPLOYMENT_TARGET = 10.2;\n");
    fprintf(out, "\t\t\t\tSDKROOT = \"$(DEVELOPER_SDK_DIR)/MacOSX10.5.sdk\";\n");
    fprintf(out, "\t\t\t\tUSER_HEADER_SEARCH_PATHS = ../../jpeg;\n");
    fprintf(out, "\t\t\t\tWARNING_CFLAGS = \"\";\n");
#endif
    fprintf(out, "\t\t\t};\n");
    fprintf(out, "\t\t\tname = Debug;\n");
    fprintf(out, "\t\t};\n");
    // --- project build configuration (Debug)
    fprintf(out, "\t\t%s /* Release */ = {\n", xcBuildConfigurationReleaseID);
    fprintf(out, "\t\t\tisa = XCBuildConfiguration;\n");
    fprintf(out, "\t\t\tbuildSettings = {\n");
#ifdef XCODE_DEFAULT
    fprintf(out, "\t\t\t\tARCHS = \"$(ARCHS_STANDARD_32_64_BIT)\";\n");
    fprintf(out, "\t\t\t\tGCC_C_LANGUAGE_STANDARD = gnu99;\n");
    fprintf(out, "\t\t\t\tGCC_WARN_ABOUT_RETURN_TYPE = YES;\n");
    fprintf(out, "\t\t\t\tGCC_WARN_UNUSED_VARIABLE = YES;\n");
    fprintf(out, "\t\t\t\tPREBINDING = NO;\n");
    fprintf(out, "\t\t\t\tSDKROOT = macosx10.5;\n");
#else
    fprintf(out, "\t\t\t\tARCHS = \"$(ARCHS_STANDARD_32_64_BIT_PRE_XCODE_3_1)\";\n");
    fprintf(out, "\t\t\t\tARCHS_STANDARD_32_64_BIT_PRE_XCODE_3_1 = \"ppc i386 x86_64\";\n");
    fprintf(out, "\t\t\t\tCOPY_PHASE_STRIP = YES;\n");
    fprintf(out, "\t\t\t\tGCC_GENERATE_DEBUGGING_SYMBOLS = NO;\n");
    fprintf(out, "\t\t\t\tGCC_PFE_FILE_C_DIALECTS = \"c c++\";\n");
    fprintf(out, "\t\t\t\tGCC_PRECOMPILE_PREFIX_HEADER = YES;\n");
    fprintf(out, "\t\t\t\tGCC_PREFIX_HEADER = \"\";\n");
    fprintf(out, "\t\t\t\tGCC_WARN_ABOUT_DEPRECATED_FUNCTIONS = NO;\n");
    fprintf(out, "\t\t\t\tMACOSX_DEPLOYMENT_TARGET = 10.3;\n");
    fprintf(out, "\t\t\t\tSDKROOT = \"$(DEVELOPER_SDK_DIR)/MacOSX10.5.sdk\";\n");
    fprintf(out, "\t\t\t\tUSER_HEADER_SEARCH_PATHS = ../../jpeg;\n");
    fprintf(out, "\t\t\t\tVALID_ARCHS = \"i386 ppc x86_64\";\n");
    fprintf(out, "\t\t\t\tWARNING_CFLAGS = \"\";\n");
#endif
    fprintf(out, "\t\t\t};\n");
    fprintf(out, "\t\t\tname = Release;\n");
    fprintf(out, "\t\t};\n");
    return 0;
  }
  
  /*
   *
   */
  int writeApplicationBuildConfiguration(FILE *out, Fl_Preferences &targetDB) {
    char name[80]; targetDB.get("name", name, "DBERROR", 80);
    MAKE_XCID(xcBuildConfigurationDebugID, targetDB);
    fprintf(out, "\t\t%s /* Debug */ = {\n", xcBuildConfigurationDebugID);
    fprintf(out, "\t\t\tisa = XCBuildConfiguration;\n");
    fprintf(out, "\t\t\tbuildSettings = {\n");
#ifdef XCODE_DEFAULT
    fprintf(out, "\t\t\t\tALWAYS_SEARCH_USER_PATHS = NO;\n");
    fprintf(out, "\t\t\t\tCOPY_PHASE_STRIP = NO;\n");
    fprintf(out, "\t\t\t\tGCC_DYNAMIC_NO_PIC = NO;\n");
    fprintf(out, "\t\t\t\tGCC_ENABLE_FIX_AND_CONTINUE = YES;\n");
    fprintf(out, "\t\t\t\tGCC_MODEL_TUNING = G5;\n");
    fprintf(out, "\t\t\t\tGCC_OPTIMIZATION_LEVEL = 0;\n");
    fprintf(out, "\t\t\t\tGCC_PRECOMPILE_PREFIX_HEADER = YES;\n");
    fprintf(out, "\t\t\t\tGCC_PREFIX_HEADER = fltk.pch;\n");
    fprintf(out, "\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = \"USING_XCODE=1\";\n");
    fprintf(out, "\t\t\t\tHEADER_SEARCH_PATHS = (\n");
    fprintf(out, "\t\t\t\t\t../../ide/XCode3/,\n");
    fprintf(out, "\t\t\t\t\t../../,\n");
    fprintf(out, "\t\t\t\t\t../../png,\n");
    fprintf(out, "\t\t\t\t\t../../jpeg,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tINFOPLIST_FILE = \"plists/%s-Info.plist\";\n", name);
    fprintf(out, "\t\t\t\tINSTALL_PATH = \"$(HOME)/Applications\";\n");
    fprintf(out, "\t\t\t\tOTHER_LDFLAGS = (\n");
    fprintf(out, "\t\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\t\tCocoa,\n");
    fprintf(out, "\t\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\t\tCarbon,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tPRODUCT_NAME = %s;\n", name);
    fprintf(out, "\t\t\t\tWARNING_CFLAGS = (\"-Wno-format-security\",\"-Wall\");\n");
#else
    fprintf(out, "\t\t\t\tCOPY_PHASE_STRIP = NO;\n");
    fprintf(out, "\t\t\t\tGCC_DYNAMIC_NO_PIC = NO;\n");
    fprintf(out, "\t\t\t\tGCC_ENABLE_FIX_AND_CONTINUE = YES;\n");
    fprintf(out, "\t\t\t\tGCC_MODEL_TUNING = G5;\n");
    fprintf(out, "\t\t\t\tGCC_OPTIMIZATION_LEVEL = 0;\n");
    fprintf(out, "\t\t\t\tGCC_PRECOMPILE_PREFIX_HEADER = YES;\n");
    fprintf(out, "\t\t\t\tGCC_PREFIX_HEADER = \"\";\n");
    fprintf(out, "\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = \"USING_XCODE=1\";\n");
    fprintf(out, "\t\t\t\tHEADER_SEARCH_PATHS = (\n");
    fprintf(out, "\t\t\t\t\t../../ide/XCode3/,\n");
    fprintf(out, "\t\t\t\t\t../../,\n");
    fprintf(out, "\t\t\t\t\t../../png,\n");
    fprintf(out, "\t\t\t\t\t../../jpeg,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tINFOPLIST_FILE = \"plists/%s-Info.plist\";\n", name);
    fprintf(out, "\t\t\t\tINSTALL_PATH = /Applications;\n");
    fprintf(out, "\t\t\t\tOTHER_LDFLAGS = (\n");
    fprintf(out, "\t\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\t\tCocoa,\n");
    fprintf(out, "\t\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\t\tCarbon,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tPREBINDING = NO;\n");
    fprintf(out, "\t\t\t\tPRODUCT_NAME = %s;\n", name);
    fprintf(out, "\t\t\t\tUSER_HEADER_SEARCH_PATHS = \"\";\n");
    fprintf(out, "\t\t\t\tWARNING_CFLAGS = (\"-Wno-format-security\",\"-Wall\");\n");
    fprintf(out, "\t\t\t\tWRAPPER_EXTENSION = app;\n");
    fprintf(out, "\t\t\t\tZERO_LINK = YES;\n");
#endif
    fprintf(out, "\t\t\t};\n");
    fprintf(out, "\t\t\tname = Debug;\n");
    fprintf(out, "\t\t};\n");
    MAKE_XCID(xcBuildConfigurationReleaseID, targetDB);
    fprintf(out, "\t\t%s /* Release */ = {\n", xcBuildConfigurationReleaseID);
    fprintf(out, "\t\t\tisa = XCBuildConfiguration;\n");
    fprintf(out, "\t\t\tbuildSettings = {\n");
#ifdef XCODE_DEFAULT
    fprintf(out, "\t\t\t\tALWAYS_SEARCH_USER_PATHS = NO;\n");
    fprintf(out, "\t\t\t\tDEBUG_INFORMATION_FORMAT = \"dwarf-with-dsym\";\n");
    fprintf(out, "\t\t\t\tGCC_MODEL_TUNING = G5;\n");
    fprintf(out, "\t\t\t\tGCC_PRECOMPILE_PREFIX_HEADER = YES;\n");
    fprintf(out, "\t\t\t\tGCC_PREFIX_HEADER = fltk.pch;\n");
    fprintf(out, "\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = \"USING_XCODE=1\";\n");
    fprintf(out, "\t\t\t\tHEADER_SEARCH_PATHS = (\n");
    fprintf(out, "\t\t\t\t\t../../ide/XCode3/,\n");
    fprintf(out, "\t\t\t\t\t../../,\n");
    fprintf(out, "\t\t\t\t\t../../png,\n");
    fprintf(out, "\t\t\t\t\t../../jpeg,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tINFOPLIST_FILE = \"plists/%s-Info.plist\";\n", name);
    fprintf(out, "\t\t\t\tINSTALL_PATH = \"$(HOME)/Applications\";\n");
    fprintf(out, "\t\t\t\tOTHER_LDFLAGS = (\n");
    fprintf(out, "\t\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\t\tCocoa,\n");
    fprintf(out, "\t\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\t\tCarbon,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tPRODUCT_NAME = %s;\n", name);
    fprintf(out, "\t\t\t\tWARNING_CFLAGS = (\"-Wno-format-security\",\"-Wall\");\n");
#else
    fprintf(out, "\t\t\t\tCOPY_PHASE_STRIP = YES;\n");
    fprintf(out, "\t\t\t\tDEBUG_INFORMATION_FORMAT = \"dwarf-with-dsym\";\n");
    fprintf(out, "\t\t\t\tGCC_ENABLE_FIX_AND_CONTINUE = NO;\n");
    fprintf(out, "\t\t\t\tGCC_MODEL_TUNING = G5;\n");
    fprintf(out, "\t\t\t\tGCC_PRECOMPILE_PREFIX_HEADER = YES;\n");
    fprintf(out, "\t\t\t\tGCC_PREFIX_HEADER = \"\";\n");
    fprintf(out, "\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = \"USING_XCODE=1\";\n");
    fprintf(out, "\t\t\t\tHEADER_SEARCH_PATHS = (\n");
    fprintf(out, "\t\t\t\t\t../../ide/XCode3/,\n");
    fprintf(out, "\t\t\t\t\t../../,\n");
    fprintf(out, "\t\t\t\t\t../../png,\n");
    fprintf(out, "\t\t\t\t\t../../jpeg,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tINFOPLIST_FILE = \"plists/%s-Info.plist\";\n", name);
    fprintf(out, "\t\t\t\tINSTALL_PATH = /Applications;\n");
    fprintf(out, "\t\t\t\tOTHER_LDFLAGS = (\n");
    fprintf(out, "\t\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\t\tCocoa,\n");
    fprintf(out, "\t\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\t\tCarbon,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tPREBINDING = NO;\n");
    fprintf(out, "\t\t\t\tPRODUCT_NAME = %s;\n", name);
    fprintf(out, "\t\t\t\tUSER_HEADER_SEARCH_PATHS = \"\";\n");
    fprintf(out, "\t\t\t\tWARNING_CFLAGS = (\"-Wno-format-security\",\"-Wall\");\n");
    fprintf(out, "\t\t\t\tWRAPPER_EXTENSION = app;\n");
    fprintf(out, "\t\t\t\tZERO_LINK = NO;\n");
#endif
    fprintf(out, "\t\t\t};\n");
    fprintf(out, "\t\t\tname = Release;\n");
    fprintf(out, "\t\t};\n");
    return 0;
  }
    
  /*
   *
   */
  int writeFrameworkBuildConfiguration(FILE *out, Fl_Preferences &targetDB) {
    char name[80]; targetDB.get("name", name, "DBERROR", 80);
    MAKE_XCID(xcBuildConfigurationDebugID, targetDB);
    fprintf(out, "\t\t%s /* Debug */ = {\n", xcBuildConfigurationDebugID);
    fprintf(out, "\t\t\tisa = XCBuildConfiguration;\n");
    fprintf(out, "\t\t\tbuildSettings = {\n");
#ifdef XCODE_DEFAULT
    fprintf(out, "\t\t\t\tALWAYS_SEARCH_USER_PATHS = NO;\n");
    fprintf(out, "\t\t\t\tCOPY_PHASE_STRIP = NO;\n");
    fprintf(out, "\t\t\t\tDYLIB_COMPATIBILITY_VERSION = 1;\n");
    fprintf(out, "\t\t\t\tDYLIB_CURRENT_VERSION = 1;\n");
    fprintf(out, "\t\t\t\tFRAMEWORK_VERSION = A;\n");
    fprintf(out, "\t\t\t\tGCC_DYNAMIC_NO_PIC = NO;\n");
    fprintf(out, "\t\t\t\tGCC_ENABLE_FIX_AND_CONTINUE = YES;\n");
    fprintf(out, "\t\t\t\tGCC_MODEL_TUNING = G5;\n");
    fprintf(out, "\t\t\t\tGCC_OPTIMIZATION_LEVEL = 0;\n");
    fprintf(out, "\t\t\t\tGCC_PRECOMPILE_PREFIX_HEADER = YES;\n");
    fprintf(out, "\t\t\t\tGCC_PREFIX_HEADER = fltk.pch;\n");
    fprintf(out, "\t\t\t\tHEADER_SEARCH_PATHS = (\n");
    fprintf(out, "\t\t\t\t\t../../ide/XCode3/,\n");
    fprintf(out, "\t\t\t\t\t../../,\n");
    fprintf(out, "\t\t\t\t\t../../png,\n");
    fprintf(out, "\t\t\t\t\t../../jpeg,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tINFOPLIST_FILE = \"plists/%s-Info.plist\";\n", name);
    fprintf(out, "\t\t\t\tINSTALL_PATH = \"@executable_path/../Frameworks\";\n");
    fprintf(out, "\t\t\t\tOTHER_LDFLAGS = (\n");
    fprintf(out, "\t\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\t\tCocoa,\n");
    fprintf(out, "\t\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\t\tCarbon,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tPREBINDING = NO;\n");
    fprintf(out, "\t\t\t\tPRODUCT_NAME = %s;\n", name);
    fprintf(out, "\t\t\t\tWARNING_CFLAGS = (\"-Wno-format-security\",\"-Wall\");\n");
#else
    fprintf(out, "\t\t\t\tALWAYS_SEARCH_USER_PATHS = YES;\n");
    fprintf(out, "\t\t\t\tCOPY_PHASE_STRIP = NO;\n");
    fprintf(out, "\t\t\t\tDYLIB_COMPATIBILITY_VERSION = 1;\n");
    fprintf(out, "\t\t\t\tDYLIB_CURRENT_VERSION = 1;\n");
    fprintf(out, "\t\t\t\tEXCLUDED_RECURSIVE_SEARCH_PATH_SUBDIRECTORIES = \"*.nib *.lproj *.framework *.gch (*) CVS .svn *.xcodeproj *.xcode *.pbproj *.pbxproj\";\n");
    fprintf(out, "\t\t\t\tFRAMEWORK_VERSION = A;\n");
    fprintf(out, "\t\t\t\tGCC_DYNAMIC_NO_PIC = NO;\n");
    fprintf(out, "\t\t\t\tGCC_ENABLE_FIX_AND_CONTINUE = YES;\n");
    fprintf(out, "\t\t\t\tGCC_MODEL_TUNING = G5;\n");
    fprintf(out, "\t\t\t\tGCC_OPTIMIZATION_LEVEL = 0;\n");
    fprintf(out, "\t\t\t\tGCC_PFE_FILE_C_DIALECTS = \"c c++ objective-c++\";\n");
    fprintf(out, "\t\t\t\tHEADER_SEARCH_PATHS = (\n");
    fprintf(out, "\t\t\t\t\t../../ide/XCode3/,\n");
    fprintf(out, "\t\t\t\t\t../../,\n");
    fprintf(out, "\t\t\t\t\t../../png,\n");
    fprintf(out, "\t\t\t\t\t../../jpeg,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tINFOPLIST_FILE = \"plists/%s-Info.plist\";\n", name);
    fprintf(out, "\t\t\t\tINSTALL_PATH = \"@executable_path/../Frameworks\";\n");
    fprintf(out, "\t\t\t\tOTHER_LDFLAGS = (\n");
    fprintf(out, "\t\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\t\tCocoa,\n");
    fprintf(out, "\t\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\t\tCarbon,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tPREBINDING = NO;\n");
    fprintf(out, "\t\t\t\tPRODUCT_NAME = %s;\n", name);
    fprintf(out, "\t\t\t\tUSER_HEADER_SEARCH_PATHS = \"\";\n");
    fprintf(out, "\t\t\t\tWARNING_CFLAGS = (\"-Wno-format-security\",\"-Wall\");\n");
    fprintf(out, "\t\t\t\tZERO_LINK = YES;\n");
#endif
    fprintf(out, "\t\t\t};\n");
    fprintf(out, "\t\t\tname = Debug;\n");
    fprintf(out, "\t\t};\n");
    MAKE_XCID(xcBuildConfigurationReleaseID, targetDB);
    fprintf(out, "\t\t%s /* Release */ = {\n", xcBuildConfigurationReleaseID);
    fprintf(out, "\t\t\tisa = XCBuildConfiguration;\n");
    fprintf(out, "\t\t\tbuildSettings = {\n");
#ifdef XCODE_DEFAULT
    fprintf(out, "\t\t\t\tALWAYS_SEARCH_USER_PATHS = NO;\n");
    fprintf(out, "\t\t\t\tCOPY_PHASE_STRIP = YES;\n");
    fprintf(out, "\t\t\t\tDEBUG_INFORMATION_FORMAT = \"dwarf-with-dsym\";\n");
    fprintf(out, "\t\t\t\tDYLIB_COMPATIBILITY_VERSION = 1;\n");
    fprintf(out, "\t\t\t\tDYLIB_CURRENT_VERSION = 1;\n");
    fprintf(out, "\t\t\t\tFRAMEWORK_VERSION = A;\n");
    fprintf(out, "\t\t\t\tGCC_ENABLE_FIX_AND_CONTINUE = NO;\n");
    fprintf(out, "\t\t\t\tGCC_MODEL_TUNING = G5;\n");
    fprintf(out, "\t\t\t\tGCC_PRECOMPILE_PREFIX_HEADER = YES;\n");
    fprintf(out, "\t\t\t\tGCC_PREFIX_HEADER = fltk.pch;\n");
    fprintf(out, "\t\t\t\tHEADER_SEARCH_PATHS = (\n");
    fprintf(out, "\t\t\t\t\t../../ide/XCode3/,\n");
    fprintf(out, "\t\t\t\t\t../../,\n");
    fprintf(out, "\t\t\t\t\t../../png,\n");
    fprintf(out, "\t\t\t\t\t../../jpeg,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tINFOPLIST_FILE = \"plists/%s-Info.plist\";\n", name);
    fprintf(out, "\t\t\t\tINSTALL_PATH = \"@executable_path/../Frameworks\";\n");
    fprintf(out, "\t\t\t\tOTHER_LDFLAGS = (\n");
    fprintf(out, "\t\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\t\tCocoa,\n");
    fprintf(out, "\t\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\t\tCarbon,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tPREBINDING = NO;\n");
    fprintf(out, "\t\t\t\tPRODUCT_NAME = %s;\n", name);
    fprintf(out, "\t\t\t\tWARNING_CFLAGS = (\"-Wno-format-security\",\"-Wall\");\n");
    fprintf(out, "\t\t\t\tZERO_LINK = NO;\n");
#else
    fprintf(out, "\t\t\t\tALWAYS_SEARCH_USER_PATHS = YES;\n");
    fprintf(out, "\t\t\t\tCOPY_PHASE_STRIP = YES;\n");
    fprintf(out, "\t\t\t\tDEBUG_INFORMATION_FORMAT = \"dwarf-with-dsym\";\n");
    fprintf(out, "\t\t\t\tDYLIB_COMPATIBILITY_VERSION = 1;\n");
    fprintf(out, "\t\t\t\tDYLIB_CURRENT_VERSION = 1;\n");
    fprintf(out, "\t\t\t\tEXCLUDED_RECURSIVE_SEARCH_PATH_SUBDIRECTORIES = \"*.nib *.lproj *.framework *.gch (*) CVS .svn *.xcodeproj *.xcode *.pbproj *.pbxproj\";\n");
    fprintf(out, "\t\t\t\tFRAMEWORK_VERSION = A;\n");
    fprintf(out, "\t\t\t\tGCC_ENABLE_FIX_AND_CONTINUE = NO;\n");
    fprintf(out, "\t\t\t\tGCC_MODEL_TUNING = G5;\n");
    fprintf(out, "\t\t\t\tGCC_PFE_FILE_C_DIALECTS = \"c c++ objective-c++\";\n");
    fprintf(out, "\t\t\t\tHEADER_SEARCH_PATHS = (\n");
    fprintf(out, "\t\t\t\t\t../../ide/XCode3/,\n");
    fprintf(out, "\t\t\t\t\t../../,\n");
    fprintf(out, "\t\t\t\t\t../../png,\n");
    fprintf(out, "\t\t\t\t\t../../jpeg,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tINFOPLIST_FILE = \"plists/%s-Info.plist\";\n", name);
    fprintf(out, "\t\t\t\tINSTALL_PATH = \"@executable_path/../Frameworks\";\n");
    fprintf(out, "\t\t\t\tMACOSX_DEPLOYMENT_TARGET = 10.2;\n");
    fprintf(out, "\t\t\t\tOTHER_LDFLAGS = (\n");
    fprintf(out, "\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\tCocoa,\n");
    fprintf(out, "\t\t\t\t\t\"-framework\",\n");
    fprintf(out, "\t\t\t\t\tCarbon,\n");
    fprintf(out, "\t\t\t\t);\n");
    fprintf(out, "\t\t\t\tPREBINDING = NO;\n");
    fprintf(out, "\t\t\t\tPRODUCT_NAME = %s;\n", name);
    fprintf(out, "\t\t\t\tSDKROOT = \"\";\n");
    fprintf(out, "\t\t\t\tUSER_HEADER_SEARCH_PATHS = \"\";\n");
    fprintf(out, "\t\t\t\tWARNING_CFLAGS = (\"-Wno-format-security\",\"-Wall\");\n");
    fprintf(out, "\t\t\t\tZERO_LINK = NO;\n");
#endif
    fprintf(out, "\t\t\t};\n");
    fprintf(out, "\t\t\tname = Release;\n");
    fprintf(out, "\t\t};\n");
    return 0;
  }
  
  /*
   * This block contains build configurations for every target and project
   */
  int writeBuildConfigurationSection(FILE *out) {
    writeProjectBuildConfiguration(out);
    int i;
    fprintf(out, "/* Begin XCBuildConfiguration section */\n");
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      writeApplicationBuildConfiguration(out, targetDB);
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeFrameworkBuildConfiguration(out, targetDB);
    }
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      writeApplicationBuildConfiguration(out, targetDB);
    }
    fprintf(out, "/* End XCBuildConfiguration section */\n\n");
    return 0;
  }

  /*
   * Write the build onfiguration list for the entire project.
   */
  int writeProjectConfigurationList(FILE *out) {
    fprintf(out, "\t\t%s /* Build configuration list for PBXProject \"%s\" */ = {\n", xcBuildConfigurationListID, projectName);
    fprintf(out, "\t\t\tisa = XCConfigurationList;\n");
    fprintf(out, "\t\t\tbuildConfigurations = (\n");
    fprintf(out, "\t\t\t\t%s /* Debug */,\n", xcBuildConfigurationDebugID);
    fprintf(out, "\t\t\t\t%s /* Release */,\n", xcBuildConfigurationReleaseID);
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\tdefaultConfigurationIsVisible = 0;\n");
    fprintf(out, "\t\t\tdefaultConfigurationName = Debug;\n");
    fprintf(out, "\t\t};\n");
    return 0;
  }
    
  /*
   * Write the build onfiguration list for the entire project.
   */
  int writeTargetConfigurationList(FILE *out, Fl_Preferences &targetDB) {
    char name[80]; targetDB.get("name", name, "DBERROR", 80);
    MAKE_XCID(xcBuildConfigurationListID, targetDB);
    MAKE_XCID(xcBuildConfigurationDebugID, targetDB);
    MAKE_XCID(xcBuildConfigurationReleaseID, targetDB);
    fprintf(out, "\t\t%s /* Build configuration list for PBXProject \"%s\" */ = {\n", xcBuildConfigurationListID, name);
    fprintf(out, "\t\t\tisa = XCConfigurationList;\n");
    fprintf(out, "\t\t\tbuildConfigurations = (\n");
    fprintf(out, "\t\t\t\t%s /* Debug */,\n", xcBuildConfigurationDebugID);
    fprintf(out, "\t\t\t\t%s /* Release */,\n", xcBuildConfigurationReleaseID);
    fprintf(out, "\t\t\t);\n");
    fprintf(out, "\t\t\tdefaultConfigurationIsVisible = 0;\n");
    fprintf(out, "\t\t\tdefaultConfigurationName = Debug;\n");
    fprintf(out, "\t\t};\n");
    return 0;
  }
 
  /*
   * This block contains arrays to the available build configurations
   */
  int writeConfigurationListSection(FILE *out) {
    int i;
    fprintf(out, "/* Begin XCConfigurationList section */\n");
    // --- project configuration list
    writeProjectConfigurationList(out);
    // --- configuration list for all targets
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      writeTargetConfigurationList(out, targetDB);
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writeTargetConfigurationList(out, targetDB);
    }
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      writeTargetConfigurationList(out, targetDB);
    }
    fprintf(out, "/* End XCConfigurationList section */\n\n");
    return 0;
  }
  
  /*
   * Write the project definition file
   */
  int writeProjectFile(const char *filepath) {
    char filename[2048];
    fl_snprintf(filename, 2047, "%s/project.pbxproj", filepath);
    FILE *out = fopen(filename, "wb");
    if (!out) {
      fl_alert("Can't open file:\n%s", filename);
      return -1;
    }
    fprintf(out, 
            "// !$*UTF8*$!\n" "{\n" "\tarchiveVersion = 1;\n" "\tclasses = {\n"
            "\t};\n" "\tobjectVersion = 44;\n" "\tobjects = {\n\n");
    writeBuildFileSection(out);
    writeBuildRuleSection(out);
    writeContainerItemProxySection(out);
    writeCopyFilesBuildPhaseSection(out);
    writeFileReferenceSection(out);
    writeFrameworksBuildPhaseSection(out);
    writeGroupSection(out);
    writeHeadersBuildPhaseSection(out);
    writeNativeTargetSection(out);
    writeProjectSection(out);
    writeResourcesBuildPhaseSection(out);
    writeSourcesBuildPhaseSection(out);
    writeTargetDependencySection(out);
    writeBuildConfigurationSection(out);
    writeConfigurationListSection(out);
    fprintf(out,
            "\t};\n"
            "\trootObject = %s /* Project object */;\n"
            "}\n", xcRootNodeID);

    fclose(out);
    return 0;
  }
  
  int writeConfigH(const char *filename) {
    FILE *f = fopen(filename, "wb");
    fputs("/*\n * \"$Id$\"\n"
          " *\n * Configuration file for the Fast Light Tool Kit (FLTK).\n *\n"
          " * Copyright 1998-2010 by Bill Spitzak and others.\n */\n\n", f);
    fputs("#define FLTK_DATADIR \"/usr/local/share/fltk\"\n"
          "#define FLTK_DOCDIR \"/usr/local/share/doc/fltk\"\n"
          "#define BORDER_WIDTH 2\n#define HAVE_GL 1\n#define HAVE_GL_GLU_H 1\n"
          "#define USE_COLORMAP 1\n#define HAVE_XINERAMA 0\n#define USE_XFT 0\n"
          "#define HAVE_XDBE 0\n#define USE_XDBE HAVE_XDBE\n", f);
    fputs("#define USE_QUARTZ 1\n#define __APPLE_QUARTZ__ 1\n"
          "#define __APPLE_COCOA__ 1\n#define HAVE_OVERLAY 0\n"
          "#define HAVE_GL_OVERLAY HAVE_OVERLAY\n#define WORDS_BIGENDIAN 0\n"
          "#define U16 unsigned short\n#define U32 unsigned\n"
          "#define HAVE_DIRENT_H 1\n#define HAVE_SCANDIR 1\n"
          "#define HAVE_VSNPRINTF 1\n", f);
    fputs("#define HAVE_SNPRINTF 1\n#define HAVE_STRINGS_H 1\n"
          "#define HAVE_STRCASECMP 1\n#define HAVE_STRLCAT 1\n"
          "#define HAVE_STRLCPY 1\n#define HAVE_LOCALE_H 1\n"
          "#define HAVE_LOCALECONV 1\n#define HAVE_SYS_SELECT_H 1\n"
          "#define USE_POLL 0\n#define HAVE_LIBPNG 1\n#define HAVE_LIBZ 1\n"
          "#define HAVE_LIBJPEG 1\n#define HAVE_PNG_H 1\n", f);
    fputs("#define HAVE_PTHREAD 1\n#define HAVE_PTHREAD_H 1\n"
          "#define HAVE_LONG_LONG 1\n#define FLTK_LLFMT \"%lld\"\n"
          "#define FLTK_LLCAST (long long)\n#define HAVE_STRTOLL 1\n"
          "#define HAVE_DLFCN_H 1\n#define HAVE_DLSYM 1\n\n", f);
    fputs("/*\n"
          " * End of \"$Id$\".\n"
          " */", f);
    fclose(f);
    return 0;
  }
  
  int writePCH(const char *filename) {
    FILE *f = fopen(filename, "wb");
    fputs("//\n// Prefix header for all source files\n//\n\n"
          "#ifdef __OBJC__\n#import <Cocoa/Cocoa.h>\n#endif\n\n", f);
    fclose(f);
    return 0;
  }
  
  int createIcons(const char *filepath) {
    // FIXME: LATER: create a minimum set of icon files?
    return 0;
  }
  
  /*
   * create a single plists/[name]-Info.plist 
   */
  int writePList(const char *filepath, Fl_Preferences &target_db, int fmwk=0) {
    char name[80]; target_db.get("name", name, "DBERROR", 79);
    char filename[2048]; 
    fl_snprintf(filename, 2047, "%s/%s-Info.plist", filepath, name);
    FILE *f = fopen(filename, "wb");
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
    fprintf(f, "<plist version=\"1.0\">\n");
    fprintf(f, "<dict>\n");
    fprintf(f, "\t<key>CFBundleDevelopmentRegion</key>\n");
    fprintf(f, "\t<string>English</string>\n");
    if (strcmp(name, "Fluid")==0) { // TODO: this option is not yet in the database
      fprintf(f, "\t<key>CFBundleDocumentTypes</key>\n");
      fprintf(f, "\t<array>\n");
      fprintf(f, "\t\t<dict>\n");
      fprintf(f, "\t\t\t<key>CFBundleTypeExtensions</key>\n");
      fprintf(f, "\t\t\t<array>\n");
      fprintf(f, "\t\t\t\t<string>fl</string>\n");
      fprintf(f, "\t\t\t</array>\n");
      fprintf(f, "\t\t\t<key>CFBundleTypeIconFile</key>\n");
      fprintf(f, "\t\t\t<string>fluid_doc</string>\n");
      fprintf(f, "\t\t\t<key>CFBundleTypeName</key>\n");
      fprintf(f, "\t\t\t<string>Fluid Documents</string>\n");
      fprintf(f, "\t\t\t<key>CFBundleTypeOSTypes</key>\n");
      fprintf(f, "\t\t\t<array>\n");
      fprintf(f, "\t\t\t\t<string>flid</string>\n");
      fprintf(f, "\t\t\t</array>\n");
      fprintf(f, "\t\t\t<key>CFBundleTypeRole</key>\n");
      fprintf(f, "\t\t\t<string>Editor</string>\n");
      fprintf(f, "\t\t\t<key>LSTypeIsPackage</key>\n");
      fprintf(f, "\t\t\t<false/>\n");
      fprintf(f, "\t\t\t<key>NSPersistentStoreTypeKey</key>\n");
      fprintf(f, "\t\t\t<string>Binary</string>\n");
      fprintf(f, "\t\t</dict>\n");
      fprintf(f, "\t</array>\n");
    }
    fprintf(f, "\t<key>CFBundleExecutable</key>\n");
    fprintf(f, "\t<string>${EXECUTABLE_NAME}</string>\n");
    // find the first suitable icon file if there is one
    Fl_Preferences extsDB(target_db, "externals");
    int i, n = extsDB.groups();
    for (i=0; i<n; i++) {
      Fl_Preferences extDB(extsDB, i);
      if (with_xcode(extDB.id())) {
        GET_UUID(refUUID, extDB);
        Fl_File_Prefs fileDB(filesDB, refUUID);
        if (strcmp(fileDB.fileExt(), ".icns")==0) {
          fprintf(f, "\t<key>CFBundleIconFile</key>\n\t<string>%s</string>", fileDB.fileName());
          break;
        }
      }
    }
    fprintf(f, "\t<key>CFBundleIdentifier</key>\n");
    fprintf(f, "\t<string>org.fltk.%s</string>\n", name);
    fprintf(f, "\t<key>CFBundleInfoDictionaryVersion</key>\n");
    fprintf(f, "\t<string>6.0</string>\n");
    fprintf(f, "\t<key>CFBundlePackageType</key>\n");
    if (fmwk)
      fprintf(f, "\t<string>FMWK</string>\n");
    else
      fprintf(f, "\t<string>APPL</string>\n");
    fprintf(f, "\t<key>CFBundleSignature</key>\n");
    fprintf(f, "\t<string>FLTK</string>\n");
    fprintf(f, "\t<key>CFBundleVersion</key>\n");
    fprintf(f, "\t<string>1.0</string>\n");
    fprintf(f, "</dict>\n");
    fprintf(f, "\t</plist>\n");
    fclose(f);
    return 0;
  }
  
  /*
   * Create the plist files for all apps and tests 
   */
  int writePLists(const char *filepath) {
    int i;
    for (i=0; i<nTgtApps; i++) {
      Fl_Preferences targetDB(tgtAppsDB, i);
      writePList(filepath, targetDB, 0);
    }
    for (i=0; i<nTgtLibs; i++) {
      Fl_Preferences targetDB(tgtLibsDB, i);
      writePList(filepath, targetDB, 1);
    }
    for (i=0; i<nTgtTests; i++) {
      Fl_Preferences targetDB(tgtTestsDB, i);
      writePList(filepath, targetDB, 0);
    }
    return 0;
  }
  
  /*
   * Write the entire system of files.
   */
  int write() {
    char filepath[2048];
    // --- create directory structure ide/Xcode3
    sprintf(filepath, "%s/ide", rootDir); fl_mkdir(filepath, 0777);
    sprintf(filepath, "%s/ide/Xcode3", rootDir); fl_mkdir(filepath, 0777);
    // --- create project.pbxproj
    sprintf(filepath, "%s/ide/Xcode3/FLTK.xcodeproj", rootDir); fl_mkdir(filepath, 0777);
    writeProjectFile(filepath);
    // --- create a valid config.h
    sprintf(filepath, "%s/ide/Xcode3/config.h", rootDir);
    writeConfigH(filepath);
    // --- create a valid fltk.pch
    sprintf(filepath, "%s/ide/Xcode3/fltk.pch", rootDir);
    writePCH(filepath);
    // --- FIXME: LATER: create default icons (maybe import icons for apps?)
    sprintf(filepath, "%s/ide/Xcode3/icons", rootDir); fl_mkdir(filepath, 0777);
    createIcons(filepath);
    // --- create plists/[name]-Info.plist
    sprintf(filepath, "%s/ide/Xcode3/plists", rootDir); fl_mkdir(filepath, 0777);
    writePLists(filepath);
    // --- close and finish
    return 0;
  }
};


// This creates all files needed to compile FLTK using Xcode.
// create directory structure ide/Xcode3
// create FLTK.xcodeproj directory
// create project.pbxproj
// create config.h
// create icons
// create plists/[name]-Info.plist
void generate_fltk_Xcode3_support(const char *filename, const char *targetpath)
{
  Fl_Preferences *db = 
    new Fl_Preferences(filename, "fltk.org", 0);
  Xcode3_IDE ide(*db, targetpath);
  ide.write();
  return;
}


extern int exit_early;

class Fl_IDE_Xcode_Plugin : public Fl_Commandline_Plugin
{
public:
  Fl_IDE_Xcode_Plugin() : Fl_Commandline_Plugin(name()) { }
  const char *name() { return "ideXcode.fluid.fltk.org"; }
  const char *help() { return
    " --dbxcode3 <dbname> <targetpath> : create all IDE files for an Xcode3 project"; } 
  int arg(int argc, char **argv, int &i) {
    if (argc>=i+1 && strcmp(argv[i], "--dbxcode3")==0) {
      if (argc>=i+3 && argv[i+1][0]!='-' && argv[i+2][0]!='-') {
        fprintf(stderr, "Creating Xcode 3.0 IDE from %s in %s\n", argv[i+1], argv[i+2]);
        exit_early = 1;
        generate_fltk_Xcode3_support(argv[i+1], argv[i+2]);
        i = i+3;
        return 3;
      } else {
        fprintf(stderr, "Missing argument: --dbxcode3 <dbname> <targetpath>\n");
        return 1;
      }
    }
    return 0;
  }
  int test(const char *a1, const char *a2, const char *a3) {
    generate_fltk_Xcode3_support(a1, a2);
    return 0;
  }
};
Fl_IDE_Xcode_Plugin IDE_Xcode_Plugin;

/* Random bit of information:
 
 ~/Library/Application Support/Developer/Shared/Xcode/Specifications/fluid.pbfilespec
 
 {
   Identifier = sourcecode.fluid;
   BasedOn = sourcecode;
   Extensions = (fl);
 },
 
*/

//
// End of "$Id$".
//
