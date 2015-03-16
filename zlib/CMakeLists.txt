
# source files for zlib
set(ZLIB_SRCS
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    inflate.c
    infback.c
    inftrees.c
    inffast.c
    trees.c
    uncompr.c
    zutil.c
)

#######################################################################
FL_ADD_LIBRARY(fltk_z STATIC "${ZLIB_SRCS}")
# install the zlib headers
install(FILES zconf.h;zlib.h;zutil.h
    DESTINATION ${FLTK_INCLUDEDIR}/FL/images
)

#######################################################################
if(OPTION_BUILD_SHARED_LIBS)
#######################################################################
FL_ADD_LIBRARY(fltk_z SHARED "${ZLIB_SRCS}")

#######################################################################
endif(OPTION_BUILD_SHARED_LIBS)
#######################################################################
