
# source files for zlib
set (ZLIB_SRCS
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

# Suppress some Visual Studio compiler warnings
set (msvc_warnings /wd4267 /wd4996)

#######################################################################
# Build the static library
#######################################################################

FL_ADD_LIBRARY (fltk_z STATIC "${ZLIB_SRCS}")

if (MSVC)
  target_compile_options (fltk_z PRIVATE ${msvc_warnings})
endif (MSVC)

#######################################################################
# Build the shared library (optional)
#######################################################################

if (OPTION_BUILD_SHARED_LIBS)
  FL_ADD_LIBRARY (fltk_z SHARED "${ZLIB_SRCS}")

  if (MSVC)
    target_compile_options (fltk_z_SHARED PRIVATE ${msvc_warnings})
  endif (MSVC)

endif (OPTION_BUILD_SHARED_LIBS)

#######################################################################
# Install the library headers
#######################################################################

install (FILES zconf.h zlib.h zutil.h
         DESTINATION ${FLTK_INCLUDEDIR}/FL/images
)
