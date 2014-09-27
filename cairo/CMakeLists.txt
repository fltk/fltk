
include_directories(${PKG_CAIRO_INCLUDE_DIRS})

# source files for cairo
set(cairo_SRCS Fl_Cairo.cxx)

#######################################################################
FL_ADD_LIBRARY(fltk_cairo STATIC "${cairo_SRCS}")

#######################################################################
if(OPTION_BUILD_SHARED_LIBS)
#######################################################################
FL_ADD_LIBRARY(fltk_cairo SHARED "${cairo_SRCS}")

#######################################################################
endif(OPTION_BUILD_SHARED_LIBS)
#######################################################################
