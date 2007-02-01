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
	"..\fl\filename.h"\
	"..\fl\fl_export.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FILEN=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\filename_expand.cxx
DEP_CPP_FILENA=\
	"..\fl\filename.h"\
	"..\fl\fl_export.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FILENA=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\filename_ext.cxx
DEP_CPP_FILENAM=\
	"..\fl\filename.h"\
	"..\fl\fl_export.h"\
	
NODEP_CPP_FILENAM=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\filename_isdir.cxx
DEP_CPP_FILENAME=\
	"..\fl\filename.h"\
	"..\fl\fl_export.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FILENAME=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\filename_list.cxx
DEP_CPP_FILENAME_=\
	"..\fl\filename.h"\
	"..\fl\fl_export.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FILENAME_=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\filename_match.cxx
DEP_CPP_FILENAME_M=\
	"..\fl\filename.h"\
	"..\fl\fl_export.h"\
	
NODEP_CPP_FILENAME_M=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\filename_setext.cxx
DEP_CPP_FILENAME_S=\
	"..\fl\filename.h"\
	"..\fl\fl_export.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FILENAME_S=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl.cxx
DEP_CPP_FL_CX=\
	"..\fl\enumerations.h"\
	"..\fl\filename.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_bar.h"\
	"..\fl\fl_menu_item.h"\
	"..\FL\Fl_Sys_Menu_Bar.H"\
	"..\fl\fl_tooltip.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\fl_font.h"\
	"..\src\Fl_mac.cxx"\
	"..\src\fl_win32.cxx"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_CX=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_abort.cxx
DEP_CPP_FL_AB=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_add_idle.cxx
DEP_CPP_FL_AD=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Adjuster.cxx
DEP_CPP_FL_ADJ=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_adjuster.h"\
	"..\fl\fl_bitmap.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	"..\src\fastarrow.h"\
	"..\src\mediumarrow.h"\
	"..\src\slowarrow.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_arc.cxx
DEP_CPP_FL_AR=\
	"..\fl\enumerations.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\FL\math.h"\
	
NODEP_CPP_FL_AR=\
	"..\..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_arci.cxx
DEP_CPP_FL_ARC=\
	"..\fl\enumerations.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\FL\math.h"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	".\config.h"\
	
NODEP_CPP_FL_ARC=\
	"..\..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_arg.cxx
DEP_CPP_FL_ARG=\
	"..\fl\enumerations.h"\
	"..\fl\filename.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_tooltip.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_ARG=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_ask.cxx
DEP_CPP_FL_AS=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_ask.h"\
	"..\fl\fl_box.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_input.h"\
	"..\fl\fl_input_.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_secret_input.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Bitmap.cxx
DEP_CPP_FL_BI=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_bitmap.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_BMP_Image.cxx
DEP_CPP_FL_BM=\
	"..\fl\enumerations.h"\
	"..\fl\fl_bmp_image.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Box.cxx
DEP_CPP_FL_BO=\
	"..\fl\enumerations.h"\
	"..\fl\fl_box.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_boxtype.cxx
DEP_CPP_FL_BOX=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_widget.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Browser.cxx
DEP_CPP_FL_BR=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_browser.h"\
	"..\fl\fl_browser_.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Browser_.cxx
DEP_CPP_FL_BRO=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_browser_.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Browser_load.cxx
DEP_CPP_FL_BROW=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_browser.h"\
	"..\fl\fl_browser_.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Button.cxx
DEP_CPP_FL_BU=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Chart.cxx
DEP_CPP_FL_CH=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_chart.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_widget.h"\
	"..\FL\math.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_CH=\
	"..\..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Check_Browser.cxx
DEP_CPP_FL_CHE=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_browser_.h"\
	"..\fl\fl_check_browser.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Check_Button.cxx
DEP_CPP_FL_CHEC=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_check_button.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_light_button.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Choice.cxx
DEP_CPP_FL_CHO=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_choice.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_widget.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Clock.cxx
DEP_CPP_FL_CL=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_clock.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_color.cxx
DEP_CPP_FL_CO=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\fl_cmap.h"\
	"..\src\fl_color_mac.cxx"\
	"..\src\fl_color_win32.cxx"\
	"..\src\Fl_XColor.H"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Color_Chooser.cxx
DEP_CPP_FL_COL=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_box.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_choice.h"\
	"..\fl\fl_color_chooser.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_input.h"\
	"..\fl\fl_input_.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_value_input.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\FL\math.h"\
	
NODEP_CPP_FL_COL=\
	"..\..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_compose.cxx
DEP_CPP_FL_COM=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Counter.cxx
DEP_CPP_FL_COU=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_counter.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_cursor.cxx
DEP_CPP_FL_CU=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_curve.cxx
DEP_CPP_FL_CUR=\
	"..\fl\enumerations.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Dial.cxx
DEP_CPP_FL_DI=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_dial.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	"..\FL\math.h"\
	
NODEP_CPP_FL_DI=\
	"..\..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_diamond_box.cxx
DEP_CPP_FL_DIA=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_display.cxx
DEP_CPP_FL_DIS=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_dnd.cxx
DEP_CPP_FL_DN=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\fl_dnd_mac.cxx"\
	"..\src\fl_dnd_win32.cxx"\
	"..\src\fl_dnd_x.cxx"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Double_Window.cxx
DEP_CPP_FL_DO=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_double_window.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_draw.cxx
DEP_CPP_FL_DR=\
	"..\fl\enumerations.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_draw_image.cxx
DEP_CPP_FL_DRA=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\fl_draw_image_mac.cxx"\
	"..\src\fl_draw_image_win32.cxx"\
	"..\src\Fl_XColor.H"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_draw_pixmap.cxx
DEP_CPP_FL_DRAW=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_encoding_latin1.cxx
DEP_CPP_FL_EN=\
	"..\fl\enumerations.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_encoding_mac_roman.cxx
DEP_CPP_FL_ENC=\
	"..\fl\enumerations.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_engraved_label.cxx
DEP_CPP_FL_ENG=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_File_Browser.cxx
DEP_CPP_FL_FI=\
	"..\fl\enumerations.h"\
	"..\fl\filename.h"\
	"..\fl\fl.h"\
	"..\fl\fl_browser.h"\
	"..\fl\fl_browser_.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\FL\Fl_File_Browser.H"\
	"..\fl\fl_file_icon.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_FI=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_File_Chooser.cxx
DEP_CPP_FL_FIL=\
	"..\fl\enumerations.h"\
	"..\fl\filename.h"\
	"..\fl\fl.h"\
	"..\fl\fl_ask.h"\
	"..\fl\fl_bitmap.h"\
	"..\fl\fl_box.h"\
	"..\fl\fl_browser.h"\
	"..\fl\fl_browser_.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_check_button.h"\
	"..\fl\fl_choice.h"\
	"..\fl\fl_double_window.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\FL\Fl_File_Browser.H"\
	"..\fl\fl_file_chooser.h"\
	"..\fl\fl_file_icon.h"\
	"..\fl\fl_file_input.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_input.h"\
	"..\fl\fl_input_.h"\
	"..\fl\fl_light_button.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_button.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_preferences.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_tile.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	
NODEP_CPP_FL_FIL=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_File_Chooser2.cxx
DEP_CPP_FL_FILE=\
	"..\fl\enumerations.h"\
	"..\fl\filename.h"\
	"..\fl\fl.h"\
	"..\fl\fl_ask.h"\
	"..\fl\fl_box.h"\
	"..\fl\fl_browser.h"\
	"..\fl\fl_browser_.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_check_button.h"\
	"..\fl\fl_choice.h"\
	"..\fl\fl_double_window.h"\
	"..\fl\fl_export.h"\
	"..\FL\Fl_File_Browser.H"\
	"..\fl\fl_file_chooser.h"\
	"..\fl\fl_file_icon.h"\
	"..\fl\fl_file_input.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_input.h"\
	"..\fl\fl_input_.h"\
	"..\fl\fl_light_button.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_button.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_preferences.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_shared_image.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_tile.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_FILE=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_file_dir.cxx
DEP_CPP_FL_FILE_=\
	"..\fl\enumerations.h"\
	"..\fl\filename.h"\
	"..\fl\fl.h"\
	"..\fl\fl_ask.h"\
	"..\fl\fl_box.h"\
	"..\fl\fl_browser.h"\
	"..\fl\fl_browser_.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_check_button.h"\
	"..\fl\fl_choice.h"\
	"..\fl\fl_double_window.h"\
	"..\fl\fl_export.h"\
	"..\FL\Fl_File_Browser.H"\
	"..\fl\fl_file_chooser.h"\
	"..\fl\fl_file_icon.h"\
	"..\fl\fl_file_input.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_input.h"\
	"..\fl\fl_input_.h"\
	"..\fl\fl_light_button.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_button.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_preferences.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_tile.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_FILE_=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_File_Icon.cxx
DEP_CPP_FL_FILE_I=\
	"..\fl\enumerations.h"\
	"..\fl\filename.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_file_icon.h"\
	"..\fl\fl_widget.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_FILE_I=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_File_Icon2.cxx
DEP_CPP_FL_FILE_IC=\
	"..\fl\enumerations.h"\
	"..\fl\filename.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_file_icon.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_shared_image.h"\
	"..\fl\fl_widget.h"\
	"..\FL\math.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_FILE_IC=\
	"..\..\..\..\..\usr\include\dirent.h"\
	"..\..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_File_Input.cxx
DEP_CPP_FL_FILE_IN=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_file_input.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_input.h"\
	"..\fl\fl_input_.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_font.cxx
DEP_CPP_FL_FO=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\fl_font.h"\
	"..\src\fl_font_mac.cxx"\
	"..\src\fl_font_win32.cxx"\
	"..\src\fl_font_x.cxx"\
	"..\src\fl_font_xft.cxx"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_get_key.cxx
DEP_CPP_FL_GE=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\Fl_get_key_mac.cxx"\
	"..\src\fl_get_key_win32.cxx"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_get_system_colors.cxx
DEP_CPP_FL_GET=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_pixmap.h"\
	"..\fl\fl_tiled_image.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\FL\math.h"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	"..\src\tile.xpm"\
	".\config.h"\
	
NODEP_CPP_FL_GET=\
	"..\..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_GIF_Image.cxx
DEP_CPP_FL_GI=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_gif_image.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_pixmap.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Gl_Choice.cxx
DEP_CPP_FL_GL=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\FL\gl.h"\
	"..\fl\gl_draw.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\Fl_Gl_Choice.H"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Gl_Overlay.cxx
DEP_CPP_FL_GL_=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\Fl_Gl_Window.H"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\FL\gl.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\Fl_Gl_Choice.H"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Gl_Window.cxx
DEP_CPP_FL_GL_W=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\Fl_Gl_Window.H"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\FL\gl.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\Fl_Gl_Choice.H"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_grab.cxx
DEP_CPP_FL_GR=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Group.cxx
DEP_CPP_FL_GRO=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_gtk.cxx
DEP_CPP_FL_GT=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Help_Dialog.cxx
DEP_CPP_FL_HE=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_ask.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_double_window.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\FL\Fl_Help_Dialog.H"\
	"..\FL\Fl_Help_View.H"\
	"..\fl\fl_image.h"\
	"..\fl\fl_input.h"\
	"..\fl\fl_input_.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_shared_image.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Help_View.cxx
DEP_CPP_FL_HEL=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\FL\Fl_Help_View.H"\
	"..\fl\fl_image.h"\
	"..\fl\fl_pixmap.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_shared_image.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Image.cxx
DEP_CPP_FL_IM=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_images_core.cxx
DEP_CPP_FL_IMA=\
	"..\fl\enumerations.h"\
	"..\fl\fl_bmp_image.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_gif_image.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_jpeg_image.h"\
	"..\fl\fl_pixmap.h"\
	"..\fl\fl_png_image.h"\
	"..\fl\fl_pnm_image.h"\
	"..\fl\fl_shared_image.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Input.cxx
DEP_CPP_FL_IN=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_ask.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_input.h"\
	"..\fl\fl_input_.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Input_.cxx
DEP_CPP_FL_INP=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_ask.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_input_.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_JPEG_Image.cxx
DEP_CPP_FL_JP=\
	"..\fl\enumerations.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_jpeg_image.h"\
	"..\jpeg\jconfig.h"\
	"..\jpeg\jerror.h"\
	"..\jpeg\jmorecfg.h"\
	"..\jpeg\jpegint.h"\
	"..\jpeg\jpeglib.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_labeltype.cxx
DEP_CPP_FL_LA=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_input_.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Light_Button.cxx
DEP_CPP_FL_LI=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_light_button.h"\
	"..\fl\fl_widget.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_line_style.cxx
DEP_CPP_FL_LIN=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_lock.cxx
DEP_CPP_FL_LO=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu.cxx
DEP_CPP_FL_ME=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_menu_window.h"\
	"..\fl\fl_single_window.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_.cxx
DEP_CPP_FL_MEN=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_widget.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_add.cxx
DEP_CPP_FL_MENU=\
	"..\fl\enumerations.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_widget.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_Bar.cxx
DEP_CPP_FL_MENU_=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_bar.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_Button.cxx
DEP_CPP_FL_MENU_B=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_button.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_global.cxx
DEP_CPP_FL_MENU_G=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_Window.cxx
DEP_CPP_FL_MENU_W=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_menu_window.h"\
	"..\fl\fl_single_window.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Multi_Label.cxx
DEP_CPP_FL_MU=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_multi_label.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_open_uri.cxx
DEP_CPP_FL_OP=\
	"..\fl\filename.h"\
	"..\fl\fl_export.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_OP=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_oval_box.cxx
DEP_CPP_FL_OV=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_overlay.cxx
DEP_CPP_FL_OVE=\
	"..\fl\enumerations.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_overlay_visual.cxx
DEP_CPP_FL_OVER=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Overlay_Window.cxx
DEP_CPP_FL_OVERL=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_double_window.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_overlay_window.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_own_colormap.cxx
DEP_CPP_FL_OW=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Pack.cxx
DEP_CPP_FL_PA=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_pack.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Pixmap.cxx
DEP_CPP_FL_PI=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_pixmap.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_plastic.cxx
DEP_CPP_FL_PL=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_PNG_Image.cxx
DEP_CPP_FL_PN=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_png_image.h"\
	"..\png\png.h"\
	"..\png\pngconf.h"\
	"..\zlib\zconf.h"\
	"..\zlib\zlib.h"\
	".\config.h"\
	
NODEP_CPP_FL_PN=\
	"..\png\pngusr.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_PNM_Image.cxx
DEP_CPP_FL_PNM=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_pnm_image.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Positioner.cxx
DEP_CPP_FL_PO=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_positioner.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Preferences.cxx
DEP_CPP_FL_PR=\
	"..\fl\enumerations.h"\
	"..\fl\filename.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_preferences.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_PR=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Progress.cxx
DEP_CPP_FL_PRO=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_progress.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_read_image.cxx
DEP_CPP_FL_RE=\
	"..\fl\enumerations.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\fl_read_image_mac.cxx"\
	"..\src\fl_read_image_win32.cxx"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_rect.cxx
DEP_CPP_FL_REC=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Repeat_Button.cxx
DEP_CPP_FL_REP=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_repeat_button.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Return_Button.cxx
DEP_CPP_FL_RET=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Roller.cxx
DEP_CPP_FL_RO=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_roller.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_round_box.cxx
DEP_CPP_FL_ROU=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Round_Button.cxx
DEP_CPP_FL_ROUN=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_light_button.h"\
	"..\fl\fl_round_button.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_rounded_box.cxx
DEP_CPP_FL_ROUND=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Scroll.cxx
DEP_CPP_FL_SC=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_scroll.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_tiled_image.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_scroll_area.cxx
DEP_CPP_FL_SCR=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Scrollbar.cxx
DEP_CPP_FL_SCRO=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_set_font.cxx
DEP_CPP_FL_SE=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\fl_font.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_set_fonts.cxx
DEP_CPP_FL_SET=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\fl_font.h"\
	"..\src\fl_set_fonts_mac.cxx"\
	"..\src\fl_set_fonts_win32.cxx"\
	"..\src\fl_set_fonts_x.cxx"\
	"..\src\fl_set_fonts_xft.cxx"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_shadow_box.cxx
DEP_CPP_FL_SH=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Shared_Image.cxx
DEP_CPP_FL_SHA=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_bitmap.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_pixmap.h"\
	"..\fl\fl_shared_image.h"\
	"..\fl\fl_xbm_image.h"\
	"..\fl\fl_xpm_image.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_shortcut.cxx
DEP_CPP_FL_SHO=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_show_colormap.cxx
DEP_CPP_FL_SHOW=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_show_colormap.h"\
	"..\fl\fl_single_window.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Single_Window.cxx
DEP_CPP_FL_SI=\
	"..\fl\enumerations.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_single_window.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Slider.cxx
DEP_CPP_FL_SL=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_symbols.cxx
DEP_CPP_FL_SY=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\FL\math.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_SY=\
	"..\..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Tabs.cxx
DEP_CPP_FL_TA=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_tabs.h"\
	"..\fl\fl_tooltip.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Text_Buffer.cxx
DEP_CPP_FL_TE=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_text_buffer.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Text_Display.cxx
DEP_CPP_FL_TEX=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_text_buffer.h"\
	"..\fl\fl_text_display.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Text_Editor.cxx
DEP_CPP_FL_TEXT=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_ask.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_text_buffer.h"\
	"..\fl\fl_text_display.h"\
	"..\fl\fl_text_editor.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Tile.cxx
DEP_CPP_FL_TI=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_tile.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Tiled_Image.cxx
DEP_CPP_FL_TIL=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_tiled_image.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Tooltip.cxx
DEP_CPP_FL_TO=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_menu_window.h"\
	"..\fl\fl_single_window.h"\
	"..\fl\fl_tooltip.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Valuator.cxx
DEP_CPP_FL_VA=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_widget.h"\
	"..\FL\math.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_VA=\
	"..\..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Value_Input.cxx
DEP_CPP_FL_VAL=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_input.h"\
	"..\fl\fl_input_.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_value_input.h"\
	"..\fl\fl_widget.h"\
	"..\FL\math.h"\
	
NODEP_CPP_FL_VAL=\
	"..\..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Value_Output.cxx
DEP_CPP_FL_VALU=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_value_output.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Value_Slider.cxx
DEP_CPP_FL_VALUE=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_value_slider.h"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\fl_vertex.cxx
DEP_CPP_FL_VE=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\FL\math.h"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	".\config.h"\
	
NODEP_CPP_FL_VE=\
	"..\..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_visual.cxx
DEP_CPP_FL_VI=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Widget.cxx
DEP_CPP_FL_WI=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_tooltip.h"\
	"..\fl\fl_widget.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window.cxx
DEP_CPP_FL_WIN=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window_fullscreen.cxx
DEP_CPP_FL_WIND=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window_hotspot.cxx
DEP_CPP_FL_WINDO=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window_iconize.cxx
DEP_CPP_FL_WINDOW=\
	"..\fl\enumerations.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Wizard.cxx
DEP_CPP_FL_WIZ=\
	"..\fl\enumerations.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\fl_wizard.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_x.cxx
DEP_CPP_FL_X_=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_tooltip.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_XBM_Image.cxx
DEP_CPP_FL_XB=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_bitmap.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_xbm_image.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\Fl_XPM_Image.cxx
DEP_CPP_FL_XP=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_pixmap.h"\
	"..\fl\fl_xpm_image.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\flstring.c
DEP_CPP_FLSTR=\
	"..\fl\fl_export.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\forms_bitmap.cxx
DEP_CPP_FORMS=\
	"..\fl\enumerations.h"\
	"..\fl\filename.h"\
	"..\fl\fl.h"\
	"..\fl\fl_ask.h"\
	"..\fl\fl_bitmap.h"\
	"..\fl\fl_box.h"\
	"..\fl\fl_browser.h"\
	"..\fl\fl_browser_.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_chart.h"\
	"..\fl\fl_check_button.h"\
	"..\fl\fl_choice.h"\
	"..\fl\fl_clock.h"\
	"..\fl\fl_counter.h"\
	"..\fl\fl_dial.h"\
	"..\fl\fl_double_window.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\FL\Fl_File_Browser.H"\
	"..\fl\fl_file_chooser.h"\
	"..\fl\fl_file_icon.h"\
	"..\fl\fl_file_input.h"\
	"..\fl\Fl_FormsBitmap.H"\
	"..\fl\Fl_FormsPixmap.H"\
	"..\fl\Fl_Free.H"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_input.h"\
	"..\fl\fl_input_.h"\
	"..\fl\fl_light_button.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_button.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_pixmap.h"\
	"..\fl\fl_positioner.h"\
	"..\fl\fl_preferences.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_round_button.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_show_colormap.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_tile.h"\
	"..\FL\Fl_Timer.H"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_value_slider.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\forms.h"\
	
NODEP_CPP_FORMS=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\forms_compatability.cxx
DEP_CPP_FORMS_=\
	"..\fl\enumerations.h"\
	"..\fl\filename.h"\
	"..\fl\fl.h"\
	"..\fl\fl_ask.h"\
	"..\fl\fl_bitmap.h"\
	"..\fl\fl_box.h"\
	"..\fl\fl_browser.h"\
	"..\fl\fl_browser_.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_chart.h"\
	"..\fl\fl_check_button.h"\
	"..\fl\fl_choice.h"\
	"..\fl\fl_clock.h"\
	"..\fl\fl_counter.h"\
	"..\fl\fl_dial.h"\
	"..\fl\fl_double_window.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\FL\Fl_File_Browser.H"\
	"..\fl\fl_file_chooser.h"\
	"..\fl\fl_file_icon.h"\
	"..\fl\fl_file_input.h"\
	"..\fl\Fl_FormsBitmap.H"\
	"..\fl\Fl_FormsPixmap.H"\
	"..\fl\Fl_Free.H"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_input.h"\
	"..\fl\fl_input_.h"\
	"..\fl\fl_light_button.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_button.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_pixmap.h"\
	"..\fl\fl_positioner.h"\
	"..\fl\fl_preferences.h"\
	"..\fl\fl_repeat_button.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_round_button.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_show_colormap.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_tile.h"\
	"..\FL\Fl_Timer.H"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_value_slider.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\forms.h"\
	
NODEP_CPP_FORMS_=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\forms_free.cxx
DEP_CPP_FORMS_F=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\Fl_Free.H"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\forms_fselect.cxx
DEP_CPP_FORMS_FS=\
	"..\fl\enumerations.h"\
	"..\fl\filename.h"\
	"..\fl\fl.h"\
	"..\fl\fl_ask.h"\
	"..\fl\fl_bitmap.h"\
	"..\fl\fl_box.h"\
	"..\fl\fl_browser.h"\
	"..\fl\fl_browser_.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_chart.h"\
	"..\fl\fl_check_button.h"\
	"..\fl\fl_choice.h"\
	"..\fl\fl_clock.h"\
	"..\fl\fl_counter.h"\
	"..\fl\fl_dial.h"\
	"..\fl\fl_double_window.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\FL\Fl_File_Browser.H"\
	"..\fl\fl_file_chooser.h"\
	"..\fl\fl_file_icon.h"\
	"..\fl\fl_file_input.h"\
	"..\fl\Fl_FormsBitmap.H"\
	"..\fl\Fl_FormsPixmap.H"\
	"..\fl\Fl_Free.H"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_input.h"\
	"..\fl\fl_input_.h"\
	"..\fl\fl_light_button.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_button.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_pixmap.h"\
	"..\fl\fl_positioner.h"\
	"..\fl\fl_preferences.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_round_button.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_show_colormap.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_tile.h"\
	"..\FL\Fl_Timer.H"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_value_slider.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\forms.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FORMS_FS=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\forms_pixmap.cxx
DEP_CPP_FORMS_P=\
	"..\fl\enumerations.h"\
	"..\fl\filename.h"\
	"..\fl\fl.h"\
	"..\fl\fl_ask.h"\
	"..\fl\fl_bitmap.h"\
	"..\fl\fl_box.h"\
	"..\fl\fl_browser.h"\
	"..\fl\fl_browser_.h"\
	"..\fl\fl_button.h"\
	"..\fl\fl_chart.h"\
	"..\fl\fl_check_button.h"\
	"..\fl\fl_choice.h"\
	"..\fl\fl_clock.h"\
	"..\fl\fl_counter.h"\
	"..\fl\fl_dial.h"\
	"..\fl\fl_double_window.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\FL\Fl_File_Browser.H"\
	"..\fl\fl_file_chooser.h"\
	"..\fl\fl_file_icon.h"\
	"..\fl\fl_file_input.h"\
	"..\fl\Fl_FormsBitmap.H"\
	"..\fl\Fl_FormsPixmap.H"\
	"..\fl\Fl_Free.H"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_input.h"\
	"..\fl\fl_input_.h"\
	"..\fl\fl_light_button.h"\
	"..\fl\fl_menu_.h"\
	"..\fl\fl_menu_button.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_pixmap.h"\
	"..\fl\fl_positioner.h"\
	"..\fl\fl_preferences.h"\
	"..\fl\fl_return_button.h"\
	"..\fl\fl_round_button.h"\
	"..\fl\fl_scrollbar.h"\
	"..\fl\fl_show_colormap.h"\
	"..\fl\fl_slider.h"\
	"..\fl\fl_tile.h"\
	"..\FL\Fl_Timer.H"\
	"..\fl\fl_valuator.h"\
	"..\fl\fl_value_slider.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\forms.h"\
	
NODEP_CPP_FORMS_P=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\forms_timer.cxx
DEP_CPP_FORMS_T=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\FL\Fl_Timer.H"\
	"..\fl\fl_widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\freeglut_geometry.cxx
DEP_CPP_GL_DR=\
	"..\FL\gl.h"\
	"..\FL\glut.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\freeglut_stroke_mono_roman.cxx
DEP_CPP_GL_DR=\
	"..\FL\gl.h"\
	"..\FL\glut.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\freeglut_stroke_roman.cxx
DEP_CPP_GL_DR=\
	"..\FL\gl.h"\
	"..\FL\glut.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\freeglut_teapot.cxx
DEP_CPP_GL_DR=\
	"..\FL\gl.h"\
	"..\FL\glut.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\gl_draw.cxx
DEP_CPP_GL_DR=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\FL\gl.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\fl_font.h"\
	"..\src\Fl_Gl_Choice.H"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\gl_start.cxx
DEP_CPP_GL_ST=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_draw.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\FL\gl.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	"..\src\Fl_Gl_Choice.H"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\glut_compatability.cxx
DEP_CPP_GLUT_=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\Fl_Gl_Window.H"\
	"..\fl\fl_group.h"\
	"..\fl\fl_image.h"\
	"..\fl\fl_menu_item.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\glut.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\glut_font.cxx
DEP_CPP_GLUT_F=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\Fl_Gl_Window.H"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\FL\gl.h"\
	"..\fl\glut.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\numericsort.c
DEP_CPP_NUMER=\
	"..\fl\filename.h"\
	"..\fl\fl_export.h"\
	".\config.h"\
	
NODEP_CPP_NUMER=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\scandir.c
DEP_CPP_SCAND=\
	"..\fl\filename.h"\
	"..\fl\fl_export.h"\
	"..\src\flstring.h"\
	"..\src\scandir_win32.c"\
	".\config.h"\
	
NODEP_CPP_SCAND=\
	"..\..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\screen_xywh.cxx
DEP_CPP_SCREE=\
	"..\fl\enumerations.h"\
	"..\fl\fl.h"\
	"..\fl\fl_export.h"\
	"..\fl\fl_group.h"\
	"..\fl\fl_widget.h"\
	"..\fl\fl_window.h"\
	"..\fl\mac.H"\
	"..\fl\win32.h"\
	"..\fl\x.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\src\vsnprintf.c
DEP_CPP_VSNPR=\
	"..\fl\fl_export.h"\
	"..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# End Target
# End Project
