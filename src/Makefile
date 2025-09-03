#
# Library Makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2024 by Bill Spitzak and others.
#
# This library is free software. Distribution and use rights are outlined in
# the file "COPYING" which should have been included with this file.  If this
# file is missing or damaged, see the license at:
#
#      https://www.fltk.org/COPYING.php
#
# Please see the following page on how to report bugs and issues:
#
#      https://www.fltk.org/bugs.php
#

# Note:	see ../configure.ac and/or ../makeinclude for definition of
#	FL_VERSION (x.y.z), FL_ABI_VERSION (x.y.0), and FL_DSO_VERSION (x.y)

CPPFILES = \
	Fl.cxx \
	Fl_Adjuster.cxx \
	Fl_Bitmap.cxx \
	Fl_Browser.cxx \
	Fl_Browser_.cxx \
	Fl_Browser_load.cxx \
	Fl_Box.cxx \
	Fl_Button.cxx \
	Fl_Cairo.cxx \
	Fl_Chart.cxx \
	Fl_Check_Browser.cxx \
	Fl_Check_Button.cxx \
	Fl_Choice.cxx \
	Fl_Clock.cxx \
	Fl_Color_Chooser.cxx \
	Fl_Copy_Surface.cxx \
	Fl_Counter.cxx \
	Fl_Dial.cxx \
	Fl_Device.cxx \
	Fl_Double_Window.cxx \
	Fl_File_Browser.cxx \
	Fl_File_Chooser.cxx \
	Fl_File_Chooser2.cxx \
	Fl_File_Icon.cxx \
	Fl_File_Input.cxx \
	Fl_Flex.cxx \
	Fl_Graphics_Driver.cxx \
	Fl_Grid.cxx \
	Fl_Group.cxx \
	Fl_Help_View.cxx \
	Fl_Image.cxx \
	Fl_Image_Surface.cxx \
	Fl_Input.cxx \
	Fl_Input_.cxx \
	Fl_Input_Choice.cxx \
	Fl_Int_Vector.cxx \
	Fl_Light_Button.cxx \
	Fl_Menu.cxx \
	Fl_Menu_.cxx \
	Fl_Menu_Bar.cxx \
	Fl_Menu_Button.cxx \
	Fl_Menu_Window.cxx \
	Fl_Menu_add.cxx \
	Fl_Menu_global.cxx \
	Fl_Message.cxx \
	Fl_Multi_Label.cxx \
	Fl_Native_File_Chooser.cxx \
	Fl_Overlay_Window.cxx \
	Fl_Pack.cxx \
	Fl_Paged_Device.cxx \
	Fl_Pixmap.cxx \
	Fl_Positioner.cxx \
	Fl_Preferences.cxx \
	Fl_Printer.cxx \
	Fl_Progress.cxx \
	Fl_Repeat_Button.cxx \
	Fl_Return_Button.cxx \
	Fl_Roller.cxx \
	Fl_Round_Button.cxx \
	Fl_Screen_Driver.cxx \
	Fl_Scheme.cxx \
	Fl_Scheme_Choice.cxx \
	Fl_Scroll.cxx \
	Fl_Scrollbar.cxx \
	Fl_Shared_Image.cxx \
	Fl_Shortcut_Button.cxx \
	Fl_Single_Window.cxx \
	Fl_Slider.cxx \
	Fl_Spinner.cxx \
	Fl_String.cxx \
	Fl_Sys_Menu_Bar.cxx \
	Fl_System_Driver.cxx \
	Fl_Table.cxx \
	Fl_Table_Row.cxx \
	Fl_Tabs.cxx \
	Fl_Terminal.cxx \
	Fl_Text_Buffer.cxx \
	Fl_Text_Display.cxx \
	Fl_Text_Editor.cxx \
	Fl_Tile.cxx \
	Fl_Tiled_Image.cxx \
	Fl_Timeout.cxx \
	Fl_Tree.cxx \
	Fl_Tree_Item.cxx \
	Fl_Tree_Item_Array.cxx \
	Fl_Tree_Prefs.cxx \
	Fl_Tooltip.cxx \
	Fl_Valuator.cxx \
	Fl_Value_Input.cxx \
	Fl_Value_Output.cxx \
	Fl_Value_Slider.cxx \
	Fl_Widget.cxx \
	Fl_Widget_Surface.cxx \
	Fl_Window.cxx \
	Fl_Window_Driver.cxx \
	Fl_Window_fullscreen.cxx \
	Fl_Window_hotspot.cxx \
	Fl_Window_iconize.cxx \
	Fl_Wizard.cxx \
	Fl_XBM_Image.cxx \
	Fl_XPM_Image.cxx \
	Fl_abort.cxx \
	Fl_add_idle.cxx \
	Fl_arg.cxx \
	Fl_compose.cxx \
	Fl_display.cxx \
	Fl_get_system_colors.cxx \
	Fl_grab.cxx \
	Fl_lock.cxx \
	Fl_own_colormap.cxx \
	Fl_visual.cxx \
	filename_absolute.cxx \
	filename_expand.cxx \
	filename_ext.cxx \
	filename_isdir.cxx \
	filename_list.cxx \
	filename_match.cxx \
	filename_setext.cxx \
	fl_arc.cxx \
	fl_ask.cxx \
	fl_boxtype.cxx \
	fl_color.cxx \
	fl_contrast.cxx \
	fl_cursor.cxx \
	fl_curve.cxx \
	fl_diamond_box.cxx \
	fl_draw.cxx \
	fl_draw_arrow.cxx \
	fl_draw_pixmap.cxx \
	fl_encoding_latin1.cxx \
	fl_encoding_mac_roman.cxx \
	fl_engraved_label.cxx \
	fl_file_dir.cxx \
	fl_font.cxx \
	fl_gleam.cxx \
	fl_gtk.cxx \
	fl_labeltype.cxx \
	fl_open_uri.cxx \
	fl_oval_box.cxx \
	fl_overlay.cxx \
	fl_oxy.cxx \
	fl_plastic.cxx \
	fl_read_image.cxx \
	fl_rect.cxx \
	fl_round_box.cxx \
	fl_rounded_box.cxx \
	fl_set_font.cxx \
	fl_scroll_area.cxx \
	fl_shadow_box.cxx \
	fl_shortcut.cxx \
	fl_show_colormap.cxx \
	fl_string_functions.cxx \
	fl_symbols.cxx \
	fl_utf8.cxx \
	fl_vertex.cxx \
	print_button.cxx \
	screen_xywh.cxx

OBJCPPFILES = \
	Fl_cocoa.mm \
	drivers/Cocoa/Fl_Cocoa_Printer_Driver.mm \
	Fl_Native_File_Chooser_MAC.mm \
	Fl_MacOS_Sys_Menu_Bar.mm

FLCPPFILES = \
	forms_compatibility.cxx \
	forms_bitmap.cxx \
	forms_free.cxx \
	forms_fselect.cxx \
	forms_pixmap.cxx \
	forms_timer.cxx

GLCPPFILES = \
	Fl_Gl_Choice.cxx \
	Fl_Gl_Overlay.cxx \
	Fl_Gl_Device_Plugin.cxx \
	Fl_Gl_Window.cxx \
	freeglut_geometry.cxx \
	freeglut_stroke_mono_roman.cxx \
	freeglut_stroke_roman.cxx \
	freeglut_teapot.cxx \
	gl_draw.cxx \
	gl_start.cxx \
	glut_compatibility.cxx \
	glut_font.cxx \
	drivers/OpenGL/Fl_OpenGL_Display_Device.cxx \
	drivers/OpenGL/Fl_OpenGL_Graphics_Driver_arci.cxx \
	drivers/OpenGL/Fl_OpenGL_Graphics_Driver_color.cxx \
	drivers/OpenGL/Fl_OpenGL_Graphics_Driver_font.cxx \
	drivers/OpenGL/Fl_OpenGL_Graphics_Driver_line_style.cxx \
	drivers/OpenGL/Fl_OpenGL_Graphics_Driver_rect.cxx \
	drivers/OpenGL/Fl_OpenGL_Graphics_Driver_vertex.cxx

GLOBJCPPFILES = drivers/Cocoa/Fl_Cocoa_Gl_Window_Driver.mm
GLCPPFILES_X11 = drivers/X11/Fl_X11_Gl_Window_Driver.cxx
GLCPPFILES_XFT = $(GLCPPFILES_X11)
GLCPPFILES_CAIRO = $(GLCPPFILES_X11)
GLCPPFILES_WIN = drivers/WinAPI/Fl_WinAPI_Gl_Window_Driver.cxx
GLCPPFILES_WAYLAND = drivers/Wayland/Fl_Wayland_Gl_Window_Driver.cxx
GLCPPFILES_WAYLANDX11 = $(GLCPPFILES_WAYLAND) \
		        drivers/X11/Fl_X11_Gl_Window_Driver.cxx

GLCPPFILES += $(GLCPPFILES_$(BUILD))
GLMMFILES_OSX = $(GLOBJCPPFILES)
GLMMFILES = $(GLMMFILES_$(BUILD))

#	the following file currently doesn't contribute code to GLCPPFILES
#	drivers/OpenGL/Fl_OpenGL_Graphics_Driver.cxx

IMGCPPFILES = \
	fl_images_core.cxx \
	fl_write_png.cxx \
	Fl_BMP_Image.cxx \
	Fl_File_Icon2.cxx \
	Fl_GIF_Image.cxx \
	Fl_Anim_GIF_Image.cxx \
	Fl_Help_Dialog.cxx \
	Fl_ICO_Image.cxx \
	Fl_JPEG_Image.cxx \
	Fl_PNG_Image.cxx \
	Fl_PNM_Image.cxx \
	Fl_Image_Reader.cxx \
	Fl_SVG_Image.cxx \
	nanosvg.cxx \
	drivers/SVG/Fl_SVG_File_Surface.cxx

CFILES = flstring.c numericsort.c vsnprintf.c
CFILES_WIN = fl_call_main.c

UTF8CFILES = \
	xutf8/case.c \
	xutf8/is_right2left.c \
	xutf8/is_spacing.c

QUARTZCPPFILES = \
	drivers/Quartz/Fl_Quartz_Graphics_Driver.cxx \
	drivers/Quartz/Fl_Quartz_Graphics_Driver_color.cxx \
	drivers/Quartz/Fl_Quartz_Graphics_Driver_rect.cxx \
	drivers/Quartz/Fl_Quartz_Graphics_Driver_font.cxx \
	drivers/Quartz/Fl_Quartz_Graphics_Driver_vertex.cxx \
	drivers/Quartz/Fl_Quartz_Graphics_Driver_image.cxx \
	drivers/Quartz/Fl_Quartz_Graphics_Driver_arci.cxx \
	drivers/Quartz/Fl_Quartz_Graphics_Driver_line_style.cxx \
	drivers/Quartz/Fl_Quartz_Copy_Surface_Driver.cxx \
	drivers/Quartz/Fl_Quartz_Image_Surface_Driver.cxx \
	drivers/Cocoa/Fl_Cocoa_Window_Driver.cxx \
	drivers/Cocoa/Fl_Cocoa_Screen_Driver.cxx \
        drivers/Posix/Fl_Posix_System_Driver.cxx \
	drivers/Darwin/Fl_Darwin_System_Driver.cxx \
	drivers/Darwin/fl_macOS_platform_init.cxx \
        Fl_get_key_mac.cxx

# These C++ files are used under condition: BUILD_X11
XLIBCPPFILES = \
	drivers/Xlib/Fl_Xlib_Copy_Surface_Driver.cxx \
	drivers/Xlib/Fl_Xlib_Image_Surface_Driver.cxx \
	drivers/X11/Fl_X11_Window_Driver.cxx \
	drivers/X11/Fl_X11_Screen_Driver.cxx \
	drivers/Posix/Fl_Posix_System_Driver.cxx \
        drivers/Unix/Fl_Unix_System_Driver.cxx \
	drivers/Unix/Fl_Unix_Screen_Driver.cxx \
	drivers/Posix/Fl_Posix_Printer_Driver.cxx \
	drivers/X11/fl_X11_platform_init.cxx \
	Fl_x.cxx \
	fl_dnd_x.cxx \
        Fl_Native_File_Chooser_FLTK.cxx \
	Fl_Native_File_Chooser_GTK.cxx\
	Fl_Native_File_Chooser_Kdialog.cxx \
	Fl_Native_File_Chooser_Zenity.cxx \
        Fl_get_key.cxx

# These graphics driver files are used under condition: BUILD_CAIRO
CAIROGDFILES = \
	drivers/Cairo/Fl_Cairo_Graphics_Driver.cxx \
	drivers/Cairo/Fl_X11_Cairo_Graphics_Driver.cxx

# These graphics driver files are used under condition: BUILD_X11 AND BUILD_XFT
XLIBGDFILES = drivers/Xlib/Fl_Xlib_Graphics_Driver.cxx \
	drivers/Xlib/Fl_Xlib_Graphics_Driver_arci.cxx \
	drivers/Xlib/Fl_Xlib_Graphics_Driver_color.cxx \
	drivers/Xlib/Fl_Xlib_Graphics_Driver_image.cxx \
	drivers/Xlib/Fl_Xlib_Graphics_Driver_line_style.cxx \
	drivers/Xlib/Fl_Xlib_Graphics_Driver_rect.cxx \
	drivers/Xlib/Fl_Xlib_Graphics_Driver_vertex.cxx

# These C++ files are used under conditions: BUILD_WAYLAND or BUILD_WAYLANDX11
WLCPPFILES = \
	drivers/Posix/Fl_Posix_Printer_Driver.cxx \
        Fl_Native_File_Chooser_FLTK.cxx \
        Fl_Native_File_Chooser_GTK.cxx \
	Fl_Native_File_Chooser_Kdialog.cxx \
	Fl_Native_File_Chooser_Zenity.cxx \
	drivers/Posix/Fl_Posix_System_Driver.cxx \
        drivers/Unix/Fl_Unix_System_Driver.cxx \
        drivers/Unix/Fl_Unix_Screen_Driver.cxx \
	drivers/Wayland/Fl_Wayland_Screen_Driver.cxx \
        drivers/Wayland/Fl_Wayland_Window_Driver.cxx \
	drivers/Wayland/Fl_Wayland_Image_Surface_Driver.cxx \
	drivers/Wayland/Fl_Wayland_Copy_Surface_Driver.cxx \
	drivers/Wayland/Fl_Wayland_Graphics_Driver.cxx \
	drivers/Wayland/fl_wayland_platform_init.cxx \
	drivers/Wayland/fl_wayland_clipboard_dnd.cxx

# These C++ files are used under condition: BUILD_WAYLANDX11
WLX11CPPFILES = \
	drivers/X11/Fl_X11_Screen_Driver.cxx \
	drivers/X11/Fl_X11_Window_Driver.cxx \
	drivers/Cairo/Fl_X11_Cairo_Graphics_Driver.cxx \
	drivers/Xlib/Fl_Xlib_Copy_Surface_Driver.cxx \
	drivers/Xlib/Fl_Xlib_Image_Surface_Driver.cxx \
	Fl_x.cxx \
	fl_dnd_x.cxx \
	Fl_get_key.cxx


#   fl_dnd_x.cxx  Fl_Native_File_Chooser_GTK.cxx

# This C file is used under condition: BUILD_X11
XLIBCFILES = \
        xutf8/keysym2Ucs.c \
        scandir_posix.c

# These C files are used under condition: BUILD_X11 AND NOT BUILD_XFT
XLIBXCFILES = \
        xutf8/utf8Input.c \
        xutf8/utf8Utils.c \
        xutf8/utf8Wrap.c

# These C++ files are used under condition: BUILD_X11 AND NOT BUILD_XFT
XLIBFONTFILES = \
	drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.cxx

# These C++ files are used under condition: BUILD_X11 AND BUILD_XFT
XLIBXFTFILES = \
	drivers/Xlib/Fl_Xlib_Graphics_Driver_font_xft.cxx \
	drivers/Cairo/Fl_Cairo_Graphics_Driver.cxx

# This C file is used under condition: BUILD_WAYLAND
WLCFILES = \
        xutf8/keysym2Ucs.c \
        scandir_posix.c

# These C++ files are used under conditions: BUILD_WAYLAND or BUILD_WAYLANDX11
WLXFTFILES = \
	drivers/Cairo/Fl_Cairo_Graphics_Driver.cxx

# These C++ files are used under condition: BUILD_GDI
GDICPPFILES = \
	drivers/GDI/Fl_GDI_Graphics_Driver.cxx \
	drivers/GDI/Fl_GDI_Graphics_Driver_arci.cxx \
	drivers/GDI/Fl_GDI_Graphics_Driver_color.cxx \
	drivers/GDI/Fl_GDI_Graphics_Driver_font.cxx \
	drivers/GDI/Fl_GDI_Graphics_Driver_image.cxx \
	drivers/GDI/Fl_GDI_Graphics_Driver_line_style.cxx \
	drivers/GDI/Fl_GDI_Graphics_Driver_rect.cxx \
	drivers/GDI/Fl_GDI_Graphics_Driver_vertex.cxx \
	drivers/GDI/Fl_GDI_Copy_Surface_Driver.cxx \
	drivers/GDI/Fl_GDI_Image_Surface_Driver.cxx \
	drivers/WinAPI/Fl_WinAPI_Window_Driver.cxx \
	drivers/WinAPI/Fl_WinAPI_Screen_Driver.cxx \
	drivers/WinAPI/Fl_WinAPI_System_Driver.cxx \
	drivers/WinAPI/Fl_WinAPI_Printer_Driver.cxx \
	drivers/WinAPI/fl_WinAPI_platform_init.cxx \
	Fl_win32.cxx \
	fl_dnd_win32.cxx \
        Fl_Native_File_Chooser_WIN32.cxx \
        Fl_get_key_win32.cxx

# These C files are used under condition: BUILD_GDI
GDICFILES = \
        scandir_win32.c

PSCPPFILES = \
	drivers/PostScript/Fl_PostScript.cxx \
	drivers/PostScript/Fl_PostScript_image.cxx

################################################################
FLTKFLAGS = -DFL_LIBRARY
include ../makeinclude

# makeinclude has set this variable:
# BUILD = {WIN|X11|XFT|CAIRO|OSX|WAYLAND|WAYLANDX11}

MMFILES_OSX = $(OBJCPPFILES)
MMFILES = $(MMFILES_$(BUILD))

CPPFILES += $(PSCPPFILES)
CPPFILES_OSX = $(QUARTZCPPFILES)

CPPFILES_XFT = $(XLIBCPPFILES) $(XLIBGDFILES) $(XLIBXFTFILES)
CPPFILES_X11 = $(XLIBCPPFILES) $(XLIBGDFILES) $(XLIBFONTFILES)
CPPFILES_CAIRO = $(XLIBCPPFILES) $(CAIROGDFILES)

CPPFILES_WAYLAND = $(WLCPPFILES) $(WLXFTFILES)
CPPFILES_WAYLANDX11 = $(CPPFILES_WAYLAND) $(WLX11CPPFILES)

CPPFILES_WIN = $(GDICPPFILES)

CPPFILES += $(CPPFILES_$(BUILD))


CFILES_X11 = $(XLIBCFILES) $(XLIBXCFILES)
CFILES_CAIRO = $(XLIBCFILES)
CFILES_XFT = $(XLIBCFILES)

CFILES_WAYLANDX11 = $(WLCFILES)
EXTRA_OBJECTS_WAYLAND =  ../libdecor/build/fl_libdecor.o ../libdecor/build/libdecor-cairo-blur.o \
  ../libdecor/build/fl_libdecor-plugins.o \
  xdg-decoration-protocol.o xdg-shell-protocol.o text-input-protocol.o \
  gtk-shell-protocol.o \
  ../libdecor/build/desktop-settings.o ../libdecor/build/os-compatibility.o
EXTRA_OBJECTS_WAYLANDX11 = $(EXTRA_OBJECTS_WAYLAND)
EXTRA_CXXFLAGS_WAYLAND = -I.
EXTRA_CXXFLAGS_WAYLANDX11 = $(EXTRA_CXXFLAGS_WAYLAND)

CFILES_WIN += $(GDICFILES)

CFILES += $(CFILES_$(BUILD))
CXXFLAGS += $(EXTRA_CXXFLAGS_$(BUILD))


OBJECTS =  $(MMFILES:.mm=.o) $(CPPFILES:.cxx=.o) $(CFILES:.c=.o) $(UTF8CFILES:.c=.o)
OBJECTS += $(EXTRA_OBJECTS_$(BUILD))
GLOBJECTS = $(GLCPPFILES:.cxx=.o) $(GLMMFILES:.mm=.o)
FLOBJECTS = $(FLCPPFILES:.cxx=.o)
IMGOBJECTS = $(IMGCPPFILES:.cxx=.o)

all:	$(LIBNAME) $(DSONAME) \
	$(FLLIBNAME) $(FLDSONAME) \
	$(GLLIBNAME) $(GLDSONAME) \
	$(IMGLIBNAME) $(IMGDSONAME)

$(LIBNAME): $(OBJECTS)
	echo $(LIBCOMMAND) $@ ...
	$(RM) $@
	$(LIBCOMMAND) $@ $(OBJECTS)
	$(RANLIB) $@

libfltk.so.$(FL_DSO_VERSION): $(OBJECTS)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(OBJECTS) $(LDFLAGS) $(LDLIBS) $(CAIROLIBS)
	$(RM) libfltk.so
	$(LN) libfltk.so.$(FL_DSO_VERSION) libfltk.so

libfltk.sl.$(FL_DSO_VERSION): $(OBJECTS)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(OBJECTS)
	$(RM) libfltk.sl
	$(LN) libfltk.sl.$(FL_DSO_VERSION) libfltk.sl

libfltk.$(FL_DSO_VERSION).dylib: $(OBJECTS)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ \
		-install_name $(libdir)/$@ \
		-current_version $(FL_VERSION) \
		-compatibility_version $(FL_DSO_VERSION).0 \
		$(OBJECTS) $(LDFLAGS) $(LDLIBS) $(CAIROLIBS)
	$(RM) libfltk.dylib
	$(LN) libfltk.$(FL_DSO_VERSION).dylib libfltk.dylib

libfltk_s.a: $(OBJECTS)
	echo $(DSOCOMMAND) libfltk_s.o ...
	$(DSOCOMMAND) libfltk_s.o $(OBJECTS) $(IMAGELIBS)
	echo $(LIBCOMMAND) libfltk_s.a libfltk_s.o
	$(RM) $@
	$(LIBCOMMAND) libfltk_s.a libfltk_s.o
	$(CHMOD) +x libfltk_s.a

$(FLLIBNAME): $(FLOBJECTS)
	echo $(LIBCOMMAND) $@ ...
	$(RM) $@
	$(LIBCOMMAND) $@ $(FLOBJECTS)
	$(RANLIB) $@

libfltk_forms.so.$(FL_DSO_VERSION): $(FLOBJECTS) libfltk.so.$(FL_DSO_VERSION)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(FLOBJECTS) -L. -lfltk $(LDFLAGS) $(LDLIBS)
	$(RM) libfltk_forms.so
	$(LN) libfltk_forms.so.$(FL_DSO_VERSION) libfltk_forms.so

libfltk_forms.sl.$(FL_DSO_VERSION): $(FLOBJECTS) libfltk.sl.$(FL_DSO_VERSION)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(FLOBJECTS) -L. -lfltk
	$(RM) libfltk_forms.sl
	$(LN) libfltk_forms.sl.$(FL_DSO_VERSION) libfltk_forms.sl

libfltk_forms.$(FL_DSO_VERSION).dylib: $(FLOBJECTS) libfltk.$(FL_DSO_VERSION).dylib
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ \
		-install_name $(libdir)/$@ \
		-current_version $(FL_VERSION) \
		-compatibility_version $(FL_ABI_VERSION) \
		$(FLOBJECTS) -L. $(LDFLAGS) $(LDLIBS) -lfltk
	$(RM) libfltk_forms.dylib
	$(LN) libfltk_forms.$(FL_DSO_VERSION).dylib libfltk_forms.dylib

libfltk_forms_s.a: $(FLOBJECTS)
	echo $(DSOCOMMAND) libfltk_forms_s.o ...
	$(DSOCOMMAND) libfltk_forms_s.o $(FLOBJECTS)
	echo $(LIBCOMMAND) libfltk_forms_s.a libfltk_forms_s.o
	$(RM) $@
	$(LIBCOMMAND) libfltk_forms_s.a libfltk_forms_s.o
	$(CHMOD) +x libfltk_forms_s.a

$(GLLIBNAME): $(GLOBJECTS)
	echo $(LIBCOMMAND) $@ ...
	$(RM) $@
	$(LIBCOMMAND) $@ $(GLOBJECTS)
	$(RANLIB) $@

libfltk_gl.so.$(FL_DSO_VERSION): $(GLOBJECTS) libfltk.so.$(FL_DSO_VERSION)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(GLOBJECTS) -L. -lfltk $(LDFLAGS) $(LDLIBS) $(GLDLIBS)
	$(RM) libfltk_gl.so
	$(LN) libfltk_gl.so.$(FL_DSO_VERSION) libfltk_gl.so

libfltk_gl.sl.$(FL_DSO_VERSION): $(GLOBJECTS) libfltk.sl.$(FL_DSO_VERSION)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(GLOBJECTS) -L. -lfltk
	$(RM) libfltk_gl.sl
	$(LN) libfltk_gl.sl.$(FL_DSO_VERSION) libfltk_gl.sl

libfltk_gl.$(FL_DSO_VERSION).dylib: $(GLOBJECTS) libfltk.$(FL_DSO_VERSION).dylib
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ \
		-install_name $(libdir)/$@ \
		-current_version $(FL_VERSION) \
		-compatibility_version $(FL_ABI_VERSION) \
		$(GLOBJECTS) -L. $(GLDLIBS) -lfltk
	$(RM) libfltk_gl.dylib
	$(LN) libfltk_gl.$(FL_DSO_VERSION).dylib libfltk_gl.dylib

libfltk_gl_s.a: $(GLOBJECTS)
	echo $(DSOCOMMAND) libfltk_gl_s.o ...
	$(DSOCOMMAND) libfltk_gl_s.o $(GLOBJECTS)
	echo $(LIBCOMMAND) libfltk_gl_s.a libfltk_gl_s.o
	$(RM) $@
	$(LIBCOMMAND) libfltk_gl_s.a libfltk_gl_s.o
	$(CHMOD) +x libfltk_gl_s.a

$(IMGLIBNAME): $(IMGOBJECTS)
	echo $(LIBCOMMAND) $@ ...
	$(RM) $@
	$(LIBCOMMAND) $@ $(IMGOBJECTS)
	$(RANLIB) $@

libfltk_images.so.$(FL_DSO_VERSION): $(IMGOBJECTS) libfltk.so.$(FL_DSO_VERSION)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(IMGOBJECTS) -L. $(IMAGELIBS) -lfltk $(LDFLAGS) $(LDLIBS)
	$(RM) libfltk_images.so
	$(LN) libfltk_images.so.$(FL_DSO_VERSION) libfltk_images.so

libfltk_images.sl.$(FL_DSO_VERSION): $(IMGOBJECTS) libfltk.sl.$(FL_DSO_VERSION)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(IMGOBJECTS) -L. $(IMAGELIBS) -lfltk
	$(RM) libfltk_images.sl
	$(LN) libfltk_images.sl.$(FL_DSO_VERSION) libfltk_images.sl

libfltk_images.$(FL_DSO_VERSION).dylib: $(IMGOBJECTS) libfltk.$(FL_DSO_VERSION).dylib
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ \
		-install_name $(libdir)/$@ \
		-current_version $(FL_VERSION) \
		-compatibility_version $(FL_ABI_VERSION) \
		$(IMGOBJECTS)  -L. $(LDFLAGS) $(LDLIBS) $(IMAGELIBS) -lfltk
	$(RM) libfltk_images.dylib
	$(LN) libfltk_images.$(FL_DSO_VERSION).dylib libfltk_images.dylib

libfltk_images_s.a: $(IMGOBJECTS)
	echo $(DSOCOMMAND) libfltk_images_s.o ...
	$(DSOCOMMAND) libfltk_images_s.o $(IMGOBJECTS)
	echo $(LIBCOMMAND) libfltk_images_s.a libfltk_images_s.o
	$(RM) $@
	$(LIBCOMMAND) libfltk_images_s.a libfltk_images_s.o
	$(CHMOD) +x libfltk_images_s.a

#-----------------------------------------------------------------
# - the import libraries libfltk*.dll.a and the .dll files
#   are created from the libfltk*.a files. They are built
#   into the src dir.
# - The _images, _gl, _forms and dlls must be linked
#   against the import libraries in the src dir.
#-----------------------------------------------------------------

#-----------------------------------------------------------------
# cygwin GDI shared libraries
#-----------------------------------------------------------------

cygfltknox-$(FL_DSO_VERSION).dll: $(LIBNAME)
	echo $(DSOCOMMAND) $(LIBNAME) ...
	$(DSOCOMMAND) $(LIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk.dll.a $(LDLIBS)

cygfltknox_forms-$(FL_DSO_VERSION).dll: $(FLLIBNAME) cygfltknox-$(FL_DSO_VERSION).dll
	echo $(DSOCOMMAND) $(FLLIBNAME) ...
	$(DSOCOMMAND) $(FLLIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk_forms.dll.a \
		-L. -lfltk $(LDLIBS)

cygfltknox_gl-$(FL_DSO_VERSION).dll: $(GLLIBNAME) cygfltknox-$(FL_DSO_VERSION).dll
	echo $(DSOCOMMAND) $(GLLIBNAME) ...
	$(DSOCOMMAND) $(GLLIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk_gl.dll.a \
		-L. -lfltk $(GLDLIBS)

cygfltknox_images-$(FL_DSO_VERSION).dll: $(IMGLIBNAME) cygfltknox-$(FL_DSO_VERSION).dll
	echo $(DSOCOMMAND) $(IMGLIBNAME) ...
	$(DSOCOMMAND) $(IMGLIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk_images.dll.a \
		-L. -lfltk -Wl,--exclude-libs -Wl,libfltk_png.a \
			$(IMAGELIBS) $(LDLIBS)

#-----------------------------------------------------------------
# cygwin X11 shared libraries
#-----------------------------------------------------------------

cygfltk-$(FL_DSO_VERSION).dll: $(LIBNAME)
	echo $(DSOCOMMAND) $(LIBNAME) ...
	$(DSOCOMMAND) $(LIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk.dll.a $(LDLIBS)

cygfltk_forms-$(FL_DSO_VERSION).dll: $(FLLIBNAME) cygfltk-$(FL_DSO_VERSION).dll
	echo $(DSOCOMMAND) $(FLLIBNAME) ...
	$(DSOCOMMAND) $(FLLIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk_forms.dll.a \
		-L. -lfltk $(LDLIBS)

cygfltk_gl-$(FL_DSO_VERSION).dll: $(GLLIBNAME) cygfltk-$(FL_DSO_VERSION).dll
	echo $(DSOCOMMAND) $(GLLIBNAME) ...
	$(DSOCOMMAND) $(GLLIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk_gl.dll.a \
		-L. -lfltk $(GLDLIBS)

cygfltk_images-$(FL_DSO_VERSION).dll: $(IMGLIBNAME) cygfltk-$(FL_DSO_VERSION).dll
	echo $(DSOCOMMAND) $(IMGLIBNAME) ...
	$(DSOCOMMAND) $(IMGLIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk_images.dll.a \
		-L. -lfltk -Wl,--exclude-libs -Wl,libfltk_png.a \
			$(IMAGELIBS) $(LDLIBS)

mgwfltknox-$(FL_DSO_VERSION).dll: $(LIBNAME)
	echo $(DSOCOMMAND) $(LIBNAME) ...
	$(DSOCOMMAND) $(LIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk.dll.a $(LDLIBS)

mgwfltknox_forms-$(FL_DSO_VERSION).dll: $(FLLIBNAME) mgwfltknox-$(FL_DSO_VERSION).dll
	echo $(DSOCOMMAND) $(FLLIBNAME) ...
	$(DSOCOMMAND) $(FLLIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk_forms.dll.a \
		-L. -lfltk $(LDLIBS)

mgwfltknox_gl-$(FL_DSO_VERSION).dll: $(GLLIBNAME) mgwfltknox-$(FL_DSO_VERSION).dll
	echo $(DSOCOMMAND) $(GLLIBNAME) ...
	$(DSOCOMMAND) $(GLLIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk_gl.dll.a \
		-L. -lfltk $(GLDLIBS)

#-----------------------------------------------------
# See STR #1585 for --exclude-libs
#-----------------------------------------------------

mgwfltknox_images-$(FL_DSO_VERSION).dll: $(IMGLIBNAME) mgwfltknox-$(FL_DSO_VERSION).dll
	echo $(DSOCOMMAND) $(IMGLIBNAME) ...
	$(DSOCOMMAND) $(IMGLIBNAME) -Wl,--no-whole-archive \
		-Wl,--out-implib=libfltk_images.dll.a \
		-L. -lfltk -Wl,--exclude-libs -Wl,libfltk_png.a \
			$(IMAGELIBS) $(LDLIBS)

# Note: The rebuild target can only be used if you have the original .fl
#	files. This is normally only used by the FLTK maintainers...
#	It *must* be executed *after* fluid has been built and
#	the libs must be rebuilt if any {.fl|.cxx|.h} files were changed.

rebuild:
	echo 'Rebuilding fluid (.fl) and .cxx/.h files from .fl files ...'
	../fluid/fluid$(EXEEXT) -u -c Fl_File_Chooser.fl
	../fluid/fluid$(EXEEXT) -u -c Fl_Help_Dialog.fl

clean:
	-$(RM)	*.o xutf8/*.o *.dll.a core.* *~ *.bak *.bck
	-$(RM)	drivers/Cairo/*.o
	-$(RM)	drivers/Cocoa/*.o
	-$(RM)	drivers/Darwin/*.o
	-$(RM)	drivers/GDI/*.o
	-$(RM)	drivers/OpenGL/*.o
	-$(RM)	drivers/Posix/*.o
	-$(RM)	drivers/PostScript/*.o
	-$(RM)	drivers/Quartz/*.o
	-$(RM)	drivers/SVG/*.o
	-$(RM)	drivers/Unix/*.o
	-$(RM)	drivers/WinAPI/*.o
	-$(RM)	drivers/X11/*.o
	-$(RM)	drivers/Xlib/*.o
	-$(RM)	drivers/Wayland/*.o
	-$(RM)	$(DSONAME) $(FLDSONAME) $(GLDSONAME) $(IMGDSONAME) \
		$(LIBNAME) $(FLLIBNAME) $(GLLIBNAME) \
		$(IMGLIBNAME) \
		libfltk.so libfltk_forms.so libfltk_gl.so libfltk_images.so \
		libfltk.sl libfltk_forms.sl libfltk_gl.sl libfltk_images.sl \
		libfltk.dylib libfltk_forms.dylib \
		libfltk_gl.dylib libfltk_images.dylib \
		cmap core

depend:	$(CPPFILES) $(MMFILES) $(FLCPPFILES) $(GLCPPFILES) $(IMGCPPFILES) $(CFILES) $(UTF8CFILES) $(XLIBCFILES)
	makedepend -Y -I.. -f makedepend -w 20 $(CPPFILES) $(MMFILES) $(FLCPPFILES) \
		$(GLCPPFILES) $(IMGCPPFILES) $(CFILES) $(UTF8CFILES)  $(XLIBCFILES)
	echo "# DO NOT DELETE THIS LINE -- make depend depends on it." > makedepend.tmp
	echo "" >> makedepend.tmp
	grep '^[a-zA-Z]' makedepend | ( LC_ALL=C sort -u -f >> makedepend.tmp; )
	mv makedepend.tmp makedepend

# Automatically generated dependencies... generated on a Linux/Unix host !
include makedepend

# These dependencies aren't part of the makedepend file since
# they are part of the Windows and MacOS code base...
# Please add only non-Linux/Unix files or such that are optional
# (like "*non-Xft*") here:
Fl_get_key_mac.o:	Fl_get_key_mac.cxx
Fl_get_key_win32.o:	Fl_get_key_win32.cxx
scandir_win32.o:   scandir_win32.c
Fl_Native_File_Chooser_WIN32.o: Fl_Native_File_Chooser_WIN32.cxx
Fl_Native_File_Chooser_MAC.o:	Fl_Native_File_Chooser_MAC.mm
Fl_Native_File_Chooser_FLTK.o:	Fl_Native_File_Chooser_FLTK.cxx
Fl_Native_File_Chooser_GTK.o:	Fl_Native_File_Chooser_GTK.cxx
Fl_win32.o:		Fl_win32.cxx
Fl_cocoa.o:	Fl_cocoa.mm
fl_color.o:
fl_dnd_win32.o:	fl_dnd_win32.cxx
fl_dnd_x.o:	fl_dnd_x.cxx
Fl_Printer.o: ../src/drivers/PostScript/Fl_PostScript.cxx
Fl_Quartz_Printer.o:	Fl_Quartz_Printer.mm
fl_arci.o:	../FL/mac.H ../FL/win32.H
Fl_arg.o:	../FL/mac.H ../FL/win32.H
fl_ask.o:	../FL/mac.H ../FL/win32.H
Fl_Bitmap.o:	../FL/mac.H ../FL/win32.H
fl_color.o:	../FL/mac.H ../FL/win32.H
fl_cursor.o:	../FL/mac.H ../FL/win32.H
Fl_Double_Window.o:	../FL/mac.H ../FL/win32.H
fl_draw_image.o:	../FL/mac.H ../FL/win32.H
fl_draw_pixmap.o:	../FL/mac.H ../FL/win32.H
fl_encoding_latin1.o:	../FL/mac.H ../FL/win32.H
fl_encoding_mac_roman.o:   ../FL/mac.H ../FL/win32.H
Fl_File_Chooser2.o:	../FL/mac.H ../FL/win32.H
fl_font.o:	../FL/mac.H ../FL/win32.H
Fl_get_system_colors.o:	../FL/mac.H ../FL/win32.H
Fl_Gl_Choice.o:	../FL/mac.H ../FL/win32.H
Fl_Gl_Overlay.o:	../FL/mac.H ../FL/win32.H
Fl_Gl_Window.o:	../FL/mac.H ../FL/win32.H
Fl_grab.o:	../FL/mac.H ../FL/win32.H
Fl_Image.o:	../FL/mac.H ../FL/win32.H
fl_line_style.o:	../FL/mac.H ../FL/win32.H
Fl_Menu_Window.o:	../FL/mac.H ../FL/win32.H
fl_overlay.o:	../FL/mac.H ../FL/win32.H
Fl_Overlay_Window.o:	../FL/mac.H ../FL/win32.H
Fl_own_colormap.o:	../FL/mac.H ../FL/win32.H
Fl_Pixmap.o:	../FL/mac.H ../FL/win32.H
Fl_Printer.o:  ../FL/mac.H ../FL/win32.H
fl_rect.o:	../FL/mac.H ../FL/win32.H
fl_scroll_area.o:	../FL/mac.H ../FL/win32.H
fl_set_font.o:	../FL/mac.H ../FL/win32.H
fl_set_fonts.o:	../FL/mac.H ../FL/win32.H
fl_shortcut.o:	../FL/mac.H ../FL/win32.H
fl_vertex.o:	../FL/mac.H ../FL/win32.H
Fl_visual.o:	../FL/mac.H ../FL/win32.H
Fl_Window_fullscreen.o:	../FL/mac.H ../FL/win32.H
Fl_Window_iconize.o:	../FL/mac.H ../FL/win32.H
gl_draw.o:	../FL/mac.H ../FL/win32.H
gl_start.o:	../FL/mac.H ../FL/win32.H
# these additional dependencies are for non-Xft builds on Linux/Unix
drivers/X11/Fl_X11_Screen_Driver.o: Xutf8.h
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../config.h
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: drivers/Xlib/Fl_Font.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: drivers/Xlib/Fl_Xlib_Graphics_Driver.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/fl_config.h
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/Enumerations.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/Fl_Bitmap.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/Fl_Device.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/fl_draw.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/Fl_Export.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/Fl_Graphics_Driver.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/Fl.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/Fl_Image.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/Fl_Pixmap.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/Fl_Plugin.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/Fl_Preferences.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/Fl_RGB_Image.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/fl_types.h
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/fl_utf8.h
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/Fl_Widget.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/platform.H
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: ../FL/platform_types.h
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: flstring.h
drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.o: Xutf8.h
drivers/Xlib/Fl_Xlib_Graphics_Driver.o: Xutf8.h
gl_draw.o: Xutf8.h
xutf8/utf8Utils.o: ../FL/Fl_Export.H
xutf8/utf8Utils.o: Xutf8.h
xutf8/utf8Wrap.o: ../FL/Fl_Export.H
xutf8/utf8Wrap.o: utf8_internal.h
xutf8/utf8Wrap.o: Xutf8.h
xutf8/utf8Wrap.o: xutf8/headers/dingbats_.h
xutf8/utf8Wrap.o: xutf8/headers/symbol_.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/big5.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/cp1251.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/cp936ext.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/gb2312.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/iso8859_10.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/iso8859_11.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/iso8859_13.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/iso8859_14.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/iso8859_15.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/iso8859_2.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/iso8859_3.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/iso8859_4.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/iso8859_5.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/iso8859_6.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/iso8859_7.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/iso8859_8.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/iso8859_9.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/jisx0201.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/jisx0208.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/jisx0212.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/koi8_r.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/koi8_u.h
xutf8/utf8Wrap.o: xutf8/lcUniConv/ksc5601.h
xutf8/utf8Wrap.o: xutf8/ucs2fontmap.c

################################################################

install: $(LIBNAME) $(DSONAME) \
	$(FLLIBNAME) $(FLDSONAME) \
	$(GLLIBNAME) $(GLDSONAME) \
	$(IMGLIBNAME) $(IMGDSONAME)
	echo "Installing libraries in $(DESTDIR)$(libdir)..."
	-$(INSTALL_DIR) "$(DESTDIR)$(libdir)"
	-$(INSTALL_DIR) "$(DESTDIR)$(bindir)"
	$(RM) "$(DESTDIR)$(libdir)/$(LIBBASENAME)"
	$(INSTALL_LIB) $(LIBNAME) "$(DESTDIR)$(libdir)"
	$(INSTALL_LIB) $(FLLIBNAME) "$(DESTDIR)$(libdir)"
	$(INSTALL_LIB) $(IMGLIBNAME) "$(DESTDIR)$(libdir)"
	$(RANLIB) "$(DESTDIR)$(libdir)/$(LIBBASENAME)"
	$(RANLIB) "$(DESTDIR)$(libdir)/$(FLLIBBASENAME)"
	if test x$(GLLIBNAME) != x; then \
		$(INSTALL_LIB) $(GLLIBNAME) "$(DESTDIR)$(libdir)"; \
		$(RANLIB) "$(DESTDIR)$(libdir)/$(GLLIBBASENAME)"; \
	fi
	$(RANLIB) "$(DESTDIR)$(libdir)/$(IMGLIBBASENAME)"
	if test x$(DSONAME) = xlibfltk.so.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk.so*;\
		$(INSTALL_LIB) libfltk.so.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)"; \
		$(LN) libfltk.so.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)/libfltk.so";\
	fi
	if test x$(DSONAME) = xlibfltk.sl.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk.sl*;\
		$(INSTALL_LIB) libfltk.sl.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)"; \
		$(LN) libfltk.sl.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)/libfltk.sl";\
	fi
	if test x$(DSONAME) = xlibfltk.$(FL_DSO_VERSION).dylib; then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk.*dylib;\
		$(INSTALL_LIB) libfltk.$(FL_DSO_VERSION).dylib "$(DESTDIR)$(libdir)"; \
		$(LN) libfltk.$(FL_DSO_VERSION).dylib "$(DESTDIR)$(libdir)/libfltk.dylib";\
	fi
	if test x$(DSONAME) = xlibfltk_s.a; then\
		$(RM) "$(DESTDIR)$(libdir)/libfltk_s.a";\
		$(INSTALL_LIB) libfltk_s.a "$(DESTDIR)$(libdir)"; \
	fi
	if test x$(DSONAME) = xcygfltknox-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(DSONAME)";\
		$(INSTALL_LIB) $(DSONAME) "$(DESTDIR)$(bindir)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk.dll.a";\
		$(INSTALL_LIB) libfltk.dll.a "$(DESTDIR)$(libdir)"; \
	fi
	if test x$(DSONAME) = xmgwfltknox-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(DSONAME)";\
		$(INSTALL_LIB) $(DSONAME) "$(DESTDIR)$(bindir)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk.dll.a";\
		$(INSTALL_LIB) libfltk.dll.a "$(DESTDIR)$(libdir)"; \
	fi
	if test x$(FLDSONAME) = xlibfltk_forms.so.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_forms.so*;\
		$(INSTALL_LIB) libfltk_forms.so.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)"; \
		$(LN) libfltk_forms.so.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)/libfltk_forms.so";\
	fi
	if test x$(FLDSONAME) = xlibfltk_forms.sl.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_forms.sl*;\
		$(INSTALL_LIB) libfltk_forms.sl.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)"; \
		$(LN) libfltk_forms.sl.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)/libfltk_forms.sl";\
	fi
	if test x$(FLDSONAME) = xlibfltk_forms.$(FL_DSO_VERSION).dylib; then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_forms.*dylib;\
		$(INSTALL_LIB) libfltk_forms.$(FL_DSO_VERSION).dylib "$(DESTDIR)$(libdir)"; \
		$(LN) libfltk_forms.$(FL_DSO_VERSION).dylib "$(DESTDIR)$(libdir)/libfltk_forms.dylib";\
	fi
	if test x$(FLDSONAME) = xlibfltk_forms_s.a; then\
		$(RM) "$(DESTDIR)$(libdir)/libfltk_forms_s.a";\
		$(INSTALL_LIB) libfltk_forms_s.a "$(DESTDIR)$(libdir)"; \
	fi
	if test x$(FLDSONAME) = xcygfltknox_forms-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(FLDSONAME)";\
		$(INSTALL_LIB) $(FLDSONAME) "$(DESTDIR)$(bindir)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk_forms.dll.a";\
		$(INSTALL_LIB) libfltk_forms.dll.a "$(DESTDIR)$(libdir)"; \
	fi
	if test x$(FLDSONAME) = xmgwfltknox_forms-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(FLDSONAME)";\
		$(INSTALL_LIB) $(FLDSONAME) "$(DESTDIR)$(bindir)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk_forms.dll.a";\
		$(INSTALL_LIB) libfltk_forms.dll.a "$(DESTDIR)$(libdir)"; \
	fi
	if test x$(GLDSONAME) = xlibfltk_gl.so.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_gl.so*;\
		$(INSTALL_LIB) libfltk_gl.so.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)"; \
		$(LN) libfltk_gl.so.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)/libfltk_gl.so";\
	fi
	if test x$(GLDSONAME) = xlibfltk_gl.sl.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_gl.sl*;\
		$(INSTALL_LIB) libfltk_gl.sl.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)"; \
		$(LN) libfltk_gl.sl.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)/libfltk_gl.sl";\
	fi
	if test x$(GLDSONAME) = xlibfltk_gl.$(FL_DSO_VERSION).dylib; then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_gl.*dylib;\
		$(INSTALL_LIB) libfltk_gl.$(FL_DSO_VERSION).dylib "$(DESTDIR)$(libdir)"; \
		$(LN) libfltk_gl.$(FL_DSO_VERSION).dylib "$(DESTDIR)$(libdir)/libfltk_gl.dylib";\
	fi
	if test x$(GLDSONAME) = xlibfltk_gl_s.a; then\
		$(RM) "$(DESTDIR)$(libdir)/libfltk_gl_s.a";\
		$(INSTALL_LIB) libfltk_gl_s.a "$(DESTDIR)$(libdir)"; \
	fi
	if test x$(GLDSONAME) = xcygfltknox_gl-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(GLDSONAME)";\
		$(INSTALL_LIB) $(GLDSONAME) "$(DESTDIR)$(bindir)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk_gl.dll.a";\
		$(INSTALL_LIB) libfltk_gl.dll.a "$(DESTDIR)$(libdir)"; \
	fi
	if test x$(GLDSONAME) = xmgwfltknox_gl-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(GLDSONAME)";\
		$(INSTALL_LIB) $(GLDSONAME) "$(DESTDIR)$(bindir)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk_gl.dll.a";\
		$(INSTALL_LIB) libfltk_gl.dll.a "$(DESTDIR)$(libdir)"; \
	fi
	if test x$(IMGDSONAME) = xlibfltk_images.so.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_images.so*;\
		$(INSTALL_LIB) libfltk_images.so.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)"; \
		$(LN) libfltk_images.so.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)/libfltk_images.so";\
	fi
	if test x$(IMGDSONAME) = xlibfltk_images.sl.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_images.sl*;\
		$(INSTALL_LIB) libfltk_images.sl.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)"; \
		$(LN) libfltk_images.sl.$(FL_DSO_VERSION) "$(DESTDIR)$(libdir)/libfltk_images.sl";\
	fi
	if test x$(IMGDSONAME) = xlibfltk_images.$(FL_DSO_VERSION).dylib; then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_images.*dylib;\
		$(INSTALL_LIB) libfltk_images.$(FL_DSO_VERSION).dylib "$(DESTDIR)$(libdir)"; \
		$(LN) libfltk_images.$(FL_DSO_VERSION).dylib "$(DESTDIR)$(libdir)/libfltk_images.dylib";\
	fi
	if test x$(IMGDSONAME) = xlibfltk_images_s.a; then\
		$(RM) "$(DESTDIR)$(libdir)/libfltk_images_s.a";\
		$(INSTALL_LIB) libfltk_images_s.a "$(DESTDIR)$(libdir)"; \
	fi
	if test x$(IMGDSONAME) = xcygfltknox_images-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(IMGDSONAME)"; \
		$(INSTALL_LIB) $(IMGDSONAME) "$(DESTDIR)$(bindir)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk_images.dll.a";\
		$(INSTALL_LIB) libfltk_images.dll.a "$(DESTDIR)$(libdir)"; \
	fi
	if test x$(IMGDSONAME) = xmgwfltknox_images-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(IMGDSONAME)"; \
		$(INSTALL_LIB) $(IMGDSONAME) "$(DESTDIR)$(bindir)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk_images.dll.a";\
		$(INSTALL_LIB) libfltk_images.dll.a "$(DESTDIR)$(libdir)"; \
	fi


uninstall:
	echo "Uninstalling libraries..."
	$(RM) "$(DESTDIR)$(libdir)/$(LIBBASENAME)"
	if test x$(DSONAME) = xlibfltk.so.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk.so*;\
	fi
	if test x$(DSONAME) = xlibfltk.sl.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk.sl*;\
	fi
	if test x$(DSONAME) = xlibfltk.$(FL_DSO_VERSION).dylib; then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk.*dylib;\
	fi
	if test x$(DSONAME) = xlibfltk_s.a; then\
		$(RM) "$(DESTDIR)$(libdir)/libfltk_s.a";\
	fi
	if test x$(DSONAME) = xcygfltknox-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(DSONAME)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk.dll.a";\
	fi
	if test x$(DSONAME) = xcygfltk-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(DSONAME)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk.dll.a";\
	fi
	if test x$(DSONAME) = xmgwfltknox-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(DSONAME)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk.dll.a";\
	fi
	$(RM) "$(DESTDIR)$(libdir)/$(FLLIBBASENAME)";
	if test x$(FLDSONAME) = xlibfltk_forms.so.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_forms.so*;\
	fi
	if test x$(FLDSONAME) = xlibfltk_forms.sl.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_forms.sl*;\
	fi
	if test x$(FLDSONAME) = xlibfltk_forms.$(FL_DSO_VERSION).dylib; then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_forms.*dylib;\
	fi
	if test x$(FLDSONAME) = xlibfltk_forms_s.a; then\
		$(RM) "$(DESTDIR)$(libdir)/libfltk_forms_s.a";\
	fi
	if test x$(FLDSONAME) = xcygfltknox_forms-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(FLDSONAME)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk_forms.dll.a";\
	fi
	if test x$(FLDSONAME) = xcygfltk_forms-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(FLDSONAME)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk_forms.dll.a";\
	fi
	if test x$(FLDSONAME) = xmgwfltknox_forms-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(FLDSONAME)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk_forms.dll.a";\
	fi
	if test x$(GLLIBNAME) != x; then\
		$(RM) "$(DESTDIR)$(libdir)/$(GLLIBBASENAME)";\
	fi
	if test x$(GLDSONAME) = xlibfltk_gl.so.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_gl.so*;\
	fi
	if test x$(GLDSONAME) = xlibfltk_gl.sl.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_gl.sl*;\
	fi
	if test x$(GLDSONAME) = xlibfltk_gl.$(FL_DSO_VERSION).dylib; then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_gl.*dylib;\
	fi
	if test x$(GLDSONAME) = xlibfltk_gl_s.a; then\
		$(RM) "$(DESTDIR)$(libdir)/libfltk_gl_s.a";\
	fi
	if test x$(GLDSONAME) = xcygfltknox_gl-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(GLDSONAME)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk_gl.dll.a";\
	fi
	if test x$(GLDSONAME) = xcygfltk_gl-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(GLDSONAME)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk_gl.dll.a";\
	fi
	if test x$(GLDSONAME) = xmgwfltknox_gl-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(GLDSONAME)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk_gl.dll.a";\
	fi
	if test x$(IMGLIBNAME) != x; then\
		$(RM) "$(DESTDIR)$(libdir)/$(IMGLIBBASENAME)";\
	fi
	if test x$(IMGDSONAME) = xlibfltk_images.so.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_images.so*;\
	fi
	if test x$(IMGDSONAME) = xlibfltk_images.sl.$(FL_DSO_VERSION); then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_images.sl*;\
	fi
	if test x$(IMGDSONAME) = xlibfltk_images.$(FL_DSO_VERSION).dylib; then\
		$(RM) "$(DESTDIR)$(libdir)"/libfltk_images.*dylib;\
	fi
	if test x$(IMGDSONAME) = xlibfltk_images_s.a; then\
		$(RM) "$(DESTDIR)$(libdir)/libfltk_images_s.a";\
	fi
	if test x$(IMGDSONAME) = xcygfltknox_images-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(IMGDSONAME)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk_images.dll.a";\
	fi
	if test x$(IMGDSONAME) = xcygfltk_images-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(IMGDSONAME)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk_images.dll.a";\
	fi
	if test x$(IMGDSONAME) = xmgwfltknox_images-$(FL_DSO_VERSION).dll; then\
		$(RM) "$(DESTDIR)$(bindir)/$(IMGDSONAME)"; \
		$(RM) "$(DESTDIR)$(libdir)/libfltk_images.dll.a";\
	fi
