# Microsoft Developer Studio Project File - Name="fltkdll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
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
!MESSAGE "fltkdll - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "fltkdll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fltkdll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fltkdll_"
# PROP BASE Intermediate_Dir "fltkdll_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "fltkdll_"
# PROP Intermediate_Dir "fltkdll_"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /GX /Os /Ob1 /I "." /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 glu32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /incremental:yes /machine:I386 /out:"../lib/fltk.dll"

!ELSEIF  "$(CFG)" == "fltkdll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fltkdll0"
# PROP BASE Intermediate_Dir "fltkdll0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "fltkdll0"
# PROP Intermediate_Dir "fltkdll0"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /Gm /GX /Zi /Od /Ob1 /I "." /I ".." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 glu32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../lib/fltkd.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "fltkdll - Win32 Release"
# Name "fltkdll - Win32 Debug"
# Begin Source File

SOURCE=..\src\dummymain.c
# End Source File
# Begin Source File

SOURCE=..\src\filename_absolute.cxx
# End Source File
# Begin Source File

SOURCE=..\src\filename_expand.cxx
# End Source File
# Begin Source File

SOURCE=..\src\filename_ext.cxx
# End Source File
# Begin Source File

SOURCE=..\src\filename_isdir.cxx
# End Source File
# Begin Source File

SOURCE=..\src\filename_list.cxx
# End Source File
# Begin Source File

SOURCE=..\src\filename_match.cxx
# End Source File
# Begin Source File

SOURCE=..\src\filename_setext.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_abort.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Adjuster.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_arc.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_arci.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_arg.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_ask.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Bitmap.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Box.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_boxtype.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Browser.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Browser_.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Browser_load.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Button.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Chart.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Check_Button.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Choice.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Clock.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_color.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Color_Chooser.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Counter.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_cursor.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_curve.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_cutpaste.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Dial.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_diamond_box.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_display.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Double_Window.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_draw.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_draw_image.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_draw_pixmap.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_engraved_label.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_file_chooser.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_font.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_get_key.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_get_system_colors.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Gl_Choice.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Gl_Choice.H
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Gl_Overlay.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Gl_Window.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Group.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Image.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Input.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Input_.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_labeltype.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Light_Button.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_add.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_Bar.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_Button.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_global.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_Window.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Multi_Label.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Output.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_oval_box.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_overlay.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_overlay_visual.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Overlay_Window.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_own_colormap.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Pack.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Pixmap.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Positioner.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_rect.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Repeat_Button.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Return_Button.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Roller.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_round_box.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Round_Button.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_rounded_box.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Scroll.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_scroll_area.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Scrollbar.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_set_font.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_set_fonts.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_shadow_box.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_shortcut.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_show_colormap.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Single_Window.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Slider.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_symbols.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Tabs.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Tile.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Valuator.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Value_Input.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Value_Output.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Value_Slider.cxx
# End Source File
# Begin Source File

SOURCE=..\src\fl_vertex.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_visual.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Widget.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window_fullscreen.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window_hotspot.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window_iconize.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Fl_x.cxx
# End Source File
# Begin Source File

SOURCE=..\src\forms_bitmap.cxx
# End Source File
# Begin Source File

SOURCE=..\src\forms_compatability.cxx
# End Source File
# Begin Source File

SOURCE=..\src\forms_free.cxx
# End Source File
# Begin Source File

SOURCE=..\src\forms_fselect.cxx
# End Source File
# Begin Source File

SOURCE=..\src\forms_pixmap.cxx
# End Source File
# Begin Source File

SOURCE=..\src\forms_timer.cxx
# End Source File
# Begin Source File

SOURCE=..\src\gl_draw.cxx
# End Source File
# Begin Source File

SOURCE=..\src\gl_start.cxx
# End Source File
# Begin Source File

SOURCE=..\src\glut_compatability.cxx
# End Source File
# Begin Source File

SOURCE=..\src\glut_font.cxx
# End Source File
# Begin Source File

SOURCE=..\src\numericsort.c
# End Source File
# Begin Source File

SOURCE=..\src\scandir.c
# End Source File
# Begin Source File

SOURCE=..\src\vsnprintf.c
# End Source File
# End Target
# End Project
