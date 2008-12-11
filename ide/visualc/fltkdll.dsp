# Microsoft Developer Studio Project File - Name="fltkdll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=fltkdll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fltkdll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fltkdll.mak" CFG="fltkdll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fltkdll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "fltkdll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fltkdll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fltkdll0"
# PROP BASE Intermediate_Dir "fltkdll0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../test"
# PROP Intermediate_Dir "fltkdll"
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
# ADD LINK32 opengl32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltkdll.pdb" /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "fltkdll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fltkdll1"
# PROP BASE Intermediate_Dir "fltkdll1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../test"
# PROP Intermediate_Dir "fltkdlld"
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
# ADD LINK32 opengl32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltkdlld.pdb" /debug /machine:I386 /out:"../../test/fltkdlld.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "fltkdll - Win32 Release"
# Name "fltkdll - Win32 Debug"
# Begin Source File

SOURCE=..\..\src\xutf8\case.c
DEP_CPP_CASE_=\
	"..\..\src\xutf8\headers\case.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_absolute.cxx
DEP_CPP_FILEN=\
	"..\..\FL\filename.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FILEN=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_expand.cxx
DEP_CPP_FILENA=\
	"..\..\FL\filename.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FILENA=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_ext.cxx
DEP_CPP_FILENAM=\
	"..\..\FL\filename.h"\
	"..\..\FL\fl_export.h"\
	
NODEP_CPP_FILENAM=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_isdir.cxx
DEP_CPP_FILENAME=\
	"..\..\FL\filename.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FILENAME=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_list.cxx
DEP_CPP_FILENAME_=\
	"..\..\FL\filename.h"\
	"..\..\FL\fl_export.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FILENAME_=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_match.cxx
DEP_CPP_FILENAME_M=\
	"..\..\FL\filename.h"\
	"..\..\FL\fl_export.h"\
	
NODEP_CPP_FILENAME_M=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_setext.cxx
DEP_CPP_FILENAME_S=\
	"..\..\FL\filename.h"\
	"..\..\FL\fl_export.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FILENAME_S=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl.cxx
DEP_CPP_FL_CX=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\filename.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_bar.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\FL\Fl_Sys_Menu_Bar.H"\
	"..\..\FL\fl_tooltip.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\aimm.h"\
	"..\..\src\fl_font.h"\
	"..\..\src\Fl_mac.cxx"\
	"..\..\src\fl_win32.cxx"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_CX=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_abort.cxx
DEP_CPP_FL_AB=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_add_idle.cxx
DEP_CPP_FL_AD=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Adjuster.cxx
DEP_CPP_FL_ADJ=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_adjuster.h"\
	"..\..\FL\fl_bitmap.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\fastarrow.h"\
	"..\..\src\mediumarrow.h"\
	"..\..\src\slowarrow.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_arc.cxx
DEP_CPP_FL_AR=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\FL\math.h"\
	
NODEP_CPP_FL_AR=\
	"..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_arci.cxx
DEP_CPP_FL_ARC=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\math.h"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
NODEP_CPP_FL_ARC=\
	"..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_arg.cxx
DEP_CPP_FL_ARG=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\filename.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_tooltip.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_ARG=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_ask.cxx
DEP_CPP_FL_AS=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_ask.h"\
	"..\..\FL\fl_box.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_input.h"\
	"..\..\FL\fl_input_.h"\
	"..\..\FL\fl_return_button.h"\
	"..\..\FL\fl_secret_input.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Bitmap.cxx
DEP_CPP_FL_BI=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_bitmap.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_BMP_Image.cxx
DEP_CPP_FL_BM=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl_bmp_image.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\fl\fl_types.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Box.cxx
DEP_CPP_FL_BO=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl_box.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\FL\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_boxtype.cxx
DEP_CPP_FL_BOX=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Browser.cxx
DEP_CPP_FL_BR=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_browser.h"\
	"..\..\FL\fl_browser_.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Browser_.cxx
DEP_CPP_FL_BRO=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_browser_.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Browser_load.cxx
DEP_CPP_FL_BROW=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_browser.h"\
	"..\..\FL\fl_browser_.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Button.cxx
DEP_CPP_FL_BU=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Chart.cxx
DEP_CPP_FL_CH=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_chart.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\math.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_CH=\
	"..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Check_Browser.cxx
DEP_CPP_FL_CHE=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_browser_.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_check_browser.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Check_Button.cxx
DEP_CPP_FL_CHEC=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_check_button.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_light_button.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Choice.cxx
DEP_CPP_FL_CHO=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_choice.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Clock.cxx
DEP_CPP_FL_CL=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_clock.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_color.cxx
DEP_CPP_FL_CO=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\fl_cmap.h"\
	"..\..\src\fl_color_mac.cxx"\
	"..\..\src\fl_color_win32.cxx"\
	"..\..\src\Fl_XColor.H"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Color_Chooser.cxx
DEP_CPP_FL_COL=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_box.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_choice.h"\
	"..\..\FL\fl_color_chooser.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_input.h"\
	"..\..\FL\fl_input_.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\FL\fl_return_button.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_value_input.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\math.h"\
	"..\..\FL\Xutf8.h"\
	
NODEP_CPP_FL_COL=\
	"..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_compose.cxx
DEP_CPP_FL_COM=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Counter.cxx
DEP_CPP_FL_COU=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_counter.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_cursor.cxx
DEP_CPP_FL_CU=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_curve.cxx
DEP_CPP_FL_CUR=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Dial.cxx
DEP_CPP_FL_DI=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_dial.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\math.h"\
	"..\..\FL\Xutf8.h"\
	
NODEP_CPP_FL_DI=\
	"..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_diamond_box.cxx
DEP_CPP_FL_DIA=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_display.cxx
DEP_CPP_FL_DIS=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_dnd.cxx
DEP_CPP_FL_DN=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\fl_dnd_mac.cxx"\
	"..\..\src\fl_dnd_win32.cxx"\
	"..\..\src\fl_dnd_x.cxx"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Double_Window.cxx
DEP_CPP_FL_DO=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_double_window.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_draw.cxx
DEP_CPP_FL_DR=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_draw_image.cxx
DEP_CPP_FL_DRA=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\fl_draw_image_mac.cxx"\
	"..\..\src\fl_draw_image_win32.cxx"\
	"..\..\src\Fl_XColor.H"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_draw_pixmap.cxx
DEP_CPP_FL_DRAW=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_encoding_latin1.cxx
DEP_CPP_FL_EN=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_encoding_mac_roman.cxx
DEP_CPP_FL_ENC=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_engraved_label.cxx
DEP_CPP_FL_ENG=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_File_Browser.cxx
DEP_CPP_FL_FI=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\filename.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_browser.h"\
	"..\..\FL\fl_browser_.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_File_Browser.H"\
	"..\..\FL\fl_file_icon.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_FI=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_File_Chooser.cxx
DEP_CPP_FL_FIL=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\filename.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_ask.h"\
	"..\..\FL\fl_bitmap.h"\
	"..\..\FL\fl_box.h"\
	"..\..\FL\fl_browser.h"\
	"..\..\FL\fl_browser_.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_check_button.h"\
	"..\..\FL\fl_choice.h"\
	"..\..\FL\fl_double_window.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_File_Browser.H"\
	"..\..\FL\fl_file_chooser.h"\
	"..\..\FL\fl_file_icon.h"\
	"..\..\FL\fl_file_input.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_input.h"\
	"..\..\FL\fl_input_.h"\
	"..\..\FL\fl_light_button.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_button.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\FL\fl_preferences.h"\
	"..\..\FL\fl_return_button.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\FL\fl_tile.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\Xutf8.h"\
	
NODEP_CPP_FL_FIL=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_File_Chooser2.cxx
DEP_CPP_FL_FILE=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\filename.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_ask.h"\
	"..\..\FL\fl_box.h"\
	"..\..\FL\fl_browser.h"\
	"..\..\FL\fl_browser_.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_check_button.h"\
	"..\..\FL\fl_choice.h"\
	"..\..\FL\fl_double_window.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_File_Browser.H"\
	"..\..\FL\fl_file_chooser.h"\
	"..\..\FL\fl_file_icon.h"\
	"..\..\FL\fl_file_input.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_input.h"\
	"..\..\FL\fl_input_.h"\
	"..\..\FL\fl_light_button.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_button.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\FL\fl_preferences.h"\
	"..\..\FL\fl_return_button.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_shared_image.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\FL\fl_tile.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_FILE=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_file_dir.cxx
DEP_CPP_FL_FILE_=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\filename.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_ask.h"\
	"..\..\FL\fl_box.h"\
	"..\..\FL\fl_browser.h"\
	"..\..\FL\fl_browser_.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_check_button.h"\
	"..\..\FL\fl_choice.h"\
	"..\..\FL\fl_double_window.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_File_Browser.H"\
	"..\..\FL\fl_file_chooser.h"\
	"..\..\FL\fl_file_icon.h"\
	"..\..\FL\fl_file_input.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_input.h"\
	"..\..\FL\fl_input_.h"\
	"..\..\FL\fl_light_button.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_button.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\FL\fl_preferences.h"\
	"..\..\FL\fl_return_button.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\FL\fl_tile.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_FILE_=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_File_Icon.cxx
DEP_CPP_FL_FILE_I=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\filename.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_file_icon.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_FILE_I=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_File_Icon2.cxx
DEP_CPP_FL_FILE_IC=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\filename.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_file_icon.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_shared_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\math.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_FILE_IC=\
	"..\..\..\..\usr\include\dirent.h"\
	"..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_File_Input.cxx
DEP_CPP_FL_FILE_IN=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_file_input.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_input.h"\
	"..\..\FL\fl_input_.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_font.cxx
DEP_CPP_FL_FO=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\fl_font.h"\
	"..\..\src\fl_font_mac.cxx"\
	"..\..\src\fl_font_win32.cxx"\
	"..\..\src\fl_font_x.cxx"\
	"..\..\src\fl_font_xft.cxx"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_get_key.cxx
DEP_CPP_FL_GE=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\Fl_get_key_mac.cxx"\
	"..\..\src\fl_get_key_win32.cxx"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_get_system_colors.cxx
DEP_CPP_FL_GET=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_pixmap.h"\
	"..\..\FL\fl_tiled_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\math.h"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	"..\..\src\tile.xpm"\
	".\config.h"\
	
NODEP_CPP_FL_GET=\
	"..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_GIF_Image.cxx
DEP_CPP_FL_GI=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_gif_image.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_pixmap.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Gl_Choice.cxx
DEP_CPP_FL_GL=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\gl.h"\
	"..\..\FL\gl_draw.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\Fl_Gl_Choice.H"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Gl_Overlay.cxx
DEP_CPP_FL_GL_=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_Gl_Window.H"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\gl.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\Fl_Gl_Choice.H"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Gl_Window.cxx
DEP_CPP_FL_GL_W=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_Gl_Window.H"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\gl.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\Fl_Gl_Choice.H"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_grab.cxx
DEP_CPP_FL_GR=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Group.cxx
DEP_CPP_FL_GRO=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_gtk.cxx
DEP_CPP_FL_GT=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Help_Dialog.cxx
DEP_CPP_FL_HE=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_ask.h"\
	"..\..\FL\fl_box.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_double_window.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\Fl_Help_Dialog.H"\
	"..\..\FL\Fl_Help_View.H"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_input.h"\
	"..\..\FL\fl_input_.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_shared_image.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Help_View.cxx
DEP_CPP_FL_HEL=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\Fl_Help_View.H"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_pixmap.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_shared_image.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Image.cxx
DEP_CPP_FL_IM=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_images_core.cxx
DEP_CPP_FL_IMA=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl_bmp_image.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_gif_image.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_jpeg_image.h"\
	"..\..\FL\fl_pixmap.h"\
	"..\..\FL\fl_png_image.h"\
	"..\..\FL\fl_pnm_image.h"\
	"..\..\FL\fl_shared_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Input.cxx
DEP_CPP_FL_IN=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_ask.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_input.h"\
	"..\..\FL\fl_input_.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Input_.cxx
DEP_CPP_FL_INP=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_ask.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_input_.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_JPEG_Image.cxx
DEP_CPP_FL_JP=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_jpeg_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_labeltype.cxx
DEP_CPP_FL_LA=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_input_.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Light_Button.cxx
DEP_CPP_FL_LI=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_light_button.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_line_style.cxx
DEP_CPP_FL_LIN=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_lock.cxx
DEP_CPP_FL_LO=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu.cxx
DEP_CPP_FL_ME=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\FL\fl_menu_window.h"\
	"..\..\FL\fl_single_window.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu_.cxx
DEP_CPP_FL_MEN=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu_add.cxx
DEP_CPP_FL_MENU=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\fl\fl_types.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu_Bar.cxx
DEP_CPP_FL_MENU_=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_bar.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu_Button.cxx
DEP_CPP_FL_MENU_B=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_button.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu_global.cxx
DEP_CPP_FL_MENU_G=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu_Window.cxx
DEP_CPP_FL_MENU_W=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_menu_window.h"\
	"..\..\FL\fl_single_window.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Multi_Label.cxx
DEP_CPP_FL_MU=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\FL\fl_multi_label.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_open_uri.cxx
DEP_CPP_FL_OP=\
	"..\..\FL\filename.h"\
	"..\..\FL\fl_export.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_OP=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_oval_box.cxx
DEP_CPP_FL_OV=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_overlay.cxx
DEP_CPP_FL_OVE=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_overlay_visual.cxx
DEP_CPP_FL_OVER=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Overlay_Window.cxx
DEP_CPP_FL_OVERL=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_double_window.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_overlay_window.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_own_colormap.cxx
DEP_CPP_FL_OW=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Pack.cxx
DEP_CPP_FL_PA=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_pack.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Pixmap.cxx
DEP_CPP_FL_PI=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\FL\fl_pixmap.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_plastic.cxx
DEP_CPP_FL_PL=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_PNG_Image.cxx
DEP_CPP_FL_PN=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_png_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\png\png.h"\
	"..\..\png\pngconf.h"\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	".\config.h"\
	
NODEP_CPP_FL_PN=\
	"..\..\png\pngusr.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_PNM_Image.cxx
DEP_CPP_FL_PNM=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_pnm_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Positioner.cxx
DEP_CPP_FL_PO=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_positioner.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Preferences.cxx
DEP_CPP_FL_PR=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\filename.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_preferences.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_PR=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Progress.cxx
DEP_CPP_FL_PRO=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_progress.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_read_image.cxx
DEP_CPP_FL_RE=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\fl_read_image_mac.cxx"\
	"..\..\src\fl_read_image_win32.cxx"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_rect.cxx
DEP_CPP_FL_REC=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Repeat_Button.cxx
DEP_CPP_FL_REP=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_repeat_button.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Return_Button.cxx
DEP_CPP_FL_RET=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_return_button.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Roller.cxx
DEP_CPP_FL_RO=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_roller.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_round_box.cxx
DEP_CPP_FL_ROU=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Round_Button.cxx
DEP_CPP_FL_ROUN=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_light_button.h"\
	"..\..\FL\fl_round_button.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_rounded_box.cxx
DEP_CPP_FL_ROUND=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Scroll.cxx
DEP_CPP_FL_SC=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_scroll.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\FL\fl_tiled_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_scroll_area.cxx
DEP_CPP_FL_SCR=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Scrollbar.cxx
DEP_CPP_FL_SCRO=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_set_font.cxx
DEP_CPP_FL_SE=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\fl_font.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_set_fonts.cxx
DEP_CPP_FL_SET=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\fl_font.h"\
	"..\..\src\fl_set_fonts_mac.cxx"\
	"..\..\src\fl_set_fonts_win32.cxx"\
	"..\..\src\fl_set_fonts_x.cxx"\
	"..\..\src\fl_set_fonts_xft.cxx"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_shadow_box.cxx
DEP_CPP_FL_SH=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Shared_Image.cxx
DEP_CPP_FL_SHA=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_bitmap.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_pixmap.h"\
	"..\..\FL\fl_shared_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_xbm_image.h"\
	"..\..\FL\fl_xpm_image.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_shortcut.cxx
DEP_CPP_FL_SHO=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_show_colormap.cxx
DEP_CPP_FL_SHOW=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_show_colormap.h"\
	"..\..\FL\fl_single_window.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Single_Window.cxx
DEP_CPP_FL_SI=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_single_window.h"\
	"..\..\fl\fl_types.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Slider.cxx
DEP_CPP_FL_SL=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_symbols.cxx
DEP_CPP_FL_SY=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\math.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_SY=\
	"..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Tabs.cxx
DEP_CPP_FL_TA=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_tabs.h"\
	"..\..\FL\fl_tooltip.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Text_Buffer.cxx
DEP_CPP_FL_TE=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_text_buffer.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Text_Display.cxx
DEP_CPP_FL_TEX=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\FL\fl_text_buffer.h"\
	"..\..\FL\fl_text_display.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Text_Editor.cxx
DEP_CPP_FL_TEXT=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_ask.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\FL\fl_text_buffer.h"\
	"..\..\FL\fl_text_display.h"\
	"..\..\FL\fl_text_editor.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Tile.cxx
DEP_CPP_FL_TI=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_tile.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Tiled_Image.cxx
DEP_CPP_FL_TIL=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_tiled_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Tooltip.cxx
DEP_CPP_FL_TO=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_menu_window.h"\
	"..\..\FL\fl_single_window.h"\
	"..\..\FL\fl_tooltip.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_utf.c
DEP_CPP_FL_UT=\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_utf8.cxx
DEP_CPP_FL_UTF=\
	"..\..\FL\filename.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
NODEP_CPP_FL_UTF=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Valuator.cxx
DEP_CPP_FL_VA=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\math.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_VA=\
	"..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Value_Input.cxx
DEP_CPP_FL_VAL=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_input.h"\
	"..\..\FL\fl_input_.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_value_input.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\math.h"\
	"..\..\FL\Xutf8.h"\
	
NODEP_CPP_FL_VAL=\
	"..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Value_Output.cxx
DEP_CPP_FL_VALU=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_value_output.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Value_Slider.cxx
DEP_CPP_FL_VALUE=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_value_slider.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_vertex.cxx
DEP_CPP_FL_VE=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\math.h"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
NODEP_CPP_FL_VE=\
	"..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_visual.cxx
DEP_CPP_FL_VI=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Widget.cxx
DEP_CPP_FL_WI=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_tooltip.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Window.cxx
DEP_CPP_FL_WIN=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Window_fullscreen.cxx
DEP_CPP_FL_WIND=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Window_hotspot.cxx
DEP_CPP_FL_WINDO=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Window_iconize.cxx
DEP_CPP_FL_WINDOW=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Wizard.cxx
DEP_CPP_FL_WIZ=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\fl_wizard.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_x.cxx
DEP_CPP_FL_X_=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_tooltip.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_XBM_Image.cxx
DEP_CPP_FL_XB=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_bitmap.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_xbm_image.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_XPM_Image.cxx
DEP_CPP_FL_XP=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_pixmap.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_xpm_image.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\flstring.c
DEP_CPP_FLSTR=\
	"..\..\FL\fl_export.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\forms_bitmap.cxx
DEP_CPP_FORMS=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\filename.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_ask.h"\
	"..\..\FL\fl_bitmap.h"\
	"..\..\FL\fl_box.h"\
	"..\..\FL\fl_browser.h"\
	"..\..\FL\fl_browser_.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_chart.h"\
	"..\..\FL\fl_check_button.h"\
	"..\..\FL\fl_choice.h"\
	"..\..\FL\fl_clock.h"\
	"..\..\FL\fl_counter.h"\
	"..\..\FL\fl_dial.h"\
	"..\..\FL\fl_double_window.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_File_Browser.H"\
	"..\..\FL\fl_file_chooser.h"\
	"..\..\FL\fl_file_icon.h"\
	"..\..\FL\fl_file_input.h"\
	"..\..\FL\Fl_FormsBitmap.H"\
	"..\..\FL\Fl_FormsPixmap.H"\
	"..\..\FL\Fl_Free.H"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_input.h"\
	"..\..\FL\fl_input_.h"\
	"..\..\FL\fl_light_button.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_button.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\FL\fl_pixmap.h"\
	"..\..\FL\fl_positioner.h"\
	"..\..\FL\fl_preferences.h"\
	"..\..\FL\fl_return_button.h"\
	"..\..\FL\fl_round_button.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_show_colormap.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\FL\fl_tile.h"\
	"..\..\FL\Fl_Timer.H"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_value_slider.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\forms.h"\
	"..\..\FL\Xutf8.h"\
	
NODEP_CPP_FORMS=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\forms_compatability.cxx
DEP_CPP_FORMS_=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\filename.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_ask.h"\
	"..\..\FL\fl_bitmap.h"\
	"..\..\FL\fl_box.h"\
	"..\..\FL\fl_browser.h"\
	"..\..\FL\fl_browser_.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_chart.h"\
	"..\..\FL\fl_check_button.h"\
	"..\..\FL\fl_choice.h"\
	"..\..\FL\fl_clock.h"\
	"..\..\FL\fl_counter.h"\
	"..\..\FL\fl_dial.h"\
	"..\..\FL\fl_double_window.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_File_Browser.H"\
	"..\..\FL\fl_file_chooser.h"\
	"..\..\FL\fl_file_icon.h"\
	"..\..\FL\fl_file_input.h"\
	"..\..\FL\Fl_FormsBitmap.H"\
	"..\..\FL\Fl_FormsPixmap.H"\
	"..\..\FL\Fl_Free.H"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_input.h"\
	"..\..\FL\fl_input_.h"\
	"..\..\FL\fl_light_button.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_button.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\FL\fl_pixmap.h"\
	"..\..\FL\fl_positioner.h"\
	"..\..\FL\fl_preferences.h"\
	"..\..\FL\fl_repeat_button.h"\
	"..\..\FL\fl_return_button.h"\
	"..\..\FL\fl_round_button.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_show_colormap.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\FL\fl_tile.h"\
	"..\..\FL\Fl_Timer.H"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_value_slider.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\forms.h"\
	"..\..\FL\Xutf8.h"\
	
NODEP_CPP_FORMS_=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\forms_free.cxx
DEP_CPP_FORMS_F=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_Free.H"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\forms_fselect.cxx
DEP_CPP_FORMS_FS=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\filename.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_ask.h"\
	"..\..\FL\fl_bitmap.h"\
	"..\..\FL\fl_box.h"\
	"..\..\FL\fl_browser.h"\
	"..\..\FL\fl_browser_.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_chart.h"\
	"..\..\FL\fl_check_button.h"\
	"..\..\FL\fl_choice.h"\
	"..\..\FL\fl_clock.h"\
	"..\..\FL\fl_counter.h"\
	"..\..\FL\fl_dial.h"\
	"..\..\FL\fl_double_window.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_File_Browser.H"\
	"..\..\FL\fl_file_chooser.h"\
	"..\..\FL\fl_file_icon.h"\
	"..\..\FL\fl_file_input.h"\
	"..\..\FL\Fl_FormsBitmap.H"\
	"..\..\FL\Fl_FormsPixmap.H"\
	"..\..\FL\Fl_Free.H"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_input.h"\
	"..\..\FL\fl_input_.h"\
	"..\..\FL\fl_light_button.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_button.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\FL\fl_pixmap.h"\
	"..\..\FL\fl_positioner.h"\
	"..\..\FL\fl_preferences.h"\
	"..\..\FL\fl_return_button.h"\
	"..\..\FL\fl_round_button.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_show_colormap.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\FL\fl_tile.h"\
	"..\..\FL\Fl_Timer.H"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_value_slider.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\forms.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FORMS_FS=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\forms_pixmap.cxx
DEP_CPP_FORMS_P=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\filename.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\fl_ask.h"\
	"..\..\FL\fl_bitmap.h"\
	"..\..\FL\fl_box.h"\
	"..\..\FL\fl_browser.h"\
	"..\..\FL\fl_browser_.h"\
	"..\..\FL\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_chart.h"\
	"..\..\FL\fl_check_button.h"\
	"..\..\FL\fl_choice.h"\
	"..\..\FL\fl_clock.h"\
	"..\..\FL\fl_counter.h"\
	"..\..\FL\fl_dial.h"\
	"..\..\FL\fl_double_window.h"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_File_Browser.H"\
	"..\..\FL\fl_file_chooser.h"\
	"..\..\FL\fl_file_icon.h"\
	"..\..\FL\fl_file_input.h"\
	"..\..\FL\Fl_FormsBitmap.H"\
	"..\..\FL\Fl_FormsPixmap.H"\
	"..\..\FL\Fl_Free.H"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_input.h"\
	"..\..\FL\fl_input_.h"\
	"..\..\FL\fl_light_button.h"\
	"..\..\FL\fl_menu_.h"\
	"..\..\FL\fl_menu_button.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\FL\fl_pixmap.h"\
	"..\..\FL\fl_positioner.h"\
	"..\..\FL\fl_preferences.h"\
	"..\..\FL\fl_return_button.h"\
	"..\..\FL\fl_round_button.h"\
	"..\..\FL\fl_scrollbar.h"\
	"..\..\FL\fl_show_colormap.h"\
	"..\..\FL\fl_slider.h"\
	"..\..\FL\fl_tile.h"\
	"..\..\FL\Fl_Timer.H"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_valuator.h"\
	"..\..\FL\fl_value_slider.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\forms.h"\
	"..\..\FL\Xutf8.h"\
	
NODEP_CPP_FORMS_P=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\forms_timer.cxx
DEP_CPP_FORMS_T=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_Timer.H"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\freeglut_geometry.cxx
DEP_CPP_FREEG=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_Gl_Window.H"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\glut.h"\
	"..\..\FL\math.h"\
	"..\..\FL\Xutf8.h"\
	
NODEP_CPP_FREEG=\
	"..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\freeglut_stroke_mono_roman.cxx
DEP_CPP_FREEGL=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_Gl_Window.H"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\glut.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\freeglut_stroke_roman.cxx
DEP_CPP_FREEGLU=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_Gl_Window.H"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\glut.h"\
	"..\..\FL\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\freeglut_teapot.cxx
DEP_CPP_FREEGLUT=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_Gl_Window.H"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\glut.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\freeglut_teapot_data.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\gl_draw.cxx
DEP_CPP_GL_DR=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\gl.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\fl_font.h"\
	"..\..\src\Fl_Gl_Choice.H"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\gl_start.cxx
DEP_CPP_GL_ST=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_draw.h"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\gl.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\Fl_Gl_Choice.H"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\glut_compatability.cxx
DEP_CPP_GLUT_=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_Gl_Window.H"\
	"..\..\FL\fl_group.h"\
	"..\..\FL\fl_image.h"\
	"..\..\FL\fl_menu_item.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\glut.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\glut_font.cxx
DEP_CPP_GLUT_F=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\Fl_Gl_Window.H"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\glut.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\xutf8\is_right2left.c
# End Source File
# Begin Source File

SOURCE=..\..\src\xutf8\is_spacing.c
DEP_CPP_IS_SP=\
	"..\..\src\xutf8\headers\spacing.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\numericsort.c
DEP_CPP_NUMER=\
	"..\..\FL\filename.h"\
	"..\..\FL\fl_export.h"\
	".\config.h"\
	
NODEP_CPP_NUMER=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\scandir.c
DEP_CPP_SCAND=\
	"..\..\FL\filename.h"\
	"..\..\FL\fl_export.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\Xutf8.h"\
	"..\..\src\flstring.h"\
	"..\..\src\scandir_win32.c"\
	".\config.h"\
	
NODEP_CPP_SCAND=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\screen_xywh.cxx
DEP_CPP_SCREE=\
	"..\..\FL\enumerations.h"\
	"..\..\FL\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\FL\fl_export.h"\
	"..\..\FL\fl_group.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\FL\fl_widget.h"\
	"..\..\FL\fl_window.h"\
	"..\..\FL\mac.H"\
	"..\..\FL\win32.h"\
	"..\..\FL\x.h"\
	"..\..\FL\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\vsnprintf.c
DEP_CPP_VSNPR=\
	"..\..\FL\fl_export.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# End Target
# End Project
