# Microsoft Developer Studio Project File - Name="fluid" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
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
# PROP AllowPerConfigDependencies 0
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
# ADD CPP /nologo /MD /GX /Ot /Op /Ob2 /I "." /I ".." /I "../png" /I "../zlib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /FD /c
# SUBTRACT CPP /Os
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 wsock32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libcd" /nodefaultlib:"libcmt" /out:"../fluid/fluid.exe" /libpath:"..\lib"
# SUBTRACT LINK32 /pdb:none /incremental:yes

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
# ADD CPP /nologo /MDd /Gm /GX /ZI /Od /I "." /I ".." /I "../png" /I "../zlib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 fltkd.lib wsock32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /nodefaultlib:"libcmt" /nodefaultlib:"libcmtd" /out:"../fluid/fluidd.exe" /pdbtype:sept /libpath:"..\lib"
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "fluid - Win32 Release"
# Name "fluid - Win32 Debug"
# Begin Source File

SOURCE=..\fluid\about_panel.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\align_widget.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\alignment_panel.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\code.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\CodeEditor.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\factory.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\file.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\Fl_Function_Type.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\Fl_Group_Type.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\Fl_Menu_Type.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\Fl_Type.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\Fl_Widget_Type.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\Fl_Window_Type.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\fluid.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\Fluid_Image.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\function_panel.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\template_panel.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\undo.cxx
# End Source File
# Begin Source File

SOURCE=..\fluid\widget_panel.cxx
# End Source File
# End Target
# End Project
