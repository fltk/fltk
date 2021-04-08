#
# CMakeLists.txt to build the FLTK library using CMake (www.cmake.org)
#
# Copyright 1998-2021 by Bill Spitzak and others.
#
# This library is free software. Distribution and use rights are outlined in
# the file "COPYING" which should have been included with this file.  If this
# file is missing or damaged, see the license at:
#
#     https://www.fltk.org/COPYING.php
#
# Please see the following page on how to report bugs and issues:
#
#     https://www.fltk.org/bugs.php
#

set (CPPFILES
  Fl.cxx
  Fl_Adjuster.cxx
  Fl_Bitmap.cxx
  Fl_Browser.cxx
  Fl_Browser_.cxx
  Fl_Browser_load.cxx
  Fl_Box.cxx
  Fl_Button.cxx
  Fl_Chart.cxx
  Fl_Check_Browser.cxx
  Fl_Check_Button.cxx
  Fl_Choice.cxx
  Fl_Clock.cxx
  Fl_Color_Chooser.cxx
  Fl_Copy_Surface.cxx
  Fl_Counter.cxx
  Fl_Device.cxx
  Fl_Dial.cxx
  Fl_Help_Dialog_Dox.cxx
  Fl_Double_Window.cxx
  Fl_File_Browser.cxx
  Fl_File_Chooser.cxx
  Fl_File_Chooser2.cxx
  Fl_File_Icon.cxx
  Fl_File_Input.cxx
  Fl_Group.cxx
  Fl_Help_View.cxx
  Fl_Image.cxx
  Fl_Image_Surface.cxx
  Fl_Input.cxx
  Fl_Input_.cxx
  Fl_Light_Button.cxx
  Fl_Menu.cxx
  Fl_Menu_.cxx
  Fl_Menu_Bar.cxx
  Fl_Menu_Button.cxx
  Fl_Menu_Window.cxx
  Fl_Menu_add.cxx
  Fl_Menu_global.cxx
  Fl_Multi_Label.cxx
  Fl_Native_File_Chooser.cxx
  Fl_Overlay_Window.cxx
  Fl_Pack.cxx
  Fl_Paged_Device.cxx
  Fl_Pixmap.cxx
  Fl_Positioner.cxx
  Fl_PostScript.cxx
  Fl_Preferences.cxx
  Fl_Printer.cxx
  Fl_Progress.cxx
  Fl_Repeat_Button.cxx
  Fl_Return_Button.cxx
  Fl_Roller.cxx
  Fl_Round_Button.cxx
  Fl_Scroll.cxx
  Fl_Scrollbar.cxx
  Fl_Shared_Image.cxx
  Fl_Single_Window.cxx
  Fl_Slider.cxx
  Fl_Table.cxx
  Fl_Table_Row.cxx
  Fl_Tabs.cxx
  Fl_Text_Buffer.cxx
  Fl_Text_Display.cxx
  Fl_Text_Editor.cxx
  Fl_Tile.cxx
  Fl_Tiled_Image.cxx
  Fl_Tooltip.cxx
  Fl_Tree.cxx
  Fl_Tree_Item_Array.cxx
  Fl_Tree_Item.cxx
  Fl_Tree_Prefs.cxx
  Fl_Valuator.cxx
  Fl_Value_Input.cxx
  Fl_Value_Output.cxx
  Fl_Value_Slider.cxx
  Fl_Widget.cxx
  Fl_Window.cxx
  Fl_Window_fullscreen.cxx
  Fl_Window_hotspot.cxx
  Fl_Window_iconize.cxx
  Fl_Window_shape.cxx
  Fl_Wizard.cxx
  Fl_XBM_Image.cxx
  Fl_XPM_Image.cxx
  Fl_abort.cxx
  Fl_add_idle.cxx
  Fl_arg.cxx
  Fl_compose.cxx
  Fl_display.cxx
  Fl_get_key.cxx
  Fl_get_system_colors.cxx
  Fl_grab.cxx
  Fl_lock.cxx
  Fl_own_colormap.cxx
  Fl_visual.cxx
  Fl_x.cxx
  filename_absolute.cxx
  filename_expand.cxx
  filename_ext.cxx
  filename_isdir.cxx
  filename_list.cxx
  filename_match.cxx
  filename_setext.cxx
  fl_arc.cxx
  fl_arci.cxx
  fl_ask.cxx
  fl_boxtype.cxx
  fl_color.cxx
  fl_cursor.cxx
  fl_curve.cxx
  fl_diamond_box.cxx
  fl_dnd.cxx
  fl_draw.cxx
  fl_draw_image.cxx
  fl_draw_pixmap.cxx
  fl_engraved_label.cxx
  fl_file_dir.cxx
  fl_font.cxx
  fl_gleam.cxx
  fl_gtk.cxx
  fl_labeltype.cxx
  fl_line_style.cxx
  fl_open_uri.cxx
  fl_oval_box.cxx
  fl_overlay.cxx
  fl_overlay_visual.cxx
  fl_plastic.cxx
  fl_read_image.cxx
  fl_rect.cxx
  fl_round_box.cxx
  fl_rounded_box.cxx
  fl_set_font.cxx
  fl_set_fonts.cxx
  fl_scroll_area.cxx
  fl_shadow_box.cxx
  fl_shortcut.cxx
  fl_show_colormap.cxx
  fl_symbols.cxx
  fl_vertex.cxx
  ps_image.cxx
  screen_xywh.cxx
  fl_utf8.cxx
  fl_encoding_latin1.cxx
  fl_encoding_mac_roman.cxx
)

# find all header files in source directory <FL/...>
file (GLOB
  HEADER_FILES
  "../FL/*.[hH]"
)

# add generated header files in build directory
list (APPEND HEADER_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/../FL/abi-version.h
  ${CMAKE_CURRENT_BINARY_DIR}/../config.h
)

set (GL_HEADER_FILES)  # FIXME: not (yet?) defined

set (FORMS_FILES
  forms_compatability.cxx
  forms_bitmap.cxx
  forms_free.cxx
  forms_fselect.cxx
  forms_pixmap.cxx
  forms_timer.cxx
)

set (GLCPPFILES
  Fl_Gl_Choice.cxx
  Fl_Gl_Device_Plugin.cxx
  Fl_Gl_Overlay.cxx
  Fl_Gl_Window.cxx
  freeglut_geometry.cxx
  freeglut_stroke_mono_roman.cxx
  freeglut_stroke_roman.cxx
  freeglut_teapot.cxx
  gl_draw.cxx
  gl_start.cxx
  glut_compatability.cxx
  glut_font.cxx
)

set (IMGCPPFILES
  fl_images_core.cxx
  Fl_BMP_Image.cxx
  Fl_File_Icon2.cxx
  Fl_GIF_Image.cxx
  Fl_Help_Dialog.cxx
  Fl_JPEG_Image.cxx
  Fl_PNG_Image.cxx
  Fl_PNM_Image.cxx
)

set (CFILES
  flstring.c
  scandir.c
  numericsort.c
  vsnprintf.c
  xutf8/is_right2left.c
  xutf8/is_spacing.c
  xutf8/case.c
  xutf8/utf8Input.c
  xutf8/utf8Utils.c
  xutf8/utf8Wrap.c
  xutf8/keysym2Ucs.c
  fl_utf.c
)

if (APPLE AND NOT OPTION_APPLE_X11)
  set (MMFILES
    Fl_cocoa.mm
    Fl_Quartz_Printer.mm
    Fl_Native_File_Chooser_MAC.mm
    Fl_Sys_Menu_Bar.mm
  )
else()
  set (MMFILES
  )
endif (APPLE AND NOT OPTION_APPLE_X11)


#######################################################################

# prepare source files for shared and static FLTK libraries

set (SHARED_FILES ${CPPFILES} ${MMFILES} ${CFILES})
list (APPEND SHARED_FILES ${HEADER_FILES})

set (STATIC_FILES ${SHARED_FILES})

if (WIN32)
  list (APPEND STATIC_FILES fl_call_main.c)
endif (WIN32)

add_definitions(-DFL_LIBRARY)

#######################################################################

# prepare optional libs for shared and static FLTK libraries

set (OPTIONAL_LIBS)

if (LIB_dl)
  list (APPEND OPTIONAL_LIBS ${LIB_dl})
endif (LIB_dl)

if (USE_THREADS)
  list (APPEND OPTIONAL_LIBS ${CMAKE_THREAD_LIBS_INIT})
endif (USE_THREADS)

if (USE_X11)
  list (APPEND OPTIONAL_LIBS ${X11_LIBRARIES})
endif (USE_X11)

if (WIN32)
  list (APPEND OPTIONAL_LIBS comctl32 ws2_32)
endif (WIN32)

if (FLTK_HAVE_CAIRO)
  list (APPEND OPTIONAL_LIBS fltk_cairo ${PKG_CAIRO_LIBRARIES})
endif (FLTK_HAVE_CAIRO)

if (HAVE_XINERAMA)
  list (APPEND OPTIONAL_LIBS ${X11_Xinerama_LIB})
endif (HAVE_XINERAMA)

if (HAVE_XFIXES)
  list (APPEND OPTIONAL_LIBS ${X11_Xfixes_LIB})
endif (HAVE_XFIXES)

if (HAVE_XCURSOR)
  list (APPEND OPTIONAL_LIBS ${X11_Xcursor_LIB})
endif (HAVE_XCURSOR)

if (HAVE_XRENDER)
  list (APPEND OPTIONAL_LIBS ${X11_Xrender_LIB})
endif (HAVE_XRENDER)

if (USE_XFT)
  list (APPEND OPTIONAL_LIBS ${X11_Xft_LIB})
  if (LIB_fontconfig)
    list (APPEND OPTIONAL_LIBS ${LIB_fontconfig})
  endif (LIB_fontconfig)
endif (USE_XFT)

#######################################################################

FL_ADD_LIBRARY (fltk STATIC "${STATIC_FILES}")
target_link_libraries (fltk ${OPTIONAL_LIBS})

#######################################################################

FL_ADD_LIBRARY (fltk_forms STATIC "${FORMS_FILES}")
target_link_libraries (fltk_forms fltk)

#######################################################################

FL_ADD_LIBRARY (fltk_images STATIC "${IMGCPPFILES}")
target_link_libraries (fltk_images fltk)

if (OPTION_USE_SYSTEM_LIBJPEG)
  target_link_libraries (fltk_images ${FLTK_JPEG_LIBRARIES})
else()
  target_link_libraries (fltk_images fltk_jpeg)
endif (OPTION_USE_SYSTEM_LIBJPEG)

if (OPTION_USE_SYSTEM_ZLIB)
  target_link_libraries (fltk_images ${FLTK_ZLIB_LIBRARIES})
else()
  target_link_libraries (fltk_images fltk_z)
endif (OPTION_USE_SYSTEM_ZLIB)

if (OPTION_USE_SYSTEM_LIBPNG)
  target_link_libraries (fltk_images ${FLTK_PNG_LIBRARIES})
else()
  target_link_libraries (fltk_images fltk_png)
endif (OPTION_USE_SYSTEM_LIBPNG)

#######################################################################

if (OPENGL_FOUND)
  FL_ADD_LIBRARY (fltk_gl STATIC "${GLCPPFILES}")
  target_link_libraries (fltk_gl fltk ${OPENGL_LIBRARIES})
endif (OPENGL_FOUND)

#######################################################################
# Build shared libraries (optional)
#######################################################################
# Shared libraries, part 1: everything except Visual Studio (MSVC)
#######################################################################

if (OPTION_BUILD_SHARED_LIBS AND NOT MSVC)

  FL_ADD_LIBRARY (fltk SHARED "${SHARED_FILES}")
  target_link_libraries (fltk_SHARED ${OPTIONAL_LIBS})

  ###################################################################

  FL_ADD_LIBRARY (fltk_forms SHARED "${FORMS_FILES}")
  target_link_libraries (fltk_forms_SHARED fltk_SHARED)

  ###################################################################

  FL_ADD_LIBRARY (fltk_images SHARED "${IMGCPPFILES}")
  target_link_libraries (fltk_images_SHARED fltk_SHARED)

  if (OPTION_USE_SYSTEM_LIBJPEG)
    target_link_libraries (fltk_images_SHARED ${FLTK_JPEG_LIBRARIES})
  else()
    target_link_libraries (fltk_images_SHARED fltk_jpeg_SHARED)
  endif (OPTION_USE_SYSTEM_LIBJPEG)

  if (OPTION_USE_SYSTEM_LIBPNG)
    target_link_libraries (fltk_images_SHARED ${FLTK_PNG_LIBRARIES})
  else()
    target_link_libraries (fltk_images_SHARED fltk_png_SHARED)
  endif (OPTION_USE_SYSTEM_LIBPNG)

  if (OPTION_USE_SYSTEM_ZLIB)
    target_link_libraries (fltk_images_SHARED ${FLTK_ZLIB_LIBRARIES})
  else()
    target_link_libraries (fltk_images_SHARED fltk_z_SHARED)
  endif (OPTION_USE_SYSTEM_ZLIB)

  ###################################################################

  if (OPENGL_FOUND)
    FL_ADD_LIBRARY (fltk_gl SHARED "${GLCPPFILES};${GL_HEADER_FILES};${GL_DRIVER_HEADER_FILES}")
    target_link_libraries (fltk_gl_SHARED fltk_SHARED ${OPENGL_LIBRARIES})
  endif (OPENGL_FOUND)

endif (OPTION_BUILD_SHARED_LIBS AND NOT MSVC)

#######################################################################
# Shared libraries, part 2: Visual Studio (MSVC)
#######################################################################

# Note to devs: As of June 2020 we can't build separate shared libs (DLL's)
# under Windows with Visual Studio (MSVC) but we can build one big DLL
# that comprises all FLTK and optional PNG, JPEG, and ZLIB libs.
# AlbrechtS

if (OPTION_BUILD_SHARED_LIBS AND MSVC)

  set (SOURCES ${SHARED_FILES} ${FORMS_FILES} ${IMGCPPFILES})
  if (OPENGL_FOUND)
    list (APPEND SOURCES ${GLCPPFILES} ${GL_HEADER_FILES} ${GL_DRIVER_HEADER_FILES})
  endif (OPENGL_FOUND)

  FL_ADD_LIBRARY (fltk SHARED "${SOURCES}")
  target_link_libraries (fltk_SHARED ${OPTIONAL_LIBS})

  if (OPTION_USE_SYSTEM_LIBJPEG)
    target_link_libraries (fltk_SHARED ${FLTK_JPEG_LIBRARIES})
  else()
    target_link_libraries (fltk_SHARED fltk_jpeg_SHARED)
  endif (OPTION_USE_SYSTEM_LIBJPEG)

  if (OPTION_USE_SYSTEM_LIBPNG)
    target_link_libraries (fltk_SHARED ${FLTK_PNG_LIBRARIES})
  else()
    target_link_libraries (fltk_SHARED fltk_png_SHARED)
  endif (OPTION_USE_SYSTEM_LIBPNG)

  if (OPTION_USE_SYSTEM_ZLIB)
    target_link_libraries (fltk_SHARED ${FLTK_ZLIB_LIBRARIES})
  else()
    target_link_libraries (fltk_SHARED fltk_z_SHARED)
  endif (OPTION_USE_SYSTEM_ZLIB)

  if (OPENGL_FOUND)
    target_link_libraries (fltk_SHARED ${OPENGL_LIBRARIES})
  endif (OPENGL_FOUND)

endif (OPTION_BUILD_SHARED_LIBS AND MSVC)

#######################################################################
