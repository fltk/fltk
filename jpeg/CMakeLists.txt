project(FLTKJPEG)

include_regular_expression("^j.*[.][c|h]$")

include_directories(${FLTKJPEG_SOURCE_DIR})
include_directories(${FLTKJPEG_BINARY_DIR})

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

#######################################################################
add_library(fltk_jpeg ${systemdependent_SRCS} ${common_SRCS} ${compression_SRCS} ${decompression_SRCS})

if(MSVC)
   set_target_properties(fltk_jpeg
      PROPERTIES
         OUTPUT_NAME fltkjpeg
         DEBUG_OUTPUT_NAME fltkjpegd
   )
   if(OPTION_LARGE_FILE)
      set_target_properties(fltk_jpeg PROPERTIES LINK_FLAGS /LARGEADDRESSAWARE)
   endif(OPTION_LARGE_FILE)
endif(MSVC)

install(TARGETS fltk_jpeg
   EXPORT fltk-install
   DESTINATION ${PREFIX_LIB}
)
