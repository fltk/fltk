# Microsoft Developer Studio Project File - Name="fltkgldll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=fltkgldll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fltkgldll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fltkgldll.mak" CFG="fltkgldll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fltkgldll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "fltkgldll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fltkgldll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fltkgldll0"
# PROP BASE Intermediate_Dir "fltkgldll0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../test"
# PROP Intermediate_Dir "fltkgldll"
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
# ADD LINK32 opengl32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltkgldll.pdb" /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "fltkgldll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fltkgldll1"
# PROP BASE Intermediate_Dir "fltkgldll1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../test"
# PROP Intermediate_Dir "fltkgldlld"
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
# ADD LINK32 opengl32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltkgldlld.pdb" /debug /machine:I386 /out:"../../test/fltkgldlld.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "fltkgldll - Win32 Release"
# Name "fltkgldll - Win32 Debug"
# Begin Source File

SOURCE=..\..\src\Fl_Gl_Choice.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Gl_Overlay.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Gl_Window.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\freeglut_geometry.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\freeglut_stroke_mono_roman.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\freeglut_stroke_roman.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\freeglut_teapot.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\gl_draw.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\glut_compatability.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\glut_font.cxx
# End Source File
# End Target
# End Project
