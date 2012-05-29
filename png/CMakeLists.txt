project(FLTKPNG)
include_regular_expression("^png.*[.][c|h]$")

include_directories(${FLTKZLIB_SOURCE_DIR})
include_directories(${ZLIB_INCLUDE_DIR})

include_directories(${FLTKPNG_SOURCE_DIR})
include_directories(${FLTKPNG_BINARY_DIR})

# source files for png
set(PNG_SRCS
  pngget.c    pngrio.c    pngwrite.c
  png.c       pngmem.c    pngrtran.c  pngtrans.c  pngwtran.c
  pngerror.c  pngpread.c  pngrutil.c  pngwutil.c
  pngread.c   pngset.c   pngwio.c
)

#######################################################################
add_library(fltk_png ${PNG_SRCS})

if(MSVC)
   set_target_properties(fltk_png PROPERTIES OUTPUT_NAME fltkpng)
   if(OPTION_LARGE_FILE)
      set_target_properties(fltk_png PROPERTIES LINK_FLAGS /LARGEADDRESSAWARE)
   endif(OPTION_LARGE_FILE)
else()
   set_target_properties(fltk_png PROPERTIES OUTPUT_NAME fltk_png)
endif(MSVC)

target_link_libraries(fltk_png ${FLTK_ZLIB_LIBRARIES})

install(TARGETS fltk_png
   EXPORT fltk-install
   DESTINATION ${PREFIX_LIB}
)
