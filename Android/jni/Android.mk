# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
TOP_PATH := $(call my-dir)/../..


#################
## FLTK library
include $(CLEAR_VARS)
LOCAL_PATH       += $(TOP_PATH)/Android
LOCAL_PATH       += $(TOP_PATH)
LOCAL_MODULE     := native-activity
LOCAL_C_INCLUDES := $(TOP_PATH)

LOCAL_SRC_FILES  := \
  src/flstring.c \
  src/scandir.c \
  src/numericsort.c \
  src/vsnprintf.c \
  src/xutf8/is_right2left.c \
  src/xutf8/is_spacing.c \
  src/xutf8/case.c \
  src/xutf8/utf8Input.c \
  src/xutf8/utf8Utils.c \
  src/xutf8/utf8Wrap.c \
  src/xutf8/keysym2Ucs.c \
  src/fl_utf.c \
  src/Fl.cxx \
  src/Fl_Adjuster.cxx \
  src/Fl_Bitmap.cxx \
  src/Fl_Browser.cxx \
  src/Fl_Browser_.cxx \
  src/Fl_Browser_load.cxx \
  src/Fl_Box.cxx \
  src/Fl_Button.cxx \
  src/Fl_Chart.cxx \
  src/Fl_Check_Browser.cxx \
  src/Fl_Check_Button.cxx \
  src/Fl_Choice.cxx \
  src/Fl_Clock.cxx \
  src/Fl_Color_Chooser.cxx \
  src/Fl_Copy_Surface.cxx \
  src/Fl_Counter.cxx \
  src/Fl_Device.cxx \
  src/Fl_Dial.cxx \
  src/Fl_Help_Dialog_Dox.cxx \
  src/Fl_Double_Window.cxx \
  src/Fl_File_Browser.cxx \
  src/Fl_File_Chooser.cxx \
  src/Fl_File_Chooser2.cxx \
  src/Fl_File_Icon.cxx \
  src/Fl_File_Input.cxx \
  src/Fl_Graphics_Driver.cxx \
  src/Fl_Group.cxx \
  src/Fl_Help_View.cxx \
  src/Fl_Image.cxx \
  src/Fl_Image_Surface.cxx \
  src/Fl_Input.cxx \
  src/Fl_Input_.cxx \
  src/Fl_Light_Button.cxx \
  src/Fl_Menu.cxx \
  src/Fl_Menu_.cxx \
  src/Fl_Menu_Bar.cxx \
  src/Fl_Menu_Button.cxx \
  src/Fl_Menu_Window.cxx \
  src/Fl_Menu_add.cxx \
  src/Fl_Menu_global.cxx \
  src/Fl_Multi_Label.cxx \
  src/Fl_Native_File_Chooser.cxx \
  src/Fl_Overlay_Window.cxx \
  src/Fl_Pack.cxx \
  src/Fl_Paged_Device.cxx \
  src/Fl_Pixmap.cxx \
  src/Fl_Positioner.cxx \
  src/Fl_Preferences.cxx \
  src/Fl_Printer.cxx \
  src/Fl_Progress.cxx \
  src/Fl_Repeat_Button.cxx \
  src/Fl_Return_Button.cxx \
  src/Fl_Roller.cxx \
  src/Fl_Round_Button.cxx \
  src/Fl_Screen_Driver.cxx \
  src/Fl_Scroll.cxx \
  src/Fl_Scrollbar.cxx \
  src/Fl_Shared_Image.cxx \
  src/Fl_Single_Window.cxx \
  src/Fl_Slider.cxx \
  src/Fl_Table.cxx \
  src/Fl_Table_Row.cxx \
  src/Fl_Tabs.cxx \
  src/Fl_Text_Buffer.cxx \
  src/Fl_Text_Display.cxx \
  src/Fl_Text_Editor.cxx \
  src/Fl_Tile.cxx \
  src/Fl_Tiled_Image.cxx \
  src/Fl_Tooltip.cxx \
  src/Fl_Tree.cxx \
  src/Fl_Tree_Item_Array.cxx \
  src/Fl_Tree_Item.cxx \
  src/Fl_Tree_Prefs.cxx \
  src/Fl_Valuator.cxx \
  src/Fl_Value_Input.cxx \
  src/Fl_Value_Output.cxx \
  src/Fl_Value_Slider.cxx \
  src/Fl_Widget.cxx \
  src/Fl_Widget_Surface.cxx \
  src/Fl_Window.cxx \
  src/Fl_Window_Driver.cxx \
  src/Fl_Window_fullscreen.cxx \
  src/Fl_Window_hotspot.cxx \
  src/Fl_Window_iconize.cxx \
  src/Fl_Wizard.cxx \
  src/Fl_XBM_Image.cxx \
  src/Fl_XPM_Image.cxx \
  src/Fl_abort.cxx \
  src/Fl_add_idle.cxx \
  src/Fl_arg.cxx \
  src/Fl_compose.cxx \
  src/Fl_display.cxx \
  src/Fl_get_key.cxx \
  src/Fl_get_system_colors.cxx \
  src/Fl_grab.cxx \
  src/Fl_lock.cxx \
  src/Fl_own_colormap.cxx \
  src/Fl_visual.cxx \
  src/Fl_x.cxx \
  src/filename_absolute.cxx \
  src/filename_expand.cxx \
  src/filename_ext.cxx \
  src/filename_isdir.cxx \
  src/filename_list.cxx \
  src/filename_match.cxx \
  src/filename_setext.cxx \
  src/fl_arc.cxx \
  src/fl_ask.cxx \
  src/fl_boxtype.cxx \
  src/fl_color.cxx \
  src/fl_cursor.cxx \
  src/fl_curve.cxx \
  src/fl_diamond_box.cxx \
  src/fl_dnd.cxx \
  src/fl_draw.cxx \
  src/fl_draw_pixmap.cxx \
  src/fl_engraved_label.cxx \
  src/fl_file_dir.cxx \
  src/fl_font.cxx \
  src/fl_gleam.cxx \
  src/fl_gtk.cxx \
  src/fl_labeltype.cxx \
  src/fl_line_style.cxx \
  src/fl_open_uri.cxx \
  src/fl_oval_box.cxx \
  src/fl_overlay.cxx \
  src/fl_overlay_visual.cxx \
  src/fl_plastic.cxx \
  src/fl_read_image.cxx \
  src/fl_rect.cxx \
  src/fl_round_box.cxx \
  src/fl_rounded_box.cxx \
  src/fl_set_font.cxx \
  src/fl_scroll_area.cxx \
  src/fl_shadow_box.cxx \
  src/fl_shortcut.cxx \
  src/fl_show_colormap.cxx \
  src/fl_symbols.cxx \
  src/fl_vertex.cxx \
  src/screen_xywh.cxx \
  src/fl_utf8.cxx \
  src/fl_encoding_latin1.cxx \
  src/fl_encoding_mac_roman.cxx \
  src/drivers/Pico/Fl_Pico_System_Driver.cxx \
  src/drivers/Pico/Fl_Pico_Screen_Driver.cxx \
  src/drivers/Pico/Fl_Pico_Window_Driver.cxx \
  src/drivers/Pico/Fl_Pico_Graphics_Driver.cxx \
  src/drivers/Pico/Fl_Pico_Copy_Surface.cxx \
  src/drivers/Pico/Fl_Pico_Image_Surface.cxx \
  src/drivers/PicoAndroid/Fl_PicoAndroid_System_Driver.cxx \
  src/drivers/PicoAndroid/Fl_PicoAndroid_Screen_Driver.cxx \
  src/drivers/PicoAndroid/Fl_PicoAndroid_Window_Driver.cxx \
  src/drivers/PicoAndroid/Fl_PicoAndroid_Graphics_Driver.cxx \
  src/drivers/PicoAndroid/Fl_PicoAndroid_Copy_Surface.cxx \
  src/drivers/PicoAndroid/Fl_PicoAndroid_Image_Surface.cxx \
  test/hello.cxx

#Android/jni/main.c


LOCAL_CFLAGS     := -DFL_PORTING -DANDROID -DFL_PICO_ANDROID -DFL_LIBRARY

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)




$(call import-module,android/native_app_glue)
