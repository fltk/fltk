# Microsoft Developer Studio Project File - Name="fltk" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=fltk - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fltk.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fltk.mak" CFG="fltk - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fltk - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "fltk - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fltk - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release/fltk"
# PROP BASE Intermediate_Dir "Release/fltk"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release/fltk"
# PROP Intermediate_Dir "Release/fltk"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "FL_LIBRARY" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /GX /Ot /Op /Ob2 /I "." /I "../.." /I "../../zlib" /I "../../png" /I "../../jpeg" /D "WIN32" /D "FL_LIBRARY" /D "NDEBUG" /D "_WINDOWS" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /FD /c
# SUBTRACT CPP /Os
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\fltk.lib"

!ELSEIF  "$(CFG)" == "fltk - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug/fltk"
# PROP BASE Intermediate_Dir "Debug/fltk"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug/fltk"
# PROP Intermediate_Dir "Debug/fltk"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "FL_LIBRARY" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /GX /Z7 /Od /I "." /I "../.." /I "../../zlib" /I "../../png" /I "../../jpeg" /D "WIN32" /D "FL_LIBRARY" /D "_DEBUG" /D "_WINDOWS" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\fltkd.lib"

!ENDIF 

# Begin Target

# Name "fltk - Win32 Release"
# Name "fltk - Win32 Debug"
# Begin Source File

SOURCE="..\..\FL\abi-version.h"

!IF  "$(CFG)" == "fltk - Win32 Release"

USERDEP__ABI_V="..\..\abi-version.ide"	
# Begin Custom Build - Creating FL\abi-version.h from abi-version.ide
InputPath=..\..\FL\abi-version.h

"..\..\FL\abi-version.h" : "..\..\abi-version.ide"
	copy /Y ..\..\abi-version.ide ..\..\FL\abi-version.h

# End Custom Build

!ELSEIF  "$(CFG)" == "fltk - Win32 Debug"

USERDEP__ABI_V="..\..\abi-version.ide"	
# Begin Custom Build - Creating FL\abi-version.h from abi-version.ide
InputPath=..\..\FL\abi-version.h

"..\..\FL\abi-version.h" : "..\..\abi-version.ide"
	copy /Y ..\..\abi-version.ide ..\..\FL\abi-version.h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\xutf8\case.c
# End Source File
# Begin Source File

SOURCE=..\..\src\cmap.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_absolute.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_expand.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_ext.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_isdir.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_list.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_match.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_setext.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_abort.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_add_idle.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Adjuster.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_arc.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_arci.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_arg.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_ask.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Bitmap.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Box.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_boxtype.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Browser.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Browser_.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Browser_load.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Button.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_call_main.c
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Chart.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Check_Browser.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Check_Button.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Choice.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Clock.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_color.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Color_Chooser.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Copy_Surface.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_compose.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Counter.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_cursor.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_curve.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Device.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Dial.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_diamond_box.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_display.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_dnd.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Double_Window.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_draw.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_draw_image.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_draw_pixmap.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_encoding_latin1.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_encoding_mac_roman.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_engraved_label.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_File_Browser.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_File_Chooser.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_File_Chooser2.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_file_dir.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_File_Icon.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_File_Input.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_font.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_get_key.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_get_system_colors.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_grab.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Group.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_gtk.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_gleam.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Help_View.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Image.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Image_Surface.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Input.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Input_.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_labeltype.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Light_Button.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_line_style.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_lock.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu_.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu_add.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu_Bar.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu_Button.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu_global.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu_Window.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Multi_Label.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Native_File_Chooser.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_open_uri.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_oval_box.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_overlay.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_overlay_visual.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Overlay_Window.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_own_colormap.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Pack.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Paged_Device.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Pixmap.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_plastic.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Positioner.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_PostScript.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Preferences.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Printer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Progress.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_read_image.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_rect.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Repeat_Button.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Return_Button.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Roller.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_round_box.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Round_Button.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_rounded_box.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Scroll.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_scroll_area.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Scrollbar.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_set_font.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_set_fonts.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_shadow_box.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Shared_Image.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_shortcut.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_show_colormap.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Single_Window.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Slider.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_symbols.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Table.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Table_Row.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Tabs.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Text_Buffer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Text_Display.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Text_Editor.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Tile.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Tiled_Image.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Tooltip.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Tree.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Tree_Item.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Tree_Item_Array.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Tree_Prefs.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_utf.c
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_utf8.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Valuator.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Value_Input.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Value_Output.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Value_Slider.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_vertex.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_visual.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Widget.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Window.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Window_fullscreen.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Window_hotspot.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Window_iconize.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Window_shape.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Wizard.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_x.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_XBM_Image.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_XPM_Image.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\flstring.c
# End Source File
# Begin Source File

SOURCE=..\..\src\xutf8\is_right2left.c
# End Source File
# Begin Source File

SOURCE=..\..\src\xutf8\is_spacing.c
# End Source File
# Begin Source File

SOURCE=..\..\src\numericsort.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ps_image.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\scandir.c
# End Source File
# Begin Source File

SOURCE=..\..\src\screen_xywh.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\vsnprintf.c
# End Source File
# End Target
# End Project
