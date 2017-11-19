//
// "$Id$"
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
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//
// Contents:
//
//   main() - Display the help GUI...
//

//
// Include necessary headers...
//

#include <FL/Fl_Help_Dialog.H>
#include <FL/filename.H>	/* FL_PATH_MAX */
#include <string.h>		/* strcpy(), etc */

//
// 'main()' - Display the help GUI...
//

int				// O - Exit status
main(int  argc,			// I - Number of command-line arguments
     char *argv[])		// I - Command-line arguments
{
  Fl_Help_Dialog *help = new Fl_Help_Dialog;
  char htmlname[FL_PATH_MAX];
  if (argc > 1) {
    strcpy(htmlname, argv[1]);
  } else {
#ifdef __APPLE__
    // bundled apps do not set the current directory
    strcpy(htmlname, argv[0]);
    char *slash = strrchr(htmlname, '/');
    if (slash) strcpy(slash, "/../Resources/help_dialog.html");
#else
    strcpy(htmlname, "help_dialog.html");
#endif
  }

  help->load(htmlname);	// TODO: add error check (when load() returns int instead of void)

  help->show(1, argv);

  Fl::run();

  delete help;

  return 0;
}

//
// End of "$Id$".
//
