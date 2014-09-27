
# memmgr back ends: compile only one of these into a working library
# (For now, let's use the mode that requires the image fit into memory.
# This is the recommended mode for Win32 anyway.)
set(systemdependent_SRCS jmemnobs.c)

# library object files common to compression and decompression
set(common_SRCS
   jaricom.c jcomapi.c jutils.c jerror.c jmemmgr.c
)

# compression library object files
set(compression_SRCS
   jcapimin.c jcapistd.c jcarith.c jctrans.c jcparam.c jdatadst.c jcinit.c
   jcmaster.c jcmarker.c jcmainct.c jcprepct.c jccoefct.c jccolor.c
   jcsample.c jchuff.c jcdctmgr.c jfdctfst.c jfdctflt.c
   jfdctint.c
)

# decompression library object files
set(decompression_SRCS
   jdapimin.c jdapistd.c jdarith.c jdtrans.c jdatasrc.c jdmaster.c
   jdinput.c jdmarker.c jdhuff.c jdmainct.c jdcoefct.c
   jdpostct.c jddctmgr.c jidctfst.c jidctflt.c jidctint.c
   jdsample.c jdcolor.c jquant1.c jquant2.c jdmerge.c
)

list(APPEND BUILD_SRCS "${systemdependent_SRCS};${common_SRCS}")
list(APPEND BUILD_SRCS "${compression_SRCS};${decompression_SRCS}")
#######################################################################
FL_ADD_LIBRARY(fltk_jpeg STATIC "${BUILD_SRCS}")

#######################################################################
if(OPTION_BUILD_SHARED_LIBS)
#######################################################################
FL_ADD_LIBRARY(fltk_jpeg SHARED "${BUILD_SRCS}")

#######################################################################
endif(OPTION_BUILD_SHARED_LIBS)
#######################################################################
