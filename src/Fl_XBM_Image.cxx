//
// "$Id$"
//
// Fl_XBM_Image routines.
//
// Copyright 1997-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//
// Contents:
//
//   Fl_XBM_Image::Fl_XBM_Image() - Load an XBM file.
//

//
// Include necessary header files...
//

#include <FL/Fl.H>
#include <FL/Fl_XBM_Image.H>
#include <stdio.h>
#include <stdlib.h>
#include <FL/fl_utf8.h>
#include "flstring.h"

//
// 'Fl_XBM_Image::Fl_XBM_Image()' - Load an XBM file.
//

/**  
  The constructor loads the named XBM file from the given name filename.

  The destructor frees all memory and server resources that are used by
  the image.
*/
Fl_XBM_Image::Fl_XBM_Image(const char *name) : Fl_Bitmap((const char *)0,0,0) {
  FILE	*f;
  uchar	*ptr;

  if ((f = fl_fopen(name, "rb")) == NULL) return;

  char buffer[1024];
  char junk[1024];
  int wh[2]; // width and height
  int i;
  for (i = 0; i<2; i++) {
    for (;;) {
      if (!fgets(buffer,1024,f)) {
        fclose(f);
	return;
      }
      int r = sscanf(buffer,"#define %s %d",junk,&wh[i]);
      if (r >= 2) break;
    }
  }

  // skip to data array:
  for (;;) {
    if (!fgets(buffer,1024,f)) {
      fclose(f);
      return;
    }
    if (!strncmp(buffer,"static ",7)) break;
  }

  // Allocate memory...
  w(wh[0]);
  h(wh[1]);

  int n = ((wh[0]+7)/8)*wh[1];
  array = new uchar[n];

  // read the data:
  for (i = 0, ptr = (uchar *)array; i < n;) {
    if (!fgets(buffer,1024,f)) {
      fclose(f);
      return;
    }
    const char *a = buffer;
    while (*a && i<n) {
      unsigned int t;
      if (sscanf(a," 0x%x",&t)>0) {
        *ptr++ = (uchar)t;
	i ++;
      }
      while (*a && *a++ != ',') {/*empty*/}
    }
  }

  fclose(f);
}


//
// End of "$Id$".
//
