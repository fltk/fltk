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
# PROP Output_Dir "../test"
# PROP Intermediate_Dir "fltkdll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Os /Ob2 /I "." /I ".." /I "..\zlib" /I "..\png" /I "..\jpeg" /D "FL_DLL" /D "FL_LIBRARY" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 opengl32.lib wsock32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltkdll.pdb" /machine:I386 /out:"fltkdll.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "fltkdll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fltkdll1"
# PROP BASE Intermediate_Dir "fltkdll1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../test"
# PROP Intermediate_Dir "fltkdlld"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /GX /ZI /Od /I "." /I ".." /I "..\zlib" /I "..\png" /I "..\jpeg" /D "FL_DLL" /D "FL_LIBRARY" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib wsock32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltkdlld.pdb" /debug /machine:I386 /out:"fltkdlld.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "fltkdll - Win32 Release"
# Name "fltkdll - Win32 Debug"
# Begin Source File

SOURCE=..\src\filename_absolute.cxx
DEP_CPP_FILEN=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
NODEP_CPP_FILEN=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\filename_expand.cxx
DEP_CPP_FILENA=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
NODEP_CPP_FILENA=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\filename_ext.cxx
DEP_CPP_FILENAM=\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
NODEP_CPP_FILENAM=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\filename_isdir.cxx
DEP_CPP_FILENAME=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
NODEP_CPP_FILENAME=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\filename_list.cxx
DEP_CPP_FILENAME_=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
NODEP_CPP_FILENAME_=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\filename_match.cxx
DEP_CPP_FILENAME_M=\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
NODEP_CPP_FILENAME_M=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\filename_setext.cxx
DEP_CPP_FILENAME_S=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
NODEP_CPP_FILENAME_S=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl.cxx
DEP_CPP_FL_CX=\
	"..\fl\fl_tooltip.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_abort.cxx
DEP_CPP_FL_AB=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_add_idle.cxx
DEP_CPP_FL_AD=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Adjuster.cxx
DEP_CPP_FL_ADJ=\
	"..\fl\fl_adjuster.h"\
	"..\src\fastarrow.h"\
	"..\src\mediumarrow.h"\
	"..\src\slowarrow.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_bitmap.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_arc.cxx
DEP_CPP_FL_AR=\
	"..\FL\math.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
NODEP_CPP_FL_AR=\
	"..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_arci.cxx
DEP_CPP_FL_ARC=\
	"..\FL\math.h"\
	"..\fl\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
NODEP_CPP_FL_ARC=\
	"..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_arg.cxx
DEP_CPP_FL_ARG=\
	"..\fl\fl_tooltip.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
NODEP_CPP_FL_ARG=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_ask.cxx
DEP_CPP_FL_AS=\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_secret_input.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_ask.h"\
	{$(INCLUDE)}"fl\fl_box.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_input.h"\
	{$(INCLUDE)}"fl\fl_input_.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Bitmap.cxx
DEP_CPP_FL_BI=\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_bitmap.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_BMP_Image.cxx
DEP_CPP_FL_BM=\
	"..\fl\fl_bmp_image.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Box.cxx
DEP_CPP_FL_BO=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl_box.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_boxtype.cxx
DEP_CPP_FL_BOX=\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Browser.cxx
DEP_CPP_FL_BR=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_browser.h"\
	{$(INCLUDE)}"fl\fl_browser_.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Browser_.cxx
DEP_CPP_FL_BRO=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_browser_.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Browser_load.cxx
DEP_CPP_FL_BROW=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_browser.h"\
	{$(INCLUDE)}"fl\fl_browser_.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Button.cxx
DEP_CPP_FL_BU=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Chart.cxx
DEP_CPP_FL_CH=\
	"..\FL\math.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_chart.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
NODEP_CPP_FL_CH=\
	"..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Check_Browser.cxx
DEP_CPP_FL_CHE=\
	"..\fl\fl_check_browser.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_browser_.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Check_Button.cxx
DEP_CPP_FL_CHEC=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_check_button.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_light_button.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Choice.cxx
DEP_CPP_FL_CHO=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_choice.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Clock.cxx
DEP_CPP_FL_CL=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_clock.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_color.cxx
DEP_CPP_FL_CO=\
	"..\fl\x.h"\
	"..\src\fl_cmap.h"\
	"..\src\fl_color_mac.cxx"\
	"..\src\fl_color_win32.cxx"\
	"..\src\Fl_XColor.H"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Color_Chooser.cxx
DEP_CPP_FL_COL=\
	"..\fl\fl_color_chooser.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_value_input.h"\
	"..\FL\math.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_box.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_choice.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_input.h"\
	{$(INCLUDE)}"fl\fl_input_.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
NODEP_CPP_FL_COL=\
	"..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_compose.cxx
DEP_CPP_FL_COM=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Counter.cxx
DEP_CPP_FL_COU=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_counter.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_cursor.cxx
DEP_CPP_FL_CU=\
	"..\fl\x.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_curve.cxx
DEP_CPP_FL_CUR=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Dial.cxx
DEP_CPP_FL_DI=\
	"..\FL\math.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_dial.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
NODEP_CPP_FL_DI=\
	"..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_diamond_box.cxx
DEP_CPP_FL_DIA=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_display.cxx
DEP_CPP_FL_DIS=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_dnd.cxx
DEP_CPP_FL_DN=\
	"..\fl\x.h"\
	"..\src\fl_dnd_mac.cxx"\
	"..\src\fl_dnd_win32.cxx"\
	"..\src\fl_dnd_x.cxx"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Double_Window.cxx
DEP_CPP_FL_DO=\
	"..\fl\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_double_window.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_draw.cxx
DEP_CPP_FL_DR=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_draw_image.cxx
DEP_CPP_FL_DRA=\
	"..\fl\x.h"\
	"..\src\fl_draw_image_mac.cxx"\
	"..\src\fl_draw_image_win32.cxx"\
	"..\src\Fl_XColor.H"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_draw_pixmap.cxx
DEP_CPP_FL_DRAW=\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_engraved_label.cxx
DEP_CPP_FL_EN=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_File_Browser.cxx
DEP_CPP_FL_FI=\
	"..\FL\Fl_File_Browser.H"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_browser.h"\
	{$(INCLUDE)}"fl\fl_browser_.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_file_icon.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
NODEP_CPP_FL_FI=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_File_Chooser.cxx
DEP_CPP_FL_FIL=\
	"..\FL\Fl_File_Browser.H"\
	"..\fl\fl_file_input.h"\
	"..\fl\fl_preferences.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_tile.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_ask.h"\
	{$(INCLUDE)}"fl\fl_bitmap.h"\
	{$(INCLUDE)}"fl\fl_box.h"\
	{$(INCLUDE)}"fl\fl_browser.h"\
	{$(INCLUDE)}"fl\fl_browser_.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_check_button.h"\
	{$(INCLUDE)}"fl\fl_choice.h"\
	{$(INCLUDE)}"fl\fl_double_window.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_file_chooser.h"\
	{$(INCLUDE)}"fl\fl_file_icon.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_input.h"\
	{$(INCLUDE)}"fl\fl_input_.h"\
	{$(INCLUDE)}"fl\fl_light_button.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_button.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
NODEP_CPP_FL_FIL=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_File_Chooser2.cxx
DEP_CPP_FL_FILE=\
	"..\FL\Fl_File_Browser.H"\
	"..\fl\fl_file_input.h"\
	"..\fl\fl_preferences.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_tile.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_ask.h"\
	{$(INCLUDE)}"fl\fl_box.h"\
	{$(INCLUDE)}"fl\fl_browser.h"\
	{$(INCLUDE)}"fl\fl_browser_.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_check_button.h"\
	{$(INCLUDE)}"fl\fl_choice.h"\
	{$(INCLUDE)}"fl\fl_double_window.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_file_chooser.h"\
	{$(INCLUDE)}"fl\fl_file_icon.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_input.h"\
	{$(INCLUDE)}"fl\fl_input_.h"\
	{$(INCLUDE)}"fl\fl_light_button.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_button.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_shared_image.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
NODEP_CPP_FL_FILE=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_file_dir.cxx
DEP_CPP_FL_FILE_=\
	"..\FL\Fl_File_Browser.H"\
	"..\fl\fl_file_input.h"\
	"..\fl\fl_preferences.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_tile.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_ask.h"\
	{$(INCLUDE)}"fl\fl_box.h"\
	{$(INCLUDE)}"fl\fl_browser.h"\
	{$(INCLUDE)}"fl\fl_browser_.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_check_button.h"\
	{$(INCLUDE)}"fl\fl_choice.h"\
	{$(INCLUDE)}"fl\fl_double_window.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_file_chooser.h"\
	{$(INCLUDE)}"fl\fl_file_icon.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_input.h"\
	{$(INCLUDE)}"fl\fl_input_.h"\
	{$(INCLUDE)}"fl\fl_light_button.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_button.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
NODEP_CPP_FL_FILE_=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_File_Icon.cxx
DEP_CPP_FL_FILE_I=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_file_icon.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
NODEP_CPP_FL_FILE_I=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_File_Icon2.cxx
DEP_CPP_FL_FILE_IC=\
	"..\FL\math.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_file_icon.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_shared_image.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
NODEP_CPP_FL_FILE_IC=\
	"..\..\..\usr\include\dirent.h"\
	"..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_File_Input.cxx
DEP_CPP_FL_FILE_IN=\
	"..\fl\fl_file_input.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_input.h"\
	{$(INCLUDE)}"fl\fl_input_.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_font.cxx
DEP_CPP_FL_FO=\
	"..\fl\x.h"\
	"..\src\fl_font.h"\
	"..\src\fl_font_mac.cxx"\
	"..\src\fl_font_win32.cxx"\
	"..\src\fl_font_x.cxx"\
	"..\src\fl_font_xft.cxx"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_get_key.cxx
DEP_CPP_FL_GE=\
	"..\fl\x.h"\
	"..\src\Fl_get_key_mac.cxx"\
	"..\src\fl_get_key_win32.cxx"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_get_system_colors.cxx
DEP_CPP_FL_GET=\
	"..\fl\fl_tiled_image.h"\
	"..\FL\math.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	"..\src\tile.xpm"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_pixmap.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
NODEP_CPP_FL_GET=\
	"..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_GIF_Image.cxx
DEP_CPP_FL_GI=\
	"..\fl\fl_gif_image.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_pixmap.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Gl_Choice.cxx
DEP_CPP_FL_GL=\
	"..\FL\gl.h"\
	"..\fl\gl_draw.h"\
	"..\fl\x.h"\
	"..\src\Fl_Gl_Choice.H"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Gl_Overlay.cxx
DEP_CPP_FL_GL_=\
	"..\FL\gl.h"\
	"..\fl\x.h"\
	"..\src\Fl_Gl_Choice.H"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_gl_window.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Gl_Window.cxx
DEP_CPP_FL_GL_W=\
	"..\FL\gl.h"\
	"..\fl\x.h"\
	"..\src\Fl_Gl_Choice.H"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_gl_window.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_grab.cxx
DEP_CPP_FL_GR=\
	"..\fl\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Group.cxx
DEP_CPP_FL_GRO=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Help_Dialog.cxx
DEP_CPP_FL_HE=\
	"..\FL\Fl_Help_Dialog.H"\
	"..\FL\Fl_Help_View.H"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_ask.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_double_window.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_input.h"\
	{$(INCLUDE)}"fl\fl_input_.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_shared_image.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Help_View.cxx
DEP_CPP_FL_HEL=\
	"..\FL\Fl_Help_View.H"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_pixmap.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_shared_image.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Image.cxx
DEP_CPP_FL_IM=\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_images_core.cxx
DEP_CPP_FL_IMA=\
	"..\fl\fl_bmp_image.h"\
	"..\fl\fl_gif_image.h"\
	"..\fl\fl_jpeg_image.h"\
	"..\fl\fl_png_image.h"\
	"..\fl\fl_pnm_image.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_pixmap.h"\
	{$(INCLUDE)}"fl\fl_shared_image.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Input.cxx
DEP_CPP_FL_IN=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_ask.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_input.h"\
	{$(INCLUDE)}"fl\fl_input_.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Input_.cxx
DEP_CPP_FL_INP=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_ask.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_input_.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_JPEG_Image.cxx
DEP_CPP_FL_JP=\
	"..\fl\fl_jpeg_image.h"\
	"..\jpeg\jpeglib.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"jpeg\jconfig.h"\
	{$(INCLUDE)}"jpeg\jerror.h"\
	{$(INCLUDE)}"jpeg\jmorecfg.h"\
	{$(INCLUDE)}"jpeg\jpegint.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_labeltype.cxx
DEP_CPP_FL_LA=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_input_.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Light_Button.cxx
DEP_CPP_FL_LI=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_light_button.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_line_style.cxx
DEP_CPP_FL_LIN=\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_lock.cxx
DEP_CPP_FL_LO=\
	"..\fl\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu.cxx
DEP_CPP_FL_ME=\
	"..\fl\fl_menu_window.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_single_window.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_.cxx
DEP_CPP_FL_MEN=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_add.cxx
DEP_CPP_FL_MENU=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_Bar.cxx
DEP_CPP_FL_MENU_=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_bar.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_Button.cxx
DEP_CPP_FL_MENU_B=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_button.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_global.cxx
DEP_CPP_FL_MENU_G=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_Window.cxx
DEP_CPP_FL_MENU_W=\
	"..\fl\fl_menu_window.h"\
	"..\fl\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_single_window.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Multi_Label.cxx
DEP_CPP_FL_MU=\
	"..\fl\fl_multi_label.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_oval_box.cxx
DEP_CPP_FL_OV=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_overlay.cxx
DEP_CPP_FL_OVE=\
	"..\fl\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_overlay_visual.cxx
DEP_CPP_FL_OVER=\
	"..\fl\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Overlay_Window.cxx
DEP_CPP_FL_OVERL=\
	"..\fl\fl_overlay_window.h"\
	"..\fl\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_double_window.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_own_colormap.cxx
DEP_CPP_FL_OW=\
	"..\fl\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Pack.cxx
DEP_CPP_FL_PA=\
	"..\fl\fl_pack.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Pixmap.cxx
DEP_CPP_FL_PI=\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_pixmap.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_plastic.cxx
DEP_CPP_FL_PL=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_PNG_Image.cxx
DEP_CPP_FL_PN=\
	"..\fl\fl_png_image.h"\
	"..\png\png.h"\
	"..\zlib\zlib.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"png\pngconf.h"\
	{$(INCLUDE)}"zlib\zconf.h"\
	
NODEP_CPP_FL_PN=\
	"..\png\pngusr.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_PNM_Image.cxx
DEP_CPP_FL_PNM=\
	"..\fl\fl_pnm_image.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Positioner.cxx
DEP_CPP_FL_PO=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_positioner.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Preferences.cxx
DEP_CPP_FL_PR=\
	"..\fl\fl_preferences.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
NODEP_CPP_FL_PR=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Progress.cxx
DEP_CPP_FL_PRO=\
	"..\fl\fl_progress.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_rect.cxx
DEP_CPP_FL_RE=\
	"..\fl\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Repeat_Button.cxx
DEP_CPP_FL_REP=\
	"..\fl\fl_repeat_button.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Return_Button.cxx
DEP_CPP_FL_RET=\
	"..\fl\fl_return_button.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Roller.cxx
DEP_CPP_FL_RO=\
	"..\fl\fl_roller.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_round_box.cxx
DEP_CPP_FL_ROU=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Round_Button.cxx
DEP_CPP_FL_ROUN=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_light_button.h"\
	{$(INCLUDE)}"fl\fl_round_button.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_rounded_box.cxx
DEP_CPP_FL_ROUND=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Scroll.cxx
DEP_CPP_FL_SC=\
	"..\fl\fl_scroll.h"\
	"..\fl\fl_tiled_image.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_scroll_area.cxx
DEP_CPP_FL_SCR=\
	"..\fl\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Scrollbar.cxx
DEP_CPP_FL_SCRO=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_set_font.cxx
DEP_CPP_FL_SE=\
	"..\fl\x.h"\
	"..\src\fl_font.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_set_fonts.cxx
DEP_CPP_FL_SET=\
	"..\fl\x.h"\
	"..\src\fl_font.h"\
	"..\src\fl_set_fonts_mac.cxx"\
	"..\src\fl_set_fonts_win32.cxx"\
	"..\src\fl_set_fonts_x.cxx"\
	"..\src\fl_set_fonts_xft.cxx"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_shadow_box.cxx
DEP_CPP_FL_SH=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Shared_Image.cxx
DEP_CPP_FL_SHA=\
	"..\fl\fl_xbm_image.h"\
	"..\fl\fl_xpm_image.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_bitmap.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_pixmap.h"\
	{$(INCLUDE)}"fl\fl_shared_image.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_shortcut.cxx
DEP_CPP_FL_SHO=\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_show_colormap.cxx
DEP_CPP_FL_SHOW=\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_show_colormap.h"\
	{$(INCLUDE)}"fl\fl_single_window.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Single_Window.cxx
DEP_CPP_FL_SI=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_single_window.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Slider.cxx
DEP_CPP_FL_SL=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_symbols.cxx
DEP_CPP_FL_SY=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Tabs.cxx
DEP_CPP_FL_TA=\
	"..\fl\fl_tabs.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Text_Buffer.cxx
DEP_CPP_FL_TE=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_text_buffer.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Text_Display.cxx
DEP_CPP_FL_TEX=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_text_buffer.h"\
	{$(INCLUDE)}"fl\fl_text_display.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Text_Editor.cxx
DEP_CPP_FL_TEXT=\
	"..\fl\fl_text_editor.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_ask.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_text_buffer.h"\
	{$(INCLUDE)}"fl\fl_text_display.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Tile.cxx
DEP_CPP_FL_TI=\
	"..\fl\fl_tile.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Tiled_Image.cxx
DEP_CPP_FL_TIL=\
	"..\fl\fl_tiled_image.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Tooltip.cxx
DEP_CPP_FL_TO=\
	"..\fl\fl_menu_window.h"\
	"..\fl\fl_tooltip.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_single_window.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Valuator.cxx
DEP_CPP_FL_VA=\
	"..\FL\math.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
NODEP_CPP_FL_VA=\
	"..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Value_Input.cxx
DEP_CPP_FL_VAL=\
	"..\fl\fl_value_input.h"\
	"..\FL\math.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_input.h"\
	{$(INCLUDE)}"fl\fl_input_.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
NODEP_CPP_FL_VAL=\
	"..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Value_Output.cxx
DEP_CPP_FL_VALU=\
	"..\fl\fl_value_output.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Value_Slider.cxx
DEP_CPP_FL_VALUE=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_value_slider.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_vertex.cxx
DEP_CPP_FL_VE=\
	"..\FL\math.h"\
	"..\fl\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
NODEP_CPP_FL_VE=\
	"..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_visual.cxx
DEP_CPP_FL_VI=\
	"..\fl\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Widget.cxx
DEP_CPP_FL_WI=\
	"..\fl\fl_tooltip.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window.cxx
DEP_CPP_FL_WIN=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window_fullscreen.cxx
DEP_CPP_FL_WIND=\
	"..\fl\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window_hotspot.cxx
DEP_CPP_FL_WINDO=\
	"..\fl\x.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window_iconize.cxx
DEP_CPP_FL_WINDOW=\
	"..\fl\x.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Wizard.cxx
DEP_CPP_FL_WIZ=\
	"..\fl\fl_wizard.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_x.cxx
DEP_CPP_FL_X_=\
	"..\FL\Fl_Sys_Menu_Bar.H"\
	"..\fl\fl_tooltip.h"\
	"..\fl\x.h"\
	"..\src\Fl_mac.cxx"\
	"..\src\fl_win32.cxx"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_bar.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
NODEP_CPP_FL_X_=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_XBM_Image.cxx
DEP_CPP_FL_XB=\
	"..\fl\fl_xbm_image.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_bitmap.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_XPM_Image.cxx
DEP_CPP_FL_XP=\
	"..\fl\fl_xpm_image.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_pixmap.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\flstring.c
DEP_CPP_FLSTR=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\forms_bitmap.cxx
DEP_CPP_FORMS=\
	"..\FL\Fl_File_Browser.H"\
	"..\fl\fl_file_input.h"\
	"..\fl\fl_preferences.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_tile.h"\
	"..\fl\forms.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_ask.h"\
	{$(INCLUDE)}"fl\fl_bitmap.h"\
	{$(INCLUDE)}"fl\fl_box.h"\
	{$(INCLUDE)}"fl\fl_browser.h"\
	{$(INCLUDE)}"fl\fl_browser_.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_chart.h"\
	{$(INCLUDE)}"fl\fl_check_button.h"\
	{$(INCLUDE)}"fl\fl_choice.h"\
	{$(INCLUDE)}"fl\fl_clock.h"\
	{$(INCLUDE)}"fl\fl_counter.h"\
	{$(INCLUDE)}"fl\fl_dial.h"\
	{$(INCLUDE)}"fl\fl_double_window.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_file_chooser.h"\
	{$(INCLUDE)}"fl\fl_file_icon.h"\
	{$(INCLUDE)}"fl\fl_formsbitmap.h"\
	{$(INCLUDE)}"fl\fl_formspixmap.h"\
	{$(INCLUDE)}"fl\fl_free.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_input.h"\
	{$(INCLUDE)}"fl\fl_input_.h"\
	{$(INCLUDE)}"fl\fl_light_button.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_button.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_pixmap.h"\
	{$(INCLUDE)}"fl\fl_positioner.h"\
	{$(INCLUDE)}"fl\fl_round_button.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_show_colormap.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_timer.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_value_slider.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
NODEP_CPP_FORMS=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\forms_compatability.cxx
DEP_CPP_FORMS_=\
	"..\FL\Fl_File_Browser.H"\
	"..\fl\fl_file_input.h"\
	"..\fl\fl_preferences.h"\
	"..\fl\fl_repeat_button.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_tile.h"\
	"..\fl\forms.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_ask.h"\
	{$(INCLUDE)}"fl\fl_bitmap.h"\
	{$(INCLUDE)}"fl\fl_box.h"\
	{$(INCLUDE)}"fl\fl_browser.h"\
	{$(INCLUDE)}"fl\fl_browser_.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_chart.h"\
	{$(INCLUDE)}"fl\fl_check_button.h"\
	{$(INCLUDE)}"fl\fl_choice.h"\
	{$(INCLUDE)}"fl\fl_clock.h"\
	{$(INCLUDE)}"fl\fl_counter.h"\
	{$(INCLUDE)}"fl\fl_dial.h"\
	{$(INCLUDE)}"fl\fl_double_window.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_file_chooser.h"\
	{$(INCLUDE)}"fl\fl_file_icon.h"\
	{$(INCLUDE)}"fl\fl_formsbitmap.h"\
	{$(INCLUDE)}"fl\fl_formspixmap.h"\
	{$(INCLUDE)}"fl\fl_free.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_input.h"\
	{$(INCLUDE)}"fl\fl_input_.h"\
	{$(INCLUDE)}"fl\fl_light_button.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_button.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_pixmap.h"\
	{$(INCLUDE)}"fl\fl_positioner.h"\
	{$(INCLUDE)}"fl\fl_round_button.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_show_colormap.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_timer.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_value_slider.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
NODEP_CPP_FORMS_=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\forms_free.cxx
DEP_CPP_FORMS_F=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_free.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\forms_fselect.cxx
DEP_CPP_FORMS_FS=\
	"..\FL\Fl_File_Browser.H"\
	"..\fl\fl_file_input.h"\
	"..\fl\fl_preferences.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_tile.h"\
	"..\fl\forms.h"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_ask.h"\
	{$(INCLUDE)}"fl\fl_bitmap.h"\
	{$(INCLUDE)}"fl\fl_box.h"\
	{$(INCLUDE)}"fl\fl_browser.h"\
	{$(INCLUDE)}"fl\fl_browser_.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_chart.h"\
	{$(INCLUDE)}"fl\fl_check_button.h"\
	{$(INCLUDE)}"fl\fl_choice.h"\
	{$(INCLUDE)}"fl\fl_clock.h"\
	{$(INCLUDE)}"fl\fl_counter.h"\
	{$(INCLUDE)}"fl\fl_dial.h"\
	{$(INCLUDE)}"fl\fl_double_window.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_file_chooser.h"\
	{$(INCLUDE)}"fl\fl_file_icon.h"\
	{$(INCLUDE)}"fl\fl_formsbitmap.h"\
	{$(INCLUDE)}"fl\fl_formspixmap.h"\
	{$(INCLUDE)}"fl\fl_free.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_input.h"\
	{$(INCLUDE)}"fl\fl_input_.h"\
	{$(INCLUDE)}"fl\fl_light_button.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_button.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_pixmap.h"\
	{$(INCLUDE)}"fl\fl_positioner.h"\
	{$(INCLUDE)}"fl\fl_round_button.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_show_colormap.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_timer.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_value_slider.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
NODEP_CPP_FORMS_FS=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\forms_pixmap.cxx
DEP_CPP_FORMS_P=\
	"..\FL\Fl_File_Browser.H"\
	"..\fl\fl_file_input.h"\
	"..\fl\fl_preferences.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_tile.h"\
	"..\fl\forms.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_ask.h"\
	{$(INCLUDE)}"fl\fl_bitmap.h"\
	{$(INCLUDE)}"fl\fl_box.h"\
	{$(INCLUDE)}"fl\fl_browser.h"\
	{$(INCLUDE)}"fl\fl_browser_.h"\
	{$(INCLUDE)}"fl\fl_button.h"\
	{$(INCLUDE)}"fl\fl_chart.h"\
	{$(INCLUDE)}"fl\fl_check_button.h"\
	{$(INCLUDE)}"fl\fl_choice.h"\
	{$(INCLUDE)}"fl\fl_clock.h"\
	{$(INCLUDE)}"fl\fl_counter.h"\
	{$(INCLUDE)}"fl\fl_dial.h"\
	{$(INCLUDE)}"fl\fl_double_window.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_file_chooser.h"\
	{$(INCLUDE)}"fl\fl_file_icon.h"\
	{$(INCLUDE)}"fl\fl_formsbitmap.h"\
	{$(INCLUDE)}"fl\fl_formspixmap.h"\
	{$(INCLUDE)}"fl\fl_free.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_input.h"\
	{$(INCLUDE)}"fl\fl_input_.h"\
	{$(INCLUDE)}"fl\fl_light_button.h"\
	{$(INCLUDE)}"fl\fl_menu_.h"\
	{$(INCLUDE)}"fl\fl_menu_button.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_pixmap.h"\
	{$(INCLUDE)}"fl\fl_positioner.h"\
	{$(INCLUDE)}"fl\fl_round_button.h"\
	{$(INCLUDE)}"fl\fl_scrollbar.h"\
	{$(INCLUDE)}"fl\fl_show_colormap.h"\
	{$(INCLUDE)}"fl\fl_slider.h"\
	{$(INCLUDE)}"fl\fl_timer.h"\
	{$(INCLUDE)}"fl\fl_valuator.h"\
	{$(INCLUDE)}"fl\fl_value_slider.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
NODEP_CPP_FORMS_P=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\forms_timer.cxx
DEP_CPP_FORMS_T=\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_timer.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\gl_draw.cxx
DEP_CPP_GL_DR=\
	"..\FL\gl.h"\
	"..\fl\x.h"\
	"..\src\fl_font.h"\
	"..\src\Fl_Gl_Choice.H"\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\gl_start.cxx
DEP_CPP_GL_ST=\
	"..\FL\gl.h"\
	"..\fl\x.h"\
	"..\src\Fl_Gl_Choice.H"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_draw.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	{$(INCLUDE)}"fl\mac.H"\
	{$(INCLUDE)}"fl\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\glut_compatability.cxx
DEP_CPP_GLUT_=\
	"..\fl\glut.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_gl_window.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_image.h"\
	{$(INCLUDE)}"fl\fl_menu_item.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\glut_font.cxx
DEP_CPP_GLUT_F=\
	"..\FL\gl.h"\
	"..\fl\glut.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\enumerations.h"\
	{$(INCLUDE)}"fl\fl.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	{$(INCLUDE)}"fl\fl_gl_window.h"\
	{$(INCLUDE)}"fl\fl_group.h"\
	{$(INCLUDE)}"fl\fl_widget.h"\
	{$(INCLUDE)}"fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\numericsort.c
DEP_CPP_NUMER=\
	".\config.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
NODEP_CPP_NUMER=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\scandir.c
DEP_CPP_SCAND=\
	"..\src\flstring.h"\
	"..\src\scandir_win32.c"\
	".\config.h"\
	{$(INCLUDE)}"fl\filename.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
NODEP_CPP_SCAND=\
	"..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\vsnprintf.c
DEP_CPP_VSNPR=\
	"..\src\flstring.h"\
	".\config.h"\
	{$(INCLUDE)}"fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\screen_xywh.cxx
# End Source File
# End Target
# End Project
