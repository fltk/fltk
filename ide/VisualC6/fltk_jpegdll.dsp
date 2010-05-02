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
# PROP BASE Output_Dir "Release/$(ProjectName)"
# PROP BASE Intermediate_Dir "Release/$(ProjectName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release/$(ProjectName)"
# PROP Intermediate_Dir "Release/$(ProjectName)"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Os /Ob2 /I "../../zlib" /I "../../png" /I "../../jpeg" /I "." /I "../.." /D "FL_DLL" /D "FL_LIBRARY" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /c
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
# PROP BASE Output_Dir "Debug/$(ProjectName)"
# PROP BASE Intermediate_Dir "Debug/$(ProjectName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug/$(ProjectName)"
# PROP Intermediate_Dir "Debug/$(ProjectName)"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /GX /ZI /Od /I "." /I "../.." /I "..\..\zlib" /I "..\..\png" /I "..\..\jpeg" /D "FL_DLL" /D "FL_LIBRARY" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltk_jpegdlld.pdb" /debug /machine:I386 /out:"../../test/fltk_jpegdlld.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "fltk_jpegdll - Win32 Release"
# Name "fltk_jpegdll - Win32 Debug"
# Begin Source File

SOURCE=..\..\jpeg\jcapimin.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcapistd.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jccoefct.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jccolor.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcdctmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jchuff.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcinit.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcmainct.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcmarker.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcmaster.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcomapi.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcparam.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcphuff.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcprepct.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcsample.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jctrans.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdapimin.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdapistd.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdatadst.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdatasrc.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdcoefct.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdcolor.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jddctmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdhuff.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdinput.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdmainct.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdmarker.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdmaster.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdmerge.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdphuff.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdpostct.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdsample.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jdtrans.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jerror.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jfdctflt.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jfdctfst.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jfdctint.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jidctflt.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jidctfst.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jidctint.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jidctred.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jmemmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jmemnobs.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jquant1.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jquant2.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jutils.c
# End Source File
# End Target
# End Project
