
/*
 *	Fluid Tool.c	-	Fluid Tool (68K and PowerPC) for CodeWarriorª IDE
 *
 *  Based on Metrowerks Sample Plugin:
 *  Copyright © 1995-2002 Metrowerks Inc.  All rights reserved.
 *
 */

/* standard headers */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stat.h>
#include <errno.h>

/* system headers */
#if macintosh
#include <Files.h>
#include <Strings.h>
#endif

/* compiler headers */
#include "DropInCompilerLinker.h"
#include "CompilerMapping.h"
#include "CWPluginErrors.h"

/* project headers */
#include "../commonFiles/FluidPanel.h"


/* prototypes of local functions */
static CWResult	Compile(CWPluginContext context);
static CWResult	ProcessFile(CWPluginContext context, const char* text, Boolean include);
static CWResult	ParseText(CWPluginContext context, const char* text);
static CWResult	IncludeFile(CWPluginContext context, const char* inclname, 
					Boolean fullSearch);
int IsNewlineChar(char c);

/* local variables */
static long		linecount;


#if CW_USE_PRAGMA_EXPORT
#pragma export on
#endif

/**
 * inform the API about our capabilities by returning the DropInFlags structure
 */
CWPLUGIN_ENTRY(CWPlugin_GetDropInFlags)(const DropInFlags** flags, long* flagsSize)
{
	static const DropInFlags sFlags = {
		kCurrentDropInFlagsVersion,
		CWDROPINCOMPILERTYPE,
		DROPINCOMPILERLINKERAPIVERSION_7,
		/*kCanpreprocess | */ kCompAllowDupFileNames,
		Lang_MISC,
		DROPINCOMPILERLINKERAPIVERSION
	};
	
	*flags = &sFlags;
	*flagsSize = sizeof(sFlags);
	
	return cwNoErr;
}

CWPLUGIN_ENTRY(CWPlugin_GetDropInName)(const char** dropinName)
{
	static const char* sDropInName = "Fluid Resource Compiler";
	
	*dropinName = sDropInName;
	
	return cwNoErr;
}

CWPLUGIN_ENTRY(CWPlugin_GetDisplayName)(const char** displayName)
{
	static const char* sDisplayName = "Fluid Resource Compiler";
	
	*displayName = sDisplayName;
	
	return cwNoErr;
}

CWPLUGIN_ENTRY(CWPlugin_GetPanelList)(const CWPanelList** panelList)
{
	static const char* sPanelName = "Fluid Panel";
	static CWPanelList sPanelList = {kCurrentCWPanelListVersion, 1, &sPanelName};
	
	*panelList = &sPanelList;
	
	return cwNoErr;
}

/**
 * tell CW about the processors and platforms that we support
 * (which would be any that we can compile Fluid under)
 */
CWPLUGIN_ENTRY(CWPlugin_GetTargetList)(const CWTargetList** targetList)
{
	static CWDataType sCPU = targetCPUAny;
	static CWDataType sOS = targetOSAny;
	static CWTargetList sTargetList = {kCurrentCWTargetListVersion, 1, &sCPU, 1, &sOS};
	
	*targetList = &sTargetList;
	
	return cwNoErr;
}

/**
 * tell CW which file types we can process
 */
CWPLUGIN_ENTRY(CWPlugin_GetDefaultMappingList)(const CWExtMapList** defaultMappingList)
{
	static CWExtensionMapping sExtension = {'TEXT', ".fl", 0};
	static CWExtMapList sExtensionMapList = {kCurrentCWExtMapListVersion, 1, &sExtension};
	
	*defaultMappingList = &sExtensionMapList;
	
	return cwNoErr;
}

#if CW_USE_PRAGMA_EXPORT
#pragma export off
#endif

/*
 *	main	-	main entry-point for Drop-In Fluid Tool
 *
 */

CWPLUGIN_ENTRY(main)(CWPluginContext context)
{
	short		result;
	long		request;
	
	if (CWGetPluginRequest(context, &request) != cwNoErr)
		return cwErrRequestFailed;
	
	result = cwNoErr;
		
	/* dispatch on compiler request */
	switch (request)
	{
	case reqInitCompiler:
		/* compiler has just been loaded into memory */
		break;
		
	case reqTermCompiler:
		/* compiler is about to be unloaded from memory */
		break;
		
	case reqCompile:
  	{
  	  Boolean pp;
  	  result = CWIsPreprocessing( context, &pp );
      //if ( pp ) 
        result = Compile(context);
    }
		break;
		
	default:
		result = cwErrRequestFailed;
		break;
	}
	
	/* return result code */
	return (result);
}

/**
 * check if the path points to an executable file
 */
static char exist( const char *filename )
{
  struct stat eStat;
  int ret = stat( filename, &eStat );
  if ( ret != 0 ) return 0;
  if ( eStat.st_mode && S_IEXEC ) return 1;
  return 0;
}

/**
 * call (or incorporate) Fluid with our source file as an argument
 */
static CWResult	Compile(CWPluginContext context)
{
	CWResult	err;
	CWFileSpec	sourcefile;
	int ret;
	char preFileAction = 0;
	char *fluidCmd = 0;
	char *fluidCall = 0;
	char *flPath = 0;
	const char *flFile = 0;
  CWFileInfo fileInfo = {
    true, cwNoDependency, 0, true, 0
  };
  CWFileSpec cxxPath, hPath;

  // find FLUID: check the target path for fluid.exe and fluidd.exe
  
  err = CWFindAndLoadFile( context, "fluid.exe", &fileInfo );
  if ( err != cwNoErr )
    err = CWFindAndLoadFile( context, "fluidd.exe", &fileInfo );    
  if ( err == cwNoErr )
    fluidCmd = strdup( fileInfo.filespec.path );
    
  // No FLUID in the project path, search the CW compiler paths
  
  if ( !fluidCmd )
  {
	  const char *cwFolder = 0;
    cwFolder = getenv( "CWFOLDER" );
    if ( cwFolder )
    {
      char found = 0;
      fluidCmd = (char*)malloc( strlen( cwFolder ) + 38 );
      if (!found)
      {
        sprintf( fluidCmd, "%s\\Bin\\fluid.exe", cwFolder );
        found = exist( fluidCmd );
      }
      if (!found)
      {
        sprintf( fluidCmd, "%s\\Bin\\fluidd.exe", cwFolder );
        found = exist( fluidCmd );
      }
      if (!found)
      {
        sprintf( fluidCmd, "%s\\Bin\\Plugins\\Compiler\\fluid.exe", cwFolder );
        found = exist( fluidCmd );
      }
      if (!found)
      {
        sprintf( fluidCmd, "%s\\Bin\\Plugins\\Compiler\\fluidd.exe", cwFolder );
        found = exist( fluidCmd );
      }
      if (!found)
      {
        free( fluidCmd );
        fluidCmd = 0;
      }
    }
  }
  
  if ( !fluidCmd )
  {
    CWReportMessage( context, NULL,
      "Fluid resource compiler not found",
      "Could not find fluid.exe in project path or CodeWarrior parth",
      messagetypeError, 0 );	
    return cwErrRequestFailed;
  }
  
  // get the path to the file we want to compile

	err = CWGetMainFileSpec(context, &sourcefile);
	if (!CWSUCCESS(err))
		goto bail;

  flFile = strrchr(sourcefile.path, '.' );
  if ( flFile )
  {
    int len = strlen(sourcefile.path);
    strcpy( cxxPath.path, sourcefile.path );
    strcpy( cxxPath.path+(flFile-sourcefile.path), ".cxx" );
    strcpy( hPath.path, sourcefile.path );
    strcpy( hPath.path+(flFile-sourcefile.path), ".h" );
    CWPreFileAction( context, &cxxPath );
    CWPreFileAction( context, &hPath );
    preFileAction = 1;
  }

  // chdir to our .fl file and call fluid  

  flFile = strrchr( sourcefile.path, '\\' );
  if ( !flFile )
    flFile = strrchr( sourcefile.path, '/' );
  if ( !flFile )
  {
    CWReportMessage( context, NULL,
      "Can't determine .fl file path",
      NULL,
      messagetypeError, 0 );	
    err = cwErrSilent;
    goto bail;
  }
  flPath = strdup( sourcefile.path );
  flPath[ flFile-sourcefile.path ] = 0;
  flFile++;
  
  ret = chdir( flPath );
  if ( ret )
  {
    CWReportMessage( context, NULL,
      "Can't access .fl file path",
      flPath,
      messagetypeError, 0 );	
    err = cwErrSilent;
    goto bail;
  }

  // create and run our command

  fluidCall = (char*)malloc( strlen(fluidCmd) + strlen( flFile ) + 24 );
  sprintf( fluidCall, "%s -c \"%s\"", fluidCmd, flFile );

  ret = spawnl( P_WAIT, fluidCmd, fluidCmd, "-c", flFile, 0 );

  if ( ret )
  {
    CWReportMessage( context, NULL,
      "Fluid system call failed.",
      fluidCall,
      messagetypeError, 0 );	
    err = cwErrSilent;
    goto bail;
  }

  CWReportMessage( context, NULL,
    "Fluid Resource compiled successfully",
    fluidCmd, 
    messagetypeInfo, 0 );	
  //CWReportMessage( context, NULL,
  //  hPath.path,
  //  cxxPath.path, 
  //  messagetypeInfo, 0 );	
  
	err = cwNoErr;
	
  // clean up 
bail:   

  if ( preFileAction )
  {
    FILETIME filetime;
    GetSystemTimeAsFileTime( &filetime );
    CWPostFileAction( context, &cxxPath );
    CWSetModDate( context, &cxxPath, &filetime, true );
    CWPostFileAction( context, &hPath );
    CWSetModDate( context, &hPath, &filetime, true );
  }

  if ( fluidCmd ) free( fluidCmd );
  if ( fluidCall ) free( fluidCall );
  if ( flPath ) free( flPath );
	
	return (err);
}


