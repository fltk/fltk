# Microsoft Developer Studio Project File - Name="fluid" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=fluid - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fluid.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fluid.mak" CFG="fluid - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fluid - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "fluid - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fluid - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fluid___"
# PROP BASE Intermediate_Dir "fluid___"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "fluid___"
# PROP Intermediate_Dir "fluid___"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Os /Ob2 /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /TP /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ..\lib\fltk.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /incremental:yes /machine:I386 /nodefaultlib:"libcd" /out:"../fluid/fluid.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "fluid - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fluid__0"
# PROP BASE Intermediate_Dir "fluid__0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "fluid__0"
# PROP Intermediate_Dir "fluid__0"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I ".." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /TP /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\lib\fltkd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd" /out:"../fluid/fluid.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "fluid - Win32 Release"
# Name "fluid - Win32 Debug"
# Begin Source File

SOURCE=..\fluid\about_panel.C
# End Source File
# Begin Source File

SOURCE=..\fluid\alignment_panel.C
# End Source File
# Begin Source File

SOURCE=..\fluid\code.C
# End Source File
# Begin Source File

SOURCE=..\fluid\factory.C
# End Source File
# Begin Source File

SOURCE=..\fluid\file.C
# End Source File
# Begin Source File

SOURCE=..\fluid\Fl_Function_Type.C
# End Source File
# Begin Source File

SOURCE=..\fluid\Fl_Group_Type.C
# End Source File
# Begin Source File

SOURCE=..\fluid\Fl_Menu_Type.C
# End Source File
# Begin Source File

SOURCE=..\fluid\Fl_Type.C
# End Source File
# Begin Source File

SOURCE=..\fluid\Fl_Widget_Type.C
# End Source File
# Begin Source File

SOURCE=..\fluid\Fl_Window_Type.C
# End Source File
# Begin Source File

SOURCE=..\fluid\fluid.C
# End Source File
# Begin Source File

SOURCE=..\fluid\Fluid_Image.C
# End Source File
# Begin Source File

SOURCE=..\fluid\function_panel.C
# End Source File
# Begin Source File

SOURCE=..\fluid\gif.C
# End Source File
# Begin Source File

SOURCE=..\fluid\widget_panel.C
# End Source File
# End Target
# End Project
