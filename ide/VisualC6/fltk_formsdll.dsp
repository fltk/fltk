# Microsoft Developer Studio Project File - Name="fltk_formsdll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=fltk_formsdll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fltk_formsdll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fltk_formsdll.mak" CFG="fltk_formsdll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fltk_formsdll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "fltk_formsdll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fltk_formsdll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release/fltk_formsdll"
# PROP BASE Intermediate_Dir "Release/fltk_formsdll"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release/fltk_formsdll"
# PROP Intermediate_Dir "Release/fltk_formsdll"
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
# ADD LINK32 opengl32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltk_formsdll.pdb" /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "fltk_formsdll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug/fltk_formsdll"
# PROP BASE Intermediate_Dir "Debug/fltk_formsdll"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug/fltk_formsdll"
# PROP Intermediate_Dir "Debug/fltk_formsdll"
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
# ADD LINK32 opengl32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltk_formsdlld.pdb" /debug /machine:I386 /out:"Debug/fltk_formsdll/fltk_formsdlld.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "fltk_formsdll - Win32 Release"
# Name "fltk_formsdll - Win32 Debug"
# Begin Source File

SOURCE=..\..\src\forms_bitmap.cxx
DEP_CPP_FORMS=\
	"..\..\fl\enumerations.h"\
	"..\..\fl\filename.h"\
	"..\..\fl\fl.h"\
	"..\..\fl\fl_ask.h"\
	"..\..\fl\fl_bitmap.h"\
	"..\..\fl\fl_box.h"\
	"..\..\fl\fl_browser.h"\
	"..\..\fl\fl_browser_.h"\
	"..\..\fl\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\fl\fl_chart.h"\
	"..\..\fl\fl_check_button.h"\
	"..\..\fl\fl_choice.h"\
	"..\..\fl\fl_clock.h"\
	"..\..\fl\fl_counter.h"\
	"..\..\fl\fl_device.h"\
	"..\..\fl\fl_dial.h"\
	"..\..\fl\fl_double_window.h"\
	"..\..\fl\fl_draw.h"\
	"..\..\fl\fl_export.h"\
	"..\..\fl\fl_file_browser.h"\
	"..\..\fl\fl_file_chooser.h"\
	"..\..\fl\fl_file_icon.h"\
	"..\..\fl\fl_file_input.h"\
	"..\..\fl\fl_formsbitmap.h"\
	"..\..\fl\fl_formspixmap.h"\
	"..\..\fl\fl_free.h"\
	"..\..\fl\fl_group.h"\
	"..\..\fl\fl_image.h"\
	"..\..\fl\fl_input.h"\
	"..\..\fl\fl_input_.h"\
	"..\..\fl\fl_light_button.h"\
	"..\..\fl\fl_menu_.h"\
	"..\..\fl\fl_menu_button.h"\
	"..\..\fl\fl_menu_item.h"\
	"..\..\fl\fl_pixmap.h"\
	"..\..\fl\fl_plugin.h"\
	"..\..\fl\fl_positioner.h"\
	"..\..\fl\fl_preferences.h"\
	"..\..\fl\fl_return_button.h"\
	"..\..\fl\fl_rgb_image.h"\
	"..\..\fl\fl_round_button.h"\
	"..\..\fl\fl_scrollbar.h"\
	"..\..\fl\fl_show_colormap.h"\
	"..\..\fl\fl_slider.h"\
	"..\..\fl\fl_tile.h"\
	"..\..\fl\fl_timer.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\fl\fl_valuator.h"\
	"..\..\fl\fl_value_slider.h"\
	"..\..\fl\fl_widget.h"\
	"..\..\fl\fl_window.h"\
	"..\..\fl\forms.h"\
	"..\..\fl\mac.H"\
	"..\..\fl\win32.h"\
	"..\..\fl\x.h"\
	"..\..\fl\Xutf8.h"\
	
NODEP_CPP_FORMS=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\forms_compatability.cxx
DEP_CPP_FORMS_=\
	"..\..\fl\enumerations.h"\
	"..\..\fl\filename.h"\
	"..\..\fl\fl.h"\
	"..\..\fl\fl_ask.h"\
	"..\..\fl\fl_bitmap.h"\
	"..\..\fl\fl_box.h"\
	"..\..\fl\fl_browser.h"\
	"..\..\fl\fl_browser_.h"\
	"..\..\fl\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\fl\fl_chart.h"\
	"..\..\fl\fl_check_button.h"\
	"..\..\fl\fl_choice.h"\
	"..\..\fl\fl_clock.h"\
	"..\..\fl\fl_counter.h"\
	"..\..\fl\fl_device.h"\
	"..\..\fl\fl_dial.h"\
	"..\..\fl\fl_double_window.h"\
	"..\..\fl\fl_draw.h"\
	"..\..\fl\fl_export.h"\
	"..\..\fl\fl_file_browser.h"\
	"..\..\fl\fl_file_chooser.h"\
	"..\..\fl\fl_file_icon.h"\
	"..\..\fl\fl_file_input.h"\
	"..\..\fl\fl_formsbitmap.h"\
	"..\..\fl\fl_formspixmap.h"\
	"..\..\fl\fl_free.h"\
	"..\..\fl\fl_group.h"\
	"..\..\fl\fl_image.h"\
	"..\..\fl\fl_input.h"\
	"..\..\fl\fl_input_.h"\
	"..\..\fl\fl_light_button.h"\
	"..\..\fl\fl_menu_.h"\
	"..\..\fl\fl_menu_button.h"\
	"..\..\fl\fl_menu_item.h"\
	"..\..\fl\fl_pixmap.h"\
	"..\..\fl\fl_plugin.h"\
	"..\..\fl\fl_positioner.h"\
	"..\..\fl\fl_preferences.h"\
	"..\..\fl\fl_repeat_button.h"\
	"..\..\fl\fl_return_button.h"\
	"..\..\fl\fl_rgb_image.h"\
	"..\..\fl\fl_round_button.h"\
	"..\..\fl\fl_scrollbar.h"\
	"..\..\fl\fl_show_colormap.h"\
	"..\..\fl\fl_slider.h"\
	"..\..\fl\fl_tile.h"\
	"..\..\fl\fl_timer.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\fl\fl_valuator.h"\
	"..\..\fl\fl_value_slider.h"\
	"..\..\fl\fl_widget.h"\
	"..\..\fl\fl_window.h"\
	"..\..\fl\forms.h"\
	"..\..\fl\mac.H"\
	"..\..\fl\win32.h"\
	"..\..\fl\x.h"\
	"..\..\fl\Xutf8.h"\
	
NODEP_CPP_FORMS_=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\forms_free.cxx
DEP_CPP_FORMS_F=\
	"..\..\fl\enumerations.h"\
	"..\..\fl\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\fl\fl_export.h"\
	"..\..\fl\fl_free.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\fl\fl_widget.h"\
	"..\..\fl\Xutf8.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\forms_fselect.cxx
DEP_CPP_FORMS_FS=\
	"..\..\fl\enumerations.h"\
	"..\..\fl\filename.h"\
	"..\..\fl\fl.h"\
	"..\..\fl\fl_ask.h"\
	"..\..\fl\fl_bitmap.h"\
	"..\..\fl\fl_box.h"\
	"..\..\fl\fl_browser.h"\
	"..\..\fl\fl_browser_.h"\
	"..\..\fl\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\fl\fl_chart.h"\
	"..\..\fl\fl_check_button.h"\
	"..\..\fl\fl_choice.h"\
	"..\..\fl\fl_clock.h"\
	"..\..\fl\fl_counter.h"\
	"..\..\fl\fl_device.h"\
	"..\..\fl\fl_dial.h"\
	"..\..\fl\fl_double_window.h"\
	"..\..\fl\fl_draw.h"\
	"..\..\fl\fl_export.h"\
	"..\..\fl\fl_file_browser.h"\
	"..\..\fl\fl_file_chooser.h"\
	"..\..\fl\fl_file_icon.h"\
	"..\..\fl\fl_file_input.h"\
	"..\..\fl\fl_formsbitmap.h"\
	"..\..\fl\fl_formspixmap.h"\
	"..\..\fl\fl_free.h"\
	"..\..\fl\fl_group.h"\
	"..\..\fl\fl_image.h"\
	"..\..\fl\fl_input.h"\
	"..\..\fl\fl_input_.h"\
	"..\..\fl\fl_light_button.h"\
	"..\..\fl\fl_menu_.h"\
	"..\..\fl\fl_menu_button.h"\
	"..\..\fl\fl_menu_item.h"\
	"..\..\fl\fl_pixmap.h"\
	"..\..\fl\fl_plugin.h"\
	"..\..\fl\fl_positioner.h"\
	"..\..\fl\fl_preferences.h"\
	"..\..\fl\fl_return_button.h"\
	"..\..\fl\fl_rgb_image.h"\
	"..\..\fl\fl_round_button.h"\
	"..\..\fl\fl_scrollbar.h"\
	"..\..\fl\fl_show_colormap.h"\
	"..\..\fl\fl_slider.h"\
	"..\..\fl\fl_tile.h"\
	"..\..\fl\fl_timer.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\fl\fl_valuator.h"\
	"..\..\fl\fl_value_slider.h"\
	"..\..\fl\fl_widget.h"\
	"..\..\fl\fl_window.h"\
	"..\..\fl\forms.h"\
	"..\..\fl\mac.H"\
	"..\..\fl\win32.h"\
	"..\..\fl\x.h"\
	"..\..\fl\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FORMS_FS=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\forms_pixmap.cxx
DEP_CPP_FORMS_P=\
	"..\..\fl\enumerations.h"\
	"..\..\fl\filename.h"\
	"..\..\fl\fl.h"\
	"..\..\fl\fl_ask.h"\
	"..\..\fl\fl_bitmap.h"\
	"..\..\fl\fl_box.h"\
	"..\..\fl\fl_browser.h"\
	"..\..\fl\fl_browser_.h"\
	"..\..\fl\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\fl\fl_chart.h"\
	"..\..\fl\fl_check_button.h"\
	"..\..\fl\fl_choice.h"\
	"..\..\fl\fl_clock.h"\
	"..\..\fl\fl_counter.h"\
	"..\..\fl\fl_device.h"\
	"..\..\fl\fl_dial.h"\
	"..\..\fl\fl_double_window.h"\
	"..\..\fl\fl_draw.h"\
	"..\..\fl\fl_export.h"\
	"..\..\fl\fl_file_browser.h"\
	"..\..\fl\fl_file_chooser.h"\
	"..\..\fl\fl_file_icon.h"\
	"..\..\fl\fl_file_input.h"\
	"..\..\fl\fl_formsbitmap.h"\
	"..\..\fl\fl_formspixmap.h"\
	"..\..\fl\fl_free.h"\
	"..\..\fl\fl_group.h"\
	"..\..\fl\fl_image.h"\
	"..\..\fl\fl_input.h"\
	"..\..\fl\fl_input_.h"\
	"..\..\fl\fl_light_button.h"\
	"..\..\fl\fl_menu_.h"\
	"..\..\fl\fl_menu_button.h"\
	"..\..\fl\fl_menu_item.h"\
	"..\..\fl\fl_pixmap.h"\
	"..\..\fl\fl_plugin.h"\
	"..\..\fl\fl_positioner.h"\
	"..\..\fl\fl_preferences.h"\
	"..\..\fl\fl_return_button.h"\
	"..\..\fl\fl_rgb_image.h"\
	"..\..\fl\fl_round_button.h"\
	"..\..\fl\fl_scrollbar.h"\
	"..\..\fl\fl_show_colormap.h"\
	"..\..\fl\fl_slider.h"\
	"..\..\fl\fl_tile.h"\
	"..\..\fl\fl_timer.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\fl\fl_valuator.h"\
	"..\..\fl\fl_value_slider.h"\
	"..\..\fl\fl_widget.h"\
	"..\..\fl\fl_window.h"\
	"..\..\fl\forms.h"\
	"..\..\fl\mac.H"\
	"..\..\fl\win32.h"\
	"..\..\fl\x.h"\
	"..\..\fl\Xutf8.h"\
	
NODEP_CPP_FORMS_P=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\forms_timer.cxx
DEP_CPP_FORMS_T=\
	"..\..\fl\enumerations.h"\
	"..\..\fl\fl.h"\
	"..\..\fl\fl_bitmap.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\fl\fl_device.h"\
	"..\..\fl\fl_draw.h"\
	"..\..\fl\fl_export.h"\
	"..\..\fl\fl_group.h"\
	"..\..\fl\fl_image.h"\
	"..\..\fl\fl_pixmap.h"\
	"..\..\fl\fl_plugin.h"\
	"..\..\fl\fl_preferences.h"\
	"..\..\fl\fl_rgb_image.h"\
	"..\..\fl\fl_timer.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\fl\fl_widget.h"\
	"..\..\fl\fl_window.h"\
	"..\..\fl\mac.H"\
	"..\..\fl\win32.h"\
	"..\..\fl\x.h"\
	"..\..\fl\Xutf8.h"\
	
# End Source File
# End Target
# End Project
