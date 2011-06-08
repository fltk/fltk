project(FLTKZLIB)
include_regular_expression(regex_match "[.][h|c]$")

include_directories(${FLTKZLIB_SOURCE_DIR})
include_directories(${FLTKZLIB_BINARY_DIR})

# source files for zlib
set(ZLIB_SRCS
   adler32.c compress.c crc32.c deflate.c 
   gzclose.c gzlib.c gzread.c gzwrite.c
   inffast.c infback.c inflate.c inftrees.c 
   trees.c uncompr.c zutil.c
)

#######################################################################
add_library(fltk_z ${ZLIB_SRCS})

if(MSVC)
   set_target_properties(fltk_z
      PROPERTIES
         OUTPUT_NAME zlib
         DEBUG_OUTPUT_NAME zlibd
   )
   if(OPTION_LARGE_FILE)
      set_target_properties(fltk_z PROPERTIES LINK_FLAGS /LARGEADDRESSAWARE)
   endif(OPTION_LARGE_FILE)
endif(MSVC)

install(TARGETS fltk_z
   EXPORT fltk-install
   DESTINATION ${PREFIX_LIB}
)
