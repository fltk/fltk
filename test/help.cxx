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


#ifdef USING_XCODE
#include <ApplicationServices/ApplicationServices.h>
void set_app_dir() {
  char app_path[2048];
  CFBundleRef app = CFBundleGetMainBundle();
  CFURLRef url = CFBundleCopyBundleURL(app);    
  CFStringRef cc_app_path = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
  CFStringGetCString(cc_app_path, app_path, 2048, kCFStringEncodingUTF8);
  if (*app_path) {
    char *n = strrchr(app_path, '/');
    if (n) {
      *n = 0;
      chdir(app_path);
    }
  }
}
#endif


//
// 'main()' - Display the help GUI...
//

int				// O - Exit status
main(int  argc,			// I - Number of command-line arguments
     char *argv[])		// I - Command-line arguments
{
  Fl_Help_Dialog	*help;		// Help dialog


  help = new Fl_Help_Dialog;

  int argn = 1;
  
#ifdef USING_XCODE
  
  if (argc>argn && strncmp(argv[1], "-psn_", 5)==0) argn++;
  else if (argc>argn && strncmp(argv[1], "-NSDocumentRevisionsDebugMode", 29)==0) argn += 2;
  set_app_dir();
  
  if (argc <= argn)
    help->load("../../../../test/help-test.html");
  else
    help->load(argv[argn]);
  
#else
  
  if (argc <= argn)
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
