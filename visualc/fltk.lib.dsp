# Microsoft Developer Studio Project File - Name="fltk" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=fltk - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fltk.lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fltk.lib.mak" CFG="fltk - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fltk - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "fltk - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "fltk - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Os /Ob2 /I "." /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /TP /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\fltk.lib"

!ELSEIF  "$(CFG)" == "fltk - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "." /I ".." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /TP /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\fltkd.lib"

!ENDIF 

# Begin Target

# Name "fltk - Win32 Release"
# Name "fltk - Win32 Debug"
# Begin Source File

SOURCE=..\src\filename_absolute.C
# End Source File
# Begin Source File

SOURCE=..\src\filename_expand.C
# End Source File
# Begin Source File

SOURCE=..\src\filename_ext.C
# End Source File
# Begin Source File

SOURCE=..\src\filename_isdir.C
# End Source File
# Begin Source File

SOURCE=..\src\filename_list.C
# End Source File
# Begin Source File

SOURCE=..\src\filename_match.C
# End Source File
# Begin Source File

SOURCE=..\src\filename_setext.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_abort.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Adjuster.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_arc.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_arci.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_arg.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_ask.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Bitmap.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Box.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_boxtype.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Browser.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Browser_.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Browser_load.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Button.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Chart.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Check_Button.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Choice.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Clock.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_color.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Color_Chooser.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Counter.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_cursor.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_curve.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_cutpaste.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Dial.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_diamond_box.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_display.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Double_Window.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_draw.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_draw_image.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_draw_pixmap.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_engraved_label.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_file_chooser.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_font.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_get_key.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_get_system_colors.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Gl_Choice.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Gl_Choice.H
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Gl_Overlay.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Gl_Window.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Group.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Image.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Input.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Input_.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_labeltype.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Light_Button.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_add.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_Bar.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_Button.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_global.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Menu_Window.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Multi_Label.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Output.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_oval_box.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_overlay.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_overlay_visual.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Overlay_Window.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_own_colormap.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Pack.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Pixmap.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Positioner.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_rect.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Repeat_Button.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Return_Button.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Roller.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_round_box.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Round_Button.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_rounded_box.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Scroll.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_scroll_area.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Scrollbar.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_set_font.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_set_fonts.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_shadow_box.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_shortcut.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_show_colormap.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Single_Window.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Slider.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_symbols.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Tabs.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Tile.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Valuator.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Value_Input.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Value_Output.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Value_Slider.C
# End Source File
# Begin Source File

SOURCE=..\src\fl_vertex.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_visual.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Widget.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window_fullscreen.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window_hotspot.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_Window_iconize.C
# End Source File
# Begin Source File

SOURCE=..\src\Fl_x.C
# End Source File
# Begin Source File

SOURCE=..\src\forms_bitmap.C
# End Source File
# Begin Source File

SOURCE=..\src\forms_compatability.C
# End Source File
# Begin Source File

SOURCE=..\src\forms_free.C
# End Source File
# Begin Source File

SOURCE=..\src\forms_fselect.C
# End Source File
# Begin Source File

SOURCE=..\src\forms_pixmap.C
# End Source File
# Begin Source File

SOURCE=..\src\forms_timer.C
# End Source File
# Begin Source File

SOURCE=..\src\gl_draw.C
# End Source File
# Begin Source File

SOURCE=..\src\gl_start.C
# End Source File
# Begin Source File

SOURCE=..\src\glut_compatability.C
# End Source File
# Begin Source File

SOURCE=..\src\glut_font.C
# End Source File
# Begin Source File

SOURCE=..\src\numericsort.c
# End Source File
# Begin Source File

SOURCE=..\src\scandir.c
# End Source File
# End Target
# End Project
