//
// "$Id$"
//
// Fl_Help_Dialog test program.
//
// Copyright 1999-2010 by Easy Software Products.
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

//
// 'main()' - Display the help GUI...
//

int				// O - Exit status
main(int  argc,			// I - Number of command-line arguments
     char *argv[])		// I - Command-line arguments
{
  Fl_Help_Dialog	*help;		// Help dialog


  help = new Fl_Help_Dialog;

  
#ifdef __APPLE__

    char buf[2048];
    strcpy(buf, argv[0]);
    char *slash = strrchr(buf, '/');
    if (slash)
      strcpy(slash, "/../Resources/help-test.html");
    help->load(buf);
  
#else
  
  if (argc <= 1)
    help->load("help-test.html");
  else
    help->load(argv[1]);
  
#endif
  
  help->show(1, argv);

  Fl::run();

  delete help;

  return (0);
}


//
// End of "$Id$".
//
