# Microsoft Developer Studio Project File - Name="fltk_jpegdll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=fltk_jpegdll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fltk_jpegdll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fltk_jpegdll.mak" CFG="fltk_jpegdll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fltk_jpegdll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "fltk_jpegdll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fltk_jpegdll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release/fltk_jpegdll"
# PROP BASE Intermediate_Dir "Release/fltk_jpegdll"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release/fltk_jpegdll"
# PROP Intermediate_Dir "Release/fltk_jpegdll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Os /Ob2 /I "../../zlib" /I "../../png" /I "../../jpeg" /I "." /I "../.." /D "FL_DLL" /D "FL_LIBRARY" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 opengl32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltk_jpegdll.pdb" /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "fltk_jpegdll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug/fltk_jpegdll"
# PROP BASE Intermediate_Dir "Debug/fltk_jpegdll"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug/fltk_jpegdll"
# PROP Intermediate_Dir "Debug/fltk_jpegdll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /GX /ZI /Od /I "." /I "../.." /I "..\..\zlib" /I "..\..\png" /I "..\..\jpeg" /D "FL_DLL" /D "FL_LIBRARY" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltk_jpegdlld.pdb" /debug /machine:I386 /out:"Debug/fltk_jpegdll/fltk_jpegdlld.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "fltk_jpegdll - Win32 Release"
# Name "fltk_jpegdll - Win32 Debug"
# Begin Source File

SOURCE=..\..\jpeg\jaricom.c
DEP_CPP_JCPHU=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcapimin.c
DEP_CPP_JCAPI=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcapistd.c
DEP_CPP_JCAPIS=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcarith.c
DEP_CPP_JCPHU=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jccoefct.c
DEP_CPP_JCCOE=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jccolor.c
DEP_CPP_JCCOL=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcdctmgr.c
DEP_CPP_JCDCT=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jdct.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jchuff.c
DEP_CPP_JCHUF=\
	"..\..\jpeg\jchuff.h"\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcinit.c
DEP_CPP_JCINI=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcmainct.c
DEP_CPP_JCMAI=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcmarker.c
DEP_CPP_JCMAR=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcmaster.c
DEP_CPP_JCMAS=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcomapi.c
DEP_CPP_JCOMA=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcparam.c
DEP_CPP_JCPAR=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcprepct.c
DEP_CPP_JCPRE=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcsample.c
DEP_CPP_JCSAM=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jctrans.c
DEP_CPP_JCTRA=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdapimin.c
DEP_CPP_JDAPI=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdapistd.c
DEP_CPP_JDAPIS=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdarith.c
DEP_CPP_JCPHU=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdatadst.c
DEP_CPP_JDATA=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdatasrc.c
DEP_CPP_JDATAS=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdcoefct.c
DEP_CPP_JDCOE=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdcolor.c
DEP_CPP_JDCOL=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jddctmgr.c
DEP_CPP_JDDCT=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jdct.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdhuff.c
DEP_CPP_JDHUF=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jdhuff.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdinput.c
DEP_CPP_JDINP=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdmainct.c
DEP_CPP_JDMAI=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdmarker.c
DEP_CPP_JDMAR=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdmaster.c
DEP_CPP_JDMAS=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdmerge.c
DEP_CPP_JDMER=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdpostct.c
DEP_CPP_JDPOS=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdsample.c
DEP_CPP_JDSAM=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdtrans.c
DEP_CPP_JDTRA=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jerror.c
DEP_CPP_JERRO=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	"..\..\jpeg\jversion.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jfdctflt.c
DEP_CPP_JFDCT=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jdct.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jfdctfst.c
DEP_CPP_JFDCTF=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jdct.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jfdctint.c
DEP_CPP_JFDCTI=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jdct.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jidctflt.c
DEP_CPP_JIDCT=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jdct.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jidctfst.c
DEP_CPP_JIDCTF=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jdct.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jidctint.c
DEP_CPP_JIDCTI=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jdct.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jmemmgr.c
DEP_CPP_JMEMM=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmemsys.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jmemnobs.c
DEP_CPP_JMEMN=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmemsys.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jquant1.c
DEP_CPP_JQUAN=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jquant2.c
DEP_CPP_JQUANT=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jutils.c
DEP_CPP_JUTIL=\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jinclude.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	
# End Source File
# End Target
# End Project
