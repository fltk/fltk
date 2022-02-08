
include_directories (${PKG_CAIRO_INCLUDE_DIRS})

# source files for cairo
set (cairo_SRCS Fl_Cairo.cxx)

#######################################################################
FL_ADD_LIBRARY (fltk_cairo STATIC "${cairo_SRCS}")

#######################################################################
# Build shared library (optional)
#######################################################################

if (OPTION_BUILD_SHARED_LIBS)

  FL_ADD_LIBRARY (fltk_cairo SHARED "${cairo_SRCS}")
  target_link_libraries (fltk_cairo_SHARED fltk_SHARED ${PKG_CAIRO_LIBRARIES})
  fl_target_link_directories (fltk_cairo_SHARED PRIVATE "${PKG_CAIRO_LIBRARY_DIRS}")

endif (OPTION_BUILD_SHARED_LIBS)

#######################################################################
