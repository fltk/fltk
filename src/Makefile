#
# "$Id"
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
	Fl.C \
	Fl_Adjuster.C \
	Fl_Bitmap.C \
	Fl_Browser.C \
	Fl_Browser_.C \
	Fl_Browser_load.C \
	Fl_Box.C \
	Fl_Button.C \
	Fl_Chart.C \
	Fl_Check_Button.C \
	Fl_Choice.C \
	Fl_Clock.C \
	Fl_Color_Chooser.C \
	Fl_Counter.C \
	Fl_Dial.C \
	Fl_Double_Window.C \
	Fl_Gl_Choice.C \
	Fl_Gl_Overlay.C \
	Fl_Gl_Window.C \
	Fl_Group.C \
	Fl_Image.C \
	Fl_Input.C \
	Fl_Input_.C \
	Fl_Light_Button.C \
	Fl_Menu.C \
	Fl_Menu_.C \
	Fl_Menu_Bar.C \
	Fl_Menu_Button.C \
	Fl_Menu_Window.C \
	Fl_Menu_add.C \
	Fl_Menu_global.C \
	Fl_Multi_Label.C \
	Fl_Output.C \
	Fl_Overlay_Window.C \
	Fl_Pack.C \
	Fl_Pixmap.C \
	Fl_Positioner.C \
	Fl_Repeat_Button.C \
	Fl_Return_Button.C \
	Fl_Roller.C \
	Fl_Round_Button.C \
	Fl_Scroll.C \
	Fl_Scrollbar.C \
	Fl_Single_Window.C \
	Fl_Slider.C \
	Fl_Tabs.C \
	Fl_Tile.C \
	Fl_Valuator.C \
	Fl_Value_Input.C \
	Fl_Value_Output.C \
	Fl_Value_Slider.C \
	Fl_Widget.C \
	Fl_Window.C \
	Fl_Window_fullscreen.C \
	Fl_Window_hotspot.C \
	Fl_Window_iconize.C \
	Fl_abort.C \
	Fl_add_idle.C \
	Fl_arg.C \
	Fl_cutpaste.C \
	Fl_display.C \
	Fl_get_key.C \
	Fl_get_system_colors.C \
	Fl_own_colormap.C \
	Fl_visual.C \
	Fl_x.C \
	filename_absolute.C \
	filename_expand.C \
	filename_ext.C \
	filename_isdir.C \
	filename_list.C \
	filename_match.C \
	filename_setext.C \
	fl_arc.C \
	fl_arci.C \
	fl_ask.C \
	fl_boxtype.C \
	fl_color.C \
	fl_cursor.C \
	fl_curve.C \
	fl_diamond_box.C \
	fl_draw.C \
	fl_draw_image.C \
	fl_draw_pixmap.C \
	fl_engraved_label.C \
	fl_file_chooser.C \
	fl_font.C \
	fl_labeltype.C \
	fl_oval_box.C \
	fl_overlay.C \
	fl_overlay_visual.C \
	fl_rect.C \
	fl_round_box.C \
	fl_rounded_box.C \
	fl_set_font.C \
	fl_set_fonts.C \
	fl_scroll_area.C \
	fl_shadow_box.C \
	fl_shortcut.C \
	fl_show_colormap.C \
	fl_symbols.C \
	fl_vertex.C \
	forms_compatability.C \
	forms_bitmap.C \
	forms_free.C \
	forms_fselect.C \
	forms_pixmap.C \
	forms_timer.C \
	gl_draw.C \
	gl_start.C \
	glut_compatability.C \
	glut_font.C

CFILES = scandir.c numericsort.c

CLEAN =

################################################################

include ../makeinclude

LIBRARY = ../lib/$(LIBNAME)

OBJECTS = $(CPPFILES:.C=.o) $(CFILES:.c=.o)

$(LIBRARY) : $(OBJECTS)
	@echo Building $(LIBRARY)
	@$(LIBCOMMAND) $(LIBRARY) $(OBJECTS)
	@$(RANLIB) $(LIBRARY)

.C.o :
	@echo $<:
	@$(CXX) -I.. $(CXXFLAGS) -c $<
.c.o :
	@echo $<:
	@$(CC) -I.. $(CFLAGS) -c -o $@ $<

clean :
	-@ rm -f *.o *.do $(LIBRARY) $(CLEAN) core *~ ../include/*~ makedepend cmap
	@touch makedepend

depend:
	@echo Making makedepend
	@$(MAKEDEPEND) -I.. $(CXXFLAGS) $(CPPFILES) $(CFILES) > makedepend
include makedepend

################################################################

install: ../lib/$(LIBNAME)
	-mkdir -p $(libdir)
	cp -f ../lib/$(LIBNAME) $(libdir)
	@chmod a+r,u+w,g-w,o-w $(libdir)/$(LIBNAME)*
	-mkdir -p $(includedir)
	cp -rf ../FL $(includedir)
	@chmod -R a+r,u+w,g-w,o-w $(includedir)/FL

#
# End of "$Id: Makefile,v 1.3 1998/10/20 21:06:18 mike Exp $".
#
