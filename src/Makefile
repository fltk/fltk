#
# "$Id: Makefile,v 1.14 1999/02/22 18:05:20 mike Exp $"
#
# Library makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998 by Bill Spitzak and others.
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
# Please report all bugs and problems to "fltk-bugs@easysw.com".
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
	Fl_Check_Button.cxx \
	Fl_Choice.cxx \
	Fl_Clock.cxx \
	Fl_Color_Chooser.cxx \
	Fl_Counter.cxx \
	Fl_Dial.cxx \
	Fl_Double_Window.cxx \
	Fl_Gl_Choice.cxx \
	Fl_Gl_Overlay.cxx \
	Fl_Gl_Window.cxx \
	Fl_Group.cxx \
	Fl_Image.cxx \
	Fl_Input.cxx \
	Fl_Input_.cxx \
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
	Fl_Positioner.cxx \
	Fl_Repeat_Button.cxx \
	Fl_Return_Button.cxx \
	Fl_Roller.cxx \
	Fl_Round_Button.cxx \
	Fl_Scroll.cxx \
	Fl_Scrollbar.cxx \
	Fl_Single_Window.cxx \
	Fl_Slider.cxx \
	Fl_Tabs.cxx \
	Fl_Tile.cxx \
	Fl_Valuator.cxx \
	Fl_Value_Input.cxx \
	Fl_Value_Output.cxx \
	Fl_Value_Slider.cxx \
	Fl_Widget.cxx \
	Fl_Window.cxx \
	Fl_Window_fullscreen.cxx \
	Fl_Window_hotspot.cxx \
	Fl_Window_iconize.cxx \
	Fl_abort.cxx \
	Fl_add_idle.cxx \
	Fl_arg.cxx \
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
	fl_file_chooser.cxx \
	fl_font.cxx \
	fl_labeltype.cxx \
	fl_oval_box.cxx \
	fl_overlay.cxx \
	fl_overlay_visual.cxx \
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
	forms_timer.cxx \
	gl_draw.cxx \
	gl_start.cxx \
	glut_compatability.cxx \
	glut_font.cxx

CFILES = scandir.c numericsort.c vsnprintf.c

CLEAN =

################################################################

include ../makeinclude

LIBRARY = ../lib/$(LIBNAME)

OBJECTS = $(CPPFILES:.cxx=.o) $(CFILES:.c=.o)

$(LIBRARY) : $(OBJECTS)
	@echo $(LIBCOMMAND) $(LIBRARY) ...
	@$(LIBCOMMAND) $(LIBRARY) $(OBJECTS)
	@$(RANLIB) $(LIBRARY)

.SUFFIXES:	.cxx .h .o

.cxx.o :
	$(CXX) -I.. $(CXXFLAGS) -c $<
.c.o :
	$(CC) -I.. $(CFLAGS) -c -o $@ $<

clean :
	-@ rm -f *.o *.do $(LIBRARY) $(CLEAN) core *~ ../include/*~ makedepend cmap
	@touch makedepend

depend:
	$(MAKEDEPEND) -I.. $(CXXFLAGS) $(CPPFILES) $(CFILES) > makedepend
include makedepend

################################################################

install: ../lib/$(LIBNAME)
	-mkdir -p $(libdir)
	-cp ../lib/$(LIBNAME)* $(libdir)
	if test $(LIBNAME) = libfltk.so.1; then\
		ln -s $(libdir)/libfltk.so.1 $(libdir)/libfltk.so;\
	fi
	if test $(LIBNAME) = libfltk.sl.1; then\
		ln -s $(libdir)/libfltk.sl.1 $(libdir)/libfltk.sl;\
	fi
	@-chmod a+r,u+w,g-w,o-w $(libdir)/$(LIBNAME)*
	-mkdir -p $(includedir)
	-rm -rf $(includedir)/FL $(includedir)/Fl
	-cp -r ../FL $(includedir)
	@-chmod -R a+r,u+w,g-w,o-w $(includedir)/FL
	-ln -s FL $(includedir)/Fl

#
# End of "$Id: Makefile,v 1.14 1999/02/22 18:05:20 mike Exp $".
#
