# Microsoft Developer Studio Project File - Name="fltk_jpeg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=fltk_jpeg - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fltk_jpeg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fltk_jpeg.mak" CFG="fltk_jpeg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fltk_jpeg - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "fltk_jpeg - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fltk_jpeg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release/fltk_jpeg"
# PROP BASE Intermediate_Dir "Release/fltk_jpeg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release/fltk_jpeg"
# PROP Intermediate_Dir "Release/fltk_jpeg"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /GX /Ot /Op /Ob2 /I "." /I "../.." /I "../../zlib" /I "../../png" /I "../../jpeg" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /FD /c
# SUBTRACT CPP /Os
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\fltk_jpeg.lib"

!ELSEIF  "$(CFG)" == "fltk_jpeg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug/fltk_jpeg"
# PROP BASE Intermediate_Dir "Debug/fltk_jpeg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug/fltk_jpeg"
# PROP Intermediate_Dir "Debug/fltk_jpeg"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /GX /Z7 /Od /I "." /I "../.." /I "../../zlib" /I "../../png" /I "../../jpeg" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\fltk_jpegd.lib"

!ENDIF 

# Begin Target

# Name "fltk_jpeg - Win32 Release"
# Name "fltk_jpeg - Win32 Debug"
# Begin Source File

SOURCE=..\..\jpeg\jcapimin.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcapistd.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jaricom.c
# End Source File
# Begin Source File

SOURCE=..\..\jpeg\jcarith.c
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

SOURCE=..\..\jpeg\jdarith.c
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
