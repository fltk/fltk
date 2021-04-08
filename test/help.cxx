//
// Fl_Help_Dialog test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1999-2010 by Easy Software Products.
// Copyright 2011-2021 by Bill Spitzak and others.
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
#include <FL/filename.H>        /* FL_PATH_MAX */
#include <string.h>             /* strcpy(), etc */

//
// 'main()' - Display the help GUI...
//

int                             // O - Exit status
main(int  argc,                 // I - Number of command-line arguments
     char *argv[])              // I - Command-line arguments
{
  Fl_Help_Dialog *help = new Fl_Help_Dialog;
  int i;
  if (!Fl::args(argc, argv, i)) Fl::fatal(Fl::help);
  const char *fname = (i < argc) ? argv[i] : "help_dialog.html";

  help->load(fname); // TODO: add error check (when load() returns int instead of void)

  help->show(1, argv);

  Fl::run();

  delete help;

  return 0;
}
