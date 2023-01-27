//
// fl_open_uri() code for FLTK.
//
// Test with:
//
//    gcc -I/fltk/dir -I/fltk/dir/src -DTEST -o fl_open_uri fl_open_uri.cxx -lfltk
//
// Copyright 2003-2010 by Michael R Sweet
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

//
// Include necessary headers...
//

#include <config.h>
#include <FL/filename.H>
#include <FL/Fl.H>
#include "Fl_System_Driver.H"
#include <stdio.h>
#include <stdlib.h>
#include "flstring.h"

/** \addtogroup filenames
 \{ */

/**
 Opens the specified Uniform Resource Identifier (URI).
 Uses an operating-system dependent program or interface. For URIs
 using the "ftp", "http", or "https" schemes, the system default web
 browser is used to open the URI, while "mailto" and "news" URIs are
 typically opened using the system default mail reader and "file" URIs
 are opened using the file system navigator.

 On success, the (optional) msg buffer is filled with the command that
 was run to open the URI; on Windows, this will always be "open uri".

 On failure, the msg buffer is filled with an English error message.

 \note
 \b Platform \b Specific \b Issues: \b Windows \n
 With "file:" based URIs on Windows, you may encounter issues with
 anchors being ignored. Example: "file:///c:/some/index.html#anchor"
 may open in the browser without the "#anchor" suffix. The behavior
 seems to vary across different Windows versions. Workaround: open a link
 to a separate html file that redirects to the desired "file:" URI.

 \b Example
 \code
 #include <FL/filename.H>
 [..]
 char errmsg[512];
 if ( !fl_open_uri("http://google.com/", errmsg, sizeof(errmsg)) ) {
     char warnmsg[768];
     sprintf(warnmsg, "Error: %s", errmsg);
     fl_alert(warnmsg);
 }
 \endcode

 \param uri The URI to open
 \param msg Optional buffer which contains the command or error message
 \param msglen Length of optional buffer
 \return 1 on success, 0 on failure
 */
int fl_open_uri(const char *uri, char *msg, int msglen) {
  // Supported URI schemes...
  static const char * const schemes[] = {
    "file://",
    "ftp://",
    "http://",
    "https://",
    "mailto:",
    "news://",
    NULL
  };

  // Validate the URI scheme...
  int i;
  for (i = 0; schemes[i]; i ++)
    if (!strncmp(uri, schemes[i], strlen(schemes[i])))
      break;

  if (!schemes[i]) {
    if (msg) {
      char scheme[255];
      if (sscanf(uri, "%254[^:]", scheme) == 1) {
        snprintf(msg, msglen, "URI scheme \"%s\" not supported.", scheme);
      } else {
        snprintf(msg, msglen, "Bad URI \"%s\"", uri);
      }
    }

    return 0;
  }
  return Fl::system_driver()->open_uri(uri, msg, msglen);
}

/** Decodes a URL-encoded string.

 In a Uniform Resource Identifier (URI), all non-ASCII bytes and several others (e.g., '<', '%', ' ')
 are URL-encoded using 3 bytes by "%XY" where XY is the hexadecimal value of the byte. This function
 decodes the URI restoring its original UTF-8 encoded content. Decoding is done in-place.
 */
void fl_decode_uri(char *uri)
{
  char *last = uri + strlen(uri);
  while (uri < last-2) {
    if (*uri == '%') {
      int h;
      if ( sscanf(uri+1, "%2X", &h) != 1 ) break;
      *uri = h;
      memmove(uri+1, uri+3, last - (uri+2));
      last -= 2;
    }
    uri++;
  }
}

/**   @} */

#ifdef TEST
//
// Test code...
//

// Open the URI on the command-line...
int main(int argc, char **argv) {
  char msg[1024];


  if (argc != 2) {
    puts("Usage: fl_open_uri URI");
    return 1;
  }

  if (!fl_open_uri(argv[1], msg, sizeof(msg))) {
    puts(msg);
    return 1;
  } else return 0;
}
#endif // TEST
