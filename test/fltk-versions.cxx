//
// "$Id$"
//
// Library version test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>

static char version[8][80] = { "","","","","","","","" };

int main(int argc, char **argv) {

  int versions = 0;

  sprintf(version[versions++],"FL_VERSION        = %6.4f",FL_VERSION);
  sprintf(version[versions++],"Fl::version()     = %6.4f  %s",Fl::version(),
    (FL_VERSION == Fl::version()) ? "" : "***");

#ifdef FL_API_VERSION
  sprintf(version[versions++],"FL_API_VERSION    = %6d",FL_API_VERSION);
  sprintf(version[versions++],"Fl::api_version() = %6d  %s",Fl::api_version(),
    (FL_API_VERSION == Fl::api_version()) ? "" : "***");
#endif

#ifdef FL_ABI_VERSION
  sprintf(version[versions++],"FL_ABI_VERSION    = %6d",FL_ABI_VERSION);
  sprintf(version[versions++],"Fl::abi_version() = %6d  %s",Fl::abi_version(),
    (FL_ABI_VERSION == Fl::abi_version()) ? "" : "***");
#endif

#ifdef FLTK_ABI_VERSION
  sprintf(version[versions++],"FLTK_ABI_VERSION  = %6d",FLTK_ABI_VERSION);
  sprintf(version[versions++],"NOTE: FLTK_ABI_VERSION is deprecated.\n"
			      "Please use FL_ABI_VERSION instead !");
#endif

  for (int i=0; i<versions; i++) {
    printf("%s\n",version[i]);
  }

#ifdef FL_ABI_VERSION
  if (FL_ABI_VERSION != Fl::abi_version()) {
    printf("*** FLTK ABI version mismatch: headers = %d, lib = %d ***\n",
      FL_ABI_VERSION, Fl::abi_version());
    fflush(stdout);
    fl_message("*** FLTK ABI version mismatch: headers = %d, lib = %d ***",
      FL_ABI_VERSION, Fl::abi_version());
    // exit(1);
  }
#endif

  Fl_Window *window = new Fl_Window(670,300);

  Fl_Box *box[8];
  for (int i=0; i<4; i++) {
    box[2*i]   = new Fl_Box( 10,40+40*i,320,30,version[2*i]);
    box[2*i+1] = new Fl_Box(340,40+40*i,320,30,version[2*i+1]);
  }

  for (int i=0; i<8; i++) {
    box[i]->labelfont(FL_COURIER);
    box[i]->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  }

  window->end();
  window->show(argc, argv);
  return Fl::run();
}

//
// End of "$Id$".
//
