#
# "$Id$"
#
# Library makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2010 by Bill Spitzak and others.
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
# Please report all bugs and problems on the following page:
#
#     http://www.fltk.org/str.php
#

CPPFILES = &
    Fl.obj &
    Fl_Adjuster.obj &
    Fl_Bitmap.obj &
    Fl_Browser.obj &
    Fl_Browser_.obj &
    Fl_Browser_load.obj &
    Fl_Box.obj &
    Fl_Button.obj &
    Fl_Chart.obj &
    Fl_Check_Browser.obj &
    Fl_Check_Button.obj &
    Fl_Choice.obj &
    Fl_Clock.obj &
    Fl_Color_Chooser.obj &
    Fl_Counter.obj &
    Fl_Dial.obj &
    Fl_Double_Window.obj &
    Fl_File_Browser.obj &
    Fl_File_Chooser.obj &
    Fl_File_Chooser2.obj &
    Fl_File_Icon.obj &
    Fl_File_Input.obj &
    Fl_Group.obj &
    Fl_Help_View.obj &
    Fl_Image.obj &
    Fl_Input.obj &
    Fl_Input_.obj &
    Fl_Light_Button.obj &
    Fl_Menu.obj &
    Fl_Menu_.obj &
    Fl_Menu_Bar.obj &
    Fl_Sys_Menu_Bar.obj &
    Fl_Menu_Button.obj &
    Fl_Menu_Window.obj &
    Fl_Menu_add.obj &
    Fl_Menu_global.obj &
    Fl_Multi_Label.obj &
    Fl_Native_File_Chooser.obj &
    Fl_Overlay_Window.obj &
    Fl_Pack.obj &
    Fl_Pixmap.obj &
    Fl_Positioner.obj &
    Fl_Preferences.obj &
    Fl_Progress.obj &
    Fl_Repeat_Button.obj &
    Fl_Return_Button.obj &
    Fl_Roller.obj &
    Fl_Round_Button.obj &
    Fl_Scroll.obj &
    Fl_Scrollbar.obj &
    Fl_Shared_Image.obj &
    Fl_Single_Window.obj &
    Fl_Slider.obj &
    Fl_Tabs.obj &
    Fl_Text_Buffer.obj &
    Fl_Text_Display.obj &
    Fl_Text_Editor.obj &
    Fl_Tile.obj &
    Fl_Tiled_Image.obj &
    Fl_Tooltip.obj &
    Fl_Valuator.obj &
    Fl_Value_Input.obj &
    Fl_Value_Output.obj &
    Fl_Value_Slider.obj &
    Fl_Widget.obj &
    Fl_Window.obj &
    Fl_Window_fullscreen.obj &
    Fl_Window_hotspot.obj &
    Fl_Window_iconize.obj &
    Fl_Wizard.obj &
    Fl_XBM_Image.obj &
    Fl_XPM_Image.obj &
    Fl_abort.obj &
    Fl_add_idle.obj &
    Fl_arg.obj &
    Fl_compose.obj &
    Fl_display.obj &
    Fl_get_key.obj &
    Fl_get_system_colors.obj &
    Fl_grab.obj &
    Fl_lock.obj &
    Fl_own_colormap.obj &
    Fl_visual.obj &
    Fl_x.obj &
    filename_absolute.obj &
    filename_expand.obj &
    filename_ext.obj &
    filename_isdir.obj &
    filename_list.obj &
    filename_match.obj &
    filename_setext.obj &
    fl_arc.obj &
    fl_arci.obj &
    fl_ask.obj &
    fl_boxtype.obj &
    fl_color.obj &
    fl_cursor.obj &
    fl_curve.obj &
    fl_diamond_box.obj &
    fl_dnd.obj &
    fl_draw.obj &
    fl_draw_image.obj &
    fl_draw_pixmap.obj &
    fl_encoding_latin1.obj &
    fl_encoding_mac_roman.obj &
    fl_engraved_label.obj &
    fl_file_dir.obj &
    fl_font.obj &
    fl_gtk.obj &
    fl_labeltype.obj &
    fl_line_style.obj &
    fl_open_uri.obj &
    fl_oval_box.obj &
    fl_overlay.obj &
    fl_overlay_visual.obj &
    fl_plastic.obj &
    fl_read_image.obj &
    fl_rect.obj &
    fl_round_box.obj &
    fl_rounded_box.obj &
    fl_set_font.obj &
    fl_set_fonts.obj &
    fl_scroll_area.obj &
    fl_shadow_box.obj &
    fl_shortcut.obj &
    fl_show_colormap.obj &
    fl_symbols.obj &
    fl_vertex.obj &
    screen_xywh.obj

FLCPPFILES = &
    forms_compatability.obj &
    forms_bitmap.obj &
    forms_free.obj &
    forms_fselect.obj &
    forms_pixmap.obj &
    forms_timer.obj

GLCPPFILES = &
    Fl_Gl_Choice.obj &
    Fl_Gl_Overlay.obj &
    Fl_Gl_Window.obj &
    freeglut_geometry.obj &
    freeglut_stroke_mono_roman.obj &
    freeglut_stroke_roman.obj &
    freeglut_teapot.obj &
    gl_draw.obj &
    gl_start.obj &
    glut_compatability.obj &
    glut_font.obj

IMGCPPFILES = &
    fl_images_core.obj &
    Fl_BMP_Image.obj &
    Fl_File_Icon2.obj &
    Fl_GIF_Image.obj &
    Fl_Help_Dialog.obj &
    Fl_JPEG_Image.obj &
    Fl_PNG_Image.obj &
    Fl_PNM_Image.obj

CFILES = fl_call_main.obj flstring.obj scandir.obj numericsort.obj vsnprintf.obj

################################################################

!include ../watcom.mif

OBJECTS = $(CPPFILES) $(CFILES)
FLOBJECTS = $(FLCPPFILES)
GLOBJECTS = $(GLCPPFILES)
IMGOBJECTS = $(IMGCPPFILES)

# The four basic fltk libraries are defined in ../watcom.mif, so that appliactions
# can also use them.
all: $(LIBNAME) &
     $(LIBNAMEFL) &
     $(LIBNAMEGL) &
     $(LIBNAMEIMG)

# $(DSONAME) &
# $(FLDSONAME) &
# $(GLDSONAME) &
# $(IMGDSONAME)


$(LIBNAME): $(OBJECTS)
    $(LIB) $(LIBOPTS) $@ $<

$(LIBNAMEFL): $(FLOBJECTS)
    $(LIB) $(LIBOPTS) $@ $<

$(LIBNAMEGL): $(GLOBJECTS)
    $(LIB) $(LIBOPTS) $@ $<

$(LIBNAMEIMG): $(IMGOBJECTS)
    $(LIB) $(LIBOPTS) $@ $<

#
# Clean all directories
#
clean : .SYMBOLIC
    @echo Cleaning up.
CLEANEXTS = exe map sym obj lk1
    @for %a in ($(CLEANEXTS)) do -rm -f $(ODIR)\*.%a
    -rm -f *.err
    -rm -f $(LIBNAME)
    -rm -f $(LIBNAMEFL)
    -rm -f $(LIBNAMEGL)
    -rm -f $(LIBNAMEIMG)

