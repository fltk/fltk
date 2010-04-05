project(FLTKcairo)

include_directories(${FLTKcairo_SOURCE_DIR})
include_directories(${FLTKcairo_BINARY_DIR})
include_directories(${PKG_CAIRO_INCLUDE_DIRS})

# source files for cairo
set(cairo_SRCS Fl_Cairo.cxx)

#######################################################################
add_library(fltk_cairo ${cairo_SRCS})

if(MSVC)
   set_target_properties(fltk_cairo
      PROPERTIES
         OUTPUT_NAME fltkcairo
         DEBUG_OUTPUT_NAME fltkcairod
   )
   if(OPTION_LARGE_FILE)
      set_target_properties(fltk_cairo PROPERTIES LINK_FLAGS /LARGEADDRESSAWARE)
   endif(OPTION_LARGE_FILE)
endif(MSVC)

install(TARGETS fltk_cairo
   EXPORT fltk-install
   DESTINATION ${PREFIX_LIB}
)
