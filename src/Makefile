#
# "$Id: Makefile,v 1.18.2.14.2.23 2001/12/06 18:23:29 easysw Exp $"
#
# Library makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2001 by Bill Spitzak and others.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.
#
# Please report all bugs and problems to "fltk-bugs@fltk.org".
#

CPPFILES = \
	Fl.cxx \
	Fl_Adjuster.cxx \
	Fl_Bitmap.cxx \
	Fl_Browser.cxx \
	Fl_Browser_.cxx \
	Fl_Browser_load.cxx \
	Fl_Box.cxx \
	Fl_Button.cxx \
	Fl_Chart.cxx \
	Fl_Check_Browser.cxx \
	Fl_Check_Button.cxx \
	Fl_Choice.cxx \
	Fl_Clock.cxx \
	Fl_Color_Chooser.cxx \
	Fl_Counter.cxx \
	Fl_Dial.cxx \
	Fl_Double_Window.cxx \
	Fl_File_Browser.cxx \
	Fl_File_Chooser.cxx \
	Fl_File_Chooser2.cxx \
	Fl_File_Icon.cxx \
	Fl_File_Icon2.cxx \
	Fl_GIF_Image.cxx \
	Fl_Group.cxx \
	Fl_Help_Dialog.cxx \
	Fl_Help_View.cxx \
	Fl_Image.cxx \
	Fl_Input.cxx \
	Fl_Input_.cxx \
	Fl_JPEG_Image.cxx \
	Fl_Light_Button.cxx \
	Fl_Menu.cxx \
	Fl_Menu_.cxx \
	Fl_Menu_Bar.cxx \
	Fl_Menu_Button.cxx \
	Fl_Menu_Window.cxx \
	Fl_Menu_add.cxx \
	Fl_Menu_global.cxx \
	Fl_Multi_Label.cxx \
	Fl_Output.cxx \
	Fl_Overlay_Window.cxx \
	Fl_Pack.cxx \
	Fl_Pixmap.cxx \
	Fl_PNG_Image.cxx \
	Fl_PNM_Image.cxx \
	Fl_Positioner.cxx \
	Fl_Progress.cxx \
	Fl_Repeat_Button.cxx \
	Fl_Return_Button.cxx \
	Fl_Roller.cxx \
	Fl_Round_Button.cxx \
	Fl_Scroll.cxx \
	Fl_Scrollbar.cxx \
	Fl_Shared_Image.cxx \
	Fl_Single_Window.cxx \
	Fl_Slider.cxx \
	Fl_Tabs.cxx \
	Fl_Text_Buffer.cxx \
	Fl_Text_Display.cxx \
	Fl_Text_Editor.cxx \
	Fl_Tile.cxx \
	Fl_Tiled_Image.cxx \
	Fl_Tooltip.cxx \
	Fl_Valuator.cxx \
	Fl_Value_Input.cxx \
	Fl_Value_Output.cxx \
	Fl_Value_Slider.cxx \
	Fl_Widget.cxx \
	Fl_Window.cxx \
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
	Fl_cutpaste.cxx \
	Fl_display.cxx \
	Fl_get_key.cxx \
	Fl_get_system_colors.cxx \
	Fl_grab.cxx \
	Fl_own_colormap.cxx \
	Fl_visual.cxx \
	Fl_x.cxx \
	filename_absolute.cxx \
	filename_expand.cxx \
	filename_ext.cxx \
	filename_isdir.cxx \
	filename_list.cxx \
	filename_match.cxx \
	filename_setext.cxx \
	fl_arc.cxx \
	fl_arci.cxx \
	fl_ask.cxx \
	fl_boxtype.cxx \
	fl_color.cxx \
	fl_cursor.cxx \
	fl_curve.cxx \
	fl_diamond_box.cxx \
	fl_draw.cxx \
	fl_draw_image.cxx \
	fl_draw_pixmap.cxx \
	fl_engraved_label.cxx \
	fl_file_dir.cxx \
	fl_font.cxx \
	fl_labeltype.cxx \
	fl_line_style.cxx \
	fl_oval_box.cxx \
	fl_overlay.cxx \
	fl_overlay_visual.cxx \
	fl_plastic.cxx \
	fl_rect.cxx \
	fl_round_box.cxx \
	fl_rounded_box.cxx \
	fl_set_font.cxx \
	fl_set_fonts.cxx \
	fl_scroll_area.cxx \
	fl_shadow_box.cxx \
	fl_shortcut.cxx \
	fl_show_colormap.cxx \
	fl_symbols.cxx \
	fl_vertex.cxx \
	forms_compatability.cxx \
	forms_bitmap.cxx \
	forms_free.cxx \
	forms_fselect.cxx \
	forms_pixmap.cxx \
	forms_timer.cxx

GLCPPFILES = \
	Fl_Gl_Choice.cxx \
	Fl_Gl_Overlay.cxx \
	Fl_Gl_Window.cxx \
	gl_draw.cxx \
	gl_start.cxx \
	glut_compatability.cxx \
	glut_font.cxx

CFILES = fl_call_main.c scandir.c numericsort.c vsnprintf.c

CLEAN =

################################################################

include ../makeinclude

OBJECTS = $(CPPFILES:.cxx=.o) $(CFILES:.c=.o)
GLOBJECTS = $(GLCPPFILES:.cxx=.o)

all:	$(LIBNAME) $(DSONAME) $(GLLIBNAME) $(GLDSONAME)

$(LIBNAME): $(OBJECTS)
	echo $(LIBCOMMAND) $@ ...
	-$(RM) $@
	$(LIBCOMMAND) $@ $(OBJECTS)
	$(RANLIB) $@

libfltk.so.1.1 libfltk.sl.1.1: $(OBJECTS)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(OBJECTS) $(IMAGELIBS)

libfltk_s.a: $(OBJECTS)
	echo $(DSOCOMMAND) libfltk_s.o ...
	$(DSOCOMMAND) libfltk_s.o $(OBJECTS) $(IMAGELIBS)
	echo $(LIBCOMMAND) libfltk_s.a libfltk_s.o
	-$(RM) $@
	$(LIBCOMMAND) libfltk_s.a libfltk_s.o
	chmod +x libfltk_s.a

$(GLLIBNAME): $(GLOBJECTS)
	echo $(LIBCOMMAND) $@ ...
	-$(RM) $@
	$(LIBCOMMAND) $@ $(GLOBJECTS)
	$(RANLIB) $@

libfltk_gl.so.1.1 libfltk_gl.sl.1.1: $(GLOBJECTS)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(GLOBJECTS)

libfltk_gl_s.a: $(GLOBJECTS)
	echo $(DSOCOMMAND) libfltk_gl_s.o ...
	$(DSOCOMMAND) libfltk_gl_s.o $(GLOBJECTS)
	echo $(LIBCOMMAND) libfltk_gl_s.a libfltk_gl_s.o
	-$(RM) $@
	$(LIBCOMMAND) libfltk_gl_s.a libfltk_gl_s.o
	chmod +x libfltk_gl_s.a

clean :
	-@ rm -f *.o *.do $(DSONAME) $(LIBRARY) $(CLEAN) core *~ ../include/*~ makedepend cmap
	touch makedepend

depend:
#	$(MAKEDEPEND) -I.. $(CXXFLAGS) $(CPPFILES) $(CFILES) > makedepend
	makedepend -Y -I.. -f makedepend $(CPPFILES) $(GLCPPFILES) $(CFILES)

# Automatically generated dependencies...
include makedepend

# These dependencies aren't part of the makedepend file since
# they are part of the WIN32 and MacOS code base...
Fl_cutpaste.o:	Fl_cutpaste_mac.cxx Fl_cutpaste_win32.cxx
Fl_get_key.o:	Fl_get_key_mac.cxx Fl_get_key_win32.cxx
Fl_x.o:		Fl_mac.cxx Fl_win32.cxx
fl_color.o:	fl_color_mac.cxx fl_color_win32.cxx
fl_draw_image.o:	fl_draw_image_mac.cxx fl_draw_image_win32.cxx
fl_font.o:	fl_font_mac.cxx fl_font_win32.cxx
fl_set_fonts.o:	fl_set_fonts_mac.cxx fl_set_fonts_win32.cxx


################################################################

install: $(LIBNAME) $(DSONAME) $(GLLIBNAME) $(GLDSONAME)
	echo "Installing libraries..."
	-mkdir -p $(libdir)
	rm -f $(libdir)/$(LIBNAME)
	-cp $(LIBNAME) $(GLLIBNAME) $(libdir)
	$(RANLIB) $(libdir)/$(LIBNAME)
	$(RANLIB) $(libdir)/$(GLLIBNAME)
	if test x$(DSONAME) = xlibfltk.so.1.1; then\
		rm -f $(libdir)/libfltk.so*;\
		cp libfltk.so.1.1 $(libdir); \
		ln -s libfltk.so.1.1 $(libdir)/libfltk.so;\
	fi
	if test x$(DSONAME) = xlibfltk.sl.1.1; then\
		rm -f $(libdir)/libfltk.sl*;\
		cp libfltk.sl.1.1 $(libdir); \
		ln -s libfltk.sl.1.1 $(libdir)/libfltk.sl;\
	fi
	if test x$(DSONAME) = xlibfltk_s.a; then\
		rm -f $(libdir)/libfltk_s.a;\
		cp libfltk_s.a $(libdir); \
	fi
	if test x$(GLDSONAME) = xlibfltk_gl.so.1.1; then\
		rm -f $(libdir)/libfltk_gl.so*;\
		cp libfltk_gl.so.1.1 $(libdir); \
		ln -s libfltk_gl.so.1.1 $(libdir)/libfltk_gl.so;\
	fi
	if test x$(GLDSONAME) = xlibfltk_gl.sl.1.1; then\
		rm -f $(libdir)/libfltk_gl.sl*;\
		cp libfltk_gl.sl.1.1 $(libdir); \
		ln -s libfltk_gl.sl.1.1 $(libdir)/libfltk_gl.sl;\
	fi
	if test x$(GLDSONAME) = xlibfltk_gl_s.a; then\
		rm -f $(libdir)/libfltk_gl_s.a;\
		cp libfltk_gl_s.a $(libdir); \
	fi


uninstall:
	echo "Uninstalling libraries..."
	rm -f $(libdir)/$(LIBNAME)
	if test x$(DSONAME) = xlibfltk.so.1.1; then\
		rm -f $(libdir)/libfltk.so*;\
	fi
	if test x$(DSONAME) = xlibfltk.sl.1.1; then\
		rm -f $(libdir)/libfltk.sl*;\
	fi
	if test x$(DSONAME) = xlibfltk_s.a; then\
		rm -f $(libdir)/libfltk_s.a;\
	fi
	if test x$(GLDSONAME) = xlibfltk_gl.so.1.1; then\
		rm -f $(libdir)/libfltk_gl.so*;\
	fi
	if test x$(GLDSONAME) = xlibfltk_gl.sl.1.1; then\
		rm -f $(libdir)/libfltk_gl.sl*;\
	fi
	if test x$(GLDSONAME) = xlibfltk_gl_s.a; then\
		rm -f $(libdir)/libfltk_gl_s.a;\
	fi


#
# End of "$Id: Makefile,v 1.18.2.14.2.23 2001/12/06 18:23:29 easysw Exp $".
#
