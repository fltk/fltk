/*
 *  FluidPanel.h - 'Fluid Panel' Drop-In Preferences
 *
 *  Copyright © 1995-2002 Metrowerks Inc.  All rights reserved.
 *
 */

#ifndef _H_FluidPanel
#define _H_FluidPanel

#ifndef __TYPES__
#	include <Types.h>
#endif


#pragma options align=mac68k


/* this is the name of the panel, as shown in the Finder */
#define kFluidPanelName	"Fluid Panel"


/*
 *	AppleScript dictionary info.  As a rule of thumb, dropin panels should use the 
 *	same terminology and numeric code in their 'aete' that the IDE uses if there 
 *	is already a similar item in the IDE's 'aete'.  That is the case here, so we 
 *	merely duplicate applicable 68K Project and 68K Linker user terms below.
 */

enum {
/*	Symbolic Name				   Code		AETE Terminology		*/
	class_Fluid				= 'Flid',

	prefsPR_ProjectType			= 'PR01',	/* Project Type			*/
	prefsPR_FileName			= 'PR02',	/* File Name			*/
	prefsLN_GenerateSymFile		= 'LN02',	/* Generate SYM File	*/
	
	/* enumeration for project type */
	enumeration_ProjectType		= 'PRPT',
	enum_Project_Application	= 'PRPA',	/* application			*/
	enum_Project_Library		= 'PRPL',	/* library				*/
	enum_Project_SharedLibrary	= 'PRPS',	/* shared library		*/
	enum_Project_CodeResource	= 'PRPC',	/* code resource		*/
	enum_Project_MPWTool		= 'PRPM'	/* MPW tool				*/
};


/* internal codes for project type */
enum { 
	kProjTypeApplication,
	kProjTypeLibrary,
	kProjTypeSharedLib,
	kProjTypeCodeResource,
	kProjTypeMPWTool
};


/*	This is the structure that is manipulated by the panel.  The Fluid 
 *	compiler & linker both "know" about this structure.
 */

typedef struct FluidPref {
	short	version;			/* version # of prefs data	*/
	short	projtype;			/* project type				*/
	char	outfile[32];		/* output file name			*/
	Boolean	linksym;			/* generate SYM file		*/
	short	dotx;				/* position of dot (!)		*/
	short	doty;
} FluidPref, **FluidPrefHandle;

#pragma options align=reset
#endif	/* _H_FluidPanel */
