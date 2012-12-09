# Microsoft Developer Studio Project File - Name="fltk_imagesdll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=fltk_imagesdll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fltk_imagesdll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fltk_imagesdll.mak" CFG="fltk_imagesdll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fltk_imagesdll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "fltk_imagesdll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fltk_imagesdll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release/fltk_imagesdll"
# PROP BASE Intermediate_Dir "Release/fltk_imagesdll"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release/fltk_imagesdll"
# PROP Intermediate_Dir "Release/fltk_imagesdll"
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
# ADD LINK32 opengl32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltk_imagesdll.pdb" /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "fltk_imagesdll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug/fltk_imagesdll"
# PROP BASE Intermediate_Dir "Debug/fltk_imagesdll"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug/fltk_imagesdll"
# PROP Intermediate_Dir "Debug/fltk_imagesdll"
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
# ADD LINK32 opengl32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltk_imagesdlld.pdb" /debug /machine:I386 /out:"Debug/fltk_imagesdll/fltk_imagesdlld.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "fltk_imagesdll - Win32 Release"
# Name "fltk_imagesdll - Win32 Debug"
# Begin Source File

SOURCE=..\..\src\Fl_BMP_Image.cxx
DEP_CPP_FL_BM=\
	"..\..\fl\enumerations.h"\
	"..\..\fl\fl_bmp_image.h"\
	"..\..\fl\fl_export.h"\
	"..\..\fl\fl_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\fl\Xutf8.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_File_Icon2.cxx
DEP_CPP_FL_FI=\
	"..\..\fl\enumerations.h"\
	"..\..\fl\filename.h"\
	"..\..\fl\fl.h"\
	"..\..\fl\fl_bitmap.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\fl\fl_device.h"\
	"..\..\fl\fl_draw.h"\
	"..\..\fl\fl_export.h"\
	"..\..\fl\fl_file_icon.h"\
	"..\..\fl\fl_group.h"\
	"..\..\fl\fl_image.h"\
	"..\..\fl\fl_pixmap.h"\
	"..\..\fl\fl_plugin.h"\
	"..\..\fl\fl_preferences.h"\
	"..\..\fl\fl_rgb_image.h"\
	"..\..\fl\fl_shared_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\fl\fl_widget.h"\
	"..\..\fl\fl_window.h"\
	"..\..\fl\mac.H"\
	"..\..\FL\math.h"\
	"..\..\fl\win32.h"\
	"..\..\fl\x.h"\
	"..\..\fl\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_FI=\
	"..\..\..\..\usr\include\dirent.h"\
	"..\..\..\..\usr\include\math.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_GIF_Image.cxx
DEP_CPP_FL_GI=\
	"..\..\fl\enumerations.h"\
	"..\..\fl\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\fl\fl_export.h"\
	"..\..\fl\fl_gif_image.h"\
	"..\..\fl\fl_image.h"\
	"..\..\fl\fl_pixmap.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\fl\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Help_Dialog.cxx
DEP_CPP_FL_HE=\
	"..\..\fl\enumerations.h"\
	"..\..\fl\filename.h"\
	"..\..\fl\fl.h"\
	"..\..\fl\fl_ask.h"\
	"..\..\fl\fl_bitmap.h"\
	"..\..\fl\fl_box.h"\
	"..\..\fl\fl_button.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\fl\fl_device.h"\
	"..\..\fl\fl_double_window.h"\
	"..\..\fl\fl_draw.h"\
	"..\..\fl\fl_export.h"\
	"..\..\fl\fl_group.h"\
	"..\..\fl\fl_help_dialog.h"\
	"..\..\fl\fl_help_view.h"\
	"..\..\fl\fl_image.h"\
	"..\..\fl\fl_input.h"\
	"..\..\fl\fl_input_.h"\
	"..\..\fl\fl_pixmap.h"\
	"..\..\fl\fl_plugin.h"\
	"..\..\fl\fl_preferences.h"\
	"..\..\fl\fl_rgb_image.h"\
	"..\..\fl\fl_scrollbar.h"\
	"..\..\fl\fl_shared_image.h"\
	"..\..\fl\fl_slider.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\fl\fl_valuator.h"\
	"..\..\fl\fl_widget.h"\
	"..\..\fl\fl_window.h"\
	"..\..\fl\mac.H"\
	"..\..\fl\win32.h"\
	"..\..\fl\x.h"\
	"..\..\fl\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
NODEP_CPP_FL_HE=\
	"..\..\..\..\usr\include\dirent.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fl_images_core.cxx
DEP_CPP_FL_IM=\
	"..\..\fl\enumerations.h"\
	"..\..\fl\fl_bmp_image.h"\
	"..\..\fl\fl_export.h"\
	"..\..\fl\fl_gif_image.h"\
	"..\..\fl\fl_image.h"\
	"..\..\fl\fl_jpeg_image.h"\
	"..\..\fl\fl_pixmap.h"\
	"..\..\fl\fl_png_image.h"\
	"..\..\fl\fl_pnm_image.h"\
	"..\..\fl\fl_shared_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_JPEG_Image.cxx
DEP_CPP_FL_JP=\
	"..\..\fl\enumerations.h"\
	"..\..\fl\fl_export.h"\
	"..\..\fl\fl_image.h"\
	"..\..\fl\fl_jpeg_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\fl\Xutf8.h"\
	"..\..\jpeg\jconfig.h"\
	"..\..\jpeg\jerror.h"\
	"..\..\jpeg\jmorecfg.h"\
	"..\..\jpeg\jpegint.h"\
	"..\..\jpeg\jpeglib.h"\
	".\config.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_PNG_Image.cxx
DEP_CPP_FL_PN=\
	"..\..\fl\enumerations.h"\
	"..\..\fl\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\fl\fl_export.h"\
	"..\..\fl\fl_image.h"\
	"..\..\fl\fl_png_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\fl\Xutf8.h"\
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
	"..\..\fl\enumerations.h"\
	"..\..\fl\fl.h"\
	"..\..\FL\Fl_Cairo.H"\
	"..\..\fl\fl_export.h"\
	"..\..\fl\fl_image.h"\
	"..\..\fl\fl_pnm_image.h"\
	"..\..\fl\fl_types.h"\
	"..\..\fl\fl_utf8.h"\
	"..\..\fl\Xutf8.h"\
	"..\..\src\flstring.h"\
	".\config.h"\
	
# End Source File
# End Target
# End Project
