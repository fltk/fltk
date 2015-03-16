
# source files for png
set(PNG_SRCS
  png.c
  pngerror.c
  pngget.c
  pngmem.c
  pngpread.c
  pngread.c
  pngrio.c
  pngrtran.c
  pngrutil.c
  pngset.c
  pngtrans.c
  pngwio.c
  pngwrite.c
  pngwtran.c
  pngwutil.c
)

#######################################################################
FL_ADD_LIBRARY(fltk_png STATIC "${PNG_SRCS}")
# install the png headers
install(FILES png.h;pngconf.h;pnglibconf.h
    DESTINATION ${FLTK_INCLUDEDIR}/FL/images
)

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
