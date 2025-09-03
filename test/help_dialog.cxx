//
// Fl_Help_Dialog test program.
//
// Copyright 1999-2010 by Easy Software Products.
// Copyright 2011-2017 by Bill Spitzak and others.
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
// Contents:
//
//   main() - Display the help GUI...
//

//
// Include necessary headers...
//

#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_GIF_Image.H>
#include <FL/filename.H>        /* FL_PATH_MAX */
#include <FL/Fl.H>              /* Fl::first_window(), etc */
#include <string.h>             /* strcpy(), etc */

static void cb_refresh(void *d) {
  // trigger a redraw of the window to see animated GIF's
  if (Fl::first_window())
    Fl::first_window()->redraw();
  Fl::repeat_timeout(1./10, cb_refresh, d);
}

//
// 'main()' - Display the help GUI...
//

int                             // O - Exit status
main(int  argc,                 // I - Number of command-line arguments
     char *argv[])              // I - Command-line arguments
{
  Fl_GIF_Image::animate = true; // create animated shared .GIF images
  Fl_Help_Dialog *help = new Fl_Help_Dialog;
  int i;
  Fl::args_to_utf8(argc, argv); // for MSYS2/MinGW
  if (!Fl::args(argc, argv, i)) Fl::fatal(Fl::help);
  const char *fname = (i < argc) ? argv[i] : "help_dialog.html";

  help->load(fname); // TODO: add error check (when load() returns int instead of void)

  help->show(1, argv);

  Fl::add_timeout(1./10, cb_refresh, help); // to animate GIF's
  Fl::run();

  delete help;

  return 0;
}
