PROJECT(FLTKZLIB)
INCLUDE_REGULAR_EXPRESSION("^(deflate|inf|trees|zconf|zlib|zutil).*$")

INCLUDE_DIRECTORIES(${FLTKZLIB_SOURCE_DIR})
INCLUDE_DIRECTORIES(${FLTKZLIB_BINARY_DIR})

# source files for zlib
SET(ZLIB_SRCS
adler32.c compress.c crc32.c deflate.c gzio.c inffast.c
inflate.c inftrees.c trees.c uncompr.c zutil.c
)

ADD_LIBRARY(fltk_zlib ${ZLIB_SRCS})

