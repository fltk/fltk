
# source files for png
set(PNG_SRCS
  pngget.c    pngrio.c    pngwrite.c
  png.c       pngmem.c    pngrtran.c  pngtrans.c  pngwtran.c
  pngerror.c  pngpread.c  pngrutil.c  pngwutil.c
  pngread.c   pngset.c   pngwio.c
)

#######################################################################
FL_ADD_LIBRARY(fltk_png STATIC "${PNG_SRCS}")

if(OPTION_USE_SYSTEM_ZLIB)
    target_link_libraries(fltk_png ${FLTK_ZLIB_LIBRARIES})
else()
    target_link_libraries(fltk_png fltk_z)
endif(OPTION_USE_SYSTEM_ZLIB)

#######################################################################
if(OPTION_BUILD_SHARED_LIBS)
#######################################################################
FL_ADD_LIBRARY(fltk_png SHARED "${PNG_SRCS}")

if(OPTION_USE_SYSTEM_ZLIB)
    target_link_libraries(fltk_png_SHARED ${FLTK_ZLIB_LIBRARIES})
else()
    target_link_libraries(fltk_png_SHARED fltk_z_SHARED)
endif(OPTION_USE_SYSTEM_ZLIB)

#######################################################################
endif(OPTION_BUILD_SHARED_LIBS)
#######################################################################
