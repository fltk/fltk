//
// "$Id$"
//
// CMake platform tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

//
// The platform tests in this file are invoked by the CMake macro
// PERFORM_CMAKE_TEST in CMakeLists.txt (or maybe other files).
//
// Each platform test in this file must begin and end with
//   #ifdef FOO
//     ...
//   #endif
// where FOO is the compiler macro to be tested/defined in config.h.
//
// It must contain a main() function and return 0 if the test succeeded.
//

#ifdef HAVE_LIBZ

#include <zlib.h>

int main() {
  unsigned long compressedSize = 0;
  unsigned char cd[100];
  const unsigned char ud[100] = "";
  unsigned long uncompressedSize = 0;

  // Call zlib's compress function.
  if (compress(cd, &compressedSize, ud, uncompressedSize) != Z_OK) {
    return 0;
  }
  return 1;
}

#endif


#ifdef HAVE_LIBJPEG

#include <stdio.h>
#include <jpeglib.h>

int main() {
  struct jpeg_decompress_struct cinfo;
  jpeg_create_decompress(&cinfo);
  jpeg_read_header(&cinfo, TRUE);
  return 1;
}

#endif


#ifdef HAVE_LIBPNG
#include <png.h>
int main()
{
  png_structp png_ptr = png_create_read_struct
	(PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
	 NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);

  return 0;
}
#endif


// This one is probably not used:
#ifdef HAVE_PNG_H
#include <png.h>
int main() { return 0;}
#endif


#ifdef HAVE_PNG_GET_VALID
#include <png.h>

int main() {
  png_structp png_ptr = png_create_read_struct
	(PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
	 NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS);
  return 0;
}
#endif


#ifdef HAVE_PNG_SET_TRNS_TO_ALPHA
#include <png.h>

int main() {
  png_structp png_ptr = png_create_read_struct
	(PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
	 NULL, NULL);
  png_set_tRNS_to_alpha(png_ptr);
  return 0;
}
#endif


#ifdef HAVE_SCANDIR_POSIX
#include <dirent.h>

int func (const char *d, dirent ***list, void *sort) {
  int n = scandir(d, list, 0, (int(*)(const dirent **, const dirent **))sort);
}

int main() {
  return 0;
}
#endif

//
// End of "$Id$".
//
