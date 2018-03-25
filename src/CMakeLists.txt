#
# "$Id$"
#
# CMakeLists.txt to build the FLTK library using CMake (www.cmake.org)
#
# Copyright 1998-2018 by Bill Spitzak and others.
#
# This library is free software. Distribution and use rights are outlined in
# the file "COPYING" which should have been included with this file.  If this
# file is missing or damaged, see the license at:
#
#     http://www.fltk.org/COPYING.php
#
# Please report all bugs and problems on the following page:
#
#     http://www.fltk.org/str.php
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
  Fl_Graphics_Driver.cxx
  Fl_Group.cxx
  Fl_Help_View.cxx
  Fl_Image.cxx
  Fl_Image_Surface.cxx
  Fl_Input.cxx
  Fl_Input_.cxx
  Fl_Input_Choice.cxx
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
  Fl_Preferences.cxx
  Fl_Printer.cxx
  Fl_Progress.cxx
  Fl_Repeat_Button.cxx
  Fl_Return_Button.cxx
  Fl_Roller.cxx
  Fl_Round_Button.cxx
  Fl_Screen_Driver.cxx
  Fl_Scroll.cxx
  Fl_Scrollbar.cxx
  Fl_Shared_Image.cxx
  Fl_Simple_Terminal.cxx
  Fl_Single_Window.cxx
  Fl_Slider.cxx
  Fl_Spinner.cxx
  Fl_Sys_Menu_Bar.cxx
  Fl_System_Driver.cxx
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
  Fl_Widget_Surface.cxx
  Fl_Window.cxx
  Fl_Window_Driver.cxx
  Fl_Window_fullscreen.cxx
  Fl_Window_hotspot.cxx
  Fl_Window_iconize.cxx
  Fl_Wizard.cxx
  Fl_XBM_Image.cxx
  Fl_XPM_Image.cxx
  Fl_abort.cxx
  Fl_add_idle.cxx
  Fl_arg.cxx
  Fl_compose.cxx
  Fl_display.cxx
  Fl_get_system_colors.cxx
  Fl_grab.cxx
  Fl_lock.cxx
  Fl_own_colormap.cxx
  Fl_visual.cxx
  filename_absolute.cxx
  filename_expand.cxx
  filename_ext.cxx
  filename_isdir.cxx
  filename_list.cxx
  filename_match.cxx
  filename_setext.cxx
  fl_arc.cxx
  fl_ask.cxx
  fl_boxtype.cxx
  fl_color.cxx
  fl_cursor.cxx
  fl_curve.cxx
  fl_diamond_box.cxx
  fl_draw.cxx
  fl_draw_pixmap.cxx
  fl_engraved_label.cxx
  fl_file_dir.cxx
  fl_font.cxx
  fl_gleam.cxx
  fl_gtk.cxx
  fl_labeltype.cxx
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
  fl_scroll_area.cxx
  fl_shadow_box.cxx
  fl_shortcut.cxx
  fl_show_colormap.cxx
  fl_symbols.cxx
  fl_vertex.cxx
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

set (GL_HEADER_FILES)		# FIXME: not (yet?) defined

if ((USE_X11 OR USE_SDL) AND NOT OPTION_PRINT_SUPPORT)
  set (PSFILES
  )
else ()
  set (PSFILES
    drivers/PostScript/Fl_PostScript.cxx
    drivers/PostScript/Fl_PostScript_image.cxx
  )
endif ((USE_X11 OR USE_SDL) AND NOT OPTION_PRINT_SUPPORT)

set (DRIVER_FILES)

if (USE_X11)

  # X11 (including APPLE with X11)

  set (DRIVER_FILES
    drivers/Posix/Fl_Posix_System_Driver.cxx
    drivers/Posix/Fl_Posix_Printer_Driver.cxx
    drivers/X11/Fl_X11_Screen_Driver.cxx
    drivers/X11/Fl_X11_Window_Driver.cxx
    drivers/X11/Fl_X11_System_Driver.cxx
    drivers/Xlib/Fl_Xlib_Graphics_Driver.cxx
    drivers/Xlib/Fl_Xlib_Graphics_Driver_arci.cxx
    drivers/Xlib/Fl_Xlib_Graphics_Driver_color.cxx
    drivers/Xlib/Fl_Xlib_Graphics_Driver_image.cxx
    drivers/Xlib/Fl_Xlib_Graphics_Driver_line_style.cxx
    drivers/Xlib/Fl_Xlib_Graphics_Driver_rect.cxx
    drivers/Xlib/Fl_Xlib_Graphics_Driver_vertex.cxx
    drivers/Xlib/Fl_Xlib_Copy_Surface_Driver.cxx
    drivers/Xlib/Fl_Xlib_Image_Surface_Driver.cxx
    Fl_x.cxx
    fl_dnd_x.cxx
    Fl_Native_File_Chooser_FLTK.cxx
    Fl_Native_File_Chooser_GTK.cxx
    Fl_get_key.cxx
  )
  if (USE_XFT)
    set (DRIVER_FILES ${DRIVER_FILES}
      drivers/Xlib/Fl_Xlib_Graphics_Driver_font_xft.cxx
    )
  else ()
    set (DRIVER_FILES ${DRIVER_FILES}
      drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.cxx
    )  
  endif (USE_XFT)
  set (DRIVER_HEADER_FILES
    drivers/Posix/Fl_Posix_System_Driver.H
    drivers/X11/Fl_X11_Screen_Driver.H
    drivers/X11/Fl_X11_Window_Driver.H
    drivers/X11/Fl_X11_System_Driver.H
    drivers/Xlib/Fl_Font.H
  )

elseif (USE_SDL)

  # SDL2 

  set (DRIVER_FILES
    drivers/Pico/Fl_Pico_System_Driver.cxx
    drivers/Pico/Fl_Pico_Screen_Driver.cxx
    drivers/Pico/Fl_Pico_Window_Driver.cxx
    drivers/Pico/Fl_Pico_Graphics_Driver.cxx
    drivers/Pico/Fl_Pico_Copy_Surface.cxx
    drivers/Pico/Fl_Pico_Image_Surface.cxx
    drivers/PicoSDL/Fl_PicoSDL_System_Driver.cxx
    drivers/PicoSDL/Fl_PicoSDL_Screen_Driver.cxx
    drivers/PicoSDL/Fl_PicoSDL_Window_Driver.cxx
    drivers/PicoSDL/Fl_PicoSDL_Graphics_Driver.cxx
    drivers/PicoSDL/Fl_PicoSDL_Copy_Surface.cxx
    drivers/PicoSDL/Fl_PicoSDL_Image_Surface.cxx
  )
  set (DRIVER_HEADER_FILES
    drivers/Pico/Fl_Pico_System_Driver.H
    drivers/Pico/Fl_Pico_Screen_Driver.H
    drivers/Pico/Fl_Pico_Window_Driver.H
    drivers/Pico/Fl_Pico_Graphics_Driver.H
    drivers/PicoSDL/Fl_PicoSDL_System_Driver.H
    drivers/PicoSDL/Fl_PicoSDL_Screen_Driver.H
    drivers/PicoSDL/Fl_PicoSDL_Window_Driver.H
    drivers/PicoSDL/Fl_PicoSDL_Graphics_Driver.H
  )

elseif (APPLE)

  # Apple Quartz

  set (DRIVER_FILES
    drivers/Quartz/Fl_Quartz_Graphics_Driver.cxx
    drivers/Quartz/Fl_Quartz_Graphics_Driver_color.cxx
    drivers/Quartz/Fl_Quartz_Graphics_Driver_rect.cxx
    drivers/Quartz/Fl_Quartz_Graphics_Driver_font.cxx
    drivers/Quartz/Fl_Quartz_Graphics_Driver_vertex.cxx
    drivers/Quartz/Fl_Quartz_Graphics_Driver_image.cxx
    drivers/Quartz/Fl_Quartz_Graphics_Driver_arci.cxx
    drivers/Quartz/Fl_Quartz_Graphics_Driver_line_style.cxx
    drivers/Quartz/Fl_Quartz_Copy_Surface_Driver.cxx
    drivers/Quartz/Fl_Quartz_Image_Surface_Driver.cxx
    drivers/Cocoa/Fl_Cocoa_Window_Driver.cxx
    drivers/Cocoa/Fl_Cocoa_Screen_Driver.cxx
    drivers/Posix/Fl_Posix_System_Driver.cxx
    drivers/Darwin/Fl_Darwin_System_Driver.cxx
    Fl_get_key_mac.cxx
  )
  set (DRIVER_HEADER_FILES
    drivers/Posix/Fl_Posix_System_Driver.H
    drivers/Darwin/Fl_Darwin_System_Driver.H
    drivers/Cocoa/Fl_Cocoa_Screen_Driver.H
    drivers/Cocoa/Fl_Cocoa_Window_Driver.H
    drivers/Cocoa/Fl_MacOS_Sys_Menu_Bar_Driver.H
    drivers/Quartz/Fl_Quartz_Graphics_Driver.H
    drivers/Quartz/Fl_Quartz_Copy_Surface_Driver.H
    drivers/Quartz/Fl_Font.H
  )

elseif (ANDROID)

  # Android

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

  set (DRIVER_FILES
    drivers/Android/Fl_Android_Application.cxx
    drivers/Android/Fl_Android_System_Driver.cxx
    drivers/Android/Fl_Android_Screen_Driver.cxx
    drivers/Android/Fl_Android_Screen_Keyboard.cxx
    drivers/Android/Fl_Android_Window_Driver.cxx
    drivers/Android/Fl_Android_Image_Surface_Driver.cxx
    drivers/Android/Fl_Android_Graphics_Driver.cxx
    drivers/Android/Fl_Android_Graphics_Clipping.cxx
    drivers/Android/Fl_Android_Graphics_Font.cxx
  )
  set (DRIVER_HEADER_FILES
    drivers/Android/Fl_Android_Application.H
    drivers/Android/Fl_Android_System_Driver.H
    drivers/Android/Fl_Android_Screen_Driver.H
    drivers/Android/Fl_Android_Window_Driver.H
    drivers/Android/Fl_Android_Graphics_Driver.H
    drivers/Android/Fl_Android_Graphics_Clipping.H
    drivers/Android/Fl_Android_Graphics_Font.H
  )


else ()

  # Windows (GDI)

  set (DRIVER_FILES
    drivers/WinAPI/Fl_WinAPI_System_Driver.cxx
    drivers/WinAPI/Fl_WinAPI_Screen_Driver.cxx
    drivers/WinAPI/Fl_WinAPI_Window_Driver.cxx
    drivers/WinAPI/Fl_WinAPI_Printer_Driver.cxx
    drivers/GDI/Fl_GDI_Graphics_Driver.cxx
    drivers/GDI/Fl_GDI_Graphics_Driver_arci.cxx
    drivers/GDI/Fl_GDI_Graphics_Driver_color.cxx
    drivers/GDI/Fl_GDI_Graphics_Driver_font.cxx
    drivers/GDI/Fl_GDI_Graphics_Driver_image.cxx
    drivers/GDI/Fl_GDI_Graphics_Driver_line_style.cxx
    drivers/GDI/Fl_GDI_Graphics_Driver_rect.cxx
    drivers/GDI/Fl_GDI_Graphics_Driver_vertex.cxx
    drivers/GDI/Fl_GDI_Copy_Surface_Driver.cxx
    drivers/GDI/Fl_GDI_Image_Surface_Driver.cxx
    Fl_win32.cxx
    fl_dnd_win32.cxx
    Fl_Native_File_Chooser_WIN32.cxx
    Fl_get_key_win32.cxx
  )
  set (DRIVER_HEADER_FILES
    drivers/WinAPI/Fl_WinAPI_System_Driver.H
    drivers/WinAPI/Fl_WinAPI_Screen_Driver.H
    drivers/WinAPI/Fl_WinAPI_Window_Driver.H
    drivers/GDI/Fl_GDI_Graphics_Driver.H
    drivers/GDI/Fl_Font.H
  )

endif (USE_X11)

source_group("Header Files" FILES ${HEADER_FILES})
source_group("Driver Source Files" FILES ${DRIVER_FILES})
source_group("Driver Header Files" FILES ${DRIVER_HEADER_FILES})

set (CPPFILES
  ${CPPFILES}
  ${DRIVER_FILES}
)

set (FLCPPFILES
  forms_compatibility.cxx
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
  glut_compatibility.cxx
  glut_font.cxx
)

set (GL_DRIVER_FILES
  drivers/OpenGL/Fl_OpenGL_Display_Device.cxx
  # the following file doesn't contribute any code:
  # drivers/OpenGL/Fl_OpenGL_Graphics_Driver.cxx
  drivers/OpenGL/Fl_OpenGL_Graphics_Driver_arci.cxx
  drivers/OpenGL/Fl_OpenGL_Graphics_Driver_color.cxx
  drivers/OpenGL/Fl_OpenGL_Graphics_Driver_font.cxx
  drivers/OpenGL/Fl_OpenGL_Graphics_Driver_image.cxx
  drivers/OpenGL/Fl_OpenGL_Graphics_Driver_line_style.cxx
  drivers/OpenGL/Fl_OpenGL_Graphics_Driver_rect.cxx
  drivers/OpenGL/Fl_OpenGL_Graphics_Driver_vertex.cxx
)
set (GL_DRIVER_HEADER_FILES
  drivers/OpenGL/Fl_OpenGL_Display_Device.H
  drivers/OpenGL/Fl_OpenGL_Graphics_Driver.H
)

source_group("Driver Header Files" FILES ${GL_DRIVER_HEADER_FILES})
source_group("Driver Source Files" FILES ${GL_DRIVER_FILES})

set (GLCPPFILES
  ${GLCPPFILES}
  ${GL_DRIVER_FILES}
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
  Fl_SVG_Image.cxx
)

set (CFILES
  flstring.c
  numericsort.c
  vsnprintf.c
  xutf8/is_right2left.c
  xutf8/is_spacing.c
  xutf8/case.c
)

if (USE_X11)
  list (APPEND CFILES
    xutf8/keysym2Ucs.c
    scandir_posix.c
  )
  if (NOT USE_XFT)
    list (APPEND CFILES
      xutf8/utf8Utils.c
      xutf8/utf8Wrap.c
      # xutf8/utf8Input.c  # not used
    )
  endif (NOT USE_XFT)
endif (USE_X11)

if (WIN32)
  list (APPEND CFILES
    scandir_win32.c
  )
endif (WIN32)

if (APPLE AND (NOT OPTION_APPLE_X11) AND (NOT OPTION_APPLE_SDL))
    set (MMFILES
        Fl_cocoa.mm
        drivers/Cocoa/Fl_Cocoa_Printer_Driver.mm
        Fl_Native_File_Chooser_MAC.mm
        Fl_MacOS_Sys_Menu_Bar.mm
        )
else()
   set (MMFILES
      )
endif (APPLE AND (NOT OPTION_APPLE_X11) AND (NOT OPTION_APPLE_SDL))


#######################################################################

# prepare source files for shared and static FLTK libraries

set (SHARED_FILES ${CPPFILES} ${MMFILES} ${CFILES} ${PSFILES})
list (APPEND SHARED_FILES ${HEADER_FILES} ${DRIVER_HEADER_FILES})

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

if (USE_SDL)
    list (APPEND OPTIONAL_LIBS ${SDL2_LIBRARY})
endif(USE_SDL)

if (WIN32)
    list (APPEND OPTIONAL_LIBS comctl32)
endif (WIN32)

if (FLTK_HAVE_CAIRO)
    list (APPEND OPTIONAL_LIBS fltk_cairo ${PKG_CAIRO_LIBRARIES})
ENDif (FLTK_HAVE_CAIRO)

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

if (USE_PANGO)
  list (APPEND OPTIONAL_LIBS ${HAVE_LIB_PANGO} ${HAVE_LIB_PANGOXFT})
  if(NOT APPLE)
  	list (APPEND OPTIONAL_LIBS ${HAVE_LIB_GOBJECT} )
  endif(NOT APPLE)
endif(USE_PANGO)

if (USE_XFT)
    list (APPEND OPTIONAL_LIBS ${X11_Xft_LIB})
    if (LIB_fontconfig)
	list (APPEND OPTIONAL_LIBS ${LIB_fontconfig})
    endif (LIB_fontconfig)
endif (USE_XFT)

#######################################################################

FL_ADD_LIBRARY(fltk STATIC "${STATIC_FILES}")
target_link_libraries(fltk ${OPTIONAL_LIBS})

#######################################################################

FL_ADD_LIBRARY(fltk_forms STATIC "${FLCPPFILES}")
target_link_libraries(fltk_forms fltk)

#######################################################################

FL_ADD_LIBRARY(fltk_images STATIC "${IMGCPPFILES}")
target_link_libraries(fltk_images fltk)

if (OPTION_USE_SYSTEM_LIBJPEG)
    target_link_libraries(fltk_images ${FLTK_JPEG_LIBRARIES})
else()
    target_link_libraries(fltk_images fltk_jpeg)
endif (OPTION_USE_SYSTEM_LIBJPEG)

if (OPTION_USE_SYSTEM_ZLIB)
    target_link_libraries(fltk_images ${FLTK_ZLIB_LIBRARIES})
else()
    target_link_libraries(fltk_images fltk_z)
endif (OPTION_USE_SYSTEM_ZLIB)

if (OPTION_USE_SYSTEM_LIBPNG)
    target_link_libraries(fltk_images ${FLTK_PNG_LIBRARIES})
else()
    target_link_libraries(fltk_images fltk_png)
endif (OPTION_USE_SYSTEM_LIBPNG)

#######################################################################

if (OPENGL_FOUND)
    FL_ADD_LIBRARY(fltk_gl STATIC "${GLCPPFILES};${GL_HEADER_FILES};${GL_DRIVER_HEADER_FILES}")
   target_link_libraries(fltk_gl fltk ${OPENGL_LIBRARIES})
endif (OPENGL_FOUND)

#######################################################################
# Build shared libraries (optional)
#######################################################################

# Note to devs: As of 08 Aug 2016 we can build shared libs (dlls)
# under Windows with Visual Studio (MSVC) but we can't link for instance
# fltk_images_SHARED with fltk_SHARED - we have to use fltk, i.e. the
# "static" object library instead. I don't know how to fix this, but
# at least we can build shared libraries (dlls) with Visual Studio.
# AlbrechtS

if (OPTION_BUILD_SHARED_LIBS)

    FL_ADD_LIBRARY(fltk SHARED "${SHARED_FILES}")
    target_link_libraries(fltk_SHARED ${OPTIONAL_LIBS})

    ###################################################################

    FL_ADD_LIBRARY(fltk_forms SHARED "${FLCPPFILES}")
    if (MSVC)
      target_link_libraries(fltk_forms_SHARED fltk)
    else ()
      target_link_libraries(fltk_forms_SHARED fltk_SHARED)
    endif (MSVC)

    ###################################################################

    FL_ADD_LIBRARY(fltk_images SHARED "${IMGCPPFILES}")
    if (MSVC)
      target_link_libraries(fltk_images_SHARED fltk)
    else ()
      target_link_libraries(fltk_images_SHARED fltk_SHARED)
    endif (MSVC)

    if (OPTION_USE_SYSTEM_LIBJPEG)
	target_link_libraries(fltk_images_SHARED ${FLTK_JPEG_LIBRARIES})
    else()
	target_link_libraries(fltk_images_SHARED fltk_jpeg_SHARED)
    endif (OPTION_USE_SYSTEM_LIBJPEG)

    if (OPTION_USE_SYSTEM_LIBPNG)
	target_link_libraries(fltk_images_SHARED ${FLTK_PNG_LIBRARIES})
    else()
	target_link_libraries(fltk_images_SHARED fltk_png_SHARED)
    endif (OPTION_USE_SYSTEM_LIBPNG)

    if (OPTION_USE_SYSTEM_ZLIB)
	target_link_libraries(fltk_images_SHARED ${FLTK_ZLIB_LIBRARIES})
    else()
	target_link_libraries(fltk_images_SHARED fltk_z_SHARED)
    endif (OPTION_USE_SYSTEM_ZLIB)

    ###################################################################

    if (OPENGL_FOUND)
      FL_ADD_LIBRARY(fltk_gl SHARED "${GLCPPFILES};${GL_HEADER_FILES};${GL_DRIVER_HEADER_FILES}")
      if (MSVC)
	target_link_libraries(fltk_gl_SHARED fltk ${OPENGL_LIBRARIES})
      else ()
	target_link_libraries(fltk_gl_SHARED fltk_SHARED ${OPENGL_LIBRARIES})
      endif (MSVC)
    endif (OPENGL_FOUND)

endif (OPTION_BUILD_SHARED_LIBS)

#######################################################################
