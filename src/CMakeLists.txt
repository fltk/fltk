#
# CMakeLists.txt to build the FLTK library using CMake (www.cmake.org)
#
# Copyright 1998-2024 by Bill Spitzak and others.
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

# Local macro to check existence of a required file with warning message
#
# In:  FILENAME (string): File to search
# Out: WARN     (string): *Name* of a variable that is set to true if NOT found
#
# Set variable ${WARN} to FALSE before calling this macro

macro(fl_check_required_file WARN FILENAME)
  if(NOT EXISTS ${FILENAME})
    message("*** Required file not found: ${FILENAME} ***")
    set(${WARN} TRUE)
  endif()
endmacro(fl_check_required_file WARN FILENAME)

set(CPPFILES
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
  Fl_Double_Window.cxx
  Fl_File_Browser.cxx
  Fl_File_Chooser.cxx
  Fl_File_Chooser2.cxx
  Fl_File_Icon.cxx
  Fl_File_Input.cxx
  Fl_Flex.cxx
  Fl_Graphics_Driver.cxx
  Fl_Grid.cxx
  Fl_Group.cxx
  Fl_Help_View.cxx
  Fl_Image.cxx
  Fl_Image_Surface.cxx
  Fl_Input.cxx
  Fl_Input_.cxx
  Fl_Input_Choice.cxx
  Fl_Int_Vector.cxx
  Fl_Light_Button.cxx
  Fl_Menu.cxx
  Fl_Menu_.cxx
  Fl_Menu_Bar.cxx
  Fl_Menu_Button.cxx
  Fl_Menu_Window.cxx
  Fl_Menu_add.cxx
  Fl_Menu_global.cxx
  Fl_Message.cxx
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
  Fl_Scheme.cxx
  Fl_Scheme_Choice.cxx
  Fl_Screen_Driver.cxx
  Fl_Scroll.cxx
  Fl_Scrollbar.cxx
  Fl_Shared_Image.cxx
  Fl_Shortcut_Button.cxx
  Fl_Single_Window.cxx
  Fl_Slider.cxx
  Fl_Spinner.cxx
  Fl_String.cxx
  Fl_Sys_Menu_Bar.cxx
  Fl_System_Driver.cxx
  Fl_Table.cxx
  Fl_Table_Row.cxx
  Fl_Tabs.cxx
  Fl_Terminal.cxx
  Fl_Text_Buffer.cxx
  Fl_Text_Display.cxx
  Fl_Text_Editor.cxx
  Fl_Tile.cxx
  Fl_Tiled_Image.cxx
  Fl_Timeout.cxx
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
  fl_contrast.cxx
  fl_cursor.cxx
  fl_curve.cxx
  fl_diamond_box.cxx
  fl_draw.cxx
  fl_draw_arrow.cxx
  fl_draw_pixmap.cxx
  fl_encoding_latin1.cxx
  fl_encoding_mac_roman.cxx
  fl_engraved_label.cxx
  fl_file_dir.cxx
  fl_font.cxx
  fl_gleam.cxx
  fl_gtk.cxx
  fl_labeltype.cxx
  fl_open_uri.cxx
  fl_oval_box.cxx
  fl_overlay.cxx
  fl_oxy.cxx
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
  fl_string_functions.cxx
  fl_symbols.cxx
  fl_utf8.cxx
  fl_vertex.cxx
  print_button.cxx
  screen_xywh.cxx
)

if(FLTK_HAVE_CAIRO) # FLTK_OPTION_CAIRO_WINDOW or FLTK_OPTION_CAIRO_EXT
  list(APPEND CPPFILES Fl_Cairo.cxx)
endif()

# find all header files in source directory <FL/...>
file(GLOB
  HEADER_FILES
  "../FL/*.[hH]"
)

# add generated header files in build directory
list(APPEND HEADER_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/../FL/fl_config.h
  ${CMAKE_CURRENT_BINARY_DIR}/../config.h
)

set(GL_HEADER_FILES)  # FIXME: not (yet?) defined

if(FLTK_USE_X11 AND NOT FLTK_OPTION_PRINT_SUPPORT)
  set(PSFILES)
else()
  set(PSFILES
    drivers/PostScript/Fl_PostScript.cxx
    drivers/PostScript/Fl_PostScript_image.cxx
  )
endif(FLTK_USE_X11 AND NOT FLTK_OPTION_PRINT_SUPPORT)

set(DRIVER_FILES)

if(FLTK_USE_X11 AND NOT FLTK_BACKEND_WAYLAND)

  # X11 (including APPLE with X11)

  set(DRIVER_FILES
    drivers/Posix/Fl_Posix_Printer_Driver.cxx
    drivers/X11/Fl_X11_Screen_Driver.cxx
    drivers/X11/Fl_X11_Window_Driver.cxx
    drivers/Posix/Fl_Posix_System_Driver.cxx
    drivers/Unix/Fl_Unix_System_Driver.cxx
    drivers/Unix/Fl_Unix_Screen_Driver.cxx
    drivers/Xlib/Fl_Xlib_Copy_Surface_Driver.cxx
    drivers/Xlib/Fl_Xlib_Image_Surface_Driver.cxx
    drivers/X11/fl_X11_platform_init.cxx
    Fl_x.cxx
    fl_dnd_x.cxx
    Fl_Native_File_Chooser_FLTK.cxx
    Fl_Native_File_Chooser_GTK.cxx
    Fl_get_key.cxx
  )

    list(APPEND DRIVER_FILES
      Fl_Native_File_Chooser_Kdialog.cxx
      Fl_Native_File_Chooser_Zenity.cxx)

  if(FLTK_USE_CAIRO)
    list(APPEND DRIVER_FILES
      drivers/Cairo/Fl_Cairo_Graphics_Driver.cxx
      drivers/Cairo/Fl_X11_Cairo_Graphics_Driver.cxx
    )
  else()
    if(USE_XFT)
      list(APPEND DRIVER_FILES
        drivers/Xlib/Fl_Xlib_Graphics_Driver_font_xft.cxx
      )
      if(USE_PANGO)
        list(APPEND DRIVER_FILES drivers/Cairo/Fl_Cairo_Graphics_Driver.cxx)
      endif(USE_PANGO)
    else()
      list(APPEND DRIVER_FILES
        drivers/Xlib/Fl_Xlib_Graphics_Driver_font_x.cxx
      )
    endif(USE_XFT)
    list(APPEND DRIVER_FILES
      drivers/Xlib/Fl_Xlib_Graphics_Driver.cxx
      drivers/Xlib/Fl_Xlib_Graphics_Driver_arci.cxx
      drivers/Xlib/Fl_Xlib_Graphics_Driver_color.cxx
      drivers/Xlib/Fl_Xlib_Graphics_Driver_image.cxx
      drivers/Xlib/Fl_Xlib_Graphics_Driver_line_style.cxx
      drivers/Xlib/Fl_Xlib_Graphics_Driver_rect.cxx
      drivers/Xlib/Fl_Xlib_Graphics_Driver_vertex.cxx
    )
  endif(FLTK_USE_CAIRO)

  set(DRIVER_HEADER_FILES
    drivers/Posix/Fl_Posix_System_Driver.H
    drivers/X11/Fl_X11_Screen_Driver.H
    drivers/X11/Fl_X11_Window_Driver.H
    drivers/Xlib/Fl_Xlib_Graphics_Driver.H
    drivers/Xlib/Fl_Font.H
    drivers/Xlib/Fl_Xlib_Copy_Surface_Driver.H
    drivers/Xlib/Fl_Xlib_Image_Surface_Driver.H
    drivers/Unix/Fl_Unix_System_Driver.H
 )
 if(FLTK_USE_CAIRO)
   set(DRIVER_HEADER_FILES ${DRIVER_HEADER_FILES}
     drivers/Cairo/Fl_Cairo_Graphics_Driver.H
     drivers/Cairo/Fl_X11_Cairo_Graphics_Driver.H
   )
 elseif(USE_PANGO)
   set(DRIVER_HEADER_FILES ${DRIVER_HEADER_FILES}
     drivers/Cairo/Fl_Cairo_Graphics_Driver.H
   )
 endif(FLTK_USE_CAIRO)

elseif(FLTK_BACKEND_WAYLAND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_CURRENT_BINARY_DIR}")
  set(DRIVER_FILES
    drivers/Posix/Fl_Posix_System_Driver.cxx
    drivers/Posix/Fl_Posix_Printer_Driver.cxx
    drivers/Unix/Fl_Unix_Screen_Driver.cxx
    drivers/Wayland/Fl_Wayland_Screen_Driver.cxx
    drivers/Wayland/Fl_Wayland_Window_Driver.cxx
    drivers/Unix/Fl_Unix_System_Driver.cxx
    drivers/Wayland/Fl_Wayland_Graphics_Driver.cxx
    drivers/Wayland/Fl_Wayland_Copy_Surface_Driver.cxx
    drivers/Wayland/Fl_Wayland_Image_Surface_Driver.cxx
    drivers/Wayland/fl_wayland_clipboard_dnd.cxx
    drivers/Wayland/fl_wayland_platform_init.cxx
    drivers/Cairo/Fl_Cairo_Graphics_Driver.cxx
    Fl_Native_File_Chooser_FLTK.cxx
    Fl_Native_File_Chooser_GTK.cxx
  )

    list(APPEND DRIVER_FILES
      Fl_Native_File_Chooser_Kdialog.cxx
      Fl_Native_File_Chooser_Zenity.cxx)

  if(FLTK_USE_X11)
    list(APPEND DRIVER_FILES
      drivers/Cairo/Fl_X11_Cairo_Graphics_Driver.cxx
      drivers/X11/Fl_X11_Screen_Driver.cxx
      drivers/X11/Fl_X11_Window_Driver.cxx
      drivers/Xlib/Fl_Xlib_Copy_Surface_Driver.cxx
      drivers/Xlib/Fl_Xlib_Image_Surface_Driver.cxx
      Fl_x.cxx
      fl_dnd_x.cxx
      Fl_get_key.cxx
    )
  endif(FLTK_USE_X11)

  set(DRIVER_HEADER_FILES
    drivers/Posix/Fl_Posix_System_Driver.H
    drivers/Wayland/Fl_Wayland_Screen_Driver.H
    drivers/Wayland/Fl_Wayland_Window_Driver.H
    drivers/Wayland/Fl_Wayland_Graphics_Driver.H
    drivers/Cairo/Fl_Cairo_Graphics_Driver.H
    drivers/Cairo/Fl_X11_Cairo_Graphics_Driver.H
    drivers/Wayland/Fl_Wayland_Copy_Surface_Driver.H
    drivers/Wayland/Fl_Wayland_Image_Surface_Driver.H
    drivers/Unix/Fl_Unix_System_Driver.H
  )

elseif(APPLE)

  # Apple Quartz

  set(DRIVER_FILES
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
    drivers/Darwin/fl_macOS_platform_init.cxx
  )
  set(DRIVER_HEADER_FILES
    drivers/Posix/Fl_Posix_System_Driver.H
    drivers/Darwin/Fl_Darwin_System_Driver.H
    drivers/Cocoa/Fl_Cocoa_Screen_Driver.H
    drivers/Cocoa/Fl_Cocoa_Window_Driver.H
    drivers/Cocoa/Fl_MacOS_Sys_Menu_Bar_Driver.H
    drivers/Quartz/Fl_Quartz_Graphics_Driver.H
    drivers/Quartz/Fl_Quartz_Copy_Surface_Driver.H
    drivers/Quartz/Fl_Font.H
    drivers/Quartz/Fl_Quartz_Image_Surface_Driver.H
  )

else()

  # Windows (GDI)

  set(DRIVER_FILES
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
    drivers/WinAPI/fl_WinAPI_platform_init.cxx
  )
  set(DRIVER_HEADER_FILES
    drivers/WinAPI/Fl_WinAPI_System_Driver.H
    drivers/WinAPI/Fl_WinAPI_Screen_Driver.H
    drivers/WinAPI/Fl_WinAPI_Window_Driver.H
    drivers/GDI/Fl_GDI_Graphics_Driver.H
    drivers/GDI/Fl_Font.H
    drivers/GDI/Fl_GDI_Copy_Surface_Driver.H
    drivers/GDI/Fl_GDI_Image_Surface_Driver.H
  )

endif(FLTK_USE_X11 AND NOT FLTK_BACKEND_WAYLAND)

source_group("Header Files" FILES ${HEADER_FILES})
source_group("Driver Source Files" FILES ${DRIVER_FILES})
source_group("Driver Header Files" FILES ${DRIVER_HEADER_FILES})

set(CPPFILES
  ${CPPFILES}
  ${DRIVER_FILES}
)

if(FLTK_BUILD_FORMS)
  set(FORMS_FILES
    forms_compatibility.cxx
    forms_bitmap.cxx
    forms_free.cxx
    forms_fselect.cxx
    forms_pixmap.cxx
    forms_timer.cxx
  )
else()
  set(FORMS_FILES "")
endif()

set(GLCPPFILES
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

set(GL_DRIVER_FILES
  drivers/OpenGL/Fl_OpenGL_Display_Device.cxx
  # the following file doesn't contribute any code:
  # drivers/OpenGL/Fl_OpenGL_Graphics_Driver.cxx
  drivers/OpenGL/Fl_OpenGL_Graphics_Driver_arci.cxx
  drivers/OpenGL/Fl_OpenGL_Graphics_Driver_color.cxx
  drivers/OpenGL/Fl_OpenGL_Graphics_Driver_font.cxx
  drivers/OpenGL/Fl_OpenGL_Graphics_Driver_line_style.cxx
  drivers/OpenGL/Fl_OpenGL_Graphics_Driver_rect.cxx
  drivers/OpenGL/Fl_OpenGL_Graphics_Driver_vertex.cxx
)
if(FLTK_BACKEND_WAYLAND)
  set(GL_DRIVER_FILES ${GL_DRIVER_FILES} drivers/Wayland/Fl_Wayland_Gl_Window_Driver.cxx)
  set(GL_DRIVER_HEADER_FILES drivers/Wayland/Fl_Wayland_Gl_Window_Driver.H)
  if(FLTK_USE_X11)
    list(APPEND GL_DRIVER_FILES drivers/X11/Fl_X11_Gl_Window_Driver.cxx)
    list(APPEND GL_DRIVER_HEADER_FILES drivers/X11/Fl_X11_Gl_Window_Driver.H)
  endif(FLTK_USE_X11)
elseif(FLTK_USE_X11)
  set(GL_DRIVER_FILES ${GL_DRIVER_FILES} drivers/X11/Fl_X11_Gl_Window_Driver.cxx)
  set(GL_DRIVER_HEADER_FILES drivers/X11/Fl_X11_Gl_Window_Driver.H)
elseif(APPLE)
  set(GL_DRIVER_FILES ${GL_DRIVER_FILES} drivers/Cocoa/Fl_Cocoa_Gl_Window_Driver.mm)
  set(GL_DRIVER_HEADER_FILES drivers/Cocoa/Fl_Cocoa_Gl_Window_Driver.H)
elseif(WIN32)
  set(GL_DRIVER_FILES ${GL_DRIVER_FILES} drivers/WinAPI/Fl_WinAPI_Gl_Window_Driver.cxx)
  set(GL_DRIVER_HEADER_FILES drivers/WinAPI/Fl_WinAPI_Gl_Window_Driver.H)
endif(FLTK_BACKEND_WAYLAND)

set(GL_DRIVER_HEADER_FILES ${GL_DRIVER_FILES}
  drivers/OpenGL/Fl_OpenGL_Display_Device.H
  drivers/OpenGL/Fl_OpenGL_Graphics_Driver.H
)

source_group("Driver Header Files" FILES ${GL_DRIVER_HEADER_FILES})
source_group("Driver Source Files" FILES ${GL_DRIVER_FILES})

set(GLCPPFILES
  ${GLCPPFILES}
  ${GL_DRIVER_FILES}
)

set(IMGCPPFILES
  fl_images_core.cxx
  fl_write_png.cxx
  Fl_BMP_Image.cxx
  Fl_File_Icon2.cxx
  Fl_GIF_Image.cxx
  Fl_Anim_GIF_Image.cxx
  Fl_Help_Dialog.cxx
  Fl_ICO_Image.cxx
  Fl_JPEG_Image.cxx
  Fl_PNG_Image.cxx
  Fl_PNM_Image.cxx
  Fl_Image_Reader.cxx
  Fl_SVG_Image.cxx
  nanosvg.cxx
  drivers/SVG/Fl_SVG_File_Surface.cxx
)

set(CFILES
  flstring.c
  numericsort.c
  vsnprintf.c
  xutf8/is_right2left.c
  xutf8/is_spacing.c
  xutf8/case.c
)

if(FLTK_USE_X11)
  list(APPEND CFILES
    xutf8/keysym2Ucs.c
    scandir_posix.c
  )
  if(NOT USE_XFT)
    list(APPEND CFILES
      xutf8/utf8Utils.c
      xutf8/utf8Wrap.c
      xutf8/utf8Input.c
    )
  endif(NOT USE_XFT)
endif(FLTK_USE_X11)

if(FLTK_BACKEND_WAYLAND)
  pkg_check_modules(DBUS IMPORTED_TARGET dbus-1)
  set(IDIRS "${CMAKE_CURRENT_BINARY_DIR}")
  set(CDEFS "_GNU_SOURCE;HAVE_MEMFD_CREATE;HAVE_MKOSTEMP;HAVE_POSIX_FALLOCATE")
  set(COPTS "-fPIC")
  if(DBUS_FOUND)
    set(CDEFS "${CDEFS};HAS_DBUS")
    set(IDIRS "${IDIRS};${DBUS_INCLUDE_DIRS}")
  endif(DBUS_FOUND)
  if(USE_SYSTEM_LIBDECOR)
    set(CDEFS "${CDEFS};USE_SYSTEM_LIBDECOR;LIBDECOR_PLUGIN_DIR=${LIBDECOR_PLUGIN_DIR}")
    if(GTK_FOUND)
      set(CDEFS "${CDEFS};HAVE_GTK")
    endif(GTK_FOUND)
    set_source_files_properties(
      ${FLTK_SOURCE_DIR}/libdecor/build/fl_libdecor-plugins.c
      ${FLTK_SOURCE_DIR}/libdecor/src/os-compatibility.c
      ${FLTK_SOURCE_DIR}/libdecor/src/desktop-settings.c
      PROPERTIES COMPILE_DEFINITIONS "${CDEFS}"
      INCLUDE_DIRECTORIES "${IDIRS}"
      COMPILE_OPTIONS "${COPTS}"
      )
  else()
    set(IDIRS "${IDIRS};${FLTK_SOURCE_DIR}/libdecor/src"
      "${FLTK_SOURCE_DIR}/libdecor/src/plugins")
    set(CDEFS "${CDEFS};USE_SYSTEM_LIBDECOR=0;LIBDECOR_PLUGIN_API_VERSION=1"
      "LIBDECOR_PLUGIN_DIR=\"\"")
    if(GTK_FOUND AND FLTK_USE_LIBDECOR_GTK)
      set(CDEFS "${CDEFS};HAVE_GTK")
    endif(GTK_FOUND AND FLTK_USE_LIBDECOR_GTK)
    set_source_files_properties(
      ${FLTK_SOURCE_DIR}/libdecor/build/fl_libdecor.c
      ${FLTK_SOURCE_DIR}/libdecor/build/fl_libdecor-plugins.c
      ${FLTK_SOURCE_DIR}/libdecor/src/os-compatibility.c
      ${FLTK_SOURCE_DIR}/libdecor/src/desktop-settings.c
      PROPERTIES
      COMPILE_DEFINITIONS "${CDEFS}"
      INCLUDE_DIRECTORIES "${IDIRS}"
      COMPILE_OPTIONS "${COPTS}"
      )
  endif(USE_SYSTEM_LIBDECOR)
  set_source_files_properties(
    ${FLTK_SOURCE_DIR}/src/drivers/Wayland/Fl_Wayland_Window_Driver.cxx
    ${FLTK_SOURCE_DIR}/src/drivers/Wayland/Fl_Wayland_Screen_Driver.cxx
    PROPERTIES COMPILE_DEFINITIONS "USE_SYSTEM_LIBDECOR=${USE_SYSTEM_LIBDECOR}"
    )

  list(APPEND CFILES
    scandir_posix.c
    ../libdecor/src/desktop-settings.c
    ../libdecor/src/os-compatibility.c
    ../libdecor/build/fl_libdecor-plugins.c
  )
  if(FLTK_USE_X11)
    list(APPEND CFILES xutf8/keysym2Ucs.c)
  endif(FLTK_USE_X11)
  if(NOT USE_SYSTEM_LIBDECOR)
    list(APPEND CFILES
      ../libdecor/build/fl_libdecor.c
      ../libdecor/src/plugins/common/libdecor-cairo-blur.c
    )
  endif(NOT USE_SYSTEM_LIBDECOR)
endif(FLTK_BACKEND_WAYLAND)

if(WIN32)
  list(APPEND CFILES
    scandir_win32.c
  )
endif(WIN32)

if(APPLE AND NOT FLTK_BACKEND_X11)
  set(MMFILES
    Fl_cocoa.mm
    drivers/Cocoa/Fl_Cocoa_Printer_Driver.mm
    Fl_Native_File_Chooser_MAC.mm
    Fl_MacOS_Sys_Menu_Bar.mm
  )
else()
  set(MMFILES)
endif()


#######################################################################

# prepare source files for shared and static FLTK libraries

set(SHARED_FILES ${CPPFILES} ${MMFILES} ${CFILES} ${PSFILES})
list(APPEND SHARED_FILES ${HEADER_FILES} ${DRIVER_HEADER_FILES})

set(STATIC_FILES ${SHARED_FILES})

# Visual Studio (MSVC) is known to need WinMain() and maybe BORLAND
# needs it as well, hence we include it on all Windows platforms.
# The GNU compilers (MinGW, MSYS2, Cygwin) disable compilation inside
# the source file which is what we finally want and need.

if(WIN32)
  list(APPEND STATIC_FILES fl_call_main.c)
endif()

#######################################################################
#
# Prepare optional libs for shared and static FLTK libraries.
#
# Notes:
#  -  'OPTIONAL_LIBS' is a CMake 'list' but must not contain arbitrary
#     CMake targets because these targets would be propagated to
#     consumer projects. The macro below simplifies adding link
#     libraries of such targets to 'OPTIONAL_LIBS'.
#  -  'OPTIONAL_INCLUDES' is a similar CMake list that defines additional
#     include directories.
#
# This macro appends link libraries to 'OPTIONAL_LIBS' and include
# directories to 'OPTIONAL_INCLUDES'.
#
# Input:
# 'targets' may be a CMake list of targets or a single target.
#     It must be quoted if multiple targets are to be added in
#     one call (see examples below).
#
#######################################################################

macro(append_optional_libs targets)
  foreach(_target ${targets})
    get_target_property(_link_libraries ${_target} INTERFACE_LINK_LIBRARIES)
    get_target_property(_include_dirs ${_target} INTERFACE_INCLUDE_DIRECTORIES)
    list(APPEND OPTIONAL_LIBS ${_link_libraries})
    if(_include_dirs)
      list(APPEND OPTIONAL_INCLUDES ${_include_dirs})
    endif()
  endforeach()
  unset(_target)
  unset(_link_libraries)
  unset(_include_dirs)
endmacro()

set(OPTIONAL_LIBS)
set(OPTIONAL_INCLUDES)

if(LIB_dl)
  list(APPEND OPTIONAL_LIBS ${LIB_dl})
endif(LIB_dl)

if(USE_THREADS)
  list(APPEND OPTIONAL_LIBS ${CMAKE_THREAD_LIBS_INIT})
endif(USE_THREADS)

if(FLTK_USE_X11)
  list(APPEND OPTIONAL_LIBS ${X11_LIBRARIES})
endif(FLTK_USE_X11)

if(WIN32)
  list(APPEND OPTIONAL_LIBS comctl32 ws2_32)
  if(USE_GDIPLUS)
    list(APPEND OPTIONAL_LIBS gdiplus)
  endif()
endif(WIN32)

if(HAVE_XINERAMA)
  list(APPEND OPTIONAL_LIBS ${X11_Xinerama_LIB})
endif(HAVE_XINERAMA)

if(HAVE_XFIXES)
  list(APPEND OPTIONAL_LIBS ${X11_Xfixes_LIB})
endif(HAVE_XFIXES)

if(HAVE_XCURSOR)
  list(APPEND OPTIONAL_LIBS ${X11_Xcursor_LIB})
endif(HAVE_XCURSOR)

if(HAVE_XRENDER)
  list(APPEND OPTIONAL_LIBS ${X11_Xrender_LIB})
endif(HAVE_XRENDER)

if(USE_PANGO)
  list(APPEND OPTIONAL_LIBS ${HAVE_LIB_PANGO})
  append_optional_libs(PkgConfig::PANGOCAIRO)
  list(APPEND OPTIONAL_LIBS ${HAVE_LIB_GOBJECT})
  if(USE_PANGOXFT)
    append_optional_libs(PkgConfig::PANGOXFT)
  endif(USE_PANGOXFT)
endif(USE_PANGO)

if(USE_XFT)
  list(APPEND OPTIONAL_LIBS ${X11_Xft_LIB})
  if(LIB_fontconfig)
    list(APPEND OPTIONAL_LIBS ${LIB_fontconfig})
  endif(LIB_fontconfig)
endif(USE_XFT)

if(UNIX AND FLTK_BACKEND_WAYLAND)
  pkg_get_variable(PROTOCOLS wayland-protocols pkgdatadir)
  # replace "//" with "/"
  string(REPLACE "//" "/" PROTOCOLS ${PROTOCOLS})

  # The following variable is used for finding required files and to terminate
  # the build if one or more files are not found. For user convenience
  # the build is terminated after *all* files have been checked.
  set(STOP_REQUIRED FALSE)

  set(INFILE ${PROTOCOLS}/stable/xdg-shell/xdg-shell.xml)
  fl_check_required_file(STOP_REQUIRED ${INFILE})
  add_custom_command(
    OUTPUT xdg-shell-protocol.c xdg-shell-client-protocol.h
    COMMAND wayland-scanner private-code  ${INFILE} xdg-shell-protocol.c
    COMMAND wayland-scanner client-header ${INFILE} xdg-shell-client-protocol.h
    DEPENDS ${INFILE}
    VERBATIM
  )
  list(APPEND STATIC_FILES "xdg-shell-protocol.c")
  list(APPEND SHARED_FILES "xdg-shell-protocol.c")

  if(NOT USE_SYSTEM_LIBDECOR)
    set(INFILE ${PROTOCOLS}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml)
    fl_check_required_file(STOP_REQUIRED ${INFILE})
    add_custom_command(
      OUTPUT xdg-decoration-protocol.c xdg-decoration-client-protocol.h
      COMMAND wayland-scanner private-code  ${INFILE} xdg-decoration-protocol.c
      COMMAND wayland-scanner client-header ${INFILE} xdg-decoration-client-protocol.h
      DEPENDS ${INFILE}
      VERBATIM
    )
    list(APPEND STATIC_FILES "xdg-decoration-protocol.c")
    list(APPEND SHARED_FILES "xdg-decoration-protocol.c")
  endif(NOT USE_SYSTEM_LIBDECOR)

  set(INFILE ${FLTK_SOURCE_DIR}/libdecor/build/gtk-shell.xml)
  add_custom_command(
    OUTPUT gtk-shell-protocol.c gtk-shell-client-protocol.h
    COMMAND wayland-scanner private-code  ${INFILE} gtk-shell-protocol.c
    COMMAND wayland-scanner client-header ${INFILE} gtk-shell-client-protocol.h
    DEPENDS ${INFILE}
    VERBATIM
  )
  list(APPEND STATIC_FILES "gtk-shell-protocol.c")
  list(APPEND SHARED_FILES "gtk-shell-protocol.c")

  set(INFILE ${PROTOCOLS}/unstable/text-input/text-input-unstable-v3.xml)
  fl_check_required_file(STOP_REQUIRED ${INFILE})
  add_custom_command(
    OUTPUT text-input-protocol.c text-input-client-protocol.h
    COMMAND wayland-scanner private-code  ${INFILE} text-input-protocol.c
    COMMAND wayland-scanner client-header ${INFILE} text-input-client-protocol.h
    DEPENDS ${INFILE}
    VERBATIM
  )
  list(APPEND STATIC_FILES "text-input-protocol.c")
  list(APPEND SHARED_FILES "text-input-protocol.c")

  if(STOP_REQUIRED)
    message(FATAL_ERROR "*** Terminating: one or more required file(s) were not found. ***")
  endif()

  if(FLTK_USE_GL)
    append_optional_libs("PkgConfig::WLD_EGL;PkgConfig::PKG_EGL")
  endif(FLTK_USE_GL)

  if(USE_SYSTEM_LIBDECOR)
    append_optional_libs(PkgConfig::SYSTEM_LIBDECOR)
  elseif(GTK_FOUND AND FLTK_USE_LIBDECOR_GTK)
    append_optional_libs(PkgConfig::GTK)
  endif()

  append_optional_libs("PkgConfig::WLDCURSOR;PkgConfig::WLDCLIENT;PkgConfig::XKBCOMMON")

  if(DBUS_FOUND)
    append_optional_libs(PkgConfig::DBUS)
  endif()

endif(UNIX AND FLTK_BACKEND_WAYLAND)

list(REMOVE_DUPLICATES OPTIONAL_LIBS)
list(REMOVE_DUPLICATES OPTIONAL_INCLUDES)

#######################################################################

fl_add_library(fltk STATIC "${STATIC_FILES}")
target_link_libraries(fltk PRIVATE ${OPTIONAL_LIBS})
target_include_directories(fltk PRIVATE ${OPTIONAL_INCLUDES})

#######################################################################

if(FLTK_BUILD_FORMS)
  fl_add_library(fltk_forms STATIC "${FORMS_FILES}")
  target_link_libraries(fltk_forms PUBLIC fltk::fltk)
endif()

#######################################################################

if(0)
  message(STATUS "----------------------   fltk_images   -----------------------")
  fl_debug_var(FLTK_JPEG_LIBRARIES)
  fl_debug_var(FLTK_PNG_LIBRARIES)
  fl_debug_var(FLTK_ZLIB_LIBRARIES)
  fl_debug_var(FLTK_IMAGE_LIBRARIES)
  message(STATUS "----------------------   fltk_images   -----------------------")
endif()

fl_add_library(fltk_images STATIC "${IMGCPPFILES}")
target_link_libraries(fltk_images PUBLIC  fltk::fltk)
target_link_libraries(fltk_images PRIVATE ${FLTK_IMAGE_LIBRARIES})

if(FLTK_USE_BUNDLED_JPEG)
  target_include_directories(fltk_images PUBLIC
    $<BUILD_INTERFACE:${FLTK_SOURCE_DIR}/jpeg>
    $<INSTALL_INTERFACE:include>)
endif()

#######################################################################

if(FLTK_USE_GL)
  fl_add_library(fltk_gl STATIC "${GLCPPFILES};${GL_HEADER_FILES};${GL_DRIVER_HEADER_FILES}")
  target_link_libraries(fltk_gl PUBLIC ${OPENGL_LIBRARIES} fltk::fltk)
  target_include_directories(fltk_gl PUBLIC ${OPENGL_INCLUDE_DIR} ${OPENGL_INCLUDE_DIRS})

  # Add "optional libs" (FIXME: this can be optimized, we don't need *all* these libs here)
  target_link_libraries(fltk_gl PRIVATE ${OPTIONAL_LIBS})
  target_include_directories(fltk_gl PRIVATE ${OPTIONAL_INCLUDES})

  if(FLTK_OPENGL_GLU_INCLUDE_DIR)
    target_include_directories(fltk_gl PUBLIC ${FLTK_OPENGL_GLU_INCLUDE_DIR})
  endif()
endif(FLTK_USE_GL)

#######################################################################
# Build shared libraries (optional)
#######################################################################
# Shared libraries, part 1: everything except Visual Studio (MSVC)
#######################################################################

if(FLTK_BUILD_SHARED_LIBS AND NOT MSVC)

  fl_add_library(fltk SHARED "${SHARED_FILES}")
  target_link_libraries(fltk-shared PRIVATE ${OPTIONAL_LIBS})
  target_include_directories(fltk-shared PRIVATE ${OPTIONAL_INCLUDES})

  ###################################################################

  if(FLTK_BUILD_FORMS)
    fl_add_library(fltk_forms SHARED "${FORMS_FILES}")
    target_link_libraries(fltk_forms-shared PUBLIC fltk::fltk-shared)
  endif()

  ###################################################################

  fl_add_library(fltk_images SHARED "${IMGCPPFILES}")
  target_link_libraries(fltk_images-shared PUBLIC fltk::fltk-shared)

  target_link_libraries(fltk_images PUBLIC ${FLTK_JPEG_LIBRARIES})
  target_link_libraries(fltk_images PUBLIC ${FLTK_PNG_LIBRARIES})
  target_link_libraries(fltk_images PUBLIC ${FLTK_IMAGE_LIBRARIES})

  if(FLTK_USE_BUNDLED_JPEG)
    target_include_directories(fltk_images-shared PUBLIC
      $<BUILD_INTERFACE:${FLTK_SOURCE_DIR}/jpeg>
      # $<INSTALL_INTERFACE:include>
    )
  endif()

  if(FLTK_USE_BUNDLED_JPEG)
    target_link_libraries(fltk_images-shared PUBLIC fltk::jpeg-shared)
  else()
    target_link_libraries(fltk_images-shared PUBLIC ${JPEG_LIBRARIES})
  endif()

  if(FLTK_USE_BUNDLED_PNG)
    target_link_libraries(fltk_images-shared PUBLIC fltk::png-shared)
  else()
    target_link_libraries(fltk_images-shared PUBLIC ${PNG_LIBRARIES})
  endif()

  ###################################################################

  if(FLTK_USE_GL)
    fl_add_library(fltk_gl SHARED "${GLCPPFILES};${GL_HEADER_FILES};${GL_DRIVER_HEADER_FILES}")
    target_link_libraries(fltk_gl-shared PUBLIC ${OPENGL_LIBRARIES} fltk::fltk-shared)
    target_include_directories(fltk_gl-shared PUBLIC ${OPENGL_INCLUDE_DIR} ${OPENGL_INCLUDE_DIRS})

    # Add "optional libs" (FIXME: this can be optimized, we don't need *all* these libs here)
    target_link_libraries(fltk_gl-shared PRIVATE ${OPTIONAL_LIBS})
    target_include_directories(fltk_gl-shared PRIVATE ${OPTIONAL_INCLUDES})

    if(FLTK_OPENGL_GLU_INCLUDE_DIR)
      target_include_directories(fltk_gl-shared PUBLIC ${FLTK_OPENGL_GLU_INCLUDE_DIR})
    endif()
  endif(FLTK_USE_GL)

endif(FLTK_BUILD_SHARED_LIBS AND NOT MSVC)

#######################################################################
# Shared libraries, part 2: Visual Studio (MSVC)
#######################################################################

# Note to devs: As of June 2020 we can't build separate shared libs
# (DLL's) under Windows with Visual Studio (MSVC) but we can build one
# big DLL that comprises all FLTK and optional PNG, JPEG, and ZLIB libs.
# The reason is the common DLL linkage (FL_EXPORT) for all libs. This
# might be changed in the future but it would require a lot of work.
# AlbrechtS

if(FLTK_BUILD_SHARED_LIBS AND MSVC)

  set(SOURCES ${SHARED_FILES} ${FORMS_FILES} ${IMGCPPFILES})
  if(OPENGL_FOUND)
    list(APPEND SOURCES ${GLCPPFILES} ${GL_HEADER_FILES} ${GL_DRIVER_HEADER_FILES})
  endif(OPENGL_FOUND)

  fl_add_library(fltk SHARED "${SOURCES}")
  target_link_libraries(fltk-shared PRIVATE ${OPTIONAL_LIBS})
  target_include_directories(fltk-shared PRIVATE ${OPTIONAL_INCLUDES})

  if(FLTK_USE_BUNDLED_JPEG)
    target_link_libraries(fltk-shared PUBLIC fltk::jpeg-shared)
  else()
    target_link_libraries(fltk-shared PUBLIC ${FLTK_JPEG_LIBRARIES})
  endif()

  if(FLTK_USE_BUNDLED_PNG)
    target_link_libraries(fltk-shared PUBLIC fltk::png-shared)
  else()
    target_link_libraries(fltk-shared PUBLIC ${FLTK_PNG_LIBRARIES})
  endif()

  if(OPENGL_FOUND)
    target_link_libraries(fltk-shared PUBLIC ${OPENGL_LIBRARIES})
    target_include_directories(fltk-shared PUBLIC ${OPENGL_INCLUDE_DIR} ${OPENGL_INCLUDE_DIRS})
    if(FLTK_OPENGL_GLU_INCLUDE_DIR)
      target_include_directories(fltk-shared PUBLIC ${FLTK_OPENGL_GLU_INCLUDE_DIR})
    endif()
  endif(OPENGL_FOUND)

endif(FLTK_BUILD_SHARED_LIBS AND MSVC)

#######################################################################

set(FLTK_LIBRARIES ${FLTK_LIBRARIES} PARENT_SCOPE)
set(FLTK_LIBRARIES_SHARED ${FLTK_LIBRARIES_SHARED} PARENT_SCOPE)
